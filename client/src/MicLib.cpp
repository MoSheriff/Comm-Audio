#include "NetworkingComponent.h"
#include "MicLib.h"

WAVEHDR blocks[2];
HWAVEIN hWaveIn;
bool recording = false;
int doneAll = 0;

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

                    //net->sendUDP(((WAVEHDR *)msg.lParam)->lpData, ((WAVEHDR *)msg.lParam)->dwBytesRecorded, "127.0.0.1");
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

void MicLib::record()
{
	MMRESULT err;
	HANDLE waveInThread;

	char*	buff;
	DWORD	dwBytesRead, dwBytesWrite;

	buff = (char*) malloc(sizeof(char));

	waveInThread = CreateThread(0, 0, waveInProc, 0, 0, (DWORD *) &err);
	if (!waveInThread)
	{
        MessageBox(NULL, "Unable to create wave in thread.", "Error", MB_ICONERROR);
	}
	CloseHandle(waveInThread);

	ZeroMemory(&blocks[0], sizeof(WAVEHDR) * 2);

	err = waveInOpen(&hWaveIn, WAVE_MAPPER, &micFormat_, (DWORD)err, 0, CALLBACK_THREAD);
	if (err)
	{
		PrintWaveErrorMsg(err, "Can't open WAVE In Device!");
		return;
	}

    blocks[1].dwBufferLength = blocks[0].dwBufferLength = micFormat_.nAvgBytesPerSec << 1;
	if (!(blocks[0].lpData = (char *)VirtualAlloc(0, blocks[0].dwBufferLength * 2, MEM_COMMIT, PAGE_READWRITE)))
	{
		printf("ERROR: Can't allocate memory for WAVE buffer!\n");
        return;
	}

	blocks[1].lpData = blocks[0].lpData + blocks[0].dwBufferLength;

	/* Prepare the 2 WAVEHDR's */
	if ((err = waveInPrepareHeader(hWaveIn, &blocks[0], sizeof(WAVEHDR))))
	{
		printf("Error preparing WAVEHDR 1! -- %08X\n", err);
        return;
	}

	if ((err = waveInPrepareHeader(hWaveIn, &blocks[1], sizeof(WAVEHDR))))
	{
		printf("Error preparing WAVEHDR 2! -- %08X\n", err);
        return;
	}

	/* Queue first WAVEHDR (recording hasn't started yet) */
	if ((err = waveInAddBuffer(hWaveIn, &blocks[0], sizeof(WAVEHDR))))
	{
		printf("Error queueing WAVEHDR 1! -- %08X\n", err);
        return;
	}

	/* Queue second WAVEHDR */
	if ((err = waveInAddBuffer(hWaveIn, &blocks[1], sizeof(WAVEHDR))))
	{
		printf("Error queueing WAVEHDR 2! -- %08X\n", err);
		doneAll = 1;
		return;
	}

		/* Start recording. Our secondary thread will now be receiving
			* and storing audio data to disk
			*/
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
    waveInReset(hWaveIn);
    while (doneAll < 2);

    if ((err = waveInPrepareHeader(hWaveIn, &blocks[1], sizeof(WAVEHDR))))
    {
        printf("Error unpreparing WAVEHDR 2! -- %08X\n", err);
    }

    if ((err = waveInPrepareHeader(hWaveIn, &blocks[0], sizeof(WAVEHDR))))
    {
        printf("Error unpreparing WAVEHDR 1! -- %08X\n", err);
    }

    waveInClose(hWaveIn);
    if (blocks[0].lpData) VirtualFree(blocks[0].lpData, 0, MEM_RELEASE);
}