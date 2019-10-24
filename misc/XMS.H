#ifndef __XMS_H__
#define __XMS_H__

typedef unsigned short XMSHnd;  // A normal 16 bit XMS handle
typedef unsigned long XMSHnd32;   // A 32 bit XMS handle used in the advanced functions


struct XMS_EMBInfo {
    XMSHnd hEMB;            /* Handle of the EMB */
	char LockCount;
	unsigned int  Length;			/* Blocks length in K */
};

struct XMS_EMMS {				/* Extended Memory Move Structure */
	unsigned long Length;		/* Bytes to transfer */
	XMSHnd	hSource;		    /* Source handle */
	unsigned long offSource;	/* Source Offset */
	XMSHnd  hDest;				/* Destination handle */
	unsigned long offDest;		/* Destination Offset */
};

struct XMS_FREE {			   /* Long so the 32 bit version can use as well */
	unsigned long Largest;	   /* Largest block that can be allocated in K */
	unsigned long Total;	   /* Total amount of free XMS in K */
	unsigned long TopAddr;	   /* Top address, only for the 32 bit version. */
};

class cXMS {
public:
	cXMS();
	~cXMS();
	int  Init();				/* Initialize the class */
	int  CheckXMS();			/* Check if XMS is installed */
	void SetXMSHnd();
	int XMSVersion();			/* Return XMS version */
	int  QueryA20();        /* Query status of A20 line */
	int  RequestA20();      /* Local request of A20 line enable */
	int  ReleaseA20();      /* Local request of A20 line */
	int GetEMBInfo(struct XMS_EMBInfo *s);   /* Get EMB information about handle */
	int AllocEMB(struct XMS_EMBInfo *s);     /* Allocate an EMB block */
	int FreeEMB(struct XMS_EMBInfo *s);      /* Free an EMB block */
	int StoreEMB(struct XMS_EMBInfo *s, char *p, long offset, long length);   
	int FetchEMB(struct XMS_EMBInfo *s, char *p, long offset, long length);   
	int GetFreeXMS(struct XMS_FREE *s);		 /* Get free XMS information */
	int GetFreeXMS32(struct XMS_FREE *s);    /* 32 bit version of it */
public:
	int FreeHandles;		/* 3 of free EMB handles. Updated by GetEMBInfo.  */
	void (*pHnd)(void);		/* Pointer to XMS Handler */
};



#endif
