#include <exec/types.h>
#include <sage/sage_timer.h>

#include "spy_config.h"
#include "spy_tunnel_effects.h"
#include "spy_tunnel_reveal.h"

static SAGE_Timer *intro_timer = NULL;
static ULONG intro_hold_us = 0;
static ULONG intro_reveal_us = 0;
static ULONG intro_elapsed_us = 0;
static ULONG intro_phase_us = 0;
static BOOL intro_hidden = FALSE;
static BOOL intro_active = FALSE;

static ULONG SpyTunnelEffectsSecondsToMicros(float seconds)
{
  if (seconds <= 0.0f) {
    return 0;
  }

  return (ULONG)((seconds * (float)STIM_TICKS) + 0.5f);
}

static ULONG SpyTunnelEffectsSageTimeToMicros(ULONG sage_time)
{
  return ((sage_time >> STIM_SECONDS_SHIFT) * STIM_TICKS) + (sage_time & STIM_MICRO_MASK);
}

BOOL SpyTunnelEffectsInit(void)
{
  SpyTunnelEffectsRelease();

  intro_hold_us = SpyTunnelEffectsSecondsToMicros(SPY_TUNNEL_EFFECT_INTRO_HOLD_SECONDS);
  intro_reveal_us = SpyTunnelEffectsSecondsToMicros(SPY_TUNNEL_EFFECT_REVEAL_SECONDS);
  intro_elapsed_us = 0;
  intro_phase_us = 0;
  intro_hidden = intro_reveal_us > 0 && intro_hold_us > 0;
  intro_active = intro_reveal_us > 0 && intro_hold_us == 0;

  if (!intro_hidden && !intro_active) {
    return TRUE;
  }

  intro_timer = SAGE_AllocTimer();
  if (intro_timer == NULL) {
    intro_hidden = FALSE;
    intro_active = FALSE;
    return TRUE;
  }

  SAGE_ElapsedTime(intro_timer);
  return TRUE;
}

void SpyTunnelEffectsUpdate(void)
{
  ULONG elapsed;
  ULONG delta_us;

  if ((!intro_hidden && !intro_active) || intro_timer == NULL) {
    return;
  }

  elapsed = SAGE_ElapsedTime(intro_timer);
  if (elapsed == STIM_OVERFLOW) {
    return;
  }

  delta_us = SpyTunnelEffectsSageTimeToMicros(elapsed);
  if (intro_hidden) {
    intro_elapsed_us += delta_us;
    if (intro_elapsed_us >= intro_hold_us) {
      intro_hidden = FALSE;
      intro_active = TRUE;
      intro_phase_us = 0;
    }
    return;
  }

  intro_phase_us += delta_us;
  if (intro_phase_us >= intro_reveal_us) {
    intro_active = FALSE;
    intro_phase_us = 0;
  }
}

BOOL SpyTunnelEffectsIntroIsHidden(void)
{
  return intro_hidden;
}

BOOL SpyTunnelEffectsIntroIsActive(void)
{
  return intro_active;
}

void SpyTunnelEffectsApplyIntroReveal(
  UWORD *target,
  ULONG target_stride,
  const UWORD *source,
  ULONG source_stride,
  UWORD width,
  UWORD height
)
{
  SpyTunnelRevealApply(
    target,
    target_stride,
    source,
    source_stride,
    width,
    height,
    intro_phase_us,
    intro_reveal_us,
    SPY_TUNNEL_EFFECT_REVEAL_ORIGIN
  );
}

void SpyTunnelEffectsRelease(void)
{
  if (intro_timer != NULL) {
    SAGE_ReleaseTimer(intro_timer);
    intro_timer = NULL;
  }

  intro_hidden = FALSE;
  intro_active = FALSE;
  intro_hold_us = 0;
  intro_reveal_us = 0;
  intro_elapsed_us = 0;
  intro_phase_us = 0;
}
