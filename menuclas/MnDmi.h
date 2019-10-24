#ifndef __DMIVIEW_H__
#define __DMIVIEW_H__

#include "menuclas/MnMain.h"
#include "menuclas/keyinput.h"
#include "util/dmi/dmiclass.h"

class cMnDMI : cTextVideo, cKeyInput, cDMI {
public:
	cMnDMI();
	~cMnDMI();
	void Main(int menudir);
	void DrawAscii();
    void Draw();
	void chassis_state_string(int state);
	unsigned int mpow(unsigned int b);
    void dmi_print_bitfield(int value, const char **s, int maxitem);
    void aprintf(char *format, int stringnum);
	void dmi_cpu_family(int type);
	void dmi_mem_interleave(int type);
    void dmi_sram_type(int item);
	void DrawDMIItem(int item);
	void ClrDump();
	void ClrMenu();
private:
	int selected;
	int topitem;
   char *d;   /* generic pointer for displaying elements */
   char **s;  /* same */

};

#endif