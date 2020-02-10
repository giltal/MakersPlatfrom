/*
 Name:		ESP32_MP_Template.ino
 Created:	2/7/2020 9:59:34 AM
 Author:	giltal
*/

#include <GeneralLib.h>
#include <Adafruit_FT6206.h>
#include "PCF8574.h"
#include <Wire.h>
#include <RTClib.h>
#include "graphics.h"

#define SPEAKER_PIN 5
#define MP_BUTTON_1	27
#define MP_BUTTON_2	33
#define TOUCH_1_PIN	15
#define TOUCH_2_PIN	32
#define JOY_X_PIN	36
#define JOY_Y_PIN	39


PCF8574 pcf8574(0x39);
Adafruit_FT6206 ts = Adafruit_FT6206();

#define _262K_COLORS
//#define _8_COLORS

#if defined(_262K_COLORS)
ILI9488SPI_264KC lcd(480, 320);
#else
ILI9488SPI_8C lcd(480, 320);
#endif

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

	// Initialize the LCD screen
#if defined(_262K_COLORS)
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
}

// the loop function runs over and over again until power down or reset
void loop()
{
	while (1);
}