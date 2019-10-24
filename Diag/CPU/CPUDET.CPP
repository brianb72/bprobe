#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <time.h>
#include <limits.h>

#include "declare/global.h"
#include "diag/cpu/cpudet.h"

/* CPUDET.CPP

   Class Functions
   * Gather CPUID Information
   * Benchmark CPU via RDTSC instruction
   * Return text strings reporting information
*/


/* External asm functions.
   CPUID_1() returns standard level 0. Vendor ID, max levels.
   CPUID_2() returns standard level 1. Family, Type, Stepping.
   CPUID_2() returns standard level 1. Cache information
   CPUID_IDENTIFY() returns 1 for 386, 2 for 486, 3 for 486 w/cpuid or 586+
*/
extern "C" { int cpuid_1(unsigned long *tregs); }
extern "C" { int cpuid_2(unsigned long *tregs); }
extern "C" { int cpuid_3(unsigned long *tregs); }
extern "C" { int cpuid_identify(void); }




cCPUDet::cCPUDet() {
   basetype = 0;
   tested = FALSE;
   family = 0;
   model = 0;
   vendorid[0] = 0;
   modelname[0] = 0;
   stepname[0] = 0;
   familyname[0] = 0;
   mhz = 0;
}


/* CPUID() - Attempts to execute CPUID instruction, gathers info.
   Returns TRUE if successful, FALSE if not.
*/
int cCPUDet::CPUID() {
	unsigned long regs[4] = {0,0,0,0};    /* eax ebx ecx dcx */
	unsigned long treg = 0;
	char tmp[20];
	int i = 0, j = 0, r = 0;
   
	if (basetype == 0)
	   basetype = cpuid_identify();   // First fetch the basetype

	LOGLO("CPUID Test Begining, basetype %i.", basetype);
	LOGLO("---------------------------------------------");
	
	/* Processor is incapable of doing CPUID. No other tests possible. */
	if (basetype != 3) {
       max_level = 0;
	   mhz = 0;
	   tested = TRUE;
	   LOGLO("No CPUID instruction, no tests possible.");
	   return FALSE; 
	}

	if (!cpuid_1(regs)) { /* An error indicates we are a 386 or cpuid failed */
       LOG("CPU Test Error: CPUID Level 0 failed.");
	   return FALSE;
	}


	LOGHI("trycpuid() - VendorID:  %lu %lu %lu", regs[1], regs[2], regs[3]);

	for (j = 1; j < 4; ++j) {
	   /* Cheap way of flipping the bytes around */
	   if (j == 1) r = 0; if (j == 2) r = 2; if (j == 3) r = 1;

	   for (i = 0; i < 4; ++i) {
		  treg = regs[j];            // Copy to our temp variable
		  treg = treg >> (8 * i);    // Shift the correct byte into place
		  treg = treg & 0xFF;        // Mask out the other bits
		  tmp[(i + (r * 4))] = (char) treg;  // Copy it into our position
	   }
	}
	tmp[12] = '\0';

	/* Copy the information we discovered */
	strcpy(vendorid, tmp);
	max_level = regs[0];

	LOGLO("trycpuid() - VendorID: %s   Max_Level: %i", vendorid, max_level);
			   

	if (regs[0] == 0) {
		LOG("CPU Test Notice: Max Level is 0, strange CPU. Please report.");
	   return TRUE;   // This may never happen, if it does
	}                 // examine the processer by hand.

	if (!cpuid_2(regs)) { /* This shouldn't fail */
		LOGLO("CPU Test Error: CPUID Level 1 failed, test failed.");
		return FALSE;  // This should be considered an error.
	}

	type     = (regs[0] & 0x3000) >> 12;
	family   = (regs[0] & 0x0F00) >> 8;
	model    = (regs[0] & 0xF0) >> 4;
	stepping = regs[0] & 0xF;

	if (max_level < 2)
	  return TRUE;

	if (!cpuid_3(regs)) { /* This shouldn't fail */
		LOGLO("CPU Test Error: CPUID Level 2 failed, test failed.");
	   return FALSE;  // This should be considered an error.
	}

	int regc = 0;
	int num_call = 0;

	num_call = regs[0] & 0xFF;			   // Mask out the # of times to call
	if (num_call > 5)        // Avoid possible infinate loop
	   num_call = 5;

 for (i = 0; i < num_call; ++i) {             // Cycle through calls
   regs[0] = regs[0] & 0xFFFFFF00;            // Mask out the right num_call
   for (regc = 0; regc < 4; ++regc)          // Cycle through registers
	  for (j = 0; j < 4; ++j) {
		treg = regs[regc];
		treg = treg >> (j * 8);
		treg = treg & 0xFF;
		parse_desc(treg);
	  }
   cpuid_3(regs);                            // Call it again so
 }


  return TRUE;
}


/* -------------------------------------------------------------------------*/
/* Take each byte from the cpuid level 2 instruction and set right bytes    */
/* -------------------------------------------------------------------------*/
void cCPUDet::parse_desc(int code) {
   switch(code) {   /* Values are listed in K, 4000 = 4 megs */
	  case 0x0:    break;   // Lots of zero's for some reason, just ignore
	  case 0x01:   tlbcode += 4; break;
	  case 0x02:   tlbcode += 4000; break;
	  case 0x03:   tlbdata += 4; break;
	  case 0x04:   tlbdata += 4000; break;
	  case 0x06:   l1code += 8; break;
	  case 0x08:   l1code += 16; break;
	  case 0x0A:   l1data += 8; break;
	  case 0x0C:   l1data += 16; break;
	  case 0x40:   l2 += 0; break;
	  case 0x41:   l2 += 128; break;
	  case 0x42:   l2 += 256; break;
	  case 0x43:   l2 += 512; break;
	  case 0x44:   l2 += 1024; break;
	  case 0x45:   l2 += 2048; break;
	  case 0x70:   cyrix_tlb = 4; break;
	  case 0x80:   cyrix_l1 = 16; break;
	  /* There are a few Cyrix specific values that are unknown,
	  values 74h, 77h, 82h, and 84h according to sandpile.org. */
	  default: LOGLO("Parse Desc - Unknown Value: %X\n", code);
			   break;
   }

   LOGHI("Parse Desc - Value %.4X", code);
}



/* ------------------------------------------------------------------------ */
/* BENCHMARK CPU: Will calculate the speed of the processor in MHZ by       */
/* performing a set number of operations and timing how many clock cycles   */
/* and how many seconds/milliseconds it took.                               */
/* This should be accurate to +/- 3% of what the processor is labeled       */
/* THIS WILL NOT WORK UNDER WINDOWS. This also will not work on 486's or    */
/* less as it uses the pentium RDTSC instruction.                           */
/* ------------------------------------------------------------------------ */

/* Notice: There was a batch of motherboards that crashed randomly in windows
   and occasionally caused the delay() used here to return in half the time.
   There were other systems that did not crash but had the delay() problems.
   This would cause the CPU speed to show half speed. This only occured in
   this one batch of motherboards, but there may be future problems with
   Borlands delay() function. Look into hard coding a delay function if
   any future problems occur.
*/

void cCPUDet::Benchmark() {
unsigned long starth = 0, endh = 0, startl = 0, endl = 0;
unsigned long dif = 0, tlong = 0;
float freq = 0;
int bigroll = 0;


  if (basetype != 3) return;

   asm {
	  db 0xf; db 0x31;   // rdtsc
	  db 0x66; mov word ptr startl, ax;
	  db 0x66; mov word ptr starth, dx;

   }

   if (g_delay_method == 0)
	  delay(2000);  // sleep for 2 seconds
   else tdelay(2035);  // sleep for 2035 seconds


   asm {
	  db 0xf; db 0x31   // rdtsc
	  db 0x66; mov word ptr endl, ax
	  db 0x66; mov word ptr endh, dx;
   }

   /* Here's the plan ...
	  ULONG / 1e6 / 2 = 2147 mhz max.
	  If bit 0 of starth/endh is same, no roll over. Do 32 bit subtract.
	  If bit 0 is different, do 32 bit sub and add 2147.
   */

   if (startl > endl) {
	  tlong = ULONG_MAX - startl;
	  dif = tlong + endl;
	  if ( (endh - starth) == 2) bigroll = 1;
   } else {
	  dif = endl - startl;
	  if ( (endh - starth) >= 1) bigroll = 1;
   }


   LOGLO("start  %lu : %lu   end  %lu : %lu", starth, startl, endh, endl);

/* 64 bit subtract, do i need it
   asm {
	 db 0x66; mov dx, word ptr endh;
	 db 0x66; mov ax, word ptr endl;
	 db 0x66; mov cx, word ptr starth;
	 db 0x66; mov bx, word ptr startl;

	 db 0x66; sub ax, bx;
	 db 0x66; sbb dx, cx;
   }
*/




   if (g_delay_method == 0) tlong = dif / 2000;
   else tlong = dif / 2035;

   freq  = tlong / 1000.0;
   if (bigroll) freq += 2147.48;


   LOGHI("CPU Benchmark - Aproximate Frequency: %.2f mhz", freq);

   /* NOTE:
		 (Actual Mhz) * 1e6 * 2 cannot exceed ULONG. This places a
	  limit on the max speed that can be detected of about 2.0-2.1ghz.
	  A 64 bit version will have to be created to do higher processors.
   */
   mhz = freq;

}


int cCPUDet::Main() {
   CPUID();
//   if (basetype == 3)
//	   Benchmark();
   find_cpu_text();
   return 1;
}



#define UNKNOWN 0
#define INTEL   1
#define UMC     2
#define AMD     3
#define CYRIX   4
#define NEXGEN  5
#define IDT     6
#define RISE    7




/* Matches the VendorID string and return a numeric cputype. Used by many
   of the other functions */
int cCPUDet::find_cpu_vendor() {
  int cputype = 0;

   if (strcmp(vendorid, "GenuineIntel") == 0) cputype = INTEL;
   else if (strcmp(vendorid, "UMC UMC UMC ") == 0) cputype = UMC;
   else if (strcmp(vendorid, "AuthenticAMD") == 0) cputype = AMD;
   else if (strcmp(vendorid, "CyrixInstead") == 0) cputype = CYRIX;
   else if (strcmp(vendorid, "NexGenDriven") == 0) cputype = NEXGEN;
   else if (strcmp(vendorid, "CentaurHauls") == 0) cputype = IDT;
   else if (strcmp(vendorid, "RiseRiseRise") == 0) cputype = RISE;
   /* Supposidly found in some demo chips from 1994 */
   else if (strcmp(vendorid, "AMD ISBETTER") == 0) cputype = AMD;

   return cputype;
}

// ³   Model: P54C                   ³

/* Find the processor family name based on the cputype */
void cCPUDet::find_cpu_text()  {
   int cputype = find_cpu_vendor();
   struct cpu_text_data { char model[40]; char family[40]; char step[40]; };
   struct cpu_text_data cpu_text;   // wth did I do this? oh well 
   struct cpu_text_data *ct = &cpu_text;

   ct->model[0] = 0; ct->family[0] = 0; ct->step[0] = 0;

   switch (cputype) {
	  case INTEL:
		 switch (family) {
			case 4: strcpy(ct->family, "486 (with CPUID)"); break;
			case 5: strcpy(ct->family, "Pentium");
			   switch(model) {
					  case 0: strcpy(ct->model, "P5 A-Step"); break;
					  case 1: strcpy(ct->model, "P5");  break;
					  case 2: strcpy(ct->model, "P54C"); break;
					  case 3: strcpy(ct->model, "P24T Overdrive"); break;
					  case 4: strcpy(ct->model, "P55C"); break;
					  case 5: strcpy(ct->model, "DX4 Overdrive?"); break;
					  case 6: strcpy(ct->model, "P5 Overdrive?"); break;
					  case 7: strcpy(ct->model, "P54C"); break;
					  case 8: strcpy(ct->model, "low voltage P55"); break;
					  default: strcpy(ct->model, "Unknown Intel"); break;
					}
			   break;
			case 6: strcpy(ct->family, "PPro, P2, and P3");
			   switch(model) {
				  case 0: strcpy(ct->model, "Pentium Pro A-step"); break;
				  case 1: strcpy(ct->model, "Pentium Pro");
					  switch (stepping) {
						 case 1: strcpy(ct->step, "(B0)"); break;
						 case 2: strcpy(ct->step, "(C0)"); break;
						 case 6: strcpy(ct->step, "(sA0)"); break;
						 case 7: strcpy(ct->step, "(sA1)"); break;
						 case 9: strcpy(ct->step, "(sB1)"); break;
						 default: strcpy(ct->step, ""); break;
					  }
					  break;
				  case 3: strcpy(ct->model, "P2 Klamath");
					  switch (stepping) {
						 case 2: strcpy(ct->step, "(tdB0)"); break;
						 case 3: strcpy(ct->step, "(C0)"); break;
						 case 4: strcpy(ct->step, "(C1)"); break;
						 default: strcpy(ct->step, ""); break;
					  }
					  break;
				  case 4: strcpy(ct->model, "P55CT? P54 OvrDrv?"); break;
				  case 5: strcpy(ct->model, "P2 Deschutes/Celeron");
					  switch (stepping) {
						 case 0: strcpy(ct->step, "(dA0)"); break;
						 case 1: strcpy(ct->step, "(dA1)"); break;
						 case 2: strcpy(ct->step, "(dB0)"); break;
						 default: strcpy(ct->step, ""); break;
					  }
					  break;
				  case 6: strcpy(ct->model, "P2 Celeron A");
					  switch (stepping) {
						 case 0: strcpy(ct->step, "(mA0)"); break;
						 default: strcpy(ct->step, ""); break;
					  }
					  break;
				  case 7: strcpy(ct->model, "P3"); break;
				  default: strcpy(ct->model, "Unknown Intel P2/P3"); break;
			   } /* end of intel 6 model switch */
			   break;
			default: strcpy(ct->model, "Unknown Intel"); break;
		 } /* end of INTEL switch */
		 break;
	  case AMD:
		 switch (family) {
			case 4: strcpy(ct->family, "486 with CPUID"); break;
			case 5: strcpy(ct->family, "K5 and K6");
			   switch (model) {
				  case 0: strcpy(ct->model, "PR75/PR90/PR100"); break;
				  case 1: strcpy(ct->model, "PR120/PR133"); break;
				  case 2: strcpy(ct->model, "PR150/PR166"); break;
				  case 3: strcpy(ct->model, "PR200"); break;
				  case 6: strcpy(ct->model, "K6 166-233mhz"); break;
				  case 7: strcpy(ct->model, "K6 266-300mhz"); break;
				  case 8: strcpy(ct->model, "K6-2 w/3D Now!"); break;
				  case 9: strcpy(ct->model, "K6-3 w/3D Now!"); break;
				  default: strcpy(ct->model, ""); break;
			   }
			   break;
			case 6: strcpy(ct->family, "K7"); break;
			default: strcpy(ct->family, "Unknown AMD"); break;
		 }
		 break;
	  /* NOTE: According to http://grafi.ii.pw.edu.pl/gbm/x86/cpuid.html
		 you must set a bit to enable the CPU ID instruction for Cyrix
		 processors, or use "DIR registers". Will have to get hands on
		 a few Cyrix's to see how detection works */
	  case CYRIX:
		 switch (family) {
			case 4: strcpy(ct->family, "Cx5x86"); break;
			case 5: strcpy(ct->family, "Cx6x86"); break;
			case 6: strcpy(ct->family, "6x86MX/MII"); break;
			default: strcpy(ct->family, "Unknown Cyrix"); break;
		 }
		 break;
	  case NEXGEN:
		 switch (family) {
			case 5: strcpy(ct->family, "Nx586"); break;
			default: strcpy(ct->family, "Unknown NextGen"); break;
		 }
		 break;
	  case IDT:
		 switch (family) {
			case 5: strcpy(ct->family, "IDT C6 WinChip"); break;
			default: strcpy(ct->family, "Unknown IDT"); break;
		 }
		 break;
	  case RISE:
		 switch (family) {
			default: strcpy(ct->family, "Unknown RISE"); break;
		 }
		 break;
	  default: strcpy(ct->family, "Unknown"); break;
   }

   strcpy(familyname, ct->family);
   strcpy(modelname, ct->model);
   strcpy(stepname, ct->step);
}

