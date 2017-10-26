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
enum MSGTYPE
{
	MSG_CMD = 1,	/* Process Command Message */
	MSG_DEVREG,		/* Device Registration Message */
	MSG_USRREQ,		/* User Request Message */
	MSG_USRRES,		/* User Response Message */
	MSG_SENINF,		/* Sensor Information Message */
	MSG_ACTCMD,		/* Actuator Command Message */
	MSG_ACTCMDRES,	/* Actuator Command Response Message */
	MSG_THRESHCROSS,/* Threshold Crossing Message */
	MSG_DEVINFO,	/* Device Info Message */
	MSG_DEVRES,		/* Device Response for Request for Information */
};

enum ERRORRESPONSE
{
	ERR_DEVICE_PID_NOT_FOUND = -5,
}typedef ERRORRESPONSE;
/* Process Command Message Types */
enum PROCESSCOMMAND
{
	CMD_REGACK		=	0xA,/* Device Registration Acknowledged Command */
	CMD_SENREQ		=	0xB,/* Sensor Request Message */
	CMD_ACTREQ		=	0xC,/* Actuator Request Message */
	CMD_CLOSEACK	=	0xD,/* Device Close Acknowledged */
	CMD_QUIT		=	0xE,/* Quit Process Command */
	CMD_FORCEQUIT 	=	0xF	/* Force Quit Process Command */
};


// Structure for Message Header (All Message Structs must include this)
struct MESSAGEHEADER
{
	long int msgType;
	pid_t sourcePid;
	pid_t destinationPid;

}typedef MESSAGEHEADER;

/* The structure for a User Request Message from the mobile user */
struct USERREQUESTMESSAGE{
	MESSAGEHEADER msgHdr;
	pid_t devicePID;
	DeviceType devType;
	ThresholdAction thac;
} typedef USERREQUESTMESSAGE;

/* The structure for a User Response Message from the mobile user */
struct USERRSPONSEMESSAGE{
	MESSAGEHEADER msgHdr;
	char status;
	pid_t devicePID;
	DeviceType devType;
	SENSORINFO senInfo;
} typedef USERRESPONSEMESSAGE;

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

}typedef SENSORDATAMESSAGE;

// Structure for Command Messages
struct PROCESSCOMMANDMESSAGE
{
	MESSAGEHEADER msgHdr;
	char command;
	char data[1];
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

// Threshold Crossing Message
struct THRESHOLDCROSSINGMESSAGE
{
	MESSAGEHEADER msgHdr;
	DEVICEINFO devInfo;
	SENSORINFO sensInfo;

} typedef THRESHOLDCROSSINGMESSAGE;

/* Function Prototypes */
int SetMessageHeader(MESSAGEHEADER *msgHdr, pid_t destPid, long int msgType);
int ReceiveMessage(void *msg, size_t msgsz, long int msgtype);
int SendMessage(void *msg, size_t msgsz);
int SendProcessCommand(char command, pid_t destinationPID);
void CloseMessageQueue();

// The key used for the message queue
#define MSGKEY 69169

// Create a global variable for access to
// Message Queue ID:
int msqid;

#endif /* MESSAGES_H_ */
