#ifndef __MNPHY_H__
#define __MNPHY_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"

class cMnPhy : cTextVideo, cKeyInput {
public:
   void Main();
   void DrawAscii();
};


#endif