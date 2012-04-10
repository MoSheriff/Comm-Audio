#include "main.h"

HWND hDlg;

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{ 
    MSG msg;

    hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC) DlgProc);
    ShowWindow(hDlg, SW_SHOW);

    while (GetMessage (&msg, NULL, 0, 0))
    {
        if(!IsDialogMessage(hDlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}
