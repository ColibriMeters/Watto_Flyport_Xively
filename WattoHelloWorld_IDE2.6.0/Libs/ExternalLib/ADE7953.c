/*********************************************************************
 *
 *	ADE7953 Communication Protocol Header
 *
 *********************************************************************
 * FileName:        ADE7953.h
 * Dependencies:    
 * Processor:       PIC24F
 * Ide:        		FLYPORT IDE 2.6.0
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

#include "ADE7953.h"

void OffsetCal()
{
	//WriteAdeRegUart(AIRMSOS, 0xA42F);			//VRMS Offset
	//WriteAdeRegUart(AIRMSOS, INITREGDATA2);	//IRMS Offset
	//WriteAdeRegUart(AWATTOS, INITREGDATA2);	//AWATT Offset
	WriteAdeRegUart32(VRMSOS, 0x900000);		/**< GAIN AWATT CALIBRATION (0x45c28f == 1.09 gain factor)*/
}


/**ADE7953 INITIALIZATION FUNCTION.
*This function initialize the ade after power-up
*/
void InitAde()
{
	WriteAdeRegUart(INITREGADDR1, INITREGDATA1);	//unlock the register 0x120
	WriteAdeRegUart(INITREGADDR2, INITREGDATA2);	//configures the optimum settings
} 

/**ADE7953 END OF POWER UP PROCEDURE.
*This function poll the IRQSTAT register to see when power up reset flag is set
*/
void PwrUp()
{
	UINT32 Data = 0;
	int i = 0;
	while (Data != 1)
	{
		Data = ReadAdeRegUart(IRQSTATA);	/**< read IRQSTATA register */
		//UARTWriteCh(1, Data>>16);			//OFD (Only For Debug)
		Data = (Data & 0x100000) >> 20;		/**< select reset bit */
		//UARTWriteCh(1, Data>>20);			//OFD
		//UARTWriteCh(1, Data);				//OFD
		DelayMs(100);						/**< Wait for flag set */
		
		i++;
		if(i >= 10)
		{
			Reset();
		}
		
	}
	//UARTWriteCh(1, Data);	OFD
}
 
/**ADE7953 END OF POWER UP PROCEDURE.
*This function set the V_PGA gain
*default GAIN1
*/
 void IAGainSet()
 {
	WriteAdeRegUart(PGA_IA, GAIN16);
 }
 
/**ADE7953 END OF POWER UP PROCEDURE.
*This function set the V_PGA gain
*default GAIN1
*/
 void VGainSet()
 {
	WriteAdeRegUart(PGA_V, GAIN1);
 }
 
/**LOCK THE COMMUNICATION INTERFACE
*clearing the COMM_LOCK bit (Bit 15) in the CONFIG register (Address 0x102)
*a write should be issued shortly after power-up
*/
void LockComm()
{
	UINT16 Data;
	Data = 4;//0x8004 & 0x7FFF;
	WriteAdeRegUart(CONFIG, Data);	/**< CLEAR BIT 15 TO LOCK COMMUNICATION */
}

/**WRITE ON ADE7953 32 BIT DATA REGISTERS FUNCTION.
*This function write a data in the ade 24 bit register
*ADDR: 16 bit formatted address
*DATA: 32 bit formatted data
*/ 
void WriteAdeRegUart32(UINT16 ADDR,  UINT32 DATA)
{
	unsigned char Buf[7] = {0};			/**< command, address and data buffer */
	int i = 0;
	
	/**
	*buffer population
	*/
	Buf[0] = WCOMM;							/**< write command */
	Buf[1] = (unsigned char)(ADDR >> 8);	/**< address MSB */
	Buf[2] = (unsigned char)ADDR;			/**< address LSB */
	Buf[3] = (unsigned char)DATA;			/**< data LSB */
	Buf[4] = (unsigned char)(DATA >> 8);	
	Buf[5] = (unsigned char)(DATA >> 16);				
	Buf[6] = (unsigned char)(DATA >> 24);	/**< data MSB */
	for (i=0; i<7; i++)
	{
		UARTWriteCh(2, Buf[i]);	/**< writing data */
		DelayMs(3);				/**< 4ms frame to frame delay */
	}
	DelayMs(6);					/**< 6ms packet to packet delay */		
}

/**WRITE ON ADE7953 REGISTERS FUNCTION.
*This function write a data in the ade 24 bit register
*ADDR: 16 bit formatted address
*DATA: 16 bit formatted data
*/ 
void WriteAdeRegUart(UINT16 ADDR,  UINT16 DATA)
{
	unsigned char Buf[5] = {0};			/**< command, address and data buffer */
	int i = 0;
	
	/**
	*buffer population
	*/
	Buf[0] = WCOMM;							/**< write command */
	Buf[1] = (unsigned char)(ADDR >> 8);	/**< address MSB */
	Buf[2] = (unsigned char)ADDR;			/**< address LSB */
	Buf[3] = (unsigned char)DATA;			/**< data LSB */
	Buf[4] = (unsigned char)(DATA >> 8);	/**< data MSB */
	

	for (i=0; i<5; i++)
	{
		UARTWriteCh(2, Buf[i]);	/**< writing data */
		DelayMs(3);				/**< 4ms frame to frame delay */
	}
	DelayMs(6);					/**< 6ms packet to packet delay */		
}

/**WRITE ON ADE7953 REGISTERS FUNCTION.
*This function write a data in the ade 8 bit register
*ADDR: 16 bit formatted address
*DATA: 8 bit formatted data
*/ 
void WriteAdeRegUart8(UINT16 ADDR,  UINT8 DATA)
{
	unsigned char Buf[4] = {0};			/**< command, address and data buffer */
	int i = 0;
	
	/**
	*buffer population
	*/
	Buf[0] = WCOMM;							/**< write command */
	Buf[1] = (unsigned char)(ADDR >> 8);	/**< address MSB */
	Buf[2] = (unsigned char)ADDR;			/**< address LSB */
	Buf[3] = (unsigned char)DATA;			/**< data LSB */
	
	for (i=0; i<4; i++)
	{
		UARTWriteCh(2, Buf[i]);	/**< writing data */
		DelayMs(3);				/**< 4ms frame to frame delay */
	}
	DelayMs(6);					/**< 6ms packet to packet delay */		
}

/**READ FROM ADE7953 REGISTERS FUNCTION.
*This function read a data from the ade 24 bit registers
*ADDR: 16 bit formatted address
*/ 
UINT32 ReadAdeRegUart(UINT16 ADDR)
{
	unsigned char Buf[4] = {0, 0, 0, 0};
	unsigned char RCVDBuf[4] = {0, 0, 0, 0};
	int i;
	int N = 0;
	int count = 4;
	UINT32 Data = 0;
		
	Buf[0] = RCOMM;				/**< read command */
	Buf[1] = (unsigned char)(ADDR >> 8);
	Buf[2] = (unsigned char)ADDR;
	//Buf[3] = '\0';
	
	for (i=0; i<3; i++)
	{
		UARTWriteCh(2, Buf[i]);	/**< write read command */
		DelayMs(3);				/**< 4ms frame to frame delay */
		//UARTWriteCh(1, Buf[i]);			//OFD
	}
	
	DelayMs(20);					/**< waiting for incoming data */
	
	/** 
	*receiving frames block
	*LSBfirst, at RCVDBuf[0] position
	*/
	
	N = UARTRead(2, (char*)RCVDBuf, count);	/**< receive until all transmitted char */	
	//UARTWriteCh(1, (char)N);			//OFD

	while(N > 0)
	{
		Data |= (UINT32)(RCVDBuf[N-1]) << (N-1)*8;	/**< Data output variable population whit MSB first and LSB last */
		//UARTWriteCh(1, (char)Data);			//OFD
 		N--;	
	}
	return Data;
}
