#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "tween.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define HACKER_GREEN (Color){ 0, 255, 65, 255 }
#define HACKER_DARK_GREEN (Color){ 0, 143, 17, 255 }
#define DARK_BG (Color){ 5, 5, 5, 255 }
#define MAX_INPUT_CHARS 15
#define MAX_FAKE_CODE 2048

typedef enum GameScreen { NAME_INPUT = 0, TITLE, GAMEPLAY, SETTINGS, HACKING_MINIGAME } GameScreen;
GameScreen currentScreen = NAME_INPUT;
char currentLang[10] = "it";

// Localizzazione
cJSON *localeData = NULL;
const char* GetText(const char* key) {
    if(!localeData) return key;
    cJSON *item = cJSON_GetObjectItemCaseSensitive(localeData, key);
    if(cJSON_IsString(item) && (item->valuestring != NULL)) return item->valuestring;
    return key;
}

// Suoni
Sound sndKarenIntro;
Sound sndKarenError;
Sound sndKarenSuccess;

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
    
    if(IsAudioDeviceReady()) {
        UnloadSound(sndKarenIntro);
        UnloadSound(sndKarenError);
        UnloadSound(sndKarenSuccess);
        sndKarenIntro = LoadSound(TextFormat("assets/audio/voice/%s/tutorial_syskaren_1.wav", currentLang));
        sndKarenError = LoadSound(TextFormat("assets/audio/voice/%s/syskaren_error.wav", currentLang));
        sndKarenSuccess = LoadSound(TextFormat("assets/audio/voice/%s/syskaren_success.wav", currentLang));
    }
}

// Variabili globali
char playerName[MAX_INPUT_CHARS + 1] = "\0";
int letterCount = 0;
Texture2D texUI;
Texture2D texNode;
Font customFont;
NPatchInfo uiPatch;

// Asset Bersagli
Texture2D texBank;
Texture2D texCCTV;
Texture2D texPC;

// UI
Rectangle windowMap = { 50, 50, 800, 500 };
bool draggingMap = false;
Vector2 dragOffset = {0, 0};

// Animazioni
float titleAnimTime = 0.0f;
float settingsScrollY = 0.0f;
float typewriterTime = 0.0f;
int menuSelection = 0; 
int selectedTarget = 0; // 0=Bank, 1=CCTV, 2=PC

// Hacker Typer
char typedCode[MAX_FAKE_CODE] = {0};
int typedLen = 0;
bool hackGranted = false;
float grantedTimer = 0.0f;
bool errorPlayed = false;

const char* fakeSourceBank = 
    "#include <sys/socket.h>\n"
    "int main() {\n"
    "    struct sockaddr_in bank_server;\n"
    "    int sockfd = socket(AF_INET, SOCK_STREAM, 0);\n"
    "    connect(sockfd, (struct sockaddr*)&bank_server, sizeof(bank_server));\n"
    "    send(sockfd, \"OVERRIDE_AUTH_FUNDS\", 19, 0);\n"
    "    // BYPASSING FIREWALL...\n"
    "    inject_dll(0x004050A0);\n"
    "    return 0;\n"
    "}\n";

const char* fakeSourceCCTV = 
    "#!/bin/bash\n"
    "echo 'Intercepting CCTV feed...'\n"
    "airmon-ng start wlan0\n"
    "airodump-ng -c 6 --bssid 00:14:6C:7E:40:80 wlan0mon\n"
    "aireplay-ng -0 5 -a 00:14:6C:7E:40:80 wlan0mon\n"
    "ffmpeg -i rtsp://192.168.1.10:554/feed -vcodec copy out.mp4\n"
    "echo 'Feed hijacked.'\n";

const char* fakeSourcePC = 
    "import os, requests\n"
    "def extract_files(target_ip):\n"
    "    print('Connecting to backdoor...')\n"
    "    r = requests.get(f'http://{target_ip}:8080/exploit')\n"
    "    if r.status_code == 200:\n"
    "        print('Decrypting personal folders...')\n"
    "        os.system('tar -xzf /mnt/C/Users/Victim/Desktop/Secret.tar.gz')\n"
    "extract_files('192.168.1.101')\n";

void DrawTextHacker(const char *text, float x, float y, float fontSize, Color color) {
    DrawTextEx(customFont, text, (Vector2){x, y}, fontSize, 2, color);
}

void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HacksToHacks - Hacker Typer");
    InitAudioDevice();

    texUI = LoadTexture("assets/ui_1.jpg");
    texNode = LoadTexture("assets/node_1.jpg");
    texBank = LoadTexture("assets/face2_1.jpg"); 
    texCCTV = LoadTexture("assets/target_cctv.jpg");
    texPC = LoadTexture("assets/target_pc.jpg");
    customFont = LoadFontEx("assets/font.ttf", 64, 0, 0);
    
    LoadLanguage("it");
    
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
    UnloadSound(sndKarenError);
    UnloadSound(sndKarenSuccess);
    UnloadFont(customFont);
    UnloadTexture(texUI);
    UnloadTexture(texBank);
    UnloadTexture(texCCTV);
    UnloadTexture(texPC);
    UnloadTexture(texNode);
    if(localeData) cJSON_Delete(localeData);
    
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void)
{
    float dt = GetFrameTime();

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
                PlaySound(sndKarenIntro);
            }
            break;
        }
        case TITLE:
            titleAnimTime += dt;
            typewriterTime += dt * 30.0f;
            
            if (IsKeyPressed(KEY_DOWN)) menuSelection = (menuSelection + 1) % 3;
            if (IsKeyPressed(KEY_UP)) menuSelection = (menuSelection - 1 + 3) % 3;
            
            if (IsKeyPressed(KEY_ENTER)) {
                StopSound(sndKarenIntro);
                if (menuSelection == 0) {
                    currentScreen = GAMEPLAY;
                } else if (menuSelection == 1) {
                    currentScreen = SETTINGS;
                } else if (menuSelection == 2) {
#if !defined(PLATFORM_WEB)
                    CloseWindow();
                    exit(0);
#endif
                }
            }
            break;
            
        case GAMEPLAY:
            if (IsKeyPressed(KEY_DOWN)) selectedTarget = (selectedTarget + 1) % 3;
            if (IsKeyPressed(KEY_UP)) selectedTarget = (selectedTarget - 1 + 3) % 3;
            
            if (IsKeyPressed(KEY_H)) {
                currentScreen = HACKING_MINIGAME;
                typedLen = 0;
                memset(typedCode, 0, MAX_FAKE_CODE);
                hackGranted = false;
                errorPlayed = false;
            }
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            
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
            
            if (!hackGranted) {
                int key = GetCharPressed();
                const char* src = (selectedTarget == 0) ? fakeSourceBank : (selectedTarget == 1 ? fakeSourceCCTV : fakeSourcePC);
                
                if (key > 0) {
                    int charsToAdd = 5; 
                    for(int i=0; i<charsToAdd; i++) {
                        if (typedLen < strlen(src) && typedLen < MAX_FAKE_CODE-1) {
                            typedCode[typedLen] = src[typedLen];
                            typedLen++;
                        }
                    }
                    
                    if (typedLen >= strlen(src) - 5) {
                        hackGranted = true;
                        grantedTimer = 0.0f;
                        PlaySound(sndKarenSuccess);
                    }
                }
                
                if (IsKeyPressed(KEY_BACKSPACE) && !errorPlayed) {
                    PlaySound(sndKarenError);
                    errorPlayed = true;
                }
            } else {
                grantedTimer += dt;
                if (grantedTimer > 4.0f) {
                    currentScreen = GAMEPLAY;
                }
            }
            break;
            
        case SETTINGS:
            settingsScrollY -= 100.0f * dt;
            if (settingsScrollY < -500.0f) settingsScrollY = SCREEN_HEIGHT;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO)) {
                if (IsKeyPressed(KEY_ONE)) LoadLanguage("it");
                if (IsKeyPressed(KEY_TWO)) LoadLanguage("en");
            }
            break;
    }

    BeginDrawing();
    ClearBackground(DARK_BG);

    switch(currentScreen)
    {
        case NAME_INPUT:
            DrawTextHacker(GetText("NAME_PROMPT"), 350, 300, 50, HACKER_GREEN);
            DrawTextHacker(playerName, 350, 360, 60, WHITE);
            if (((int)(GetTime() * 2)) % 2 == 0) DrawTextHacker("_", 350 + MeasureTextEx(customFont, playerName, 60, 2).x, 360, 60, HACKER_GREEN);
            break;
            
        case TITLE: {
            float yPos = TweenValue(-100.0f, 150.0f, titleAnimTime, TWEEN_EASE_OUT_BOUNCE);
            DrawTextHacker(GetText("TITLE"), SCREEN_WIDTH/2 - MeasureTextEx(customFont, GetText("TITLE"), 100, 2).x/2, yPos, 100, HACKER_GREEN);
            
            int menuY = 400;
            const char* mPlay = GetText("MENU_PLAY");
            const char* mOpt = GetText("MENU_OPTIONS");
            const char* mExit = GetText("MENU_EXIT");
            
            DrawTextHacker(mPlay, SCREEN_WIDTH/2 - MeasureTextEx(customFont, mPlay, 40, 2).x/2, menuY, 40, menuSelection == 0 ? WHITE : HACKER_DARK_GREEN);
            DrawTextHacker(mOpt, SCREEN_WIDTH/2 - MeasureTextEx(customFont, mOpt, 40, 2).x/2, menuY + 60, 40, menuSelection == 1 ? WHITE : HACKER_DARK_GREEN);
            DrawTextHacker(mExit, SCREEN_WIDTH/2 - MeasureTextEx(customFont, mExit, 40, 2).x/2, menuY + 120, 40, menuSelection == 2 ? WHITE : HACKER_DARK_GREEN);
            
            if(titleAnimTime > 1.0f) {
                Rectangle karenBox = {20, 20, 400, 250};
                DrawRectangle(karenBox.x + 10, karenBox.y + 10, karenBox.width, karenBox.height, (Color){0,0,0,150});
                DrawRectangleRec(karenBox, DARK_BG);
                DrawTextureNPatch(texUI, uiPatch, karenBox, (Vector2){0,0}, 0.0f, WHITE);
                DrawTextHacker("SYS-KAREN", karenBox.x + 20, karenBox.y + 15, 30, RED);
                
                const char* fullText = GetText("TUTORIAL_SYSKAREN_1");
                int displayChars = (int)typewriterTime;
                if(displayChars > strlen(fullText)) displayChars = strlen(fullText);
                
                char displayStr[512] = {0};
                strncpy(displayStr, fullText, displayChars);
                DrawTextEx(customFont, displayStr, (Vector2){karenBox.x + 20, karenBox.y + 60}, 24, 2, WHITE);
            }
            break;
        }
        case GAMEPLAY:
            DrawTextHacker("HACKS TO HACKS - VIRTUAL OS", 10, 10, 20, GRAY);
            DrawTextHacker(TextFormat("USER: %s", playerName), 10, 30, 20, HACKER_GREEN);
            DrawTextHacker("FRECCE SU/GIU: CAMBIA BERSAGLIO | H: HACKERA", 10, 50, 20, WHITE);
            
            DrawRectangle(windowMap.x + 10, windowMap.y + 10, windowMap.width, windowMap.height, (Color){0,0,0,200});
            DrawRectangleRec(windowMap, DARK_BG);
            DrawTextureNPatch(texUI, uiPatch, windowMap, (Vector2){0,0}, 0.0f, WHITE);
            DrawRectangle(windowMap.x+2, windowMap.y+2, windowMap.width-4, 40, HACKER_GREEN);
            DrawTextHacker(GetText("MAP_TITLE"), windowMap.x + 20, windowMap.y + 5, 30, BLACK);
            
            const char* targetName = "";
            Texture2D tTex;
            if (selectedTarget == 0) { targetName = GetText("TARGET_BANK"); tTex = texBank; }
            if (selectedTarget == 1) { targetName = GetText("TARGET_CCTV"); tTex = texCCTV; }
            if (selectedTarget == 2) { targetName = GetText("TARGET_PC"); tTex = texPC; }
            
            DrawTextHacker(TextFormat("TARGET: %s", targetName), windowMap.x + 30, windowMap.y + 80, 40, WHITE);
            DrawTextureEx(tTex, (Vector2){windowMap.x + 300, windowMap.y + 150}, 0.0f, 0.4f, WHITE); // Scala per farle stare nella finestra
            DrawTexture(texNode, windowMap.x + 50, windowMap.y + 150, WHITE);
            
            if (((int)(GetTime() * 4)) % 2 == 0) {
                DrawTextHacker(GetText("NODE_DETECTED"), windowMap.x + 50, windowMap.y + 420, 30, RED);
            }
            break;
            
        case HACKING_MINIGAME:
            DrawTextHacker(">>> TERMINALE DI INJECTION <<<", 20, 20, 40, HACKER_GREEN);
            DrawTextHacker("PREMI TASTI A CASO PER GENERARE IL PAYLOAD...", 20, 60, 20, GRAY);
            
            DrawTextEx(customFont, typedCode, (Vector2){20, 100}, 24, 2, HACKER_GREEN);
            
            if (hackGranted) {
                Texture2D resTex = (selectedTarget == 0) ? texBank : (selectedTarget == 1 ? texCCTV : texPC);
                DrawTextureEx(resTex, (Vector2){SCREEN_WIDTH/2 - (resTex.width*0.5f)/2, 100}, 0.0f, 0.5f, WHITE); // Mostra l'asset estratto
                
                if (((int)(GetTime() * 8)) % 2 == 0) {
                    DrawRectangle(SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2 + 50, 500, 100, HACKER_GREEN);
                    DrawTextHacker(GetText("ACCESS_GRANTED"), SCREEN_WIDTH/2 - 190, SCREEN_HEIGHT/2 + 70, 60, BLACK);
                }
            }
            break;
            
        case SETTINGS:
            DrawTextHacker("IMPOSTAZIONI", 50, 20, 60, HACKER_GREEN);
            DrawTextHacker("1: ITA | 2: ENG", 50, 90, 30, WHITE);
            DrawTextHacker("PREMI ESC PER TORNARE", 50, 130, 30, GRAY);
            break;
    }
    
    for(int i = 0; i < SCREEN_HEIGHT; i+=3) {
        DrawLine(0, i, SCREEN_WIDTH, i, (Color){0, 10, 0, 70});
    }

    EndDrawing();
}
