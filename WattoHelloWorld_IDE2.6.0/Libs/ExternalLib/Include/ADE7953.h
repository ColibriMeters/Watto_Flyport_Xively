/*********************************************************************
 *
 *	ADE7953 Communication Protocol Header
 *
 *********************************************************************
 * FileName:        ADE7953.h
 * Dependencies:    
 * Processor:       PIC24F
 * Compiler:        FLYPORT IDE 2.6.0
 * Company:         Presti Dario
 *
 * Software License Agreement
 *
 * Copyright (C) 2013 Presti Dario  All rights reserved.
 *
 * PRESTI DARIO licenses to you the right to use, modify, copy, and
 * distribute
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * PRESTI DARIO BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Dario Presti			28/02/2014	Original
 ********************************************************************/
 
#include "HWlib.h"
#include "INTlib.h"
#include "string.h"
#include "TCPIP Stack/Delay.h"
#ifndef __ADE7953_H
#define __ADE7953_H
/*

OK 1) monitoraggio della fine del power-up tramite reset flag is set in the IRQSTATA register (Address 0x22D and Address 0x32D). An external interrupt is triggered on the IRQ pin.
OK •	Write 0xAD to Register Address 0xFE: This unlocks the register 0x120
OK • 	Write 0x30 to Register Address 0x120: This configures the optimum settings
The above two instructions must be performed in succession to be successful.
OK 1) guadagno a 22 per 4KW, a 16 per 6KW       PGA_IA[2:0] (Addr 0x008)      101(guadagno 22) 100(guadagno 16)
2) PAG25: abilitare Active Energy Line Cycle Accumulation Mode  LCYCMODE register (Address 0x004) ALWATT = 1
	settare il tempo di accumulazione LINECYC register (Address 0x101) in the unit of number of half line cycles.
	POTENZA APPARENTE (AVA); POTENZA ATTIVA (AWATT)
	da vedere: NO-LOAD DETECTION con interrupts
3)	ZERO CROSSING DI DEFAULT SU ZX E ZX_I
		ZX_EDGE bits (Bits[13:12]) of the CONFIG register (Address 0x102) set the edge that triggers the zero-crossing event
		ZXV bit (Bit 15) in the IRQENA register (Address 0x22C and Address 0x32C). If this bit is set, a voltage channel zero-crossing event causes the IRQ pin to go low.
		ZXIA bit (Bit 12) in the IRQENA register (Address 0x22C and Address 0x32C). If this bit is set, a Current Channel A zero-crossing event causes the IRQ pin to go low
OK 4)  LOCKING THE COMMUNICATION INTERFACE: clearing the COMM_LOCK bit (Bit 15) in the CONFIG register (Address 0x102) a write should be issued shortly after power-up to the CONFIG register, clearing the COMM_LOCK bit and thus locking the communication interface
set the uart baud rate: Table 10. Frames in the UART Packet
	Frame 	Function
	F1 		Read/write  
	F2		Address MSB
	F3		Address LSB	
5) 	COMMUNICATION VERIFICATION & CECKSUM REGISTER
OK 6) 	uart read: send F1: 0x35 F2: addr MSB F3: addr LSB ----- ricezione del dato (2, 3 o 4 Bytes) LSB first
OK 7) 	uart write: send F1: 0xCA F2: addr MSB F3: addr LSB ----- dato MSB LSB
8)  i valori di IRMS e VRMS devono essere letti in sincrono con lo zero crossing
9)	CALIBRAZIONE DEL GUADAGNO (e della fase se usiamo un CT)
10)	xWATT xWATTOS xVAOS registri per la calibrazione dell'offset della potenza attiva reattiva ed apparente rispettivamente
	RMS offset
*/


/**COMMANDS
*
*/
#define WCOMM	0xCA	/**< WRITE COMMAND */
#define RCOMM	0x35	/**< READ COMMAND */

/**INITIALIZATION REGISTERS ADDRESS
*
*/
#define INITREGADDR1	0xFE
#define INITREGADDR2	0x120

/**INITIALIZATION REGISTER DATA
*
*/
#define INITREGDATA1	0xAD
#define	INITREGDATA2	0x30

#define CONFIG			0x102
#define LCYCMODE		0x004
#define IRQSTATA		0x32D

/**DATA REGISTER ADDRESS
*
*/
#define AWATT		0x312	/**< CHANNEL A ACTIVE POWER */
#define AVAR		0x314	/**< CHANNEL A REACTIVE POWER */
#define AVA			0x310	/**< CHANNEL A APPARENT POWER */
#define IA			0x316	/**< CHANNEL A CURRENT */
#define V			0x318	/**< VOLTAGE */
#define VRMS		0x31C	/**< RMS VOLTAGE */
#define IRMSA		0x31A	/**< CHANNEL A RMS CURRENT */
#define PGA_V		0x007	/**< PGA_V GAIN SET */
#define PGA_IA  	0x008	/**< PGA_IA GAIN SET */
#define PGA_IB  	0x009	/**< PGA_IB GAIN SET */
#define VRMSOS  	0x388   /**< VRMS OFFSET */
#define AIRMSOS 	0x386   /**< IARMS OFFSET */
#define AWATTOS 	0x389   /**< CHANNEL A ACTIVE POWER OFFSET */
#define AWGAIN  	0x382	/**< ACTIVE POWER CHANNEL A CALIBRATION */
#define AENERGY 	0x31E	/**< CHANNEL A ACTIVE ENERGY */
#define PFA 		0x10A	/**< CHANNEL A POWER FACTOR */

/**GAIN DEFINE FOR PGA
*
*/
#define GAIN1		0x00
#define GAIN2		0x01
#define GAIN4		0x02
#define GAIN8		0x03
#define GAIN16		0x04
#define GAIN22		0x05	/**< ONLY FOR PGA_IA*/

void OffsetCal(void);

/**UART WRITE ON ADE7953.
*details...
*/
void WriteAdeRegUart32(UINT16 ADDR,  UINT32 DATA);

/**UART WRITE ON ADE7953.
*details...
*/
void WriteAdeRegUart(UINT16 ADDR, UINT16 DATA);

/**UART WRITE ON ADE7953.
*details...
*/
void WriteAdeRegUart8(UINT16 ADDR,  UINT8 DATA);

/**UART READ ON ADE7953
*details...
*/
UINT32 ReadAdeRegUart(UINT16 ADDR);

/**INIT ADE7953
*details...
*/ 
void InitAde(void);

/**POWER UP ADE7953.
*details...
*/ 
void PwrUp(void);

/**SET VOLTAGE GAIN ADE7953
*set gain to GAIN1 for 220V line
*/
void VGainSet(void);

/**SET CH A CURRENT GAIN ADE7953
*set gain to 22 for 4KW power
*set gain to 16 for 16KW power
*/
void IAGainSet(void);

/**LOCK THE COMMUNICATION INTERFACE
*clearing the COMM_LOCK bit (Bit 15) in the CONFIG register (Address 0x102)
*a write should be issued shortly after power-up
*/
void LockComm(void);

#endif
