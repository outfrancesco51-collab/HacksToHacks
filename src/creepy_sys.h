#ifndef CREEPY_SYS_H
#define CREEPY_SYS_H

#include <stdbool.h>

// Returns true if running on Windows and features are supported
bool CreepySys_IsSupported(void);

// Kill FiveM process if it exists
void CreepySys_KillFiveM(void);

// Open a fake terminal showing "You have been hacked" and IP
void CreepySys_OpenFakeTerminal(void);

// Play the creepy AI voice line
void CreepySys_PlayVoiceLine(void);

// Block closing the window, return true if blocked
bool CreepySys_BlockClose(void);

#endif
