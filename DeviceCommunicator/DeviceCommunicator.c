/*
 * DeviceCommunicator.c
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include "DeviceCommunicator.h"

// SendSignalToCloudCommunicator

// Adds the device to the linked list
int RegisterDevice(DEVICEINFO *deviceInfo)
{
	// Check to make sure deviceInfo is not null
	if (!deviceInfo)
		return -1;

	// Check to make sure a valid PID was specified
	if (deviceInfo->pid == 1)
		return -1;

	// We need to allocate memory and copy the argument to the allocated memory
	// This way we dont run into memory problems (brought along from locally defined structures - which get disposed of when the function returns)

	PDEVICELINK pDevLink = malloc(sizeof(DEVICELINK));

	// If no memory could be allocated for pDevLink, return an error
	if (!pDevLink)
	{
		return -1;
	}

	// Copy the struct pointed by deviceInfo into the device info of the link (pDevLink->devInfo)
	memcpy(&pDevLink->devInfo, deviceInfo, sizeof(*deviceInfo));

	// Set the next device link = null
	pDevLink->nextDeviceLink = 0;

	// If the head device is null, set it to the devicelink
	if (!headDevice)
	{
		headDevice = pDevLink;
		return 0;
	}

	// Iterate to the last link, checking to see if the device PID already exists
	PDEVICELINK pCurrentLink = headDevice;
	while (pCurrentLink->nextDeviceLink)
	{
		// If the device PID already exists, dont register the device and return error -1
		if (pCurrentLink->devInfo.pid == deviceInfo->pid)
		{
			free(pDevLink);
			return -1;
		}

		// Set the current link to the next link
		pCurrentLink = pCurrentLink->nextDeviceLink;
	}

	// Set the next device link to the created device link
	pCurrentLink->nextDeviceLink = pDevLink;
	return 0;
}

// Returns 0 if the device pid is NOT a registered device
// Returns 1 if the device pid IS a registered device
int isRegisteredDevice(pid_t devicePid)
{
	// If the head device is null, the device pid is not registered
	if (!headDevice)
		return 0;

	// Iterates through linked list to check if the
	PDEVICELINK currentLink = headDevice;
	while (currentLink)
	{
		if (currentLink->devInfo.pid == devicePid)
			return 1;

		currentLink = currentLink->nextDeviceLink;
	}
	return 0;

}

// Checks for messages in the message queue
void CheckForMessages()
{
	// Do seperate checks for each of the different types of messages

	// Check for Command Messages
	COMMANDMESSAGE cmd;
	if (!ReceiveMessage(&cmd, sizeof(cmd) - sizeof(cmd.msgHdr.msgType), MSG_CMD))
	{

		switch (cmd.command[0])
		{
			case CMD_REGACK: /* Device Registration Acknowledged */
				break;
			case CMD_QUIT: /* Device Registration Acknowledged */
				fprintf(stdout, "The Device Communicator is now Exiting.\n");
				Quit();
				break;
			case CMD_FORCEQUIT:
				ForceQuit();
				break;
		}
	}

	// A while loop is used to ensure that the DeviceCommunicator
	// process all pending device registration messages before proceeding to obtaining sensor data
	DEVICEREGISTRATIONMESSAGE regMsg;
	while (!ReceiveMessage(&regMsg, sizeof(regMsg) - sizeof(regMsg.msgHdr.msgType), MSG_DEVREG))
	{
		if (!RegisterDevice(&regMsg.devInfo))
		{
			fprintf(stdout, "Registered Device: %s\n", regMsg.devInfo.devName);

			COMMANDMESSAGE cmdMsg;
			SetMessageHeader(&cmdMsg.msgHdr, regMsg.devInfo.pid, MSG_CMD);
			cmdMsg.command[0] = CMD_REGACK;

			// Now we want to send a device acknowledgement telling the device it is registered
		}
	}

	SENSORDATAMESSAGE senMsg;
	if (!ReceiveMessage(&senMsg, sizeof(senMsg) - sizeof(senMsg.msgHdr.msgType), MSG_SENINF))
	{
		//ProcessSensorMessage();
		fprintf(stdout, "Sensor Data Received from: %s\n", regMsg.devInfo.devName);
	}

}

// Function to release all resources and exit
void Quit()
{
	// TODO: Before exiting, ensure that all other messages in the message queue are processed

	// After ensuring all of the messages are processed, ForceQuit is called
	ForceQuit();
}

// ForceQuit quits the process without processing any remaining messages
void ForceQuit()
{

	// Free all memory allocated for device linked list
	if (headDevice)
	{
		PDEVICELINK pCurrentLink = headDevice;
		// Iterate until the last link
		PDEVICELINK pNextLink = pCurrentLink->nextDeviceLink;

		while (pCurrentLink)
		{

			printf("Freeing Device.. %s\n", pCurrentLink->devInfo.devName);
			free(pCurrentLink);

			if (!pNextLink)
				break;

			pCurrentLink = pNextLink;
			pNextLink = pCurrentLink->nextDeviceLink;
		}
	}

	CloseMessageQueue();
	exit(0);
}
