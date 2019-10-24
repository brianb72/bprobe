#ifndef __MNMEMTST_H__
#define __MNMEMTST_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include "misc/xms.h"


class cMnMemTst : cTextVideo, cKeyInput, cXMS {
public:
	cMnMemTst();
	~cMnMemTst();
	void Main();
	void DrawAscii();
	void Draw();
	void ReleaseAll();
	void FreeTst();
	void FillTst();
public:
	int GetMemBlocks();
	int TestXMS(int rep);
	int TestXMSBlock(int tblock, unsigned long *pattern);
public:
	struct XMS_EMBInfo emb[50];
	int NumEmb;
	unsigned long *tstbuf1;
	unsigned long *tstbuf2;
	unsigned long tbuf[16000];
	unsigned int max_hnd;   	// max amount of handles
	unsigned long max_mem;	    // max amount of mem in K
	unsigned long max_block;	// Max block size in K
	unsigned long block_size;    // Calculated block size
};

#endif
