#include <exec/types.h>
#include <sage/sage_error.h>
#include <sage/sage_screen.h>
#include <sage/sage_timer.h>

#include "spy_config.h"
#include "spy_debug.h"

#define FPS_INTERVAL_US STIM_TICKS
#define SPY_RADIANS_TO_DEGREES 57.2957795130823208768f

static ULONG SageTimeToMicros(ULONG sage_time)
{
  return ((sage_time >> STIM_SECONDS_SHIFT) * STIM_TICKS) + (sage_time & STIM_MICRO_MASK);
}

BOOL SpyDebugConfigureOverlay(void)
{
  BOOL configured = TRUE;

  if (!SAGE_SetFont((STRPTR)SPY_DEBUG_FONT_NAME, SPY_DEBUG_FONT_SIZE)) {
    configured = FALSE;
    SAGE_SetError(SERR_NO_ERROR);
  }
  if (!SAGE_SetColor(SPY_DEBUG_BACK_PEN, SPY_DEBUG_BACK_COLOR)) {
    configured = FALSE;
  }
  if (!SAGE_SetColor(SPY_DEBUG_FRONT_PEN, SPY_DEBUG_FRONT_COLOR)) {
    configured = FALSE;
  }
  if (!SAGE_RefreshColors(SPY_DEBUG_BACK_PEN, 2)) {
    configured = FALSE;
  }
  if (!SAGE_SetTextColor(SPY_DEBUG_FRONT_PEN, SPY_DEBUG_BACK_PEN)) {
    configured = FALSE;
  }
  if (!SAGE_SetDrawingMode(SSCR_TXTTRANSP)) {
    configured = FALSE;
  }

  return configured;
}

UWORD SpyUpdateFpsCounter(SAGE_Timer *timer, UWORD *frame_count, ULONG *elapsed_time, UWORD fps)
{
  ULONG elapsed;

  if (timer == NULL) {
    return fps;
  }

  (*frame_count)++;
  elapsed = SAGE_ElapsedTime(timer);
  if (elapsed != STIM_OVERFLOW) {
    *elapsed_time += SageTimeToMicros(elapsed);
    if (*elapsed_time >= FPS_INTERVAL_US) {
      fps = *frame_count;
      *frame_count = 0;
      *elapsed_time %= FPS_INTERVAL_US;
    }
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
  const char *camera_mode_name
)
{
  char xsign, ysign, zsign, yawsign, pitchsign;
  LONG xwhole, ywhole, zwhole, yawwhole, pitchwhole;
  LONG xfrac, yfrac, zfrac, yawfrac, pitchfrac;
  ULONG animation_second;

  if (camera_position == NULL) {
    return;
  }
  if (camera_mode_name == NULL) {
    camera_mode_name = "UNKNOWN";
  }

  GetCameraCoordParts(camera_position->x, &xsign, &xwhole, &xfrac);
  GetCameraCoordParts(camera_position->y, &ysign, &ywhole, &yfrac);
  GetCameraCoordParts(camera_position->z, &zsign, &zwhole, &zfrac);
  GetCameraCoordParts(camera_yaw * SPY_RADIANS_TO_DEGREES, &yawsign, &yawwhole, &yawfrac);
  GetCameraCoordParts(camera_pitch * SPY_RADIANS_TO_DEGREES, &pitchsign, &pitchwhole, &pitchfrac);
  animation_second = (animation_frame * maggie_ticks_per_frame) / SPY_MAGGIE_TICKS_PER_SECOND;

  SAGE_PrintFText(
    SPY_DEBUG_SCREEN_X,
    SPY_DEBUG_SCREEN_Y + SPY_DEBUG_FONT_SIZE + 5,
    "%dfps sec:%lu frm:%lu/%lu tk/frm:%lu",
    fps,
    animation_second,
    animation_frame,
    animation_frame_count,
    maggie_ticks_per_frame
  );
  SAGE_PrintFText(
    SPY_DEBUG_SCREEN_X,
    SPY_DEBUG_SCREEN_Y + SPY_DEBUG_FONT_SIZE + SPY_DEBUG_LINE_HEIGHT + 9,
    "cam:%s x:%c%ld.%02ld y:%c%ld.%02ld z:%c%ld.%02ld",
    camera_mode_name,
    xsign, xwhole, xfrac,
    ysign, ywhole, yfrac,
    zsign, zwhole, zfrac
  );
  SAGE_PrintFText(
    SPY_DEBUG_SCREEN_X,
    SPY_DEBUG_SCREEN_Y + SPY_DEBUG_FONT_SIZE + (SPY_DEBUG_LINE_HEIGHT * 2) + 12,
    "yaw:%c%ld.%02ld pitch:%c%ld.%02ld stream:%s",
    yawsign, yawwhole, yawfrac,
    pitchsign, pitchwhole, pitchfrac,
    stream_paused ? "PAUSE" : "RUN"
  );
}
