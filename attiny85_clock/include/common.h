#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define DDR_USI     DDRB
#define PORT_USI    PORTB
#define PIN_USI     PINB
#define PIN_USI_SDA PINB0
#define PIN_USI_SCL PINB2

#endif
