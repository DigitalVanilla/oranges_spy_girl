#include <exec/types.h>
#include <graphics/text.h>
#include <proto/graphics.h>
#include <sage/sage_screen.h>
#include <sage/sage_timer.h>
#include <stdarg.h>
#include <stdio.h>

#include "spy_config.h"
#include "spy_debug.h"
#include "spy_log.h"

#define FPS_INTERVAL_US STIM_TICKS
#define SPY_RADIANS_TO_DEGREES 57.2957795130823208768f
#define OVERLAY_GLYPH_SIZE 8
#define OVERLAY_FIRST_CHAR ' '
#define OVERLAY_LAST_CHAR '~'

static UBYTE overlay_glyphs[OVERLAY_LAST_CHAR - OVERLAY_FIRST_CHAR + 1][OVERLAY_GLYPH_SIZE];
static UWORD overlay_font_baseline;
static BOOL overlay_font_ready;

static BOOL SpyDebugCacheGlyphs(const struct TextFont *font)
{
  UWORD character, row, column;
  const UWORD *locations;
  const UBYTE *strike;

  if (font == NULL || (font->tf_Style & FSF_COLORFONT) ||
      font->tf_XSize != OVERLAY_GLYPH_SIZE || font->tf_YSize != OVERLAY_GLYPH_SIZE ||
      font->tf_Baseline >= OVERLAY_GLYPH_SIZE ||
      font->tf_LoChar > OVERLAY_FIRST_CHAR || font->tf_HiChar < OVERLAY_LAST_CHAR ||
      font->tf_CharLoc == NULL || font->tf_CharData == NULL) {
    return FALSE;
  }

  locations = (const UWORD *)font->tf_CharLoc;
  strike = (const UBYTE *)font->tf_CharData;
  // Each strike entry gives a bit offset and width within a row of glyphs.
  for (character = OVERLAY_FIRST_CHAR; character <= OVERLAY_LAST_CHAR; character++) {
    UWORD entry = (character - font->tf_LoChar) * 2;
    ULONG bit_offset = locations[entry];
    UWORD width = locations[entry + 1];

    if (width > OVERLAY_GLYPH_SIZE || bit_offset + width > (ULONG)font->tf_Modulo * 8) {
      return FALSE;
    }
    for (row = 0; row < OVERLAY_GLYPH_SIZE; row++) {
      const UBYTE *source = strike + (ULONG)row * font->tf_Modulo;
      UBYTE bits = 0;
      for (column = 0; column < width; column++) {
        ULONG bit = bit_offset + column;
        if (source[bit >> 3] & (0x80U >> (bit & 7))) {
          bits |= 0x80U >> column;
        }
      }
      overlay_glyphs[character - OVERLAY_FIRST_CHAR][row] = bits;
    }
  }

  overlay_font_baseline = font->tf_Baseline;
  return TRUE;
}

BOOL SpyDebugConfigureOverlay(void)
{
  UBYTE topaz_name[] = "topaz.font";
  struct TextAttr attr = { topaz_name, OVERLAY_GLYPH_SIZE, FS_NORMAL, FPF_ROMFONT };
  struct TextFont *topaz = OpenFont(&attr);
  SAGE_Screen *screen;

  overlay_font_ready = SpyDebugCacheGlyphs(topaz);
  if (topaz != NULL) {
    CloseFont(topaz);
  }
  if (!overlay_font_ready) {
    screen = SAGE_GetScreen();
    if (screen != NULL) {
      overlay_font_ready = SpyDebugCacheGlyphs(screen->screen_buffer.work_rastport.Font);
    }
  }
  return overlay_font_ready;
}

static void SpyDebugPrintLine(SAGE_Bitmap *bitmap, ULONG baseline, UWORD color, const char *format, ...)
{
  char text[96];
  va_list args;
  LONG y = (LONG)baseline - overlay_font_baseline;
  ULONG x;
  int written;
  const char *character;

  va_start(args, format);
  written = vsnprintf(text, sizeof(text), format, args);
  va_end(args);
  if (written < 0) {
    return;
  }

  x = SPY_DEBUG_SCREEN_X;
  for (character = text; *character != '\0' && x + OVERLAY_GLYPH_SIZE <= bitmap->width;
       character++, x += OVERLAY_GLYPH_SIZE) {
    UBYTE value = (UBYTE)*character;
    const UBYTE *glyph;
    UWORD row, column;

    if (value < OVERLAY_FIRST_CHAR || value > OVERLAY_LAST_CHAR) {
      value = '?';
    }
    if (value == ' ') {
      continue;
    }
    glyph = overlay_glyphs[value - OVERLAY_FIRST_CHAR];
    for (row = 0; row < OVERLAY_GLYPH_SIZE; row++) {
      LONG screen_y = y + row;
      UBYTE bits = glyph[row];
      UWORD *pixels;

      if (bits == 0 || screen_y < 0 || (ULONG)screen_y >= bitmap->height) {
        continue;
      }
      pixels = (UWORD *)((UBYTE *)bitmap->bitmap_buffer + (ULONG)screen_y * bitmap->bpr) + x;
      for (column = 0; bits != 0; column++, bits <<= 1) {
        if (bits & 0x80U) {
          pixels[column] = color;
        }
      }
    }
  }
}

UWORD SpyUpdateFpsCounter(ULONG frame_elapsed_us, UWORD *frame_count, ULONG *elapsed_time, UWORD fps)
{
  (*frame_count)++;
  *elapsed_time += frame_elapsed_us;
  if (*elapsed_time >= FPS_INTERVAL_US) {
    fps = (UWORD)((*frame_count * (ULONG)STIM_TICKS + *elapsed_time / 2) / *elapsed_time);
    *frame_count = 0;
    *elapsed_time = 0;
  }

  return fps;
}

static void GetCameraCoordParts(float value, char *sign, LONG *whole, LONG *frac)
{
  LONG centis;

  if (value < 0.0f) {
    *sign = '-';
    centis = (LONG)((-value * 100.0f) + 0.5f);
  } else {
    *sign = ' ';
    centis = (LONG)((value * 100.0f) + 0.5f);
  }

  *whole = centis / 100;
  *frac = centis % 100;
}

void SpyDrawDebugOverlay(
  UWORD fps,
  const vec3 *camera_position,
  float camera_yaw,
  float camera_pitch,
  ULONG animation_frame,
  ULONG animation_frame_count,
  ULONG maggie_ticks_per_frame,
  BOOL stream_paused,
  const char *camera_mode_name,
  ULONG demo_elapsed_us,
  ULONG demo_vblanks,
  BOOL trace_draw
)
{
  char xsign, ysign, zsign, yawsign, pitchsign;
  LONG xwhole, ywhole, zwhole, yawwhole, pitchwhole;
  LONG xfrac, yfrac, zfrac, yawfrac, pitchfrac;
  ULONG animation_second;
  SAGE_Bitmap *bitmap = SAGE_GetBackBitmap();
  UWORD color;

  if (!overlay_font_ready || camera_position == NULL || bitmap == NULL ||
      bitmap->bitmap_buffer == NULL || bitmap->depth != 16 ||
      bitmap->bpr < bitmap->width * sizeof(UWORD) || (bitmap->bpr & 1)) {
    if (trace_draw) {
      SpyLog("Overlay: no cached font or usable 16-bit back buffer\n");
    }
    return;
  }
  color = (UWORD)SAGE_RemapColorToPixFormat(SPY_DEBUG_FRONT_COLOR, bitmap->pixformat);
  if (camera_mode_name == NULL) {
    camera_mode_name = "UNKNOWN";
  }

  GetCameraCoordParts(camera_position->x, &xsign, &xwhole, &xfrac);
  GetCameraCoordParts(camera_position->y, &ysign, &ywhole, &yfrac);
  GetCameraCoordParts(camera_position->z, &zsign, &zwhole, &zfrac);
  GetCameraCoordParts(camera_yaw * SPY_RADIANS_TO_DEGREES, &yawsign, &yawwhole, &yawfrac);
  GetCameraCoordParts(camera_pitch * SPY_RADIANS_TO_DEGREES, &pitchsign, &pitchwhole, &pitchfrac);
  animation_second = (animation_frame * maggie_ticks_per_frame) / SPY_MAGGIE_TICKS_PER_SECOND;

  if (trace_draw) {
    SpyLog("Overlay: drawing animation line\n");
  }
  SpyDebugPrintLine(
    bitmap,
    SPY_DEBUG_SCREEN_Y + SPY_DEBUG_LINE_HEIGHT + 5,
    color,
    "%dfps sec:%lu frm:%lu/%lu tk/frm:%lu",
    (int)fps,
    animation_second,
    animation_frame,
    animation_frame_count,
    maggie_ticks_per_frame
  );
  if (trace_draw) {
    SpyLog("Overlay: drawing camera position\n");
  }
  SpyDebugPrintLine(
    bitmap,
    SPY_DEBUG_SCREEN_Y + (SPY_DEBUG_LINE_HEIGHT * 2) + 9,
    color,
    "cam:%s x:%c%ld.%02ld y:%c%ld.%02ld z:%c%ld.%02ld",
    camera_mode_name,
    xsign, xwhole, xfrac,
    ysign, ywhole, yfrac,
    zsign, zwhole, zfrac
  );
  if (trace_draw) {
    SpyLog("Overlay: drawing camera rotation\n");
  }
  SpyDebugPrintLine(
    bitmap,
    SPY_DEBUG_SCREEN_Y + (SPY_DEBUG_LINE_HEIGHT * 3) + 12,
    color,
    "yaw:%c%ld.%02ld pitch:%c%ld.%02ld stream:%s",
    yawsign, yawwhole, yawfrac,
    pitchsign, pitchwhole, pitchfrac,
    stream_paused ? "PAUSE" : "RUN"
  );
  if (trace_draw) {
    SpyLog("Overlay: drawing demo timing\n");
  }
  SpyDebugPrintLine(
    bitmap,
    SPY_DEBUG_SCREEN_Y + (SPY_DEBUG_LINE_HEIGHT * 4) + 15,
    color,
    "demo:%lu.%lus vbl:%lu",
    demo_elapsed_us / STIM_TICKS,
    (demo_elapsed_us % STIM_TICKS) / (STIM_TICKS / 10),
    demo_vblanks
  );
}
