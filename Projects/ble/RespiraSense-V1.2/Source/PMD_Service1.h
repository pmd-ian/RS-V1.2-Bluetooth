/*===========================================================================
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

?? add time of application of device???

 *=========================================================================*/

#ifndef PMD_SERVICE1_H
#define PMD_SERVICE1_H

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

// PMD Service 1 UUID
#define PMD_SERVICE1_UUID       0xFFE0
    
// Characteristic UUIDs
#define FIRMWARE_UUID   0xFFE1
#define RENAME_UUID     0xFFE2
#define NAME_UUID       0xFFE3
#define SURNAME_UUID    0xFFE4
#define GENDER_UUID     0xFFE5
#define DoB_UUID        0xFFE6
#define DoA_UUID        0xFFE7
#define BED_UUID        0xFFE8
#define ROOM_UUID       0xFFE9
#define WARD_UUID       0xFFEA


// Profile Parameters
#define FIRMWARE        0
#define RENAME          1
#define NAME            2
#define SURNAME         3
#define GENDER          4
#define DoB             5
#define DoA             6
#define BED             7
#define ROOM            8
#define WARD            9
  
  
// Length of Characteristic's in bytes
#define FIRMWARE_LEN   4
#define RENAME_LEN     20
#define NAME_LEN       20
#define SURNAME_LEN    20
#define GENDER_LEN     1
#define DoB_LEN        3        // byte1=day, byte2=month, byte3=(year-1900)
#define DoA_LEN        3        // byte1=day, byte2=month, byte3=(year-1900)
#define BED_LEN        1
#define ROOM_LEN       20
#define WARD_LEN       20


// Firmware Revisions
#define Firmware_Revision       0       // Release Revision
#define Firmware_Eng_Rev        2       // Engineering Revision
  

// SPI Flags
#define Firmware_SPI    0x02
#define Rename_SPI      0x04  
  

// Other
#define dummyData       0xDD
   
/*===========================================================================
 * TYPES
 *=========================================================================*/



/*===========================================================================
 * FUNCTIONS
 *=========================================================================*/
extern bStatus_t PMD_Service1_AddService(void);
void Get_Firmware(void);

#ifdef __cplusplus
}
#endif

#endif /* PMD_SERVICE1_H */
