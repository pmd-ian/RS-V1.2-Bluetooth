#ifndef _PMD_HW_INIT_H_
#define _PMD_HW_INIT_H_

/*===========================================================================
  Filename:     PMD_hw.h
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin       

  Description:  Initialization of all IO pins..
 *=========================================================================*/

/*===========================================================================
 * DEFINES
 *=========================================================================*/

// Define size of buffer.
#define BUFFER_SIZE 252

// These values will give a baud rate of approx. 2.00 Mbps at 32 MHz system clock.
#define SPI_BAUD_M  59
#define SPI_BAUD_E  8

/*===========================================================================
 * TYPES
 *=========================================================================*/


/*===========================================================================
 * FUNCTIONS
 *=========================================================================*/

/*---------------------------------------------------------------------------
 * Description of function. Optional verbose description.
 *-------------------------------------------------------------------------*/
void pmdHW_init(void);
void pmdHW_refClkOnP1_2(void);
void pmdHW_config_spi(void);
void spiWriteByte(uint8 write);
uint8 spiReadByte(uint8 write);

#endif 






