#include "NetworkingComponent.h"
#include "AudioOutput.h"
#include "main.h"

std::list<std::string> getTitles(NetworkingComponent *nc);

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND listenButton, quitButton, skipButton, songListBox, clientListBox,
		requestButton, updateButton, callButton, endcallButton;
	
    static NetworkingComponent *nc;    
    static AudioOutput         *output;
    static char                 *hostName;
    HDC hdc;
	PAINTSTRUCT ps;
    static std::list<std::string> titles;
    std::list<std::string>::iterator it;
	int i, j, cxClient, cyClient;
    WSABUF  buffer;
	char* sendBuf;

     switch (message)
     {
	 case WM_SIZE:
		 cxClient = LOWORD(lParam);
		 cyClient = HIWORD(lParam);
		 break;

	 case WM_CREATE:
		 
         nc = new NetworkingComponent(NetworkingComponent::CLIENT);
		 buffer.buf = (char*)malloc(1024);
		 //ZeroMemory(buffer.buf, 1024);
		 memset(buffer.buf,'\0', 1024);

         output = new AudioOutput();
         output->setNc(nc);

         listenButton = CreateWindow("button", "Listen", WS_CHILD|WS_VISIBLE, 65, 480, 40, 40, hwnd, (HMENU)LISTEN_BUTTON, NULL, NULL);
		 quitButton = CreateWindow("button", "Silence",  WS_CHILD|WS_VISIBLE, 110, 480, 40, 40, hwnd, (HMENU)LISTEN_BUTTON, NULL, NULL);
		 skipButton = CreateWindow("button", "Skip",  WS_CHILD|WS_VISIBLE, 155, 480, 40, 40, hwnd, (HMENU)LISTEN_BUTTON, NULL, NULL);

		 requestButton = CreateWindow("button", "Request",  WS_CHILD|WS_VISIBLE, 100, 445, 80, 20, hwnd, (HMENU)REQUEST_BUTTON, NULL, NULL);
		 updateButton = CreateWindow("button", "Update",  WS_CHILD|WS_VISIBLE, 600, 435, 100, 20, hwnd, (HMENU)UPDATE_BUTTON, NULL, NULL);
		 callButton = CreateWindow("button", "Call",  WS_CHILD|WS_VISIBLE, 560, 460, 80, 20, hwnd, (HMENU)CALL_BUTTON, NULL, NULL);
		 endcallButton = CreateWindow("button", "End Call",  WS_CHILD|WS_VISIBLE, 660, 460, 80, 20, hwnd, (HMENU)ENDCALL_BUTTON, NULL, NULL);

		 songListBox = CreateWindow("listbox", NULL, WS_CHILD | WS_VISIBLE |WS_BORDER| LBS_NOTIFY, 20, 20, 240, 420, hwnd,(HMENU)SONG_LISTBOX, NULL, NULL);
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
		 
         case LISTEN_BUTTON:

             output->playAudio();

			 break;
		 
         case LEAVE_BUTTON:
			 break;
		 
         case SKIP_BUTTON:
			 break;
		 
         case REQUEST_BUTTON:
             
             i = SendMessage(hwnd, LB_GETCURSEL, NULL, NULL);
             j = SendMessage(hwnd, LB_GETITEMDATA, (WPARAM)i, NULL);

             if(j == LB_ERR)
                 break;

             sendBuf = "Shine On You Crazy Diamond\0";

             nc->sendData(sendBuf, strlen(sendBuf));
             
             break;
		 
         case UPDATE_BUTTON:
			 SendDlgItemMessage(hwnd, CLIENT_LISTBOX, LB_ADDSTRING, 0, (LPARAM)"test"); //listbox test
			 break;
		 
         case CALL_BUTTON:
             output->connect("127.0.0.1", NetworkingComponent::LISTENPORT);

             // use the return value of this to display the available titles
             titles = getTitles(nc);

             for (it = titles.begin(); it != titles.end(); ++it)
             {
				 //*it += '\0';
                 SendDlgItemMessage(hwnd, SONG_LISTBOX, LB_ADDSTRING, 0, (LPARAM)it->c_str() /*(LPARAM)"Poop"*/); //listbox test
             }
			 break;
		 
         case ENDCALL_BUTTON:
			 break;
		 }
		 break;

	 case WM_DESTROY:
         delete output;         
         PostQuitMessage (0) ;
		 break;
	 }
	 return DefWindowProc (hwnd, message, wParam, lParam) ;
}


std::list<std::string> getTitles(NetworkingComponent *nc)
{
    DWORD                   bytesRead;
    std::list<std::string>  titles;
    std::string             temp;
    WSABUF                  buffer;

    bytesRead = nc->receiveData(&buffer);
	buffer.buf[bytesRead] = '\0';

    for(int i = 0; i < buffer.len+1; i++)
    {
        if(buffer.buf[i] != ',' && buffer.buf[i] != '\0')
        {
            temp += buffer.buf[i];
			continue;
        }
        titles.push_back(temp);
        temp.resize(0);
    }

    return titles;
}