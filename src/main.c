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
#define HACKER_GREEN (Color){ 0, 255, 65, 255 }
#define HACKER_DARK_GREEN (Color){ 0, 143, 17, 255 }
#define DARK_BG (Color){ 5, 5, 5, 255 }
#define MAX_INPUT_CHARS 15

// Stato globale
typedef enum GameScreen { NAME_INPUT = 0, TITLE, GAMEPLAY, SETTINGS, HACKING_MINIGAME } GameScreen;
GameScreen currentScreen = NAME_INPUT;
char currentLang[10] = "it";

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
    strncpy(currentLang, langCode, sizeof(currentLang)-1);
    
    char *data = LoadFileText(path);
    if(data) {
        if(localeData) cJSON_Delete(localeData);
        localeData = cJSON_Parse(data);
        UnloadFileText(data);
    }
}

// Variabili globali
char playerName[MAX_INPUT_CHARS + 1] = "\0";
int letterCount = 0;
Texture2D texUI;
Texture2D texFace;
Texture2D texNode;
Font customFont;
Sound sndKarenIntro;

// Finestre UI (Desktop Simulator)
Rectangle windowMap = { 50, 50, 800, 500 };
bool draggingMap = false;
Vector2 dragOffset = {0, 0};
NPatchInfo uiPatch;

// Animazioni e Effetti
float titleAnimTime = 0.0f;
float settingsScrollY = 0.0f;
float typewriterTime = 0.0f;

// Helper per testo personalizzato
void DrawTextHacker(const char *text, float x, float y, float fontSize, Color color) {
    DrawTextEx(customFont, text, (Vector2){x, y}, fontSize, 2, color);
}

void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HacksToHacks - Hacker OS Edition");
    InitAudioDevice();

    LoadLanguage("it"); // Default ITA

    // Carica assets
    texUI = LoadTexture("assets/ui_1.jpg");
    texFace = LoadTexture("assets/face1_1.jpg");
    texNode = LoadTexture("assets/node_1.jpg");
    customFont = LoadFontEx("assets/font.ttf", 64, 0, 0);
    
    // Suono intro SYS-KAREN (usiamo l'IT di default all'avvio)
    sndKarenIntro = LoadSound("assets/audio/voice/it/intro_1.wav");

    // Imposta NPatch per i bordi delle finestre (adatta questi valori alla tua texture)
    uiPatch.source = (Rectangle){0, 0, texUI.width, texUI.height};
    uiPatch.left = texUI.width / 4;
    uiPatch.top = texUI.height / 4;
    uiPatch.right = texUI.width / 4;
    uiPatch.bottom = texUI.height / 4;
    uiPatch.layout = NPATCH_NINE_PATCH;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadSound(sndKarenIntro);
    UnloadFont(customFont);
    UnloadTexture(texUI);
    UnloadTexture(texFace);
    UnloadTexture(texNode);
    if(localeData) cJSON_Delete(localeData);
    
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void)
{
    float dt = GetFrameTime();

    // Aggiornamento Logica
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
                typewriterTime = 0.0f;
                // Ricarica il suono giusto se han cambiato lingua prima?
                // Qui è la prima volta, suoniamo l'intro:
                PlaySound(sndKarenIntro);
            }
            break;
        }
        case TITLE:
            titleAnimTime += dt;
            typewriterTime += dt * 30.0f; // 30 caratteri al secondo
            if (IsKeyPressed(KEY_ENTER)) {
                currentScreen = GAMEPLAY;
                StopSound(sndKarenIntro);
            }
            if (IsKeyPressed(KEY_S)) {
                currentScreen = SETTINGS;
                StopSound(sndKarenIntro);
            }
            break;
        case GAMEPLAY:
            if (IsKeyPressed(KEY_H)) currentScreen = HACKING_MINIGAME;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            
            // Finestra Draggable
            Vector2 mouse = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, (Rectangle){windowMap.x, windowMap.y, windowMap.width, 40})) {
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
            settingsScrollY -= 100.0f * dt;
            if (settingsScrollY < -500.0f) settingsScrollY = SCREEN_HEIGHT;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            // Cambio lingua dinamico e ricarica suoni
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_THREE)) {
                if (IsKeyPressed(KEY_ONE)) LoadLanguage("it");
                if (IsKeyPressed(KEY_TWO)) LoadLanguage("en");
                // if (IsKeyPressed(KEY_THREE)) LoadLanguage("ja");
                
                // Aggiorna l'audio
                UnloadSound(sndKarenIntro);
                sndKarenIntro = LoadSound(TextFormat("assets/audio/voice/%s/intro_1.wav", currentLang));
            }
            break;
    }

    // Disegno Grafica
    BeginDrawing();
    ClearBackground(DARK_BG);

    switch(currentScreen)
    {
        case NAME_INPUT:
            DrawTextHacker(GetText("NAME_PROMPT"), 400, 300, 40, HACKER_GREEN);
            DrawTextHacker(playerName, 400, 350, 60, WHITE);
            if (((int)(GetTime() * 2)) % 2 == 0) DrawTextHacker("_", 400 + MeasureTextEx(customFont, playerName, 60, 2).x, 350, 60, HACKER_GREEN);
            break;
            
        case TITLE: {
            float yPos = TweenValue(-100.0f, 100.0f, titleAnimTime, TWEEN_EASE_OUT_BOUNCE);
            DrawTextHacker(GetText("TITLE"), 50, yPos, 80, HACKER_GREEN);
            DrawTextHacker(TextFormat("BENTORNATO, %s", playerName), 50, yPos + 80, 30, WHITE);
            DrawTextHacker(GetText("PRESS_ENTER"), 50, 300, 30, HACKER_GREEN);
            DrawTextHacker("> PREMI 'S' PER IMPOSTAZIONI / INFO", 50, 340, 30, HACKER_DARK_GREEN);
            
            // SYS-KAREN Tutorial popup con effetto Typewriter
            if(titleAnimTime > 1.0f) {
                Rectangle karenBox = {700, 150, 500, 200};
                // Drop shadow
                DrawRectangle(karenBox.x + 10, karenBox.y + 10, karenBox.width, karenBox.height, (Color){0,0,0,150});
                // Sfondo solido e bordo NPatch
                DrawRectangleRec(karenBox, DARK_BG);
                DrawTextureNPatch(texUI, uiPatch, karenBox, (Vector2){0,0}, 0.0f, WHITE);
                
                DrawTextHacker("SYS-KAREN", karenBox.x + 20, karenBox.y + 10, 30, RED);
                
                const char* fullText = GetText("TUTORIAL_SYSKAREN_1");
                int totalChars = strlen(fullText);
                int displayChars = (int)typewriterTime;
                if(displayChars > totalChars) displayChars = totalChars;
                
                char displayStr[512] = {0};
                strncpy(displayStr, fullText, displayChars);
                
                // Draw text with word wrap (basic)
                DrawTextEx(customFont, displayStr, (Vector2){karenBox.x + 20, karenBox.y + 50}, 24, 2, WHITE);
            }
            break;
        }
        case GAMEPLAY:
            DrawTextHacker("HACKS TO HACKS - VIRTUAL OS", 10, 10, 20, GRAY);
            
            // Disegna Finestra Mappa (Shadow + NPatch)
            DrawRectangle(windowMap.x + 10, windowMap.y + 10, windowMap.width, windowMap.height, (Color){0,0,0,200});
            DrawRectangleRec(windowMap, DARK_BG);
            DrawTextureNPatch(texUI, uiPatch, windowMap, (Vector2){0,0}, 0.0f, WHITE);
            
            // Barra del titolo
            DrawRectangle(windowMap.x+2, windowMap.y+2, windowMap.width-4, 40, HACKER_GREEN);
            DrawTextHacker(GetText("MAP_TITLE"), windowMap.x + 20, windowMap.y + 5, 30, BLACK);
            
            // Fake Close Button
            DrawRectangle(windowMap.x + windowMap.width - 40, windowMap.y + 5, 30, 30, RED);
            DrawTextHacker("X", windowMap.x + windowMap.width - 32, windowMap.y + 5, 30, WHITE);
            
            // Contenuto Mappa
            DrawTextHacker(GetText("PRESS_H_BANK"), windowMap.x + 30, windowMap.y + 60, 30, HACKER_GREEN);
            DrawTextHacker("TARGET: NISIDA SATELLITE (SIMULATO)", windowMap.x + 30, windowMap.y + 100, 24, WHITE);
            
            DrawTexture(texNode, windowMap.x + 100, windowMap.y + 150, WHITE);
            if (((int)(GetTime() * 4)) % 2 == 0) {
                DrawTextHacker(GetText("NODE_DETECTED"), windowMap.x + 100, windowMap.y + 400, 30, RED);
            }
            break;
            
        case HACKING_MINIGAME:
            DrawTextHacker(GetText("HACKING_IN_PROGRESS"), 20, 20, 50, RED);
            DrawTextHacker(GetText("INSERT_PAYLOAD"), 20, 80, 30, HACKER_GREEN);
            DrawTexture(texFace, 200, 200, WHITE);
            DrawTextHacker(GetText("OBJECTIVE_FUNDS"), 200, 500, 40, WHITE);
            break;
            
        case SETTINGS:
            DrawTextHacker("INFO E CREDITI", 50, 20, 60, HACKER_GREEN);
            DrawTextHacker("1: ITA | 2: ENG", 50, 90, 30, WHITE);
            DrawTextHacker("PREMI ESC PER TORNARE INDIETRO", 50, 130, 30, GRAY);
            
            // Scrolling text
            int sy = (int)settingsScrollY;
            DrawTextHacker("ENGINE: RAYLIB 5.0", 400, sy, 50, HACKER_GREEN);
            DrawTextHacker("AUTORE: FRANCESCO & IA", 400, sy + 60, 50, HACKER_GREEN);
            DrawTextHacker("ASSET: NANO BANANA PRO", 400, sy + 120, 50, HACKER_GREEN);
            break;
    }
    
    // Effetto scanline globale (più fitto per retro-feel)
    for(int i = 0; i < SCREEN_HEIGHT; i+=3) {
        DrawLine(0, i, SCREEN_WIDTH, i, (Color){0, 10, 0, 70});
    }

    EndDrawing();
}
