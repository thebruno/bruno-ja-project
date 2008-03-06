#include "stdafx.h"

TKontener * glowa_kontener, *ogon_kontener ;
HANDLE ThreadSzukaj, ThreadMD5, Semafor;

TElement *zadania [256];
// ilosc wszystkich zadan, nr_zadania do przetworzenia jest <=ilosci_zadan
int ilosc_zadan = 0, nr_zadania = 0, koniec = 0;

const wchar_t* folder = L"d:\\sem6";




// sciezka bez gwiazdki!!
int search (wchar_t* biezacykat, HANDLE hFile) {
	// do sygnalizowania co liczyc
	TElement * element = 0;
	TKontener * kontener = 0;
	WIN32_FIND_DATA finddata;
	wchar_t * sciezka = new wchar_t [MAX_PATH];
	unsigned int dlugosc;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD blad = 0;
	unsigned long int written = 0, rozmiar;
   
   // Check that the input path plus 2 is not longer than MAX_PATH.
	dlugosc = (unsigned int) wcsnlen(biezacykat, MAX_PATH);

	if (dlugosc > (MAX_PATH - 2)) {
		// za dluga sciezka
		return (-1);
   }
	wcsncpy(sciezka, biezacykat, MAX_PATH);
	wcscat(sciezka, L"\\*");
	
   // Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.

	
	hFind = FindFirstFile(sciezka, &finddata);
	if (INVALID_HANDLE_VALUE == hFind)   {
		blad = GetLastError();
		return blad;
	} 
	else {
		
      // List all the other files in the directory.
		int i = 0;
		while (FindNextFile(hFind, &finddata) != 0) {
			if ( !wcscmp(finddata.cFileName,L".."))
				continue;
			dlugosc = (unsigned int)wcsnlen(finddata.cFileName, MAX_PATH);
			WriteFile(hFile,finddata.cFileName,dlugosc*2,&written,NULL);
			dlugosc = (unsigned int)wcslen(L"\n\r");
			WriteFile(hFile,L"\r\n",dlugosc *2,&written,NULL);

			if (finddata.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ){
				wchar_t * nowasciezka;
				nowasciezka = new wchar_t [MAX_PATH];

				wcsncpy(nowasciezka,biezacykat, MAX_PATH);
				wcscat(nowasciezka,L"\\");
				wcscat(nowasciezka,finddata.cFileName);
				search(nowasciezka, hFile);
				delete [] nowasciezka;

			}
			else {
			// dodaj plik do listy
				wchar_t * nowasciezka;
				nowasciezka = new wchar_t [MAX_PATH];
				wcsncpy(nowasciezka,biezacykat, MAX_PATH);
				wcscat(nowasciezka,L"\\");
				wcscat(nowasciezka,finddata.cFileName);
				rozmiar = finddata.nFileSizeHigh;
				rozmiar <<= 32;
				rozmiar = finddata.nFileSizeLow;
				dodaj_kontener(rozmiar,element, nowasciezka);

				delete [] nowasciezka;


			}

			i++;
		}
	}
	blad = GetLastError();
	if (blad != ERROR_NO_MORE_FILES) {
		// to koniec
	}
	delete [] sciezka;
	FindClose(hFind);
	return blad;
}




// nieuzywana
/*
void znajdz() {
	HANDLE hFile = INVALID_HANDLE_VALUE, hThread = INVALID_HANDLE_VALUE;
	DWORD ThreadID;
	hFile = CreateFile(L"c:\\out.txt", GENERIC_WRITE,  0, NULL, CREATE_ALWAYS, 0, NULL);			
		if (hFile == INVALID_HANDLE_VALUE) {
			return;
		}
	hThread = CreateThread(NULL,              // default security attributes
            0,                 // use default stack size  
            fun,          // thread function 
            hFile,             // argument to thread function 
            0,                 // use default creation flags 
            &ThreadID);   // returns the thread identifier 

        if (hThread == NULL) {
//           HeapFree(GetProcessHeap(), 0, pData);
           ExitProcess(1);
         }

		//WaitForSingleObject(hThread, INFINITE);
		SuspendThread(hThread);


		Sleep(5000);
		ResumeThread(hThread);
	return ;
}*/




int init_kontener(){
	glowa_kontener = 0;
	ogon_kontener = 0;
	return 0;
}

int dodaj_kontener(unsigned long int rozmiar, TElement * element, wchar_t * sciezka){
	TKontener *temp = 0, *wstaw = 0;
	// tworz element
	element = 0;
	element = new TElement;
	if (!element) {
		return 2; // blad
	}
	element->nast = 0;
	wcscpy(element->sciezka, sciezka);
	// pusto
	if (!glowa_kontener) {
		temp = new TKontener;
		if (!temp) {
			delete element;
			return 2; //blad
		}
		glowa_kontener = temp;
		temp->glowa = element;
		temp ->nast = 0;
		temp->ogon = element;
		temp->rozmiar = rozmiar;
	}
		
	else{
		temp = wstaw = glowa_kontener;
		while (temp && temp->rozmiar < rozmiar) {
			wstaw = temp;			
			temp = temp->nast;
		}


		if (temp && temp->rozmiar == rozmiar){
			dodaj_element(temp, element);
			dodaj_zadanie(element);
			// sygnalizuj ze gdzies sa 2 elementy o tym samym rozmiarze
		} else {
			temp = new TKontener;
			if (!temp) {
				delete element;
				return 2; //blad
			}
			// wstawiamy za kontenerem wstaw, w kontenerze 1 element
			temp->nast = wstaw->nast;
			wstaw->nast = temp;
			temp->glowa = element;
			temp->ogon = element;
			temp->rozmiar = rozmiar;
		}
	}
	return 0;
}

// elem - utworzony element z przydzielona pamiecia
// dodanie na koniec listy
int dodaj_element(TKontener * kont, TElement * elem){
	TElement *temp = kont->ogon;
	// co najmniej 1 istanieje, trzeba dla niego policzyc MD5
	if (kont->ogon == kont->glowa)
		dodaj_zadanie(kont->glowa);
	temp->nast = elem;
	kont->ogon = elem;
	return 0;
}



DWORD WINAPI LiczMd5( LPVOID lpParam ) {
	DWORD wynik;
	while (1) {
		wynik = WaitForSingleObject(Semafor,INFINITE);
		if (wynik == WAIT_OBJECT_0) {
			if (ilosc_zadan - nr_zadania){ // ilosc zadan do obsluzenia
				CountMD5(zadania[nr_zadania]->sciezka,zadania[nr_zadania]->MD5);
				// wykonano 1 zadanie
				nr_zadania++;

				if (nr_zadania == ilosc_zadan || nr_zadania > 255){
					// wykonano wszystkie zadania
					ilosc_zadan =  nr_zadania = 0;
				}
			} else {
				// nie ma zadan sytuacja 0,0 albo 256, 256
				ilosc_zadan = nr_zadania = 0;
			}

		}
		ReleaseSemaphore(Semafor,1,NULL);
		if (koniec) 
			break;
	}
	return 0;
}





DWORD WINAPI WatekSzukaj( LPVOID lpParam ) {
	HANDLE hPlik = INVALID_HANDLE_VALUE;
	DWORD ThreadID = 0;
	hPlik = CreateFile((*(TOpcje*)lpParam).Raport, GENERIC_WRITE,  0, NULL, CREATE_ALWAYS, 0, NULL);			
	if (hPlik == INVALID_HANDLE_VALUE) {
		return 1;
	}
	init_kontener();
	// semafor dostepu do zmiennych
	Semafor = CreateSemaphore(NULL, 1, 1, NULL);
    if (Semafor == NULL) {
        return 1 ; // blad
    }
	ThreadMD5 = INVALID_HANDLE_VALUE;
	ThreadMD5 = CreateThread(NULL, 0, LiczMd5, hPlik, 0, &ThreadID); 
	if (ThreadMD5 == NULL) {
        ExitProcess(1);
	}


	search((*(TOpcje*)lpParam).PodstawowaSciezka, hPlik);
	koniec = 1; // daj znac ze konczymy i czekaj na MD5


	//////////// tutaj sie nie czeka na zakonczenie MD5
	/// lista nie jest posortowania po dodaniu elementow
	// za czesnie wyrzuca kontenery
	/// blednie byly dodane elementy o tym samym rozmiarze (jeden za drugim w kontenerach
	WaitForSingleObject(ThreadMD5, INFINITE);
	kasuj_liste_kontenerow();
	CloseHandle(hPlik);
	// czy oby na pewno tutaj?
	CloseHandle(Semafor);
	return 0;

}



void start(TOpcje & opcje){
	DWORD ThreadID;
	ThreadSzukaj = INVALID_HANDLE_VALUE;
	ThreadSzukaj = CreateThread(NULL, 0, WatekSzukaj, &opcje, 0, &ThreadID); 
	if (ThreadSzukaj == NULL) {
        ExitProcess(1);
	}
	//WaitForSingleObject(hThread, INFINITE);
	//SuspendThread(hThread);
	//Sleep(5000);
	//ResumeThread(hThread);

	return ;
}



int kasuj_liste_kontenerow(){
	TKontener * temp = 0;
	while (glowa_kontener) {
		temp = glowa_kontener;
		glowa_kontener = glowa_kontener->nast;
		kasuj_elementy(temp);
		delete temp;
	}
	return 0;
}


int kasuj_elementy(TKontener * kontener){
	TElement * element = 0;
	if (kontener) {
		while (kontener->glowa) {
			element = kontener->glowa;
			kontener->glowa = kontener->glowa->nast;
			delete element;
		}
	}
	return 0;
}





int dodaj_zadanie(TElement *element) {
	DWORD wynik;
	bool wykonuj = true;
	while (wykonuj) {
		wynik = WaitForSingleObject(Semafor,INFINITE);
		if (wynik == WAIT_OBJECT_0) {
			if (ilosc_zadan < 255){
				zadania[ilosc_zadan++] = element;
				wykonuj = false;
				ReleaseSemaphore(Semafor,1, NULL);
			}
			else{
				// nie ma miejsca musimy poczekac i sprobowac pozniej
				ReleaseSemaphore(Semafor,1,NULL);
				Sleep(500);
			}
		}
	}
	return 0;
}



/*

/////////// przydzial pamieci

        // Allocate memory for thread data.

        pData = (typ) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(typ));

      if( pData == NULL )
           ExitProcess(2);


	*/