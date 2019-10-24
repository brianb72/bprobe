#ifndef __MNLOG_H__
#define __MNLOG_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include "diag/logdrv/logdrv.h"

class cMnLog : cTextVideo, cKeyInput, cLogDrv {
public:
   void Main();
   void Draw();
   void DrawAscii();
   void DrawPartition(struct partition_data *p);
   void HLDrive(int state);

public:
	int curdrv;			// The currently selected bios drive
};


#endif