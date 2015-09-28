/*===========================================================================
  Filename:     PMD_hw.c
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin       

  Description:  Initialization of all IO pins..
 *=========================================================================*/

#include "hal_mcu.h"
#include "PMD_hw.h"
   
// Include Name definitions of individual bits and bit-fields in the CC254x device registers.
#include <hal_mcu.h>

/*===========================================================================
 * DEFINES
 *=========================================================================*/
 #define PERCFG_U0CFG_ALT1             0x00          // Alternative 2 location
 #define PERCFG_U0CFG                  0x01          // USART 0 I/O location
 #define P2DIR_PRIP0_USART0           (0x00 << 6)    // USART 0 has priority, then USART 1, then Timer 1

/*===========================================================================
 * TYPES
 *=========================================================================*/


/*===========================================================================
 * DECLARATIONS
 *=========================================================================*/


/*===========================================================================
 * DEFINITIONS
 *=========================================================================*/


/*===========================================================================
 * FUNCTIONS
 *=========================================================================*/
void pmdHW_init(void)
{
  
  /****************************************************************************
  * Clock setup
  * See basic software example "clk_xosc_cc254x"
  */
  
  // Set system clock source to HS XOSC, with no pre-scaling.
  CLKCONCMD = (CLKCONCMD & ~(CLKCON_OSC | CLKCON_CLKSPD)) | CLKCON_CLKSPD_32M;
  // Wait until clock source has changed.
  while (CLKCONSTA & CLKCON_OSC);
  
  // Note the 32 kHz RCOSC starts calibrating, if not disabled.
  
  /***************************************************************************
  * Setup I/O ports
  *
  * Port and pins used USART0 operating in SPI-mode are
  * MISO (MI): P0_2
  * MOSI (MO): P0_3
  * SSN (SS) : P0_4
  * SCK (C)  : P0_5
  *
  * These pins can be set to function as peripheral I/O so that they
  * can be used by USART0 SPI. Note however, when SPI is in master mode,
  * only MOSI, MISO, and SCK should be configured as peripheral I/O's.
  * If the external slave device requires a slave select signal (SSN),
  * this can be implemented by using general-purpose I/O pin configured as
  * output.
  ***************************************************************************/
  
  PERCFG = (PERCFG & ~PERCFG_U0CFG) | PERCFG_U0CFG_ALT1; 
  
  // Give priority to USART 0 over Timer 1 for port 0 pins.
  P2DIR &= P2DIR_PRIP0_USART0;
  
  
  // P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO
  
  // P0
  // P0.0 ADC0 (IN TRI-STATE)
  // P0.1 ADC1 (IN TRI-STATE)
  // P0.2 UART_RxD* / SPI_MISO* / SPI_SS** (IN TRI-STATE)
  // P0.3 UART_TxD* / SPI_MOSI* / SPI_CLK** (IN TRI-STATE)
  // P0.4 UART_CTS* / SPI_SS* / SPI_MOSI** (IN TRI-STATE)
  // P0.5 UART_RTS* / SPI_CLK* / SPI_MISO* (IN TRI-STATE)
  // P0.6 ADC_IN2 (IN TRI-STATE)
  // P0.7 ADC_IN3 / Interrupt 2 (TMP112 interrupt) (IN TRI-STATE)
  
  // P0DIR = 0x00;   // All inputs
  //P0DIR = 0xFF;     // All outputs
  //P0INP = 0xFF; // Tristate
  // P0INP = 0x10;   // PU/PD
  //glen  P2INP &= ~0x20; // Port 0 PU
  //P2INP = 0x20; // Port 0 PD
  // P0 = 0x00; 
  
  // Set pins 2, 3 and 5 as peripheral I/O and pin 4 as GPIO output. SPI Config Code
  P0SEL = (P0SEL & ~BIT4) | BIT5 | BIT3 | BIT2;
  P0DIR |= BIT4;
  
  P0 = 0x10;
  
  // P1
  // 11110011
  // P1.0 VCC_Peripheral  (OUT - LOW)
  // P1.1 Blue LED (OUT - HIGH)
  // P1.2 Switch-0 (IN)
  // P1.3 Interrupt 1 (IN) 
  // P1.4 Green/Switch-1 (OUT)
  // P1.5 I2C_CLK (IN - PU)
  // P1.6 I2C_SDA (IN - PU)
  // P1.7 Red (OUT)
  P1DIR =  ((1<<7) | (1<<4) | (1<<1)); 
  P1INP =  0x00; // All pins PU/PD
  P1    =  (1<<7) | (1<<4) | (1<<1);
  P2INP |= 0x40;// Port 1 PU
  //P2INP &= ~0x40;// Port 1 PD
  
  // P2
  // P2.0 ExtInt (IN)
  // P2.1 Debug_Data / UART_DTR (IN)
  // P2.2 Debug_Clk / UART_DSR (IN)
  P2DIR = 0x00; 
  //P2    = 0x00; 
  //P2INP |= 0x1F; // Port 2 Tri state                
  
  ///test 
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output
  P2 = 0;
  
  
}

void pmdHW_refClkOnP1_2(void)
{
  /* Timer test */
  CLKCONCMD &= ~CLKSPD(0); // 32 MHz
  CLKCONCMD &= ~(OSC); // 32 MHz crystal
  CLKCONCMD |= TICKSPD(3); // 4 MHz

  PERCFG |= (1 << 6); // T1CFG Timer 1 I/O location alternative 2 location
  P1DIR |= 0x04; // P1.2 = Output
  P1SEL |= 0x04; // P1.2 = Peripheral function
  P2SEL &= ~(1 << 4); // PRI1P1   Timer 1 has priority vs Timer 4
  P2SEL |= (1 << 3); // PRI0P1   Timer 1 has priority vs UART 1

  T1CC0L = 0x01;
  T1CC0H = 0x00;

  T1CTL = 0x02; // Tick frequency/1, Modulo mode

  T1CCTL0 = 0x38; // magic init?
  T1CCTL0 = 0x14;  

  while(1);
}

void pmdHW_config_spi(void)
{
  /***************************************************************************
     * Configure SPI
     */

    // Set USART to SPI mode and Master mode.
    U0CSR &= ~(U0CSR_MODE | U0CSR_SLAVE);
    
    // Set:
    // - mantissa value
    // - exponent value
    // - clock phase to be centered on first edge of SCK period
    // - negative clock polarity (SCK low when idle)
    // - bit order for transfers to LSB first  
    U0BAUD =  SPI_BAUD_M;
    
    //U0GCR = (U0GCR & ~(U0GCR_BAUD_E | U0GCR_CPOL ))
    //    | SPI_BAUD_E | U0GCR_ORDER  | U0GCR_CPHA ;
    
    U0GCR = (U0GCR & ~(U0GCR_CPOL | U0GCR_CPHA))
                            | SPI_BAUD_E | U0GCR_ORDER;  //U0GCR_BAUD_E | 
}


/*===========================================================================
 * @fn       spiWriteByte(uint8 write)
 *
 * @brief    Write one byte to SPI interface
 *
 * @param    write   Value to write
 *=========================================================================*/
void spiWriteByte(uint8 write)
{  
  U0DBUF = write;
  while (!(U0CSR & U0CSR_TX_BYTE));        // Wait for TX_BYTE to be set
  U0CSR &= ~U0CSR_TX_BYTE;                 // Clear TX_BYTE
}

/*===========================================================================
 * @fn       spiReadByte(uint8 *read, uint8 write)
 *
 * @brief    Read one byte from SPI interface
 *
 * @param    write   Value to write
 *=========================================================================*/
uint8 spiReadByte(uint8 write)
{
  uint8 read;
  
  U0DBUF = write;                          // Write address to 
  while (!(U0CSR & U0CSR_TX_BYTE));        // Wait for TX_BYTE to be set
  U0CSR &= ~U0CSR_TX_BYTE;                 // Clear TX_BYTE
  
  read = U0DBUF;                           // Save returned value
  
  return read;     
}
