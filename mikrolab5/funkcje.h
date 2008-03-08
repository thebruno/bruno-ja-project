int search (wchar_t* dir, HANDLE hFile) ;
struct TOpcje {
	wchar_t PodstawowaSciezka [MAX_PATH];
	wchar_t DodatkowaSciezka [MAX_PATH];
	wchar_t Raport [MAX_PATH];

};
struct TElement;
struct TKontener {
	unsigned long int rozmiar;
	unsigned int ilosc_elementow;
	TKontener * nast;
	TElement * glowa, *ogon;

};
struct TElement{
	TElement * nast;
	bool zaznaczony;
	char MD5[16];
	wchar_t sciezka [MAX_PATH];
};

struct TEnter {
	wchar_t enter[3];
	unsigned int dlugosc;
};

// na podstawie opcji startuje watek przeszukiwania 
void start(TOpcje & opcje);
int init();
// zwraca 0 - dodano element, 1 - element istnieje, 2 - blad, wskaznik na utworzony(dodany)  element przez element
int dodaj_kontener(unsigned long long int rozmiar, TElement * element, wchar_t* sciezka);
int kasuj_liste_kontenerow();
// do ktorego kontenera dodac i co
int dodaj_element(TKontener * kont, TElement * elem);
int kasuj_elementy(TKontener * kontener);
int dodaj_zadanie(TElement *element);
int generuj_raport(TOpcje *opcje);
int generuj_raport1(TOpcje *opcje);
void MD5ToWStr(wchar_t * output, char * input); //wchar_t
void MD5ToAStr(char * output, char * input); //ansi
int MD5Cmp(char *src1, char *src2);



