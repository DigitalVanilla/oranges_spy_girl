#ifndef SPY_GIRL_INPUT_H
#define SPY_GIRL_INPUT_H

#include <exec/types.h>
#include <maggie_vec.h>

#include "spy_maggie.h"

BOOL SpyInputUpdate(
  vec3 *position_offset,
  float *yaw_offset,
  float *pitch_offset,
  BOOL *debug_overlay_enabled,
  ULONG *maggie_ticks_per_frame,
  BOOL *maggie_stream_paused,
  SpyMaggieCameraMode *camera_mode,
  BOOL *demo_mode_enabled,
  BOOL *tunnel_visible
);

#endif
