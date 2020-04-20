/*
 General graphics lib to be inherited
*/

#ifndef GRAPHICS_h
#define GRAPHICS_h

class graphics
{
	protected:
		unsigned char	fgColor[3], bgColor[3];
		int				_fgColor, _bgColor;
		short			maxX, maxY;

		void	swap(short * a, short * b);

	public:
		graphics(short maxX, short maxY);
		~graphics() {};
		
		virtual void drawPixel(short x, short y) = 0;
		virtual void fillScr(char r, char g, char b) = 0;
		virtual void drawLine(short x1, short y1, short x2, short y2);
		virtual void drawHLine(short x, short y, int l);
		virtual void drawVLine(short x, short y, int l);
		
		virtual void setColor(char r, char g, char b);
		virtual void setBackColor(char r, char g, char b);

		virtual void drawRect(short x1, short y1, short x2, short y2,bool fill = false);
		void	drawRoundRect(short x1, short y1, short x2, short y2,bool fill = false);
		void	drawCircle(short x, short y, int radius, bool fill = false);
		void	drawChar(unsigned char c, short x, short y, float size, float factor = 1.0);
		void	drawString(char * string, short x, short y, float size, float factor = 1.0);
		
		int		getXSize() { return maxX; }
		int		getYSize() { return maxY; }
};


#define DRAW_PIXEL(x,y)\
	drawPixel(x,y)
/*
#define DRAW_PIXEL(x,y)\
		setXY(x, y);\
		ILI9488SPI_LCD_WRITE_DATA(_fgColor)
*/

// ILI9488 SPI Basic Class
#include "PCF8574.h"
extern PCF8574 pcf8574;

#define ILI9488SPI_COMMAND	0
#define ILI9488SPI_DATA		1
/* 0 for maker's platform , 32 for Smart Home board*/
#define ILI9488SPI_DATA_COMMAND_PIN	0 

#define ILI9488SPI_SELECT_DATA_COMM(X) digitalWrite(ILI9488SPI_DATA_COMMAND_PIN, X);

/* LCD Chip Select - Alwayes inserted */
/* LCD and Touch pannel reset */
#define ILI9488SPI_PRE_INIT() {\
			pcf8574.pinMode(P0, OUTPUT);\ 
			pcf8574.pinMode(P1, OUTPUT);\
			pcf8574.begin();\
			pcf8574.digitalWrite(P0, HIGH);\
			pcf8574.digitalWrite(P1, HIGH);\
			pinMode(ILI9488SPI_DATA_COMMAND_PIN, OUTPUT);\
			}

#define	ILI9488SPI_RESET()\
			pcf8574.digitalWrite(P1, HIGH);\
			delay(250);\
			pcf8574.digitalWrite(P1, LOW);\
			delay(250);\
			pcf8574.digitalWrite(P1, HIGH);

#define ILI9488SPI_ASSERT_CS()\
			pcf8574.digitalWrite(P0, LOW);

#define ILI9488SPI_DEASSERT_CS()\
			pcf8574.digitalWrite(P0, HIGH);

#define ILI9488SPI_LCD_WRITE_DATA8(VAL)\
			SPI.writeBYTE((char)VAL);

#define	ILI9488SPI_LCD_WRITE_DATA(X)\
			SPI.writeRGB(X);

#if 1
#define ILI9488SPI_LCD_WRITE_COM(VAL)\
			ILI9488SPI_SELECT_DATA_COMM(ILI9488SPI_COMMAND);\
			ILI9488SPI_LCD_WRITE_DATA8((char)VAL);\
			ILI9488SPI_SELECT_DATA_COMM(ILI9488SPI_DATA);
#else
#define GPIO_W1T_SET(data) (*((volatile unsigned int *)(((unsigned int)(0x3FF44008)))) = data)
#define GPIO_W1T_CLEAR(data) (*((volatile unsigned int *)(((unsigned int)(0x3FF4400c)))) = data)
#define ILI9488SPI_LCD_WRITE_COM(VAL)\
			GPIO_W1T_CLEAR(0x1);\
			ILI9488SPI_LCD_WRITE_DATA8((char)VAL);\
			GPIO_W1T_SET(0x1);
#endif


#define ILI9488_8COLORS		0
#define ILI9488_264KCOLORS	1
typedef enum { rgb111 = ILI9488_8COLORS, rgb666 = ILI9488_264KCOLORS }
			ili9488_mode;


class ILI9488SPI_BASE : public graphics
{
protected:
	void _init(unsigned char sck = 18, unsigned char miso = 19, unsigned char mosi = 23, 
			   unsigned char ss = 5, unsigned int freq = 40000000L, ili9488_mode mode = rgb666);

public:
	ILI9488SPI_BASE(short maxX, short maxY) : graphics(maxX, maxY) {};
	~ILI9488SPI_BASE() {};

	void setXY(short x1, short y1, short x2, short y2);
	void setXY(short x, short y);
};

class ILI9488SPI_264KC : public ILI9488SPI_BASE
{
public:
	ILI9488SPI_264KC(short maxX, short maxY) : ILI9488SPI_BASE(maxX, maxY) {};
	~ILI9488SPI_264KC() {};
	void init(unsigned char sck = 18, unsigned char miso = 19, unsigned char mosi = 23, unsigned char ss = 5, unsigned int freq = 40000000L);
	inline void drawPixel(short x, short y);
	void fillScr(char r, char g, char b);
	void drawHLine(short x, short y, int l);
	void drawVLine(short x, short y, int l);
	void drawRect(short x1, short y1, short x2, short y2, bool fill = false);
	void drawCompressed24bitBitmap(short x, short y, const unsigned int * dataArray);
	void drawCompressedGrayScaleBitmap(short x, short y, const unsigned short * dataArray, bool invert = false);
	void drawHebStringUTF8(short x, short y, const char * str, bool swapString, bool invert = false);
	int	 getStringWidth(const char * str);
	void testFunc();
};

typedef enum { singleFrameBuffer = 0, dualFrameBuffers = 1, directMode = 3 }
ili9488_8C_mode;

typedef enum { noflip = 0, flipX = 1, flipY = 2 , flipXY = 3}
ili9488_8C_flipOption;

class ILI9488SPI_8C : public ILI9488SPI_BASE
{
private:
	unsigned char *	frameBuffers[2];
	unsigned int	currentFrameBufferIndex;
	ili9488_8C_mode	workingMode;
	unsigned char fgColorH, fgColorL, bgColorH, bgColorL,fgColorHL;
	bool FGbitOn;

public:
	ILI9488SPI_8C(short maxX, short maxY) : ILI9488SPI_BASE(maxX, maxY) {};
	~ILI9488SPI_8C() {};
	bool init(unsigned char sck = 18, unsigned char miso = 19, unsigned char mosi = 23, unsigned char ss = 5, unsigned int freq = 40000000L, ili9488_8C_mode mode = singleFrameBuffer);
	void setColor(char r, char g, char b);
	void setBackColor(char r, char g, char b);
	void setFGbitOn() { FGbitOn = true; }; // All Pixels being drawn will be marked as foreground 
	void setFGbitOff() { FGbitOn = false; };

	inline void drawPixel(short x, short y);
	inline void drawPixel(short x, short y, unsigned char color);
	void draw2Pixels(short x, short y, unsigned char color);
	inline bool isFGbitSet(short x, short y);
	void fillScr(char r, char g, char b);
	void drawHLine(short x, short y, int l);
	void drawBitmap(short x, short y, const unsigned char * dataArray, bool useSkipBit, ili9488_8C_flipOption flipOpt = noflip);
	void flushFrameBuffer();
	//int	 swapFrameBuffer();
	//void testFunc();
};


#endif