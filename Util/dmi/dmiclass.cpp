#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>



#include "declare/global.h"
#include "util/dmi/dmiclass.h"
#include "util/dmi/dmiconst.h"



cDMI::cDMI() {
	memset(dmi, 0, sizeof(dmi));
	dmi_items = 0;
}
cDMI::~cDMI() { }



/* *************************************************************************** */
/* Locate the smbios entry point											   */	
/* *************************************************************************** */
char *cDMI::get_smbios_entry(unsigned int off, unsigned int seg) {
   char far *bios;
   unsigned long offset, segment;

   offset  = off;
   segment = seg;


   while (1) {
	  bios = (unsigned char *)MK_FP(offset, segment);
	  if (bios[0] == '_' && bios[1] == 'S' && bios[2] == 'M' &&
		  bios[3] == '_')
		if ( (bios[16] && '_') && (bios[17] == 'D') && (bios[18] == 'M') &&
			(bios[19] == 'I') && (bios[20] && '_'))
			break;
	  if ( (bios[0] == '_') && (bios[1] == 'D') && (bios[2] == 'M') &&
			(bios[3] == 'I') && (bios[4] == '_'))
			break;
	  segment += 16;
	  if (segment > 0xffff) {
		oThrob->Step();
		segment = 0; ++offset;
		if (offset > 0xffff) {
		   LOGLO(" Error: Offset rolled over.");
		 break;
		}
	  }
   }

   if (offset > 0xFFFF)
	  return 0;
   else
	  return bios;
}


/* *************************************************************************** */
/* Rollconvert - Hack to get around borland << limitations                     */
/* *************************************************************************** */

unsigned long cDMI::rconv(char *addr) {
	return ( (FP_SEG(addr) * 16) + FP_OFF(addr) );
}


/* *************************************************************************** */
/* scan_dmi - Scan the DMI table and populate our structure                    */
/* *************************************************************************** */

int cDMI::scan_dmi(void) {
   char far *SMEntry;
   char far *bp;
   struct dmi_entry_data *sm;
   int i, t, nc;
   unsigned long off = 0, seg = 0;

   LOGLO("--- DMI SCAN ---");

   if ((SMEntry = get_smbios_entry(0xf000, 0x0000)) == 0) {
	  LOGLO("Fatal Error: Cannot locate SMBIOS entry point. No table found.");
	  return FALSE;
   }

   LOGHI("SMBIOS found at %X:%X\n", FP_SEG(SMEntry), FP_OFF(SMEntry));

   if (SMEntry[1] == 'S')
	  sm = &((struct smbios_entry_data*) SMEntry)->dmi;
   else if (SMEntry[1] == 'D')
	  sm = (struct dmi_entry_data *)SMEntry;
   else {
	  LOGLO("Fatal Error: SMBIOS entry point is invalid. No table found.");
	  return FALSE;
   }

   /* Reset item count, we're starting over */
   dmi_items = 0;


   seg = sm->TableAddr / 16;
   off = sm->TableAddr % 16;
   bp = (char *)MK_FP(seg, off);
   LOGHI("START found at %.4X:%.4X (%X%Xh) %X:%X\n", FP_SEG(bp), FP_OFF(bp),
	   rconv(bp), seg, off);
   LOGHI("Number of structures: %i\n", sm->NumStructure);

   dmi_items = sm->NumStructure;
   /* HUGE FOR LOOP, parse all devices */
   for (i = 0; i < sm->NumStructure; ++i) {
	  oThrob->Step();
	  dmi[i].p = bp;                     /* set pointer */

	  if (*dmi[i].p < MAX_DMI_NAME)
		 LOGHI("%.4X:%.4X (%X%Xh) - %i - %-25s (Device %i) has length %i.\n",
			 FP_SEG(bp), FP_OFF(bp), rconv(bp), i,
			 dmi_name[*dmi[i].p], *dmi[i].p, bp[1]);
	  else if (*dmi[i].p == 126)
		 LOGHI("%.4X:%.4X - %i - Inactive Device has length %i.\n",
			 FP_SEG(bp), FP_OFF(bp), i, bp[1]);
	  else if (*dmi[i].p == 127)
		 LOGHI("%.4X:%.4X - %i - End-of-table has length %i.\n",
			 FP_SEG(bp), FP_OFF(bp), i, bp[1]);
	  else
		 LOGHI("%.4X:%.4X  - %i - Unknown %i.\n",
						 FP_SEG(bp), FP_OFF(bp), i, *(char *)dmi[i].p);
	  if ((bp[0] == 127) && (bp[1] == 4)) {
		  dmi_items = i + 1;
		  LOGHI("Number of structures found to really be: %i\n", dmi_items);
		  break;  /* break the for loop */
	  }


	  /* One Laptop BIOS I found so far has length pointing AFTER the
		 double null on the 32-bit memory error structure. Detect this to
		 avoid screwing up the structure chain. */
	  if ((bp[0] == 18) && (bp[1] != 0x17) ) {
		 if (bp[0x17] == 0 && bp[0x18] == 0) {
			#if DEBUGLOW
//			   fprintf(db, "   Structure 18 size error detected. Using workaround.\n");
			#endif
			bp += 0x17;
		 }
	  }
	  /* Add any additional workarounds in else if's */
	  /* The default method is in this else, don't let it get fubared */
	  else
		 bp += bp[1];         /* Skip past */

	  LOGHI("   Strings start at %X:%X\n", FP_SEG(bp), FP_OFF(bp));
	  t = 1;
	  dmi[i].s[0] = nonestring[0];
	  while (*((int *)bp) != 0 && t < MAX_DMI_DATA_STRING) {
		 if (*bp == '\0') ++bp;
		 dmi[i].s[t] = bp;
		 bp += strlen(dmi[i].s[t]);
		 LOGHI("   found string: %.64s\n", dmi[i].s[t]);
		 ++t;
	  }

	  bp += 2;

	  /* A few bios's have several 00 's after the string terminator  00 00.
		 For now attempt to condense this */
	  if (*bp == 0) {
		 for (nc = 0; nc < 20 && (*bp == 0); ++nc)  ++bp;
			LOGHI("   %X:%Xh - %i Excess Null's found.\n", FP_SEG(bp), FP_OFF(bp), nc);
		 if (*bp == 0) {
			dmi_items = i + 1;
			   LOGHI("   Too many null's, truncating list to %i items.\n", dmi_items);
			break;  /* break huge for */
		 }
	  } /* end of if (*bp == 0) */

   } /* END OF HUGE FOR LOOP */


   return 1;
}






