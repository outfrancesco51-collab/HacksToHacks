#include "cutscene.h"
#include <string.h>

static CutsceneFrame *currentFrames = NULL;
static int totalFrames = 0;
static int currentFrameIdx = 0;
static float textTimer = 0.0f;
static bool isFinished = true;

extern Font customFont;

void InitCutscene(void) {
    isFinished = true;
}

void PlayCutscene(CutsceneFrame *frames, int numFrames) {
    currentFrames = frames;
    totalFrames = numFrames;
    currentFrameIdx = 0;
    textTimer = 0.0f;
    isFinished = false;
}

void UpdateDrawCutscene(float dt) {
    if (isFinished || !currentFrames) return;
    
    CutsceneFrame *frame = &currentFrames[currentFrameIdx];
    textTimer += dt;
    
    int charsToShow = (int)(textTimer / frame->timePerChar);
    int len = strlen(frame->text);
    
    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (charsToShow < len) {
            textTimer = len * frame->timePerChar; // Skip text typing
        } else {
            currentFrameIdx++;
            textTimer = 0.0f;
            if (currentFrameIdx >= totalFrames) {
                isFinished = true;
            }
        }
    }
    
    if (isFinished) return;
    
    // Draw
    DrawTexturePro(frame->image, 
        (Rectangle){0, 0, frame->image.width, frame->image.height}, 
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 
        (Vector2){0, 0}, 0.0f, WHITE);
        
    // Text Box
    DrawRectangle(50, GetScreenHeight() - 200, GetScreenWidth() - 100, 150, (Color){0, 0, 0, 200});
    DrawRectangleLines(50, GetScreenHeight() - 200, GetScreenWidth() - 100, 150, GREEN);
    
    char displayStr[1024] = {0};
    if (charsToShow > len) charsToShow = len;
    strncpy(displayStr, frame->text, charsToShow);
    
    DrawTextEx(customFont, displayStr, (Vector2){70, GetScreenHeight() - 180}, 30, 2, GREEN);
}

bool IsCutsceneFinished(void) {
    return isFinished;
}

void UnloadCutscene(void) {
    currentFrames = NULL;
}
