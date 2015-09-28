/*---------------------------------------------------------------------------
 * Copyright (c) 2000, 2001 connectBlue AB, Sweden.
 * Any reproduction without written permission is prohibited by law.
 *
 * Component   : System initialization
 * File        : main.c
 *
 * Description : Init system
 *-------------------------------------------------------------------------*/

/* Hal Drivers */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"
#include "hal_uart.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OnBoard.h"
#include "PMD_hw.h"

/*===========================================================================
 * DEFINES
 *=========================================================================*/

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

/*---------------------------------------------------------------------------
 * main
 *-------------------------------------------------------------------------*/
int main(void)
{

   
   
  /* Initialize hardware */
  HAL_BOARD_INIT();
  
  // Initialize board I/O
  InitBoard( OB_COLD );

  /* PCB specific initialization */
  pmdHW_init(); 
  
  
  pmdHW_config_spi();
  
  P0 = 0x10;
  
   /* Initialze the HAL driver */
  HalDriverInit();

  /* Initialize NV system */
  osal_snv_init();

  /* Initialize the operating system */
  osal_init_system();

  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();

  /* Final board initialization */
  InitBoard( OB_READY );   
  
  osal_pwrmgr_device( PWRMGR_BATTERY );

  /* Start OSAL */
  osal_start_system(); // No Return from here

  return 0;
}