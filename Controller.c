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
 *		- ... (See assignment hand out)
 *
 *
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#include "Structs.h"
#include "CloudCommunicator/CloudCommunicator.h"
#include "DeviceCommunicator/DeviceCommunicator.h"

#define MSGKEY 69169



// Array of all registered device information
//DeviceInfo registeredDeviceInfo[2];


int main(int argsv, char *args[]){



	// Initialize the message queue
	msqid = msgget((key_t)MSGKEY, 0666 | IPC_CREAT);


	// Ensure the message queue was initialized successfully
	if(msqid == -1)
	{
		fprintf(stderr, "msgget failed with error %d\n", errno);
		return -1;
	}

	// Create the child process through fork()
	pid_t childpid = fork();


	// Use switch cases to determine if fork completed successfully, and to determine
	// who the parent and child processes are
	switch(childpid)
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

			while(1)
			{
				// We want to check for messages twice a second
				CheckForMessages();

				// 10^3 microseconds in 1 milliseconds
				// Therefore 500*10^3 = 500 milliseconds = 0.5 seconds
				usleep(500*(10^3));

				// We can exit out of the child process by sending it an exit command
			}

			break;
		}
		// Parent process where pid = PID of child
		default:
		{
			// Print Child and Parent PID
			printf("Child PID: %d\nParent PID: %d\n", childpid, getpid());


			DeviceRegistrationMessage devMsg;
			strcpy(devMsg.devInfo.devName, "asd");
			devMsg.msgHdr.msgType = MSG_DEVREG;



			// Send a message to the message queue
			if(msgsnd(msqid, (void *) &devMsg,sizeof(devMsg)-sizeof(devMsg.msgHdr.msgType),0) == -1)
			{
				fprintf(stderr, "msgsnd failed, err: %d\n", errno);
			}
			else
			{
				printf("Message Sent...\n");
			}

			sleep(2);

			CommandMessage cmdMsg;
			cmdMsg.msgHdr.msgType = MSG_CMD;
			cmdMsg.command[0] = 0xE;
			cmdMsg.msgHdr.destinationPid = childpid;
			cmdMsg.msgHdr.sourcePid = getpid();

			// Send a message to the message queue
			if(msgsnd(msqid, (void *) &cmdMsg,sizeof(cmdMsg)-sizeof(cmdMsg.msgHdr.msgType),0) == -1)
			{
				fprintf(stderr, "msgsnd failed, err: %d\n", errno);
			}
			else
			{
				printf("Message Sent...\n");
			}

			wait((void*)0);

			break;
		}
	}



	fprintf(stdout, "%d has quit!\n", getpid());
	return 0;


}


