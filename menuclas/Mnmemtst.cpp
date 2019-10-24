#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <mem.h>
#include <stdlib.h>


#include "declare/global.h"
#include "menuclas/MnMemTst.h"



cMnMemTst::cMnMemTst() {
	NumEmb = 0;
	tstbuf1 = NULL;
	tstbuf2 = NULL;
}

cMnMemTst::~cMnMemTst() {
   ReleaseAll();
}


void cMnMemTst::Main() {
	int exitflag = FALSE;
    char c = 0;

	if (!Init()) {
       gotoxy(5, 5);
	   printf("Failed to init XMS.");
	   getch();
	   return;
	}
	while (!exitflag) {
		c = Input();
		switch (c) {
		   case ESCAPE: exitflag = TRUE; break;
		   case KEY_UP:
		   case KEY_8:    
			   break;
		   case KEY_DOWN:
		   case KEY_2:   
			   break;
		   case KEY_PGUP:
		   case KEY_9:    
			   break;
		   case KEY_PGDN:
		   case KEY_3: 
			   break;
		   case ENTER: TestXMS(1);
			   break;
		} /* switch () */
	}  /* while () */
}


void cMnMemTst::DrawAscii() {

}

void cMnMemTst::Draw() {

}


/* Grab up as many memory blocks as possible. Split them blocks divisible by 64k, and are
   as small as possible.
   TotalBlocks / (Handles * 64) = page size
   TotalBlocks / 64 = Total64kBlocks
   Total64kBlocks / Handles = 64kBlocksPerHandle
   64kBPH * Handles = KPerHandles
   (TotalBlocks / 64) / Handles * 64 = 64KBPH
*/
int cMnMemTst::GetMemBlocks() {
	struct XMS_FREE s;
	int i;

	s.Largest = 0; s.Total = 0;
	printf("inside get mem blocks");
	/* Allocate 1k block and free it to get handle count */
	emb[0].Length = 1;
	if (AllocEMB(&emb[0]) != 1) return -1;
	max_hnd = FreeHandles - 2;     // Purposfully use 2 less handles
	if (GetFreeXMS(&s)   != 1) return -1;
	max_block = s.Largest;
	if (GetFreeXMS32(&s) != 1) return -1;
	max_mem = s.Total;
	if (FreeEMB(&emb[0]) != 1) return -1;


	if (max_hnd == 0) {
		LOGLO("GetMemBlocks Error - No more handles.");
		return -1;
	}
//     (TotalBlocks / 64) / Handles * 64 = 64KBPH

	block_size = (unsigned long) max_mem / (unsigned long) 64;
	block_size /= (unsigned long) max_hnd;
	block_size *= (unsigned long) 64;

	if (block_size > max_block) {
		LOGLO("GetMemBlocks Error - Block Size %lu greater than Max %lu, adjusting.",block_size, max_mem);
		block_size = max_block;
	}
	LOGLO("Max Handles: %i  Max Memory: %lu  Max Block Size: %lu  Blocks: %lu",
		max_hnd, max_mem, max_block, block_size);

   /* Now allocate each block */
   for (i = 0; i < max_hnd; ++i) {
	  emb[i].hEMB = 0; emb[i].Length = block_size;
	  if (AllocEMB(&emb[i]) != 1) {
		 LOGLO("GetMemBlocks Error - Failure while allocating %i!", i);
		 ReleaseAll();
		 return -1;
	  }
//	  GetFreeXMS32(&s);
//	  LOGLO("Size at %i    Free: %lu", i, s.Total);
	  ++NumEmb;
   }
	return 1;
}


/* Test XMS memory.

   Return: 1 success, 0 mem error, -1 class failure
*/
int cMnMemTst::TestXMS(int rep) {
	int i;
	LOGLO("--- TestXMS ---");

	memset(emb, 0, sizeof(emb));

	tstbuf1 = (unsigned long *) malloc(16000);
	tstbuf2 = (unsigned long *)malloc(16000);
	if (tstbuf1 == NULL || tstbuf2 == NULL) {
	   LOGLO("Failed to malloc buff for TestXMS.");
	   FreeTst();
	   return -1;
	}

	gotoxy(5, 5);
	printf("Testing XMS memory.");
	if (GetMemBlocks() == -1) {
		LOGLO("GetMemBlocks failed.");
		FreeTst();
		return -1;
	}

	FillTst();

	TestXMSBlock(0, tstbuf1);

	gotoxy(5, 5);
	printf("XMS test is done.         ");

	ReleaseAll();
	FreeTst();
	return 1;
}

void cMnMemTst::FillTst() {
   int i;
   for (i = 0; i < 16000; ++i) {
	  tstbuf1[i] = 0xAA55AA55;
	  tstbuf2[i] = 0x55AA55AA;
   }
}

void cMnMemTst::FreeTst() {
	if (tstbuf1 != NULL) { free(tstbuf1); tstbuf1 = NULL; }
	if (tstbuf2 != NULL) { free(tstbuf2); tstbuf2 = NULL; }
}

/* Free all memory we may have allocated */
void cMnMemTst::ReleaseAll() {
	int i;
	for (i = 0; i < NumEmb; ++i) {
		FreeEMB(&emb[i]);
	}
	NumEmb = 0;
}

/* Test the block by writing a 64k pattern, putting it in XMS, fetching it,
   and examining it. Break 64k bytes into 16000 dwords for fastest speed.
*/
int cMnMemTst::TestXMSBlock(int tblock, unsigned long *pattern) {
   int step, i;
   int stepnum  = block_size / 64;  // number of 64k blocks 

   for (step = 0; step < stepnum; ++step) {
	   LOGLO("XMS Test tblock %i dword %i", tblock, i);
		/* First put our pattern into XMS */
	   if (StoreEMB(&emb[tblock], (char *)pattern, step * 64, 64000) != 1) {
		   LOGLO("XMS Test - Failed while storing.");
		   return -1;
	   }
	   /* Now pull the pattern back out */
	   if (FetchEMB(&emb[tblock], (char *)tbuf, step * 64, 64000) != 1) {
		   LOGLO("XMS Test - Failed while Fetching.");
		   return -1;
	   }
	   /* Now test to see if they are equal */
	   for (i = 0; i < 16000; ++i) {
		   if (pattern[i] != tbuf[i]) {
				LOGLO("XMS Test - Error detected block %i dword %i", tblock, i);
				return -1;
		   }
	   }
   }
	return 1;	   		   
}
