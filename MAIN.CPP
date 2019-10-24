#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <conio.h>
#include <time.h>
#include <dos.h>

#include "declare/global.h"
#include "menuclas/MnMain.h"
#include "misc/Log.h"
#include "misc/int24/int24.h"

/* Just a main wrapper here. Init some global config variables
   and then call the toolbar                                    */

/* Global list of objects - Passed to all files */
cLog    *oLog = new cLog;   /* The global log object */
cThrob  *oThrob = new cThrob;  // Throbber

// MI CR OM OD UL ES   // cipher word
// IM RC MO DO LU SE
// 3E lU @O M0 R< 1M

// BPROBE should refuse to run unless bsig[0] is 3.
char bsig[50] = { "4ElU@OM0R<1M" };

char serialnumber[50] = { "ABCD-EFGH-IJKL-MNOP" };
int g_delay_method = 0;

// Pass these two by reference
int parse_cmd(int argc, char **argv) {
   int c;
   int i;
   int pos = 1;


   /* Parse command line arguements */
   while ((pos < argc) && ( (*(argv[pos]) == '-') || (*(argv[pos]) == '/') ) ) {
	  switch ( *(argv[pos]+1)) {   // get around
		 case 's':
		 case 'S':  // for backwards compatability
		 case 'v': // Dump serial number and exit
		 case 'V':
			FILE *fp = NULL;
			printf("BPROBE Version  : " VERSION "\n");
			printf("Serial Number   : %s\n", serialnumber);
			if ((fp = fopen("pci.dat", "r+b")) == NULL) {
			printf("PCI.DAT Version : Could not open!\n");
			}
			else {
			   int t;
			   fread(&t, sizeof(t), 1, fp);  // the file version
			printf("PCI.DAT Version : v%X\n", t);
			}
			printf("\nVisit http://www.aaaaaaaaaa.com/bprobe to register your serial number\n");
			printf("and download updates.\n");
			return 0;
		 case 't':
		 case 'T':
				   if (*(argv[pos]+2) == '1') {
					  g_delay_method = 1;
					  oLog->Log("ParseArgs - Switching to delay method 1");
					  break;
				   }
				   printf("Invalid argument '-d%i'\n", *(argv[pos]+2));
				   return 0;
		 case 'L': oLog->OpenLog("log.txt");
				   oLog->SetPriority(3);
				   break;
		 default: printf("\n"
					"Usage: %s <switches>\n"
					"-V  Show version and serial number.\n"
					"-T1 Use alternate delay method.\n"
					, argv[0]);
					exit(1);
	  }
	  ++pos;
   }

   return 1;
}



int main(int argc, char **argv) {
	cMnMain *oMain = new cMnMain;  /* The main toolbar menu */

	if (bsig[0] != '3') { return 0; }
	if (!parse_cmd(argc, argv)) return 0;

//	outp(0x43, 0x34);
//	outp(0x40, 0xFF);
//	outp(0x40, 0xFF);


	install_int24_hnd();
	_setcursortype(_NOCURSOR);

	oMain->Main();
	_setcursortype(_NORMALCURSOR);

	oLog->CloseLog();

	delete oMain;
	delete oLog;
	remove_int24_hnd();


	return 1;

}





