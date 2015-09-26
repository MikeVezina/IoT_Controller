/*
 * Devices.h
 *
 *  Created on: Sep 25, 2015
 *      Author: MVezina
 */

#ifndef DEVICES_H_
#define DEVICES_H_

/* --- Device Enums --- */

// (DEV)ice (TYPE). The Device Type
enum DeviceType{
	DEVTYPE_SENSOR = 1,
	DEVTYPE_ACTUATOR = 2
} typedef DeviceType;

// (TH)reshold (AC)tions. Actions that are executed when a threshold is exceeded
enum ThresholdAction
{
	THAC_NONE = 0x0,
	THAC_AC_OFF = 0x01,
	THAC_AC_ON = 0x02,
	THAC_BELL_RING = 0x04,
	THAC_FIRE_ALARM_RING = 0x08,
	THAC_LIGHT_OFF = 0x10,
	THAC_LIGHT_ON = 0x20

} typedef ThresholdAction;

// (SENS)or (TYPE). The type of sensor
enum SensorType{
	SENSTYPE_TEMPERATURE = 11,
	SENSTYPE_SMOKE_DETECTOR = 12,
	SENSTYPE_LIGHT = 13
}typedef SensorType;

// (ACT)uator (TYPE). The type of actuator
enum ActuatorType{
	ACTTYPE_AC_POWER_SWITCH = 21,
	ACTTYPE_BELL = 22,
	ACTTYPE_FIRE_ALARM = 23,
	ACTTYPE_WATER_SPRINKLER = 24,
	ACTTYPE_LIGHT_SWITCH = 25

}typedef ActuatorType;

/* --- End Device Enums --- */





/* --- Device Structures --- */

// Structure for Device Info
struct DEVICEINFO
{
	// Process ID of the device
	pid_t pid;

	// Device name (null-terminated string)
	char devName[25];

	// Device type
	DeviceType devType;

	// If devType == DEVTYPE_SENSOR use SensorType enum
	SensorType sensType;

	// else, if devType == DEVTYPE_ACTUATOR use ActuatorType enum
	ActuatorType actType;

	// This value is either true ( != 0 ) or False ( = 0 ) to show that the device has a specified threshold
	char hasThreshold;

	// The threshold value that is looked at if and only if the 'hasThreshold' variable is != 0
	int threshold;

	// Threshold Action (Only used for sensors)
	ThresholdAction thresholdAction;


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

/* --- End Device Structures --- */


/* Device Function Prototypes */
int SetDeviceInfo();

#endif /* DEVICES_H_ */
