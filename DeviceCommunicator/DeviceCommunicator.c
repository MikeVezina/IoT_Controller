/*
 * DeviceCommunicator.c
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */



#include "DeviceCommunicator.h"
// SendSignalToCloudCommunicator


// Initializes by setting the signal action handler and initializing the message queue
// This returns -1 if an error occurred, otherwise it returns the messagequeue id
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

	if(sigaction(SIGINT,sigact,0))
	{
		sprintf(stderr,"sigaction failed with error %d\n", errno);
		return -1;
	}

	// Initializes the message queue, checks for errors and returns either -1 for an error
	// or the id for the message queue
	int msgid = InitializeMessageQueue();
	if(msgid == -1)
	{
		sprintf(stderr, "InitializeMessageQueue failed with error %d\n", errno);
		return -1;
	}

	// Returns the id of the message queue
	return msgid;

}

// TODO: DeviceRegistration



// Signal Handler
void MessageRecieved()
{
	// Debug message
	sprintf(stdout, "A Message has been recieved");

	// Read message from message queue
	// Echo Information
}

// Initializes Message Queue and returns the Queue identifier
int InitializeMessageQueue()
{
	return msgget((key_t)4551, IPC_CREAT);
}

