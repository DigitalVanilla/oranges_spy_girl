#include <exec/types.h>

#include <sage/sage.h>
#include <sage/sage_timer.h>

#include "spy_config.h"
#include "spy_log.h"
#include "spy_preload.h"

#define SPY_PRELOAD_BAR_WIDTH   192
#define SPY_PRELOAD_BAR_HEIGHT  10
#define SPY_PRELOAD_BAR_BORDER  2

static SAGE_Picture *preload_background = NULL;
static SAGE_Timer *preload_reveal_timer = NULL;
static ULONG preload_reveal_elapsed_us = 0;
static ULONG preload_reveal_duration_us = 0;
static BOOL preload_reveal_manual = FALSE;

static ULONG SpyPreloadSageTimeToMicros(ULONG sage_time)
{
  return ((sage_time >> STIM_SECONDS_SHIFT) * STIM_TICKS) + (sage_time & STIM_MICRO_MASK);
}

static ULONG SpyPreloadSecondsToMicros(float seconds)
{
  if (seconds <= 0.0f) {
    return 0;
  }
  return (ULONG)((seconds * (float)STIM_TICKS) + 0.5f);
}

static ULONG SpyPreloadDivideRoundUp(ULONG value, ULONG divisor)
{
  if (divisor == 0) {
    return 0;
  }
  return (value + divisor - 1) / divisor;
}

BOOL SpyPreloadInit(void)
{
  SAGE_Screen *screen;

  SpyPreloadRelease();
  preload_reveal_duration_us = SpyPreloadSecondsToMicros(SPY_PRELOAD_BACKGROUND_REVEAL_SECONDS);
  preload_reveal_elapsed_us = 0;
  preload_reveal_timer = SAGE_AllocTimer();
  if (preload_reveal_timer != NULL) {
    SAGE_ElapsedTime(preload_reveal_timer);
  } else {
    SpyLog("Preload background reveal timer unavailable; reveal will follow progress updates\n");
  }

  preload_background = SAGE_LoadPicture((STRPTR)SPY_PRELOAD_BACKGROUND_FILENAME);
  if (preload_background == NULL || preload_background->bitmap == NULL) {
    SpyLog("Preload background unavailable: cannot load %s\n", SPY_PRELOAD_BACKGROUND_FILENAME);
    SpyPreloadRelease();
    return FALSE;
  }

  screen = SAGE_GetScreen();
  SpyLog(
    "Preload background loaded: %lux%lu depth=%lu pixfmt=%lu screen=%ldx%ld\n",
    preload_background->bitmap->width,
    preload_background->bitmap->height,
    preload_background->bitmap->depth,
    preload_background->bitmap->pixformat,
    screen != NULL ? screen->width : 0,
    screen != NULL ? screen->height : 0
  );

  preload_reveal_elapsed_us = 0;
  if (preload_reveal_timer != NULL) {
    SAGE_ElapsedTime(preload_reveal_timer);
  }

  return TRUE;
}

static void SpyPreloadUpdateRevealTimer(void)
{
  ULONG elapsed;

  if (preload_reveal_manual || preload_reveal_timer == NULL || preload_reveal_duration_us == 0) {
    return;
  }

  elapsed = SAGE_ElapsedTime(preload_reveal_timer);
  if (elapsed == STIM_OVERFLOW) {
    preload_reveal_elapsed_us = preload_reveal_duration_us;
    return;
  }

  preload_reveal_elapsed_us += SpyPreloadSageTimeToMicros(elapsed);
  if (preload_reveal_elapsed_us > preload_reveal_duration_us) {
    preload_reveal_elapsed_us = preload_reveal_duration_us;
  }
}

static UWORD SpyPreloadRevealPercent(UWORD progress_percent)
{
  ULONG percent;

  if (preload_reveal_timer == NULL || preload_reveal_duration_us == 0) {
    return progress_percent;
  }

  percent = (preload_reveal_elapsed_us * 100UL) / preload_reveal_duration_us;
  if (percent > 100) {
    percent = 100;
  }
  return (UWORD)percent;
}

static BOOL SpyPreloadBlitFullBackground(void)
{
  ULONG src_x = 0;
  ULONG src_y = 0;
  ULONG width;
  ULONG height;
  ULONG dest_x = 0;
  ULONG dest_y = 0;
  LONG centered_x;
  LONG centered_y;

  if (preload_background == NULL || preload_background->bitmap == NULL) {
    return TRUE;
  }

  width = preload_background->bitmap->width;
  height = preload_background->bitmap->height;
  centered_x = ((LONG)SPY_SCREEN_WIDTH - (LONG)width) / 2;
  centered_y = ((LONG)SPY_SCREEN_HEIGHT - (LONG)height) / 2;

  if (centered_x < 0) {
    src_x = (ULONG)(-centered_x);
    width -= src_x;
  } else {
    dest_x = (ULONG)centered_x;
  }

  if (centered_y < 0) {
    src_y = (ULONG)(-centered_y);
    height -= src_y;
  } else {
    dest_y = (ULONG)centered_y;
  }

  if (dest_x >= SPY_SCREEN_WIDTH || dest_y >= SPY_SCREEN_HEIGHT) {
    return TRUE;
  }
  if (width > (SPY_SCREEN_WIDTH - dest_x)) {
    width = SPY_SCREEN_WIDTH - dest_x;
  }
  if (height > (SPY_SCREEN_HEIGHT - dest_y)) {
    height = SPY_SCREEN_HEIGHT - dest_y;
  }
  if (width == 0 || height == 0) {
    return TRUE;
  }

  return SAGE_BlitPictureToScreen(preload_background, src_x, src_y, width, height, dest_x, dest_y);
}

static BOOL SpyPreloadBlitBackgroundBlock(ULONG column, ULONG row, ULONG block_size)
{
  ULONG x = column * block_size;
  ULONG y = row * block_size;
  ULONG width = block_size;
  ULONG height = block_size;

  if (preload_background == NULL || preload_background->bitmap == NULL) {
    return TRUE;
  }
  if (x >= SPY_SCREEN_WIDTH || y >= SPY_SCREEN_HEIGHT) {
    return TRUE;
  }
  if (x + width > SPY_SCREEN_WIDTH) {
    width = SPY_SCREEN_WIDTH - x;
  }
  if (y + height > SPY_SCREEN_HEIGHT) {
    height = SPY_SCREEN_HEIGHT - y;
  }
  if (width == 0 || height == 0) {
    return TRUE;
  }

  return SAGE_BlitPictureToScreen(preload_background, x, y, width, height, x, y);
}

static BOOL SpyPreloadDrawVortexReveal(UWORD percent)
{
  ULONG block_size = SPY_PRELOAD_BACKGROUND_REVEAL_BLOCK_SIZE;
  ULONG column_count;
  ULONG row_count;
  ULONG total_blocks;
  ULONG visible_blocks;
  ULONG drawn_blocks = 0;
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
  LONG column;
  LONG row;

  if (
    preload_background == NULL ||
    preload_background->bitmap == NULL ||
    block_size == 0 ||
    preload_background->bitmap->width != SPY_SCREEN_WIDTH ||
    preload_background->bitmap->height != SPY_SCREEN_HEIGHT
  ) {
    return SpyPreloadBlitFullBackground();
  }

  column_count = SpyPreloadDivideRoundUp(SPY_SCREEN_WIDTH, block_size);
  row_count = SpyPreloadDivideRoundUp(SPY_SCREEN_HEIGHT, block_size);
  total_blocks = column_count * row_count;
  if (total_blocks == 0) {
    return TRUE;
  }

  visible_blocks = (total_blocks * (ULONG)percent + 99) / 100;
  if (visible_blocks > total_blocks) {
    visible_blocks = total_blocks;
  }

  left = 0;
  top = 0;
  right = (LONG)column_count - 1;
  bottom = (LONG)row_count - 1;

  while (left <= right && top <= bottom) {
    for (column = left; column <= right; column++) {
      if (drawn_blocks >= visible_blocks) {
        return TRUE;
      }
      if (!SpyPreloadBlitBackgroundBlock((ULONG)column, (ULONG)top, block_size)) {
        return FALSE;
      }
      drawn_blocks++;
    }

    for (row = top + 1; row <= bottom; row++) {
      if (drawn_blocks >= visible_blocks) {
        return TRUE;
      }
      if (!SpyPreloadBlitBackgroundBlock((ULONG)right, (ULONG)row, block_size)) {
        return FALSE;
      }
      drawn_blocks++;
    }

    if (top < bottom) {
      for (column = right - 1; column >= left; column--) {
        if (drawn_blocks >= visible_blocks) {
          return TRUE;
        }
        if (!SpyPreloadBlitBackgroundBlock((ULONG)column, (ULONG)bottom, block_size)) {
          return FALSE;
        }
        drawn_blocks++;
      }
    }

    if (left < right) {
      for (row = bottom - 1; row > top; row--) {
        if (drawn_blocks >= visible_blocks) {
          return TRUE;
        }
        if (!SpyPreloadBlitBackgroundBlock((ULONG)left, (ULONG)row, block_size)) {
          return FALSE;
        }
        drawn_blocks++;
      }
    }

    left++;
    top++;
    right--;
    bottom--;
  }

  return TRUE;
}

static BOOL SpyPreloadDrawBackground(UWORD percent)
{
  UWORD reveal_percent = SpyPreloadRevealPercent(percent);

  if (!SAGE_FillScreen(0x000000)) {
    return FALSE;
  }
  if (reveal_percent >= 100) {
    return SpyPreloadBlitFullBackground();
  }
  return SpyPreloadDrawVortexReveal(reveal_percent);
}

BOOL SpyPreloadDraw(UWORD percent)
{
  ULONG bar_width = SPY_PRELOAD_BAR_WIDTH;
  ULONG bar_height = SPY_PRELOAD_BAR_HEIGHT;
  ULONG inner_width;
  ULONG fill_width;
  ULONG left;
  ULONG top;

  if (percent > 100) {
    percent = 100;
  }
  SpyPreloadUpdateRevealTimer();

  if (bar_width > SPY_SCREEN_WIDTH) {
    bar_width = SPY_SCREEN_WIDTH;
  }
  if (bar_height > SPY_SCREEN_HEIGHT) {
    bar_height = SPY_SCREEN_HEIGHT;
  }

  left = (SPY_SCREEN_WIDTH - bar_width) / 2;
  top = (SPY_SCREEN_HEIGHT - bar_height) / 2;
  inner_width = bar_width - (SPY_PRELOAD_BAR_BORDER * 2);
  fill_width = (inner_width * percent) / 100;

  if (
    !SpyPreloadDrawBackground(percent) ||
    !SAGE_FillArea(left, top, bar_width, bar_height, 0x303030) ||
    !SAGE_FillArea(
      left + SPY_PRELOAD_BAR_BORDER,
      top + SPY_PRELOAD_BAR_BORDER,
      inner_width,
      bar_height - (SPY_PRELOAD_BAR_BORDER * 2),
      0x080808
    )
  ) {
    return FALSE;
  }

  if (fill_width > 0) {
    if (!SAGE_FillArea(
      left + SPY_PRELOAD_BAR_BORDER,
      top + SPY_PRELOAD_BAR_BORDER,
      fill_width,
      bar_height - (SPY_PRELOAD_BAR_BORDER * 2),
      0xc8e8ff
    )) {
      return FALSE;
    }
  }

  return SAGE_RefreshScreen();
}

BOOL SpyPreloadRunBackgroundReveal(UWORD percent)
{
  ULONG frame_count;
  ULONG frame;

  if (preload_reveal_duration_us == 0) {
    return SpyPreloadDraw(percent);
  }

  frame_count = (preload_reveal_duration_us + 19999UL) / 20000UL;
  if (frame_count == 0) {
    frame_count = 1;
  }

  preload_reveal_manual = TRUE;
  for (frame = 0; frame <= frame_count; frame++) {
    preload_reveal_elapsed_us = (preload_reveal_duration_us * frame) / frame_count;
    if (!SpyPreloadDraw(percent)) {
      preload_reveal_manual = FALSE;
      return FALSE;
    }
    if (frame < frame_count) {
      SAGE_Pause(1);
    }
  }

  preload_reveal_manual = FALSE;
  preload_reveal_elapsed_us = preload_reveal_duration_us;
  if (preload_reveal_timer != NULL) {
    SAGE_ElapsedTime(preload_reveal_timer);
  }

  return TRUE;
}

void SpyPreloadRelease(void)
{
  if (preload_reveal_timer != NULL) {
    SAGE_ReleaseTimer(preload_reveal_timer);
    preload_reveal_timer = NULL;
  }
  preload_reveal_elapsed_us = 0;
  preload_reveal_duration_us = 0;
  preload_reveal_manual = FALSE;

  if (preload_background != NULL) {
    SAGE_ReleasePicture(preload_background);
    preload_background = NULL;
  }
}
