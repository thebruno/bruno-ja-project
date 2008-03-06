; dll file
.586
.MODEL FLAT, STDCALL
OPTION CASEMAP:NONE
.NOLIST
INCLUDE    include\windows.inc
INCLUDE    include\user32.inc
INCLUDE    include\kernel32.inc
INCLUDE    include\md5.inc
.LIST
INCLUDELIB   lib\kernel32.lib
INCLUDELIB   lib\user32.lib



 




; ####################################################################################################################
; ############################################  CODE   ###############################################################
.CODE


DllEntry PROC hInstDLL:HINSTANCE, reason:DWORD, reserved1:DWORD
    mov eax, TRUE
    ret
DllEntry ENDP




;###########TO DO
; sprawdzic logicznie CountMD5

;------------------------------------------------------------------------------------------------------------------------

;###########################################################
; CountMD5 oblicza wartosc MD5
; przyjmuje wskaznik na sciezke (filename), typ char *
; oraz wskaznik na przydzielona pamiec - 16 B dla wyniku
; typu unsigned char *
; zwraca 0 gdy OK, 1 gdy blad
; dopisac uses...
;###########################################################

CountMD5 PROC filename:DWORD, MD5:DWORD
	LOCAL file: HANDLE						; uchwyt na plik
	LOCAL read: DWORD						; ilosc przeczytanych ostatnio bajtow
	LOCAL context: MD5_CTX					; struktura z obliczana suma, iloscia bitow i buforem podrecznym


	invoke CreateFileW, filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL		; otworz plik
	.IF EAX == INVALID_HANDLE_VALUE
		mov EAX, 1
		ret																			; nie udalo sie utworzyc pliku
	.ENDIF
	
	; udalo sie utworzyc plik
	mov file, EAX					; file zawiera uchwyta na plik
	invoke MD5Init, ADDR context	; inicjalizacja MD5

petla: 
	invoke ReadFile, file, ADDR buffer, BUF_SIZE, ADDR read, NULL	; wypelnij duzy buffor
	; trzeba sprawdzic co ReadFile zwroci oraz co parametr read
	.IF EAX == 0													; sprawdz co zwraca ReadFile 
		jmp koniec_petli											; 0 (chyba blad odczytu) - koniec petli
	.ENDIF
	
	.IF read == 0													; odczytano 0 znakow - koniec petli
		jmp koniec_petli
	.ENDIF
	
	invoke MD5Update, ADDR context, ADDR buffer, read				; licz dalej MD5
	jmp petla														; kontynuacja petli

koniec_petli:
	invoke MD5Final, MD5, ADDR context					; dokoncz liczenie
	invoke CloseHandle, file; konczenie
	mov EAX, 0											; zero zero gdy OK 
	ret
CountMD5 ENDP
;###########################################################
; koniec CountMD5
;###########################################################
;-----------------------------------------------------------------------------------------------------------------------



;###########################################################
; inicjalizacja MD5, nowy context
; przyjmuje wskaznik na strukture MD5_CTX
; nic nie zwraca
;###########################################################
MD5Init PROC uses EBX, context:DWORD
	mov EBX, context					; EBX zawiera adres struktury MD5_CTX

	; zerowanie ilosci bitow, odwolanie do n-tego elementu struktury
	; wskazywanej przez context
	mov (MD5_CTX ptr [EBX]).count[0*DWORD_LENGTH], 0
	mov (MD5_CTX ptr [EBX]).count[1*DWORD_LENGTH], 0

	; stan poczatkowy struktury MD5_CTX
	mov (MD5_CTX ptr [EBX]).state[0*DWORD_LENGTH], 067452301h
	mov (MD5_CTX ptr [EBX]).state[1*DWORD_LENGTH], 0efcdab89h
	mov (MD5_CTX ptr [EBX]).state[2*DWORD_LENGTH], 098badcfeh
	mov (MD5_CTX ptr [EBX]).state[3*DWORD_LENGTH], 010325476h

	ret
MD5Init ENDP
;###########################################################
; koniec MD5Init 
;###########################################################

;------------------------------------------------------------------------------------------------------------------------

;###########################################################
; liczy dalej MD5, przetwarza kolejny blok, uaktualnia 
; context
; przyjmuje wskaznik na MD5_CTX, wskaznik na obszar pamieci
; typu unsigned char * i ilosc bajtow wejscoiwych typu
; unsigned int
; nic nie zwraca
;###########################################################
MD5Update PROC uses EBX ECX EDI ESI, context:DWORD, input:DWORD, inputLen:DWORD
	; LOCAL i:DWORD ; zamiast i jest ECX
	LOCAL index:DWORD, partLen:DWORD

	mov EBX, context					; EBX zawiera adres struktury MD5_CTX

	; oblicz ilosc bajtow modulo 64
	mov EAX, (MD5_CTX ptr [EBX]).count[0*DWORD_LENGTH]
	shr EAX, 3
	; dlaczego w reference tutaj jest 03Fh a nie 01Fh????????#########################################################################
	and EAX, 03Fh
	mov index, EAX

	; sprawdzenie czy przebija 2^32, jednoczesnie odcina 3 najstarsze bity
	mov EAX, inputLen 
	shl EAX, 3; dlugosc w bajtach, count w bitach (mnozymy przez 8)
	
	add (MD5_CTX ptr [EBX]).count[0*DWORD_LENGTH], EAX
	
	cmp (MD5_CTX ptr [EBX]).count[0*DWORD_LENGTH], EAX		; czy przekroczylo zakres, nie da sie bezporrednio przez .IF
	jae @F
		inc (MD5_CTX ptr [EBX]).count[1*DWORD_LENGTH]		; jesli przebilo to trzeba do starszej czesci count dodac 1
@@:


	; zawsze dodajemy 3 najstarsze bity inputLen (bo liczba bitow wej jest 35 bitowa
	; inputLen (32bity) przesuwamy o 3 w lewo
	mov EAX, inputLen ; modyfikowalismy wczesniej, wiec ponownie wczytujemy
	shr EAX, 29
	add (MD5_CTX ptr [EBX]).count[1*DWORD_LENGTH], EAX
	
	; pozycja w buforze 64 bajtowym
	mov EAX, 64
	sub EAX, index
	mov partLen,EAX
	
	.IF inputLen >= EAX					; EAX == partLen
		; adres wyjscia, odwolanie do elementu o numerze index (bajty)
		lea EDI, (MD5_CTX ptr [EBX]).buffer 
		add EDI, index
		mov ESI, input						; adres wejscia, input jest juz wskaznikiem
		MD5_memcpy EDI, ESI, partLen		; kopiuj - jest to makro, nie funkcja

		invoke MD5Transform, ADDR (MD5_CTX ptr [EBX]).state, ADDR (MD5_CTX ptr [EBX]).buffer	; wykonaj co najmniej 1 raz 
	
		mov ECX, partLen	
	petla:
		mov EAX, ECX
		add EAX, 63
		.IF EAX >= inputLen
			jmp koniec_petli
		.ENDIF
		
		mov EAX, input						; input to adres, dlatego trzeba sie przez rejestr odwolywac
		invoke MD5Transform, ADDR (MD5_CTX ptr [EBX]).state, ADDR [EAX][ECX] 
		

		add ECX, 64
		jmp petla
		
koniec_petli:		
		mov index, 0
			
	.ELSEIF
		mov ECX, 0
	.ENDIF

	sub inputLen, ECX						; == inputLen - i
	
	.IF inputLen == 0						; moja modyfikacja - jesli nie ma co kopiowac to nic nie rob
		ret
	.ENDIF
	
	; kopiuj input - ECX elementow
	lea edi, (MD5_CTX ptr [EBX]).buffer		; adres bufora
	add EDI, index							; element z buforu o numerze index
	mov ESI, input							; kolejny bajt z wejscia
	add ESI, ECX
	
	
	MD5_memcpy EDI, ESI, inputLen			; bez " - i"
	ret
MD5Update ENDP
;###########################################################
; koniec MD5Update
;###########################################################

;------------------------------------------------------------------------------------------------------------------------

;###########################################################
; koniczenie obliczania, zapisuje wynik dzialania, zeruje
; context
; przyjmuje wskaznik na 16 bajtow wyniku typu
; unsigned char [16] oraz wskaznik na MD5_CTX
; nic nie zwraca
; przez digest zwraca obliczone MD5
;###########################################################
MD5Final PROC uses EAX EBX, digest:DWORD, context:DWORD
LOCAL index:DWORD, padLen:DWORD, bits[8]:DWORD

	mov EBX, context									; zamieniamy bity na bajty
	mov EAX, (MD5_CTX ptr [EBX]).count[0*DWORD_LENGTH]
	shr EAX, 3
	and EAX, 03fh
	
	
	; zapamietaj aktualna ilosc bitow zamiast encode
	lea ESI, (MD5_CTX ptr [EBX]).count				; polozenie MD5 (zrodlo)
	lea EDI, bits									; polozenie MD5 (cel)
	MD5_memcpy EDI, ESI, 8				
	
	.IF EAX < 56				; EAX to jest index
		mov padLen, 56
	.ELSE
		mov padLen, 120
	.ENDIF
	mov index, EAX
	sub padLen, EAX				; od 56 albo 120 odejmujemy index (jest w EAX)
	
	; dopelnianie do 56 bajtow mod 64
	invoke MD5Update, context, ADDR PADDING, padLen

	; dodaje dlugosc do wiadomosci
	invoke MD5Update, context, ADDR bits, 8
	
	; zamiast encode jest zwykle kopiowanie
	lea ESI, (MD5_CTX ptr [EBX]).state				; polozenie MD5 (zrodlo)
	mov EDI, digest									; polozenie MD5 (cel)
	MD5_memcpy EDI, ESI, 16							; kopiuj 16 B
	
	;; dopisac MD5_memset


ret
MD5Final ENDP
;###########################################################
; koniec MD5Final
;###########################################################





;###########################################################
; bazowe przeksztalcenia MD5, przetwarza 64 bajtowy blok
; przyjmuje wskaznik na 4 integery oraz wskaznik na 
; 64 bajtowy blok danych typu unsigned char 
; nic nie zwraca

; napisac co robi
; zoptymalizowaæ uzywane rejestry
;###########################################################
; uzywane jeszcze EDX EDI ESI
MD5Transform PROC uses EBX ECX , state:DWORD, block:DWORD
	; zmienne a,b,c,d zast¹pione przez rejestry
	
	;invoke Decode, ADDR MD5x, block, 64			; block to juz jest adres niepotrzebne operujemy bezposrenio na bloku
	
	; inicjalizacja 4 czesci MD5 (a,b,c,d) z tablicy wskazywanej przez state
	mov ESI, state
	mov EAX, [ESI]
	mov EBX, [ESI + 1 * DWORD_LENGTH]
	mov ECX, [ESI + 2 * DWORD_LENGTH]
	mov EDX, [ESI + 3 * DWORD_LENGTH]
	
	mov ESI, block
	
	; runda 1
	FF EAX, EBX, ECX, EDX, [ESI + 0 * DWORD_LENGTH], S11, 0d76aa478h ; /* 1 */
	FF EDX, EAX, EBX, ECX, [ESI + 1 * DWORD_LENGTH], S12, 0e8c7b756h ; /* 2 */
	FF ECX, EDX, EAX, EBX, [ESI + 2 * DWORD_LENGTH], S13, 0242070dbh ; /* 3 */
	FF EBX, ECX, EDX, EAX, [ESI + 3 * DWORD_LENGTH], S14, 0c1bdceeeh ; /* 4 */
	FF EAX, EBX, ECX, EDX, [ESI + 4 * DWORD_LENGTH], S11, 0f57c0fafh ; /* 5 */
	FF EDX, EAX, EBX, ECX, [ESI + 5 * DWORD_LENGTH], S12, 04787c62ah ; /* 6 */
	FF ECX, EDX, EAX, EBX, [ESI + 6 * DWORD_LENGTH], S13, 0a8304613h ; /* 7 */
	FF EBX, ECX, EDX, EAX, [ESI + 7 * DWORD_LENGTH], S14, 0fd469501h ; /* 8 */
	FF EAX, EBX, ECX, EDX, [ESI + 8 * DWORD_LENGTH], S11, 0698098d8h ; /* 9 */
	FF EDX, EAX, EBX, ECX, [ESI + 9 * DWORD_LENGTH], S12, 08b44f7afh ; /* 10 */
	FF ECX, EDX, EAX, EBX, [ESI +10 * DWORD_LENGTH], S13, 0ffff5bb1h ; /* 11 */
	FF EBX, ECX, EDX, EAX, [ESI +11 * DWORD_LENGTH], S14, 0895cd7beh ; /* 12 */
	FF EAX, EBX, ECX, EDX, [ESI +12 * DWORD_LENGTH], S11, 06b901122h ; /* 13 */
	FF EDX, EAX, EBX, ECX, [ESI +13 * DWORD_LENGTH], S12, 0fd987193h ; /* 14 */
	FF ECX, EDX, EAX, EBX, [ESI +14 * DWORD_LENGTH], S13, 0a679438eh ; /* 15 */
	FF EBX, ECX, EDX, EAX, [ESI +15 * DWORD_LENGTH], S14, 049b40821h ; /* 16 */	




	; runda 2
	GG EAX, EBX, ECX, EDX, [ESI + 1 * DWORD_LENGTH], S21, 0f61e2562h ; /* 17 */
	GG EDX, EAX, EBX, ECX, [ESI + 6 * DWORD_LENGTH], S22, 0c040b340h ; /* 18 */
	GG ECX, EDX, EAX, EBX, [ESI +11 * DWORD_LENGTH], S23, 0265e5a51h ; /* 19 */
	GG EBX, ECX, EDX, EAX, [ESI + 0 * DWORD_LENGTH], S24, 0e9b6c7aah ; /* 20 */
	GG EAX, EBX, ECX, EDX, [ESI + 5 * DWORD_LENGTH], S21, 0d62f105dh ; /* 21 */
	GG EDX, EAX, EBX, ECX, [ESI +10 * DWORD_LENGTH], S22,  02441453h ; /* 22 */
	GG ECX, EDX, EAX, EBX, [ESI +15 * DWORD_LENGTH], S23, 0d8a1e681h ; /* 23 */
	GG EBX, ECX, EDX, EAX, [ESI + 4 * DWORD_LENGTH], S24, 0e7d3fbc8h ; /* 24 */
	GG EAX, EBX, ECX, EDX, [ESI + 9 * DWORD_LENGTH], S21, 021e1cde6h ; /* 25 */
	GG EDX, EAX, EBX, ECX, [ESI +14 * DWORD_LENGTH], S22, 0c33707d6h ; /* 26 */
	GG ECX, EDX, EAX, EBX, [ESI + 3 * DWORD_LENGTH], S23, 0f4d50d87h ; /* 27 */
	GG EBX, ECX, EDX, EAX, [ESI + 8 * DWORD_LENGTH], S24, 0455a14edh ; /* 28 */
	GG EAX, EBX, ECX, EDX, [ESI +13 * DWORD_LENGTH], S21, 0a9e3e905h ; /* 29 */
	GG EDX, EAX, EBX, ECX, [ESI + 2 * DWORD_LENGTH], S22, 0fcefa3f8h ; /* 30 */
	GG ECX, EDX, EAX, EBX, [ESI + 7 * DWORD_LENGTH], S23, 0676f02d9h ; /* 31 */
	GG EBX, ECX, EDX, EAX, [ESI +12 * DWORD_LENGTH], S24, 08d2a4c8ah ; /* 32 */

	; runda 3
	HH EAX, EBX, ECX, EDX, [ESI + 5 * DWORD_LENGTH], S31, 0fffa3942h ; /* 33 */
	HH EDX, EAX, EBX, ECX, [ESI + 8 * DWORD_LENGTH], S32, 08771f681h ; /* 34 */
	HH ECX, EDX, EAX, EBX, [ESI +11 * DWORD_LENGTH], S33, 06d9d6122h ; /* 35 */
	HH EBX, ECX, EDX, EAX, [ESI +14 * DWORD_LENGTH], S34, 0fde5380ch ; /* 36 */
	HH EAX, EBX, ECX, EDX, [ESI + 1 * DWORD_LENGTH], S31, 0a4beea44h ; /* 37 */
	HH EDX, EAX, EBX, ECX, [ESI + 4 * DWORD_LENGTH], S32, 04bdecfa9h ; /* 38 */
	HH ECX, EDX, EAX, EBX, [ESI + 7 * DWORD_LENGTH], S33, 0f6bb4b60h ; /* 39 */
	HH EBX, ECX, EDX, EAX, [ESI +10 * DWORD_LENGTH], S34, 0bebfbc70h ; /* 40 */
	HH EAX, EBX, ECX, EDX, [ESI +13 * DWORD_LENGTH], S31, 0289b7ec6h ; /* 41 */
	HH EDX, EAX, EBX, ECX, [ESI + 0 * DWORD_LENGTH], S32, 0eaa127fah ; /* 42 */
	HH ECX, EDX, EAX, EBX, [ESI + 3 * DWORD_LENGTH], S33, 0d4ef3085h ; /* 43 */
	HH EBX, ECX, EDX, EAX, [ESI + 6 * DWORD_LENGTH], S34,  04881d05h ; /* 44 */
	HH EAX, EBX, ECX, EDX, [ESI + 9 * DWORD_LENGTH], S31, 0d9d4d039h ; /* 45 */
	HH EDX, EAX, EBX, ECX, [ESI +12 * DWORD_LENGTH], S32, 0e6db99e5h ; /* 46 */
	HH ECX, EDX, EAX, EBX, [ESI +15 * DWORD_LENGTH], S33, 01fa27cf8h ; /* 47 */
	HH EBX, ECX, EDX, EAX, [ESI + 2 * DWORD_LENGTH], S34, 0c4ac5665h ; /* 48 */

	; runda 4
	II EAX, EBX, ECX, EDX, [ESI + 0 * DWORD_LENGTH], S41, 0f4292244h ; /* 49 */
	II EDX, EAX, EBX, ECX, [ESI + 7 * DWORD_LENGTH], S42, 0432aff97h ; /* 50 */
	II ECX, EDX, EAX, EBX, [ESI +14 * DWORD_LENGTH], S43, 0ab9423a7h ; /* 51 */
	II EBX, ECX, EDX, EAX, [ESI + 5 * DWORD_LENGTH], S44, 0fc93a039h ; /* 52 */
	II EAX, EBX, ECX, EDX, [ESI +12 * DWORD_LENGTH], S41, 0655b59c3h ; /* 53 */
	II EDX, EAX, EBX, ECX, [ESI + 3 * DWORD_LENGTH], S42, 08f0ccc92h ; /* 54 */
	II ECX, EDX, EAX, EBX, [ESI +10 * DWORD_LENGTH], S43, 0ffeff47dh ; /* 55 */
	II EBX, ECX, EDX, EAX, [ESI + 1 * DWORD_LENGTH], S44, 085845dd1h ; /* 56 */
	II EAX, EBX, ECX, EDX, [ESI + 8 * DWORD_LENGTH], S41, 06fa87e4fh ; /* 57 */
	II EDX, EAX, EBX, ECX, [ESI +15 * DWORD_LENGTH], S42, 0fe2ce6e0h ; /* 58 */
	II ECX, EDX, EAX, EBX, [ESI + 6 * DWORD_LENGTH], S43, 0a3014314h ; /* 59 */
	II EBX, ECX, EDX, EAX, [ESI +13 * DWORD_LENGTH], S44, 04e0811a1h ; /* 60 */
	II EAX, EBX, ECX, EDX, [ESI + 4 * DWORD_LENGTH], S41, 0f7537e82h ; /* 61 */
	II EDX, EAX, EBX, ECX, [ESI +11 * DWORD_LENGTH], S42, 0bd3af235h ; /* 62 */
	II ECX, EDX, EAX, EBX, [ESI + 2 * DWORD_LENGTH], S43, 02ad7d2bbh ; /* 63 */
	II EBX, ECX, EDX, EAX, [ESI + 9 * DWORD_LENGTH], S44, 0eb86d391h ; /* 64 */
	
	; Zapamietanie wynikow w tablicy state
	mov ESI, state
	add [ESI], EAX
	add [ESI + 1 * DWORD_LENGTH], EBX
	add [ESI + 2 * DWORD_LENGTH], ECX
	add [ESI + 3 * DWORD_LENGTH], EDX
	
	; ewentualnie dopisac zerowanie jesli trzeba

ret
MD5Transform ENDP

;###########################################################
; koniec MD5Transform
;###########################################################



;###########################################################
; napisac co robi
;unsigned char *output, UINT4 *input, unsigned int len
;###########################################################

Encode PROC output:DWORD, input:DWORD, len:DWORD

ret
Encode ENDP

;###########################################################
; koniec Encode
;###########################################################






;###########################################################
; napisac co robi
;UINT4 *output, unsigned char *input, unsigned int len
;###########################################################
Decode PROC output:DWORD, input:DWORD, len:DWORD

ret
Decode ENDP

;###########################################################
; koniec Dencode
;###########################################################













; do wywalenia 
ReadTxtFile PROC path:DWORD, OutSize:DWORD
LOCAL FileSize:DWORD
	invoke CreateFileW,path, GENERIC_READ , 0, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL; otworz plik 
	mov File,eax 
	invoke GetFileSize, File, NULL; odczytaj jego wielkosc
	mov ebx, OutSize; zwroc na zewnatrz wielkosc pliku
	mov dword ptr [ebx], eax
	mov FileSize, eax ; zapamietaj 
	
	invoke CreateFileMapping,File,NULL,PAGE_READONLY,0,0,NULL; zamapuj plik	
	mov FileMap, eax 	
	invoke MapViewOfFile,FileMap,FILE_MAP_READ,0,0,0; wczytaj go i udostepnij pamiec tylko do odczytu
	mov Mem,eax	
	ret 

ReadTxtFile ENDP

; do wywalenia 
CloseMapFile PROC
	; odmapuj plik, zwolnij uchwyty do zamapowanego pliku oraz pliku
	invoke UnmapViewOfFile,Mem
	mov Mem, 0
    invoke CloseHandle,FileMap
    mov    FileMap,0
    invoke CloseHandle,File
    mov File, 0
    ret
CloseMapFile endp 

END DllEntry















; ta linijka zawiesza masma
; mov EBX, offset (MD5_CTX ptr [EBX]).state