#include <exec/types.h>

#include <sage/sage.h>
#include <sage/sage_timer.h>

#include "spy_anim.h"
#include "spy_config.h"
#include "spy_logo.h"
#include "spy_log.h"

static SAGE_Picture *logo_picture = NULL;
static BOOL logo_ready = FALSE;
static BOOL logo_bank_ready = FALSE;
static ULONG logo_start_us = 0;
static ULONG logo_piece_delay_us = 0;
static ULONG logo_bounce_us = 0;

static ULONG SpyLogoSecondsToMicros(float seconds)
{
  if (seconds <= 0.0f) {
    return 0;
  }

  return (ULONG)((seconds * (float)STIM_TICKS) + 0.5f);
}

static float SpyLogoClamp01(float value)
{
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

static ULONG SpyLogoRevealEndTime(void)
{
  if (SPY_LOGO_PIECES == 0) {
    return logo_start_us;
  }

  return logo_start_us + ((ULONG)(SPY_LOGO_PIECES - 1) * logo_piece_delay_us) + logo_bounce_us;
}

BOOL SpyLogoInit(void)
{
  UWORD piece;

  SpyLogoRelease();

  logo_start_us = SpyLogoSecondsToMicros(SPY_LOGO_START_SECONDS);
  logo_piece_delay_us = SpyLogoSecondsToMicros(SPY_LOGO_PIECE_DELAY_SECONDS);
  logo_bounce_us = SpyLogoSecondsToMicros(SPY_LOGO_BOUNCE_SECONDS);

  if (SPY_LOGO_PIECES == 0 || SPY_LOGO_PIECE_WIDTH == 0 || SPY_LOGO_HEIGHT == 0) {
    SpyLog("Logo disabled: invalid sprite layout\n");
    return TRUE;
  }

  logo_picture = SAGE_LoadPicture((STRPTR)SPY_LOGO_FILENAME);
  if (logo_picture == NULL || logo_picture->bitmap == NULL) {
    SpyLog("Logo disabled: cannot load %s\n", SPY_LOGO_FILENAME);
    SpyLogoRelease();
    return FALSE;
  }

  if (logo_picture->bitmap->width != SPY_LOGO_WIDTH || logo_picture->bitmap->height != SPY_LOGO_HEIGHT) {
    SpyLog(
      "Logo disabled: %s must be %dx%d, got %dx%d\n",
      SPY_LOGO_FILENAME,
      SPY_LOGO_WIDTH,
      SPY_LOGO_HEIGHT,
      logo_picture->bitmap->width,
      logo_picture->bitmap->height
    );
    SpyLogoRelease();
    return FALSE;
  }

  if (SPY_LOGO_WIDTH != (SPY_LOGO_PIECES * SPY_LOGO_PIECE_WIDTH)) {
    SpyLog("Logo disabled: width does not match pieces * piece width\n");
    SpyLogoRelease();
    return FALSE;
  }

  if (!SAGE_CreateSpriteBank(SPY_LOGO_SPRITE_BANK, SPY_LOGO_PIECES, logo_picture)) {
    SpyLogoRelease();
    return FALSE;
  }
  logo_bank_ready = TRUE;

  if (!SAGE_SetPictureTransparency(logo_picture, SPY_LOGO_TRANSPARENT)) {
    SpyLogoRelease();
    return FALSE;
  }

  if (!SAGE_SetSpriteBankTransparency(SPY_LOGO_SPRITE_BANK, SPY_LOGO_TRANSPARENT)) {
    SpyLogoRelease();
    return FALSE;
  }

  for (piece = 0; piece < SPY_LOGO_PIECES; piece++) {
    if (!SAGE_AddSpriteToBank(
      SPY_LOGO_SPRITE_BANK,
      piece,
      piece * SPY_LOGO_PIECE_WIDTH,
      0,
      SPY_LOGO_PIECE_WIDTH,
      SPY_LOGO_HEIGHT,
      SSPR_HS_TOPLEFT
    )) {
      SpyLogoRelease();
      return FALSE;
    }
  }

  logo_ready = TRUE;
  return TRUE;
}

BOOL SpyLogoRender(ULONG elapsed_us)
{
  UWORD piece;

  if (!logo_ready || logo_picture == NULL) {
    return TRUE;
  }

  if (elapsed_us < logo_start_us) {
    return TRUE;
  }

  if (elapsed_us >= SpyLogoRevealEndTime()) {
    return SAGE_BlitPictureToScreen(logo_picture, 0, 0, SPY_LOGO_WIDTH, SPY_LOGO_HEIGHT, SPY_LOGO_FINAL_X, SPY_LOGO_FINAL_Y);
  }

  for (piece = 0; piece < SPY_LOGO_PIECES; piece++) {
    ULONG piece_start_us = logo_start_us + ((ULONG)piece * logo_piece_delay_us);
    ULONG piece_elapsed_us;
    float t;
    float eased;
    LONG x_pos;
    LONG y_pos;

    if (elapsed_us < piece_start_us) {
      continue;
    }

    piece_elapsed_us = elapsed_us - piece_start_us;
    if (logo_bounce_us == 0 || piece_elapsed_us >= logo_bounce_us) {
      x_pos = SPY_LOGO_FINAL_X + (piece * SPY_LOGO_PIECE_WIDTH);
      y_pos = SPY_LOGO_FINAL_Y;
    } else {
      t = SpyLogoClamp01((float)piece_elapsed_us / (float)logo_bounce_us);
      eased = SpyAnimEaseOutElastic(t);
      x_pos = (LONG)(SpyAnimLerp((float)(SPY_LOGO_START_X + (piece * SPY_LOGO_PIECE_WIDTH)), (float)(SPY_LOGO_FINAL_X + (piece * SPY_LOGO_PIECE_WIDTH)), eased) + 0.5f);
      y_pos = (LONG)(SpyAnimLerp((float)SPY_LOGO_START_Y, (float)SPY_LOGO_FINAL_Y, eased) + 0.5f);
    }

    if (!SAGE_BlitSpriteToScreen(SPY_LOGO_SPRITE_BANK, piece, x_pos, y_pos)) {
      return FALSE;
    }
  }

  return TRUE;
}

void SpyLogoRelease(void)
{
  if (logo_bank_ready) {
    SAGE_ReleaseSpriteBank(SPY_LOGO_SPRITE_BANK);
    logo_bank_ready = FALSE;
  }

  if (logo_ready) {
    logo_ready = FALSE;
  }

  if (logo_picture != NULL) {
    SAGE_ReleasePicture(logo_picture);
    logo_picture = NULL;
  }
}
