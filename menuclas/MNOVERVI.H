#ifndef __MNOVERVI_H__
#define __MNOVERVI_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include "diag/cpu/cpudet.h"
#include "diag/port/lptdet.h"
#include "diag/port/serial.h"
#include <LowLevel/RamDet.h>

class cMnOverView : cKeyInput, cTextVideo {
public:
	cMnOverView();
	~cMnOverView();
	void Main(int menudir);
	void DrawAscii();

	float cpuspeed;
	int   cputested;
	void CheckBenchmark();

public:
   cCPUDet    *oCPU;
   cRamDet    *oRAM;
   cLPTDet    *oLPT;
   cSerialDet *oCOM;

   char *cpu_vendorid() { return oCPU->vendorid; }
   float cpu_speed() { return cpuspeed; }
   char *cpu_family() { return oCPU->familyname; }
   char *cpu_model()  { return oCPU->modelname; }
   int   cpu_step()   { return oCPU->stepping; }
   char *cpu_stepname() { return oCPU->stepname; }
   long  ram_mapped()  { return oRAM->MappedRam(); }
   long  ram_reported() { return oRAM->ReportedRam(); }
   int   com_comm(int i) { return oCOM->port[i].comm; }
   int   com_base(int i) { return oCOM->port[i].base; }
   int   com_irq(int i) { return oCOM->port[i].irq; }
   char *com_uart(int i) { return oCOM->UART_Name(i); }
   int   lpt_base(int i) { return oLPT->port[i].address; }
   char *lpt_mode(int i) { return oLPT->LPT_Mode(i); }


};





#endif
