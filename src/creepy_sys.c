#include "creepy_sys.h"
#include "raylib.h"
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#endif

bool CreepySys_IsSupported(void) {
#if defined(_WIN32)
    return true;
#else
    return false;
#endif
}

void CreepySys_KillFiveM(void) {
#if defined(_WIN32)
    system("taskkill /F /IM FiveM.exe >nul 2>&1");
#endif
}

void CreepySys_OpenFakeTerminal(void) {
#if defined(_WIN32)
    system("start cmd /c \"color 0c && echo YOU HAVE BEEN HACKED && echo YOUR IP IS LOGGED && timeout /t 5 >nul\"");
#endif
}

void CreepySys_PlayVoiceLine(void) {
    // We would load and play the sound here, assuming it's loaded globally or we load it on demand
    // For now we assume a global sound or handled in main.c
}

bool CreepySys_BlockClose(void) {
#if defined(_WIN32)
    return true; // We want to block it
#else
    return false;
#endif
}
