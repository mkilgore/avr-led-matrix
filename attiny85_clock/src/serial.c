/*
 * This is based on the code from:
 * github.com/sourceperl/tiny.serial
 *
 * It has the addition of a buffered output
 */
#include "common.h"

#include <avr/interrupt.h>
#include <stdio.h>

#include "serial.h"

#define SERIAL_BUFFER_SIZE 64

static volatile uint8_t bit_index = 0;

static volatile uint8_t buffer_in = 0;
static volatile uint8_t buffer_out = 0;
static volatile uint8_t tx_buffer[SERIAL_BUFFER_SIZE];
static volatile uint8_t buffer_is_full = 0;

ISR(TIM0_COMPA_vect)
{
    if (buffer_out != buffer_in || buffer_is_full) {
        if (bit_index == 0) {
            /* Send start bit */
            cbi(SERIAL_PORT, SERIAL_PIN);
            bit_index++;
        } else if (bit_index <= 8) {
            /* LSB to MSB */
            if (tx_buffer[buffer_out] & 1) {
                sbi(SERIAL_PORT, SERIAL_PIN);
            } else {
                cbi(SERIAL_PORT, SERIAL_PIN);
            }

            tx_buffer[buffer_out] >>= 1;
            bit_index++;
        } else if (bit_index == 9) {
            /* Send stop bit */
            sbi(SERIAL_PORT, SERIAL_PIN);
            if (buffer_out == SERIAL_BUFFER_SIZE - 1)
                buffer_out = 0;
            else
                buffer_out++;

            bit_index = 0;
            buffer_is_full = 0;

            /* If the tx buffer is now empty, turn the timer interrupt off */
            if (buffer_in == buffer_out)
                cbi(TIMSK, OCIE0A);
        }

    }
}

#ifdef USE_PRINTF
static int serial_putc(char c, FILE *f)
{
    serial_write(c);
    return c;
}
#endif

void serial_init(void)
{
    sbi(SERIAL_DDR, SERIAL_PIN);
    sbi(SERIAL_PORT, SERIAL_PIN); /* serial idles at '1' */

    /* Set CTC mode for timer 0 */
    TCCR0A = (1 << WGM01);

    /* Set prescaler - 1 tick will be 1us */
    TCCR0B = (1 << CS01);

    /* Fire interrupt every 103us - Tihs gives us 9600 baud */
    OCR0A = 103;

#ifdef USE_PRINTF
    fdevopen(&serial_putc, NULL);
#endif
}

void serial_write(uint8_t tx_byte)
{
    while (buffer_is_full)
        ;

    cli();
    tx_buffer[buffer_in] = tx_byte;

    if (buffer_in == SERIAL_BUFFER_SIZE - 1)
        buffer_in = 0;
    else
        buffer_in++;

    if (buffer_in == buffer_out)
        buffer_is_full = 1;

    /* Turn on the timer interrupt, it's possible it was not on if the buffer was empty */
    if (!(TIMSK & _BV(OCIE0A))) {
        sbi(TIMSK, OCIE0A);
        TCNT0 = 0;
    }
    sei();
}

