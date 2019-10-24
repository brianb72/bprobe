#ifndef __MEMDET_H__
#define __MEMDET_H__


class cMemDet {
public:
	cMemDet();
	int Main();
public:
	long GetRamSize();    // Returns number of 1K blocks */
private:
	long try_E801h(void);
	long try_DA88h(void);
	long try_52h_int21(void); 
	long try_E820(void);
private:
	long ramsize;   /* In k */
	int tested;

};


#endif
