#ifndef INCLUDES_H
#define INCLUDES_H

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#define BLOCK_SIZE 8192
#define BUFSIZE    1024
#define BLOCK_COUNT 20

typedef struct _globals
{
    HWAVEOUT waveOut;
    WAVEHDR* blocks;
    WAVEFORMATEX wfx;
    int freeBlocks;
    int currentBlock;
    char buffer[BUFSIZE];
    CRITICAL_SECTION countGuard;

}GLOBALS, *PGLOBALS;







#endif