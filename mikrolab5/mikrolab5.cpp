// mikrolab5.cpp : Defines the entry point for the application.
//
#define _CRT_SECURE_NO_DEPRECATE
#include "stdafx.h"
// dodatkowy naglowek
#include "mikrolab5.h"


#define MAX_LOADSTRING 100
// dodatkowe zmienne
TOpcje WybraneOpcje;

void* Mem = 0;
int TextLength = 0;
// Global Variables:
HINSTANCE hInst;	// current instance
HWND hWnd;    // handle do okna
HWND hwndEdit, hwndTV;		
wchar_t *edittext ; // zawartosc pola edit
int EditID = 0;// pole edit na glownej formatce
WCHAR NazwaPliku [MAX_PATH+1];				// nazwa pliku typu WCHAR *

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


	//wcscpy(WybraneOpcje.PodstawowaSciezka,L"c:\\asm51");	

	wcscpy(WybraneOpcje.Raport, L"c:\\raport.txt");

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
	wcex.hbrBackground	= (HBRUSH)(11);
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

	TV_INSERTSTRUCT TV_insert;
	TV_ITEM    item;
	// dla dodawania folderow;
	wchar_t * filename, *path;
	int i,j;
	BROWSEINFO browseinfo;
	LPCITEMIDLIST pidlRoot;
	OPENFILENAMEW savefilename;
	
	
	
/*	TV_insert.hParent = 0;
	TV_insert.hInsertAfter = TVI_FIRST;
	TV_insert.item = item;

*/

	switch (message)
	{
	// obsuga zmiany rozmiaru (trzeba modyfikowac wielkosc pola edit)
	case WM_SIZE:
		RECT rect;
		tempW = tempH= int(lParam);
		/*tempH = tempW;
		tempH >>= 16;
		tempW &= 0xffff;*/
		GetClientRect(hWnd,&rect);
		
		MoveWindow(hwndEdit,2,2*(rect.bottom-rect.top)/3,LOWORD(tempW)-4,HIWORD(tempH)/3,TRUE);
		MoveWindow(hwndTV,2,0,LOWORD(tempW)-4,2*HIWORD(tempH)/3,TRUE);
		break;
	// wykonywane 1 raz - tworzenie pola edit na ca³ym okienku
	case WM_CREATE:

		GetClientRect(hWnd,&rect);
		InitCommonControls(); 
		hwndEdit = CreateWindowEx(NULL,L"edit",NULL, 
			WS_VISIBLE | WS_CHILD | WS_VSCROLL| ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			0,0,0,0,hWnd,0, hInst,NULL);

		
		hwndTV = CreateWindowEx(0,L"SysTreeView32",0,WS_VISIBLE | WS_CHILD | WS_BORDER |TVS_HASLINES, 
			0, 0, 200,200,hWnd, (HMENU)5, hInst, NULL); 

		SendMessage(hwndTV,TVM_INSERTITEM,0,(LPARAM)&TV_insert);
		//InitTreeViewImageLists(hwndTV);
		Edit_Enable(hwndEdit,false);
		
		SetFocus (hwndEdit);
		edittext = new wchar_t [MAX_PATH*MAX_PATH];
		wcscpy(edittext,L"Chosen folders:\r\n");
		SendMessageW(hwndEdit,WM_SETTEXT,NULL,(LPARAM)edittext);
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

		// nowe szukanie
		case IDM_FILE_NEWSEARCH:
			TerminateThread(ThreadMD5, 0);
			TerminateThread(ThreadSzukaj,0);
			kasuj_liste_kontenerow();
			wcscpy(edittext,L"Chosen folders:\r\n");
			SendMessageW(hwndEdit,WM_SETTEXT,NULL,(LPARAM)edittext);


			/*// inicjalizacja struktury do otwierania pliku
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
			}*/
		break; // IDM_FILE_NEWSEARCH

		case ID_OPTIONS_FOLDERLIST:
			// dodawanie sciezek do pola edit
			filename = new wchar_t [MAX_PATH];  
			path = new wchar_t [MAX_PATH];  
			browseinfo.hwndOwner=hWnd;
			browseinfo.iImage = 0;
			browseinfo.lpszTitle = L"Choose a folder";
			browseinfo.pszDisplayName = filename;
			browseinfo.ulFlags = 9;
			browseinfo.lParam = 0;
			browseinfo.lpfn = 0;
			browseinfo.pidlRoot = 0;
			pidlRoot = SHBrowseForFolder(&browseinfo);
			SHGetPathFromIDList(pidlRoot, path);
			if (!*path == (wchar_t )0) {
				wcscat(edittext,path);
				wcscat(edittext,L"\r\n");
				SendMessageW(hwndEdit,WM_SETTEXT,NULL,(LPARAM)edittext);
			}
			delete [] filename;
			delete [] path;

		break;

	
		case ID_FILE_COUNTMD5:
			generuj_raport(&WybraneOpcje);
			kasuj_liste_kontenerow();


		break;

		case ID_OPTIONS_RAPORTFILE:
			memset(&savefilename,0,sizeof(OPENFILENAME));
			savefilename.lStructSize = sizeof(OPENFILENAME);
			savefilename.hwndOwner = hWnd;
			savefilename.hInstance = hInst;
			savefilename.lpstrFilter = L"Txt Files\0*.txt\0";
			savefilename.nMaxFile = MAX_PATH;
			savefilename.lpstrFile = WybraneOpcje.Raport;
			savefilename.lpstrTitle = L"Choose one file";
			savefilename.Flags = OFN_EXPLORER || OFN_LONGNAMES;
			
			GetSaveFileName(&savefilename);


		break;

		case ID_FILE_FINDDUPLICATES:
		
//			char MD5[16];
//			CountMD5(L"c:\\asm51\\film.avi",MD5);
//			char * sumakontrolna;
//			sumakontrolna = new char [33];
//			sumakontrolna[32] = 0;
//			MD5ToAStr(sumakontrolna,MD5);
//			MessageBoxA(NULL,sumakontrolna,"tekst",0);
//			delete [] sumakontrolna;


			kasuj_liste_kontenerow();
			i = 17;  // poczatek folderow
			if (edittext[i] == wchar_t (0)) {
				MessageBox(hWnd,L"Add at least 1 folder to search",L"Error",0);
				break;


			}
			while (edittext[i] != wchar_t (0)) {
				j = 0;
				while ((edittext[i] != wchar_t(13)) && (edittext[i]!= wchar_t (0)) )  {//enter
					WybraneOpcje.PodstawowaSciezka[j] = edittext[i];
					i++;
					j++;
				}
				// sciezki typu c:\ koncza sie na \ - trzeba usunac
				if (j && WybraneOpcje.PodstawowaSciezka[j-1] == wchar_t (92))
					j--;
				WybraneOpcje.PodstawowaSciezka[j] = wchar_t (0);
				i++;
				i++;
				start(WybraneOpcje);

		}
		 
			
	
			
			int wynik;
		break; //ID_FILE_COUNTMD5


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
		/*if (Mem!= 0)
			CloseMapFile();*/
		kasuj_liste_kontenerow();
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




