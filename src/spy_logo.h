#ifndef SPY_LOGO_H
#define SPY_LOGO_H

#include <exec/types.h>

BOOL SpyLogoInit(void);
BOOL SpyLogoRender(ULONG elapsed_us);
void SpyLogoRelease(void);

#endif
