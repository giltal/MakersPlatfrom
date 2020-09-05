#include <GeneralLib.h>
#include <Adafruit_FT6206.h>

bool readTouchPannel(int * xPos, int * yPos, Adafruit_FT6206 * TS)
{
	TS_Point p;
	if (TOUCH_PANNEL_TOUCHED())
	{
		p = TS->getPoint();

		*xPos = 480 - p.y;
		*yPos = p.x;

		return true;
	}
	else
		return false;
}

bool readTouchPannel(TouchAreaElement * element, Adafruit_FT6206 * TS)
{
	TS_Point p;
	int x, y;
	if (TOUCH_PANNEL_TOUCHED())
	{
		p = TS->getPoint();

		x = 480 - p.y;
		y = p.x;
		if ((x >= element->x1) && (x <= element->x2) && (y >= element->y1) && (y <= element->y2))
		{
			return true;
		}
	}
	
	return false;
}

int getInputFromTouchPannel(TouchAreaElement elementsArray[], unsigned int numOfElements, Adafruit_FT6206 * TS, unsigned int timeOutMs, bool * timeOut) 
{
	int x, y,i;
	unsigned int counter = timeOutMs;
	bool noTimeOut = false;
	
	if (counter == 0)
	{
		noTimeOut = true;
		counter = 1;
	}
	
	do
	{
		if (noTimeOut == false)
		{
			counter--;
			delay(1);
		}
		
		if (readTouchPannel(&x, &y,TS))
		{
			for (i = 0; i < numOfElements; i++)
			{
				if ((x >= elementsArray[i].x1) && (x <= elementsArray[i].x2)
					&& (y >= elementsArray[i].y1) && (y <= elementsArray[i].y2))
				{
					*timeOut = false;
					return elementsArray[i].key;
				}
			}
		}
#ifdef ESP
		ESP.wdtFeed();
#endif
	} while (counter);
	*timeOut = true;
	return elementsArray[0].key;
}

analogJoyStick::analogJoyStick(unsigned int analogPinForXaxis, unsigned int analogPinForYaxis, unsigned int deadZoneVal)
{
	XaxisPin = analogPinForXaxis;
	YaxisPin = analogPinForYaxis;
	deadZone = deadZoneVal % 2048;
	xInitVal = 0;
	yInitVal = 0;
}

void analogJoyStick::init()
{
	xInitVal = analogRead(XaxisPin);
	yInitVal = analogRead(YaxisPin);
	printf("xInitVal = %d yInitVal = %d\n", xInitVal, yInitVal);
}

analogJoyStick::~analogJoyStick()
{
}

int	analogJoyStick::readXaxis()
{
	int readOut;
	readOut = analogRead(XaxisPin);
	if (readOut < (xInitVal - deadZone))
	{
		return map(readOut, 0, xInitVal, -2048, 0);
	}
	if (readOut > (xInitVal + deadZone))
	{
		return map(readOut, xInitVal, 4096 , 0, 2048);
	}
	return 0;
}

int	analogJoyStick::readYaxis()
{
	int readOut;
	readOut = analogRead(YaxisPin);
	if (readOut < (yInitVal - deadZone))
	{
		return map(readOut, yInitVal, 0, 0, 2048);
	}
	if (readOut > (yInitVal + deadZone))
	{
		return map(readOut, yInitVal, 4096, 0 , -2048);
	}
	return 0;
}

bool analogJoyStick::updateDeadZoneVal(unsigned int deadZoneVal)
{
	if (deadZoneVal > 512)
		return false;
	deadZone = deadZoneVal;
		return true;
}