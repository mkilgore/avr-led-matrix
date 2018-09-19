#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#define USE_PRINTF

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define I2C_ADDRESS 0x2A

extern volatile uint8_t buf_index;
extern uint8_t buf[2][32];

#endif
