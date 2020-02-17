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

TaskHandle_t Task1;
TaskHandle_t Task2;

bool virtualPin1 = true, virtualPin2 = true;

void Task1code(void * pvParameters)
{
	while (1)
	{
		virtualPin1 = true;
		delay(1000);
		virtualPin1 = false;
		delay(1000);
	}
}

void Task2code(void * pvParameters)
{
	while (1)
	{
		virtualPin2 = true;
		delay(1000);
		virtualPin2 = false;
		delay(1000);
	}
}


void setup()
{
	// Debug console
	Serial.begin(115200);
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
	xTaskCreatePinnedToCore(
		Task1code, /* Task function. */
		"Task1",   /* name of task. */
		10000,     /* Stack size of task */
		NULL,      /* parameter of the task */
		1,         /* priority of the task */
		&Task1,    /* Task handle to keep track of created task */
		0);        /* pin task to core 0 */

	xTaskCreatePinnedToCore(
		Task2code, /* Task function. */
		"Task1",   /* name of task. */
		10000,     /* Stack size of task */
		NULL,      /* parameter of the task */
		1,         /* priority of the task */
		&Task1,    /* Task handle to keep track of created task */
		1);        /* pin task to core 0 */

}

void loop()
{
	lcd.fillScr(0, 0, 0);
	if (virtualPin1)
	{
		lcd.setColor(1, 0, 0);
		lcd.drawCircle(120, 160, 30, true);
	}
	else
	{
		lcd.setColor(0, 0, 0);
		lcd.drawCircle(120, 160, 30, true);
	}
	if (virtualPin2)
	{
		lcd.setColor(0, 1, 0);
		lcd.drawCircle(360, 160, 30, true);
	}
	else
	{
		lcd.setColor(0, 0, 0);
		lcd.drawCircle(360, 160, 30, true);
	}

	lcd.flushFrameBuffer();
}