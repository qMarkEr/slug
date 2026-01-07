#include "slug.h"

unsigned int prev_state = 0;

void button_watchdog(int gpio, int level, uint32_t tick)
{
    if (level == PI_TIMEOUT)
        return;
    if (prev_state != level) {
        prev_state = level;
        if (level == 1) system(WAKE_UP);
        else if (level == 0)
            system(SLEEP_DOWN);
    }
}

void setup_button(unsigned int pin)
{
    gpioSetMode(pin, PI_INPUT);
    gpioGlitchFilter(pin, DEBOUNCE_MS);
    gpioSetPullUpDown(pin, PI_PUD_DOWN);

    // sync initial state
    prev_state = gpioRead(pin);

    gpioSetAlertFunc(pin, button_watchdog);
    gpioSetWatchdog(pin, 5);
}