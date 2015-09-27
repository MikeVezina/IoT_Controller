/*
 * DeviceCommunicator.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "../Devices/Devices.h"
#include "../Messages/Messages.h"

// Amount of time to wait for device response before timeout has occurred
// Ex: Wait 7 (default) Seconds before determining that the device is unresponsive
#define DEVCLOSE_TIMEOUT 7


#ifndef DEVICECOMMUNICATOR_H_
#define DEVICECOMMUNICATOR_H_

/* Function Prototypes */
int RegisterDevice(DEVICEINFO *deviceInfo);
DEVICEINFO *getRegisteredDevice(pid_t devicePid);
void CheckForMessages();
void ProcessSensorMessage(SENSORDATAMESSAGE *senMsg);
void SendActCommand(ThresholdAction threshAct);
void Quit();
void ForceQuit();


// Create the head of the device linked list
PDEVICELINK headRegisteredDevice;

// Tracks the command sequence number
// So that actions can be completed in sequence by actuators
unsigned int currentCommandSequence;


#endif /* DEVICECOMMUNICATOR_H_ */
