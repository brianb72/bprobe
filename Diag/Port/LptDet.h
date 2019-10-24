#ifndef __LPTDET_H__
#define __LPTDET_H__

#define LPT_NONE  0   // No mode, no printer port detected
#define LPT_SPP   1
#define LPT_EPP   2
#define LPT_ECP   3


#define R_DATA  0x0     // Data Register OR ECP ADDRESS FIFO  WR
#define R_DSR   0x1     // Status Register    R
#define R_DCR   0x2     // Control Register   WR
#define R_EPPA  0x3     // EPP Address
#define R_EPPD  0x4     // EPP Data
#define R_CFIFO 0x400   // ECP Data FIFO OR Test FIFO OR Config Register A
#define R_CONB  0x401   // Configuration Register B
#define R_ECR   0x402   // Extended Control Register

struct lpt_data {
   unsigned int address;    /* Base Adress of port */
   unsigned char mode;      /* Guessed mode */
   unsigned char emode;     /* ECP Only: Current Mode */
};

class cLPTDet {
public:
    void Init();    
public:
	void clear_epp_timeout(int address);
	int guess_mode(int i, unsigned int address);
	struct lpt_data port[4];	
	char *LPT_Mode(int i);
};


#endif
