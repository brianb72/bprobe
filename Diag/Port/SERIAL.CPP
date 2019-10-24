/* www.repairfaq.org/filipg/LINK/PORTS/F_The_Serial_Port6.html */

#include <stdio.h>
#include <conio.h>

#include <Global.h>
#include "declare/global.h"
#include "diag/port/serial.h"


/* Prints the UART name of the specified port number */
char *cSerialDet::UART_Name(int num) {

	switch (port[num].uart) {
	case 1: return "8250";
	case 2: return "16450";
	case 3: return "16550";
	case 4: return "16550A";
	default: return "Unknown"; 
	}

}


void cSerialDet::Init() {
   int i;
   const unsigned short default_comm_io[4] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };

   LOGLO("---Serial Port Detection---\n");


   for (i = 0; i < 4; ++i) {
	    port[i].comm = 0;
	  if ((port[i].uart = detect_UART(default_comm_io[i])) != 0) {
		port[i].comm = i + 1;
		port[i].base = default_comm_io[i];
		port[i].irq  = detect_IRQ(default_comm_io[i]);
		detect_serial_status(i, default_comm_io[i]);
		LOGLO("  - Port %Xh is IRQ %i  Baud %i  Status: %.4s", 
			port[i].base, port[i].irq, port[i].baud, port[i].status);
	  }

	  /* For now just assume base address = comm port */

   }

}



/* 8N1                           bit values in LCR register
   status[0] = bits per word     0-1: bits per word
   status[1] = parity              2: stop bits
   status[2] = stop bits           3: parity on/off
   status[3] = '\0'                4: parity even/off
*/

/* ************************************************************************* */
int cSerialDet::detect_serial_status(int pnum, unsigned baseaddr) {
	unsigned value = 0;
	int baudlow = 0, baudhigh = 0;
	long tbaud = 0;

	value = inp(baseaddr + LCR);     /* Input the status of the port */

	 /* NOTE: Add 48 so we start at ascii character 48 or '0' */
	 /* Extract bits per word info: 00=5 01=6 10=7 11=8 */
	 port[pnum].status[0] = (value & 3) + 5 + 48;

	 /* Extract stop bits: 0 = 1 stop bit   1 = 2 stop bits */
	 port[pnum].status[2] = (value & 4) + 1 + 48;

	 port[pnum].status[3] = '\0';   /* tack on the null */

	 /* Check for parity on/off.  0 = off  1 = on */
	 if (value & 8) {
		 /* Check for even/odd.   0 = odd  1 = even */
		 if (value & 9)
			port[pnum].status[1] = 'E';
		 else
			port[pnum].status[1] = 'O';
	 }
	 else
		  port[pnum].status[1] = 'N';   /* no parity */

	 asm cli;
	 /* Now get the baud rate. We must activate Divisor-Latch Access-Bit.   */
	 /* OR it against what we pulled off the LCR, then push that value onto */
	 /* the port so we do not disturb the settings                          */
	 if (!(value & 128)) {
		outp(baseaddr + LCR, value | 128);
	 }
	 else
		   LOGLO("  - (%Xh) Divisor-Latch Access-Bit already active?", baseaddr);

	 /* Fetch the values */
	 baudlow = inp(baseaddr + DLL);
	 baudhigh = inp(baseaddr + DLH);

	 /* Restore the LCR */
	 outp(baseaddr + LCR, value);

	 asm sti;

	 tbaud = (baudhigh << 8) + baudlow;

	 /* Avoid a divide by zero */
	 if (tbaud > 0)
		port[pnum].baud = (int) (115200L / tbaud);
	 else {
		LOGLO("  - %Xh returned zero baud divisor, ignoring.", baseaddr);
		port[pnum].baud = 0;
	 }
   
	 return TRUE;
}






/* ************************************************************************* */
int cSerialDet::detect_UART(unsigned baseaddr)
{
   // this function returns 0 if no UART is installed.
   // 1: 8250, 2: 16450 or 8250 with scratch reg., 3: 16550, 4: 16550A
   int x,olddata;

   // check if a UART is present anyway
   olddata=inp(baseaddr+4);
   outp(baseaddr+4,0x10);
   if ((inp(baseaddr+6)&0xf0)) return 0;
   outp(baseaddr+4,0x1f);
   if ((inp(baseaddr+6)&0xf0)!=0xf0) return 0;
   outp(baseaddr+4,olddata);
   // next thing to do is look for the scratch register
   olddata=inp(baseaddr+7);
   outp(baseaddr+7,0x55);
   if (inp(baseaddr+7)!=0x55) return 1;
   outp(baseaddr+7,0xAA);
   if (inp(baseaddr+7)!=0xAA) return 1;
   outp(baseaddr+7,olddata); // we don't need to restore it if it's not there
   // then check if there's a FIFO
   outp(baseaddr+2,1);
   x=inp(baseaddr+2);
   // some old-fashioned software relies on this!
   outp(baseaddr+2,0x0);
   if ((x&0x80)==0) return 2;
   if ((x&0x40)==0) return 3;
   return 4;
}

int cSerialDet::detect_IRQ(unsigned base)
{
  // returns: -1 if no intlevel found, or intlevel 0-15
  char ier,mcr,imrm,imrs,maskm,masks,irqm,irqs;
  long counter = 0;
  
  _asm cli;            // disable all CPU interrupts
  ier = inp(base+1);   // read IER
  outp(base+1,0);      // disable all UART ints
  
  /* Rough number of iterations of following loop in seconds
     on a 500mhz process.                                      */
  #define SERLOOPTIME 35000    
  
  while (!(inp(base+5)&0x20)) {    // wait for the THR to be empty
	  // Avoid potential infinite loop. 
	  ++counter;
	  if ((long)counter > ((long)SERLOOPTIME * (long)3)) {
         LOGLO("  - THR Timeout detecting IRQ for port %Xh"); 
		 /* Restore UART and interrupts */
         outp(base+1,ier);    // restore old value of IER
		 _asm sti;
		 return -1;
	  }
  }	  

  mcr = inp(base+4);   // read MCR
  outp(base+4,0x0F);   // connect UART to irq line
  imrm = inp(0x21);    // read contents of master ICU mask register
  imrs = inp(0xA1);    // read contents of slave ICU mask register
  outp(0xA0,0x0A);     // next read access to 0xA0 reads out IRR
  outp(0x20,0x0A);     // next read access to 0x20 reads out IRR
  outp(base+1,2);      // let's generate interrupts...
  maskm = inp(0x20);   // this clears all bits except for the one
  masks = inp(0xA0);   // that corresponds to the int
  outp(base+1,0);      // drop the int line
  maskm &= ~inp(0x20); // this clears all bits except for the one
  masks &= ~inp(0xA0); // that corresponds to the int
  outp(base+1,2);      // and raise it again just to be sure...
  maskm &= inp(0x20);  // this clears all bits except for the one
  masks &= inp(0xA0);  // that corresponds to the int
  outp(0xA1,~masks);   // now let us unmask this interrupt only
  outp(0x21,~maskm);
  outp(0xA0,0x0C);     // enter polled mode; Mike Surikov reported
  outp(0x20,0x0C);     // that order is important with Pentium/PCI systems
  irqs = inp(0xA0);    // and accept the interrupt
  irqm = inp(0x20);
  inp(base+2);         // reset transmitter interrupt in UART
  outp(base+4,mcr);    // restore old value of MCR
  outp(base+1,ier);    // restore old value of IER
  if (masks) outp(0xA0,0x20);  // send an EOI to slave
  if (maskm) outp(0x20,0x20);  // send an EOI to master
  outp(0x21,imrm);     // restore old mask register contents
  outp(0xA1,imrs);
  _asm sti;
  if (irqs&0x80)       // slave interrupt occured
	return (irqs&0x07)+8;
  if (irqm&0x80)       // master interrupt occured
	return irqm&0x07;
  return -1;
}


