/*
 * Controller.c
 *
 *	The Controller contains two main components.
 *
 *	1. The Parent process (CloudCommunicator) will communicate with the cloud component (using FIFO -> the cloud = server & parent process = client)
 *		- The parent process may catch a signal sent by the child process and read a message from the queue. It sends the info
 *				to the cloud process
 *		- The parent process may receive commands from the user (the cloud process).
 *			- The command is either get or put
 *				- Get = request info from sensor
 *				- put = send action to actuator
 *
 *
 *	2. The Child process (DeviceCommunicator) will be used to send and receive device messages (from sensors or actuators) using a message queue
 *		- The first message of the device is a registration message. The child process registers this device (using PID, device name and threshold (if available)
 *		- Needs to echo any information it receives from any process
 *
 *
 *
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

/* Controller.c is the main source file for execution of the controller */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#include "CloudCommunicator/CloudCommunicator.h"
#include "DeviceCommunicator/DeviceCommunicator.h"
#include "Messages/Messages.h"
#include "Devices/Devices.h"
#include "SignalAssist/SignalAssist.h"

char controllerName[25];

void PrintUsage()
{
	printf("Usage: IoT_Controller [ControllerName]\n\t[ControllerName]: The name of the controller (1 - 24 Characters).\n");
}

int main(int argsv, char *args[])
{
	// Checks to make sure number of arguments is 2 for controller name
	if (argsv != 2 || strlen(args[1]) == 0 || !strcasecmp(args[1], "-help"))
	{
		PrintUsage();
		exit(1);
	}

	// Ensures Controller Name is 24 or less characters
	if (strlen(args[1]) > 24)
	{
		fprintf(stdout, "[Error]: The controller name must be a maximum of 24 characters\n");
		PrintUsage();
		exit(1);
	}

	// Copy Controller Name argument into controller name variable
	strcpy(controllerName, args[1]);

	// Clear the screen
	system("clear");

	// Controller has started message
	printf("=========================================\nController '%s': has started!\n", controllerName);

	// Initialize the message queue
	msqid = msgget((key_t) MSGKEY, 0666 | IPC_CREAT );

	// Ensure the message queue was initialized successfully
	if (msqid == -1)
	{
		fprintf(stdout, "[Error]: msgget failed with error %d\n", errno);
		return -1;
	}

	// Install Signal handlers
	InstallControlCSignalHandlers();

	// Install Message Signal Handlers
	InstallMessageSignalHandler();

	// Create the child process through fork()
	devComPID = fork();

	// Use switch cases to determine if fork completed successfully, and to determine
	// who the parent and child processes are
	switch (devComPID)
	{
		// fork failed
		case -1:
		{
			fprintf(stdout, "[Error]: Fork failed with error code %d. The Controller failed to start.\n", errno);
			return errno;
		}
			// Child Process
		case 0:
		{
			// Print Child and Parent PID

			while (1)
			{
				// We want to check for messages twice a second
				CheckForMessages();

				// 10^3 microseconds = 1 millisecond
				// Therefore 500*10^3 = 500 milliseconds = 0.5 seconds
				usleep(500 * (10 ^ 3));

				// NOTE: We can exit out of the child process by sending it an exit command
			}

			break;
		}
			// Parent process where pid = PID of child
		default:
		{
			printf("Child PID: %d\nParent PID: %d\n=========================================\n\n", devComPID, getpid());
			RunCloudCommunicator();

			break;
		}
	}

	return 0;

}

// Signal Handler: Child Has Sent Sensor Info To The Parent (CloudCommunicator)
void MsgRcvd()
{
	if (devComPID)
	{

		// Read in the threshold crossing message (sent from the child)
		THRESHOLDCROSSINGMESSAGE tcm;
		if (!ReceiveMessage(&tcm, sizeof(tcm) - sizeof(tcm.msgHdr.msgType), MSG_THRESHCROSS))
		{
			printf("[Received]: Threshold Crossing Message Received! Device PID: %d\n\n", tcm.devInfo.pid);

			// Send Request to server to send information about threshold crossing
			SendThresholdExceededInformationSendRequest(&tcm);

		}
		else
		{
			printf("[Error]: Failed to receive Threshold Crossing Message\n");
		}
	}


}

// Signal Handler: The process has received a CTRL-C signal and must act appropriately
void CtrlCPressed()
{
	// Since this handler is used by both the child and parent process,
	// We can seperate the handler for both processes
	// We need to use this handler for both processes because if one process quits, the other process will quit.
	// Therefore, the following approach handles the input of Ctrl-C. Once the parent catches this, it will safely exit using quit commands.

	if (devComPID)
	{
		printf("\n[Info]: Control-C has been pressed. Sending Quit Commands to all Devices\n\n");
		ClientServerQuit(0);
	}
}

