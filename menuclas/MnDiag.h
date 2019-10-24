#ifndef __MNDIAG_H__
#define __MNDIAG_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"

class cMnDiag : cTextVideo, cKeyInput {
public:   
	void Main();
	void SpawnMenu();
private:
	int selected;
};

#endif