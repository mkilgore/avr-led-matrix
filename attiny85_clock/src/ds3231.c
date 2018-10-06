
#include "common.h"

#include <stdio.h>
#include <string.h>

#include "usi_i2c_master.h"
#include "ds3231.h"

#define DS3231_ADDRESS (0b1101000)

void ds3231_read_state(struct ds3231 *ds3231)
{
    uint8_t buf[sizeof(*ds3231)];

    buf[0] = 0x00;
    usi_twi_write_data(DS3231_ADDRESS, buf, 1);

    usi_twi_read_data(DS3231_ADDRESS, buf, sizeof(buf) / sizeof(*buf));
    memcpy(ds3231, buf, sizeof(*ds3231));

#if 0
    ds3231->sec_fst_digit = buf[0] >> 4;
    ds3231->sec_snd_digit = buf[0] & 0x0F;

    ds3231->min_fst_digit = buf[1] >> 4;
    ds3231->min_snd_digit = buf[1] & 0x0F;

    ds3231->hour_is_12 = !!(buf[2] & 0b01000000);
    ds3231->hour_is_pm = !!(buf[2] & 0b00100000);
    ds3231->hour_fst_digit = (buf[2] >> 4) & 0x01;
    ds3231->hour_snd_digit = buf[2] & 0x0F;

    ds3231->day_of_week = buf[3] & 0x07;

    ds3231->date_fst_digit = buf[4] >> 4;
    ds3231->date_snd_digit = buf[4] & 0x0F;

    ds3231->month_fst_digit = (buf[5] >> 4) & 0x01;
    ds3231->month_snd_digit = (buf[5] & 0x0F);

    ds3231->year_fst_digit = buf[6] >> 4;
    ds3231->year_snd_digit = buf[6] & 0x0F;
#endif
}

void ds3231_write_state(const struct ds3231 *ds3231)
{
    uint8_t buf[sizeof(*ds3231) + 1];

    buf[0] = 0x00;
    memcpy(buf + 1, ds3231, sizeof(*ds3231));

    usi_twi_write_data(DS3231_ADDRESS, buf, sizeof(buf) / sizeof(*buf));
}

