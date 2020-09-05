// General Lib - Gil Tal
// Ver 0.1
#ifndef __INCGeneralLibh
#define __INCGeneralLibh

#include <Adafruit_FT6206.h>

struct TouchAreaElement
{
	unsigned int x1, y1;
	unsigned int x2, y2;
	int key;
};

#define nESP8266
#define ESP32

#ifdef ESP8266
	#define TOUCH_PANNEL_TOUCHED()	(analogRead(A0) < 100)
#elif defined(ESP32)
	//#define TOUCH_PANNEL_TOUCHED()	(analogRead(34) < 100)
	#define TOUCH_PANNEL_TOUCHED()	(digitalRead(34) == LOW)
#else // Arduino Due
	#define TOUCH_PANNEL_STATUS()	digitalRead(34)
#endif 

bool readTouchPannel(int * xPos, int * yPos, Adafruit_FT6206 * TS);
bool readTouchPannel(TouchAreaElement * element, Adafruit_FT6206 * TS);

int	 getInputFromTouchPannel(TouchAreaElement elementsArray[], unsigned int numOfElements, Adafruit_FT6206 * TS,unsigned int timeOutMs,bool * timeOut); // TimeOutMs = 0 => no time out

class analogJoyStick
{
private:
	unsigned int XaxisPin, YaxisPin;
	int			 xInitVal, yInitVal;
	int deadZone;
public:
	analogJoyStick(unsigned int analogPinForXaxis, unsigned int analogPinForYaxis, unsigned int deadZoneVal);
	~analogJoyStick();

	int		readXaxis();
	int		readYaxis();
	void	init(); // When called joystick must not touched 
	bool	updateDeadZoneVal(unsigned int deadZoneVal);
};

#endif // GeneralLib