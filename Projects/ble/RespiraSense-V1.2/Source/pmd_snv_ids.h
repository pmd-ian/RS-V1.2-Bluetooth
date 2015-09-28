#ifndef _PMD_SNV_IDS_H_
#define _PMD_SNV_IDS_H_
/*===========================================================================*
  Filename:     pmd_snv_ids.h
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin

  Description:  Definition of svn ids.
                Note that the stack used ids below 0xA0. This could change.

 *=========================================================================*/
#include "osal_snv.h"

/*===========================================================================
 * DEFINES
 *=========================================================================*/
#ifndef OSAL_SNV_UINT16_ID
  #define pmdNVI_START                   (0x100)
#else
  #define pmdNVI_START                   (0xA0)
#endif

#define pmdNVI_ERROR_CODE_ID             (osalSnvId_t)(pmdNVI_START + 1)
#define pmdNVI_WATCHDOG_ID               (osalSnvId_t)(pmdNVI_START + 2)
#define pmdNVI_SERVER_PROFILE_ID         (osalSnvId_t)(pmdNVI_START + 3)

#endif 






