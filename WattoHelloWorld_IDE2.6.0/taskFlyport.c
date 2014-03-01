#include "taskFlyport.h"
#include "ade7953.h"
#include "Xively.h"
#include "math.h"

//Defines for state machine 
#define SM_DELAY_WAIT 1
#define SM_DELAY_ALL_DONE 2

//Energy Meter Measures to be read
UINT32 REGTOREAD_ARRAY[5] = {PFA, VRMS, IRMSA, AWATT, 0};
UINT8 NUM_MEAS = 4;	//Number of measures to be read

//Xively variables and defines
#define xDelay(x) (portTickType)(x/portTICK_RATE_MS)
char ServerName[] =   "api.xively.com";	// Defines the server to be accessed	
char ServerPort[] = "8081";	// Defines the port to be accessed	
static ROM char Resource[] = "/feeds/YOURFEEDID";	// Defines the URL to be requested, It containes the feed number to use.
static ROM char XApiKey[] = "YOURAPIKEY";	// Defines the Pachube API Key to use
static ROM char GoodHTTPResponse[] = "{\"status\":200";	// Defines the URL to be requested, It containes the feed number to use.


//Defines for ADE data conversion
static float PFA_Coef = 0.000030518;
static float VRMS_Coef = 0.0000391;
static float IRMSA_Coef = 0.00000178;
static float AWATT_Coeff = 0.00116;

//Calibration values (example values, to adapt for every single board)
static float IRMSAGain = 1.2702;
static float VRMSGain = 1.0300;
static float AWATTGain = 1.2412;
	
//Defines for watto board
#define LED1 	p25	//WATTO LED1
#define LED2 	p8	//WATTO LED2
#define ZX_IIN 	p7	//WATTO current channel A zero crossing
#define ZX_VIN 	p5	//WATTO voltage channel zero crossing
	
void FlyportTask()
{
	UINT32 DATA_ARRAY[4] = {0, 0, 0, 0};		//Raw ADE data array
	double CONV_DATA_ARRAY[4] = {0, 0, 0, 0};	//Converted floating point ADE data array
	static TCP_SOCKET   MySocket = INVALID_SOCKET;
	char tmpString[500];					//Message buffer

	//Zero crossing info variable
	int i, noV, noI;						
	BOOL ZX, ZX_I, ZX_prev;
	BOOL PutResult;
	//SET DELAY
	UINT8 sm = SM_DELAY_WAIT;	
	LONG startTimeAll;//TICK startTimeAll;
	int second = 30; 	//time between ADE requests
	LONG DELAY = (LONG)second*TICK_SECOND;	
	
	//Pin init
	IOInit(p11, in);	//REVP
	IOInit(p9, in);		//IRQ
	IOInit(ZX_VIN, in);	//ZX
	IOInit(ZX_IIN, in);	//ZX_I
	IOInit(6, out);		//RESET OUT
	IOPut(6, on);		//PUT RESET HIGH
	IOInit(LED1, out);	//WATTO LED1
	IOInit(LED2, out);	//WATTO LED2
	IOInit(26, inup);	//Active Low Reset Init (NOT CONNECTED)
	IOInit(18, in);		//CF1
	IOInit(20, in);		//CF2
	IOInit(21, out);	//Flyport LED5 is output
	IOInit(19, out);	//Flyport LED4 is output

	//UART2 INIT
	IOInit(p2,UART2RX); //Remap the p2 pin as UART2RX
	IOInit(p4,UART2TX); //Remap the p4 pin as UART2TX
	UARTInit(2,4800);	//Set baud rate for ADE communication
	uarton(2);			//Uart ON
	
	IOPut (LED2,on);	//LED2 on: start ADE init signal
	
	//ADE7953 INITIALIZATION
	vTaskSuspendAll();
	PwrUp();			//ADE power up function
	DelayMs(100);
	LockComm();			//ADE lock communication
	DelayMs(100);
	InitAde();			//ADE initialization function
	DelayMs(100);
	IAGainSet();		//ADE Curr ch.A Gain 

	xTaskResumeAll();
	IOPut (LED2,off);	//LED2 off: end of ADE init signal
		
	//Connection
	IOPut (LED1,on);	//LED1 on: connection start signal
	WFConnect(WF_DEFAULT);
	while (WFStatus != CONNECTED);
	MySocket = XivelyConn(ServerName, ServerPort);
	IOPut (LED1,off);	//LED1 off: connection end signal
	
	startTimeAll = (LONG)TickGet();	//start of reference time
	
	//Main Loop
	while(1)
	{
		//State Machine
		switch (sm)
		{			
			//Waiting
			case SM_DELAY_WAIT:
					
				if((LONG)(TickGet() - startTimeAll) < DELAY)
				{
					//Waiting...
				}
				else if((LONG)(TickGet() - startTimeAll) >= DELAY)
				{
					IOPut (o4,on);		//led out4 on for delay done signal 
					sm = SM_DELAY_ALL_DONE;
					startTimeAll = (LONG)TickGet();
				}
			break;
			
			//Delay done - Get Measures
			case SM_DELAY_ALL_DONE:
				IOPut(LED1, on);	//LED1 on - start ADE request signal
	
				//ADE Requests
				for(i=0; i<NUM_MEAS; i++)//for every parameter to be read (set in regtoread_array)
				{
					vTaskSuspendAll();
					//zero crossing reading
					if(i==1)
					{
						ZX_prev = IOGet(ZX_VIN);
						noV = 0;
						for(noV=0; noV<5; noV++)
						{
							noV++;
							ZX = IOGet(ZX_VIN);
							if(ZX != ZX_prev)
								noV = 5;
						}
					}
					else if(i==2 || i==3)
					{
						ZX_prev = IOGet(ZX_IIN);
						noI = 0;
						for(noI=0; noI<5; noI++)
						{
							noI++;
							ZX_I = IOGet(ZX_VIN);
							if(ZX_I != ZX_prev)
								noI = 5;
						}
					}
					
					DelayMs(100);
					DATA_ARRAY[i] = ReadAdeRegUart(REGTOREAD_ARRAY[i]);
					xTaskResumeAll();
					
					//Data Conversion
					if(i == 0)
						CONV_DATA_ARRAY[0] = (INT16)DATA_ARRAY[0] * PFA_Coef;//COSPHY - signed value
					else if(i == 1)
						CONV_DATA_ARRAY[1] = DATA_ARRAY[1] * VRMS_Coef / VRMSGain;//VOLTAGE - unsigned value
					else if(i == 2)
						CONV_DATA_ARRAY[2] = DATA_ARRAY[2] * IRMSA_Coef * IRMSAGain;//CURRENT (Ampere) - unsigned value
					else if(i == 3)
						CONV_DATA_ARRAY[3] = (INT32)DATA_ARRAY[3] * AWATT_Coeff * AWATTGain;//POWER (Watt) - signed value
				}
				IOPut (LED1,off);	//ADE communication ended signal

				// Place the Xively request into the transmit buffer.
				sprintf( tmpString,"{\"method\":\"PUT\", \"resource\":\"%s\", \"headers\":{\"X-ApiKey\":\"%s\"},"\
				"\"body\":{\"version\" : \"1.0.0\", \"datastreams\":[{\"id\":\"PowerFactor\", \"current_value\":\"%.2f\"},"\
				"\{\"id\":\"Voltage\", \"current_value\":\"%.0f\"},"\
				"\{\"id\":\"Current\", \"current_value\":\"%.2f\"},"\
				"\{\"id\":\"ActivePower\", \"current_value\":\"%.2f\"}]}}",
				Resource, XApiKey, *CONV_DATA_ARRAY, *(CONV_DATA_ARRAY+1), *(CONV_DATA_ARRAY+2), *(CONV_DATA_ARRAY+3));

				//Transmit request
				PutResult = XivelyPut(MySocket, tmpString);
				if(PutResult == FALSE)
					Reset();
				
				IOPut (o4,off);	//end of transit request and start new waiting period signal
				sm = SM_DELAY_WAIT;	//go directly to the "wait case"
				
			break;
		}
	}
}





