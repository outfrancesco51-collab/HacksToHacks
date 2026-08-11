#include "minigame_qte.h"
#include <math.h>

static QteType currentQte;
static float qteTimer;
static float qteProgress;
static bool qteFinished;
static int qteStage;

void InitQTE(void) {
    currentQte = (QteType)GetRandomValue(0, 2);
    qteTimer = 5.0f; // 5 seconds to complete
    qteProgress = 0.0f;
    qteFinished = false;
    qteStage = 0;
}

void UpdateDrawQTE(float dt) {
    if (qteFinished) return;
    
    qteTimer -= dt;
    if (qteTimer <= 0) {
        // Failed
        qteTimer = 0;
        return; 
    }
    
    int gamepad = 0;
    bool padActive = IsGamepadAvailable(gamepad);
    
    // Draw Detroit-style UI
    int cx = GetScreenWidth() / 2;
    int cy = GetScreenHeight() / 2;
    
    DrawText("OVERRIDE REQUIRED", cx - MeasureText("OVERRIDE REQUIRED", 20)/2, cy - 100, 20, GREEN);
    
    if (currentQte == QTE_MASH) {
        // Repeated tapping X (Gamepad) or SPACE (Keyboard)
        if ((padActive && IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) || IsKeyPressed(KEY_SPACE)) {
            qteProgress += 0.1f;
        }
        qteProgress -= dt * 0.05f; // decays over time
        if (qteProgress < 0) qteProgress = 0;
        
        DrawText(padActive ? "MASH (X)" : "MASH [SPACE]", cx - 50, cy, 20, (qteProgress > 0.5f) ? YELLOW : WHITE);
        
        if (qteProgress >= 1.0f) qteFinished = true;
        
    } else if (currentQte == QTE_ANALOG_DOWN_RIGHT) {
        // Hold down, then push right
        float axisY = padActive ? GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) : 0.0f;
        float axisX = padActive ? GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) : 0.0f;
        
        if (!padActive) {
            if (IsKeyDown(KEY_S)) axisY = 1.0f;
            if (IsKeyDown(KEY_D)) axisX = 1.0f;
        }
        
        if (qteStage == 0) {
            DrawText(padActive ? "PULL DOWN (L-Stick)" : "HOLD [S]", cx - 80, cy, 20, WHITE);
            if (axisY > 0.8f) {
                qteStage = 1;
                qteProgress = 0.5f;
            }
        } else if (qteStage == 1) {
            DrawText(padActive ? "PUSH RIGHT ->" : "PRESS [D]", cx - 80, cy, 20, YELLOW);
            if (axisX > 0.8f && axisY > 0.5f) {
                qteProgress = 1.0f;
                qteFinished = true;
            } else if (axisY < 0.5f) {
                qteStage = 0; // reset if let go
                qteProgress = 0.0f;
            }
        }
    } else if (currentQte == QTE_ANALOG_UP_LEFT) {
        // Hold up, then push left
        float axisY = padActive ? GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y) : 0.0f;
        float axisX = padActive ? GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X) : 0.0f;
        
        if (!padActive) {
            if (IsKeyDown(KEY_W)) axisY = -1.0f;
            if (IsKeyDown(KEY_A)) axisX = -1.0f;
        }
        
        if (qteStage == 0) {
            DrawText(padActive ? "PUSH UP (L-Stick)" : "HOLD [W]", cx - 80, cy, 20, WHITE);
            if (axisY < -0.8f) {
                qteStage = 1;
                qteProgress = 0.5f;
            }
        } else if (qteStage == 1) {
            DrawText(padActive ? "<- PUSH LEFT" : "PRESS [A]", cx - 80, cy, 20, YELLOW);
            if (axisX < -0.8f && axisY < -0.5f) {
                qteProgress = 1.0f;
                qteFinished = true;
            } else if (axisY > -0.5f) {
                qteStage = 0; // reset if let go
                qteProgress = 0.0f;
            }
        }
    }
    
    // Draw Progress Bar
    DrawRectangleLines(cx - 100, cy + 50, 200, 20, WHITE);
    DrawRectangle(cx - 100, cy + 50, (int)(200 * qteProgress), 20, GREEN);
}

bool IsQTEFinished(void) {
    return qteFinished;
}

float GetQTEProgress(void) {
    return qteProgress;
}

void UnloadQTE(void) {}
