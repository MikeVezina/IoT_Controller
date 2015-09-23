/*
 * Structs.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#include <stdlib.h>
#include <unistd.h>

#ifndef STRUCTS_H_
#define STRUCTS_H_


// Enum used to specify the type of device
enum DeviceType
{
	Sensor,
	Actuator
} typedef DeviceType;


// Structure for Message Header (All Message Structs must include this)
struct MessageHeader
{
	long int msgType;
	pid_t sourcePid;
	pid_t destinationPid;

} typedef MessageHeader;


// Structure for Device Info
struct DeviceInfo
{
	// Process ID of the device
	pid_t pid;

	// Device name (null-terminated string)
	char devName[25];

	// Device type (see DeviceType enum)
	DeviceType devType;

	// This value is either true ( != 0 ) or False ( = 0 ) to show that the device has a specified threshold
	char hasThreshold;

	// The threshold value that is looked at if and only if the 'hasThreshold' variable is != 0
	int threshold;

} typedef DeviceInfo;

// Structure for device registration messages
struct DeviceRegistrationMessage
{
	MessageHeader msgHdr;

	DeviceInfo devInfo;

} typedef DeviceRegistrationMessage;

// Structure for Sensor data messages
struct SensorDataMessage
{
	MessageHeader msgHdr;

	// For now, only one byte will be used for sensing data
	char data[1];

} typedef SensorDataMessage;

// Structure for Command Messages
struct CommandMessage
{
	MessageHeader msgHdr;

	// One byte used for a command
	/*
	 * Commands:
	 * 0xE = Exit
	 */
	char command[1];
} typedef CommandMessage;


#endif /* STRUCTS_H_ */
