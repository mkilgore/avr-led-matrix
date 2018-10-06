
#include "common.h"

#include <string.h>

#include "digits.h"
#include "animation.h"

static void scroll_frame(struct disp_matrix *matrix, struct disp_buf *start, struct disp_buf *end, const struct disp_loc *loc, int frame)
{
    if (frame < 13) {
        struct disp_loc first_loc = {
            .bitstart = loc->bitstart,
            .byte_start = loc->byte_start - frame,
        };

        struct disp_loc second_loc = {
            .bitstart = loc->bitstart,
            .byte_start = loc->byte_start + 12 - frame,
        };

        digital_write2(matrix, start, &first_loc);
        digital_write2(matrix, end, &second_loc);
    } else {
        digital_write2(matrix, end, loc);
    }
}

void animate_frame(struct disp_matrix *matrix, struct disp_buf *start, struct disp_buf *end, const struct disp_loc *loc, int frame)
{
    uint8_t blank[start->byte_length];
    memset(blank, 0, sizeof(blank));
    struct disp_buf blank_buf = {
        .buf = blank,
        .bitwidth = start->bitwidth,
        .byte_length = start->byte_length,
    };

    /* First, clear out the animation space */
    digital_write2(matrix, &blank_buf, loc);

    scroll_frame(matrix, start, end, loc, frame);
}

void animate_digit_switch(struct disp_matrix *matrix, int old_digit, int new_digit, const struct disp_loc *loc, int frame)
{
    uint8_t old_digit_buf[8], new_digit_buf[8];

    old_digit_buf[0] = pgm_read_byte(&digits[old_digit][0]);
    old_digit_buf[1] = pgm_read_byte(&digits[old_digit][1]);
    old_digit_buf[2] = pgm_read_byte(&digits[old_digit][2]);
    old_digit_buf[3] = pgm_read_byte(&digits[old_digit][3]);
    old_digit_buf[4] = pgm_read_byte(&digits[old_digit][4]);
    old_digit_buf[5] = pgm_read_byte(&digits[old_digit][5]);
    old_digit_buf[6] = pgm_read_byte(&digits[old_digit][6]);
    old_digit_buf[7] = pgm_read_byte(&digits[old_digit][7]);

    struct disp_buf old_buf = {
        .buf = old_digit_buf,
        .bitwidth = 5,
        .byte_length = 8,
    };

    if (old_digit == new_digit) {
        digital_write2(matrix, &old_buf, loc);
        return ;
    }

    new_digit_buf[0] = pgm_read_byte(&digits[new_digit][0]);
    new_digit_buf[1] = pgm_read_byte(&digits[new_digit][1]);
    new_digit_buf[2] = pgm_read_byte(&digits[new_digit][2]);
    new_digit_buf[3] = pgm_read_byte(&digits[new_digit][3]);
    new_digit_buf[4] = pgm_read_byte(&digits[new_digit][4]);
    new_digit_buf[5] = pgm_read_byte(&digits[new_digit][5]);
    new_digit_buf[6] = pgm_read_byte(&digits[new_digit][6]);
    new_digit_buf[7] = pgm_read_byte(&digits[new_digit][7]);

    struct disp_buf new_buf = {
        .buf = new_digit_buf,
        .bitwidth = 5,
        .byte_length = 8,
    };

    animate_frame(matrix, &old_buf, &new_buf, loc, frame);
}
