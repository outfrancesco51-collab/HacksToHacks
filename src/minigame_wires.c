#include "minigame_wires.h"
#include <stdlib.h>

void InitWiresHack(WiresHackState *state) {
    state->solved = false;
    state->timeRemaining = 45.0f;
    state->startY = 1;
    state->endY = 2;
    
    // Fill grid with random pieces and rotations
    for (int y = 0; y < WIRE_GRID_H; y++) {
        for (int x = 0; x < WIRE_GRID_W; x++) {
            state->grid[y][x].type = (WireType)GetRandomValue(1, 2); // Straight or Corner
            state->grid[y][x].rotation = GetRandomValue(0, 3);
            state->grid[y][x].powered = false;
        }
    }
}

static void PropagatePower(WiresHackState *state) {
    // Reset power
    for (int y = 0; y < WIRE_GRID_H; y++) {
        for (int x = 0; x < WIRE_GRID_W; x++) {
            state->grid[y][x].powered = false;
        }
    }
    
    // Simple mock logic: if the left-to-right path is visually complete
    // In a real game, this would be a recursive flood fill checking connections.
    // For this simple version, we check if they clicked enough times to solve it.
    // To make it easy: let's just make the user rotate all pieces in a specific row to rotation 1 (horizontal).
    
    bool connected = true;
    for (int x = 0; x < WIRE_GRID_W; x++) {
        if (state->grid[state->startY][x].rotation != 1 && state->grid[state->startY][x].rotation != 3) {
            connected = false;
        } else {
            state->grid[state->startY][x].powered = true;
        }
    }
    
    if (connected) {
        state->solved = true;
    }
}

void UpdateWiresHack(WiresHackState *state, float dt) {
    if (state->solved || state->timeRemaining <= 0) return;
    
    state->timeRemaining -= dt;
    if (state->timeRemaining < 0) state->timeRemaining = 0;
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        
        // Check grid clicks
        float startX = 200;
        float startY = 150;
        float cellSize = 80;
        
        for (int y = 0; y < WIRE_GRID_H; y++) {
            for (int x = 0; x < WIRE_GRID_W; x++) {
                Rectangle cell = { startX + x * cellSize, startY + y * cellSize, cellSize, cellSize };
                if (CheckCollisionPointRec(mouse, cell)) {
                    state->grid[y][x].rotation = (state->grid[y][x].rotation + 1) % 4;
                    PropagatePower(state);
                }
            }
        }
    }
}

void DrawWiresHack(const WiresHackState *state, Rectangle bounds) {
    DrawRectangleRec(bounds, (Color){10, 20, 10, 255});
    DrawRectangleLinesEx(bounds, 2.0f, DARKGREEN);
    
    float startX = bounds.x + 200;
    float startY = bounds.y + 150;
    float cellSize = 80;
    
    DrawText("SABOTAGE WIRES - ROUTE POWER TO OUTPUT", bounds.x + 50, bounds.y + 30, 20, GREEN);
    DrawText(TextFormat("TIME: %.1f", state->timeRemaining), bounds.x + bounds.width - 150, bounds.y + 30, 20, RED);
    
    // Draw Inputs/Outputs
    DrawText("PWR IN ->", startX - 100, startY + state->startY * cellSize + 30, 20, GREEN);
    DrawText("-> OUT", startX + WIRE_GRID_W * cellSize + 20, startY + state->startY * cellSize + 30, 20, RED);
    
    for (int y = 0; y < WIRE_GRID_H; y++) {
        for (int x = 0; x < WIRE_GRID_W; x++) {
            Rectangle cell = { startX + x * cellSize, startY + y * cellSize, cellSize, cellSize };
            DrawRectangleLinesEx(cell, 1.0f, (Color){0, 100, 0, 100});
            
            Color wireCol = state->grid[y][x].powered ? GREEN : DARKGRAY;
            Vector2 center = { cell.x + cellSize/2, cell.y + cellSize/2 };
            
            // Draw a line based on rotation
            if (state->grid[y][x].rotation == 0 || state->grid[y][x].rotation == 2) {
                // Vertical
                DrawLineEx((Vector2){center.x, cell.y}, (Vector2){center.x, cell.y + cellSize}, 8.0f, wireCol);
            } else {
                // Horizontal
                DrawLineEx((Vector2){cell.x, center.y}, (Vector2){cell.x + cellSize, center.y}, 8.0f, wireCol);
            }
        }
    }
    
    if (state->solved) {
        DrawText("SABOTAGE SUCCESSFUL", bounds.x + bounds.width/2 - 150, bounds.y + bounds.height - 50, 30, GREEN);
    } else if (state->timeRemaining <= 0) {
        DrawText("SYSTEM LOCKOUT", bounds.x + bounds.width/2 - 100, bounds.y + bounds.height - 50, 30, RED);
    }
}
