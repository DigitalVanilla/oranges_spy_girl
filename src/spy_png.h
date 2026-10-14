#ifndef SPY_GIRL_PNG_H
#define SPY_GIRL_PNG_H

#include <sage/sage_picture.h>

/* Load a non-interlaced, indexed 8-bit PNG without using datatypes. */
SAGE_Picture *SpyPngLoadPicture(const char *filename);

#endif
