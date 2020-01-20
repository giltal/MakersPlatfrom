#include "graphics.h"
#include "SPI.h"

#define ESP32

/*
 General graphics lib to be inherited
*/
graphics::graphics(short maxX, short maxY)
{	
	this->maxX = maxX;
	this->maxY = maxY;
}

void graphics::drawRect(short x1, short y1, short x2, short y2, bool fill)
{
	if (x1 > x2)
	{
		swap(&x1, &x2);
	}
	if (y1 > y2)
	{
		swap(&y1, &y2);
	}

	if (fill)
	{
		for (int i = 0; i < ((y2 - y1) / 2) + 1; i++)
		{
			drawHLine(x1, y1 + i, x2 - x1);
			drawHLine(x1, y2 - i, x2 - x1);
		}
	}
	else
	{
		drawHLine(x1, y1, x2 - x1);
		drawHLine(x1, y2, x2 - x1);
		drawVLine(x1, y1, y2 - y1);
		drawVLine(x2, y1, y2 - y1);
	}
}

void graphics::drawRoundRect(short x1, short y1, short x2, short y2, bool fill)
{
	if (x1 > x2)
	{
		swap(&x1, &x2);
	}
	if (y1 > y2)
	{
		swap(&y1, &y2);
	}
	if ((x2 - x1) > 4 && (y2 - y1) > 4)
	{
		if (fill)
		{
			for (int i = 0; i < ((y2 - y1) / 2) + 1; i++)
			{
				switch (i)
				{
				case 0:
					drawHLine(x1 + 2, y1 + i, x2 - x1 - 4);
					drawHLine(x1 + 2, y2 - i, x2 - x1 - 4);
					break;
				case 1:
					drawHLine(x1 + 1, y1 + i, x2 - x1 - 2);
					drawHLine(x1 + 1, y2 - i, x2 - x1 - 2);
					break;
				default:
					drawHLine(x1, y1 + i, x2 - x1);
					drawHLine(x1, y2 - i, x2 - x1);
				}
			}
		}
		else
		{
			drawPixel(x1 + 1, y1 + 1);
			drawPixel(x2 - 1, y1 + 1);
			drawPixel(x1 + 1, y2 - 1);
			drawPixel(x2 - 1, y2 - 1);
			drawHLine(x1 + 2, y1, x2 - x1 - 4);
			drawHLine(x1 + 2, y2, x2 - x1 - 4);
			drawVLine(x1, y1 + 2, y2 - y1 - 4);
			drawVLine(x2, y1 + 2, y2 - y1 - 4);
		}
	}
}

void graphics::drawCircle(short x, short y, int radius, bool fill)
{
	if (fill)
	{
		for (int y1 = -radius; y1 <= 0; y1++)
			for (int x1 = -radius; x1 <= 0; x1++)
				if (x1*x1 + y1 * y1 <= radius * radius)
				{
					drawHLine(x + x1, y + y1, 2 * (-x1));
					drawHLine(x + x1, y - y1, 2 * (-x1));
					break;
				}
	}
	else
	{
		int f = 1 - radius;
		int ddF_x = 1;
		int ddF_y = -2 * radius;
		int x1 = 0;
		int y1 = radius;

		drawPixel(x, y + radius);
		drawPixel(x, y - radius);
		drawPixel(x + radius, y);
		drawPixel(x - radius, y);

		while (x1 < y1)
		{
			if (f >= 0)
			{
				y1--;
				ddF_y += 2;
				f += ddF_y;
			}
			x1++;
			ddF_x += 2;
			f += ddF_x;
			drawPixel(x + x1, y + y1);
			drawPixel(x - x1, y + y1);
			drawPixel(x + x1, y - y1);
			drawPixel(x - x1, y - y1);
			drawPixel(x + y1, y + x1);
			drawPixel(x - y1, y + x1);
			drawPixel(x + y1, y - x1);
			drawPixel(x - y1, y - x1);
		}
	}
}

void graphics::drawChar(unsigned char c, short x, short y, float size, float factor)
{
	float s = size, _factor;
	_factor = s * factor;
	switch (c)
	{
	case 'a':
	case 'A':
		drawLine(x, y + (unsigned int)s, x + (unsigned int)(_factor / 4), y);                /*  line no I    */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /*  line no II   */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)s);          /*  line no III  */
		drawLine(x + (unsigned int)(_factor / 8), y + (unsigned int)(s / 2), x + (unsigned int)(7 * _factor / 8), y + (unsigned int)(s / 2));  /*  line no IV   */
		break;

	case 'b':
	case 'B':
		drawLine(x, y, x, y + (unsigned int)s);                      /*  line no I    */
		drawLine(x, y, x + (unsigned int)(3 * _factor / 4), y);                /*  line no II   */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*  line no III  */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(s / 4));    /*  line no IV   */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));        /*  line no V    */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));  /*  line no VI   */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));    /*  line no VII  */
		drawLine(x, y + (unsigned int)s, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);            /*  line no VIII */
		break;

	case 'c':
	case 'C':
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)_factor, y);              /*  line no I    */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*  line no II   */
		drawLine(x, y + (unsigned int)(s / 4), x, y + (unsigned int)(3 * s / 4));              /*  line no III  */
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)s);          /*  line no IV   */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)s);          /*  line no V    */
		break;

	case 'd':
	case 'D':
		drawLine(x, y, x, y + (unsigned int)s);                      /*  line no I    */
		drawLine(x, y, x + (unsigned int)(3 * _factor / 4), y);                /*  line no II   */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*  line no III  */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));      /*  line no IV   */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /*  line no V    */
		drawLine(x, y + (unsigned int)s, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);
		break;

	case 'e':
	case 'E':
		drawLine(x, y + (unsigned int)(s / 4), x, y + (unsigned int)(3 * s / 4));              /*  line no I    */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*  line no II   */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)_factor, y);              /*  line no III  */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(s / 2));            /*  line no VI   */
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)s);          /*  line no V    */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)s);          /*  line no VI   */
		break;

	case 'f':
	case 'F':
		drawLine(x, y, x, y + (unsigned int)s);                      /*  line no I    */
		drawLine(x, y, x + (unsigned int)(_factor / 2), y);                  /*  line no II   */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2));          /*  line no III  */
		break;

	case 'g':
	case 'G':
		drawLine(x, y + (unsigned int)(s / 4), x, y + (unsigned int)(3 * s / 4));              /*  line no I    */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*  line no II   */
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)s);          /*  line no III  */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /*  line no IV   */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);      /*  line no V    */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));    /*  line no VI   */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)_factor, y + (unsigned int)(s / 2));      /*  line no VII  */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 2), x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2));      /*  line no VIII */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*  line no IX   */
		break;

	case 'h':
	case 'H':
		drawLine(x, y, x, y + (unsigned int)s);                      /*  line no I    */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(s / 2));            /*  line no II   */
		drawLine(x + (unsigned int)_factor, y, x + (unsigned int)_factor, y + (unsigned int)s);              /*  line no III  */
		break;

	case 'i':
	case 'I':
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /*  line no I    */
		drawLine(x + (unsigned int)(_factor / 2), y, x + (unsigned int)(_factor / 2), y + (unsigned int)s);          /*  line no II   */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);      /*  line no III  */
		break;

	case 'j':
	case 'J':
		drawLine(x, y, x + (unsigned int)_factor, y);                    /*  line no I    */
		drawLine(x + (unsigned int)(_factor / 2), y, x + (unsigned int)(_factor / 2), y + (unsigned int)s);          /*  line no II   */
		drawLine(x + (unsigned int)(_factor / 2), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);        /*  line no III  */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));          /*  line no IV   */
		break;

	case 'k':
	case 'K':
		drawLine(x, y, x, y + (unsigned int)s);                       /*  line no I    */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)_factor, y);                 /*  line no II   */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)s);               /*  line no III  */
		break;

	case 'l':
	case 'L':
		drawLine(x, y, x, y + (unsigned int)s);                        /*  line no I    */
		drawLine(x, y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)s);                  /*  line no II   */
		break;

	case 'm':
	case 'M':
		drawLine(x, y, x, y + (unsigned int)s);                        /*  line no I    */
		drawLine(x, y, x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2));                /*  line no II   */
		drawLine(x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y);            /*  line no III  */
		drawLine(x + (unsigned int)_factor, y, x + (unsigned int)_factor, y + (unsigned int)s);                /*  line no IV   */
		break;

	case 'n':
	case 'N':
		drawLine(x, y, x, y + (unsigned int)s);                        /*  line no I    */
		drawLine(x, y, x + (unsigned int)_factor, y + (unsigned int)s);                    /*  line no II   */
		drawLine(x + (unsigned int)_factor, y, x + (unsigned int)_factor, y + (unsigned int)s);                /*  line no III  */
		break;

	case 'o':
	case 'O':
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /* line no I     */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /* line no II    */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /* line no III   */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));      /* line no IV    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /* line no V     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);      /* line no VI    */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));          /* line no VII   */
		drawLine(x, y + (unsigned int)(3 * s / 4), x, y + (unsigned int)(s / 4));              /* line no VIII  */
		break;

	case 'p':
	case 'P':
		drawLine(x, y, x, y + (unsigned int)s);                      /*  line no I    */
		drawLine(x, y, x + (unsigned int)(3 * _factor / 4), y);                /*  line no II   */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));        /*  line no III  */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*  line no IV   */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));    /*  line no V    */
		break;

	case 'q':
	case 'Q':
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /* line no I     */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /* line no II    */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /* line no III   */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));      /* line no IV    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /* line no V     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);      /* line no VI    */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));          /* line no VII   */
		drawLine(x, y + (unsigned int)(3 * s / 4), x, y + (unsigned int)(s / 4));              /* line no VIII  */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(3 * s / 4), x + (unsigned int)_factor, y + (unsigned int)s);    /* line no IX    */
		break;

	case 'r':
	case 'R':
		drawLine(x, y, x, y + (unsigned int)s);                      /*  line no I     */
		drawLine(x, y, x + (unsigned int)(3 * _factor / 4), y);                /*  line no II    */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));        /*  line no III   */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*  line no IV    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));    /*  line no VI    */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)s);              /*  line no VII   */
		break;

	case 's':
	case 'S':
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*  line no I     */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /*  line no II    */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*  line no III   */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2));          /*  line no IV    */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));  /*  line no V     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));  /*  line no VI    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /*  line no VII   */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);      /*  line no VIII  */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));          /*  line no IX    */
		break;

	case 't':
	case 'T':
		drawLine(x, y, x + (unsigned int)_factor, y);                  /*   line no I     */
		drawLine(x + (unsigned int)(_factor / 2), y, x + (unsigned int)(_factor / 2), y + (unsigned int)s);        /*   line no II    */
		break;

	case 'u':
	case 'U':
		drawLine(x, y, x, y + (unsigned int)(3 * s / 4));                /* line no I       */
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)s);        /* line no II      */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /* line no III     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));  /* line no IV      */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)_factor, y);        /* line no V       */
		break;

	case 'v':
	case 'V':
		drawLine(x, y, x + (unsigned int)(_factor / 2), y + (unsigned int)s);             /*  line no I      */
		drawLine(x + (unsigned int)(_factor / 2), y + (unsigned int)s, x + (unsigned int)_factor, y);         /*  line no II     */
		break;

	case 'w':
	case 'W':
		drawLine(x, y, x, y + (unsigned int)s);                    /*  line no I      */
		drawLine(x, y + (unsigned int)s, x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2));          /*  line no II     */
		drawLine(x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)s);      /*  line no III    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)s, x + (unsigned int)_factor, y);            /*  line no IV     */
		break;

	case 'x':
	case 'X':
		drawLine(x, y, x + (unsigned int)_factor, y + (unsigned int)s);
		drawLine(x + (unsigned int)_factor, y, x, y + (unsigned int)s);
		break;

	case 'y':
	case 'Y':
		drawLine(x, y, x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2));            /*  line no I      */
		drawLine(x + (unsigned int)_factor, y, x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2));        /*  line no II     */
		drawLine(x + (unsigned int)(_factor / 2), y + (unsigned int)(s / 2), x + (unsigned int)(_factor / 2), y + (unsigned int)s);    /*  line no III    */
		break;

	case 'z':
	case 'Z':
		drawLine(x, y, x + (unsigned int)_factor, y);                    /*line no I       */
		drawLine(x + (unsigned int)_factor, y, x, y + (unsigned int)s);                  /*line no II      */
		drawLine(x, y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)s);                /*line no III     */
		break;

	case '0':
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);            /* line no I       */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);        /* line no II      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));      /* line no III     */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));    /* line no IV      */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);  /* line no V       */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);    /* line no VI      */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));        /* line no VII     */
		drawLine(x, y + (unsigned int)(3 * s / 4), x, y + (unsigned int)(s / 4));            /* line no VIII    */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);      /* line no IX      */
		break;

	case '1':
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 2), y);      /* line no I      */
		drawLine(x + (unsigned int)(_factor / 2), y, x + (unsigned int)(_factor / 2), y + (unsigned int)s);        /* line no II     */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /* line no III    */
		break;

	case '2':
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);             /* line no I      */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);         /* line no II     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));       /* line no III    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)_factor, y + (unsigned int)(s / 2));       /* line no IV     */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 2), x, y + (unsigned int)s);             /* line no V      */
		drawLine(x, y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)s);               /* line no IV     */
		break;

	case '3':
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);             /* line no I      */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);         /* line no II     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));       /* line no III    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));   /* line no IV     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4)); /* line no V      */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);   /* line no VI     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);     /* line no VII    */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));         /* line no VIII   */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2)); /* line no IX     */
		break;

	case '4':
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x, y + (unsigned int)(3 * s / 4));        /* line no I      */
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y);          /*line no II      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);      /*line no III     */
		break;

	case '5':
		drawLine(x + (unsigned int)_factor, y, x, y);                    /*line no I       */
		drawLine(x, y, x, y + (unsigned int)(s / 2));                    /*line no II      */
		drawLine(x, y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));        /*line no III     */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));  /*line no IV      */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /*line no V       */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x, y + (unsigned int)s);            /* line no VI     */
		break;
	case '6':
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2));        /*line no I       */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));  /*line no II      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));  /*line no III     */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /*line no IV      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);      /*line no V       */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));          /*line no VI      */
		drawLine(x, y + (unsigned int)(3 * s / 4), x, y + (unsigned int)(s / 4));              /*line no VII     */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*line no VIII    */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /*line no IX      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*line no X       */
		break;
	case '7':
		drawLine(x, y + (unsigned int)(s / 4), x, y);                    /*line no I       */
		drawLine(x, y, x + (unsigned int)_factor, y);                    /*line no II      */
		drawLine(x + (unsigned int)_factor, y, x, y + (unsigned int)s);                  /*line no III     */
		break;
	case '8':
		drawLine(x, y + (unsigned int)(3 * s / 4), x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2));        /*line no I       */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));  /*line no II      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));  /*line no III     */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /*line no IV      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x + (unsigned int)(_factor / 4), y + (unsigned int)s);      /*line no V       */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)s, x, y + (unsigned int)(3 * s / 4));          /*line no VI      */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2), x, y + (unsigned int)(s / 4));          /*line no VII     */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*line no VIII    */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /*line no IX      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*line no X       */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));    /*line no XI      */
		break;
	case '9':
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2));    /*line no I       */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)(s / 2), x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2));  /*line no II      */
		drawLine(x + (unsigned int)(_factor / 4), y + (unsigned int)(s / 2), x, y + (unsigned int)(s / 4));          /*line no III     */
		drawLine(x, y + (unsigned int)(s / 4), x + (unsigned int)(_factor / 4), y);              /*line no IV      */
		drawLine(x + (unsigned int)(_factor / 4), y, x + (unsigned int)(3 * _factor / 4), y);          /* line no V      */
		drawLine(x + (unsigned int)(3 * _factor / 4), y, x + (unsigned int)_factor, y + (unsigned int)(s / 4));        /*line no  VI     */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(s / 4), x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4));      /*line no  VII    */
		drawLine(x + (unsigned int)_factor, y + (unsigned int)(3 * s / 4), x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s);    /*line no VIII    */
		drawLine(x + (unsigned int)(3 * _factor / 4), y + (unsigned int)s, x, y + (unsigned int)s);            /*line no IX      */
		break;
	case '.':
		drawLine(x + (unsigned int)(3 * _factor / 8), y + (unsigned int)(7 * s / 8), x + (unsigned int)(5 * _factor / 8), y + (unsigned int)(7 * s / 8));
		drawLine(x + (unsigned int)(5 * _factor / 8), y + (unsigned int)(7 * s / 8), x + (unsigned int)(5 * _factor / 8), y + (unsigned int)s);
		drawLine(x + (unsigned int)(5 * _factor / 8), y + (unsigned int)s, x + (unsigned int)(3 * _factor / 8), y + (unsigned int)s);
		drawLine(x + (unsigned int)(3 * _factor / 8), y + (unsigned int)s, x + (unsigned int)(3 * _factor / 8), y + (unsigned int)(7 * s / 8));
		break;
	default:
		drawLine(x, y, x + (unsigned int)_factor, y);
		drawLine(x, y + (unsigned int)s, x + (unsigned int)_factor, y + (unsigned int)s);
		drawLine(x, y, x, y + (unsigned int)s);
		drawLine(x + (unsigned int)_factor, y, x + (unsigned int)_factor, y + (unsigned int)s);
	}

}

void graphics::drawString(char * string, short x, short y, float size, float factor)
{
	unsigned char   c;
	float           s = size, f = factor;
	int i = 0, x1 = x;

	while ((c = string[i]) != '\0')
	{
		if (c != ' ')
		{
			drawChar(c, x1, y, s, f);
			x1 += (s*f + s * f / 4);
		}
		else
		{
			x1 += ((s*f + s * f / 4) / 2);
		}
		i++;
	}
}

void graphics::swap(short * a, short * b)
{
	short c;
	c = *b;
	*b = *a;
	*a = c;
}

void graphics::drawLine(short x1, short y1, short x2, short y2)
{
	if (y1 == y2)
	{
		if(x1 > x2)
			drawHLine(x2, y1, abs(x2 - x1));
		else
			drawHLine(x1, y1, abs(x2 - x1));
	}
	else if (x1 == x2)
	{
		if(y1 > y2)
			drawVLine(x1, y2, abs(y2 - y1));
		else
			drawVLine(x1, y1, abs(y2 - y1));
	}
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep = x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep = y2 > y1 ? 1 : -1;
		int				col = x1, row = y1;

		if (dx < dy)
		{
			int t = -(dy >> 1);
			while (true)
			{
				drawPixel(col,row);				
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t -= dy;
				}
			}
		}
		else
		{
			int t = -(dx >> 1);
			while (true)
			{
				drawPixel(col, row);
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t -= dx;
				}
			}
		}
	}
}

void graphics::drawHLine(short x, short y, int l)
{
	for (size_t i = 0; i < l; i++)
	{
		drawPixel(x + i, y);
	}
}

void graphics::drawVLine(short x, short y, int l)
{
	for (size_t i = 0; i < l; i++)
	{
		drawPixel(x, y + i);
	}
}

void graphics::setColor(char r, char g, char b)
{
	fgColor[2] = b;
	fgColor[1] = g;
	fgColor[0] = r;
	_fgColor = ((int)b << 16) | ((int)g << 8) | r;
}

void graphics::setBackColor(char r, char g, char b)
{
	bgColor[2] = b;
	bgColor[1] = g;
	bgColor[0] = r;
	_bgColor = ((int)b << 16) | ((int)g << 8) | r;
}

void ILI9488SPI_BASE::_init(unsigned char sck, unsigned char miso, unsigned char mosi, unsigned char ss, unsigned int freq, ili9488_mode mode)
{
	SPI.begin((char)sck, (char)miso, (char)mosi, (char)ss);
	SPI.setBitOrder(MSBFIRST);
	SPI.setFrequency(freq);
#ifdef ESP32
	ESP32_SPI_DIS_MOSI_MISO_FULL_DUPLEX(ESP32_VSPI);
#endif
	
	ILI9488SPI_PRE_INIT();
	ILI9488SPI_SELECT_DATA_COMM(ILI9488SPI_DATA);
	ILI9488SPI_ASSERT_CS();

	ILI9488SPI_LCD_WRITE_COM(0xE0);
	ILI9488SPI_LCD_WRITE_DATA8(0x00);
	ILI9488SPI_LCD_WRITE_DATA8(0x03);
	ILI9488SPI_LCD_WRITE_DATA8(0x09);
	ILI9488SPI_LCD_WRITE_DATA8(0x08);
	ILI9488SPI_LCD_WRITE_DATA8(0x16);
	ILI9488SPI_LCD_WRITE_DATA8(0x0A);
	ILI9488SPI_LCD_WRITE_DATA8(0x3F);
	ILI9488SPI_LCD_WRITE_DATA8(0x78);
	ILI9488SPI_LCD_WRITE_DATA8(0x4C);
	ILI9488SPI_LCD_WRITE_DATA8(0x09);
	ILI9488SPI_LCD_WRITE_DATA8(0x0A);
	ILI9488SPI_LCD_WRITE_DATA8(0x08);
	ILI9488SPI_LCD_WRITE_DATA8(0x16);
	ILI9488SPI_LCD_WRITE_DATA8(0x1A);
	ILI9488SPI_LCD_WRITE_DATA8(0x0F);

	ILI9488SPI_LCD_WRITE_COM(0XE1);
	ILI9488SPI_LCD_WRITE_DATA8(0x00);
	ILI9488SPI_LCD_WRITE_DATA8(0x16);
	ILI9488SPI_LCD_WRITE_DATA8(0x19);
	ILI9488SPI_LCD_WRITE_DATA8(0x03);
	ILI9488SPI_LCD_WRITE_DATA8(0x0F);
	ILI9488SPI_LCD_WRITE_DATA8(0x05);
	ILI9488SPI_LCD_WRITE_DATA8(0x32);
	ILI9488SPI_LCD_WRITE_DATA8(0x45);
	ILI9488SPI_LCD_WRITE_DATA8(0x46);
	ILI9488SPI_LCD_WRITE_DATA8(0x04);
	ILI9488SPI_LCD_WRITE_DATA8(0x0E);
	ILI9488SPI_LCD_WRITE_DATA8(0x0D);
	ILI9488SPI_LCD_WRITE_DATA8(0x35);
	ILI9488SPI_LCD_WRITE_DATA8(0x37);
	ILI9488SPI_LCD_WRITE_DATA8(0x0F);

	ILI9488SPI_LCD_WRITE_COM(0XC0);     //Power Control 1
	ILI9488SPI_LCD_WRITE_DATA8(0x17);    //Vreg1out
	ILI9488SPI_LCD_WRITE_DATA8(0x15);    //Verg2out

	ILI9488SPI_LCD_WRITE_COM(0xC1);     //Power Control 2
	ILI9488SPI_LCD_WRITE_DATA8(0x41);    //VGH,VGL

	ILI9488SPI_LCD_WRITE_COM(0xC5);     //Power Control 3
	ILI9488SPI_LCD_WRITE_DATA8(0x00);
	ILI9488SPI_LCD_WRITE_DATA8(0x12);    //Vcom
	ILI9488SPI_LCD_WRITE_DATA8(0x80);

	ILI9488SPI_LCD_WRITE_COM(0x36);      //Memory Access
	//ILI9488SPI_LCD_WRITE_DATA8(0x4A);
							 ////76543210
	ILI9488SPI_LCD_WRITE_DATA8(0b11101010);

	ILI9488SPI_LCD_WRITE_COM(0x3A);      // Interface Pixel Format

	if (mode == rgb666)
	{	
		ILI9488SPI_LCD_WRITE_DATA8(0x66); 	  // 0x66 = 18 bits (3 bytes per pixel), 0x55 = 16 bits is not supported in SPI mode! 
	}
	else
	{
		ILI9488SPI_LCD_WRITE_DATA8(0x11); 	  // 0x11 = 3 bits (3 bits per pixel) 
	}	
	
	ILI9488SPI_LCD_WRITE_COM(0XB0);      // Interface Mode Control
	ILI9488SPI_LCD_WRITE_DATA8(0x80);     //SDO NOT USE

	ILI9488SPI_LCD_WRITE_COM(0xB1);      //Frame rate
	ILI9488SPI_LCD_WRITE_DATA8(0xA0);    //60Hz

	ILI9488SPI_LCD_WRITE_COM(0xB4);      //Display Inversion Control
	ILI9488SPI_LCD_WRITE_DATA8(0x02);    //2-dot

	ILI9488SPI_LCD_WRITE_COM(0XB6);     //Display Function Control  RGB/MCU Interface Control

	ILI9488SPI_LCD_WRITE_DATA8(0x02);    //MCU
	ILI9488SPI_LCD_WRITE_DATA8(0x02);    //Source,Gate scan direction

	ILI9488SPI_LCD_WRITE_COM(0XE9);     // Set Image Function
	ILI9488SPI_LCD_WRITE_DATA8(0x00);    // Disable 24 bit data

	ILI9488SPI_LCD_WRITE_COM(0xF7);      // Adjust Control
	ILI9488SPI_LCD_WRITE_DATA8(0xA9);
	ILI9488SPI_LCD_WRITE_DATA8(0x51);
	ILI9488SPI_LCD_WRITE_DATA8(0x2C);
	ILI9488SPI_LCD_WRITE_DATA8(0x82);    // D7 stream, loose

	ILI9488SPI_LCD_WRITE_COM(0x11);    //Exit Sleep
	delay(120);
	ILI9488SPI_LCD_WRITE_COM(0x29);    //Display on

}

void ILI9488SPI_BASE::setXY(short x1, short y1, short x2, short y2)
{
	int ex = x2 | (x1 << 16);
	int ey = y2 | (y1 << 16);

	ILI9488SPI_LCD_WRITE_COM(0x2a);
	SPI.writeUINT(ex);
	ILI9488SPI_LCD_WRITE_COM(0x2b);
	SPI.writeUINT(ey);
	ILI9488SPI_LCD_WRITE_COM(0x2c);
}

void ILI9488SPI_BASE::setXY(short x, short y)
{
	int ex = x | (x << 16);
	int ey = y | (y << 16);
	ILI9488SPI_LCD_WRITE_COM(0x2a);
	SPI.writeUINT(ex); // Need to optimize !
	ILI9488SPI_LCD_WRITE_COM(0x2b);
	SPI.writeUINT(ey);
	ILI9488SPI_LCD_WRITE_COM(0x2c);
}

void ILI9488SPI_264KC::init(unsigned char sck, unsigned char miso, unsigned char mosi, unsigned char ss, unsigned int freq)
{
	ILI9488SPI_BASE::_init(sck, miso, mosi, ss, freq, rgb666);
}

inline void ILI9488SPI_264KC::drawPixel(short x, short y)
{
	setXY(x, y);
	ILI9488SPI_LCD_WRITE_DATA(_fgColor);
}

void ILI9488SPI_264KC::fillScr(char r, char g, char b)
{
	unsigned int color = ((unsigned int)b << 16) | ((unsigned int)g << 8) | (unsigned int)r;

	setXY(0, 0, maxX, maxY);
	SPI.writeRGB(color, maxX * maxY);
}

void ILI9488SPI_264KC::drawHLine(short x, short y, int l)
{
	if (l < 0)
	{
		l = -l;
		x -= l;
	}
	setXY(x, y, x + l, y);
	SPI.writePattern(fgColor, 3, l);
}

void ILI9488SPI_264KC::drawVLine(short x, short y, int l)
{
	if (l < 0)
	{
		l = -l;
		y -= l;
	}
	setXY(x, y, x, y + l);
	SPI.writePattern(fgColor, 3, l);
}

void ILI9488SPI_264KC::drawRect(short x1, short y1, short x2, short y2, bool fill)
{
	if (x1 > x2)
	{
		swap(&x1, &x2);
	}
	if (y1 > y2)
	{
		swap(&y1, &y2);
	}

	if (fill)
	{
		setXY(x1, y1, x2, y2);
		SPI.writeRGB(_fgColor, (x2-x1 + 1)*(y2-y1 + 1));
	}
	else
	{
		drawHLine(x1, y1, x2 - x1);
		drawHLine(x1, y2, x2 - x1);
		drawVLine(x1, y1, y2 - y1);
		drawVLine(x2, y1, y2 - y1);
	}
}

#define DATA_START_OFFSET 2

void ILI9488SPI_264KC::drawCompressed24bitBitmap(short x, short y, const unsigned int * dataArray)
{
	unsigned int	hight, width;
	unsigned int	buffer;
	int				index = 0;

	width = dataArray[index];
	index++;

	buffer = dataArray[index];
	hight = buffer;
	index++;

	unsigned int dataArraySize = hight * width, i, j, counter = 0;
	unsigned char copies;

	setXY(x, y, x + width - 1, y + hight - 1);
	for (i = DATA_START_OFFSET; counter < dataArraySize; i++)
	{
		buffer = dataArray[index];
		index++;
		copies = (buffer >> 24);
		SPI.writeRGB(buffer, copies);
		counter += copies;
	}
}

void ILI9488SPI_264KC::drawCompressedGrayScaleBitmap(short x, short y, const unsigned short * dataArray, bool invert)
{
	unsigned int hight = dataArray[0], width = dataArray[1];
	unsigned int dataArraySize = hight * width, i, j, counter = 0;
	unsigned char copies;

	setXY(x, y, x + hight - 1, y + width - 1);
	for (i = DATA_START_OFFSET; counter < dataArraySize; i++)
	{
		copies = (dataArray[i] >> 8);
		if (invert)
		{
			SPI.writeGrayScale(255 - dataArray[i], copies);
		}
		else
		{
			SPI.writeGrayScale(dataArray[i], copies);
		}
		counter += copies;
	}
}

extern unsigned short * hebCharSet[];
extern unsigned short * numberCharSet[];
extern unsigned short apostrophesCF[], periodCF[], minusCF[], tagCF[], colonCF[];
#define SPACE_SIZE		10
#define HEBREW_CHAR_SET	215

void ILI9488SPI_264KC::drawHebStringUTF8(short x, short y, const char * str, bool swapString)
{
	// Heb Strings are swapped //
	char *	tempStr;
	int *	tempStrNum;
	int i, j, counter = 0, index;

	if (swapString)
	{
		int len = strlen(str);
		tempStr = new char[len + 1];
		tempStrNum = new int[len + 1];

		for (i = 0; i < len; i++)
		{
			tempStrNum[i] = 0;
		}
		for (i = 0; i < len; i++)
		{
			if (str[i] >= '0' && str[i] <= '9')
			{
				counter++;
				tempStrNum[i] = counter;
			}
			else
				counter = 0;
		}
		index = -1;
		for (i = len - 1, j = 0; i >= 0; i--, j++)
		{
			if (str[i] >= '0' && str[i] <= '9')
			{
				tempStr[index + tempStrNum[i]] = str[i];
			}
			else
			{
				index = j;
			}
		}
		j = len - 1;
		for (i = 0; i < len; )
		{
			if (str[i] == HEBREW_CHAR_SET)
			{
				tempStr[j - 1] = str[i];
				tempStr[j] = str[i + 1];
				i += 2;
				j -= 2;
			}
			else
			{
				if (!(str[i] >= '0' && str[i] <= '9'))
					tempStr[j] = str[i];
				i += 1;
				j -= 1;
			}
		}
		tempStr[len] = '\0';
	}
	else
	{
		tempStr = (char *)str;
	}

	/////////////////////////////
	int xIndex = x, strLen = strlen(tempStr), tempX, tempY;
	unsigned short * currentChar, tempChar;
	for (i = 0; i < strLen;) // UTF8 -> each char is 2 bytes 
	{
		if (tempStr[i] == HEBREW_CHAR_SET) // UTF8 char
		{
			i++;
			if (tempStr[i] >= 144 && tempStr[i] <= 170)
			{
				// Heb Char
				currentChar = hebCharSet[tempStr[i] - 144];
				tempX = xIndex;// -currentChar[0];
				if (tempStr[i] == 156 /* Lamed*/)
					tempY = y - 9;
				else
					tempY = y;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			i++; // Un-supported UTF8 char
		}
		else
		{
			if (tempStr[i] >= 48 && tempStr[i] <= 57) // Digit
			{
				currentChar = numberCharSet[tempStr[i] - 48];
				tempX = xIndex;// -currentChar[0];
				tempY = y;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			if (tempStr[i] == 32 /* space */)
				xIndex += SPACE_SIZE;
			if (tempStr[i] == 34)
			{
				// " 
				currentChar = apostrophesCF;
				tempX = xIndex;// -currentChar[0];
				tempY = y - 3;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			if (tempStr[i] == 45)
			{
				// - 
				currentChar = minusCF;
				tempX = xIndex;// -currentChar[0];
				tempY = y + 11;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			if (tempStr[i] == 46)
			{
				// . 
				currentChar = periodCF;
				tempX = xIndex;// -currentChar[0];
				tempY = y + 19;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			if (tempStr[i] == 39)
			{
				// ' 
				currentChar = tagCF;
				tempX = xIndex;// -currentChar[0];
				tempY = y - 3;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			if (tempStr[i] == 58)
			{
				// : 
				currentChar = colonCF;
				tempX = xIndex;// -currentChar[0];
				tempY = y + 3;
				drawCompressedGrayScaleBitmap(tempX, tempY, currentChar);
				xIndex += (currentChar[0] + 1);
			}
			i++;
		}
	}
	if (swapString)
	{
		delete[] tempStr;
		delete[] tempStrNum;
	}
}

int	 ILI9488SPI_264KC::getStringWidth(const char * str)
{
	int size = 0, strLen = strlen(str),i;
	unsigned short * currentChar;

	for (i = 0; i < strLen;) // UTF8 -> each char is 2 bytes 
	{
		if (str[i] == HEBREW_CHAR_SET) // UTF8 char
		{
			i++;
			if (str[i] >= 144 && str[i] <= 170)
			{
				currentChar = hebCharSet[str[i] - 144];
				size += (currentChar[0] + 1);
			}
			i++; // Un-supported UTF8 char
		}
		else
		{
			if (str[i] >= 48 && str[i] <= 57) // Digit
			{
				currentChar = numberCharSet[str[i] - 48];
				size += (currentChar[0] + 1);
			}
			if (str[i] == 32 /* space */)
				size += SPACE_SIZE;
			if (str[i] == 34)
			{
				// " 
				currentChar = apostrophesCF;
				size += (currentChar[0] + 1);
			}
			if (str[i] == 45)
			{
				// - 
				currentChar = minusCF;
				size += (currentChar[0] + 1);
			}
			if (str[i] == 46)
			{
				// . 
				currentChar = periodCF;
				size += (currentChar[0] + 1);
			}
			if (str[i] == 39)
			{
				// ' 
				currentChar = tagCF;
				size += (currentChar[0] + 1);
			}
			if (str[i] == 58)
			{
				// : 
				currentChar = colonCF;
				size += (currentChar[0] + 1);
			}
			i++;
		}
	}
	return size;
}

void ILI9488SPI_264KC::testFunc()
{
	fillScr(255, 255, 255);
	setColor(255, 0, 0);
	drawRect(49, 49, 101, 101);
	setColor(0, 0, 0);
	setXY(50, 50, 100, 100);
	SPI.writeRGB(_fgColor);
	SPI.writeRGB(_fgColor);
	SPI.writeRGB(_fgColor);
}

#define ILI9488_8C_FRAME_BUFFER_SIZE	(480*320/2)

bool ILI9488SPI_8C::init(unsigned char sck, unsigned char miso, unsigned char mosi, unsigned char ss, unsigned int freq, ili9488_8C_mode mode)
{
	ILI9488SPI_BASE::_init(sck, miso, mosi, ss, freq, rgb111);
	
	currentFrameBufferIndex = 0;
	workingMode = mode;
	FGbitOn = false;

	switch (mode)
	{
	case singleFrameBuffer:
	case directMode:
		frameBuffers[0] = (unsigned char *)malloc(ILI9488_8C_FRAME_BUFFER_SIZE);
		if (frameBuffers[0] == NULL)
		{
			return false;
		}
		break;
	case dualFrameBuffers:
		frameBuffers[0] = (unsigned char *)malloc(ILI9488_8C_FRAME_BUFFER_SIZE);
		if (frameBuffers[0] == NULL)
		{
			return false;
		}
		frameBuffers[1] = (unsigned char *)malloc(ILI9488_8C_FRAME_BUFFER_SIZE);
		if (frameBuffers[1] == NULL)
		{
			free(frameBuffers[0]);
			return false;
		}
		break;
	default:
		return false;
	}

	return true;
}

void ILI9488SPI_8C::setColor(char r, char g, char b)
{
	fgColorL = ((r == 1) << 2) | ((g == 1) << 1) | (b == 1);
	fgColorH = fgColorL << 3;
	fgColorHL = fgColorL | fgColorH;
	if (FGbitOn)
	{
		fgColorHL = fgColorHL | 0xc0;
	}

}

void ILI9488SPI_8C::setBackColor(char r, char g, char b)
{
	bgColorL = ((r == 1) << 2) | ((g == 1) << 1) | (b == 1);
	bgColorH = bgColorL << 3;
}

void ILI9488SPI_8C::fillScr(char r, char g, char b)
{
	unsigned char tempColorL = ((r == 1) << 2) | ((g == 1) << 1) | (b == 1);
	unsigned char tempColor = (tempColorL << 3) | tempColorL;
	if (FGbitOn)
	{
		tempColorL = tempColorL | 0xc0;
	}
	unsigned int tempC = (tempColor << 24) | (tempColor << 16) | (tempColor << 8 | tempColor);
	unsigned int *tempBuffer = (unsigned int *)frameBuffers[currentFrameBufferIndex];
	for (size_t i = 0; i < (ILI9488_8C_FRAME_BUFFER_SIZE / 4); i++)
	{
		tempBuffer[i] = tempC;
	}
}

void ILI9488SPI_8C::flushFrameBuffer()
{
	setXY(0, 0, maxX, maxY);
	SPI.writeBuffer((unsigned int *)frameBuffers[currentFrameBufferIndex], ILI9488_8C_FRAME_BUFFER_SIZE / 4);
}

inline void ILI9488SPI_8C::drawPixel(short x, short y)
{
	unsigned int index = (y*maxX + x) >> 1;

	if (FGbitOn)
	{
		if (x & 0x1 == 1) // x%2 == 1
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x38 | fgColorL | 0x40;
		}
		else
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x07 | fgColorH | 0x80;
		}
	}
	else
	{
		if (x & 0x1 == 1) // x%2 == 1
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x38 | fgColorL;
		}
		else
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x07 | fgColorH;
		}
	}
}

inline void ILI9488SPI_8C::drawPixel(short x, short y, unsigned char color)
{
	if (x < 0 || x > 479 || y < 0 || y > 319)
	{
		return;
	}
	unsigned int index = (y*maxX + x) >> 1;

	if (FGbitOn)
	{
		if (x & 0x1 == 1) // x%2 == 1
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x38 | color | 0x40;
		}
		else
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x07 | (color << 3) | 0x80;
		}
	}
	else
	{
		if (x & 0x1 == 1) // x%2 == 1
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x38 | color;
		}
		else
		{
			frameBuffers[currentFrameBufferIndex][index] = frameBuffers[currentFrameBufferIndex][index] & 0x07 | (color << 3);
		}
	}

}

void ILI9488SPI_8C::drawHLine(short x, short y, int l)
{
	size_t i = 0,lo = l;
	unsigned int index;
	
	if (x & 0x1 == 1) // x%2 == 1
	{
		drawPixel(x, y);
		i++;
		lo -= 1;
	}
	while (i <= l) // We are alighned to 2
	{
		if(lo >= 2)
		{
			index = (y*maxX + x + i) >> 1;
			if (index >= 480 * 320 / 2)
				return;
			frameBuffers[currentFrameBufferIndex][index] = fgColorHL;
			i += 2;
			lo -= 2;
		}
		else
		{
			drawPixel(x + i, y);
			i++;
		}
	}
}

inline bool ILI9488SPI_8C::isFGbitSet(short x, short y)
{
	unsigned int index = (y*maxX + x) >> 1;
	if (x & 0x1 == 1) // x%2 == 1
	{
		return ((frameBuffers[currentFrameBufferIndex][index] & 0x40) == 0x40);
	}
	else
	{
		return ((frameBuffers[currentFrameBufferIndex][index] & 0x80) == 0x80);
	}
}
/* Structure of bitmap: */
/* array of unsigned chars*/
/* wh,wl,hh,hl,data .....*/
/* data: */
/* Each byte represents two pixels: SRGBSRGB - S = Skip, if set to 1 pixel is not drawn */
void ILI9488SPI_8C::drawBitmap(short x, short y, const unsigned char * dataArray,bool useSkipBit, ili9488_8C_flipOption flipOpt)
{
	unsigned char pixelData;
	unsigned short width, hight;
	unsigned int index = 4,size;
	width = (dataArray[0] << 8) | dataArray[1];
	hight = (dataArray[2] << 8) | dataArray[3];
	size = (width / 2 + (width & 01 == 1)) * hight;

	if (flipOpt == flipY || flipOpt == flipXY)
	{
		//for (int iy = y; iy < (y + hight); iy++)
		for (int iy = (y + hight - 1); iy >= y ; iy--)
		{
			if (flipOpt == flipX || flipOpt == flipXY)
			{
				//for (int ix = x; ix < (x + width); ix += 2)
				for (int ix = (x + width - 1); ix >= x; ix -= 2)
				{
					pixelData = dataArray[index++];
					if (useSkipBit)
					{
						if ((pixelData & 0x80) != 0x80) // First pixel
						{
							if (!isFGbitSet(ix + 1, iy))
								drawPixel(ix + 1, iy, (pixelData >> 4) & 0x7);
						}
						if (((pixelData & 0x08) != 0x08) && ((ix + 1) < (x + width))) // Second Pixel
						{
							if (!isFGbitSet(ix, iy))
								drawPixel(ix, iy, pixelData & 0x07);
						}
					}
					else
					{
						if (!isFGbitSet(ix + 1, iy)) // First pixel
							drawPixel(ix + 1, iy, (pixelData >> 4) & 0x7);
						if ((ix + 1) < (x + width)) // Second Pixel
						{
							if (!isFGbitSet(ix, iy))
								drawPixel(ix, iy, pixelData & 0x07);
						}
					}
				}
			}
			else
			{
				for (int ix = x; ix < (x + width); ix += 2)
				{
					pixelData = dataArray[index++];
					if (useSkipBit)
					{
						if ((pixelData & 0x80) != 0x80) // First pixel
						{
							if (!isFGbitSet(ix, iy))
								drawPixel(ix, iy, (pixelData >> 4) & 0x7);
						}
						if (((pixelData & 0x08) != 0x08) && ((ix + 1) < (x + width))) // Second Pixel
						{
							if (!isFGbitSet(ix + 1, iy))
								drawPixel(ix + 1, iy, pixelData & 0x07);
						}
					}
					else
					{
						if (!isFGbitSet(ix, iy)) // First pixel
							drawPixel(ix, iy, (pixelData >> 4) & 0x7);
						if ((ix + 1) < (x + width)) // Second Pixel
						{
							if (!isFGbitSet(ix + 1, iy))
								drawPixel(ix + 1, iy, pixelData & 0x07);
						}
					}
				}
			}
		}
	}
	else
	{
		for (int iy = y; iy < (y + hight); iy++)
		{
			if (flipOpt == flipX || flipOpt == flipXY)
			{
				//for (int ix = x; ix < (x + width); ix += 2)
				for (int ix = (x + width - 1); ix >= x; ix -= 2)
				{
					pixelData = dataArray[index++];
					if (useSkipBit)
					{
						if ((pixelData & 0x80) != 0x80) // First pixel
						{
							if (!isFGbitSet(ix + 1, iy))
								drawPixel(ix + 1, iy, (pixelData >> 4) & 0x7);
						}
						if (((pixelData & 0x08) != 0x08) && ((ix + 1) < (x + width))) // Second Pixel
						{
							if (!isFGbitSet(ix, iy))
								drawPixel(ix, iy, pixelData & 0x07);
						}
					}
					else
					{
						if (!isFGbitSet(ix + 1, iy)) // First pixel
							drawPixel(ix + 1, iy, (pixelData >> 4) & 0x7);
						if ((ix + 1) < (x + width)) // Second Pixel
						{
							if (!isFGbitSet(ix, iy))
								drawPixel(ix, iy, pixelData & 0x07);
						}
					}
				}
			}
			else
			{
				for (int ix = x; ix < (x + width); ix += 2)
				{
					pixelData = dataArray[index++];
					if (useSkipBit)
					{
						if ((pixelData & 0x80) != 0x80) // First pixel
						{
							if (!isFGbitSet(ix, iy))
								drawPixel(ix, iy, (pixelData >> 4) & 0x7);
						}
						if (((pixelData & 0x08) != 0x08) && ((ix + 1) < (x + width))) // Second Pixel
						{
							if (!isFGbitSet(ix + 1, iy))
								drawPixel(ix + 1, iy, pixelData & 0x07);
						}
					}
					else
					{
						if (!isFGbitSet(ix, iy)) // First pixel
							drawPixel(ix, iy, (pixelData >> 4) & 0x7);
						if ((ix + 1) < (x + width)) // Second Pixel
						{
							if (!isFGbitSet(ix + 1, iy))
								drawPixel(ix + 1, iy, pixelData & 0x07);
						}
					}
				}
			}
		}
	}
}
