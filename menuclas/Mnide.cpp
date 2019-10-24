#include <stdio.h>
#include <conio.h>
#include <dos.h>

#include <Global.h>
#include "declare/global.h"
#include "menuclas/MnIDE.h"


/* Notice: This class assumes that every PC will have two IDE
   interfaces, with the primary at 1F0 and the secondary at 170,
   with a posibility for the secondary to be disabled or nonexistant. 
   If systems exist with different ports they won't be detected.    
   Systems with a 3rd port will not have that port detected.       */
   

/* Commands
   Diagnostic
   Reset Device
   Seek Test
   Verify Test
   Eject

*/

#define X_DIAG 65
#define Y_DIAG 16


/* Setup pointers so we can stick with -> instead of . */
cMnIDE::cMnIDE() {
	io[0] = new cIDE;
	io[1] = new cIDE;
	io[0]->SetBase(PORTPRIMARY);
	io[1]->SetBase(PORTSECONDARY);
}

cMnIDE::~cMnIDE() {
    delete io[0];
	delete io[1];
}

/* Set ionum and drvnum based on dselect */
void cMnIDE::SetDrvNum() {
	switch (dselect) {
		case 0: ionum = 0; drvnum = 0; break;
		case 1: ionum = 0; drvnum = 1; break;
		case 2: ionum = 1; drvnum = 0; break;
		case 3: ionum = 1; drvnum = 1; break;
	}
}

/* Highlight or unhighlight the current drive */
void cMnIDE::HLDrive(int state) {
	int color;
	if (state) color = C_ID_HL;  /* pick the highlight color */
	else color = C_ID_DATA;

	switch (dselect) {
		case 0: HighLight(4, 6, 29, color); break;
		case 1: HighLight(4, 7, 29, color); break;
		case 2: HighLight(4, 9, 29, color); break;
		case 3: HighLight(4, 10, 29, color); break;
	}
}

/* Highlight or unhighlight the current drive */
void cMnIDE::HLMenu(int state) {
	int color;
	if (state) color = C_ID_HL;  /* pick the highlight color */
	else color = C_ID_DATA;

	HighLight(X_DIAG - 1, Y_DIAG + mnum, 14, color); 
	
}

void cMnIDE::Main() {
	int exitflag = FALSE;
    char c = 0;

	/* Scan both interfaces */
    io[0]->InitDrives();
	io[1]->InitDrives();

	textattr(COLOR(WHITE, BLACK));

	dselect = 0;
	mnum = 0;
	SetDrvNum();

 
	/* Do the first draw and setup the loop */
	DrawAscii();
	Draw();
	DisplayDrv();
	HLDrive(true);
	HLMenu(true);

	while (!exitflag) {
		c = Input();
		switch (c) {
		   case ESCAPE: exitflag = TRUE; break;
		   case KEY_UP:
		   case KEY_8:    
			   if (mnum > 0) {
				   HLMenu(false);
				   --mnum; HLMenu(true);
			   }
			   break;
		   case KEY_DOWN:
		   case KEY_2:   
			   if (mnum < 4) {
				   HLMenu(false);
				   ++mnum; HLMenu(true);
			   }
			   break;
		   case KEY_PGUP: 
		   case KEY_9:    
			   if (dselect > 0) {
				   HLDrive(false);
				   --dselect; SetDrvNum(); 
				   HLDrive(true);
				   DisplayDrv();
				   
			   }
			   break;
	       case KEY_PGDN: 
		   case KEY_3: 
			   if (dselect < 3) {
				   HLDrive(false);
				   ++dselect; SetDrvNum();
				   HLDrive(true);
   				   DisplayDrv();
			   }
			   break;
		   case ENTER:
			   if (!io[ionum]->IsDrive(drvnum)) break; // no drive
			   switch (mnum) {
					case 0: Diag(); break;
					case 1: Reset(); break;
					case 2: SeekTest(TEST_SEEK); break;
					case 3: SeekTest(TEST_VERIFY); break;
					case 4: Eject(); break;
			   }
			   break;
		} /* switch () */
	}  /* while () */
}

void cMnIDE::Draw() {
   
	/* Draw the drive list */
    window(3, 5, 80, 25);
	cprintf("Primary Interface\n\r");
	cprintf("   [Master]  %.18s\n\r", io[0]->Model(DMASTER));
	cprintf("   [Slave ]  %.18s\n\r", io[0]->Model(DSLAVE));
	cprintf("Secondary Interface\n\r");
	cprintf("   [Master]  %.18s\n\r", io[1]->Model(DMASTER));
	cprintf("   [Slave ]  %.18s\n\r", io[1]->Model(DSLAVE));

	/* Draw the command list */
	window(X_DIAG, Y_DIAG, 80, 25);
    cprintf("Diagnostic\r\n");
    cprintf("Reset Device\r\n");
    cprintf("Seek Test\r\n");
    cprintf("Verify Test\r\n");
    cprintf("Eject\r\n");
	window(1, 1, 80, 25);

	/* Do the output labels */
	gotoxy(4, Y_STATUS);
	cprintf(" [STATUS]\n\r");
	gotoxy(4, Y_STATUS + 1);
	cprintf("[MESSAGE]\n\r");


}



/*	Display the currently selected drive */
void cMnIDE::DisplayDrv() {
   
   window(38, 5, 78, 10);
   gotoxy(1, 1);
   delline();
   delline();
   delline();
   delline();
   delline();

   if (io[ionum]->IsDrive(drvnum)) {
   cprintf("Model: %.40s\n\r", io[ionum]->Model(drvnum));
   cprintf("Firmware: %.8s   Serial#: %.20s\n\r",
	   io[ionum]->Firmware(drvnum), io[ionum]->Serial(drvnum));
   cprintf("Capacity: %lu MB\n\r", io[ionum]->Capacity(drvnum));
   cprintf("Type: %s\n\r", io[ionum]->ConfigType(drvnum));
   } else {
      cprintf("No device detected.");
   }

   window(1, 1, 80, 25);
}

/* Clears the message box */
void cMnIDE::ClrStatus() {
   int i;

   for (i = 14; i < 61; ++i)
	   PutChar(i, Y_STATUS, ' ', C_ID_DATA);

}

/* Clears the status box */
void cMnIDE::ClrMsg() {
   int i;

   for (i = 14; i < 61; ++i) {
	   PutChar(i, Y_STATUS + 1, ' ', C_ID_DATA);
	   PutChar(i, Y_STATUS + 2, ' ', C_ID_DATA);
   }

}



void cMnIDE::DrawAscii() {
   int i;

   for (i = 3; i <= 78; ++i) {  /* Horizontal bars drive list */
	   PutChar(i, 4, ASC_LINEH, C_ID_LINE);
	   PutChar(i, 11, ASC_LINEH, C_ID_LINE);
	   SetCharColor(i, 5, C_ID_DATA);
	   SetCharColor(i, 6, C_ID_DATA);
	   SetCharColor(i, 7, C_ID_DATA);
	   SetCharColor(i, 8, C_ID_DATA);
	   SetCharColor(i, 9, C_ID_DATA);
	   SetCharColor(i, 10, C_ID_DATA);
	   if (i == 34) i = 37; 
   }

   for (i = 5; i <= 10; ++i) {  /* Vertical bars drive list */
	   PutChar(2, i, ASC_LINEV, C_ID_LINE);
	   PutChar(34, i, ASC_LINEV, C_ID_LINE);
	   PutChar(37, i, ASC_LINEV, C_ID_LINE);
	   PutChar(79, i, ASC_LINEV, C_ID_LINE);
   }

   for (i = 3; i <= 77; ++i) {   /* Bottom horizontal */
	   PutChar(i, 15, ASC_LINEH, C_ID_LINE); 
	   PutChar(i, 21, ASC_LINEH, C_ID_LINE);
   	   SetCharColor(i, 16, C_ID_DATA);
   	   SetCharColor(i, 17, C_ID_DATA);
   	   SetCharColor(i, 18, C_ID_DATA);
   	   SetCharColor(i, 19, C_ID_DATA);
   	   SetCharColor(i, 20, C_ID_DATA);
	   if (i == 60) i = 63;
   }

   for (i = 16; i <= 20; ++i) {  /* Vertical bars drive list */
	   PutChar(2, i, ASC_LINEV, C_ID_LINE);
	   PutChar(61, i, ASC_LINEV, C_ID_LINE);
	   PutChar(63, i, ASC_LINEV, C_ID_LINE);
	   PutChar(78, i, ASC_LINEV, C_ID_LINE);
	
   }
	   
   /* Corners on status info box */
   PutChar(2,  15, ASC_CORNUL, C_ID_LINE);
   PutChar(61, 15, ASC_CORNUR, C_ID_LINE);
   PutChar(2, 21, ASC_CORNLL, C_ID_LINE);
   PutChar(61, 21, ASC_CORNLR, C_ID_LINE);

   /* Corners on command box */
   PutChar(63, 15, ASC_CORNUL, C_ID_LINE);
   PutChar(78, 15, ASC_CORNUR, C_ID_LINE);
   PutChar(63, 21, ASC_CORNLL, C_ID_LINE);
   PutChar(78, 21, ASC_CORNLR, C_ID_LINE);

   
   /* Corners on drive list box */
   PutChar(2,  4, ASC_CORNUL, C_ID_LINE);
   PutChar(34, 4, ASC_CORNUR, C_ID_LINE);
   PutChar(2, 11, ASC_CORNLL, C_ID_LINE);
   PutChar(34, 11, ASC_CORNLR, C_ID_LINE);

   /* Corners on drive info box */
   PutChar(37,  4, ASC_CORNUL, C_ID_LINE);
   PutChar(79, 4, ASC_CORNUR, C_ID_LINE);
   PutChar(37, 11, ASC_CORNLL, C_ID_LINE);
   PutChar(79, 11, ASC_CORNLR, C_ID_LINE);



}


/* Perform all the seek tests one after another. Print an error
   message if any fail and exit                                 */
int cMnIDE::SeekTest(int type) {

   if (TestLinear(type))
	   TestFunnel(type);
   return 1;
}

//  maxsec / scale 
int cMnIDE::TestFunnel(int type) {
   long sbeg, send, tbeg, tend, per, max;
   int ret, scale;
   char c = 0;
   scale = 250;

   sbeg = 0;
   send = io[ionum]->MaxLastSector(drvnum) - 1;
   max  = send;
   per = io[ionum]->MaxLastSector(drvnum) / scale;
   tend = send / per;
   tbeg = 0;


   ClrStatus();
   gotoxy(X_STATUS, Y_STATUS);
   cprintf("Performing funnel %s test. (%lu tracks)", (type?"Verify":"Seek"),
	  tend);

   /* Seek to track 0 to get us started. */
   io[ionum]->Seek(drvnum, send);


   for (;sbeg <= max; sbeg += per, send -= per, ++tbeg, --tend) {
	  if (kbhit())  c = Input();
	  if (c == ESCAPE) {
		 ClrStatus();
		 ClrMsg();
		 gotoxy(X_STATUS, Y_STATUS);
		 cprintf("Aborted by user.");
		 return false;
	  }

	  ClrMsg();
	  gotoxy(X_MSG, Y_MSG);
	  cprintf("%sing from track %lu -> %lu", (type?"Verify":"Seek"),
		  tbeg, tend);
	  
	  /* First seek low */
	  ret = io[ionum]->Seek(drvnum, sbeg);
	  if (!ret) {
		 ClrMsg();
		 ClrStatus();
		 gotoxy(X_MSG, Y_MSG);
		 cprintf("%s from %lu to %lu failed.", (type?"Verify":"Seek"),
			 tend, tbeg);
		 gotoxy(X_STATUS, Y_STATUS);
		 cprintf("%s test encountered error.", (type?"Verify":"Seek"));
		 return false;
	  }
	  /* Then seek high */
	  ret = io[ionum]->Seek(drvnum, send);
	  if (!ret) {
		 ClrMsg();
	     ClrStatus();
		 gotoxy(X_MSG, Y_MSG);
		 cprintf("%s from %lu to %lu failed.", (type?"Verify":"Seek"),
			 tbeg, tend);
		 gotoxy(X_STATUS, Y_STATUS);
		 cprintf("%s test encountered error.", (type?"Verify":"Seek"));
		 return false;
	  }
   }
   ClrMsg();
   ClrStatus();
   gotoxy(X_STATUS, Y_STATUS);
   cprintf("%s test completed.", (type?"Verify":"Seek"));
   gotoxy(X_MSG, Y_MSG);
   cprintf("Idle.                      ");


   return true;
}

int cMnIDE::TestLinear(int type) {
   long cur, max, per, seccnt, maxcnt;
   int ret, scale;
   char c = 0;

   scale = 250;

   cur = 0;
   per = io[ionum]->MaxLastSector(drvnum) / scale;
   max = io[ionum]->MaxLastSector(drvnum);
   seccnt = 0;
   maxcnt = max / per;

   ClrStatus();
   ClrMsg();

   gotoxy(X_STATUS, Y_STATUS);
   cprintf("Performing %s test. (%lu tracks)", (type?"Verify":"Seek"),
	  maxcnt);

   /* Seek to track 0 to get us started. */
   io[ionum]->Seek(drvnum, 1);

   for (cur = 1; cur <= max; (cur += per, seccnt += 1)) {
//	  ClrMsg();
	  gotoxy(X_MSG, Y_MSG);
	  cprintf("%sing from track %lu -> %lu               ", (type?"Verify":"Seek"),
		  seccnt, seccnt + 1);
      if (kbhit())  c = Input();
	  if (c == ESCAPE) {
		  ClrStatus();
		  ClrMsg();
		 gotoxy(X_STATUS, Y_STATUS);
		 cprintf("Aborted by user.");
		 return false;
	  }
	  if (type == TEST_SEEK)
		  ret = io[ionum]->Seek(drvnum, cur);
	  else
		  ret = io[ionum]->Verify(drvnum, cur, false);
	  if (!ret) {
		 ClrMsg();
	     ClrStatus();
		 gotoxy(X_MSG, Y_MSG);
		 cprintf("%s from %lu to %lu failed.", (type?"Verify":"Seek"),
			 seccnt, seccnt + 1);
		 gotoxy(X_STATUS, Y_STATUS);
		 cprintf("%s test encountered error.", (type?"Verify":"Seek"));
		 return false;
	  }
   }

   ClrMsg();
   ClrStatus();
   gotoxy(X_STATUS, Y_STATUS);
   cprintf("%s test completed.", (type?"Verify":"Seek"));
   gotoxy(Y_MSG, Y_MSG);
   cprintf("Idle.");
   return true;
}

int cMnIDE::TestRandom(int type) {
   return type;
}

/* Execute controller diagnostic */
int cMnIDE::Diag() {
	BYTE res;

	ClrStatus();
	ClrMsg();

    gotoxy(X_STATUS, Y_STATUS);
	cprintf("Executing Diagnostic On Selected Controller");

	res = io[ionum]->ExeDiag();
	if (res == 0xFF) {
		gotoxy(X_MSG, Y_MSG);
		cprintf("Diagnostic Command Failed!");
		return false;
	}

	gotoxy(X_MSG, Y_MSG);
	if (res & 0x10) 
		cprintf("Master passed or nonexistant.");
	else
		cprintf("Master failed!");
	gotoxy(X_MSG, Y_MSG + 1);
	if (res & 0x01) 
		cprintf("Slave passed or nonexistant.");
	else
		cprintf("Slave failed!");

	return true;
}


int cMnIDE::Reset() {
	BYTE res;

	ClrStatus();
	ClrMsg();

	gotoxy(X_STATUS, Y_STATUS);
	cprintf("Reseting Selected Drive");
	res = io[ionum]->Recal(drvnum);

	gotoxy(X_MSG, Y_MSG);
	if (res)
		cprintf("Reset successful.");
	else
		cprintf("Reset failed!");

	return true;
}

int cMnIDE::Eject() {
	int res;

	ClrStatus();
	ClrMsg();
	gotoxy(X_STATUS, Y_STATUS);
	cprintf("Ejecting Selected Drive");

	res = io[ionum]->Eject(drvnum);
	gotoxy(X_MSG, Y_MSG);
	if (res)
		cprintf("Eject successful.");
	else
		cprintf("Eject failed!");
    
	return true;
}


