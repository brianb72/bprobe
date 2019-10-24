#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include "structs.h"
#include "vmain.h"
#include "dmi/dmi.h"
#include "constant.h"

// Locate the smbios entry point
char *get_smbios_entry(unsigned int off, unsigned int seg) {
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
		segment = 0; ++offset;
		if (offset > 0xffff) {
		 #if DEBUGLOW
			fprintf(db, "   Offset rolled over\n");
		 #endif
		 break;
		}
	  }
   }

   if (offset > 0xFFFF)
	  return 0;
   else
	  return bios;
}

unsigned long rconv(char *addr) {
	return ( (FP_SEG(addr) * 16) + FP_OFF(addr) );
}

int scan_dmi(void) {
   char far *SMEntry;
   char far *bp;
   struct dmi_entry_data *sm;
   int i, t, nc;
   unsigned long off = 0, seg = 0;

   #if DEBUGLOW
	  fprintf(db, "\n---DMI Scan-----------\n");
   #endif

   if ((SMEntry = get_smbios_entry(0xf000, 0x0000)) == 0) {
	  #if DEBUGLOW
		 fprintf(db, "Cannot locate SMBIOS entry point.\n");
	  #endif
	  return FALSE;
   }
   #if DEBUGLOW
	  fprintf(db, "SMBIOS found at %X:%X\n", FP_SEG(SMEntry), FP_OFF(SMEntry));
   #endif

   if (SMEntry[1] == 'S')
	  sm = &((struct smbios_entry_data*) SMEntry)->dmi;
   else if (SMEntry[1] == 'D')
	  sm = (struct dmi_entry_data *)SMEntry;
   else {
	  #if DEBUGLOW
		 fprintf(db, "SMBIOS invalid.\n");
	  #endif
	  return FALSE;
   }

   /* calloc off our space for dmi information, extra for safety */
   sysinfo.dmi = (struct dmi_data *) calloc(sm->NumStructure,
				 sizeof(struct dmi_data) + 2);


   seg = sm->TableAddr / 16;
   off = sm->TableAddr % 16;
   bp = (char *)MK_FP(seg, off);
   fprintf(db,"START found at %.4X:%.4X (%X%Xh) %X:%X\n", FP_SEG(bp), FP_OFF(bp),
	   rconv(bp), seg, off);
   fprintf(db,"Number of structures: %i\n", sm->NumStructure);

   sysinfo.dmi_items = sm->NumStructure;
   /* HUGE FOR LOOP, parse all devices */
   for (i = 0; i < sm->NumStructure; ++i) {
	  sysinfo.dmi[i].p = bp;                     /* set pointer */

	  if (*sysinfo.dmi[i].p < MAX_DMI_NAME)
		 fprintf(db,"%.4X:%.4X (%X%Xh) - %i - %-25s (Device %i) has length %i.\n",
			 FP_SEG(bp), FP_OFF(bp), rconv(bp), i,
			 dmi_name[*sysinfo.dmi[i].p], *sysinfo.dmi[i].p, bp[1]);
	  else if (*sysinfo.dmi[i].p == 126)
		 fprintf(db, "%.4X:%.4X - %i - Inactive Device has length %i.\n",
			 FP_SEG(bp), FP_OFF(bp), i, bp[1]);
	  else if (*sysinfo.dmi[i].p == 127)
		 fprintf(db, "%.4X:%.4X - %i - End-of-table has length %i.\n",
			 FP_SEG(bp), FP_OFF(bp), i, bp[1]);
	  else
		 fprintf(db,"%.4X:%.4X  - %i - Unknown %i.\n",
						 FP_SEG(bp), FP_OFF(bp), i, *(char *)sysinfo.dmi[i].p);
	  if ((bp[0] == 127) && (bp[1] == 4)) {
		  sysinfo.dmi_items = i + 1;
		  #if DEBUGLOW
			 fprintf(db, "Number of structures found to really be: %i\n", sysinfo.dmi_items);
		  #endif
		  break;  /* break the for loop */
	  }


	  /* One Laptop BIOS I found so far has length pointing AFTER the
		 double null on the 32-bit memory error structure. Detect this to
		 avoid screwing up the structure chain. */
	  if ((bp[0] == 18) && (bp[1] != 0x17) ) {
		 if (bp[0x17] == 0 && bp[0x18] == 0) {
			#if DEBUGLOW
			   fprintf(db, "   Structure 18 size error detected. Using workaround.\n");
			#endif
			bp += 0x17;
		 }
	  }
	  /* Add any additional workarounds in else if's */
	  /* The default method is in this else, don't let it get fubared */
	  else
		 bp += bp[1];         /* Skip past */

	  fprintf(db,"                 Strings start at %X:%X\n", FP_SEG(bp), FP_OFF(bp));
	  t = 1;
	  sysinfo.dmi[i].s[0] = nonestring[0];
	  while (*((int *)bp) != 0 && t < MAX_DMI_DATA_STRING) {
		 if (*bp == '\0') ++bp;
		 sysinfo.dmi[i].s[t] = bp;
		 bp += strlen(sysinfo.dmi[i].s[t]);
		 fprintf(db, "		 found string: %.64s\n", sysinfo.dmi[i].s[t]);
		 ++t;
	  }

	  bp += 2;

	  /* A few bios's have several 00 's after the string terminator  00 00.
		 For now attempt to condense this */
	  if (*bp == 0) {
		 for (nc = 0; nc < 20 && (*bp == 0); ++nc)  ++bp;
		 #if DEBUGLOW
			fprintf(db, "   %X:%Xh - %i Excess Null's found.\n", FP_SEG(bp), FP_OFF(bp), nc);
		 #endif
		 if (*bp == 0) {
			sysinfo.dmi_items = i + 1;
			#if DEBUGLOW
			   fprintf(db, "   Too many null's, truncating list to %i items.\n", sysinfo.dmi_items);
			#endif
			break;  /* break huge for */
		 }
	  } /* end of if (*bp == 0) */

   } /* END OF HUGE FOR LOOP */


   return 1;
}
