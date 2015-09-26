/*
 * Messages.c
 *
 *  Created on: Sep 23, 2015
 *      Author: MVezina
 */

#include "Messages.h"

int SetMessageHeader(MESSAGEHEADER *msgHdr, pid_t destPid, long int msgType)
{
	// If msgHeader is null
	if (!msgHdr)
		return -1;

	msgHdr->sourcePid = getpid();
	msgHdr->destinationPid = destPid;

	return 0;
}

// Function that checks for a message in the queue
// Uses msgrcv to write any available messages of msgtype into the pointer specified with the specified size
// Returns -1 if error occurs or no message of specified type was found, else returns 0 for success
int ReceiveMessage(void *msg, size_t msgsz, long int msgtype)
{
	// Check if msg is null
	if (!msg)
		return -1;

	// We want to check the queue for a specified message
	if (msgrcv(msqid, msg, msgsz, msgtype, IPC_NOWAIT) == -1)
	{
		switch (errno)
		{
			// If the error was no message of specified type, no action will be taken and execution will continue
			case ENOMSG:
			{
				// Don't Do anything. The message of the specified type was not found
				break;
			}
			default:
			{
				fprintf(stderr, "Error: msgrcv(msgtype: %d) failed! Error Number: %d. Error Desc.:%s\n", msgtype, errno, strerror(errno));
				CloseMessageQueue();
				break;
			}
		}

		// Error occurred or message was not found
		return -1;
	}
	else
	{
		// Successfully written to passed in structure
		return 0;
	}

}

int SendMessage(void *msg, size_t msgsz)
{

	// Send a message into specified message queue with message queue id = msqid
	if (msgsnd(msqid, msg, msgsz, 0) == -1)
	{
		fprintf(stderr, "msgsnd failed, err: %d\n", errno);
		return -1;
	}
	return 0;
}

void CloseMessageQueue()
{
	// We must close the message queue
	if (msgctl(msqid, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "Error: msgctl remove failed. Error: %d\n", errno);
	}
	printf("PID %d: has Quit\n", getpid());
}
