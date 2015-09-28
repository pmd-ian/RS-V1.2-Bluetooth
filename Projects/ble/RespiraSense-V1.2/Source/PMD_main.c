/*===========================================================================*
  Filename:     PMD_main.c
  Date:         2015-08-04
  Revision:     Rev: 1
  Written by:   Ian Martin

  Description:  Application based on the keyfob demo from TI.
                Functionality have been changed to fit the RespiraSense PCB.

 *=========================================================================*/

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "hal_types.h"

#include "PMD_hw.h"
#include "gatt.h"
#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "linkdb.h"

#include "peripheral.h"

#include "PMD_main.h"


// Services
#include "gapbondmgr.h"
#include "PMD_Service1.h"
#include "PMD_Service2.h"



/*===========================================================================
* DEFINES
*=========================================================================*/

// Delay between power-up and starting advertising (in ms)
#define STARTDELAY                    500

//GAP Peripheral Role desired connection parameters

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Use limited discoverable mode to advertise for 30.72s, and then stop, or 
// use general discoverable mode to advertise indefinitely 
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
//#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     16

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     32  //32

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000


// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                              0x000D


/*===========================================================================
* TYPES
*=========================================================================*/

typedef struct 
{
  uint8             taskId;
  gaprole_States_t  gapProfileState;
} PMD_Class;


uint8 PMDtaskId;

/*===========================================================================
* DECLARATIONS
*=========================================================================*/

void _NOP(void){asm("NOP");}
static void gapApplicationInit(void);
static void processOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void passcodeCB(uint8 *deviceAddr, uint16 connectionHandle, uint8 uiInputs, uint8 uiOutputs);
static void pairStateCB( uint16 connHandle, uint8 state, uint8 status );
static void PMD_Service2_ChangeCB( uint8 paramID );

/*===========================================================================
* DEFINITIONS
*=========================================================================*/
static PMD_Class pmd;

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertising)
static uint8 advertData[] = 
{ 
  0x02,   // length of first data structure (2 bytes excluding length byte)
  GAP_ADTYPE_FLAGS,   // AD Type = Flags
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
};

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 deviceName[21] =
{
  // complete name  
  0x0C,   // length of first data structure 
  0x09,   // AD Type = Complete local name  
  'P',
  'M',
  'D',
  ' ',
  'R',
  '2',
  '-',
  '0',    // Will be replaced with number from the device address
  '0',    // Will be replaced with number from the device address
  '0',    // Will be replaced with number from the device address
  '0',    // Will be replaced with number from the device address
};

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 deviceNameNew[20] =
{
  // complete name  
  0x0C,   // length of first data structure 
  0x09,   // AD Type = Complete local name  
  'P',
  'M',
  'D',
  ' ',
  'R',
  '2',
  '-',
  '0',    // Will be replaced with number from the device address
  '0',    // Will be replaced with number from the device address
  '0',    // Will be replaced with number from the device address
  '0',    // Will be replaced with number from the device address
};

// GAP GATT Attributes
static uint8 attDeviceName[21] = "OLP425-0000";

static uint8 attDeviceNameNew[21] = "OLP425-0000";

// GAP Role callbacks
static gapRolesCBs_t peripheralRoleCallbacks =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller
};

// GAP Bond Manager callbacks
static gapBondCBs_t bondMgrCallbacks =
{
  passcodeCB,                     // Passcode callback
  pairStateCB                     // Pairing / Bonding state Callback
};

// Simple GATT Profile Callbacks
static PMD_Service2_CBs_t pmd_service2_CBs =
{
  PMD_Service2_ChangeCB    // Charactersitic value change callback
};

/*===========================================================================
* FUNCTIONS
*=========================================================================*/

/*---------------------------------------------------------------------------
* Initialization function for the Demo application task.
* This is called during initialization and should contain
* any application specific initialization.
* -task_id: The ID assigned by OSAL.  This ID should be
*            used to send messages and set timers.
*-------------------------------------------------------------------------*/
void PMD_init( uint8 taskId )
{   
  pmd.taskId = taskId;
  pmd.gapProfileState = GAPROLE_INIT;
  
  PMDtaskId = taskId;
  
  gapApplicationInit();

  // initialize leds here
  
  // Initialize GATT attributes
  PMD_Service1_AddService();
  PMD_Service2_AddService();
  VOID PMD_Service2_RegisterAppCBs( &pmd_service2_CBs );

  // Setup a delayed profile startup
  osal_start_timerEx(pmd.taskId, PMD_START_DEVICE_EVT, STARTDELAY);
}


/*---------------------------------------------------------------------------
* Initialization of GAP properties. 
*-------------------------------------------------------------------------*/
static void gapApplicationInit(void)
{
  // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
  uint8 initial_advertising_enable = TRUE;
  // By setting this to zero, the device will go into the waiting state after
  // being discoverable for 30.72 second, and will not being advertising again
  // until the enabler is set back to TRUE
  uint16 gapRole_AdvertOffTime = 0;

  uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
  uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
  uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
  uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
  uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

  // Set the GAP Role Parameters
  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
  GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

  GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( deviceName ), deviceName );
  GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

  GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
  GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
  GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
  GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
  GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );

  // Set the GAP Attributes
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Setup the GAP Bond Manager
  {
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }  
  
  // Set max output power and reciever gain
  HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_4_DBM);
  HCI_EXT_SetRxGainCmd(HCI_EXT_RX_GAIN_HIGH);
}

/*---------------------------------------------------------------------------
* Passcode callback, will be called during pairing. 
* The passcode is set to "0".
*-------------------------------------------------------------------------*/
static void passcodeCB( uint8 *deviceAddr, uint16 connectionHandle, uint8 uiInputs, uint8 uiOutputs )
{    
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, 0 );
}

/*---------------------------------------------------------------------------
* Callback indicating the current state of pairing.
*-------------------------------------------------------------------------*/
static void pairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  if ( state == GAPBOND_PAIRING_STATE_STARTED )
  {    
    //      
  }
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )
  {
    if ( status == SUCCESS )
    {
      linkDBItem_t  *pItem;

      if ( (pItem = linkDB_Find(connHandle )) != NULL )
      {
        if (( (pItem->stateFlags & LINK_BOUND) == LINK_BOUND ))
        {
          //cbLED_flash(cbLED_GREEN, 3, 250, 500);             
        }
      }      
    }
    else
    {
      //     
    }
  }
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )
  {
    //cbLED_flash(cbLED_GREEN, 5, 250, 500);               
  }
}

/*---------------------------------------------------------------------------
* Application event processor.  This function
* is called to process all events for the task. Events
* include timers, messages and any other user defined events.
* - task_id: The OSAL assigned task ID.
* - events: Events to process.  This is a bit map and can contain more 
*           than one event.
*-------------------------------------------------------------------------*/
uint16 PMD_processEvent( uint8 taskId, uint16 events )
{
  
  if ( events & START_STREAMING)
  {
    attHandleValueNoti_t nData2;
    nData2.len = 20;
    nData2.handle = 20;
    getNameWithAddressInfo(nData2);
    
    GATT_Notification( 0, &nData2, FALSE );

    return (events ^ START_STREAMING);
  }
  
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( pmd.taskId )) != NULL )
    {
      processOSALMsg( (osal_event_hdr_t *)pMsg );

      VOID osal_msg_deallocate( pMsg );
    }

    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & PMD_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &peripheralRoleCallbacks );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &bondMgrCallbacks );       

    
    // Flash red LED twice
    //cbLED_flash(cbLED_RED, 2, 250, 500);
    PMD_INT_init();

    return ( events ^ PMD_START_DEVICE_EVT );    
  }


  // Discard unknown events
  return 0;
}

/*---------------------------------------------------------------------------
* Process an incoming task message.
* - pMsg: Message to process
*-------------------------------------------------------------------------*/
static void processOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
  case KEY_CHANGE:
    break;
    
  default:
      // do nothing!
    break;
  }
}

/*---------------------------------------------------------------------------
* Peripheral role of a state change handler.
* - newState: new state
*-------------------------------------------------------------------------*/
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  uint16 connHandle = INVALID_CONNHANDLE;

  if ( pmd.gapProfileState != newState )
  {
    switch( newState )
    {
    case GAPROLE_STARTED:
      {
        // Set the system ID from the bd addr
        uint8 systemId[8];              // from #define DEVINFO_SYSTEM_ID_LEN  8
        GAPRole_GetParameter(GAPROLE_BD_ADDR, systemId);

        // shift three bytes up
        systemId[7] = systemId[5];
        systemId[6] = systemId[4];
        systemId[5] = systemId[3];

        // set middle bytes to zero
        systemId[4] = 0;
        systemId[3] = 0;

        updateNameWithAddressInfo();        
      }
      break;      

    case GAPROLE_ADVERTISING:       
      break;

    case GAPROLE_CONNECTED:
      GAPRole_GetParameter( GAPROLE_CONNHANDLE, &connHandle );     

#if defined ( PLUS_BROADCASTER )
      osal_start_timerEx( pmd.taskId, PMD_ADV_IN_CONNECTION_EVT, ADV_IN_CONN_WAIT );
#endif
      break;

    case GAPROLE_WAITING:
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      break;

    default:
      // do nothing
      break;      
    }
  }

  pmd.gapProfileState = newState;
}


/*---------------------------------------------------------------------------
* Get character representing a hexadecimal digit.
*-------------------------------------------------------------------------*/
uint8 getAscii(uint8 value)
{
  char hex[] = "0123456789ABCDEF"; // needs error checking
  return hex[value];
}

/*---------------------------------------------------------------------------
* Add the set the last part of hte device name to the final bytes of the 
* device address. Useful when mane demo devices are located in the same room.
*-------------------------------------------------------------------------*/
void updateNameWithAddressInfo(void)
{
  //uint8 status;
  uint8 numberString[4];
  uint8 address[6];
  uint8 value;

  //status = GAPRole_GetParameter(GAPROLE_BD_ADDR, address);
  GAPRole_GetParameter(GAPROLE_BD_ADDR, address);

  value = (address[1] & 0xF0) >> 4;
  numberString[0] = getAscii(value);

  value = address[1] & 0x0F;
  numberString[1] = getAscii(value);     

  value = (address[0] & 0xF0) >> 4;
  numberString[2] = getAscii(value);

  value = address[0] & 0x0F;
  numberString[3] = getAscii(value);     

  // Replace "0000" part of "OLP425-0000"
  osal_memcpy(&attDeviceName[7], numberString, 4);
  osal_memcpy(&deviceName[9], numberString, 4);
  
  osal_memcpy(&attDeviceNameNew[7], numberString, 4);
  osal_memcpy(&deviceNameNew[9], numberString, 4);

//  status = GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( deviceName ), deviceName );
//  status = GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN - 1, attDeviceName );   

  GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( deviceName ), deviceName );
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN - 1, attDeviceName );   
}

/*---------------------------------------------------------------------------
* Add the set the last part of hte device name to the final bytes of the 
* device address. Useful when mane demo devices are located in the same room.
*-------------------------------------------------------------------------*/
void Rename_Device(uint8 *name)
{
  //uint8 status;

  osal_memcpy(&attDeviceName[0], name, 21);
  osal_memcpy(&deviceName[0], name, 21);

  //status = GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof (deviceName), deviceName);  

  //status = GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN - 1, attDeviceName );   
  
  GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof (deviceName), deviceName);  

  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN - 1, attDeviceName );   
}


/*---------------------------------------------------------------------------
* Add the set the last part of hte device name to the final bytes of the 
* device address. Useful when mane demo devices are located in the same room.
*-------------------------------------------------------------------------*/
void getNameWithAddressInfo(attHandleValueNoti_t data)
{
  
    osal_memcpy( &data, &deviceName[0], 20 );
}


/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void PMD_Service2_ChangeCB( uint8 paramID )
{
  switch( paramID )
  {
    case STREAMING:
      //
      break;

    default:
      // should not reach here!
      break;
  }
}