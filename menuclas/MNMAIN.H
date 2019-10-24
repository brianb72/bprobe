#ifndef __MNMAIN_H__
#define __MNMAIN_H__


#include "menuclas/keyinput.h"
#include <UserIO/TxtVid.h>
#include <UserIO/DropBox.h>
#include <UserIO/EntryBox.h>
#include <util/printer.h>


/* Main menu class. */
class cMnMain : cTextVideo, cKeyInput {
public:
	cMnMain();
	~cMnMain();
public:
	void Main();
	void SpawnMenu(int enter);
	void PrintReport(int port);
	void Report();
	void SetupReport();
	void SaveSettings();
	int aprint(char *fmt, ...);
private:
#define T_FILE  1
#define T_PRINT 2
	int target;
	int selected;
	FILE *fp;               // Output file pointer
	FILE *inf;  			// Input config file pointer
	cDropBox *oDrop[10];
	cEntryBox *oText[5];
	cPrinter *LPT;
};



#endif
