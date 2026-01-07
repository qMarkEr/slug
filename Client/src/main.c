#include "slug.h"
#include <stdint.h>

static uint64_t state = 0x853c49e6748fea9bULL;
static uint64_t inc   = 0xda3e39cb94b95bdbULL;

uint32_t pcg32(void) {
    uint64_t oldstate = state;
    state = oldstate * 6364136223846793005ULL + (inc | 1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

int rand_range(int min, int max) {
    return min + (int)(pcg32() % (max - min + 1));
}
// signal handle
volatile sig_atomic_t done = 0;
void term(int signum) { done = 1; }

// mutex
volatile int input_flag = 1;
pthread_mutex_t display_lock = PTHREAD_MUTEX_INITIALIZER;

// utils
int read_angle(int h) {
    int lo = i2cReadByteData(h, 0x0F);
    int hi = i2cReadByteData(h, 0x0E) << 8;
    return  hi | lo;
}

int clamp(int val, int rest) { return (rest + val) % rest; }

void interruptable_sleep(int seconds) {
    for (int i = 0; i < (seconds * 10) && !input_flag; i++)
        usleep(100000);
}

void *mode_thread(void *arg)
{
    WorkerArgs* args = (WorkerArgs*)arg;
    int mode = args->mode;
    WEH002004A* display = args->display;
    free(args);

    pthread_mutex_lock(&display_lock);
    display_clear(display);
    switch (mode)
    {
        case 0: // temps
            while (!input_flag)
            {
                int cpu, gpu;
                char lines[3][LINE_SIZE] = {};
                int code = get_cpu_gpu_stats(&cpu, &gpu);
                if (code && gpioRead(BUTTON_PIN)) {
                    display_write_line(display, 0, "cpu\xDF""c   slug   gpu\xDF""c");
                    write_temps(cpu, gpu, lines);
                    for (size_t i = 0; i < 3; i++)
                        display_write_line(display, i + 1, lines[i]);
                    interruptable_sleep(1);
                        
                } else {
                    int i = 0;
                    while (i < 60 && !input_flag && !gpioRead(BUTTON_PIN)) {
                        screensaver(display);
                        interruptable_sleep(5);
                        i += 5;
                    }
                }
            }
            break;
        case 1: // time
            while (!input_flag)
            {
                char lines[3][LINE_SIZE] = {};
                display_write_line(display, 0, "time     to     goon");
                write_time(lines);
                for (size_t i = 0; i < 3; i++)
                    display_write_line(display, i + 1, lines[i]);
                interruptable_sleep(60);
            }
            break;
        case 2: // display off
            display_clear(display);
            break;
        case 3: // screensaver
            uint8_t x = 0;
            uint8_t y = 0;
            uint8_t x_dir = 1;
            uint8_t y_dir = 1;
            while (!input_flag)
            {
                display_write_word(display, y, x, "    ");
                
                if (x == 0) x_dir = 1;
                else if (x > 14) x_dir = -1;
                if (y == 0) y_dir = 1;
                else if (y == 3) y_dir = -1;
                
                x += x_dir * 2;
                y += y_dir;

                display_write_word(display, y, x, "slug");
                interruptable_sleep(1);
            }
            break;
        case 4:
            int comb[3] = {
                rand_range(0, 6),
                rand_range(0, 6),
                rand_range(0, 6)
            };
            char lines[4][LINE_SIZE] = {};
            print_icons(comb, lines);
            if (comb[0] == comb[1] && comb[1]  == comb[2]) {
                for (size_t i = 0; i < 4; i++)
                    display_write_line(display, i, lines[i]);
                vibrate_request_signal();
                usleep(500000);
                display_clear(display);
                usleep(500000);
                for (size_t i = 0; i < 4; i++)
                    display_write_line(display, i, lines[i]);
                vibrate_request_signal();
                usleep(500000);
                display_clear(display);
                usleep(500000);
                vibrate_request_signal();
            }
            for (size_t i = 0; i < 4; i++)
                display_write_line(display, i, lines[i]);
            break;
        default:
            break;
    }
    pthread_mutex_unlock(&display_lock);
    return NULL;
}


int main() {
    // init
    if (gpioInitialise() < 0) {
        printf("pigpio failed.\n");
        return 1;
    }
    
    int h = i2cOpen(1, AS5600_ADDR, 0);
    if (h < 0)
    {
        printf("Cannot open I2C device\n");
        return 1;
    }

    struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTSTP, &action, NULL);

    WEH002004A display = {
        .RS = 14, .E = 15,
        .data_pins = {18, 23, 24, 25},

        .E_PULSE = 2e-6,
        .E_DELAY = 2e-6,
        .CMD_DELAY_SHORT = 0.0001,
        .CMD_DELAY_LONG = 0.002,
        .WRITE_LINE_WIDTH = 20
    };

    display_initialize(&display);
    setup_button(BUTTON_PIN);
    setup_vibe();
    pthread_t tid;
    pthread_create(&tid, NULL, vibe_thread, NULL);

    int mode = 1;
    int inc = 1;
    uint16_t prev_angle = read_angle(h);
    uint16_t abs_pos = prev_angle;
    char preview[7];
    const char* modes[] = {"slug", "goon", "offw", "scsv", "gamb"};

    // main loop
    while (!done)
    {
        uint16_t angle = read_angle(h);
        int movement = angle - prev_angle;

        if (abs(movement) > 16) {
            if (!input_flag)
            {
                display_clear(&display);
                snprintf(preview, sizeof(preview), "-%s-", modes[mode]);
                display_write_word(&display, 0, 8, modes[clamp(mode + 1, MODE_COUNT)]);
                display_write_word(&display, 1, 7, preview);
                display_write_word(&display, 2, 8, modes[clamp(mode - 1, MODE_COUNT)]);
            }
            input_flag = 1;

            int direction = movement > 0;
            uint16_t delta = clamp(angle - abs_pos, 4096);
            if (abs(movement) >= 2048) direction = !direction;

            if ((delta / 512) % 2 == 1) inc = 0;
            if ((delta / 512) % 2 == 0 && !inc) {
                inc = 1;
                vibrate_request_signal();
                mode = clamp(mode + (direction ? 1 : -1), MODE_COUNT);
                snprintf(preview, sizeof(preview), "-%s-", modes[mode]);
                display_write_word(&display, 0, 8, modes[clamp(mode + 1, MODE_COUNT)]);
                display_write_word(&display, 1, 7, preview);
                display_write_word(&display, 2, 8, modes[clamp(mode - 1, MODE_COUNT)]);
            }

            prev_angle = angle;
        } else {
            if (input_flag) {
                inc = 1;
                WorkerArgs *args = malloc(sizeof(WorkerArgs));
                args->mode = clamp(mode, MODE_COUNT);
                args->display = &display;
                pthread_t mode_thrd;
                pthread_create(&mode_thrd, NULL, mode_thread, args);
                pthread_detach(mode_thrd);
            }
            input_flag = 0;
            abs_pos = prev_angle;
        }
        usleep(100000);
    }

    // clear
    display_clear(&display);
    display_write_line(&display, 0, "Не поминайте лихом!");
    sleep(1);
    display_power(&display, 0);
    i2cClose(h);
    gpioTerminate();

    return 0;
}