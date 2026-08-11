#ifndef MINIGAME_QTE_H
#define MINIGAME_QTE_H

#include "raylib.h"

typedef enum {
    QTE_MASH,
    QTE_ANALOG_DOWN_RIGHT,
    QTE_ANALOG_UP_LEFT
} QteType;

void InitQTE(void);
void UpdateDrawQTE(float dt);
bool IsQTEFinished(void);
void UnloadQTE(void);
float GetQTEProgress(void); // For rumble intensity

#endif
