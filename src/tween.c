#include "tween.h"
#include <math.h>

float TweenValue(float start, float end, float t, TweenType type) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float change = end - start;

    switch(type) {
        case TWEEN_LINEAR:
            return start + change * t;
        case TWEEN_EASE_IN_QUAD:
            return start + change * t * t;
        case TWEEN_EASE_OUT_QUAD:
            return start + change * t * (2 - t);
        case TWEEN_EASE_IN_OUT_QUAD:
            if (t < 0.5f) return start + change * 2 * t * t;
            return start + change * (-1 + (4 - 2 * t) * t);
        case TWEEN_EASE_OUT_BOUNCE:
            if (t < (1 / 2.75f)) {
                return start + change * (7.5625f * t * t);
            } else if (t < (2 / 2.75f)) {
                t -= (1.5f / 2.75f);
                return start + change * (7.5625f * t * t + 0.75f);
            } else if (t < (2.5f / 2.75f)) {
                t -= (2.25f / 2.75f);
                return start + change * (7.5625f * t * t + 0.9375f);
            } else {
                t -= (2.625f / 2.75f);
                return start + change * (7.5625f * t * t + 0.984375f);
            }
        default:
            return start + change * t;
    }
}
