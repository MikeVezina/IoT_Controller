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
		fprintf(stdout, "[Error]: SetMessageHeader(): msgType cannot be equal to 0");
		return -1;
	}

	// If the pointer to the message header struct is null
	if (!msgHdr)
	{
		fprintf(stdout, "[Error]: SetMessageHeader(): msgHeader is null");
		return -1;
	}

	msgHdr->sourcePid = getpid();
	msgHdr->destinationPid = destPid;
	msgHdr->msgType = msgType;

	return 0;
}

int SendProcessCommand(char command, pid_t destinationPID)
{

	PROCESSCOMMANDMESSAGE cmd;
	SetMessageHeader(&cmd.msgHdr, destinationPID, MSG_CMD);

	cmd.command[0] = command;

	if (!SendMessage(&cmd, sizeof(cmd) - sizeof(cmd.msgHdr.msgType)))
	{
		return 0;
	}
	return -1;

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

	if (!oldMsg)
	{
		fprintf(stdout, "[Error]: ReceiveMessage() Failed To Allocate Memory. Quitting\n");
		CloseMessageQueue();
		exit(1);
		return -1;
	}

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
			case EINVAL:
				CloseMessageQueue();
				exit(1);
				break;
			default:
			{

				fprintf(stdout, "[Error]: (PID %d) ReceiveMessage(): msgrcv(msgtype: %li) failed! Error Number: %d. Error: %s\n", getpid(), msgtype, errno, strerror(errno));
				CloseMessageQueue();
				printf("[Quit]: PID %d: has Quit\n", getpid());
				exit(1);
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

		if (msgtype == MSG_CMD || msgtype == MSG_ACTCMD || msgtype == MSG_ACTCMDRES)
		{
			if (((MESSAGEHEADER *) msg)->destinationPid != getpid())
			{
				// Now, we must re-add the message to the queue
				if (SendMessage(msg, msgsz))
				{
					fprintf(stdout, "[Error]: ReceiveMessage() failed to add message back to queue after failed pid match.\n");
				}

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

	// Return if message struct pointer is null
	if (!msg)
		return -1;

	// Return if message queue id is invalid
	if (msqid == -1)
		return -1;

	// msgsz must be larger than sizeof(MESSAGEHEADER)-sizeof(long int)
	if (msgsz < (sizeof(MESSAGEHEADER) - sizeof(long int)))
	{
		fprintf(stdout, "[Error]: SendMessage(): The msgsz paramater is smaller than the minimum message size");
		return -1;
	}

	// Send a message into specified message queue with message queue id = msqid
	if (msgsnd(msqid, msg, msgsz, 0) == -1)
	{
		switch (errno)
		{
			case EINVAL: /* Silently Close Message Queue and exits the program if the message queue was not discarded properly */
				CloseMessageQueue();
				exit(1);
				break;

			default:
				fprintf(stdout, "[Error]: SendMessage(): msgsnd failed. (Error %d)\n", errno);
				CloseMessageQueue();
				printf("[Quit]: PID %d: has Quit\n", getpid());
				exit(1);
				return -1;
		}

	}
	return 0;
}


// Closes and removes the message queue
void CloseMessageQueue()
{
	// We must close the message queue
	if (msgctl(msqid, IPC_RMID, 0) == -1)
	{
		// Invalid Message queue id. (Caused by a message queue with the same key or caused by previous abnormal program termination
		if (errno == EINVAL)
		{
			printf("\n===========================================\n[Error]:\nMessage Queue Abnormally Terminated.\nFlushing Message Queue.\nPlease Restart Controller\n===========================================\n\n");
			return;
		}


		// We only want to print the error message when the key is valid (it is invalid when errno == EINVAL)
		fprintf(stdout, "[Error]: CloseMessageQueue(): msgctl remove failed. (Error %d)\n", errno);

	}

}
