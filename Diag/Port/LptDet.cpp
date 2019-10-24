/* LPT Port Detection */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include <Global.h>
#include "declare/global.h"
#include "diag/port/lptdet.h"


/* Will attempt to guess the mode that a printer port is in. Remember, the
   port may be capable of doing higher modes than what it is set at. We will
   only detect the current mode it is set to.  [Note: Or maybe not?]
   Play with this and verify the results are correct, may have questionable
   accuracy.

   http://wave.campus.luth.se/~pb/comp/standards/ecpmode.htm
   http://www.lvr.com/jansfaq.htm
*/



/* Bit 0 of the status port is the EPP timeout flag, if this is set
   problems can occur. After each operation clear it. Unfortunantly this
   process varies. To be safe, attempt to clear by reading from the status
   register, set the bit to 1, and then set it to 0. Even though the
   status port is read only, attempting to change the bit supposidly
   clears it. See what happens on real printer ports, remove all the debug
   stuff after tested. */
void cLPTDet::clear_epp_timeout(int address) {
   unsigned char b, t;

   b = inp(address + R_DSR); /* This satisfies the read to clear */
//   LOGLO("reset_epp_timeout() - %Xh %Xh", address, b);
   b |= 0x1;  outp(address + R_DSR, b);  /* Writing 1 */
//   t = inp(address + R_DSR);     // remove this after debugging is done
//   LOGLO(" %Xh", t);
   b &= 0xFE; outp(address + R_DSR, b);  /* Writing 0 */
//   t = inp(address + R_DSR);     // remove this after debugging is done
//   LOGLO(" %Xh", t);
}


int cLPTDet::guess_mode(int i, unsigned int address) {
	unsigned char b, b2, odata, odsr, odcr, oeppa, oeppd, oecr;
	int toggle = FALSE;   // for epp toggle

	/* In order to avoid poking at ports we shouldn't only do this if the
	   port matches a known address. Maybe should do this back at identify */
	if (address != 0x3BC && address != 0x378 && address != 0x278) {
		LOGLO("guess_mode() - Port %Xh doesn't match lpt address.", address);
		return FALSE;
	}
	/* First check for ECP. In the ECR Bit 0 = 1 and 1 = 0 */
	oecr = inp(address + R_ECR);
	if ( (oecr & 3) == 1) {           // Make sure bit 0 = 1  1 = 0
	   /* Verify that modifying bits 0 and 1 in the control register doesn't
		  change the ECR bits [Will it ever?] */
	   b2 = oecr ^ 3;                 // exclusive or, toggle both
	   outp(address + R_ECR, b2);     // Write the toggled bits out
	   b = inp(address + R_ECR);      // Read again

	   if ( (b & 3) == 1) {           // Bit 0 and 1 should be unchanged
		  /* Those two are Read Only, if we write 34h and read back 35h it
			 _probably_ is an ECP */
			 outp(address + R_ECR, 0x34);
			 b2 = inp(address + R_ECR);
			 outp(address + R_ECR, oecr);   // Just to be safe restore it

			 if (b2 == 0x35) {  // We are an ECP Port. Get our emulation
				port[i].mode = LPT_ECP;
				b = oecr & 0xE0;  // Bytes 7 5 6
				port[i].emode = b;
				LOGLO("guess_mode() - Port %Xh is ECP emulating %Xh, ECR: %Xh", address, b, oecr);
				return TRUE;
			 }  // Not ECP, failed on ECR 0 and 1 not being read only
			 else LOGLO("guess_mode() - Port %Xh not ECP, ECR not Read Only", address);
	   } // Not ECP, failed on DCR changing ECR.
	   else LOGLO("guess_mode() - Port %Xh not ECP, DCR changed ECR.", address);
	} // Not ECP, failed on ECR bits 0 and 1
	else LOGLO("guess_mode() - Port %Xh not ECP, ECR bits 0 and 1 wrong.", address);

	/* Not an ECP, check if we are an EPP */
	/* First set the port in forward mode by writing xx0x0100 to Control Reg */
	odcr = inp(address + R_DCR);
	b = (odcr & 0xD0) | 0x4;      // Set it to xx0x0100
	outp(address + R_DCR, b);

	clear_epp_timeout(address);  // Should we reset just in case? Maybe.

	/* Write to the EPP address and data registers and see if they stay */
	/* I don't think there's any need to restore these */
	oeppa = inp(address + R_EPPA);  // Read epp address register
	outp(address + R_EPPA, (oeppa ^ 0xFF));  // Output the inverse
	clear_epp_timeout(address);     // Do we have to reset here?
	b = inp(address + R_EPPA);      // Read the register again
	clear_epp_timeout(address);
	outp(address + R_EPPA, 0x42);   // Write a magic byte
	b2 = inp(address + R_EPPA);     // Read it
	if (b == (oeppa ^ 0xFF) && b2 == 0x42) {      // Our byte stuck
		toggle = TRUE;              // Just to be safe check Addr and Data
		// If one 'sticks', assume its EPP.
		   LOGLO("guess_mode() - %Xh EPP address register passed check.", address);
	}
	else
		LOGLO("guess_mode() - %Xh EPP address register failed. Read %Xh Wrote %Xh Read %Xh b2 %Xh",
			address, oeppa, (oeppa ^ 0xFF), b, b2);
	clear_epp_timeout(address);    // Clear the timeout again

	// Do the same for the data registers
	oeppd = inp(address + R_EPPD);  // Read epp data register
	outp(address + R_EPPD, (oeppd ^ 0xFF));  // Output the inverse
	clear_epp_timeout(address);     // Do we have to reset here?
	b = inp(address + R_EPPD);      // Read the register again
	clear_epp_timeout(address);
	outp(address + R_EPPA, 0x42);   // Write a magic byte
	b2 = inp(address + R_EPPA);     // Read it
	if (b == (oeppd ^ 0xFF) && b2 == 0x42) {      // Our byte stuck
		toggle = TRUE;              // Just to be safe check Addr and Data
		// If one 'sticks', assume its EPP.
		   LOGLO("guess_mode() - %Xh EPP data register passed check.", address);
	}
	else
		LOGLO("guess_mode() - %Xh EPP data register failed. Read %Xh Wrote %Xh Read %Xh b2 %Xh",
			address, oeppd, (oeppd ^ 0xFF), b, b2);
	clear_epp_timeout(address);    // Clear the timeout again

	// One of the registers stuck, we are an EPP port
	if (toggle == TRUE) {
		  LOGLO("guess_mode() - Port %Xh is EPP.", address);
	   port[i].mode = LPT_EPP;
	   return TRUE;
	}


	/* Now check if we are an SPP port. This basically checks to see if
	   either the BIOS incorrectly found an lpt port, or the port is bad.
	   If it is bad, what should we do? Just report so for now
	   Just do the data port as one document suggests */
	/* Our config register still should be setup from the EPP test */
	outp(address + R_DATA, 0x42);
	b  = inp(address + R_DATA);
	outp(address + R_DATA, 0xAA);
	b2 = inp(address + R_DATA);
	if (b == 0x42 && b2 == 0xAA) {
		 LOGLO("guess_mode() - Port %Xh is SPP.", address);
	   port[i].mode = LPT_SPP;
	   return TRUE;
	}


	LOGLO("guess_mode() - Port %Xh failed SPP test! Wrote %Xh Read %Xh Wrote %Xh Read %Xh",
		address, 0x42, b, 0xAA, b2);

	port[i].mode = LPT_NONE;

	return FALSE;
}



/* From http://www.cubic.org/source/archive/hardware/parallel/parallel.html
   0000:0408   Base Address of Lpt1
   0000:040A   Base Address of Lpt2
   0000:040C   Base Address of Lpt3
*/


char *cLPTDet::LPT_Mode(int i) {
	switch (port[i].mode) {
	case 1: return "SPP";
	case 2: return "EPP";
	case 3: return "ECP";
	default: return "Unknown";
	}
}


/* For now just get the lpt data out of bios */
/* I'm going to assume the BIOS will place 0x0 over nonexistant ports and
   just assign each location to the sysinfo location. If this is not the
   case some sort of check must be put into place */
void cLPTDet::Init() {
   int i;
   unsigned int far *ptraddr;


   ptraddr=(unsigned int far *)0x00000408;

   LOGLO("--- LPT DETECTION ---");

   for (i = 0; i < 3; ++i) {
	  port[i].address = *ptraddr;
	  LOGLO("   Port %i address is %Xh", i,
				port[i].address);

	  if (*ptraddr != 0)
		 guess_mode(i, *ptraddr);
	  *ptraddr++;
   }

}

