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

#include "../Devices/Devices.h"
#include "../Messages/Messages.h"



#ifndef DEVICECOMMUNICATOR_H_
#define DEVICECOMMUNICATOR_H_

/* Function Prototypes */
int RegisterDevice(DEVICEINFO *deviceInfo);
DEVICEINFO *getRegisteredDevice(pid_t devicePid);
void CheckForMessages();
void ProcessSensorMessage(SENSORDATAMESSAGE *senMsg);
void SendActCommand(ActuatorType actType, ThresholdAction threshAct);
void PrintDeviceInfo(DEVICEINFO *devInfo);
void PrintSensorInfo(SENSORINFO *senInfo);
void Quit();
void ForceQuit();


// Create the head of the device linked list
PDEVICELINK headRegisteredDevice;

// Tracks the command sequence number
// So that actions can be completed in sequence by actuators
int currentCommandSequence;


#endif /* DEVICECOMMUNICATOR_H_ */
