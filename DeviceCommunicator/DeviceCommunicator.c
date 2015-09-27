/*
 * DeviceCommunicator.c
 *	- The DeviceCommunicator is the child process of the Controller.
 *		- This file is responsible for all functions that look for messages in the message queue from all devices
 *		- The functions are responsible for registering devices, processing sensor data and sending commands to devices for
 *			completing actions. (Such as executing actions and exiting)
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include "DeviceCommunicator.h"

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
	if (!headRegisteredDevice)
	{
		headRegisteredDevice = pDevLink;
		return 0;
	}

	// Iterate to the last link, checking to see if the device PID already exists
	PDEVICELINK pCurrentLink = headRegisteredDevice;

	// Makes sure the device pid does not exist in the head device
	if (pCurrentLink->devInfo.pid == pDevLink->devInfo.pid)
	{
		free(pDevLink);
		return -1;
	}

	// Iterates until the last element is pointed to by pCurrentLink
	while (pCurrentLink->nextDeviceLink)
	{

		// If the device PID already exists in an element in the list, dont register the device and return error -1
		if (pCurrentLink->devInfo.pid == pDevLink->devInfo.pid)
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

// Unregisters the device from the linked list
int UnregisterDevice(pid_t devicePID)
{
	// We look for the node before the node with devicePID in the linked list

	// Check to make sure the head node is not null
	if (!headRegisteredDevice)
	{
		printf("No Registered Devices. \n");
		return -1;
	}

	// Case 1: The registered device is the head node
	if (headRegisteredDevice->devInfo.pid == devicePID)
	{

		// Keep a reference to any remaining device links
		PDEVICELINK next = headRegisteredDevice->nextDeviceLink;

		// Free the head node
		free(headRegisteredDevice);

		// Set the headRegisteredDevice to the next device
		headRegisteredDevice = next;

		return 0;
	}

	// Case 2: Iterate through the devices and remove the link
	PDEVICELINK current = headRegisteredDevice;

	// Iterate while the next device link is not null
	while (current->nextDeviceLink)
	{
		if (current->nextDeviceLink->devInfo.pid == devicePID)
		{
			// Get the link after the devicelink to be deleted
			PDEVICELINK pNextLink = current->nextDeviceLink->nextDeviceLink;

			// Free the device link
			free(current->nextDeviceLink);

			// Set the next link to the link after the deleted device link
			current->nextDeviceLink = pNextLink;

			return 0;
		}

		current = current->nextDeviceLink;
	}

	return -1;
}

// Returns 0 if the device pid is NOT a registered device
// Returns the pointer to the DeviceInfo in a DeviceLink that is part of the registered devices linked list
DEVICEINFO *getRegisteredDevice(pid_t devicePid)
{
	// If the head device is null, the device pid is not registered
	if (!headRegisteredDevice)
	{
		return (void *) 0;
	}

	// Iterates through linked list to check if the
	PDEVICELINK currentLink = headRegisteredDevice;
	while (currentLink)
	{
		if (currentLink->devInfo.pid == devicePid)
			return &currentLink->devInfo;

		currentLink = currentLink->nextDeviceLink;
	}
	return (void *) 0;

}

// Checks for messages in the message queue
void CheckForMessages()
{
	// Do seperate checks for each of the different types of messages

	// Check for Command Messages
	PROCESSCOMMANDMESSAGE cmd;
	if (!ReceiveMessage(&cmd, sizeof(cmd) - sizeof(cmd.msgHdr.msgType), MSG_CMD))
	{

		switch (cmd.command[0])
		{
			case CMD_REGACK: /* Device Registration Acknowledged */
				break;
			case CMD_QUIT: /* Request To Quit Application */
				fprintf(stdout, "PID %d: The Device Communicator is now Exiting. (Requested By PID: %d)\n\n*********** Quitting Devices ***********\n\n", getpid(), cmd.msgHdr.sourcePid);
				Quit();
				break;
			case CMD_FORCEQUIT: /* Request To Force Quit */
				ForceQuit();
				break;
			case CMD_CLOSEACK: /* Close Acknowledgement from device. The device is quitting without request */

				if (UnregisterDevice(cmd.msgHdr.sourcePid))
				{
					printf("PID %d: Failed to unregister device (PID: %d).\n", getpid(), cmd.msgHdr.sourcePid);
				}
				else
				{
					printf("PID %d: Successfully unregistered device (PID: %d)\n", getpid(), cmd.msgHdr.sourcePid);
				}

				break;
		}
	}

	ACTUATORCOMMANDRESPONSEMESSAGE acrm;

	// Proces all actuator command response messages
	while (!ReceiveMessage(&acrm, sizeof(acrm) - sizeof(acrm.msgHdr.msgType), MSG_ACTCMDRES))
	{
		printf("Actuator command has been processed! Command Sequence: %d. Status: %d. Error Number: %d\n\n", acrm.commandSequence, acrm.completedSuccessfully, acrm.errorNum);
	}

	// A while loop is used to ensure that the DeviceCommunicator
	// process all pending device registration messages before proceeding to obtaining sensor data
	DEVICEREGISTRATIONMESSAGE regMsg;
	while (!ReceiveMessage(&regMsg, sizeof(regMsg) - sizeof(regMsg.msgHdr.msgType), MSG_DEVREG))
	{
		if (!RegisterDevice(&regMsg.devInfo))
		{
			fprintf(stdout, "PID %d: A New Device Has Been Registered!\n", getpid());
			PrintDeviceInfo(&regMsg.devInfo);
			printf("\n");

			// Create a device registration acknowledgement command
			PROCESSCOMMANDMESSAGE cmdMsg;

			SetMessageHeader(&cmdMsg.msgHdr, regMsg.devInfo.pid, MSG_CMD);
			cmdMsg.command[0] = CMD_REGACK;

			// Sends the Registration acknowledgement to the device
			if (SendMessage(&cmdMsg, sizeof(cmdMsg) - sizeof(cmdMsg.msgHdr.msgType)))
			{
				printf("PID %d: Device ACK Failed. SendMessage error: %d", getpid(), errno);

				// We force quit because this will only occur when there is a problem with the message queue
				ForceQuit();
			}

		}
	}
	// Receives and Processes Sensor Data message
	SENSORDATAMESSAGE senMsg;
	if (!ReceiveMessage(&senMsg, sizeof(senMsg) - sizeof(senMsg.msgHdr.msgType), MSG_SENINF))
	{
		ProcessSensorMessage(&senMsg);
	}

}

// Processes the sensor message and any sensor information
void ProcessSensorMessage(SENSORDATAMESSAGE *senMsg)
{
	// Ensure senMsg is not null
	if (!senMsg)
	{
		fprintf(stderr, "ProcessSensorMessage(): *senMsg is null");
		return;
	}

	// Get the device info from the received sensor message
	DEVICEINFO *devInfo = getRegisteredDevice(senMsg->msgHdr.sourcePid);

	// If devInfo is null, return without processing the message
	if (!devInfo)
	{
		return;
	}

	// Ensure that only devices that are sensors can send sensor information messages
	if (devInfo->devType != DEVTYPE_SENSOR)
	{
		fprintf(stdout, "PID %d: A Non-Sensor Device Has Sent a Sensor Information Message! (Only sensors can send sensor information messages)", getpid());
		return;
	}

	// Print Sensor Info Received
	fprintf(stdout, "***  PID %d: Sensor Data Received From PID %d!  ***\n", getpid(), devInfo->pid);

	// Process The Sensor Information
	PrintSensorInfo(&senMsg->sensorInfo);

	// Add New Line After Sensor Info
	fprintf(stdout, "\n");

	// If the device has a threshold, then check if that threshold has been exceeded
	if (devInfo->hasThreshold)
	{
		if (senMsg->sensorInfo.data[0] > devInfo->threshold)
		{
			// Print Valid Information
			printf("* WARNING: Threshold Exceeded! *\n");
			PrintDeviceInfo(devInfo);

			// Use a variable to track the difference in sent sequences.
			// If the difference is 0, No actuators are responsible for the selected action
			int currentCMDSeq = currentCommandSequence;

			printf("\n");

			// Send Threshold Actions to responding actuators
			SendActCommand(devInfo->thresholdAction);

			// If the difference is 0, No threshold action was executed
			if (!(currentCommandSequence - currentCMDSeq))
			{
				printf("No Actuators are responsible for the specified threshold action (Action: %x) or No Actuators have been registered.\n", devInfo->thresholdAction);
			}

			THRESHOLDCROSSINGMESSAGE tcm;
			SetMessageHeader(&tcm.msgHdr, getppid(), MSG_THRESHCROSS);
			tcm.devInfo = *devInfo;
			tcm.sensInfo = senMsg->sensorInfo;

			// And now want to notify the parent of the threshold crossing through a user defined signal
			// sendmessage -> device sensor info, sensing data, action
			if (!SendMessage((void *) &tcm, sizeof(tcm) - sizeof(tcm.msgHdr.msgType)))
			{
				printf("\nPID %d: Notifying Cloud Communicator..\n", getpid());
				// Send the user defined signal to the parent
				kill(getppid(), SIGUSR2);
			}

		}
	}


}

// Sends a Command To An Actuator
void SendActCommand(ThresholdAction threshAct)
{

	// It is assumed that if we have more than one of the same type of actuator,
	// That the Threshold action will be applied to all of the actuators. For example:
	// If smoke is detected, turn on ALL Sprinklers

	// Look through all devices to check for actuators that handle the specified threshold action

	PDEVICELINK pDevLink = headRegisteredDevice;
	while (pDevLink)
	{
		// Ensure the device is an Actuator and that it handles the specified threshold action
		// Using the & bit-wise operator to check if any of the relevant bits are set
		// If the actuator handles 1 (or more) of the matching threshold actions, send a message command
		if (pDevLink->devInfo.devType == DEVTYPE_ACTUATOR && (pDevLink->devInfo.thresholdAction & threshAct))
		{
			ACTUATORCOMMANDMESSAGE actCmd;
			SetMessageHeader(&actCmd.msgHdr, pDevLink->devInfo.pid, MSG_ACTCMD);

			// Set the command Sequence
			actCmd.commandSequence = currentCommandSequence++;

			// Set the threshold action
			actCmd.threshAction = pDevLink->devInfo.thresholdAction & threshAct;

			if (!SendMessage((void *) &actCmd, sizeof(actCmd) - sizeof(actCmd.msgHdr.msgType)))
			{
				printf("Command Sent to PID %d ('%s')! Sequence: %d, Action: 0x%x\n", actCmd.msgHdr.destinationPid, pDevLink->devInfo.devName, actCmd.commandSequence, (pDevLink->devInfo.thresholdAction & threshAct));
			}

		}

		pDevLink = pDevLink->nextDeviceLink;
	}

}

// Function to release all resources and exit
void Quit()
{
	// Send Quit Command to all registered devices
	PDEVICELINK pDevLink = headRegisteredDevice;
	while (pDevLink)
	{
		// Sends the device process a quit command message
		if (SendProcessCommand(CMD_QUIT, pDevLink->devInfo.pid))
		{
			fprintf(stderr, "PID %d: Failed To Send Process Command Signal to Device PID: %d\n", getpid(), pDevLink->devInfo.pid);
		}

		printf("PID %d: Quit Command Sent To Device %d. Waiting for close acknowledgement from device..\n", getpid(), pDevLink->devInfo.pid);

		// Wait for a process quit acknowledgement command message from the Device
		PROCESSCOMMANDMESSAGE cmd;

		// Set a timeout limit to wait for close acknowledgement
		time_t startTime = time(0);
		while (ReceiveMessage(&cmd, sizeof(cmd) - sizeof(cmd.msgHdr.msgType), MSG_CMD) || cmd.command[0] != CMD_CLOSEACK || cmd.msgHdr.sourcePid != pDevLink->devInfo.pid)
		{
			if (time(0) - startTime >= DEVCLOSE_TIMEOUT)
			{

				printf("PID %d: Device Quit Acknowledgement Timeout (Response Time Exceeded %d Seconds).\nDevice '%s' (PID: %d) failed to quit or is already closed.\n\n", getpid(), DEVCLOSE_TIMEOUT, pDevLink->devInfo.devName, pDevLink->devInfo.pid);
				break;
			}

			usleep(50000);
		}

		if (cmd.command[0] == CMD_CLOSEACK)
		{
			printf("PID %d: Device '%s' (PID %d) has quit Successfully!\n\n", getpid(), pDevLink->devInfo.devName, cmd.msgHdr.sourcePid);
		}

		// Continue to quit next device
		pDevLink = pDevLink->nextDeviceLink;
	}

// After ensuring all of the messages are processed, ForceQuit is called
	ForceQuit();
}

// ForceQuit quits the process without sending any exit request commands to the devices

void ForceQuit()
{
	printf("*********** End Quitting Devices ***********\n\n");

// Free all memory allocated for device linked list
	if (headRegisteredDevice)
	{
		PDEVICELINK pCurrentLink = headRegisteredDevice;

		// Iterate through linked list and free memory allocated
		PDEVICELINK pNextLink = pCurrentLink->nextDeviceLink;

		while (pCurrentLink)
		{
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
