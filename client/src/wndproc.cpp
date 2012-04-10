#include "NetworkingComponent.h"
#include "AudioOutput.h"
#include "main.h"
#include "MicLib.h"
#include <sstream>

extern HWND hDlg;
std::list<std::string> getTitles(NetworkingComponent *nc);

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static NetworkingComponent *nc;
    static AudioOutput *output;
    static MicLib *mic;
    static std::list<std::string> titles;

    std::list<std::string>::iterator it;
    std::vector<std::string>::iterator it1;
    std::string clients;
    std::vector<std::string> clientsList;
    char buffer[4096];
    char data[4096];
	int selected;
    WSABUF buf;

    switch(message)
    {
    case WM_INITDIALOG:
        nc = new NetworkingComponent(NetworkingComponent::CLIENT);
        nc->initialize();
        output = new AudioOutput;
        mic = new MicLib(nc);
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
            selected = SendDlgItemMessage(hDlg, IDC_LSTSONGLIST, LB_GETCURSEL, 0, 0);

            if (selected == LB_ERR)
                MessageBox(hDlg, "You need to select a song first.", "Error", MB_ICONINFORMATION);
            else
                SendDlgItemMessage(hDlg, IDC_LSTSONGLIST, LB_GETTEXT, selected, (LPARAM) buffer);

            sprintf(data, "0:%s", buffer);
            nc->sendData(data, strlen(data));
            break;

        case IDC_BTNCHAT:
            mic->record();
            break;

        case IDC_BTNSTOP:
            mic->stop();
            break;

        case IDC_BTNUPDATE:
            GetDlgItemText(hDlg, IDC_TXTIP, buffer, sizeof(buffer));
            nc->connectToServer(std::string(buffer), NetworkingComponent::LISTENPORT);
            nc->sendData("2:update", 8);
            while(nc->receiveData(&buf))
            {
                buf.buf[buf.len] = '\0';
                clients += std::string(buf.buf);
            }
            clientsList = split(clients, ',');
            for (it1 = clientsList.begin(); it1 != clientsList.end(); ++it1)
                SendDlgItemMessage(hDlg, IDC_LSTSONGLIST, LB_ADDSTRING, 0, (LPARAM) it1->c_str());
            break;

        case IDC_BTNSKIP:
            sprintf(data, "3:%s", "skip");
            nc->sendData(data, strlen(data));
            break;

        case IDC_BTNQUIT:
            output->quit();
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