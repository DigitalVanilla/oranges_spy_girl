#include <exec/types.h>
#include <sage/sage.h>
#include <maggie_flags.h>
#include <maggie_vec.h>

#include "spy_config.h"
#include "spy_input.h"
#include "spy_log.h"

#define KEY_NBR 128
#define SPY_HALF_PI 1.57079632679489661923f
#define SPY_CAMERA_PITCH_LIMIT (SPY_HALF_PI - 0.01f)
#define SPY_TWO_PI 6.28318530717958647692f

static UBYTE keyboard_state[KEY_NBR];

static void ResetCameraDebugOffset(vec3 *position_offset, float *yaw_offset, float *pitch_offset)
{
  if (position_offset != NULL) {
    position_offset->x = 0.0f;
    position_offset->y = 0.0f;
    position_offset->z = 0.0f;
  }
  if (yaw_offset != NULL) {
    *yaw_offset = 0.0f;
  }
  if (pitch_offset != NULL) {
    *pitch_offset = 0.0f;
  }
}

static void ClearCameraKeyState(void)
{
  keyboard_state[SKEY_EN_A] = FALSE;
  keyboard_state[SKEY_EN_D] = FALSE;
  keyboard_state[SKEY_EN_Q] = FALSE;
  keyboard_state[SKEY_EN_E] = FALSE;
  keyboard_state[SKEY_EN_W] = FALSE;
  keyboard_state[SKEY_EN_S] = FALSE;
  keyboard_state[SKEY_EN_UP] = FALSE;
  keyboard_state[SKEY_EN_DOWN] = FALSE;
  keyboard_state[SKEY_EN_LEFT] = FALSE;
  keyboard_state[SKEY_EN_RIGHT] = FALSE;
}

static void WrapAngle(float *angle)
{
  if (angle == NULL) {
    return;
  }

  while (*angle > SPY_TWO_PI) {
    *angle -= SPY_TWO_PI;
  }
  while (*angle < -SPY_TWO_PI) {
    *angle += SPY_TWO_PI;
  }
}

static void ClampPitchAngle(float *angle)
{
  if (angle == NULL) {
    return;
  }

  if (*angle > SPY_CAMERA_PITCH_LIMIT) {
    *angle = SPY_CAMERA_PITCH_LIMIT;
  }
  if (*angle < -SPY_CAMERA_PITCH_LIMIT) {
    *angle = -SPY_CAMERA_PITCH_LIMIT;
  }
}

static void UpdateFreeCameraDebugOffset(vec3 *position_offset, float *yaw_offset, float *pitch_offset)
{
  if (keyboard_state[SKEY_EN_A] == TRUE) {
    position_offset->x += SPY_CAMERA_STEP;
  }
  if (keyboard_state[SKEY_EN_D] == TRUE) {
    position_offset->x -= SPY_CAMERA_STEP;
  }
  if (keyboard_state[SKEY_EN_Q] == TRUE) {
    position_offset->y -= SPY_CAMERA_STEP;
  }
  if (keyboard_state[SKEY_EN_E] == TRUE) {
    position_offset->y += SPY_CAMERA_STEP;
  }
  if (keyboard_state[SKEY_EN_W] == TRUE) {
    position_offset->z -= SPY_CAMERA_STEP;
  }
  if (keyboard_state[SKEY_EN_S] == TRUE) {
    position_offset->z += SPY_CAMERA_STEP;
  }

  if (yaw_offset != NULL) {
    if (keyboard_state[SKEY_EN_LEFT] == TRUE) {
      *yaw_offset += SPY_CAMERA_ROTATION_STEP;
    }
    if (keyboard_state[SKEY_EN_RIGHT] == TRUE) {
      *yaw_offset -= SPY_CAMERA_ROTATION_STEP;
    }
    WrapAngle(yaw_offset);
  }

  if (pitch_offset != NULL) {
    if (keyboard_state[SKEY_EN_UP] == TRUE) {
      *pitch_offset += SPY_CAMERA_ROTATION_STEP;
    }
    if (keyboard_state[SKEY_EN_DOWN] == TRUE) {
      *pitch_offset -= SPY_CAMERA_ROTATION_STEP;
    }
    ClampPitchAngle(pitch_offset);
  }
}

static void UpdateLookAtCameraDebugOffset(vec3 *position_offset, float *yaw_offset, float *pitch_offset)
{
  if (yaw_offset != NULL) {
    if (keyboard_state[SKEY_EN_A] == TRUE) {
      *yaw_offset -= SPY_CAMERA_ROTATION_STEP;
    }
    if (keyboard_state[SKEY_EN_D] == TRUE) {
      *yaw_offset += SPY_CAMERA_ROTATION_STEP;
    }
    WrapAngle(yaw_offset);
  }

  if (pitch_offset != NULL) {
    if (keyboard_state[SKEY_EN_Q] == TRUE) {
      *pitch_offset -= SPY_CAMERA_ROTATION_STEP;
    }
    if (keyboard_state[SKEY_EN_E] == TRUE) {
      *pitch_offset += SPY_CAMERA_ROTATION_STEP;
    }
    WrapAngle(pitch_offset);
  }

  if (position_offset != NULL) {
    if (keyboard_state[SKEY_EN_W] == TRUE) {
      position_offset->z -= SPY_CAMERA_STEP;
    }
    if (keyboard_state[SKEY_EN_S] == TRUE) {
      position_offset->z += SPY_CAMERA_STEP;
    }
  }
}

static void UpdateCameraDebugOffset(
  vec3 *position_offset,
  float *yaw_offset,
  float *pitch_offset,
  SpyMaggieCameraMode camera_mode
)
{
  if (camera_mode == SPY_MAGGIE_CAMERA_LOOK_AT) {
    UpdateLookAtCameraDebugOffset(position_offset, yaw_offset, pitch_offset);
  } else {
    UpdateFreeCameraDebugOffset(position_offset, yaw_offset, pitch_offset);
  }
}

static void ToggleCameraMode(SpyMaggieCameraMode *camera_mode)
{
  if (camera_mode == NULL) {
    return;
  }

  if (*camera_mode == SPY_MAGGIE_CAMERA_FREE) {
    *camera_mode = SPY_MAGGIE_CAMERA_LOOK_AT;
  } else {
    *camera_mode = SPY_MAGGIE_CAMERA_FREE;
  }
}

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
)
{
  SAGE_Event *event = NULL;
  BOOL exit_requested = FALSE;

  /* Runtime controls: X demo mode; demo mode unlocks V overlay, T tunnel, C camera, Space pause, O/P stream speed. */
  while ((event = SAGE_GetEvent()) != NULL) {
    if (event->type == SEVT_KEYDOWN && event->code < KEY_NBR) {
      if (
        event->code == SKEY_EN_V &&
        keyboard_state[event->code] == FALSE &&
        debug_overlay_enabled != NULL &&
        demo_mode_enabled != NULL &&
        *demo_mode_enabled
      ) {
        *debug_overlay_enabled = !*debug_overlay_enabled;
        SpyLog("Debug overlay %s\n", *debug_overlay_enabled ? "ON" : "OFF");
      }
      if (
        event->code == SKEY_EN_X &&
        keyboard_state[event->code] == FALSE &&
        demo_mode_enabled != NULL &&
        !*demo_mode_enabled
      ) {
        *demo_mode_enabled = TRUE;
        ClearCameraKeyState();
        SpyLog("Demo mode ON\n");
      }
      if (
        event->code == SKEY_EN_T &&
        keyboard_state[event->code] == FALSE &&
        demo_mode_enabled != NULL &&
        *demo_mode_enabled &&
        tunnel_visible != NULL
      ) {
        *tunnel_visible = !*tunnel_visible;
        SpyLog("Tunnel %s\n", *tunnel_visible ? "ON" : "OFF");
      }
      if (
        SPY_CAMERA_DEBUG_INPUT_ENABLED &&
        event->code == SKEY_EN_C &&
        keyboard_state[event->code] == FALSE &&
        demo_mode_enabled != NULL &&
        *demo_mode_enabled
      ) {
        ToggleCameraMode(camera_mode);
        ResetCameraDebugOffset(position_offset, yaw_offset, pitch_offset);
        ClearCameraKeyState();
      }
      if (
        event->code == SKEY_EN_SPACE &&
        keyboard_state[event->code] == FALSE &&
        maggie_stream_paused != NULL &&
        demo_mode_enabled != NULL &&
        *demo_mode_enabled
      ) {
        *maggie_stream_paused = !*maggie_stream_paused;
        SpyLog("Maggie stream %s\n", *maggie_stream_paused ? "PAUSED" : "RUNNING");
      }
      if (
        event->code == SKEY_EN_O &&
        keyboard_state[event->code] == FALSE &&
        maggie_ticks_per_frame != NULL &&
        demo_mode_enabled != NULL &&
        *demo_mode_enabled
      ) {
        if (*maggie_ticks_per_frame > 1) {
          (*maggie_ticks_per_frame)--;
        }
      }
      if (
        event->code == SKEY_EN_P &&
        keyboard_state[event->code] == FALSE &&
        maggie_ticks_per_frame != NULL &&
        demo_mode_enabled != NULL &&
        *demo_mode_enabled
      ) {
        if (*maggie_ticks_per_frame < 0xffffffffUL) {
          (*maggie_ticks_per_frame)++;
        }
      }
      keyboard_state[event->code] = TRUE;
    }
    if (event->type == SEVT_KEYUP && event->code < KEY_NBR) {
      keyboard_state[event->code] = FALSE;
    }

    if (event->type == SEVT_MOUSEBT) {
      exit_requested = TRUE;
    } else if (event->type == SEVT_RAWKEY && event->code == SKEY_EN_ESC) {
      exit_requested = TRUE;
    }
  }

  if (
    SPY_CAMERA_DEBUG_INPUT_ENABLED &&
    position_offset != NULL &&
    demo_mode_enabled != NULL &&
    *demo_mode_enabled
  ) {
    UpdateCameraDebugOffset(
      position_offset,
      yaw_offset,
      pitch_offset,
      camera_mode != NULL ? *camera_mode : SPY_MAGGIE_CAMERA_LOOK_AT
    );
  }

  return exit_requested;
}
