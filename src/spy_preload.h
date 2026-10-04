#ifndef SPY_PRELOAD_H
#define SPY_PRELOAD_H

#include <exec/types.h>

BOOL SpyPreloadInit(void);
BOOL SpyPreloadRunBackgroundReveal(UWORD percent);
BOOL SpyPreloadDraw(UWORD percent);
void SpyPreloadRelease(void);

#endif
