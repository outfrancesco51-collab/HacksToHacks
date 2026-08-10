#include "city3d.h"
#include <stdlib.h>
#include <math.h>

#define NUM_BUILDINGS 100
#define NUM_CARS 20

typedef struct {
    Vector3 position;
    Vector3 size;
    Color color;
} Building;

static Building buildings[NUM_BUILDINGS];
static HackableCar cars[NUM_CARS];

void InitCity3D(void) {
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        buildings[i].position = (Vector3){ (float)(GetRandomValue(-100, 100)), 0, (float)(GetRandomValue(-100, 100)) };
        buildings[i].size = (Vector3){ (float)GetRandomValue(5, 15), (float)GetRandomValue(10, 60), (float)GetRandomValue(5, 15) };
        buildings[i].position.y = buildings[i].size.y / 2.0f;
        
        // Cyberpunk colors
        int col = GetRandomValue(0, 3);
        if(col == 0) buildings[i].color = (Color){ 20, 20, 25, 255 }; // Dark slate
        else if(col == 1) buildings[i].color = (Color){ 10, 30, 40, 255 }; // Dark teal
        else buildings[i].color = (Color){ 5, 5, 5, 255 }; // Black
    }
    
    for(int i = 0; i < NUM_CARS; i++) {
        cars[i].position = (Vector3){ (float)GetRandomValue(-80, 80), 1.0f, (float)GetRandomValue(-80, 80) };
        cars[i].velocity = (Vector3){ (float)(GetRandomValue(0, 1) ? 10 : -10), 0, 0 };
        cars[i].color = (Color){ 255, 0, 0, 255 };
        cars[i].hacked = false;
        cars[i].hackTimer = 0.0f;
    }
}

void UpdateCity3D(Camera3D *camera, float dt) {
    for(int i = 0; i < NUM_CARS; i++) {
        if(!cars[i].hacked) {
            cars[i].position.x += cars[i].velocity.x * dt;
            cars[i].position.z += cars[i].velocity.z * dt;
            
            if(cars[i].position.x > 100) cars[i].position.x = -100;
            if(cars[i].position.x < -100) cars[i].position.x = 100;
        } else {
            cars[i].hackTimer -= dt;
            if(cars[i].hackTimer <= 0.0f) {
                cars[i].hacked = false;
                cars[i].color = RED;
            }
        }
    }
}

void DrawCity3D(void) {
    DrawGrid(200, 2.0f);
    
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        DrawCubeV(buildings[i].position, buildings[i].size, buildings[i].color);
        DrawCubeWiresV(buildings[i].position, buildings[i].size, (Color){ 0, 255, 255, 100 }); // Neon cyan wireframe
    }
    
    for(int i = 0; i < NUM_CARS; i++) {
        DrawCube(cars[i].position, 4.0f, 2.0f, 2.0f, cars[i].color);
        if(cars[i].hacked) {
            DrawCubeWires(cars[i].position, 4.5f, 2.5f, 2.5f, GREEN); // Glitch/hacked effect
        }
    }
}

bool HackTargetInView(Camera3D camera) {
    // Simple hack raycast for cars
    Ray ray = {0};
    ray.position = camera.position;
    // Calculate direction from camera target
    Vector3 dir = { camera.target.x - camera.position.x, camera.target.y - camera.position.y, camera.target.z - camera.position.z };
    float length = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    ray.direction = (Vector3){ dir.x/length, dir.y/length, dir.z/length };
    
    for(int i = 0; i < NUM_CARS; i++) {
        BoundingBox box = {
            (Vector3){ cars[i].position.x - 2, cars[i].position.y - 1, cars[i].position.z - 1 },
            (Vector3){ cars[i].position.x + 2, cars[i].position.y + 1, cars[i].position.z + 1 }
        };
        
        RayCollision col = GetRayCollisionBox(ray, box);
        if(col.hit) {
            cars[i].hacked = true;
            cars[i].hackTimer = 5.0f;
            cars[i].color = GREEN;
            return true;
        }
    }
    
    return false;
}

void UnloadCity3D(void) {
    // Free resources if any dynamic memory is used
}
