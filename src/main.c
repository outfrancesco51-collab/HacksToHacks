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

#include "city3d.h"

#include "cutscene.h"
#include "minigame_fingerprint.h"
#include "minigame_wires.h"

typedef enum GameScreen { EPILEPSY_WARNING = 0, NAME_INPUT, TITLE, CUTSCENE, GAMEPLAY, SETTINGS, HACKING_MINIGAME, FINGERPRINT_MINIGAME, WIRES_MINIGAME } GameScreen;
GameScreen currentScreen = EPILEPSY_WARNING;
char currentLang[10] = "it";
char pcUsername[128] = "UNKNOWN";

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
Sound sndHackerUI[200];
FingerprintHackState fpState;
WiresHackState wiresState;

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
    
    if (sndKarenIntro.stream.buffer != NULL) UnloadSound(sndKarenIntro);
    if (sndKarenError.stream.buffer != NULL) UnloadSound(sndKarenError);
    if (sndKarenSuccess.stream.buffer != NULL) UnloadSound(sndKarenSuccess);
    
    if (strcmp(langCode, "it") == 0 || strcmp(langCode, "en") == 0) {
        sndKarenIntro = LoadSound(TextFormat("assets/audio/voice/%s/tutorial_syskaren_1.mp3", currentLang));
        sndKarenError = LoadSound(TextFormat("assets/audio/voice/%s/syskaren_error.mp3", currentLang));
        sndKarenSuccess = LoadSound(TextFormat("assets/audio/voice/%s/syskaren_success.mp3", currentLang));
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
Texture2D texEpilepsy;
Texture2D texCutscene1;
Texture2D texCutscene2;
Texture2D texSpain;
Texture2D texArgentina;
Texture2D texFingerprint;

// UI
Rectangle windowMap = { 50, 50, 800, 500 };
bool draggingMap = false;
Vector2 dragOffset = {0, 0};

// Animazioni
float titleAnimTime = 0.0f;
float settingsScrollY = 0.0f;
float typewriterTime = 0.0f;
float warningTime = 0.0f;
int menuSelection = 0; 
int selectedTarget = 0;
const char* targets[] = { "CENTRAL BANK NAPLES", "CCTV GRID MILAN", "UNKNOWN PC", "ELEVATOR CONTROL (MILAN)", "BROADCAST TOWER (MADRID)", "SMART TV NETWORK (BUENOS AIRES)" };
int numTargets = 6;
Shader glitchShader;
Shader bloomShader;
#define NUM_ADVANCED_SHADERS 104
Shader advancedShaders[NUM_ADVANCED_SHADERS];
int currentAdvancedShader = 0;
float advancedShaderTimer = 0.0f;

RenderTexture2D target;
int timeLoc = -1;
float gameTime = 0.0f;

// Camera
Camera3D camera = { 0 };

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

void InitGame(void)
{
#if !defined(PLATFORM_WEB) && !defined(PLATFORM_ANDROID) && !defined(PLATFORM_IOS)
    // Generate 500MB payload for advanced complexity
    FILE* fPayload = fopen("HacksToHacks_Payload.dat", "rb");
    if (!fPayload) {
        printf("Generating 500MB advanced hacking payload...\n");
        fPayload = fopen("HacksToHacks_Payload.dat", "wb");
        if (fPayload) {
            char buffer[1024 * 1024]; // 1MB buffer
            memset(buffer, 0x01, sizeof(buffer));
            for (int i = 0; i < 500; i++) {
                fwrite(buffer, 1, sizeof(buffer), fPayload);
            }
            fclose(fPayload);
            printf("Payload generation complete.\n");
        }
    } else {
        fclose(fPayload);
    }

#if defined(PLATFORM_DESKTOP)
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HacksToHacks - Hacker Typer");
    SetWindowPosition(0, 0);
    // Simple fullscreen simulation, actual Monitor dims can be used in advanced setup
#else
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HacksToHacks - Hacker Typer");
#endif
#endif
    InitAudioDevice();

    sndKarenError = LoadSound("assets/karen_error.mp3");
    sndKarenSuccess = LoadSound("assets/karen_success.mp3");
    
    for(int i=0; i<200; i++) {
        sndHackerUI[i] = LoadSound(TextFormat("assets/sounds/sfx_%d.wav", i));
    }

    const char* userEnv = getenv("USERNAME");
    if (!userEnv) userEnv = getenv("USER");
    if (userEnv) {
        strncpy(pcUsername, userEnv, 127);
    }

    texUI = LoadTexture("assets/ui_retro.jpg");
    texNode = LoadTexture("assets/item_hack.jpg");
    texBank = LoadTexture("assets/face_hacker.jpg"); 
    texCCTV = LoadTexture("assets/target_cctv.png");
    texPC = LoadTexture("assets/target_pc.png");
    texEpilepsy = LoadTexture("assets/epilepsy.jpg");
    texCutscene1 = LoadTexture("assets/spy1.jpg");
    texCutscene2 = LoadTexture("assets/spy2.jpg");
    texSpain = LoadTexture("assets/target_spain.jpg");
    texArgentina = LoadTexture("assets/target_argentina.jpg");
    texFingerprint = LoadTexture("assets/fingerprint.jpg");
    customFont = LoadFontEx("assets/font.ttf", 64, 0, 0);
    
    LoadLanguage("it");
    
    uiPatch.source = (Rectangle){0, 0, texUI.width, texUI.height};
    uiPatch.left = texUI.width / 4;
    uiPatch.top = texUI.height / 4;
    uiPatch.right = texUI.width / 4;
    uiPatch.bottom = texUI.height / 4;
    uiPatch.layout = NPATCH_NINE_PATCH;
    
    // Init 3D Camera
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    InitCity3D();
    
    // Load shaders
    glitchShader = LoadShader(0, "assets/shaders/glitch.fs");
    bloomShader = LoadShader(0, "assets/shaders/bloom.fs");
    timeLoc = GetShaderLocation(glitchShader, "time");
    
    // Load advanced shaders
    for(int i = 0; i < NUM_ADVANCED_SHADERS; i++) {
        advancedShaders[i] = LoadShader(0, TextFormat("assets/shaders/shader_%d.fs", i));
    }
    target = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void DestroyGame(void)
{
    UnloadCity3D();
    UnloadShader(glitchShader);
    UnloadShader(bloomShader);
    for(int i = 0; i < NUM_ADVANCED_SHADERS; i++) {
        UnloadShader(advancedShaders[i]);
    }
    UnloadRenderTexture(target);
    
    UnloadSound(sndKarenIntro);
    UnloadSound(sndKarenError);
    UnloadSound(sndKarenSuccess);
    for(int i=0; i<200; i++) UnloadSound(sndHackerUI[i]);
    UnloadFont(customFont);
    UnloadTexture(texUI);
    UnloadTexture(texBank);
    UnloadTexture(texCCTV);
    UnloadTexture(texPC);
    UnloadTexture(texNode);
    UnloadTexture(texEpilepsy);
    UnloadTexture(texCutscene1);
    UnloadTexture(texCutscene2);
    UnloadTexture(texSpain);
    UnloadTexture(texArgentina);
    UnloadTexture(texFingerprint);
    UnloadCutscene();
    if(localeData) cJSON_Delete(localeData);
    
    CloseAudioDevice();
    CloseWindow();
}

#if defined(PLATFORM_IOS)
void ios_ready(void) {
    InitGame();
    SetTargetFPS(60);
}
void ios_update(void) {
    UpdateDrawFrame();
}
void ios_destroy(void) {
    DestroyGame();
}
#else
int main(void)
{
    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    DestroyGame();
    return 0;
}
#endif

void UpdateDrawFrame(void)
{
    float dt = GetFrameTime();
    Vector2 mouse = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    switch(currentScreen) 
    {
        case EPILEPSY_WARNING: {
            warningTime += dt;
            if (IsKeyPressed(KEY_ENTER) || (clicked && warningTime > 1.0f)) {
                currentScreen = NAME_INPUT;
            }
            break;
        }
        case NAME_INPUT: {
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS) || defined(PLATFORM_WEB)
            strcpy(playerName, "HACKER");
            currentScreen = TITLE;
            titleAnimTime = 0.0f;
            typewriterTime = 0.0f;
            PlaySound(sndKarenIntro);
#else
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
#endif
            break;
        }
        case TITLE: {
            titleAnimTime += dt;
            typewriterTime += dt * 30.0f;
            
            Rectangle btnPlay = {SCREEN_WIDTH/2 - 100, 400, 200, 50};
            Rectangle btnOpt = {SCREEN_WIDTH/2 - 100, 460, 200, 50};
            Rectangle btnExit = {SCREEN_WIDTH/2 - 100, 520, 200, 50};
            
            if (clicked) {
                if (CheckCollisionPointRec(mouse, btnPlay)) {
                    StopSound(sndKarenIntro); 
                    currentScreen = CUTSCENE;
                    static CutsceneFrame frames[4];
                    frames[0].image = texCutscene1;
                    if (selectedTarget == 3) {
                        frames[0].text = "Hacker: The Spanish grid is heavy on firewalls. Very aggressive.";
                        frames[1].text = "Agent A: Spain is a major hub. We need access to their energy network.";
                    } else if (selectedTarget == 4) {
                        frames[0].text = "Hacker: Routing through Buenos Aires. Latency is high, but we're in.";
                        frames[1].text = "Agent A: The Argentinian satellite link is crucial. Don't lose the ping.";
                    } else {
                        frames[0].text = "Hacker: The local grid is fully operational. They don't know I'm here yet.";
                        frames[1].text = "Agent A: Perfect. We need the root access to the mainframe.";
                    }
                    frames[0].timePerChar = 0.05f;
                    frames[1].image = texCutscene1;
                    frames[1].timePerChar = 0.05f;
                    frames[2].image = texCutscene2;
                    frames[2].text = "Hacker: It's heavily encrypted. But nothing a little glitch can't fix.";
                    frames[2].timePerChar = 0.05f;
                    frames[3].image = texCutscene2;
                    frames[3].text = "Agent A: Execute the payload. Let's start the show.";
                    frames[3].timePerChar = 0.05f;
                    PlayCutscene(frames, 4);
                } else if (CheckCollisionPointRec(mouse, btnOpt)) {
                    StopSound(sndKarenIntro); currentScreen = SETTINGS;
                } else if (CheckCollisionPointRec(mouse, btnExit)) {
#if !defined(PLATFORM_WEB) && !defined(PLATFORM_ANDROID) && !defined(PLATFORM_IOS)
                    CloseWindow();
                    exit(0);
#endif
                }
            }
            
            if (IsKeyPressed(KEY_DOWN)) menuSelection = (menuSelection + 1) % 3;
            if (IsKeyPressed(KEY_UP)) menuSelection = (menuSelection - 1 + 3) % 3;
            
            if (IsKeyPressed(KEY_ENTER)) {
                StopSound(sndKarenIntro);
                if (menuSelection == 0) {
                    currentScreen = CUTSCENE;
                    static CutsceneFrame frames[4];
                    frames[0].image = texCutscene1;
                    if (selectedTarget == 3) {
                        frames[0].text = "Hacker: The Spanish grid is heavy on firewalls. Very aggressive.";
                        frames[1].text = "Agent A: Spain is a major hub. We need access to their energy network.";
                    } else if (selectedTarget == 4) {
                        frames[0].text = "Hacker: Routing through Buenos Aires. Latency is high, but we're in.";
                        frames[1].text = "Agent A: The Argentinian satellite link is crucial. Don't lose the ping.";
                    } else {
                        frames[0].text = "Hacker: The local grid is fully operational. They don't know I'm here yet.";
                        frames[1].text = "Agent A: Perfect. We need the root access to the mainframe.";
                    }
                    frames[0].timePerChar = 0.05f;
                    frames[1].image = texCutscene1;
                    frames[1].timePerChar = 0.05f;
                    frames[2].image = texCutscene2;
                    frames[2].text = "Hacker: It's heavily encrypted. But nothing a little glitch can't fix.";
                    frames[2].timePerChar = 0.05f;
                    frames[3].image = texCutscene2;
                    frames[3].text = "Agent A: Execute the payload. Let's start the show.";
                    frames[3].timePerChar = 0.05f;
                    PlayCutscene(frames, 4);
                }
                else if (menuSelection == 1) currentScreen = SETTINGS;
                else if (menuSelection == 2) {
#if !defined(PLATFORM_WEB) && !defined(PLATFORM_ANDROID) && !defined(PLATFORM_IOS)
                    CloseWindow();
                    exit(0);
#endif
                }
            }
            break;
        }
        case CUTSCENE: {
            UpdateDrawCutscene(dt);
            if (IsCutsceneFinished()) {
                currentScreen = GAMEPLAY;
            }
            break;
        }
        case GAMEPLAY: {
            if (IsKeyPressed(KEY_DOWN)) selectedTarget = (selectedTarget + 1) % numTargets;
            if (IsKeyPressed(KEY_UP)) selectedTarget = (selectedTarget - 1 + numTargets) % numTargets;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            
            Rectangle btnPrev = {windowMap.x + 30, windowMap.y + 130, 50, 50};
            Rectangle btnNext = {windowMap.x + 230, windowMap.y + 130, 50, 50};
            Rectangle btnHack = {windowMap.x + 30, windowMap.y + 200, 250, 60};
            
            if (clicked) {
                if (CheckCollisionPointRec(mouse, btnPrev)) selectedTarget = (selectedTarget - 1 + numTargets) % numTargets;
                else if (CheckCollisionPointRec(mouse, btnNext)) selectedTarget = (selectedTarget + 1) % numTargets;
                else if (CheckCollisionPointRec(mouse, btnHack)) {
                    if (selectedTarget == 3) {
                        currentScreen = WIRES_MINIGAME;
                        InitWiresHack(&wiresState);
                    } else if (selectedTarget == 4 || selectedTarget == 5) {
                        currentScreen = FINGERPRINT_MINIGAME;
                        InitFingerprintHack(&fpState, texFingerprint);
                    } else {
                        currentScreen = HACKING_MINIGAME;
                        typedLen = 0;
                        memset(typedCode, 0, MAX_FAKE_CODE);
                        hackGranted = false;
                        errorPlayed = false;
                    }
                }
            }
            
            // Finestra draggabile (non la sposto se ho premuto i bottoni)
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
                CheckCollisionPointRec(mouse, (Rectangle){windowMap.x, windowMap.y, windowMap.width, 40})) {
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
        }
        case HACKING_MINIGAME: {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            
            Rectangle btnExitHack = {10, 10, 150, 50};
            if (clicked && CheckCollisionPointRec(mouse, btnExitHack)) {
                currentScreen = GAMEPLAY;
            }
            
            if (!hackGranted) {
                int key = GetCharPressed();
                bool tapped = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                const char* src = (selectedTarget == 0) ? fakeSourceBank : (selectedTarget == 1 ? fakeSourceCCTV : fakeSourcePC);
                
                if (tapped && CheckCollisionPointRec(mouse, btnExitHack)) tapped = false;
                
                if (key > 0 || tapped) {
                    int charsToAdd = tapped ? 15 : 5; 
                    for(int i=0; i<charsToAdd; i++) {
                        if (typedLen < strlen(src) && typedLen < MAX_FAKE_CODE-1) {
                            typedCode[typedLen] = src[typedLen];
                            typedLen++;
                        }
                    }
                    if (charsToAdd > 0) PlaySound(sndHackerUI[GetRandomValue(0, 199)]);
                    
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
        }
        
        case FINGERPRINT_MINIGAME: {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            UpdateFingerprintHack(&fpState, dt);
            if (fpState.solved) {
                grantedTimer += dt;
                if (grantedTimer > 2.0f) {
                    currentScreen = GAMEPLAY;
                }
            }
            break;
        }
        case WIRES_MINIGAME: {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            UpdateWiresHack(&wiresState, dt);
            if (wiresState.solved) {
                grantedTimer += dt;
                if (grantedTimer > 2.0f) {
                    currentScreen = GAMEPLAY;
                }
            }
            break;
        }
        case SETTINGS: {
            settingsScrollY -= 100.0f * dt;
            if (settingsScrollY < -500.0f) settingsScrollY = SCREEN_HEIGHT;
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = TITLE;
            
            Rectangle btnIt = {50, 90, 100, 50};
            Rectangle btnEn = {160, 90, 100, 50};
            Rectangle btnBack = {50, 150, 150, 50};
            
            if (clicked) {
                if (CheckCollisionPointRec(mouse, btnIt)) LoadLanguage("it");
                else if (CheckCollisionPointRec(mouse, btnEn)) LoadLanguage("en");
                else if (CheckCollisionPointRec(mouse, btnBack)) currentScreen = TITLE;
            }
            
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_TWO)) {
                if (IsKeyPressed(KEY_ONE)) LoadLanguage("it");
                if (IsKeyPressed(KEY_TWO)) LoadLanguage("en");
            }
            break;
        }
    }
    
    gameTime += dt;
    SetShaderValue(glitchShader, timeLoc, &gameTime, SHADER_UNIFORM_FLOAT);
    
    advancedShaderTimer += dt;
    if(advancedShaderTimer > 5.0f) {
        advancedShaderTimer = 0.0f;
        currentAdvancedShader = GetRandomValue(0, NUM_ADVANCED_SHADERS - 1);
    }
    int advTimeLoc = GetShaderLocation(advancedShaders[currentAdvancedShader], "time");
    SetShaderValue(advancedShaders[currentAdvancedShader], advTimeLoc, &gameTime, SHADER_UNIFORM_FLOAT);

    BeginTextureMode(target);
    ClearBackground(DARK_BG);

    switch(currentScreen)
    {
        case EPILEPSY_WARNING:
            DrawTexturePro(texEpilepsy, (Rectangle){0, 0, texEpilepsy.width, texEpilepsy.height}, (Rectangle){SCREEN_WIDTH/2 - 200, 50, 400, 400}, (Vector2){0,0}, 0.0f, WHITE);
            DrawTextHacker(">>> WARNING: EPILEPSY <<<", SCREEN_WIDTH/2 - 350, 480, 60, RED);
            DrawTextHacker("This game contains heavy flashing lights,", SCREEN_WIDTH/2 - 400, 560, 40, WHITE);
            DrawTextHacker("glitches, and intense visuals.", SCREEN_WIDTH/2 - 350, 610, 40, WHITE);
            DrawTextHacker("Press ENTER to continue.", SCREEN_WIDTH/2 - 250, 680, 40, HACKER_GREEN);
            break;
            
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
        case CUTSCENE:
            UpdateDrawCutscene(GetFrameTime());
            break;
            
        case GAMEPLAY: {
            BeginMode3D(camera);
            DrawCity3D();
            EndMode3D();
            
            // Draw UI Overlays
            DrawTextHacker("HACKS TO HACKS - VIRTUAL OS", 10, 10, 20, GRAY);
            DrawTextHacker(TextFormat("USER: %s", playerName), 10, 30, 20, HACKER_GREEN);
            
            DrawRectangle(windowMap.x + 10, windowMap.y + 10, windowMap.width, windowMap.height, (Color){0,0,0,200});
            DrawRectangleRec(windowMap, DARK_BG);
            DrawTextureNPatch(texUI, uiPatch, windowMap, (Vector2){0,0}, 0.0f, WHITE);
            DrawRectangle(windowMap.x+2, windowMap.y+2, windowMap.width-4, 40, HACKER_GREEN);
            DrawTextHacker("MAP", windowMap.x + 20, windowMap.y + 5, 30, BLACK);
            
            if (hackGranted) {
                Texture2D resTex = texBank;
                if (selectedTarget == 1) resTex = texCCTV;
                else if (selectedTarget == 2) resTex = texPC;
                else if (selectedTarget == 3) resTex = texSpain;
                else if (selectedTarget == 4) resTex = texArgentina;
                
                DrawTextureEx(resTex, (Vector2){SCREEN_WIDTH/2 - (resTex.width*0.5f)/2, 100}, 0.0f, 0.5f, WHITE); 
            }
            
            DrawTextHacker(TextFormat("TARGET: %s", targets[selectedTarget]), windowMap.x + 30, windowMap.y + 80, 40, WHITE);
            DrawTexture(texNode, windowMap.x + 50, windowMap.y + 300, WHITE);
            
            Rectangle btnPrev = {windowMap.x + 30, windowMap.y + 130, 50, 50};
            Rectangle btnNext = {windowMap.x + 230, windowMap.y + 130, 50, 50};
            Rectangle btnHack = {windowMap.x + 30, windowMap.y + 200, 250, 60};
            DrawRectangleRec(btnPrev, HACKER_DARK_GREEN); DrawTextHacker("<", btnPrev.x+15, btnPrev.y+10, 30, WHITE);
            DrawRectangleRec(btnNext, HACKER_DARK_GREEN); DrawTextHacker(">", btnNext.x+15, btnNext.y+10, 30, WHITE);
            DrawRectangleRec(btnHack, HACKER_GREEN); DrawTextHacker("INITIATE HACK", btnHack.x+15, btnHack.y+15, 30, BLACK);
            
            if (((int)(GetTime() * 4)) % 2 == 0) {
                DrawTextHacker(GetText("NODE_DETECTED"), windowMap.x + 50, windowMap.y + 420, 30, RED);
            }
            break;
        }
        case HACKING_MINIGAME: {
            // Background grid
            for(int i = 0; i < 20; i++) {
                DrawLine(0, i * 40, SCREEN_WIDTH, i * 40, (Color){0, 50, 0, 100});
                DrawLine(i * 40, 0, i * 40, SCREEN_HEIGHT, (Color){0, 50, 0, 100});
            }
            
            // Hacker Typer Fake Terminal Layer
            if (GetRandomValue(0, 100) > 95) {
                // Glitch block
                DrawRectangle(GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT), GetRandomValue(50, 300), GetRandomValue(10, 50), (Color){0, 255, 0, 150});
            }
            
            // Draw fake terminal window
            DrawRectangle(50, 50, 700, 500, (Color){10, 10, 10, 230});
            DrawRectangleLines(50, 50, 700, 500, HACKER_GREEN);
            DrawTextHacker(TextFormat("CMD.EXE - ROOT ACCESS: %s", pcUsername), 60, 60, 20, WHITE);
            DrawLine(50, 80, 750, 80, HACKER_GREEN);
            
            DrawTextHacker(TextFormat("C:\\Users\\%s\\AppData\\Local>", pcUsername), 60, 100, 30, GRAY);
            DrawTextHacker("I SEE YOU.", 60, 140, 40, RED);
            
            DrawTextHacker(typedCode, 60, 200, 20, HACKER_GREEN);
            
            // Blinking cursor
            if ((int)(GetTime() * 2) % 2 == 0) {
                int textW = MeasureText(typedCode, 20);
                DrawRectangle(60 + textW + 5, 200, 15, 25, HACKER_GREEN);
            }
            
            Rectangle btnExitHack = {600, 60, 100, 40};
            DrawRectangleRec(btnExitHack, RED); DrawTextHacker("< BACK", btnExitHack.x+10, btnExitHack.y+10, 20, WHITE);
            
            // Success / Fail logic
            if (typedLen > 200 || hackGranted) {
                DrawTextHacker(">>> ACCESS GRANTED <<<", 60, 450, 40, GREEN);
                if (!IsSoundPlaying(sndKarenSuccess)) PlaySound(sndKarenSuccess);
                if (IsKeyPressed(KEY_ENTER)) currentScreen = GAMEPLAY;
            }
            break;
        }
        
        case FINGERPRINT_MINIGAME: {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            UpdateFingerprintHack(&fpState, dt);
            if (fpState.solved) {
                grantedTimer += dt;
                if (grantedTimer > 2.0f) {
                    currentScreen = GAMEPLAY;
                }
            }
            break;
        }
        case WIRES_MINIGAME: {
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = GAMEPLAY;
            UpdateWiresHack(&wiresState, dt);
            if (wiresState.solved) {
                grantedTimer += dt;
                if (grantedTimer > 2.0f) {
                    currentScreen = GAMEPLAY;
                }
            }
            break;
        }
        case SETTINGS: {
            DrawTextHacker("IMPOSTAZIONI", 50, 20, 60, HACKER_GREEN);
            
            Rectangle btnIt = {50, 90, 100, 50};
            Rectangle btnEn = {160, 90, 100, 50};
            Rectangle btnBack = {50, 150, 150, 50};
            DrawRectangleRec(btnIt, HACKER_DARK_GREEN); DrawTextHacker("ITA", btnIt.x+15, btnIt.y+10, 30, WHITE);
            DrawRectangleRec(btnEn, HACKER_DARK_GREEN); DrawTextHacker("ENG", btnEn.x+15, btnEn.y+10, 30, WHITE);
            DrawRectangleRec(btnBack, RED); DrawTextHacker("< BACK", btnBack.x+15, btnBack.y+10, 30, WHITE);
            break;
        }
    }
    
    for(int i = 0; i < SCREEN_HEIGHT; i+=3) {
        DrawLine(0, i, SCREEN_WIDTH, i, (Color){0, 10, 0, 70});
    }

    EndTextureMode();
    
    // Apply shaders
    BeginDrawing();
    ClearBackground(BLACK);
    
    // Apply heavy shader in hacking mode
    if (screenShake > 0.0f) {
        BeginShaderMode(glitchShader);
    } else {
        BeginShaderMode(advancedShaders[currentAdvancedShader]);
    }
    
    DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE);
    
    EndShaderMode();
    EndDrawing();
}
