#include <stdio.h>
#include <conio.h>
#include <alloc.h>
#include <string.h>
#include <dos.h>
#include "declare/global.h"

#include "diag/pci/PciClass.h"
#include "diag/pci/class.h"





PciClass::PciClass() {
   unsigned int t = 0;
   maxdev = 0;
   if ((fp = fopen("pci.dat", "r+b")) == NULL) {
	  pcidatvalid = PCIDATFAIL;
	  LOG("Failed to open pci.dat.");
	  return;  // Make sure to do any other constructor code before
			   // this or we'll never get by this if the PCI.DAT is missing
   }
   else pcidatvalid = PCIDATOK;

   fread(&t, sizeof(t), 1, fp);  // the file version
//   LOG(" ");
   LOG("Opened PCI.DAT version %Xh", t);
//   LOG(" ");
   fread(&t, sizeof(t), 1, fp);  // The CHK
   if (t != PCITABLEVALUE) {
	  LOG("PCI.DAT has Invalid first check!");
	  pcidatvalid = PCIDATINVALID;
   }
}

PciClass::~PciClass() {
   if (fp != NULL) fclose(fp);
}


/* outputs a doubleword to a port */
void PciClass::outpd(int portnum, dword val) {
   static dword value = 0;

   value = val;
   asm mov dx, portnum;
   asm lea bx, value;
   __emit__(0x66, 0x50,            // push EAX
			0x66, 0x8B, 0x07,      // mov EAX, [BX]
			0x66, 0xEF,            // out DX, EAX
			0x66, 0x58);           // pop EAX
   return;
}

/* Inputs a doubleword from a port */
dword PciClass::inpd(int portnum) {
   static dword value = 0;

   asm mov dx, portnum;
   asm lea bx, value;
   __emit__(0x66, 0x50,            // push EAX
			0x66, 0xED,            // in EAX, DX
			0x66, 0x89, 0x07,      // mov [BX], EAX
			0x66, 0x58);           // POP EAX
   return value;
}


/* To get around a Borland problem of shifting over 15, 
   multiplying by 65536 is the same as shifting left by 16   */
ULONG PciClass::ReadConfigDWORD(UCHAR bBus, UCHAR bDevice, UCHAR bFunc,  UCHAR bDWord)
{
	ULONG addr;
	UCHAR bRegister;
	
	bRegister = bDWord;
	addr = 0x80000000 | ((bBus & 0xff) * 65536) | ((bDevice & 0x1F) << 11)
		  | ((bFunc & 0x7) << 8) | (bRegister & 0xFC);
	outpd(0xCF8, addr);
	return(inpd(0xCFC));
}

/* Break memory up into 510 words, bRegister specifies which word we want 
   Words 0-1 are dword 0, 2-3 are 4, 4-5 are 8, 6-7 are 12.. ... */
USHORT PciClass::ReadConfigWORD(UCHAR bBus, UCHAR bDevice, UCHAR bFunc,  UCHAR bWord)
{
	ULONG addr;
    UCHAR bRegister;
	
	bRegister = bWord * 2;
	addr = 0x80000000 | ((bBus & 0xff) * 65536) | ((bDevice & 0x1F) << 11) 
		  | ((bFunc & 0x7) << 8) | (bRegister & 0xFC);
	outpd(0xCF8, addr);

	if ((bWord + 1) % 2) 
	   return (inpw(0xCFC));
	else
	   return (inpw(0xCFE));
}

/* Bytes 0 1 2 3 are dword 0,   4 5 6 7 are 4,   8 9 10 11 are 8 */
UCHAR  PciClass::ReadConfigBYTE(UCHAR bBus, UCHAR bDevice, UCHAR bFunc,  UCHAR bByte)
{
	ULONG addr = 0;
	UCHAR bRegister = bByte;
	addr = 0x80000000 | ((bBus & 0xff) * 65536) | ((bDevice & 0x1F) << 11) 
		  | ((bFunc & 0x7) << 8) | (bRegister & 0xFC);
	outpd(0xCF8, addr);

	if (!(bByte % 4)) 
		return (inp(0xCFC));
	if ( (bByte > 0) && !((bByte - 1) % 4) )
		return (inp(0xCFD));
	if ( (bByte > 1) && !((bByte - 2) % 4) )
		return (inp(0xCFE));
	if ( (bByte > 2) && !((bByte - 3) % 4) )  
		return (inp(0xCFF));

	return 0;
}


int PciClass::ScanPciBus() {
   int bus, dev, func;
   char buf[100];
   
   maxdev = 0;

   LOGLO("Begining PCI Bus Scan");
   LOGLO("---------------------");

   for (bus = 0; bus < 2; ++bus) {
	 for (dev = 0; dev < 32; ++dev) {
	   for (func = 0; func < 8; ++func) {
		  device[maxdev].dw[0] = ReadConfigDWORD(bus, dev, func, 0);
		  if (device[maxdev].reg.VendorID == 0xFFFF || 
			  device[maxdev].reg.DeviceID == 0xFFFF ||
			  device[maxdev].reg.VendorID == 0 ||
			  device[maxdev].reg.DeviceID == 0)  {
			  continue;  
          }

		  device[maxdev].dw[1] = ReadConfigDWORD(bus, dev, func, 4);
          device[maxdev].dw[2] = ReadConfigDWORD(bus, dev, func, 8);
		  device[maxdev].bus = bus;
		  device[maxdev].dev = dev;
		  device[maxdev].func = func;
		  LOGLO("Found device b: %i d: %i f: %i  %4Xh %4Xh", 
			  bus, dev, func, device[maxdev].reg.VendorID,
			  device[maxdev].reg.DeviceID);
          if (maxdev > 0) {   // Skip Multifunction devices
             if ( (device[maxdev - 1].reg.VendorID == device[maxdev].reg.VendorID) &&          
                  (device[maxdev - 1].reg.DeviceID == device[maxdev].reg.DeviceID) &&          
				  (device[maxdev - 1].bus == device[maxdev].bus) &&
				  (device[maxdev - 1].dev == device[maxdev].dev)) 
				  continue;
		  }
		  ++maxdev;
	   }
	 }
   }
   LOGLO("Found %i devices.", maxdev);

   return TRUE;
}


/* returns 00BBDDFF bus dev function or 0 if nothing found */
int PciClass::FindDev(USHORT VendorID, USHORT DeviceID, UCHAR *bus, UCHAR *dev, UCHAR *func) {
   int i;
   for (i = 0; i < maxdev; ++i) {
	  if (device[i].reg.VendorID == VendorID &&
		  device[i].reg.DeviceID == DeviceID) {
		 *bus = device[i].bus;
		 *dev = device[i].dev;
		 *func = device[i].func;
		 return TRUE;
	  }
   }
   return FALSE;
}


/* Returns a PCI device name given a vendorid and a deviceid */
/* In order to avoid any fseek problems, only use rewind and fread() to
   advance the pointer */

int PciClass::get_pci_name(char venname[][70], char devname[][70], int wy) {
   unsigned int num_ven = 0, num_dev = 0;
   unsigned int t;
   int numleft, entry, i;
   struct pci_vendor_data vdata;
   struct pci_device_data ddata;

   if (maxdev == 0) return 1;  // no devs

   if (wy < 25) {
   gotoxy(1, wy);
   delline();
   gotoxy(1, wy);
   cprintf("Loading ... ");

   // NOTE : oThrob doesn't recognize windows. We must add window offsets
   oThrob->SetPos(wherex() + 3, wherey() + 6);
   }
   /* First, prep the list by setting all entries to their hex */
   for (i = 0; i < maxdev; ++i) {
	  sprintf(venname[i], "%.4Xh", device[i].reg.VendorID);
	  sprintf(devname[i], "%.4Xh", device[i].reg.DeviceID);
   }

   /* Check if the file was previously invalid */
   if (pcidatvalid != PCIDATOK) {
	  sprintf(venname[0], "%.4Xh", device[i].reg.VendorID);
	  if (pcidatvalid == PCIDATINVALID)
		 sprintf(devname[0], "%.4Xh PCI.DAT Corrupt.", device[0].reg.DeviceID);
	  if (pcidatvalid == PCIDATFAIL)
		 sprintf(devname[0], "%.4Xh PCI.DAT Missing", device[0].reg.DeviceID);
	  return FALSE;
   }

   /* First seek to the number of vendors */
   fseek(fp, 4, SEEK_SET);
   fread(&num_ven, sizeof(num_ven), 1, fp);
   fread(&t, sizeof(t), 1, fp);   // just junk the sizeof struct

//   LOG("Number of Vendors: %i", num_ven);
   /* Begin searching for the vendor */
//   LOG("Scanning vendor table at %lu", ftell(fp));
   numleft = maxdev;

   for (entry = 0; entry < num_ven; ++entry) {
	  if (wy < 25) oThrob->Step();
	  fread(&vdata, sizeof(vdata), 1, fp);   // read a vendor
	  if (vdata.chk != PCICHKVALUE) {   // if it ever fails, invalidate file
		 sprintf(venname[i], "%.4Xh", device[i].reg.VendorID);
		 sprintf(devname[i], "%.4Xh PCI.DAT Corrupt..", device[i].reg.DeviceID);
		 LOG("PCI.DAT Invalid check while parsing vendor.");
		 pcidatvalid = PCIDATINVALID;
		 return FALSE;
	  }
	  if (vdata.vid == 0xFFFF) break;   // hit the end
	  for (i = 0; i < maxdev; ++i) {  // let run to end in case multiple dev
		 if (vdata.vid == device[i].reg.VendorID) {  // found it
			strncpy(venname[i], vdata.name, 54);
			--numleft;
		 }
		 if (numleft == 0) break;  // break our for()
	  }
	  if (numleft == 0) break;     // then break the big for()
   }

   /* All vendors should be matched up */
   /* Skip to the begining of the device list */
   fseek(fp, (long)8 + ((long)sizeof(vdata) * (long)(num_ven + 0)), SEEK_SET);
//   LOG("Device Header: %lu", ftell(fp));

   fread(&t, sizeof(t), 1, fp);

   if (t != PCITABLEVALUE) {
	  LOG("PCI.DAT has invalid check at begining of device table.");
	  sprintf(devname[0], "%.4Xh PCI.DAT Corrupt...", device[0].reg.VendorID);
	  {
		 int i;
		 for (i = 0; i < 50; ++i) {
		 LOG("%Xh  %c %c", t, t & 0x00FF, t >> 8);
		 fread(&t, sizeof(t), 1, fp);
		 }
	  }
		 pcidatvalid = PCIDATINVALID;
		 return FALSE;
   }

   fread(&num_dev, sizeof(num_dev), 1, fp);
   fread(&t, sizeof(t), 1, fp);   // just junk the sizeof struct
//   LOG("Number of Devices: %i", num_dev);

   /* Now do the device */
//   LOG("Scanning Device Table at %lu", ftell(fp));
   /* Begin searching for the device */
   numleft = maxdev;

   for (entry = 0; entry < num_dev; ++entry) {
	  if (wy < 25) oThrob->Step();
	  fread(&ddata, sizeof(ddata), 1, fp);   // read a vendor
	  if (ddata.chk != PCICHKVALUE) {   // if it ever fails, invalidate file
		 sprintf(venname[i], "%.4Xh", device[i].reg.VendorID);
		 sprintf(devname[i], "%.4Xh PCI.DAT Corrupt....", device[i].reg.DeviceID);
		 LOG("PCI.DAT Invalid check while parsing device.");
		 pcidatvalid = PCIDATINVALID;
		 return FALSE;
	  }
	  if (vdata.vid == 0xFFFF) break;   // hit the end
	  for (i = 0; i < maxdev; ++i) {  // let run to end in case multiple dev
		 if (ddata.vid == device[i].reg.VendorID &&
			 ddata.did == device[i].reg.DeviceID)   {  // found it
			strncpy(devname[i], ddata.name, 64);
			--numleft;
		 }
		 if (numleft == 0) break;  // break our for()
	  }
	  if (numleft == 0) break;     // then break the big for()
   }


   return TRUE;
}






/* Returns a PCI class name given a classid and a subclassid */
int PciClass::get_pci_classname(word classid, word subclassid, char *classname,
					  char *subclassname) {
   int c, s;


   /* Scan the class list and find the class */
   for (c = 0; pci_class[c].classid || !c; ++c) {
	  if (pci_class[c].classid == classid)
		 break;			// We found it
   }

   if (pci_class[c].classid == NULL) { // end of list, we failed
	  sprintf(classname, "%Xh", classid);
	  sprintf(subclassname, "%Xh", subclassid);
	  return FALSE;   /* Not really important currently */
   }

   /* Scan through the device list and find the device */
   for (s = 0; pci_class[c].subclass[s].subclassid != -1; ++s) {
	  if(pci_class[c].subclass[s].subclassid == subclassid)
		 break;			// We found it
   }

   if (pci_class[c].subclass[s].subclassid == -1) { // end of list, we failed
	  sprintf(classname, "%s", pci_class[c].name);
	  sprintf(subclassname, "%Xh", subclassid);
	  return FALSE;
   }

   sprintf(classname, "%s", pci_class[c].name);
   sprintf(subclassname, "%s", pci_class[c].subclass[s].name);

   return TRUE;
}



