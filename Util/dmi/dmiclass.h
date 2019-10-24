#ifndef __DMICLASS_H__
#define __DMICLASS_H__

#include "util/dmi/Dmistruc.h"

class cDMI {
public:
	cDMI();
	~cDMI();
	char *get_smbios_entry(unsigned int off, unsigned int seg);
	unsigned long rconv(char *addr);
	int scan_dmi(void);
protected:
	int dmi_items;
	struct dmi_data dmi[100];
};

#endif