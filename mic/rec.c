#include "rec.h"

void PrintWaveErrorMsg(DWORD err, TCHAR * str)
{
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


DWORD WINAPI waveInProc(LPVOID arg)
{
	MSG		msg;

	while (GetMessage(&msg, 0, 0, 0) == 1)
	{
		switch (msg.message)
		{
			/* A buffer has been filled by the driver */
			case MM_WIM_DATA:
			{
				if (((WAVEHDR *)msg.lParam)->dwBytesRecorded)
				{
					if (!WriteFile(WaveFileHandle, ((WAVEHDR *)msg.lParam)->lpData, ((WAVEHDR *)msg.lParam)->dwBytesRecorded, &msg.time, 0) ||
						msg.time != ((WAVEHDR *)msg.lParam)->dwBytesRecorded)
					{
					}
				}

				if (InRecord)
				{
					waveInAddBuffer(WaveInHandle, (WAVEHDR *)msg.lParam, sizeof(WAVEHDR));
				}
				else
				{
					++DoneAll;
				}
                continue;
			}

			case MM_WIM_OPEN:
			{
				DoneAll = 0;
                continue;
			}

			case MM_WIM_CLOSE:
			{
				break;
			}
		}
	}

	return(0);
}


void set_mute(MIXERLINE *mixerLine, DWORD val, BOOL errmsg)
{
	MIXERCONTROL					mixerControlArray;
	MIXERLINECONTROLS				mixerLineControls;
	MIXERCONTROLDETAILS_UNSIGNED	value[2];
	MIXERCONTROLDETAILS				mixerControlDetails;
	MMRESULT						err;

	mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
	mixerLineControls.dwLineID = mixerLine->dwLineID;
	mixerLineControls.cControls = 1;
	mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
	mixerLineControls.pamxctrl = &mixerControlArray;
	mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);

	if ((err = mixerGetLineControls((HMIXEROBJ)MixerHandle, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE)))
	{
		/* An error */
		if (errmsg == ERRMSG_PRINT) printf("%s has no mute control!\n", mixerLine->szName);
	}
	else
	{
		mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
		mixerControlDetails.dwControlID = mixerControlArray.dwControlID;
		mixerControlDetails.cChannels = mixerLine->cChannels;

		if (mixerControlDetails.cChannels > 2) mixerControlDetails.cChannels = 2;
		if (mixerControlArray.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) mixerControlDetails.cChannels = 1;

		mixerControlDetails.cMultipleItems = 0;
		mixerControlDetails.paDetails = &value[0];
		mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);

		value[0].dwValue = value[1].dwValue = val;

		if ((err = mixerSetControlDetails((HMIXEROBJ)MixerHandle, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE)))
		{
			printf("Error #%d setting mute for %s!\n", err, mixerLine->szName);
		}
	}
}
