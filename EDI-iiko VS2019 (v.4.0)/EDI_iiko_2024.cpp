#include "stdafx.h"
#include "stdio.h"
#include "commctrl.h"
#include "resource.h"
#include "io.h"
#include "windows.h"
#include <sys/timeb.h>
#include <time.h>
#include <direct.h>

//#define _MT
//#include <process.h>
//#pragma comment(lib, "libcmt.lib")
//#pragma comment(linker, "/NODEFAULTLIB:libc.lib")
//#define DLG_MAIN  500

HINSTANCE			hInst;
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg; char *a; HACCEL hAccelTable; MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_EDI_iiko_2024);
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg); DispatchMessage(&msg);}}
	return msg.wParam; }

ATOM MyRegisterClass(HINSTANCE hInstance) {		WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;		wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;					wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_EDI_iiko_2024);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_EDI_iiko_2024;	wcex.lpszClassName	= "EDI_iiko_2024";
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_EDI_iiko_2024);
	return RegisterClassEx(&wcex); }

extern int EDI_iiko_2024_xml(), EDI_iiko_2024_web();

struct {
	char INP_DIR[100], HOST[100], KEY[100], LOG_FIL[120];
	unsigned int PORT;
} USER[10];
int USER_NR;

SYSTEMTIME st;
 bool XML_SUMA_BE_PVM = false;
FILE* fi, * fo, * f_log;
char CURRENT_INP[200], CURRENT_OUT[200], LOG_TXT[2000], LOG_NAME[300], Fresh = 0;
char* BUF, * data, * logs; bool ReturnedInvoice; bool SAVIKAINA_INI = false; long TOTAL_SUM;

char* DATE()
{
	static char date[20];
	GetLocalTime(&st);
	sprintf(date, "%u-%02u-%02u %02u:%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	return(date);
}

void LOG(char* t1, char* t2, char* t3, char* t4, char* t5)
{
	strcpy(LOG_NAME, USER[USER_NR].LOG_FIL); GetLocalTime(&st);
	sprintf(LOG_NAME + strlen(LOG_NAME) - 4, "-%u.%02u.%02u-ALL.LOG", st.wYear, st.wMonth, st.wDay);
	if ((f_log = fopen(LOG_NAME, "a")) == NULL) { MessageBox(NULL, "Neatsidaro LOG failas", LOG_NAME, MB_OK); exit(0); }
	fprintf(f_log, "%s (%d) %s%s%s%s%s\n", DATE(), USER_NR, t1, t2, t3, t4, t5); fclose(f_log);
}
void LOG(char* t1, char* t2, char* t3, char* t4)
{
	LOG(t1, t2, t3, t4, "");
}
void LOG(char* t1, char* t2, char* t3)
{
	LOG(t1, t2, t3, "", "");
}
void LOG(char* t1, char* t2)
{
	LOG(t1, t2, "", "", "");
}
void LOG(char* t1)
{
	LOG(t1, "", "", "", "");
}

void LOG_2(char* t1, char* t2, char* t3, char* t4, char* t5) {
	GetLocalTime(&st);
	strcpy(LOG_NAME, USER[USER_NR].LOG_FIL); sprintf(LOG_NAME + strlen(LOG_NAME) - 4, "-%u.%02u.%02u-ALL.LOG", st.wYear, st.wMonth, st.wDay);
	if ((f_log = fopen(LOG_NAME, "a")) == NULL) { MessageBox(NULL, "Neatsidaro LOG_1 failas", LOG_NAME, MB_OK); exit(0); }
	fprintf(f_log, "%s (%d) %s%s%s%s%s\n", DATE(), USER_NR, t1, t2, t3, t4, t5); fclose(f_log);
	strcpy(LOG_NAME, USER[USER_NR].LOG_FIL); sprintf(LOG_NAME + strlen(LOG_NAME) - 4, "-%u.%02u.%02u.LOG", st.wYear, st.wMonth, st.wDay);
	if ((f_log = fopen(LOG_NAME, "a")) == NULL) { MessageBox(NULL, "Neatsidaro LOG_2 failas", LOG_NAME, MB_OK); exit(0); }
	fprintf(f_log, "%s (%d) %s%s%s%s%s\n", DATE(), USER_NR, t1, t2, t3, t4, t5); fclose(f_log);
}
void LOG_2(char* t1, char* t2, char* t3, char* t4)
{
	LOG_2(t1, t2, t3, t4, "");
}
void LOG_2(char* t1, char* t2, char* t3)
{
	LOG_2(t1, t2, t3, "", "");
}
void LOG_2(char* t1, char* t2)
{
	LOG_2(t1, t2, "", "", "");
}
void LOG_2(char* t1)
{
	LOG_2(t1, "", "", "", "");
}

void LOG_XML(char* t) {
	strcpy(LOG_NAME, USER[USER_NR].LOG_FIL); GetLocalTime(&st);
	sprintf(LOG_NAME + strlen(LOG_NAME) - 4, "-%u.%02u.%02u-ALL.LOG", st.wYear, st.wMonth, st.wDay);
	if ((f_log = fopen(LOG_NAME, "a")) == NULL) { MessageBox(NULL, "Neatsidaro LOG failas", LOG_NAME, MB_OK); exit(0); }
	fprintf(f_log, "%s", t); fclose(f_log);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance;
	INITCOMMONCONTROLSEX InitCtrlEx;
	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;	
	InitCtrlEx.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&InitCtrlEx);

	CreateMutex(NULL, FALSE, "EDI_iiko_2024"); 
	if (GetLastError() == ERROR_ALREADY_EXISTS)		// Nepaleisti programos vykdymui antrà kartà
	{
		//MessageBox(NULL, "'EDI_iiko_2024.exe' jau veikia", "Klaida", MB_OK);
		exit(0);
	}

	int i; char* adr;
	if ((BUF = (char*)malloc(500000)) == NULL) { MessageBox(NULL, "Nepavyko ið­skirti atmintá BUF", "Klaida", MB_OK); exit(0); }
	if ((data = (char*)malloc(1500000)) == NULL) { MessageBox(NULL, "Nepavyko i­ðskirti atmintá 'data'", "Klaida", MB_OK); exit(0); }
	if ((logs = (char*)malloc(1500000)) == NULL) { MessageBox(NULL, "Nepavyko i­ðskirti atmintá 'logs'", "Klaida", MB_OK); exit(0); }

	/***********************************************************************************************************************************/
	/*                                                 Nuskaitome konfigûracijà                                                        */
	/***********************************************************************************************************************************/
	if ((fi = fopen("EDI_iiko_2024.ini", "r")) == NULL) {
		MessageBox(NULL, "Nëra konfiguracijos failo 'EDI_iiko_2024.ini'", "Klaida", MB_OK); exit(0);
	}
	for (;;) {
		fgets(BUF, 200, fi);
		if (feof(fi) || ferror(fi)) break;
		if (!memicmp(BUF, "XML_SUMA_BE_PVM", 15))
		{
			XML_SUMA_BE_PVM = true;
			continue;
		}
		BUF[0] = toupper(BUF[0]);
		for (i = 1; i < 200; i++) if (BUF[i] == '\"') BUF[i] = 0;
		adr = BUF + strlen(BUF) + 1;
		if (!memicmp(BUF, "Fresh", 5)) Fresh = 1;
		if (!isdigit(BUF[1])) continue;
		USER_NR = BUF[1] - '0';
		if (*BUF == 'L') strcpy(USER[USER_NR].LOG_FIL, adr);
		if (*BUF == 'F') strcpy(USER[USER_NR].INP_DIR, adr);
		if (*BUF == 'H') strcpy(USER[USER_NR].HOST, adr);
		if (*BUF == 'K') {							// Patikriname, ar teisinga KEY struktura
			for (i = 0; i < 40; i++) {
				if (isdigit(adr[i])) continue;
				adr[i] = tolower(adr[i]); if (adr[i] < 'a' || adr[i]>'f') break;
			}
			if (i == 40 && !adr[40]) { strcpy(USER[USER_NR].KEY, adr); continue; }
			sprintf(BUF + 100, "Konfiguracijos faile 'EDI_iiko_2024.ini' vartotojui NR %d blogas parametras KEY '%s'",
				USER_NR, adr);
			MessageBox(NULL, BUF + 100, "Klaida", MB_OK); exit(0);
		}
		if (*BUF == 'P') {
			for (i = 0; i < 5 && isdigit(adr[i]); i++) USER[USER_NR].PORT = USER[USER_NR].PORT * 10 + adr[i] - '0';
		}
	}
	fclose(fi);
	if (!USER[0].INP_DIR[0] || !USER[0].HOST[0] || !USER[0].KEY[0] || !USER[0].PORT) {
		MessageBox(NULL, "Konfiguracijos faile nenurodytas nulinis vartotojas", "Klaida", MB_OK); exit(0);
	}
	if ((fi = fopen("SAVIKAINA.INI", "r")) != NULL) { SAVIKAINA_INI = true; fclose(fi); }

	for (USER_NR = 0; USER_NR < 10; USER_NR++)
		if (strlen(USER[USER_NR].INP_DIR) == 0) continue;
		else LOG("EDI_iiko_2024 START (versija A01 2024-04-22)\n\n");

	struct _finddata_t c_file; long hFile;
	/***********************************************************************************************************************************/
	/*                                                       Apklausos ciklas                                                          */
	/***********************************************************************************************************************************/
	for (USER_NR = 0;; USER_NR = (++USER_NR) % 10) {
		if (USER_NR == 9) Sleep(60000);
		if (USER[USER_NR].INP_DIR[0] == 0) { Sleep(100); continue; }
		sprintf(BUF, "%s\\*.xml", USER[USER_NR].INP_DIR);
		hFile = -1;
		__try { hFile = _findfirst(BUF, &c_file); }
		__except (EXCEPTION_EXECUTE_HANDLER) { hFile = -1L; }
		if (hFile == -1L) { Sleep(1000); continue; }

		GetLocalTime(&st);
		for (Sleep(2000);;) {
			sprintf(CURRENT_INP, "%s\\%s", USER[USER_NR].INP_DIR, c_file.name);
			sprintf(CURRENT_OUT, "%s\\%s", USER[USER_NR].INP_DIR, c_file.name);
			sprintf(CURRENT_OUT + strlen(CURRENT_OUT) - 4, "_iiko.xml");
			LOG_2("Surastas failas ", CURRENT_INP);
			Sleep(5000);
			if (!EDI_iiko_2024_xml()) {
				sprintf(BUF, "%s\\KLAIDOS", USER[USER_NR].INP_DIR); _mkdir(BUF);
				sprintf(BUF, "%s\\KLAIDOS\\EDI_%u%02u%02u%02u%02u_%s", USER[USER_NR].INP_DIR,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, c_file.name);
				rename(CURRENT_INP, BUF);				// EDI -> KLAIDOS
				sprintf(logs, "Klaidos formuojant %s\n", CURRENT_OUT); LOG_2(logs);
				_unlink(CURRENT_OUT);
				if (_findnext(hFile, &c_file)) break; else continue;
			}

			if (!EDI_iiko_2024_web()) {
				sprintf(BUF, "%s\\KLAIDOS", USER[USER_NR].INP_DIR); _mkdir(BUF);
				sprintf(BUF, "%s\\KLAIDOS\\EDI_%u%02u%02u%02u%02u_%s", USER[USER_NR].INP_DIR,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, c_file.name);
				sprintf(logs, "Klaidos perduodant %s\n", CURRENT_OUT); LOG_2(logs);
			}
			else {
				sprintf(BUF, "%s\\KOPIJOS", USER[USER_NR].INP_DIR); _mkdir(BUF);
				sprintf(BUF, "%s\\KOPIJOS\\EDI_%u%02u%02u%02u%02u_%s", USER[USER_NR].INP_DIR,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, c_file.name);
			}
			rename(CURRENT_INP, BUF);					// EDI -> KLAIDOS/KOPIJOS
			_unlink(CURRENT_OUT);
			if (_findnext(hFile, &c_file)) break;
		}
		_findclose(hFile);
	}
	return TRUE; }

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId; PAINTSTRUCT ps; HDC hdc;
	switch (message) {
		case WM_COMMAND: wmId = LOWORD(wParam);
			switch (wmId) {
				default: return DefWindowProc(hWnd, message, wParam, lParam); }
			break;
	case WM_PAINT: hdc = BeginPaint(hWnd, &ps); EndPaint(hWnd, &ps); break;
	case WM_CLOSE: PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, message, wParam, lParam); }
	return 0;}
