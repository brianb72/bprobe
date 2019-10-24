/* Global include file, shared to all files */
#ifndef __LOCGLOBAL_H__
#define __LOCGLOBAL_H__

/* Global Strings */
#define VERSION "v2.1b (1005)"

/* Include any global declares */
#include "declare/colors.h"

/* Global utility macros */

/* Highlights selected item with color, menustruct in scope */
#define HLIGHT(row, col) HighLight(menustruct[selected].start, row, menustruct[selected].length, col)


/* Values passed to menus */
#define MENULEFT      0x80     // The menu can go left
#define MENURIGHT     0x01     // The menu can go right
#define MENUBOTH      0x81
#define MENU_NOREDRAW 0x40
#include "menuclas/mnmain.h"

extern int g_delay_method;

#endif
