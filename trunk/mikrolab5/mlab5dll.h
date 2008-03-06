// wywalic
extern "C" void * __stdcall ReadTxtFile(void * , int * );
// wywalic
extern "C" void __stdcall CloseMapFile(void);


// definicje MD5 widoczne na zewnatrz
extern "C" int __stdcall CountMD5(wchar_t*,  char *);


void MD5ToStr(char * output, char * input);