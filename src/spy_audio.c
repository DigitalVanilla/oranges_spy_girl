#include <sage/sage.h>

#include "spy_audio.h"
#include "spy_config.h"

static BOOL audio_ready = FALSE;
static BOOL music_added = FALSE;
static BOOL music_playing = FALSE;
static SAGE_Music *music = NULL;

void SpyAudioInit(void)
{
  if (SAGE_InitAudioModule()) {
    audio_ready = TRUE;
  } else {
    SAGE_AppliLog("Audio disabled: %s", SAGE_GetErrorString());
    SAGE_SetError(SERR_NO_ERROR);
  }
}

void SpyAudioStartMusic(void)
{
  if (!audio_ready) {
    return;
  }

  music = SAGE_LoadMusic(SPY_MUSIC_FILENAME);
  if (music == NULL) {
    SAGE_AppliLog("Music disabled: %s", SAGE_GetErrorString());
    SAGE_SetError(SERR_NO_ERROR);
  } else if (!SAGE_AddMusic(SPY_MUSIC_SLOT, music)) {
    SAGE_AppliLog("Music disabled: %s", SAGE_GetErrorString());
    SAGE_ReleaseMusic(music);
    music = NULL;
    SAGE_SetError(SERR_NO_ERROR);
  } else {
    music = NULL;
    music_added = TRUE;
    if (!SAGE_PlayMusic(SPY_MUSIC_SLOT)) {
      SAGE_AppliLog("Music disabled: %s", SAGE_GetErrorString());
      SAGE_FreeMusic(SPY_MUSIC_SLOT);
      music_added = FALSE;
      SAGE_SetError(SERR_NO_ERROR);
    } else {
      music_playing = TRUE;
    }
  }
}

void SpyAudioRelease(void)
{
  if (music_playing) {
    SAGE_StopMusic();
    music_playing = FALSE;
  }
  if (music_added) {
    SAGE_FreeMusic(SPY_MUSIC_SLOT);
    music_added = FALSE;
  } else if (music != NULL) {
    SAGE_ReleaseMusic(music);
    music = NULL;
  }
  if (audio_ready) {
    SAGE_ReleaseAudioModule();
    audio_ready = FALSE;
  }
}
