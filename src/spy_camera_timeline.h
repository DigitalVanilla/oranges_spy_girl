#ifndef SPY_GIRL_CAMERA_TIMELINE_H
#define SPY_GIRL_CAMERA_TIMELINE_H

#include <exec/types.h>
#include <maggie_vec.h>

#include "spy_anim.h"

#define SPY_CAMERA_DEGREES(value) ((value) * 0.01745329251994329577f)

typedef enum SpyCameraTransition {
  SPY_CAMERA_TRANSITION_TWEEN = 0,
  SPY_CAMERA_TRANSITION_CUT
} SpyCameraTransition;

typedef struct SpyCameraPose {
  vec3 position;
  float yaw;
  float pitch;
} SpyCameraPose;

typedef struct SpyCameraShot {
  vec3 position;
  float yaw;
  float pitch;
  float hold_seconds;
  float transition_seconds;
  SpyCameraTransition transition;
  SpyAnimEaseMode easing;
} SpyCameraShot;

BOOL SpyCameraTimelineEvaluate(
  const SpyCameraShot *shots,
  UWORD shot_count,
  ULONG elapsed_us,
  BOOL loop,
  UWORD loop_start_index,
  float loop_step_seconds,
  SpyCameraPose *pose
);
void SpyCameraPoseApplyOffset(SpyCameraPose *pose, const SpyCameraPose *offset);

#endif
