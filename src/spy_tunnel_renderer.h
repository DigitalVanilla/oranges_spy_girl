#ifndef SPY_GIRL_TUNNEL_RENDERER_H
#define SPY_GIRL_TUNNEL_RENDERER_H

#include <exec/types.h>

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
);

#endif
