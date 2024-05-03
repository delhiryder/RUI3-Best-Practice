/**
 * @file LoRa_P2P_RX.ino
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Simple LoRa P2P RX with callbacks
 * @version 0.1
 * @date 2024-05-03
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include "app.h"

/** Payload buffer */
uint8_t g_solution_data[64];

/**
 * @brief LoRa P2P callback if a packet was received
 *
 * @param data pointer to the data with the received data
 */
void recv_cb(rui_lora_p2p_recv_t data)
{
	MYLOG("RX-P2P-CB", "P2P RX, RSSI %d, SNR %d", data.Rssi, data.Snr);
	for (int i = 0; i < data.BufferSize; i++)
	{
		Serial.printf("%02X", data.Buffer[i]);
	}
	Serial.print("\r\n");
}

/**
 * @brief LoRa P2P callback if a packet was sent
 *
 */
void send_cb(void)
{
	MYLOG("TX-P2P-CB", "P2P TX finished");
	digitalWrite(LED_BLUE, LOW);
}

/**
 * @brief LoRa P2P callback for CAD result
 *
 * @param result true if activity was detected, false if no activity was detected
 */
void cad_cb(bool result)
{
	MYLOG("CAD-P2P-CB", "P2P CAD reports %s", result ? "activity" : "no activity");
}

/**
 * @brief Arduino setup, called once after reboot/power-up
 *
 */
void setup()
{
	// Force P2P
	api.lorawan.nwm.set(0);

	// Force P2P setup
	api.lorawan.pfreq.set(916100000);
	api.lorawan.psf.set(7);
	api.lorawan.pbw.set(0);
	api.lorawan.pcr.set(1);
	api.lorawan.ppl.set(8);
	api.lorawan.ptp.set(22);

	// Setup for LoRa P2P
	api.lorawan.registerPRecvCallback(recv_cb);
	api.lorawan.registerPSendCallback(send_cb);
	api.lorawan.registerPSendCADCallback(cad_cb);

	pinMode(LED_GREEN, OUTPUT);
	digitalWrite(LED_GREEN, HIGH);
	pinMode(LED_BLUE, OUTPUT);
	digitalWrite(LED_BLUE, HIGH);

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, LOW);

	// Start Serial
	Serial.begin(115200);

	// Delay for 5 seconds to give the chance for AT+BOOT
	delay(5000);

	api.system.firmwareVersion.set("RUI3-P2P-TX-V1.0.0");

	Serial.println("RAKwireless RUI3 Node");
	Serial.println("------------------------------------------------------");
	Serial.println("Setup the device with WisToolBox or AT commands before using it");
	Serial.printf("Version %s\n", api.system.firmwareVersion.get().c_str());
	Serial.println("------------------------------------------------------");

	// Initialize module
	Wire.begin();

	// Register the custom AT command to get device status
	if (!init_status_at())
	{
		MYLOG("SETUP", "Add custom AT command STATUS fail");
	}

	// Register the custom AT command to set the send interval
	if (!init_interval_at())
	{
		MYLOG("SETUP", "Add custom AT command Send Interval fail");
	}

	// Get saved sending interval from flash
	get_at_setting();

	digitalWrite(LED_GREEN, LOW);

	// Create a timer.
	api.system.timer.create(RAK_TIMER_0, sensor_handler, RAK_TIMER_PERIODIC);
	if (custom_parameters.send_interval != 0)
	{
		api.system.timer.start(RAK_TIMER_0, custom_parameters.send_interval, NULL);
	}

	digitalWrite(LED_BLUE, LOW);
	// Enable RX permanent with TX possible
	api.lorawan.precv(65533);

	// Enable low power mode
	api.system.lpm.set(2);

	// If available, enable BLE advertising for 30 seconds and open the BLE UART channel
#if defined(_VARIANT_RAK3172_) || defined(_VARIANT_RAK3172_SIP_)
// No BLE
#else
	Serial6.begin(115200, RAK_AT_MODE);
	api.ble.advertise.start(30);
#endif
}

/**
 * @brief sensor_handler is a timer function called every
 * custom_parameters.send_interval milliseconds. Default is 120000. Can be
 * changed with ATC+SENDINT command
 *
 */
void sensor_handler(void *)
{
	MYLOG("UPLINK", "Start");
	digitalWrite(LED_BLUE, HIGH);

	// Create payload (Cayenne LPP format for voltage)
	g_solution_data[0] = 0x01;
	g_solution_data[1] = 0x74;
	g_solution_data[2] = 0x01;
	g_solution_data[3] = 0x8c;

	// Send the packet
	send_packet();
}

/**
 * @brief Send the data packet that was prepared in
 * Cayenne LPP format by the different sensor and location
 * aqcuision functions
 *
 */
void send_packet(void)
{
	// It is P2P
	MYLOG("UPLINK", "Send packet with size 4 over P2P");

	digitalWrite(LED_BLUE, LOW);

	if (api.lorawan.psend(4, g_solution_data, true))
	{
		MYLOG("UPLINK", "Packet enqueued");
	}
	else
	{
		MYLOG("UPLINK", "Send failed");
	}
}

/**
 * @brief This example is complete timer driven.
 * The loop() does nothing than sleep.
 *
 */
void loop()
{
	api.system.sleep.all();
}
