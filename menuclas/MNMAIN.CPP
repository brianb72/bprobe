#include <stdio.h>       
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdarg.h>

#include "declare/global.h"

#include "MenuClas/MnMain.h"
#include "menuclas/MnSysIn.h"
#include "menuclas/MnOverVi.h"
#include "menuclas/MnPCIBus.h"
#include "menuclas/MnDrv.h"
#include "menuclas/MnDmi.h"
#include "util/printer.h"

cMnOverView *oMnOverView;
cMnDrv *oMnDrv;
cMnPCIBus *oMnPCIBus;
cMnDMI *oMnDMI;


cMnMain::cMnMain() {
   int i;
   target = T_PRINT;
   fp = NULL;
   inf = NULL;
   LPT = new cPrinter;
   for (i = 0; i < 10; ++i) oDrop[i] = new cDropBox;
   for (i = 0; i < 4; ++i)  oText[i] = new cEntryBox;
}

cMnMain::~cMnMain() {
}


/* Called to start the menu system. When we return the program exits.
   We own y = 1. Handle the toolbar and call the appropriate class */
void cMnMain::Main() {
#define MAX_MM 3
   struct menu_data { int start; int length; } menustruct[] = {
	{ 3, 7 },  { 15, 6 }, { 26, 7 }, { 38, 10 } };
   int x, y;
   int exitflag = FALSE;
   char c;
   int oldtextattr;
   struct text_info ti;

   gettextinfo(&ti);
   oldtextattr = ti.attribute;


   textattr(COLOR(WHITE, BLACK));
   oThrob->SetColor(COLOR(WHITE,BLACK));
   /* Start by clearing the screen */
   for (y = 1; y <= 25; ++y)
	   for (x = 1; x <= 80; ++x)
		   PutChar(x, y, ' ', COLOR(WHITE, BLACK));

   gotoxy(1,4);
   oMnOverView = new cMnOverView;
   oMnDrv = new cMnDrv;
   oMnPCIBus = new cMnPCIBus;
   oMnDMI = new cMnDMI;

   gotoxy(32, 1);
   cprintf("[ BPROBE v2.1 ]");
   HighLight(1, 1, 80, COLOR(WHITE, BLUE));
   gotoxy(1, 2);
   cprintf("   Scanning Overview ... ");
   oThrob->SetPos(wherex(), wherey());
   oMnOverView->Main(0x72);
   gotoxy(wherex() - 1, wherey());
   cprintf("Done\r\n   Scanning Drives ... ");
   oThrob->SetPos(wherex(), wherey());
   oMnDrv->Main(0x72);
   gotoxy(wherex() - 1, wherey());
   cprintf("Done\r\n   Scanning PCI Bus ... ");
   oThrob->SetPos(wherex(), wherey());
   oMnPCIBus->Main(0x72);
   gotoxy(wherex() - 1, wherey());
   cprintf("Done\r\n   Scanning DMI Table ... ");
   oThrob->SetPos(wherex(), wherey());
   oMnDMI->Main(0x72);
   gotoxy(wherex() - 1, wherey());
   cprintf("Done\r\n");

	  /* Start by clearing the screen */
   for (y = 1; y <= 25; ++y)
	   for (x = 1; x <= 80; ++x)
		   PutChar(x, y, ' ', COLOR(WHITE, BLACK));


   /* Draw the toolbar */
   PaintBG(C_BGROUND);
   gotoxy(32, 1);
   cprintf("[ BPROBE v2.1 ]");
   gotoxy(80 - 16, 1);
   cprintf("[P] Print Report");
   HighLight(1, 1, 80, COLOR(WHITE, BLUE));
   SetCharColor(80 - 16,      1, LC_BRACKET);
   SetCharColor(80 - 16 + 1,  1, LC_KEY);
   SetCharColor(80 - 16 + 2,  1, LC_BRACKET);

   gotoxy(1, 2);
   cprintf("  Sysinfo  -  Drives  -  PCI Bus  -  DMI Viewer ");
   gotoxy(80 - 16, 2);
   cprintf("[Escape] to Quit");

   HighLight(1, 2, 80, C_MM_TOOLBAR);
   SetCharColor(80 - 16,  2,     LC_BRACKET);
   SetCharColor(80 - 16 + 7,  2, LC_BRACKET);
   HighLight(80-15, 2, 6, LC_KEY);



   /* Set the selected item and highlight it */
   selected = 0;
   HLIGHT(2, C_MM_HL);

   SpawnMenu(FALSE);
   /* Main keyloop */
   while (!exitflag) {
	   c = Input();
	   switch (c)
	   {
		  case ESCAPE:
			  exitflag = TRUE;
			  break;
		  case KEY_LEFT:
		  case KEY_4:
			  if (selected > 0) {
				 HLIGHT(2, C_MM_ITEM);
				 --selected;
				 HLIGHT(2, C_MM_HL);
				 SpawnMenu(FALSE);
			  }
			  break;
		  case KEY_RIGHT:
		  case KEY_6:
			  if (selected < MAX_MM) {
				 HLIGHT(2, C_MM_ITEM);
				 ++selected;
				 HLIGHT(2, C_MM_HL);
				 SpawnMenu(FALSE);
			  }
			  break;
		  case 'p':
		  case 'P': Report();
				 SpawnMenu(TRUE);
			  break;
	   } /* switch() */
   } /* while() */


   delete oMnOverView;
   delete oMnDrv;
   delete oMnPCIBus;
   delete oMnDMI;

   /* We're about to bomb out, clean up after ourself */
   /* ... */
   textattr(oldtextattr);
   for (y = 1; y <= 25; ++y)
	   for (x = 1; x <= 80; ++x)
		   PutChar(x, y, ' ', COLOR(WHITE, BLACK));
   gotoxy(1, 1);

   printf("BPROBE " VERSION " Copyright 2002 by Micromodules, Inc.\n");
   printf("Thank you for using BPROBE. For updates, questions, or bug reports\n");
   printf("visit our web site at http://www.micromodules.com/bprobe\n");

}

/* Spawn a menu by creating its object and calling it's main. Delete
   it when it returns. The submenu MAY NOT draw to y=1, and is not
   responsible for cleaning up after itself. We will do so. */
void cMnMain::SpawnMenu(int enter) {
	int x, y;

	if (!enter) PaintBG(C_BGROUND);

	switch (selected) {
	   case 0:       /* Sysinfo menu */
		   if (enter) oMnOverView->Main(MENURIGHT | MENU_NOREDRAW);
		   else oMnOverView->Main(MENURIGHT);
		   break;
	   case 1:       /* Logical Drives */
		   if (enter) oMnDrv->Main(MENUBOTH | MENU_NOREDRAW);
		   else oMnDrv->Main(MENUBOTH);
		   break;
	   case 2:       /* PCI Bus Menu */
		   if (enter) oMnPCIBus->Main(MENUBOTH | MENU_NOREDRAW);
		   else oMnPCIBus->Main(MENUBOTH);
		   break;
	   case 3:       /* DMI Viewer */
		   if (enter) oMnDMI->Main(MENULEFT | MENU_NOREDRAW);
		   else oMnDMI->Main(MENULEFT);
		   break;
	}
}

/*
	   Company Name / Title   : Jim Bobs Computer Repair
	  Computer Name / Comment : Front Computer
						 Date : 03/27/02

						Pages : ALL
*/
#define S_TITLE  0
#define S_NAME   1
#define S_DATE  2

#define D_PAGE  0   // Drop box, which pages to print
#define DP_ALL  0   // Drop Box, print all pages
#define DP_MAIN 1   // Only main info
#define DP_PCI  2   // Only PCI

void cMnMain::SetupReport() {
   struct date d;
   char tbuf[100];

   inf = fopen("report.ini", "rt");  // try to open it. check status later

   getdate(&d);
   sprintf(tbuf, "%.2i/%.2i/%.4i", d.da_mon, d.da_day, d.da_year);
   oText[S_DATE]->SetText(tbuf);

   oText[S_TITLE]->SetText("");
   oText[S_NAME]->SetText("");
   oDrop[D_PAGE]->SetCurrent(DP_ALL);

   if (inf == NULL) return;

   while ( fgets(tbuf, 80, inf) != NULL ) {
	  if (strncmpi(tbuf, "TITLE", 5) == 0) {
		 if (fgets(tbuf, 80, inf) != NULL) {
			if (strlen(tbuf) > 0) tbuf[strlen(tbuf) - 1] = 0;
			oText[S_TITLE]->SetText(tbuf);
		 }
	  }
	  if (strncmpi(tbuf, "NAME", 4) == 0) {
		 if (fgets(tbuf, 80, inf) != NULL) {
			if (strlen(tbuf) > 0) tbuf[strlen(tbuf) - 1] = 0;
			oText[S_NAME]->SetText(tbuf);
		 }
	  }
	  if (strncmpi(tbuf, "ITEMS", 5) == 0) {
		 if (fgets(tbuf, 80, inf) != NULL) {
			if ( (tbuf[0] - 48) <= 2)
			oDrop[0]->SetCurrent(tbuf[0] - 48);
		 }
	  }
   }

   if (inf != NULL) {
	  fclose(inf); inf = NULL;
   }

}



#define S_X  30
#define S_Y  11
#define C_TEXT  COLOR(DARKGRAY, LIGHTGRAY)
#define C_HIGH  COLOR(YELLOW, LIGHTGRAY)
#define C_BOX   COLOR(WHITE, BLUE)
#define C_LABEL  COLOR(WHITE, BLUE)



void cMnMain::SaveSettings() {
   char c = 0;

   DrawBox(50, 1, 0, 0, TRUE, TRUE, C_BOX, C_BOX);
   SetWindowBox();
   SetColor(C_BOX);
   gotoxy(25 - 36/2, 1);
   cprintf("Really save default settings? (Y/N)");
   RestoreColor();
   while (c != 'y' && c != 'Y' && c != 'n' && c !='N') { c = Input(); }

   if (c == 'y' || c == 'Y') {
	  if ( (inf = fopen("report.ini", "wt")) == NULL) {
		 gotoxy(25 - 48/2, 1);
		 cprintf("Could not open file for output! (Press any key)");
		 c = Input();
	  } else {  // file open, do output
		fprintf(inf, "TITLE\n");
		fprintf(inf, "%s\n", oText[S_TITLE]->Get());
		fprintf(inf, "NAME\n");
		fprintf(inf, "%s\n", oText[S_NAME]->Get());
		fprintf(inf, "ITEMS\n");
		fprintf(inf, "%i\n", oDrop[0]->Get());
		fclose(inf);
	  }
   }

   window(1, 1, 80, 25);
   RestoreBox();
}


/*
	   Company Name / Title   : Jim Bobs Computer Repair
	  Computer Name / Comment : Front Computer
						 Date : 03/27/02

						Pages : ALL
*/

void cMnMain::Report() {
   static dconfig = FALSE;
   int i;
   int curSel = 3;
   char c = 0;

   DrawBox(47, 8, 0, 0, TRUE, TRUE, COLOR(WHITE, BLUE), C_BOX);

   if (!dconfig) {
	  oText[0]->Create(S_X, S_Y + 0, 30, 29, C_TEXT, C_HIGH);
	  oText[1]->Create(S_X, S_Y + 1, 30, 29, C_TEXT, C_HIGH);
	  oText[2]->Create(S_X, S_Y + 2, 30, 29, C_TEXT, C_HIGH);

	  oText[0]->SetLabel("Title :", C_LABEL);
	  oText[1]->SetLabel("Comment :", C_LABEL);
	  oText[2]->SetLabel("Date :", C_LABEL);

	  oDrop[0]->Create(S_X, S_Y + 4, 8, 3, C_TEXT, C_HIGH, C_HIGH, C_BOX);
	  oDrop[0]->SetLabel("Pages :", C_LABEL);
	  oDrop[0]->Insert("All");
	  oDrop[0]->Insert("System");
	  oDrop[0]->Insert("PCI");
	  dconfig = TRUE;
   }

   SetupReport();

   reporttop:;
   DrawBox(47, 8, 0, 0, TRUE, FALSE, COLOR(WHITE, BLUE), C_BOX);
   SetColor(C_BOX);
   gotoxy(40 - 10, S_Y - 1);
   cprintf("CREATE BPROBE REPORT");
   gotoxy(19, S_Y + 6);
   cprintf("[F] Print Report to File  [S] Save defaults");
   SetCharColor(20, S_Y + 6, COLOR(YELLOW, BLUE));
   SetCharColor(19, S_Y + 6, COLOR(BROWN, BLUE));
   SetCharColor(21, S_Y + 6, COLOR(BROWN, BLUE));
   SetCharColor(46, S_Y + 6, COLOR(YELLOW, BLUE));
   SetCharColor(45, S_Y + 6, COLOR(BROWN, BLUE));
   SetCharColor(47, S_Y + 6, COLOR(BROWN, BLUE));

   gotoxy(43, S_Y + 5);
   cprintf("[1-3] Print to LPT#");
   SetCharColor(43, S_Y + 5, COLOR(BROWN, BLUE));
   SetCharColor(47, S_Y + 5, COLOR(BROWN, BLUE));
   SetCharColor(44, S_Y + 5, COLOR(YELLOW, BLUE));
   SetCharColor(46, S_Y + 5, COLOR(YELLOW, BLUE));

   RestoreColor();
//   HighLight(11, S_Y - 1, 60, COLOR(WHITE, BROWN));
   for (i = 0; i < 3; ++i) {
	  oText[i]->Select(FALSE); oText[i]->Draw();
   }
   oDrop[0]->Select(FALSE);
   oDrop[0]->Draw(TRUE);
   curSel = 0;
   oText[0]->Select(TRUE);

   while (c != ESCAPE && c != 'f' && c != 'F' && c != '1' && c != '2' && c != '3') {
	  c = Input();
	  switch (c) {
		 case 's':
		 case 'S': RestoreBox();
				   SaveSettings();
				   DrawBox(47, 8, 0, 0, TRUE, TRUE, COLOR(WHITE, BLUE), C_BOX);
				   goto reporttop;
		 case ENTER:
			if (curSel <= 2) oText[curSel]->EnterText();
			else oDrop[0]->MenuChoose();
			break;
		 case KEY_LEFT:
			if (curSel == 3) oDrop[0]->MoveCurrent(-1);
			break;
		 case KEY_RIGHT:
			if (curSel == 3) oDrop[0]->MoveCurrent(1);
			break;
		 case KEY_DOWN:
			if (curSel < 3) {
			   for (i = 0; i < 3; ++i) oText[i]->Select(FALSE);
			   oDrop[0]->Select(FALSE);
			   ++curSel;
			   if (curSel <= 2) oText[curSel]->Select(TRUE);
			   else oDrop[0]->Select(TRUE);
			}
			break;
		 case KEY_UP:
			if (curSel > 0) {
			   for (i = 0; i < 3; ++i) oText[i]->Select(FALSE);
			   oDrop[0]->Select(FALSE);
			   --curSel;
			   if (curSel <= 2) oText[curSel]->Select(TRUE);
			   else oDrop[0]->Select(TRUE);
			}
			break;


	  } // switch()
   } // while()

   RestoreBox();
   if (c == 'f' || c == 'F') {
	  target = T_FILE;
	  PrintReport(0);
   }
   if (c == '1' || c == '2' || c == '3') {
	  target = T_PRINT;
	  PrintReport(c - 48);
   }
}

int cMnMain::aprint(char *fmt, ...) {
   char buf[150];
   va_list argptr;

   va_start(argptr, fmt);
   vsprintf(buf, fmt, argptr);
   va_end(argptr);

   if (target == T_FILE) { fprintf(fp, "%s", buf); return TRUE; }

   if (LPT->lprint("%s", buf)) return TRUE;

   DrawBox(70, 2, 0, 0, TRUE, TRUE, C_BOX, C_BOX);
   SetColor(C_BOX);
   gotoxy(22, 13);
   cprintf("Print Error: %s", LPT->LastError());
   gotoxy(33, 14);
   cprintf("Press any key");
   getch();
   RestoreColor();
   RestoreBox();

   return FALSE;
}

#define PRINT(s) { if (!s) return; }

void cMnMain::PrintReport(int port) {
   int i, t;
   char tBuf[150];
   char *cp;
   char ClassName[50], SubClassName[50];

   if (target == T_PRINT) {
	  if (!LPT->InitPort(port)) {
		 DrawBox(70, 2, 0, 0, TRUE, TRUE, C_BOX, C_BOX);
		 SetColor(C_BOX);
		 gotoxy(22, 13);
		 cprintf("Print Error: %s", LPT->LastError());
		 gotoxy(33, 14);
		 cprintf("Press any key");
		 getch();
		 RestoreColor();
		 RestoreBox();
	  }
   }

   if ( (target == T_FILE) && (fp = fopen("report.txt", "wt")) == NULL) {
	  DrawBox(50, 2, 0, 0, TRUE, TRUE, C_BOX, C_BOX);
	  SetColor(C_BOX);
	  gotoxy(22, 13);
	  cprintf("Could not open output file for writing!");
	  gotoxy(33, 14);
	  cprintf("Press any key");
	  getch();
	  RestoreColor();
	  RestoreBox();
	  return;
   }

   // Force Benchmark if not already done
   if (oDrop[0]->Get() == DP_ALL || oDrop[0]->Get() == DP_MAIN) {
	  DrawBox(33, 1, 0, 0, TRUE, TRUE, C_BOX, C_BOX);
	  SetColor(C_BOX);
	  gotoxy(28, 13);
	  cprintf(" Calculating CPU Speed ... ");
	  oThrob->SetColor(C_BOX);
	  oMnOverView->CheckBenchmark();
	  RestoreColor();
	  RestoreBox();
   }

   // Force PCI.DAT loading if not already done
   if (oDrop[0]->Get() == DP_ALL || oDrop[0]->Get() == DP_PCI) {
	  DrawBox(50, 1, 0, 0, TRUE, TRUE, C_BOX, C_BOX);
	  SetColor(C_BOX);
	  gotoxy(28, 13);
	  cprintf("Loading PCI.DAT Names ... ");
	  oThrob->SetColor(C_BOX);
	  oMnPCIBus->Main(0x73);
	  RestoreColor();
	  RestoreBox();
   }



   PRINT(aprint("\n"));
   strcpy(tBuf, "BPROBE System Report");
   i = (strlen(tBuf) / 2) + 40;
   PRINT(aprint("%*s%*s\n", i, tBuf, 75 - i, oText[S_DATE]->Get()));

   sprintf(tBuf, "%s", oText[S_TITLE]->Get());
   i = (strlen(tBuf) / 2) + 40; PRINT(aprint("%*s\n", i, tBuf));


   // Find out if we need to print a system name
   {
	  char *p;
	  p = oText[S_NAME]->Get();
	  for (i = 0; i < 10; ++i) {
		 if (p[i] == '\0') { i = 10; break; }
		 if (p[i] != ' ')  break;
	  }
   }
   if (i != 10) {
   sprintf(tBuf, "%s", oText[S_NAME]->Get());
   i = (strlen(tBuf) / 2) + 40; PRINT(aprint("%*s\n", i, tBuf));
   }
   PRINT(aprint("\n"));
/*  Central Processor
	Vendor - GenuineIntel       Speed - 500mhz
	Family -                    Model -                      Step -

*/

   if (oDrop[0]->Get() == DP_ALL || oDrop[0]->Get() == DP_MAIN) {

   PRINT(aprint("- [ Central Processor ]\n"));
   PRINT(aprint("  Vendor - %-18sSpeed - %.1f MHz       Step - %i\n",
	 oMnOverView->cpu_vendorid(), oMnOverView->cpu_speed(), oMnOverView->cpu_step()));
   PRINT(aprint("  Family - %-18sModel - %s\n\n",
			oMnOverView->cpu_family(), oMnOverView->cpu_model()));


   PRINT(aprint("- [ RAM Capacity ]\n"));
   PRINT(aprint("   "));
   if (oMnOverView->ram_mapped() != 0)
	  PRINT(aprint("Mapped: %lu Mb          ", oMnOverView->ram_mapped() / 1024L));
   PRINT(aprint("Bios Reported: %lu Mb\n", oMnOverView->ram_reported()));
   PRINT(aprint("\n"));

   PRINT(aprint("- [ I/O Ports ]\n"));
   PRINT(aprint("  COM Ports - ")); t = 0;
   for (i = 0; i < 4; ++i) {
	  if (oMnOverView->com_comm(i) != 0) {
		 if (t == 2) PRINT(aprint("\n              "));
		 PRINT(aprint("COM%i (%.3Xh) IRQ %c %s    ",
				oMnOverView->com_comm(i), oMnOverView->com_base(i),
				(oMnOverView->com_irq(i))?(oMnOverView->com_irq(i)+48):'?',
				oMnOverView->com_uart(i))); ++t; }
   }
   if (t == 0) PRINT(aprint("None"));  PRINT(aprint("\n"));

   PRINT(aprint("  LPT Ports - ")); t = 0;

   for (i = 0; i < 3; ++i) {
	  if (oMnOverView->lpt_base(i) != 0) {
		 if (t == 2) PRINT(aprint("\n              "));
		 PRINT(aprint("LPT%i (%.3Xh) Mode %s        ", i + 1,
				oMnOverView->lpt_base(i), oMnOverView->lpt_mode(i))); ++t; }
   }
   if (t == 0) PRINT(aprint("None"));  PRINT(aprint("\n\n"));


   PRINT(aprint("- [ IDE Drives ]\n"));
   oMnDrv->PrintDrives(fp);
   PRINT(aprint("\n"));

   PRINT(aprint("- [ DOS Drive Letters ]\n"));
   oMnDrv->oBox[0].DumpReset(); t = 0;
   while ( (cp = oMnDrv->oBox[0].DumpBuffer()) != NULL) {
	  PRINT(aprint(" %s\n", cp)); ++t;
   }
   if (t == 0) PRINT(aprint(" None."));
   PRINT(aprint("\n"));

   PRINT(aprint("- [ Logical Partitions ]\n"));
   oMnDrv->oBox[1].DumpReset(); t = 0;
   while ( (cp = oMnDrv->oBox[1].DumpBuffer()) != NULL) {
	  PRINT(aprint(" %-.33s %s\n", cp, (cp + 43))); ++t;
	  //fprintf(fp, " %s\n", cp); ++t;
   }
   if (t == 0) PRINT(aprint(" None.\n"));
   }

   if (oDrop[0]->Get() == DP_ALL || oDrop[0]->Get() == DP_PCI) {
   t = 0;
   PRINT(aprint("- [ PCI Device List ]\n"));
   PRINT(aprint("   B  D F   Class/Sub      Vendor/Device\n"));
   for (i = 0; i < 15 && i <= oMnPCIBus->pci_maxdev(); ++i) {
	  if ((oMnPCIBus->pci_device(i))->reg.VendorID != 0 &&
		  (oMnPCIBus->pci_device(i))->reg.VendorID != 0xFFFF) {
		  oMnPCIBus->pci_classname((oMnPCIBus->pci_device(i))->reg.BaseClass,
			 (oMnPCIBus->pci_device(i))->reg.SubClass, ClassName, SubClassName);
		  PRINT(aprint("   %i/%2i/%i   %-15s-%s\n",
			  (oMnPCIBus->pci_device(i))->bus, (oMnPCIBus->pci_device(i))->dev,
			  (oMnPCIBus->pci_device(i))->func, ClassName,
			  oMnPCIBus->VendorName[i]));
		  PRINT(aprint("            %-15s %s\n", SubClassName,
			  oMnPCIBus->DeviceName[i]));
		 ++t;
	  }
   }
   if (t == 0) PRINT(aprint(" No devices detected.\n"));
   }

   if (target == T_FILE) fclose(fp);
   if (target == T_PRINT) {
	  LPT->lprint("\f");
	  LPT->Close();
   }
}

