/*
 Name:		GameBoy32.ino
 Created:	4/17/2020 9:17:34 AM
 Author:	giltal
*/

#include <stdio.h>
#include "ESP32_GB_EMU.h"
#include "gbrom.h"

// Our platform related code
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

analogJoyStick JS(JOY_X_PIN, JOY_Y_PIN, 5);

ILI9488SPI_8C lcd(480, 320);

hw_timer_t * timer = NULL;

bool oneSecFlag = false;

void IRAM_ATTR onTimer()
{
	oneSecFlag = true;
}

unsigned int gb_rom_size;

void setup()
{
	Serial.begin(115200);

	// Our platform related code
	// Setup the IOs
	pinMode(34, INPUT);		// Touch pannel interrupt
	pinMode(36, ANALOG);	// Joystick X
	pinMode(39, ANALOG);	// Joystick Y
	pinMode(MP_BUTTON_1, INPUT);
	pinMode(MP_BUTTON_2, INPUT);
	// Setup the speaker
	pinMode(SPEAKER_PIN, OUTPUT);
	ledcSetup(0, 2000, 8);
	ledcAttachPin(SPEAKER_PIN, 0);

	// Initialize the LCD screen
	if (!lcd.init(18, 19, 23, 5, 40000000L/*64000000L - is working*/, singleFrameBuffer))
	{
		printf("Cannot initialize LCD!");
		while (1);
	}
	else
	{
		printf("LCD initialized...\n");
	}
	lcd.fillScr(0, 0, 0);
	lcd.flushFrameBuffer();
	printf("Joystick initialization, please don't touch it!!\n");
	JS.init();
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

	// Emulator code:
	rom_init(SuperMarioLand); gb_rom_size = 65536;
	//rom_init(Alleyway); gb_rom_size = 32768;
	//rom_init(MsPacMan); gb_rom_size = 65536;
	//rom_init(SolarStriker); gb_rom_size = 65536;
	//rom_init(SpaceInvaders); gb_rom_size = 32768;
	//rom_init(R_Type); gb_rom_size = 131072;
	printf("ROM Init Done!\n");

	sdl_init();

	if (mem_init() == 0)
	{
		printf("mem_init: failed!\n");
		lcd.setColor(0, 0, 0);
		lcd.drawString("MEM INIT FAILED", 30, 150, 20);
		lcd.flushFrameBuffer();
		while (1);
	}

	printf("Heap size = %x\n", ESP.getHeapSize());
	printf("Mem OK!\n");

	cpu_init();
	printf("CPU OK!\n");

	timer = timerBegin(0, 80, true); // Set to run @ 1MHz
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, 1000000, true);
	timerAlarmEnable(timer);
}

unsigned long measure, i = 0;

extern unsigned long frames;
void loop()
{
	if (i == 10000)
	{
		measure = ESP.getCycleCount();
		cpu_cycle();
		printf("1 %d\n", ESP.getCycleCount() - measure);

		measure = ESP.getCycleCount();
		lcd_cycle();
		printf("2 %d\n", ESP.getCycleCount() - measure);

		measure = ESP.getCycleCount();
		timer_cycle();
		printf("3 %d\n\n", ESP.getCycleCount() - measure);
		i = 0;
	}
	else
	{
		//i++;
		cpu_cycle();
		lcd_cycle();
		timer_cycle();
		if (oneSecFlag)
		{
			printf("FPS: %d\n", frames);
			oneSecFlag = false;
			frames = 0;
		}
	}
}