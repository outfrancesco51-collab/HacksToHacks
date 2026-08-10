#ifndef TWEEN_H
#define TWEEN_H

typedef enum {
    TWEEN_LINEAR,
    TWEEN_EASE_IN_QUAD,
    TWEEN_EASE_OUT_QUAD,
    TWEEN_EASE_IN_OUT_QUAD,
    TWEEN_EASE_OUT_BOUNCE,
    TWEEN_CUBIC_BEZIER
} TweenType;

float TweenValue(float start, float end, float t, TweenType type);
float TweenCubicBezier(float start, float end, float t, float x1, float y1, float x2, float y2);

#endif
