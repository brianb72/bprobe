#ifndef __CPUDET_H__
#define __CPUDET_H__

#include <Global.h>
#include "declare/global.h"
#include <Util\Timer.h>

class cCPUDet : cTimer {
public:
	cCPUDet();
public:
	int CPUID();
	void Benchmark();
	int Main();
	void find_cpu_text();
	int find_cpu_vendor();
private:
	void parse_desc(int code);

public:
   int tested;      /* All info invalid until this is true */
   int basetype;  	/* Base type  1: 386, 2: 486, 3: 486 w/cpuid or 586+ */
   float mhz;       /* Speed in mhz to 2 decimals */
   char modelname[40];
   char familyname[40];
   char stepname[40];

   /* Standard level 0000_0000h, all processors w/cpuid */	
   char vendorid[20];
   int  max_level;      /* Max level supported. All should be at least 1. */

   /* Standard level 0000_0001h, all 486's w/cpuid and most p2's. */
   int type;            /* Primary, Overdrive, Secondary (for MP) */
   int family;          /* 386, 486, 586, P2, etc */
   int model;           /* 486 sx/dx, Pentium models, etc */
   int stepping;        /* Stepping information */

   /* Standard level 0000_0002h, I think only p2+'s */
   word num_times;         /* Num times level must be called to get all desc */
   byte tlbcode;           /* Code tlb information */
   byte tlbdata;           /* Data tlb information */
   byte l1code;            /* l1 code cache */
   byte l1data;            /* l1 data cache */
   byte l2;                /* l2 code/data cache */
   byte cyrix_tlb;         /* Cyrix specific code/data tlb and c/d l1 */
   byte cyrix_l1;

};


#endif
