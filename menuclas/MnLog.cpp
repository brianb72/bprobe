#include <stdio.h>
#include <conio.h>

#include "declare/global.h"
#include "menuclas/MnLog.h"






void cMnLog::Main() {
	int exitflag = FALSE;
    char c = 0;

	curdrv = 0;

    Init();

	DrawAscii();
	Draw();
	DrawPartition(biosdrv[curdrv].p);

	HLDrive(true);

	while (!exitflag) {
		c = Input();
		switch (c) {
		   case ESCAPE: exitflag = TRUE; break;
		   case KEY_UP:
		   case KEY_8:    
			   if (curdrv > 0) {
				   HLDrive(false);
				   --curdrv; HLDrive(true);
				   	DrawPartition(biosdrv[curdrv].p);
			   }
			   break;
		   case KEY_DOWN:
		   case KEY_2:   
			   if (curdrv < (numdrvs - 1)) {
				   HLDrive(false);
				   ++curdrv; HLDrive(true);
				   	DrawPartition(biosdrv[curdrv].p);
			   }
			   break;
		}
	}
}


/* Highlight or unhighlight the current drive */
void cMnLog::HLDrive(int state) {
	int color;
	if (state) color = C_LO_HL;  /* pick the highlight color */
	else color = C_LO_DATA;

	switch (curdrv) {
		case 0: HighLight(3, 6, 22, color); break;
		case 1: HighLight(3, 7, 22, color); break;
		case 2: HighLight(3, 8, 22, color); break;
		case 3: HighLight(3, 9, 22, color); break;
	}
}


void cMnLog::Draw() {
	int i;
	
	//       4567890123456789123
   window(3, 5, 80, 25);
   gotoxy(1, 1);
   cprintf(" Drv#   Capacity   IO\r\n");
   
   for (i = 0; i < 5; ++i) {
		if (biosdrv[i].drive == 0) continue;
		cprintf("  %.2Xh%9lumb\r\n", biosdrv[i].drive, biosdrv[i].capacity);
   }
   window(1, 1, 80, 25);

   //       80h    30000mb   PM
}



void cMnLog::DrawAscii() {
   int i;

   /* BIOS drive list horizontal */
   for (i = 3; i <= 25; ++i) {
	  PutChar(i, 4, ASC_LINEH, C_LO_LINE);
	  PutChar(i, 11, ASC_LINEH, C_LO_LINE);
      SetCharColor(i, 5, C_LO_DATA);
      SetCharColor(i, 6, C_LO_DATA);
      SetCharColor(i, 7, C_LO_DATA);
      SetCharColor(i, 8, C_LO_DATA);
      SetCharColor(i, 9, C_LO_DATA);
      SetCharColor(i, 10, C_LO_DATA);
   }
   /* BIOS and partition box drive list vertical */
   for (i = 5; i <= 10; ++i) {
	  PutChar(2, i, ASC_LINEV, C_LO_LINE);
	  PutChar(25, i, ASC_LINEV, C_LO_LINE);
	  PutChar(39, i, ASC_LINEV, C_LO_LINE);
	  PutChar(79, i, ASC_LINEV, C_LO_LINE);
   }

   /* Corners on BIOS box */
   PutChar( 2,  4, ASC_CORNUL, C_LO_LINE);
   PutChar(25,  4, ASC_CORNUR, C_LO_LINE);
   PutChar( 2, 11, ASC_CORNLL, C_LO_LINE);
   PutChar(25, 11, ASC_CORNLR, C_LO_LINE);

   /* Partition box horizontal */
   for (i = 40; i <= 78; ++i) {
	   PutChar(i, 4, ASC_LINEH, C_LO_LINE);
	   PutChar(i, 11, ASC_LINEH, C_LO_LINE);
       SetCharColor(i, 5, C_LO_DATA);
       SetCharColor(i, 6, C_LO_DATA);
       SetCharColor(i, 7, C_LO_DATA);
       SetCharColor(i, 8, C_LO_DATA);
	   SetCharColor(i, 9, C_LO_DATA);
       SetCharColor(i, 10, C_LO_DATA);
   }

   /* Corners on Partition box box */
   PutChar(39,  4, ASC_CORNUL, C_LO_LINE);
   PutChar(79,  4, ASC_CORNUR, C_LO_LINE);
   PutChar(39, 11, ASC_CORNLL, C_LO_LINE);
   PutChar(79, 11, ASC_CORNLR, C_LO_LINE);


}


/* Draw all the specified partitions */
void cMnLog::DrawPartition(struct partition_data *p) {
	int i;

	window(40, 5, 78, 10);
	gotoxy(1, 1);
	delline();
	delline();
	delline();
	delline();
	cprintf("       Drive %.2Xh Partition Table  \r\n", biosdrv[curdrv].drive);
	cprintf("B  #  Filesystem Type            Size\r\n");

	for (i = 0; i < 4; ++i) {
		if (p[i].stype == 0) continue;  // no partition
		cprintf("%c  %i  %-23s%6lumb\r\n", (p[i].boot?'*':' '), i + 1,
			GetPartitionName(p[i].stype), p[i].capacity);
	}

	window(1, 1, 80, 25);
}











