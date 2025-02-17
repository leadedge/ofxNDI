/*
=========================================================================

                        WinReceiverNDI.cpp

				Using ofxNDI (http://spout.zeal.co/)

   A Windows Desktop Application project generated by Visual Studio
   and adapted for NDI output (https://www.ndi.tv/)

   This is an example of using the ofxNDIreceive class independent of Openframeworks
   to receive to a pixel buffer and display the output using Windows bitmap functions.

   Functions used : 

	Receiver setup
		SetAudio
		SetFormat
	
	Receiver sender monitoring
		FindSenders
		GetSenderCount
		SenderSelected
		ReceiverCreated

	Receiver creation
		CreateReceiver

	Receive sender pixels
		ReceiveImage

	Hold render frame rate
		HoldFps

	Select sender
		GetSenderList
		GetSenderIndex
   

   Compare with the Spout DirectX Windows example.

=========================================================================

                 Copyright(C) 2024-2025 Lynn Jarvis.

   This program is free software : you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.If not, see < http://www.gnu.org/licenses/>.
========================================================================

*/
#include "framework.h"
#include "WinReceiverNDI.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                       // current instance
WCHAR szTitle[MAX_LOADSTRING]{};       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]{}; // the main window class name

ofxNDIreceive receiver;                // Receiver object
HWND g_hWnd = NULL;                    // Window handle
unsigned char *pixelBuffer = nullptr;  // Receiving pixel buffer
std::string g_senderName;              // full NDI sender name used by a receiver
int g_senderIndex = -1;                // index into the list of NDI senders
unsigned int g_SenderWidth = 0;        // Received sender width
unsigned int g_SenderHeight = 0;       // Received sender height
double g_SenderFps = 0.0;              // For fps display aver7aging
bool bShowInfo = true;                 // Show on-screen info
bool bInitialized = false;             // Receiver is initialized

// Statics for sender selection dialog
static int nSenders = 0;
static char sendername[256]{};
static std::vector<std::string> senderList; // Name list for selection

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool CheckSenders();
void ReleaseNDIreceiver();
void Render();
void ShowSenderInfo(HDC hdc); // Show sender information on screen
void DrawString(std::string str, HDC hdc, COLORREF col, int xpos, int ypos); // Draw text


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// Console window so printf works for debugging
	FILE* pCout;
	AllocConsole();
	freopen_s(&pCout, "CONOUT$", "w", stdout);
	// Disable console close button to prevent accidental shutdown
	HMENU hmenu = GetSystemMenu(GetConsoleWindow(), FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
	printf("WinReceiverNDI\n");


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN_NDI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	// =======================================
	receiver.SetAudio(false); // Set to receive no audio

	// =======================================
	// Set to prefer BGRA to match with windows bitmpap format (see WM_PAINT)
	receiver.SetFormat(NDIlib_recv_color_format_BGRX_BGRA);

    // Main message loop:
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Continue if a message is received
		{
			Render();
		}
	}

	if (pixelBuffer) delete[] pixelBuffer;

	// Release the receiver
	receiver.ReleaseReceiver();

    return (int) msg.wParam;
}


void Render()
{
	// =======================================
	// Receive a pixel buffer


	// Test for network change and presence of senders.
	// Create or re-create a receiver if the selected sender is changed.
	if(CheckSenders()) {

		unsigned int width = 0;
		unsigned int height = 0;

		// Receive from the NDI sender
		// Frame rate might be much less than the draw cycle
		// ReceiveImage succeeds if it finds a sender
		if (receiver.ReceiveImage(width, height)) {
			// Have the sender dimensions changed ?
			if (g_SenderWidth != width || g_SenderHeight != height) {
				// Update the sender name name in case the one in this position has changed
				g_senderName = receiver.GetSenderName(g_senderIndex);
				// Set the sender name or the old one will be used
				receiver.SetSenderName(g_senderName.c_str());
				g_SenderWidth  = width;
				g_SenderHeight = height;
				// Update the receiving pixel buffer
				if (pixelBuffer) free((void*)pixelBuffer);
				pixelBuffer = (unsigned char*)malloc(width*height * 4 * sizeof(unsigned char));
			}

			// Copy NDI pixels to the local pixel buffer
			// CopyImage uses SSE2 functions if the width is 16bit aligned
			ofxNDIutils::CopyImage((const unsigned char*)receiver.GetVideoData(),
				pixelBuffer, width, height, receiver.GetVideoStride());

			// Free NDI video frame data
			receiver.FreeVideoData();

			// Trigger a re-paint to draw the pixel buffer - see WM_PAINT
			InvalidateRect(g_hWnd, NULL, FALSE);

			// Update immediately
			UpdateWindow(g_hWnd);

		}
		else {
			// No senders or the sender closed
			// Take no action if the sender closed
			// Receiving will resume when it re-opens
		}

	}

	// =======================================
	// Control the render cycle rate

	// Hold a target frame rate - e.g. 60 or 30fps.
	// This is not necessary if the application already has
	// fps control. But in this example, rendering is done
	// during idle time and render rate can be extremely high
	ofxNDIutils::HoldFps(30);

} // end Render


// =======================================
// Test for network change
// Create receiver if nor initialized
// or a new sender has been selected
//
bool CheckSenders() {

	// Update the NDI sender list to find new senders
	// There is no delay if no new senders are found
	int nsenders = receiver.FindSenders();

	// Check the sender count
	if (nsenders > 0) {
		// Has the user changed the sender index ?
		if(receiver.GetSenderIndex() != g_senderIndex) {
			// Retain the last sender in case of network delay
			// Wait for the network to come back up or for the
			// user to select another sender when it does
			if (nsenders > 1) {
				// Release the current receiver.
				// A new one is then created from the selected sender index.
				receiver.ReleaseReceiver();
			}
		}

		// Update global variables
		g_senderName = receiver.GetSenderName();
		g_senderIndex = receiver.GetSenderIndex();

		// Create a new receiver if not already
		if (!receiver.ReceiverCreated()) {
			// A receiver is created from an index into a list of sender names.
			// The current user selected index is saved in the NDIreceiver class
			// and is used to create the receiver unless you specify a particular index.
			// The receiver is created with default preferred format BGRA
			bInitialized = receiver.CreateReceiver();
		}

	}

	return (nsenders > 0);

} // end CheckSenders


// Release receiver and resources
void ReleaseNDIreceiver()
{
	if (!bInitialized)
		return;

	// Release NDI receiver
	receiver.ReleaseReceiver();
	// Free the receiving buffer because the
	// receiving resolution might have changed
	if (pixelBuffer) free((void*)pixelBuffer);
	pixelBuffer = nullptr;
	g_SenderWidth = 0;
	g_SenderHeight = 0;
	bInitialized = false;

} // end ReleaseNDIreceiver


void ShowSenderInfo(HDC hdc)
{
	std::string str = receiver.GetSenderName();
	str += "   : ";
	str += std::to_string(receiver.GetSenderWidth()); str += "x";
	str += std::to_string(receiver.GetSenderHeight());
	// Show sender fps    
	str += "   fps ";
	// Average to stabilise fps display
	g_SenderFps = g_SenderFps*.85 + 0.15*receiver.GetSenderFps();
	// Round first or integer cast will truncate to the whole part
	str += std::to_string((int)(round(g_SenderFps)));
	// Draw text
	DrawString(str, hdc, 0x00FFFFFF, 20, 20);
	str = "Right click - select sender   :   Space - hide info";
	DrawString(str, hdc, 0x00FFFFFF, 20, 42);

} // end ShowSenderInfo


void DrawString(std::string str, HDC hdc, COLORREF col, int xpos, int ypos)
{
	HFONT hFont, hOldFont;
	hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	hOldFont = (HFONT)SelectObject(hdc, hFont);
	if (hOldFont)
	{
		// Text colour
		// Can be any colour (00 BB GG RR) hex
		// e.g. red = 0x000000FF
		COLORREF oldText = SetTextColor(hdc, col);
		// Transparent background
		SetBkMode(hdc, TRANSPARENT);
		// Display the text string
		TextOutA(hdc, xpos, ypos, str.c_str(), (int)str.length());
		SetTextColor(hdc, oldText);
		SelectObject(hdc, hOldFont);
	}
} // end DrawString


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN_NDI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground  = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN_NDI);
    wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_WIN_NDI));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Create window
   RECT rc = { 0, 0, 640, 360 }; // Desired client size
   AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE); // Allow for menu
   HWND hWnd = CreateWindowW(szWindowClass,
	   szTitle,
	   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
	   CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
	   nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // Centre the window on the desktop work area
   GetWindowRect(hWnd, &rc);
   RECT WorkArea;
   int WindowPosLeft = 0;
   int WindowPosTop = 0;
   SystemParametersInfo(SPI_GETWORKAREA, 0, (LPVOID)&WorkArea, 0);
   WindowPosLeft += ((WorkArea.right  - WorkArea.left) - (rc.right - rc.left)) / 2;
   WindowPosTop  += ((WorkArea.bottom - WorkArea.top)  - (rc.bottom - rc.top)) / 2;
   MoveWindow(hWnd, WindowPosLeft, WindowPosTop, (rc.right - rc.left), (rc.bottom - rc.top), false);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;
   
   return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_OPEN:
					// Sender selection dialog box 
					sendername[0] = 0; // Clear static name for dialog
					senderList = receiver.GetSenderList(); // Update the sender name list
					g_senderIndex = receiver.GetSenderIndex(); // and the sender index
					DialogBox(hInst, MAKEINTRESOURCE(IDD_SENDERBOX), hWnd, (DLGPROC)SenderProc);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;

    case WM_PAINT:
        {
			// Draw the received image
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT dr = { 0 };
			GetClientRect(hWnd, &dr);

			// Create a double buffer so that both image and text
			// can be drawn to a memory DC to avoid flicker.
			HDC hdcMem = CreateCompatibleDC(hdc);
			int ndcmem = SaveDC(hdcMem);
			HBITMAP hbmMem = CreateCompatibleBitmap(hdc, (dr.right-dr.left), (dr.bottom-dr.top));
			SelectObject(hdcMem, hbmMem);

			// No sender - draw default background
			if (!receiver.ReceiverConnected()) {
				HBRUSH backbrush = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
				FillRect(hdc, &dr, backbrush);
				DeleteObject(backbrush);
			}
			else if (pixelBuffer) {

				// The received sender format is BGRA and matches the Windows bitmap
				BITMAPINFO bmi;
				ZeroMemory(&bmi, sizeof(BITMAPINFO));
				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				bmi.bmiHeader.biSizeImage = (LONG)(g_SenderWidth * g_SenderHeight * 4); // Pixel buffer size
				bmi.bmiHeader.biWidth = (LONG)g_SenderWidth; // Width of buffer
				bmi.bmiHeader.biHeight = -(LONG)g_SenderHeight; // Height of buffer allowing for upside-downs bitmap
				bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = 32;
				bmi.bmiHeader.biCompression = BI_RGB;

				// Draw into the memory DC
				// For double buffer, setting the blit mode is necessary here
				SetStretchBltMode(hdcMem, STRETCH_DELETESCANS);

				// StretchDIBits adapts the pixel buffer received from the sender
				// to the window size. The sender can be resized or changed.
				StretchDIBits(hdcMem,
					0, 0, (dr.right - dr.left), (dr.bottom - dr.top), // destination rectangle 
					0, 0, g_SenderWidth, g_SenderHeight, // source rectangle 
					pixelBuffer,
					&bmi, DIB_RGB_COLORS, SRCCOPY);

				// Draw the sender information text unless no show by space bar
				if (bShowInfo)
					ShowSenderInfo(hdcMem);

				// Copy the double buffer to screen
				BitBlt(hdc, 0, 0, (dr.right-dr.left), (dr.bottom-dr.top), hdcMem, 0, 0, SRCCOPY);

				// Clean up
				RestoreDC(hdcMem, ndcmem);
				DeleteObject(hbmMem);
				DeleteDC(hdcMem);

			} // endif pixelBuffer
			EndPaint(hWnd, &ps);
        }
        break;

	
	case WM_RBUTTONDOWN:
		// RH click to select a sender
		{
			sendername[0] = 0; // Clear static name for dialog
			senderList = receiver.GetSenderList(); // Update the sender name list
			g_senderIndex = receiver.GetSenderIndex(); // and the sender index
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SENDERBOX), hWnd, (DLGPROC)SenderProc);
		}
		break;

	case WM_KEYUP:
		{
			// Space bar to show/hide on-screen text
			if (wParam == 0x20) {
				bShowInfo = !bShowInfo;
			}
		}
		break;
    
	case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);

    }
    return 0;
}

// Message handler for about box.
// adapted for this example.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
 	UNREFERENCED_PARAMETER(lParam);
	char tmp[MAX_PATH]{};
	char about[1024]{};
	LPDRAWITEMSTRUCT lpdis{};
	HWND hwnd = NULL;
	HCURSOR cursorHand = NULL;

    switch (message)
    {
    case WM_INITDIALOG:

		sprintf_s(about, 256, "              WinReceiverNDI");
		strcat_s(about, 1024, "\n\n\n\n");
		strcat_s(about, 1024, "     Windows NDI receiver example.\n");
		strcat_s(about, 1024, "      Receive to a pixel buffer using\n         the ofxNDIreceive class.");
		SetDlgItemTextA(hDlg, IDC_ABOUT_TEXT, (LPCSTR)about);
		
		//
		// Url hyperlink hand cursor
		//

		// Spout 
		cursorHand = LoadCursor(NULL, IDC_HAND);
		hwnd = GetDlgItem(hDlg, IDC_SPOUT_URL);
		SetClassLongPtrA(hwnd, GCLP_HCURSOR, (LONG_PTR)cursorHand);

		// NDI
		hwnd = GetDlgItem(hDlg, IDC_NDI_URL);
		SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)cursorHand);

        return (INT_PTR)TRUE;

	case WM_DRAWITEM:

		// The blue hyperlinks
		lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->itemID == -1) break;
		SetTextColor(lpdis->hDC, RGB(6, 69, 173));
		switch (lpdis->CtlID) {
			case IDC_SPOUT_URL:
				DrawTextA(lpdis->hDC, "https://spout.zeal.co", -1, &lpdis->rcItem, DT_LEFT);
				break;
			case IDC_NDI_URL:
				DrawTextA(lpdis->hDC, "https://www.ndi.tv", -1, &lpdis->rcItem, DT_LEFT);
				break;
			default:
				break;
		}
		break;

    case WM_COMMAND:

		if (LOWORD(wParam) == IDC_SPOUT_URL) {
			// Open the Spout website url
			sprintf_s(tmp, MAX_PATH, "http://spout.zeal.co");
			ShellExecuteA(hDlg, "open", tmp, NULL, NULL, SW_SHOWNORMAL);
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDC_NDI_URL) {
			// Open the NDI website url
			sprintf_s(tmp, MAX_PATH, "https://www.ndi.tv/");
			ShellExecuteA(hDlg, "open", tmp, NULL, NULL, SW_SHOWNORMAL);
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;
		}

        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// =======================================
// Message handler for selecting sender
INT_PTR  CALLBACK SenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam); // suppress warning

	static int activeindex = 0;

	switch (message) {

	case WM_INITDIALOG:
	{
		// Create a sender name list for the combo box
		HWND hwndList = GetDlgItem(hDlg, IDC_SENDERS);

		// Populate the combo box
		char name[256]{};
		if (senderList.size() > 0) {
			for (int i = 0; i < (int)senderList.size(); i++) {
				strcpy_s(name, 256, senderList[i].c_str());
				// Current sender for the initial combo box item
				if (i == g_senderIndex) {
					activeindex = i;
					strcpy_s(sendername, 256, name);
				}
				SendMessageA(hwndList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)name);
			}
			// Show the current sender as the initial item
			SendMessageA(hwndList, CB_SETCURSEL, (WPARAM)activeindex, (LPARAM)0);
		}
	}
	return TRUE;

	case WM_COMMAND:

		// Combo box selection
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			// Get the selected sender name
			const int index = (int)SendMessageA((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			SendMessageA((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)index, (LPARAM)sendername);
			activeindex = index;
		}
		// Drop through

		switch (LOWORD(wParam)) {

		case IDOK:
			// Selected sender
			if (sendername[0]) {
				// Reset the receiving sender index
				g_senderIndex = activeindex;
				// Set the new sender name for the receiver class
				receiver.SetSenderName(senderList[g_senderIndex]);
				// A new sender is detected by CheckSenders
				// and the receiver is re-created.
			}
			EndDialog(hDlg, 1);
			break;

		case IDCANCEL:
			// User pressed cancel.
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;

		default:
			return (INT_PTR)FALSE;
		}
	}

	return (INT_PTR)FALSE;
}

// That's all..
