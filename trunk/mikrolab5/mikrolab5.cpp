// mikrolab5.cpp : Defines the entry point for the application.
//
#define _CRT_SECURE_NO_DEPRECATE
#include "stdafx.h"
// dodatkowy naglowek
#include "mikrolab5.h"


#define MAX_LOADSTRING 100
// dodatkowe zmienne
#define MAX_PATH_LENGTH 256
void* Mem = 0;
int TextLength = 0;
// Global Variables:
HINSTANCE hInst;								// current instance
HWND hwndEdit;									// pole edit na glownej formatce
int EditID = 0;
WCHAR NazwaPliku [MAX_PATH_LENGTH];				// nazwa pliku typu WCHAR *

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);




int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MIKROLAB5, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MIKROLAB5));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MIKROLAB5));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MIKROLAB5);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int tempH, tempW;

	switch (message)
	{
	// obsuga zmiany rozmiaru (trzeba modyfikowac wielkosc pola edit)
	case WM_SIZE:
		tempW = int(lParam);
		tempH = tempW;
		tempH >>= 16;
		tempW &= 0xffff;
		MoveWindow(hwndEdit,0,0,tempW,tempH,TRUE);
		break;
	// wykonywane 1 raz - tworzenie pola edit na ca³ym okienku
	case WM_CREATE:
		hwndEdit = CreateWindowEx(NULL,L"edit",NULL, 
			WS_VISIBLE | WS_CHILD | WS_VSCROLL| ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			0,0,0,0,hWnd,0, hInst,NULL);
        SetFocus (hwndEdit);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:

			DestroyWindow(hWnd);
			break;

		// otwieranie pliku
		case IDM_FILE_OPEN:
			// inicjalizacja struktury do otwierania pliku
			OPENFILENAME opfn;
			ZeroMemory(&opfn,sizeof(opfn));
			opfn.lStructSize = sizeof(opfn);
			opfn.hwndOwner = hWnd;
			opfn.hInstance = hInst;
			opfn.lpstrFile = NazwaPliku;
			opfn.nMaxFile = sizeof(NazwaPliku);
			opfn.lpstrTitle = L"Open a txt file";
			opfn.Flags = OFN_EXPLORER |OFN_FILEMUSTEXIST | OFN_LONGNAMES;
			opfn.lpstrFilter = L"Text files [*.txt]\0*.txt\0";
			// okienko dialogowe otwierania pliku
			GetOpenFileName(&opfn);
			// pobrano nazwe
			if (opfn.lpstrFile[0] != 0) {
				// zamknij otwarty plik 
				if (Mem){
					CloseMapFile();
					Mem = 0;
				}

				WCHAR *komunikat = 0;
				//odczytaj caly plik tekstowy, i zwroc wskaznik na pamiec, gdzie sie znajduje oraz dlugosc komunikatu
				Mem = ReadTxtFile(opfn.lpstrFile, &TextLength);
				//TextLength+1 dla zera
				komunikat = new WCHAR [TextLength+1];
				// konwersja zawartosci pliku na wchar_t
				MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(char*)Mem, TextLength, komunikat, TextLength);
				komunikat[TextLength] = (WCHAR) 0;
				// mozna tez bez konwersji
				//SendMessageA(hwndEdit,WM_SETTEXT,NULL,(LPARAM)Mem);
				// wyslanie do komponentu
				SendMessageW(hwndEdit,WM_SETTEXT,NULL,(LPARAM)komunikat);
				delete [] komunikat;
			}
			break;

		case ID_FILE_MD5:
			/*TKontener *k;
			TElement *e;
			for (int j = 0; j < 500000; j++){
				k = new TKontener ;
				e = new TElement ;


			}
*/


			TOpcje opcje;
			wcscpy(opcje.PodstawowaSciezka,L"c:\\asm51");
			wcscpy(opcje.Raport, L"c:\\raport.txt");
			start(opcje);
			//znajdz();
			
			char MD5[16];
			int wynik;
//			wynik = CountMD5(L"c:\\test.pdf", MD5);
			int t1, t2, t3;
			//t1 = clock();
			//wynik = CountMD5("c:\\film.avi", MD5);
			//t2 = clock();
			//t3 = t2 - t1;

			char * sumakontrolna;
			sumakontrolna = new char [33];
			sumakontrolna[32] = 0;
			MD5ToStr(sumakontrolna,MD5);
			MessageBoxA(NULL,sumakontrolna,"tekst",0);


			delete [] sumakontrolna;


			break;


		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

    case WM_SETFOCUS: 
            SetFocus(hwndEdit); 
            break; 
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		if (Mem!= 0)
			CloseMapFile();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



void MD5ToStr(char * output, char * input){
	int i;
	for (i = 0; i < 16; i++ ) {
		output[2*i] = ((input[i]>>4) & 0x0F) < 10?  ((input[i]>>4) & 0x0F)+ 48:((input[i]>>4) & 0x0F) + 55; 
		output[2*i+1] = (input[i] & 0x0F) < 10?  (input[i] & 0x0F)+ 48:(input[i] & 0x0F) + 55; 
		
	}
}
