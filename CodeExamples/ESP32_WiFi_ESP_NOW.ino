/*
 Name:		ESP32_MakersTemplate.ino
 Created:	1/7/2020 2:25:01 PM
 Author:	giltal
*/

#include <GeneralLib.h>
#include <Adafruit_FT6206.h>
#include "PCF8574.h"
#include <Wire.h>
#include <RTClib.h>
#include "graphics.h"

#include "WiFi.h"
#include "esp_now.h"


#define SPEAKER_PIN 5
#define MP_BUTTON_1	27
#define MP_BUTTON_2	33
#define TOUCH_1_PIN	15
#define TOUCH_2_PIN	32
#define JOY_X_PIN	36
#define JOY_Y_PIN	39


PCF8574 pcf8574(0x39);
Adafruit_FT6206 ts = Adafruit_FT6206();

#define _264K_COLORS
//#define _8_COLORS

#if defined(_264K_COLORS)
ILI9488SPI_264KC lcd(480, 320);
#else
ILI9488SPI_8C lcd(480, 320);
#endif

/* ESP Now Related*/
// declaration of callback functions
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

//#define MASTER
#define PEER



// MAC addresses
#define MAC_LEN 6
uint8_t masterMACaddress[]		= { 0x24, 0x6F, 0x28, 0x17, 0xFC, 0x48 };
uint8_t peer01MACaddress[]		= { 0x24, 0x6F, 0x28, 0x17, 0x0D, 0x60 };
uint8_t* targetMACaddress;

// the setup function runs once when you press reset or power the board
void setup()
{
	// Setup the IOs
	pinMode(34, INPUT); // Touch pannel interrupt
	pinMode(36, ANALOG); // Joystick X
	pinMode(39, ANALOG); // Joystick Y
	pinMode(MP_BUTTON_1, INPUT);
	pinMode(MP_BUTTON_2, INPUT);
	// Setup the speaker
	pinMode(SPEAKER_PIN, OUTPUT);
	ledcSetup(0, 2000, 8);
	ledcAttachPin(SPEAKER_PIN, 0);

	Serial.begin(115200);

	// Setup the LCD screen
#if defined(_264K_COLORS)
	lcd.init();
#else
	if (!lcd.init(18, 19, 23, 5, 40000000L, singleFrameBuffer))
	{
		printf("Cannot initialize LCD!");
		while (1);
	}
#endif

	// Setup the touch pannel
	delay(250);

	if (!ts.begin(20))
	{
		Serial.println("Unable to start touchscreen.");
	}
	else
	{
		Serial.println("Touchscreen started.");
	}


	// Initialize as WiFi station
	Serial.begin(115200);
	WiFi.mode(WIFI_MODE_STA);
	Serial.println("MAC");

	// Code to get MAC address of ESP32
	Serial.println(WiFi.macAddress());

	// Init ESP-NOW
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW");
	}

	/* Callback Functions */
	// On Tx
	esp_now_register_send_cb(OnDataSent);
	// On Rx
	esp_now_register_recv_cb(OnDataRecv);

	/* Register peer */
	esp_now_peer_info_t peerInfo;

	// when flashing the board
#if defined(MASTER)
	targetMACaddress = peer01MACaddress;
#else
	targetMACaddress = masterMACaddress;
#endif

	memcpy(peerInfo.peer_addr, targetMACaddress, MAC_LEN);

	peerInfo.channel = 0;
	peerInfo.encrypt = false;

	// Add peer
	if (esp_now_add_peer(&peerInfo) != ESP_OK) {
		Serial.println("Failed to add peer");
	}
}

// the loop function runs over and over again until power down or reset
void loop()
{
#if defined(_264K_COLORS)
	lcd.fillScr(0, 0, 0);
	lcd.setColor(255, 0, 0);
	lcd.drawString("LCD DEMO", 100, 100, 25);
	lcd.setColor(0, 255, 0);
	lcd.drawString("LCD DEMO", 150, 150, 25);
	lcd.setColor(0, 0, 255);
	lcd.drawString("LCD DEMO", 200, 190, 25);
	delay(1000);
#else
	lcd.fillScr(0, 0, 0);
	lcd.setColor(1, 0, 0);
	lcd.drawString("LCD DEMO", 100, 100, 25);
	lcd.setColor(0, 1, 0);
	lcd.drawString("LCD DEMO", 150, 150, 25);
	lcd.setColor(0, 0, 1);
	lcd.drawString("LCD DEMO", 200, 190, 25);
	lcd.flushFrameBuffer();
	delay(1000);
#endif

#if defined(MASTER)
	size_t dataLength = 2;
	uint8_t data[2] = { 0xAA, 0x55 };
#else
	size_t dataLength = 3;
	uint8_t data[3] = { 0x88, 0x33, 0x44 };
#endif

	int iteration = 0;
	esp_err_t result;
	while (1)
	{
		Serial.println(iteration);
		iteration++;
		// Send message via ESP-NOW
		result = esp_now_send(targetMACaddress, data, dataLength);
		sleep(500);
	}
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
	Serial.print("\r\nLast Packet Send Status:\t");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Successfull Delivery " : "Delivery Failed");
}


// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
	printf("%d Bytes received\n", len);
}

