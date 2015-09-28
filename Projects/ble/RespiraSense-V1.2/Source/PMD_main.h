#ifndef _PMD_MAIN_H_
#define _PMD_MAIN_H_

/*===========================================================================*
  Filename:     PMD_main.c
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin

  Description:  Application based on the keyfob demo from TI.
                Functionality have been changed to fit the RespiraSense PCB.

 *=========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "peripheral.h"

/*===========================================================================
 * DEFINES
 *=========================================================================*/
// Application Events handled in task function
#define PMD_START_DEVICE_EVT                              0x0001
#define PMD_ADV_IN_CONNECTION_EVT                         0x0004
#define START_STREAMING                                      0x0020
#define STOP_STREAMING                                       0x0030

/*===========================================================================
 * TYPES
 *=========================================================================*/ 
  
/*===========================================================================
 * FUNCTIONS
 *=========================================================================*/
extern void PMD_init(uint8 taskId);

extern uint16 PMD_processEvent(uint8 taskId, uint16 events);

void updateNameWithAddressInfo(void);

void Rename_Device(uint8 *name);

void getNameWithAddressInfo(attHandleValueNoti_t data);

#ifdef __cplusplus
}
#endif

#endif
