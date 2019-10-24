#ifndef __MNIDE_H__
#define __MNIDE_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include "diag/ide/ataide.h"


#define TEST_SEEK   0 
#define TEST_VERIFY 1

#define X_STATUS    14
#define Y_STATUS    16
#define X_MSG       14
#define Y_MSG       17

class cMnIDE : cTextVideo, cKeyInput {
public:
	cMnIDE();
	~cMnIDE();
	void Main();
	void DrawAscii();
	void Draw();
	void DisplayDrv(); 
	void SetDrvNum();
	void HLDrive(int state);
	void HLMenu(int state);
	void ClrMsg();
	void ClrStatus();
 
public:
	int SeekTest(int type);
	int TestFunnel(int type);
	int TestRandom(int type);
	int TestLinear(int type);
	int Diag();
	int Reset();
	int Eject();
public:
	int dselect;    // What menu drive we are on
    int ionum;      // What io num we're on
	int drvnum;     // What drive we're on 
	int mnum;       // What diag menu we're on 
    cIDE *io[2];    // Interfaces


};

#endif
