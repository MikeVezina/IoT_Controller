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
				fprintf(stdout, "PID %d: The Device Communicator is now Exiting. (Requested By PID: %d)\n\n", getpid(), cmd.msgHdr.sourcePid);
				Quit();
				break;
			case CMD_FORCEQUIT: /* Request To Force Quit */
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
				fprintf(stderr, "PID %d: Device ACK Failed. SendMessage error: %d", getpid(), errno);
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
		fprintf(stdout, "PID %d: The sensor message does not correspond to a registered Device PID (PID = %d)\n", getpid(), senMsg->msgHdr.sourcePid);
		return;
	}

	// Ensure that only devices that are sensors can send sensor information messages
	if (devInfo->devType != DEVTYPE_SENSOR)
	{
		fprintf(stdout, "PID %d: A Non-Sensor Device Has Sent a Sensor Information Message! (Only sensors can send sensor information messages)", getpid());
		return;
	}

	// Print Sensor Info Received
	fprintf(stdout, "PID %d: Sensor Data Received!\n", getpid());

	// Print Device Information
	PrintDeviceInfo(devInfo);

	// Process The Sensor Information
	PrintSensorInfo(&senMsg->sensorInfo);

	fprintf(stdout, "\n");

	// If the device has a threshold, then check if that threshold has been exceeded
	if (devInfo->hasThreshold)
	{
		if (senMsg->sensorInfo.data[0] > devInfo->threshold)
		{
			printf("** Threshold Exceeded! **\n");
			PrintDeviceInfo(devInfo);
			printf("\n   Current Value (From Sensor): %d\n\n", senMsg->sensorInfo.data[0]);

			switch (devInfo->thresholdAction)
			{
				case THAC_AC_OFF:
				case THAC_AC_ON:
					SendActCommand(ACTTYPE_AC_POWER_SWITCH, devInfo->thresholdAction);
					break;

				case THAC_BELL_RING:
					SendActCommand(ACTTYPE_BELL, devInfo->thresholdAction);
					break;

				case THAC_FIRE_ALARM_RING:
					SendActCommand(ACTTYPE_FIRE_ALARM, devInfo->thresholdAction);
					break;

				case THAC_LIGHT_OFF:
				case THAC_LIGHT_ON:
					SendActCommand(ACTTYPE_LIGHT_SWITCH, devInfo->thresholdAction);
					break;

					// Default action is to do nothing
				case THAC_NONE:
				default:
					break;

			}

		}
	}

	fprintf(stdout, "\n");
}



// Sends a Command To An Actuator
void SendActCommand(ActuatorType actType, ThresholdAction threshAct)
{
	printf("Actuator Type: %d, Action: %d\n", actType, threshAct);

	// It is assumed that if we have more than one of the same type of actuator,
	// That the Threshold action will be applied to all of the actuators. For example:
	// If smoke is detected, turn on ALL Sprinklers

	ACTUATORCOMMANDMESSAGE actCmd;
	SetMessageHeader(&actCmd.msgHdr, 0, MSG_ACTCMD);

	actCmd.commandSequence = currentCommandSequence++;
	actCmd.threshAction = threshAct;

	if(!SendMessage((void *) &actCmd, sizeof(actCmd) - sizeof(actCmd.msgHdr)))
	{
		printf("Command Sent!");
	}


}



// Prints the device info to stdout
void PrintDeviceInfo(DEVICEINFO *devInfo)
{
	if (devInfo == 0)
		return;

	// Print out the different fields of device information
	fprintf(stdout, "- Device Information:\n   Device PID:\t%d\n   Device Name:\t%s\n   Device Type:\t%d\n   Threshold? \t%c\n", devInfo->pid, devInfo->devName, devInfo->devType, (devInfo->hasThreshold) ? 'Y' : 'N');

	// Only print the device threshold if the device has a threshold
	if (devInfo->hasThreshold)
		fprintf(stdout, "   Threshold:\t%d", devInfo->threshold);

	fprintf(stdout, "\n");
}

// Prints the Sensor Data to stdout
void PrintSensorInfo(SENSORINFO *senInfo)
{
	if (senInfo == 0)
		return;

	fprintf(stdout, "\n- Sensor Info:\n");
	for (int i = 0; i < (sizeof(senInfo->data) / sizeof(senInfo->data[0])); i++)
	{
		fprintf(stdout, "   [%d]: %d\n", i, senInfo->data[i]);
	}
}

// Function to release all resources and exit
void Quit()
{
// After ensuring all of the messages are processed, ForceQuit is called
	ForceQuit();
}

// ForceQuit quits the process without processing any remaining messages
void ForceQuit()
{

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
