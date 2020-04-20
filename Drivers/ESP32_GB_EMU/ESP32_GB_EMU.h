#ifndef ESP32_GB_EMU_H
#define ESP32_GB_EMU_H

// Unify all emulator files
////// SDL
int sdl_update(void);
void sdl_init(void);
void flushFrameToLCD(void);
unsigned char* sdl_get_framebuffer(void);
unsigned int sdl_get_buttons(void);
unsigned int sdl_get_directions(void);

////// ROM
int rom_load(const char *);
int rom_init(const unsigned char *);
const unsigned char *rom_getbytes(void);
unsigned int rom_get_mapper(void);

enum {
	NROM,
	MBC1,
	MBC2,
	MMM01,
	MBC3,
	MBC4,
	MBC5,
};

////// MEM
unsigned int mem_init();
unsigned char mem_get_byte(unsigned short);
unsigned short mem_get_word(unsigned short);
void mem_write_byte(unsigned short, unsigned char);
void mem_write_word(unsigned short, unsigned short);
void mem_bank_switch(unsigned int);
unsigned char mem_get_raw(unsigned short);

#define NULL 0

class GBmemory
{
private:
	bool valid;
	unsigned char *ROMbank0, *ROMbank1, *VRAM, *ExternalRAM, *WorkRAM;
	unsigned char IOports[128], HRAM[128], OAM[160];
	unsigned char InterruptEnableRegister[1];
public:
	GBmemory()
	{
		ROMbank0 = NULL; // 16KB
		ROMbank1 = NULL; // 16KB
		VRAM = NULL;	// 8KB
		ExternalRAM = NULL; // 8KB
		WorkRAM = NULL; // 
		InterruptEnableRegister[0] = 0xff;
		valid = false;
	}
	~GBmemory()
	{
		if (!ROMbank0)
		{
			delete ROMbank0;
		}
		if (!ROMbank1)
		{
			delete ROMbank1;
		}
		if (!VRAM)
		{
			delete VRAM;
		}
		if (!ExternalRAM)
		{
			delete ExternalRAM;
		}
		if (!WorkRAM)
		{
			delete WorkRAM;
		}
	}
	bool init();
	unsigned char& operator[](unsigned short const address);
};

////// MBC
unsigned int MBC1_write_byte(unsigned short, unsigned char);
unsigned int MBC3_write_byte(unsigned short, unsigned char);

////// LCD
int lcd_cycle(void);
int lcd_get_line(void);
unsigned char lcd_get_stat();
void lcd_write_control(unsigned char);
void lcd_write_stat(unsigned char);
void lcd_write_scroll_x(unsigned char);
void lcd_write_scroll_y(unsigned char);
void lcd_write_bg_palette(unsigned char);
void lcd_write_spr_palette1(unsigned char);
void lcd_write_spr_palette2(unsigned char);
void lcd_set_window_y(unsigned char);
void lcd_set_window_x(unsigned char);
void lcd_set_ly_compare(unsigned char);

////// TIMER
void timer_set_tac(unsigned char);
void timer_cycle(void);
unsigned char timer_get_div(void);
unsigned char timer_get_counter(void);
unsigned char timer_get_modulo(void);
unsigned char timer_get_tac(void);
void timer_set_div(unsigned char);
void timer_set_counter(unsigned char);
void timer_set_modulo(unsigned char);

////// INTERRUPT
void interrupt(unsigned int);
void interrupt_disable(void);
void interrupt_enable(void);
unsigned char interrupt_get_IF(void);
void interrupt_set_IF(unsigned char);
unsigned char interrupt_get_mask(void);
void interrupt_set_mask(unsigned char);
int interrupt_flush(void);

enum {
	INTR_VBLANK = 0x01,
	INTR_LCDSTAT = 0x02,
	INTR_TIMER = 0x04,
	INTR_SERIAL = 0x08,
	INTR_JOYPAD = 0x10
};

////// CPU
void cpu_init(void);
int cpu_cycle(void);
unsigned int cpu_get_cycles(void);
void cpu_interrupt(unsigned short);

#endif