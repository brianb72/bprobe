#ifndef __LOGDRV_H__
#define __LOGDRV_H__

#include <Global.h>

#define BOOT_SIG    0x1FE   /* Boot sector Signature */

/* Partition table bytes */
#define P_BOOT      0x1BE  /* Boot indicator byte */
#define P_SHEAD     0x1BF  /* Starting head/side of partition */
#define P_SCYLINDER 0x1C0  /* Starting Cylinder */
#define P_SSECTOR   0x1C1  /* Starting Sector */
#define P_TYPE      0x1C2  /* Partition type */
#define P_EHEAD     0x1C3  /* Ending Head */
#define P_ECYLINDER 0x1C4  /* Ending Cylinder */
#define P_ESECTOR   0x1C5  /* Ending Sector */
#define P_OFFSET    0x1C6  /* Relative Sector Offset */
#define P_SIZE      0x1CA  /* Number of sectors in partition */


// Given cl will result in sectors
#define sector(s)               ((s) & 0x3f)

// Given cx with sectors calculated will result in cylinders
#define cylinder(s, c)  ((c) | (((s) & 0xc0) << 2))

// Given h s c returns a logical sector, used in formula.
#define calculate(h,s,c) (sector(s) - 1 + sectors * \
								((h) + heads * cylinder(s,c)))

// Given a logical sector calculates and sets h s c. Sectors and Heads
// must be set correctly and in scope for the current drive.
#define set_hsc(h,s,c,sector) { \
								s = sector % sector + 1;        \
								sector /= sectors;         \
								h = sector % heads;         \
								sector /= heads;           \
								c = sector & 0xff;         \
								s |= (sector >> 2) & 0xc0; \
							  }



/* Structure returned by Int 21/AX=7303 Extended Freespace Information */
struct extfree_data {
   word sizeret;			/* Size of returned structure */
   word verret;				/* version of ret. 0 = ret is version instead*/
   dword sectorpercluster;	/* Number of sectors per cluster */
   dword bytepersector;     /* Bytes per sector (Normally 512) */
   dword freecluster;		/* Number of clusters free */
   dword totalcluster;		/* Total number of clusters per drive */
   /* The following variables have no adjustment for compression */
   dword freesector;		/* Number of free sectors on drive */
   dword totalsector;       /* Total sectors */
   dword freealloc;			/* Number of free allocation units */
   dword totalalloc;		/* Number of total allocation units */
   char reserved[8];		/* reserved bytes */
};

/* Bios Paramater Block - Each drive data table has one of these, stored in DOS's memory */
struct bpb_data {		
	word   bytepersector;         /* Bytes per sector */
	byte   sectorpercluster;      /* FFh if unknown */
	word   reservedsector;        /* Number of reserved sectors */
	byte   numfat;                /* Number of FATs */
	word   numroot;               /* Number of root dir entries */
	word   totalsector;           /* See bigtotsector if this is 0h */
	byte   mediadescriptor;       /* 00h if unknown */
	word   sectorperfat;          /* Sectors per fat */
	word   sectorpertrack;
	word   numhead;
	dword  hiddensector;		  /* Number of hidden sectors */
	dword  bigtotsector;          /* Total sectors if totalsector is 0h */
	byte   flags;         /* bit 6 - 16 bit fat instead of 12 */
						  /* bit 7 - Unsupported disk */
	word   open_count;            /* Device open count */
	byte   device_type;           /* 05h = fixed disk */
	word   bit_flags;             /* Describing the device */
	word   numcylinder;           /* Number of cylinders for HD's only */
};

/* Drive Data Table - Dos drive table list */
struct ddt_data {                 /* driver.sys drive data table */
	struct ddt_data *next;        /* Pointer to next table offset FFFF if last */
	byte   biosnum;               /* Physical Unit Number 80h, 81h, 82h ...*/
	byte   logicalnum;            /* 0=A: 1=B: 2=C: ... */
	struct bpb_data bpb;          /* This drives BPB */

};


/* Gathered information on a single partition table entry */
struct partition_data {
   byte boot;        /* Boot indicator byte */

   byte s_head;      /* Starting H C S */
   word s_cylinder;
   byte s_sector;
   byte stype;      /* System indicator byte */
   byte e_head;     /* Ending H C S */
   word e_cylinder;
   byte e_sector;

   long offset;
   long sectors;
   long capacity;

   int drive;       /* Which drive this partition is on */
};

/* Bios drive structure, gathered from various functions */
struct biosdrive_data {
   byte int_13ext;   /* Does drive support int 13 extensions? */
   byte int_13ext_version;

   /* Physical */
   int  physical;    /* Matched to physical drive # 1 = pri/mas 2 = pri/sec*/
   byte drive;       /* Drive number */
   dword cylinder;    /* Max cylinders */
   dword head;        /* Max heads */
   dword sector;      /* Max Sectors */
   unsigned long total_sectors;   /* Total sectors this drive has */
   unsigned long capacity;        /* Drive capacity in megs */
   unsigned long info;         /* Extended information (through function 48h) */

   /* Logical */
   byte type;        /* Drive type */
   struct partition_data p[4];     /* 4 partitions per drive */
};

#define FS_FAT12  1
#define FS_FAT16  2
#define FS_FAT32  3
/* Hard drive logical data per drive letter */
/* NOTE: The ddt is a pointer into dos's internal memory. We don't own it
   and we didn't create it. DO NOT ATTEMPT TO MODIFY IT'S DATA. */
struct dletter_data {
   char letter; 			      /* Drive letter C, D, E... */
   struct extfree_data extfree;   /* Extended freespace on the drive */
   int drive;					  /* Bios drive we think this partitions on */
   int fstype;
   struct ddt_data *ddt;          /* drive data table (if it exists) */
};



class cLogDrv {
public:
	cLogDrv();
	~cLogDrv();
	void Init();
public:   /* Bios drive functions */
	int GetBiosNumDrvs();
	int GetBiosDriveParams();
	int CheckLBAExtensions(struct biosdrive_data *bdrv);
    int GetExtendedDriveParam(struct biosdrive_data *hdtemp);
    int GetNonExtendedDriveParam(struct biosdrive_data *hdtemp);
    int ReadBiosSector(word drive, word cylinder, byte head, byte sector, byte *buf);
    int ReadPartitionTable(int drive, struct partition_data *p);
    char *GetPartitionName(unsigned char type);
public:   /* Drive Letter functions */
	int ScanDrvLetters();
	int IsLetterCD(int letnum);			// Determine if letter is a cdrom  drive
	int IsLetterRemote(int letnum);		// Determine if letter is a remote drive
	int GetLetterFreeSpace(struct dletter_data *drv); // Gets freespace structure of drive
	int GuessLetterFS(struct dletter_data *drv);      // Guess Letter FS, duh
	int ReadDosSector(char dletter, unsigned long sector, char *buf);
	int ScanDDT();
	int GuessOSVersion();                 // Guess OS version of drive
	void trim_end(char *str);
public:
	int numdrvs;		// Number of drives in the system
	int numlets;        // Number of drive letters on the system
	int lbaext;			// Are int 13 extensions avail
	int unmatched;      // Are there any unmatched drive letters?
public:
	char DosVerStr[20];            /* Version string in msdos.sys */
	long comsize;                  /* Size of command.com in bytes */

	struct dletter_data      dlet[10];	// Structure for a drive letter, its ddt, and bpb
	struct biosdrive_data biosdrv[10];  // Structure for a bios drive and partition table
};


#endif
