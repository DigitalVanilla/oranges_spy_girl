#include <exec/types.h>
#include <sage/sage.h>

#include "spy_config.h"
#include "spy_text.h"

static UWORD SpyTextGlyphIndex(const SpyFont *font, UBYTE character)
{
  UWORD index;

  if (font == NULL || character < font->first_char) {
    return 0;
  }

  index = character - font->first_char;
  if (index >= font->glyph_count) {
    return 0;
  }

  return index;
}

static void SpyTextGetGlyphPosition(const SpyFont *font, UBYTE character, UWORD *x, UWORD *y)
{
  UWORD index = SpyTextGlyphIndex(font, character);

  *x = (index % font->columns) * font->glyph_width;
  *y = (index / font->columns) * font->glyph_height;
}

BOOL SpyTextLoadFont(
  SpyFont *font,
  const char *filename,
  UWORD glyph_width,
  UWORD glyph_height,
  UWORD first_char,
  UWORD glyph_count
)
{
  UWORD rows;

  if (
    font == NULL ||
    filename == NULL ||
    glyph_width == 0 ||
    glyph_height == 0 ||
    glyph_count == 0
  ) {
    return FALSE;
  }

  font->picture = SAGE_LoadPicture((STRPTR)filename);
  if (font->picture == NULL || font->picture->bitmap == NULL) {
    SpyTextReleaseFont(font);
    return FALSE;
  }

  font->glyph_width = glyph_width;
  font->glyph_height = glyph_height;
  font->first_char = first_char;
  font->glyph_count = glyph_count;
  font->columns = font->picture->bitmap->width / glyph_width;
  rows = font->picture->bitmap->height / glyph_height;

  if (font->columns == 0 || rows == 0 || (font->columns * rows) < glyph_count) {
    SpyTextReleaseFont(font);
    return FALSE;
  }

  return TRUE;
}

void SpyTextReleaseFont(SpyFont *font)
{
  if (font == NULL) {
    return;
  }

  if (font->picture != NULL) {
    SAGE_ReleasePicture(font->picture);
  }

  font->picture = NULL;
  font->glyph_width = 0;
  font->glyph_height = 0;
  font->first_char = 0;
  font->glyph_count = 0;
  font->columns = 0;
}

BOOL SpyTextSetFontTransparency(SpyFont *font, ULONG color)
{
  if (font == NULL || font->picture == NULL) {
    return FALSE;
  }

  return SAGE_SetPictureTransparency(font->picture, color);
}

BOOL SpyTextDrawCharToLayer(const SpyFont *font, UWORD layer, UBYTE character, UWORD x, UWORD y)
{
  UWORD glyph_x;
  UWORD glyph_y;

  if (font == NULL || font->picture == NULL || font->columns == 0) {
    return FALSE;
  }

  SpyTextGetGlyphPosition(font, character, &glyph_x, &glyph_y);
  return SAGE_BlitPictureToLayer(
    font->picture,
    glyph_x,
    glyph_y,
    font->glyph_width,
    font->glyph_height,
    layer,
    x,
    y
  );
}

BOOL SpyTextDrawStringToLayer(const SpyFont *font, UWORD layer, const char *text, UWORD x, UWORD y)
{
  if (font == NULL || text == NULL) {
    return FALSE;
  }

  return SpyTextDrawStringPartToLayer(font, layer, text, 0xffff, x, y);
}

BOOL SpyTextDrawStringPartToLayer(const SpyFont *font, UWORD layer, const char *text, UWORD count, UWORD x, UWORD y)
{
  UWORD drawn = 0;

  if (font == NULL || text == NULL) {
    return FALSE;
  }

  while (*text != 0 && drawn < count) {
    if (!SpyTextDrawCharToLayer(font, layer, (UBYTE)*text, x, y)) {
      return FALSE;
    }
    x += font->glyph_width;
    text++;
    drawn++;
  }

  return TRUE;
}

static void SpyTextResetLayer(SpyTextLayer *text_layer)
{
  if (text_layer == NULL) {
    return;
  }

  text_layer->layer = 0;
  text_layer->width = 0;
  text_layer->height = 0;
  text_layer->created = FALSE;
}

BOOL SpyTextCreateLayer(SpyTextLayer *text_layer, UWORD layer, UWORD width, UWORD height)
{
  if (text_layer == NULL || width == 0 || height == 0) {
    return FALSE;
  }

  SpyTextResetLayer(text_layer);
  text_layer->layer = layer;
  text_layer->width = width;
  text_layer->height = height;

  if (!SAGE_CreateLayer(layer, width, height)) {
    SpyTextResetLayer(text_layer);
    return FALSE;
  }

  text_layer->created = TRUE;
  return TRUE;
}

BOOL SpyTextCreateScreenLayer(SpyTextLayer *text_layer, UWORD layer)
{
  return SpyTextCreateLayer(text_layer, layer, SPY_SCREEN_WIDTH, SPY_SCREEN_HEIGHT);
}

void SpyTextReleaseLayer(SpyTextLayer *text_layer)
{
  if (text_layer == NULL) {
    return;
  }

  if (text_layer->created) {
    SAGE_ReleaseLayer(text_layer->layer);
  }

  SpyTextResetLayer(text_layer);
}

BOOL SpyTextSetLayerTransparency(const SpyTextLayer *text_layer, ULONG color)
{
  if (text_layer == NULL || !text_layer->created) {
    return FALSE;
  }

  return SAGE_SetLayerTransparency(text_layer->layer, color);
}

BOOL SpyTextClearLayer(const SpyTextLayer *text_layer, ULONG color)
{
  if (text_layer == NULL || !text_layer->created) {
    return FALSE;
  }

  return SAGE_FillLayer(text_layer->layer, color);
}

BOOL SpyTextDrawChar(const SpyTextLayer *text_layer, const SpyFont *font, UBYTE character, UWORD x, UWORD y)
{
  if (text_layer == NULL || !text_layer->created) {
    return FALSE;
  }

  return SpyTextDrawCharToLayer(font, text_layer->layer, character, x, y);
}

BOOL SpyTextDrawString(const SpyTextLayer *text_layer, const SpyFont *font, const char *text, UWORD x, UWORD y)
{
  if (text_layer == NULL || !text_layer->created) {
    return FALSE;
  }

  return SpyTextDrawStringToLayer(font, text_layer->layer, text, x, y);
}

BOOL SpyTextDrawStringPart(const SpyTextLayer *text_layer, const SpyFont *font, const char *text, UWORD count, UWORD x, UWORD y)
{
  if (text_layer == NULL || !text_layer->created) {
    return FALSE;
  }

  return SpyTextDrawStringPartToLayer(font, text_layer->layer, text, count, x, y);
}

BOOL SpyTextBlitLayerToScreen(const SpyTextLayer *text_layer, UWORD screen_x, UWORD screen_y)
{
  if (text_layer == NULL || !text_layer->created) {
    return FALSE;
  }

  return SAGE_BlitLayerToScreen(text_layer->layer, screen_x, screen_y);
}
