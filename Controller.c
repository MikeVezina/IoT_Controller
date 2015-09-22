/*
 * Controller.c
 *
 *	The Controller contains two main components.
 *
 *	1. The Parent process will communicate with the cloud component (using FIFO -> the cloud = server & parent process = client)
 *		- The parent process may catch a signal sent by the child process and read a message from the queue. It sends the info
 *				to the cloud process
 *		- The parent process may receive commands from the user (the cloud process).
 *			- The command is either get or put
 *				- Get = request info from sensor
 *				- put =
 *
 *
 *	2. The Child process will be used to send and receive device messages (from sensors or actuators) using a message queue
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

int main(int argsv, char *args[]){

	// Start off the assignment


}
