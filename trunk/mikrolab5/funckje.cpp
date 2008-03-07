#include "stdafx.h"


TKontener * glowa_kontener, *ogon_kontener ;
HANDLE ThreadSzukaj, ThreadMD5, Semafor;
const int MAKSYMALNA_ILOSC_ZADAN = 256;
TEnter enter;
TElement *zadania [MAKSYMALNA_ILOSC_ZADAN];
// nieobsluzonych jest (ilosc_zadan - nr_zadania) 
int ilosc_zadan = 0, nr_zadania = 0, koniec = 0;

// sciezka bez gwiazdki!!
int search (wchar_t* biezacykat, HANDLE hFile) {
	WIN32_FIND_DATA finddata;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD blad = 0;
	wchar_t * sciezka = new wchar_t [MAX_PATH];
	unsigned int dlugosc;
	unsigned long int written = 0, rozmiar;
	TElement * element = 0;
	TKontener * kontener = 0;

   // sciezka nie moze byc dluzsza niz MAX_PATH + 2 znakow
	dlugosc = (unsigned int) wcsnlen(biezacykat, MAX_PATH);
	if (dlugosc > (MAX_PATH - 2)) {
		return (1); // za dluga sciezka
   }
	// kopiuj i dodaj "\\*" na koniec
	wcsncpy(sciezka, biezacykat, MAX_PATH);
	wcscat(sciezka, L"\\*");
	// najpierw znaleziona jest ".", potem ".." i dopiero potem pliki
	hFind = FindFirstFile(sciezka, &finddata);
	if (INVALID_HANDLE_VALUE == hFind)   {
		blad = GetLastError();
		return blad;
	} 
	else {
		// listuj pliki
		while (FindNextFile(hFind, &finddata) != 0) {
			// kropke znajduje przy findfirst
			if ( !wcscmp(finddata.cFileName,L".."))
				continue;
			dlugosc = (unsigned int)wcsnlen(finddata.cFileName, MAX_PATH);
			WriteFile(hFile,finddata.cFileName,dlugosc*2,&written,NULL);
			// pisz enter
			WriteFile(hFile,enter.enter,enter.dlugosc * 2,&written,NULL);
			if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
				wchar_t * nowasciezka;
				// czy nazwa nie jest za dluga
				if ((unsigned int)wcsnlen(finddata.cFileName, MAX_PATH) 
					+ (unsigned int)wcsnlen(biezacykat, MAX_PATH) + 1 > MAX_PATH){ // 1 dla znaku "\\"
					delete [] sciezka;
					FindClose(hFind);
					return blad;
				}
				nowasciezka = new wchar_t [MAX_PATH];
				wcsncpy(nowasciezka, biezacykat, MAX_PATH);
				wcscat(nowasciezka,L"\\");
				wcscat(nowasciezka,finddata.cFileName);
				search(nowasciezka, hFile);
				delete [] nowasciezka;
			}
			else {
				// dodaj plik do listy
				wchar_t * nowasciezka;
				// pomin pliki o za dlugiej nazwie
				if ((unsigned int)wcsnlen(finddata.cFileName, MAX_PATH) 
					+ (unsigned int)wcsnlen(biezacykat, MAX_PATH) + 1 > MAX_PATH){ // 1 dla znaku "\\"
					delete [] sciezka;
					FindClose(hFind);
					return blad;
				}
				nowasciezka = new wchar_t [MAX_PATH];
				wcsncpy(nowasciezka,biezacykat, MAX_PATH);
				wcscat(nowasciezka,L"\\");
				wcscat(nowasciezka,finddata.cFileName);
				rozmiar = finddata.nFileSizeHigh;
				rozmiar <<= 32;
				rozmiar = finddata.nFileSizeLow;
				dodaj_kontener(rozmiar, element, nowasciezka);
				// element - ostatni dodany element
				delete [] nowasciezka;
			}
		}
	}
	blad = GetLastError();
	delete [] sciezka;
	FindClose(hFind);
	return blad;
}

// inicjalizacja
int init(){
	ThreadMD5 = ThreadSzukaj = Semafor = INVALID_HANDLE_VALUE;
	ilosc_zadan = nr_zadania = koniec = 0; 
	glowa_kontener = 0;
	ogon_kontener = 0;
	enter.dlugosc= (unsigned int)wcslen(L"\r\n");
	wcsncpy(enter.enter, L"\r\n", 3);
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
		temp->ilosc_elementow = 1;
		temp->nast = 0;
		temp->glowa = element;
		temp->ogon = element;
		temp->rozmiar = rozmiar;
	}
	else{
		temp = wstaw = glowa_kontener;
		// wstawiamy za elementem wstaw, z wyjatkiem glowy
		while (temp && rozmiar < temp->rozmiar) {
			wstaw = temp;			
			temp = temp->nast;
		}
		// dodaj element do istniejacego kontenera
		if (temp && temp->rozmiar == rozmiar){
			dodaj_element(temp, element);
			// sygnalizuj ze gdzies sa 2 elementy o tym samym rozmiarze
			dodaj_zadanie(element);
		} else {
			// towrzymy nowy kontener
			temp = new TKontener;
			if (!temp) {
				delete element;
				return 2; //blad
			}
			// kontener jest nowy, wiec mozna to przypisac
			temp->glowa = temp->ogon = element;
			temp->rozmiar = rozmiar;
			temp->ilosc_elementow = 1;
			if (wstaw == glowa_kontener){
				// wstawiamy jako glowa lub za glowa (trzeba osobno rozpatrzyc)
				if (rozmiar > glowa_kontener->rozmiar) {
					//nowa glowa
					temp->nast = glowa_kontener;
					glowa_kontener = temp;
				} else {
					// wstawiamy za glowa
					temp->nast = glowa_kontener->nast;
					glowa_kontener->nast = temp;
				}
			} else {
				temp->nast = wstaw->nast;
				wstaw->nast = temp;
			}
		}
	}
	return 0;
}

// elem - utworzony element z przydzielona pamiecia
// dodanie na koniec listy
int dodaj_element(TKontener * kont, TElement * elem){
	TElement *temp = kont->ogon;
	// istnieje dokladnie 1 element, dla ktorego nie policzono jeszcze MD5
	if (kont->ogon == kont->glowa)
		dodaj_zadanie(kont->glowa);
	temp->nast = elem;
	kont->ogon = elem;
	kont->ilosc_elementow++;
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
				if (nr_zadania == ilosc_zadan || nr_zadania > MAKSYMALNA_ILOSC_ZADAN - 1){
					// wykonano wszystkie zadania
					ilosc_zadan =  nr_zadania = 0;
				}
			} else {
				// nie ma zadan sytuacja 0,0 albo MAKSYMALNA_ILOSC_ZADAN, MAKSYMALNA_ILOSC_ZADAN
				ilosc_zadan = nr_zadania = 0;
				if (koniec) 
					break;
			}
		}
		ReleaseSemaphore(Semafor,1,NULL);
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

	WaitForSingleObject(ThreadMD5, INFINITE);
	//kasuj_liste_kontenerow(); - kasujemy na zyczenie
	CloseHandle(hPlik);
	CloseHandle(Semafor);
	return 0;
}



void start(TOpcje & opcje){
	init();
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
	glowa_kontener = 0;
	ogon_kontener = 0;
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
			if (ilosc_zadan < MAKSYMALNA_ILOSC_ZADAN - 1){
				zadania[ilosc_zadan++] = element;
				wykonuj = false;
				ReleaseSemaphore(Semafor,1, NULL);
			}
			else{
				// nie ma miejsca musimy poczekac i sprobowac pozniej
				ReleaseSemaphore(Semafor,1,NULL);
				Sleep(5);
			}
		}
	}
	return 0;
}

int generuj_raport(TOpcje *opcje){
	TKontener * glowa = glowa_kontener;
	HANDLE hPlik = INVALID_HANDLE_VALUE;
	wchar_t WMD5[32+1];  // MD5 ma 16 bajtow, 32 znaki (char albo wchar_t)
	unsigned long int written = 0;
	unsigned long int dlugosc;
	WMD5[32] = (WCHAR) 0; // dodaj zero na koniec
	TElement * elem = 0;
	//hPlik = CreateFile((*(TOpcje*)lpParam).Raport, GENERIC_WRITE,  0, NULL, CREATE_ALWAYS, 0, NULL);			
	hPlik = CreateFile(L"c:\\duplikaty.txt", GENERIC_WRITE,  0, NULL, CREATE_ALWAYS, 0, NULL);			
	if (hPlik == INVALID_HANDLE_VALUE) {
		return 1;
	}
	while (glowa) {
		if (glowa->ilosc_elementow > 1){
			elem = glowa->glowa;
		}
		else {
			elem = 0;

		}
		while(elem) {
			dlugosc = (unsigned int)wcslen(elem->sciezka);
			WriteFile(hPlik,elem->sciezka,dlugosc * 2,&written,NULL);
			dlugosc = (unsigned int)wcslen(L"\n\r");
			WriteFile(hPlik,L"\r\n",dlugosc * 2,&written,NULL);
			MD5ToWStr(WMD5,elem->MD5);
			dlugosc = (unsigned int)wcslen(WMD5);
			WriteFile(hPlik,WMD5,dlugosc * 2,&written,NULL);
			dlugosc = (unsigned int)wcslen(L"\n\r");
			WriteFile(hPlik,L"\r\n",dlugosc *2,&written,NULL);
			elem = elem->nast;

		}
		glowa = glowa->nast;
	}
	CloseHandle(hPlik);
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

// konwersja input binarnego na znaki typu wide char
void MD5ToWStr(wchar_t * output, char * input){
	int i;
	char temp[32];
	for (i = 0; i < 16; i++ ) {
		temp[2*i] = ((input[i]>>4) & 0x0F) < 10?  ((input[i]>>4) & 0x0F)+ 48:((input[i]>>4) & 0x0F) + 55; 
		temp[2*i+1] = (input[i] & 0x0F) < 10?  (input[i] & 0x0F)+ 48:(input[i] & 0x0F) + 55; 
	}
	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,temp, 32, output, 32);
}

// konwersja input binarnego na znaki typu char
void MD5ToAStr(char * output, char * input){
	int i;
	for (i = 0; i < 16; i++ ) {
		output[2*i] = ((input[i]>>4) & 0x0F) < 10?  ((input[i]>>4) & 0x0F)+ 48:((input[i]>>4) & 0x0F) + 55; 
		output[2*i+1] = (input[i] & 0x0F) < 10?  (input[i] & 0x0F)+ 48:(input[i] & 0x0F) + 55; 
	}
}







///////////////TODO
// dodac warunek aby nie sprawdzal sumy kontrolnej dla pustych plikow
// pliki, ktorych sie nie da otworzyc generuja bledna sume (nie oblicza sie dla niech smua)