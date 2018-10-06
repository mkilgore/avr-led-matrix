
#include "common.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>

#include "font_info.h"
#include "digits.h"

void digit_write(int digit, uint8_t *buf, int bitstart, int matrix_width)
{
    uint8_t digit_cpy[8];

    digit_cpy[0] = pgm_read_byte(&digits[digit][0]);
    digit_cpy[1] = pgm_read_byte(&digits[digit][1]);
    digit_cpy[2] = pgm_read_byte(&digits[digit][2]);
    digit_cpy[3] = pgm_read_byte(&digits[digit][3]);
    digit_cpy[4] = pgm_read_byte(&digits[digit][4]);
    digit_cpy[5] = pgm_read_byte(&digits[digit][5]);
    digit_cpy[6] = pgm_read_byte(&digits[digit][6]);
    digit_cpy[7] = pgm_read_byte(&digits[digit][7]);

    digital_write(buf, matrix_width, digit_cpy, 5, bitstart, 0, 8);
}

void digit_write_small(int digit, uint8_t *buf, int bitstart, int byte_start, int matrix_width)
{
    uint8_t digit_cpy[5];

    digit_cpy[0] = pgm_read_byte(&digits_small[digit][0]);
    digit_cpy[1] = pgm_read_byte(&digits_small[digit][1]);
    digit_cpy[2] = pgm_read_byte(&digits_small[digit][2]);
    digit_cpy[3] = pgm_read_byte(&digits_small[digit][3]);
    digit_cpy[4] = pgm_read_byte(&digits_small[digit][4]);

    digital_write(buf, matrix_width, digit_cpy, 3, bitstart, byte_start, 5);
}

void digital_write(uint8_t *buf, int matrix_width, const uint8_t *disp_buf, int bitwidth, int bitstart, int byte_start, int byte_length)
{
    uint8_t i;
    int bit_offset = 0;

    if (byte_start < 0) {
        byte_length += byte_start;
        disp_buf -= byte_start;
        byte_start = 0;
    }

    if (byte_start + byte_length > 8)
        byte_length = 8 - byte_start;

    if (bitstart + bitwidth > matrix_width * 8)
        bitwidth = matrix_width * 8 - bitstart;

    uint8_t cpy[8];
    if (byte_length > 0)
        memcpy(cpy, disp_buf, byte_length);

    if (bitstart < 0) {
        bit_offset = -bitstart;
        bitstart = 0;

        int i = 0;
        for (; i < byte_length; i++)
            cpy[i] >>= bit_offset;
    }

    for (i = 0; i < bitwidth; i++) {
        int byte_offset = (bitstart + i) >> 3;
        int bit_offset = (bitstart + i) % 8;

        int k;
        for (k = byte_start; k < byte_start + byte_length; k++) {
            uint8_t *byte = buf + byte_offset + k * matrix_width;
            int bit = !!(cpy[k - byte_start] & (1 << (bitwidth - 1 - i)));

            *byte &= ~(1 << bit_offset);
            *byte |= bit << bit_offset;
        }
    }
}

void digital_write2(struct disp_matrix *matrix, struct disp_buf *disp, const struct disp_loc *loc)
{
    digital_write(matrix->buf, matrix->width, disp->buf, disp->bitwidth, loc->bitstart, loc->byte_start, disp->byte_length);
}

