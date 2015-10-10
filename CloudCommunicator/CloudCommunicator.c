/*
 * CloudCommunicator.c
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include "CloudCommunicator.h"

int serverPipeFD;
int clientPipeFD;

void RunCloudCommunicator()
{
	serverPipeFD = OpenPipe(SERVERPIPEPATH, O_WRONLY | O_NONBLOCK);

	if (serverPipeFD == -1 && (errno == ENXIO || errno == ENOENT))
	{
		// The server has not been started.
		printf("===============================\n[Error]\nThe cloud has not been started\nExiting Controller.\n===============================\n\n");
		CloudCommunicatorExit();
	}
	else
	{
		printf("[Info]: Successfully Connected To Server Pipe.\n");
	}

	printf("[Info]: Waiting for Device Registration Messages...\n\n");


	char clientPipeName[CLIENT_PIPE_NAME_LENGTH];

	// Create Pipe Name using client pipe name standard (CLIENTPIPEPATH) and the PID of the process
	sprintf(clientPipeName, CLIENTPIPEPATH, getpid());

	// Unlink any previous existing client pipe names
	unlink(clientPipeName);

	// Create and open a new Client Pipe
	clientPipeFD = CreateAndOpenPipe(clientPipeName, O_RDONLY | O_NONBLOCK);

	// Data message used for reading data messages from pipe
	DATAMESSAGE dataMsg;

	// Loop and check client pipe
	while (1)
	{
		// Read data message
		int readRes = ReadFromPipe(clientPipeFD, clientPipeName, &dataMsg, sizeof(dataMsg));

		// If the read result is the number of bytes read ( > 0 && != -1 ), then process the message
		if (readRes && readRes != -1)
		{
			switch (dataMsg.type)
			{
				case MSG_REQUEST: /* Processes Message Requests (Either From Server or potentially other clients) */
					ProcessServerRequest(&dataMsg);
					break;

				case MSG_RESPONSE: /* Processes Message Responses */
				{
					switch (dataMsg.response)
					{
						case RESPONSE_SUCCESS:
							printf("[Response]: Server Processed Request Succesfully\n\n");
							break;

						case RESPONSE_FAILED:
							printf("[Response]: Server Failed to Process Request\n\n");
							break;

						case RESPONSE_SENSOR_INFO: // For now, this wont be used by the cloud controller
						default:
							break;
					}
				}
					break;
			}
		}

		usleep(50000);
	}
}

void ProcessServerRequest(DATAMESSAGE *dataMsg)
{
	if(dataMsg->request == REQUEST_READ_SENSOR_INFO)
	{

	}
	else if (dataMsg->request == REQUEST_ACTUATOR_ACTION)
	{

	}


}

void SendThresholdExceededInformationSendRequest(THRESHOLDCROSSINGMESSAGE *threshCrossMsg)
{

	DATAMESSAGE dataMsg;
	dataMsg.clientPID = getpid();
	dataMsg.request = REQUEST_THRESHOLD_EXCEEDED;
	dataMsg.type = MSG_REQUEST;

	// Write the request to send threshold crossing information to the server pipe
	if (WriteToPipe(serverPipeFD, SERVERPIPEPATH, &dataMsg, sizeof(dataMsg)) == -1)
	{
		// If errno == ENOENT, that means that the pipe no longer exists. Safely release all resources and quit
		if (errno == ENOENT)
		{
			ClientServerQuit(1);
		}
	}

	// Followed By writing the two main data structures for the threshold crossing to the server pipe
	if (WriteToPipe(serverPipeFD, SERVERPIPEPATH, &(threshCrossMsg->devInfo), sizeof(threshCrossMsg->devInfo)) == -1)
	{
		// If errno == ENOENT, that means that the pipe no longer exists. Safely release all resources and quit
		if (errno == ENOENT)
		{
			ClientServerQuit(1);
		}
	}

	if (WriteToPipe(serverPipeFD, SERVERPIPEPATH, &(threshCrossMsg->sensInfo), sizeof(threshCrossMsg->sensInfo)) == -1)
	{
		// If errno == ENOENT, that means that the pipe no longer exists. Safely release all resources and quit
		if (errno == ENOENT)
		{
			ClientServerQuit(1);
		}
	}

	printf("[Sent]: Request To Send Threshold Crossing Information to Server\n");

}

void ClientServerQuit(int status)
{
	if (status == 1 && errno == ENOENT)
	{
		printf("\n=======================================\n[Error]\nConnection to the server has been lost.\nClient Is Now Exiting.\n=======================================\n\n");
	}

	char clientPipeName[CLIENT_PIPE_NAME_LENGTH];

	// Create Pipe Name using client pipe name standard (CLIENTPIPEPATH) and the PID of the process
	sprintf(clientPipeName, CLIENTPIPEPATH, getpid());

	// Unlink all associated files
	unlink(clientPipeName);
	unlink(SERVERPIPEPATH);

	// Close all file descriptors
	close(clientPipeFD);
	close(serverPipeFD);

	// Exit the cloud communications controller
	CloudCommunicatorExit();
}

void CloudCommunicatorExit()
{

printf("[Sent]: Sent Quit Message To Device Communicator\n");
	// Send Quit Message to Child (DeviceCommunicator)
	SendProcessCommand(CMD_QUIT, devComPID);

	// The child handles the safe unregistering and termination of all registered devices and removes the message queue
	// Wait for Child to quit

	wait(0);

	printf("[Quit]: PID %d: has Quit\n", getpid());
	exit(0);
}
