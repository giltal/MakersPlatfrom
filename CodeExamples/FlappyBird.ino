/*
 Name:		FLAPPYBird.ino
 Created:	12/31/2019 6:40:39 PM
 Author:	giltal
*/

//
//  FLAPPY BIRD ARDUINO
//

#include <GeneralLib.h>
#include <Adafruit_FT6206.h>
#include "PCF8574.h"
#include <Wire.h>
#include <RTClib.h>
#include "graphics.h"

PCF8574 pcf8574(0x39);
Adafruit_FT6206 ts = Adafruit_FT6206();
ILI9488SPI_8C lcd(480, 320);

#define SPEAKER_PIN 5
#define MP_BUTTON_1	27
#define MP_BUTTON_2	33
#define TOUCH_1_PIN	15
#define TOUCH_2_PIN	32

// (we can change the size of the game easily that way)
#define TFTW				480     // screen width
#define TFTH				320     // screen height
#define TFTW2				240     // half screen width
#define TFTH2				160     // half screen height
// game constant
#define SPEED             2
#define GRAVITY         9.8
#define JUMP_FORCE		2.15
#define SKIP_TICKS		22.0     // 1000 / 50fps
#define MAX_FRAMESKIP    22
// bird size
#define BIRDW             40     // bird width
#define BIRDH             40     // bird height
#define BIRDW2            20     // half width
#define BIRDH2            20     // half height
// pipe size
#define PIPEW            36     // pipe width
#define GAPHEIGHT        96     // pipe gap height
// floor size
#define FLOORH           20     // floor height (from bottom of the screen)
// grass size
#define GRASSH            4     // grass height (inside floor, starts at floor y)

// background
#define BCKGRDCOL	0,0,1
// pipe
#define PIPECOL		0,1,0
// pipe highlight
#define PIPEHIGHCOL 1,1,0
// pipe seam
#define PIPESEAMCOL 0,1,1
// floor
#define FLOORCOL    1,1,1
// grass (col2 is the stripe color)
#define GRASSCOL    1,0,1
#define GRASSCOL2   0,1,1



// bird sprite
// bird sprite colors (Cx name for values to keep the array readable)

const unsigned char FlappyBird[] =
{
0x00,0x28,0x00,0x28,
0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x00,
0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x44,0x40,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x44,0x44,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x44,0x44,0x40,0x99,0x99,
0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x00,0x00,0x00,0x04,0x44,0x44,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x04,
0x44,0x44,0x44,0x00,0x44,0x44,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x04,0x44,0x44,0x44,0x40,0x04,0x44,0x40,0x99,0x99,0x99,0x99,0x99,
0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x04,0x44,0x44,0x44,0x00,0x44,0x40,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x04,0x44,0x44,
0x44,0x44,0x44,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x00,0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
0x99,0x99,0x99,0x90,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
0x44,0x09,0x99,0x99,0x99,0x99,0x90,0x00,0x09,0x99,0x04,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x99,0x99,0x99,0x99,0x00,0x00,0x09,0x90,0x44,0x44,
0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x99,0x99,0x99,0x99,0x90,0x00,0x99,0x04,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x00,0x99,
0x99,0x00,0x90,0x00,0x90,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x99,0x90,0x00,0x09,0x00,0x00,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x99,0x90,0x00,0x09,0x00,0x04,0x44,0x44,0x44,0x44,0x44,0x40,0x00,0x44,0x44,0x44,0x44,0x44,0x00,0x44,0x09,0x99,0x00,0x00,0x00,
0x04,0x44,0x44,0x44,0x44,0x44,0x00,0x00,0x04,0x44,0x44,0x44,0x00,0x00,0x04,0x09,0x99,0x99,0x00,0x00,0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x00,0x00,0x04,0x44,0x40,
0x00,0x00,0x04,0x09,0x99,0x99,0x00,0x00,0x44,0x44,0x44,0x44,0x40,0x44,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x44,0x09,0x99,0x00,0x00,0x04,0x44,0x44,0x44,0x44,
0x44,0x04,0x44,0x07,0x70,0x00,0x00,0x00,0x07,0x74,0x44,0x09,0x99,0x00,0x09,0x04,0x44,0x44,0x40,0x44,0x44,0x04,0x40,0x77,0x77,0x77,0x00,0x07,0x77,0x70,0x44,0x09,
0x99,0x09,0x99,0x04,0x44,0x44,0x44,0x44,0x40,0x44,0x40,0x77,0x70,0x07,0x70,0x70,0x77,0x70,0x44,0x09,0x99,0x99,0x99,0x04,0x44,0x44,0x44,0x04,0x44,0x44,0x40,0x77,
0x77,0x77,0x70,0x00,0x00,0x00,0x44,0x09,0x99,0x99,0x99,0x94,0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x00,0x00,0x00,0x46,0x40,0x04,0x44,0x44,0x09,0x99,0x99,0x99,0x90,
0x44,0x44,0x44,0x44,0x44,0x44,0x40,0x44,0x44,0x06,0x66,0x66,0x00,0x40,0x44,0x09,0x99,0x99,0x99,0x90,0x44,0x44,0x44,0x77,0x77,0x77,0x77,0x74,0x40,0x06,0x66,0x66,
0x66,0x04,0x44,0x09,0x99,0x99,0x99,0x90,0x44,0x47,0x77,0x77,0x77,0x77,0x77,0x77,0x70,0x70,0x66,0x66,0x66,0x60,0x44,0x09,0x99,0x99,0x99,0x99,0x07,0x77,0x77,0x77,
0x77,0x77,0x77,0x77,0x70,0x70,0x00,0x06,0x66,0x64,0x40,0x99,0x99,0x99,0x99,0x99,0x07,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x06,0x66,0x40,0x66,0x66,0x40,0x99,
0x99,0x99,0x99,0x99,0x90,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x06,0x66,0x60,0x04,0x66,0x00,0x99,0x99,0x99,0x99,0x99,0x99,0x07,0x77,0x77,0x77,0x77,0x77,0x77,
0x77,0x70,0x66,0x07,0x77,0x44,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x00,0x77,0x77,0x00,0x99,0x99,0x99,0x99,0x99,0x99,
0x99,0x99,0x07,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x70,0x09,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x07,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
0x00,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x90,0x07,0x77,0x77,0x77,0x77,0x77,0x74,0x00,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
0x90,0x00,0x04,0x77,0x74,0x00,0x00,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x00,0x00,0x00,0x99,0x99,0x99,0x99,0x99,0x99,0x99
};
const unsigned char AngryBird[] = {
0x00,0x28,0x00,0x28,
0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x88,
0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x44,0x48,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x44,0x44,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x44,0x44,0x48,0x11,0x11,
0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x88,0x88,0x88,0x84,0x44,0x44,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x84,
0x44,0x44,0x44,0x88,0x44,0x44,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x84,0x44,0x44,0x44,0x48,0x84,0x44,0x48,0x11,0x11,0x11,0x11,0x11,
0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x84,0x44,0x44,0x44,0x88,0x44,0x48,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x84,0x44,0x44,
0x44,0x44,0x44,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x88,0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
0x11,0x11,0x11,0x18,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
0x44,0x81,0x11,0x11,0x11,0x11,0x18,0x88,0x81,0x11,0x84,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x11,0x11,0x11,0x11,0x88,0x88,0x81,0x18,0x44,0x44,
0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x11,0x11,0x11,0x11,0x18,0x88,0x11,0x84,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x88,0x11,
0x11,0x88,0x18,0x88,0x18,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x11,0x18,0x88,0x81,0x88,0x88,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x11,0x18,0x88,0x81,0x88,0x84,0x44,0x44,0x44,0x44,0x44,0x48,0x88,0x44,0x44,0x44,0x44,0x44,0x88,0x44,0x81,0x11,0x88,0x88,0x88,
0x84,0x44,0x44,0x44,0x44,0x44,0x88,0x88,0x84,0x44,0x44,0x44,0x88,0x88,0x84,0x81,0x11,0x11,0x88,0x88,0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x88,0x88,0x84,0x44,0x48,
0x88,0x88,0x84,0x81,0x11,0x11,0x88,0x88,0x44,0x44,0x44,0x44,0x48,0x44,0x44,0x88,0x88,0x88,0x88,0x88,0x88,0x84,0x44,0x81,0x11,0x88,0x88,0x84,0x44,0x44,0x44,0x44,
0x44,0x84,0x44,0x87,0x78,0x88,0x88,0x88,0x87,0x74,0x44,0x81,0x11,0x88,0x81,0x84,0x44,0x44,0x48,0x44,0x44,0x84,0x48,0x77,0x77,0x77,0x88,0x87,0x77,0x78,0x44,0x81,
0x11,0x81,0x11,0x84,0x44,0x44,0x44,0x44,0x48,0x44,0x48,0x77,0x78,0x87,0x78,0x78,0x77,0x78,0x44,0x81,0x11,0x11,0x11,0x84,0x44,0x44,0x44,0x84,0x44,0x44,0x48,0x77,
0x77,0x77,0x78,0x88,0x88,0x88,0x44,0x81,0x11,0x11,0x11,0x14,0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x88,0x88,0x88,0x46,0x48,0x84,0x44,0x44,0x81,0x11,0x11,0x11,0x18,
0x44,0x44,0x44,0x44,0x44,0x44,0x48,0x44,0x44,0x86,0x66,0x66,0x88,0x48,0x44,0x81,0x11,0x11,0x11,0x18,0x44,0x44,0x44,0x77,0x77,0x77,0x77,0x74,0x48,0x86,0x66,0x66,
0x66,0x84,0x44,0x81,0x11,0x11,0x11,0x18,0x44,0x47,0x77,0x77,0x77,0x77,0x77,0x77,0x78,0x78,0x66,0x66,0x66,0x68,0x44,0x81,0x11,0x11,0x11,0x11,0x87,0x77,0x77,0x77,
0x77,0x77,0x77,0x77,0x78,0x78,0x88,0x86,0x66,0x64,0x48,0x11,0x11,0x11,0x11,0x11,0x87,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x86,0x66,0x48,0x66,0x66,0x48,0x11,
0x11,0x11,0x11,0x11,0x18,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x86,0x66,0x68,0x84,0x66,0x88,0x11,0x11,0x11,0x11,0x11,0x11,0x87,0x77,0x77,0x77,0x77,0x77,0x77,
0x77,0x78,0x66,0x87,0x77,0x44,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x88,0x77,0x77,0x88,0x11,0x11,0x11,0x11,0x11,0x11,
0x11,0x11,0x87,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x78,0x81,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x87,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,
0x88,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x18,0x87,0x77,0x77,0x77,0x77,0x77,0x74,0x88,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
0x18,0x88,0x84,0x77,0x74,0x88,0x88,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x88,0x88,0x88,0x11,0x11,0x11,0x11,0x11,0x11,0x11
};


// bird structure
typedef struct _bird {
	int x, y, old_y;
	int col;
	float vel_y;
} BIRD;

// pipe structure
typedef struct _pipeF {
	int x, gap_y;
	int col;
} PIPEF;

// score
static short score;
// temporary x and y var
static short tmpx, tmpy;

BIRD bird;
PIPEF pipef;

void setup()
{
	pinMode(34, INPUT);
	pinMode(36, ANALOG);
	pinMode(39, ANALOG);
	pinMode(MP_BUTTON_1, INPUT);
	pinMode(MP_BUTTON_2, INPUT);
	// Setup the speaker
	pinMode(SPEAKER_PIN, OUTPUT);
	ledcSetup(0, 2000, 8);
	ledcAttachPin(SPEAKER_PIN, 0);

	Serial.begin(115200);
	// Setup the LCD screen
	if (!lcd.init(18, 19, 23, 5, 40000000L, singleFrameBuffer))
	{
		printf("Cannot initialize LCD!");
		while (1);
	}
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

// ---------------
// main loop
// ---------------
void loop()
{
	game_start();
	game_loop();
	game_over();
}

// ---------------
// game loop
// ---------------
void game_loop()
{
	char tempStr[10];
	float speed = 2.0;
	// ===============
	// prepare game variables
	// ===============
	// instead of calculating the distance of the floor from the screen height each time store it in a variable
	unsigned short GAMEH = TFTH - FLOORH;
	// game loop time variables
	float delta, old_time, current_time;
	current_time = millis();

	// passed pipef flag to count score
	bool passed_pipe = false;
	pipef.gap_y = random(10, GAMEH - (10 + GAPHEIGHT));
	while (1)
	{
		lcd.fillScr(BCKGRDCOL);

		lcd.setColor(0, 0, 0);
		lcd.drawHLine(0, GAMEH, TFTW);
		lcd.setColor(0, 0, 0);
		lcd.drawHLine(0, GAMEH + GRASSH, TFTW);

		// ===============
		// input
		// ===============
		if (digitalRead(MP_BUTTON_1) == LOW || TOUCH_PANNEL_TOUCHED())
		{
			// if the bird is not too close to the top of the screen apply jump force
			if (bird.y > BIRDH2*0.5) bird.vel_y = -JUMP_FORCE;
			// else zero velocity
			else bird.vel_y = 0;
		}

		// ===============
		// update
		// ===============
		// calculate delta time
		// ---------------
		old_time = current_time;
		current_time = millis();
		delta = (current_time - old_time) / 1000.0;

		// bird
		// ---------------
		bird.vel_y += GRAVITY * delta;
		bird.y += bird.vel_y;

		// pipef
		// ---------------
		pipef.x -= speed;
		// if pipef reached edge of the screen reset its position and gap
		if (pipef.x < 0)
		{
			pipef.x = TFTW - PIPEW;
			pipef.gap_y = random(10, GAMEH - (10 + GAPHEIGHT));
		}

		// ===============
		// draw
		// ===============
		// pipef
		// ---------------
		// we save cycles if we avoid drawing the pipef when outside the screen
		if (pipef.x >= 0 && pipef.x < TFTW)
		{
			// pipef color
			lcd.setColor(PIPECOL);
			lcd.drawRect(pipef.x, 0, pipef.x + PIPEW, pipef.gap_y, true);
			lcd.drawRect(pipef.x, pipef.gap_y + GAPHEIGHT + 1, pipef.x + PIPEW, GAMEH, true);
		}

		// bird
		lcd.drawBitmap(bird.x, bird.y, FlappyBird, true);
		// save position to erase bird on next draw
		//bird.old_y = bird.y;


		// ===============
		// collision
		// ===============
		// if the bird hit the ground game over
		if (bird.y > (GAMEH - BIRDH))
			break;
		// checking for bird collision with pipef
		//if ((bird.x + BIRDW) >= (pipef.x - BIRDW2) && bird.x <= (pipef.x + PIPEW - BIRDW)) 
		if ((bird.x + BIRDW) >= (pipef.x) && bird.x <= (pipef.x + PIPEW))
		{
			// bird entered a pipef, check for collision
			if (bird.y < pipef.gap_y || (bird.y + BIRDH) >(pipef.gap_y + GAPHEIGHT))
				break;
			else
				passed_pipe = true;
		}
		// if bird has passed the pipef increase score
		else if (bird.x > pipef.x + PIPEW - BIRDW && passed_pipe)
		{
			passed_pipe = false;
			score++;
		}

		// update score
		lcd.setColor(1, 1, 1);
		sprintf(tempStr, "%d", score);
		lcd.drawString(tempStr, TFTW2, 4, 20);
		lcd.flushFrameBuffer();
	}

	// add a small delay to show how the player lost
	delay(1000);
}

// ---------------
// game start
// ---------------
void game_start()
{
	lcd.fillScr(0, 0, 0);
	lcd.setColor(1, 1, 1);
	lcd.drawBitmap(50, 150, FlappyBird, true);
	lcd.drawBitmap(370, 150, FlappyBird, true);

	lcd.drawString("FLAPPY BIRD", 100, 150, 20);
	lcd.drawString("PRESS BUTTON to start", 110, 180, 10);
	lcd.flushFrameBuffer();

	sound(523, 500);
	sound(587, 500);
	sound(659, 500);

	while (!TOUCH_PANNEL_TOUCHED());
	// init game settings
	game_init();
}

// ---------------
// game init
// ---------------
void game_init()
{
	// clear screen
	//TFT.fillScreen(BCKGRDCOL);
	lcd.fillScr(BCKGRDCOL);
	// reset score
	score = 0;
	// init bird
	bird.x = 60;
	bird.y = bird.old_y = TFTH2 - BIRDH;
	bird.vel_y = -JUMP_FORCE;
	tmpx = tmpy = 0;
	// generate new random seed for the pipef gape
	randomSeed(ESP.getCycleCount());
	// init pipef
	pipef.x = TFTW;
}

// ---------------
// game over
// ---------------
void game_over()
{
	char tempStr[50];
	lcd.fillScr(0, 0, 0);
	lcd.setColor(1, 1, 1);
	lcd.drawString("GAME OVER", 110, 50, 25);
	sprintf(tempStr, "SCORE %d", score);
	lcd.setColor(0, 1, 0);
	lcd.drawString(tempStr, 130, 100, 25);
	lcd.setColor(0, 0, 1);
	lcd.drawString("PRESS BUTTON", 70, 150, 25);
	lcd.flushFrameBuffer();
	while (digitalRead(MP_BUTTON_1) == HIGH && !TOUCH_PANNEL_TOUCHED());
}