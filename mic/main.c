#include "rec.h"


int main(int argc , char ** argv)
{
	MMRESULT						err;
	WAVEFORMATEX					waveFormat;
	MIXERLINE						mixerLine;
	HANDLE							waveInThread;
	unsigned long					n, numSrc;

	waveInThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)waveInProc, 0, 0, &err);
	if (!waveInThread)
	{
		printf("Can't create WAVE recording thread! -- %08X\n", GetLastError());
		return(-1);
	}
	CloseHandle(waveInThread);

	ZeroMemory(&WaveHeader[0], sizeof(WAVEHDR) * 2);

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 2;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample/8);
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	err = waveInOpen(&WaveInHandle, WAVE_MAPPER, &waveFormat, (DWORD)err, 0, CALLBACK_THREAD);
	if (err)
	{
		PrintWaveErrorMsg(err, "Can't open WAVE In Device!");
		return(-2);
	}
	else
	{
		err = mixerOpen(&MixerHandle, (DWORD)WaveInHandle, 0, 0, MIXER_OBJECTF_HWAVEIN);
		if (err)
		{
			printf("Device does not have mixer support! -- %08X\n", err);
		}

		else
		{
			mixerLine.cbStruct = sizeof(MIXERLINE);
			mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
			err = mixerGetLineInfo((HMIXEROBJ)MixerHandle, &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE);
			if (err)
			{
				printf("Device does not have a WAVE recording control! -- %08X\n", err);
				goto record;
			}

			numSrc = mixerLine.cConnections;

			mixerLine.dwSource = mixerLine.dwDestination;
			set_mute(&mixerLine, 1, ERRMSG_NONE);

			/* Make sure that there is at least one WAVEIN source line available. If not, then just go with the
			 * WAVEIN master volume and mute above
			 */
			if (!numSrc)
			{
				printf("ERROR: There are no WAVE inputs to adjust!\n");
				goto record;
			}

remute:
			printf("The device has the following WAVE inputs. Choose which one to mute,\nor press ENTER to start recording.\n\n");

			/* Print the names of the source lines for the MIXERLINE_COMPONENTTYPE_DST_WAVEIN destination. But,
			 * omit any MIDI In
			 */
			for (n = 0; n < numSrc; n++)
			{
				mixerLine.cbStruct = sizeof(MIXERLINE);
				mixerLine.dwSource = n;

				if (!(err = mixerGetLineInfo((HMIXEROBJ)MixerHandle, &mixerLine, MIXER_GETLINEINFOF_SOURCE)))
				{
					if (mixerLine.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER)
						printf("\t#%lu: %s\n", n, mixerLine.szName);
				}
			}

			/* Get his choice of source line to mute, or ENTER to continue with recording */
			{
			TCHAR	buffer[3];

			fgets(&buffer[0], 3, stdin);

			if (buffer[0] >= '1' && buffer[0] <= '9')
			{
				buffer[0] -= '1';
				if ((unsigned long)buffer[0] < numSrc)
				{
					mixerLine.cbStruct = sizeof(MIXERLINE);
					mixerLine.dwSource = buffer[0];
					mixerGetLineInfo((HMIXEROBJ)MixerHandle, &mixerLine, MIXER_GETLINEINFOF_SOURCE);

					set_mute(&mixerLine, 0, ERRMSG_PRINT);
				}

				/* Go back to check for more muted sources */
				goto remute;
			}
			}

record:		mixerClose(MixerHandle);
		}

		WaveFileHandle = CreateFile("test.snd", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (WaveFileHandle == INVALID_HANDLE_VALUE)
		{
			printf("Can't create test.snd disk file!\n");
		}
		else
		{
			WaveHeader[1].dwBufferLength = WaveHeader[0].dwBufferLength = waveFormat.nAvgBytesPerSec << 1;
			if (!(WaveHeader[0].lpData = (char *)VirtualAlloc(0, WaveHeader[0].dwBufferLength * 2, MEM_COMMIT, PAGE_READWRITE)))
			{
				printf("ERROR: Can't allocate memory for WAVE buffer!\n");
			}
			else
			{
				WaveHeader[1].lpData = WaveHeader[0].lpData + WaveHeader[0].dwBufferLength;

				if ((err = waveInPrepareHeader(WaveInHandle, &WaveHeader[0], sizeof(WAVEHDR))))
				{
					printf("Error preparing WAVEHDR 1! -- %08X\n", err);
				}
				else
				{
					if ((err = waveInPrepareHeader(WaveInHandle, &WaveHeader[1], sizeof(WAVEHDR))))
					{
						printf("Error preparing WAVEHDR 2! -- %08X\n", err);
					}
					else
					{
						/* Queue first WAVEHDR (recording hasn't started yet) */
						if ((err = waveInAddBuffer(WaveInHandle, &WaveHeader[0], sizeof(WAVEHDR))))
						{
							printf("Error queueing WAVEHDR 1! -- %08X\n", err);
						}
						else
						{
							if ((err = waveInAddBuffer(WaveInHandle, &WaveHeader[1], sizeof(WAVEHDR))))
							{
								printf("Error queueing WAVEHDR 2! -- %08X\n", err);
								DoneAll = 1;
								goto abort;
							}
							else
							{
								//record
								InRecord = TRUE;
								if ((err = waveInStart(WaveInHandle)))
								{
									printf("Error starting record! -- %08X\n", err);
								}
								else
								{
									printf("Recording has started. Press ENTER key to stop recording...\n");
									getchar();
								}

								/* Tell our recording thread not to queue any WAVEHDRs it gets back via MM_WIM_DONE */
abort:							InRecord = FALSE;
								
								waveInReset(WaveInHandle);

								while (DoneAll < 2) Sleep(100);
							}
						}

						if ((err = waveInPrepareHeader(WaveInHandle, &WaveHeader[1], sizeof(WAVEHDR))))
						{
							printf("Error unpreparing WAVEHDR 2! -- %08X\n", err);
						}
					}

					if ((err = waveInPrepareHeader(WaveInHandle, &WaveHeader[0], sizeof(WAVEHDR))))
					{
						printf("Error unpreparing WAVEHDR 1! -- %08X\n", err);
					}
				}
			}
		}
	}

	/* Close the WAVE In device */
	do
	{
		err = waveInClose(WaveInHandle);
		if (err) PrintWaveErrorMsg(err, "Can't close WAVE In Device!");
	} while (err);

	/* Close the disk file if it opened */
	if (WaveFileHandle != INVALID_HANDLE_VALUE) CloseHandle(WaveFileHandle);

	/* Free any memory allocated for our buffers */
	if (WaveHeader[0].lpData) VirtualFree(WaveHeader[0].lpData, 0, MEM_RELEASE);

	return(0);
}