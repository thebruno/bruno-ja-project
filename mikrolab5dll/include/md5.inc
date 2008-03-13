;###########################################################
; Deklaracje stalych
;###########################################################

; ilosc bajtow wczytywanych na raz z dysku
BUF_SIZE equ 4*1024

; Stale dla funkcji MD5Transform dotyczace rotacji w lewo
S11 EQU 7
S12 EQU 12
S13 EQU 17
S14 EQU 22
S21 EQU 5
S22 EQU 9
S23 EQU 14
S24 EQU 20
S31 EQU 4
S32 EQU 11
S33 EQU 16
S34 EQU 23
S41 EQU 6
S42 EQU 10
S43 EQU 15
S44 EQU 21

; ilosb bajtow typu DWORD (4B)
DWORD_LENGTH EQU sizeof DWORD


;###########################################################
; Deklaracje struktur
;###########################################################

; kontekst MD5, ABCD - 16B obliczonej sumy MD5
MD5_CTX STRUCT 4
	state DWORD 4 dup (0) ; stan ABCD (wynik)
	count DWORD 2 dup (0) ; ilosc bitow mod 2^64, najpierw lsb 
	buffer db 64 dup(0)   ; wejsciowy bufor
MD5_CTX ENDS


;###########################################################
; Deklaracje prototypow funkcji (opisy w pliku asm
;###########################################################
; najwazniejsza
CountMD5 PROTO :DWORD, :DWORD
ThreadCountMD5 PROTO :DWORD, :DWORD, :DWORD
ThreadCountMD5Body PROTO :DWORD
; pomocnicze
MD5Init PROTO :DWORD
MD5Update PROTO :DWORD, :DWORD, :DWORD
MD5Final PROTO :DWORD, :DWORD
MD5Transform PROTO
MD5_meset PROTO :DWORD, :BYTE,  :DWORD

;dodaj_element PROTO C :DWORD, :DWORD
;###########################################################
; Deklaracje zmiennych
;###########################################################

.DATA 
; deklaracje MD5
buffer DB BUF_SIZE dup (0)
PADDING BYTE 80h, 63 dup (0) ; wyrownywanie
ThreadParam DWORD 3 dup (0)  ; 12 B adres sciezki, adres 16 tab
; na MD5 oraz zwracany HANDLE do watku
Tekst DB 'M','D',5,0


;###########################################################
; Definicje MAKR
;###########################################################

; podstawowe funckcje MD5
; Funkcja F, oblicza ((x) & (y)) | ((~x) & (z))
; x = EAX, y = EBX, z = ECX, out = EDI, wynik do out
F MACRO x, y, z, out
	push y
	mov out, x
	and y, x
	not out
	and out, z
	or out, y
	pop y
ENDM

; Funkcja G oblicza ((x) & (z)) | ((y) & (~z))
; czyli ((z) & (x) | ((~z) & (y))
; x = EAX, y = EBX, z = ECX, out = EDI, wynik do out
G MACRO x, y, z, out
	push x
	mov out, z
	and x, z
	not out
	and out, y
	or out, x
	pop x
ENDM

; Funkcja H, oblicza (x) ^ (y) ^ (z)
; x = EAX, y = EBX, z = ECX, out = EDI, wynik do out
H MACRO x, y, z, out
	mov out, x
	xor out, y
	xor out, z
ENDM

; Funkcja I, oblicza (y) ^ ((x) | (~z))
; x = EAX, y = EBX, z = ECX, out = EDI, wynik do out
I MACRO x, y, z, out
	mov out, z
	not out
	or out, x
	xor out, y
ENDM

; obraca x o n bitow w lewo
; x i n - rozne rejestry, out - EDI
; wynik przez out zwracamy
ROTATE_LEFT MACRO x, n, out
	mov out, x
	rol out, n
ENDM


; kopiowanie - podana ilosc bajtow musi byc podzielna przez 4
; wej to ESI, wyj to EDI
; kopiowanie bajtow doooooooooooooo analizy


MD5_memcpy MACRO dlug
	mov ECX, dlug
	cld					; inkrementacja rejestrow indeksowych
	rep movsb
ENDM


; transformacje

; a = EAX, b = EBX, c = ECX, d = EDX, x = tablica, s - wielkosc przesuniecia, ac - stala, 
; wynik zostaje w rejestrach, modyfikuje dodatkowo EDI
FF MACRO a, b, c, d, x, s, ac
	F b, c, d, EDI				; podstawowa funkcja F, EDI zawiera wynik
	add EDI, x
	add EDI, ac
	add a, EDI
	ROTATE_LEFT a, s, EDI
	mov a, EDI
	add a, b
ENDM

; a = EAX, b = EBX, c = ECX, d = EDX, x = tablica, s - wielkosc przesuniecia, ac - stala, 
; wynik zostaje w rejestrach, modyfikuje dodatkowo EDI
GG MACRO a, b, c, d, x, s, ac
	G b, c, d, EDI				; podstawowa funkcja G, EDI zawiera wynik
	add EDI, x
	add EDI, ac
	add a, EDI
	ROTATE_LEFT a, s, EDI
	mov a, EDI
	add a, b
ENDM


; a = EAX, b = EBX, c = ECX, d = EDX, x = tablica, s - wielkosc przesuniecia, ac - stala, 
; wynik zostaje w rejestrach, modyfikuje dodatkowo EDI
HH MACRO a, b, c, d, x, s, ac
	H b, c, d, EDI				; podstawowa funkcja G, EDI zawiera wynik
	add EDI, x
	add EDI, ac
	add a, EDI
	ROTATE_LEFT a, s, EDI
	mov a, EDI
	add a, b
ENDM

; a = EAX, b = EBX, c = ECX, d = EDX, x = tablica, s - wielkosc przesuniecia, ac - stala, 
; wynik zostaje w rejestrach, modyfikuje dodatkowo EDI
II MACRO a, b, c, d, x, s, ac
	I b, c, d, EDI				; podstawowa funkcja G, EDI zawiera wynik
	add EDI, x
	add EDI, ac
	add a, EDI
	ROTATE_LEFT a, s, EDI
	mov a, EDI
	add a, b
ENDM
