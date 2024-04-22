#include "stdafx.h"
#include "stdio.h"
#include "io.h"
#include "commctrl.h"
#include "resource.h"
#include "windows.h"
#include <sys/timeb.h>
#include <time.h>
#include <direct.h>
#define _MT
#include <WinInet.h>
#include <process.h>
#pragma comment (lib, "Wininet.lib")
#pragma comment (lib, "urlmon.lib")
#pragma comment(lib, "libcmt.lib")
#pragma comment(linker, "/NODEFAULTLIB:libc.lib")

char url_key[] = "resto/api/auth?login=sps&pass=";
char url_sandel[] = "resto/api/corporation/stores/search?key=";
char url_tiekej[] = "resto/api/suppliers/search?key=";
char url_product[] = "resto/api/suppliers/";   // resto/api/suppliers/110443493/pricelist?key=d99e31ef-7fca-c56e-b823-e249baf28000
char url2[200] = "resto/api/documents/import/incomingInvoice?key=";
char url2gr[200] = "resto/api/documents/import/returnedInvoice?key=";
char url_logout[200] = "resto/api/logout?key=";
char header[] = "Content-Type:application/xml";
char TOKEN[200] = "\0", buf[5000], httpUseragent[512], SAND_KOD[100], SAND_PAV[500], TIEK_KOD[100], TIEK_PAV[500]; DWORD dwRead;
HINTERNET internet, connect, request;

extern FILE *fi, *fo;
extern bool ReturnedInvoice;
extern long TOTAL_SUM;
extern char *BUF, CURRENT_OUT[], TESTAS[], *data, *logs, LOG_NAME[];

extern struct
{
	char INP_DIR[100], HOST[100], KEY[100], LOG_FIL[120];
	unsigned int PORT;
} USER[10];
extern int USER_NR;
extern void LOG(char *t1, char *t2, char *t3, char *t4, char *t5), LOG(char *t1, char *t2, char *t3, char *t4),
            LOG(char *t1, char *t2, char *t3), LOG(char *t1, char *t2), LOG(char *t1), LOG_XML(char *t),
            LOG_2(char *t1, char *t2, char *t3, char *t4, char *t5), LOG_2(char *t1, char *t2, char *t3, char *t4),
            LOG_2(char *t1, char *t2, char *t3), LOG_2(char *t1, char *t2), LOG_2(char *t1);

void web_logout_TOKEN();
DWORD INTERNET_FLAG,
      INTERNET_FLAG_HTTP  = INTERNET_FLAG_RELOAD | INTERNET_FLAG_EXISTING_CONNECT,
      INTERNET_FLAG_HTTPS = INTERNET_FLAG_RELOAD | INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_SECURE;

int EDI_iiko_2024_TOKEN() {
	if (strlen(TOKEN) == 36) web_logout_TOKEN(); // Jei egzistuoja ankstesnis token - atsijungti

	INTERNET_FLAG = INTERNET_FLAG_HTTP;
CONNECT:
	internet = InternetOpenA(httpUseragent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (internet == NULL)
	{
		LOG("   Get_TOKEN Klaida prisijungiant prie interneto-token");
		return(0);
	}
	connect = InternetConnectA(internet,USER[USER_NR].HOST,USER[USER_NR].PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,0);
	if (connect == NULL)
	{
		InternetCloseHandle(internet);
		LOG("    Klaida interneto duomenyse-token (host ", USER[USER_NR].HOST);
		return(0);
	}
	Sleep(1000);
	sprintf(BUF, "%s%s", url_key, USER[USER_NR].KEY);
	request = HttpOpenRequestA(connect, "GET", BUF, "HTTP/1.1",	NULL, NULL,	INTERNET_FLAG, NULL); // Pagal KEY gauti TOKEN
	if (request == NULL)
	{
		InternetCloseHandle(internet);
		InternetCloseHandle(connect);
		LOG("    Klaida internete GET TOKEN ", BUF);
		return(0);
	}

	HttpSendRequestA(request, NULL, 0, NULL, 0);
	memset(TOKEN, 0, sizeof(TOKEN)); int TOKEN_SIZE;
	::InternetReadFile(request, TOKEN, sizeof(buf) - 1, &dwRead);				// SKAITOME TOKEN
	TOKEN[TOKEN_SIZE = dwRead] = 0;
	if (TOKEN_SIZE < 36)
	{
		::InternetReadFile(request, TOKEN + TOKEN_SIZE, sizeof(buf) - 1, &dwRead);
		TOKEN[TOKEN_SIZE + dwRead] = 0;
	}

	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
	if (strlen(TOKEN) != 36) {
		if (INTERNET_FLAG != INTERNET_FLAG_HTTPS)
		{
			INTERNET_FLAG = INTERNET_FLAG_HTTPS;
			goto CONNECT;
		}
		LOG("   Blogas TOKEN '", TOKEN); return(0);}
	internet = InternetOpenA(httpUseragent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (internet == NULL)
	{
		web_logout_TOKEN();
		LOG("    Klaida prisijungiant prie interneto");
		return(0);
	}
	connect = InternetConnectA(internet,USER[USER_NR].HOST,USER[USER_NR].PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,0);
	if (connect == NULL)
	{
		InternetCloseHandle(internet);
		web_logout_TOKEN();
		LOG("   Get_TOKEN Klaida interneto duomenyse");
		return(0);
	}
  	LOG("   TOKEN=", TOKEN);
	return(1);}

int EDI_iiko_2024_web_sandel(char *sandel_code, char *sandel_name) {
	int token_get_nr;
	for (token_get_nr = 0; token_get_nr < 10; token_get_nr++)
		if (EDI_iiko_2024_TOKEN() != 0) token_get_nr = 99; else Sleep(10*1000);
	if (token_get_nr < 99) return(0);
	strcpy(SAND_KOD, sandel_code);
	sprintf(BUF, "%s%s&code=%s", url_sandel, TOKEN, sandel_code);
	request = HttpOpenRequestA(connect, "GET", BUF, "HTTP/1.1", NULL, NULL,	INTERNET_FLAG, NULL);
	if (request == NULL) {
		InternetCloseHandle(internet);
		InternetCloseHandle(connect);
		web_logout_TOKEN();
		LOG("   Klaida internete GET sandel ", BUF);
		return(0);
	}
	HttpSendRequestA(request, NULL, 0, NULL, 0);
	memset(data, 0, 1500000); 
	::InternetReadFile(request, data, 1500000, &dwRead);
	data[dwRead] = 0;
    sprintf(logs,"TESTAS SANDËLIAI\nURL  %s\n%s", BUF, data); LOG(logs);
	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
	web_logout_TOKEN();
    int i, adr_ID, adr_name, siz=strlen(data), len=strlen(sandel_code);
	memset(sandel_name,0,200);
	for (i=0, adr_ID=0;; i++)
	{
		if (i > siz || !data[i]) return(0);
		if (!memicmp(data + i, "<id>", 4) && !memicmp(data + i + 40, "</id>", 5)) adr_ID = i;
		if (!memicmp(data + i, "<code>",6) && !memicmp(data + i + 6,sandel_code,len) && !memicmp(data+i+6+len, "</code>", 7) && adr_ID > 0) break;
	}
	memcpy(sandel_code, data + adr_ID + 4, 36);
	sandel_code[36] = 0;
    for (adr_name=0; !adr_name && i<siz; i++)
		if (!memicmp(data+i,"<name>",6)) adr_name = i + 6;
	if (adr_name > 0)
		for (i = adr_name; memcmp(data+i, "</name>", 7) && strlen(sandel_name) < 200 ; i++) sandel_name[strlen(sandel_name)] = data[i];
    strcpy(SAND_PAV, sandel_name);
	return(1);}

int EDI_iiko_2024_web_tiekej(char *tiekej_code) {
	int i, adr, siz, len;
	strcpy(TIEK_KOD, tiekej_code);
	EDI_iiko_2024_TOKEN();
	sprintf(BUF, "%s%s&code=%s", url_tiekej, TOKEN, tiekej_code);
	request = HttpOpenRequestA(connect, "GET", BUF, "HTTP/1.1", NULL, NULL,	INTERNET_FLAG, NULL);
	if (request == NULL)
	{
		InternetCloseHandle(internet); InternetCloseHandle(connect);
		sprintf(logs, "Klaida internete GET tiekej '%s'", BUF); LOG(logs); web_logout_TOKEN(); return(0);
	}
	HttpSendRequestA(request, NULL, 0, NULL, 0);
	memset(data, 0, 1500000); 
	::InternetReadFile(request, data, 1500000, &dwRead);
	data[dwRead] = 0;
    sprintf(logs,"TESTAS TIEKËJAI\nURL  %s\n%s", BUF, data);
	LOG(logs);
	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
	web_logout_TOKEN();
	for (i = 0, adr = 0, siz = strlen(data), len = strlen(tiekej_code);; i++)
	{
		if (i>siz || !data[i]) return(0);
		if (!memicmp(data + i, "<id>", 4) && !memicmp(data + i + 40, "</id>", 5)) adr = i;
		if (!memicmp(data + i, "<code>", 6) && !memicmp(data + i + 6, tiekej_code, len) && !memicmp(data + i + 6 + len, "</code>", 7) && adr > 0) break;
	}
	memcpy(tiekej_code, data+adr+4, 36);
	tiekej_code[36] = 0;
	memset(TIEK_PAV, 0, sizeof(TIEK_PAV));
	for (;data[adr]; adr++) if (!memicmp(data + adr, "<name>", 6)) break;
	if (data[adr])
		for (adr += 6; memcmp(data + adr, "</name>", 7) && strlen(TIEK_PAV) < sizeof(TIEK_PAV); adr++) TIEK_PAV[strlen(TIEK_PAV)] = data[adr]; 
    LOG(TIEK_PAV);
	return(1);}

int EDI_iiko_2024_web_product(char *tiekej_code) { 
	ULONG i = 0, adr = 0, siz, Read_adr;
	EDI_iiko_2024_TOKEN();
	sprintf(BUF, "%s%s/pricelist?key=%s", url_product, tiekej_code, TOKEN);
	request = HttpOpenRequestA(connect, "GET", BUF, "HTTP/1.1", NULL, NULL,	INTERNET_FLAG, NULL);
	if (request == NULL)
	{
		InternetCloseHandle(internet);
		InternetCloseHandle(connect);
		sprintf(logs, "Klaida internete GET product '%s'", BUF);
		LOG(logs);
		web_logout_TOKEN();
		return(0);
	}
	HttpSendRequestA(request, NULL, 0, NULL, 0);
	memset(data, 0, 1500000);
	sprintf(logs,"PRODUKTAI URL %s\nPRODUKTAI XML", BUF); LOG(logs);
	for (ULONG Read_adr = 0, dwRead = -1; ; Read_adr += dwRead)
	{
		::InternetReadFile(request, data + Read_adr, 1024, &dwRead);
		data[Read_adr + dwRead] = 0;
		if (dwRead <= 0) break;
		LOG_XML(data + Read_adr);
	}
	sprintf(logs,"\nXML size=%ld", Read_adr);
	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
	web_logout_TOKEN();
	//sprintf(logs,"\nREZULTATAS PRODUKTAI\n%s", data); LOG(logs);

	// Formuojame trumpà sàraðà: iðorës_kodas 0(hex) iiko_kodas+ContainerId(36b) 0(hex) - gale 00
	i = adr = 0; siz = strlen(data);
	for (memset(BUF, 0, 100), memset(buf, 0, 100); i < siz; i++)
	{
		if (!memicmp(data + i, "<nativeProductNum>", 18))
		{
			for (i += 18; i < siz && data[i] != '<'; i++) buf[46 + strlen(buf + 46)] = data[i];		  // Prekës iiko kodas        (buf->46-..)
		}
		int ii; for (ii = 46+strlen(buf+46) - 1; ii > 46; ii--)
			if (buf[ii] == ' ') buf[ii] = 0; else break;
		if (!memicmp(data + i, "<supplierProductNum>", 20) && buf[46])
		{
			for (i += 20; i < siz && data[i] != '<'; i++) BUF[strlen(BUF)] = data[i];         // Prekës iðorës (tiekëjo) kodas
		}
		for (ii = strlen(BUF) - 1; ii > 0; ii--)
			if (BUF[ii] == ' ') BUF[ii] = 0; else break;
		if (!memicmp(data+i, "<id>", 4) && buf[46] && *BUF) memcpy(buf, data+i+4, 36);		// Prekës taros ID          (buf-> 1-36)
		if (!memicmp(data+i, "<count>", 7) && *buf && *BUF) memcpy(buf+36, data+i+7, 10);	// Vienetø kiekis taroje    (buf->36-46)
		if (!memicmp(data+i, "</supplierPriceListItemDto>", 27) && buf[46] && *BUF && !*buf) memset(buf, '*', 46);
		if (*BUF && *buf && buf[36] && buf[46])
		{
			strcpy(data + adr, BUF);
			adr += strlen(BUF);
			data[adr++] = 0;
			strcpy(data + adr, buf);
			adr += strlen(buf);
			data[adr++] = 0;
			memset(BUF, 0, 100),
			memset(buf, 0 ,100);
		}
	}
	memset(data+adr, 0, 100);
	sprintf(logs, "\n");
	for (i=0; data[i] && i<100000;)
	{
		sprintf(logs, " ->%s",data+i); i+=strlen(data+i)+1;
		sprintf(logs + strlen(logs), " %s %.36s",data+i+36,data+i);
		i+=strlen(data+i)+1;
		LOG(logs);
	}
	return(1);
}

int EDI_iiko_2024_web() { int i, f_siz;

exit(0);

	EDI_iiko_2024_TOKEN();
	memset(data, 0, 1500000); 
	fi = fopen(CURRENT_OUT, "rb");
	for (f_siz=0; !feof(fi); f_siz++) data[f_siz] = fgetc(fi);   // Patalpinti failà
	fclose(fi);
	if ((data[f_siz - 1] & 255) == 255) f_siz--;
	data[f_siz] = 0;

	if (ReturnedInvoice == false) sprintf(BUF, "%s%s", url2, TOKEN); else sprintf(BUF, "%s%s", url2gr, TOKEN);
	memset(buf, 0, strlen(buf));
	request = HttpOpenRequestA(connect, "POST", BUF, "HTTP/1.1", NULL, NULL, INTERNET_FLAG, NULL);
	if (request == NULL)
	{
		InternetCloseHandle(internet);
		InternetCloseHandle(connect);
		sprintf(logs, "Klaida internete POST '%s'\n", BUF); LOG(logs);
		web_logout_TOKEN();
		return(0);
	}
	sprintf(logs ,"HEADER '%s'\nData size = %d\n", header, f_siz);
	for (i = 0; i < (f_siz - 2); i++) if (data[i] != '\r') sprintf(logs + strlen(logs), "%.1s", data + i);
	LOG(logs); 
	memset(buf, 0, 500);
	HttpSendRequestA(request, header, strlen(header), data, strlen(data));		// Perduodame duomenis
	::InternetReadFile(request, buf, 500, &dwRead);
	buf[dwRead] = 0;
	int buf_len = (int)strlen(buf);
	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
	sprintf(logs, "Perdavimo rezultatas: %s", buf);
	LOG(logs);

	web_logout_TOKEN();
	extern char DOCUMENT_NUMBER[], CURRENT_INP[];
	if (!memcmp(buf, "Wrong key value: resto.RestoException: Can't parse guid", 55))
	{
		for (i=55; i<buf_len; i++ ) if (!memcmp(buf+i, " admin ", 7)) break;
		if (i<buf_len) sprintf(logs, "NEPERDUOTA - SERVERIS NEAKTYVUS - %s\n", buf);
		else sprintf(logs, "NEPERDUOTA - BLOGAS KEY - %s\n", buf);
		LOG(logs);
		return(0);
	}
	for (i=0; i<buf_len; i++) if (!memicmp(buf+i, "<documentNumber>", 16)) break;							// Ar yra dokumento Nr ?
	if (i < buf_len)
	{
		for (i = 0; i < buf_len; i++) if (!memicmp(buf + i, "<valid>true</valid>", 19)) break;					// Ar sëkmingai ?
		if (i == buf_len) for (i = 0; i < buf_len;i++) if (!memicmp(buf + i, "in status PROCESSED", 19)) break;	// Ar negràþintas anksèiau ?
	}
	if (i == buf_len)
	{
		sprintf(logs, "NEPERDUOTA - %s\n", buf); LOG_2(logs);
		for  (i = 0; i < buf_len; i++) if (!memcmp(buf + i, "Product is not found neither by id=null nor by article=", 55)) break;
		if (i == buf_len)
		{
			sprintf(logs, "FAILAS=%s DOK=%s\n      SAND=%s(%s) TIEK=%s(%s) KLAIDA=%s\n",
				CURRENT_INP,DOCUMENT_NUMBER, SAND_KOD, SAND_PAV, TIEK_KOD, TIEK_PAV, buf);
			LOG_2(logs);
			web_logout_TOKEN();
			return(0);
		}
		memset(data, 0, 1500); 
		if ((fi = fopen(CURRENT_INP, "r")) != NULL)
		{
			for (; !feof(fi);) fgets(data + strlen(data), 1000 ,fi);
			fclose(fi);
		}
		sprintf(BUF, "<SuppliersProductCode>%s</SuppliersProductCode>", buf + 55);
		for (i = 0; i < (f_siz = strlen(data)); i++) if (!memcmp(data + i, BUF, strlen(BUF))) break;
		memset(BUF, 0, 500);
		if (i < f_siz)
		{
			for (; i < f_siz; i++) if (!memcmp(data + i, "<Description>", 13)) break;
			for (i += 13; i < f_siz; i++)
			{
				if (!memcmp(data + i, "</Description>", 14)) break;
				else
				{
					if (data[i] == 34 || data[i] == 38 || data[i] == 39 || data[i] == 60 || data[i] == 62)
						sprintf(BUF + strlen(BUF), "&#%d;", data[i]);
					else BUF[strlen(BUF)] = data[i];
				}
			}
			for (; i < f_siz; i++) if (!memcmp(data + i, "<UnitPrice>", 11)) break;
			for (i += 11, buf_len = strlen(BUF) + 1; i < f_siz; i++)
				if (!memcmp(data + i, "</UnitPrice>", 12)) break; else BUF[buf_len + strlen(BUF + buf_len)] = data[i];
		}
		sprintf(logs, "FAILAS=%s DOK=%s\n      SAND=%s(%s) TIEK=%s(%s)\n      NERASTAS KODAS=%s(%s) kaina=%s\n",
			CURRENT_INP,DOCUMENT_NUMBER, SAND_KOD, SAND_PAV, TIEK_KOD, TIEK_PAV, buf + 55, BUF, BUF + buf_len); LOG_2(logs);
		return(0);
	}
	sprintf(logs, "Perduota sëkmingai (suma=%ld.%02ld) %s\n", TOTAL_SUM/100, TOTAL_SUM%100, buf);
	LOG_2(logs);
    return(1);
}

void web_logout_TOKEN()
{
	HttpSendRequestA(request, header, strlen(header), data, strlen(data));		// LOGOUT
	internet = InternetOpenA(httpUseragent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (internet == NULL)
	{
		LOG("    Klaida prisijungiant prie interneto-logout");
		return;
	}
	connect = InternetConnectA(internet,USER[USER_NR].HOST,USER[USER_NR].PORT,NULL,NULL,INTERNET_SERVICE_HTTP,0,0);
	if (connect == NULL)
	{
		InternetCloseHandle(internet);
		LOG("Klaida interneto duomenyse logout");
		return;
	}
	sprintf(BUF, "%s%s", url_logout, TOKEN);
	request = HttpOpenRequestA(connect, "GET", BUF, "HTTP/1.1", NULL, NULL, INTERNET_FLAG, NULL);
	if (request == NULL)
	{
		InternetCloseHandle(internet);
		InternetCloseHandle(connect);
		return;
	}
	HttpSendRequestA(request, NULL, 0, NULL, 0);
	::InternetReadFile(request, data, 500000, &dwRead);
	InternetCloseHandle(request);
	InternetCloseHandle(connect);
	InternetCloseHandle(internet);
	sprintf(logs, "CLOSE-TOKEN=%s", TOKEN);
	LOG(logs);
	*TOKEN = 0;
}