#include <math.h>
#include <stdio.h>

#include <sage/sage.h>
#include <sage/sage_timer.h>

#include "spy_config.h"
#include "spy_log.h"
#include "spy_tunnel.h"
#include "spy_tunnel_effects.h"
#include "spy_tunnel_pattern_effects.h"
#include "spy_tunnel_renderer.h"

#define TUNNEL_TEXTURE_WIDTH SPY_TUNNEL_PATTERN_WIDTH
#define TUNNEL_TEXTURE_HEIGHT SPY_TUNNEL_PATTERN_HEIGHT
#define TUNNEL_MAX_FRAMES 2048
#define TUNNEL_TEXTURE_WIDTH_MASK (TUNNEL_TEXTURE_WIDTH - 1)
#define TUNNEL_TEXTURE_HEIGHT_MASK (TUNNEL_TEXTURE_HEIGHT - 1)
#define TUNNEL_FRAME_FP_SHIFT 8
#define TUNNEL_FRAME_FP_ONE (1UL << TUNNEL_FRAME_FP_SHIFT)
#define TUNNEL_FRAME_FP_MASK ((TUNNEL_MAX_FRAMES * TUNNEL_FRAME_FP_ONE) - 1UL)
#define TUNNEL_FALLBACK_FRAME_US (STIM_TICKS / SPY_TUNNEL_MOTION_FPS)
#define TUNNEL_MAX_DELTA_US 100000UL
#define TUNNEL_TWO_PI 6.28318530717958647692f
#define TUNNEL_SCROLL_X_CYCLES 34
#define TUNNEL_SCROLL_Y_CYCLES 9
#define TUNNEL_LOOK_X_CYCLES 1.0f
#define TUNNEL_LOOK_Y_CYCLES 2.0f
#define TUNNEL_RENDER_SCALE SPY_TUNNEL_RENDER_PIXEL_SCALE
#define TUNNEL_RENDER_WIDTH (SPY_TUNNEL_LAYER_WIDTH / TUNNEL_RENDER_SCALE)
#define TUNNEL_RENDER_HEIGHT (SPY_TUNNEL_LAYER_HEIGHT / TUNNEL_RENDER_SCALE)
#define TUNNEL_RENDER_CORE_RADIUS (SPY_TUNNEL_CORE_RADIUS / TUNNEL_RENDER_SCALE)

#if TUNNEL_RENDER_SCALE != 1 && TUNNEL_RENDER_SCALE != 2
#error Tunnel render pixel scale must be 1 or 2
#endif

#if (SPY_TUNNEL_LAYER_WIDTH % TUNNEL_RENDER_SCALE) || (SPY_TUNNEL_LAYER_HEIGHT % TUNNEL_RENDER_SCALE)
#error Tunnel dimensions must be divisible by the render pixel scale
#endif

#if SPY_TUNNEL_CORE_RADIUS % TUNNEL_RENDER_SCALE
#error Tunnel core radius must be divisible by the render pixel scale
#endif

#if (TUNNEL_TEXTURE_WIDTH & (TUNNEL_TEXTURE_WIDTH - 1))
#error Tunnel pattern width must be a power of two
#endif

#if (TUNNEL_TEXTURE_HEIGHT & (TUNNEL_TEXTURE_HEIGHT - 1))
#error Tunnel pattern height must be a power of two
#endif

#if TUNNEL_TEXTURE_WIDTH != 512 || TUNNEL_TEXTURE_HEIGHT != 512
#error The 060 tunnel ASM renderer currently expects 512x512 patterns
#endif

#if SPY_TUNNEL_LAYER_WIDTH > SPY_SCREEN_WIDTH || SPY_TUNNEL_LAYER_HEIGHT > SPY_SCREEN_HEIGHT
#error Tunnel layer must fit inside the screen
#endif

#if SPY_TUNNEL_CORE_RADIUS < 0 || SPY_TUNNEL_CORE_RADIUS > SPY_TUNNEL_LAYER_WIDTH || SPY_TUNNEL_CORE_RADIUS > SPY_TUNNEL_LAYER_HEIGHT
#error Tunnel core radius must fit inside the tunnel layer
#endif

static UWORD distance_table[2 * TUNNEL_RENDER_HEIGHT][2 * TUNNEL_RENDER_WIDTH];
static UWORD angle_table[2 * TUNNEL_RENDER_HEIGHT][2 * TUNNEL_RENDER_WIDTH];

static const char *tunnel_pattern_names[SPY_TUNNEL_PATTERN_COUNT] = {
  SPY_TUNNEL_PATTERN_FILENAMES
};
static UWORD tunnel_textures[SPY_TUNNEL_PATTERN_COUNT][TUNNEL_TEXTURE_WIDTH][TUNNEL_TEXTURE_HEIGHT];
static UWORD tunnel_remap_palette[SPIC_MAXCOLORS];
static WORD shift_look_x_table[TUNNEL_MAX_FRAMES];
static WORD shift_look_y_table[TUNNEL_MAX_FRAMES];
static WORD shift_x_table[TUNNEL_MAX_FRAMES];
static WORD shift_y_table[TUNNEL_MAX_FRAMES];

static SAGE_Timer *motion_timer = NULL;
static UWORD current_pattern = 0;
static UWORD pattern_effect_from_pattern = 0;
static ULONG motion_phase_fp = 0;
static BOOL reveal_source_ready = FALSE;
static UWORD reveal_source_render_countdown = 0;
static UWORD reveal_source_buffer[SPY_TUNNEL_LAYER_HEIGHT][SPY_TUNNEL_LAYER_WIDTH];
static UWORD tunnel_core_half_width[TUNNEL_RENDER_CORE_RADIUS + 1];
static UWORD tunnel_core_skip_start[TUNNEL_RENDER_HEIGHT];
static UWORD tunnel_core_skip_count[TUNNEL_RENDER_HEIGHT];

static BOOL SpyTunnelGetTargetBuffer(UWORD **target_buffer, ULONG *target_stride, UWORD tunnel_y)
{
  SAGE_Bitmap *back_bitmap;

  if (target_buffer == NULL || target_stride == NULL) {
    return FALSE;
  }

  back_bitmap = SAGE_GetBackBitmap();
  if (
    back_bitmap == NULL ||
    back_bitmap->bitmap_buffer == NULL ||
    back_bitmap->depth != SBMP_DEPTH16 ||
    back_bitmap->width < (SPY_TUNNEL_SCREEN_X + SPY_TUNNEL_LAYER_WIDTH) ||
    back_bitmap->height < (tunnel_y + SPY_TUNNEL_LAYER_HEIGHT)
  ) {
    return FALSE;
  }

  *target_stride = back_bitmap->width;
  *target_buffer = ((UWORD *)back_bitmap->bitmap_buffer) +
    (tunnel_y * back_bitmap->width) +
    SPY_TUNNEL_SCREEN_X;
  return TRUE;
}

static void SpyTunnelInitCoreGeometry(void)
{
  int offset;
  int radius_squared = TUNNEL_RENDER_CORE_RADIUS * TUNNEL_RENDER_CORE_RADIUS;

  for (offset = 0; offset <= TUNNEL_RENDER_CORE_RADIUS; offset++) {
    tunnel_core_half_width[offset] = (UWORD)sqrt((float)(radius_squared - (offset * offset)));
  }
}

static void SpyTunnelPrepareCoreSpans(int centre_x, int centre_y)
{
  int row;
  int offset;
  int offset_index;
  int half_width;
  int left;
  int right;

  for (row = 0; row < TUNNEL_RENDER_HEIGHT; row++) {
    tunnel_core_skip_start[row] = TUNNEL_RENDER_WIDTH;
    tunnel_core_skip_count[row] = 0;
  }

  for (offset = -TUNNEL_RENDER_CORE_RADIUS; offset <= TUNNEL_RENDER_CORE_RADIUS; offset++) {
    row = centre_y + offset;
    if (row < 0 || row >= TUNNEL_RENDER_HEIGHT) {
      continue;
    }

    offset_index = offset < 0 ? -offset : offset;
    half_width = tunnel_core_half_width[offset_index];
    left = centre_x - half_width;
    right = centre_x + half_width;
    if (right < 0 || left >= TUNNEL_RENDER_WIDTH) {
      continue;
    }
    if (left < 0) {
      left = 0;
    }
    if (right >= TUNNEL_RENDER_WIDTH) {
      right = TUNNEL_RENDER_WIDTH - 1;
    }

    tunnel_core_skip_start[row] = (UWORD)left;
    tunnel_core_skip_count[row] = (UWORD)(right - left + 1);
  }
}

static void SpyTunnelClearCore(UWORD *target, ULONG target_stride)
{
  ULONG row;
  ULONG scaled_row;
  ULONG scale_row;
  ULONG start;
  ULONG count;
  UWORD *pixel;

  if (target == NULL) {
    return;
  }

  for (row = 0; row < TUNNEL_RENDER_HEIGHT; row++) {
    start = tunnel_core_skip_start[row] * TUNNEL_RENDER_SCALE;
    count = tunnel_core_skip_count[row] * TUNNEL_RENDER_SCALE;
    for (scale_row = 0; scale_row < TUNNEL_RENDER_SCALE; scale_row++) {
      scaled_row = (row * TUNNEL_RENDER_SCALE) + scale_row;
      pixel = target + (scaled_row * target_stride) + start;
      {
        ULONG clear_count = count;
        while (clear_count > 0) {
          *pixel++ = 0;
          clear_count--;
        }
      }
    }
  }
}

static ULONG SpyTunnelSageTimeToMicros(ULONG sage_time)
{
  return ((sage_time >> STIM_SECONDS_SHIFT) * STIM_TICKS) + (sage_time & STIM_MICRO_MASK);
}

static WORD SpyTunnelLerpWord(WORD start, WORD end, UWORD fraction)
{
  return (WORD)(start + (((LONG)(end - start) * (LONG)fraction) >> TUNNEL_FRAME_FP_SHIFT));
}

static WORD SpyTunnelLerpWrapped(WORD start, WORD end, WORD modulo, UWORD fraction)
{
  LONG delta = (LONG)end - (LONG)start;
  LONG value;

  if (delta > modulo / 2) {
    delta -= modulo;
  } else if (delta < -(modulo / 2)) {
    delta += modulo;
  }

  value = (LONG)start + ((delta * (LONG)fraction) >> TUNNEL_FRAME_FP_SHIFT);
  value %= modulo;
  if (value < 0) {
    value += modulo;
  }
  return (WORD)value;
}

static void SpyTunnelAdvanceMotionPhase(void)
{
  ULONG elapsed_us = TUNNEL_FALLBACK_FRAME_US;
  ULONG elapsed;
  ULONG frame_step_fp;

  if (motion_timer != NULL) {
    elapsed = SAGE_ElapsedTime(motion_timer);
    if (elapsed != STIM_OVERFLOW) {
      elapsed_us = SpyTunnelSageTimeToMicros(elapsed);
    }
  }

  if (elapsed_us > TUNNEL_MAX_DELTA_US) {
    elapsed_us = TUNNEL_MAX_DELTA_US;
  }

  frame_step_fp = ((elapsed_us * SPY_TUNNEL_MOTION_FPS * TUNNEL_FRAME_FP_ONE) + (STIM_TICKS / 2)) / STIM_TICKS;
  if (frame_step_fp == 0) {
    frame_step_fp = 1;
  }
  motion_phase_fp = (motion_phase_fp + frame_step_fp) & TUNNEL_FRAME_FP_MASK;
}

static void SpyTunnelRenderPattern(
  UWORD *target_buffer,
  ULONG target_stride,
  UWORD pattern_index,
  int shift_look_x,
  int shift_look_y,
  int shift_x,
  int shift_y
)
{
  SpyTunnelRendererDraw(
    target_buffer,
    &distance_table[shift_look_y][shift_look_x],
    &angle_table[shift_look_y][shift_look_x],
    sizeof(distance_table[0]),
    target_stride * sizeof(UWORD),
    &tunnel_textures[pattern_index][0][0],
    TUNNEL_RENDER_WIDTH,
    TUNNEL_RENDER_HEIGHT,
    (ULONG)shift_x,
    (ULONG)shift_y,
    tunnel_core_skip_start,
    tunnel_core_skip_count
  );
}

static BOOL SpyTunnelShouldRenderRevealSource(UWORD render_interval)
{
  if (!reveal_source_ready) {
    return TRUE;
  }

  if (render_interval <= 1 || reveal_source_render_countdown == 0) {
    return TRUE;
  }

  reveal_source_render_countdown--;
  return FALSE;
}

static void SpyTunnelMarkRevealSourceRendered(UWORD render_interval)
{
  reveal_source_ready = TRUE;
  reveal_source_render_countdown = render_interval > 1 ? render_interval - 1 : 0;
}

static BOOL SpyTunnelClearAreaIfNeeded(ULONG left, ULONG top, ULONG width, ULONG height)
{
  if (width == 0 || height == 0) {
    return TRUE;
  }

  return SAGE_ClearArea(left, top, width, height);
}

static BOOL SpyTunnelLoadPattern(const char *picture_name, UWORD pattern_index)
{
  int x, y;
  UBYTE *colormap;
  SAGE_Picture *picture;

  if (picture_name == NULL || pattern_index >= SPY_TUNNEL_PATTERN_COUNT) {
    SAGE_SetError(SERR_NULL_POINTER);
    return FALSE;
  }

  SAGE_AutoRemapPicture(FALSE);
  picture = SAGE_LoadPicture((STRPTR)picture_name);
  SAGE_AutoRemapPicture(TRUE);
  if (picture == NULL) {
    return FALSE;
  }

  if (picture->bitmap == NULL || picture->bitmap->depth != SBMP_DEPTH8) {
    SpyLog("Tunnel pattern must be an indexed 8-bit picture: %s\n", picture_name);
    SAGE_SetError(SERR_PIXFORMAT);
    SAGE_ReleasePicture(picture);
    return FALSE;
  }

  if (picture->bitmap->width != TUNNEL_TEXTURE_WIDTH || picture->bitmap->height != TUNNEL_TEXTURE_HEIGHT) {
    SpyLog(
      "Tunnel pattern must be %dx%d: %s is %dx%d\n",
      TUNNEL_TEXTURE_WIDTH,
      TUNNEL_TEXTURE_HEIGHT,
      picture_name,
      picture->bitmap->width,
      picture->bitmap->height
    );
    SAGE_SetError(SERR_PICTURE_SIZE);
    SAGE_ReleasePicture(picture);
    return FALSE;
  }

  colormap = (UBYTE *)SAGE_GetBitmapBuffer(picture->bitmap);
  if (colormap == NULL) {
    SAGE_SetError(SERR_NULL_POINTER);
    SAGE_ReleasePicture(picture);
    return FALSE;
  }

  for (x = 0; x < SPIC_MAXCOLORS; ++x) {
    tunnel_remap_palette[x] = (UWORD)(SAGE_RemapColor(picture->color_map[x]) & 0xffff);
  }

  for (y = 0; y < TUNNEL_TEXTURE_HEIGHT; ++y) {
    for (x = 0; x < TUNNEL_TEXTURE_WIDTH; ++x) {
      tunnel_textures[pattern_index][x][y] = tunnel_remap_palette[colormap[x + y * TUNNEL_TEXTURE_WIDTH]];
    }
  }
  SAGE_ReleasePicture(picture);
  return TRUE;
}

static BOOL SpyTunnelLoadPatterns(void)
{
  UWORD pattern_index;

  for (pattern_index = 0; pattern_index < SPY_TUNNEL_PATTERN_COUNT; pattern_index++) {
    if (!SpyTunnelLoadPattern(tunnel_pattern_names[pattern_index], pattern_index)) {
      return FALSE;
    }
  }

  return TRUE;
}

static void SpyTunnelNextPattern(void)
{
  current_pattern++;
  if (current_pattern >= SPY_TUNNEL_PATTERN_COUNT) {
    current_pattern = 0;
  }
}

BOOL SpyTunnelInit(const char *picture_name)
{
  int x, y, frame_index;
  int dx, dy, distance_square;
  int angle, distance;
  float ratio = 32.0f;
  float phase;

  (void)picture_name;

  SpyTunnelInitCoreGeometry();

  if (!SpyTunnelLoadPatterns()) {
    return FALSE;
  }

  for (frame_index = 0; frame_index < TUNNEL_MAX_FRAMES; frame_index++) {
    phase = (float)frame_index / (float)TUNNEL_MAX_FRAMES;
    shift_x_table[frame_index] = (WORD)(((frame_index * TUNNEL_SCROLL_X_CYCLES * TUNNEL_TEXTURE_WIDTH) / TUNNEL_MAX_FRAMES) & TUNNEL_TEXTURE_WIDTH_MASK);
    shift_y_table[frame_index] = (WORD)(((frame_index * TUNNEL_SCROLL_Y_CYCLES * TUNNEL_TEXTURE_HEIGHT) / TUNNEL_MAX_FRAMES) & TUNNEL_TEXTURE_HEIGHT_MASK);
    shift_look_x_table[frame_index] = (WORD)(TUNNEL_RENDER_WIDTH / 2 + (int)(TUNNEL_RENDER_WIDTH / 2 * sin(TUNNEL_TWO_PI * TUNNEL_LOOK_X_CYCLES * phase)));
    shift_look_y_table[frame_index] = (WORD)(TUNNEL_RENDER_HEIGHT / 2 + (int)(TUNNEL_RENDER_HEIGHT / 2 * sin(TUNNEL_TWO_PI * TUNNEL_LOOK_Y_CYCLES * phase)));
  }

  for (y = 0; y < TUNNEL_RENDER_HEIGHT * 2; y++) {
    for (x = 0; x < TUNNEL_RENDER_WIDTH * 2; x++) {
      dx = (x - TUNNEL_RENDER_WIDTH) * TUNNEL_RENDER_SCALE;
      dy = (y - TUNNEL_RENDER_HEIGHT) * TUNNEL_RENDER_SCALE;
      distance_square = dx * dx + dy * dy;
      if (distance_square == 0) {
        distance = 0;
      } else {
        distance = (int)(ratio * TUNNEL_TEXTURE_HEIGHT / sqrt((float)distance_square)) % TUNNEL_TEXTURE_HEIGHT;
      }
      angle = (int)(0.5f * TUNNEL_TEXTURE_WIDTH * atan2((float)dy, (float)dx) / 3.1416f);
      distance_table[y][x] = (UWORD)distance;
      angle_table[y][x] = (UWORD)angle;
    }
  }

  motion_phase_fp = 0;
  current_pattern = 0;
  pattern_effect_from_pattern = 0;
  reveal_source_ready = FALSE;
  reveal_source_render_countdown = 0;
  motion_timer = SAGE_AllocTimer();
  if (motion_timer != NULL) {
    SAGE_ElapsedTime(motion_timer);
  } else {
    SpyLog("Tunnel motion timer unavailable, using frame-based fallback\n");
  }
  SpyTunnelEffectsInit();
  SpyTunnelPatternEffectsInit();
  return TRUE;
}

void SpyTunnelRestartIntroReveal(void)
{
  reveal_source_ready = FALSE;
  reveal_source_render_countdown = 0;
  SpyTunnelEffectsInit();
}

void SpyTunnelRenderFrame(UWORD tunnel_y)
{
  int shift_x, shift_y, shift_look_x, shift_look_y;
  UWORD table_index, next_table_index, frame_fraction;
  UWORD *target_buffer;
  ULONG target_stride;
  BOOL intro_blocks_pattern_effects;

  if (!SpyTunnelGetTargetBuffer(&target_buffer, &target_stride, tunnel_y)) {
    return;
  }

  intro_blocks_pattern_effects =
    SpyTunnelEffectsIntroIsHidden() || SpyTunnelEffectsIntroIsActive();
  SpyTunnelEffectsUpdate();
  if (SpyTunnelPatternEffectsUpdate(intro_blocks_pattern_effects)) {
    pattern_effect_from_pattern = current_pattern;
    reveal_source_ready = FALSE;
    reveal_source_render_countdown = 0;
    SpyTunnelNextPattern();
  }

  SpyTunnelAdvanceMotionPhase();
  table_index = (UWORD)(motion_phase_fp >> TUNNEL_FRAME_FP_SHIFT);
  next_table_index = (table_index + 1) & (TUNNEL_MAX_FRAMES - 1);
  frame_fraction = (UWORD)(motion_phase_fp & (TUNNEL_FRAME_FP_ONE - 1));

  shift_x = SpyTunnelLerpWrapped(
    shift_x_table[table_index],
    shift_x_table[next_table_index],
    TUNNEL_TEXTURE_WIDTH,
    frame_fraction
  );
  shift_y = SpyTunnelLerpWrapped(
    shift_y_table[table_index],
    shift_y_table[next_table_index],
    TUNNEL_TEXTURE_HEIGHT,
    frame_fraction
  );
  shift_look_x = SpyTunnelLerpWord(shift_look_x_table[table_index], shift_look_x_table[next_table_index], frame_fraction);
  shift_look_y = SpyTunnelLerpWord(shift_look_y_table[table_index], shift_look_y_table[next_table_index], frame_fraction);

  if (SpyTunnelEffectsIntroIsHidden()) {
    return;
  }

#if SPY_TUNNEL_CORE_RADIUS > 0
  SpyTunnelPrepareCoreSpans(
    TUNNEL_RENDER_WIDTH - shift_look_x,
    TUNNEL_RENDER_HEIGHT - shift_look_y
  );
#endif

  if (SpyTunnelEffectsIntroIsActive()) {
    if (SpyTunnelShouldRenderRevealSource(SPY_TUNNEL_EFFECT_REVEAL_SOURCE_RENDER_INTERVAL)) {
      SpyTunnelRenderPattern(
        &reveal_source_buffer[0][0],
        SPY_TUNNEL_LAYER_WIDTH,
        current_pattern,
        shift_look_x,
        shift_look_y,
        shift_x,
        shift_y
      );
      SpyTunnelMarkRevealSourceRendered(SPY_TUNNEL_EFFECT_REVEAL_SOURCE_RENDER_INTERVAL);
    }
    SpyTunnelEffectsApplyIntroReveal(
      target_buffer,
      target_stride,
      &reveal_source_buffer[0][0],
      SPY_TUNNEL_LAYER_WIDTH,
      SPY_TUNNEL_LAYER_WIDTH,
      SPY_TUNNEL_LAYER_HEIGHT
    );
#if SPY_TUNNEL_CORE_RADIUS > 0
    SpyTunnelClearCore(target_buffer, target_stride);
#endif
  } else if (SpyTunnelPatternEffectsIsActive()) {
    if (SpyTunnelPatternEffectsNeedsSource()) {
      SpyTunnelRenderPattern(
        target_buffer,
        target_stride,
        pattern_effect_from_pattern,
        shift_look_x,
        shift_look_y,
        shift_x,
        shift_y
      );
      if (SpyTunnelShouldRenderRevealSource(SPY_TUNNEL_PATTERN_EFFECT_SOURCE_RENDER_INTERVAL)) {
        SpyTunnelRenderPattern(
          &reveal_source_buffer[0][0],
          SPY_TUNNEL_LAYER_WIDTH,
          current_pattern,
          shift_look_x,
          shift_look_y,
          shift_x,
          shift_y
        );
        SpyTunnelMarkRevealSourceRendered(SPY_TUNNEL_PATTERN_EFFECT_SOURCE_RENDER_INTERVAL);
      }
    } else {
      SpyTunnelRenderPattern(
        target_buffer,
        target_stride,
        current_pattern,
        shift_look_x,
        shift_look_y,
        shift_x,
        shift_y
      );
    }
    SpyTunnelPatternEffectsApplyTransition(
      target_buffer,
      target_stride,
      &reveal_source_buffer[0][0],
      SPY_TUNNEL_LAYER_WIDTH,
      SPY_TUNNEL_LAYER_WIDTH,
      SPY_TUNNEL_LAYER_HEIGHT
    );
#if SPY_TUNNEL_CORE_RADIUS > 0
    SpyTunnelClearCore(target_buffer, target_stride);
#endif
  } else {
    SpyTunnelRenderPattern(
      target_buffer,
      target_stride,
      current_pattern,
      shift_look_x,
      shift_look_y,
      shift_x,
      shift_y
    );
  }
}

BOOL SpyTunnelClearScreenGaps(UWORD tunnel_y)
{
  if (!SpyTunnelClearAreaIfNeeded(0, 0, SPY_SCREEN_WIDTH, tunnel_y)) {
    return FALSE;
  }
  if (!SpyTunnelClearAreaIfNeeded(
    0,
    tunnel_y + SPY_TUNNEL_LAYER_HEIGHT,
    SPY_SCREEN_WIDTH,
    SPY_SCREEN_HEIGHT - (tunnel_y + SPY_TUNNEL_LAYER_HEIGHT)
  )) {
    return FALSE;
  }
  if (!SpyTunnelClearAreaIfNeeded(0, tunnel_y, SPY_TUNNEL_SCREEN_X, SPY_TUNNEL_LAYER_HEIGHT)) {
    return FALSE;
  }
  return SpyTunnelClearAreaIfNeeded(
    SPY_TUNNEL_SCREEN_X + SPY_TUNNEL_LAYER_WIDTH,
    tunnel_y,
    SPY_SCREEN_WIDTH - (SPY_TUNNEL_SCREEN_X + SPY_TUNNEL_LAYER_WIDTH),
    SPY_TUNNEL_LAYER_HEIGHT
  );
}

void SpyTunnelRelease(void)
{
  SpyTunnelPatternEffectsRelease();
  SpyTunnelEffectsRelease();

  if (motion_timer != NULL) {
    SAGE_ReleaseTimer(motion_timer);
    motion_timer = NULL;
  }

}
