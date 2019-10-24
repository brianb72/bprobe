struct pci_subclass_data {
   int subclassid;
   char name[30];
};

struct pci_class_data {
	int classid;
	char name[30];
	struct pci_subclass_data *subclass;
};


struct pci_subclass_data subclass_info_01[] =
   {
	 { 0x00, "SCSI" },
	 { 0x01, "IDE" },
	 { 0x02, "floppy" },
	 { 0x03, "IPI"},
	 { 0x04, "RAID" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_02[] =
   {
	 { 0x00, "Ethernet" },
	 { 0x01, "TokenRing" },
	 { 0x02, "FDDI" },
	 { 0x03, "ATM" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_03[] =
   {
	 { 0x00, "VGA" },
	 { 0x01, "SuperVGA" },
	 { 0x02, "XGA" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_04[] =
   {
	 { 0x00, "video" },
	 { 0x01, "audio" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_05[] =
   {
	 { 0x00, "RAM" },
	 { 0x01, "FlashMemory" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_06[] =
   {
	 { 0x00, "CPU/PCI" },
	 { 0x01, "PCI/ISA" },
	 { 0x02, "PCI/EISA" },
	 { 0x03, "PCI/MCA" },
	 { 0x04, "PCI/PCI" },
	 { 0x05, "PCI/PCMCIA" },
	 { 0x06, "PCI/NuBus" },
	 { 0x07, "PCI/CardBus" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_07[] =
   {
	 { 0x00, "serial" },
	 { 0x01, "parallel" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_08[] =
   {
	 { 0x00, "PIC" },
	 { 0x01, "DMAC" },
	 { 0x02, "timer" },
	 { 0x03, "RTC" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_09[] =
   {
	 { 0x00, "keyboard" },
	 { 0x01, "digitizer" },
	 { 0x02, "mouse" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_0A[] =
   {
	 { 0x00, "generic" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_0B[] =
   {
	 { 0x00, "386" },
	 { 0x01, "486" },
	 { 0x02, "Pentium" },
	 { 0x03, "P6" },
	 { 0x10, "Alpha" },
	 { 0x40, "coproc" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;

struct pci_subclass_data subclass_info_0C[] =
   {
	 { 0x00, "Firewire" },
	 { 0x01, "ACCESS.bus" },
	 { 0x02, "SSA" },
	 { 0x03, "USB" },
	 { 0x04, "Fiber Chan" },
	 { 0x80, "other" },
	 { -1, 0 },
   } ;


struct pci_class_data pci_class[] =
{
  {	0x0, "Reserved",           0                 },
  {	0x1, "Disk",               subclass_info_01  },
  {	0x2, "Network",            subclass_info_02  },
  {	0x3, "Display",            subclass_info_03  },
  {	0x4, "Multimedia",         subclass_info_04  },
  {	0x5, "Memory",             subclass_info_05  },
  {	0x6, "Bridge",             subclass_info_06  },
  {	0x7, "Comm",               subclass_info_07  },
  {	0x8, "Peripheral",         subclass_info_08  },
  {	0x9, "Input",              subclass_info_09  },
  {	0xA, "Dckstation",	       subclass_info_0A  },
  {	0xB, "CPU",		           subclass_info_0B  },
  {	0xC, "Serial Bus",	       subclass_info_0C  },
  { 0x0, 0, 0 }    // We cheat since we skip element 0
};
