#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


#include "declare/global.h"
#include "menuclas/MnOvervi.h"

/* Class include files */
#include "diag/cpu/cpudet.h"
#include "diag/mem/memdet.h"
#include <LowLevel/side.h>
#include <LowLevel/RamDet.h>

/* Display Brief information about the system 
   * CPU Info
   * Memory Size
   * Floppy
   * LPT/Serial Ports
   * IDE Devices

   Put the info on the screen and wait until escape is pressed.

*/

// Static to preserve value between menu changes
#define CPUUNTESTED  0  // CPU Speed has not been tested
#define CPUTESTED    1  // CPU speed has been tested
#define CPUUNKNOWN   2  // CPU Speed imposible to test (nonpentium)
static float cpuspeed  = 0;
static int   cputested = CPUUNTESTED;

cMnOverView::cMnOverView() {
   cpuspeed = 0;
   cputested = CPUUNTESTED;
   oCPU = new cCPUDet;
   oRAM = new cRamDet;
   oLPT = new cLPTDet;
   oCOM = new cSerialDet;
}

cMnOverView::~cMnOverView() {
	free(oCPU);
	free(oRAM);
	free(oLPT);
	free(oCOM);
}


void cMnOverView::Main(int menudir) {
   /* Declare all the classes we will need */
   int i, t;
   char c;

   if (menudir == 0x72) {
	  oThrob->Step();
	  oCPU->Main();
	  oThrob->Step();
	  oRAM->Init();
	  oThrob->Step();
	  oLPT->Init();
	  oThrob->Step();
	  oCOM->Init();
	  oThrob->Step();
	  return;
   }

   if (menudir & MENU_NOREDRAW) goto entry;

   DrawAscii();
	/* Call each classes main to have it perform diagnostics */
   window(4, 5, 78, 22);
   gotoxy(1, 1);

   /* Format and display each item gathered from all the classes */
   textattr(C_OV_HEADER);
#define MSTR "%-73s \n\r"
   cprintf(MSTR, "CPU INFORMATION");
   textattr(C_OV_DATA);

   if (cpuspeed == CPUUNTESTED)
   cprintf("Vendor: %-20s   Speed: [Press Enter to Test]\n\r", oCPU->vendorid);
   else if (cputested == CPUTESTED)
   cprintf("Vendor: %-20s   Speed: %.1f MHz\n\r", oCPU->vendorid, cpuspeed);
   else
   cprintf("Vendor: %-20s   Speed: Unknown\n\r", oCPU->vendorid);



   cprintf("Family: %-20s   Model: %s\n\r", oCPU->familyname, oCPU->modelname);
   cprintf("Step: %i %s\n\r", oCPU->stepping, oCPU->stepname);

   cprintf("\n\r");

   textattr(C_OV_HEADER);
   cprintf(MSTR, "RAM SIZE");
   textattr(C_OV_DATA);
   /* Bios reported is already in K */
   cprintf("Mapped RAM: %7lu KB         Bios Reported: %7lu KB\n\r ",
		   oRAM->MappedRam() / 1024L,  oRAM->ReportedRam());

   cprintf("\n\r");
   textattr(C_OV_HEADER);
   cprintf(MSTR, "Serial Ports");
   textattr(C_OV_DATA);

   t = 0;
   for (i = 0; i < 4; ++i) {
	   if (oCOM->port[i].comm == 0) continue;
	   ++t;
	   if ((int)oCOM->port[i].irq == -1)
		  cprintf("COM%i (%Xh)  IRQ %c  UART %s\r\n",
			oCOM->port[i].comm, oCOM->port[i].base, '?',
			oCOM->UART_Name(i));
	   else
		  cprintf("COM%i (%Xh)  IRQ %i  UART %s\r\n",
			oCOM->port[i].comm, oCOM->port[i].base, oCOM->port[i].irq,
			oCOM->UART_Name(i));
   }
   if (t == 0)
	  cprintf("No Ports Detected.\r\n");

   // t is num of lines we entered. We want at least 4 lines.
   for (i = t; i < 2; ++i)
	  cprintf("\r\n");

   cprintf("\n\r");
   textattr(C_OV_HEADER);
   c = 0;
   cprintf(MSTR, "LPT Ports");
   textattr(C_OV_DATA);
   for (i = 0; i < 3; ++i) {
	   if (oLPT->port[i].address == 0) continue;
	   cprintf("LPT%i (%Xh)  Mode %s\r\n", i + 1,
		   oLPT->port[i].address, oLPT->LPT_Mode(i));
	   ++c;
   }
   if (c == 0)
	  cprintf("No Ports Detected.\r\n");


   window(1, 1, 80, 25);
   gotoxy(2,25);
   cprintf("[Left/Right] Select Menu");

   HighLight(1, 25, 80, LC_TOOLBAR);
	SetCharColor(2,  25, LC_BRACKET);
	SetCharColor(13, 25, LC_BRACKET);
	HighLight(3, 25, 10, LC_KEY);



   entry:;
   c = 0;
   while (c != ESCAPE) {
	  c = Input();
	  if (c == ENTER) {
		 gotoxy(42,6);
		 cprintf("Testing ...             ");
		 oCPU->Benchmark();
		 if (oCPU->mhz == 0) {
			LOGLO("CPU MHZ is 0 after benchmark.");
			cputested = CPUUNKNOWN;
			gotoxy(42,6);
			cprintf("Unknown              ");
		 } else {
			cpuspeed = oCPU->mhz;
			cputested = CPUTESTED;
			LOGLO("CPU MHZ is %.2f after benchmark.", cpuspeed);
			gotoxy(42,6);
			cprintf("%.1f MHz                   ", cpuspeed);
		 }
	  }
	  switch (c) {
		 case KEY_LEFT:
		 case KEY_4:     if (!(menudir & MENULEFT)) continue;
						ungetch(c); c = ESCAPE;  break;
		 case KEY_RIGHT:
		 case KEY_6:     if (!(menudir & MENURIGHT)) continue;
						ungetch(c); c = ESCAPE;  break;
		 case 'm':
		 case 'M':      oRAM->DrawMap(); break;
		 case 'p':
		 case 'P':
		 case ESCAPE:   ungetch(c); c = ESCAPE; break;

	  }

   } /* while() */
}


void cMnOverView::CheckBenchmark() {
   if (cputested != CPUUNTESTED) return;  // If already tested, just return

   oCPU->Benchmark();
   if (oCPU->mhz == 0) cputested = CPUUNKNOWN;
   else {   cpuspeed = oCPU->mhz; cputested = CPUTESTED; }

}


void cMnOverView::DrawAscii() {
   int i;

   DrawBox(3, 5, 78, 22, FALSE, FALSE, C_PC_DATA, C_DM_LINE);


}

