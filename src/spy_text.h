#ifndef SPY_GIRL_TEXT_H
#define SPY_GIRL_TEXT_H

#include <exec/types.h>
#include <sage/sage.h>

typedef struct {
  SAGE_Picture *picture;
  UWORD glyph_width;
  UWORD glyph_height;
  UWORD first_char;
  UWORD glyph_count;
  UWORD columns;
} SpyFont;

typedef struct {
  UWORD layer;
  UWORD width;
  UWORD height;
  BOOL created;
} SpyTextLayer;

BOOL SpyTextLoadFont(
  SpyFont *font,
  const char *filename,
  UWORD glyph_width,
  UWORD glyph_height,
  UWORD first_char,
  UWORD glyph_count
);
void SpyTextReleaseFont(SpyFont *font);
BOOL SpyTextSetFontTransparency(SpyFont *font, ULONG color);

BOOL SpyTextDrawCharToLayer(const SpyFont *font, UWORD layer, UBYTE character, UWORD x, UWORD y);
BOOL SpyTextDrawStringToLayer(const SpyFont *font, UWORD layer, const char *text, UWORD x, UWORD y);
BOOL SpyTextDrawStringPartToLayer(const SpyFont *font, UWORD layer, const char *text, UWORD count, UWORD x, UWORD y);

BOOL SpyTextCreateLayer(SpyTextLayer *text_layer, UWORD layer, UWORD width, UWORD height);
BOOL SpyTextCreateScreenLayer(SpyTextLayer *text_layer, UWORD layer);
void SpyTextReleaseLayer(SpyTextLayer *text_layer);
BOOL SpyTextSetLayerTransparency(const SpyTextLayer *text_layer, ULONG color);
BOOL SpyTextClearLayer(const SpyTextLayer *text_layer, ULONG color);
BOOL SpyTextDrawChar(const SpyTextLayer *text_layer, const SpyFont *font, UBYTE character, UWORD x, UWORD y);
BOOL SpyTextDrawString(const SpyTextLayer *text_layer, const SpyFont *font, const char *text, UWORD x, UWORD y);
BOOL SpyTextDrawStringPart(const SpyTextLayer *text_layer, const SpyFont *font, const char *text, UWORD count, UWORD x, UWORD y);
BOOL SpyTextBlitLayerToScreen(const SpyTextLayer *text_layer, UWORD screen_x, UWORD screen_y);

#endif
