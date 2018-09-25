/*
 * Xilinx_ports_glue.c
 *
 *  Created on: Jun 6, 2014
 *      Author: John
 */

#include "Application.h"
// I have to implement the routines declared in Xilinx\ports.h for my environment

#define printf(x) DebugPrint(4, x)

const short PortMap[] = { JTAG_TCK, JTAG_TMS, JTAG_TDI };

/* set the port "p" (TCK, TMS, or TDI) to val (0 or 1) */
void setPort(short p, short val)
{
	CyU3PGpioSetValue(PortMap[p], val);
}

/* read the TDO bit and store it in val */
unsigned char readTDOBit(void)
{
	CyBool_t Value;
	CyU3PGpioGetValue(JTAG_TDO, &Value);
	return (unsigned char)(Value);
}

/* make clock go down->up->down*/
void pulseClock(void)
{
	setPort(JTAG_TCK, 0);
	setPort(JTAG_TCK, 1);
	setPort(JTAG_TCK, 0);
}

/* read the next byte of data from the xsvf file */
//void readByte(unsigned char *data)
//{
//	*data = GetDataByte();
//}

void waitTime(long microsec)
{
	CyU3PThreadSleep((microsec+999)/1000);
}


