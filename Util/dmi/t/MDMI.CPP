#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <math.h>

#include "structs.h"
#include "vmain.h"
#include "ioh.h"
#include "constant.h"
#include "video.h"
#include "color.h"

#include "mclass/mdmi.h"
#include "dmi/dmi.h"

#define THISMENU 3

/* Global pointers for special printing functions */
char *d;
char **s;


/* Return TRUE if last operation successful.
   Return -1 if no message was serviced */
int cMDMI::MsgHandler(int msg) {
   /* Ignore messages not for us */
   if (curmenu != THISMENU) return -1;

   switch (msg) {
	  case MMSG_FOCUS: ComeToFocus(); return TRUE;
	  case MMSG_DRAW:  Draw();        return TRUE;
	  case MMSG_DOWN:  if (selected < sysinfo.dmi_items - 1) {
						  ++selected;
						  if (topitem + 18 <= selected) ++topitem;
					   }
					   Draw(); return TRUE;
	  case MMSG_UP:    if (selected > 0) {
						 --selected;
						 if (topitem > selected) --topitem;
					   }
					   Draw(); return TRUE;
	  case MMSG_PGUP:  if (selected > 10) {
						  selected -= 10;
						  if (topitem > 10) topitem -= 10;
						  else topitem = 0;
					   } else {
						  selected = 0; topitem = 0;
					   }
					   Draw(); return TRUE;
	  case MMSG_PGDN: if (selected + 10 < sysinfo.dmi_items)
						 selected += 10;
					  else selected = sysinfo.dmi_items - 1;
					  if (selected > topitem + 18)
						 topitem = selected - 17;
					   Draw(); return TRUE;

   }

   return -1;
}

void cMDMI::InitBuf() {
}

/* Reset our internal variables, reset globals, draw ourselves */
void cMDMI::ComeToFocus() {
   selected = 0;
   topitem = 0;
   Draw();
}


void cMDMI::DrawAscii() {
   int i;

   vput(24, 5, 'Þ', COLOR(C_BACK(CHEADER), C_BACK(BGROUND)));
   vput(24, 6, 'Þ', COLOR(C_BACK(CGUIDE), C_BACK(BGROUND)));
   vput(79, 5, 'Ü', COLOR(C_FORE(CSHADOW), C_BACK(BGROUND)));
   vput(79, 6, 'Û', COLOR(C_FORE(CSHADOW), BLACK));


   vput(2, 5, 'Þ', COLOR(C_BACK(CHEADER), C_BACK(BGROUND)));
   vput(2, 6, 'Þ', COLOR(BLACK, C_BACK(BGROUND)));
   vput(23, 5, 'Ý', COLOR(C_BACK(CHEADER), C_BACK(BGROUND)));
   vput(23, 6, 'Ý', COLOR(BLACK, C_BACK(BGROUND)));


   for (i = 7; i < 25; ++i) {
	  vput(24, i, 'Þ', COLOR(C_BACK(CDATA), C_BACK(BGROUND)));
	  vput(79, i, 'Û', COLOR(C_FORE(CSHADOW), BLACK));
	  if (i < 24)  { vput(2, i, 'Þ', COLOR(BLACK, C_BACK(BGROUND)));
		 vput(23, i, 'Ý', COLOR(BLACK, C_BACK(BGROUND))); }
   }

   for (i = 25; i < 80; ++i)
	  vput(i, 25, 'ß', COLOR(C_FORE(CSHADOW), C_BACK(BGROUND)));

}


void cMDMI::Draw() {
   int i, x, y;

   for (i = 3; i < 23; ++i)
	  set_color(i, 5, CGUIDE);
   textattr(CGUIDE);
   gotoxy(3, 5);
   cprintf("DMI Records: %i  ", sysinfo.dmi_items);

   DrawAscii();

   if (sysinfo.dmi_items == 0) {
	  for (x = 25; x < 79; ++x) {
		 vput(x, 5, ' ', CHEADER); vput(x, 6, ' ', CGUIDE);  }

	  for (y = 7; y < 25; ++y)
		 for (x = 25; x < 79; ++x)
			vput(x, y, ' ', CDATA);

	  for (y = 6; y < 24; ++y)
		 for (x = 3; x < 23; ++x)
			vput(x, y, ' ', COLOR(WHITE, BLACK));

	  textattr(CDATA);
	  gotoxy(26, 7);
	  cprintf("No DMI Structure found.");
	  return;
   }

   for (i = 0; i < 18; ++i) {
	  if ((topitem + i) <= sysinfo.dmi_items) {
		 if (topitem + i == selected) textattr(COLOR(BLACK, GREEN));
		 else textattr(COLOR(WHITE, BLACK));
		 gotoxy(3, 6 + i);
		 if (*sysinfo.dmi[topitem + i].p < MAX_DMI_NAME)
			cprintf("%-20.20s", dmi_name[*sysinfo.dmi[topitem + i].p]);
		 else if (*sysinfo.dmi[topitem + i].p == 126)
			cprintf("%-20.20s", "Inactive Device");
		 else if (*sysinfo.dmi[topitem + i].p == 127)
			cprintf("%-20.20s", "End-of-table");
		 else
			cprintf("%-20.20s", "Unknown Device");
	  }

   } /* for() */

   DrawDMIItem(selected);

}


void chassis_state_string(int state) {
   switch (state) {
	  case 1: cprintf("Other\r\n"); break;
	  case 2: cprintf("Unknown\r\n"); break;
	  case 3: cprintf("Safe\r\n"); break;
	  case 4: cprintf("Warning\r\n"); break;
	  case 5: cprintf("Critical\r\n"); break;
	  case 6: cprintf("Non-recoverable\r\n"); break;
	  default: cprintf("Undefined\r\n");
   }
}


unsigned int mpow(unsigned int b) {
   unsigned int i, r = 2;
   if (b > 16) return 0;
   if (b == 0) return 0;
   for (i = 0; i < b - 1; ++i)
	 r *= 2;
   return r;
}

/* This is a huge function that knows how to display all DMI types. Pass it
   an item and it will display the properly formated data. */
void cMDMI::DrawDMIItem(int item) {
   int i, x, y;
   unsigned long l;
   char c;

   d = sysinfo.dmi[item].p;
   s = sysinfo.dmi[item].s;

   for (x = 25; x < 79; ++x) {
	  vput(x, 5, ' ', CHEADER);
	  vput(x, 6, ' ', CGUIDE);
   }

   for (y = 7; y < 25; ++y)
	  for (x = 25; x < 79; ++x)
		 vput(x, y, ' ', CDATA);


   window(25, 5, 78, 24);
   textattr(CHEADER);
   if (*sysinfo.dmi[item].p < MAX_DMI_NAME)
		 cprintf("%30s [Type %i]\r\n", dmi_name[*sysinfo.dmi[item].p],
			 *sysinfo.dmi[item].p);
   else if (*sysinfo.dmi[item].p == 126)
		 cprintf("%30s [Type %i]\r\n", "Inactive Device", *sysinfo.dmi[item].p);
   else if (*sysinfo.dmi[item].p == 127)
		 cprintf("%30s [Type %i]\r\n", "End-of-table", *sysinfo.dmi[item].p);
   else
		 cprintf("%30s [Type %i]\r\n", "Unknown Device", *sysinfo.dmi[item].p);

   textattr(CGUIDE);
   cprintf("Location %.4X:%.4X   Handle: %.4Xh   Length: %i      \r\n",
			 FP_SEG(d), FP_OFF(d), ((struct dmi_bios *)d)->handle, ((char *)d)[1]);

   textattr(CDATA);
/* OPEN OF HUGE SWITCH */
switch (*sysinfo.dmi[item].p) {
   case DMI_BIOS:
	  aprintf("Vendor: %.50s\r\n", ((struct dmi_bios *)d)->nVendor);
	  aprintf("Version: %.50s\r\n", ((struct dmi_bios *)d)->nVersion);
	  aprintf("Release: %.50s\r\n", ((struct dmi_bios *)d)->nDate);
	  cprintf("Starting Segment: %.4Xh    ROM Size: %ikb\r\n",
		 ((struct dmi_bios *)d)->StartingSeg, (((struct dmi_bios*)d)->RomSize + 1) * 64);
	  cprintf("\r\n");
	  cprintf("Supported Features: ");
	  for (l = 0; l < 32; ++l) {
		 if (  (((struct dmi_bios *)d)->char1) & (((unsigned long) 1) << l)){
			cprintf(" %s", dmi_bios_char[l]);
			if (l < 30) cprintf(" |"); }
	  }
	  if (((char *)d)[1] >= 13) {
	   cprintf("\r\nExtended Characteristics: ");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x1) cprintf("ACPI");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x2) cprintf(" | USB Legacy");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x4) cprintf(" | AGP");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x8) cprintf(" | I20 Boot");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x10) cprintf(" | LS-120 Boot");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x20) cprintf(" | Zip Boot");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x40) cprintf(" | 1394 Boot");
	   if ( ((struct dmi_bios *)d)->ext[0] & 0x80) cprintf(" | Smart Battery");
	  }
	  break;
   case DMI_SYSINFO:
	  aprintf("Manufacturer: %.50s\r\n", ((struct dmi_sysinfo *)d)->nManuf);
	  aprintf("Product: %.50s\r\n", ((struct dmi_sysinfo *)d)->nProduct);
	  aprintf("Version: %.50s\r\n", ((struct dmi_sysinfo *)d)->nVersion);
	  aprintf("Serial: %.50s\r\n", ((struct dmi_sysinfo *)d)->nSerial);
	  cprintf("Wakeup Type: ");
	  switch ( ((struct dmi_sysinfo *)d)->wakeup ) {
		 case 0: cprintf("Reserved\r\n"); break;
		 case 1: cprintf("Other\r\n"); break;
		 case 2: cprintf("Unknown\r\n"); break;
		 case 3: cprintf("APM Timer\r\n"); break;
		 case 4: cprintf("Modem Ring\r\n"); break;
		 case 5: cprintf("LAN Remote\r\n"); break;
		 case 6: cprintf("Power Switch\r\n"); break;
		 case 7: cprintf("PCI PME#\r\n"); break;
		 case 8: cprintf("AC Power Restored\r\n"); break;
		 default: cprintf("Undefined\r\n"); }

	  cprintf("\r\nUUID: ");
	  for (i = 0; i < 16; ++i)
		 cprintf("%.2X ", ((struct dmi_sysinfo *)d)->uuid[i]);
	  cprintf("\r\n");
	  break;
  case DMI_BASE:
	  aprintf("Manufacturer: %.50s\r\n", ((struct dmi_sysinfo *)d)->nManuf);
	  aprintf("Product: %.50s\r\n", ((struct dmi_sysinfo *)d)->nProduct);
	  aprintf("Version: %.50s\r\n", ((struct dmi_sysinfo *)d)->nVersion);
	  aprintf("Serial: %.50s\r\n", ((struct dmi_sysinfo *)d)->nSerial);
	  break;
  case DMI_ENC:
	  aprintf("Manufacturer: %.50s\r\n", ((struct dmi_enc *)d)->nManuf);
	  aprintf("Version: %.50s\r\n", ((struct dmi_enc *)d)->nVersion);
	  aprintf("Serial: %.50s\r\n", ((struct dmi_enc *)d)->nSerial);
	  aprintf("Asset: %.50s\r\n", ((struct dmi_enc *)d)->nAsset);
	  if ( (((struct dmi_enc *)d)->enctype & 0X7F) < MAX_ENC_NAME)
		 cprintf(" Type: %s\r\n", dmi_encname[((struct dmi_enc *)d)->enctype & 0X7F]);
	  else cprintf(" Type: Undefined\r\n");
	  cprintf("\r\n");
	  cprintf("Boot-up State: ");
	  chassis_state_string( ((struct dmi_enc *)d)->state );
	  cprintf("Power Supply State: ");
	  chassis_state_string( ((struct dmi_enc *)d)->power );
	  cprintf("Thermal State: ");
	  chassis_state_string( ((struct dmi_enc *)d)->thermal );
	  cprintf("Security Status: ");
	  switch ( ((struct dmi_enc *)d)->security ) {
		 case 1: cprintf("Other\r\n"); break;
		 case 2: cprintf("Unknown\r\n"); break;
		 case 3: cprintf("None\r\n"); break;
		 case 4: cprintf("External interface locked out\r\n"); break;
		 case 5: cprintf("External interface enabled\r\n"); break;
		 default: cprintf("Undefined\r\n");
	  }
	  break;
  case DMI_PROC:
	  cprintf("Type: ");
	  switch ( ((struct dmi_proc *)d)->proctype ) {
		 case 1: cprintf("Other\r\n"); break;
		 case 2: cprintf("Unknown\r\n"); break;
		 case 3: cprintf("Central Processor\r\n"); break;
		 case 4: cprintf("Math Processor\r\n"); break;
		 case 5: cprintf("DSP Processor\r\n"); break;
		 case 6: cprintf("Video Processor\r\n"); break;
		 default: cprintf("Undefined\r\n");
	  }
	  aprintf("Manufacturer: %.50s\r\n", ((struct dmi_proc *)d)->nManuf);
	  cprintf("Family: ");
	  dmi_cpu_family(((struct dmi_proc *)d)->family);
	  aprintf("Version: %.50s\r\n", ((struct dmi_proc *)d)->nVersion);
	  aprintf("Part Number: %.50s\r\n", ((struct dmi_proc *)d)->nPart);
	  aprintf("Slot: %.50s\r\n", ((struct dmi_proc *)d)->nSocket);
	  cprintf("Upgrade: ");
	  switch (((struct dmi_proc *)d)->upgrade) {
		 case 1: cprintf("Other\r\n"); break;
		 case 2: cprintf("Unknown\r\n"); break;
		 case 3: cprintf("Daughter Board\r\n"); break;
		 case 4: cprintf("ZIF Socket\r\n"); break;
		 case 5: cprintf("Replacement Piggy Back\r\n"); break;
		 case 6: cprintf("None\r\n"); break;
		 case 7: cprintf("LIF Socket\r\n"); break;
		 case 8: cprintf("Slot 1\r\n"); break;
		 case 9: cprintf("Slot 2\r\n"); break;
		 default: cprintf("Undefined [%Xh]\r\n", ((struct dmi_proc *)d)->upgrade);
	  }
	  cprintf(" Status: ");
	  if ( ((struct dmi_proc *)d)->status & (1 << 6) )
		 cprintf("Populated"); else cprintf("Unpopulated");
	  switch ( ((struct dmi_proc *)d)->status & 7) {
		 case 0: cprintf(", Unknown\r\n"); break;
		 case 1: cprintf(", Enabled\r\n"); break;
		 case 2: cprintf(", Disabled via BIOS Setup\r\n"); break;
		 case 3: cprintf(", Disabled via POST Error\r\n"); break;
		 case 4: cprintf(", Idle (waiting for enable)\r\n"); break;
		 case 7: cprintf(", Other\r\n"); break;
		 default: cprintf(", Undefined\r\n"); break;
	  }
	  cprintf("Speed: %i Mhz (Max: %i Mhz, Ext Clock: %i MHz)\r\n",
		 ((struct dmi_proc *)d)->curspeed, ((struct dmi_proc *)d)->maxspeed,
		 ((struct dmi_proc *)d)->clock);

	  cprintf("Voltage: ");
	  if (!((struct dmi_proc *)d)->voltage & 0x80) {
		 if ((((struct dmi_proc *)d)->voltage) & 0x1) cprintf("5v ");
		 if ((((struct dmi_proc *)d)->voltage) & 0x2) cprintf("3.3v ");
		 if ((((struct dmi_proc *)d)->voltage) & 0x4) cprintf("2.9v ");
		 cprintf("\r\n");
	  } else
		 cprintf("%.1fv\r\n",
			  ((float)(((struct dmi_proc *)d)->voltage & 0x7F)) / (float) 10);

	  cprintf("Processor ID: %.4X%.4X %.4X%.4X\r\n",
		 (int) (((struct dmi_proc *)d)->id1 >> 16),
		 (int) (((struct dmi_proc *)d)->id1 & 0xFFFF),
		 (int) (((struct dmi_proc *)d)->id2 >> 16),
		 (int) (((struct dmi_proc *)d)->id2 & 0xFFFF));


	  cprintf("L1 Cache Handle: %.4Xh\r\n", ((struct dmi_proc *)d)->l1);
	  cprintf("L2 Cache Handle: %.4Xh\r\n", ((struct dmi_proc *)d)->l2);
	  cprintf("L3 Cache Handle: %.4Xh\r\n", ((struct dmi_proc *)d)->l3);
  break;
  case DMI_MEMCON:
	 cprintf("Error Correcting Method: ");
	 switch (((struct dmi_memcon *)d)->errdet) {
		case 1: cprintf("Other\r\n"); break;
		case 2: cprintf("Unknown\r\n"); break;
		case 3: cprintf("None\r\n"); break;
		case 4: cprintf("8-bit Parity\r\n"); break;
		case 5: cprintf("32-bit ECC\r\n"); break;
		case 6: cprintf("64-bit ECC\r\n"); break;
		case 7: cprintf("128-bit EEC\r\n"); break;
		case 8: cprintf("CRC\r\n"); break;
		default: cprintf("Undefined\r\n");
	 }
	 cprintf("Error Correcting Capability: ");
	 if (((struct dmi_memcon *)d)->errcor & (1 << 0)) cprintf("Other   ");
	 if (((struct dmi_memcon *)d)->errcor & (1 << 1)) cprintf("Unknown   ");
	 if (((struct dmi_memcon *)d)->errcor & (1 << 2)) cprintf("None   ");
	 if (((struct dmi_memcon *)d)->errcor & (1 << 3)) cprintf("Single Bit   ");
	 if (((struct dmi_memcon *)d)->errcor & (1 << 4)) cprintf("Double Bit   ");
	 if (((struct dmi_memcon *)d)->errcor & (1 << 5)) cprintf("Error Scrubbing   ");
	 cprintf("\r\n");
	 cprintf("Supported Interleave: ");
	 dmi_mem_interleave( ((struct dmi_memcon *)d)->supint );
	 cprintf("Current Interleave: ");
	 dmi_mem_interleave( ((struct dmi_memcon *)d)->curint );
	 cprintf("Supported Speeds: ");
	 if (((struct dmi_memcon *)d)->supspeed & (1 << 0)) cprintf("Other   ");
	 if (((struct dmi_memcon *)d)->supspeed & (1 << 1)) cprintf("Unknown   ");
	 if (((struct dmi_memcon *)d)->supspeed & (1 << 2)) cprintf("70ns  ");
	 if (((struct dmi_memcon *)d)->supspeed & (1 << 3)) cprintf("60ns  ");
	 if (((struct dmi_memcon *)d)->supspeed & (1 << 4)) cprintf("50ns  ");
	 cprintf("\r\n");
	 cprintf("Max Memory Module Size: %i MB\r\n",
		mpow(((struct dmi_memcon *)d)->maxsize));
	 cprintf("Supported Voltages: ");
	 if (((struct dmi_memcon *)d)->voltage & (1 << 0)) cprintf("5v  ");
	 if (((struct dmi_memcon *)d)->voltage & (1 << 1)) cprintf("3.3v  ");
	 if (((struct dmi_memcon *)d)->voltage & (1 << 2)) cprintf("2.9v");
	 cprintf("\r\n");
	 cprintf("Memory Slots: %i\r\n", (char)((struct dmi_memcon *)d)->numslots);
	 cprintf("Supported Memory Types: ");
	 dmi_print_bitfield(((struct dmi_memcon *)d)->suptype, dmi_memname, MAX_MEM_NAMES);
  break;

  case DMI_MEMMOD:
	 aprintf("Socket Designation: %.50s\r\n", ((struct dmi_memmod *)d)->nSocket);
	 cprintf("Current Speed: %i ns\r\n", ((struct dmi_memmod *)d)->speed);
	 cprintf("Bank Connections: ");
	 if ( ((((struct dmi_memmod *)d)->bank) >> 4) != 0xF)
		cprintf("RAS %i  ", ((struct dmi_memmod *)d)->bank >> 4);
	 if ( ((((struct dmi_memmod *)d)->bank) & 0xF) != 0xF)
		cprintf("RAS %i", ((struct dmi_memmod *)d)->bank & 0xF);
	 cprintf("\r\n");
	 cprintf("Installed Size: %i MB, %s\r\n",
		mpow(((struct dmi_memmod *)d)->installsize & 0x7F),
		(((struct dmi_memmod *)d)->installsize & 0x80)?("Single-bank"):("Double-bank"));
	 cprintf("Enabled Size: %i MB, %s\r\n",
		mpow(((struct dmi_memmod *)d)->enablesize & 0x7F),
		(((struct dmi_memmod *)d)->enablesize & 0x80)?("Single-bank"):("Double-bank"));
	 cprintf("Memory Type: ");
	 dmi_print_bitfield(((struct dmi_memmod *)d)->memtype, dmi_memname, MAX_MEM_NAMES);
	 cprintf("Error Code: %.2Xh\r\n", ((struct dmi_memmod *)d)->error);
  break;

  case DMI_CACHE:
	 aprintf("Socket Designation: %.50s\r\n", ((struct dmi_cache *)d)->nSocket);
	 cprintf("Cache Configuration: ");
	 switch ( (((struct dmi_cache *)d)->config & 0x300) >> 8 ) {
		case 0: cprintf("Write Through, "); break;
		case 1: cprintf("Write Back, "); break;
		case 2: cprintf("Mode Varies, "); break;
		case 3: cprintf("Unknown, "); break;
	 }
	 if ( ((struct dmi_cache *)d)->config & 0x80 )
		cprintf("Enabled, "); else cprintf("Disabled, ");
	 switch ( (((struct dmi_cache *)d)->config & 0x60) >> 5 ) {
		case 0: cprintf("Internal, "); break;
		case 1: cprintf("External, "); break;
		case 2: cprintf("Reserved, "); break;
		case 4: cprintf("Unknown, "); break;
	 }
	 if ( ((struct dmi_cache *)d)->config & 0x8 )
		cprintf("Socketed, "); else cprintf("Not Socketed, ");
	 cprintf("Level %i\r\n", (((struct dmi_cache *)d)->config & 0x7) + 1);
	 cprintf("Maximum Size: %i KB\r\n",
		 (word)(((struct dmi_cache *)d)->maxsize & 0x8000)?64:1 *
		 (((struct dmi_cache *)d)->maxsize & 0x7FFF));
	 cprintf("Installed Size: %i KB\r\n",
		 (word)(((struct dmi_cache *)d)->installsize & 0x8000)?64:1 *
		 (((struct dmi_cache *)d)->installsize & 0x7FFF));
	 cprintf("Speed: %i ns\r\n", ((struct dmi_cache *)d)->speed);
	 cprintf("Supported SRAM Type: ");
	 dmi_sram_type(((struct dmi_cache *)d)->suptype);
	 cprintf("Current SRAM Type: ");
	 dmi_sram_type(((struct dmi_cache *)d)->curtype);
	 cprintf("Error Correction Type: ");
	 switch ( ((struct dmi_cache *)d)->error ) {
		case 1: cprintf("Other\r\n"); break;
		case 2: cprintf("Unknown\r\n"); break;
		case 3: cprintf("None\r\n"); break;
		case 4: cprintf("Parity\r\n"); break;
		case 5: cprintf("Single-bit ECC\r\n"); break;
		case 6: cprintf("Multi-bit ECC\r\n"); break;
		default: cprintf("Undefined\r\n");
	 }
	 cprintf("Cache Type: ");
	 switch ( ((struct dmi_cache *)d)->ctype ) {
		case 1: cprintf("Other\r\n"); break;
		case 2: cprintf("Unknown\r\n"); break;
		case 3: cprintf("Instruction\r\n"); break;
		case 4: cprintf("Data\r\n"); break;
		case 5: cprintf("Unified\r\n"); break;
		default: cprintf("Undefined\r\n");
	 }
	 cprintf("Associativity: ");
	 switch ( ((struct dmi_cache *)d)->assoc ) {
		case 1: cprintf("Other\r\n"); break;
		case 2: cprintf("Unknown\r\n"); break;
		case 3: cprintf("Direct Mapped\r\n"); break;
		case 4: cprintf("2-way Set-Associative\r\n"); break;
		case 5: cprintf("4-way Set-Associative\r\n"); break;
		case 6: cprintf("Fully Associative\r\n"); break;
		default: cprintf("Undefined\r\n");
	 }
  break;

  case DMI_PORT:
	 aprintf("Internal Name: %.50s\r\n", ((struct dmi_port *)d)->nIntRef);
	 cprintf("Internal Connector: ");
	 if ( ((struct dmi_port *)d)->inttype < MAX_PORTCON_NAMES )
		cprintf("%s\r\n", dmi_portcon[((struct dmi_port *)d)->inttype]);
	 else cprintf("Unknown\r\n");
	 aprintf("External Name: %.50s\r\n", ((struct dmi_port *)d)->nExtRef);
	 cprintf("External Connector: ");
	 if ( ((struct dmi_port *)d)->exttype < MAX_PORTCON_NAMES )
		cprintf("%s\r\n", dmi_portcon[((struct dmi_port *)d)->exttype]);
	 else cprintf("Unknown\r\n");
  break;

  case DMI_SLOT:
	 aprintf("Slot Name: %.50s\r\n", ((struct dmi_slot *)d)->nName);
	 cprintf("Slot Type: %.50s\r\n",
	   ((struct dmi_slot *)d)->stype < MAX_SLOT_TYPE_NAMES
	   ? dmi_slot_type[((struct dmi_slot *)d)->stype]: "Undefined" );
	 cprintf("Slot Data Bus Width: %.50s\r\n",
	   ((struct dmi_slot *)d)->width < MAX_SLOT_WIDTH_NAMES
	   ? dmi_slot_width[((struct dmi_slot *)d)->width]: "Undefined" );
	 cprintf("Current Usage: %.50s\r\n",
	   ((struct dmi_slot *)d)->width < MAX_SLOT_USAGE_NAMES
	   ? dmi_slot_usage[((struct dmi_slot *)d)->usage]: "Unknown" );
	 cprintf("Slot Length: %.50s\r\n",
	   ((struct dmi_slot *)d)->slength < MAX_SLOT_LENGTH_NAMES
	   ? dmi_slot_length[((struct dmi_slot *)d)->slength]: "Unknown" );
	 cprintf("Slot ID: %.4Xh\r\n", ((struct dmi_slot *)d)->id);
	 cprintf("Slot Charar1: ");
	 dmi_print_bitfield( ((struct dmi_slot *)d)->char1, dmi_slot_char1, MAX_SLOT_CHAR1);
	 cprintf("Slot Charar2: ");
	 dmi_print_bitfield( ((struct dmi_slot *)d)->char2, dmi_slot_char2, MAX_SLOT_CHAR2);
  break;

  /* I'm not sure how the strings variable works. I'm assuming that 4
	 strings means s[4] is the last string. Type 11 and 12 are same, so
	 just have them use the same function and struct.                   */
  case DMI_OEMINFO:
  case DMI_SYSCONFIG:
	 for (i = 1; (i <= ((struct dmi_oeminfo *)d)->strings) && i < MAX_DMI_DATA_STRING; ++i)
		cprintf("String %i: %.50s\r\n", i, s[i]);
  break;

  /* Again I assume s[numlang] is the last string */
  case DMI_LANG:
	 cprintf("Installable Languages: %i\r\n", ((struct dmi_lang *)d)->numlang);
	 cprintf("Flags: %.2Xh\r\n", ((struct dmi_lang *)d)->flags);
	 aprintf("Current Language  : %.50s\r\n", ((struct dmi_lang *)d)->nLanguage);
	 cprintf("\r\n");
	 for (i = 1; (i <= ((struct dmi_lang *)d)->numlang) && i < MAX_DMI_DATA_STRING; ++i)
		cprintf("Supported Language: %.50s\r\n", s[i]);
  break;

  case DMI_PHYMEM:
	 cprintf("Location: %.50s\r\n",
	   ((struct dmi_phymem *)d)->loc < MAX_MEM_LOC
	   ? dmi_mem_loc[((struct dmi_phymem *)d)->loc]: "Undefined" );
	 cprintf("Use: %.50s\r\n",
	   ((struct dmi_phymem *)d)->loc < MAX_MEM_USE
	   ? dmi_mem_use[((struct dmi_phymem *)d)->use]: "Undefined" );
	 cprintf("Error Correction: %.50s\r\n",
	   ((struct dmi_phymem *)d)->error < MAX_MEM_ERROR
	   ? dmi_mem_error[((struct dmi_phymem *)d)->error]: "Undefined" );
	 cprintf("Error Handle: %.4Xh\r\n", ((struct dmi_phymem *)d)->errhandle);
	 cprintf("Num of Devices: %i\r\n", ((struct dmi_phymem *)d)->num);
	 cprintf("Maximum Capacity: ");
	 if ( ((struct dmi_phymem *)d)->capacity == 0x80000000)
		cprintf("Unknown\r\n");
	 else
		cprintf("%lu KB\r\n", ((struct dmi_phymem *)d)->capacity);
  break;

  case DMI_MEMDEV:
	 aprintf("Device: %.50s\r\n", ((struct dmi_memdev *)d)->nDev);
	 aprintf("Bank: %.50s\r\n", ((struct dmi_memdev *)d)->nBank);
	 cprintf("Speed: ");
	 if ( ((struct dmi_memdev *)d)->speed == 0 ) cprintf("Unknown\r\n");
	 else cprintf("%i MHz  (%.0f ns)\r\n", ((struct dmi_memdev *)d)->speed,
		(float) 1000000 / (float) ((struct dmi_memdev *)d)->speed);
	 cprintf("Memory Array Handle: %.4Xh\r\n", ((struct dmi_memdev *)d)->memhandle);
	 cprintf("Error Array Handle: %.4Xh\r\n", ((struct dmi_memdev *)d)->errhandle);

	 if ( ((struct dmi_memdev *)d)->datawidth == 0xFFFF )
		cprintf("Datal Width: Unknown   ");
	 else cprintf("Data Width: %i   ", ((struct dmi_memdev *)d)->datawidth);
	 if ( ((struct dmi_memdev *)d)->totalwidth == 0xFFFF )
		cprintf("(Total: Unknown)\r\n");
	 else cprintf("(Total: %i)\r\n", ((struct dmi_memdev *)d)->totalwidth);

	 cprintf("Module Size: ");
	 if (((struct dmi_memdev *)d)->size == 0xFFFF) cprintf("Unknown\r\n");
	 else {
		if (((struct dmi_memdev *)d)->size & 0x8000)
		   cprintf("%i KB\r\n", ((struct dmi_memdev *)d)->size & 0x7FFF);
		else
		   cprintf("%i MB\r\n", ((struct dmi_memdev *)d)->size & 0x7FFF);
	 }

	 cprintf("Memory Type: %.50s\r\n",
	   ((struct dmi_memdev *)d)->memtype < MAX_MEM_TYPE
	   ? dmi_mem_type[((struct dmi_memdev *)d)->memtype]: "Undefined" );
	 cprintf("Type Detail: %.50s\r\n",
	   ((struct dmi_memdev *)d)->typedetail < MAX_MEM_TYPE
	   ? dmi_mem_detail[((struct dmi_memdev *)d)->typedetail]: "Undefined" );
	 cprintf("Form Factor: %.50s\r\n",
	   ((struct dmi_memdev *)d)->form < MAX_MEM_FORM
	   ? dmi_mem_form[((struct dmi_memdev *)d)->form]: "Undefined" );

	 aprintf("Manufacturer: %.50s\r\n", ((struct dmi_memdev *)d)->nManuf);
	 aprintf("Serial: %.50s\r\n", ((struct dmi_memdev *)d)->nSerial);
	 aprintf("Part Number: %.50s\r\n", ((struct dmi_memdev *)d)->nPart);
	 aprintf("Asset: %.50s\r\n", ((struct dmi_memdev *)d)->nAsset);
	 cprintf("Device Set: %.2Xh\r\n", ((struct dmi_memdev *)d)->deviceset);
  break;

  case DMI_MEMERR:
	 cprintf("Error Type: %s\r\n",
	   ((struct dmi_memerr *)d)->errtype < MAX_ERR_TYPE
	   ? dmi_err_type[((struct dmi_memerr *)d)->errtype]: "Undefined" );
	 cprintf("Error Granularity: %s\r\n",
	   ((struct dmi_memerr *)d)->errgran < MAX_ERR_GRAN
	   ? dmi_err_gran[((struct dmi_memerr *)d)->errgran]: "Undefined" );
	 cprintf("Error Operation: %s\r\n",
	   ((struct dmi_memerr *)d)->errop < MAX_ERR_OP
	   ? dmi_err_op[((struct dmi_memerr *)d)->errop]: "Undefined" );

	 cprintf("Vendor Syndrome: %X %Xh\r\n",
		FP_OFF(((struct dmi_memerr *)d)->vendsyn),
		FP_SEG(((struct dmi_memerr *)d)->vendsyn));
	 cprintf("Memory Array Error Address: %X %Xh\r\n",
		FP_OFF(((struct dmi_memerr *)d)->memerr),FP_SEG(((struct dmi_memerr *)d)->memerr));
	 cprintf("Device Error Address: %X %Xh\r\n",
		FP_SEG(((struct dmi_memerr *)d)->deverr),FP_OFF(((struct dmi_memerr *)d)->deverr));
	 cprintf("Error Resolution: %X %Xh\r\n",
		FP_SEG(((struct dmi_memerr *)d)->errres), FP_OFF(((struct dmi_memerr *)d)->errres));
  break;

  case DMI_MEMARRAY:
	 cprintf("Starting Address: %.4X%.4Xh\r\n",
		FP_SEG(((struct dmi_memarray *)d)->start),FP_OFF(((struct dmi_memarray *)d)->start));
	 cprintf("Ending Address: %.4X%.4Xh\r\n",
		FP_SEG(((struct dmi_memarray *)d)->ending),FP_OFF(((struct dmi_memarray *)d)->ending));
	 cprintf("Memory Array Handle: %.4Xh\r\n", ((struct dmi_memarray *)d)->memhandle);
	 cprintf("Partition Width: %i\r\n", ((struct dmi_memarray *)d)->width);
  break;

  case DMI_MAPPED:
	 cprintf("Starting Address: %.4X%.4Xh\r\n",
		FP_SEG(((struct dmi_mapped *)d)->start),FP_OFF(((struct dmi_mapped *)d)->start));
	 cprintf("Ending Address: %.4X%.4Xh\r\n",
		FP_SEG(((struct dmi_mapped *)d)->ending),FP_OFF(((struct dmi_mapped *)d)->ending));
	 cprintf("Memory Device Handle: %.4Xh\r\n", ((struct dmi_mapped *)d)->devhandle);
	 cprintf("Memory Array Mapped Address Handle: %.4Xh\r\n", ((struct dmi_mapped *)d)->addrhandle);
	 cprintf("Partition Row Position: %.2Xh\r\n", ((struct dmi_mapped *)d)->row);
	 cprintf("Interleave Position: %.2Xh\r\n", ((struct dmi_mapped *)d)->inter);
	 cprintf("Interleaved Data Depth: %.2Xh\r\n", ((struct dmi_mapped *)d)->interdepth);
  break;

  case DMI_BOOT:   /* type 32 */
	 cprintf("Byte dump of boot status:\r\n   - ");
	 for (i = 0xA; i < ((char *)d)[1]; ++i) {
		cprintf("%.2X ", ((char *)d)[i]);
	 }
  break;

  default: cprintf("Undecoded DMI Type\r\n");
		   cprintf("Byte Dump\r\n  - ");
		   for (i = 0; i < ((char *)d)[1]; ++i)
				  cprintf("%.2X ", ((char *)d)[i]);


} /* END OF HUGE SWITCH */

  window(1, 1,  80, 25);
}


/* I'm not listing a bunch of the wierd crap in here, like 8088's or 68k's. */
void cMDMI::dmi_cpu_family(int type) {
   switch (type) {
	  case 1: cprintf("Other\r\n"); break;
	  case 2: cprintf("Unknown\r\n"); break;
	  case 5: cprintf("Intel 386\r\n"); break;
	  case 6: cprintf("Intel 486\r\n"); break;
	  case 0xA: cprintf("80487\r\n"); break;
	  case 0xB: cprintf("Pentium\r\n"); break;
	  case 0xC: cprintf("Pentium Pro\r\n"); break;
	  case 0xD: cprintf("Pentium 2\r\n"); break;
	  case 0xE: cprintf("Pentium w/MMX\r\n"); break;
	  case 0xF: cprintf("Celeron\r\n"); break;
	  case 0x10: cprintf("Pentium 2 Xeon\r\n"); break;
	  case 0x11: cprintf("Pentium 3\r\n"); break;
	  case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	  case 0x18: cprintf("M1 Family\r\n"); break;
	  case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
	  case 0x1F: cprintf("K5 Family\r\n"); break;
	  case 0x20: cprintf("Power PC\r\n"); break;
	  case 0xB0: cprintf("Pentium 3 Xenon\r\n"); break;
	  default: cprintf("Undefined [%.2Xh]\r\n", type);
   }
}



void cMDMI::dmi_mem_interleave(int type) {
   switch (type) {
	  case 1: cprintf("Other\r\n"); break;
	  case 2: cprintf("Unknown\r\n"); break;
	  case 3: cprintf("1-Way\r\n"); break;
	  case 4: cprintf("2-Way\r\n"); break;
	  case 5: cprintf("4-Way\r\n"); break;
	  case 6: cprintf("8-Way\r\n"); break;
	  case 7: cprintf("16-Way\r\n"); break;
	  default: cprintf("Undefined\r\n");
   }
}


void cMDMI::dmi_sram_type(int item) {
   int i, once = 0;

   for (i = 0; i < MAX_SRAM_NAMES; ++i)
	  if (item & (1 << i))
		 if (once)
			cprintf(", %s", dmi_sramname[i]);
		 else {
			once = 1;
			cprintf("%s", dmi_sramname[i]);
		 }
   cprintf("\r\n");
}

void cMDMI::dmi_print_bitfield(int value, const char **s, int maxitem) {
   int i, once = 0;

   for (i = 0; i < maxitem; ++i) {
	  if (value & (1 << i)) {
		 if (once)
			cprintf(", %s", s[i]);
		 else {
			once = 1;
			cprintf("%s", s[i]);
		 }
	  }
   }
   cprintf("\r\n");
}



/* This function takes a formating string and a string number, and if
   its a valid string, cprint it out. A simple wrapper for cprintf()   */
void cMDMI::aprintf(char *format, int stringnum) {


   if (stringnum < MAX_DMI_DATA_STRING)
	  cprintf(format, s[stringnum]);
   else
	  cprintf(format, "  ");
}