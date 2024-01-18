/**
 * @file RAK12037-co2.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize and read values from the SCD30 sensor
 * @version 0.1
 * @date 2022-04-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <SparkFun_SCD30_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_SCD30

/** Sensor instance */
SCD30 scd30;

/**
 * @brief Initialize MQ2 gas sensor
 *
 * @return true success
 * @return false failed
 */
bool init_rak12037(void)
{
	// Enable power
	pinMode(WB_IO2, OUTPUT);
	// digitalWrite(WB_IO2, HIGH); // power on RAK12037

	Wire.begin();
	if (!scd30.begin(Wire))
	{
		// MYLOG("SCD30", "SCD30 not found");
		// digitalWrite(WB_IO2, LOW); // power down RAK12037
		return false;
	}

	//**************init SCD30 sensor *****************************************************
	// Change number of seconds between measurements: 2 to 1800 (30 minutes), stored in non-volatile memory of SCD30
	scd30.setMeasurementInterval(2);

	// Enable self calibration
	scd30.setAutoSelfCalibration(false);

	// // Start the measurements
	scd30.beginMeasuring();

	// scd30.StopMeasurement();

	return true;
}

void start_rak12037(void)
{
	// Start the measurements
	scd30.beginMeasuring();
}

/**
 * @brief Read CO2 sensor data
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_CO2_2, LPP_CHANNEL_CO2_Temp_2 and LPP_CHANNEL_CO2_HUMID_2
 *
 */
void read_rak12037(void)
{
	time_t start_time = millis();
	while (!scd30.dataAvailable())
	{
		MYLOG("SCD30", "Waiting for data");
		delay(500);
		if ((millis() - start_time) > 5000)
		{
			// timeout, no data available
			MYLOG("SCD30", "Timeout");
			// Stop the measurements
			scd30.StopMeasurement();
			return;
		}
	}

	uint16_t co2_reading = scd30.getCO2();
	float temp_reading = scd30.getTemperature();
	float humid_reading = scd30.getHumidity();

	MYLOG("SCD30", "CO2 level %dppm", co2_reading);
	MYLOG("SCD30", "Temperature %.2f", temp_reading);
	MYLOG("SCD30", "Humidity %.2f", humid_reading);

	g_solution_data.addConcentration(LPP_CHANNEL_CO2_2, co2_reading);
	// Add temperature and humidity only if no RAK1906 is available
	if (!has_rak1906)
	{
		g_solution_data.addTemperature(LPP_CHANNEL_CO2_Temp_2, temp_reading);
		g_solution_data.addRelativeHumidity(LPP_CHANNEL_CO2_HUMID_2, humid_reading);
	}
	else
	{
		_last_bme_temp = temp_reading;
		_last_bme_humid = humid_reading;
		_has_last_bme_values = true;
	}
	// Stop the measurements
	scd30.StopMeasurement();
}

/**
 * @brief Returns the latest temperature and humidity values from the sensor
 *        or returns 0 if no readings are available
 *
 * @param values array for temperature [0] and humidity [1]
 */
void get_rak12037_values(float *values)
{
	if (_has_last_bme_values)
	{
		// Discard old values after 10 VOC readings
		t_h_discard_counter++;
		if (t_h_discard_counter == 9)
		{
			t_h_discard_counter = 0;
			_has_last_bme_values = false;
		}
		values[0] = _last_bme_temp;
		values[1] = _last_bme_humid;
		return;
	}
	else
	{
		values[0] = 0;
		values[1] = 0;
	}
	return;
}
