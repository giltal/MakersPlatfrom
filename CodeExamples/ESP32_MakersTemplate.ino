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

void arcadeModeDemo();
void touchPannelDemo(); // Set _264K_COLORS

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
	touchPannelDemo();
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
	arcadeModeDemo();
#endif
	while (1);
}

extern const unsigned char PM1[], PM2[], PM3[];

void arcadeModeDemo()
{
#if defined(_8_COLORS)
	const unsigned char *	pacManArray[4] = { PM1 ,PM3 , PM1 ,PM2 };
	unsigned short			pmX = 240, pmY = 160, pmSize = 50;
	short					dx = 0, dy = 0,jx,jy;
	unsigned int			index = 0,changeImageCounter = 0;
	ili9488_8C_flipOption	flip = noflip;

	analogJoyStick JS(JOY_X_PIN, JOY_Y_PIN, 5);
	JS.init();
	while (1)
	{
		lcd.fillScr(0, 0, 0);
		lcd.setFGbitOn();
		lcd.setColor(1, 0, 0);
		lcd.drawRect(340,110,380,210,true);
		lcd.setFGbitOff();
		lcd.setColor(0, 0, 1);
		lcd.drawRect(100, 110, 140, 210, true);
		
		jx = JS.readXaxis();
		lcd.drawBitmap(pmX - pmSize / 2, pmY - pmSize / 2, pacManArray[index], true, flip);
		changeImageCounter++;
		if (changeImageCounter == 5)
		{
			changeImageCounter = 0;		
			index++;
		}
		if (index == 4)
		{
			index = 0;
		}
		dx = (jx < 0)*jx/300 + (jx > 0)*jx/300;
		if (dx < 0)
		{
			flip = flipX;
		}
		if (dx > 0)
		{
			flip = noflip;
		}
		pmX += dx;
		if (pmX < pmSize/2)
		{
			pmX = pmSize / 2;
		}
		if (pmX > (480 - pmSize/2))
		{
			pmX = 480 - pmSize / 2;
		}
		lcd.flushFrameBuffer();//16ms
	}
#endif
}

void touchPannelDemo()
{
	unsigned short x1, y1, x2, y2, count = 0;

	lcd.fillScr(255, 255, 255);
	lcd.setColor(255, 0, 0);
	while (1)
	{
		if (TOUCH_PANNEL_TOUCHED())
		{
			count = ts.getTouchedPoints(&x1, &y1, &x2, &y2);
			printf("TP: x1 = %d, y1 = %d\n", x1, y1);
			if (count == 1)
				lcd.drawCircle(x1, y1, 5,true);
			if (count == 2)
			{
				lcd.drawCircle(x1, y1, 5, true);
				lcd.setColor(0, 0, 255);
				lcd.drawCircle(x2, y2, 5, true);
				lcd.setColor(255, 0, 0);
			}
		}
	}
}