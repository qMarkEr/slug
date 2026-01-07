#include "slug.h"

pthread_mutex_t vib_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t vib_cond = PTHREAD_COND_INITIALIZER;
volatile int vibrate_request = 0;

static inline uint64_t millis()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void *vibe_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&vib_lock);
        while (!vibrate_request)
            pthread_cond_wait(&vib_cond, &vib_lock);

        vibrate_request = 0;
        pthread_mutex_unlock(&vib_lock);

        gpioWrite(VIBRO_PIN, 1);

        // Normalize nanoseconds
        uint64_t end_time = millis() + VIBE_DURATION_MS;

        // loop until end_time reached, extend if new requests
        while (millis() < end_time)
        {
            pthread_mutex_lock(&vib_lock);
            if (vibrate_request)
            {
                vibrate_request = 0;
                end_time = millis() + VIBE_DURATION_MS; // extend vibration
            }
            pthread_mutex_unlock(&vib_lock);

            usleep(10000); // 10 ms sleep
        }

        gpioWrite(VIBRO_PIN, 0);
    }
    return NULL;
}

void vibrate_request_signal() {
    pthread_mutex_lock(&vib_lock);
    vibrate_request = 1;
    pthread_cond_signal(&vib_cond);
    pthread_mutex_unlock(&vib_lock);
}

void setup_vibe() {
    gpioSetMode(VIBRO_PIN, PI_OUTPUT);
    gpioWrite(VIBRO_PIN, 0);
}