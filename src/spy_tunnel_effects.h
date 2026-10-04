#ifndef SPY_GIRL_TUNNEL_EFFECTS_H
#define SPY_GIRL_TUNNEL_EFFECTS_H

#include <exec/types.h>

BOOL SpyTunnelEffectsInit(void);
void SpyTunnelEffectsUpdate(void);
BOOL SpyTunnelEffectsIntroIsHidden(void);
BOOL SpyTunnelEffectsIntroIsActive(void);
void SpyTunnelEffectsApplyIntroReveal(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  UWORD width,
  UWORD height
);
void SpyTunnelEffectsRelease(void);

#endif
