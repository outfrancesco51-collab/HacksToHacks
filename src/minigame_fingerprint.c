#include "minigame_fingerprint.h"
#include <stdlib.h>

void InitFingerprintHack(FingerprintHackState *state, Texture2D texFp) {
    state->selectedRow = 0;
    state->solved = false;
    state->timeRemaining = 60.0f; // 60 seconds to hack
    state->texFingerprint = texFp;
    
    // Scramble rows, ensuring it's not solved immediately
    for (int i = 0; i < FINGERPRINT_SLICES; i++) {
        state->currentRowSlices[i] = GetRandomValue(0, FINGERPRINT_SLICES - 1);
    }
}

void UpdateFingerprintHack(FingerprintHackState *state, float dt) {
    if (state->solved || state->timeRemaining <= 0) return;
    
    state->timeRemaining -= dt;
    if (state->timeRemaining < 0) state->timeRemaining = 0;
    
    if (IsKeyPressed(KEY_UP)) {
        if (state->selectedRow > 0) state->selectedRow--;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        if (state->selectedRow < FINGERPRINT_SLICES - 1) state->selectedRow++;
    }
    if (IsKeyPressed(KEY_LEFT)) {
        state->currentRowSlices[state->selectedRow] = 
            (state->currentRowSlices[state->selectedRow] + FINGERPRINT_SLICES - 1) % FINGERPRINT_SLICES;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        state->currentRowSlices[state->selectedRow] = 
            (state->currentRowSlices[state->selectedRow] + 1) % FINGERPRINT_SLICES;
    }
    
    // Check solution
    bool allMatch = true;
    for (int i = 0; i < FINGERPRINT_SLICES; i++) {
        if (state->currentRowSlices[i] != i) {
            allMatch = false;
            break;
        }
    }
    
    if (allMatch) {
        state->solved = true;
    }
}

void DrawFingerprintHack(const FingerprintHackState *state, Rectangle bounds) {
    // Draw Background
    DrawRectangleRec(bounds, (Color){10, 10, 20, 255});
    DrawRectangleLinesEx(bounds, 2.0f, GREEN);
    
    // Draw Target Fingerprint Preview on the left
    Rectangle previewBounds = { bounds.x + 20, bounds.y + 40, 200, 300 };
    DrawRectangleLinesEx(previewBounds, 1.0f, DARKGREEN);
    DrawTexturePro(state->texFingerprint, 
                   (Rectangle){0, 0, state->texFingerprint.width, state->texFingerprint.height},
                   previewBounds, (Vector2){0,0}, 0.0f, WHITE);
    
    DrawText("TARGET", previewBounds.x, previewBounds.y - 25, 20, GREEN);
    
    // Draw slices on the right
    float sliceHeight = 300.0f / FINGERPRINT_SLICES;
    float texSliceHeight = (float)state->texFingerprint.height / FINGERPRINT_SLICES;
    
    for (int i = 0; i < FINGERPRINT_SLICES; i++) {
        Rectangle dest = { bounds.x + 300, bounds.y + 40 + i * sliceHeight, 200, sliceHeight };
        
        // Draw the current active slice image
        int sliceIdx = state->currentRowSlices[i];
        Rectangle source = { 0, sliceIdx * texSliceHeight, state->texFingerprint.width, texSliceHeight };
        
        DrawTexturePro(state->texFingerprint, source, dest, (Vector2){0,0}, 0.0f, WHITE);
        
        // Draw Selection box
        if (i == state->selectedRow) {
            DrawRectangleLinesEx((Rectangle){dest.x - 5, dest.y - 2, dest.width + 10, dest.height + 4}, 2.0f, GREEN);
            DrawText("<   >", dest.x - 30, dest.y + sliceHeight/2 - 10, 20, GREEN);
        }
    }
    
    DrawText(TextFormat("TIME: %.1f", state->timeRemaining), bounds.x + bounds.width - 150, bounds.y + 20, 20, RED);
    
    if (state->solved) {
        DrawText("ACCESS GRANTED", bounds.x + bounds.width/2 - 100, bounds.y + bounds.height - 50, 30, GREEN);
    } else if (state->timeRemaining <= 0) {
        DrawText("SYSTEM LOCKOUT", bounds.x + bounds.width/2 - 100, bounds.y + bounds.height - 50, 30, RED);
    }
}
