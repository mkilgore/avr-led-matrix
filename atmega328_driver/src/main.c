
#include "common.h"
#include "shift.h"

#include <stdio.h>

#include <avr/interrupt.h>

#include "twi_slave.h"

#define ROR(S,i) (((S)>>(i)) | ((S)<<(8-(i))))
#define ROL(S,i) (((S)<<(i)) | ((S)>>(8-(i))))

static struct shift_register anode_register = {
    .data_port = &PORTC,
    .data_pin = PORTC0,
    .clock_port = &PORTC,
    .clock_pin = PORTC1,
    .latch_port = &PORTC,
    .latch_pin = PORTC2,
};

volatile uint8_t buf_index = 0;
uint8_t i2c_addr_pointer = 0;

uint8_t buf[2][32] = {
    {
        0b11000000, 0b11000000, 0b11000000, 0b11000000,
        0b11000000, 0b11000000, 0b11000000, 0b11000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
    }
};

static void twi_recv_callback(struct twi_handler *handler, uint8_t data)
{
    int buf_idx = buf_index ^ 1;
    if (handler->cur_reg < 32) {
        buf[buf_idx][handler->cur_reg] = data;
    } else {
        PORTB |= _BV(PORTB1);
        buf_index = buf_idx;
    }

    handler->cur_reg++;
}

static uint8_t twi_request_callback(struct twi_handler *handler)
{
    return 0;
}

static struct twi_handler handler = {
    .recv = twi_recv_callback,
    .request = twi_request_callback,
    .cur_reg = 0,
    .address = I2C_ADDRESS,
};

int main(void)
{
    twi_init(&handler);

    DDRC |= _BV(DDC0);
    DDRC |= _BV(DDC1);
    DDRC |= _BV(DDC2);
    DDRC |= _BV(DDC3);

    PORTC |= _BV(PORTC3);

    DDRB |= _BV(DDB1);
    PORTB &= ~_BV(PORTB1);

    DDRD = 0;
    PORTD = 0;

    DDRB |= _BV(DDB2);

    while (1) {
        uint8_t pb_orig_state = PORTB & _BV(PORTB1);
        uint8_t k;
        uint8_t index = buf_index;
        uint8_t *buf_ptr = buf[index] + 31;

        for (k = 7; k < 8; k--) {
            shift_register_write_byte(&anode_register, *buf_ptr--);
            shift_register_write_byte(&anode_register, *buf_ptr--);
            shift_register_write_byte(&anode_register, *buf_ptr--);
            shift_register_write_byte(&anode_register, *buf_ptr--);

            DDRD = 0;
            shift_register_latch_output(&anode_register);
            DDRD = (1 << k);

            sei();
            _delay_ms(1);
            cli();
        }

        if (pb_orig_state)
            PORTB &= ~_BV(PORTB1);
    }

    return 0;
}

