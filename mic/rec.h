#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <mmsystem.h>

#define BUFFERSIZE 128
#define ERRMSG_PRINT	0
#define ERRMSG_NONE		1

HWAVEIN				WaveInHandle;
HMIXER				MixerHandle;
HANDLE				WaveFileHandle = INVALID_HANDLE_VALUE;
WAVEHDR				WaveHeader[2];
BOOL				InRecord = FALSE;
unsigned char		DoneAll;

void PrintWaveErrorMsg(DWORD err, TCHAR * str);
DWORD WINAPI waveInProc(LPVOID arg);
void set_mute(MIXERLINE *mixerLine, DWORD val, BOOL errmsg);
