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


char controllerName[25];

void TestMessageSystem()
{
	pid_t devComPid;

	DEVICEREGISTRATIONMESSAGE devMsg;
	SetMessageHeader((void *)&devMsg,0,MSG_DEVREG);

	// Set Device Name
	strcpy(devMsg.devInfo.devName, "TEMPSENSOR1");


	// Set Device Information
	devMsg.devInfo.pid = getpid();
	devMsg.devInfo.devType = DEVTYPE_SENSOR;
	devMsg.devInfo.sensType = SENSTYPE_TEMPERATURE;
	devMsg.devInfo.hasThreshold = 1;
	devMsg.devInfo.threshold = 25;
	devMsg.devInfo.thresholdAction = THAC_AC_ON;

	// Send a message to the message queue
	if (!SendMessage((void *) &devMsg, sizeof(devMsg) - sizeof(devMsg.msgHdr.msgType)))
	{
		printf("PID %d: Device Registration Message Sent.\n-- Waiting for Registration Acknowledgement --\n\n", getpid());
	}


	PROCESSCOMMANDMESSAGE cmd;

	// Wait for device registration ack from DeviceCommunicator
	while(ReceiveMessage(&cmd,sizeof(cmd)-sizeof(cmd.msgHdr.msgType),MSG_CMD) || cmd.command[0] != CMD_REGACK);

	// Ack has been recieved. The device is now registered!
	printf("PID %d: -- Ack Received! --\n",getpid());

	// Set the devComPid to source pid
	devComPid = cmd.msgHdr.sourcePid;



	SENSORDATAMESSAGE sdm;
	SetMessageHeader((void *)&sdm,devComPid,MSG_SENINF);
	sdm.sensorInfo.data[0] = 26;

	// Send a message to the message queue
	if (!SendMessage((void *) &sdm, sizeof(sdm) - sizeof(sdm.msgHdr.msgType)))
	{
		printf("PID %d: Device Sensor Message Sent\n\n", getpid());
	}

	sleep(1);

	PROCESSCOMMANDMESSAGE cmdMsg;
	SetMessageHeader(&cmdMsg.msgHdr, devComPid, MSG_CMD);
	cmdMsg.command[0] = CMD_QUIT;


	// Send a message to the message queue
	if (!SendMessage((void *) &cmdMsg, sizeof(cmdMsg) - sizeof(cmdMsg.msgHdr.msgType)))
	{
		printf("PID %d: Quit Command Sent To %d\n", getpid(), cmdMsg.msgHdr.destinationPid);

	}
}

int main(int argsv, char *args[])
{
	// Checks to make sure number of arguments is 2 for controller name
	if (argsv != 2)
	{
		printf("Usage: IoT_Controller [ControllerName]\n\t[ControllerName]: The name of the controller. Maximum of 24 Characters\n");
		exit(1);
	}

	// Ensures Controller Name is 24 or less characters
	if (strlen(args[1]) > 24)
	{
		fprintf(stdout, "The controller name must be a maximum of 24 characters\n");
		exit(1);
	}

	// Copy Controller Name argument into controller name variable
	strcpy(controllerName, args[1]);

	// Controller has started message
	printf("========================================\nController '%s': has started!\n", controllerName);

	// Initialize the message queue
	msqid = msgget((key_t) MSGKEY, 0666 | IPC_CREAT);

	// Ensure the message queue was initialized successfully
	if (msqid == -1)
	{
		fprintf(stderr, "msgget failed with error %d\n", errno);
		return -1;
	}

	// Create the child process through fork()
	pid_t childpid = fork();

	// Use switch cases to determine if fork completed successfully, and to determine
	// who the parent and child processes are
	switch (childpid)
	{
		// fork failed
		case -1:
		{
			fprintf(stderr, "Fork failed with error code %d. The Controller failed to start.\n", errno);
			return errno;
		}
			// Child Process
		case 0:
		{

			while (1)
			{
				// We want to check for messages twice a second
				CheckForMessages();

				// 10^3 microseconds in 1 milliseconds
				// Therefore 500*10^3 = 500 milliseconds = 0.5 seconds
				usleep(500 * (10 ^ 3));

				// NOTE: We can exit out of the child process by sending it an exit command
			}

			break;
		}
			// Parent process where pid = PID of child
		default:
		{
			// Print Child and Parent PID
			printf("Child PID: %d\nParent PID: %d\n========================================\n\n", childpid, getpid());

			// Tests the message system by sending test messages
			TestMessageSystem();

			// Wait for child process to exit
			wait((void*) 0);

			break;
		}
	}

	fprintf(stdout, "PID %d: has Quit\n", getpid());
	return 0;

}

