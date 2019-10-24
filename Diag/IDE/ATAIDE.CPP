/* ATAIDE Interface Class 
   Provides basic low level access to ATA and ATAPI IDE devices.
   
   This class handles one ide port, and the master and slave drives that
   may be on them.
*/

#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <string.h>


#include "declare/global.h"
#include "diag/ide/ataide.h"



/* ***********************************************************************
   Initalization Functions
   *********************************************************************** */
cIDE::cIDE() {
   base  = 0;
   bMaster = 0;
   bSlave = 0;
   memset(&sMaster, 0, sizeof(sMaster));
   memset(&sSlave, 0, sizeof(sSlave));
}

cIDE::~cIDE() {

}

void cIDE::SetBase(WORD addr) { base = addr; }



/* *********************************************************************** */
/* Byteswap the string if needed, clear out crap, pad with nulls */
/* *********************************************************************** */
void cIDE::IdeFixString(BYTE *s, const int bytecount, const int byteswap)
{
   BYTE *p = s, *end = &s[bytecount & ~1];  /* Bytecount must be even */
   int i; char c;
   if (byteswap) {
	 for(i=0; i < (bytecount / 2) - 1; ++i) {
	  c = s[i*2];
	  s[i*2] = s[i*2+1];
	  s[i*2+1] = c;
	 }
   }
   while (s != end && *s == ' ') ++s;
   /* compress internal blanks and strip trailing blanks */
   while (s != end && *s) {
	   if (*s++ != ' ' || (s != end && *s && *s != ' ')) *p++ = *(s-1); }
   /* wipe out trailing garbage */
   while (p != end)	*p++ = '\0';
}

/* Handles all strings and determines what needs to be fixed */
void cIDE::FixStrings(struct ideinfo_data *id) {
   int bswap = 1;

   /* EATA SCSI controllers do hardware ATA emulation, warn about then */
   if ((id->model[0] == 'P' && id->model[1] == 'M') ||
	   (id->model[0] == 'S' && id->model[1] == 'K') ) {
	   LOG("Warning! EATA SCSI controller detected...");
	   LOG("   These controllers do hardware ATA emulation.");
	   LOG("   Please contact program author if you see this, as");
	   LOG("   these controllers have not been tested with this scan.");
   }

   /* WIN_PIDENTIFY usually returns little endian. Detect for NEC, 
      Mitsumi, and Pioneer respectivly   */
   if ( (id->model[0] == 'N' && id->model[1] == 'E') ||
        (id->model[0] == 'F' && id->model[1] == 'X') ||
		(id->model[0] == 'P' && id->model[1] == 'I')) bswap = 0;

   IdeFixString(id->firmware, 8, bswap);
   IdeFixString(id->model, 40, bswap);
   IdeFixString(id->serial, 20, bswap);
}




/* *********************************************************************** */
/* Scan the master and the slave and gather all information about them     */
/* *********************************************************************** */
void cIDE::InitDrives() {
//   asm cli;                    // No interrupts while we do it

   bMaster = 0; bSlave = 0;

   asm cli;

   LOGLO("-- Detecting IDE Drives Base %Xh---", base);
   /* Do the master. If it fails to select it probably doesn't exist. If
	  either of the identifies succeed mark it as existant. */
   if (Select(SMASTER)) {
	  oThrob->Step();
	   if (!Command(WIN_IDENTIFY)) {
		  LOGLO("[IDE %Xh Drv 0] Drive failed IDENTIFY.", base);
		  if (!Command(WIN_PIDENTIFY))
			LOGLO("[IDE %Xh Drv 0] Drive failed PIDENTIFY, refuses to identify or nonexistant.", base);
		  else bMaster = WIN_PIDENTIFY;
	   } else bMaster = WIN_IDENTIFY;
	   if (bMaster) {
		   PIORead((WORD *)(&sMaster), 256);
		  oThrob->Step();
		   FixStrings(&sMaster);
	   }
   } /* select */

   /* Same routine for the slave */
   if (Select(SSLAVE)) {
	  oThrob->Step();
	   if (!Command(WIN_IDENTIFY)) {
		  LOGLO("[IDE %Xh Drv 1] Drive failed IDENTIFY.", base);
		  if (!Command(WIN_PIDENTIFY))
			LOGLO("[IDE %Xh Drv 1] Drive failed PIDENTIFY, refuses to identify or nonexistant.", base);
		  else bSlave = WIN_PIDENTIFY;
	   } else bSlave = WIN_IDENTIFY;
	   if (bSlave) {
		   PIORead((WORD *)(&sSlave), 256);
		   oThrob->Step();
		   FixStrings(&sSlave);
	   }
   } /* select */
   LOG(" ");
   LOG(" ");

   asm sti;
}





/* *********************************************************************** */
/* WaitBusy - Wait for the busy flag to clear, return false if timeout     */
/*            Returns true if BSY is clear in time.                        */
/*            Returns false if BSY is set after timeout.                   */
/* *********************************************************************** */
bool cIDE::WaitBusy() {
   int timeout = 0;

   asm cli;

   FindStatusReg();
   while ((timeout++) < 700) {
	  if (!(inp(rStatus) & STATUS_BUSY)) return true;
	   delay(20);
   }
   LOGHI("[IDE %Xh] Timeout - Busy did not clear in time.", base);
   return false;
}

/* *********************************************************************** */
/* WaitDRQ - Waits for the DRQ flag to set so we may begin a data transfer */
/*           Returns true if DRQ is set in time                            */
/*           Returns false if DRQ is clear after timeout                   */
/* *********************************************************************** */
bool cIDE::WaitDRQ() {
   int timeout = 0;

   asm cli;

   while ((timeout++) < 700) {
	  if ((inp(rStatus) & STATUS_DRQ)) return true;
	  delay(20);
   }
   LOG("[IDE %Xh] Timeout - DRQ did not set in time.", base);
   return false;

}

/* *********************************************************************** */
/* CheckStat - Checks the status flags to verify DRQ is clear and DRDY is  */
/*    set. If either is not so, the device is in limbo mode. Either data   */
/*	  we are not expecting is ready to be read, or the device lost its     */
/*	  mind and cleared DRDY. Reseting it is probably best.                 */
/*    The caller should wait for BSY to clear before calling us!           */
/*    Returns true if everything is ok for normal operations               */
/*    Returns false if either flag is wrong                                */
/* *********************************************************************** */
bool cIDE::CheckStat() {
   int res = 0, ret = true;

   asm cli;

   res = inp(rStatus);
   if ( (res & STATUS_DRQ) ) {
	   LOGLO("[IDE %Xh] CheckStat Error: DRQ is set when it should not be!", base);  ret = false; }
   if ( !(res & STATUS_DRDY) ) {
	   LOGLO("[IDE %Xh] CheckStat Error: DRDY is clear when it should not be!", base); ret = false; }
   return ret;
}



/* *********************************************************************** */
/* PIORead - Reads the number of words into the buffer off the data reg.   */
/*           The caller handles everything else, this just pulls it off    */
/* *********************************************************************** */
void cIDE::PIORead(WORD *szBuffer, int iWords) {
   int i;

   asm cli;

   if (!WaitBusy()) return;
   if (!WaitDRQ())  return;
   for (i = 0; i < iWords; ++i)
	   szBuffer[i] = inpw(IDE_DATA_REG);
}


/* *********************************************************************** */
/* Select - Selects a drive, returns true or false on success              */
/* *********************************************************************** */
bool cIDE::Select(BYTE drive) {

	asm cli;

	outp(IDE_SELECT_REG, drive);
	if (inp(IDE_SELECT_REG) != drive) {  /* We are not selected */
	  outp(IDE_SELECT_REG, 0xa0);       /* exit with drive0 selected */
	  delay(50);					    /* allow BUSY_STAT to assert & clear */
	  LOGHI("[IDE %Xh %i] Drive Failed to Select - Nonexistant?", base, drive);
	  return false;
   }
   return true;
}


/* *********************************************************************** */
/* FindStatusReg - Finds the status register to use, not sure when this
   should be called. According to two sources, on all modern controllers
   Status == AltStatus always.                                             */
/* *********************************************************************** */
void cIDE::FindStatusReg() {
   asm cli;

   if ((inp(IDE_ALTSTATUS_REG) ^ inp(IDE_STATUS_REG)) & ~STATUS_INDEX) {
	  rStatus = IDE_STATUS_REG;
   } else {
	  rStatus = IDE_ALTSTATUS_REG;
   }
}


/* *********************************************************************** */
/* Command - Issues a command                                              */
/*    Only check Busy, Error, and DRDY. Don't check for DRQ because the    */
/*	  command may be to initate a transfer, setting DRQ.                   */
/*    Since this is a class for diagnostic use, check DRDY to see if the   */
/*    device loses its mind after it receives a command.                   */
/* *********************************************************************** */
bool cIDE::Command(BYTE cmd) {
   int res, ret = true;

   asm cli;

   if (!WaitBusy()) return false;
   CheckStat(); /* Its ok to check DRQ here because we haven't sent cmd yet */

   if (IDE_CONTROL_REG)
	   outp(IDE_CONTROL_REG, 0x08);
   outp(IDE_COMMAND_REG, cmd);
   if (!WaitBusy()) {
	  LOGLO("[IDE %Xh] Command %.2Xh timed out waiting for Busy.", base, cmd); ret = false; }

   res = inp(rStatus);
   if ( !(res & STATUS_DRDY) ) { ret = false;
	  LOG("[IDE %Xh] Command %.2Xh Failed, DRDY cleared after command. Device may be hung or reseting.", base, cmd); }
   if ( (res & STATUS_ERROR) ) { ret = false;
	  LOGLO("[IDE %Xh] Command %.2Xh Failed, Error set.", base, cmd); }

   return ret;
}

/* Execute a command quickly. Don't check any flags. Assume the caller
   already has. */
bool cIDE::CommandFast(BYTE cmd) {
   int res, ret = true;

	asm cli;

//   if (IDE_CONTROL_REG)
//	   outp(IDE_CONTROL_REG, 0x08);
   outp(IDE_COMMAND_REG, cmd);
   if (!WaitBusy()) {
	  LOGLO("[IDE %Xh] Command %.2Xh timed out waiting for Busy.", base, cmd); ret = false; }

   res = inp(rStatus);
   if ( !(res & STATUS_DRDY) ) { ret = false;
	  LOG("[IDE %Xh] Command %.2Xh Failed, DRDY cleared after command. Device may be hung or reseting.", base, cmd); }
   if ( (res & STATUS_ERROR) ) { ret = false;
	  LOGLO("[IDE %Xh] Command %.2Xh Failed, Error set.", base, cmd); }

   return ret;
}



/* *********************************************************************** */
/* Seek - LBA version                                                      */
/* *********************************************************************** */
bool cIDE::Seek(BYTE dev, DWORD sector) {
	char c;


   if (!WaitBusy()) return false;
   if (!CheckStat()) return false;

   asm cli;

   outp(IDE_NSECTOR_REG, 1);
   outp(IDE_SECTOR_REG,  (sector & 0xFF));
   outp(IDE_LCYL_REG, ((sector >> 8) & 0xFF));
   outp(IDE_HCYL_REG, ((sector >> 16) & 0xFF));
   outp(IDE_SELECT_REG, 0xE0 | ((0x1 & dev) << 4) | ((sector >> 24) & 0xF));

   return Command(CMD_SEEK);
}


/* *********************************************************************** */
/* Seek - CHS version                                                      */
/* *********************************************************************** */

bool cIDE::Seek(BYTE dev, WORD cylinder, BYTE head, BYTE sector) {
   if (!WaitBusy()) return false;
   if (!CheckStat()) return false;

   asm cli;

   outp(IDE_SECTOR_REG,  sector);
   outp(IDE_LCYL_REG,    cylinder & 0xFF);
   outp(IDE_HCYL_REG,    (cylinder >> 8) & 0xFF);
   outp(IDE_SELECT_REG,  0xA0 | ((0x1 & dev) << 4) | (head & 0xF) );
   return CommandFast(CMD_SEEK);
}

/* *********************************************************************** */
/* Verify - LBA Version. Causes device to read a sector but not set DRQ or */
/*   send it. Useful for testing reads.                                    */
/*   retry specifies if retries should be attempted by the device          */
/* *********************************************************************** */
bool cIDE::Verify(BYTE dev, DWORD sector, bool retry) {
   if (!WaitBusy()) return false;
   if (!CheckStat()) return false;

   asm cli;

   outp(IDE_NSECTOR_REG, 1);
   outp(IDE_SECTOR_REG,  (sector & 0xFF));
   outp(IDE_LCYL_REG, ((sector >> 8) & 0xFF));
   outp(IDE_HCYL_REG, ((sector >> 16) & 0xFF));
   outp(IDE_SELECT_REG, 0xE0 | ((0x1 & dev) << 4) | ((sector >> 24) & 0xF));


   if (retry) return Command(CMD_VERIFYR);
   return Command(CMD_VERIFY);
}

/* *********************************************************************** */
/* Verify - CHS Version.                                                   */
/* *********************************************************************** */

bool cIDE::Verify(BYTE dev, WORD cylinder, BYTE head, BYTE sector, bool retry) {
   if (!WaitBusy()) return false;
   if (!CheckStat()) return false;

   asm cli;

   outp(IDE_SECTOR_REG,  sector);
   outp(IDE_LCYL_REG,    cylinder & 0xFF);
   outp(IDE_HCYL_REG,    (cylinder >> 8) & 0xFF);
   outp(IDE_SELECT_REG,  0xA0 | ((0x1 & dev) << 4) | (head & 0xF) );
   if (retry) return Command(CMD_VERIFYR);
   return Command(CMD_VERIFY);
}


/* *********************************************************************** */
/* Recal - Issues recallibrate command                                     */
/* *********************************************************************** */
bool cIDE::Recal(BYTE dev) {
   WaitBusy();

   asm cli;

   if (!Select(dev)) return false;
   return Command(CMD_RECAL);
}


/* *********************************************************************** */
/* ExeDiag - Executes Device Diagnostic, performs internal diagnostics.    */
/*   We have to do the command here since the results are in the error reg */
/*   Returns a one byte value, ABh. A is the master return, B is slave.    */
/*   0 is fail. 1 is passed or not present           .                     */
/*   FF the command failed                                                 */
/*   I'm going to assume at this time that if STATUS_ERROR is set the cmd  */
/*   itself failed and the results are meaningless.                        */
/* *********************************************************************** */
BYTE cIDE::ExeDiag() {
   int stat, err ;
   if (!WaitBusy()) return false;
   if (!CheckStat()) return false;
   outp(IDE_COMMAND_REG, CMD_DIAG);
   if (!WaitBusy()) return false;
   if (!CheckStat()) return false;

   asm cli;

   stat = inp(rStatus);
   err = inp(IDE_ERROR_REG);
   if (stat & STATUS_ERROR) {
   LOGLO("[IDE %Xh] ExeDiag Error Flag Set, command failed", base); return 0xFF; }

   err = err & 0x81;   // Only care about end bits
   if (err == 0x00) return 0x01;
   if (err == 0x80) return 0x00;
   if (err == 0x01) return 0x11;
   if (err == 0x81) return 0x10;
   return 0xFF;  // should never get here
}


/* *********************************************************************** */
/* Eject - Issues an eject command                                         */
/* *********************************************************************** */
bool cIDE::Eject(BYTE dev) {
	WaitBusy();
	CheckStat();
	if (!Select(dev)) return false;
	return Command(CMD_EJECT);
}


/* *********************************************************************** */
/* *********************************************************************** */
/* Accessor functions, access variables and return information             */
/* *********************************************************************** */
/* *********************************************************************** */

char *cIDE::Model(BYTE drive) {
   if (drive == 0) return sMaster.model;
   return sSlave.model;
}

char *cIDE::Firmware(BYTE drive) {
   if (drive == 0) return sMaster.firmware;
   return sSlave.firmware;
}

char *cIDE::Serial(BYTE drive) {
   if (drive == 0) return sMaster.serial;
   return sSlave.serial;
}


bool cIDE::IsDrive(BYTE drive) {
   if (drive == 0) return IsMaster();
   return IsSlave();
}

bool cIDE::IsMaster() {
   if (bMaster) return true;
   return false;
}

bool cIDE::IsSlave() {
   if (bSlave) return true;
   return false;
}


/* Returns a static string that contains the current geometry
   formated as C/H/S. Ensure this works with LBA translations. */
char *cIDE::CHSString(BYTE drive) {
   static char tchs[20];
   struct ideinfo_data *d = NULL;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;
   else return NULL;

   sprintf(tchs, "%i/%i/%i", d->cyls, d->heads, d->sectors);

   return tchs;
}



/* Return the capacity in megs of a drive. This function
   can decide how to calculate it. */
long cIDE::Capacity(BYTE drive) {
   struct ideinfo_data *d = NULL;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;
   else return 0;

   if (d->cyls == 16383 && d->heads == 16 && d->sectors == 63) {
      return CapacityLBA(drive);    // we are an lba drive
   }

   return CapacityCHS(drive);  // we are chs drive 
   
}

/* Return the capacity of the drive in megs with 
   ((C * H * S) / 1000000) * 512. Be careful to divide first
   so ulong does not overflow.           */

long cIDE::CapacityCHS(BYTE drive) {
   struct ideinfo_data *d = NULL;
   unsigned long cap = 0;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;
   else return 0;

   cap = (unsigned long)d->cyls * (unsigned long)d->heads *
	     (unsigned long)d->sectors;
   cap = (unsigned long) 
	     ((((unsigned long) cap) / ((float) QBYTEM)) * (float) 512);

   return cap;
}

long cIDE::CapacityLBA(BYTE drive) {
   struct ideinfo_data *d = NULL;
   unsigned long cap = 0;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;
   else return 0;

   cap = (unsigned long)
	    ((((unsigned long)d->lba_capacity) / ((float) QBYTEM)) * (float) 512);

   return cap;  
}


/* Return the number of absolute sectors per track.
   CHECK NOTICE: I'm assuming that if a drive is in LBA mode, then
   sectors will be set to 63, and that value is true. Find out if
   the value can be different even though the sector value is set
   to 63. */


int  cIDE::SectorPerTrack(BYTE drive) {
   struct ideinfo_data *d = NULL;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;

   return d->sectors;
}


/* Return the last absolute sector addressable.
   CHECK NOTICE: I'm assuming if CHS is valid (not set to 16383/16/63)
   then C + H + S = LBA. Check if this is the case. 
*/
long cIDE::MaxLastSector(BYTE drive) {
   struct ideinfo_data *d = NULL;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;
   
   /* Are we LBA? */
   if (d->cyls == 16383 && d->heads == 16 && d->sectors == 63) {
      return (d->lba_capacity);   
   }

   /* Not LBA, return CHS */
   return (d->cyls * d->heads * d->sectors);
}



/* Decode the config byte and return a string reporting the type of dev 
   CHECK NOTICE: Bit 7 is for removable media, bit 6 is for nonremovable
   controller and or device. Right now just check 7, find out more later.
*/
char *cIDE::ConfigType(BYTE drive) {
   struct ideinfo_data *d = NULL;
   static char buf[50];

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;
	
   if (d->config & 0x8000) 
   sprintf(buf, "ATAPI ");
   else
	   sprintf(buf, "ATA ");

   if (d->config & 0x0080)
	   strcat(buf, "Removable");
//   else
//	   strcat(buf, "Nonremovable");

   return buf;
}


/* Returns if the drive is a hard drive. Just return ATA devices */
bool cIDE::IsHD(BYTE drive) {
   struct ideinfo_data *d = NULL;

   if (drive == 0 && !bMaster) return false;
   if (drive == 1 && !bSlave) return false;

   if (drive == 0) d = &sMaster;     // setup pointer
   else if (drive == 1) d = &sSlave;


   if (!(d->config & 0x8000)) return true;
   return false;
}
