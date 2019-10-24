#include <stdio.h>
#include <conio.h>

#include "declare/global.h"
#include "diag/mem/memdet.h"



cMemDet::cMemDet() {
	tested = FALSE;
    ramsize = 0;
}




/* -------------------------------------------------------------------------*/
/* Phoenix Bios v4.0 - Get memory size for >64M configurations              */
/* Supported by Compaq Contura, Deskpro/I, AMI BIOSes 8/23/94 or later,     */
/* and any system using phoenix v4.0                                        */
/* This function is ideal as it can report up to 4 gigs of memory.          */
/* -------------------------------------------------------------------------*/

long cMemDet::try_E801h(void) {
   int i;
   long res;
   int  ext_1to16   = 0,   /* extnd mem between 1m-16m in K  3C00 = 15mb */
		ext_above16 = 0,   /* extnd mem above 16m in 64k blocks */
		con_1to16   = 0,   /* configured memory 1m to 16m in K */
		con_above16 = 0;   /* configured memory above 16M in 64k blocks */

   asm {
	  mov ax, 0xe801       // Get memory size for > 64
	  int 0x15

	  pushf    // Recover the carry flag
	  pop i

	  mov word ptr ext_1to16,   ax   // Copy the results
	  mov word ptr ext_above16, bx
	  mov word ptr con_1to16,   cx
	  mov word ptr con_above16, dx

   }

   if (i & 1) {    /* Carry flag set, operation failed */
	  return FALSE;
   }


   /* Some BIOSes return AX=BX=0000h. Make sure we didn't. */
   if ((ext_1to16 != 0) && (ext_above16 != 0)) {
	  /* Add 1k blocks to 64k blocks multiplied by 64 */
	  return(((long) con_1to16 + (((long) con_above16) * 64)));
   }


   /* We have a strange bios, or Win9x is running and fubaring the interupt.
	  Use configured memory instead */

   /* Add 1k blocks to 64k blocks multiplied by 64 */

   res = (((long) con_1to16 + (((long) con_above16) * 64)));
   return res;

}

/* -------------------------------------------------------------------------*/
/* AMI PCI Bios - Get Extended Memory Size                                  */
/* Fallback function in the event E801h does not work.                      */
/* Will report up to 16 gigs of memory.                                     */
/* -------------------------------------------------------------------------*/
long cMemDet::try_DA88h(void) {
	 /* NOTE: Results in KB are supposed to be CL:BX. No machines to test
		this routine on. Routine will dump to debug log notifying that
		it has found a machine to test on.                              */
  byte code = 0;
  unsigned long tcl = 0, tbx = 0, res;
  int i;
  asm {
	 mov ax, 0xDA88
	 int 0x15

	 pushf
	 pop i

	 mov code, ah
	 mov byte ptr tcl,  cl
	 mov word ptr tbx, bx
  }

  if (i & 1) {      /* Carry flag set, operation failed */
	 return FALSE;
  }

  /* I don't trust this function, double check the code doesn't report an
	 error in case the carry flag does not set correctly.                */
  if ((code == 0x80) || (code == 0x86)) {
	 return FALSE;
  }


  /* Now that it has passed the tests, perform the CL:BX */
  res = ( ((tcl << 16) | tbx) );
  return res;
}


/* -------------------------------------------------------------------------*/
/* Dos 4.0+ internal - SYSVARS - Get list of lists                          */
/* Will report <64M of memory. This will work on any machine running Dos 4.0+ */
/* It might return incorrect information while Win95 is running.            */
/* Make sure to debug this function fully on many systems                   */
/* -------------------------------------------------------------------------*/

long cMemDet::try_52h_int21(void) {
	long totalmem = 0;
	int i = 0, tax = 0;

	asm {
	 push es
	 mov ah, 0x52
	 int 0x21
	 pushf
	 pop i
	 add bx, 0x45
	 mov tax, ax         // Preserve AX to examine later for error code
	 mov ax, es:[bx]
	 mov word ptr totalmem, ax    // This MIGHT crash if call failed and we read
	 pop es
	 }

	 /* No mention of carry flag reporting an error, but report it. */
	 if (i & 1) {
	 }

	 /* 80h and 86h is standard general error, should only happen if some
		shmuck is still running Dos 3.0                                  */
	 if ((tax == 0x80) || (tax == 0x86)) {
	 }


	 /* Everything passed, return mem in K */
	 totalmem = totalmem & 0x0000FFFF;
	 return( totalmem );
}


/* -------------------------------------------------------------------------*/
/* Function E820h - Supported by most newer BIOSes.
/* -------------------------------------------------------------------------*/

long cMemDet::try_E820(void) {
   unsigned long txt_SMAP = 0x534D4150;   // SMAP string
   unsigned long res;
   unsigned long tbuf[30];
   unsigned long *p;
   unsigned int carry;

   struct e820_return {
	  DWORD basehigh;
	  DWORD baselow;
	  DWORD lengthhigh;
	  DWORD lengthlow;
   };

   p = tbuf;

   asm {
	  push es
	  push di

	  db 0x66; xor ax, ax;   // xor EAX, EAX
	  db 0x66; xor bx, bx;   // xor EBX, EBX
	  db 0x66; xor cx, cx;   // xor ECX, ECX

	  mov ax, 0xE820             // Function E820
	  db 0x66; mov dx, word ptr txt_SMAP;   // mov edx, txt_smap
	  les di, dword ptr p        // setup ES:DI to our buffer
	  mov cx, 20				 // Buffer size

	  int 0x15
	  pushf
	  pop carry
	  db 0x66; mov word ptr txt_SMAP, dx

	  pop di
	  pop es
   }

   if (carry & 1) {
	  LOGLO("Carry was set for E820...");
   return FALSE;
   }
   /* Log an error, I dont know why this would happen */
   if (txt_SMAP != 0x534D4150) {
	  LOGLO("try_E820 Error! EAX is %lu after call!", txt_SMAP);
   }

   /* Length is a QWORD and is bytes of RAM. A DWORD will let
	  us report up to 4 gig of RAM. This should be sufficient. */
   LOG("tbuf[0] = %lu", tbuf[0]);
   LOG("tbuf[1] = %lu", tbuf[1]);
   LOG("tbuf[2] = %lu", tbuf[2]);
   LOG("tbuf[3] = %lu", tbuf[3]);
   LOG("tbuf[4] = %lu", tbuf[4]);
   LOG("tbuf[5] = %lu", tbuf[5]);
   LOG("tbuf[6] = %lu", tbuf[6]);
   LOG("tbuf[7] = %lu", tbuf[7]);

   res = tbuf[0];
   res = res / 1024;   // turn it into K
   return res;
}



/* -------------------------------------------------------------------------*/
/* Calls all possible functions in an attempt to find the best to return    */
/* the amount of physical ram in the system. Will return in Kilobytes.      */
/* -------------------------------------------------------------------------*/

int cMemDet::Main() {

   unsigned long result[3];

   tested = TRUE;

   LOGHI("Memory Detection Begining");
   LOGHI("-------------------------");

   /* This is most reliable (supposidly) */
//   LOGLO("Calling try_e820...");
//   result[0] = try_E820();
//   LOGLO("Done, result was %lu", result[0]);

//   if (result[0] != FALSE) {
//	  ramsize = result[0];
//	  LOGLO("Ram Size Reported by E820 is %luk.", result[0]);
//	  return 1;  // this wins if it succeeds
//   }

   /* AMI PCI Bios  <16Gb */
   if ((result[0] = try_DA88h()) != FALSE) {
	  LOGLO("RAM Detection - DA88h %luk of memory reported.", result[0]);
   }

   /* Phoenix Bios 4.0 and AmiBios 8/23/94+  <4Gb */
   if ((result[1] = try_E801h()) != FALSE) {
	  LOGLO("RAM Detection - E801h %luk of memory reported.", result[1]);
   }

   /* Dos 4.0+, <64Mb, may report incorrectly under Win95 <GUI> */
   if ((result[2] = try_52h_int21()) != FALSE) {
	  LOGLO("RAM Detection - 52h int 21 %luk of memory reported.", result[2]);
   }

   ramsize = 0;

   /* Use the biggest value */
   if ((result[0] > result[1]) && (result[0] > result[2])) ramsize = result[0];
   else if ((result[1] > result[2]) && (result[1] > result[0])) ramsize = result[1];
   else if ((result[2] > result[0]) && (result[2] > result[1])) ramsize = result[2];
   else if (result[0] != 0) ramsize = result[0];  // all the same, nonzero
   else if (result[1] != 0) ramsize = result[1];  // all the same, nonzero
   else if (result[2] != 0) ramsize = result[2];  // all the same, nonzero

   if (ramsize == 0) {
	 LOG("RAM Detection Error - No detection methods worked.");
	 return 0;
   }

   LOG("RAM Size Reported: %luk\n", ramsize);
   return 1;
}



long cMemDet::GetRamSize() {
	return ramsize;
}


