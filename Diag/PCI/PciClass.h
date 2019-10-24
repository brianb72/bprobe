/*
 
*/

#ifndef __pciclass_h__
#define __pciclass_h__
/*
#define REG_VENDORID  0
#define REG_DEVICEID  2
#define REG_COMMAND   4
#define REG_STATUS    6
#define REG_SUBCLASS  10
#define REG_CLASS     11
*/

#include <Global.h>

#define REG_VENDORID  0
#define REG_DEVICEID  2
#define REG_COMMAND   4
#define REG_STATUS    6
#define REG_SUBCLASS  10
#define REG_CLASS     11


/* For PCI device information */
#define PCICHKVALUE    0x5A5A   // Value at end of each vendor/dev
#define PCITABLEVALUE  0xA5A5   // Value at begining of table
#define PCIVERSION     0x2001

/* For pci.dat filestatus */
#define PCIDATOK       1   // File is ok
#define PCIDATINVALID -1   // File is invalid
#define PCIDATFAIL    -2   // File could nto be opened

struct pci_vendor_data {
   unsigned int vid;
   char name[55];
   unsigned int chk;
};

struct pci_device_data {
   unsigned int vid;
   unsigned int did;
   char name[65];
   unsigned int chk;
};



struct pcidev_data {
   int bus, dev, func;
   union {
	struct {
	USHORT  VendorID;                   // (ro)
    USHORT  DeviceID;                   // (ro)
    USHORT  Command;                    // Device control
    USHORT  Status;
    UCHAR   RevisionID;                 // (ro)
	UCHAR   ProgIf;                     // (ro)
    UCHAR   SubClass;                   // (ro)
    UCHAR   BaseClass;                  // (ro)
    } reg;
	ULONG  dw[3];
   };
};

class PciClass {
   public:
	  PciClass();
	  ~PciClass();
      ULONG  ReadConfigDWORD(UCHAR bBus, UCHAR bDevice, UCHAR bFunc,  UCHAR bDWord);
      USHORT ReadConfigWORD(UCHAR bBus, UCHAR bDevice, UCHAR bFunc,  UCHAR bWord);
      UCHAR  ReadConfigBYTE(UCHAR bBus, UCHAR bDevice, UCHAR bFunc,  UCHAR bByte);
	  int ScanPciBus();
	  int FindDev(USHORT VendorID, USHORT DeviceID, UCHAR *bus, UCHAR *dev, UCHAR *func);
      void outpd(int portnum, dword val);
	  dword PciClass::inpd(int portnum);
	  int get_pci_name(char venname[][70], char devname[][70], int wy);
	  int get_pci_classname(word classid, word subclassid, char *classname, char *subclassname);
	  int open_pcidat();
   public:
	  int pcidatvalid;
	  struct pcidev_data device[30];
	  int maxdev;
	  FILE *fp;
};

#endif
