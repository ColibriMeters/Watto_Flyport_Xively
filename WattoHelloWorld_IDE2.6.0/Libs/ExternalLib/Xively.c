#include "Xively.h"

#define xDelay(x) (portTickType)(x/portTICK_RATE_MS)

TCP_SOCKET XivelyConn(char* ServerName, char* ServerPort)
{
	int j;
	TCP_SOCKET   MySocket = INVALID_SOCKET;
	#if defined(STACK_USE_UART)
	UARTWrite(1,"Connecting to Xively...\r\n");
	#endif
	MySocket = TCPClientOpen(ServerName, ServerPort);
	
	#if defined(STACK_USE_UART)
	if(MySocket == INVALID_SOCKET) 
	{
		UARTWrite(1,"INVALID_SOCKET\r\n");
	}
	UARTWrite(1,"Checking connection.\r\n");
	#endif
	// Make sure it connects
	j = 10;
	while (j>0 && !TCPisConn(MySocket))
	{
		//vTaskDelay(xDelay(500));
		vTaskDelay(500);
		j--;
		//sprintf(j, "%d\r\n", j+1);
	}
	if(j==0) 
	{
		#if defined(STACK_USE_UART)
		UARTWrite(1,"Connection failed.\r\n");
		#endif
		// Close the socket so it can be used by other modules
		TCPClientClose(MySocket);
		MySocket = INVALID_SOCKET;
		// We are in an error condition, so light up the Flyport led
	}
	return MySocket;
}

BOOL XivelyPut(TCP_SOCKET MySocket, char* tmpString)
{
	char resString[250];
	BOOL result;
	int i;
	static ROM char GoodHTTPResponse[] = "{\"status\":200";
	
/*	// Place the Xively request into the transmit buffer.
	sprintf( tmpString,"{\"method\":\"PUT\", \"resource\":\"%s\", \"headers\":{\"X-ApiKey\":\"%s\"},"\
	"\"body\":{\"version\" : \"1.0.0\", \"datastreams\":[{\"id\":\"ActivePower\", \"current_value\":\"%d\"},"\
	"\{\"id\":\"CosPhy\", \"current_value\":\"%d\"},"\
	"\{\"id\":\"Voltage\", \"current_value\":\"%d\"},"\
	"\{\"id\":\"Current\", \"current_value\":\"%d\"}"\
	"\]}}", Resource, XApiKey, *buf, *buf+1, *buf+2, *buf+3);
*/
	#if defined(STACK_USE_UART)
	UARTWrite(1,tmpString);
	#endif
	// Send the blob
	TCPWrite( MySocket, tmpString, (int)strlen(tmpString) );
	#if defined(STACK_USE_UART)
	UARTWrite(1,"Data sent, Waiting response.\r\n");
	#endif
	//vTaskDelay( xDelay(500) );
	vTaskDelay(500);

	
	// Make sure there are enough chars to read
	i = 10;
	while (i>0 && (TCPRxLen(MySocket) < 16))
	{
		//vTaskDelay( xDelay(500) );
		vTaskDelay(500);

		i--;
	}
		if(i!=0) 
	{
		// Get the response from the server
		TCPRead( MySocket, resString, 250 );
		#if defined(STACK_USE_UART)
		UARTWrite(1,resString);
		UARTWrite(1,"\r\n");
		#endif
		if (strstr( resString, GoodHTTPResponse) == NULL)
		{
			// We are in an error condition, so light up the Flyport led
			result = FALSE;
			#if defined(STACK_USE_UART)
			UARTWrite(1,"Request failed.\r\n");
			#endif
		} 
		else 
		{
			#if defined(STACK_USE_UART)
			UARTWrite(1,"Request succeeded.\r\n");
			#endif
			result = TRUE;
		}
	}
	
	else 
	{
		#if defined(STACK_USE_UART)
		UARTWrite(1,"Server did not reply.\r\n");
		#endif
		result = TRUE;
		Reset();
	}
	
	//vTaskDelay( xDelay(500) );
	vTaskDelay(500);
	
	/*NO DISCONNETTING*/
	/*#if defined(STACK_USE_UART)
	UARTWrite(1,"Disconnecting...");
	#endif
	TCPClientClose(MySocket);
	MySocket = INVALID_SOCKET;
	#if defined(STACK_USE_UART)
	UARTWrite(1,"Done.\r\n");
	#endif*/

	vTaskDelay(500);
	
	return result;
}
