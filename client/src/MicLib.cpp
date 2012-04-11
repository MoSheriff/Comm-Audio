#include "NetworkingComponent.h"
#include "MicLib.h"

#define BLOCKSIZE 1024
#define BLOCKCOUNT 20

WAVEHDR blocks[BLOCKCOUNT];
HWAVEIN hWaveIn;
HANDLE hFile = INVALID_HANDLE_VALUE;
HANDLE hWaveFile = INVALID_HANDLE_VALUE;
bool recording = false;
int doneAll = 0;
std::string ip;

static void PrintWaveErrorMsg(DWORD err, TCHAR * str)
{
    #define BUFFERSIZE 128
    char	buffer[BUFFERSIZE];

    printf("ERROR 0x%08X: %s\r\n", err, str);
    if (mciGetErrorString(err, &buffer[0], sizeof(buffer)))
    {
        printf("%s\r\n", &buffer[0]);
    }
    else
    {
        printf("0x%08X returned!\r\n", err);
    }
}

static DWORD WINAPI waveInProc(void *nc)
{    
	MSG msg;
    NetworkingComponent *net = (NetworkingComponent *) nc;

	while (GetMessage(&msg, 0, 0, 0) == 1)
	{
		switch (msg.message)
		{
			case MM_WIM_DATA:
			{
				if (((WAVEHDR *)msg.lParam)->dwBytesRecorded)
				{
                    if(net->sendUDP(((WAVEHDR *)msg.lParam)->lpData, ((WAVEHDR *)msg.lParam)->dwBytesRecorded, ip.c_str()) == -1)
                    {
                        MessageBox(NULL, "Send failed.", "error", MB_ICONERROR);
                    }

                    if (!WriteFile(hWaveFile, ((WAVEHDR *)msg.lParam)->lpData, ((WAVEHDR *)msg.lParam)->dwBytesRecorded, &msg.time, 0))
                    {
                        int err = GetLastError();
                    }
				}

				if (recording)
				{
					waveInAddBuffer(hWaveIn, (WAVEHDR *)msg.lParam, sizeof(WAVEHDR));
				}
				else
				{
					++doneAll;
				}

				/* Keep waiting for more messages */
                continue;
			}

			/* Our main thread is opening the WAVE device */
			case MM_WIM_OPEN:
			{
                doneAll = 0;
                continue;
			}

			/* Our main thread is closing the WAVE device */
			case MM_WIM_CLOSE:
			{
				/* Terminate this thread (by return'ing) */
				break;
			}
		}
	}

	return 0;
}

MicLib::MicLib(NetworkingComponent *nc)
    : netcomp(nc)
{
    micFormat_.nSamplesPerSec = 44100; /*sample rate*/
    micFormat_.wBitsPerSample = 16; /*sample size*/
    micFormat_.nChannels= 2;  /*channels*/
    micFormat_.cbSize = 0;  /*size of extra info*/
    micFormat_.wFormatTag = WAVE_FORMAT_PCM;
    micFormat_.nBlockAlign = (micFormat_.wBitsPerSample * micFormat_.nChannels) >> 3;
    micFormat_.nAvgBytesPerSec = micFormat_.nBlockAlign * micFormat_.nSamplesPerSec;
}

void MicLib::record(const std::string& cip)
{
	MMRESULT err;
	HANDLE waveInThread;

	char*	buff;
	DWORD	dwBytesRead, dwBytesWrite;

    ip = cip;
	buff = (char*) malloc(sizeof(char));

	waveInThread = CreateThread(0, 0, waveInProc, netcomp, 0, (DWORD *) &err);
	if (!waveInThread)
	{
        MessageBox(NULL, "Unable to create wave in thread.", "Error", MB_ICONERROR);
	}
	CloseHandle(waveInThread);

	ZeroMemory(&blocks[0], sizeof(WAVEHDR) * BLOCKCOUNT);

	err = waveInOpen(&hWaveIn, WAVE_MAPPER, &micFormat_, (DWORD)err, 0, CALLBACK_THREAD);
	if (err)
	{
		PrintWaveErrorMsg(err, "Can't open WAVE In Device!");
		return;
	}

    if ((hWaveFile = CreateFile("C:\\test.snd", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, "Error opening file.", "Error", MB_ICONERROR);
        return;
    }

    for (int i = 0; i < BLOCKCOUNT; i++)
        blocks[i].dwBufferLength = BLOCKSIZE;

    if (!(blocks[0].lpData = (char *)VirtualAlloc(0, blocks[0].dwBufferLength * BLOCKCOUNT, MEM_COMMIT, PAGE_READWRITE)))
    {
        printf("ERROR: Can't allocate memory for WAVE buffer!\n");
        return;
    }

    for (int i = 1; i < BLOCKCOUNT; i++)
        blocks[i].lpData = blocks[i - 1].lpData + blocks[i - 1].dwBufferLength;

    for (int i = 0; i < BLOCKCOUNT; i++)
    {
        if ((err = waveInPrepareHeader(hWaveIn, &blocks[i], sizeof(WAVEHDR))))
        {
            printf("Error preparing WAVEHDR 1! -- %08X\n", err);
            return;
        }

        /* Queue first WAVEHDR (recording hasn't started yet) */
        if ((err = waveInAddBuffer(hWaveIn, &blocks[i], sizeof(WAVEHDR))))
        {
            printf("Error queueing WAVEHDR 1! -- %08X\n", err);
            return;
        }
    }

    recording = true;
	if ((err = waveInStart(hWaveIn)))
	{
		printf("Error starting record! -- %08X\n", err);
	}
	else
	{
		/* Wait for user to stop recording */
		printf("Recording has started. Press ENTER key to stop recording...\n");
	}
}

void MicLib::stop()
{
    DWORD err;

    recording = false;
    CloseHandle(hWaveFile);
    waveInReset(hWaveIn);

    for (int i = 0; i < BLOCKCOUNT; i++)
    {
        if ((err = waveInPrepareHeader(hWaveIn, &blocks[i], sizeof(WAVEHDR))))
        {

            printf("Error unpreparing WAVEHDR 2! -- %08X\n", err);
        }
    }

    waveInClose(hWaveIn);
    if (blocks[0].lpData) VirtualFree(blocks[0].lpData, 0, MEM_RELEASE);
}