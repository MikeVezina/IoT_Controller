/*
 * Devices.c
 *
 *  Created on: Sep 25, 2015
 *      Author: MVezina
 */

// SetBasicDeviceInfo allows easy creation for a DEVICEINFO structure
// Sets the DeviceName, DeviceType and Device PID
#include "Devices.h"

int SetBasicDeviceInfo(DEVICEINFO *devInfo, char szDeviceName[], DeviceType devType)
{
	if (!devInfo)
	{
		fprintf(stderr, "[Error]: SetBasicDeviceInfo(): devInfo is null\n");
		return -1;
	}

	// Check to ensure device name is 24 characters or less
	if (strlen(szDeviceName) > 24)
	{
		fprintf(stderr, "[Error]: SetBasicDeviceInfo(): szDeviceName is larger than 24 characters\n");
		return -1;
	}

	devInfo->pid = getpid();
	devInfo->devType = devType;
	strcpy(devInfo->devName, szDeviceName);
	return 0;

}

// Prints the Sensor Data to stdout
// Prints the Sensor Data to stdout
void PrintSensorInfo(SENSORINFO *senInfo)
{
	// Check to make sure sensor info is not null
	if (!senInfo)
		return;

	// Print Sensor Info
	fprintf(stdout, "[=== Sensor Info ===]\n");
	for (int i = 0; i < (sizeof(senInfo->data) / sizeof(senInfo->data[0])); i++)
	{
		fprintf(stdout, "   Data [%d]: %d\n", i, senInfo->data[i]);
	}

}

// Prints the device info to stdout
void PrintDeviceInfo(DEVICEINFO *devInfo)
{
	if (devInfo == 0)
		return;

	// Print Device Type for sensors
	if (devInfo->devType == DEVTYPE_SENSOR)
	{
		char *devType;

		switch (devInfo->sensType)
		{
			case SENSTYPE_TEMPERATURE:
				devType = "Temperature";
				break;
			case SENSTYPE_LIGHT:
				devType = "Light";
				break;
			case SENSTYPE_SMOKE_DETECTOR:
				devType = "Smoke Detector";
				break;
			default:
				devType = "UNKNOWN";
				break;
		}

		// Print out the different fields of device information
		fprintf(stdout, "[=== Device Info ===]\n   Device PID:\t%d\n   Device Name:\t%s\n   Device Type:\t%d (SENSOR) (%s)\n   Threshold? \t%c\n", devInfo->pid, devInfo->devName, devInfo->sensType, devType,
		(devInfo->hasThreshold) ? 'Y' : 'N');

		// Only print the device threshold if the device has a threshold
		if (devInfo->hasThreshold)
			fprintf(stdout, "   Threshold:\t%d", devInfo->threshold);

		printf("\n");

	}
	else // Print Device Info for actuators
	{

		char *devType;

		switch (devInfo->actType)
		{
			case ACTTYPE_LIGHT_SWITCH:
				devType = "Light Switch";
				break;
			case ACTTYPE_BELL:
				devType = "Bell";
				break;
			case ACTTYPE_AC_POWER_SWITCH:
				devType = "A/C Power Switch";
				break;
			case ACTTYPE_WATER_SPRINKLER:
				devType = "Water Sprinkler";
				break;
			case ACTTYPE_FIRE_ALARM:
				devType = "Fire Alarm";
				break;

			default:
				devType = "UNKNOWN";
				break;
		}

		// Print out the different fields of device information
		fprintf(stdout, "[=== Device Info ===]\n   Device PID:\t%d\n   Device Name:\t%s\n   Device Type:\t%d (ACTUATOR) (%s)\n   Actions: \t0x%x\n", devInfo->pid, devInfo->devName, devInfo->devType, devType, devInfo->thresholdAction);


	}

	fprintf(stdout, "\n");
}

