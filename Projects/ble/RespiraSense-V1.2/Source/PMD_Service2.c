/*===========================================================================*
  Filename:     PMD_Service2.c
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin       

  Description:  This file contains the following characteristics...

  Identifier    Description                     Connection Method
  ----------    -----------                     -----------------
  FFF0          Data Service                    Service
  FFF1	        Latest Data	                Read
  FFF2	        Set Thresholds	                Write
  FFF3	        Silence Alarm	                Write
  FFF4	        Maintenance	                Write
  FFF5	        Trend Data - RR	                Read
  FFF6	        Trend Data - Act. / Body Pos.	Read
  FFF7	        Streaming	                Notify
  FFF8	        Streaming Configuration	        Write
  FFF9	        Error Codes	                Read
  FFFA          Connection Tone                 Read
  FFFB          Device Settings                 Read/Write

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
#include "PMD_main.h"
#include "PMD_hw.h"
#include "hal_mcu.h"
   
#include "PMD_Service2.h"

#include "peripheral.h"
#include "gapgattserver.h"

/*===========================================================================
 * MACROS
 *=========================================================================*/

/*===========================================================================
 * CONSTANTS
 *=========================================================================*/

#define SERVAPP_NUM_ATTR_SUPPORTED        35
#define ATTRIBUTE16(uuid, pProps, pValue)  { {ATT_BT_UUID_SIZE, uuid}, pProps, 0, (uint8*)pValue}

/*===========================================================================
 * TYPEDEFS
 *=========================================================================*/

/*===========================================================================
 * GLOBAL VARIABLES
 *=========================================================================*/

CONST uint8 pmd_service2_UUID[ATT_BT_UUID_SIZE] = { LO_UINT16(PMD_SERVICE2_UUID), HI_UINT16(PMD_SERVICE2_UUID)};
CONST uint8 latest_data_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(LATEST_DATA_UUID), HI_UINT16(LATEST_DATA_UUID)};
CONST uint8 thresholds_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(THRESHOLDS_UUID), HI_UINT16(THRESHOLDS_UUID)};
CONST uint8 silence_alarm_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(SILENCE_ALARM_UUID), HI_UINT16(SILENCE_ALARM_UUID)};
CONST uint8 maintenance_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(MAINTENANCE_UUID), HI_UINT16(MAINTENANCE_UUID)};
CONST uint8 trend_rr_data_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(TREND_RR_DATA_UUID), HI_UINT16(TREND_RR_DATA_UUID)};
CONST uint8 trend_act_data_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(TREND_ACT_DATA_UUID), HI_UINT16(TREND_ACT_DATA_UUID)};
CONST uint8 streaming_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(STREAMING_UUID), HI_UINT16(STREAMING_UUID)};
CONST uint8 streaming_config_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(STREAMING_CONFIG_UUID), HI_UINT16(STREAMING_CONFIG_UUID)};
CONST uint8 error_codes_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(ERROR_CODES_UUID), HI_UINT16(ERROR_CODES_UUID)};
CONST uint8 connection_tone_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(CONNECTION_TONE_UUID), HI_UINT16(CONNECTION_TONE_UUID)};
CONST uint8 settings_uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(SETTINGS_UUID), HI_UINT16(SETTINGS_UUID)};

uint8 MAINTENANCE_FLAG=0;
uint8 maintenanceValue=0;
uint8 FPC_ALARM=0;
uint8 INT_FLAG=0;
uint8 STREAMING_FLAG;
uint8 pmdStreamValue[STREAMING_LEN];

static uint8 latest_dsp_data[3] = {0};
static uint8 latest_trend_data[2] = {0};
static uint8 advertisement_data[8] = {0};
uint8 BatteryLevel=0;
static uint8 trend_rr[1160] = {0};
static uint8 trend_act[1160] = {0};
static uint16 counter = 0;
uint8 dummy=0;

/*===========================================================================
 * EXTERNAL VARIABLES
 *=========================================================================*/
extern uint8 PMDtaskId;
/*===========================================================================
 * EXTERNAL FUNCTIONS
 *=========================================================================*/


/*===========================================================================
 * LOCAL VARIABLES
 *=========================================================================*/
static PMD_Service2_CBs_t *PMD_Service2_AppCBs = NULL;

/*===========================================================================
 * Profile Attributes - variables
 *=========================================================================*/

// Service attribute
static CONST gattAttrType_t PMD_Service2 = { ATT_BT_UUID_SIZE, pmd_service2_UUID };

// Attribute Properties
static uint8 AttrProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8 StreamingProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Attribute Variables
static uint8 latest_data[LATEST_DATA_LEN] = {0};
static uint8 thresholds_data[THRESHOLDS_LEN] = {0};
static uint8 silence_alarm_data[SILENCE_ALARM_LEN] = {0};
static uint8 maintenance_data[MAINTENANCE_LEN] = {0};
static uint8 trend_rr_data[TREND_RR_DATA_LEN] = {0};
static uint8 trend_act_data[TREND_ACT_DATA_LEN] = {0};
static uint8 streaming_data[STREAMING_LEN] = {0};
static uint8 streaming_config_data[STREAMING_CONFIG_LEN] = {0};
static uint8 error_codes_data[ERROR_CODES_LEN] = {0};
static uint8 settings_data[SETTINGS_LEN] = {0};


// Attribute Descriptions
static uint8 latest_data_desc[12] = "Latest Data\0";
static uint8 thresholds_desc[11] = "Thresholds\0";
static uint8 silence_alarm_desc[14] = "Silence Alarm\0";
static uint8 maintenance_desc[12] = "Maintenance\0";
static uint8 trend_rr_data_desc[14] = "Trend RR Data\0";
static uint8 trend_act_data_desc[19] = "Trend Act/Pos Data\0";
static uint8 streaming_desc[10] = "Streaming\0";
static uint8 streaming_config_desc[17] = "Streaming Config\0";
static uint8 error_codes_desc[12] = "Error Codes\0";
static uint8 connection_tone_desc[16] = "Connection Tone\0";
static uint8 settings_desc[9] = "Settings\0";

// Streaming Characteristic Configuration
static gattCharCfg_t *streaming_data_config;

/*===========================================================================
 * Profile Attributes - Table
 *=========================================================================*/

static gattAttribute_t pmd2AttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
   // PMD Service 2 Primary Service 0xFFFE     
  ATTRIBUTE16( primaryServiceUUID, GATT_PERMIT_READ, &PMD_Service2),

   // Latest Data  0xFFF1
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(latest_data_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, latest_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, latest_data_desc),  
  
   // Thresholds 0xFFF2
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(thresholds_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, thresholds_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, thresholds_desc),  
 
   // Silence Alarm 0xFFF3
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(silence_alarm_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, silence_alarm_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, silence_alarm_desc),  
  
   // Maintenance 0xFFF4
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(maintenance_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, maintenance_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, maintenance_desc),  
  
   // Trend RR Data 0xFFF5
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(trend_rr_data_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, trend_rr_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, trend_rr_data_desc),  
  
   // Trend Act./Pos. Data 0xFFF6
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(trend_act_data_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, trend_act_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, trend_act_data_desc),  
 
   // Streaming Data 0xFFF7
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &StreamingProps),
  ATTRIBUTE16(streaming_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, streaming_data), 
  ATTRIBUTE16(clientCharCfgUUID, GATT_PERMIT_READ | GATT_PERMIT_WRITE, (uint8 *)&streaming_data_config),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, streaming_desc),  
  
   // Streaming Config 0xFFF8
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(streaming_config_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, streaming_config_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, streaming_config_desc),  
  
   // Error Codes 0xFFF9
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(error_codes_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, error_codes_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, error_codes_desc),
  
   // Connection Tone 0xFFFA
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(connection_tone_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, &dummy),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, connection_tone_desc),
  
   // Settings 0xFFFB
  ATTRIBUTE16(characterUUID, GATT_PERMIT_READ, &AttrProps),
  ATTRIBUTE16(settings_uuid, GATT_PERMIT_READ | GATT_PERMIT_WRITE, settings_data),
  ATTRIBUTE16(charUserDescUUID, GATT_PERMIT_READ, settings_desc),
  
};
  

/*===========================================================================
 * LOCAL FUNCTIONS
 *=========================================================================*/
static bStatus_t PMD2_readAttrHandler(  uint16 connHandle, gattAttribute_t *pAttr,
					uint8 *pValue, uint8 *pLen, uint16 offset,
					uint8 maxLen, uint8 method );
static bStatus_t PMD2_writeAttrHandler( uint16 connHandle, gattAttribute_t *pAttr,
					uint8 *pValue, uint8 len, uint16 offset,
					uint8 method);

/*===========================================================================
 * PROFILE CALLBACKS
 *=========================================================================*/
// Service callbacks registered to GATT 
CONST gattServiceCBs_t PMD2s =
{
  PMD2_readAttrHandler,         // Read callback
  PMD2_writeAttrHandler,        // Write callback
  NULL                          // Authorization callback
};


/*===========================================================================
 * PUBLIC FUNCTIONS
 *=========================================================================*/

/*===========================================================================
 * @fn      PMD_Service2_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 *=========================================================================*/

bStatus_t PMD_Service2_AddService(void)
{
  uint8 status = SUCCESS;
  
  // Allocate Client Characteristic Configuration table
  streaming_data_config = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                              linkDBNumConns );
  if ( streaming_data_config == NULL )
  {     
    return ( bleMemAllocError );
  }
   
  // Initialize Client Characteristic Configuration Attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, streaming_data_config );

  // Register GATT attribute list and callbacks with GATT Server App
  status = GATTServApp_RegisterService( pmd2AttrTbl, GATT_NUM_ATTRS( pmd2AttrTbl ),
					GATT_MAX_ENCRYPT_KEY_SIZE, &PMD2s );
  
  return (status);
}


/*===========================================================================
 * @fn      PMD_Service2_RegisterAppCBs
 *
 * @brief   Registers the application callback function.
 *          Only call this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  Success or bleAlreadyInRequestedMode
 *=========================================================================*/
bStatus_t PMD_Service2_RegisterAppCBs( PMD_Service2_CBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    PMD_Service2_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*===========================================================================
 * @fn          PMD2_ReadAttrCB
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
static bStatus_t PMD2_readAttrHandler( uint16 connHandle, gattAttribute_t *pAttr, 
					uint8 *pValue, uint8 *pLen, uint16 offset,
					uint8 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;
  static uint8 trend_rr_counter = 0;
  static uint8 trend_act_counter = 0;

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
      case LATEST_DATA_UUID:                            // Latest Data
        *pLen = LATEST_DATA_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        
        P0_4 = 0;  //CS PIN
        spiWriteByte(Connection_Complete_SPI);          // send Connection Complete to DSP
        P0_4 = 1;  //CS PIN   
        
        if CHECK_BIT(latest_data[5], 0)                 // Auto-silence Alarm
        {
          P0_4 = 0;  //CS PIN
          spiWriteByte(Silence_Alarm_SPI);
          P0_4 = 1;  //CS PIN
        }
        trend_rr_counter = 0;
        trend_act_counter = 0;
        break;
      
        
        
      case TREND_RR_DATA_UUID:                            // Trend Data - RR
        osal_memcpy(pValue, &trend_rr[trend_rr_counter*TREND_RR_DATA_LEN], TREND_RR_DATA_LEN);
        trend_rr_counter++;
        
        if( trend_rr_counter > (counter/TREND_RR_DATA_LEN) )
          trend_rr_counter = 0;
        break;
        
      case TREND_ACT_DATA_UUID:                            // Trend Data - Act. / Body Pos.
        osal_memcpy(pValue, &trend_act[trend_act_counter*TREND_ACT_DATA_LEN], TREND_ACT_DATA_LEN);
                
        if( trend_act_counter > (counter/TREND_ACT_DATA_LEN) )
          trend_act_counter = 0;
        break;
      
        
        
      case ERROR_CODES_UUID:                            // Error Codes
        *pLen = ERROR_CODES_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case THRESHOLDS_UUID:                             // put in to verify being written - remove from end product
        *pLen = THRESHOLDS_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case STREAMING_UUID:                            // Streaming Packet
        *pLen = STREAMING_LEN;
        osal_memcpy(pValue, pAttr->pValue, *pLen);
        break;
        
      case CONNECTION_TONE_UUID:                      // Connection Tone
          P0_4 = 0;  //CS PIN
          spiWriteByte(Connection_Complete_SPI);
          P0_4 = 1;  //CS PIN
        break;
        
      case SETTINGS_UUID:                            // Device Settings
        *pLen = SETTINGS_LEN;
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
 * @fn      PMD2_WriteAttrCB
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
static bStatus_t PMD2_writeAttrHandler( uint16 connHandle, gattAttribute_t *pAttr,
					uint8 *pValue, uint8 len, uint16 offset,
					uint8 method )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
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
      case THRESHOLDS_UUID:                     // Set Thresholds
        osal_memcpy(pAttr->pValue, pValue, len);
        
        if (len == 2)
        { 
          P0_4 = 0;  //CS PIN
          spiWriteByte(Thresholds_SPI);         // send THRESHOLDS TO DSP
          spiWriteByte(pValue[0]);              //      upper     
          spiWriteByte(pValue[1]);              //      lower
          P0_4 = 1;  //CS PIN
        }
        break;
        
      case SILENCE_ALARM_UUID:                  // Silence Alarm
        if (!STREAMING)
        {
          P0_4 = 0;  //CS PIN
          spiWriteByte(Silence_Alarm_SPI);
          P0_4 = 1;  //CS PIN
        }
        else
        {
          FPC_ALARM = 1;
        }
        break;
        
      case MAINTENANCE_UUID:                    // Maintenance Routine
        if (!STREAMING)
        {
          if(len == 1)                      
          {
            P0_4 = 0;  //CS PIN
            spiWriteByte(Maintenance_SPI);
            spiWriteByte(pValue[0]);
            P0_4 = 1;  //CS PIN
            
            if(pValue[0] == 0x02)             // make device UNDISCOVERABLE
            {         
              uint8 adv_filter = GAP_FILTER_POLICY_WHITE;
              GAPRole_SetParameter( GAPROLE_ADV_FILTER_POLICY, sizeof( uint8 ), &adv_filter );
            }
          }
         }
        else
        {
          if(len ==1)
          {
            MAINTENANCE_FLAG = 1;
            maintenanceValue = pValue[0];
          }
        }
        break;
        

      case STREAMING_CONFIG_UUID:               // Streaming Configuration
        osal_memcpy(pAttr->pValue, pValue, len);
        
        if (pValue[0] == 0)
        {
          P0_4 = 0;  //CS PIN
          spiWriteByte(Streaming_Stop_SPI);     // Streaming OFF
          P0_4 = 1;  //CS PIN
          STREAMING_FLAG=0;
        }
        if (pValue[0] == 1)
        {
          P0_4 = 0;  //CS PIN
          spiWriteByte(Streaming_Start_SPI);     // Streaming ON
          P0_4 = 1;  //CS PIN
          STREAMING_FLAG=1;
          
          notifyApp = STREAMING_CONFIG;
        }
        break;
  
        
      case SETTINGS_UUID:                         // Settings
        if (len == 2)
        {
          osal_memcpy(pAttr->pValue, pValue, len);
          
          P0_4 = 0;  //CS PIN
          spiWriteByte(Settings_SPI);             // send settings to DSP
          spiWriteByte(pValue[0]);
          spiWriteByte(pValue[1]);
          P0_4 = 1;  //CS PIN
        }
        break;
 
        
      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
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
  
  if ( (notifyApp != 0xFF ) && PMD_Service2_AppCBs && PMD_Service2_AppCBs->
                                                        pfnPMD_Service2_Change )
  {
    PMD_Service2_AppCBs->pfnPMD_Service2_Change( notifyApp );
  }

  return ( status );
}

/*===========================================================================
 * @fn      PMD2_SetParameter
 *
 * @brief   Set a PMD Service 2 parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 *=========================================================================*/
bStatus_t PMD2_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
     case STREAMING:
      if ( len == STREAMING_LEN ) 
      {
        VOID osal_memcpy( streaming_data, value, STREAMING_LEN );
      
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( streaming_data_config, streaming_data,
                                    FALSE, pmd2AttrTbl,
                                    GATT_NUM_ATTRS( pmd2AttrTbl ),
                                    INVALID_TASK_ID, PMD2_readAttrHandler );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*===========================================================================
 * @fn      Get_Latest_Data
 *
 * @brief   Retrieve latest data from DSP via SPI when interrupt detected.
 *
 * @param   none
 *
 * @return  none
 *=========================================================================*/
void Get_Latest_Data(void)
{
  uint8 advertising_enable = FALSE;
  
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising_enable );
    
  P0_4 = 0;  //CS PIN
  spiWriteByte(Latest_Data_SPI);
  for (int i=0; i<LATEST_DSP_DATA_LEN; i++)
    latest_dsp_data[i] = spiReadByte(dummyData);
  P0_4 = 1;  //CS PIN
  
  if CHECK_BIT(latest_dsp_data[2], 2)
  {
    P0_4 = 0;  //CS PIN
    spiWriteByte(Error_Codes_SPI);
    for (int i=0; i<ERROR_CODES_LEN; i++)
      error_codes_data[i] = spiReadByte(dummyData);
    P0_4 = 1;  //CS PIN
  }
  
  if CHECK_BIT(latest_dsp_data[2], 3)
  {
    P0_4 = 0;  //CS PIN
    spiWriteByte(Trend_Data_SPI);
    for (int i=0; i<2; i++)
      latest_trend_data[i] = spiReadByte(dummyData);
    P0_4 = 1;  //CS PIN
  
    trend_rr[counter] = latest_trend_data[0];
    trend_act[counter] = latest_trend_data[1];
    counter++;
    latest_data[LATEST_DATA_LEN-2] = LO_UINT16(counter);
    latest_data[LATEST_DATA_LEN-1] = HI_UINT16(counter);
  }
  
  if CHECK_BIT(latest_dsp_data[2], 4)
  {
    P0_4 = 0;  //CS PIN
    spiWriteByte(Battery_SPI);
    BatteryLevel = spiReadByte(dummyData);
    P0_4 = 1;  //CS PIN
  }
  
  if (BatteryLevel > BATTERY_LOW)
      latest_dsp_data[2] &= ~(1<<4);
  else
    latest_dsp_data[2] |= (1<<4);
  
  latest_data[0] = latest_dsp_data[0];
  latest_data[1] = latest_dsp_data[1];
  latest_data[2] = thresholds_data[0];
  latest_data[3] = thresholds_data[1];
  latest_data[4] = BatteryLevel;
  latest_data[5] = latest_dsp_data[2];
  
  
  advertisement_data[0] = 0x07;
  advertisement_data[1] = GAP_ADTYPE_FLAGS;
  advertisement_data[2] = (GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED);    // or use GAP_ADTYPE_FLAGS_GENERAL for indefinite advertising
  advertisement_data[3] = latest_dsp_data[0];
  advertisement_data[4] = latest_dsp_data[1];
  advertisement_data[5] = thresholds_data[0];
  advertisement_data[6] = thresholds_data[1];
  advertisement_data[7] = latest_dsp_data[2];
  
  GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, 1600);  // desired_min_advertising_interval
  GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, 2000);  // desired_max_advertising_interval
  
  advertising_enable = TRUE;
  
  GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertisement_data ), advertisement_data );
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising_enable );
    
}


/*===========================================================================
 * @fn      Get_Streaming_Data
 *
 * @brief   Retrieve streaming data from DSP via SPI when interrupt detected.
 *
 * @param   none
 *
 * @return  none
 *=========================================================================*/
void Get_Streaming_Data( void )
{
  PMD_GetData(Streaming_SPI, STREAMING_LEN);
  PMD2_SetParameter( STREAMING, STREAMING_LEN, pmdStreamValue );
  
  if (FPC_ALARM)
  {
    P0_4 = 0;  //CS PIN
    spiWriteByte(Silence_Alarm_SPI);
    P0_4 = 1;  //CS PIN
    
    FPC_ALARM=0;
  }

  if (MAINTENANCE_FLAG)
  {
    P0_4 = 0;  //CS PIN
    spiWriteByte(Maintenance_SPI);
    spiWriteByte(maintenanceValue);
    P0_4 = 1;  //CS PIN
    
    if(maintenanceValue == 0x02)             // make device UNDISCOVERABLE
    {         
      uint8 adv_filter = GAP_FILTER_POLICY_WHITE;
      GAPRole_SetParameter( GAPROLE_ADV_FILTER_POLICY, sizeof( uint8 ), &adv_filter );
    }
    
    MAINTENANCE_FLAG =0;
  }
}


/*===========================================================================
 * @fn      PMD_GetData
 *
 * @brief   Retrieve bytes of data from DSP via SPI
 *
 * @param   spi_flag - SPI number to communicate over
 * @param   num - number of bytes to read over SPI
 *
 * @return  none
 *=========================================================================*/
void PMD_GetData(int spi_flag, int num )
{
  P0_4 = 0;  //CS PIN
  spiWriteByte(spi_flag);

  for( int i=0 ; i<num ; i++)
    pmdStreamValue[i] = spiReadByte(dummyData);
  BatteryLevel = pmdStreamValue[num-1];
  
  P0_4 = 1;  //CS PIN
}


/*===========================================================================
 * @fn      PMD_INT_init
 *
 * @brief   Configure Bluetooth Module for interrupt.
 *
 * @param   none
 *
 * @return  none
 *=========================================================================*/
extern  void PMD_INT_init(void)
{
  EA = 0;
  P1SEL |= (0x04);
  P1DIR &= ~(0x04);
  P1INP |= (0x04);
  P1IEN |= (0x04);
  PICTL |= (0x02); // interrupt on Falling Edge   
  IEN2 |= (0x10);
  P1IFG = 0;
  P1IF = 0;
  EA = 1;
}


/*===========================================================================
 * @fn      HAL_ISR_FUNCTION
 *
 * @brief   Interrupt Service Routine.
 *
 * @param   P1_2_ISR - interrupt for Port 1_2
 * @param   P1INT_VECTOR - interrupt vector for Port 1
 *
 * @return  none
 *=========================================================================*/
HAL_ISR_FUNCTION( P1_2_ISR, P1INT_VECTOR )
{
  HAL_ENTER_ISR();
  
  if (!STREAMING_FLAG)
  {
    Get_Latest_Data();
  }
  else if (STREAMING_FLAG)
  {
    Get_Streaming_Data();
  }
  
  // do routine here - LED just for visual indication
  //cbLED_flash(cbLED_GREEN, 1, 250, 500);

  P1IFG = 0;  
  P1IF = 0;
  
  HAL_EXIT_ISR();
  
}
