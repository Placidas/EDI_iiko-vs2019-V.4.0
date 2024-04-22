// Prekë randama pagal <productArticle>
// Bûtinai skaièius <num>
// Dokumento numeris <documentNumber>
#include "stdafx.h"
#include "stdio.h"
#include "io.h"
#include "commctrl.h"
#include "resource.h"
#include "windows.h"
#include <sys/timeb.h>
#include <time.h>
#include <direct.h>
#include <string>
#define _MT
#include <process.h>
#pragma comment(lib, "libcmt.lib")
#pragma comment(linker, "/NODEFAULTLIB:libc.lib")

extern FILE *fi, *fo; extern int USER_NR; extern bool ReturnedInvoice, SAVIKAINA_INI;
char KONCEPCIJA[100], DOCUMENT_NUMBER[100];
extern char *BUF, CURRENT_INP[], CURRENT_OUT[], TESTAS[], *data, *logs; extern long TOTAL_SUM;
extern struct { char INP_DIR[100], HOST[41], KEY[41], LOG_FIL[120]; unsigned int PORT;} USER[10];
extern void LOG(char *t1, char *t2, char *t3, char *t4, char *t5), LOG(char *t1, char *t2, char *t3, char *t4),
            LOG(char *t1, char *t2, char *t3), LOG(char *t1, char *t2), LOG(char *t1),
            LOG_2(char *t1, char *t2, char *t3, char *t4, char *t5), LOG_2(char *t1, char *t2, char *t3, char *t4),
            LOG_2(char *t1, char *t2, char *t3), LOG_2(char *t1, char *t2), LOG_2(char *t1);

long EDI_RECS, iiko_RECS, ITEM_COUNT, FI_ADR; int HEADER_STATE, ITEM_STATE;
struct {
	char encoding[200],
		/*InvoiceNumber*/               invoice[200],
		/*InvoiceDate*/                 dateIncoming[200],
		/*InvoicePaymentDueDate*/       dueDate[200],
		/*DocumentNameCode*/			documentNumber[200],
		/*Remarks*/                     comment[200],
		/*BuyerOrderNumber*/
		/*SellerOrderNumber*/           incomingDocumentNumber[200],
		/*Reference-Date*/
		/*DeliveryLocationNumber*/      store[200], store_name[200],
		/*CodeBySeller*/
		/*DeliveryDate*/
		/*DespatchNumber*/
		/*Sender_ILN     1*/            supplier[200], supplier_code[200],
		/*Receiver_ILN   2*/
		/*Buyer_ILN      3*/
		/*Buyer_TaxID    3*/
		/*Payer_ILN      4*/
		/*Payer_TaxID    4*/
		/*Seller_ILN     5*/
		/*Seller_TaxID   5*/
		/*Payee_ILN      6*/
		/*Payee_TaxID    6*/
		/*SourceDocumentNo*/            SourceDocumentNo[200],
		/*SourceDocumentDate*/          SourceDocumentDate[20];
} XML_HEADER;

struct {
	char
		/*LineNumber*/				num[200],
		/*EAN/*/
		/*BuyerItemCode/*/
		/*SupplierItemCode*/		product[200],			// supplierProduct
		/*ItemDescription*/			productArticle[200],	// supplierProductArticle
		/*InvoiceQuantity*/			amount[200],			// actualAmount
		/*InvoiceUnitNetPrice*/     price[200],
		/*Discount*/				discountSum[200],
		/*InvoiceUnitBasePrice*/
		/*InvoiceUnitGrossPrice*/
		/*UnitOfMeasure*/			amountUnit[200],
		/*InvoicedUnitPackSize*/
		/*TaxRate*/					vatPercent[200],
		/*<TaxCategoryCode>S</TaxCategoryCode>*/
		/*TaxAmount>*/				vatSum[200],
		/*NetAmount*/				sum[200];
} XML_ITEM;

int FIND(char *a) {
	char BEG[200], END[200]; int adr, i;
	sprintf(BEG, "<%s>", a);
	sprintf(END, "</%s>", a);
	for (adr = 0; adr < 500 && BUF[adr]; adr++)
		if (!memicmp(BUF+adr, BEG, strlen(BEG))) break; 
	if (memicmp(BUF + adr, BEG, strlen(BEG))) return(0);
	for (i = adr; i < 500 && (BUF[i] & 255) >= 32; i++)
		if (!memicmp(BUF + i, END, strlen(END))) break;
	if (i == (int)strlen(BEG)) memcpy(BUF + i, "\xFF\0", 2); else BUF[i] = 0;
	adr += strlen(BEG);
	if (BUF[adr] == '-' && isdigit(BUF[adr+1]))		 // Kreditinës sàskaitos (sumos neigiamos)
	{
		adr++;
		ReturnedInvoice = true;
	}
	return(adr); }

int GET_LINE() { int i, f1_end, f2_beg; EDI_RECS++;
	memset(BUF, 0, 50000); fseek(fi, FI_ADR, SEEK_SET);
	for (; !feof(fi) && *BUF != '<'; FI_ADR++) *BUF = fgetc(fi);
	for (f1_end = f2_beg = 0, i = 1; i < 500 && !feof(fi); i++)
	{
		if ((BUF[i] = fgetc(fi)) == 0x0A)
		{
			FI_ADR += i;
			break;
		}
		if (!f1_end)
		{
			if (BUF[i] != '>') continue; 
			if (BUF[1] == '/')
			{ 
				FI_ADR += i;
				break;
			}
			f1_end = i;
		}
		if (!f2_beg)
		{
			if (BUF[i] == '<') f2_beg = i;
			continue;
		}
		if ((i - f2_beg) == 1) if (BUF[i] == '/') continue;
		else
		{
			FI_ADR += (f2_beg - 1);
			break;
		}
		if (BUF[i] != '>') continue;
		FI_ADR += i; break; }
	if (feof(fi)) return 0;
	for (i=0; i<500; i++) if ((BUF[i] & 255) < 32) break;
	BUF[i]=0;
	return 1;
}

__int64 GET_NUM2(char *adr)			// Du þenklai po kablelio
{
	__int64 l;
	int i;
	for (l=i=0; i<10 && isdigit(adr[i]); i++) l = l*10 + adr[i]-'0';
	l=l*10000;
	if (adr[i] != '.' && adr[i] != ',') return(l);
	if (!isdigit(adr[++i])) return(l);
	l += (adr[i]-'0')*1000;
	if (!isdigit(adr[++i])) return(l);
	l += (adr[i]-'0')*100;
	if (!isdigit(adr[++i])) return(l);
	if (adr[i] >= '5') l += 100;
	//l += (adr[i]-'0')*10; if (!isdigit(adr[++i])) return(l);
	//l += (adr[i]-'0');
	return(l);
}

__int64 GET_NUM4(char *adr)			// Keturi þenklai po kablelio
{
	__int64 l;
	int i;
	for (l=i=0; i<10 && isdigit(adr[i]); i++) l = l*10 + adr[i]-'0';
	l=l*10000;
	if (adr[i] != '.' && adr[i] != ',') return(l);
	if (!isdigit(adr[++i])) return(l);
	l += (adr[i]-'0')*1000;
	if (!isdigit(adr[++i])) return(l);
	l += (adr[i]-'0')*100;
	if (!isdigit(adr[++i])) return(l);
	l += (adr[i]-'0')*10;
	if (!isdigit(adr[++i])) return(l);
	l += (adr[i]-'0');
	return(l);
}

void ZIN(char *code) {
	extern char *data; int siz;
	for (int i = 0; data[i]; i += (siz + strlen(data+i+siz) + 1)) {
		siz = strlen(data+i) + 1; if (strcmp(code, data+i)) continue;
		strcpy(code, data+i+siz); break;}}

int EDI_iiko_2024_xml() {
	int i, adr;
	EDI_RECS = iiko_RECS = ITEM_COUNT = FI_ADR = 0;
	HEADER_STATE = ITEM_STATE = 0;
	if ((fi = fopen(CURRENT_INP, "rb")) == NULL)
	{
		sprintf(logs, "Neatsidaro EDI failas '%s'", CURRENT_INP);
		LOG_2(logs);
		return(0);
	}

	/* Pereiname per EDI header'á ir randame pagrindinius laukus */
	ReturnedInvoice = false;
	for (memset(XML_HEADER.encoding, 0, sizeof(XML_HEADER)), HEADER_STATE = ITEM_STATE = FI_ADR = 0; !feof(fi);) {
		if (!GET_LINE()) break;
		for (i = 0; i < 200 && BUF[i]; i++)
		{
			if (!memicmp(BUF + i, "encoding", 8))
			{
				strcpy(XML_HEADER.encoding, BUF);
				break;
			}
		}
		if (adr = FIND("BuyersOrderNumber")) strcpy(XML_HEADER.invoice, BUF + adr);
		if (adr = FIND("InvoiceDate"))
		{
			strcpy(XML_HEADER.dateIncoming,BUF+adr);
			XML_HEADER.dateIncoming[4] = XML_HEADER.dateIncoming[7] = '-';
		}
		if (adr = FIND("dueDate"))
		{
			strcpy(XML_HEADER.dueDate,BUF+adr);
			XML_HEADER.dueDate[4] = XML_HEADER.dueDate[7] = '-';
		}
		if (adr = FIND("BuyersCodeForSupplier")) strcpy(XML_HEADER.supplier, BUF + adr);
		if (adr = FIND("DeliveryLocationNumber")) strcpy(XML_HEADER.store, BUF + adr);
		
		// GRÀÞINAMOS SÀSKAITOS
		if (adr = FIND("SubType"))
			if (!strcmp("CREDITINVOICE", BUF + adr))
				ReturnedInvoice = true;
		if (adr = FIND("SourceDocumentNo"))
		{
			ReturnedInvoice = true;
			strcpy(XML_HEADER.SourceDocumentNo, BUF + adr);
		}
		if (adr = FIND("SourceDocumentDate"))
		{
			ReturnedInvoice = true;
			strcpy(XML_HEADER.SourceDocumentDate, BUF + adr);
		}
		if (FIND("InvoiceLine"))
		{
			ITEM_STATE = 1;
			break;
		}
	}

	if (!*XML_HEADER.invoice || !*XML_HEADER.dateIncoming || !*XML_HEADER.supplier || !*XML_HEADER.store)
	{
		fclose(fi);
		if (!*XML_HEADER.store)       LOG_2("Trûksta HEADER duomenø: DeliveryLocationNumber");
		if (!*XML_HEADER.supplier)    LOG_2("Trûksta HEADER duomenø: BuyersCodeForSupplier");
		if (!*XML_HEADER.invoice)     LOG_2("Trûksta HEADER duomenø: InvoiceNumber");
		if (!*XML_HEADER.dateIncoming)LOG_2("Trûksta HEADER duomenø: InvoiceDate");
		return(0);
	}
	if (ReturnedInvoice == true && !*XML_HEADER.SourceDocumentNo)
	{
		fclose(fi);
		sprintf(logs, "Gràþinamai sàskaitai '%s' trûksta 'SourceDocumentNo'", XML_HEADER.invoice);
		LOG_2(logs);
		return(0);
	}
	if (ReturnedInvoice == true && !*XML_HEADER.SourceDocumentDate)
	{
		fclose(fi);
		sprintf(logs, "Gràþinamai sàskaitai '%s' trûksta 'SourceDocumentDate'", XML_HEADER.invoice);
		LOG_2(logs);
		return(0);
	}

	extern int EDI_iiko_2024_web_sandel(char *sandel_code, char *sandel_name);
////Sand + KONCEPCIJA
	FILE *fSand; static char Sandeliai[5000];
	memset(Sandeliai, 0, sizeof(Sandeliai));
	if ((fSand = fopen("Sandeliai.txt", "r")) == NULL)
	{
		LOG_2("Neatsidaro failas 'Sandeliai.txt'");
		return(0);
	}
	for (;!feof(fSand);)
		fgets(Sandeliai + strlen(Sandeliai), 200, fSand);
	fclose(fSand);
	int siz, j;
	for (siz = strlen(XML_HEADER.store), i = 0; Sandeliai[i]; )
	{ 
		for (j = i; Sandeliai[j] && Sandeliai[j] != ';' ; j++); j++;
		if ((!memcmp(Sandeliai + i, XML_HEADER.store, siz) && Sandeliai[i + siz] == ';') ||
		    (!memcmp(Sandeliai + j, XML_HEADER.store, siz) && Sandeliai[j + siz] == ';'))
		{
			memset(XML_HEADER.store, 0, sizeof(XML_HEADER.store)); memset(KONCEPCIJA, 0, sizeof(KONCEPCIJA));
			for (i = j; (Sandeliai[i] & 255) >= 32 && Sandeliai[i] != ';'; i++)
				XML_HEADER.store[strlen(XML_HEADER.store)] = Sandeliai[i];
			for (i++; (Sandeliai[i] & 255) >= 32; i++)
				KONCEPCIJA[strlen(KONCEPCIJA)] = Sandeliai[i];
			break;
		}
		for (;;) if ((Sandeliai[i++] & 255) < 32) break;
	}
////Sand
	if (!EDI_iiko_2024_web_sandel(XML_HEADER.store, XML_HEADER.store_name))
	{
		fclose(fi);
		sprintf(logs, "FAILAS='%s' nerastas sandëlys '%s'\n", CURRENT_INP, XML_HEADER.store);
		LOG_2(logs);
		return(0);
	}
	extern char Fresh;
	if (Fresh) for (i=0; XML_HEADER.supplier[i]; i++)
		XML_HEADER.supplier[i] = (XML_HEADER.supplier[i+2]==0) ? 0 : XML_HEADER.supplier[i+1];
	extern int EDI_iiko_2024_web_tiekej(char *tiekej_code);
	strcpy(XML_HEADER.supplier_code, XML_HEADER.supplier);

	if (!EDI_iiko_2024_web_tiekej(XML_HEADER.supplier))
	{
		fclose(fi);
		sprintf(logs, "FAILAS='%s' nerastas tiekëjas '%s'\n", CURRENT_INP, XML_HEADER.supplier);
		LOG_2(logs);
		return(0);
	}

	extern int EDI_iiko_2024_web_product(char *tiekej_code);
	EDI_iiko_2024_web_product(XML_HEADER.supplier_code);

	if ((fo = fopen(CURRENT_OUT, "w")) == NULL)
	{
		sprintf(logs, "Neatsidaro iiko failas raðymui '%s'", CURRENT_OUT);
		LOG_2(logs);
		return(0);
	}
	if (*XML_HEADER.encoding)
	{
		for (i=adr=0; i < (int)strlen(XML_HEADER.encoding); i++) if (XML_HEADER.encoding[i] == '>') adr = i;
		XML_HEADER.encoding[adr + 1] = 0;
		fprintf(fo, "%s\n", XML_HEADER.encoding);
	}

	TOTAL_SUM = 0;
	iiko_RECS++; fprintf(fo, "<document>\n");
	/********************************************************************************************************************/
	/**                                                                     H E A D E R                                 */
	/********************************************************************************************************************/
	if (ReturnedInvoice == false)
										/********************************************************************************/
										/**                               Paprasta sàskaita                             */
	{									/********************************************************************************/
		iiko_RECS++; fprintf(fo, "  <conception>%s</conception>\n", KONCEPCIJA);
		iiko_RECS++; fprintf(fo, "  <comment>%s</comment>\n", XML_HEADER.invoice);
		iiko_RECS++; fprintf(fo, "  <documentNumber>%s</documentNumber>\n", XML_HEADER.invoice);   // XML_HEADER.documentNumber);
		iiko_RECS++; fprintf(fo, "  <dateIncoming>%sT00:00:00</dateIncoming>\n", XML_HEADER.dateIncoming);
		iiko_RECS++; fprintf(fo, "  <invoice>%s</invoice>\n", XML_HEADER.invoice);
		iiko_RECS++; fprintf(fo, "  <defaultStore>%s</defaultStore>\n", XML_HEADER.store);
		iiko_RECS++; fprintf(fo, "  <supplier>%s</supplier>\n", XML_HEADER.supplier);
		iiko_RECS++; if (strlen(XML_HEADER.dueDate) > 0) fprintf(fo, "  <dueDate>%sT00:00:00</dueDate>\n", XML_HEADER.dueDate);
			                                        else fprintf(fo, "  <dueDate>%sT00:00:00</dueDate>\n", XML_HEADER.dateIncoming);
		iiko_RECS++; fprintf(fo, "  <incomingDate/>\n");
		iiko_RECS++; fprintf(fo, "  <useDefaultDocumentTime>true</useDefaultDocumentTime>\n");
		iiko_RECS++; fprintf(fo, "  <incomingDocumentNumber></incomingDocumentNumber>\n",XML_HEADER.invoice);//HEADER.incomingDocumentNumber
		iiko_RECS++; fprintf(fo, "  <employeePassToAccount/>\n");
		iiko_RECS++; fprintf(fo, "  <transportInvoiceNumber/>\n");
		iiko_RECS++; fprintf(fo, "  <documentStatus>PROCESSED</documentStatus>\n");
	}
	else
										/********************************************************************************/
										/**                               Gràþinimo sàskaita                            */
	{									/********************************************************************************/
		iiko_RECS++; fprintf(fo, "  <conceptionId>%s</conceptionId>\n", KONCEPCIJA);
		iiko_RECS++; fprintf(fo, "  <comment>%s</comment>\n", XML_HEADER.invoice);
		iiko_RECS++; fprintf(fo, "  <documentNumber>%s</documentNumber>\n", XML_HEADER.invoice);
		iiko_RECS++; fprintf(fo, "  <defaultStore>%s</defaultStore>\n", XML_HEADER.store);
		iiko_RECS++; fprintf(fo, "  <dateIncoming>%sT00:00:00</dateIncoming>\n", XML_HEADER.dateIncoming);
		iiko_RECS++; fprintf(fo, "  <status>PROCESSED</status>\n");
		iiko_RECS++; fprintf(fo, "  <incomingInvoiceNumber>%s</incomingInvoiceNumber>\n", XML_HEADER.SourceDocumentNo);
		iiko_RECS++; fprintf(fo, "  <incomingInvoiceDate>%s</incomingInvoiceDate>\n", XML_HEADER.SourceDocumentDate);
		iiko_RECS++; fprintf(fo, "  <counteragentId>%s</counteragentId>\n", XML_HEADER.supplier);
	}
	iiko_RECS++; fprintf(fo, "  <items>\n");
	/********************************************************************************************************************/
	/**                                                                     I T E M S                                   */
	/********************************************************************************************************************/
	for (int LINE_NUMBER = 0; !feof(fi);)
	{
		memset(XML_ITEM.num, 0, sizeof(XML_ITEM));
		for (; !feof(fi);)
		{
			if (!GET_LINE()) break;
			if (FIND("InvoiceLine")) ITEM_STATE = 1;
			if (ITEM_STATE == 0) continue;
			if (adr=FIND("SuppliersProductCode"))	strcpy(XML_ITEM.product, BUF + adr);
			if (adr=FIND("Description"))			strcpy(XML_ITEM.productArticle,BUF+adr);
			if (adr=FIND("Amount"))					strcpy(XML_ITEM.amount, BUF + adr);
			if (adr=FIND("UnitPrice"))				strcpy(XML_ITEM.price, BUF + adr);
			if (adr=FIND("Percentage"))		  		strcpy(XML_ITEM.discountSum, BUF + adr);
			if (adr=FIND("TaxValue"))				strcpy(XML_ITEM.vatSum, BUF + adr);
			if (adr=FIND("LineTotal"))		  		strcpy(XML_ITEM.sum, BUF + adr);
			if (adr=FIND("TaxRate"))		  		strcpy(XML_ITEM.vatPercent, BUF + adr);
			if (FIND("/InvoiceLine")) break;
		}
		if (ITEM_STATE == 0) continue;
		if (strlen(XML_ITEM.product) < 1) continue;
		
		if (!*XML_ITEM.num && !*XML_ITEM.product && !*XML_ITEM.productArticle && !*XML_ITEM.amount && !*XML_ITEM.discountSum &&
			!*XML_ITEM.amountUnit && !*XML_ITEM.vatSum && !*XML_ITEM.sum)
		{
			ITEM_STATE = 0;
			continue;
		}
		sprintf(XML_ITEM.num, "%d", ++LINE_NUMBER);

		__int64 L, LL, L_sum, L_sumBe, L_vatSum, L_amount, L_PVM, L_amount_pak;
		L_sum = L_sumBe = GET_NUM4(XML_ITEM.sum);
		L_vatSum = GET_NUM4(XML_ITEM.vatSum);
		L_amount_pak = L_amount = GET_NUM4(XML_ITEM.amount);

		/************ SUMOS BE PVM - PERSKAIÈIUOTI ***********************************/
		char sumWithoutNDS[100]; strcpy(sumWithoutNDS, XML_ITEM.sum);
		if (L_vatSum > 0) sprintf(XML_ITEM.sum,"%.04f",((double)(L_sum = L_sumBe + L_vatSum)) / 10000);

		/************ PASKAIÈIUOTI KAINÀ (EDI vieneto kaina neatsiþvelgia á nuolaidà) *******/
		L = (L_sumBe*100000) / L_amount; if ((L%10)<5) L=L/10; else L=L/10+1;
		sprintf(XML_ITEM.price, "%.04f", ((double)L)/10000);

		/************ GAUTI iiko PREKËS KODÀ PAGAL IÐORINÁ TIEKËJO KODÀ *************/
		ZIN(XML_ITEM.product);
		if (XML_ITEM.product[0] != '*' && strlen(XML_ITEM.product) > 46) {				// Perskaièiuojame kieká pagal tarà
			LL=GET_NUM4(XML_ITEM.product+36);
			L_amount = (L_amount * LL) / 10000;
		 	if ((L_amount%10)<5) L_amount=L_amount/10; else L_amount=L_amount/10+1;	// Apvaliname
		 	sprintf(XML_ITEM.amount,"%ld.",L_amount/1000); sprintf(XML_ITEM.amount+strlen(XML_ITEM.amount),"%03ld",L_amount%1000);}

		iiko_RECS++; fprintf(fo, "    <item>\n");
		{
			iiko_RECS++; fprintf(fo, "      <amount>%s</amount>\n", XML_ITEM.amount);
			{
				// iiko_RECS++; fprintf(fo, "      <supplierProduct>%s</supplierProduct>\n", XML_ITEM.product);
				// iiko_RECS++; fprintf(fo, "      <supplierProductArticle>%s</supplierProductArticle>\n", XML_ITEM.product);
				// iiko_RECS++; fprintf(fo, "      <product>%s</product>\n", XML_ITEM.product); // Èia iiko vidinis kodas
				iiko_RECS++;
				if (strlen(XML_ITEM.product) < 47) fprintf(fo, "      <productArticle>%s</productArticle>\n", XML_ITEM.product);
				else fprintf(fo, "      <productArticle>%s</productArticle>\n", XML_ITEM.product + 46);
			}
			iiko_RECS++; fprintf(fo, "      <producer/>\n");
			iiko_RECS++; fprintf(fo, "      <num>%s</num>\n", XML_ITEM.num);
			iiko_RECS++; if (XML_ITEM.product[0] == '*' || strlen(XML_ITEM.product) < 47) fprintf(fo, "      <containerId/>\n");
			else fprintf(fo, "      <containerId>%.36s</containerId>\n", XML_ITEM.product);
			
			// iiko_RECS++; fprintf(fo, "      <amountUnit>%s</amountUnit>\n", XML_ITEM.amountUnit);
			iiko_RECS++; fprintf(fo, "      <actualUnitWeight/>\n");
			extern bool XML_SUMA_BE_PVM;
			if (XML_SUMA_BE_PVM == true && ReturnedInvoice == false) {
				iiko_RECS++; fprintf(fo, "      <sum>%s</sum>\n", XML_ITEM.sum);
				iiko_RECS++; fprintf(fo, "      <sumWithoutNds>%s</sumWithoutNds>\n", sumWithoutNDS);
				iiko_RECS++; fprintf(fo, "      <price>%s</price>\n", XML_ITEM.price);
			}
			iiko_RECS++; fprintf(fo, "      <discountSum>%s</discountSum>\n", XML_ITEM.discountSum);
			/********** PVM reikia nurodyti: tarifà ir PVM sumà, arba nieko (tada tarifà ima ið prekës kortelës ir paskaièiuoja PVM sumà) *************/
			{
				if (L_vatSum == 0)
				{
					iiko_RECS++; fprintf(fo, "      <vatPercent>0</vatPercent>\n");
					iiko_RECS++; fprintf(fo, "      <vatSum>0</vatSum>\n");
				}
				else
				{
					L_PVM = (L_vatSum * 10000) / L_sumBe;
					/* PVM mustatyti 21 % arba 9 % */
					// if (vatPercent < 1500) vatPercent = 900; else vatPercent = 2100;
					iiko_RECS++; fprintf(fo, "      <vatPercent>%.2f</vatPercent>\n", (double)L_PVM / 100);
					iiko_RECS++; fprintf(fo, "      <vatSum>%.4f</vatSum>\n", (double)L_vatSum / 10000);
				}
			}
			iiko_RECS++; fprintf(fo, "      <code/>\n");
			TOTAL_SUM += (long)(L_sum / 100);
			if (ReturnedInvoice == false) {
				iiko_RECS++; fprintf(fo, "      <store>%s</store>\n", XML_HEADER.store);
				/**/ iiko_RECS++; fprintf(fo, "      <sumWithoutVat>%s</sumWithoutVat>\n", sumWithoutNDS);
				/**/ iiko_RECS++; fprintf(fo, "      <sum>%s</sum>\n", XML_ITEM.sum);
				if (SAVIKAINA_INI == false)
				{
					iiko_RECS++; fprintf(fo, "      <priceUnit/>\n");
					if (XML_SUMA_BE_PVM == false) {
						iiko_RECS++; fprintf(fo, "      <priceWithoutVat>%s</priceWithoutVat>\n", XML_ITEM.price);
					}
				}
				else
				{
					iiko_RECS++; fprintf(fo, "      <price>0</price>\n");
					iiko_RECS++; fprintf(fo, "      <sum>%s</sum>\n", XML_ITEM.sum);
				}
			}
			else
			{
				iiko_RECS++; fprintf(fo, "      <storeId>%s</storeId>\n", XML_HEADER.store);
				iiko_RECS++; fprintf(fo, "      <priceUnit/>\n");
				iiko_RECS++; fprintf(fo, "      <sum>%s</sum>\n", XML_ITEM.sum);
				iiko_RECS++; fprintf(fo, "      <price>%s</price>\n", XML_ITEM.price);
			}
			iiko_RECS++; fprintf(fo, "      <customsDeclarationNumber/>\n");
			iiko_RECS++; fprintf(fo, "      <actualAmount>%s</actualAmount>\n", XML_ITEM.amount);
			iiko_RECS++; fprintf(fo, "    </item>\n");
		}
		ITEM_COUNT++; ITEM_STATE = 0;
	}
	iiko_RECS++; fprintf(fo, "  </items>\n");
	iiko_RECS++; fprintf(fo, "</document>\n");
	fclose(fi);
	fclose(fo);
	
	if (ReturnedInvoice == true && !*XML_HEADER.SourceDocumentNo)
	{
		sprintf(logs, "Gràþinamai sàskaitai '%s' trûksta 'SourceDocumentNo'", XML_HEADER.invoice);
		LOG_2(logs);
		return(0);
	}
	if (ReturnedInvoice == true && !*XML_HEADER.SourceDocumentDate)
	{
		sprintf(logs, "%Gràþinamai sàskaitai '%s' trûksta 'SourceDocumentDate'\n", XML_HEADER.invoice);
		LOG_2(logs);
		return(0);
	}

	sprintf(logs, "Suformuotas '%s' EDI(eil.%d) iiko(eil.%d) prekiø %d", CURRENT_INP, EDI_RECS, iiko_RECS, ITEM_COUNT);
	LOG_2(logs);
	strcpy(DOCUMENT_NUMBER, XML_HEADER.invoice);
	return(1);
}