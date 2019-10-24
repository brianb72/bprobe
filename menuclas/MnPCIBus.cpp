#include <stdio.h>
#include <conio.h>
#include <string.h>

#include "declare/global.h"
#include "menuclas/MnPCIBus.h"


cMnPCIBus::cMnPCIBus() {
   namedone = FALSE;
   scanned = FALSE;
   wy = 0;
   memset(VendorName, 0, sizeof(VendorName));
   memset(DeviceName, 0, sizeof(DeviceName));
}

cMnPCIBus::~cMnPCIBus() {
}

/* Very simple, scan the bus with our inherited functions, display it, 
   and wait for escape. We CANNOT write to y=1 or y=2.                
   y = 4 is our toolbar, y = 6-23 is devices. Truncate if too many
   Max Text Strings    
   class    = 10
   subclass = 11
   Vendor   = 15
   Device   = 30
 B  D F  Class      Subclass    Vendor           Device Name
 3 23 5  1234567890 12345678901 123456789012345  123456789012345678901234567890

*/


void cMnPCIBus::Main(int menudir) {
   int i, change;
   char c;

   if (menudir == 0x72) {
	  if (ScanPciBus()) scanned = TRUE;
	  return;
   }
   if (menudir == 0x73) {
	  if (namedone) return;
	  get_pci_name(VendorName, DeviceName, 255);
	  namedone = TRUE;
	  return;
   }


   DrawAscii();
	gotoxy( (75 / 2) - (15 / 2) + 3, 5);
	cprintf("PCI BUS DEVICES");
	HighLight(4, 5, 74, COLOR(WHITE, BLUE));

   window(4, 6, 78, 21);
   gotoxy(1, 1);
   cprintf("B D  F  Class      Subclass    Vendor          Device Name\n\r");
   /* Highlight does not recognize window */
//   HighLight(4, 6, 74, C_PC_GUIDE);


  if (scanned == FALSE) {
	   cprintf("Unable to locate or scan PCI bus.\n\r");
  }
  else {
	  /* Populate the device name structure */
//
	  // Draw the fake list
	  if (!namedone) {
	  for (i = 0; (i < 15 && i <= maxdev); ++i) {
		 if (device[i].reg.VendorID != 0 && device[i].reg.VendorID != 0xFFFF) {
			sprintf(VendorName[i], "%.4Xh", device[i].reg.VendorID);
			sprintf(DeviceName[i], "%.4Xh", device[i].reg.DeviceID);
		 }
	  }
	  }
	  DrawNames();
  }
  window(1, 1, 80, 25);


	gotoxy(2,25);
	cprintf("[Left/Right] Select Menu  [Up/Down] Select Device");

	HighLight(1, 25, 80, LC_TOOLBAR);

	SetCharColor(2,  25, LC_BRACKET);
	SetCharColor(13, 25, LC_BRACKET);
	HighLight(3, 25, 10, LC_KEY);

	SetCharColor(28,  25, LC_BRACKET);
	SetCharColor(36, 25, LC_BRACKET);
	HighLight(29, 25, 7, LC_KEY);


  c = 0;
  i = -1;
  while(c != ESCAPE) {
	 change = FALSE;
	 c = Input();
	 switch (c) {
		case KEY_UP:
		case KEY_8: if (i > 0) {
					   change = TRUE;
					   HighLight(4, 7 + i, 74, C_PC_DATA); // unhl
					   --i;
					   HighLight(4, 7 + i, 74, C_PC_HL); // unhl
					}
					break;

		case KEY_DOWN:
		case KEY_2:
					if (i < (maxdev - 1) && i < 15) {
					   change = TRUE;
					   if (i != -1)
						   HighLight(4, 7 + i, 74, C_PC_DATA); // unhl
					   ++i;
					   HighLight(4, 7 + i, 74, C_PC_HL); // unhl
					}
					break;
		 case KEY_LEFT:
		 case KEY_4:     if (!(menudir & MENULEFT)) continue;
						ungetch(c); c = ESCAPE;  break;
		 case KEY_RIGHT:
		 case KEY_6:     if (!(menudir & MENURIGHT)) continue;
						ungetch(c); c = ESCAPE;  break;
		 case 'p':
		 case 'P':
		 case ESCAPE:   ungetch(c); c = ESCAPE; break;
		 case ENTER:    if (!namedone) {
					   if (i != -1)
						   HighLight(4, 7 + i, 74, C_PC_DATA); // unhl
						   window(4, 21, 78, 22);   // fast erase of old text
						   gotoxy(1, 1); delline();delline();
						   window(1, 1, 80, 25);
						   window(4, 7, 78, 21);
						   get_pci_name(VendorName, DeviceName, wy);
//						   gotoxy(1, 1); delline();
						   namedone = TRUE;
						   DrawNames();
						   window(1, 1, 80, 25);
					   if (i != -1) {
						 HighLight(4, 7 + i, 74, C_PC_HL); // unhl
						 change = TRUE;
					   }
						 }

	 } /* switch() */

	 if (change) {
		window(4, 21, 78, 22);   // fast erase of old text
		gotoxy(1, 1); delline();delline();
		window(1, 1, 80, 25);
		gotoxy(4, 21);
		cprintf("Vendor [%.4Xh] %.58s", device[i].reg.VendorID,
											VendorName[i]);
		gotoxy(4, 22);
		cprintf("Device [%.4Xh] %.58s", device[i].reg.DeviceID,
											DeviceName[i]);
	 }
  } /* while() */



 }



void cMnPCIBus::DrawAscii() {
   int i;


   DrawBox(3, 5, 78, 22, FALSE, FALSE, C_PC_DATA, C_DM_LINE);


}


void cMnPCIBus::DrawNames() {
   int i, vlen, dlen;
   char tbuf[150];
   char ClassName[50], SubClassName[50];

   window(4, 7, 77, 21);
   gotoxy(1, 1);

	  for (i = 0; (i < 15 && i <= maxdev); ++i) {
		  if (device[i].reg.VendorID != 0 && device[i].reg.VendorID != 0xFFFF) {
			 get_pci_classname(device[i].reg.BaseClass, device[i].reg.SubClass,
					ClassName, SubClassName);
			 vlen = strlen(VendorName[i]);
			 dlen = strlen(DeviceName[i]);
/*			  cprintf("%-2i%-3i%-2i %-11.10s%-12.11s%-17.15s%.27s",
				device[i].bus, device[i].dev, device[i].func,
				ClassName, SubClassName, VendorName, DeviceName);  */
			  cprintf("%-2i%-3i%-2i %-11.10s%-12.11s",
				device[i].bus, device[i].dev, device[i].func,
				ClassName, SubClassName);

			 if (vlen <= 14) cprintf("%-16.14s", VendorName[i]);
			 else {
				sprintf(tbuf, "%.12s..", VendorName[i]);
				cprintf("%-16s", tbuf);
			 }
			 if (dlen <= 26) cprintf("%.26s", DeviceName[i]);
			 else cprintf("%.24s..", DeviceName[i]);
			 cprintf("\r\n");
		  }
	  }

	  if (!namedone) {
		 wy = wherey() + 1;
		 gotoxy(20, wy);
		 cprintf("[Press Enter To Load Device Names]");
        
	  }
	  else {
		 gotoxy(1, wy);
		 delline();
		 delline();
	  }

   window(1, 1, 80, 25);
}