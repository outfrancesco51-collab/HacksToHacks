#include "city3d.h"
#include <stdlib.h>
#include <math.h>

#define NUM_BUILDINGS 1500
#define NUM_CARS 100
#define NUM_NODES 50

typedef struct {
    Vector3 position;
    Vector3 size;
    Color color;
    int district; // 0: Napoli Centro, 1: Varcaturo, 2: Soccavo, 3: Bagnoli, 4: Coroglio
    bool isBank;
} Building;

typedef struct {
    Vector3 position;
    int fraudActive;
    float fraudTimer;
    bool isMurderScene;
} CrimeNode;

static Building buildings[NUM_BUILDINGS];
static HackableCar cars[NUM_CARS];
static CrimeNode nodes[NUM_NODES];

// Basic 2D pseudo-random hash for procedural math
float hash2d(float x, float y) {
    float n = sinf(x * 12.9898f + y * 78.233f) * 43758.5453f;
    return n - floorf(n);
}

void InitCity3D(void) {
    // Generate Procedural City with districts
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        float px = (float)GetRandomValue(-400, 400);
        float pz = (float)GetRandomValue(-400, 400);
        
        // Procedural height based on distance to center (Napoli Centro) and math noise
        float distToCenter = sqrtf(px*px + pz*pz);
        float noise = hash2d(px*0.1f, pz*0.1f);
        
        float height = 10.0f + (noise * 50.0f);
        int district = 0;
        
        if(distToCenter < 100.0f) { district = 0; height += 40.0f; } // Centro: alti grattacieli
        else if(px > 100.0f && pz < -100.0f) { district = 1; height *= 0.5f; } // Varcaturo
        else if(px < -100.0f && pz < -100.0f) { district = 2; height *= 0.7f; } // Soccavo
        else if(px < -100.0f && pz > 100.0f) { district = 3; height *= 0.6f; } // Bagnoli
        else { district = 4; height = 15.0f + (noise * 10.0f); } // Coroglio (costiero)

        buildings[i].position = (Vector3){ px, height / 2.0f, pz };
        buildings[i].size = (Vector3){ (float)GetRandomValue(10, 25), height, (float)GetRandomValue(10, 25) };
        buildings[i].district = district;
        buildings[i].isBank = (GetRandomValue(0, 100) > 95); // 5% chance it's a bank
        
        // Cyberpunk colors based on district
        Color c;
        if(district == 0) c = (Color){ 20, 20, 40, 255 };
        else if(district == 1) c = (Color){ 10, 30, 20, 255 };
        else if(district == 2) c = (Color){ 30, 15, 15, 255 };
        else if(district == 3) c = (Color){ 15, 15, 35, 255 };
        else c = (Color){ 10, 20, 30, 255 };
        
        if(buildings[i].isBank) c = (Color){ 50, 40, 10, 255 }; // Gold-ish for banks
        buildings[i].color = c;
    }
    
    // Spawn crime nodes
    for (int i = 0; i < NUM_NODES; i++) {
        nodes[i].position = buildings[GetRandomValue(0, NUM_BUILDINGS-1)].position;
        nodes[i].position.y += 50.0f;
        nodes[i].fraudActive = GetRandomValue(0, 1);
        nodes[i].isMurderScene = (GetRandomValue(0, 100) > 80);
        nodes[i].fraudTimer = (float)GetRandomValue(10, 30);
    }
    
    // Spline-based cars
    for(int i = 0; i < NUM_CARS; i++) {
        cars[i].position = (Vector3){ (float)GetRandomValue(-300, 300), 1.0f, (float)GetRandomValue(-300, 300) };
        cars[i].velocity = (Vector3){ (float)(GetRandomValue(0, 1) ? 20 : -20), 0, (float)(GetRandomValue(0, 1) ? 20 : -20) };
        cars[i].color = (Color){ 255, 0, 50, 255 };
        cars[i].hacked = false;
        cars[i].hackTimer = 0.0f;
    }
}

void UpdateCity3D(Camera3D *camera, float dt) {
    for(int i = 0; i < NUM_CARS; i++) {
        if(!cars[i].hacked) {
            // Pseudo-spline movement (wavy)
            float wave = sinf(GetTime() * 2.0f + i) * 5.0f;
            cars[i].position.x += cars[i].velocity.x * dt;
            cars[i].position.z += (cars[i].velocity.z + wave) * dt;
            
            if(cars[i].position.x > 400) cars[i].position.x = -400;
            if(cars[i].position.x < -400) cars[i].position.x = 400;
            if(cars[i].position.z > 400) cars[i].position.z = -400;
            if(cars[i].position.z < -400) cars[i].position.z = 400;
        } else {
            cars[i].hackTimer -= dt;
            cars[i].position.y = 1.0f + sinf(GetTime()*10.0f)*2.0f; // Glitch flying effect
            if(cars[i].hackTimer <= 0.0f) {
                cars[i].hacked = false;
                cars[i].position.y = 1.0f;
                cars[i].color = RED;
            }
        }
    }
    
    // Update crime nodes
    for(int i = 0; i < NUM_NODES; i++) {
        if(nodes[i].fraudActive) {
            nodes[i].fraudTimer -= dt;
            if(nodes[i].fraudTimer <= 0) nodes[i].fraudActive = 0;
        }
    }
}

void DrawCity3D(void) {
    DrawGrid(800, 10.0f);
    
    for (int i = 0; i < NUM_BUILDINGS; i++) {
        DrawCubeV(buildings[i].position, buildings[i].size, buildings[i].color);
        Color wire = (buildings[i].isBank) ? GOLD : (Color){ 0, 255, 255, 40 };
        DrawCubeWiresV(buildings[i].position, buildings[i].size, wire);
    }
    
    for(int i = 0; i < NUM_CARS; i++) {
        DrawCube(cars[i].position, 4.0f, 2.0f, 2.0f, cars[i].color);
        if(cars[i].hacked) {
            DrawCubeWires(cars[i].position, 5.0f, 3.0f, 3.0f, GREEN); // Glitch/hacked effect
        }
    }
    
    for(int i = 0; i < NUM_NODES; i++) {
        if(nodes[i].fraudActive) {
            Color c = nodes[i].isMurderScene ? RED : ORANGE;
            DrawSphereWires(nodes[i].position, 3.0f + sinf(GetTime()*5.0f), 8, 8, c);
        }
    }
}

bool HackTargetInView(Camera3D camera) {
    Ray ray = {0};
    ray.position = camera.position;
    Vector3 dir = { camera.target.x - camera.position.x, camera.target.y - camera.position.y, camera.target.z - camera.position.z };
    float length = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    ray.direction = (Vector3){ dir.x/length, dir.y/length, dir.z/length };
    
    for(int i = 0; i < NUM_CARS; i++) {
        BoundingBox box = {
            (Vector3){ cars[i].position.x - 4, cars[i].position.y - 2, cars[i].position.z - 2 },
            (Vector3){ cars[i].position.x + 4, cars[i].position.y + 2, cars[i].position.z + 2 }
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
}

