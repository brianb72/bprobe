
#ifndef __ATAIDE_H__
#define __ATAIDE_H__

#include <Global.h>


struct ideinfo_data {
	WORD config;
	WORD cyls;
	WORD reserved2;
	WORD heads;
	WORD track_bytes;
	WORD sector_bytes;
	WORD sectors;
	WORD vendor0;
	WORD vendor1;
	WORD vendor2;
	BYTE serial[20];
	WORD buf_type;
	WORD buf_size;
	WORD ecc_bytes;
	BYTE  firmware[8];
	BYTE  model[40];
	BYTE  max_multsect;
	BYTE  vendor3;
	WORD  dword_io;
	BYTE  vendor4;
	BYTE  capability;
	WORD reserved50;
	BYTE vendor5;
	BYTE tPIO;
	BYTE vendor6;
	BYTE tDMA;
	WORD field_valid;      /* 53 */
	WORD cur_cyls;         /* 54 */
	WORD cur_heads;        /* 55 */
	WORD cur_sectors;      /* 56 */
	WORD cur_capacity0;    /* 57 */
	WORD cur_capacity1;    /* 58 */
	BYTE multsect;
	BYTE multsect_valid;   /* 59 */
	DWORD lba_capacity;   /* 60-61 Really a int, but in dos int = 2 byte */
	WORD dma_1word;      /* 62 */
	WORD dma_mword;      /* 63 */
	WORD eide_pio_modes; /* 64 */
	WORD eide_dma_min;   /* 65 */
	WORD eide_dma_time;  /* 66 */
	WORD eide_piol;      /* 67 */
	WORD eide_pio_iordy; /* 68 */
	WORD word69;         /* 69 */
	WORD word70;         /* 70 */
	/* HDIO_GET_IDENTITY returns only words 0 through 70 */
	WORD word71, word72, word73, word74, word75, word76, word77,
	word78, word79, word80, word81, word82, word83, word84, word85, word86,
	word87, dma_ultra, reserved[167];
};

/* IDE register offsets */
#define IDE_DATA_OFFSET    0
#define IDE_ERROR_OFFSET   1
#define IDE_NSECTOR_OFFSET 2
#define IDE_SECTOR_OFFSET  3
#define IDE_LCYL_OFFSET    4
#define IDE_HCYL_OFFSET    5
#define IDE_SELECT_OFFSET  6
#define IDE_STATUS_OFFSET  7
#define IDE_FEATURE_OFFSET IDE_ERROR_OFFSET
#define IDE_COMMAND_OFFSET IDE_STATUS_OFFSET

/* IDE registers */
#define IDE_DATA_REG     (base + IDE_DATA_OFFSET)
#define IDE_ERROR_REG    (base + IDE_ERROR_OFFSET)
#define IDE_NSECTOR_REG  (base + IDE_NSECTOR_OFFSET)
#define IDE_SECTOR_REG   (base + IDE_SECTOR_OFFSET)
#define IDE_LCYL_REG     (base + IDE_LCYL_OFFSET)
#define IDE_HCYL_REG     (base + IDE_HCYL_OFFSET)
#define IDE_SELECT_REG   (base + IDE_SELECT_OFFSET)
#define IDE_STATUS_REG   (base + IDE_STATUS_OFFSET)
#define IDE_CONTROL_REG  (base + 0x206)
#define IDE_FEATURE_REG IDE_ERROR_REG
#define IDE_COMMAND_REG IDE_STATUS_REG
#define IDE_ALTSTATUS_REG IDE_CONTROL_REG
#define IDE_IREASON_REG IDE_NSECTOR_REG
#define IDE_BCOUNTL_REG IDE_LCYL_REG
#define IDE_BCOUNTH_REG IDE_HCYL_REG

/* Status flags */
#define STATUS_ERROR    0x01  // Error detected, error register has details 
#define STATUS_INDEX    0x02  // Vendor specific 
#define STATUS_ECC      0x04  // Correctable data error detected
#define STATUS_DRQ      0x08  // Device ready to transfer data
#define STATUS_COMPLETE 0x10  // Set when seek is complete, if error doesn't change until read
#define STATUS_WRFAULT  0x20  // Device Fault Error detected
#define STATUS_DRDY     0x40  // Set rdy for cmd.
#define STATUS_BUSY     0x80  // Set Device is busy, all registers locked and status undetermined


/* IDE Commands */
#define WIN_IDENTIFY  0xEC  /* Ask drive to identify itself */

/* Additional commands used by ATAPI devices */
#define WIN_PIDENTIFY 0xA1  /* identify ATAPI device */

#define CMD_SEEK    0x70
#define CMD_RECAL   0x10
#define CMD_EJECT   0xED
#define CMD_VERIFYR 0x40    // With retries
#define CMD_VERIFY  0x41    // No retries
#define CMD_DIAG    0x90    // Execute device diagnostics


/* Macro's for status */
#define GET_STAT()   inp(IDE_STATUS_REG)
#define OK_STAT(stat, good, bad)  (((stat)&((good) | (bad))) == (good))
#define BAD_R_STAT    (STATUS_BUSY | STATUS_ERROR)

/* Defines for Master and Slave */
#define DMASTER 0
#define DSLAVE  1
#define SMASTER 0xA0
#define SSLAVE  0xB0
#define PORTPRIMARY   0x1F0
#define PORTSECONDARY 0x170 

class cIDE {
public:   /* Initalization functions */
	cIDE();
   ~cIDE();
   void SetBase(WORD addr);
   void InitDrives();
   void IdeFixString(BYTE *s, const int bytecount, const int byteswap);
   void FixStrings(struct ideinfo_data *id);    // Handles all strings
public:  /* Accessor functions */
   char *Model(BYTE drive);
   char *Firmware(BYTE drive);
   char *Serial(BYTE drive);
   bool IsMaster();
   bool IsSlave();
   bool IsDrive(BYTE drive);
   bool IsHD(BYTE drive);
   long Capacity(BYTE drive);
   long CapacityCHS(BYTE drive);
   long CapacityLBA(BYTE drive);
   char *ConfigType(BYTE drive);
   char *CHSString(BYTE drive);
public:
   int  SectorPerTrack(BYTE drive);
   long MaxLastSector(BYTE drive);
private: /* Utility Functions */
   bool WaitBusy();
   bool WaitDRQ();
   bool CheckStat();
   void FindStatusReg();

public: /* Interface Functions */
   void PIORead(WORD *szBuffer, int iWords);
   bool Command(BYTE cmd);
   bool CommandFast(BYTE cmd);
   bool Select(BYTE drive);
   bool Seek(BYTE dev, DWORD sector);                           // LBA seek
   bool Seek(BYTE dev, WORD cylinder, BYTE head, BYTE sector);  // CHS seek
   bool Verify(BYTE dev, DWORD sector, bool retry);                           // LBA verify
   bool Verify(BYTE dev, WORD cylinder, BYTE head, BYTE sector, bool retry);  // CHS verify
   bool Recal(BYTE dev);
   bool Eject(BYTE dev);
   BYTE ExeDiag();

private:
	WORD base;       // Base Address
    WORD rStatus;    // Status Register
public:
    struct ideinfo_data sMaster;    
    struct ideinfo_data sSlave;
    BYTE bMaster;    // Does master exist
	BYTE bSlave;     // Does slave exist
	
};




#endif	