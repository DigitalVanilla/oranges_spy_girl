#ifndef SPY_TYPEWRITER_H
#define SPY_TYPEWRITER_H

#include <exec/types.h>

#include "spy_text.h"

typedef enum {
  SPY_TEXT_ALIGN_LEFT = 0,
  SPY_TEXT_ALIGN_RIGHT
} SpyTextAlign;

typedef struct {
  const char * const *pages;
  UWORD page_count;
  UWORD current_page;
  UWORD drawn_page;
  UWORD drawn_chars;
  UWORD x;
  UWORD y;
  UWORD width;
  UWORD max_lines;
  UWORD line_spacing;
  SpyTextAlign align;
  ULONG start_delay_us;
  ULONG letter_delay_us;
  ULONG page_hold_us;
  BOOL loop_pages;
} SpyTypewriter;

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
);
BOOL SpyTypewriterUpdate(
  SpyTypewriter *typewriter,
  ULONG elapsed_us,
  const SpyTextLayer *text_layer,
  const SpyFont *font,
  ULONG clear_color
);

#endif
