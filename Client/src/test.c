#include "slug.h"
#include <math.h>
volatile sig_atomic_t done = 0;
void term(int signum) { done = 1; }


int main() {
    // init
    if (gpioInitialise() < 0) {
        printf("pigpio failed.\n");
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
    set_gfx_mode(&display);
    gfx_set_pixel(&display, 5, 14, 1);
    gfx_set_pixel(&display, 4, 14, 1);
    for (size_t x = 0; x < 100; x++)
    {
        float y = sin((float)x / 5.f);
        // printf("%d - %d\n",x, (int)(floorf((y + 1.f) * 8)));
        gfx_set_pixel(&display, x, 7, 1);
        usleep(10000);
    }
    int dum;
    scanf("%d", &dum);
    
    // gfx_set_pixel(&display, 1, 0, 1);
    // gfx_set_pixel(&display, 1, 15, 1);

    // main loop
    // while (!done)
    // {
       
    // }

    // clear
    display_clear(&display);
    // display_write_line(&display, 0, "Не поминайте лихом!");
    sleep(1);
    display_power(&display, 0);
    gpioTerminate();

    return 0;
}