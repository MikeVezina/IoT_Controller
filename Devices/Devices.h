/*
 * Devices.h
 *
 *  Created on: Sep 25, 2015
 *      Author: MVezina
 */

#ifndef DEVICES_H_
#define DEVICES_H_

#define DEVTYPE_SENSOR 1
#define DEVTYPE_ACTUATOR 2

// Structure for Device Info
struct DEVICEINFO
{
	// Process ID of the device
	pid_t pid;

	// Device name (null-terminated string)
	char devName[25];

	// Device type
	char devType;

	// This value is either true ( != 0 ) or False ( = 0 ) to show that the device has a specified threshold
	char hasThreshold;

	// The threshold value that is looked at if and only if the 'hasThreshold' variable is != 0
	int threshold;

} typedef DEVICEINFO;

// Structure for Sensor Info
struct SENSORINFO
{
	// For now, only one byte will be used for sensing data
	char data[1];

}typedef SENSORINFO;

struct DEVICELINK
{
	DEVICEINFO devInfo;
	struct DEVICELINK* nextDeviceLink;

} typedef DEVICELINK, *PDEVICELINK;

int SetDeviceInfo();

#endif /* DEVICES_H_ */
