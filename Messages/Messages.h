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
#define MSG_CMD			1 /* Process Command Message */
#define MSG_DEVREG		2 /* Device Registration Message */
#define MSG_SENINF		3 /* Sensor Information Message */
#define MSG_ACTCMD		4 /* Actuator Command Message */
#define MSG_ACTCMDRES	5 /* Actuator Command Response Message */


/* Process Command Message Types */
#define CMD_REGACK 		0xA /* Device Registration Acknowledged Command */
#define CMD_QUIT 		0xE /* Quit Process Command */
#define CMD_FORCEQUIT 	0xF /* Force Quit Process Command */



// Structure for Message Header (All Message Structs must include this)
struct MESSAGEHEADER
{
	long int msgType;
	pid_t sourcePid;
	pid_t destinationPid;

}typedef MESSAGEHEADER;

// Structure for device registration messages
struct DEVICEREGISTRATIONMESSAGE
{
	MESSAGEHEADER msgHdr;
	DEVICEINFO devInfo;

}typedef DEVICEREGISTRATIONMESSAGE;

// Structure for Sensor data messages
struct SENSORDATAMESSAGE
{
	MESSAGEHEADER msgHdr;
	SENSORINFO sensorInfo;

}typedef SENSORDATAMESSAGE;

// Structure for Command Messages
struct PROCESSCOMMANDMESSAGE
{
	MESSAGEHEADER msgHdr;
	char command[1];
}typedef PROCESSCOMMANDMESSAGE;

// Actuator Command Message
struct ACTUATORCOMMANDMESSAGE
{
	MESSAGEHEADER msgHdr;
	char commandSequence;
	ThresholdAction threshAction;

}typedef ACTUATORCOMMANDMESSAGE;

// Actuator Command Response Message
struct ACTUATORCOMMANDRESPONSEMESSAGE
{
	// Standard MessageHeader
	MESSAGEHEADER msgHdr;

	// Command Sequence Number
	char commandSequence;

	// 1 if Command Was processed
	char completedSuccessfully;

	// 0 if no error
	// Else, specified the error number
	char errorNum;


}typedef ACTUATORCOMMANDRESPONSEMESSAGE;

/* Function Prototypes */
int SetMessageHeader(MESSAGEHEADER *msgHdr, pid_t destPid, long int msgType);
int ReceiveMessage(void *msg, size_t msgsz, long int msgtype);
int SendMessage(void *msg, size_t msgsz);
void CloseMessageQueue();

// The key used for the message queue
#define MSGKEY 69169

// Create a global variable for access to
// Message Queue ID:
int msqid;

#endif /* MESSAGES_H_ */
