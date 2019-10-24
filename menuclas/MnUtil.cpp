#include <stdio.h>
#include <conio.h>

#include "declare/global.h"
#include "menuclas/MnUtil.h"
#include "menuclas/MnDmi.h"


/* Utils submenu, we are not allowed to write to y = 1, that is
   reserved for the toolbar. We don't have to clean up after ourself.
   Just return when we are done. Free any objects we spawned.      */
void cMnUtil::Main() {
#define MAX_SI 0
	struct menu_data { int start; int length; } menustruct[] = {
		{ 3, 10 } };
	int exitflag = FALSE;
	char c;
 
    selected = 0;  

   /* Main keyloop */
   while (!exitflag) {
    gotoxy(1, 2);
	cprintf("  DMI Viewer");
    HighLight(1, 2, 80, C_SI_TOOLBAR);
    HLIGHT(2, C_SI_HL);


	   c = Input();
	   switch (c) 
	   {
	      case ESCAPE: 
			  exitflag = TRUE; break;
          case KEY_LEFT:
		  case KEY_4:
			  if (selected > 0) {
                 HLIGHT(2, C_SI_ITEM);
				 --selected;
                 HLIGHT(2, C_SI_HL);
			  }
			  break;
          case KEY_RIGHT:
		  case KEY_6:
			  if (selected < MAX_SI) {
                 HLIGHT(2, C_SI_ITEM);
				 ++selected;
                 HLIGHT(2, C_SI_HL);
			  }
			  break;
		  case ENTER:
			  SpawnMenu();
			  break;
	   } /* switch() */
   } /* while() */

   /* We're about to bomb out, clean up after ourself */
   /* ... */

}

void cMnUtil::SpawnMenu() {
    int x, y;
	switch (selected) {
	   case 0:       /* DMI Viewer */
		   cMnDMI *oMnDMI = new cMnDMI;
		   oMnDMI->Main();
		   delete oMnDMI;
		   break;
	}

   PaintBG();
}

