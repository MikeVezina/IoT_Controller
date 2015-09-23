/*
 * DeviceCommunicator.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include <sys/msg.h>
#include <signal.h>
#include <errno.h>

#ifndef DEVICECOMMUNICATOR_H_
#define DEVICECOMMUNICATOR_H_

int InitializeDevCommunicator()
void MessageRecieved();
int InitializeMessageQueue();



#endif /* DEVICECOMMUNICATOR_H_ */
