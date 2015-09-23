/*
 * DeviceCommunicator.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include <sys/msg.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "../Structs.h"

// Message Types
#define MSG_CMD		1 /* Command Message */
#define MSG_DEVREG	2 /* Device Registration Message */
#define MSG_SENINF	3 /* Sensor Information Message */


#ifndef DEVICECOMMUNICATOR_H_
#define DEVICECOMMUNICATOR_H_




int CheckForMessage(void *msg, size_t msgsz, int msgtype);
int InitializeDevCommunicator();
void MessageRecieved();
void Exit();

// Create a global variable for access to queue ID
// Message Queue ID
int msqid;


#endif /* DEVICECOMMUNICATOR_H_ */
