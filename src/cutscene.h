#ifndef CUTSCENE_H
#define CUTSCENE_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Texture2D image;
    const char *text;
    float timePerChar;
} CutsceneFrame;

void InitCutscene(void);
void PlayCutscene(CutsceneFrame *frames, int numFrames);
void UpdateDrawCutscene(float dt);
bool IsCutsceneFinished(void);
void UnloadCutscene(void);

#endif
