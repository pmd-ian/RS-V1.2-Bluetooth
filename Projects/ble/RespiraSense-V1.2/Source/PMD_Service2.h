/**************************************************************************************************
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

**************************************************************************************************/

#ifndef PMD_SERVICE2_H
#define PMD_SERVICE2_H

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================
 * INCLUDES
 *=========================================================================*/


/*===========================================================================
 * #DEFINES
 *=========================================================================*/

// PMD Service 2 UUID
#define PMD_SERVICE2_UUID       0xFFF0
    
// Characteristic UUIDs
#define LATEST_DATA_UUID        0xFFF1
#define THRESHOLDS_UUID         0xFFF2
#define SILENCE_ALARM_UUID      0xFFF3
#define MAINTENANCE_UUID        0xFFF4
#define TREND_RR_DATA_UUID      0xFFF5
#define TREND_ACT_DATA_UUID     0xFFF6
#define STREAMING_UUID          0xFFF7
#define STREAMING_CONFIG_UUID   0xFFF8
#define ERROR_CODES_UUID        0xFFF9
#define CONNECTION_TONE_UUID    0xFFFA
#define SETTINGS_UUID           0xFFFB


// Profile Parameters
#define LATEST_DATA             0
#define THRESHOLDS              1
#define SILENCE_ALARM           2
#define MAINTENANCE             3
#define TREND_RR_DATA           4
#define TREND_ACT_DATA          5
#define STREAMING               6
#define STREAMING_CONFIG        7
#define ERROR_CODES             8
#define CONNECTION_TONE         9
#define SETTINGS                10

  
// Length of Characteristic's in bytes
#define LATEST_DATA_LEN         8
#define THRESHOLDS_LEN          2
#define SILENCE_ALARM_LEN       1
#define MAINTENANCE_LEN         1
#define TREND_RR_DATA_LEN       20
#define TREND_ACT_DATA_LEN      20
#define STREAMING_LEN           14
#define STREAMING_CONFIG_LEN    1
#define ERROR_CODES_LEN         3
#define SETTINGS_LEN            2

   
// SPI Flags
#define Maintenance_SPI         0x01
#define Battery_SPI             0x03     
#define Error_Codes_SPI         0x05
#define Silence_Alarm_SPI       0x06     
#define Thresholds_SPI          0x07
#define Streaming_Start_SPI     0x08
#define Streaming_SPI           0x09
#define Streaming_Stop_SPI      0x0A
#define Latest_Data_SPI         0x0B
#define Trend_Data_SPI          0x0C
#define Connection_Complete_SPI 0x0D
#define Settings_SPI            0x0E  
 
  
// Check to determine if particular bits are set
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

 
// Other
#define LATEST_DSP_DATA_LEN     3
#define BATTERY_LOW             30
#define dummyData               0xDD
   
  
/*===========================================================================
 * TYPES
 *=========================================================================*/

/*===========================================================================
 * Profile Callbacks
 *=========================================================================*/
typedef void (*PMD_Service2_Change_t)( uint8 paramID );

typedef struct
{
  PMD_Service2_Change_t        pfnPMD_Service2_Change;  // Called when characteristic value changes
} PMD_Service2_CBs_t;
   
   
/*===========================================================================
 * FUNCTIONS
 *=========================================================================*/
extern bStatus_t PMD_Service2_AddService(void);
extern bStatus_t PMD_Service2_RegisterAppCBs( PMD_Service2_CBs_t *appCallbacks );
extern bStatus_t PMD2_SetParameter( uint8 param, uint8 len, void *value );
extern  void PMD_INT_init(void);
void Get_Latest_Data(void);
void PMD_GetData(int spi_flag, int num);
void Get_Streaming_Data(void);

#ifdef __cplusplus
}
#endif

#endif /* PMD_SERVICE2_H */
