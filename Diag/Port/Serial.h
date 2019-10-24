#ifndef __SERIAL_H__
#define __SERIAL_H__

/* Some defines for accessing ports */
#define LCR		3		/* Line Control */
#define MCR		4       /* Modem Control */
#define LSR     5       /* Line Status */
#define MSR     6       /* Modem Status */
#define DLL     0       /* Low byte of baud rate divisor */
#define DLH     1       /* High byte of baud rate divisor */

/* UART types */
#define UART_UNKNOWN 0
#define UART_8250    1
#define UART_16450   2
#define UART_16550   3
#define UART_16550A  4


struct serial_data {
   unsigned int base; 	  /* Base address */
   unsigned int irq;	  /* IRQ */
   int uart;		      /* Detected UART */
   int comm;		      /* Guessed comm port */
   unsigned int baud;     /* Baud rate of port */
   char status[4];        /* Status (parity, stop bits, word length) */
};


class cSerialDet {
public:
    void Init();    
public:
	int detect_IRQ(unsigned base);
	int detect_UART(unsigned baseaddr);
	int detect_serial_status(int pnum, unsigned baseaddr);
    char *UART_Name(int num);
	struct serial_data port[4];	
};


#endif
