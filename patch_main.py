import re

with open('src/main.c', 'r') as f:
    content = f.read()

# 1. Include minigames
content = content.replace('#include "cutscene.h"', '#include "cutscene.h"\n#include "minigame_fingerprint.h"\n#include "minigame_wires.h"')

# 2. Add GameScreens
content = content.replace('HACKING_MINIGAME } GameScreen;', 'HACKING_MINIGAME, FINGERPRINT_MINIGAME, WIRES_MINIGAME } GameScreen;')

# 3. Add Minigame states
content = content.replace('Sound sndHackerUI[10];', '''Sound sndHackerUI[200];
FingerprintHackState fpState;
WiresHackState wiresState;''')

# 4. Add textures
content = content.replace('Texture2D texArgentina;', 'Texture2D texArgentina;\nTexture2D texFingerprint;')
content = content.replace('texArgentina = LoadTexture("assets/target_argentina.jpg");', 'texArgentina = LoadTexture("assets/target_argentina.jpg");\n    texFingerprint = LoadTexture("assets/fingerprint.jpg");')
content = content.replace('UnloadTexture(texArgentina);', 'UnloadTexture(texArgentina);\n    UnloadTexture(texFingerprint);')
content = content.replace('texNode = LoadTexture("assets/node_1.png");', 'texNode = LoadTexture("assets/node_new.jpg");')

# 5. Expand targets
content = content.replace('int numTargets = 5;', 'int numTargets = 6;')
content = content.replace('int selectedTarget = 0; // 0=Naples, 1=CCTV, 2=PC, 3=Spain, 4=Argentina', 'int selectedTarget = 0;')
content = content.replace('const char* targets[] = { "CENTRAL BANK NAPLES", "CCTV GRID MILAN", "UNKNOWN PC", "SPAIN POWER GRID", "ARGENTINA SAT LINK" };', 
                          'const char* targets[] = { "CENTRAL BANK NAPLES", "CCTV GRID MILAN", "UNKNOWN PC", "ELEVATOR CONTROL (MILAN)", "BROADCAST TOWER (MADRID)", "SMART TV NETWORK (BUENOS AIRES)" };')

# 6. Random sound macro
content = content.replace('PlaySound(sndHackerUI[GetRandomValue(0, 9)]);', 'PlaySound(sndHackerUI[GetRandomValue(0, 199)]);')

# 7. Loading Sounds loop
content = content.replace('for(int i=0; i<10; i++) {', 'for(int i=0; i<200; i++) {')
content = content.replace('for(int i=0; i<10; i++) UnloadSound(sndHackerUI[i]);', 'for(int i=0; i<200; i++) UnloadSound(sndHackerUI[i]);')

# 8. Hack Button Logic
hack_btn_logic = '''if (CheckCollisionPointRec(mouse, btnPrev)) selectedTarget = (selectedTarget - 1 + numTargets) % numTargets;
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
                }'''

content = re.sub(
    r'if \(CheckCollisionPointRec\(mouse, btnPrev\)\) selectedTarget = \(selectedTarget - 1 \+ numTargets\) % numTargets;.*?else if \(CheckCollisionPointRec\(mouse, btnHack\)\) \{.*?\}',
    hack_btn_logic,
    content,
    flags=re.DOTALL
)

# 9. Main Loop Switch cases
states = '''
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
        case SETTINGS: {'''
content = content.replace('case SETTINGS: {', states)

draw_states = '''
        case FINGERPRINT_MINIGAME: {
            DrawFingerprintHack(&fpState, windowMap);
            break;
        }
        case WIRES_MINIGAME: {
            DrawWiresHack(&wiresState, windowMap);
            break;
        }
        case CUTSCENE: {'''
content = content.replace('case CUTSCENE: {', draw_states)

with open('src/main.c', 'w') as f:
    f.write(content)
