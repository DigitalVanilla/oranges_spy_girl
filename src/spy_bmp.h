#ifndef SPY_BMP_H
#define SPY_BMP_H

#include <sage/sage_picture.h>

/* Load the BMP formats used by Spy Girl and remap them to the active screen. */
SAGE_Picture *SpyBmpLoadPicture(const char *filename);

#endif
