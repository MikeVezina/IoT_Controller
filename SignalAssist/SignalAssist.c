/*
 * SignalAssist.c
 *
 *  Created on: Sep 26, 2015
 *      Author: MVezina
 */

#include "SignalAssist.h"

void InstallControlCSignalHandlers()
{
	struct sigaction sigAct;
	sigAct.sa_handler = CtrlCPressed;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;

	// Set Signal Handler (For Ctrl-C) and (For Interrupt Signal from child)
	if(sigaction(SIGINT, &sigAct, 0))
	{
		printf("PID %d: sigaction failed. Error Number: %d\n", getpid(), errno);
		CloseMessageQueue();
	}
}



void InstallMessageSignalHandler()
{
	// Install the user defined signal handler
	// So that the DeviceCommunicator can communicate with the cloud communicator
	struct sigaction sigact;
	sigact.sa_handler = MsgRcvd;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;

	if (sigaction(SIGUSR2, &sigact, 0))
	{
		printf("PID %d: sigaction failed. Error Number: %d\n", getpid(), errno);
		CloseMessageQueue();
		exit(0);
	}
	else
	{
		printf("Successfully Installed Message Handler %d\n", getpid());
	}

}





