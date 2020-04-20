#include "ESP32_GB_EMU.h"
#include "SPI.h"
#include "graphics.h"
#include <GeneralLib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Arduino.h>

////// SDL
#define GAMEBOY_HEIGHT 160
#define GAMEBOY_WIDTH 144
unsigned char pixels[GAMEBOY_HEIGHT * GAMEBOY_WIDTH];

extern ILI9488SPI_8C lcd;
extern analogJoyStick JS;
extern Adafruit_FT6206 ts;

static int button_start, button_select, button_a, button_b, button_down, button_up, button_left, button_right;

unsigned char getColorIndexFromFrameBuffer(int x, int y)
{
	int offset = x + y * 160;
	return pixels[offset];
}

const int color[] = { 0x000000, 0x555555, 0xAAAAAA, 0xFFFFFF };
//unsigned char conversionTable[4] = { 0x0,0x12,0x36,0x3f }; // Dark
unsigned char conversionTable[4] = { 0x3f,0x12,0x36,0x00 }; // White
unsigned char tempLCDbuffer[160];

void sdl_init(void)
{

}

int sdl_update(void)
{
	return 0;
}

#define MP_BUTTON_A	27
#define MP_BUTTON_B	33

unsigned int sdl_get_buttons(void)
{
	unsigned short x, y, x2, y2;
	button_a = digitalRead(MP_BUTTON_A);
	button_b = digitalRead(MP_BUTTON_B);
	button_select = 0x4;
	button_start = 0x8;
	if (TOUCH_PANNEL_TOUCHED())
	{
		ts.getTouchedPoints(&x, &y, &x2, &y2);
		if (x > 320 && x < 480)
		{
			button_select = 0;
		}
		else if (x > 0 && x < 80)
		{
			button_start = 0;
		}
	}

	return button_start | button_select | (button_b << 1) | button_a;
}

unsigned int sdl_get_directions(void)
{
	int x, y;
	x = JS.readXaxis();
	y = JS.readYaxis();

	//printf("x %d y %d\n", x, y);
	button_right = 0x1;
	if (x > 500)
	{
		button_right = 0;
	}
	button_left = 0x2;
	if (x < -500)
	{
		button_left = 0;
	}
	button_up = 0x4;
	if (y < -500)
	{
		button_up = 0;
	}
	button_down = 0x8;
	if (y > 500)
	{
		button_down = 0;
	}

	return button_down | button_up | button_left | button_right;
}

unsigned char* sdl_get_framebuffer(void)
{
	return pixels;
}

void flushFrameToLCD(void)
{
	short i, j;
	int offset;
	lcd.setXY(80, 16, 399, 303);
	for (i = 0; i < GAMEBOY_WIDTH; i++)
	{
		offset = i * 160;
		for (j = 0; j < GAMEBOY_HEIGHT; j++)
		{
			tempLCDbuffer[j] = conversionTable[pixels[offset + j]];
			pixels[offset + j] = 0;
		}
		SPI.writeBuffer((unsigned int*)tempLCDbuffer, 40);
		SPI.writeBuffer((unsigned int*)tempLCDbuffer, 40);
	}
}

////// ROM
const unsigned char *bytes;
unsigned int mapper;

static char *banks[] =
{
	" 32KiB",
	" 64KiB",
	"128KiB",
	"256KiB",
	"512KiB",
	"  1MiB",
	"  2MiB",
	"  4MiB",
	/* 0x52 */
	"1.1MiB",
	"1.2MiB",
	"1.5MiB",
	"Unknown"
};

static char *rams[] = {
	"None",
	"  2KiB",
	"  8KiB",
	" 32KiB",
	"Unknown"
};

static char *regions[] = {
	"Japan",
	"Non-Japan",
	"Unknown"
};

static unsigned char header[] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

int rom_init(const unsigned char *rombytes)
{
	char buf[17];
	int type, bank_index, ram, region, version, i, pass;
	unsigned char checksum = 0;

	if (memcmp(&rombytes[0x104], header, sizeof(header)) != 0)
		return 0;

	memcpy(buf, &rombytes[0x134], 16);
	buf[16] = '\0';
	printf("Rom title: %s\n", buf);

	type = rombytes[0x147];

	bank_index = rombytes[0x148];
	/* Adjust for the gap in the bank indicies */
	if (bank_index >= 0x52 && bank_index <= 0x54)
		bank_index -= 74;
	else if (bank_index > 7)
		bank_index = 11;

	printf("Rom size: %s\n", banks[bank_index]);

	ram = rombytes[0x149];
	if (ram > 3)
		ram = 4;

	printf("RAM size: %s\n", rams[ram]);

	region = rombytes[0x14A];
	if (region > 2)
		region = 2;
	printf("Region: %s\n", regions[region]);

	version = rombytes[0x14C];
	printf("Version: %02X\n", version);

	for (i = 0x134; i <= 0x14C; i++)
		checksum = checksum - rombytes[i] - 1;

	pass = rombytes[0x14D] == checksum;

	printf("Checksum: %s (%02X)\n", pass ? "OK" : "FAIL", checksum);
	if (!pass)
		return 0;

	bytes = rombytes;

	switch (type)
	{
	case 0x00:
	case 0x08:
	case 0x09:
		mapper = NROM;
		break;
	case 0x01:
	case 0x02:
	case 0x03:
		mapper = MBC1;
		break;
	case 0x05:
	case 0x06:
		mapper = MBC2;
		break;
	case 0x0B:
	case 0x0C:
		mapper = MMM01;
		break;
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		mapper = MBC3;
		break;
	case 0x15:
	case 0x16:
	case 0x17:
		mapper = MBC4;
		break;
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
		mapper = MBC5;
		break;
	}

	return 1;
}

unsigned int rom_get_mapper(void)
{
	return mapper;
}

int rom_load(const char *filename)
{
	return 0;
}

const unsigned char *rom_getbytes(void)
{
	return bytes;
}

////// MEM
//GBmemory	GBmem;
unsigned char * GBmem;
unsigned char * SecROMbank,* ROMbank2,* ROMbank3;
static int DMA_pending = 0;
static unsigned char joypad_select_buttons, joypad_select_directions;

void mem_bank_switch(unsigned int n)
{
	const unsigned char *b = rom_getbytes();
	//memcpy(&GBmem[0x4000], &b[n * 0x4000], 0x4000);
	switch (n)
	{
	case 1:
		SecROMbank = &GBmem[0x4000];
		break;
	/*case 2:
		SecROMbank = ROMbank2;
		break;
	case 3:
		SecROMbank = ROMbank3;
		break;*/
	default: // n > 3 - connect directly to ROM
		SecROMbank = (unsigned char*)&b[n * 0x4000];
		break;
	}
}

/* LCD's access to VRAM */
unsigned char mem_get_raw(unsigned short p)
{
	return GBmem[p];
}

unsigned char mem_get_byte(unsigned short i)
{
	unsigned long elapsed;
	unsigned char mask = 0xF;

	if (DMA_pending && i < 0xFF80)
	{
		elapsed = cpu_get_cycles() - DMA_pending;
		if (elapsed >= 160)
			DMA_pending = 0;
		else
		{
			return GBmem[0xFE00 + elapsed];
		}
	}

	if (i >= 0x4000 && i <= 0x8000)
	{
		return SecROMbank[i - 0x4000];
	}

	if (i < 0xFF00)
		return GBmem[i];
	switch (i)
	{
	case 0xFF00:	/* Joypad */
		mask = 0xf;
		if (!joypad_select_directions)
			mask = sdl_get_directions();
		else if (!joypad_select_buttons)
			mask = sdl_get_buttons();
		return mask | joypad_select_buttons | joypad_select_directions;
		break;
	case 0xFF04:
		return timer_get_div();
		break;
	case 0xFF05:
		return timer_get_counter();
		break;
	case 0xFF06:
		return timer_get_modulo();
		break;
	case 0xFF07:
		return timer_get_tac();
		break;
	case 0xFF0F:
		return interrupt_get_IF();
		break;
	case 0xFF41:
		return lcd_get_stat();
		break;
	case 0xFF44:
		return lcd_get_line();
		break;
	case 0xFF4D:	/* GBC speed switch */
		return 0xFF;
		break;
	case 0xFFFF:
		return interrupt_get_mask();
		break;
	}

	return GBmem[i];
}

unsigned short mem_get_word(unsigned short i)
{
	unsigned long elapsed;

	if (DMA_pending && i < 0xFF80)
	{
		elapsed = cpu_get_cycles() - DMA_pending;
		if (elapsed >= 160)
			DMA_pending = 0;
		else
		{
			return GBmem[0xFE00 + elapsed];
		}
	}
	if (i >= 0x4000 && i<= 0x8000)
	{
		return SecROMbank[i-0x4000] | (SecROMbank[i - 0x4000 + 1] << 8);
	}
	else
		return GBmem[i] | (GBmem[i + 1] << 8);
}

void mem_write_byte(unsigned short d, unsigned char i)
{
	unsigned int filtered = 0;

	switch (rom_get_mapper())
	{
	case NROM:
		if (d < 0x8000)
			filtered = 1;
		break;
	case MBC2:
	case MBC3:
		filtered = MBC3_write_byte(d, i);
		break;
	case MBC1:
		filtered = MBC1_write_byte(d, i);
		break;
	}

	if (filtered)
		return;

	switch (d)
	{
	case 0xFF00:	/* Joypad */
		joypad_select_buttons = (i & 0x20);
		joypad_select_directions = (i & 0x10);
		break;
	case 0xFF01: /* Link port data */
		break;
	case 0xFF04:
		timer_set_div(i);
		break;
	case 0xFF05:
		timer_set_counter(i);
		break;
	case 0xFF06:
		timer_set_modulo(i);
		break;
	case 0xFF07:
		timer_set_tac(i);
		break;
	case 0xFF0F:
		interrupt_set_IF(i);
		break;
	case 0xFF40:
		lcd_write_control(i);
		break;
	case 0xFF41:
		lcd_write_stat(i);
		break;
	case 0xFF42:
		lcd_write_scroll_y(i);
		break;
	case 0xFF43:
		lcd_write_scroll_x(i);
		break;
	case 0xFF45:
		lcd_set_ly_compare(i);
		break;
	case 0xFF46: /* OAM DMA */
		/* Copy bytes from i*0x100 to OAM */
		memcpy(&GBmem[0xFE00], &GBmem[i * 0x100], 0xA0);
		DMA_pending = cpu_get_cycles();
		break;
	case 0xFF47:
		lcd_write_bg_palette(i);
		break;
	case 0xFF48:
		lcd_write_spr_palette1(i);
		break;
	case 0xFF49:
		lcd_write_spr_palette2(i);
		break;
	case 0xFF4A:
		lcd_set_window_y(i); break;
	case 0xFF4B:
		lcd_set_window_x(i); break;
	case 0xFFFF:
		interrupt_set_mask(i);
		return;
		break;
	}

	GBmem[d] = i;
}

void mem_write_word(unsigned short d, unsigned short i)
{
	GBmem[d] = i & 0xFF;
	GBmem[d + 1] = i >> 8;
}

extern unsigned int gb_rom_size;

unsigned int mem_init()
{
	const unsigned char *bytes = rom_getbytes();

	bool res = 1;
	//res = GBmem.init();
	GBmem = (unsigned char*)malloc(0x10000);
	if (GBmem == 0)
	{
		return 0;
	}

	/*if (gb_rom_size > 32*1024)
	{
		ROMbank2 = (unsigned char*)malloc(0x4000);
		ROMbank3 = (unsigned char*)malloc(0x4000);
		if (ROMbank2 == 0 || ROMbank3 == 0)
		{
			return 0;
		}
	}*/

	memcpy(&GBmem[0x0000], &bytes[0x0000], 0x4000);
	memcpy(&GBmem[0x4000], &bytes[0x4000], 0x4000);
	/*if (gb_rom_size > 32 * 1024)
	{
		memcpy(ROMbank2, &bytes[0x8000], 0x4000);
		memcpy(ROMbank3, &bytes[0xC000], 0x4000);
	}*/
	SecROMbank = &GBmem[0x4000];

	GBmem[0xFF10] = 0x80;
	GBmem[0xFF11] = 0xBF;
	GBmem[0xFF12] = 0xF3;
	GBmem[0xFF14] = 0xBF;
	GBmem[0xFF16] = 0x3F;
	GBmem[0xFF19] = 0xBF;
	GBmem[0xFF1A] = 0x7F;
	GBmem[0xFF1B] = 0xFF;
	GBmem[0xFF1C] = 0x9F;
	GBmem[0xFF1E] = 0xBF;
	GBmem[0xFF20] = 0xFF;
	GBmem[0xFF23] = 0xBF;
	GBmem[0xFF24] = 0x77;
	GBmem[0xFF25] = 0xF3;
	GBmem[0xFF26] = 0xF1;
	GBmem[0xFF40] = 0x91;
	GBmem[0xFF47] = 0xFC;
	GBmem[0xFF48] = 0xFF;
	GBmem[0xFF49] = 0xFF;

	if (res)
		return (unsigned int)GBmem;
	else
		return 0x0;
}

#define __1KB 1024
bool GBmemory::init()
{
	ROMbank0 = (unsigned char *)malloc(__1KB * 16);
	if (ROMbank0 == NULL)
		return false;

	ROMbank1 = (unsigned char *)malloc(__1KB * 16);
	if (ROMbank1 == NULL)
	{
		delete ROMbank0;
		return false;
	}

	VRAM = (unsigned char *)malloc(__1KB * 8);
	if (VRAM == NULL)
	{
		delete ROMbank0;
		delete ROMbank1;
		return false;
	}

	ExternalRAM = (unsigned char *)malloc(__1KB * 8);
	if (ExternalRAM == NULL)
	{
		delete ROMbank0;
		delete ROMbank1;
		delete VRAM;
		return false;
	}

	WorkRAM = (unsigned char *)malloc(__1KB * 8);
	if (WorkRAM == NULL)
	{
		delete ROMbank0;
		delete ROMbank1;
		delete VRAM;
		delete ExternalRAM;
		return false;
	}

	return true;
}

unsigned char & GBmemory::operator[](unsigned short const address)
{
	if ((address >= 0) && (address < 0x4000))
	{
		return ROMbank0[address];
	}
	if ((address >= 0x4000) && (address < 0x8000))
	{
		return ROMbank1[address - 0x4000];
	}
	if ((address >= 0x8000) && (address < 0xA000))
	{
		return VRAM[address - 0x8000];
	}
	if ((address >= 0xA000) && (address < 0xC000))
	{
		return ExternalRAM[address - 0xA000];
	}
	if ((address >= 0xC000) && (address < 0xE000))
	{
		return WorkRAM[address - 0xC000];
	}
	if ((address >= 0xE000) && (address <= 0xFDFF))
	{
		return WorkRAM[address - 0xC000];
	}
	if ((address >= 0xFE00) && (address <= 0xFE9F))
	{
		return OAM[address - 0xFE00];
	}
	if ((address >= 0xFF00) && (address <= 0xFF7F))
	{
		return IOports[address - 0xFF00];
	}
	if ((address >= 0xFF80) && (address <= 0xFFFE))
	{
		return HRAM[address - 0xFF80];
	}
	if (address == 0xffff)
	{
		return InterruptEnableRegister[0];
	}
}

////// MBC
enum {
	NO_FILTER_WRITE,
	FILTER_WRITE
};

static unsigned int bank_upper_bits;
static unsigned int ram_select;

/* Unfinished, no clock etc */
unsigned int MBC3_write_byte(unsigned short d, unsigned char i)
{
	int bank;

	if (d < 0x2000)
	{
		return FILTER_WRITE;
	}

	if (d < 0x4000)
	{
		bank = i & 0x7F;

		if (bank == 0)
			bank++;

		mem_bank_switch(bank);

		return FILTER_WRITE;
	}

	if (d < 0x8000)
		return FILTER_WRITE;

	return NO_FILTER_WRITE;
}
unsigned int MBC1_write_byte(unsigned short d, unsigned char i)
{
	int bank;

	if (d < 0x2000)
	{
		return FILTER_WRITE;
		/* TODO: Enable/disable SRAM */
	}

	/* Switch rom bank at 4000-7fff */
	if (d >= 0x2000 && d < 0x4000)
	{
		/* Bits 0-4 come from the value written to memory here,
		 * bits 5-6 come from a seperate write to 4000-5fff if
		 * RAM select is 1.
		 */
		bank = i & 0x1F;
		if (!ram_select)
			bank |= bank_upper_bits;

		/* "Writing to this address space selects the lower 5 bits of the
		 * ROM Bank Number (in range 01-1Fh). When 00h is written, the MBC
		 * translates that to bank 01h also."
		 * http://nocash.emubase.de/pandocs.htm#mbc1max2mbyteromandor32kbyteram
		 */

		if (bank == 0 || bank == 0x20 || bank == 0x40 || bank == 0x60)
			bank++;

		mem_bank_switch(bank);

		return FILTER_WRITE;
	}

	/* Bit 5 and 6 of the bank selection */
	if (d >= 0x4000 && d < 0x6000)
	{
		bank_upper_bits = (i & 0x3) << 5;
		return FILTER_WRITE;
	}

	if (d >= 0x6000 && d <= 0x7FFF)
	{
		ram_select = i & 1;
		return FILTER_WRITE;
	}
	return NO_FILTER_WRITE;
}

////// LCD
static int lcd_line = 0;
static int lcd_ly_compare;

/* LCD STAT */
static int ly_int;	/* LYC = LY coincidence interrupt enable */
static int mode2_oam_int;
static int mode1_vblank_int;
static int mode0_hblank_int;
static int ly_int_flag;
static int lcd_mode;

/* LCD Control */
static int lcd_enabled;
static int window_tilemap_select;
static int window_enabled;
static int tilemap_select;
static int bg_tiledata_select;
static int sprite_size;
static int sprites_enabled;
static int bg_enabled;
static int scroll_x, scroll_y;
static int window_x, window_y;

//static unsigned char bgpalette[] = { 3, 2, 1, 0 };
static unsigned char bgpalette[] = { 0, 1, 2, 3 };
static unsigned char sprpalette1[] = { 0, 1, 2, 3 };
static unsigned char sprpalette2[] = { 0, 1, 2, 3 };
//static unsigned long colours[4] = { 0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000 };
static unsigned long colours[4] = { 0, 1, 2, 3 };

struct sprite {
	int y, x, tile, flags;
};

enum {
	PRIO = 0x80,
	VFLIP = 0x40,
	HFLIP = 0x20,
	PNUM = 0x10
};

unsigned char lcd_get_stat(void)
{
	return (ly_int) << 6 | lcd_mode;
}

void lcd_write_bg_palette(unsigned char n)
{
	bgpalette[0] = (n >> 0) & 3;
	bgpalette[1] = (n >> 2) & 3;
	bgpalette[2] = (n >> 4) & 3;
	bgpalette[3] = (n >> 6) & 3;
}

void lcd_write_spr_palette1(unsigned char n)
{
	sprpalette1[0] = 0;
	sprpalette1[1] = (n >> 2) & 3;
	sprpalette1[2] = (n >> 4) & 3;
	sprpalette1[3] = (n >> 6) & 3;
}

void lcd_write_spr_palette2(unsigned char n)
{
	sprpalette2[0] = 0;
	sprpalette2[1] = (n >> 2) & 3;
	sprpalette2[2] = (n >> 4) & 3;
	sprpalette2[3] = (n >> 6) & 3;
}

void lcd_write_scroll_x(unsigned char n)
{
	//	printf("x scroll changed to %02x\n", n);
	scroll_x = n;
}

void lcd_write_scroll_y(unsigned char n)
{
	scroll_y = n;
}

int lcd_get_line(void)
{
	return lcd_line;
}

void lcd_write_stat(unsigned char c)
{
	ly_int = !!(c & 0x40);
}

void lcd_write_control(unsigned char c)
{
	//	printf("LCDC set to %02x\n", c);
	//	cpu_print_debug();
	bg_enabled = !!(c & 0x01);
	sprites_enabled = !!(c & 0x02);
	sprite_size = !!(c & 0x04);
	tilemap_select = !!(c & 0x08);
	bg_tiledata_select = !!(c & 0x10);
	window_enabled = !!(c & 0x20);
	window_tilemap_select = !!(c & 0x40);
	lcd_enabled = !!(c & 0x80);
}

void lcd_set_ly_compare(unsigned char c)
{
	lcd_ly_compare = c;
}

void lcd_set_window_y(unsigned char n) {
	window_y = n;
}

void lcd_set_window_x(unsigned char n) {
	window_x = n;
}

static void swap(struct sprite *a, struct sprite *b)
{
	struct sprite c;

	c = *a;
	*a = *b;
	*b = c;
}

static void sort_sprites(struct sprite *s, int n)
{
	int swapped, i;

	do
	{
		swapped = 0;
		for (i = 0; i < n - 1; i++)
		{
			if (s[i].x < s[i + 1].x)
			{
				swap(&s[i], &s[i + 1]);
				swapped = 1;
			}
		}
	} while (swapped);
}

void drawColorIndexToFrameBuffer(int x, int y, unsigned char idx, unsigned char *b) {
	int offset = x + y * 160;
	b[offset] = idx;
}

static void draw_bg_and_window(unsigned char *b, int line)
{
	int x, offset = line * 160;

	for (x = 0; x < 160; x++)
	{
		unsigned int map_select, map_offset, tile_num, tile_addr, xm, ym;
		unsigned char b1, b2, mask, colour;

		/* Convert LCD x,y into full 256*256 style internal coords */
		if (line >= window_y && window_enabled && line - window_y < 144)
		{
			xm = x;
			ym = line - window_y;
			map_select = window_tilemap_select;
		}
		else
		{
			if (!bg_enabled)
			{
				//drawColorIndexToFrameBuffer(x,line,0,b);
				b[offset + x] = 0;
				return;
			}
			//a mod 2i = a & (2i–1)
			//xm = (x + scroll_x)%256;
			xm = (x + scroll_x) & (255);
			//ym = (line + scroll_y)%256;
			ym = (line + scroll_y) & (255);
			map_select = tilemap_select;
		}

		/* Which pixel is this tile on? Find its offset. */
		/* (y/8)*32 calculates the offset of the row the y coordinate is on.
		 * As 256/32 is 8, divide by 8 to map one to the other, this is the row number.
		 * Then multiply the row number by the width of a row, 32, to find the offset.
		 * Finally, add x/(256/32) to find the offset within that row.
		 */
		map_offset = (ym / 8) * 32 + xm / 8;

		//tile_num = mem_get_raw(0x9800 + map_select*0x400 + map_offset);
		tile_num = GBmem[0x9800 + map_select * 0x400 + map_offset];
		if (bg_tiledata_select)
			tile_addr = 0x8000 + tile_num * 16;
		else
			tile_addr = 0x9000 + ((signed char)tile_num) * 16;

		//GBmem
		//b1 = mem_get_raw(tile_addr + (ym & 7) * 2);
		//b2 = mem_get_raw(tile_addr + (ym & 7) * 2 + 1);
		b1 = GBmem[tile_addr + (ym & 7) * 2];
		b2 = GBmem[tile_addr + (ym & 7) * 2 + 1];

		mask = 128 >> (xm & 7);

		colour = (!!(b2&mask) << 1) | !!(b1&mask);
		//drawColorIndexToFrameBuffer(x,line,bgpalette[colour],b);
		b[offset + x] = bgpalette[colour];
	}
}

static void draw_sprites(unsigned char *b, int line, int nsprites, struct sprite *s)
{
	int i, offset = line * 160;
	unsigned char temp;

	for (i = 0; i < nsprites; i++)
	{
		unsigned int b1, b2, tile_addr, sprite_line, x;

		/* Sprite is offscreen */
		if (s[i].x < -7)
			continue;

		/* Which line of the sprite (0-7) are we rendering */
		sprite_line = s[i].flags & VFLIP ? (sprite_size ? 15 : 7) - (line - s[i].y) : line - s[i].y;

		/* Address of the tile data for this sprite line */
		tile_addr = 0x8000 + (s[i].tile * 16) + sprite_line * 2;

		/* The two bytes of data holding the palette entries */
		b1 = GBmem[tile_addr];
		b2 = GBmem[tile_addr + 1];

		/* For each pixel in the line, draw it */
		for (x = 0; x < 8; x++)
		{
			unsigned char mask, colour;
			unsigned char *pal;

			if ((s[i].x + x) >= 160)
				continue;

			mask = s[i].flags & HFLIP ? 128 >> (7 - x) : 128 >> x;
			colour = ((!!(b2&mask)) << 1) | !!(b1&mask);
			if (colour == 0)
				continue;

			pal = (s[i].flags & PNUM) ? sprpalette2 : sprpalette1;
			/* Sprite is behind BG, only render over palette entry 0 */
			if (s[i].flags & PRIO)
			{
				temp = b[offset + x + s[i].x];
				if (temp != colours[bgpalette[0]])
					continue;
			}
			//drawColorIndexToFrameBuffer(x + s[i].x,line,pal[colour],b);
			b[offset + x + s[i].x] = pal[colour];
		}
	}
}

void render_line(int line)
{
	int i, c = 0;

	struct sprite s[10];
	unsigned char *b = pixels;

	for (i = 0; i < 40; i++)
	{
		int y;

		y = GBmem[0xFE00 + (i << 2)] - 16;
		if (line < y || line >= y + 8 + (sprite_size * 8))
			continue;

		s[c].y = y;
		s[c].x = GBmem[0xFE00 + (i << 2) + 1] - 8;
		s[c].tile = GBmem[0xFE00 + (i << 2) + 2];
		s[c].flags = GBmem[0xFE00 + (i << 2) + 3];
		c++;

		if (c == 10)
			break;
	}

	if (c)
		sort_sprites(s, c);

	/* Draw the background layer */
	draw_bg_and_window(b, line);

	draw_sprites(b, line, c, s);
}

int count = 0;
unsigned long et, frames = 0;
#define SKIP_FRAMES	3
int skipFramesCounter = 0;

int lcd_cycle(void)
{
	int cycles = cpu_get_cycles();
	int this_frame, subframe_cycles;
	static int prev_line;

	this_frame = cycles % (70224 / 4);
	lcd_line = this_frame / (456 / 4);

	if (this_frame < 204 / 4)
		lcd_mode = 2;
	else if (this_frame < 284 / 4)
		lcd_mode = 3;
	else if (this_frame < 456 / 4)
		lcd_mode = 0;
	if (lcd_line >= 144)
		lcd_mode = 1;

	if ((lcd_line != prev_line) && (lcd_line < 144))
	{
		if (skipFramesCounter == SKIP_FRAMES)
		{
			render_line(lcd_line);
		}
	}

	if (ly_int && (lcd_line == lcd_ly_compare))
		interrupt(INTR_LCDSTAT);

	if ((prev_line == 143) && (lcd_line == 144))
	{
		interrupt(INTR_VBLANK);
		if (skipFramesCounter == SKIP_FRAMES)
		{
			skipFramesCounter = 0;
			flushFrameToLCD();
		}
		else
		{
			skipFramesCounter++;
		}
		frames++;
	}
	prev_line = lcd_line;

	return 1;
}

////// TIMER
static unsigned int prev_time;
static unsigned int elapsed;
static unsigned int ticks;

static unsigned char tac;
static unsigned int started;
static unsigned int speed;
static unsigned int counter;
static unsigned int divider;
static unsigned int modulo;

void timer_set_div(unsigned char v)
{
	(void)v;
	divider = 0;
}

unsigned char timer_get_div(void)
{
	return divider;
}

void timer_set_counter(unsigned char v)
{
	counter = v;
}

unsigned char timer_get_counter(void)
{
	return counter;
}

void timer_set_modulo(unsigned char v)
{
	modulo = v;
}

unsigned char timer_get_modulo(void)
{
	return modulo;
}

void timer_set_tac(unsigned char v)
{
	//int speeds[] = {64, 1, 4, 16};
	int speeds[] = { 1024, 16, 64, 256 };
	tac = v;
	started = v & 4;
	speed = speeds[v & 3];
}

unsigned char timer_get_tac(void)
{
	return tac;
}

static void timer_tick(void)
{
	/* 1/262144Hz has elapsed */
	ticks++;

	/* Divider updates at 16384Hz */
	if (ticks == 16)
	{
		divider++;
		ticks = 0;
	}

	if (!started)
		return;

	if (ticks == speed)
	{
		counter++;
		ticks = 0;
	}

	//if(counter == 0x100)
	if (counter >= 0x100)
	{
		interrupt(INTR_TIMER);
		counter = modulo;
	}
}

void timer_cycle(void)
{
	/* The amount of ticks since we last ran */
	unsigned int delta = cpu_get_cycles() - prev_time;
	prev_time = cpu_get_cycles();

	elapsed += delta * 4; /* 4 cycles to a timer tick */
	if (elapsed >= 16)
	{
		timer_tick();
		elapsed -= 16;	/* keep track of the time overflow */
	}
}

////// INTERRUPT
static int enabled;
static int pending;

/* Pending interrupt flags */
static unsigned int vblank;
static unsigned int lcdstat;
static unsigned int timer;
static unsigned int serial;
static unsigned int joypad;

/* Interrupt masks */
static unsigned int vblank_masked = 1;
static unsigned int lcdstat_masked = 1;
static unsigned int timer_masked = 1;
static unsigned int serial_masked = 1;
static unsigned int joypad_masked = 1;

/* Returns true if the cpu should be unhalted */
int inline interrupt_flush(void)
{
	/* Flush the highest priority interrupt and/or resume the cpu */
	if (pending == 2)
	{
		pending--;
		return 0;
	}

	pending = 0;

	/* There's a pending interrupt but interrupts are disabled, just resume the cpu */
	if (!enabled && ((vblank && !vblank_masked)
		|| (lcdstat && !lcdstat_masked)
		|| (timer && !timer_masked)
		|| (serial && !serial_masked)
		|| (joypad && !joypad_masked))
		)
		return 1;

	/* Interrupts are enabled - Check if any need to fire */
	if (vblank && !vblank_masked)
	{
		vblank = 0;
		cpu_interrupt(0x40);
	}
	else if (lcdstat && !lcdstat_masked)
	{
		lcdstat = 0;
		cpu_interrupt(0x48);
	}
	else if (timer && !timer_masked)
	{
		timer = 0;
		cpu_interrupt(0x50);
	}
	else if (serial && !serial_masked)
	{
		serial = 0;
		cpu_interrupt(0x58);
	}
	else if (joypad && !joypad_masked)
	{
		joypad = 0;
		cpu_interrupt(0x60);
	}

	return 0;
}

void interrupt_enable(void)
{
	enabled = 1;
	pending = 2;
}

void interrupt_disable(void)
{
	enabled = 0;
}

void interrupt(unsigned int n)
{
	/* Add this interrupt to pending queue */
	switch (n)
	{
	case INTR_VBLANK:
		vblank = 1;
		break;
	case INTR_LCDSTAT:
		lcdstat = 1;
		break;
	case INTR_TIMER:
		timer = 1;
		break;
	case INTR_SERIAL:
		serial = 1;
		break;
	case INTR_JOYPAD:
		joypad = 1;
		break;
	}

	/* If interrupts are already enabled, flush one now, otherwise wait for
	 * interrupts to be re-enabled.
	 */
	if (enabled)
		interrupt_flush();
}

unsigned char interrupt_get_IF(void)
{
	unsigned char mask = 0xE0;

	mask |= (vblank << 0);
	mask |= (lcdstat << 1);
	mask |= (timer << 2);
	mask |= (serial << 3);
	mask |= (joypad << 4);

	return mask;
}

void interrupt_set_IF(unsigned char mask)
{
	//	printf("IF set to %02x\n", mask);
	vblank = !!(mask & 0x01);
	lcdstat = !!(mask & 0x02);
	timer = !!(mask & 0x04);
	serial = !!(mask & 0x08);
	joypad = !!(mask & 0x10);

	if (enabled && mask)
		pending = 1;
}

unsigned char interrupt_get_mask(void)
{
	unsigned char mask = 0;

	mask |= (!vblank_masked << 0);
	mask |= (!lcdstat_masked << 1);
	mask |= (!timer_masked << 2);
	mask |= (!serial_masked << 3);
	mask |= (!joypad_masked << 4);

	return mask;
}

void interrupt_set_mask(unsigned char mask)
{
	vblank_masked = !(mask & 0x01);
	lcdstat_masked = !(mask & 0x02);
	timer_masked = !(mask & 0x04);
	serial_masked = !(mask & 0x08);
	joypad_masked = !(mask & 0x10);
}

////// CPU
#define set_HL(x) do {unsigned int macro = (x); c.L = macro&0xFF; c.H = macro>>8;} while(0)
#define set_BC(x) do {unsigned int macro = (x); c.C = macro&0xFF; c.B = macro>>8;} while(0)
#define set_DE(x) do {unsigned int macro = (x); c.E = macro&0xFF; c.D = macro>>8;} while(0)
#define set_AF(x) do {unsigned int macro = (x); c.F = macro&0xFF; c.A = macro>>8;} while(0)

#define get_AF() ((c.A<<8) | c.F)
#define get_BC() ((c.B<<8) | c.C)
#define get_DE() ((c.D<<8) | c.E)
#define get_HL() ((c.H<<8) | c.L)

/* Flags */
#define set_Z(x) c.F = ((c.F&0x7F) | ((x)<<7))
#define set_N(x) c.F = ((c.F&0xBF) | ((x)<<6))
#define set_H(x) c.F = ((c.F&0xDF) | ((x)<<5))
#define set_C(x) c.F = ((c.F&0xEF) | ((x)<<4))

#define flag_Z !!((c.F & 0x80))
#define flag_N !!((c.F & 0x40))
#define flag_H !!((c.F & 0x20))
#define flag_C !!((c.F & 0x10))

struct CPU {
	unsigned char H;
	unsigned char L;

	unsigned char D;
	unsigned char E;

	unsigned char B;
	unsigned char C;

	unsigned char A;
	unsigned char F;

	unsigned short SP;
	unsigned short PC;
	unsigned int cycles;
};

static struct CPU c;
static int is_debugged;
static int halted;

void cpu_init(void)
{
	set_AF(0x01B0);
	set_BC(0x0013);
	set_DE(0x00D8);
	set_HL(0x014D);
	c.SP = 0xFFFE;
	c.PC = 0x0100;
	c.cycles = 0;
}

static void RLC(unsigned char reg)
{
	unsigned char t, old;

	switch (reg)
	{
	case 0:	/* B */
		old = !!(c.B & 0x80);
		c.B = (c.B << 1) | old;
		set_C(old);
		set_Z(!c.B);
		break;
	case 1:	/* C */
		old = !!(c.C & 0x80);
		set_C(old);
		c.C = c.C << 1 | old;
		set_Z(!c.C);
		break;
	case 2:	/* D */
		old = !!(c.D & 0x80);
		set_C(old);
		c.D = c.D << 1 | old;
		set_Z(!c.D);
		break;
	case 3:	/* E */
		old = !!(c.E & 0x80);
		set_C(old);
		c.E = c.E << 1 | old;
		set_Z(!c.E);
		break;
	case 4:	/* H */
		old = !!(c.H & 0x80);
		set_C(old);
		c.H = c.H << 1 | old;
		set_Z(!c.H);
		break;
	case 5:	/* L */
		old = !!(c.L & 0x80);
		set_C(old);
		c.L = c.L << 1 | old;
		set_Z(!c.L);
		break;
	case 6:	/* (HL) */
		t = mem_get_byte(get_HL());
		old = !!(t & 0x80);
		set_C(old);
		t = t << 1 | old;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		break;
	case 7:	/* A */
		old = !!(c.A & 0x80);
		c.A = (c.A << 1) | old;
		set_C(old);
		set_Z(!c.A);
		break;
	}

	set_N(0);
	set_H(0);
}

static void RRC(unsigned char reg)
{
	unsigned char t, old;

	switch (reg)
	{
	case 0:	/* B */
		old = c.B & 1;
		set_C(old);
		c.B = c.B >> 1 | old << 7;
		set_Z(!c.B);
		break;
	case 1:	/* C */
		old = c.C & 1;
		set_C(old);
		c.C = c.C >> 1 | old << 7;
		set_Z(!c.C);
		break;
	case 2:	/* D */
		old = c.D & 1;
		set_C(old);
		c.D = c.D >> 1 | old << 7;
		set_Z(!c.D);
		break;
	case 3:	/* E */
		old = c.E & 1;
		set_C(old);
		c.E = c.E >> 1 | old << 7;
		set_Z(!c.E);
		break;
	case 4:	/* H */
		old = c.H & 1;
		set_C(old);
		c.H = c.H >> 1 | old << 7;
		set_Z(!c.H);
		break;
	case 5:	/* L */
		old = c.L & 1;
		set_C(old);
		c.L = c.L >> 1 | old << 7;
		set_Z(!c.L);
		break;
	case 6:	/* (HL) */
		t = mem_get_byte(get_HL());
		old = t;
		set_C(old);
		t = t >> 1 | old << 7;
		c.cycles += 2;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		break;
	case 7:	/* A */
		old = c.A & 1;
		set_C(old);
		c.A = c.A >> 1 | old << 7;
		set_Z(!c.A);
		break;
	}
	set_N(0);
	set_H(0);
}

static void RL(unsigned char reg)
{
	unsigned char t, t2;

	switch (reg)
	{
	case 0: /* B */
		t2 = flag_C;
		set_C(!!(c.B & 0x80));
		c.B = (c.B << 1) | !!(t2);
		set_Z(!c.B);
		break;
	case 1: /* C */
		t2 = flag_C;
		set_C(!!(c.C & 0x80));
		c.C = (c.C << 1) | !!(t2);
		set_Z(!c.C);
		break;
	case 2: /* D */
		t2 = flag_C;
		set_C(!!(c.D & 0x80));
		c.D = (c.D << 1) | !!(t2);
		set_Z(!c.D);
		break;
	case 3: /* E */
		t2 = flag_C;
		set_C(!!(c.E & 0x80));
		c.E = (c.E << 1) | !!(t2);
		set_Z(!c.E);
		break;
	case 4: /* H */
		t2 = flag_C;
		set_C(!!(c.H & 0x80));
		c.H = (c.H << 1) | !!(t2);
		set_Z(!c.H);
		break;
	case 5: /* L */
		t2 = flag_C;
		set_C(!!(c.L & 0x80));
		c.L = (c.L << 1) | !!(t2);
		set_Z(!c.L);
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		t2 = flag_C;
		set_C(!!(t & 0x80));
		t = (t << 1) | !!(t2);
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		c.cycles += 2;
		break;
	case 7: /* A */
		t2 = flag_C;
		set_C(!!(c.A & 0x80));
		c.A = (c.A << 1) | t2;
		set_Z(!c.A);
		break;
	}

	set_N(0);
	set_H(0);
}

static void RR(unsigned char reg)
{
	unsigned char t, t2;

	switch (reg)
	{
	case 0:	/* B */
		t2 = flag_C;
		set_C(c.B & 1);
		c.B = (c.B >> 1) | t2 << 7;
		set_Z(!c.B);
		break;
	case 1: /* C */
		t2 = flag_C;
		set_C(c.C & 1);
		c.C = (c.C >> 1) | t2 << 7;
		set_Z(!c.C);
		break;
	case 2: /* D */
		t2 = flag_C;
		set_C(c.D & 1);
		c.D = (c.D >> 1) | t2 << 7;
		set_Z(!c.D);
		break;
	case 3: /* E */
		t2 = flag_C;
		set_C(c.E & 1);
		c.E = (c.E >> 1) | t2 << 7;
		set_Z(!c.E);
		break;
	case 4: /* H */
		t2 = flag_C;
		set_C(c.H & 1);
		c.H = (c.H >> 1) | t2 << 7;
		set_Z(!c.H);
		break;
	case 5: /* L */
		t2 = flag_C;
		set_C(c.L & 1);
		c.L = (c.L >> 1) | t2 << 7;
		set_Z(!c.L);
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		t2 = flag_C;
		set_C(t & 1);
		t = (t >> 1) | t2 << 7;
		set_Z(!t);
		mem_write_byte(get_HL(), t);
		c.cycles += 2;
		break;
	case 7: /* A */
		t2 = flag_C;
		set_C(c.A & 1);
		c.A = (c.A >> 1) | (t2 << 7);
		set_Z(!c.A);
		break;
	}
	set_N(0);
	set_H(0);
}

static void SLA(unsigned char reg)
{
	unsigned char t;

	switch (reg)
	{
	case 0: /* B */
		set_C(!!(c.B & 0x80));
		c.B = c.B << 1;
		set_Z(!c.B);
		break;
	case 1: /* C */
		set_C(!!(c.C & 0x80));
		c.C = c.C << 1;
		set_Z(!c.C);
		break;
	case 2: /* D */
		set_C(!!(c.D & 0x80));
		c.D = c.D << 1;
		set_Z(!c.D);
		break;
	case 3: /* E */
		set_C(!!(c.E & 0x80));
		c.E = c.E << 1;
		set_Z(!c.E);
		break;
	case 4: /* H */
		set_C(!!(c.H & 0x80));
		c.H = c.H << 1;
		set_Z(!c.H);
		break;
	case 5: /* L */
		set_C(!!(c.L & 0x80));
		c.L = c.L << 1;
		set_Z(!c.L);
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		set_C(!!(t & 0x80));
		t = t << 1;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		c.cycles += 2;
		break;
	case 7: /* A */
		set_C(!!(c.A & 0x80));
		c.A = c.A << 1;
		set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);
}

static void SRA(unsigned char reg)
{
	unsigned char old, t;

	switch (reg)
	{
	case 0: /* B */
		set_C(c.B & 1);
		old = c.B & 0x80;
		c.B = c.B >> 1 | old;
		set_Z(!c.B);
		break;
	case 1: /* C */
		set_C(c.C & 1);
		old = c.C & 0x80;
		c.C = c.C >> 1 | old;
		set_Z(!c.C);
		break;
	case 2: /* D */
		set_C(c.D & 1);
		old = c.D & 0x80;
		c.D = c.D >> 1 | old;
		set_Z(!c.D);
		break;
	case 3: /* E */
		set_C(c.E & 1);
		old = c.E & 0x80;
		c.E = c.E >> 1 | old;
		set_Z(!c.E);
		break;
	case 4: /* H */
		set_C(c.H & 1);
		old = c.H & 0x80;
		c.H = c.H >> 1 | old;
		set_Z(!c.H);
		break;
	case 5: /* L */
		set_C(c.L & 1);
		old = c.L & 0x80;
		c.L = c.L >> 1 | old;
		set_Z(!c.L);
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		set_C(t & 1);
		old = t & 0x80;
		t = t >> 1 | old;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		break;
	case 7: /* A */
		set_C(c.A & 1);
		old = c.A & 0x80;
		c.A = c.A >> 1 | old;
		set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);
}

static void SRL(unsigned char reg)
{
	unsigned char t;

	switch (reg)
	{
	case 0: /* B */
		set_C(c.B & 1);
		c.B = c.B >> 1;
		set_Z(!c.B);
		break;
	case 1: /* C */
		set_C(c.C & 1);
		c.C = c.C >> 1;
		set_Z(!c.C);
		break;
	case 2: /* D */
		set_C(c.D & 1);
		c.D = c.D >> 1;
		set_Z(!c.D);
		break;
	case 3: /* E */
		set_C(c.E & 1);
		c.E = c.E >> 1;
		set_Z(!c.E);
		break;
	case 4: /* H */
		set_C(c.H & 1);
		c.H = c.H >> 1;
		set_Z(!c.H);
		break;
	case 5: /* L */
		set_C(c.L & 1);
		c.L = c.L >> 1;
		set_Z(!c.L);
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		set_C(t & 1);
		t = t >> 1;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		c.cycles += 2;
		break;
	case 7: /* A */
		set_C(c.A & 1);
		c.A = c.A >> 1;
		set_Z(!c.A);
		break;
	}

	set_H(0);
	set_N(0);
}

static void SWAP(unsigned char reg)
{
	unsigned char t;

	switch (reg)
	{
	case 0: /* B */
		c.B = ((c.B & 0xF) << 4) | ((c.B & 0xF0) >> 4);
		c.F = (!c.B) << 7;
		break;
	case 1: /* C */
		c.C = ((c.C & 0xF) << 4) | ((c.C & 0xF0) >> 4);
		c.F = (!c.C) << 7;
		break;
	case 2: /* D */
		c.D = ((c.D & 0xF) << 4) | ((c.D & 0xF0) >> 4);
		c.F = (!c.D) << 7;
		break;
	case 3: /* E */
		c.E = ((c.E & 0xF) << 4) | ((c.E & 0xF0) >> 4);
		c.F = (!c.E) << 7;
		break;
	case 4: /* H */
		c.H = ((c.H & 0xF) << 4) | ((c.H & 0xF0) >> 4);
		c.F = (!c.H) << 7;
		break;
	case 5: /* L */
		c.L = ((c.L & 0xF) << 4) | ((c.L & 0xF0) >> 4);
		c.F = (!c.L) << 7;
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		t = ((t & 0xF) << 4) | ((t & 0xF0) >> 4);
		mem_write_byte(get_HL(), t);
		c.F = (!t) << 7;
		c.cycles += 2;
		break;
	case 7: /* A */
		c.A = ((c.A & 0xF) << 4) | ((c.A & 0xF0) >> 4);
		c.F = (!c.A) << 7;
		break;
	}
}

static void BITfunc(unsigned char bit, unsigned char reg)
{
	unsigned char t, f = 0 /* Make GCC happy */;

	switch (reg)
	{
	case 0: /* B */
		f = !(c.B & bit);
		break;
	case 1: /* C */
		f = !(c.C & bit);
		break;
	case 2: /* D */
		f = !(c.D & bit);
		break;
	case 3: /* E */
		f = !(c.E & bit);
		break;
	case 4: /* H */
		f = !(c.H & bit);
		break;
	case 5: /* L */
		f = !(c.L & bit);
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		f = !(t & bit);
		c.cycles += 1;
		break;
	case 7: /* A */
		f = !(c.A & bit);
		break;
	}

	set_Z(f);
	set_N(0);
	set_H(1);
}

static void RES(unsigned char bit, unsigned char reg)
{
	unsigned char t;

	switch (reg)
	{
	case 0: /* B */
		c.B &= ~bit;
		break;
	case 1: /* C */
		c.C &= ~bit;
		break;
	case 2: /* D */
		c.D &= ~bit;
		break;
	case 3: /* E */
		c.E &= ~bit;
		break;
	case 4: /* H */
		c.H &= ~bit;
		break;
	case 5: /* L */
		c.L &= ~bit;
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		t &= ~bit;
		mem_write_byte(get_HL(), t);
		c.cycles += 2;
		break;
	case 7: /* A */
		c.A &= ~bit;
		break;
	}
}

static void SET(unsigned char bit, unsigned char reg)
{
	unsigned char t;

	switch (reg)
	{
	case 0: /* B */
		c.B |= bit;
		break;
	case 1: /* C */
		c.C |= bit;
		break;
	case 2: /* D */
		c.D |= bit;
		break;
	case 3: /* E */
		c.E |= bit;
		break;
	case 4: /* H */
		c.H |= bit;
		break;
	case 5: /* L */
		c.L |= bit;
		break;
	case 6: /* (HL) */
		t = mem_get_byte(get_HL());
		t |= bit;
		mem_write_byte(get_HL(), t);
		c.cycles += 2;
		break;
	case 7: /* A */
		c.A |= bit;
		break;
	}
}

/*
00000xxx = RLC xxx
00001xxx = RRC xxx
00010xxx = RL xxx
00011xxx = RR xxx
00100xxx = SLA xxx
00101xxx = SRA xxx
00110xxx = SWAP xxx
00111xxx = SRL xxx
01yyyxxx = BIT yyy, xxx
10yyyxxx = RES yyy, xxx
11yyyxxx = SET yyy, xxx
*/
static void decode_CB(unsigned char t)
{
	unsigned char reg, opcode, bit;
	void(*f[])(unsigned char) = { RLC, RRC, RL, RR, SLA, SRA, SWAP, SRL };
	void(*f2[])(unsigned char, unsigned char) = { BITfunc, RES, SET };

	reg = t & 7;
	opcode = t >> 3;
	if (opcode < 8)
	{
		f[opcode](reg);
		return;
	}

	bit = opcode & 7;
	opcode >>= 3;
	f2[opcode - 1](1 << bit, reg);
}

void cpu_interrupt(unsigned short vector)
{
	halted = 0;

	c.SP -= 2;
	mem_write_word(c.SP, c.PC);
	c.PC = vector;
	interrupt_disable();
}

unsigned int cpu_get_cycles(void)
{
	return c.cycles;
}

void cpu_print_debug(void)
{
	printf("%04X: %02X\n", c.PC, mem_get_byte(c.PC));
	printf("\tAF: %02X%02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X SP: %04X, cycles %d\n",
		c.A, c.F, c.B, c.C, c.D, c.E, c.H, c.L, c.SP, c.cycles);
}

int cpu_cycle(void)
{
	unsigned char b, t;
	unsigned short s;
	unsigned int i;

	if (halted)
	{
		c.cycles += 1;
		return 1;
	}

	if (interrupt_flush())
	{
		halted = 0;
	}

	b = mem_get_byte(c.PC);

#ifdef EBUG
	//	if(c.PC == 0x2F38 && c.cycles > 10000000)
	//	if(c.PC == 0xff87 && c.cycles > 14000000)
	//		is_debugged = 0;
#endif
	is_debugged = 0;
	if (is_debugged)
	{
		cpu_print_debug();
	}
	switch (b)
	{
	case 0x00:	/* NOP */
		c.PC++;
		c.cycles += 1;
		break;
	case 0x01:	/* LD BC, imm16 */
		s = mem_get_word(c.PC + 1);
		set_BC(s);
		c.PC += 3;
		c.cycles += 3;
		break;
	case 0x02:	/* LD (BC), A */
		mem_write_byte(get_BC(), c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x03:	/* INC BC */
		set_BC(get_BC() + 1);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x04:	/* INC B */
		set_H((c.B & 0xF) == 0xF);
		c.B++;
		set_Z(!c.B);
		set_N(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x05:	/* DEC B */
		c.B--;
		set_Z(!c.B);
		set_N(1);
		set_H((c.B & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x06:	/* LD B, imm8 */
		c.B = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x07:	/* RLCA */
		RLC(7);
		set_Z(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x08:	/* LD (imm16), SP */
		mem_write_word(mem_get_word(c.PC + 1), c.SP);
		c.PC += 3;
		c.cycles += 5;
		break;
	case 0x09:	/* ADD HL, BC */
		i = get_HL() + get_BC();
		set_N(0);
		set_C(i >= 0x10000);
		set_H((i & 0xFFF) < (get_HL() & 0xFFF));
		set_HL(i & 0xFFFF);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x0A:	/* LD A, (BC) */
		c.A = mem_get_byte(get_BC());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x0B:	/* DEC BC */
		s = get_BC();
		s--;
		set_BC(s);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x0C:	/* INC C */
		set_H((c.C & 0xF) == 0xF);
		c.C++;
		set_Z(!c.C);
		set_N(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x0D:	/* DEC C */
		set_H((c.C & 0xF) == 0);
		c.C--;
		set_Z(!c.C);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x0E:	/* LD C, imm8 */
		c.C = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x0F:	/* RRCA */
		RRC(7);
		set_Z(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x11:	/* LD DE, imm16 */
		s = mem_get_word(c.PC + 1);
		set_DE(s);
		c.PC += 3;
		c.cycles += 3;
		break;
	case 0x12:	/* LD (DE), A */
		mem_write_byte(get_DE(), c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x13:	/* INC DE */
		s = get_DE();
		s++;
		set_DE(s);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x14:	/* INC D */
		set_H((c.D & 0xF) == 0xF);
		c.D++;
		set_Z(!c.D);
		set_N(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x15:	/* DEC D */
		c.D--;
		set_Z(!c.D);
		set_N(1);
		set_H((c.D & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x16:	/* LD D, imm8 */
		c.D = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x17:	/* RLA */
		RL(7);
		set_Z(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x18:	/* JR rel8 */
		c.PC += (signed char)mem_get_byte(c.PC + 1) + 2;
		c.cycles += 3;
		break;
	case 0x19:	/* ADD HL, DE */
		i = get_HL() + get_DE();
		set_H((i & 0xFFF) < (get_HL() & 0xFFF));
		set_HL(i);
		set_N(0);
		set_C(i > 0xFFFF);
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0x1A:	/* LD A, (DE) */
		c.A = mem_get_byte(get_DE());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x1B:	/* DEC DE */
		s = get_DE();
		s--;
		set_DE(s);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x1C:	/* INC E */
		set_H((c.E & 0xF) == 0xF);
		c.E++;
		set_Z(!c.E);
		set_N(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x1D:	/* DEC E */
		c.E--;
		set_Z(!c.E);
		set_N(1);
		set_H((c.E & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x1E:	/* LD E, imm8 */
		c.E = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x1F:	/* RR A */
		RR(7);
		set_Z(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x20:	/* JR NZ, rel8 */
		if (flag_Z == 0)
		{
			c.PC += (signed char)mem_get_byte(c.PC + 1) + 2;
			c.cycles += 3;
		}
		else {
			c.PC += 2;
			c.cycles += 2;
		}
		break;
	case 0x21:	/* LD HL, imm16 */
		s = mem_get_word(c.PC + 1);
		set_HL(s);
		c.PC += 3;
		c.cycles += 3;
		break;
	case 0x22:	/* LDI (HL), A */
		i = get_HL();
		mem_write_byte(i, c.A);
		i++;
		set_HL(i);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x23:	/* INC HL */
		s = get_HL();
		s++;
		set_HL(s);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x24:	/* INC H */
		c.H++;
		set_Z(!c.H);
		set_H((c.H & 0xF) == 0);
		set_N(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x25:	/* DEC H */
		c.H--;
		set_Z(!c.H);
		set_N(1);
		set_H((c.H & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x26:	/* LD H, imm8 */
		c.H = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x27:	/* DAA */
		s = c.A;

		if (flag_N)
		{
			if (flag_H)
				s = (s - 0x06) & 0xFF;
			if (flag_C)
				s -= 0x60;
		}
		else
		{
			if (flag_H || (s & 0xF) > 9)
				s += 0x06;
			if (flag_C || s > 0x9F)
				s += 0x60;
		}

		c.A = s;
		set_H(0);
		set_Z(!c.A);
		if (s >= 0x100)
			set_C(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x28:	/* JR Z, rel8 */
		if (flag_Z == 1)
		{
			c.PC += (signed char)mem_get_byte(c.PC + 1) + 2;
			c.cycles += 3;
		}
		else {
			c.PC += 2;
			c.cycles += 2;
		}
		break;
	case 0x29:	/* ADD HL, HL */
		i = get_HL() * 2;
		set_H((i & 0x7FF) < (get_HL() & 0x7FF));
		set_C(i > 0xFFFF);
		set_HL(i);
		set_N(0);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x2A:	/* LDI A, (HL) */
		s = get_HL();
		c.A = mem_get_byte(s);
		set_HL(s + 1);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x2B: 	/* DEC HL */
		set_HL(get_HL() - 1);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x2C:	/* INC L */
		c.L++;
		set_Z(!c.L);
		set_N(0);
		set_H((c.L & 0xF) == 0x00);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x2D:	/* DEC L */
		c.L--;
		set_Z(!c.L);
		set_N(1);
		set_H((c.L & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x2E:	/* LD L, imm8 */
		c.L = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x2F:	/* CPL */
		c.A = ~c.A;
		set_N(1);
		set_H(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x30:	/* JR NC, rel8 */
		if (flag_C == 0)
		{
			c.PC += (signed char)mem_get_byte(c.PC + 1) + 2;
			c.cycles += 3;
		}
		else {
			c.PC += 2;
			c.cycles += 2;
		}
		break;
	case 0x31:	/* LD SP, imm16 */
		c.SP = mem_get_word(c.PC + 1);
		c.PC += 3;
		c.cycles += 3;
		break;
	case 0x32:	/* LDD (HL), A */
		i = get_HL();
		mem_write_byte(i, c.A);
		set_HL(i - 1);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x33:	/* INC SP */
		c.SP++;
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x34:	/* INC (HL) */
		t = mem_get_byte(get_HL());
		t++;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		set_N(0);
		set_H((t & 0xF) == 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x35:	/* DEC (HL) */
		t = mem_get_byte(get_HL());
		t--;
		mem_write_byte(get_HL(), t);
		set_Z(!t);
		set_N(1);
		set_H((t & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x36:	/* LD (HL), imm8 */
		t = mem_get_byte(c.PC + 1);
		mem_write_byte(get_HL(), t);
		c.PC += 2;
		c.cycles += 3;
		break;
	case 0x37:	/* SCF */
		set_N(0);
		set_H(0);
		set_C(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x38:  /* JR C, rel8 */
		if (flag_C == 1)
		{
			c.PC += (signed char)mem_get_byte(c.PC + 1) + 2;
			c.cycles += 3;
		}
		else {
			c.PC += 2;
			c.cycles += 2;
		}
		break;
	case 0x39:	/* ADD HL, SP */
		i = get_HL() + c.SP;
		set_H((i & 0x7FF) < (get_HL() & 0x7FF));
		set_C(i > 0xFFFF);
		set_N(0);
		set_HL(i);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x3A:	/* LDD A, (HL) */
		c.A = mem_get_byte(get_HL());
		set_HL(get_HL() - 1);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x3B:	/* DEC SP */
		c.SP--;
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x3C:	/* INC A */
		c.A++;
		set_Z(!c.A);
		set_H((c.A & 0xF) == 0);
		set_N(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x3D:	/* DEC A */
		c.A--;
		set_Z(!c.A);
		set_N(1);
		set_H((c.A & 0xF) == 0xF);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x3E:	/* LD A, imm8 */
		c.A = mem_get_byte(c.PC + 1);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0x3F:	/* CCF */
		set_N(0);
		set_H(0);
		set_C(!flag_C);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x40:	/* LD B, B */
		c.B = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x41:	/* LD B, C */
		c.B = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x42:	/* LD B, D */
		c.B = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x43:	/* LD B, E */
		c.B = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x44:	/* LD B, H */
		c.B = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x45:	/* LD B, L */
		c.B = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x46:	/* LD B, (HL) */
		c.B = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x47:	/* LD B, A */
		c.B = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x48:	/* LD C, B */
		c.C = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x49:	/* LD C, C */
		c.C = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x4A:	/* LD C, D */
		c.C = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x4B:	/* LD C, E */
		c.C = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x4C:	/* LD C, H */
		c.C = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x4D:	/* LD C, L */
		c.C = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x4E:	/* LD C, (HL) */
		c.C = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x4F:	/* LD C, A */
		c.C = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x50:	/* LD D, B */
		c.D = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x51:	/* LD D, C */
		c.D = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x52:	/* LD D, D */
		c.D = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x53:	/* LD D, E */
		c.D = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x54:	/* LD D, H */
		c.D = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x55:	/* LD D, L */
		c.D = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x56:	/* LD D, (HL) */
		c.D = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x57:	/* LD D, A */
		c.D = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x58:	/* LD E, B */
		c.E = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x59:	/* LD E, C */
		c.E = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x5A:	/* LD E, D */
		c.E = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x5B:	/* LD E, E */
		c.E = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x5C:	/* LD E, H */
		c.E = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x5D:	/* LD E, L */
		c.E = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x5E:	/* LD E, (HL) */
		c.E = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x5F:	/* LD E, A */
		c.E = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x60:	/* LD H, B */
		c.H = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x61:	/* LD H, C */
		c.H = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x62:	/* LD H, D */
		c.H = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x63:	/* LD H, E */
		c.H = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x64:	/* LD H, H */
		c.H = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x65:	/* LD H, L */
		c.H = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x66:	/* LD H, (HL) */
		c.H = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x67:	/* LD H, A */
		c.H = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x68:	/* LD L, B */
		c.L = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x69:	/* LD L, C */
		c.L = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x6A:	/* LD L, D */
		c.L = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x6B:	/* LD L, E */
		c.L = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x6C:	/* LD L, H */
		c.L = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x6D:	/* LD L, L */
		c.L = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x6E:	/* LD L, (HL) */
		c.L = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x6F:	/* LD L, A */
		c.L = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x70:	/* LD (HL), B */
		mem_write_byte(get_HL(), c.B);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x71:	/* LD (HL), C */
		mem_write_byte(get_HL(), c.C);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x72:	/* LD (HL), D */
		mem_write_byte(get_HL(), c.D);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x73:	/* LD (HL), E */
		mem_write_byte(get_HL(), c.E);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x74:	/* LD (HL), H */
		mem_write_byte(get_HL(), c.H);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x75:	/* LD (HL), L */
		mem_write_byte(get_HL(), c.L);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x76:	/* HALT */
		halted = 1;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x77:	/* LD (HL), A */
		mem_write_byte(get_HL(), c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x78:	/* LD A, B */
		c.A = c.B;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x79:	/* LD A, C */
		c.A = c.C;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x7A:	/* LD A, D */
		c.A = c.D;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x7B:	/* LD A, E */
		c.A = c.E;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x7C:	/* LD A, H */
		c.A = c.H;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x7D:	/* LD A, L */
		c.A = c.L;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x7E:	/* LD A, (HL) */
		c.A = mem_get_byte(get_HL());
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x7F:	/* LD A, A */
		c.A = c.A;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x80:	/* ADD B */
		i = c.A + c.B;
		set_H((c.A & 0xF) + (c.B & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x81:	/* ADD C */
		i = c.A + c.C;
		set_H((c.A & 0xF) + (c.C & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x82:	/* ADD D */
		i = c.A + c.D;
		set_H((c.A & 0xF) + (c.D & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x83:	/* ADD E */
		i = c.A + c.E;
		set_H((c.A & 0xF) + (c.E & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x84:	/* ADD H */
		i = c.A + c.H;
		set_H((c.A & 0xF) + (c.H & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x85:	/* ADD L */
		i = c.A + c.L;
		set_H((c.A & 0xF) + (c.L & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x86:	/* ADD (HL) */
		i = c.A + mem_get_byte(get_HL());
		set_H((i & 0xF) < (c.A & 0xF));
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x87:	/* ADD A */
		i = c.A + c.A;
		set_H((c.A & 0xF) + (c.A & 0xF) > 0xF);
		set_C(i > 0xFF);
		set_N(0);
		c.A = i;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x88:	/* ADC B */
		i = c.A + c.B + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.B & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.B + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x89:	/* ADC C */
		i = c.A + c.C + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.C & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.C + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x8A:	/* ADC D */
		i = c.A + c.D + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.D & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.D + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x8B:	/* ADC E */
		i = c.A + c.E + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.E & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.E + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x8C:	/* ADC H */
		i = c.A + c.H + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.H & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.H + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x8D:	/* ADC L */
		i = c.A + c.L + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.L & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.L + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x8E:	/* ADC (HL) */
		t = mem_get_byte(get_HL());
		i = c.A + t + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (t & 0xF) + flag_C) >= 0x10);
		c.A = c.A + t + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x8F:	/* ADC A */
		i = c.A + c.A + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (c.A & 0xF) + flag_C) >= 0x10);
		c.A = c.A + c.A + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x90:	/* SUB B */
		set_C((c.A - c.B) < 0);
		set_H(((c.A - c.B) & 0xF) > (c.A & 0xF));
		c.A -= c.B;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x91:	/* SUB C */
		set_C((c.A - c.C) < 0);
		set_H(((c.A - c.C) & 0xF) > (c.A & 0xF));
		c.A -= c.C;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x92:	/* SUB D */
		set_C((c.A - c.D) < 0);
		set_H(((c.A - c.D) & 0xF) > (c.A & 0xF));
		c.A -= c.D;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x93:	/* SUB E */
		set_C((c.A - c.E) < 0);
		set_H(((c.A - c.E) & 0xF) > (c.A & 0xF));
		c.A -= c.E;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x94:	/* SUB H */
		set_C((c.A - c.H) < 0);
		set_H(((c.A - c.H) & 0xF) > (c.A & 0xF));
		c.A -= c.H;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x95:	/* SUB L */
		set_C((c.A - c.L) < 0);
		set_H(((c.A - c.L) & 0xF) > (c.A & 0xF));
		c.A -= c.L;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x96:	/* SUB (HL) */
		t = mem_get_byte(get_HL());
		set_C((c.A - t) < 0);
		set_H(((c.A - t) & 0xF) > (c.A & 0xF));
		c.A -= t;
		set_Z(!c.A);
		set_N(1);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x97:	/* SUB A */
		set_C(0);
		set_H(0);
		c.A = 0;
		set_Z(1);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x98:	/* SBC B */
		t = flag_C + c.B;
		set_H(((c.A & 0xF) - (c.B & 0xF) - flag_C) < 0);
		set_C((c.A - c.B - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x99:	/* SBC C */
		t = flag_C + c.C;
		set_H(((c.A & 0xF) - (c.C & 0xF) - flag_C) < 0);
		set_C((c.A - c.C - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x9A:	/* SBC D */
		t = flag_C + c.D;
		set_H(((c.A & 0xF) - (c.D & 0xF) - flag_C) < 0);
		set_C((c.A - c.D - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x9B:	/* SBC E */
		t = flag_C + c.E;
		set_H(((c.A & 0xF) - (c.E & 0xF) - flag_C) < 0);
		set_C((c.A - c.E - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x9C:	/* SBC H */
		t = flag_C + c.H;
		set_H(((c.A & 0xF) - (c.H & 0xF) - flag_C) < 0);
		set_C((c.A - c.H - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x9D:	/* SBC L */
		t = flag_C + c.L;
		set_H(((c.A & 0xF) - (c.L & 0xF) - flag_C) < 0);
		set_C((c.A - c.L - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0x9E:	/* SBC (HL) */
		t = mem_get_byte(get_HL());
		b = flag_C + t;
		set_H(((c.A & 0xF) - (t & 0xF) - flag_C) < 0);
		set_C((c.A - t - flag_C) < 0);
		set_N(1);
		c.A -= b;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0x9F:	/* SBC A */
		t = flag_C + c.A;
		set_H(((c.A & 0xF) - (c.A & 0xF) - flag_C) < 0);
		set_C((c.A - c.A - flag_C) < 0);
		set_N(1);
		c.A -= t;
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA0:	/* AND B */
		c.A &= c.B;
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA1:	/* AND C */
		c.A &= c.C;
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA2:	/* AND D */
		c.A &= c.D;
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA3:	/* AND E */
		c.A &= c.E;
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA4:	/* AND H */
		c.A &= c.H;
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA5:	/* AND L */
		c.A &= c.L;
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA6:	/* AND (HL) */
		c.A &= mem_get_byte(get_HL());
		set_Z(!c.A);
		set_H(1);
		set_N(0);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA7:	/* AND A */
		set_H(1);
		set_N(0);
		set_C(0);
		set_Z(!c.A);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA8:	/* XOR B */
		c.A ^= c.B;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xA9:	/* XOR C */
		c.A ^= c.C;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xAA:	/* XOR D */
		c.A ^= c.D;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xAB:	/* XOR E */
		c.A ^= c.E;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xAC:	/* XOR H */
		c.A ^= c.H;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xAD:	/* XOR L */
		c.A ^= c.L;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xAE:	/* XOR (HL) */
		c.A ^= mem_get_byte(get_HL());
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xAF:	/* XOR A */
		c.A = 0;
		c.F = 0x80;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB0:	/* OR B */
		c.A |= c.B;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB1:	/* OR C */
		c.A |= c.C;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB2:	/* OR D */
		c.A |= c.D;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB3:	/* OR E */
		c.A |= c.E;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB4:	/* OR H */
		c.A |= c.H;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB5:	/* OR L */
		c.A |= c.L;
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB6:	/* OR (HL) */
		c.A |= mem_get_byte(get_HL());
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0xB7:	/* OR A */
		c.F = (!c.A) << 7;
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB8:	/* CP B */
		set_C((c.A - c.B) < 0);
		set_H(((c.A - c.B) & 0xF) > (c.A & 0xF));
		set_Z(c.A == c.B);
		set_N(1);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xB9:	/* CP C */
		set_Z(c.A == c.C);
		set_H(((c.A - c.C) & 0xF) > (c.A & 0xF));
		set_N(1);
		set_C((c.A - c.C) < 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xBA:	/* CP D */
		set_Z(c.A == c.D);
		set_H(((c.A - c.D) & 0xF) > (c.A & 0xF));
		set_N(1);
		set_C((c.A - c.D) < 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xBB:	/* CP E */
		set_Z(c.A == c.E);
		set_H(((c.A - c.E) & 0xF) > (c.A & 0xF));
		set_N(1);
		set_C((c.A - c.E) < 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xBC:	/* CP H */
		set_Z(c.A == c.H);
		set_H(((c.A - c.H) & 0xF) > (c.A & 0xF));
		set_N(1);
		set_C((c.A - c.H) < 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xBD:	/* CP L */
		set_Z(c.A == c.L);
		set_H(((c.A - c.L) & 0xF) > (c.A & 0xF));
		set_N(1);
		set_C((c.A - c.L) < 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xBE:	/* CP (HL) */
		t = mem_get_byte(get_HL());
		set_Z(c.A == t);
		set_H(((c.A - t) & 0xF) > (c.A & 0xF));
		set_N(1);
		set_C((c.A - t) < 0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xBF:	/* CP A */
		set_Z(1);
		set_H(0);
		set_N(1);
		set_C(0);
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xC0:	/* RET NZ */
		if (!flag_Z)
		{
			c.PC = mem_get_word(c.SP);
			c.SP += 2;
			c.cycles += 3;
		}
		else {
			c.PC += 1;
			c.cycles += 1;
		}
		break;
	case 0xC1:	/* POP BC */
		s = mem_get_word(c.SP);
		set_BC(s);
		c.SP += 2;
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xC2:	/* JP NZ, mem16 */
		if (flag_Z == 0)
		{
			c.PC = mem_get_word(c.PC + 1);
		}
		else {
			c.PC += 3;
		}
		c.cycles += 3;
		break;
	case 0xC3:	/* JP imm16 */
		c.PC = mem_get_word(c.PC + 1);
		c.cycles += 4;
		break;
	case 0xC4:	/* CALL NZ, imm16 */
		if (flag_Z == 0)
		{
			c.SP -= 2;
			mem_write_word(c.SP, c.PC + 3);
			c.PC = mem_get_word(c.PC + 1);
			c.cycles += 6;
		}
		else {
			c.PC += 3;
			c.cycles += 3;
		}
		break;
	case 0xC5:	/* PUSH BC */
		c.SP -= 2;
		mem_write_word(c.SP, get_BC());
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xC6:	/* ADD A, imm8 */
		t = mem_get_byte(c.PC + 1);
		set_C((c.A + t) >= 0x100);
		set_H(((c.A + t) & 0xF) < (c.A & 0xF));
		c.A += t;
		set_N(0);
		set_Z(!c.A);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xC7:	/* RST 00 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0;
		c.cycles += 3;
		break;
	case 0xC8:	/* RET Z */
		if (flag_Z == 1)
		{
			c.PC = mem_get_word(c.SP);
			c.SP += 2;
			c.cycles += 3;
		}
		else {
			c.PC += 1;
			c.cycles += 1;
		}
		break;
	case 0xC9:	/* RET */
		c.PC = mem_get_word(c.SP);
		c.SP += 2;
		c.cycles += 3;
		break;
	case 0xCA:	/* JP z, mem16 */
		if (flag_Z == 1)
		{
			c.PC = mem_get_word(c.PC + 1);
		}
		else {
			c.PC += 3;
		}
		c.cycles += 3;
		break;
	case 0xCB:	/* RLC/RRC/RL/RR/SLA/SRA/SWAP/SRL/BIT/RES/SET */
		decode_CB(mem_get_byte(c.PC + 1));
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xCC:	/* CALL Z, imm16 */
		if (flag_Z == 1)
		{
			c.SP -= 2;
			mem_write_word(c.SP, c.PC + 3);
			c.PC = mem_get_word(c.PC + 1);
			c.cycles += 6;
		}
		else {
			c.PC += 3;
			c.cycles += 3;
		}
		break;
	case 0xCD:	/* call imm16 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 3);
		c.PC = mem_get_word(c.PC + 1);
		c.cycles += 6;
		break;
	case 0xCE:	/* ADC a, imm8 */
		t = mem_get_byte(c.PC + 1);
		i = c.A + t + flag_C >= 0x100;
		set_N(0);
		set_H(((c.A & 0xF) + (t & 0xF) + flag_C) >= 0x10);
		c.A = c.A + t + flag_C;
		set_C(i);
		set_Z(!c.A);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xCF:	/* RST 08 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x0008;
		c.cycles += 4;
		break;
	case 0xD0:	/* RET NC */
		if (flag_C == 0)
		{
			c.PC = mem_get_word(c.SP);
			c.SP += 2;
			c.cycles += 3;
		}
		else {
			c.PC += 1;
			c.cycles += 1;
		}
		break;
	case 0xD1:	/* POP DE */
		s = mem_get_word(c.SP);
		set_DE(s);
		c.SP += 2;
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xD2:	/* JP NC, mem16 */
		if (flag_C == 0)
		{
			c.PC = mem_get_word(c.PC + 1);
		}
		else {
			c.PC += 3;
		}
		c.cycles += 3;
		break;
	case 0xD4:	/* CALL NC, mem16 */
		if (flag_C == 0)
		{
			c.SP -= 2;
			mem_write_word(c.SP, c.PC + 3);
			c.PC = mem_get_word(c.PC + 1);
			c.cycles += 6;
		}
		else {
			c.PC += 3;
			c.cycles += 3;
		}
		break;
	case 0xD5:	/* PUSH DE */
		c.SP -= 2;
		mem_write_word(c.SP, get_DE());
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xD6:	/* SUB A, imm8 */
		t = mem_get_byte(c.PC + 1);
		set_C((c.A - t) < 0);
		set_H(((c.A - t) & 0xF) > (c.A & 0xF));
		c.A -= t;
		set_N(1);
		set_Z(!c.A);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xD7:	/* RST 10 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x0010;
		c.cycles += 4;
		break;
	case 0xD8:	/* RET C */
		if (flag_C == 1)
		{
			c.PC = mem_get_word(c.SP);
			c.SP += 2;
			c.cycles += 3;
		}
		else {
			c.PC += 1;
			c.cycles += 1;
		}
		break;
	case 0xDA:	/* JP C, mem16 */
		if (flag_C)
		{
			c.PC = mem_get_word(c.PC + 1);
		}
		else {
			c.PC += 3;
		}
		c.cycles += 3;
		break;
	case 0xDC:	/* CALL C, mem16 */
		if (flag_C == 1)
		{
			c.SP -= 2;
			mem_write_word(c.SP, c.PC + 3);
			c.PC = mem_get_word(c.PC + 1);
			c.cycles += 6;
		}
		else {
			c.PC += 3;
			c.cycles += 3;
		}
		break;
	case 0xD9:	/* RETI */
		c.PC = mem_get_word(c.SP);
		c.SP += 2;
		c.cycles += 4;
		interrupt_enable();
		break;
	case 0xDE:	/* SBC A, imm8 */
		t = mem_get_byte(c.PC + 1);
		b = flag_C;
		set_H(((t & 0xF) + flag_C) > (c.A & 0xF));
		set_C(t + flag_C > c.A);
		set_N(1);
		c.A -= (b + t);
		set_Z(!c.A);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xDF:	/* RST 18 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x0018;
		c.cycles += 3;
		break;
	case 0xE0:	/* LD (FF00 + imm8), A */
		t = mem_get_byte(c.PC + 1);
		mem_write_byte(0xFF00 + t, c.A);
		c.PC += 2;
		c.cycles += 3;
		break;
	case 0xE1:	/* POP HL */
		i = mem_get_word(c.SP);
		set_HL(i);
		c.SP += 2;
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xE2:	/* LD (FF00 + C), A */
		s = 0xFF00 + c.C;
		mem_write_byte(s, c.A);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0xE5:	/* PUSH HL */
		c.SP -= 2;
		mem_write_word(c.SP, get_HL());
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xE6:	/* AND A, imm8 */
		t = mem_get_byte(c.PC + 1);
		set_N(0);
		set_H(1);
		set_C(0);
		c.A = t & c.A;
		set_Z(!c.A);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xE7:	/* RST 20 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x20;
		c.cycles += 4;
		break;
	case 0xE8:	/* ADD SP, imm8 */
		i = mem_get_byte(c.PC + 1);
		set_Z(0);
		set_N(0);
		set_C(((c.SP + i) & 0xFF) < (c.SP & 0xFF));
		set_H(((c.SP + i) & 0xF) < (c.SP & 0xF));
		c.SP = c.SP + (signed char)i;
		c.PC += 2;
		c.cycles += 4;
		break;
	case 0xE9:	/* JP HL */
		c.PC = get_HL();
		c.cycles += 1;
		break;
	case 0xEA:	/* LD (mem16), a */
		s = mem_get_word(c.PC + 1);
		mem_write_byte(s, c.A);
		c.PC += 3;
		c.cycles += 4;
		break;
	case 0xEE:	/* XOR A, imm8 */
		c.A ^= mem_get_byte(c.PC + 1);
		c.F = (!c.A) << 7;
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xEF:	/* RST 28 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x28;
		c.cycles += 4;
		break;
	case 0xF0:	/* LD A, (FF00 + imm8) */
		t = mem_get_byte(c.PC + 1);
		c.A = mem_get_byte(0xFF00 + t);
		c.PC += 2;
		c.cycles += 3;
		break;
	case 0xF1:	/* POP AF */
		s = mem_get_word(c.SP);
		set_AF(s & 0xFFF0);
		c.SP += 2;
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xF2:	/* LD A, (FF00 + c) */
		c.A = mem_get_byte(0xFF00 + c.C);
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0xF3:	/* DI */
		c.PC += 1;
		c.cycles += 1;
		interrupt_disable();
		break;
	case 0xF5:	/* PUSH AF */
		c.SP -= 2;
		mem_write_word(c.SP, get_AF());
		c.PC += 1;
		c.cycles += 3;
		break;
	case 0xF6:	/* OR A, imm8 */
		c.A |= mem_get_byte(c.PC + 1);
		c.F = (!c.A) << 7;
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xF7:	/* RST 30 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x30;
		c.cycles += 4;
		break;
	case 0xF8:	/* LD HL, SP + imm8 */
		i = mem_get_byte(c.PC + 1);
		set_N(0);
		set_Z(0);
		set_C(((c.SP + i) & 0xFF) < (c.SP & 0xFF));
		set_H(((c.SP + i) & 0xF) < (c.SP & 0xF));
		set_HL(c.SP + (signed char)i);
		c.PC += 2;
		c.cycles += 3;
		break;
	case 0xF9:	/* LD SP, HL */
		c.SP = get_HL();
		c.PC += 1;
		c.cycles += 2;
		break;
	case 0xFA:	/* LD A, (mem16) */
		s = mem_get_word(c.PC + 1);
		c.A = mem_get_byte(s);
		c.PC += 3;
		c.cycles += 4;
		break;
	case 0xFB:	/* EI */
		interrupt_enable();
		// printf("Interrupts enabled, IE: %02x\n", interrupt_get_mask());
		c.PC += 1;
		c.cycles += 1;
		break;
	case 0xFE:	/* CP a, imm8 */
		t = mem_get_byte(c.PC + 1);
		set_Z(c.A == t);
		set_N(1);
		set_H(((c.A - t) & 0xF) > (c.A & 0xF));
		set_C(c.A < t);
		c.PC += 2;
		c.cycles += 2;
		break;
	case 0xFF:	/* RST 38 */
		c.SP -= 2;
		mem_write_word(c.SP, c.PC + 1);
		c.PC = 0x0038;
		c.cycles += 4;
		break;
	default:
		printf("Unhandled opcode %02X at %04X\n", b, c.PC);
		return 0;
		break;
	}

	return 1;
}
