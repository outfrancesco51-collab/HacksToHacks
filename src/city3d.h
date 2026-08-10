#ifndef CITY3D_H
#define CITY3D_H

#include "raylib.h"

typedef struct {
    Vector3 position;
    Vector3 velocity;
    Color color;
    bool hacked;
    float hackTimer;
} HackableCar;

typedef struct {
    Vector3 position;
    bool isRed;
    bool hacked;
    float timer;
} TrafficLight;

void InitCity3D(void);
void UpdateCity3D(Camera3D *camera, float dt);
void DrawCity3D(void);
void UnloadCity3D(void);

// Returns true if a hack was successful
bool HackTargetInView(Camera3D camera);

#endif
