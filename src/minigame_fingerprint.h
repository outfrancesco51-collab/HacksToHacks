#ifndef MINIGAME_FINGERPRINT_H
#define MINIGAME_FINGERPRINT_H

#include "raylib.h"
#include <stdbool.h>

#define FINGERPRINT_SLICES 8

typedef struct {
    int currentRowSlices[FINGERPRINT_SLICES];
    int selectedRow;
    bool solved;
    float timeRemaining;
    Texture2D texFingerprint;
} FingerprintHackState;

void InitFingerprintHack(FingerprintHackState *state, Texture2D texFp);
void UpdateFingerprintHack(FingerprintHackState *state, float dt);
void DrawFingerprintHack(const FingerprintHackState *state, Rectangle bounds);

#endif
