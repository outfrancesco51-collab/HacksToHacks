#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Costanti di gioco
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define HACKER_GREEN (Color){ 0, 255, 0, 255 }
#define DARK_BG (Color){ 10, 10, 10, 255 }

// Stato globale del gioco
typedef enum GameScreen { LOGO = 0, TITLE, GAMEPLAY, HACKING_MINIGAME } GameScreen;
GameScreen currentScreen = TITLE;

// Variabili
int framesCounter = 0;
Texture2D texUI;
Texture2D texFace;
Texture2D texNode;

void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HacksToHacks - Nano Banana Pro Edition");

    // Load assets
    texUI = LoadTexture("assets/ui_1.jpg");
    texFace = LoadTexture("assets/face1_1.jpg");
    texNode = LoadTexture("assets/node_1.jpg");

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadTexture(texUI);
    UnloadTexture(texFace);
    UnloadTexture(texNode);
    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void)
{
    framesCounter++;

    switch(currentScreen) 
    {
        case TITLE:
            if (IsKeyPressed(KEY_ENTER)) currentScreen = GAMEPLAY;
            break;
        case GAMEPLAY:
            if (IsKeyPressed(KEY_H)) currentScreen = HACKING_MINIGAME;
            break;
        case HACKING_MINIGAME:
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            break;
        default: break;
    }

    BeginDrawing();
    ClearBackground(DARK_BG);

    switch(currentScreen)
    {
        case TITLE:
            DrawText("HACKS TO HACKS", 20, 20, 40, HACKER_GREEN);
            DrawText("> PREMI INVIO PER ACCEDERE AL SISTEMA _", 20, 100, 20, HACKER_GREEN);
            DrawTexture(texUI, 20, 200, WHITE);
            break;
        case GAMEPLAY:
            DrawText("MAPPA GLOBALE", 20, 20, 30, HACKER_GREEN);
            DrawText("> PREMI 'H' PER INIZIARE HACKING BANCARIO", 20, 60, 20, HACKER_GREEN);
            DrawRectangleLines(20, 100, 800, 500, HACKER_GREEN);
            DrawTexture(texNode, 300, 300, WHITE);
            DrawText("NODO VULNERABILE RILEVATO", 300, 270, 20, HACKER_GREEN);
            break;
        case HACKING_MINIGAME:
            DrawText("HACKING BANCARIO IN CORSO...", 20, 20, 30, RED);
            DrawText("> INSERIRE PAYLOAD (Premi ESC per annullare)", 20, 60, 20, HACKER_GREEN);
            DrawTexture(texFace, 200, 200, WHITE);
            DrawText("OBIETTIVO: TRASFERIMENTO FONDI", 200, 500, 20, WHITE);
            break;
        default: break;
    }
    
    // Effetto scanline per atmosfera hacker
    for(int i = 0; i < SCREEN_HEIGHT; i+=4) {
        DrawLine(0, i, SCREEN_WIDTH, i, (Color){0, 50, 0, 50});
    }

    EndDrawing();
}
