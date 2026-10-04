#include <exec/types.h>

#include "spy_config.h"
#include "spy_tunnel_renderer.h"

void SpyTunnelRenderFrame060Asm(
  UWORD *dst,
  const UWORD *distance_row,
  const UWORD *angle_row,
  ULONG table_stride,
  ULONG dst_stride,
  const UWORD *texture,
  ULONG width,
  ULONG height,
  ULONG shift_x,
  ULONG shift_y,
  const UWORD *skip_start,
  const UWORD *skip_count
);

void SpyTunnelRenderFrame2x060Asm(
  UWORD *dst,
  const UWORD *distance_row,
  const UWORD *angle_row,
  ULONG table_stride,
  ULONG dst_stride,
  const UWORD *texture,
  ULONG width,
  ULONG height,
  ULONG shift_x,
  ULONG shift_y,
  const UWORD *skip_start,
  const UWORD *skip_count
);

void SpyTunnelRendererDraw(
  UWORD *dst,
  const UWORD *distance_row,
  const UWORD *angle_row,
  ULONG table_stride,
  ULONG dst_stride,
  const UWORD *texture,
  ULONG width,
  ULONG height,
  ULONG shift_x,
  ULONG shift_y,
  const UWORD *skip_start,
  const UWORD *skip_count
)
{
#if SPY_TUNNEL_RENDER_PIXEL_SCALE == 2
  SpyTunnelRenderFrame2x060Asm(
    dst,
    distance_row,
    angle_row,
    table_stride,
    dst_stride,
    texture,
    width,
    height,
    shift_x,
    shift_y,
    skip_start,
    skip_count
  );
#else
  SpyTunnelRenderFrame060Asm(
    dst,
    distance_row,
    angle_row,
    table_stride,
    dst_stride,
    texture,
    width,
    height,
    shift_x,
    shift_y,
    skip_start,
    skip_count
  );
#endif
}
