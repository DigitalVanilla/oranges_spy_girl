#include <exec/types.h>

#include "spy_typewriter.h"

#define SPY_TYPEWRITER_INVALID_DRAWN 0xffff

static UWORD SpyTypewriterLineGlyphCount(const SpyTypewriter *typewriter, const SpyFont *font, const char *line)
{
  UWORD count = 0;
  ULONG width;

  if (
    typewriter == NULL ||
    font == NULL ||
    line == NULL ||
    font->glyph_width == 0 ||
    typewriter->width == 0
  ) {
    return 0;
  }

  width = 0;
  while (*line != 0 && *line != '\n') {
    if ((width + font->glyph_width) <= typewriter->width && count < SPY_TYPEWRITER_INVALID_DRAWN) {
      count++;
      width += font->glyph_width;
    }
    line++;
  }

  return count;
}

static ULONG SpyTypewriterLineStartX(const SpyTypewriter *typewriter, const SpyFont *font, const char *line)
{
  UWORD glyph_count;
  ULONG text_width;

  if (typewriter == NULL || font == NULL || line == NULL) {
    return 0;
  }

  if (typewriter->align != SPY_TEXT_ALIGN_RIGHT) {
    return typewriter->x;
  }

  glyph_count = SpyTypewriterLineGlyphCount(typewriter, font, line);
  text_width = (ULONG)glyph_count * (ULONG)font->glyph_width;
  if (text_width >= typewriter->width) {
    return typewriter->x;
  }

  return (ULONG)typewriter->x + ((ULONG)typewriter->width - text_width);
}

static UWORD SpyTypewriterPageGlyphCount(const SpyTypewriter *typewriter, const SpyFont *font, const char *page)
{
  UWORD count = 0;
  UWORD line = 0;
  ULONG x;
  ULONG right;

  if (
    typewriter == NULL ||
    font == NULL ||
    page == NULL ||
    font->glyph_width == 0 ||
    typewriter->max_lines == 0 ||
    typewriter->width == 0
  ) {
    return 0;
  }

  x = SpyTypewriterLineStartX(typewriter, font, page);
  right = (ULONG)typewriter->x + (ULONG)typewriter->width;

  while (*page != 0 && line < typewriter->max_lines) {
    if (*page == '\n') {
      line++;
      page++;
      x = SpyTypewriterLineStartX(typewriter, font, page);
      continue;
    }

    if ((x + font->glyph_width) <= right && count < SPY_TYPEWRITER_INVALID_DRAWN) {
      count++;
      x += font->glyph_width;
    }
    page++;
  }

  return count;
}

static ULONG SpyTypewriterPageTypingDuration(const SpyTypewriter *typewriter, UWORD glyph_count)
{
  if (typewriter == NULL || glyph_count == 0 || typewriter->letter_delay_us == 0) {
    return 0;
  }

  return (ULONG)glyph_count * typewriter->letter_delay_us;
}

static ULONG SpyTypewriterPageDuration(const SpyTypewriter *typewriter, const SpyFont *font, const char *page)
{
  UWORD glyph_count = SpyTypewriterPageGlyphCount(typewriter, font, page);

  return SpyTypewriterPageTypingDuration(typewriter, glyph_count) + typewriter->page_hold_us;
}

static UWORD SpyTypewriterVisibleChars(const SpyTypewriter *typewriter, UWORD glyph_count, ULONG page_elapsed_us)
{
  ULONG visible;
  ULONG typing_duration;

  if (typewriter == NULL || glyph_count == 0) {
    return 0;
  }

  if (typewriter->letter_delay_us == 0) {
    return glyph_count;
  }

  typing_duration = SpyTypewriterPageTypingDuration(typewriter, glyph_count);
  if (page_elapsed_us >= typing_duration) {
    return glyph_count;
  }

  visible = (page_elapsed_us / typewriter->letter_delay_us) + 1;
  if (visible > glyph_count) {
    return glyph_count;
  }
  return (UWORD)visible;
}

static ULONG SpyTypewriterTotalDuration(const SpyTypewriter *typewriter, const SpyFont *font)
{
  UWORD index;
  ULONG total = 0;

  if (typewriter == NULL || typewriter->pages == NULL || font == NULL) {
    return 0;
  }

  for (index = 0; index < typewriter->page_count; index++) {
    total += SpyTypewriterPageDuration(typewriter, font, typewriter->pages[index]);
  }

  return total;
}

static void SpyTypewriterComputePage(
  SpyTypewriter *typewriter,
  const SpyFont *font,
  ULONG elapsed_us,
  UWORD *page_index,
  UWORD *visible_chars
)
{
  UWORD index;
  UWORD glyph_count;
  ULONG page_duration;
  ULONG total_duration;

  *page_index = 0;
  *visible_chars = 0;

  if (typewriter == NULL || typewriter->pages == NULL || font == NULL || typewriter->page_count == 0) {
    return;
  }

  if (elapsed_us < typewriter->start_delay_us) {
    return;
  }

  elapsed_us -= typewriter->start_delay_us;
  if (typewriter->loop_pages) {
    total_duration = SpyTypewriterTotalDuration(typewriter, font);
    if (total_duration > 0) {
      elapsed_us %= total_duration;
    }
  }

  for (index = 0; index < typewriter->page_count; index++) {
    glyph_count = SpyTypewriterPageGlyphCount(typewriter, font, typewriter->pages[index]);
    page_duration = SpyTypewriterPageTypingDuration(typewriter, glyph_count) + typewriter->page_hold_us;

    if (index == (typewriter->page_count - 1) && !typewriter->loop_pages && elapsed_us >= page_duration) {
      *page_index = index;
      *visible_chars = glyph_count;
      return;
    }

    if (page_duration == 0 || elapsed_us < page_duration) {
      *page_index = index;
      *visible_chars = SpyTypewriterVisibleChars(typewriter, glyph_count, elapsed_us);
      return;
    }

    elapsed_us -= page_duration;
  }

  *page_index = typewriter->page_count - 1;
  *visible_chars = SpyTypewriterPageGlyphCount(typewriter, font, typewriter->pages[*page_index]);
}

static BOOL SpyTypewriterDrawPage(
  const SpyTypewriter *typewriter,
  const SpyTextLayer *text_layer,
  const SpyFont *font,
  const char *page,
  UWORD visible_chars
)
{
  UWORD drawn = 0;
  UWORD line = 0;
  ULONG x;
  ULONG y;
  ULONG right;

  if (typewriter == NULL || text_layer == NULL || font == NULL || page == NULL) {
    return FALSE;
  }

  x = SpyTypewriterLineStartX(typewriter, font, page);
  y = typewriter->y;
  right = (ULONG)typewriter->x + (ULONG)typewriter->width;

  while (*page != 0 && line < typewriter->max_lines && drawn < visible_chars) {
    if (*page == '\n') {
      line++;
      y += font->glyph_height + typewriter->line_spacing;
      page++;
      x = SpyTypewriterLineStartX(typewriter, font, page);
      continue;
    }

    if ((x + font->glyph_width) <= right) {
      if (!SpyTextDrawChar(text_layer, font, (UBYTE)*page, (UWORD)x, (UWORD)y)) {
        return FALSE;
      }
      drawn++;
      x += font->glyph_width;
    }
    page++;
  }

  return TRUE;
}

void SpyTypewriterInit(
  SpyTypewriter *typewriter,
  const char * const *pages,
  UWORD page_count,
  ULONG start_delay_us,
  ULONG letter_delay_us,
  ULONG page_hold_us,
  UWORD x,
  UWORD y,
  UWORD width,
  UWORD max_lines,
  UWORD line_spacing,
  SpyTextAlign align,
  BOOL loop_pages
)
{
  if (typewriter == NULL) {
    return;
  }

  typewriter->pages = pages;
  typewriter->page_count = page_count;
  typewriter->current_page = 0;
  typewriter->drawn_page = SPY_TYPEWRITER_INVALID_DRAWN;
  typewriter->drawn_chars = SPY_TYPEWRITER_INVALID_DRAWN;
  typewriter->x = x;
  typewriter->y = y;
  typewriter->width = width;
  typewriter->max_lines = max_lines;
  typewriter->line_spacing = line_spacing;
  typewriter->align = align;
  typewriter->start_delay_us = start_delay_us;
  typewriter->letter_delay_us = letter_delay_us;
  typewriter->page_hold_us = page_hold_us;
  typewriter->loop_pages = loop_pages;
}

BOOL SpyTypewriterUpdate(
  SpyTypewriter *typewriter,
  ULONG elapsed_us,
  const SpyTextLayer *text_layer,
  const SpyFont *font,
  ULONG clear_color
)
{
  UWORD page_index;
  UWORD visible_chars;

  if (typewriter == NULL || text_layer == NULL || font == NULL) {
    return FALSE;
  }

  SpyTypewriterComputePage(typewriter, font, elapsed_us, &page_index, &visible_chars);
  if (page_index == typewriter->drawn_page && visible_chars == typewriter->drawn_chars) {
    return TRUE;
  }

  if (!SpyTextClearLayer(text_layer, clear_color)) {
    return FALSE;
  }

  if (
    typewriter->pages != NULL &&
    page_index < typewriter->page_count &&
    visible_chars > 0 &&
    !SpyTypewriterDrawPage(typewriter, text_layer, font, typewriter->pages[page_index], visible_chars)
  ) {
    return FALSE;
  }

  typewriter->current_page = page_index;
  typewriter->drawn_page = page_index;
  typewriter->drawn_chars = visible_chars;
  return TRUE;
}
