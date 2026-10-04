#ifndef SPY_GIRL_MAGGIE_H
#define SPY_GIRL_MAGGIE_H

#include <exec/types.h>
#include <maggie_vec.h>

typedef enum {
  SPY_MAGGIE_CAMERA_LOOK_AT = 0,
  SPY_MAGGIE_CAMERA_FREE
} SpyMaggieCameraMode;

BOOL SpyMaggieInit(const char *stream_name, const vec3 *camera_position, float yaw, float pitch, SpyMaggieCameraMode camera_mode);
void SpyMaggieSetCamera(const vec3 *camera_position, float free_yaw, float free_pitch, SpyMaggieCameraMode camera_mode);
const char *SpyMaggieCameraModeName(SpyMaggieCameraMode camera_mode);
BOOL SpyMaggieIsStreamReady(void);
ULONG SpyMaggieGetTicksPerFrame(void);
ULONG SpyMaggieGetCurrentFrame(void);
ULONG SpyMaggieGetFrameCount(void);
void SpyMaggieRender(ULONG stream_ticks, ULONG ticks_per_frame, BOOL appearance_enabled, BOOL stream_paused);
void SpyMaggieRelease(void);

#endif
