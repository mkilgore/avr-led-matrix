#ifndef INCLUDE_USI_I2C_MASTER_H
#define INCLUDE_USI_I2C_MASTER_H

#include "common.h"
#include <inttypes.h>

//********** Defines **********//

// Defines controlling timing limits
#define TWI_FAST_MODE

#define SYS_CLK   (F_CPU/1000)  // [kHz]

#ifdef TWI_FAST_MODE               // TWI FAST mode timing limits. SCL = 100-400kHz
//  #define T2_TWI    ((SYS_CLK *1300) /1000000) +1 // >1,3us
//  #define T4_TWI    ((SYS_CLK * 600) /1000000) +1 // >0,6us
#  define T2_TWI	1.3
#  define T4_TWI	0.6

#else                              // TWI STANDARD mode timing limits. SCL <= 100kHz
//  #define T2_TWI    ((SYS_CLK *4700) /1000000) +1 // >4,7us
//  #define T4_TWI    ((SYS_CLK *4000) /1000000) +1 // >4,0us
#  define T2_TWI	4.7
#  define T4_TWI	4.0
#endif

/****************************************************************************
  Bit and byte definitions
****************************************************************************/
#define TWI_READ_BIT  0       // Bit position for R/W bit in "address byte".
#define TWI_ADR_BITS  1       // Bit position for LSB of the slave address bits in the init byte.
#define TWI_NACK_BIT  0       // Bit position for (N)ACK bit.

#define USI_TWI_NO_DATA             0x00  // Transmission buffer is empty
#define USI_TWI_DATA_OUT_OF_BOUND   0x01  // Transmission buffer is outside SRAM space
#define USI_TWI_UE_START_CON        0x02  // Unexpected Start Condition
#define USI_TWI_UE_STOP_CON         0x03  // Unexpected Stop Condition
#define USI_TWI_UE_DATA_COL         0x04  // Unexpected Data Collision (arbitration)
#define USI_TWI_NO_ACK_ON_DATA      0x05  // The slave did not acknowledge  all data
#define USI_TWI_NO_ACK_ON_ADDRESS   0x06  // The slave did not acknowledge  the address
#define USI_TWI_MISSING_START_CON   0x07  // Generated Start Condition not detected on bus
#define USI_TWI_MISSING_STOP_CON    0x08  // Generated Stop Condition not detected on bus

//********** Prototypes **********//
void usi_twi_master_initialize(void);

/* Note: 'address' and buffer must include read/write byte */
unsigned char usi_twi_start_transceiver_at_address_with_data(uint8_t address, unsigned char *msg, unsigned char msg_size);
unsigned char usi_twi_start_transceiver_with_data(unsigned char *, unsigned char);
unsigned char usi_twi_get_state_info(void);

/* Writes 'count' bytes from 'data' to address */
uint8_t usi_twi_write_data(uint8_t address, const uint8_t *data, uint8_t count);

/* Reads 'count' bytes from address into 'data' buffer */
uint8_t usi_twi_read_data(uint8_t address, uint8_t *data, uint8_t count);

#endif
