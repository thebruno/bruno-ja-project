//#include <windows.h>
int search (wchar_t* dir, HANDLE hFile) ;




struct TOpcje {
	wchar_t PodstawowaSciezka [MAX_PATH];
	wchar_t DodatkowaSciezka [MAX_PATH];
	wchar_t Raport [MAX_PATH];

};
struct TElement;

struct TKontener {
	unsigned long int rozmiar;
	TKontener * nast;
	TElement * glowa, *ogon;

};
struct TElement{
	TElement * nast;
	char MD5[16];
	wchar_t sciezka [MAX_PATH];



};


///////////////////////////


// na podstawie opcji startuje watek przeszukiwania 
void start(TOpcje & opcje);
int init_kontener();
// zwraca 0 - dodano element, 1 - element istnieje, 2 - blad, wskaznik na utworzony(dodany)  element przez element
int dodaj_kontener(unsigned long int rozmiar, TElement * element, wchar_t* sciezka);
int kasuj_liste_kontenerow();
// do ktorego kontenera dodac i co
int dodaj_element(TKontener * kont, TElement * elem);
int kasuj_elementy(TKontener * kontener);
int dodaj_zadanie(TElement *element);




