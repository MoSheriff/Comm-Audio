#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H


#include "NetworkingComponent.h"
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <list>

#pragma comment(lib, "WinMM.Lib")

#define BLOCK_SIZE 8192
#define BUFSIZE    1024
#define BLOCK_COUNT 20

#define EVER    (;;){}

typedef struct _globals
{
    HWAVEOUT            waveOut;
    HANDLE              hFile, hThread;
    WAVEHDR             *blocks;
    WAVEFORMATEX        wfx;
    int                 freeBlocks;
    int                 currentBlock, len;
    WSABUF              buffer;
    CRITICAL_SECTION    countGuard;
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
    std::list<std::string> getTitles();
    int connect(char *host, unsigned short socket);
    void setNc(NetworkingComponent *nc);
    static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
    static DWORD WINAPI playProc(LPVOID lpParameter);
    void skip();
    void unprepareBlocks();
    void quit();

private:
    
    PGLOBALS    globals;
};

#endif