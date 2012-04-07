#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H


#include "../../lib/NetworkingComponent.h"
#include <windows.h>
#include <mmsystem.h>
#include <string>

#pragma comment(lib, "WinMM.Lib")

#define BLOCK_SIZE 8192
#define BUFSIZE    1024
#define BLOCK_COUNT 20

#define EVER    (;;)

typedef struct _globals
{
    HWAVEOUT            waveOut;
    WAVEHDR             *blocks;
    WAVEFORMATEX        wfx;
    int                 freeBlocks;
    int                 currentBlock, len;
    WSABUF              buffer;
    CRITICAL_SECTION    countGuard;
    char                *titles;
    NetworkingComponent *nc;

}GLOBALS, *PGLOBALS;


class AudioOutput{

public:

    AudioOutput();
    ~AudioOutput();
    WAVEHDR* allocateBlocks(int size, int count);
    void freeBlocks(WAVEHDR* blockArray);
    void writeAudio(HWAVEOUT waveOut, LPSTR data, int size);
    void playAudio();
    void cleanUp();
    void initialize();
    void getHeaderData();
    void getTitles();
    int connect(char *host, unsigned short socket);
    static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
    static DWORD WINAPI playProc(LPVOID lpParameter);

private:
    
    PGLOBALS    globals;
};

#endif