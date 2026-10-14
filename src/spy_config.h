#ifndef SPY_GIRL_CONFIG_H
#define SPY_GIRL_CONFIG_H

/* Screen */
#define SPY_SCREEN_WIDTH              320
#define SPY_SCREEN_HEIGHT             200
#define SPY_SCREEN_DEPTH              16
#define SPY_PRELOAD_BACKGROUND_FILENAME "assets/gfx/loading_background.bmp"
#define SPY_PRELOAD_BACKGROUND_REVEAL_BLOCK_SIZE 16
#define SPY_PRELOAD_BACKGROUND_REVEAL_SECONDS 2.0f
#define SPY_PRELOAD_BLACK_PAUSE_TICKS 100 /* 2 seconds at 50Hz */

#if SPY_SCREEN_DEPTH != 16
#error Maggie stream rendering expects a 16-bit screen
#endif

/* Visual debug */
#define SPY_DEBUG_FONT_NAME           "topaz.font"
#define SPY_DEBUG_FONT_SIZE           8
#define SPY_DEBUG_SCREEN_X            0
#define SPY_DEBUG_SCREEN_Y            0
#define SPY_DEBUG_LINE_HEIGHT         8
#define SPY_DEBUG_FRONT_PEN           1
#define SPY_DEBUG_BACK_PEN            0
#define SPY_DEBUG_FRONT_COLOR         0x00ffffff
#define SPY_DEBUG_BACK_COLOR          0x00000000

/* Tunnel layout */
#define SPY_TUNNEL_LAYER              0
#define SPY_TUNNEL_LAYER_WIDTH        320
#define SPY_TUNNEL_LAYER_HEIGHT       80
#define SPY_TUNNEL_SCREEN_X           ((SPY_SCREEN_WIDTH - SPY_TUNNEL_LAYER_WIDTH) / 2)
#define SPY_TUNNEL_SCREEN_Y           ((SPY_SCREEN_HEIGHT - SPY_TUNNEL_LAYER_HEIGHT) / 2)
#define SPY_TUNNEL_CORE_RADIUS        40

/* Tunnel effect: Y movement */
#define SPY_TUNNEL_EFFECT_START_Y       SPY_TUNNEL_SCREEN_Y //(SPY_SCREEN_HEIGHT - SPY_TUNNEL_LAYER_HEIGHT)
#define SPY_TUNNEL_EFFECT_SECOND_Y      SPY_TUNNEL_SCREEN_Y
#define SPY_TUNNEL_EFFECT_THIRD_Y       SPY_TUNNEL_SCREEN_Y + 20
#define SPY_TUNNEL_EFFECT_INITIAL_HOLD_SECONDS 2.0f
#define SPY_TUNNEL_EFFECT_FIRST_MOVE_SECONDS   2.0f
#define SPY_TUNNEL_EFFECT_SECOND_HOLD_SECONDS  5.0f
#define SPY_TUNNEL_EFFECT_SECOND_MOVE_SECONDS  2.0f
/* { y, hold_seconds, move_seconds_to_next_y, easing } */
#define SPY_TUNNEL_EFFECT_Y_TIMELINE \
  { SPY_TUNNEL_EFFECT_START_Y, 0.0f, 0.0f, SPY_ANIM_EASE_LINEAR }, \
  // { SPY_TUNNEL_EFFECT_SECOND_Y, SPY_TUNNEL_EFFECT_SECOND_HOLD_SECONDS, SPY_TUNNEL_EFFECT_SECOND_MOVE_SECONDS, SPY_ANIM_EASE_LINEAR }, \
  // { SPY_TUNNEL_EFFECT_THIRD_Y, 0.0f, 0.0f, SPY_ANIM_EASE_LINEAR }

/* Tunnel texture and motion */
#define SPY_TUNNEL_RENDER_PIXEL_SCALE 2 /* 1 = 320x80, 2 = 160x40 rendered as 2x2 blocks */
#define SPY_TUNNEL_PATTERN_WIDTH      512
#define SPY_TUNNEL_PATTERN_HEIGHT     512
#define SPY_TUNNEL_MOTION_FPS         40

/* Tunnel effect: initial reveal */
#define SPY_TUNNEL_EFFECT_INTRO_HOLD_SECONDS 0.0f
#define SPY_TUNNEL_EFFECT_REVEAL_SECONDS 2.0f
/*
0 or 1: render the reveal source every frame
2: render every second frame and reuse it once
3: render every third frame and reuse it twice
*/
#define SPY_TUNNEL_EFFECT_REVEAL_SOURCE_RENDER_INTERVAL 2
#define SPY_TUNNEL_EFFECT_REVEAL_ORIGIN SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_LEFT

/* Tunnel pattern effect: transitions between patterns */
/* CUT, COLUMN_REVEAL, FLASH, NEGATIVE_BURST, SHUTTER, DIAMOND_WAVE, RGB_FLASH */
#define SPY_TUNNEL_PATTERN_EFFECT_MODE SPY_TUNNEL_PATTERN_EFFECT_CUT
#define SPY_TUNNEL_PATTERN_EFFECT_DURATION_SECONDS 5.0f
#define SPY_TUNNEL_PATTERN_EFFECT_FLASH_DURATION_SECONDS 0.5f
#define SPY_TUNNEL_PATTERN_EFFECT_NEGATIVE_DURATION_SECONDS 0.4f
#define SPY_TUNNEL_PATTERN_EFFECT_SHUTTER_DURATION_SECONDS 1.2f
#define SPY_TUNNEL_PATTERN_EFFECT_SHUTTER_COLOUR 0xffff
#define SPY_TUNNEL_PATTERN_EFFECT_DIAMOND_DURATION_SECONDS 3.0f
#define SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH_DURATION_SECONDS 0.5f
#define SPY_TUNNEL_PATTERN_EFFECT_RGB_FLASH_COLOUR 0xf81f
#define SPY_TUNNEL_PATTERN_EFFECT_INTERVAL_SECONDS 30.0f
#define SPY_TUNNEL_PATTERN_EFFECT_SOURCE_RENDER_INTERVAL 2
#define SPY_TUNNEL_PATTERN_EFFECT_ORIGIN SPY_TUNNEL_REVEAL_ORIGIN_BOTTOM_RIGHT

/* Shared column reveal geometry */
#define SPY_TUNNEL_REVEAL_COLUMN_WIDTH 16
#define SPY_TUNNEL_REVEAL_STEP_HEIGHT 10
#define SPY_TUNNEL_REVEAL_ROW_DELAY_STEPS 1

/* Tunnel Assets */
#define SPY_TUNNEL_FILENAME           "assets/gfx/pattern_0.png"
#define SPY_TUNNEL_PATTERN_COUNT      6
#define SPY_TUNNEL_PATTERN_FILENAMES \
  "assets/gfx/pattern_0.png", \
  "assets/gfx/pattern_1.png", \
  "assets/gfx/pattern_2.png", \
  "assets/gfx/pattern_3.png", \
  "assets/gfx/pattern_4.png", \
  "assets/gfx/pattern_5.png"

/* Logo Assets and choreography */
#define SPY_LOGO_FILENAME             "assets/gfx/logo.bmp"
#define SPY_LOGO_SPRITE_BANK          0
#define SPY_LOGO_WIDTH                192
#define SPY_LOGO_HEIGHT               88
#define SPY_LOGO_PIECES               12
#define SPY_LOGO_PIECE_WIDTH          16
#define SPY_LOGO_START_X              8
#define SPY_LOGO_START_Y              -88
#define SPY_LOGO_FINAL_X              8
#define SPY_LOGO_FINAL_Y              0
#define SPY_LOGO_START_SECONDS        40.0f
#define SPY_LOGO_PIECE_DELAY_SECONDS  .3f
#define SPY_LOGO_BOUNCE_SECONDS       2.25f
#define SPY_LOGO_TRANSPARENT          0x00787878
#define SPY_STREAM_FILENAME           "assets/3d/spy_girl_dance.mpt"
#define SPY_FONT_FILENAME             "assets/fonts/font_16x18.bmp"
#define SPY_LOG_FILENAME              "spy_girl_log.txt"

/* Music */
#define SPY_MUSIC_FILENAME      "assets/music/back_in_town.mod"
#define SPY_MUSIC_SLOT          0

/* Font */
#define SPY_FONT_WIDTH          16
#define SPY_FONT_HEIGHT         18
#define SPY_FONT_FIRST_CHAR     ' '
#define SPY_FONT_GLYPH_COUNT    60

/* Text box */
#define SPY_TEXT_BOX_X          8
#define SPY_TEXT_BOX_Y          88
#define SPY_TEXT_BOX_WIDTH      (SPY_SCREEN_WIDTH - 16)
#define SPY_TEXT_MAX_LINES      4
#define SPY_TEXT_LINE_SPACING   4
#define SPY_TEXT_LAYER_WIDTH    SPY_TEXT_BOX_WIDTH
#define SPY_TEXT_BOX_HEIGHT     ((SPY_TEXT_MAX_LINES > 0) ? ((SPY_TEXT_MAX_LINES * SPY_FONT_HEIGHT) + ((SPY_TEXT_MAX_LINES - 1) * SPY_TEXT_LINE_SPACING)) : 0)
#define SPY_TEXT_LAYER          1
#define SPY_TEXT_TRANSPARENT    0x00404040
#define SPY_TEXT_ALIGN          SPY_TEXT_ALIGN_RIGHT
// 18 characters maximum per line, 4 lines maximum per page
#define SPY_TEXT_PAGES\
  "HOWDY FOLKS!\nWELCOME TO THIS\nARTISTIC INTRO FOR\nDEADLINE MMXXVI", \
  "OBJECT STREAMING\nON MAGGIE CHIP\nWITH COOL VISUALS\nAT 25 FPS", \
  "WE HOPE IT BRINGS\nA LITTLE JOY\nAND A BIT OF AWE\nTO YOUR VAMPIRE", \
  "CREDITS:\nCODE/GFX/3D: LYNX\nMUSIC: OK3ANOS", \
  "OK3ANOS GREETS:\nTEK,PLUSH\nRABENAUGE,NUANCE\nDLG CREW", \
  "NAH/KOLOR\nHEMOROIDS,TRSI\nALCATRAZ\nRESISTANCE", \
  "LYNX GREETS:\nNIGHTFALL,DESIRE\nSPREADPOINT\nNAH/KOLOR", \
  "HEMOROIDS\nSANDER,DANNYHEY\nPROWLER,ELKMOOSE\nPELLICUS,PHAZE101", \
  "VIRGILL\nMORTEN,LEXO\nAMIGA BILL\nAMIGA CAMMY", \
  "PERIFRACTIC,CEBIT\nBULDOZER1200\nROBY T.\nAND EVERYONE ELSE.", \
  " \nSPY GIRL\nDEADLINE, MMXXVI\n \n", \
  " \n \n \n \n"
#define SPY_TEXT_START_DELAY_SECONDS    46.0f
#define SPY_TEXT_LETTER_DELAY_SECONDS   0.1f
#define SPY_TEXT_PAGE_HOLD_SECONDS      6.0f
#define SPY_TEXT_PAGE_LOOP              1

/* Demo mode */
#define SPY_DEMO_MODE_LAYER             2
#define SPY_DEMO_MODE_TEXT              "DEMO MODE"
#define SPY_DEMO_MODE_LAYER_WIDTH       (9 * SPY_FONT_WIDTH)
#define SPY_DEMO_MODE_LAYER_HEIGHT      SPY_FONT_HEIGHT
#define SPY_DEMO_MODE_SCREEN_X          0
#define SPY_DEMO_MODE_SCREEN_Y          (SPY_SCREEN_HEIGHT - SPY_DEMO_MODE_LAYER_HEIGHT)

/* Camera choreography */
#define SPY_CAMERA_CHOREOGRAPHY_ENABLED 1 /* 0 = manual camera from first pose, 1 = follow timeline */
#define SPY_CAMERA_MODE_DEFAULT SPY_MAGGIE_CAMERA_LOOK_AT /* C toggles FREE / LOOK_AT at runtime */
/* Manual LOOK_AT reset pose used when choreography is disabled and when switching back from FREE. */
#define SPY_LOOK_AT_CAMERA_POSITION { 0.0f, -1.7f, 4.5f }
#define SPY_LOOK_AT_CAMERA_YAW SPY_CAMERA_DEGREES(180.0f)
#define SPY_LOOK_AT_CAMERA_PITCH SPY_CAMERA_DEGREES(2.85f)
/* Manual FREE reset pose used when switching from LOOK_AT into FREE. */
#define SPY_FREE_CAMERA_POSITION { 0.0f, -1.7f, 4.5f }
#define SPY_FREE_CAMERA_YAW SPY_CAMERA_DEGREES(180.0f)
#define SPY_FREE_CAMERA_PITCH SPY_CAMERA_DEGREES(2.85f)
#define SPY_CAMERA_TIMELINE_LOOP 1 /* 0 = stop at the last pose, 1 = loop */
#define SPY_CAMERA_TIMELINE_LOOP_START_INDEX 0 /* First pass starts at 0; repeated loop starts here. */
#define SPY_CAMERA_TIMELINE_LOOP_STEP_SECONDS 5.0f
/* { { x, y, z }, yaw, pitch, hold, transition duration, transition, easing } */
#define SPY_CAMERA_TIMELINE \
  { { 0.6f, -2.95f, 2.55f }, SPY_CAMERA_DEGREES(-171.41f), SPY_CAMERA_DEGREES(14.31f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { -1.5f, -3.35f, 1.95f }, SPY_CAMERA_DEGREES(139.89f), SPY_CAMERA_DEGREES(34.36f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { 2.25f, -2.6f, 3.3f }, SPY_CAMERA_DEGREES(-157.0f), SPY_CAMERA_DEGREES(17.17f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { 1.65f, -3.35f, 2.4f }, SPY_CAMERA_DEGREES(-148.49f), SPY_CAMERA_DEGREES(31.5f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { 0.45f, -3.95f, 2.25f }, SPY_CAMERA_DEGREES(-162.8f), SPY_CAMERA_DEGREES(40.1f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { -1.8f, -3.05f, 2.4f }, SPY_CAMERA_DEGREES(155.18f), SPY_CAMERA_DEGREES(23.0f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { -1.95f, -1.55f, 3.15f }, SPY_CAMERA_DEGREES(157.1f), SPY_CAMERA_DEGREES(0.0f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { 0.45f, -2.75f, 2.85f }, SPY_CAMERA_DEGREES(-160.0f), SPY_CAMERA_DEGREES(25.77f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { -1.95f, -3.05f, 2.85f }, SPY_CAMERA_DEGREES(154.22f), SPY_CAMERA_DEGREES(28.63f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }, \
  { { 0.75f, -3.05f, 2.55f }, SPY_CAMERA_DEGREES(-160.0f), SPY_CAMERA_DEGREES(31.5f), 5.0f, 0.0f, SPY_CAMERA_TRANSITION_CUT, SPY_ANIM_EASE_LINEAR }
  
/* Maggie */
#define SPY_MAGGIE_TEXTURE_SIZE 9 /* Maggie texture exponent: 10 = 1024x1024. */
#define SPY_MAGGIE_TICKS_PER_SECOND 50UL /* MPT timing uses PAL-rate VBlank ticks. */
#define SPY_MAGGIE_APPEAR_SECONDS 7.0f
#define SPY_LIGHT_AMBIENT_COLOUR 0x00ffffff

/* Camera input */
#define SPY_CAMERA_DEBUG_INPUT_ENABLED 1 /* 0 = ignore A/D/Q/E/W/S, arrows, and C camera mode toggle; V overlay stays active. */
#define SPY_CAMERA_STEP 0.15f
#define SPY_CAMERA_ROTATION_STEP 0.05f
#define SPY_CAMERA_MIN_DISTANCE 0.5f

#endif
