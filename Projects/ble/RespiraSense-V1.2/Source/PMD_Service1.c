/*===========================================================================*
  Filename:     PMD_Service1.c
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin

  Description:  This file contains the following characteristics...

  Identifier    Description                     Connection Method
  ----------    -----------                     -----------------
  FFE0	        Info Service	                Service
  FFE1	        Firmware Revision	        Read
  FFE2	        Rename	                        Write
  FFE3	        Patient First Name	        Read/Write
  FFE4	        Patient Second Name	        Read/Write
  FFE5	        Sex	                        Read/Write
  FFE6	        DOB	                        Read/Write
  FFE7	        Date of Admission	        Read/Write
  FFE8	        Bed Number	                Read/Write
  FFE9	        Room Number/Name	        Read/Write
  FFEA	        Ward Number/Name	        Read/Write

 *=========================================================================*/

/*===========================================================================
 * INCLUDES
 *=========================================================================*/
#include "bcomdef.h"
#include "hal_types.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "PMD_hw.h"
#include "hal_mcu.h"
   
#include "PMD_Service1.h"

/*===========================================================================
 * MACROS
 *=========================================================================*/

/*===========================================================================
 * CONSTANTS
 *=========================================================================*/

#define SERVAPP_NUM_ATTR_SUPPORTED        31
#define ATTRIBUTE16(uuid, pProps, pValue)  { {ATT_BT_UUID_SIZE, uuid}, pProps, 0, (uint8*)pValue}

/*===========================================================================
 * TYPEDEFS
 *=========================================================================*/

/*===========================================================================
 * GLOBAL VARIABLES
 *=========================================================================*/

CONST uint8 pmd_service1_UUID[ATT_BT_UUID_SIZE] = { LO_UINT16(PMD_SERVICE1_UUID), HI_UINT16(PMD_SERVICE1_UUID)};
CONST uint8 firmware_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(FIRMWARE_UUID), HI_UINT16(FIRMWARE_UUID)};
CONST uint8 rename_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(RENAME_UUID), HI_UINT16(RENAME_UUID)};
CONST uint8 name_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(NAME_UUID), HI_UINT16(NAME_UUID)};
CONST uint8 surname_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(SURNAME_UUID), HI_UINT16(SURNAME_UUID)};
CONST uint8 gender_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(GENDER_UUID), HI_UINT16(GENDER_UUID)};
CONST uint8 dob_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(DoB_UUID), HI_UINT16(DoB_UUID)};
CONST uint8 doa_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(DoA_UUID), HI_UINT16(DoA_UUID)};
CONST uint8 bed_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(BED_UUID), HI_UINT16(BED_UUID)};
CONST uint8 room_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(ROOM_UUID), HI_UINT16(ROOM_UUID)};
CONST uint8 ward_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(WARD_UUID), HI_UINT16(WARD_UUID)};


/*===========================================================================
 * EXTERNAL VARIABLES
 *=========================================================================*/

/*===========================================================================
 * EXTERNAL FUNCTIONS
 *=========================================================================*/
extern void Rename_Device(uint8 *name);

/*===========================================================================
 * LOCAL VARIABLES
 *=========================================================================*/


/*===========================================================================
 * Profile Attributes - variables
 *=========================================================================*/

// Service attribute
static CONST gattAttrType_t PMD_Service1 = { ATT_BT_UUID_SIZE, pmd_service1_UUID };

// Attribute Properties
static uint8 AttrProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Attribute Variables
static uint8 firmware_data[FIRMWARE_LEN] = {0};
static uint8 rename_data[RENAME_LEN] = {0};
static uint8 name_data[NAME_LEN] = {0};
static uint8 surname_data[SURNAME_LEN] = {0};
static uint8 gender_data[GENDER_LEN] = {0};
static uint8 dob_data[DoB_LEN] = {0};
static uint8 doa_data[DoA_LEN] = {0};
static uint8 bed_data[BED_LEN] = {0};
static uint8 room_data[ROOM_LEN] = {0};
static uint8 ward_data[WARD_LEN] = {0};

// Attribute Descriptions
static uint8 firmware_desc[9] = "Firmware\0";
static uint8 rename_desc[7] = "Rename\0";
static uint8 name_desc[11] = "First Name\0";
static uint8 surname_desc[8] = "Surname\0";
static uint8 gender_desc[7] = "Gender\0";
static uint8 dob_desc[14] = "Date of Birth\0";
static uint8 doa_desc[18] = "Date of Admission\0";
static uint8 bed_desc[11] = "Bed Number\0";
static uint8 room_desc[17] = "Room Name/Number\0";
static uint8 ward_desc[17] = "Ward Name/Number\0";


/*===========================================================================
 * Profile Attributes - Table
 *=========================================================================*/

static gattAttribute_t pmd1AttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
   // PMD Service 1 Primary Service 0xFFFE     
  ATTRIBUTE16( primaryServiceUUID, GATT_PERMIT_READ, &PMD_Service1),

   // Firmware  0xFFE1
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(firmware_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, firmware_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, firmware_desc),  
  
   // Rename 0xFFE2
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(rename_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, rename_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, rename_desc),  
 
   // Patient Details
   // Name 0xFFE3
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(name_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, name_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, name_desc),  
  
   // Surname 0xFFE4
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(surname_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, surname_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, surname_desc),  
  
   // Gender 0xFFE5
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(gender_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, gender_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, gender_desc),  
  
   // Date of Birth 0xFFE6
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(dob_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, dob_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, dob_desc),  
 
   // Date of Admission 0xFFE7
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(doa_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, doa_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, doa_desc),  
  
   // Bed Number 0xFFE8
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(bed_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, bed_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, bed_desc),  
  
   // Room Name/Number 0xFFE9
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(room_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, room_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, room_desc),  
  
   // Ward Name/Number 0xFFEA
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(ward_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, ward_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, ward_desc),  
  
};
  

/*===========================================================================
 * LOCAL FUNCTIONS
 *=========================================================================*/
static bStatus_t PMD1_readAttrHandler(  uint16 connHandle, gattAttribute_t *pAttr,
					uint8 *pValue, uint8 *pLen, uint16 offset,
					uint8 maxLen, uint8 method );
static bStatus_t PMD1_writeAttrHandler( uint16 connHandle, gattAttribute_t *pAttr,
					uint8 *pValue, uint8 len, uint16 offset,
					uint8 method);

/*===========================================================================
 * PROFILE CALLBACKS
 *=========================================================================*/
// Service callbacks registered to GATT 
CONST gattServiceCBs_t PMD1s =
{
  PMD1_readAttrHandler,         // Read callback
  PMD1_writeAttrHandler,        // Write callback
  NULL                          // Authorization callback
};


/*===========================================================================
 * PUBLIC FUNCTIONS
 *=========================================================================*/

/*********************************************************************
 * @fn      PMD_Service1_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 *=========================================================================*/

bStatus_t PMD_Service1_AddService(void)
{
  uint8 status = SUCCESS;

  // Register GATT attribute list and callbacks with GATT Server App
  status = GATTServApp_RegisterService( pmd1AttrTbl, GATT_NUM_ATTRS( pmd1AttrTbl ),
					GATT_MAX_ENCRYPT_KEY_SIZE, &PMD1s );
  
  return (status);
}



/*===========================================================================
 * @fn          PMD1_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 *=========================================================================*/
static bStatus_t PMD1_readAttrHandler( uint16 connHandle, gattAttribute_t *pAttr, 
                                       uint8 *pValue, uint8 *pLen, uint16 offset,
                                       uint8 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case FIRMWARE_UUID:                       // Firmware revisions
        if (firmware_data[2] == 0 || firmware_data[3] == 0)
          Get_Firmware();
        *pLen = FIRMWARE_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case NAME_UUID:                           // Patient First Name
        *pLen = NAME_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case SURNAME_UUID:                        // Patient Surname
        *pLen = SURNAME_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case GENDER_UUID:                         // Patient Gender
        *pLen = GENDER_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case DoB_UUID:                            // Patient Date of Birth
        *pLen = DoB_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case DoA_UUID:                            // Patient Date of Admission
        *pLen = DoA_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case BED_UUID:                            // Patient Bed Number
        *pLen = BED_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case ROOM_UUID:                           // Patient Room Name/Number
        *pLen = ROOM_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case WARD_UUID:                           // Patient Ward Name/Number
        *pLen = WARD_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      default:
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*===========================================================================
 * @fn      PMD1_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
  * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 *=========================================================================*/
static bStatus_t PMD1_writeAttrHandler( uint16 connHandle, gattAttribute_t *pAttr,
					uint8 *pValue, uint8 len, uint16 offset,
					uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 new_name[21] = {0};
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case RENAME_UUID:                         // Renaming Routine
        if (len < 20 && len >= 1)
        { 
          new_name[0] = len+1;
          new_name[1] = 0x09;
          
          for(int i=2;i<21;i++)
          {
            if(i<=new_name[0])
              new_name[i] = pValue[i-2];      
            else
               new_name[i] = 0x00;
          }
         
          Rename_Device(new_name);              // Update Bluetooth ID - need to review how this function works!!!
          
          P0_4 = 0;  //CS PIN
          spiWriteByte(Rename_SPI);             // Initialize Algorithm on DSP
          P0_4 = 1;  //CS PIN
        }
        break;
      
      case NAME_UUID:                           // Patient First Name
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case SURNAME_UUID:                        // Patient Surname
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case GENDER_UUID:                         // Patient Gender
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case DoB_UUID:                            // Patient Date of Birth
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case DoA_UUID:                            // Patient Date of Admission
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case BED_UUID:                            // Patient Bed Number
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case ROOM_UUID:                           // Patient Room Name/Number
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      case WARD_UUID:                           // Patient Ward Name/Number
        osal_memcpy(pAttr->pValue, pValue, len);
        break;
        
      default:
        // Should never get here!
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*===========================================================================
 * @fn      Get_Firmware
 *
 * @brief   Retrieve firmware revision from DSP and attach BT firmware
 *
 * @param   N/A
 *
 * @return  N/A
 *=========================================================================*/
void Get_Firmware(void)
{  
  P0_4 = 0;  //CS PIN
  spiWriteByte(Firmware_SPI);
  firmware_data[0] = spiReadByte(dummyData);
  firmware_data[1] = spiReadByte(dummyData);
  P0_4 = 1;  //CS PIN
  
  firmware_data[2] = Firmware_Revision;
  firmware_data[3] = Firmware_Eng_Rev;
}

