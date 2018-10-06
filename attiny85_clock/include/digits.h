#ifndef INCLUDE_DIGITS_H
#define INCLUDE_DIGITS_H

#include <stdint.h>
#include <avr/pgmspace.h>

/* Digits are 4 bits wide */
extern const PROGMEM uint8_t digits[][8];

struct disp_buf {
    const uint8_t *buf;
    int bitwidth;
    int byte_length;
};

struct disp_loc {
    int bitstart;
    int byte_start;
};

struct disp_matrix {
    uint8_t *buf;
    int width;
};

void digit_write(int digit, uint8_t *buf, int bitstart, int matrix_width);
void digital_write(uint8_t *buf, int matrix_width, const uint8_t *disp_buf, int bitwidth, int bitstart, int byte_start, int byte_length);
void digit_write_small(int digit, uint8_t *buf, int bitstart, int byte_start, int matrix_width);

void digital_write2(struct disp_matrix *matrix, struct disp_buf *, const struct disp_loc *);

#endif
