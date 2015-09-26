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
int isRegisteredDevice(pid_t devicePid);
void CheckForMessages();
void Quit();
void ForceQuit();


// Create the head of the device linked list
PDEVICELINK headDevice;


#endif /* DEVICECOMMUNICATOR_H_ */
