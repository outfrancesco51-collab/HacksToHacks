#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "tween.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Costanti
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define HACKER_GREEN (Color){ 0, 255, 0, 255 }
#define DARK_BG (Color){ 10, 10, 10, 255 }
#define MAX_INPUT_CHARS 15

// Stato globale
typedef enum GameScreen { NAME_INPUT = 0, TITLE, GAMEPLAY, SETTINGS, HACKING_MINIGAME } GameScreen;
GameScreen currentScreen = NAME_INPUT;

// Lingua e Testi
cJSON *localeData = NULL;
const char* GetText(const char* key) {
    if(!localeData) return key;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(localeData, key);
    if(cJSON_IsString(item) && (item->valuestring != NULL)) {
        return item->valuestring;
    }
    return key;
}

void LoadLanguage(const char* langCode) {
    char path[256];
    snprintf(path, sizeof(path), "locales/%s.json", langCode);
    
    char *data = LoadFileText(path);
    if(data) {
        if(localeData) cJSON_Delete(localeData);
        localeData = cJSON_Parse(data);
        UnloadFileText(data);
    }
}

// Variabili
char playerName[MAX_INPUT_CHARS + 1] = "\0";
int letterCount = 0;
Texture2D texUI;
Texture2D texFace;
Texture2D texNode;

// Finestre UI (Desktop Simulator)
Rectangle windowMap = { 50, 50, 800, 500 };
bool draggingMap = false;
Vector2 dragOffset = {0, 0};

// Animazioni
float titleAnimTime = 0.0f;
float settingsScrollY = 0.0f;

void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HacksToHacks - Nano Banana Pro Edition");

    LoadLanguage("it"); // Default ITA

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
    if(localeData) cJSON_Delete(localeData);
    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void)
{
    float dt = GetFrameTime();

    // Aggiornamento
    switch(currentScreen) 
    {
        case NAME_INPUT: {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS)) {
                    playerName[letterCount] = (char)key;
                    playerName[letterCount+1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0) {
                letterCount--;
                playerName[letterCount] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
                currentScreen = TITLE;
                titleAnimTime = 0.0f;
            }
            break;
        }
        case TITLE:
            titleAnimTime += dt;
            if (IsKeyPressed(KEY_ENTER)) currentScreen = GAMEPLAY;
            if (IsKeyPressed(KEY_S)) currentScreen = SETTINGS;
            break;
        case GAMEPLAY:
            if (IsKeyPressed(KEY_H)) currentScreen = HACKING_MINIGAME;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            
            // Finestra Draggable
            Vector2 mouse = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, (Rectangle){windowMap.x, windowMap.y, windowMap.width, 30})) {
                draggingMap = true;
                dragOffset.x = mouse.x - windowMap.x;
                dragOffset.y = mouse.y - windowMap.y;
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) draggingMap = false;
            
            if (draggingMap) {
                windowMap.x = mouse.x - dragOffset.x;
                windowMap.y = mouse.y - dragOffset.y;
            }
            break;
        case HACKING_MINIGAME:
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            break;
        case SETTINGS:
            settingsScrollY -= 50.0f * dt;
            if (settingsScrollY < -500.0f) settingsScrollY = SCREEN_HEIGHT;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            // Lingue
            if (IsKeyPressed(KEY_ONE)) LoadLanguage("it");
            if (IsKeyPressed(KEY_TWO)) LoadLanguage("en");
            if (IsKeyPressed(KEY_THREE)) LoadLanguage("ja");
            break;
    }

    // Disegno
    BeginDrawing();
    ClearBackground(DARK_BG);

    switch(currentScreen)
    {
        case NAME_INPUT:
            DrawText(GetText("NAME_PROMPT"), 400, 300, 30, HACKER_GREEN);
            DrawText(playerName, 400, 350, 40, WHITE);
            if (((int)(GetTime() * 2)) % 2 == 0) DrawText("_", 400 + MeasureText(playerName, 40), 350, 40, HACKER_GREEN);
            break;
            
        case TITLE: {
            float yPos = TweenValue(-100.0f, 100.0f, titleAnimTime, TWEEN_EASE_OUT_BOUNCE);
            DrawText(GetText("TITLE"), 20, (int)yPos, 50, HACKER_GREEN);
            DrawText(TextFormat("BENTORNATO, %s", playerName), 20, (int)yPos + 80, 20, WHITE);
            DrawText(GetText("PRESS_ENTER"), 20, 300, 20, HACKER_GREEN);
            DrawText("> PREMI 'S' PER IMPOSTAZIONI / INFO", 20, 340, 20, HACKER_GREEN);
            
            // SYS-KAREN Tutorial popup
            if(titleAnimTime > 2.0f) {
                DrawRectangleLines(800, 100, 400, 150, RED);
                DrawText("SYS-KAREN", 810, 110, 20, RED);
                DrawText(GetText("TUTORIAL_SYSKAREN_1"), 810, 140, 15, WHITE);
            }
            break;
        }
        case GAMEPLAY:
            // Sfondo Desktop Fake
            DrawText("HACKS TO HACKS - VIRTUAL OS", 10, 10, 20, GRAY);
            
            // Disegna Finestra Mappa
            DrawRectangle(windowMap.x, windowMap.y, windowMap.width, windowMap.height, BLACK);
            DrawRectangleLines(windowMap.x, windowMap.y, windowMap.width, windowMap.height, HACKER_GREEN);
            // Barra del titolo
            DrawRectangle(windowMap.x, windowMap.y, windowMap.width, 30, HACKER_GREEN);
            DrawText(GetText("MAP_TITLE"), windowMap.x + 10, windowMap.y + 5, 20, BLACK);
            
            // Contenuto Mappa
            DrawText(GetText("PRESS_H_BANK"), windowMap.x + 20, windowMap.y + 50, 20, HACKER_GREEN);
            DrawText("COLLEGAMENTO GOOGLE MAPS API (SIMULATO): NISIDA", windowMap.x + 20, windowMap.y + 90, 20, WHITE);
            DrawTexture(texNode, windowMap.x + 100, windowMap.y + 150, WHITE);
            DrawText(GetText("NODE_DETECTED"), windowMap.x + 100, windowMap.y + 400, 20, RED);
            break;
            
        case HACKING_MINIGAME:
            DrawText(GetText("HACKING_IN_PROGRESS"), 20, 20, 30, RED);
            DrawText(GetText("INSERT_PAYLOAD"), 20, 60, 20, HACKER_GREEN);
            DrawTexture(texFace, 200, 200, WHITE);
            DrawText(GetText("OBJECTIVE_FUNDS"), 200, 500, 20, WHITE);
            break;
            
        case SETTINGS:
            DrawText("INFO E CREDITI", 20, 20, 40, HACKER_GREEN);
            DrawText("1: ITALIANO | 2: ENGLISH | 3: JAPANESE", 20, 80, 20, WHITE);
            DrawText("PREMI ESC PER TORNARE INDIETRO", 20, 120, 20, GRAY);
            
            // Scrolling text
            int sy = (int)settingsScrollY;
            DrawText("ENGINE: RAYLIB 5.0", 400, sy, 30, HACKER_GREEN);
            DrawText("AUTORE: FRANCESCO & IA", 400, sy + 50, 30, HACKER_GREEN);
            DrawText("ASSET: NANO BANANA PRO", 400, sy + 100, 30, HACKER_GREEN);
            break;
    }
    
    // Effetto scanline globale
    for(int i = 0; i < SCREEN_HEIGHT; i+=4) {
        DrawLine(0, i, SCREEN_WIDTH, i, (Color){0, 50, 0, 50});
    }

    EndDrawing();
}
