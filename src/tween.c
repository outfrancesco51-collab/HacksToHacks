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
        case TWEEN_CUBIC_BEZIER:
            // Fallback for basic enum, use TweenCubicBezier for full control
            return TweenCubicBezier(start, end, t, 0.25f, 0.1f, 0.25f, 1.0f); // Default CSS ease
        default:
            return start + change * t;
    }
}

// Simple cubic bezier approximation for CSS-like tweens
static float A(float aA1, float aA2) { return 1.0f - 3.0f * aA2 + 3.0f * aA1; }
static float B(float aA1, float aA2) { return 3.0f * aA2 - 6.0f * aA1; }
static float C(float aA1) { return 3.0f * aA1; }

static float CalcBezier(float aT, float aA1, float aA2) {
    return ((A(aA1, aA2) * aT + B(aA1, aA2)) * aT + C(aA1)) * aT;
}

static float GetTForX(float aX, float x1, float x2) {
    float guessT = aX;
    for (int i = 0; i < 4; ++i) {
        float currentSlope = 3.0f * A(x1, x2) * guessT * guessT + 2.0f * B(x1, x2) * guessT + C(x1);
        if (currentSlope == 0.0f) return guessT;
        float currentX = CalcBezier(guessT, x1, x2) - aX;
        guessT -= currentX / currentSlope;
    }
    return guessT;
}

float TweenCubicBezier(float start, float end, float t, float x1, float y1, float x2, float y2) {
    if (t <= 0.0f) return start;
    if (t >= 1.0f) return end;
    float change = end - start;
    float percent = CalcBezier(GetTForX(t, x1, x2), y1, y2);
    return start + change * percent;
}
