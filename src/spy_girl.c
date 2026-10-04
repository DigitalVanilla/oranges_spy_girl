#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include <math.h>

// Sage
#include <sage/sage.h>
#include <sage/sage_timer.h>

// MaggieLibrary
#include <maggie_flags.h>
#include <maggie_vec.h>

#include "spy_anim.h"
#include "spy_audio.h"
#include "spy_camera_timeline.h"
#include "spy_config.h"
#include "spy_debug.h"
#include "spy_input.h"
#include "spy_log.h"
#include "spy_logo.h"
#include "spy_maggie.h"
#include "spy_preload.h"
#include "spy_text.h"
#include "spy_tunnel.h"
#include "spy_typewriter.h"

#if SPY_MAGGIE_TICKS_PER_SECOND == 0 || SPY_MAGGIE_TICKS_PER_SECOND > STIM_TICKS
#error SPY_MAGGIE_TICKS_PER_SECOND must define a valid timer rate
#endif

#if (STIM_TICKS % SPY_MAGGIE_TICKS_PER_SECOND) != 0
#error SPY_MAGGIE_TICKS_PER_SECOND must divide evenly into STIM_TICKS
#endif

#define SPY_INTRO_CONSOLE "CON:0/0/640/512/Spy Girl/CLOSE/AUTO"
#define SPY_INTRO_DIZ_PATH "PROGDIR:file_id.diz"
#define SPY_INTRO_DIZ_FALLBACK "file_id.diz"
#define SPY_INTRO_PAUSE_TICKS 200

// ******************************************
// MAIN
// ******************************************

typedef struct {
  LONG y;
  float hold_seconds;
  float move_seconds;
  SpyAnimEaseMode easing;
} SpyTunnelYStep;

static const SpyTunnelYStep spy_tunnel_y_timeline[] = {
  SPY_TUNNEL_EFFECT_Y_TIMELINE
};

static const SpyCameraShot spy_camera_timeline[] = {
  SPY_CAMERA_TIMELINE
};

static const SpyCameraPose spy_free_camera_pose = {
  SPY_FREE_CAMERA_POSITION,
  SPY_FREE_CAMERA_YAW,
  SPY_FREE_CAMERA_PITCH
};

static const SpyCameraPose spy_look_at_camera_pose = {
  SPY_LOOK_AT_CAMERA_POSITION,
  SPY_LOOK_AT_CAMERA_YAW,
  SPY_LOOK_AT_CAMERA_PITCH
};

static BOOL SpySetDemoModeCameraPose(SpyCameraPose *pose)
{
  if (pose == NULL) {
    return FALSE;
  }

  if ((sizeof(spy_camera_timeline) / sizeof(spy_camera_timeline[0])) == 0) {
    return FALSE;
  }

  pose->position = spy_camera_timeline[0].position;
  pose->yaw = spy_camera_timeline[0].yaw;
  pose->pitch = spy_camera_timeline[0].pitch;

  return TRUE;
}

static void SpySetFreeCameraPose(SpyCameraPose *pose)
{
  if (pose != NULL) {
    *pose = spy_free_camera_pose;
  }
}

static void SpySetLookAtCameraPose(SpyCameraPose *pose)
{
  if (pose != NULL) {
    *pose = spy_look_at_camera_pose;
  }
}

static float SpyCameraVectorLength(const vec3 *position)
{
  if (position == NULL) {
    return 0.0f;
  }

  return (float)sqrt(
    ((double)position->x * (double)position->x) +
    ((double)position->y * (double)position->y) +
    ((double)position->z * (double)position->z)
  );
}

static void SpyCameraApplyLookAtOrbitOffset(SpyCameraPose *pose, SpyCameraPose *offset)
{
  vec3 target;
  vec3 relative_position;
  float base_distance;
  float distance;
  float horizontal_distance;
  float base_yaw;
  float base_pitch;
  float yaw;
  float pitch;
  float pitch_cosine;

  if (pose == NULL || offset == NULL) {
    return;
  }

  vec3_set(&target, 0.0f, -1.5f, 0.0f);
  relative_position.x = pose->position.x - target.x;
  relative_position.y = pose->position.y - target.y;
  relative_position.z = pose->position.z - target.z;

  base_distance = SpyCameraVectorLength(&relative_position);
  if (base_distance < SPY_CAMERA_MIN_DISTANCE) {
    base_distance = SPY_CAMERA_MIN_DISTANCE;
  }

  distance = base_distance + offset->position.z;
  if (distance < SPY_CAMERA_MIN_DISTANCE) {
    distance = SPY_CAMERA_MIN_DISTANCE;
    offset->position.z = SPY_CAMERA_MIN_DISTANCE - base_distance;
  }

  horizontal_distance = (float)sqrt(
    ((double)relative_position.x * (double)relative_position.x) +
    ((double)relative_position.z * (double)relative_position.z)
  );
  base_yaw = (float)atan2((double)relative_position.x, (double)relative_position.z);
  base_pitch = (float)atan2((double)relative_position.y, (double)horizontal_distance);

  yaw = base_yaw + offset->yaw;
  pitch = base_pitch + offset->pitch;
  pitch_cosine = (float)cos((double)pitch);

  pose->position.x = target.x + (distance * pitch_cosine * (float)sin((double)yaw));
  pose->position.y = target.y + (distance * (float)sin((double)pitch));
  pose->position.z = target.z + (distance * pitch_cosine * (float)cos((double)yaw));
  pose->yaw = yaw;
  pose->pitch = pitch;
}

static void SpyCameraApplyDebugOffset(
  SpyCameraPose *pose,
  SpyCameraPose *offset,
  SpyMaggieCameraMode camera_mode
)
{
  if (camera_mode == SPY_MAGGIE_CAMERA_LOOK_AT) {
    SpyCameraApplyLookAtOrbitOffset(pose, offset);
  } else {
    SpyCameraPoseApplyOffset(pose, offset);
  }
}

static ULONG SpySageTimeToMicros(ULONG sage_time)
{
  return ((sage_time >> STIM_SECONDS_SHIFT) * STIM_TICKS) + (sage_time & STIM_MICRO_MASK);
}

static ULONG SpySecondsToMicros(float seconds)
{
  if (seconds <= 0.0f) {
    return 0;
  }

  return (ULONG)((seconds * (float)STIM_TICKS) + 0.5f);
}

static ULONG SpyMaggieElapsedToTicks(ULONG elapsed_us)
{
  return elapsed_us / (STIM_TICKS / SPY_MAGGIE_TICKS_PER_SECOND);
}

static UWORD SpyClampTunnelY(LONG tunnel_y)
{
  LONG max_y = SPY_SCREEN_HEIGHT - SPY_TUNNEL_LAYER_HEIGHT;

  if (tunnel_y < 0) {
    return 0;
  }
  if (tunnel_y > max_y) {
    return (UWORD)max_y;
  }
  return (UWORD)tunnel_y;
}

static LONG SpyRoundFloatToLong(float value)
{
  if (value >= 0.0f) {
    return (LONG)(value + 0.5f);
  }
  return (LONG)(value - 0.5f);
}

static UWORD SpyCalculateTunnelY(ULONG elapsed_us)
{
  UWORD step_count = (UWORD)(sizeof(spy_tunnel_y_timeline) / sizeof(spy_tunnel_y_timeline[0]));
  UWORD index;

  if (step_count == 0) {
    return SpyClampTunnelY(SPY_TUNNEL_SCREEN_Y);
  }

  for (index = 0; index < (step_count - 1); index++) {
    const SpyTunnelYStep *step = &spy_tunnel_y_timeline[index];
    const SpyTunnelYStep *next_step = &spy_tunnel_y_timeline[index + 1];
    ULONG hold_us = SpySecondsToMicros(step->hold_seconds);
    ULONG move_us = SpySecondsToMicros(step->move_seconds);

    if (elapsed_us <= hold_us) {
      return SpyClampTunnelY(step->y);
    }
    elapsed_us -= hold_us;

    if (move_us == 0) {
      continue;
    }

    if (elapsed_us < move_us) {
      float t = (float)elapsed_us / (float)move_us;
      float eased = SpyAnimApplyEase(step->easing, t);
      float y = SpyAnimLerp((float)step->y, (float)next_step->y, eased);

      return SpyClampTunnelY(SpyRoundFloatToLong(y));
    }
    elapsed_us -= move_us;
  }

  return SpyClampTunnelY(spy_tunnel_y_timeline[step_count - 1].y);
}

static const char *spy_text_pages[] = {
  SPY_TEXT_PAGES
};

static void SpyReportSageFailure(const char *step)
{
  LONG error_code = SAGE_GetErrorCode();
  STRPTR error_text = SAGE_GetErrorString();

  if (error_text == NULL) {
    error_text = (STRPTR)"unknown";
  }

  SpyLog("Failure at %s: SAGE error %ld (%s)\n", step, error_code, error_text);
  if (error_code != SERR_NO_ERROR) {
    SAGE_DisplayError();
  }
}

static BPTR SpyIntroOpenDiz(void)
{
  BPTR file = Open((STRPTR)SPY_INTRO_DIZ_PATH, MODE_OLDFILE);

  if (file == 0) {
    file = Open((STRPTR)SPY_INTRO_DIZ_FALLBACK, MODE_OLDFILE);
  }

  return file;
}

static void SpyIntroPrintDiz(BPTR console)
{
  BPTR file;
  char buffer[256];
  LONG bytes_read;

  if (console == 0) {
    return;
  }

  file = SpyIntroOpenDiz();
  if (file == 0) {
    FPuts(console, (STRPTR)"file_id.diz not found\n");
    return;
  }

  while ((bytes_read = Read(file, buffer, sizeof(buffer))) > 0) {
    Write(console, buffer, bytes_read);
  }

  Close(file);
}

static void SpyRunIntroShell(void)
{
  BPTR console = Open((STRPTR)SPY_INTRO_CONSOLE, MODE_NEWFILE);

  if (console == 0) {
    return;
  }

  SpyIntroPrintDiz(console);
  FPuts(console, (STRPTR)"\n");
  Delay(SPY_INTRO_PAUSE_TICKS);
}

int main(int argc, char* argv[]) {
  SAGE_Timer *fps_timer = NULL;
  SAGE_Timer *demo_timer = NULL;
  ULONG fps_elapsed = 0;
  ULONG demo_elapsed_us = 0;
  ULONG stream_elapsed_us = 0;
  ULONG maggie_ticks_per_frame = 1;
  ULONG maggie_appear_us = SpySecondsToMicros(SPY_MAGGIE_APPEAR_SECONDS);
  ULONG text_start_delay_us = SpySecondsToMicros(SPY_TEXT_START_DELAY_SECONDS);
  ULONG text_letter_delay_us = SpySecondsToMicros(SPY_TEXT_LETTER_DELAY_SECONDS);
  ULONG text_page_hold_us = SpySecondsToMicros(SPY_TEXT_PAGE_HOLD_SECONDS);
  UWORD fps_frames = 0, fps_value = 0;
  UWORD tunnel_y = SpyCalculateTunnelY(0);
  BOOL sage_ready = FALSE;
  BOOL screen_open = FALSE;
  BOOL tunnel_ready = FALSE;
  BOOL logo_ready = FALSE;
  BOOL text_font_ready = FALSE;
  BOOL text_layer_ready = FALSE;
  BOOL demo_mode_layer_ready = FALSE;
  BOOL must_exit = FALSE;
  BOOL debug_overlay_enabled = FALSE;
  BOOL demo_mode_enabled = FALSE;
  BOOL maggie_stream_paused = FALSE;
  BOOL tunnel_visible = TRUE;
  SpyMaggieCameraMode camera_mode = SPY_CAMERA_MODE_DEFAULT;
  SpyFont text_font = { 0 };
  SpyTextLayer text_layer = { 0 };
  SpyTextLayer demo_mode_layer = { 0 };
  SpyTypewriter typewriter = { 0 };
  SpyCameraPose camera_base_pose = { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f };
  SpyCameraPose camera_pose = { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f };
  SpyCameraPose camera_debug_offset = { { 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f };
  int result = 0;

  SpyRunIntroShell();

  SpyLogOpen(SPY_LOG_FILENAME);
  SpyLog("Spy Girl starting\n");

  if (!SpyCameraTimelineEvaluate(
    spy_camera_timeline,
    (UWORD)(sizeof(spy_camera_timeline) / sizeof(spy_camera_timeline[0])),
    0,
    SPY_CAMERA_TIMELINE_LOOP,
    SPY_CAMERA_TIMELINE_LOOP_START_INDEX,
    SPY_CAMERA_TIMELINE_LOOP_STEP_SECONDS,
    &camera_base_pose
  )) {
    SpyLog("Camera timeline is empty or invalid\n");
    result = 1;
    goto shutdown;
  }
  if (camera_mode == SPY_MAGGIE_CAMERA_FREE) {
    SpySetFreeCameraPose(&camera_base_pose);
  } else if (!SPY_CAMERA_CHOREOGRAPHY_ENABLED) {
    SpySetLookAtCameraPose(&camera_base_pose);
  }
  camera_pose = camera_base_pose;
  
  // init the SAGE system
  if (!SAGE_Init(SMOD_VIDEO | SMOD_INTERRUPTION)) {
    SpyReportSageFailure("SAGE_Init");
    result = 1;
    goto shutdown;
  }
  sage_ready = TRUE;

  // open the screen
  if (!SAGE_OpenScreen(SPY_SCREEN_WIDTH, SPY_SCREEN_HEIGHT, SPY_SCREEN_DEPTH, SSCR_STRICTRES)) {
    SpyReportSageFailure("SAGE_OpenScreen");
    result = 1;
    goto shutdown;
  }
  screen_open = TRUE;

  if (!SpyDebugConfigureOverlay()) {
    SpyLog("Visual debug font or style unavailable, using the closest available settings\n");
  }

  SAGE_VerticalSynchro(TRUE);
  SAGE_HideMouse();

  if (!SpyPreloadInit()) {
    SpyReportSageFailure("SpyPreloadInit");
    result = 1;
    goto shutdown;
  }

  if (!SpyPreloadRunBackgroundReveal(5)) {
    SpyReportSageFailure("SpyPreloadRunBackgroundReveal 5");
    result = 1;
    goto shutdown;
  }

  if (!SpyMaggieInit(SPY_STREAM_FILENAME, &camera_pose.position, camera_pose.yaw, camera_pose.pitch, camera_mode)) {
    SpyReportSageFailure("SpyMaggieInit");
    result = 1;
    goto shutdown;
  }
  maggie_ticks_per_frame = SpyMaggieGetTicksPerFrame();
  if (maggie_ticks_per_frame == 0) {
    maggie_ticks_per_frame = 1;
  }
  if (!SpyPreloadDraw(25)) {
    SpyReportSageFailure("SpyPreloadDraw 25");
    result = 1;
    goto shutdown;
  }

  SpyAudioInit();
  if (!SpyPreloadDraw(35)) {
    SpyReportSageFailure("SpyPreloadDraw 35");
    result = 1;
    goto shutdown;
  }

  if (!SpyTextLoadFont(
    &text_font,
    SPY_FONT_FILENAME,
    SPY_FONT_WIDTH,
    SPY_FONT_HEIGHT,
    SPY_FONT_FIRST_CHAR,
    SPY_FONT_GLYPH_COUNT
  )) {
    SpyReportSageFailure("SpyTextLoadFont");
    result = 1;
    goto shutdown;
  }
  text_font_ready = TRUE;
  if (!SpyPreloadDraw(45)) {
    SpyReportSageFailure("SpyPreloadDraw 45");
    result = 1;
    goto shutdown;
  }

  if (!SpyTextSetFontTransparency(&text_font, SPY_TEXT_TRANSPARENT)) {
    SpyReportSageFailure("SpyTextSetFontTransparency");
    result = 1;
    goto shutdown;
  }

  if (!SpyLogoInit()) {
    SpyReportSageFailure("SpyLogoInit");
    result = 1;
    goto shutdown;
  }
  logo_ready = TRUE;

  if (!SpyTextCreateLayer(&text_layer, SPY_TEXT_LAYER, SPY_TEXT_LAYER_WIDTH, SPY_TEXT_BOX_HEIGHT)) {
    SpyReportSageFailure("SpyTextCreateLayer");
    result = 1;
    goto shutdown;
  }
  text_layer_ready = TRUE;

  if (
    !SpyTextSetLayerTransparency(&text_layer, SPY_TEXT_TRANSPARENT) ||
    !SpyTextClearLayer(&text_layer, SPY_TEXT_TRANSPARENT)
  ) {
    SpyReportSageFailure("SpyTextLayerPrepare");
    result = 1;
    goto shutdown;
  }

  if (!SpyTextCreateLayer(&demo_mode_layer, SPY_DEMO_MODE_LAYER, SPY_DEMO_MODE_LAYER_WIDTH, SPY_DEMO_MODE_LAYER_HEIGHT)) {
    SpyReportSageFailure("SpyDemoModeCreateLayer");
    result = 1;
    goto shutdown;
  }
  demo_mode_layer_ready = TRUE;

  if (
    !SpyTextSetLayerTransparency(&demo_mode_layer, SPY_TEXT_TRANSPARENT) ||
    !SpyTextClearLayer(&demo_mode_layer, SPY_TEXT_TRANSPARENT) ||
    !SpyTextDrawString(&demo_mode_layer, &text_font, SPY_DEMO_MODE_TEXT, 0, 0)
  ) {
    SpyReportSageFailure("SpyDemoModeLayerPrepare");
    result = 1;
    goto shutdown;
  }

  if (!SpyPreloadDraw(65)) {
    SpyReportSageFailure("SpyPreloadDraw 65");
    result = 1;
    goto shutdown;
  }

  fps_timer = SAGE_AllocTimer();
  if (fps_timer != NULL) {
    SAGE_ElapsedTime(fps_timer);
  }

  demo_timer = SAGE_AllocTimer();
  if (demo_timer != NULL) {
    SAGE_ElapsedTime(demo_timer);
  } else {
    SpyLog("Demo timer unavailable, choreography will stay at the first frame\n");
  }

  SpyTypewriterInit(
    &typewriter,
    spy_text_pages,
    (UWORD)(sizeof(spy_text_pages) / sizeof(spy_text_pages[0])),
    text_start_delay_us,
    text_letter_delay_us,
    text_page_hold_us,
    0,
    0,
    SPY_TEXT_BOX_WIDTH,
    SPY_TEXT_MAX_LINES,
    SPY_TEXT_LINE_SPACING,
    SPY_TEXT_ALIGN,
    SPY_TEXT_PAGE_LOOP
  );

  if (!SpyTunnelInit(SPY_TUNNEL_FILENAME)) {
    SpyReportSageFailure("SpyTunnelInit");
    result = 1;
    goto shutdown;
  }
  tunnel_ready = TRUE;
  if (!SpyPreloadDraw(90)) {
    SpyReportSageFailure("SpyPreloadDraw 90");
    result = 1;
    goto shutdown;
  }

  if (!SpyPreloadDraw(100)) {
    SpyReportSageFailure("SpyPreloadDraw 100");
    result = 1;
    goto shutdown;
  }
  if (!SAGE_FillScreen(0x000000) || !SAGE_RefreshScreen()) {
    SpyReportSageFailure("Preload black pause");
    result = 1;
    goto shutdown;
  }
  SpyPreloadRelease();
  SAGE_Pause(SPY_PRELOAD_BLACK_PAUSE_TICKS);
  SpyAudioStartMusic();
  SpyTunnelRestartIntroReveal();

  demo_elapsed_us = 0;
  if (demo_timer != NULL) {
    SAGE_ElapsedTime(demo_timer);
  }

  while(!must_exit) {
    ULONG frame_elapsed_us = 0;

    if (demo_timer != NULL) {
      ULONG elapsed = SAGE_ElapsedTime(demo_timer);
      if (elapsed != STIM_OVERFLOW) {
        frame_elapsed_us = SpySageTimeToMicros(elapsed);
        demo_elapsed_us += frame_elapsed_us;
      }
    } else {
      demo_elapsed_us = 0;
      stream_elapsed_us = 0;
    }

    {
      BOOL demo_mode_was_enabled = demo_mode_enabled;

      must_exit = SpyInputUpdate(
        &camera_debug_offset.position,
        &camera_debug_offset.yaw,
        &camera_debug_offset.pitch,
        &debug_overlay_enabled,
        &maggie_ticks_per_frame,
        &maggie_stream_paused,
        &camera_mode,
        &demo_mode_enabled,
        &tunnel_visible
      );

      if (!demo_mode_was_enabled && demo_mode_enabled && text_layer_ready) {
        if (!SpySetDemoModeCameraPose(&camera_base_pose)) {
          SpyLog("Demo mode camera pose unavailable\n");
          result = 1;
          goto shutdown;
        }
        camera_debug_offset.position.x = 0.0f;
        camera_debug_offset.position.y = 0.0f;
        camera_debug_offset.position.z = 0.0f;
        camera_debug_offset.yaw = 0.0f;
        camera_debug_offset.pitch = 0.0f;

        if (!SpyTextClearLayer(&text_layer, SPY_TEXT_TRANSPARENT)) {
          SAGE_DisplayError();
          result = 1;
          goto shutdown;
        }
      }
    }

    if (!maggie_stream_paused) {
      stream_elapsed_us += frame_elapsed_us;
    }

    if (must_exit) {
      break;
    }

    if (!SAGE_ClearScreen()) {
      SpyReportSageFailure("SAGE_ClearScreen");
      result = 1;
      goto shutdown;
    }

    // render the tunnel
    if (tunnel_visible) {
      tunnel_y = SpyCalculateTunnelY(demo_elapsed_us);

      if (!SpyTunnelClearScreenGaps(tunnel_y)) {
        SpyReportSageFailure("SpyTunnelClearScreenGaps");
        result = 1;
        goto shutdown;
      }
      SpyTunnelRenderFrame(tunnel_y);
    }
    
    // render the logo
    if (!demo_mode_enabled && logo_ready && !SpyLogoRender(demo_elapsed_us)) {
      SAGE_DisplayError();
      result = 1;
      goto shutdown;
    }

    // render the 3d spy girl
    if (camera_mode == SPY_MAGGIE_CAMERA_FREE) {
      SpySetFreeCameraPose(&camera_base_pose);
    } else if (!SPY_CAMERA_CHOREOGRAPHY_ENABLED) {
      SpySetLookAtCameraPose(&camera_base_pose);
    } else if (!demo_mode_enabled) {
      if (!SpyCameraTimelineEvaluate(
        spy_camera_timeline,
        (UWORD)(sizeof(spy_camera_timeline) / sizeof(spy_camera_timeline[0])),
        SPY_CAMERA_CHOREOGRAPHY_ENABLED ? demo_elapsed_us : 0,
        SPY_CAMERA_TIMELINE_LOOP,
        SPY_CAMERA_TIMELINE_LOOP_START_INDEX,
        SPY_CAMERA_TIMELINE_LOOP_STEP_SECONDS,
        &camera_base_pose
      )) {
        SpyLog("Camera timeline evaluation failed\n");
        result = 1;
        goto shutdown;
      }
    }
    camera_pose = camera_base_pose;
    SpyCameraApplyDebugOffset(&camera_pose, &camera_debug_offset, camera_mode);
    SpyMaggieSetCamera(&camera_pose.position, camera_pose.yaw, camera_pose.pitch, camera_mode);
    SpyMaggieRender(
      SpyMaggieElapsedToTicks(stream_elapsed_us),
      maggie_ticks_per_frame,
      demo_elapsed_us >= maggie_appear_us,
      maggie_stream_paused
    );

    // render the text
    if (!demo_mode_enabled && text_layer_ready) {
      if (!SpyTypewriterUpdate(
        &typewriter,
        demo_elapsed_us,
        &text_layer,
        &text_font,
        SPY_TEXT_TRANSPARENT
      )) {
        SAGE_DisplayError();
        result = 1;
        goto shutdown;
      }
    }
    if (!demo_mode_enabled && text_layer_ready && !SpyTextBlitLayerToScreen(&text_layer, SPY_TEXT_BOX_X, SPY_TEXT_BOX_Y)) {
      SAGE_DisplayError();
      result = 1;
      goto shutdown;
    }
    if (demo_mode_enabled && demo_mode_layer_ready && !SpyTextBlitLayerToScreen(&demo_mode_layer, SPY_DEMO_MODE_SCREEN_X, SPY_DEMO_MODE_SCREEN_Y)) {
      SAGE_DisplayError();
      result = 1;
      goto shutdown;
    }

    // display the debug infos
    fps_value = SpyUpdateFpsCounter(fps_timer, &fps_frames, &fps_elapsed, fps_value);
    if (debug_overlay_enabled) {
      SpyDrawDebugOverlay(
        fps_value,
        &camera_pose.position,
        camera_pose.yaw,
        camera_pose.pitch,
        SpyMaggieGetCurrentFrame(),
        SpyMaggieGetFrameCount(),
        maggie_ticks_per_frame,
        maggie_stream_paused,
        SpyMaggieCameraModeName(camera_mode)
      );
    }
 
    if (!SAGE_RefreshScreen()) {
      if (SAGE_GetErrorCode() == SERR_NO_ERROR) {
        SpyLog("Warning: SAGE_RefreshScreen returned FALSE with no SAGE error\n");
      } else {
        SpyReportSageFailure("SAGE_RefreshScreen");
        result = 1;
        goto shutdown;
      }
    }
  }

shutdown:
  SpyLog("Shutdown: begin\n");
  SpyPreloadRelease();
  if (demo_mode_layer_ready) {
    SpyLog("Shutdown: release demo mode layer\n");
    SpyTextReleaseLayer(&demo_mode_layer);
    demo_mode_layer_ready = FALSE;
  }
  if (text_layer_ready) {
    SpyLog("Shutdown: release text layer\n");
    SpyTextReleaseLayer(&text_layer);
    text_layer_ready = FALSE;
  }
  if (logo_ready) {
    SpyLog("Shutdown: release logo\n");
    SpyLogoRelease();
    logo_ready = FALSE;
  }
  if (text_font_ready) {
    SpyLog("Shutdown: release text font\n");
    SpyTextReleaseFont(&text_font);
    text_font_ready = FALSE;
  }
  if (screen_open) {
    SpyLog("Shutdown: close screen\n");
    SAGE_ShowMouse();
    SAGE_CloseScreen();
    screen_open = FALSE;
  }
  if (tunnel_ready) {
    SpyLog("Shutdown: release tunnel\n");
    SpyTunnelRelease();
    tunnel_ready = FALSE;
  }
  if (fps_timer != NULL) {
    SpyLog("Shutdown: release fps timer\n");
    SAGE_ReleaseTimer(fps_timer);
    fps_timer = NULL;
  }
  if (demo_timer != NULL) {
    SpyLog("Shutdown: release demo timer\n");
    SAGE_ReleaseTimer(demo_timer);
    demo_timer = NULL;
  }
  SpyLog("Shutdown: release audio\n");
  SpyAudioRelease();
  SpyLog("Shutdown: release Maggie\n");
  SpyMaggieRelease();
  if (sage_ready) {
    SpyLog("Shutdown: SAGE exit\n");
    SAGE_Exit();
    sage_ready = FALSE;
  }
  SpyLog("Spy Girl shutdown, result=%d\n", result);
  SpyLogClose();
  return result;
}
