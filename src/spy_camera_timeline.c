#include <exec/types.h>
#include <sage/sage_timer.h>

#include "spy_camera_timeline.h"

#define SPY_CAMERA_PI 3.14159265358979323846f
#define SPY_CAMERA_TWO_PI (2.0f * SPY_CAMERA_PI)

static ULONG SpyCameraSecondsToMicros(float seconds)
{
  if (seconds <= 0.0f) {
    return 0;
  }
  return (ULONG)((seconds * (float)STIM_TICKS) + 0.5f);
}

static float SpyCameraWrapAngle(float angle)
{
  while (angle > SPY_CAMERA_PI) {
    angle -= SPY_CAMERA_TWO_PI;
  }
  while (angle < -SPY_CAMERA_PI) {
    angle += SPY_CAMERA_TWO_PI;
  }
  return angle;
}

static void SpyCameraShotToPose(const SpyCameraShot *shot, SpyCameraPose *pose)
{
  pose->position = shot->position;
  pose->yaw = shot->yaw;
  pose->pitch = shot->pitch;
}

static void SpyCameraInterpolatePose(
  const SpyCameraShot *start,
  const SpyCameraShot *end,
  float pct,
  SpyCameraPose *pose
)
{
  float yaw_delta = SpyCameraWrapAngle(end->yaw - start->yaw);

  pose->position.x = SpyAnimLerp(start->position.x, end->position.x, pct);
  pose->position.y = SpyAnimLerp(start->position.y, end->position.y, pct);
  pose->position.z = SpyAnimLerp(start->position.z, end->position.z, pct);
  pose->yaw = SpyCameraWrapAngle(start->yaw + (yaw_delta * pct));
  pose->pitch = SpyAnimLerp(start->pitch, end->pitch, pct);
}

static BOOL SpyCameraTimelineEvaluateLoop(
  const SpyCameraShot *shots,
  UWORD shot_count,
  ULONG elapsed_us,
  UWORD start_index,
  UWORD segment_count,
  float step_seconds,
  SpyCameraPose *pose
)
{
  ULONG step_us = SpyCameraSecondsToMicros(step_seconds);
  ULONG cycle_us;
  ULONG step_elapsed_us;
  ULONG transition_us;
  ULONG transition_start_us;
  UWORD index;
  UWORD next_index;
  UWORD shot_index;
  UWORD next_shot_index;
  const SpyCameraShot *shot;

  if (
    step_us == 0 ||
    start_index >= shot_count ||
    segment_count == 0 ||
    segment_count > (shot_count - start_index) ||
    segment_count > (0xffffffffUL / step_us)
  ) {
    return FALSE;
  }

  cycle_us = step_us * segment_count;
  elapsed_us %= cycle_us;
  index = (UWORD)(elapsed_us / step_us);
  next_index = (UWORD)((index + 1) % segment_count);
  shot_index = (UWORD)(start_index + index);
  next_shot_index = (UWORD)(start_index + next_index);
  step_elapsed_us = elapsed_us % step_us;
  shot = &shots[shot_index];

  transition_us = SpyCameraSecondsToMicros(shot->transition_seconds);
  if (shot->transition == SPY_CAMERA_TRANSITION_CUT || transition_us == 0) {
    SpyCameraShotToPose(shot, pose);
    return TRUE;
  }

  if (transition_us > step_us) {
    transition_us = step_us;
  }
  transition_start_us = step_us - transition_us;
  if (step_elapsed_us < transition_start_us) {
    SpyCameraShotToPose(shot, pose);
    return TRUE;
  }

  SpyCameraInterpolatePose(
    shot,
    &shots[next_shot_index],
    SpyAnimApplyEase(
      shot->easing,
      (float)(step_elapsed_us - transition_start_us) / (float)transition_us
    ),
    pose
  );
  return TRUE;
}

BOOL SpyCameraTimelineEvaluate(
  const SpyCameraShot *shots,
  UWORD shot_count,
  ULONG elapsed_us,
  BOOL loop,
  UWORD loop_start_index,
  float loop_step_seconds,
  SpyCameraPose *pose
)
{
  UWORD index;
  ULONG step_us;
  ULONG first_pass_us;
  UWORD loop_count;

  if (shots == NULL || shot_count == 0 || pose == NULL) {
    return FALSE;
  }

  if (loop) {
    step_us = SpyCameraSecondsToMicros(loop_step_seconds);
    if (step_us == 0 || shot_count > (0xffffffffUL / step_us)) {
      return FALSE;
    }

    if (loop_start_index >= shot_count) {
      loop_start_index = 0;
    }

    first_pass_us = step_us * shot_count;
    if (elapsed_us < first_pass_us || loop_start_index == 0) {
      return SpyCameraTimelineEvaluateLoop(
        shots,
        shot_count,
        elapsed_us,
        0,
        shot_count,
        loop_step_seconds,
        pose
      );
    }

    loop_count = shot_count - loop_start_index;
    return SpyCameraTimelineEvaluateLoop(
      shots,
      shot_count,
      elapsed_us - first_pass_us,
      loop_start_index,
      loop_count,
      loop_step_seconds,
      pose
    );
  }

  SpyCameraShotToPose(&shots[0], pose);
  for (index = 0; index < (shot_count - 1); index++) {
    const SpyCameraShot *shot = &shots[index];
    const SpyCameraShot *next_shot = &shots[index + 1];
    ULONG hold_us = SpyCameraSecondsToMicros(shot->hold_seconds);
    ULONG transition_us = SpyCameraSecondsToMicros(shot->transition_seconds);

    if (elapsed_us < hold_us) {
      SpyCameraShotToPose(shot, pose);
      return TRUE;
    }
    elapsed_us -= hold_us;

    if (shot->transition == SPY_CAMERA_TRANSITION_CUT || transition_us == 0) {
      SpyCameraShotToPose(next_shot, pose);
      continue;
    }

    if (elapsed_us < transition_us) {
      float t = (float)elapsed_us / (float)transition_us;
      float eased = SpyAnimApplyEase(shot->easing, t);

      SpyCameraInterpolatePose(shot, next_shot, eased, pose);
      return TRUE;
    }

    elapsed_us -= transition_us;
    SpyCameraShotToPose(next_shot, pose);
  }

  SpyCameraShotToPose(&shots[shot_count - 1], pose);
  return TRUE;
}

void SpyCameraPoseApplyOffset(SpyCameraPose *pose, const SpyCameraPose *offset)
{
  if (pose == NULL || offset == NULL) {
    return;
  }

  pose->position.x += offset->position.x;
  pose->position.y += offset->position.y;
  pose->position.z += offset->position.z;
  pose->yaw = SpyCameraWrapAngle(pose->yaw + offset->yaw);
  pose->pitch += offset->pitch;
}
