#include <exec/types.h>
#include <sage/sage_timer.h>

#include "spy_config.h"
#include "spy_tunnel_pattern_effects.h"
#include "spy_tunnel_reveal.h"

static SAGE_Timer *pattern_effect_timer = NULL;
static ULONG pattern_effect_duration_us = 0;
static ULONG pattern_effect_interval_us = 0;
static ULONG pattern_effect_since_start_us = 0;
static ULONG pattern_effect_phase_us = 0;
static BOOL pattern_effect_enabled = FALSE;
static BOOL pattern_effect_active = FALSE;
static BOOL pattern_effect_pattern_switched = FALSE;
static SpyTunnelPatternEffectMode pattern_effect_mode = SPY_TUNNEL_PATTERN_EFFECT_COLUMN_REVEAL;
static UWORD flash_current_alpha = 0xffff;
static UWORD flash_current_colour = 0xffff;
static UBYTE flash_red_table[32];
static UBYTE flash_green_table[64];
static UBYTE flash_blue_table[32];

static ULONG SpyTunnelPatternEffectsSecondsToMicros(float seconds)
{
  if (seconds <= 0.0f) {
    return 0;
  }

  return (ULONG)((seconds * (float)STIM_TICKS) + 0.5f);
}

static ULONG SpyTunnelPatternEffectsSageTimeToMicros(ULONG sage_time)
{
  return ((sage_time >> STIM_SECONDS_SHIFT) * STIM_TICKS) + (sage_time & STIM_MICRO_MASK);
}

static SpyTunnelPatternEffectMode SpyTunnelPatternEffectsConfiguredMode(void)
{
  SpyTunnelPatternEffectMode mode = SPY_TUNNEL_PATTERN_EFFECT_MODE;

  if (
    mode != SPY_TUNNEL_PATTERN_EFFECT_COLUMN_REVEAL &&
    mode != SPY_TUNNEL_PATTERN_EFFECT_FLASH &&
    mode != SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_BURST &&
    mode != SPY_TUNNEL_PATTERN_EFFECT_SHUTTER &&
    mode != SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_WAVE &&
    mode != SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH &&
    mode != SPY_TUNNEL_PATTERN_EFFECT_CUT
  ) {
    return SPY_TUNNEL_PATTERN_EFFECT_COLUMN_REVEAL;
  }
  return mode;
}

static float SpyTunnelPatternEffectsDurationSeconds(void)
{
  switch (pattern_effect_mode) {
    case SPY_TUNNEL_PATTERN_EFFECT_CUT:
      return 0.0f;
    case SPY_TUNNEL_PATTERN_EFFECT_FLASH:
      return SPY_TUNNEL_PATTERN_EFFECT_FLASH_DURATION_SECONDS;
    case SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_BURST:
      return SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_DURATION_SECONDS;
    case SPY_TUNNEL_PATTERN_EFFECT_SHUTTER:
      return SPY_TUNNEL_PATTERN_EFFECT_SHUTTER_DURATION_SECONDS;
    case SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_WAVE:
      return SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_DURATION_SECONDS;
    case SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH:
      return SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH_DURATION_SECONDS;
    default:
      return SPY_TUNNEL_PATTERN_EFFECT_DURATION_SECONDS;
  }
}

static UBYTE SpyTunnelPatternEffectsBlendChannel(UWORD source, UWORD target, UWORD alpha)
{
  LONG scaled_delta = ((LONG)target - (LONG)source) * (LONG)alpha;

  if (scaled_delta >= 0) {
    scaled_delta += 127;
  } else {
    scaled_delta -= 127;
  }
  return (UBYTE)((LONG)source + (scaled_delta / 255));
}

static void SpyTunnelPatternEffectsBuildFlashTables(UWORD alpha, UWORD colour)
{
  UWORD index;
  UWORD target_red = (colour >> 11) & 31;
  UWORD target_green = (colour >> 5) & 63;
  UWORD target_blue = colour & 31;

  for (index = 0; index < 32; index++) {
    flash_red_table[index] = SpyTunnelPatternEffectsBlendChannel(index, target_red, alpha);
    flash_blue_table[index] = SpyTunnelPatternEffectsBlendChannel(index, target_blue, alpha);
  }

  for (index = 0; index < 64; index++) {
    flash_green_table[index] = SpyTunnelPatternEffectsBlendChannel(index, target_green, alpha);
  }

  flash_current_alpha = alpha;
  flash_current_colour = colour;
}

static UWORD SpyTunnelPatternEffectsFlashAlpha(void)
{
  ULONG alpha;

  if (!pattern_effect_active || pattern_effect_duration_us == 0) {
    return 0;
  }

  alpha = 255 - (ULONG)(((float)pattern_effect_phase_us * 255.0f) / (float)pattern_effect_duration_us);
  if (alpha > 255) {
    return 0;
  }
  return (UWORD)alpha;
}

static void SpyTunnelPatternEffectsApplyColourFlash(
  UWORD *target,
  ULONG target_stride,
  UWORD width,
  UWORD height,
  UWORD colour
)
{
  ULONG x;
  ULONG y;
  UWORD alpha;
  UWORD pixel;
  UWORD red;
  UWORD green;
  UWORD blue;
  UWORD *row;

  if (target == NULL || width == 0 || height == 0) {
    return;
  }

  alpha = SpyTunnelPatternEffectsFlashAlpha();
  if (alpha == 0) {
    return;
  }

  if (alpha != flash_current_alpha || colour != flash_current_colour) {
    SpyTunnelPatternEffectsBuildFlashTables(alpha, colour);
  }

  for (y = 0; y < height; y++) {
    row = target + (y * target_stride);
    for (x = 0; x < width; x++) {
      pixel = row[x];
      red = flash_red_table[(pixel >> 11) & 31];
      green = flash_green_table[(pixel >> 5) & 63];
      blue = flash_blue_table[pixel & 31];
      row[x] = (UWORD)((red << 11) | (green << 5) | blue);
    }
  }
}

static void SpyTunnelPatternEffectsApplyNegative(
  UWORD *target,
  ULONG target_stride,
  UWORD width,
  UWORD height
)
{
  ULONG x;
  ULONG y;
  UWORD *row;

  if (target == NULL || width == 0 || height == 0) {
    return;
  }

  for (y = 0; y < height; y++) {
    row = target + (y * target_stride);
    for (x = 0; x < width; x++) {
      row[x] ^= 0xffff;
    }
  }
}

static void SpyTunnelPatternEffectsFillColumns(
  UWORD *target,
  ULONG target_stride,
  UWORD width,
  UWORD height,
  ULONG left_width,
  ULONG right_width,
  UWORD colour
)
{
  ULONG x;
  ULONG y;
  UWORD *row;

  for (y = 0; y < height; y++) {
    row = target + (y * target_stride);
    for (x = 0; x < left_width; x++) {
      row[x] = colour;
    }
    for (x = width - right_width; x < width; x++) {
      row[x] = colour;
    }
  }
}

static void SpyTunnelPatternEffectsApplyShutter(
  UWORD *target,
  ULONG target_stride,
  UWORD width,
  UWORD height
)
{
  ULONG first_half_us;
  ULONG second_half_us;
  ULONG remaining_us;
  ULONG left_max;
  ULONG right_max;
  ULONG left_width;
  ULONG right_width;

  if (
    target == NULL || width == 0 || height == 0 ||
    pattern_effect_duration_us < 2
  ) {
    return;
  }

  first_half_us = pattern_effect_duration_us / 2;
  second_half_us = pattern_effect_duration_us - first_half_us;
  left_max = ((ULONG)width + 1) / 2;
  right_max = (ULONG)width / 2;

  if (pattern_effect_phase_us < first_half_us) {
    left_width = (left_max * pattern_effect_phase_us) / first_half_us;
    right_width = (right_max * pattern_effect_phase_us) / first_half_us;
  } else {
    remaining_us = pattern_effect_duration_us - pattern_effect_phase_us;
    left_width = (left_max * remaining_us) / second_half_us;
    right_width = (right_max * remaining_us) / second_half_us;
  }

  SpyTunnelPatternEffectsFillColumns(
    target,
    target_stride,
    width,
    height,
    left_width,
    right_width,
    SPY_TUNNEL_PATTERN_EFFECT_SHUTTER_COLOUR
  );
}

static ULONG SpyTunnelPatternEffectsDivideRoundUp(ULONG value, ULONG divisor)
{
  if (divisor == 0) {
    return 0;
  }
  return (value + divisor - 1) / divisor;
}

static ULONG SpyTunnelPatternEffectsAbsLong(LONG value)
{
  return value < 0 ? (ULONG)(-value) : (ULONG)value;
}

static void SpyTunnelPatternEffectsCopyBlock(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  ULONG x,
  ULONG y,
  ULONG width,
  ULONG height
)
{
  ULONG copy_x;
  ULONG copy_y;

  for (copy_y = 0; copy_y < height; copy_y++) {
    for (copy_x = 0; copy_x < width; copy_x++) {
      target[(y + copy_y) * target_stride + x + copy_x] =
        source[(y + copy_y) * source_stride + x + copy_x];
    }
  }
}

static void SpyTunnelPatternEffectsApplyDiamond(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  UWORD width,
  UWORD height
)
{
  ULONG column_count;
  ULONG row_count;
  ULONG centre_column_twice;
  ULONG centre_row_twice;
  ULONG minimum_distance;
  ULONG maximum_distance;
  ULONG visible_distance;
  ULONG column;
  ULONG row;
  ULONG distance;
  ULONG x;
  ULONG y;
  ULONG copy_width;
  ULONG copy_height;

  if (
    target == NULL || source == NULL || width == 0 || height == 0 ||
    pattern_effect_duration_us == 0
  ) {
    return;
  }

  column_count = SpyTunnelPatternEffectsDivideRoundUp(width, SPY_TUNNEL_REVEAL_COLUMN_WIDTH);
  row_count = SpyTunnelPatternEffectsDivideRoundUp(height, SPY_TUNNEL_REVEAL_STEP_HEIGHT);
  if (column_count == 0 || row_count == 0) {
    return;
  }

  centre_column_twice = column_count - 1;
  centre_row_twice = row_count - 1;
  minimum_distance = ((column_count & 1) == 0 ? 1 : 0) + ((row_count & 1) == 0 ? 1 : 0);
  maximum_distance = centre_column_twice + centre_row_twice;
  visible_distance = minimum_distance +
    ((pattern_effect_phase_us * (maximum_distance - minimum_distance + 1)) /
      pattern_effect_duration_us);
  if (visible_distance > maximum_distance) {
    visible_distance = maximum_distance;
  }

  for (row = 0; row < row_count; row++) {
    for (column = 0; column < column_count; column++) {
      distance =
        SpyTunnelPatternEffectsAbsLong((LONG)(column * 2) - (LONG)centre_column_twice) +
        SpyTunnelPatternEffectsAbsLong((LONG)(row * 2) - (LONG)centre_row_twice);
      if (distance > visible_distance) {
        continue;
      }

      x = column * SPY_TUNNEL_REVEAL_COLUMN_WIDTH;
      y = row * SPY_TUNNEL_REVEAL_STEP_HEIGHT;
      copy_width = SPY_TUNNEL_REVEAL_COLUMN_WIDTH;
      copy_height = SPY_TUNNEL_REVEAL_STEP_HEIGHT;
      if (x + copy_width > width) {
        copy_width = width - x;
      }
      if (y + copy_height > height) {
        copy_height = height - y;
      }

      SpyTunnelPatternEffectsCopyBlock(
        target,
        target_stride,
        source,
        source_stride,
        x,
        y,
        copy_width,
        copy_height
      );
    }
  }
}

static BOOL SpyTunnelPatternEffectsSwitchesAtMidpoint(void)
{
  return
    pattern_effect_mode == SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_BURST ||
    pattern_effect_mode == SPY_TUNNEL_PATTERN_EFFECT_SHUTTER;
}

BOOL SpyTunnelPatternEffectsInit(void)
{
  SpyTunnelPatternEffectsRelease();

  pattern_effect_mode = SpyTunnelPatternEffectsConfiguredMode();
  pattern_effect_duration_us =
    SpyTunnelPatternEffectsSecondsToMicros(SpyTunnelPatternEffectsDurationSeconds());
  pattern_effect_interval_us =
    SpyTunnelPatternEffectsSecondsToMicros(SPY_TUNNEL_PATTERN_EFFECT_INTERVAL_SECONDS);
  pattern_effect_since_start_us = 0;
  pattern_effect_phase_us = 0;
  pattern_effect_active = FALSE;
  pattern_effect_pattern_switched = FALSE;
  flash_current_alpha = 0xffff;
  flash_current_colour = 0xffff;
  pattern_effect_enabled =
    pattern_effect_interval_us > 0 &&
    (pattern_effect_mode == SPY_TUNNEL_PATTERN_EFFECT_CUT || pattern_effect_duration_us > 0);

  if (!pattern_effect_enabled) {
    return TRUE;
  }

  pattern_effect_timer = SAGE_AllocTimer();
  if (pattern_effect_timer == NULL) {
    pattern_effect_enabled = FALSE;
    return TRUE;
  }

  SAGE_ElapsedTime(pattern_effect_timer);
  return TRUE;
}

BOOL SpyTunnelPatternEffectsUpdate(BOOL paused)
{
  ULONG elapsed;
  ULONG delta_us;
  BOOL switch_pattern = FALSE;

  if (!pattern_effect_enabled || pattern_effect_timer == NULL) {
    return FALSE;
  }

  elapsed = SAGE_ElapsedTime(pattern_effect_timer);
  if (elapsed == STIM_OVERFLOW) {
    return FALSE;
  }

  if (paused) {
    pattern_effect_since_start_us = 0;
    return FALSE;
  }

  delta_us = SpyTunnelPatternEffectsSageTimeToMicros(elapsed);
  pattern_effect_since_start_us += delta_us;

  if (pattern_effect_active) {
    pattern_effect_phase_us += delta_us;
    if (
      !pattern_effect_pattern_switched &&
      SpyTunnelPatternEffectsSwitchesAtMidpoint() &&
      pattern_effect_phase_us >= pattern_effect_duration_us / 2
    ) {
      pattern_effect_pattern_switched = TRUE;
      pattern_effect_phase_us = pattern_effect_duration_us / 2;
      switch_pattern = TRUE;
    }
    if (pattern_effect_phase_us >= pattern_effect_duration_us) {
      pattern_effect_active = FALSE;
      pattern_effect_phase_us = 0;
    }
  } else if (pattern_effect_since_start_us >= pattern_effect_interval_us) {
    pattern_effect_since_start_us = 0;
    if (pattern_effect_mode == SPY_TUNNEL_PATTERN_EFFECT_CUT) {
      pattern_effect_active = FALSE;
      pattern_effect_phase_us = 0;
      pattern_effect_pattern_switched = TRUE;
      switch_pattern = TRUE;
    } else {
      pattern_effect_active = TRUE;
      pattern_effect_phase_us = 0;
      pattern_effect_pattern_switched = !SpyTunnelPatternEffectsSwitchesAtMidpoint();
      switch_pattern = pattern_effect_pattern_switched;
    }
  }

  return switch_pattern;
}

BOOL SpyTunnelPatternEffectsIsActive(void)
{
  return pattern_effect_active;
}

BOOL SpyTunnelPatternEffectsNeedsSource(void)
{
  return
    pattern_effect_mode == SPY_TUNNEL_PATTERN_EFFECT_COLUMN_REVEAL ||
    pattern_effect_mode == SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_WAVE;
}

void SpyTunnelPatternEffectsApplyTransition(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  UWORD width,
  UWORD height
)
{
  switch (pattern_effect_mode) {
    case SPY_TUNNEL_PATTERN_EFFECT_CUT:
      break;
    case SPY_TUNNEL_PATTERN_EFFECT_FLASH:
      SpyTunnelPatternEffectsApplyColourFlash(target, target_stride, width, height, 0xffff);
      break;
    case SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_BURST:
      SpyTunnelPatternEffectsApplyNegative(target, target_stride, width, height);
      break;
    case SPY_TUNNEL_PATTERN_EFFECT_SHUTTER:
      SpyTunnelPatternEffectsApplyShutter(target, target_stride, width, height);
      break;
    case SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_WAVE:
      SpyTunnelPatternEffectsApplyDiamond(
        target,
        target_stride,
        source,
        source_stride,
        width,
        height
      );
      break;
    case SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH:
      SpyTunnelPatternEffectsApplyColourFlash(
        target,
        target_stride,
        width,
        height,
        SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH_COLOUR
      );
      break;
    default:
      SpyTunnelRevealApply(
        target,
        target_stride,
        source,
        source_stride,
        width,
        height,
        pattern_effect_phase_us,
        pattern_effect_duration_us,
        SPY_TUNNEL_PATTERN_EFFECT_ORIGIN
      );
      break;
  }
}

void SpyTunnelPatternEffectsRelease(void)
{
  if (pattern_effect_timer != NULL) {
    SAGE_ReleaseTimer(pattern_effect_timer);
    pattern_effect_timer = NULL;
  }

  pattern_effect_enabled = FALSE;
  pattern_effect_active = FALSE;
  pattern_effect_pattern_switched = FALSE;
  pattern_effect_duration_us = 0;
  pattern_effect_interval_us = 0;
  pattern_effect_since_start_us = 0;
  pattern_effect_phase_us = 0;
  pattern_effect_mode = SPY_TUNNEL_PATTERN_EFFECT_COLUMN_REVEAL;
  flash_current_alpha = 0xffff;
  flash_current_colour = 0xffff;
}
