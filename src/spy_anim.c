#include <math.h>

#include "spy_anim.h"

#define SPY_ANIM_PI 3.14159265358979323846f

static float SpyAnimClamp01(float value)
{
  if (value <= 0.0f) {
    return 0.0f;
  }
  if (value >= 1.0f) {
    return 1.0f;
  }
  return value;
}

float SpyAnimLerp(float start_value, float end_value, float pct)
{
  return start_value + (end_value - start_value) * pct;
}

float SpyAnimFlip(float value)
{
  return 1.0f - value;
}

float SpyAnimApplyEase(SpyAnimEaseMode mode, float t)
{
  t = SpyAnimClamp01(t);

  switch (mode) {
    case SPY_ANIM_EASE_QUADRATIC_IN:
      return SpyAnimQuadraticEaseIn(t);
    case SPY_ANIM_EASE_QUADRATIC_OUT:
      return SpyAnimQuadraticEaseOut(t);
    case SPY_ANIM_EASE_ELASTIC_IN:
      return SpyAnimEaseInElastic(t);
    case SPY_ANIM_EASE_ELASTIC_OUT:
      return SpyAnimEaseOutElastic(t);
    case SPY_ANIM_EASE_QUINT_IN:
      return SpyAnimEaseInQuint(t);
    case SPY_ANIM_EASE_QUINT_OUT:
      return SpyAnimEaseOutQuint(t);
    case SPY_ANIM_EASE_LINEAR:
    default:
      return t;
  }
}

float SpyAnimQuadraticEaseIn(float t)
{
  return t * t;
}

float SpyAnimQuadraticEaseOut(float t)
{
  return -(t * (t - 2.0f));
}

float SpyAnimEaseInElastic(float t)
{
  float c4 = (2.0f * SPY_ANIM_PI) / 3.0f;

  if (t == 0.0f) {
    return 0.0f;
  }
  if (t == 1.0f) {
    return 1.0f;
  }

  return (float)(-pow(2.0, 10.0 * t - 10.0) * sin((t * 10.0 - 10.5) * c4));
}

float SpyAnimEaseOutElastic(float t)
{
  float c4 = (2.0f * SPY_ANIM_PI) / 3.0f;

  if (t == 0.0f) {
    return 0.0f;
  }
  if (t == 1.0f) {
    return 1.0f;
  }

  return (float)(pow(2.0, -10.0 * t) * sin((t * 10.0 - 0.5) * c4) + 1.0);
}

float SpyAnimEaseInQuint(float t)
{
  return t * t * t * t * t;
}

float SpyAnimEaseOutQuint(float t)
{
  return (float)(1.0 - pow(1.0 - t, 5.0));
}
