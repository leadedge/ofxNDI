/*
	ofxNDIdialog.h

	Sender dialog support

	http://NDI.NewTek.com

	Copyright (C) 2016 Lynn Jarvis.

	http://www.spout.zeal.co

	=========================================================================
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================

	16-06-16	- create file

*/
#include <windows.h>
#include <string>
#include <vector>
#include "ofxNDIdialog.h"
#include "resource.h" // for sender selection dialog


ofxNDIdialog::ofxNDIdialog()
{
	bSenderSelected = false;
}

ofxNDIdialog::~ofxNDIdialog()
{
}


// Has the user used the sender selection dialog
bool ofxNDIdialog::SenderSelected()
{
	bool bSelected = bSenderSelected;
	bSenderSelected = false; // one off - the user has to select again

	return bSelected;
}


// Pop up a sender selection dialog
bool ofxNDIdialog::SelectNDIPanel(std::vector<std::string> Sendernames, int &index)
{
	// Copy the sender list to a static list for the dialog
	senderlist.clear();
	if(Sendernames.size() > 0) {
		for(int i = 0; i < (int)Sendernames.size(); i++) {
			senderlist.push_back(Sendernames.at(i));
		}
	}

	// Get the current executable instance for the dialog
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

	if(DialogBoxA(hInstance, MAKEINTRESOURCEA(IDD_SENDERBOX), NULL, (DLGPROC)NDIsenderDialog )) {
		// OK selected
		bSenderSelected = true; // set a flag for the application to test
		index = listindex; // return index variable
		senderlist.clear();
		return true;
	}

	// Cancel
	bSenderSelected = false;
	senderlist.clear();

	return false;

}


// The sender selection dialog
LRESULT CALLBACK NDIsenderDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = NULL;
	char name[256];

	switch (message) {

		case WM_INITDIALOG:

			hwndList = GetDlgItem(hDlg, IDC_SENDERS);
			for (unsigned int k = 0; k < senderlist.size(); k ++) {
				strcpy_s(name, sizeof(name),  senderlist.at(k).c_str());
				SendMessageA(hwndList, (UINT)LB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}

			// prevent other windows from hiding the dialog
			// and open the window wherever the user clicked
			int x, y, w, h;
			POINT p;
			RECT rect;
			GetWindowRect(hDlg, &rect);
			w = rect.right - rect.left; 
			h = rect.bottom - rect.top;
			if (GetCursorPos(&p)) {
				//cursor position now in p.x and p.y
				x = p.x - w/2;
				y = p.y - h/3;
			}
			else {
				x = rect.left;
				y = rect.top;
			}
			SetWindowPos(hDlg, HWND_TOPMOST, x, y, w, h, SWP_ASYNCWINDOWPOS | SWP_SHOWWINDOW);

			// Display an initial item in the selection field  
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)listindex, (LPARAM)0);
			SetFocus(hwndList); 

			return FALSE; // leave highlighted

		case WM_COMMAND:

			// List box selection
			if(HIWORD(wParam) == LBN_SELCHANGE)
				listindex = (int)SendMessage((HWND)lParam, (UINT) LB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
				
			switch(LOWORD(wParam)) {

				case IDOK :
					EndDialog(hDlg, TRUE);
					return FALSE;                                          

				case IDCANCEL :
					EndDialog(hDlg, FALSE);
					return FALSE;                                          

				default:
					return FALSE;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hDlg);
			break;

	}

	return 0;

}
