/**
 * @file RAK1903_light.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize and read data from OPT3001 sensor
 * @version 0.1
 * @date 2022-04-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "app.h"
#include <ClosedCube_OPT3001.h>

/** Sensor instance */
ClosedCube_OPT3001 opt3001;
/** Sensor I2C address */
#define OPT3001_ADDRESS 0x44
/** OPT3001 configutation */
OPT3001_Config newConfig;

/**
 * @brief Initialize the Light sensor
 *
 * @return true if sensor found and configuration success
 * @return false if error occured
 */
bool init_rak1903(void)
{
	Wire.begin();
	if (opt3001.begin(OPT3001_ADDRESS) != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not initialize SHTC3");
		return false;
	}

	newConfig.RangeNumber = B1100;
	newConfig.ConvertionTime = B0;
	newConfig.Latch = B1;
	newConfig.ModeOfConversionOperation = B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
		return false;
	}
	return true;
}

/**
 * @brief Read value from light sensor
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_LIGHT
 *
 */
void read_rak1903()
{
	MYLOG("LIGHT", "Reading OPT3001");
	OPT3001 result = opt3001.readResult();
	if (result.error == NO_ERROR)
	{
		uint16_t light_int = (uint16_t)(result.lux);

		MYLOG("LIGHT", "L: %.2f", (float)light_int / 1.0);

		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, light_int);
	}
	else
	{
		MYLOG("LIGHT", "Error reading OPT3001");
	}
}

/**
 * @brief Wake up RAK1903 from sleep
 *
 */
void startup_rak1903(void)
{
	newConfig.RangeNumber = 0b1100;				// B1100;
	newConfig.ConvertionTime = 0b0;				// B0;
	newConfig.Latch = 0b1;						// B1;
	newConfig.ModeOfConversionOperation = 0b11; // B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
	}
}

/**
 * @brief Put the RAK1903 into sleep mode
 *
 */
void shutdown_rak1903(void)
{
	newConfig.RangeNumber = 0b1100;				// B1100;
	newConfig.ConvertionTime = 0b0;				// B0;
	newConfig.Latch = 0b1;						// B1;
	newConfig.ModeOfConversionOperation = 0b00; // B00;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
	}
}