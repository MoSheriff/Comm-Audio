#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>
#include "resource.h"

#define LISTEN_BUTTON		101
#define LEAVE_BUTTON		102
#define SKIP_BUTTON	        104
#define REQUEST_BUTTON		105
#define UPDATE_BUTTON		106
#define CALL_BUTTON			107
#define ENDCALL_BUTTON	    108

#define SONG_LISTBOX		109
#define CLIENT_LISTBOX		110

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif
