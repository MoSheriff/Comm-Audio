#include "NetworkingComponent.h"
#include "AudioOutput.h"
#include "main.h"

extern HWND hDlg;
std::list<std::string> getTitles(NetworkingComponent *nc);

DWORD WINAPI SaveFile(void *net)
{
    NetworkingComponent* nc = (NetworkingComponent *) net;
    WSABUF buffer;
    
    HANDLE hFile = CreateFile("C:\\Music.wav", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        MessageBox(hDlg, "Unable to create save file.", "Error", MB_ICONERROR);
        return 0;
    }

    DWORD bytesWritten;
    while(nc->receiveData(&buffer))
    {
        if (WriteFile(hFile, buffer.buf, buffer.len, &bytesWritten, NULL))
            MessageBox(hDlg, "Failed to write a chunk of file.", "Error", MB_ICONINFORMATION);
    }

    CloseHandle(hFile);
    return 0;
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static NetworkingComponent *nc;
    static AudioOutput *output;
    static std::list<std::string> titles;

    std::list<std::string>::iterator it;
    char buffer[4096];
    char data[4096];

    switch(message)
    {
    case WM_INITDIALOG:
        nc = new NetworkingComponent(NetworkingComponent::CLIENT);
        output = new AudioOutput;
        output->setNc(nc);
        return TRUE;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_BTNLISTEN:
            output->playAudio();
            break;

        case IDC_BTNCONNECT:
            GetDlgItemText(hDlg, IDC_TXTIP, buffer, sizeof(buffer));
            output->connect(buffer, NetworkingComponent::LISTENPORT);
            titles = getTitles(nc);

            for (it = titles.begin(); it != titles.end(); ++it)
                SendDlgItemMessage(hDlg, IDC_LSTSONGLIST, LB_ADDSTRING, 0, (LPARAM) it->c_str()); 
            break;

        case IDC_BTNSELECT:
        case IDC_BTNDOWNLOAD:
            int selected = SendDlgItemMessage(hDlg, IDC_LSTSONGLIST, LB_GETCURSEL, 0, 0);

            if (selected == LB_ERR)
            {
                MessageBox(hDlg, "You need to select a song first.", "Error", MB_ICONINFORMATION);
                return TRUE;
            }
            else
            {
                SendDlgItemMessage(hDlg, IDC_LSTSONGLIST, LB_GETTEXT, selected, (LPARAM) buffer);
            }

            if (LOWORD(wParam) == IDC_BTNSELECT)
                sprintf(data, "0:%s", buffer);
            else
                sprintf(data, "1:%s", buffer);

            nc->sendData(data, strlen(data));

            if (LOWORD(wParam) == IDC_BTNDOWNLOAD)
                CreateThread(NULL, 0, SaveFile, nc, 0, NULL);
            break;
        }
        return TRUE;

    case WM_CLOSE:
        PostQuitMessage(0);
        return TRUE;
    }

    return FALSE;
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