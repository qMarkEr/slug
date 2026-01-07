#include "slug.h"

static void append(char *dest, const char *src) {
    strncat(dest, src, LINE_SIZE - strlen(dest) - 1);
}

void write_digit(char line[3][LINE_SIZE], int num) {
    num = num % 10;
    append(line[0], BIG_DIGIT[num][0]);
    append(line[1], BIG_DIGIT[num][1]);
    append(line[2], BIG_DIGIT[num][2]);
}

void padding(char line[3][LINE_SIZE], int amount) {
    for (size_t i = 0; i < amount; i++)
    {
        append(line[0], " ");
        append(line[1], " ");
        append(line[2], " ");
    }
}

void write_number(char line[3][LINE_SIZE], int num) {
    int digits[3] = {0, 0, 0};
    int i = 2;
    int started = 0;

    // split number into digits (max 3 digits)
    while (num > 0 && i >= 0) {
        digits[i] = num % 10;
        num /= 10;
        i--;
    }

    for (int k = 0; k < 3; k++) {
        int d = digits[k];

        if (d == 0 && !started)
            padding(line, 3);
        else {
            write_digit(line, d);
            started = 1;
        }
    }
}

void write_temps(int cput, int gput, char out[3][LINE_SIZE]) {
    // clear lines
    for (int i = 0; i < 3; i++)
        out[i][0] = '\0';

    write_number(out, cput);
    padding(out, 2);
    write_number(out, gput);
}

void print_icons(int set[3], char out[4][LINE_SIZE]) {
    for (size_t i = 0; i < 4; i++)
    {
        append(out[i], "  ");
        append(out[i], "|");
        append(out[i], ICONS[set[0]][i]);
        append(out[i], "|");
        append(out[i], ICONS[set[1]][i]);
        append(out[i], "|");
        append(out[i], ICONS[set[2]][i]);
        append(out[i], "|");
    }
}

void write_time(char out[3][LINE_SIZE]) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    int hour = tm_info->tm_hour;
    int min  = tm_info->tm_min;

    int h1 = hour / 10;
    int h2 = hour % 10;
    int m1 = min / 10;
    int m2 = min % 10;

    padding(out, 3);

    write_digit(out, h1);
    write_digit(out, h2);

    padding(out, 2);

    write_digit(out, m1);
    write_digit(out, m2);

}