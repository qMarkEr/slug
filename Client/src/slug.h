#ifndef SLUGGGG
#define SLUGGGG

#include <stdio.h>
#include <string.h>
#include <pigpio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdint.h>

#define BUTTON_PIN 10
#define VIBRO_PIN 4
#define VIBE_DURATION_MS 200

#define AS5600_ADDR 0x36
#define DEBOUNCE_MS 50
#define WAKE_UP "sudo etherwake BC:0F:F3:D8:72:63"
#define SLEEP_DOWN "curl -X POST http://192.168.0.60:5221/api/control/sleep"
#define TEMPS  "http://192.168.0.60:5221/api/temps"
#define MODE_COUNT 5
#define LINE_SIZE 21

static const char *ICONS[7][4] = {
    {
        " __ ",
        "   |",
        "  / ",
        " |  "
    },  // 7
    {
        " __ ",
        "|  |",
        "|__|",
        " __ "
    },  // ciclop
    {
        "    ",
        "()()",
        " __ ",
        "    "
    },  // ciclop
    {
        "    ",
        "  6 ",
        " 7  ",
        "    "
    },  // sixeven
    {
        " -- ",
        " == ",
        " __ ",
        " == "
    },  // barcode
    {
        "   /",
        "  / ",
        " /  ",
        "/   "
    },  // giga slash
    {
        "(__)",
        "(__)",
        "(__)",
        "(__)"
    },  // spine bone sory 4 my bad england
};

static const char *BIG_DIGIT[10][3] = {
    {
        " _ ",
        "| |",
        "|_|"
    },  // 0
    {
        "   ",
        "  |",
        "  |"
    },  // 1
    {
        " _ ",
        " _|",
        "|_ "
    },  // 2
    {
        " _ ",
        " _|",
        " _|"
    },  // 3
    {
        "   ",
        "|_|",
        "  |"
    },  // 4
    {
        " _ ",
        "|_ ",
        " _|"
    },  // 5
    {
        " _ ",
        "|_ ",
        "|_|"
    },  // 6
    {
        " _ ",
        "  |",
        "  |"
    },  // 7
    {
        " _ ",
        "|_|",
        "|_|"
    },  // 8
    {
        " _ ",
        "|_|",
        " _|"
    }   // 9
};

typedef struct {
    int RS, E;
    int data_pins[4];

    // timing
    double E_PULSE;
    double E_DELAY;
    double CMD_DELAY_SHORT;
    double CMD_DELAY_LONG;

    int WRITE_LINE_WIDTH;
} WEH002004A;

typedef enum {
    TASK_WAKE,
    TASK_SLEEP
} TaskType;

typedef struct {
    int mode;
    WEH002004A* display;
} WorkerArgs;

// digits
void write_temps(int cput, int gput, char out[3][21]);
void write_time(char out[3][LINE_SIZE]);
void print_icons(int set[3], char out[4][LINE_SIZE]);

// winstar WEH2004A
void display_initialize(WEH002004A *display);
void display_write_line(WEH002004A *display, int line, const char *msg);
void display_write_word(WEH002004A *display, int row, int col, const char *msg);
void display_clear(WEH002004A *display);
void display_power(WEH002004A *display, int on);

// wake up / sleep down
void button_watchdog(int gpio, int level, uint32_t tick);
void setup_button(unsigned int pin);

// og hotslug
void screensaver(WEH002004A* display);
int get_cpu_gpu_stats(int *cpu_out, int *gpu_out);

// vibration (vibe)
void *vibe_thread(void *arg);
void vibrate_request_signal();
void setup_vibe();
// gfx mode test
// static inline void gfx_select_x(WEH002004A* display, uint8_t x);
// static inline void gfx_select_bank(WEH002004A* display, uint8_t bank);
// static void gfx_write_col(WEH002004A* display, uint8_t x, uint8_t bank, uint8_t value);
// void gfx_set_pixel(WEH002004A* display, int x, int y, int on);
// void gfx_flush_all(WEH002004A* display);
// void gfx_clear(WEH002004A* display);
// void set_gfx_mode(WEH002004A* display);

#endif