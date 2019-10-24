#include <stdio.h>
#include <dos.h>
#include <mem.h>
#include <string.h>
#include <conio.h>

#include "declare/global.h"
#include "misc/xms.h"

/*
	Extended Memory        = Memory above 1MB address boundry
	Extended Memory Blocks = Blocks located 
    High Memory Area       = First 64k of 1mb+ memory  
	Upper Memory Blocks    = Memory between 640k - 1MB
*/

cXMS::cXMS() {
	pHnd = NULL;
}


cXMS::~cXMS() {

}


int cXMS::Init() {
	LOGLO("--- Initing XMS Handler ---");
	if (!CheckXMS()) {
		LOGLO("No XMS Found.");
		return -1;
	}

	SetXMSHnd();
	LOGLO("XMS Handler found at %.4X:%.4X\n", FP_SEG(pHnd), FP_OFF(pHnd));
	LOGLO("XMS Version: %i\n", XMSVersion());
  
   return 1;
}	


/* Check to see if XMS is installed */
int  cXMS::CheckXMS() {
	char treg;
	asm {
		mov ax, 0x4300		// XMS install check
		int 0x2F
		mov treg, al		// Will be 80h if XMS installed
	}
	if (treg == 0x80) return 1;		
	return 0;
}

/* Sets the XMS handle. Must be called before any function except CheckXMS() */
void cXMS::SetXMSHnd() {
	int tseg, toff;
	asm {
		push es				// Preserve es
		mov ax, 0x4310		// Get XMS control address
		int 0x2F
		mov toff, bx
		mov tseg, es
		pop es				// Restore es
	}

	pHnd = (void(*)(void)) MK_FP(tseg, toff);
}



/* Return the XMS version number, which is in BCD. Should be 768 or '3.00' */
int cXMS::XMSVersion() {
	int t;

	asm xor ah, ah
    pHnd();
	asm	mov t, ax
	
	return t;
}


/* Query the status of the A20 line. 
   Return: 1 enabled, 0 not enabled, -1 function not supported
 */
int cXMS::QueryA20() {
	int tax;
	char tbl;
	asm mov ah, 0x7		
	pHnd();
	asm mov tax, ax
	asm mov tbl, bl 

	if (tbl == 0x80) return -1;
	return tax;
}

/* Local request of a20 line enable 
   Return: 1 succeeded, 0 not failed, -1 function not supported, -2 A20 error, 
           -3 A20 still enabled
*/
int cXMS::RequestA20() {
	int tax;
	char tbl;
	asm mov ah, 0x5		
	pHnd();
	asm mov tax, ax
	asm mov tbl, bl

	if (tax == 1) return 1;

	if (tbl == 0x80) return -1;
	if (tbl == 0x82) return -2;
	if (tbl == 0x94) return -3;
	return 0;
}



/* Fills the structure passed with the EMB info data. The caller sets the structures
   handle to the handle of the block in question and calls us. Returns true or false
   if the operation succeeded. If false the structure contents are invalid.
   We also will set the class variable FreeHandles every time this is called.
 */
int cXMS::GetEMBInfo(struct XMS_EMBInfo *s) {
	int tax, tdx;
	char tbl, tbh;
	int tHnd;

	tHnd = s->hEMB;

	asm { mov ah, 0x0E
		 mov dx, tHnd
		}
	pHnd();
	asm { mov tax, ax
		  mov tbl, bl
		  mov tbh, bh
		  mov tdx, dx
	}

	if (tax == 0) {
		LOGLO("XMS Handler - Failed to get EMB info. Error %.2X", tbl);
		return 0;   // Whatever the reason, bomb out
	}

	s->LockCount = tbh;
	s->Length = tdx;
	FreeHandles = tbl;

	return 1;
}

/* Store data from conventional memory to an EMB block.  MEM -> EMB
   S is the EMB handle that will receive the data.
   p is the source of the data to be transfered
   offset is the offset in the EMB block to start the transfer.
   length is the amount of data in bytes to transfer.
   Returns: 1 success 0 address error  -1 length error  -2 serious hardware error
*/
int cXMS::StoreEMB(struct XMS_EMBInfo *s, char *p, long offset, long length) {
	struct XMS_EMMS m, *t;
	int tax;
	char tbl;

	t = &m;  // borland workaround

	// The source is the pointer, handle is 0
	m.hSource = 0;
	m.offSource = (unsigned long) p;
	// The destination is the handle with given offset
	m.hDest = s->hEMB;
	m.offDest = offset;
	m.Length = length;

	asm {
		mov ah, 0x0B
		push ds				// Preserve DS:SI
		push si
		lds  si, t				// Target in DS:SI
	}
	pHnd();
	asm {
		mov tax, ax
		mov tbl, bl
		pop si				// restore DS:SI
		pop ds
	}

	if (tax == 0) {
		LOGLO("XMS Handler - Error %.2X storing EMB.", tbl);
		switch (tbl) {
			case 0x81: return 0;
			case 0xA7:
			case 0xA8: return -1;
			case 0xA9: 
			case 0x82: return -2;
		}
	}
	return 1;
}

/* Fetch memory from an EMB block to conventional memory   MEM <- EMB
   S is the EMB handle that contains the desired data
   p is the destination that will hold the data
   offset is the offset in the EMB block to start the transfer.
   length is the amount of data in bytes to transfer.
*/

int cXMS::FetchEMB(struct XMS_EMBInfo *s, char *p, long offset, long length) {
	struct XMS_EMMS m, *t;
	int tax;
	char tbl;

	t = &m;  // borland workaround

	// The source is the handle with the given offset 
	m.hSource = s->hEMB;
	m.offSource = offset;
	// The destination is the pointer with 0 handle/
	m.hDest =   0;
	m.offDest = (unsigned long) p;
	m.Length = length;

	asm {
		mov ah, 0x0B
		push ds				// Preserve DS:SI
		push si
		lds si, t				// Target in DS:SI
	}
	pHnd();
	asm {
		mov tax, ax
		mov tbl, bl
		pop si				// restore DS:SI
		pop ds
	}

	if (tax == 0) {
		LOGLO("XMS Handler - Error %.2X fetching EMB.", tbl);
		switch (tbl) {
			case 0x81: return 0;
			case 0xA7:
			case 0xA8: return -1;
			case 0xA9: 
			case 0x82: return -2;
		}
	}

	return 1;

}


/* Allocate an EMB block. The caller fills in length in the structure. hEMB must be
   zero in order to prevent accidently overwriting a populated XMS_EMBInfo structure.
   If not we will automatically fail. We will request the block and set the handle.
   Then we will call GetEMBInfo on the new handle in order to verify
   its information and update FreeHandles.
   Returns: 1 success, 0 failure, -1 out of mem, -2 out of handles, -3 vdisk, -4 info
*/
int cXMS::AllocEMB(struct XMS_EMBInfo *s) {
	int tax, tdx;
	char tbl;
	int tLength = s->Length;		// borland workaround

	if (s->hEMB != 0) return 0;
	asm mov ah, 0x09
	asm mov dx, tLength
	pHnd();
	asm mov tax, ax
	asm mov tdx, dx
	asm mov tbl, bl

	if (tax == 0) {
		LOGLO("XMS Handler - Error %.2X Allocating EMB.", tbl);
		switch (tbl) {
			case 0x81: return -3;
			case 0xA0: return -1;
			case 0xA1: return -2;
			default:   return 0;
		}
	}

	s->hEMB = tdx;
	if (!GetEMBInfo(s)) {
		LOGLO("XMS Handler - Unexpected error getting EMB info after allocating!");
		return -4;  // This should not bomb if the alloc succeeded. 
	}
	return 1;
}


/* Free's an allocated EMB block. After free'ing we will zero the structure to prevent
   reuse. This DOES NOT update FreeHandles.
   Return: 1 success, 0 failure, -1 invalid, -2 locked, -3 vdisk                   */
int cXMS::FreeEMB(struct XMS_EMBInfo *s) {
	int tax;
	char tbl;
	XMSHnd tHandle = s->hEMB;   // borland workaround
	if (s->hEMB == 0) return 0;

	asm mov ah, 0x0A
	asm mov dx, tHandle
	pHnd();

	asm mov tax, ax
	asm mov tbl, bl

	if (tax == 0) {
		LOGLO("XMS Handler - Error %.2X free'ing EMB.", tbl);
		switch (tbl) {
			case 0x81: return -3;
			case 0xA2: return -1;
			case 0xAB: return -2;
			default:   return 0;
		}
	}

	// Success. Clear the structure info
	s->hEMB = 0; s->LockCount = 0; s->Length = 0;

	return 1;
}

/* Finds the free space left and max allocated block. Return false if failed. */
int cXMS::GetFreeXMS(struct XMS_FREE *s) {
	int tax, tdx;
	char tbl;

	asm mov ah, 0x08
	pHnd();

	asm mov tax, ax
	asm mov tdx, dx
	asm mov tbl, bl

	if (tbl == 0xA0) {   // memory exhausted 
		LOGLO("XMS Handler - GetFreeXMS reports exhausted memory.", tbl);
		s->Largest = 0; 
		s->Total = 0; 
		return 1;
	}
	if (tbl == 0x81 || tbl == 0x80) { 
	    LOGLO("XMS Handler - Error %.2X Getting Free XMS.", tbl);
		return 0;			// failed, due to vdisk
	}

	s->Total = tdx;
	s->Largest = tax;
	return 1;
}


/* 32 bit version, can report a very big number of free space. */
int cXMS::GetFreeXMS32(struct XMS_FREE *s) {
	char tbl;
	long tax, tcx, tdx;

	asm mov ah, 0x88
	pHnd();

	asm { 
	   mov tbl, bl
	   db 0x66; mov word ptr tax, ax
	   db 0x66; mov word ptr tcx, cx
	   db 0x66; mov word ptr tdx, dx
	}

	if (tbl != 0) {
		LOGLO("XMS Handler - GetFreeXMS reports exhausted memory.", tbl);
		if (tbl == 0xA0) {		// Memory is full
			s->Largest = 0;
			s->Total   = 0;
			s->TopAddr = 0;     // not sure what to do here, dont trust results
			return 0;
		}
		return -1;
	}

	s->Largest = (long) tax;
	s->Total   = (long) tdx;
	s->TopAddr = (long) tcx;
	return 1;
}




