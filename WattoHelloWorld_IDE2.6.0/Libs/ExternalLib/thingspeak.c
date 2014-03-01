/* **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        thingspeak.c
 *  Dependencies:    Microchip configs files
 *  Module:          FlyPort WI-FI
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Stefano Saccucci     1.2     01/16/2012		   First release  (core team)
 *  					  www.notonlyelectronic.com
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/

#include "thingspeak.h"

/****************************************************************************************
 *				       			--- Global Parameters ---			                    *
 ***************************************************************************************/	

static TCP_SOCKET Socket;
static int i;
static BOOL flagTCP=FALSE;
static BOOL flagTCPisCON=FALSE;
static int z = 0;
static BYTE error = 0;

  /*-------------------------------------------------------------------------------------
  |	Function: 		WdataF(char* apikey, float value[], int nvalue)		  		 		|
  | Description: 	Function to write float data in a specific ThingSpeak database.		|
  | Returns:		BYTE 0 - No error										   			|
  | 		  			 1 - TCPClientOpen error							   			|
  | Parameters:		char* apikey - ThingSpeak write api key				 				|
  | 																					|
  |					int* value - array with float data to send to ThingSpeak    	    |
  |																						|
  |					int nvalue - number of value elements							    |  
  -------------------------------------------------------------------------------------*/

BYTE wfTHINGSPEAK(char* apikey, float* value, int nvalue)
{		
	error = 0;
	char messageTS[nvalue*20];
	char bufferTS[20];
	flagTCP=0;
		
	for(i=0; i<nvalue;i++)	
	{
		if(i==0)
		{
			sprintf(messageTS,"field%d=%2.1f", i+1, (double)value[i]);
		}
		else
		{
			sprintf(bufferTS,"&field%d=%2.1f", i+1, (double)value[i]);
			strcat(messageTS,bufferTS);
		}
		
	}
	
	char strPOST[195+(int)strlen(messageTS)];
	sprintf( strPOST, "POST /update HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\nX-THINGSPEAKAPIKEY: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", apikey, (int)strlen(messageTS), messageTS);
	Socket = INVALID_SOCKET; //elisa
	while(flagTCP==FALSE)
	{
		if(z==5)
		{	
			z=0;
			flagTCP=TRUE;
			error=1;
		}
		else
		{	
			Socket=TCPClientOpen ( IPThingSpeak, "80");
			//Elisa init
			char socketres[8];
			sprintf(socketres, "%d\t", z+1);
			UARTWrite(1,socketres);
			//UARTWrite(1,"Prova di connessione\r\n");
/* 			if (Socket ==254)
			{
				UARTWrite(1,"INVALID SOCKET = ");	
				sprintf(socketres, "%d\r\n", Socket);
				UARTWrite(1,socketres);
				//UARTWrite(1,"SCK=");
				//UARTWriteCh(1,(char)(((int)'0') + Socket));
			}
			//Elisa end */
			vTaskDelay(25);
			flagTCP=TCPisConn(Socket);
			flagTCPisCON=flagTCP;
			vTaskDelay(25);
			z++;
		}
	}
	
	if(flagTCPisCON==TRUE)
	{	char socketn[8];
		//UARTWrite(1,"\r\n"); //elisa
		z=0;//added by dario
		UARTWrite(1,"socket= ");
		sprintf(socketn, "%d\t", Socket);
		UARTWrite(1,socketn);

		//UARTWrite(1,"Event: TCP Conn = TRUE\r\n"); //elisa
		TCPWrite ( Socket, strPOST, (int)strlen(strPOST) );
		vTaskDelay(200);
		TCPClientClose ( Socket );
		flagTCPisCON=FALSE;
	}
	
	return error;
}

  /*-------------------------------------------------------------------------------------
  |	Function: 		WdataI(char* apikey, int* value, int nvalue)			   			|
  | Description: 	Function to write integer data in a specific ThingSpeak database.	|
  | Returns:		BYTE 0 - No error										   			|
  | 		  			 1 - TCPClientOpen error							   			|
  | Parameters:		char* apikey - ThingSpeak write api key				 				|
  | 																					|
  |					int* value - array with integer data to send to ThingSpeak    	    |
  |																						|
  |					int nvalue - number of value elements							    |  
  -------------------------------------------------------------------------------------*/

BYTE wiTHINGSPEAK(char* apikey, int* value, int nvalue)
{		
	error = 0;
	char messageTS[nvalue*20];
	char bufferTS[20];
	flagTCP=0;
	
	for(i=0; i<nvalue;i++)	
	{
		if(i==0)
		{
			sprintf(messageTS,"field%d=%d", i+1, value[i]);
		}
		else
		{
			sprintf(bufferTS,"&field%d=%d", i+1, value[i]);
			strcat(messageTS,bufferTS);
		}
		
	}
	
	char strPOST[195+(int)strlen(messageTS)];
	sprintf( strPOST, "POST /update HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\nX-THINGSPEAKAPIKEY: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s", apikey, (int)strlen(messageTS), messageTS);
	
	while(flagTCP==FALSE)
	{
		if(z==5)
		{	
			z=0;
			flagTCP=TRUE;
			error=1;
		}
		else
		{	
			Socket=TCPClientOpen ( IPThingSpeak, "80");
			vTaskDelay(25);
			flagTCP=TCPisConn(Socket);
			flagTCPisCON=flagTCP;
			vTaskDelay(25);
			z++;
		}
	}
	
	if(flagTCPisCON==TRUE)
	{
		TCPWrite ( Socket, strPOST, (int)strlen(strPOST) );
		vTaskDelay(200);
		TCPClientClose ( Socket );
		flagTCPisCON=FALSE;
	}
	
	return error;
}

  /*-------------------------------------------------------------------------------------
  |	Function: 		thingHTTP(char* apikey, char* data)						   			|
  | Description: 	Function to use ThingHTTP APP.										|
  | Returns:		BYTE 0 - No error										   			|
  | 		  			 1 - TCPClientOpen error							   			|
  | 		  			 2 - ThingSpeak response timeout     				   			|
  | Parameters:		char* apikey - ThingSpeak write api key				 				|
  | 																					|
  |					char* data - pointer to the return data/string value from ThingSpeak|
  -------------------------------------------------------------------------------------*/

BYTE thingHTTP(char* apikey, char* data)
{		
	error = 0;
	char strPOST[300];
	WORD size;
	BYTE flag = 0; 
	i=0;
	flagTCP=0;
	
	sprintf( strPOST, "POST /apps/thinghttp/send_request HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\napi_key=%s",8+(int)strlen(apikey), apikey);

	while(flagTCP==FALSE)
	{
		if(z==5)
		{	
			z=0;
			flagTCP=TRUE;
			error=1;
		}
		else
		{	
			Socket=TCPClientOpen ( IPThingSpeak, "80");
			vTaskDelay(25);
			flagTCP=TCPisConn(Socket);
			flagTCPisCON=flagTCP;
			vTaskDelay(25);
			z++;
		}
	}
	
	if(flagTCPisCON==TRUE)
	{
		TCPWrite ( Socket, strPOST, (int)strlen(strPOST) );
		while( flag==0 )
		{
			size = TCPRxLen ( Socket );
		
			if((int)size<=0)
			{
				i++;
				vTaskDelay(5);
				if(i==100)
				{
					error=2;
					break;
				}
			}	
			else
				flag = 1;
		}
		if(flag==1)
		{
			char bufferTS[(int)size];
				
			TCPRead ( Socket, bufferTS, (int)size );
				
			vTaskDelay(100);
				
			strcpy(data,bufferTS);
		}
		TCPClientClose ( Socket );
		flagTCPisCON=FALSE;
	}
	return error;
}
  /*-----------------------------------------------------------------------------------------
  |	Function: 		thingTWEET(char* apikey, char* message)					   				|
  | Description: 	Function to use ThingTWEET APP.											|
  | Returns:		BYTE 0 - No error										   		     	|
  | 		  			 1 - TCPClientOpen error							   			    |
  | Parameters:		char* apikey - ThingSpeak TWEET api key				 					|
  | 																						|
  |					char* message - message to send on Twitter to change status, you can	|
  |								    add some tag in this way:                           	|
  |									message&lat=(your latitude)&long=(your longitude) 		|
  |									for available tags see this								|
  |									https://dev.twitter.com/docs/api/1/post/statuses/update	|												
  -----------------------------------------------------------------------------------------*/
  
BYTE thingTWEET(char* apikey, char* message)
{		
	error = 0;
	char strPOST[400];
	flagTCP=0;
	
	sprintf( strPOST, "POST /apps/thingtweet/1/statuses/update HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\napi_key=%s&status=%s",8+(int)strlen(apikey)+(int)strlen(message), apikey, message);
	
	while(flagTCP==FALSE)
	{
		if(z==5)
		{	
			z=0;
			flagTCP=TRUE;
			error=1;
		}
		else
		{	
			Socket=TCPClientOpen ( IPThingSpeak, "80");
			vTaskDelay(25);
			flagTCP=TCPisConn(Socket);
			flagTCPisCON=flagTCP;
			vTaskDelay(25);
			z++;
		}
	}
	
	if(flagTCPisCON==TRUE)
	{
		TCPWrite ( Socket, strPOST, (int)strlen(strPOST) );
		vTaskDelay(200);
		TCPClientClose ( Socket );
		flagTCPisCON=FALSE;
	}
	
	return error;
}
