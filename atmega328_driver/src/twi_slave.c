
#include "common.h"

#include <avr/interrupt.h>
#include <util/twi.h>

#include "twi_slave.h"

static struct twi_handler *twi_handler;

static inline void ack_twi_int(void)
{
    TWCR = _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWEN);
}

void twi_init(struct twi_handler *handler)
{
    twi_handler = handler;

    uint8_t sreg = SREG;
    cli();
    TWSR = 0x03;

    /* Load slave address */
    TWAR = handler->address << 1;

    /* Enable TWI interrupt */
    TWCR = _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWEN);

    SREG = sreg;
}

ISR(TWI_vect)
{
    uint8_t data;

    switch (TW_STATUS) {
    case TW_SR_SLA_ACK:
        twi_handler->state = TWI_STATE_RECV_REG;
        break;

    case TW_SR_DATA_ACK:
        data = TWDR;

        switch (twi_handler->state) {
        case TWI_STATE_RECV_REG:
            twi_handler->cur_reg = data;
            twi_handler->state = TWI_STATE_READ_WRITE;
            break;

        case TWI_STATE_READ_WRITE:
            (twi_handler->recv) (twi_handler, data);
            break;
        }
        break;

    case TW_ST_SLA_ACK:
        TWDR = twi_handler->request(twi_handler);
        break;

    case TW_ST_DATA_ACK:
        TWDR = twi_handler->request(twi_handler);
        break;

    case TW_BUS_ERROR:
        /* Reset TWI */
        TWCR = 0;
        break;
    }

    ack_twi_int();
}

