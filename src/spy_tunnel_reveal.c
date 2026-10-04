#include <exec/types.h>

#include "spy_config.h"
#include "spy_tunnel_reveal.h"

static ULONG SpyTunnelRevealDivideRoundUp(ULONG value, ULONG divisor)
{
  if (divisor == 0) {
    return 0;
  }

  return (value + divisor - 1) / divisor;
}

static BOOL SpyTunnelRevealStartsFromRight(SpyTunnelRevealOrigin origin)
{
  return
    origin == SPY_TUNNEL_REVEAL_ORIGIN_TOP_RIGHT ||
    origin == SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_RIGHT;
}

static BOOL SpyTunnelRevealStartsFromBottom(SpyTunnelRevealOrigin origin)
{
  return
    origin == SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_LEFT ||
    origin == SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_RIGHT;
}

static void SpyTunnelRevealCopyBlock(
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

void SpyTunnelRevealApply(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  UWORD width,
  UWORD height,
  ULONG phase_us,
  ULONG duration_us,
  SpyTunnelRevealOrigin origin
)
{
  ULONG row_count;
  ULONG column_count;
  ULONG total_steps;
  ULONG progress_step;
  ULONG row_index;
  ULONG row_order_index;
  ULONG row_start_step;
  ULONG visible_columns;
  ULONG column_index;
  ULONG column_order_index;
  ULONG x;
  ULONG y;
  ULONG copy_width;
  ULONG copy_height;

  if (
    target == NULL || source == NULL || width == 0 || height == 0 ||
    duration_us == 0
  ) {
    return;
  }

  row_count = SpyTunnelRevealDivideRoundUp(height, SPY_TUNNEL_REVEAL_STEP_HEIGHT);
  column_count = SpyTunnelRevealDivideRoundUp(width, SPY_TUNNEL_REVEAL_COLUMN_WIDTH);
  if (row_count == 0 || column_count == 0) {
    return;
  }

  total_steps = column_count + ((row_count - 1) * SPY_TUNNEL_REVEAL_ROW_DELAY_STEPS);
  if (total_steps == 0) {
    return;
  }

  progress_step = (phase_us * total_steps) / duration_us;
  if (progress_step >= total_steps) {
    progress_step = total_steps - 1;
  }

  for (row_order_index = 0; row_order_index < row_count; row_order_index++) {
    row_start_step = row_order_index * SPY_TUNNEL_REVEAL_ROW_DELAY_STEPS;
    if (progress_step < row_start_step) {
      continue;
    }

    visible_columns = (progress_step - row_start_step) + 1;
    if (visible_columns > column_count) {
      visible_columns = column_count;
    }

    row_index = row_order_index;
    if (SpyTunnelRevealStartsFromBottom(origin)) {
      row_index = (row_count - 1) - row_order_index;
    }
    y = row_index * SPY_TUNNEL_REVEAL_STEP_HEIGHT;
    if (y >= height) {
      continue;
    }

    copy_height = SPY_TUNNEL_REVEAL_STEP_HEIGHT;
    if (y + copy_height > height) {
      copy_height = height - y;
    }

    for (column_order_index = 0; column_order_index < visible_columns; column_order_index++) {
      column_index = column_order_index;
      if (SpyTunnelRevealStartsFromRight(origin)) {
        column_index = (column_count - 1) - column_order_index;
      }
      x = column_index * SPY_TUNNEL_REVEAL_COLUMN_WIDTH;
      if (x >= width) {
        continue;
      }

      copy_width = SPY_TUNNEL_REVEAL_COLUMN_WIDTH;
      if (x + copy_width > width) {
        copy_width = width - x;
      }

      SpyTunnelRevealCopyBlock(
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
