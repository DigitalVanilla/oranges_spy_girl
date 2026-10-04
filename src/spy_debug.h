#ifndef SPY_GIRL_DEBUG_H
#define SPY_GIRL_DEBUG_H

#include <exec/types.h>
#include <sage/sage_timer.h>
#include <maggie_vec.h>

BOOL SpyDebugConfigureOverlay(void);
UWORD SpyUpdateFpsCounter(SAGE_Timer *timer, UWORD *frame_count, ULONG *elapsed_time, UWORD fps);
void SpyDrawDebugOverlay(
  UWORD fps,
  const vec3 *camera_position,
  float camera_yaw,
  float camera_pitch,
  ULONG animation_frame,
  ULONG animation_frame_count,
  ULONG maggie_ticks_per_frame,
  BOOL stream_paused,
  const char *camera_mode_name
);

#endif
