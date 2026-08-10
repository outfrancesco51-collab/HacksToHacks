#ifndef MINIGAME_WIRES_H
#define MINIGAME_WIRES_H

#include "raylib.h"
#include <stdbool.h>

#define WIRE_GRID_W 5
#define WIRE_GRID_H 4

typedef enum {
    WIRE_NONE = 0,
    WIRE_STRAIGHT, // |
    WIRE_CORNER,   // L
    WIRE_CROSS     // +
} WireType;

typedef struct {
    WireType type;
    int rotation; // 0, 1, 2, 3 (x 90 deg)
    bool powered;
} WireNode;

typedef struct {
    WireNode grid[WIRE_GRID_H][WIRE_GRID_W];
    bool solved;
    float timeRemaining;
    int startY;
    int endY;
} WiresHackState;

void InitWiresHack(WiresHackState *state);
void UpdateWiresHack(WiresHackState *state, float dt);
void DrawWiresHack(const WiresHackState *state, Rectangle bounds);

#endif
