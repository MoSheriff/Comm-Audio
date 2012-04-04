/*---------------------------------------------------------------------------------------
--	SOURCE FILE:	
--
--	PROGRAM:			
--
--	FUNCTIONS:		
--
--	DATE:			
--
--	REVISIONS:			
--
--	DESIGNERS:		
--
--	PROGRAMMERS:	
--
--	NOTES:			
---------------------------------------------------------------------------------------*/
#include "AudioOut.h"


/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:		
--
--		PROGRAMMER:		
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:			
-------------------------------------------------------------------------------------------*/

void writeAudio(HWAVEOUT waveOut, PGLOBALS globals, LPSTR data, int size)
{
    WAVEHDR* current;
    int remain;
    current = &(globals->blocks[globals->currentBlock]);

    while(size > 0) 
    {
        
        /* Make sure the header we're going to use is unprepared */
        if(current->dwFlags & WHDR_PREPARED) 
            waveOutUnprepareHeader(waveOut, current, sizeof(WAVEHDR));

        if(size < (int)(BLOCK_SIZE - current->dwUser)) 
        {
            memcpy(current->lpData + current->dwUser, data, size);
            current->dwUser += size;
            break;
        }

        remain = BLOCK_SIZE - current->dwUser;
        memcpy(current->lpData + current->dwUser, data, remain);
        size -= remain;
        data += remain;
        current->dwBufferLength = BLOCK_SIZE;
        waveOutPrepareHeader(waveOut, current, sizeof(WAVEHDR));

        waveOutWrite(waveOut, current, sizeof(WAVEHDR));

        EnterCriticalSection(&(globals->countGuard));
        globals->freeBlocks--;
        LeaveCriticalSection(&(globals->countGuard));

        /* wait for a block to become free */
        while(!globals->freeBlocks)
            Sleep(10);

        /*point to the next block*/
        globals->currentBlock++;
        globals->currentBlock %= BLOCK_COUNT;
        current = &(globals->blocks[globals->currentBlock]);
        current->dwUser = 0;
    }
}


/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:	David Overton 	
--
--		PROGRAMMER:	Mike Zobac
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:      The design of this function is from an windows audio tutorial by 
--                  David Overton.  
-------------------------------------------------------------------------------------------*/

/* CHANGE THIS FILE TO A SOCKET WHEN THE TIME COMES */


void playAudio(HANDLE hFile, PGLOBALS globals)
{
    DWORD bytesRead;

    /* try to open the default wave device  */
    if(waveOutOpen(&(globals->waveOut), WAVE_MAPPER, &(globals->wfx), (DWORD_PTR)waveOutProc, (DWORD_PTR)globals, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) 
    {
        fprintf(stderr, "unable to open wave mapper device\n");
        ExitProcess(1);
    }

    /* playback loop */
    while(1) 
    {

        if(!ReadFile(hFile, globals->buffer, BUFSIZE, &bytesRead, NULL))
            break;
        if(bytesRead == 0)
            break;

        if(bytesRead < sizeof(globals->buffer)) 
        {
            memset(globals->buffer + bytesRead, 0, BUFSIZE - bytesRead);
        }

        writeAudio(globals->waveOut, globals, globals->buffer, BUFSIZE);
    }

    /* wait for all blocks to complete */
    while(globals->freeBlocks < BLOCK_COUNT)
        Sleep(10);
}


/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:	David Overton 	
--
--		PROGRAMMER:	Mike Zobac
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:      The design of this function is from an windows audio tutorial by 
--                  David Overton.  
-------------------------------------------------------------------------------------------*/

WAVEHDR* allocateBlocks(int size, int count)
{
    unsigned char* buffer;
    int i;
    WAVEHDR* blocks;
    DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

    /* allocate memory for the entire set in one go */
    if((buffer = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalBufferSize)) == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        ExitProcess(1);
    }

    /* and set up the pointers to each bit */
    blocks = (WAVEHDR*)buffer;
    buffer += sizeof(WAVEHDR) * count;

    for(i = 0; i < count; i++)
    {
        blocks[i].dwBufferLength = size;
        blocks[i].lpData = (LPSTR)buffer;
        buffer += size;
    }
    return blocks;
}


/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:	David Overton	
--
--		PROGRAMMER: Mike Zobac	
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:		The design of this function is from an windows audio tutorial by 
--                  David Overton.  
-------------------------------------------------------------------------------------------*/

void freeBlocks(WAVEHDR* blockArray)
{
    /* and this is why allocateBlocks works the way it does */ 
    HeapFree(GetProcessHeap(), 0, blockArray);
}


/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:		
--
--		PROGRAMMER:	Mike Zobac
--		INTERFACE:	Mike Zobac	
--
--		RETURNS:		
--
--		NOTES:			
-------------------------------------------------------------------------------------------*/

void CALLBACK waveOutProc(HWAVEOUT waveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    /* pointer to free block counter */
    PGLOBALS globals = (PGLOBALS)dwInstance;

    /* Ignore all calls except when a block is finished playing */
    if(uMsg != WOM_DONE)
        return;
    EnterCriticalSection(&(globals->countGuard));
    (globals->freeBlocks)++;
    LeaveCriticalSection(&(globals->countGuard));
}


/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:	Mike Zobac	
--
--		PROGRAMMER:	Mike Zobac	
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:			
-------------------------------------------------------------------------------------------*/

void cleanUp(PGLOBALS globals)
{
    size_t  i;
    
    /* Unprepare the blocks before freeing the data buffer */
    for(i = 0; i < globals->freeBlocks; i++)
    {
        if(globals->blocks[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(globals->waveOut, &(globals->blocks)[i], sizeof(WAVEHDR));
    }
    
    DeleteCriticalSection(&(globals->countGuard));
    freeBlocks(globals->blocks);
    waveOutClose(globals->waveOut);
}



/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:   Mike Zobac		
--
--		PROGRAMMER:	Mike Zobac	
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:			
-------------------------------------------------------------------------------------------*/

void getHeaderData(PGLOBALS globals)
{
    /*after three way handshake, 1st multicast message
    will have header data.  read it into WAVEFORMATEX struct*/
    
    globals->wfx.nSamplesPerSec = 44100; /*sample rate*/
    globals->wfx.wBitsPerSample = 16; /*sample size*/
    globals->wfx.nChannels= 2;  /*channels*/
    globals->wfx.cbSize = 0;  /*size of extra info*/
    globals->wfx.wFormatTag = WAVE_FORMAT_PCM;
    globals->wfx.nBlockAlign = (globals->wfx.wBitsPerSample * globals->wfx.nChannels) >> 3;
    globals->wfx.nAvgBytesPerSec = globals->wfx.nBlockAlign * globals->wfx.nSamplesPerSec;
}

/*---------------------------------------------------------------------------------------
--		FUNCTION:		
--
--		DATE:			
--
--		DESIGNER:   Mike Zobac		
--
--		PROGRAMMER:	Mike Zobac	
--
--		INTERFACE:		
--
--		RETURNS:		
--
--		NOTES:			
-------------------------------------------------------------------------------------------*/

void initialize(PGLOBALS globals)
{
    /* initialize globals */ 
    globals->blocks = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
    globals->freeBlocks = BLOCK_COUNT;
    globals->currentBlock= 0;
    InitializeCriticalSection(&(globals->countGuard));
}