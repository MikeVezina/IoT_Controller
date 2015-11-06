/*
 * ClientServer.c
 *
 *  Created on: Oct 2, 2015
 *      Author: MVezina
 */

#include "ClientServer.h"

//
// Creates a pipe and opens the file. Returns the file descriptor of the pipe. Prints and Exits the process if an error occurs
//

int CreateAndOpenPipe(const char* pipeName, int openMode)
{

	// Check to see if the fifo exists
	if (access(pipeName, F_OK))
	{
		// If it doesn't exist, Make the fifo
		if (mkfifo(pipeName, 0777))
		{
			printf("[Error]: mkfifo() failed. (Error %d)", errno);
			ClientServerQuit(1);
		}
	}

	// Open the newly created pipe and return the file descriptor
	return OpenPipe(pipeName, openMode);
}

int OpenPipe(const char* pipeName, int openMode)
{

// Check to see if the fifo exists
	if (access(pipeName, F_OK))
	{
		return -1;
	}

	// Open the pipe
	int fileDes = open(pipeName, openMode);

	// Check to see if open the pipe succeeded
	if (fileDes == -1)
	{

		// Returns -1 when there is no pipe available
		// ( Allow calling function to handle ENXIO )
		if (errno == ENXIO)
		{
			return -1;
		}

		// Failed to open the named pipe
		fprintf(stdout, "[Error]: Failed to open named pipe %s (Error %d)\n",  pipeName, errno);
		ClientServerQuit(1);
	}

	// Return the File Descriptor
	return fileDes;

}

//
// Writes to a pipe. Returns the number of bytes written. Returns -1 or Prints and Exits the process if an error occurs
//
int WriteToPipe(int pipe_fd, const char* pipeName, void *msg, size_t szMsg)
{

	// Check to see if the fifo exists
	if (access(pipeName, F_OK))
	{
		// If the file does not exist, errno will be ENOENT
		return -1;
	}

	// Write Message to Pipe
	int writeRes = write(pipe_fd, msg, szMsg);

	// Handle Errors
	if (writeRes == -1)
	{
		switch (errno)
		{
			case EAGAIN:
			case ENXIO:
				// These two errnums will be handled by the calling function. They are not crucial and wont result in irregular program execution
				return -1;

			default: /* Any other error will result in irregular program execution, so an error message is printed */
				fprintf(stdout, "[Error]: Failed to write to Pipe %d. (Error %d)", pipe_fd, errno );
				ClientServerQuit(1);
				break;
		}
	}

	// 	If write response is not the size of the message, something failed. Return -1.
	//	else, return the number of bytes written
	return ((writeRes) != szMsg) ? -1 : writeRes;
}

//
// Read from a pipe. Returns the number of bytes read. Prints and Exits the process if an error occurs
//
int ReadFromPipe(int pipe_fd, const char *pipeName, void *msg, size_t szMsg)
{
	// Check to see if the fifo exists
	if (access(pipeName, F_OK))
		return -1;

	int readRes = 0;

	// Read message from pipe and handle any errors
	if ((readRes = read(pipe_fd, msg, szMsg)) == -1)
	{
		switch (errno)
		{
			case EAGAIN: // Non-Blocking Call sets errno to this to let the application know there was no available message
				return -1;
				break;

			default: // Any other errors will cause the program to perform abnormally, so we print an error message and quit
				fprintf(stdout, "[Error]: Failed to read from Pipe %d. (Error %d)\n", pipe_fd, errno );
				ClientServerQuit(1);
				break;
		}
	}

	// 	If read response is not the size of the message, something failed. Return -1.
	//	else, return the number of bytes read
	return ((readRes) != szMsg) ? -1 : readRes;
}
