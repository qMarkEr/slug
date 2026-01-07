#include "slug.h"

struct buffer {
    char *data;
    size_t size;
};

char get_random_char(void)
{
    int i = rand() % 8 + 8;
    int j = rand() % 16;
    int byte_value = (i << 4) | j;
    return (char)byte_value;
}

void screensaver(WEH002004A* display)
{
    display_clear(display);
    for (int k = 0; k < 7; k++)
    {
        int word_len = (rand() % 6) + 4;  // 4–9 chars

        char word[16];
        for (int n = 0; n < word_len; n++)
            word[n] = get_random_char();
        word[word_len] = 0;

        int row = rand() % 4;        // 0–3
        int col = (rand() % 17) + 4; // 4–20

        display_write_word(display, row, col, word);
    }
}

static size_t curl_write(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total = size * nmemb;
    struct buffer *buf = userdata;

    char *new_ptr = realloc(buf->data, buf->size + total + 1);
    if (!new_ptr) return 0;

    buf->data = new_ptr;
    memcpy(buf->data + buf->size, ptr, total);
    buf->size += total;
    buf->data[buf->size] = 0;

    return total;
}

int get_cpu_gpu_stats(int *cpu_out, int *gpu_out)
{
    CURL *curl = curl_easy_init();
    if (!curl) return 0;

    struct buffer buf = {0};

    curl_easy_setopt(curl, CURLOPT_URL, TEMPS);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        free(buf.data);
        return 0;
    }

    // ---- Parse JSON ----
    cJSON *json = cJSON_Parse(buf.data);
    free(buf.data);

    if (!json) return 0;

    cJSON *cpu = cJSON_GetObjectItem(json, "cpu");
    cJSON *gpu = cJSON_GetObjectItem(json, "gpu");

    if (!cJSON_IsNumber(cpu) || !cJSON_IsNumber(gpu)) {
        cJSON_Delete(json);
        return 0;
    }

    *cpu_out = cpu->valueint;
    *gpu_out = gpu->valueint;

    cJSON_Delete(json);
    return 1;
}
