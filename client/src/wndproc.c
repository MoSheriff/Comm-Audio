#include "main.h"

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND playButton, pauseButton, rewindButton, fastforwardButton, songListBox, clientListBox,
		requestButton, updateButton, callButton, endcallButton;
	HDC hdc;
	PAINTSTRUCT ps;

	int cxClient, cyClient;

     switch (message)
     {
	 case WM_SIZE:
		 cxClient = LOWORD(lParam);
		 cyClient = HIWORD(lParam);
		 break;

	 case WM_CREATE:
		 rewindButton = CreateWindow("button", "<<",  WS_CHILD|WS_VISIBLE, 20, 480, 40, 40, hwnd, (HMENU)PLAY_BUTTON, NULL, NULL);
		 playButton = CreateWindow("button", "|>", WS_CHILD|WS_VISIBLE, 65, 480, 40, 40, hwnd, (HMENU)PLAY_BUTTON, NULL, NULL);
		 pauseButton = CreateWindow("button", "||",  WS_CHILD|WS_VISIBLE, 110, 480, 40, 40, hwnd, (HMENU)PLAY_BUTTON, NULL, NULL);
		 fastforwardButton = CreateWindow("button", ">>",  WS_CHILD|WS_VISIBLE, 155, 480, 40, 40, hwnd, (HMENU)PLAY_BUTTON, NULL, NULL);

		 requestButton = CreateWindow("button", "Request",  WS_CHILD|WS_VISIBLE, 610, 205, 80, 20, hwnd, (HMENU)REQUEST_BUTTON, NULL, NULL);
		 updateButton = CreateWindow("button", "Update",  WS_CHILD|WS_VISIBLE, 600, 435, 100, 20, hwnd, (HMENU)UPDATE_BUTTON, NULL, NULL);
		 callButton = CreateWindow("button", "Call",  WS_CHILD|WS_VISIBLE, 560, 460, 80, 20, hwnd, (HMENU)CALL_BUTTON, NULL, NULL);
		 endcallButton = CreateWindow("button", "End Call",  WS_CHILD|WS_VISIBLE, 660, 460, 80, 20, hwnd, (HMENU)ENDCALL_BUTTON, NULL, NULL);

		 songListBox = CreateWindow("listbox", NULL, WS_CHILD | WS_VISIBLE |WS_BORDER| LBS_NOTIFY, 530, 20, 240, 180, hwnd,(HMENU)SONG_LISTBOX, NULL, NULL);
		 clientListBox = CreateWindow("listbox", NULL, WS_CHILD | WS_VISIBLE |WS_BORDER| LBS_NOTIFY, 530, 250, 240, 180, hwnd,(HMENU)CLIENT_LISTBOX, NULL, NULL);
		 break;

	 case WM_PAINT:
		 hdc = BeginPaint(hwnd, &ps);

		 EndPaint(hwnd, &ps);
		 break;

	 case WM_COMMAND:

		 switch(LOWORD(wParam))
		 {
		 case ID_SERVER_CONNECT:
			 break;
		 case ID_SERVER_DISCONNECT:
			 break;
		 case PLAY_BUTTON:
			 break;
		 case PAUSE_BUTTON:
			 break;
		 case REWIND_BUTTON:
			 break;
		 case FASTFORWARD_BUTTON:
			 break;
		 case REQUEST_BUTTON:
			 break;
		 case UPDATE_BUTTON:
			 SendDlgItemMessage(hwnd, CLIENT_LISTBOX, LB_ADDSTRING, 0, (LPARAM)"test"); //listbox test
			 break;
		 case CALL_BUTTON:
			 break;
		 case ENDCALL_BUTTON:
			 break;
		 }
		 break;

	 case WM_DESTROY:
		 PostQuitMessage (0) ;
		 break;
	 }
	 return DefWindowProc (hwnd, message, wParam, lParam) ;
}