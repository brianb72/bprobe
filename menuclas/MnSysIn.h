#ifndef __MNSYSIN_H__
#define __MHSYSIN_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"

class cMnSysInfo : cTextVideo, cKeyInput {
public:   
	void Main();
	void SpawnMenu();
private:
	int selected;
};

#endif
