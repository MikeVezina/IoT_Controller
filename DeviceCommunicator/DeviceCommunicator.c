/*
 * DeviceCommunicator.c
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */



#include "DeviceCommunicator.h"

// SendSignalToCloudCommunicator


// Initializes by setting the signal action handler
int InitializeDevCommunicator()
{
	// Create structure for signal action
	struct sigaction sigact = {0};

	// Set signal handler to MessageRecieved function
	sigact.sa_handler = MessageRecieved;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	// Set signal handler to MessageRecieved
	// Sigaction returns -1 on failure
	// SIGINT will be used for identifying when a message has been put into the queue.
	//  - The signal will be sent to the process of who the message was sent to

	if(sigaction(SIGINT,&sigact,0))
	{
		fprintf(stderr,"sigaction failed with error %d\n", errno);
		return -1;
	}


	// Returns the id of the message queue
	return 0;

}

// TODO: DeviceRegistration



// Signal Handler
void MessageRecieved()
{
	// Debug message
	fprintf(stdout, "A Message has been recieved. The queue id is: %d\n", msqid);
	//DeviceRegistrationMessage regMsg;
	//checkForDeviceRegMessage(&regMsg);

	CommandMessage cmd;
		if(!CheckForMessage(&cmd, sizeof(cmd) - sizeof(cmd.msgType),MSG_CMD))
		{
			if(cmd.command[0] == 0xE)
				Exit();
		}







}


// Function that checks for a command message
// The pointer and size passed in, is the structure that is written to by msgrcv
// Returns -1 if error occurs or no message of specified type was found, else returns 0 for success
int CheckForMessage(void *msg, size_t msgsz, int msgtype)
{
	// We want to check the queue for a specified message
	if(msgrcv(msqid, msg, msgsz, msgtype,IPC_NOWAIT) == -1)
	{
		switch(errno)
		{
			// If the error was no message of specified type, no action will be taken and execution will continue
			case ENOMSG:
			{
				// Don't Do anything. The message of the specified type was not found
				break;
			}
			default:
			{
				fprintf(stdout, "Error: msgrcv failed! Error: %d\n", errno);
				break;
			}
		}

		// Error occurred or message was not found
		return -1;
	}
	else
	{
		// Successfully written to passed in structure
		return 0;
	}

}



// Function to release all resources and exit
void Exit()
{
	if(msgctl(msqid,IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "msgctl remove failed. Error: %d\n", errno);
	}
	fprintf(stdout, "Exit");

}


