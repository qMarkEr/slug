#include "slug.h"

#define MODE_CMD 0
#define MODE_CHR 1

void display_set4bit(WEH002004A *display, int val, int mode) {
    gpioWrite(display->RS, mode);

    for (size_t i = 0; i < 4; i++)
        gpioWrite(display->data_pins[i], (val >> i) & 1);

    gpioDelay(1); // 1 µs setup

    gpioWrite(display->E, 1);
    gpioDelay(display->E_PULSE * 1e6);
    gpioWrite(display->E, 0);
    gpioDelay(display->E_DELAY * 1e6);
}

void display_set8bit(WEH002004A *display, int value, int mode) {
    int hi = (value >> 4) & 0xF;
    int lo = value & 0xF;

    display_set4bit(display, hi, mode);
    display_set4bit(display, lo, mode);
}

void display_clear(WEH002004A *display) {
    display_set8bit(display, 0x01, MODE_CMD);
    gpioDelay(display->CMD_DELAY_LONG * 1e6);
}

void display_power(WEH002004A *display, int on) {
    int cmd = on ? 0x0C : 0x08;
    display_set8bit(display, cmd, MODE_CMD);
    gpioDelay(display->CMD_DELAY_SHORT * 1e6);
}

void display_initialize(WEH002004A *display) {
    // configure pins
    // int pins[] = { display->RS, display->E};
    gpioSetMode(display->RS, PI_OUTPUT);
    gpioSetMode(display->E, PI_OUTPUT);
    for (int i = 0; i < 4; i++) {
        gpioSetMode(display->data_pins[i], PI_OUTPUT);
        gpioWrite(display->data_pins[i], 0);
    }

    gpioDelay(50 * 1000);  // 50 ms stabilization

    // send five zero nibbles (bus reset)
    for (int i = 0; i < 5; i++)
        display_set4bit(display, 0x0, MODE_CMD);


    // Switch to 4-bit mode
    display_set4bit(display, 0x2, MODE_CMD);
    display_set4bit(display, 0x2, MODE_CMD);
    display_set4bit(display, 0x8, MODE_CMD);  // function set

    gpioDelay(display->CMD_DELAY_SHORT * 1e6);

    display_power(display, 0);
    display_clear(display);

    // entry mode: increment
    display_set8bit(display, 0x06, MODE_CMD);
    gpioDelay(display->CMD_DELAY_SHORT * 1e6);

    display_power(display, 1);
    display_clear(display);
}

void display_move_cursor(WEH002004A *display, int row, int col) {
    int offsets[] = {0x00, 0x40, 0x14, 0x54};
    int pos = 0x80 | (offsets[row] + col);
    display_set8bit(display, pos, MODE_CMD);
}

void display_write_line(WEH002004A *display, int line, const char *msg) {
    char padded[21];
    snprintf(padded, 21, "%-20s", msg);

    display_move_cursor(display, line, 0);

    for (int i = 0; i < display->WRITE_LINE_WIDTH; i++)
        display_set8bit(display, padded[i], MODE_CHR);
}

void display_write_word(WEH002004A *display, int row, int col, const char *msg) {
    display_move_cursor(display, row, col);
    int max = display->WRITE_LINE_WIDTH - col;
    for (int i = 0; msg[i] && i < max; i++) {
        display_set8bit(display, msg[i], MODE_CHR);
    }
}




#define GFX_WIDTH  100
#define GFX_HEIGHT 16

/* Framebuffer organized as [bank][x]:
   bank 0 = rows 0..7  -> one byte per column
   bank 1 = rows 8..15 -> one byte per column
*/
static uint8_t gfx[2][GFX_WIDTH];

/* low-level helpers (provided by you)
   void lcd_cmd(uint8_t cmd);   // RS=0 write
   void lcd_data(uint8_t data); // RS=1 write
   void lcd_wait_busy(void);
*/

static inline void gfx_select_x(WEH002004A* display, uint8_t x)
{
    gpioDelay(display->CMD_DELAY_LONG * 1e6);
    display_set8bit(display, 0x80 | (x & 0x7F), MODE_CMD);   // DDRAM instruction -> GXA
}

static inline void gfx_select_bank(WEH002004A* display, uint8_t bank)
{
    gpioDelay(display->CMD_DELAY_LONG * 1e6);
    display_set8bit(display, 0x40 | (bank & 0x01), MODE_CMD); // CGRAM instruction -> GYA (only DB0 = CGA0)
}

/* write the byte for column x and bank (0 or 1) */
static void gfx_write_col(WEH002004A* display, uint8_t x, uint8_t bank, uint8_t value)
{
    gfx_select_x(display, x);
    gfx_select_bank(display, bank);
    gpioDelay(display->CMD_DELAY_LONG * 1e6);
    display_set8bit(display, value, MODE_CHR);
}

/* set or clear a pixel */
void gfx_set_pixel(WEH002004A* display, int x, int y, int on)
{
    if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT) return;

    uint8_t bank = (y >> 3) & 0x01;      // 0..1
    uint8_t bit  = 1u << (y & 0x07);     // mask D0..D7

    if (on) gfx[bank][x] |= bit;
    else    gfx[bank][x] &= ~bit;

    /* write back only this column/bank (faster than full flush) */
    gfx_write_col(display, (uint8_t)x, bank, gfx[bank][x]);
}

/* flush entire framebuffer to display (useful at init or big updates) */
void gfx_flush_all(WEH002004A* display)
{
    for (uint8_t bank = 0; bank < 2; ++bank) {
        for (uint8_t x = 0; x < GFX_WIDTH; ++x) {
            gfx_write_col(display, x, bank, gfx[bank][x]);
        }
    }
}

/* helper to clear display buffer and device */
void gfx_clear(WEH002004A* display)
{
    memset(gfx, 0, sizeof(gfx));
    gfx_flush_all(display);
}

void set_gfx_mode(WEH002004A* display) {
    display_set8bit(display, 0x1F, MODE_CMD);
}