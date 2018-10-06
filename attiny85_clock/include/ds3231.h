#ifndef INCLUDE_DS3231_H
#define INCLUDE_DS3231_H

struct ds3231 {
    uint8_t sec_snd_digit :4;
    uint8_t sec_fst_digit :4;

    uint8_t min_snd_digit :4;
    uint8_t min_fst_digit :4;

    uint8_t hour_snd_digit :4;
    uint8_t hour_fst_digit :1;
    uint8_t hour_is_pm :1;
    uint8_t hour_is_12 :1;
    uint8_t pad1 :1;


    uint8_t day_of_week :3;
    uint8_t pad2 :5;

    uint8_t date_snd_digit :4;
    uint8_t date_fst_digit :2;
    uint8_t pad3 :2;

    uint8_t month_snd_digit :4;
    uint8_t month_fst_digit :1;
    uint8_t pad4 :2;
    uint8_t century_flag :1;

    uint8_t year_snd_digit :4;
    uint8_t year_fst_digit :4;

    /* Alarm stuff is below */
};

void ds3231_read_state(struct ds3231 *);
void ds3231_write_state(const struct ds3231 *);

#endif
