#ifndef SNES_CLASSIC_H
#define SNES_CLASSIC_H

struct snes_classic_state {
    uint8_t up_pressed :1;
    uint8_t down_pressed :1;
    uint8_t right_pressed :1;
    uint8_t left_pressed :1;
    uint8_t start_pressed :1;
    uint8_t select_pressed :1;

    uint8_t a_pressed :1;
    uint8_t b_pressed :1;
    uint8_t x_pressed :1;
    uint8_t y_pressed :1;
    uint8_t l_pressed :1;
    uint8_t r_pressed :1;
    uint8_t home_pressed :1;

    uint8_t zr_pressed :1;
    uint8_t zl_pressed :1;
};

void snes_classic_init(void);
void snes_classic_read_state(struct snes_classic_state *);

#endif
