/*
 * Structs.h
 *
 *  Created on: Sep 22, 2015
 *      Author: MVezina
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_


// Create a struct for registered device information

struct DeviceInfo
{
	pid_t pid = 0;
	char* szDeviceName = (char *)0;
	char hasThreshold = 0x0;
	int threshold = 0;

} typedef DeviceInfo;



#endif /* STRUCTS_H_ */
