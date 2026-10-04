#ifndef SPY_GIRL_ANIM_H
#define SPY_GIRL_ANIM_H

typedef enum {
  SPY_ANIM_EASE_LINEAR,
  SPY_ANIM_EASE_QUADRATIC_IN,
  SPY_ANIM_EASE_QUADRATIC_OUT,
  SPY_ANIM_EASE_ELASTIC_IN,
  SPY_ANIM_EASE_ELASTIC_OUT,
  SPY_ANIM_EASE_QUINT_IN,
  SPY_ANIM_EASE_QUINT_OUT
} SpyAnimEaseMode;

float SpyAnimLerp(float start_value, float end_value, float pct);
float SpyAnimFlip(float value);
float SpyAnimApplyEase(SpyAnimEaseMode mode, float t);
float SpyAnimQuadraticEaseIn(float t);
float SpyAnimQuadraticEaseOut(float t);
float SpyAnimEaseInElastic(float t);
float SpyAnimEaseOutElastic(float t);
float SpyAnimEaseInQuint(float t);
float SpyAnimEaseOutQuint(float t);

#ifndef SPY_ANIM_NO_COMPAT_NAMES
#define Lerp SpyAnimLerp
#define Flip SpyAnimFlip
#define QuadraticEaseIn SpyAnimQuadraticEaseIn
#define QuadraticEaseOut SpyAnimQuadraticEaseOut
#define EaseInElastic SpyAnimEaseInElastic
#define EaseOutElastic SpyAnimEaseOutElastic
#define EaseInQuint SpyAnimEaseInQuint
#define EaseOutQuint SpyAnimEaseOutQuint
#endif

#endif
