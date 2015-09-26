/*
 * Messages.c
 *
 *  Created on: Sep 23, 2015
 *      Author: MVezina
 */

#include "Messages.h"

// Eases creation of a message header
// msgHeader : Pointer to the message header structure
// destPid : destination pid for the message to be sent to (optional)
// msgType : Message Type
int SetMessageHeader(MESSAGEHEADER *msgHdr, pid_t destPid, long int msgType)
{
	// If the message type is 0
	if (msgType == 0)
	{
		fprintf(stderr, "SetMessageHeader(): msgType cannot be equal to 0");
		return -1;
	}

	// If the pointer to the message header struct is null
	if (!msgHdr)
	{
		fprintf(stderr, "SetMessageHeader(): msgHeader is null");
		return -1;
	}

	msgHdr->sourcePid = getpid();
	msgHdr->destinationPid = destPid;
	msgHdr->msgType = msgType;

	return 0;
}

// Function that checks for a message in the queue
// msgsz is the size of the message struct without the message type
// Uses msgrcv to write any available messages of msgtype into the pointer specified with the specified size
// Returns -1 if error occurs or no message of specified type was found, else returns 0 for success
int ReceiveMessage(void *msg, size_t msgsz, long int msgtype)
{
	// Check if msg is null
	if (!msg)
		return -1;

	// Check to make sure this message is designated for the current process by checking the destination PID (in message header)
	// This check only needs to be done for commands to ensure the command is sent to the right process
	// We need to make a copy of the struct at msg so that we can undo any changes if the pid does not match
	size_t oldMsgSz = msgsz + sizeof(msgtype);
	void *oldMsg = malloc(oldMsgSz);

	// Copies memory from msg to msgOld
	memcpy(oldMsg, msg, oldMsgSz);

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
				fprintf(stderr, "Error: ReceiveMessage(): msgrcv(msgtype: %li) failed! Error Number: %d. Error Desc.:%s\n", msgtype, errno, strerror(errno));
				CloseMessageQueue();
				break;
			}
		}

		// Make sure to free the msg backup memory block
		free(oldMsg);

		// Error occurred or message was not found
		return -1;
	}
	else
	{
		// Once the message has been written to the pointer to the message command,
		// We want to ensure it was designated to the current process
		// If the PID does not match, we undo any writing to the block of memory

		if (msgtype == MSG_CMD)
		{
			if (((PROCESSCOMMANDMESSAGE *) msg)->msgHdr.destinationPid != getpid())
			{
				// Since the pid does not match, we will undo mscrcv by copying the original message back into the struct
				memcpy(msg, oldMsg, oldMsgSz);

				// Free msgOld memory block
				free(oldMsg);

				// Return unsuccessful
				return -1;
			}

		}

		free(oldMsg);
		// Successfully written to passed in structure
		return 0;
	}

}

int SendMessage(void *msg, size_t msgsz)
{

	// msgsz must be larger than sizeof(MESSAGEHEADER)-sizeof(long int)
	if (msgsz < (sizeof(MESSAGEHEADER) - sizeof(long int)))
	{
		fprintf(stderr, "SendMessage(): The msgsz paramater is smaller than the minimum message size");
		return -1;
	}

	// Send a message into specified message queue with message queue id = msqid
	if (msgsnd(msqid, msg, msgsz, 0) == -1)
	{
		fprintf(stderr, "SendMessage(): msgsnd failed, err: %d\n", errno);
		return -1;
	}
	return 0;
}

void CloseMessageQueue()
{
// We must close the message queue
	if (msgctl(msqid, IPC_RMID, 0) == -1)
	{
		fprintf(stderr, "Error: CloseMessageQueue(): msgctl remove failed. Error: %d\n", errno);
	}
	printf("PID %d: has Quit\n", getpid());
}
