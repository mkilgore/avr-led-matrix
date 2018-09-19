
#include "common.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "usi_i2c_master.h"

static unsigned char usi_twi_master_transfer(unsigned char);
static unsigned char usi_twi_master_stop(void);

union usi_twi_state {
    unsigned char error_state;
    struct {
        unsigned char address_mode:1;
        unsigned char master_write_data_mode:1;
        unsigned char unused:6;
    };
} usi_twi_state;

void usi_twi_master_initialize(void)
{
    PORT_USI |= (1 << PIN_USI_SDA); /* Enable pullup on SDA, to set high as released state. */
    PORT_USI |= (1 << PIN_USI_SCL); /* Enable pullup on SCL, to set high as released state. */

    DDR_USI |= (1 << PIN_USI_SCL); /* Enable SCL as output. */
    DDR_USI |= (1 << PIN_USI_SDA); /* Enable SDA as output. */

    USIDR = 0xFF; /* Preload dataregister with "released level" data. */
    USICR = (0 << USISIE) | (0 << USIOIE) | /* Disable Interrupts. */
        (1 << USIWM1) | (0 << USIWM0) | /* Set USI in Two-wire mode. */
        (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | /* Software stobe as counter clock source */
        (0 << USITC);

    USISR = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |   /* Clear flags, */
        (0x0 << USICNT0);       /* and reset counter. */
}

/*---------------------------------------------------------------
Use this function to get hold of the error message from the last transmission
---------------------------------------------------------------*/
unsigned char usi_twi_get_state_info(void)
{
    return (usi_twi_state.error_state);  // Return error state.
}

/*---------------------------------------------------------------
 USI Transmit and receive function. LSB of first byte in data 
 indicates if a read or write cycles is performed. If set a read
 operation is performed.
 Function generates (Repeated) Start Condition, sends address and
 R/W, Reads/Writes Data, and verifies/sends ACK.
 
 Success or error code is returned. Error codes are defined in 
 USI_TWI_Master.h
---------------------------------------------------------------*/
unsigned char usi_twi_start_transceiver_at_address_with_data(uint8_t address, unsigned char *msg, unsigned char msg_size)
{
    unsigned char tempUSISR_8bit = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |    /* Prepare register value to: Clear flags, and */
        (0x0 << USICNT0);       /* set USI to shift 8 bits i.e. count 16 clock edges. */
    unsigned char tempUSISR_1bit = (1 << USISIF) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |    /* Prepare register value to: Clear flags, and */
        (0xE << USICNT0);       /* set USI to shift 1 bit i.e. count 2 clock edges. */

    usi_twi_state.error_state = 0;
    usi_twi_state.address_mode = 1;

    /* The LSB in the address byte determines if is a masterRead or masterWrite operation. */
    if (!(address & (1 << TWI_READ_BIT)))
        usi_twi_state.master_write_data_mode = 1;

    /* Release SCL to ensure that (repeated) Start can be performed */
    PORT_USI |= (1 << PIN_USI_SCL); /* Release SCL. */

    while (!(PORT_USI & (1 << PIN_USI_SCL))) /* Verify that SCL becomes high. */
        ;

#ifdef TWI_FAST_MODE
    _delay_us(T4_TWI);          /* Delay for T4TWI if TWI_FAST_MODE */
#else
    _delay_us(T2_TWI);          /* Delay for T2TWI if TWI_STANDARD_MODE */
#endif

    /* Generate Start Condition */
    PORT_USI &= ~(1 << PIN_USI_SDA); /* Force SDA LOW. */
    _delay_us(T4_TWI);
    PORT_USI &= ~(1 << PIN_USI_SCL); /* Pull SCL LOW. */
    PORT_USI |= (1 << PIN_USI_SDA); /* Release SDA. */

    /* Write address and Read/Write data */
    do {
        /* If master write cycle (or inital address tranmission) */
        if (usi_twi_state.address_mode || usi_twi_state.master_write_data_mode) {
            /* Write a byte */
            PORT_USI &= ~(1 << PIN_USI_SCL); /* Pull SCL LOW. */

            if (usi_twi_state.address_mode) {
                USIDR = address;
            } else {
                USIDR = *(msg++);   // Setup data.
                msg_size--;
            }

            usi_twi_master_transfer(tempUSISR_8bit); /* Send 8 bits on bus. */

            /* Clock and verify (N)ACK from slave */
            DDR_USI &= ~(1 << PIN_USI_SDA); /* Enable SDA as input. */
            if (usi_twi_master_transfer(tempUSISR_1bit) &
                (1 << TWI_NACK_BIT)) {
                if (usi_twi_state.address_mode)
                    usi_twi_state.error_state = USI_TWI_NO_ACK_ON_ADDRESS;
                else
                    usi_twi_state.error_state = USI_TWI_NO_ACK_ON_DATA;
                return 0;
            }

            usi_twi_state.address_mode = 0; /* Only perform address transmission once. */
        } else {
            /* Read a data byte */
            DDR_USI &= ~(1 << PIN_USI_SDA); // Enable SDA as input.
            *(msg++) = usi_twi_master_transfer(tempUSISR_8bit);
            msg_size--;

            /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
            if (msg_size == 0) { /* If transmission of last byte was performed. */
                USIDR = 0xFF;   /* Load NACK to confirm End Of Transmission. */
            } else {
                USIDR = 0x00;   /* Load ACK. Set data register bit 7 (output for SDA) low. */
            }
            usi_twi_master_transfer(tempUSISR_1bit);    /* Generate ACK/NACK. */
        }
    } while (msg_size); /* Until all data sent/received. */

    usi_twi_master_stop(); /* Send a STOP condition on the TWI bus. */

    /* Transmission successfully completed*/
    return 1;
}

/*---------------------------------------------------------------
 Core function for shifting data in and out from the USI.
 Data to be sent has to be placed into the USIDR prior to calling
 this function. Data read, will be return'ed from the function.
---------------------------------------------------------------*/
static unsigned char usi_twi_master_transfer(unsigned char temp)
{
    USISR = temp; /* Set USISR according to temp. */
    /* Prepare clocking. */
    temp = (0 << USISIE) | (0 << USIOIE) |  /* Interrupts disabled */
        (1 << USIWM1) | (0 << USIWM0) | /* Set USI in Two-wire mode. */
        (1 << USICS1) | (0 << USICS0) | (1 << USICLK) | /* Software clock strobe as source. */
        (1 << USITC);           /* Toggle Clock Port. */
    do {
        _delay_us(T2_TWI);
        USICR = temp; /* Generate positve SCL edge. */

        while (!(PIN_USI & (1 << PIN_USI_SCL))) /* Wait for SCL to go high. */
            ;

        _delay_us(T4_TWI);
        USICR = temp; /* Generate negative SCL edge. */
    } while (!(USISR & (1 << USIOIF))); /* Check for transfer complete. */

    _delay_us(T2_TWI);
    temp = USIDR; /* Read out data. */
    USIDR = 0xFF; /* Release SDA. */
    DDR_USI |= (1 << PIN_USI_SDA); /* Enable SDA as output. */

    return temp; /* Return the data from the USIDR */
}

/*---------------------------------------------------------------
 Function for generating a TWI Stop Condition. Used to release 
 the TWI bus.
---------------------------------------------------------------*/
static unsigned char usi_twi_master_stop(void)
{
    PORT_USI &= ~(1 << PIN_USI_SDA); /* Pull SDA low. */
    PORT_USI |= (1 << PIN_USI_SCL); /* Release SCL. */

    while (!(PIN_USI & (1 << PIN_USI_SCL))) /* Wait for SCL to go high. */
        ;

    _delay_us(T4_TWI);
    PORT_USI |= (1 << PIN_USI_SDA); /* Release SDA. */
    _delay_us(T2_TWI);

    return 1;
}

unsigned char usi_twi_start_transceiver_with_data(unsigned char *msg,
                                                  unsigned char msg_size)
{
    return usi_twi_start_transceiver_at_address_with_data(msg[0], msg + 1, msg_size - 1);
}

uint8_t usi_twi_write_data(uint8_t address, const uint8_t *data, uint8_t count)
{
    return usi_twi_start_transceiver_at_address_with_data(address << 1, (uint8_t *)data, count);
}

uint8_t usi_twi_read_data(uint8_t address, uint8_t *data, uint8_t count)
{
    return usi_twi_start_transceiver_at_address_with_data((address << 1) | 0x01, data, count);
}

