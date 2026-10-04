#include <exec/types.h>
#include <proto/exec.h>
#include <stdio.h>
#include <math.h>

#include <sage/sage.h>

#include <proto/Maggie.h>
#include <maggie_flags.h>
#include <maggie_vec.h>

#include "spy_config.h"
#include "spy_log.h"
#include "spy_maggie.h"
#include "spy_maggie_stream.h"

#define CAMERA_EPSILON 0.0001f

struct Library *MaggieBase = NULL;

static BOOL maggie_ready = FALSE;
static BOOL stream_ready = FALSE;
static SpyMaggieStream *maggie_stream = NULL;
static mat4 world_matrix, view_matrix, perspective_matrix;

const char *SpyMaggieCameraModeName(SpyMaggieCameraMode camera_mode)
{
  return (camera_mode == SPY_MAGGIE_CAMERA_FREE) ? "FREE" : "LOOK_AT";
}

static void BuildCameraMatrixFromBasis(mat4 *matrix, const vec3 *camera_position, const vec3 *right, const vec3 *up, const vec3 *forward)
{
  matrix->m[0][0] = right->x;
  matrix->m[0][1] = right->y;
  matrix->m[0][2] = right->z;
  matrix->m[0][3] = 0.0f;
  matrix->m[1][0] = up->x;
  matrix->m[1][1] = up->y;
  matrix->m[1][2] = up->z;
  matrix->m[1][3] = 0.0f;
  matrix->m[2][0] = forward->x;
  matrix->m[2][1] = forward->y;
  matrix->m[2][2] = forward->z;
  matrix->m[2][3] = 0.0f;
  matrix->m[3][0] = camera_position->x;
  matrix->m[3][1] = camera_position->y;
  matrix->m[3][2] = camera_position->z;
  matrix->m[3][3] = 1.0f;
}

static void BuildFreeCameraBasis(float yaw, float pitch, vec3 *right, vec3 *up, vec3 *forward)
{
  vec3 reference_up;
  float pitch_cosine = (float)cos(pitch);

  forward->x = pitch_cosine * (float)sin(yaw);
  forward->y = (float)sin(pitch);
  forward->z = pitch_cosine * (float)cos(yaw);
  vec3_normalise(forward, forward);

  vec3_set(&reference_up, 0.0f, 1.0f, 0.0f);
  vec3_cross(right, &reference_up, forward);
  if (vec3_lensq(right) < CAMERA_EPSILON) {
    vec3_set(&reference_up, 0.0f, 0.0f, 1.0f);
    vec3_cross(right, &reference_up, forward);
  }
  vec3_normalise(right, right);
  vec3_cross(up, forward, right);
  vec3_normalise(up, up);
}

static void BuildLookAtViewMatrix(const vec3 *camera_position, float yaw, float pitch)
{
  mat4 camera_matrix;
  vec3 target;
  vec3 up;

  vec3_set(&target, 0.0f, -1.5f, 0.0f);
  vec3_set(&up, 0.0f, 1.0f, 0.0f);
  mat4_LookAt(&camera_matrix, camera_position, &target, &up);
  mat4_inverseLight(&view_matrix, &camera_matrix);
}

static void BuildFreeCameraViewMatrix(const vec3 *camera_position, float yaw, float pitch)
{
  mat4 camera_matrix;
  vec3 forward;
  vec3 right;
  vec3 up;

  BuildFreeCameraBasis(yaw, pitch, &right, &up, &forward);
  BuildCameraMatrixFromBasis(&camera_matrix, camera_position, &right, &up, &forward);
  mat4_inverseLight(&view_matrix, &camera_matrix);
}

static void BuildViewMatrix(const vec3 *camera_position, float yaw, float pitch, SpyMaggieCameraMode camera_mode)
{
  if (camera_mode == SPY_MAGGIE_CAMERA_FREE) {
    BuildFreeCameraViewMatrix(camera_position, yaw, pitch);
  } else {
    BuildLookAtViewMatrix(camera_position, yaw, pitch);
  }
}

static void InitMaggieScene(const vec3 *camera_position, float yaw, float pitch, SpyMaggieCameraMode camera_mode)
{
  mat4_identity(&world_matrix);
  SpyMaggieSetCamera(camera_position, yaw, pitch, camera_mode);
  mat4_perspective(&perspective_matrix, 60.0f, 5.0f / 8.0f, 0.01f, 120.0f);
}

BOOL SpyMaggieInit(const char *stream_name, const vec3 *camera_position, float yaw, float pitch, SpyMaggieCameraMode camera_mode)
{
  maggie_ready = FALSE;
  stream_ready = FALSE;

  if (!SAGE_ApolloCore()) {
    SAGE_AppliLog("Maggie disabled: Apollo/Vampire core not detected");
    SAGE_SetError(SERR_NO_MAGGIE);
    return FALSE;
  }

  MaggieBase = OpenLibrary((UBYTE *)"maggie.library", 0);
  if (MaggieBase == NULL) {
    SAGE_AppliLog("Maggie disabled: can't open maggie.library");
    SAGE_SetError(SERR_NO_MAGGIE);
    return FALSE;
  }

  InitMaggieScene(camera_position, yaw, pitch, camera_mode);

  maggie_stream = SpyMaggieStreamLoad(stream_name);
  stream_ready = SpyMaggieStreamIsReady(maggie_stream);
  if (!stream_ready) {
    SAGE_AppliLog("Maggie stream unavailable: cannot load %s", stream_name);
  }

  maggie_ready = stream_ready;
  if (maggie_ready) {
    SAGE_AppliLog("Maggie render mode stream");
  } else {
    SAGE_AppliLog("Maggie render disabled: stream source unavailable");
    if (SAGE_GetErrorCode() == SERR_NO_ERROR) {
      SAGE_SetError(SERR_FILEFORMAT);
    }
    return FALSE;
  }

  return maggie_ready;
}

void SpyMaggieSetCamera(const vec3 *camera_position, float yaw, float pitch, SpyMaggieCameraMode camera_mode)
{
  if (camera_position == NULL) {
    return;
  }

  BuildViewMatrix(camera_position, yaw, pitch, camera_mode);
}

BOOL SpyMaggieIsStreamReady(void)
{
  return stream_ready;
}

ULONG SpyMaggieGetTicksPerFrame(void)
{
  return SpyMaggieStreamGetTicksPerFrame(maggie_stream);
}

ULONG SpyMaggieGetCurrentFrame(void)
{
  return SpyMaggieStreamGetCurrentFrame(maggie_stream);
}

ULONG SpyMaggieGetFrameCount(void)
{
  return SpyMaggieStreamGetFrameCount(maggie_stream);
}

static void SpyMaggieDrawPass(
  SAGE_Bitmap *bitmap,
  UWORD draw_mode,
  BOOL clear_depth,
  ULONG stream_ticks,
  ULONG ticks_per_frame,
  BOOL appearance_enabled,
  BOOL stream_paused
)
{
  magBeginScene();

  magSetDrawMode(draw_mode);
  magSetRGB(appearance_enabled ? SPY_LIGHT_AMBIENT_COLOUR : 0x00000000);
  magSetScreenMemory(bitmap->bitmap_buffer, SPY_SCREEN_WIDTH, SPY_SCREEN_HEIGHT);
  if (clear_depth && (draw_mode & MAG_DRAWMODE_DEPTHBUFFER)) {
    magClearDepth(0xffff);
    magClear(MAG_CLEAR_DEPTH);
  }

  magSetWorldMatrix((float *)&world_matrix);
  magSetViewMatrix((float *)&view_matrix);
  magSetPerspectiveMatrix((float *)&perspective_matrix);

  if (!stream_paused) {
    SpyMaggieStreamUpdate(maggie_stream, stream_ticks, ticks_per_frame);
  }
  SpyMaggieStreamDraw(maggie_stream);

  magEndScene();
}

void SpyMaggieRender(ULONG stream_ticks, ULONG ticks_per_frame, BOOL appearance_enabled, BOOL stream_paused)
{
  SAGE_Bitmap *bitmap = SAGE_GetBackBitmap();
  UWORD draw_mode =
    MAG_DRAWMODE_DEPTHBUFFER |
    MAG_DRAWMODE_MIPMAP |
    MAG_DRAWMODE_BILINEAR |
    MAG_DRAWMODE_AFFINE_MAPPING |
    MAG_DRAWMODE_CULL_CCW;

  if (!maggie_ready || bitmap == NULL || bitmap->bitmap_buffer == NULL) {
    return;
  }

  SpyMaggieDrawPass(bitmap, draw_mode, TRUE, stream_ticks, ticks_per_frame, appearance_enabled, stream_paused);
}

void SpyMaggieRelease(void)
{
  maggie_ready = FALSE;
  stream_ready = FALSE;

  if (maggie_stream != NULL) {
    SpyMaggieStreamFree(maggie_stream);
    maggie_stream = NULL;
  }

  if (MaggieBase != NULL) {
    CloseLibrary(MaggieBase);
    MaggieBase = NULL;
  }
}
