/*
 * SignalAssist.h
 *
 *  Created on: Sep 26, 2015
 *      Author: MVezina
 */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "../Messages/Messages.h"


#ifndef SIGNALASSIST_H_
#define SIGNALASSIST_H_

// This will be defined by file including the header file
extern void MsgRcvd();
extern void CtrlCPressed();


void InstallControlCSignalHandlers();
void InstallMessageSignalHandler();

#endif /* SIGNALASSIST_H_ */
