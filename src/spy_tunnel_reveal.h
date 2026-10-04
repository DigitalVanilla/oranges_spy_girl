#ifndef SPY_GIRL_TUNNEL_REVEAL_H
#define SPY_GIRL_TUNNEL_REVEAL_H

#include <exec/types.h>

typedef enum SpyTunnelRevealOrigin {
  SPY_TUNNEL_REVEAL_ORIGIN_TOP_LEFT = 0,
  SPY_TUNNEL_REVEAL_ORIGIN_TOP_RIGHT,
  SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_LEFT,
  SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_RIGHT
} SpyTunnelRevealOrigin;

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
);

#endif
