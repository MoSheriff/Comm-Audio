#ifndef AUDIOOUT_H
#define AUDIOOUT_H

#include "includes.h"

void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
WAVEHDR* allocateBlocks(int size, int count);
void freeBlocks(WAVEHDR* blockArray);
void writeAudio(HWAVEOUT waveOut, PGLOBALS globals, LPSTR data, int size);
void playAudio(HANDLE hFile, PGLOBALS globals);
void cleanUp(PGLOBALS globals);
void initialize(PGLOBALS globals);
void getHeaderData(PGLOBALS globals);

#endif