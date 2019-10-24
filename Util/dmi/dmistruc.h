#ifndef __DMISTRUCT_H__
#define __DMISTRUCT_H__

#include <Global.h>

struct dmi_entry_data {
   byte  _dmi_[5];
   byte  DMIChecksum;
   word  TableLength;     // Total length of all tables in bytes
   dword TableAddr;       // Address to table structure
   word  NumStructure;    // Number of structures
   byte  BCDRevision;
};

struct smbios_entry_data {
   byte _sm_[4];
   byte EntryChecksum;
   byte EntryLength;      // Either 1Eh or 1Fh (should be 1Fh)
   byte MajorVersion;
   byte MinorVersion;
   word MaxStructSize;
   byte EntryRevision;
   byte formated[5];
   struct dmi_entry_data dmi;
};


// TYPE 0
struct dmi_bios_data {
   byte  type;
   byte  length;          // 12h + number of bios extension bytes
   word  handle;
   byte  nVendor;
   byte  nVersion;
   word  segment;         // Bios starting segment
   byte  nRelease;
   byte  romsize;
   dword charhigh;
   dword charlow;
   byte  extension[2];    // 1 or 2 extension bytes
};


#define MAX_DMI_DATA_STRING 10
struct dmi_data {
    char *p;						  /* Pointer to BIOS area of structure */
    char *s[10];                    /* Pointers to each of the strings */
};


#define DMI_BIOS 0
struct dmi_bios {
   byte   type;
   byte   length;
   word   handle;
   byte   nVendor;
   byte   nVersion;
   word   StartingSeg;		/* Starting segment of bios */
   byte   nDate;
   byte   RomSize;
   dword  char1;
   dword  char2;
   byte   ext[2];
};

#define DMI_SYSINFO 1
struct dmi_sysinfo {
   byte   type;
   byte   length;
   word   handle;
   byte   nManuf;
   byte   nProduct;
   byte   nVersion;
   byte   nSerial;
   byte   uuid[16];
   byte   wakeup;

};

#define DMI_BASE   2
struct dmi_base {
   byte   type;
   byte   length;
   word   handle;
   byte   nManuf;
   byte   nProduct;
   byte   nVersion;
   byte   nSerial;
};

#define DMI_ENC    3
struct dmi_enc {
   byte   type;
   byte   length;
   word   handle;
   byte   nManuf;
   byte   enctype;
   byte   nVersion;
   byte   nSerial;
   byte   nAsset;
   byte   state;
   byte   power;
   byte   thermal;
   byte   security;
   dword  oem;
};

#define DMI_PROC   4
struct dmi_proc {
   byte   type;
   byte   length;
   word   handle;
   byte   nSocket;
   byte   proctype;
   byte   family;
   byte   nManuf;
   dword  id1;
   dword  id2;
   byte   nVersion;
   byte   voltage;
   word   clock;
   word   maxspeed;
   word   curspeed;
   byte   status;
   byte   upgrade;
   word   l1;
   word   l2;
   word   l3;
   byte   nSerial;
   byte   nAsset;
   byte   nPart;
};

#define DMI_MEMCON 5
struct dmi_memcon {
   byte   type;
   byte   length;
   word   handle;
   byte   errdet;
   byte   errcor;
   byte   supint;
   byte   curint;
   byte   maxsize;
   word   supspeed;
   word   suptype;
   byte   voltage;
   byte   numslots;
   word   memhandle;
};

#define DMI_MEMMOD 6
struct dmi_memmod {
   byte   type;
   byte   length;
   word   handle;
   byte   nSocket;
   byte   bank;
   byte   speed;
   word   memtype;
   byte   installsize;
   byte   enablesize;
   byte   error;
};

#define DMI_CACHE 7
struct dmi_cache {
   byte   type;
   byte   length;
   word   handle;
   byte   nSocket;
   word   config;
   word   maxsize;
   word   installsize;
   word   suptype;
   word   curtype;
   byte   speed;
   byte   error;
   byte   ctype;
   byte   assoc;
};



#define DMI_PORT 8
struct dmi_port {
   byte   type;
   byte   length;
   word   handle;
   byte   nIntRef;
   byte   inttype;
   byte   nExtRef;
   byte   exttype;
   byte   funct;
};

#define DMI_SLOT 9
struct dmi_slot {
   byte   type;
   byte   length;
   word   handle;
   byte   nName;
   byte   stype;
   byte   width;
   byte   usage;
   byte   slength;
   word   id;
   byte   char1;
   byte   char2;
};

#define DMI_ONBOARD 10
/* SKIPPED */

#define DMI_OEMINFO 11
#define DMI_SYSCONFIG 12
struct dmi_oeminfo {
   byte   type;
   byte   length;
   word   handle;
   byte   strings;
};

#define DMI_LANG 13
struct dmi_lang {
   byte   type;
   byte   length;
   word   handle;
   byte   numlang;
   byte   flags;
   byte   reserved[15];
   byte   nLanguage;
};

#define DMI_GROUPASSOC 14
/* SKIPPED */
#define DMI_SYSLOG 15
/* SKIPPED */

#define DMI_PHYMEM 16
struct dmi_phymem {
   byte   type;
   byte   length;
   word   handle;
   byte   loc;
   byte   use;
   byte   error;
   dword  capacity;
   word   errhandle;
   word   num;
};

#define DMI_MEMDEV 17
struct dmi_memdev {
   byte   type;
   byte   length;
   word   handle;
   word   memhandle;
   word   errhandle;
   word   totalwidth;
   word   datawidth;
   word   size;
   byte   form;
   byte   deviceset;
   byte   nDev;
   byte   nBank;
   byte   memtype;
   word   typedetail;
   word   speed;
   byte   nManuf;
   byte   nSerial;
   byte   nAsset;
   byte   nPart;
};

#define DMI_MEMERR 18
struct dmi_memerr {
   byte   type;
   byte   length;
   word   handle;
   byte   errtype;
   byte   errgran;
   byte   errop;
   dword  vendsyn;
   dword  memerr;
   dword  deverr;
   dword  errres;
};

#define DMI_MEMARRAY 19
struct dmi_memarray {
   byte   type;
   byte   length;
   word   handle;
   dword  start;
   dword  ending;
   word   memhandle;
   byte   width;
};

#define DMI_MAPPED 20
struct dmi_mapped {
   byte   type;
   byte   length;
   word   handle;
   dword  start;
   dword  ending;
   word   devhandle;
   word   addrhandle;
   byte   row;
   byte   inter;
   byte   interdepth;
};

#define DMI_BOOT 32
struct dmi_boot {
   byte   type;
   byte   length;
   word   handle;
   byte   reserved[6];
   byte   status[10];
};


#endif



