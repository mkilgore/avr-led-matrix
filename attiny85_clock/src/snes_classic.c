
#include "common.h"
#include "usi_i2c_master.h"

#include <stdio.h>
#include <string.h>

#include "snes_classic.h"

#define WIIMOTE_EXTENSION_ADDRESS 0x52

void snes_classic_init(void)
{
    unsigned char buf[8];

    buf[0] = 0xF0;
    buf[1] = 0x55;

    usi_twi_write_data(WIIMOTE_EXTENSION_ADDRESS, buf, 2);

    buf[0] = 0xFB;
    buf[1] = 0x00;

    usi_twi_write_data(WIIMOTE_EXTENSION_ADDRESS, buf, 2);

    buf[0] = 0xFA;

    usi_twi_write_data(WIIMOTE_EXTENSION_ADDRESS, buf, 1);

    /* SNES Classic controller needs a small delay before responding to reads */
    _delay_ms(1);

    usi_twi_read_data(WIIMOTE_EXTENSION_ADDRESS, buf, 6);

    if (buf[0] == 0x01
        && buf[1] == 0x00
        && buf[2] == 0xA4
        && buf[3] == 0x20
        && buf[4] == 0x00
        && buf[5] == 0x01)
    {
        // printf("Correct device ID!!!!!!\n");
    } else {
        // printf("Incorrect device ID: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n",
        //         buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    }
}

void snes_classic_read_state(struct snes_classic_state *state)
{
    memset(state, 0, sizeof(*state));
    unsigned char buf[6];

    buf[0] = 0x00;

    _delay_ms(1);

    usi_twi_write_data(WIIMOTE_EXTENSION_ADDRESS, buf, 1);

    /* SNES Classic controller needs a small delay before responding to reads */
    _delay_ms(1);

    usi_twi_read_data(WIIMOTE_EXTENSION_ADDRESS, buf, 6);

    unsigned char low_buttons = buf[4];
    unsigned char high_buttons = buf[5];

    state->r_pressed      = !(low_buttons & (1 << 1));
    state->start_pressed  = !(low_buttons & (1 << 2));
    state->home_pressed   = !(low_buttons & (1 << 3));
    state->select_pressed = !(low_buttons & (1 << 4));
    state->l_pressed      = !(low_buttons & (1 << 5));
    state->down_pressed   = !(low_buttons & (1 << 6));
    state->right_pressed  = !(low_buttons & (1 << 7));

    state->up_pressed   = !(high_buttons & (1 << 0));
    state->left_pressed = !(high_buttons & (1 << 1));
    state->zr_pressed   = !(high_buttons & (1 << 2));
    state->x_pressed    = !(high_buttons & (1 << 3));
    state->a_pressed    = !(high_buttons & (1 << 4));
    state->y_pressed    = !(high_buttons & (1 << 5));
    state->b_pressed    = !(high_buttons & (1 << 6));
    state->zl_pressed   = !(high_buttons & (1 << 7));
}

