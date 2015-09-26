/*
 * Structs.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/msg.h>
#include <string.h>

#include "../Devices/Devices.h"

#ifndef MESSAGES_H_
#define MESSAGES_H_

/* Message Types */
#define MSG_CMD		1 /* Command Message */
#define MSG_DEVREG	2 /* Device Registration Message */
#define MSG_SENINF	3 /* Sensor Information Message */

/* Command Message Types */
#define CMD_REGACK 		0xA
#define CMD_QUIT 		0xE
#define CMD_FORCEQUIT 	0xF

// Structure for Message Header (All Message Structs must include this)
struct MESSAGEHEADER
{
	long int msgType;
	pid_t sourcePid;
	pid_t destinationPid;

} typedef MESSAGEHEADER;

// Structure for device registration messages
struct DEVICEREGISTRATIONMESSAGE
{
	MESSAGEHEADER msgHdr;

	DEVICEINFO devInfo;

} typedef DEVICEREGISTRATIONMESSAGE;

// Structure for Sensor data messages
struct SENSORDATAMESSAGE
{
	MESSAGEHEADER msgHdr;
	SENSORINFO sensorInfo;

} typedef SENSORDATAMESSAGE;

// Structure for Command Messages
struct COMMANDMESSAGE
{
	MESSAGEHEADER msgHdr;

	// One byte used for a command
	/*
	 * Commands:
	 * 0xE = Exit
	 */
	char command[1];
} typedef COMMANDMESSAGE;

/* Function Prototypes */
int SetMessageHeader(MESSAGEHEADER *msgHdr,pid_t destPid, long int msgType);
int ReceiveMessage(void *msg, size_t msgsz, long int msgtype);
void CloseMessageQueue();

// The key used for the message queue
#define MSGKEY 69169

// Create a global variable for access to
// Message Queue ID:
int msqid;


#endif /* MESSAGES_H_ */
