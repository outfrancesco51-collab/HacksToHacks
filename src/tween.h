#ifndef TWEEN_H
#define TWEEN_H

typedef enum {
    TWEEN_LINEAR,
    TWEEN_EASE_IN_QUAD,
    TWEEN_EASE_OUT_QUAD,
    TWEEN_EASE_IN_OUT_QUAD,
    TWEEN_EASE_OUT_BOUNCE
} TweenType;

float TweenValue(float start, float end, float time_pct, TweenType type);

#endif
