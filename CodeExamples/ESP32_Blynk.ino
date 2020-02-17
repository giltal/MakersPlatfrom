/*************************************************************
  Download latest Blynk library here:
	https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

	Downloads, docs, tutorials: http://www.blynk.cc
	Sketch generator:           http://examples.blynk.cc
	Blynk community:            http://community.blynk.cc
	Follow us:                  http://www.fb.com/blynkapp
								http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how to monitor a button state
  using interrupts mechanism.

  App project setup:
	LED widget on V1
 *************************************************************/
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

//#define _262K_COLORS
#define _8_COLORS

#if defined(_262K_COLORS)
ILI9488SPI_264KC lcd(480, 320);
#else
ILI9488SPI_8C lcd(480, 320);
#endif


/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define WIFI_BLYNK_DEMO
#ifdef WIFI_BLYNK_DEMO
#else
#define BLYNK_USE_DIRECT_CONNECT
#endif

#ifdef WIFI_BLYNK_DEMO
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "NETGEAR98";
char pass[] = "coolfire180";
#else
#include <BlynkSimpleEsp32_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#endif

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "zaa_MIl8yw5ZstFcEfwUC1i93pPxTkeH";

int virtualJoyX = 0, virtualJoyY = 0;

BLYNK_WRITE(V0)
{
	virtualJoyX = param[0].asInt();
	virtualJoyY = param[1].asInt();
	// Do something with x and y
	printf("X = %d, y = %d\n", virtualJoyX, virtualJoyY);
}

int virtualPinR = 0, virtualPinG = 0, virtualPinB = 0;

BLYNK_WRITE(V1)
{
	virtualPinR = param[0].asInt();
}
BLYNK_WRITE(V2)
{
	virtualPinG = param[0].asInt();
}
BLYNK_WRITE(V3)
{
	virtualPinB = param[0].asInt();
}

void setup()
{
	// Debug console
	Serial.begin(115200);
#ifdef WIFI_BLYNK_DEMO
	Blynk.begin(auth, ssid, pass);
#else
	Blynk.setDeviceName("Gil-Blynk");
	Blynk.begin(auth);
#endif	
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

	// Blynk related

	Serial.println("Waiting for connections...");
}

void loop()
{
	Blynk.run();
	lcd.fillScr(0, 0, 0);
	lcd.setColor(virtualPinR, virtualPinG, virtualPinB);
	lcd.drawCircle(240 + (virtualJoyX - 512) / 3, 160 - (virtualJoyY - 512) / 4, 15, true);
	lcd.flushFrameBuffer();
}