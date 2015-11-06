/*
 * ClientServer.h
 *
 *	Connection is used for basic client / server connections through IPC (Named Pipes)
 *
 *  Created on: Sep 27, 2015
 *      Author: MVezina
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../Devices/Devices.h"
#include "../Messages/Messages.h"

#ifndef CLIENTSERVER_H_
#define CLIENTSERVER_H_

/* Defines */
#define SERVERPIPEPATH "/tmp/server_pipe" 		/* Server Pipe Name */
#define CLIENTPIPEPATH "/tmp/client_%d_pipe"	/* Client Pipe Name (Where %d is the PID of the client) */
#define CLIENT_PIPE_NAME_LENGTH 30				/* Maximum Client Pipe Name Length (Including PID) */

/* The Types of Requests that can be made through Pipes */
enum REQUEST
{
	REQUEST_ACTUATOR_ACTION = 0,
	REQUEST_DEVICE_INFO,
	REQUEST_SENSOR_INFO,
	REQUEST_THRESHOLD_EXCEEDED,
	REQUEST_READ_SENSOR_INFO,
	REQUEST_READ_DEVICE_INFO
}typedef REQUEST;

/* The Different Type of responses */
enum RESPONSE
{
	RESPONSE_SUCCESS = 0,
	RESPONSE_FAILED,
	RESPONSE_USER
}typedef RESPONSE;

/* The Type of data message (Either Request or Response) */
enum DATAMSGTYPE
{
	MSG_REQUEST = 0,
	MSG_RESPONSE
}typedef DATAMSGTYPE;

/* Data Message to describe request / response messages */
struct DATAMESSAGE
{
	DATAMSGTYPE type;
	pid_t clientPID;
	RESPONSE response;
	REQUEST request;
}typedef DATAMESSAGE;

/* Function Prototypes */
int CreateAndOpenPipe(const char* pipeName, int openMode);
void CreatePipe(const char* pipeName);
int OpenPipe(const char* pipeName, int openMode);
int ReadFromPipe(int pipe_fd, const char *pipeName, void *msg, size_t szMsg);
int WriteToPipe(int pipe_fd, const char* pipeName, void *msg, size_t szMsg);
extern void ClientServerQuit(int status);

#endif /* CLIENTSERVER_H_ */
