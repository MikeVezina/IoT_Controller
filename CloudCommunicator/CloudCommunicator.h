/*
 * CloudCommunicator.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include "signal.h"
#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#include <sys/wait.h>

#include "../Messages/Messages.h"
#include "../Devices/Devices.h"
#include "ClientServer.h"

#ifndef CLOUDCOMMUNICATOR_H_
#define CLOUDCOMMUNICATOR_H_

void RunCloudCommunicator();
void ProcessServerRequest(DATAMESSAGE *dataMsg);
void SendThresholdExceededInformationSendRequest(THRESHOLDCROSSINGMESSAGE *threshCrossMsg);
void CloudCommunicatorExit();

#endif /* CLOUDCOMMUNICATOR_H_ */
