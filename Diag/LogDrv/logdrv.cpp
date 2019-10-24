#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <string.h>
#include <sys\stat.h>
#include <dir.h>

#include "declare/global.h"
#include "diag/logdrv/logdrv.h"

/* Functions all pass drive numbers in reference to the local arrays.
   0 = bios 0x80, 1 = bios 0x81 

   Bios Drive
      - Partition Table
   Drive Letters



*/


struct systypes {
   unsigned char index;
   char *name;
};

struct systypes sys_types[] = {
	{0x00, "Empty"},
	{0x01, "DOS 12-bit FAT"},
	{0x02, "XENIX root"},
	{0x03, "XENIX usr"},
	{0x04, "DOS 16-bit <32M"},
	{0x05, "Extended"},
	{0x06, "DOS 16-bit >=32M"},
	{0x07, "HPFS/NTFS"},		/* or QNX? */
	{0x08, "AIX"},
	{0x09, "AIX bootable"},
	{0x0a, "OS/2 Boot Manager"},
	{0x0b, "Win95 FAT32"},
	{0x0c, "Win95 FAT32 (LBA)"},
	{0x0e, "Win95 FAT16 (LBA)"},
	{0x0f, "Win95 Extended (LBA)"},
	{0x11, "Hidden DOS FAT12"},
	{0x14, "Hidden DOS FAT16"},
	{0x16, "Hidden DOS FAT16 (big)"},
	{0x17, "Hidden OS/2 HP/NTFS"},
	{0x40, "Venix 80286"},
	{0x51, "Novell?"},
	{0x52, "Microport"},		/* or CPM? */
	{0x63, "GNU HURD"},		/* or System V/386? */
	{0x64, "Novell Netware 286"},
	{0x65, "Novell Netware 386"},
	{0x75, "PC/IX"},
	{0x80, "Old MINIX"},		/* Minix 1.4a and earlier */

	{0x81, "Linux/MINIX"}, /* Minix 1.4b and later */
	{0x82, "Linux swap"},
	{0x83, "Linux native"},
	{0x85, "Linux extended"},

	{0x86, "NTFS volume set"},
	{0x87, "NTFS volume set"},


	{0x93, "Amoeba"},
	{0x94, "Amoeba BBT"},		/* (bad block table) */
	{0xa5, "BSD/386"},
	{0xa6, "OpenBSD"},
	{0xa7, "NEXTSTEP"},
	{0xb7, "BSDI fs"},
	{0xb8, "BSDI swap"},
	{0xc7, "Syrinx"},
	{0xdb, "CP/M"},			/* or Concurrent DOS? */
	{0xe1, "DOS access"},
	{0xe3, "DOS R/O"},
	{0xf2, "DOS secondary"},
	{0xff, "BBT"},			/* (bad track table) */
	{ 0, NULL }
};


cLogDrv::cLogDrv() {
	memset(dlet, 0, sizeof(dlet));
	memset(biosdrv, 0, sizeof(biosdrv));
	numdrvs = 0;
	numlets = 0;
	lbaext = 0;
	unmatched = FALSE;
	DosVerStr[0] = 0;  // Prenull the string
	comsize = 0;

}

cLogDrv::~cLogDrv() {

}

void cLogDrv::Init() {
	LOGHI("--- Logical Drives Initalizing ---");
	ScanDrvLetters();
	GetBiosDriveParams();
}








/* ************************************************************************* */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*                           BIOS DRIVE FUNCTIONS							 */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* ************************************************************************* */


/* Fetches the number of logical drives bios reports */
int cLogDrv::GetBiosNumDrvs() {
   unsigned char i;

   asm {
	  push ds
	  push 0x40
	  pop ds
	  mov bx, 0x0075
	  mov al, byte ptr [bx]
	  mov byte ptr i, al
	  pop ds
   }

   LOGHI("Bios reports %i drives.", i);
   return i;
}

/* Check if LBA int 13 extensions are availiable. Bdrv->drive will be 
   preloaded with what drive to check
*/
int cLogDrv::CheckLBAExtensions(struct biosdrive_data *bdrv) {
   unsigned int fi, tbx, tcx;
   unsigned char tah, tdh;
   char tdrive = bdrv->drive;

   bdrv->int_13ext = FALSE;    // preload so we can just return if no
   bdrv->int_13ext_version = 0;

   asm {
	  mov tah, 0
	  mov tdh, 0

	  mov ah, 0x41	    // Int 13h extension check
	  mov bx, 0x55AA
	  mov dl, tdrive
	  int 0x13

	  pushf 		    // Push flag register onto stack
	  pop fi             // Pop it onto our variable

	  mov byte ptr tah, ah   // Major version
	  mov byte ptr tdh, dh   // Extended version
	  mov tbx, bx            // AA55 if installed
	  mov tcx, cx
   }

   if (fi & 1) {     /* Carry flag set, no extensions */
	  LOGHI("Drive %Xh has no LBA extensions, carry set.", tdrive);
	 return FALSE;
   }

   if (tah == 0x80 || tah == 0x86) {
	  LOGHI("Drive %Xh has no LBA extensions, function not supported.", tdrive);
	  return FALSE;
   }

   if (tbx != 0xAA55) {
	  LOGHI("Drive %Xh has no LBA extensions, return incorrect.", tdrive);
	  return FALSE;
   }

   LOGHI("Drive %Xh has LBA extensions   Major: %.2Xh  Extended: %.2Xh  API: %.2Xh.", tdrive, tah, tdh, tcx);

   bdrv->int_13ext = TRUE;    // preload so we can just return if no
   bdrv->int_13ext_version = tah;

   return TRUE;   /* Computer has int 13h extensions */
}


/* Get the drive paramaters using extended int 13h function 48h. hdtemp
   will be preloaded with the drive number to scan, and the int13 extended
   functions version. Load it with returned values.
*/
int cLogDrv::GetExtendedDriveParam(struct biosdrive_data *hdtemp) {
   unsigned int i;
   unsigned char tdrive;
   unsigned char tbuf[300];     /* Big to accept any bios versions response */
   char *p;
   unsigned char tah;

   memset(tbuf, 0x0, sizeof(tbuf));
   /* This is a workaround for borland operand size dumbness */
   p = tbuf;

   /* First byte of buffer contains buffer length. If we report an unknown
	  version try the smallest buffer. */
   switch(hdtemp->int_13ext_version) {
	  case 0x1:  tbuf[0] = 0x1A; break;
	  case 0x20: tbuf[0] = 0x1E; break;
	  case 0x21: tbuf[0] = 0x42; break;
	  default:   tbuf[0] = 0x1A; break;
   }

   tbuf[1] = 0; tbuf[2] = 0; tbuf[3] = 0;  // Phoenix 4.0 workaround
   tdrive = hdtemp->drive;
   tah = 0;

   asm{
	  xor bx, bx
	  xor cx, cx
	  xor dx, dx
	  xor ax, ax

	  push ds
	  push si

	  mov ah, 0x48        // Function 48h: Enhanced Get Drive Paramaters
	  mov dl, tdrive       // Drive to query
	  lds si, dword ptr p // Our buffer
	  int 0x13

	  pushf                         // Fetch the flag register
	  pop i

	  mov byte ptr tah, ah			// Save ah, must be 0

	  pop si
	  pop ds


	}


  if (i & 1) {     /* Carry flag set, operation failed */
	   LOGLO("GetExtendedDriveParam - %Xh Carry flag set! Operation failed. AH is %XH", tdrive, tah);
	 return FALSE;
  }

  if (tah != 0) {   /* AH must be 0 to show a successful call */
	   LOGLO("GetExtendedDriveParam - Drive %Xh AH %.1Xh AH not zero, operation failed.", tdrive, tah);
	 return FALSE;
  }


  /* Quick way of extracting the double words from the buffer without ASM */
  hdtemp->cylinder = ((unsigned long) tbuf[0x4]) << 0  | ((unsigned long) tbuf[0x5]) << 8  |
					 ((unsigned long) tbuf[0x6]) << 16 | ((unsigned long) tbuf[0x7]) << 24;
  hdtemp->head     = ((unsigned long) tbuf[0x8]) << 0  | ((unsigned long) tbuf[0x9]) << 16 |
					 ((unsigned long) tbuf[0xA]) << 16 | ((unsigned long) tbuf[0xB]) << 24;
  hdtemp->sector   = ((unsigned long) tbuf[0xC]) << 0  | ((unsigned long) tbuf[0xD]) << 8  |
					 ((unsigned long) tbuf[0xE]) << 16 | ((unsigned long) tbuf[0xF]) << 24;

  /* Note: total sectors is actually a qword. However a dword can report up
	 to FFFF FFFF sectors, which is a 2,199 gig drive. I think it's
	 reasonably safe not to deal with a 64 bit sector count.              */
  hdtemp->total_sectors =
		((unsigned long) tbuf[0x10]) << 0  | ((unsigned long) tbuf[0x11]) << 8  |
		((unsigned long) tbuf[0x12]) << 16 | ((unsigned long) tbuf[0x13]) << 24;

  hdtemp->capacity = (float) hdtemp->total_sectors * 0.000512;

  hdtemp->info = ((unsigned long) tbuf[0x2]) | (((unsigned long) tbuf[0x3]) << 8);

  LOGHI("GetExtendedDriveParam - Drive: %Xh  Capacity: %u", hdtemp->drive, hdtemp->capacity);
  LOGHI("  cylinders %lu  heads %lu  sectors %lu  info %Xh",
				  hdtemp->cylinder, hdtemp->head, hdtemp->sector, hdtemp->info);
  LOGHI("total sectors %lXh  calculated capacity %u", hdtemp->total_sectors,
		(int) ((((float) hdtemp->total_sectors) / ((float) 1000000)) *
		  (int) 512));

  i = tbuf[0x18] | (tbuf[0x19] << 8);
  LOGHI("Sector per Track: %i", i);
  return TRUE;
}


/* Get the drive paramaters using the old function 08h int 13h. hdtemp
   will be preloaded with the drive number to scan. Load it with returned
   values.
*/             
int cLogDrv::GetNonExtendedDriveParam(struct biosdrive_data *hdtemp) {
   int i, tcx, tmax;
   unsigned char tdrive;
   byte tsector, thead;
   word tcylinder;


   tdrive = hdtemp->drive;

   asm{
	  xor bx, bx
	  xor cx, cx
	  xor dx, dx

	  mov ah, 0x08        // Function 08h: Get Drive Paramaters
	  mov dl, tdrive
	  int 0x13

	  pushf                         // Fetch the flag register
	  pop i

	  mov ax, cx		          // Copy the encoded sector/cyl to ax
	  and al, 0x3f		          // Zero the first two bits
	  mov byte ptr tsector, al    // Copy the result

	  xor ax, ax				  // Zero ax in prepration
	  mov al, ch				  // Move the high bits low
	  xor ch, ch			      // Zero the high so we can shift right

	  and cl, 0xc0				  // Null out the right five bits
	  shr cx, 0x6                  // Shift right 6


	  mov ah, cl			      // Move our two bits high
	  mov tcylinder, ax			  // Copy the cylinders

	  mov byte ptr thead, dh
	  mov byte ptr tmax, dl
	}

  if (i & 1) {     /* Carry flag set, operation failed */
	LOGLO("GetNonExtendedDriveParam - %Xh Carry flag set! Operation failed.", tdrive);
	 return FALSE;
  }

/* if (num_drives == -1) {    * Set max drives *
	num_drives = tmax;
	   LOGLO("nonextended_get_drive_param() - %Xh num_drives reported as %i.", num_drives);
	if (num_drives < 1) {
	   LOGLO("nonextended_get_drive_param() - %Xh num_drives reported as < 1.", tdrive);
	   return FALSE;
	}
  }
*/

  hdtemp->sector = tsector;
  hdtemp->cylinder = tcylinder;
  hdtemp->head = thead;
  hdtemp->total_sectors = (((long) tsector * (long) thead) *
						(long) tcylinder);

  hdtemp->capacity = (float) hdtemp->total_sectors * 0.000512;

  LOGHI("GetNonExtendedDriveParam - Drive: %Xh  Capacity: %u", hdtemp->drive, hdtemp->capacity);
  LOGHI("   sectors %i  cylinders %i  heads %i  tmax %Xh", tsector, tcylinder, thead, tmax);

  return TRUE;
}



/* ----------------------------------------------------------------------- */
/* Read a single sector and put it in buf. Exit if its out of bounds       */
/* ----------------------------------------------------------------------- */
int cLogDrv::ReadBiosSector(word drive, word cylinder, byte head, byte sector, byte *buf) {
  int i = 0;

  asm {
	 mov ax, cylinder
	 and ax, 0x0300
	 mov cl, 2
	 shr ax, cl
	 mov cl, al
	 mov al, sector
	 and al, 0x3f
	 or cl, al
	 mov ch, byte ptr cylinder
	 mov dh, head
	 mov dl, byte ptr drive
	 les bx, buf
	 mov ah, 0x02
	 mov al, 0x01
	 int 0x13
	 pushf
	 pop i
   }

  if (i & 1)      /* Carry flag set, operation failed */
	 return FALSE;

  return TRUE;
}






/* ----------------------------------------------------------------------- */
/* Reads secbuf[] and parses any partion tables found, populates p[]
   Return Codes:
	 -1: No boot signature found
	  0: No partitions found
  [1-4]: Number of partitions found                                        */
/* ----------------------------------------------------------------------- */

int cLogDrv::ReadPartitionTable(int drive, struct partition_data *p) {
   int i, num_partition = 0;
   word tcylinder = 0;
   byte tsector = 0;
   unsigned char secbuf[530];

   /* Read the MBR */
   ReadBiosSector(drive, 0, 0, 1, secbuf);

   /* First make sure this is a bootsector */
   if ((secbuf[BOOT_SIG] != 0x55) || (secbuf[BOOT_SIG + 1] != 0xAA)) {
	  LOGLO("ReadPartitionTable - No boot signature found in MBR! Not a bootsector.");
	  return -1;
   }
   for (i = 0; i <= 3; ++i) {
	 p[i].stype = secbuf[P_TYPE + (i * 0x10)];

	 if (p[i].stype == 0x0)  /* No partition defined */
		continue;

	 ++num_partition;

	 p[i].boot = secbuf[P_BOOT + (i * 0x10)];

	 p[i].s_head = secbuf[P_SHEAD + (i * 0x10)];
	 tcylinder = secbuf[P_SSECTOR + (i * 0x10)];
	 tsector = secbuf[P_SCYLINDER + (i * 0x10)];

	 p[i].s_cylinder = cylinder(tsector, tcylinder);
	 p[i].s_sector   = sector(tsector);

	 p[i].e_head = secbuf[P_EHEAD + (i * 0x10)];

	 tcylinder = secbuf[P_ESECTOR + (i * 0x10)];
	 tsector = secbuf[P_ECYLINDER + (i * 0x10)];
	 p[i].e_cylinder = cylinder(tsector, tcylinder);
	 p[i].e_sector   = sector(tsector);

	 /* Get the sector offset. This is a cheap hack to load the reversed
		double word instead of mov eax, ptr double word secbuf[P_OFFSET] */
	 p[i].offset = ((long) secbuf[(P_OFFSET + 0) + (i * 0x10)]) +
				   ((long) secbuf[(P_OFFSET + 1) + (i * 0x10)] << 8) +
				   ((long) secbuf[(P_OFFSET + 2) + (i * 0x10)] << 16)  +
				   ((long) secbuf[(P_OFFSET + 3) + (i * 0x10)] << 24);

	 /* Get the total number of sectors. Same as above */
	 p[i].sectors = ((long) secbuf[(P_SIZE + 0) + (i * 0x10)]) +
					((long) secbuf[(P_SIZE + 1) + (i * 0x10)] << 8) +
					((long) secbuf[(P_SIZE + 2) + (i * 0x10)] << 16) +
					((long) secbuf[(P_SIZE + 3) + (i * 0x10)] << 24);
	 p[i].capacity = (float) p[i].sectors * 0.000512;

   }  /* for() */

   LOGHI("ReadPartitionTable - %i partitions found.", num_partition);

//   return num_partition;
   return TRUE;
}







/* Main function to populate biosdrv with all bios drives.                   */
int cLogDrv::GetBiosDriveParams() {
	int i, maxdrvs;

	
	LOGLO("Scanning Bios Drives...");
	numdrvs = 0;
	maxdrvs = GetBiosNumDrvs();

	for (i = 0; ((i < maxdrvs) || (i < 10)); ++i) {
		oThrob->Step();

		biosdrv[numdrvs].drive = 0x80 + i;  // set drive number
		CheckLBAExtensions(&biosdrv[numdrvs]);
		if (biosdrv[numdrvs].int_13ext) {
			if (GetExtendedDriveParam(&biosdrv[numdrvs])) {
			   ReadPartitionTable(biosdrv[numdrvs].drive, biosdrv[numdrvs].p);	 	
			   ++numdrvs; continue;  // Success. Continue onward
			} // failure, fall through 
		}
		/* We either dont support int13 or we failed extended. Fall back
		   to nonextended test */
		if (GetNonExtendedDriveParam(&biosdrv[numdrvs])) {
			ReadPartitionTable(biosdrv[numdrvs].drive, biosdrv[numdrvs].p);
			++numdrvs; continue;   // Success
		}
		/* TODO CHECK: We failed completely. Keep going? Or bomb out?  */
		/* Continue for now */
		biosdrv[numdrvs].drive = 0;
	}

	LOGLO(" ");
	return TRUE;
}











/* ************************************************************************* */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*                         DRIVE LETTER FUNCTIONS							 */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/* ************************************************************************* */

/* Letters are 1 = A:   2 = B:  3 = C:   Since we don't care about floppies
   we'll start scanning at 3. */


/* Main function for scanning drive letters. Hit them all, populate dlet[]   */
int cLogDrv::ScanDrvLetters() {
   int lnum;
   
   numlets = 0;

   LOGLO("Scanning Drive Letters ...");
//   printf("scanning letters\n");
   for (lnum = 3; lnum < 13; ++lnum) {
	  oThrob->Step();

//   printf("is cd\n");
		if (IsLetterCD(lnum)) continue;		// Skip CDROMS
//   printf("is remote\n");
		if (IsLetterRemote(lnum)) continue; // Skip remotes
		// Set letter up, will overwrite next time around if we fail.
		dlet[numlets].letter = 'A' + lnum - 1;
//   printf("is getfreespace\n");
		if (!GetLetterFreeSpace(&dlet[numlets])) break;  // Get out if we fail
		LOGHI("ScanDrvLetters - %c Free Space %lu",
				dlet[numlets].letter, dlet[numlets].extfree.sectorpercluster *
				dlet[numlets].extfree.bytepersector * dlet[numlets].extfree.freecluster);
//   printf("is getletterfs\n");
		GuessLetterFS(&dlet[numlets]);
//   printf("done once\n");
		++numlets;
		LOGHI(" ");
   }
   LOGLO(" ");
//   printf("done scanning letters");
	  oThrob->Step();

// TODO get this working
   GuessOSVersion();                 // Guess OS version of drive

   return TRUE;
}

/* Kills any line feeds, carriage returns, tabs, in a string */
void cLogDrv::trim_end(char *str) {
   int end = strlen(str);
   int i;

   for (i = end; i > 0; --i) {
	 if (str[i] == 0x0A || str[i] == 0x0D || str[i] == 0x0A)
		str[i] = 0;
   }
}


int cLogDrv::GuessOSVersion() {
   FILE *t;
   int olddrive;
   int i;
   char b[200];
   struct stat statbuf;


   LOGLO(" ");
   LOGLO("--- Attempting to Guess OS Version ---");
   olddrive = getdisk();

   setdisk(2);   // set us to C:\
   DosVerStr[0] = 0;  // Prenull the string

   if ((t = fopen("\\msdos.sys", "r")) != NULL) {
	   i = 0; // Find the WinVer line
	   while (fgets(b, 180, t) && (i < 50)) {
		  ++i; // sanity
		  if (b[0] == 'W' && b[1] == 'i' && b[2] == 'n' &&
			  b[3] == 'V' && b[4] == 'e' && b[5] == 'r' && b[6] == '=') {
			  strncpy(DosVerStr, &b[7], 18);
			  trim_end(DosVerStr);
			  LOGLO("MSDOS.SYS Version String: %s", DosVerStr);
			  break;  // break the while
		  }
	   } /* while () */
	   if (DosVerStr[0] == 0) {
		  LOGLO("Failed to locate MSDOS.SYS Version String."); }
	   fclose(t);
   } else {
	   LOGLO("Failed to open msdos.sys");
   }

   // Now check out command.com's size
   // WARNING - I wonder if this will trigger any antivirus programs?
   //           I don't think it will since I'm just stat'ing.
   if (stat("\\command.com", &statbuf) == 0) {
	  comsize = statbuf.st_size;
	  LOGLO("Command.Com is %lu bytes.", comsize);
   } else {
	  comsize = 0;
	  LOGLO("Failed to stat Command.Com.");
   }


   LOGLO(" ");
   setdisk(olddrive);
   return 1;
}


/* Check if drive is a CDROM by asking MSCDEX (if exists).
   Return TRUE if we are a CDROM.
		  FALSE if we are NOT.
*/
int cLogDrv::IsLetterCD(int letnum) {
	unsigned int tbx, tax, c;
	static once = FALSE;

	asm {
		 mov ax, 0x150B
		 mov cx, letnum
		 dec cl          // this function starts with a = 0
		 int 0x2F
		 pushf
		 pop c
		 mov tbx, bx
		 mov tax, ax
	 }
	  /* If we get an error just continue on. ADAD indicates this is a
		 CDROM being supported by MSCDEX.                              */
	  if (c & 1 && once == FALSE) {
	     LOGLO("Error on CD check drive %c (%Xh)", 'A' - 1 + letnum, tax);
	  }

	  if (once == FALSE) {   // Print a mscdex message once
		 if (tbx == 0x0ADAD)
			   LOGLO("CD Check - MSCDEX is installed.");
		 else
			   LOGLO("CD Check - MSCDEX is NOT installed.");
		 once = TRUE;
	  }

	  /* MSCDEX is installed, check if this drive is supported
		 AX = 0 not supported, nonzero supported*/
	  if (tbx == 0x0ADAD) {
		 if (tax != 0) {    // its a cdrom
			   LOGHI("CD Check - Drive %c is a CD-ROM. (%Xh)", 'A' - 1 + letnum, tax);
			return TRUE;
		 }
		 LOGHI("CD Check - Drive %c has no CD-ROM support. (%Xh)", 'A' - 1 + letnum, tax);
	  }

	/* If we fall through we're not a CDROM. */
	  return FALSE;
}

/* Checks if the drive is a remote drive. This should only occur if someone is 
   running us under windows and has an SMB share up. 
   Return TRUE if drive is remote.
          FALSE if not.
*/
int cLogDrv::IsLetterRemote(int letnum) {
	int tax, tdx, c;

	asm {
		 mov ax, 0x4409
		 mov bx, letnum
		 int 0x21
		 pushf
		 pop c
		 mov tax, ax
		 mov tdx, dx
	}

	/* If bit 12 is set we're remote. If bit 15 set we are SUBST'ed. If
	   either is true we're not a real physical drive */
	if (!(c & 1) && ( (tdx & 0x1000) || (tdx & 0x8000) ) ) {
		 LOGLO("scan_dletter() - %c is a remote drive. (%Xh)",
				 'A' + letnum - 1, tdx);
		 return TRUE;
	}

	return FALSE;
}


/* Get the free space of the drive and populate the structure. Get the drive
   letter from structure which should be preloaded as a letter.              
   Return TRUE if we succeed
          FALSE if an error or done with list
*/
int cLogDrv::GetLetterFreeSpace(struct dletter_data *drv) {
	char lstr[10];
    unsigned int length, tax, c;
	unsigned char tal;
	/* Generic pointer for inline assembly dumbness */
	void *pletter = NULL, *pextfree = NULL;

	sprintf(lstr, "%c:\\", drv->letter);  // setup path
	pletter = (void *)lstr;
	pextfree = (void *)&drv->extfree;
	length = sizeof(struct extfree_data);
//	printf("About to check freespace\n");
	  /* Get the freespace on the drive */
	  asm {
		 push ds
		 push di
		 mov  ax, 0x7303
		 lds  dx, pletter
		 mov  cx, length
		 les  di, pextfree
		 int 0x21
		 pushf
		 pop c
		 pop di
		 pop ds
		 mov tax, ax
		 mov tal, al
	  }
//	  printf("We survived\n");

	  /* We are going to assume that when we get error 0xF (no such drive)
		 we've hit the end of the chain and should stop.  */
	  if (c & 1) {
		 if (tax == 0x0F || tax == 0x0FFFF || tax == 0x15) {  // Invalid drive
			LOGHI("GetLetterFreeSpace - %c returned %.4Xh",
					 drv->letter, tax);
		 }
		 drv->letter = 0;
		 return FALSE;
	  }

	  if (tal == 0) {  // Function unsupported
		 LOGLO("GetLetterFreeSpace - %c returned function unsupported! Aborting scan.", 
			 drv->letter);
		 drv->letter = 0;
		 return FALSE;
	  }

	return TRUE;
}



/* Guess the filesystem of the drive letter.
   Return TRUE if we succeed.
          FALSE if we fail. 
*/
int cLogDrv::GuessLetterFS(struct dletter_data *drv) {
	char tbuf[520];

	drv->fstype = 0;
	if (!ReadDosSector(drv->letter - 'A' + 1, 0, tbuf)) {
	   LOGLO("GuessLetter - Failed to read bootsector of drive %c (drv %i).", 
		   drv->letter, drv->drive - 'A' + 1);
	   return FALSE;
	}

	if (tbuf[0x36] == 'F' && tbuf[0x37] == 'A' && tbuf[0x38] == 'T') {
		if (tbuf[0x39] == '1' && tbuf[0x3A] == '6')
		   drv->fstype = FS_FAT16;
		else if (tbuf[0x39] == '1' && tbuf[0x3A] == '2')
		   drv->fstype = FS_FAT12;
		else if (tbuf[0x39] == '3' && tbuf[0x3A] == '2')
		   drv->fstype = FS_FAT32;
	}
	else if (tbuf[0x52] == 'F' && tbuf[0x53] == 'A' && tbuf[0x54] == 'T') {
		if (tbuf[0x55] == '1' && tbuf[0x56] == '6')
		   drv->fstype = FS_FAT16;
		else if (tbuf[0x55] == '1' && tbuf[0x56] == '2')
		   drv->fstype = FS_FAT12;
		else if (tbuf[0x55] == '3' && tbuf[0x56] == '2')
		   drv->fstype = FS_FAT32;
	}

	switch (drv->fstype) {
		case FS_FAT12: LOGHI("Drive %c is FAT12", drv->letter); break;
		case FS_FAT16: LOGHI("Drive %c is FAT16", drv->letter); break;
		case FS_FAT32: LOGHI("Drive %c is FAT32", drv->letter); break;
		default: LOGHI("Drive %c has unknown filesystem", drv->letter); break;
    }
	return TRUE;
}



/* Read a sector from a drive letter and put it in the buf
   dletter a = 1, b = 2, c = 3, etc 
*/
int cLogDrv::ReadDosSector(char dletter, unsigned long sector, char *buf) {
   struct drive_packet { dword sector; word numsector; void *transfer; };
   struct drive_packet dp;
   struct drive_packet *p = &dp;
   unsigned int c, tax;

   dp.sector = sector;
   dp.numsector = 1;
   dp.transfer = buf;

   asm {
	  push ds
	  push si

	  mov ax, 0x7305
	  mov cx, 0xFFFF
	  lds bx, dword ptr p
	  xor dx, dx
	  push dx
	  pop si
	  mov dl, dletter
	  int 0x21
	  pushf
	  pop c
	  mov tax, ax

	  pop si
	  pop ds
   }

   if (c & 1)
	  return FALSE;
   return TRUE;

}



char *cLogDrv::GetPartitionName(unsigned char type) {
	int i;

	for (i=0; sys_types[i].name; i++)
		if (sys_types[i].index == type)
			return sys_types[i].name;

	return NULL;
}



int ScanDDT() {
	return 1;

}
