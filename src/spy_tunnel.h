#ifndef SPY_GIRL_TUNNEL_H
#define SPY_GIRL_TUNNEL_H

#include <exec/types.h>

BOOL SpyTunnelInit(const char *picture_name);
BOOL SpyTunnelClearScreenGaps(UWORD tunnel_y);
void SpyTunnelRenderFrame(UWORD tunnel_y, ULONG demo_elapsed_us);
void SpyTunnelRestartIntroReveal(void);
void SpyTunnelRelease(void);

#endif
