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
 *				- put =
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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "CloudCommunicator/CloudCommunicator.h"
#include "DeviceCommunicator/DeviceCommunicator.h"


// Array of all registered device information
DeviceInfo registeredDeviceInfo[];



int main(int argsv, char *args[]){

	// Create the child process through fork()


	pid_t pid = fork();


	// Use switch cases to determine if fork completed successfully, and to determine
	// who the parent and child processes are

	switch(pid)
	{
		// fork failed
		case -1:
			sprintf(stderr, "Fork failed with error code %d. The Controller will quit.", errno);
			return errno;

		// Child Process
		case 0:
			// ChildInitialization
			printf("Yes, this is the child.");
			break;

		// Parent process where pid = PID of child
		default:
			//
			printf("This is the parent. If you're looking for the child, its at PID: %d.", pid);
			break;
	}




	return 0;


}
