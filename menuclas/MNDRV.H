#ifndef __MNDRV_H__
#define __MNDRV_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include <Logical\BiosDrv.h>
#include <Logical\DosLet.h>
#include <LowLevel\sIde.h>
#include <UserIO\TextBox.h>

class cMnDrv : cTextVideo, cKeyInput {
public:
	cMnDrv();
	~cMnDrv();
public:
   void Main(int menudir);
   void DrawAscii();
   void DrawDrives();
   void PrintDrives(FILE *fp);
   void MakePartitionsBox();
   void MakeLettersBox();
   void DoPartition(char *name, int num, struct partition_data *lp,
			 struct partition_data *rp, int max);

public:
   int curItem;
   cIDE *oIDE[2];
   cBiosDrv *oBios;
   cDosLet  *oDos;
   cTextBox oBox[2];

   char *ide_model(int drv) { if (drv<=2) return oIDE[0]->Model(drv);
							  else return oIDE[1]->Model(drv - 2); }
   long ide_capacity(int drv) { if (drv<=2) return oIDE[0]->Capacity(drv);
							  else return oIDE[1]->Capacity(drv - 2); }
   char *ide_config(int drv)  { if (drv<=2) return oIDE[0]->ConfigType(drv);
							  else return oIDE[1]->ConfigType(drv - 2); }
   char dos_letter(int i)   { return oDos->dlet[i].letter; }
   int  dos_filesys(int i)  { return oDos->dlet[i].fstype; }
   long dos_capacity(int i) { return oDos->dlet[i].extfree.totalsector; }
   long dos_freespace(int i) { return oDos->dlet[i].extfree.freesector; }
   char *dos_osstring()     { return oDos->osString; }


};


#endif