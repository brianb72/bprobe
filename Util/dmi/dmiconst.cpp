/* Constants for the dmi viewer */


const char *nonestring[] = {"   "};

#define MAX_DMI_NAME 40
const char *dmi_name[] = {
"BIOS Information",                 /* 0 */
"System Information",               /* 1 */
"Base Board Information",           /* 2 */
"System Enclosure",                 /* 3 */
"Processor Information",            /* 4 */
"Memory Controller Information",    /* 5 */
"Memory Module Information",        /* 6 */
"Cache Information",                /* 7 */
"Port Connector Information",       /* 8 */
"System Slots",                     /* 9 */
"On Board Devices Information",     /* 10 */
"OEM Strings",						/* 11 */
"System Configuration Options",     /* 12 */
"Bios Language Information",        /* 13 */
"Group Associations",               /* 14 */
"System Event Log",                 /* 15 */
"Physical Memory Array",            /* 16 */
"Memory Device",                    /* 17 */
"32-bit Memory Error Information",  /* 18 */
"Memory Array Mapped Address",      /* 19 */
"Memory Device Mapped Address",     /* 20 */
"Built-in Pointing Device",         /* 21 */
"Portable Battery",                 /* 22 */
"System Reset", 				    /* 23 */
"Hardware Security",                /* 24 */
"System Power Controls",            /* 25 */
"Voltage Probe",                    /* 26 */
"Cooling Device",                   /* 27 */
"Temperature Probe",                /* 28 */
"Electrical Current Probe",         /* 29 */
"Out-of-Band Remote Access",        /* 30 */
"Boot Integrity Services Entry Point",  /* 31 */
"System Boot Information",         /* 32 */
"64-bit Memory Error Information", /* 33 */
"Management Device",               /* 34 */
"Management Device Component",     /* 35 */
"Management Device Threshold Data",/* 36 */
"Memory Channel",                  /* 37 */
"IPMI Device Information",         /* 38 */
"System Power Supply",             /* 39 */
};

#define MAX_DMI_BIOS_CHAR 32
const char *dmi_bios_char[] = {
"Reserved",  /* 0 */
"Reserved",  /* 1 */
"Unknown",  /* 2 */
"Not Supported",  /* 3 */
"ISA",  /* 4 */
"MCA",  /* 5 */
"EISA",  /* 6 */
"PCI",  /* 7 */
"PCMCIA",  /* 8 */
"PNP",  /* 9 */
"APM",  /* 10 */
"Flash Bios",  /* 11 */
"Shadowing",  /* 12 */
"VL-VESA",  /* 13 */
"ESCD",  /* 14 */
"Boot from CDROM",  /* 15 */
"Selectable Boot",  /* 16 */
"ROM Socketed",  /* 17 */
"Boot from PCMCIA",  /* 18 */
"EDD",  /* 19 */
"NEC 1.2mb Flop",  /* 20 */
"Toshiba 1.2mb Flop",  /* 21 */
"360kb Flop",  /* 22 */
"1.2mb Flop",  /* 23 */
"720kb Flop",  /* 24 */
"2.88mb Flop",  /* 25 */
"PrintScrn",  /* 26 */
"8042 Keyboard",  /* 27 */
"Serial",  /* 28 */
"Printer",  /* 29 */
"CGA/Mono",  /* 30 */
"NEC PC-98",  /* 31 */
};

#define MAX_ENC_NAME 25
const char *dmi_encname[] = {
"Undefined",
"Other", /* 1 */
"Unknown", /* 2 */
"Desktop", /* 3 */
"Low Profile Desktop", /* 4 */
"Pizza Box", /* 5 */
"Mini Tower", /* 6 */
"Tower", /* 7 */
"Portable", /* 8 */
"LapTop", /* 9 */
"Notebook", /* 10 */
"Hand Held", /* 11 */
"Docking Station", /* 12 */
"All in One", /* 13 */
"Sub Notebook", /* 14 */
"Space-saving", /* 15 */
"Lunch Box", /* 16 */
"Main Server Chassis", /* 17 */
"Expansion Chassis", /* 18 */
"SubChassis", /* 19 */
"Bus Expansion Chassis", /* 20 */
"Peripheral Chassis", /* 21 */
"RAID Chassis", /* 22 */
"Rack Mount Chassis", /* 23 */
"Sealed-case PC" /* 24 */
};

#define MAX_MEM_NAMES 11
const char *dmi_memname[] = {
"Other",           // 0
"Unknown",         // 1
"Standard",        // 2
"Fast Page Mode",  // 3
"EDO",             // 4
"Parity",          // 5
"ECC",             // 6
"SIMM",            // 7
"DIMM",            // 8
"Burst EDO",       // 9
"SDRAM"            // 10
};

#define MAX_SRAM_NAMES 7
const char *dmi_sramname[] = {
"Other",
"Unknown",
"Non-Burst",
"Burst",
"Pipeline Burst",
"Synchronous",
"Asynchronous"
};


#define MAX_PORTCON_NAMES 34
const char *dmi_portcon[] = {
"None",               // 0
"Centronics",         // 1
"Mini Centronics",    // 2
"Proprietary",        // 3
"DB-25 pin male",     // 4
"DB-25 pin female",   // 5
"DB-15 pin male",     // 6
"DB-15 pin female",   // 7
"DB-9 pin male",      // 8
"DB-9 pin female",    // 9
"RJ-11",              // 10
"RJ-45",              // 11
"50 Pin MiniSCSI",    // 12
"Mini-DIN",           // 13
"Micro-DIN",          // 14
"PS/2",               // 15
"Infrared",           // 16
"HP-HIL",             // 17
"Access Bus (USB)",   // 18
"SCA SCSI",           // 19
"Circular DIN-8 male",// 20
"Circular DIN-8 female",  // 21
"On Board IDE",           // 22
"On Board Floppy",        // 23
"9 pin Dual Inline",      // 24
"25 pin Dual Inline",     // 25
"50 pin Dual Inline",     // 26
"68 pin Dual Inline",     // 27
"Sound Input from CD-ROM",// 28
"Mini-Centronics Type-14",// 29
"Mini-Centronics Type-26",// 30
"Mini-jack",              // 31
"BNC",                    // 32
"1394"                    // 33
};


#define MAX_SLOT_TYPE_NAMES 19
const char *dmi_slot_type[] = {
"Undefined",
"Other",                          /* 1 */
"Unknown",                        /* 2 */
"ISA",                            /* 3 */
"MCA",                            /* 4 */
"EISA",                           /* 5 */
"PCI",                            /* 6 */
"PC Card(PCMCIA)",                /* 7 */
"VL-VESA",                        /* 8 */
"Proprietary",                    /* 9 */
"Processor Card Slot",            /* 10 */
"Proprietary Memory Card Slot",   /* 11 */
"I/O Riser Card Slot",            /* 12 */
"NuBus",                          /* 13 */
"PCI - 66MHz Capable",            /* 14 */
"AGP",                            /* 15 */
"AGP 2X",                         /* 16 */
"AGP 4X",                         /* 17 */
"PCI-X"                           /* 18 */
};

#define MAX_SLOT_WIDTH_NAMES 8
const char *dmi_slot_width[] = {
"Undefined",
"Other",
"Unknown",
"8-bit",
"16-bit",
"32-bit",
"64-bit",
"128-bit"
};

#define MAX_SLOT_USAGE_NAMES 5
const char *dmi_slot_usage[] = {
"Undefined",
"Other", "Unknown", "Available", "In use"
};

#define MAX_SLOT_LENGTH_NAMES 5
const char *dmi_slot_length[] = {
"Undefined",
"Other", "Unknown", "Short Length", "Full Length"
};

#define MAX_SLOT_CHAR1 8
const char *dmi_slot_char1[] = {
"Unknown", "5.0v", "3.3v", "Shared", "PC Card-16", "Cardbus",
"Zoom Video", "Modem Ring Resume"
};

#define MAX_SLOT_CHAR2 3
const char *dmi_slot_char2[] = {
"PME# Signal", "Hot-plug", "SMBus Signals"
};

#define MAX_MEM_LOC 11
const char *dmi_mem_loc[] = {
"Undefined", "Other", "Unknown", "System Board", "ISA add-on card",
"EISA add-on card", "PCI add-on card", "MCA add-on card",
"PCMCIA add-on card", "Proprietary add-on card", "Nubus"};

#define MAX_MEM_USE 8
const char *dmi_mem_use[] = {
"Undefined", "Other", "Unknown", "System memory", "Video memory",
"Flash memory", "Non-volatile RAM", "Cache memory"};

#define MAX_MEM_ERROR 8
const char *dmi_mem_error[] = {
"Undefined", "Other", "Unknown", "None", "Parity", "Single-bit ECC",
"Multi-bit ECC", "CRC"};

#define MAX_MEM_FORM 15
const char *dmi_mem_form[] = {
"Undefined", "Other", "Unknown", "SIMM", "SIP", "Chip", "DIP", "ZIP",
"Proprietary Card", "DIMM", "TSOP", "Row of chips", "RIMM", "SODIMM",
"SRIMM" };

#define MAX_MEM_TYPE 18
const char *dmi_mem_type[] = {
"Undefined", "Other", "Unknown", "DRAM", "EDRAM", "VRAM", "SRAM", "RAM",
"ROM", "FLASH", "EEPROM", "FEPROM", "EPROM", "CDRAM", "3DRAM", "SDRAM",
"SGRAM", "RDRAM" };

#define MAX_MEM_DETAIL 13
const char *dmi_mem_detail[] = {
" ", "Other", "Unknown", "Fast-paged", "Static column", "Pseudo-static",
"RAMBUS", "Synchronous", "CMOS", "EDO", "Window DRAM", "Cache DRAM",
"Non-volatile"};

#define MAX_ERR_TYPE 15
const char *dmi_err_type[] = {
" ", "Other", "Unknown", "OK", "Bad read", "Parity error", "Single-bit error",
"Double-bit error", "Multi-bit error", "Nibble error", "Checksum error",
"CRC error", "Corrected single-bit error", "Corrected error", "Uncorrectable error"};

#define MAX_ERR_GRAN 5
const char *dmi_err_gran[] = {
" ", "Other", "Unknown", "Device level", "Memory partition level" };

#define MAX_ERR_OP 6
const char *dmi_err_op[] = {
" ", "Other", "Unknown", "Read", "Write", "Partial Write" };