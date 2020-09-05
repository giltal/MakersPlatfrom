/*
 General graphics lib to be inherited
*/

#ifndef GRAPHICS_h
#define GRAPHICS_h

#include "Fonts/fonts.h"

class graphics
{
	protected:
		unsigned char	fgColor[3], bgColor[3];
		int				_fgColor, _bgColor;
		unsigned short	fg565, bg565;
		short			maxX, maxY;
		customFont *	currentFonts;
		fontType		fontTypeLoaded;

		void	swap(short * a, short * b);

	public:
		graphics(short maxX, short maxY);
		~graphics() {};
		
		virtual void drawPixel(short x, short y) = 0;
		virtual void fillScr(unsigned char r, unsigned char g,unsigned char b) = 0;
		virtual void drawLine(short x1, short y1, short x2, short y2);
		virtual void drawHLine(short x, short y, int l);
		virtual void drawVLine(short x, short y, int l);
		
		virtual void setColor(unsigned char r, unsigned char g, unsigned char b);
		virtual void setBackColor(unsigned char r, unsigned char g,unsigned char b);
		virtual unsigned short rgbTo565(unsigned char r, unsigned char g, unsigned char b);
		
		virtual int	getXSize() { return maxX; }
		virtual int	getYSize() { return maxY; }

		virtual void drawRect(short x1, short y1, short x2, short y2,bool fill = false);
		void	drawRoundRect(short x1, short y1, short x2, short y2,bool fill = false);
		void	drawCircle(short x, short y, int radius, bool fill = false);
		void	drawTriangle(short x0, short y0, short x1, short y1, short x2, short y2, bool fill = false);
		void	loadFonts(fontType fontsToLoad);
		short	getFontHieght();
		short	getPrintWidth(char * string);
		void	print(char * string, short x, short y,bool center = false);
};

#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST77XXSPI_COMMAND			0
#define ST77XXSPI_DATA				1
#define NATIVE_VSPI
#ifdef NATIVE_VSPI
#define ST77XXSPI_DATA_COMMAND_PIN	4
#define ST77XXSPI_CS_PIN			5
#define ST77XXSPI_RESET_PIN			0
#define ST77XXSPI_BACK_LIGHT_PIN	-1
#define ST77XXSPI_MOSI_PIN			23
#define ST77XXSPI_CLK_PIN			18
#else
#define ST77XXSPI_DATA_COMMAND_PIN	16
#define ST77XXSPI_CS_PIN			5
#define ST77XXSPI_RESET_PIN			23
#define ST77XXSPI_BACK_LIGHT_PIN	4
#define ST77XXSPI_MOSI_PIN			19
#define ST77XXSPI_CLK_PIN			18
#endif
#define ST7789_X_OFFSET				52
#define ST7789_Y_OFFSET				40

#define ST77XXSPI_SELECT_DATA_COMM(X) digitalWrite(ST77XXSPI_DATA_COMMAND_PIN, X);

#define ST77XXSPI_ASSERT_CS()\
			digitalWrite(ST77XXSPI_CS_PIN, LOW);

#define ST77XXSPI_DEASSERT_CS()\
			digitalWrite(ST77XXSPI_CS_PIN, HIGH);

#define ST77XXSPI_LCD_WRITE_DATA8(VAL)\
			SPI.writeBYTE((char)VAL);

#define	ST77XXSPI_LCD_WRITE_DATA(X)\
			SPI.writeShort(X);

#define ST77XXSPI_LCD_WRITE_COM(VAL)\
			ST77XXSPI_SELECT_DATA_COMM(ST77XXSPI_COMMAND);\
			ST77XXSPI_LCD_WRITE_DATA8((char)VAL);\
			ST77XXSPI_SELECT_DATA_COMM(ST77XXSPI_DATA);

enum ST77XXres { _135x240, _240x135, _320x240 };

class ST77XX : public graphics
{
private:
	unsigned short Xoffset, Yoffset;
	ST77XXres _res;
public:
	ST77XX(ST77XXres res) : graphics(0,0)
	{
		_res = res;
		if (res == _135x240)
		{
			maxX = 135;
			maxY = 240;
			Xoffset = ST7789_X_OFFSET;
			Yoffset = ST7789_Y_OFFSET;
		}
		if (res == _240x135)
		{
			maxY = 135;
			maxX = 240;
			Xoffset = ST7789_Y_OFFSET;
			Yoffset = ST7789_X_OFFSET;
		}
		if (res == _320x240)
		{
			maxX = 320;
			maxY = 240;
			Xoffset = 0;
			Yoffset = 0;
		}
	};
	~ST77XX() {};
	void setXY(short x1, short y1, short x2, short y2);
	void setXY(short x, short y);
	void init(unsigned int freq = 40000000L);
	void drawPixel(short x, short y);
	void fillScr(unsigned char r, unsigned char g, unsigned char b);
	void drawHLine(short x, short y, int l);
	void drawVLine(short x, short y, int l);
	void drawCompressed24bitBitmap(short x, short y, const unsigned int * dataArray);
	void drawCompressedGrayScaleBitmap(short x, short y, const unsigned short * dataArray, bool invert = false);
};

/********** ILI9488 9 bit parallel **********/

typedef struct
{
	unsigned char dataPins[8];
	unsigned char WRpin;
	unsigned char D_Cpin;
	unsigned char ResetPin;
} ILI9488P_SETUP;

class ILI9488_9BIT_PARALLEL : public graphics
{
protected:
	void reset();
	unsigned int bCh, bCl;
public:
	ILI9488_9BIT_PARALLEL(short maxX, short maxY) : graphics(maxX, maxY) {};
	~ILI9488_9BIT_PARALLEL() {};

	void setXY(short x1, short y1, short x2, short y2);
	void setXY(short x, short y);
	void init(ILI9488P_SETUP * ili9488setup);
	void drawPixel(short x, short y);
	void fillScr(unsigned char r,unsigned char g,unsigned char b);
	void setColor(unsigned char r, unsigned char g, unsigned char b);
	void drawHLine(short x, short y, int l);
	void drawVLine(short x, short y, int l);
	void drawCompressed24bitBitmap(short x, short y, const unsigned int * dataArray);
	void drawCompressedGrayScaleBitmap(short x, short y, const unsigned short * dataArray, bool invert = false);
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

#define ESP_WRITE_REG(REG,DATA) (*((volatile unsigned int *)(((REG)))) = DATA )

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
	void fillScr(unsigned char r, unsigned char g,unsigned char b);
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
	void setColor(unsigned char r, unsigned char g,unsigned char b);
	void setBackColor(unsigned char r, unsigned char g,unsigned char b);
	void setFGbitOn() { FGbitOn = true; }; // All Pixels being drawn will be marked as foreground 
	void setFGbitOff() { FGbitOn = false; };

	inline void drawPixel(short x, short y);
	inline void drawPixel(short x, short y, unsigned char color);
	void draw2Pixels(short x, short y, unsigned char color);
	inline bool isFGbitSet(short x, short y);
	void fillScr(unsigned char r, unsigned char g,unsigned char b);
	void drawHLine(short x, short y, int l);
	void drawBitmap(short x, short y, const unsigned char * dataArray, bool useSkipBit, ili9488_8C_flipOption flipOpt = noflip);
	void flushFrameBuffer();
	//int	 swapFrameBuffer();
};


#endif