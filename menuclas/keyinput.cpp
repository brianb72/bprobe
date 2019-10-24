#include <stdio.h>
#include <conio.h>

#include <Global.h>
#include "declare/global.h"
#include "menuclas/keyinput.h"


/* 
   Very simple, return the key pressed or a value listed in 
   keys.h
*/

char cKeyInput::Input() {
   char c;
   c = getch();
   if (c == KEY_NULL) {
      c = getch();   
	  c |= 0x80;
	  return c;
   }
   return c;
}

