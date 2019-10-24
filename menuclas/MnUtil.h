#ifndef __MNUTIL_H__
#define __MNUTIL_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"

class cMnUtil : cTextVideo, cKeyInput {
public:   
	void Main();
	void SpawnMenu();
private:
	int selected;
};

#endif
