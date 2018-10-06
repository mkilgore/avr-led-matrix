
#include "common.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <string.h>

#include "usi_i2c_master.h"
#include "serial.h"
#include "ds3231.h"
#include "digits.h"
#include "font_info.h"
#include "animation.h"

#define I2C_ADDRESS 0x2A

#define ROR(S,i) (((S)>>(i)) | ((S)<<(8-(i))))
#define ROL(S,i) (((S)<<(i)) | ((S)>>(8-(i))))

static uint8_t buf[34] = {
    0x00,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0x01,
};

static struct ds3231 ds3231, old_ds3231;

static void ds3231_disp_time(struct disp_matrix *matrix, const struct ds3231 *old, const struct ds3231 *new, int frame)
{
    struct disp_buf old_buf, new_buf;
    struct disp_loc cur_loc;

    old_buf.buf = hour_digit[old->hour_fst_digit];
    old_buf.bitwidth = 3;
    old_buf.byte_length = 8;

    cur_loc.bitstart = 0;
    cur_loc.byte_start = 0;

    if (new->hour_fst_digit == old->hour_fst_digit) {
        digital_write2(matrix, &old_buf, &cur_loc);
    } else {
        new_buf.buf = hour_digit[new->hour_fst_digit];
        new_buf.bitwidth = 3;
        new_buf.byte_length = 8;

        animate_frame(matrix, &old_buf, &new_buf, &cur_loc, frame);
    }

    animate_digit_switch(matrix, old->hour_snd_digit, new->hour_snd_digit, &(const struct disp_loc) {
            .bitstart = 4,
            .byte_start = 0,
            }, frame);

    digital_write(buf + 1, 4, colon[(frame % 10) > 5], 2, 10, 0, 8);

    animate_digit_switch(matrix, old->min_fst_digit, new->min_fst_digit, &(const struct disp_loc) {
            .bitstart = 12,
            .byte_start = 0,
            }, frame);

    animate_digit_switch(matrix, old->min_snd_digit, new->min_snd_digit, &(const struct disp_loc) {
            .bitstart = 18,
            .byte_start = 0,
            }, frame);

    old_buf.buf = am_pm[old->hour_is_pm];
    old_buf.bitwidth = 8;
    old_buf.byte_length = 8;

    cur_loc.bitstart = 24;
    cur_loc.byte_start = 0;

    if (old->hour_is_pm == new->hour_is_pm) {
        digital_write2(matrix, &old_buf, &cur_loc);
    } else {
        new_buf.buf = am_pm[new->hour_is_pm];
        new_buf.bitwidth = 8;
        new_buf.byte_length = 8;

        animate_frame(matrix, &old_buf, &new_buf, &cur_loc, frame);
    }
}

int main(void)
{
    DDRB |= _BV(DDB3);
    DDRB &= ~_BV(DDB4);
    DDRB |= _BV(DDB1);

    serial_init();
    usi_twi_master_initialize();

    sei();

    memset(&ds3231, 0, sizeof(ds3231));

    ds3231_read_state(&ds3231);

    struct disp_matrix matrix = {
        .buf = buf + 1,
        .width = 4,
    };

    struct ds3231 sav;
    int frame = 0;

    while (1) {
        while (PINB & _BV(PINB4))
            ;

        ds3231_disp_time(&matrix, &old_ds3231, &ds3231, frame);

        if (frame == 120) {
            frame = 21;
        } else {
            frame++;
        }

        usi_twi_write_data(I2C_ADDRESS, buf, sizeof(buf) / sizeof(*buf));

        _delay_ms(100);

        sav = ds3231;
        ds3231_read_state(&ds3231);

        if (ds3231.min_snd_digit != sav.min_snd_digit) {
            old_ds3231 = sav;
            frame = 0;
        }
    }

    return 0;
}

