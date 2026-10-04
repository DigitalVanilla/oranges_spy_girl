#ifndef SPY_GIRL_TUNNEL_PATTERN_EFFECTS_H
#define SPY_GIRL_TUNNEL_PATTERN_EFFECTS_H

#include <exec/types.h>

typedef enum SpyTunnelPatternEffectMode {
  SPY_TUNNEL_PATTERN_EFFECT_COLUMN_REVEAL = 0,
  SPY_TUNNEL_PATTERN_EFFECT_FLASH,
  SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_BURST,
  SPY_TUNNEL_PATTERN_EFFECT_SHUTTER,
  SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_WAVE,
  SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH,
  SPY_TUNNEL_PATTERN_EFFECT_CUT
} SpyTunnelPatternEffectMode;

BOOL SpyTunnelPatternEffectsInit(void);
BOOL SpyTunnelPatternEffectsUpdate(BOOL paused);
BOOL SpyTunnelPatternEffectsIsActive(void);
BOOL SpyTunnelPatternEffectsNeedsSource(void);
void SpyTunnelPatternEffectsApplyTransition(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  UWORD width,
  UWORD height
);
void SpyTunnelPatternEffectsRelease(void);

#endif
