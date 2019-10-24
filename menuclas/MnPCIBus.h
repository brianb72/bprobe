#ifndef __MNPCIBUS_H__
#define __MNPCIBUS_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include "diag/pci/pciclass.h"

class cMnPCIBus : cTextVideo, cKeyInput, PciClass {
public:
   cMnPCIBus();
   ~cMnPCIBus();
   void Main(int menudir);
   void DrawAscii();
   void DrawNames();
   int pci_maxdev() { return maxdev; }
   struct pcidev_data *pci_device(int i) { return &device[i]; }

   char VendorName[20][70], DeviceName[20][70];
   int namedone;
   int scanned;
   int wy;

   int pci_classname(word classid, word subclassid, char *classname, char *subclassname) {
		return get_pci_classname(classid, subclassid, classname, subclassname); }
};


#endif