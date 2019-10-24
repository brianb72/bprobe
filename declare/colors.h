/*
   Color defines for every menu. Each color is a foreground and
   background combination.      
*/


#ifndef COLOR
#define COLOR(fg, bg)           ((fg) + ((bg) << 4))
#endif

#define C_BACK(col)             (col >> 4)
#define C_FORE(col)             (col & 0xF)


/* Global colors */
#define C_BGROUND     COLOR(BLACK, LIGHTGRAY)

/* Main Menu */
#define C_MM_TOOLBAR  COLOR(WHITE,  BLUE)    // Toolbar color 
#define C_MM_ITEM     COLOR(WHITE, BLUE)   // Item color
#define C_MM_HL       COLOR(BLUE, LIGHTGRAY)   // Highlighted item

#define C_TOOLBAR     COLOR(WHITE, BLUE)
/* Sysinfo */
#define C_SI_TOOLBAR  COLOR(WHITE,  BLUE)    // Toolbar color 
#define C_SI_ITEM     COLOR(WHITE, BLUE)   // Item color
#define C_SI_HL       COLOR(BLUE, WHITE)   // Highlighted item

/* Overview menu */
#define C_OV_GUIDE    COLOR(BLUE, LIGHTGRAY)
#define C_OV_DATA     COLOR(WHITE, BLACK)
#define C_OV_HEADER   COLOR(WHITE, BLUE)

/* PCI */
#define C_PC_GUIDE    COLOR(BLUE, LIGHTGRAY)
#define C_PC_DATA     COLOR(WHITE, BLACK)
#define C_PC_HL       COLOR(BLACK, GREEN)

/* DMI Viewer */
#define C_DM_HEADER     COLOR(WHITE, BLUE)   /* Header across top */
#define C_DM_GUIDE		COLOR(BLUE, LIGHTGRAY)   /* Location bar */
#define C_DM_LINE       COLOR(LIGHTBLUE, BLACK)  /* Any Line Art */
#define C_DM_SHADOW		COLOR(BLACK, LIGHTGRAY)  /* Fore is color of shadows */
#define C_DM_DATA		COLOR(WHITE, BLUE)  /* Data color */

/* IDE Drive diag */
#define C_ID_LINE       COLOR(LIGHTBLUE, BLACK)
#define C_ID_HL         COLOR(WHITE, GREEN)
#define C_ID_DATA       COLOR(WHITE, BLACK)

/* Logical Drives */
#define C_LO_LINE       COLOR(LIGHTBLUE, BLACK)
#define C_LO_HL         COLOR(WHITE, GREEN)
#define C_LO_DATA       COLOR(WHITE, BLACK)


