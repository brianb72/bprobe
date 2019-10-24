#include <stdio.h>
#include <conio.h>
#include <string.h>

/* Display class for bprobe lite that displays physical drives and
   drive letters on one page. */


#include <Global.h>
#include "declare/global.h"
#include "menuclas/MnDrv.h"
#include <Logical\DosLet.h>
#include <Logical\BiosDrv.h>
#include <UserIo\TextBox.h>

#define DISP_LET 0
#define DISP_PAR 1

cMnDrv::cMnDrv() {
	oIDE[0] = new cIDE;
	oIDE[1] = new cIDE;
	oIDE[0]->SetBase(PORTPRIMARY);
	oIDE[1]->SetBase(PORTSECONDARY);
	oDos = new cDosLet;
	oBios = new cBiosDrv;
	curItem = DISP_LET;
}

cMnDrv::~cMnDrv() {
	delete oIDE[0];
	delete oIDE[1];
	delete oDos;
	delete oBios;
}


void cMnDrv::Main(int menudir) {
	char c = 0;

	if (menudir == 0x72) {
	   oIDE[0]->InitDrives();
	   oIDE[1]->InitDrives();
	   oBios->Init();
	   oDos->Init();
	   /* Create both boxes. Set their display FALSE so they wont draw */
	   oBox[0].bid = 0; oBox[1].bid = 1;
	   oBox[0].Create(3, 14, 78, 22, 20, C_LO_DATA, C_LO_LINE);
	   oBox[0].SetTitle("DOS DRIVE LETTERS", TITLE_HEADER, COLOR(WHITE, BLUE));
	   oBox[1].Create(3, 14, 78, 22, 50, C_LO_DATA, C_LO_LINE);
	   oBox[1].SetTitle("LOGICAL PARTITIONS", TITLE_HEADER, COLOR(WHITE, BLUE));
	   MakePartitionsBox();
	   MakeLettersBox();
	   oBox[0].ScrollUp(0);
	   oBox[1].ScrollUp(0);
	   return;
	}



	DrawAscii();
	DrawDrives();
	// Call Draw() only on the first time so it draws its box
	oBox[curItem].Draw();

	gotoxy(2,25);
	cprintf("[Left/Right] Select Menu  [Up/Down] Scroll  [Space] Toggle Display");
	HighLight(1, 25, 80, LC_TOOLBAR);

	SetCharColor(2,  25, LC_BRACKET);
	SetCharColor(13, 25, LC_BRACKET);
	HighLight(3, 25, 10, LC_KEY);

	SetCharColor(28,  25, LC_BRACKET);
	SetCharColor(36, 25, LC_BRACKET);
	HighLight(29, 25, 7, LC_KEY);

	SetCharColor(28 + 18,  25, LC_BRACKET);
	SetCharColor(34 + 18, 25, LC_BRACKET);
	HighLight(29 + 18, 25, 5, LC_KEY);



	c = 0;
	while (c != ESCAPE) {
		c = Input();
	  switch (c) {
		 case KEY_UP:
		 case KEY_8:    oBox[curItem].ScrollUp(1); break;
		 case KEY_DOWN:
		 case KEY_2:    oBox[curItem].ScrollDown(1); break;
		 case KEY_PGUP: oBox[curItem].ScrollUp(5); break;
		 case KEY_PGDN: oBox[curItem].ScrollDown(5); break;
		 case KEY_HOME: oBox[curItem].ScrollUp(0); break;
		 case KEY_END: oBox[curItem].ScrollDown(0); break;


		 case KEY_LEFT:
		 case KEY_4:     if (!(menudir & MENULEFT)) continue;
						ungetch(c); c = ESCAPE;  break;
		 case KEY_RIGHT:
		 case KEY_6:     if (!(menudir & MENURIGHT)) continue;
						ungetch(c); c = ESCAPE;  break;
		 case 'p':
		 case 'P':
		 case ESCAPE:   ungetch(c); c = ESCAPE; break;
		 case SPACE:   if (curItem == 0) curItem = 1;
					   else curItem = 0;
					   oBox[curItem].Draw();
					   break;
	  }
	}


}

/*

   Drv#  Type      Size          Drv#  Type      Size
	  *  FAT32     999999mb         *  FAT32     999999mb
	  *  FAT32     999999mb         *  FAT32     999999mb
	  *  FAT32     999999mb         *  FAT32     999999mb
	  *  FAT32     999999mb         *  FAT32     999999mb
   Ext#  Type      Size

   Drv#  Type      Size          Drv#  Type      Size
	  *  FAT32     999999mb			*  FAT32     999999mb
	  *  FAT32     999999mb			*  FAT32     999999mb
	  *  FAT32     999999mb			*  FAT32     999999mb
	  *  FAT32     999999mb			*  FAT32     999999mb


*/


/* Don't pass more than num = 8 or we'll exceed biosdrv[10] */
void cMnDrv::DoPartition(char *name, int num, struct partition_data *lp,
			 struct partition_data *rp, int max) {
   int i, act = 0, found = 0;
   char sleft[15][200], sright[15][200];

   if (num > 8) return;  // protect

   memset(sleft, 0, sizeof(sleft));
   memset(sright, 0, sizeof(sright));

   if (oBios->biosdrv[num].drive != 0)
	  sprintf(sleft[0],  " %s#%i  Type                Size", name, oBios->biosdrv[num].drive - 0x80);
   if (oBios->biosdrv[num + 1].drive != 0)
	  sprintf(sright[0], " %s#%i  Type                Size", name, oBios->biosdrv[num + 1].drive - 0x80);

   act = 1;
   for (i = 0; i < max; ++i) {
	  found = 0;
	  if (lp[i].stype != 0) {
		 sprintf(sleft[i+1],  " P%i%c    %-16.16s%6lumb", i + 1,
			((lp[i].boot)?'*':'  '), oBios->GetPartitionName(lp[i].stype),
			lp[i].capacity);
		 found = 1;
	  }
	  if (rp[i].stype != 0) {
		 sprintf(sright[i+1], " P%i%c    %-16.16s%6lumb", i + 1,
			((rp[i].boot)?'*':'  '),  oBios->GetPartitionName(rp[i].stype),
			rp[i].capacity);
		 found = 1;
	  }
	  if (found) ++act;
   }

   oBox[1].CheckList("DoPartition");

   for (i = 0; i < act + 1; ++i)
	  oBox[DISP_PAR].Insert("%-43.43s%.43s", sleft[i], sright[i]);
   oBox[1].CheckList("DoPartition After Loop");


}

/* biosdrv[] contains ebpvery successful drive that was detected.
   Any gap in actual numbers is not shown here. It can go from
   80h, 82h, 84h. etc. biosdrv[numdrvs - 1] is last valid entry */
void cMnDrv::MakePartitionsBox() {
   int i, cnt;
//   char buf[50];

   if (oBios->numdrvs == 0) {
	  oBox[DISP_PAR].Insert("No Logical Drives Detected.");
	  return;
   }
   for (cnt = 0; (cnt < ((oBios->numdrvs + 1) / 2)) && cnt <= 4; ++cnt) {
	  i = cnt * 2;
	  DoPartition("Drv", i, oBios->biosdrv[i].p,
				oBios->biosdrv[i + 1].p, 4);
//	  sprintf(buf, "Make Partition cnt %i  i %i", cnt, i);
//	  oBox[1].CheckList(buf);

	  if ( (oBios->biosdrv[i].extend) || (oBios->biosdrv[i + 1].extend))
		 DoPartition("Ext", i, oBios->biosdrv[i].e,
				oBios->biosdrv[i + 1].e, 10);
   } /* master for() */
}



/* Letter is 0 */
void cMnDrv::MakeLettersBox() {
   int i;
   char fsstr[35], freesp[35], totalsp[35], osstr[50];
//   char buf[100];
   float percent;

   oBox[0].Insert(" Let  Format     Free    Total    Free%     Operating System");
   if (oDos->numlets == 0) {
	  oBox[0].Insert("No DOS Drive Letters Detected.");
	  return;
   }
   for (i = 0; i < 7 && i < oDos->numlets; ++i) {
	   fsstr[0] = 0;
	   freesp[0] = 0;
	   totalsp[0] = 0;
	   osstr[0] = 0;

	   if (oDos->dlet[i].letter == 0) {
		  continue;
	   }

	   switch (oDos->dlet[i].fstype) {
		  case 1: sprintf(fsstr, "FAT12"); break;
		  case 2: sprintf(fsstr, "FAT16"); break;
		  case 3: sprintf(fsstr, "FAT32"); break;
		  default: sprintf(fsstr, "Unknown"); break;
	   }
	   if (fsstr[0] != 'U') {
	   sprintf(freesp, "%lumb", (long)
		  ( (float) ( (float)oDos->dlet[i].extfree.sectorpercluster *
			 (float) oDos->dlet[i].extfree.bytepersector *
			 (float) oDos->dlet[i].extfree.freecluster
			) / (float) QBYTEM)	);

	   sprintf(totalsp, "%lumb", (long)
		  ( (float) ( (float)oDos->dlet[i].extfree.sectorpercluster *
			 (float) oDos->dlet[i].extfree.bytepersector *
			 (float) oDos->dlet[i].extfree.totalcluster
			) / (float) QBYTEM)	);
	   } else {
		   freesp[0] = 0;
		   totalsp[0] = 0;
	   }

/*       1         2         3         4         5         6                           x
1234567890123456789012345678901234567890123456789012345678901234567890
  Let  Format  Free     Total     Used%  Operating System
   C:  Fat32   100000mb 100000mb  80%      MSDOS.SYS - 4.10.2222 (Win98SE)
	  12123456781234567891234567890
*/

	   if (i == 0) {  // only do this on the first drive */
		  strcpy(osstr, oDos->osString);
	   }

	   percent = ((float) oDos->dlet[i].extfree.freecluster /
			   (float) oDos->dlet[i].extfree.totalcluster) * (float) 100;

	   oBox[0].Insert("  %c:  %5.5s %9.9s%9.9s     %.0f%%      %s",
		   oDos->dlet[i].letter, fsstr, freesp, totalsp, percent, osstr
			  );

   } /* big for() */


}

/*																	       x
	Interface Bios#  Model Name         Capacity     Geometry
	  Pri/Mas   80h  ATAPI CDROM        30000mb   16383/16/63
					 012345678901234561212345  123
	12345678912345612
*/

void cMnDrv::DrawDrives() {
	int i, ionum, drvnum;
	char capstr[25];
	const char *ioname[] = { "Pri/Mas", "Pri/Sla", "Sec/Mas", "Sec/Sla" };
	gotoxy( (75 / 2) - (19 / 2) + 3, 5);
//	gotoxy(4, 5);
	cprintf("PHYSICAL IDE DRIVES");
	HighLight(4, 5, 74, COLOR(WHITE, BLUE));
	gotoxy(4,6);
	cprintf("Interface      Model Name            Capacity     Type");

	for (i = 0; i < 4; ++i) {
	   gotoxy(4, 7 + i);
	   switch (i) {
		   case 0: ionum = 0; drvnum = 0; break;
		   case 1: ionum = 0; drvnum = 1; break;
		   case 2: ionum = 1; drvnum = 0; break;
		   case 3: ionum = 1; drvnum = 1; break;
	   }
	   if (oIDE[ionum]->IsDrive(drvnum) == FALSE) continue;
	   if (oIDE[ionum]->Capacity(drvnum) == 0L)
		   capstr[0] = 0;
	   else
		   sprintf(capstr, "%lumb", oIDE[ionum]->Capacity(drvnum));
	   cprintf("%9s      %-21s  %7.8s     %s",
		   ioname[i], oIDE[ionum]->Model(drvnum),
		   capstr,
		   oIDE[ionum]->ConfigType(drvnum)
		   );
	}
}

void cMnDrv::PrintDrives(FILE *fp) {
	int i, ionum, drvnum;
	char capstr[25];
	const char *ioname[] = { "Pri/Mas", "Pri/Sla", "Sec/Mas", "Sec/Sla" };

	fprintf(fp, "  Interface      Model Name            Capacity     Type\n");

	for (i = 0; i < 4; ++i) {
	   switch (i) {
		   case 0: ionum = 0; drvnum = 0; break;
		   case 1: ionum = 0; drvnum = 1; break;
		   case 2: ionum = 1; drvnum = 0; break;
		   case 3: ionum = 1; drvnum = 1; break;
	   }
	   if (oIDE[ionum]->IsDrive(drvnum) == FALSE) continue;
	   if (oIDE[ionum]->Capacity(drvnum) == 0L)
		   capstr[0] = 0;
	   else
		   sprintf(capstr, "%lumb", oIDE[ionum]->Capacity(drvnum));
	   fprintf(fp, "  %9s      %-21s  %7.8s     %s\n",
		   ioname[i], oIDE[ionum]->Model(drvnum),
		   capstr,
		   oIDE[ionum]->ConfigType(drvnum)
		   );
	}
}



void cMnDrv::DrawAscii() {
	int i, t;

	DrawBox(3, 5, 78, 10, FALSE, FALSE, C_LO_DATA, C_LO_LINE);
}


