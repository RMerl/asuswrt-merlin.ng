/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : PHY 8226 Driver
 *
 * Feature : PHY 8226 Driver
 *
 */
#ifndef __NIC_RTL8226_TYPEDEF_H__
#define __NIC_RTL8226_TYPEDEF_H__

/* from typedef.h and rtl8156_mmd.h */

#define BIT_0       0x0001
#define BIT_1       0x0002
#define BIT_2       0x0004
#define BIT_3       0x0008
#define BIT_4       0x0010
#define BIT_5       0x0020
#define BIT_6       0x0040
#define BIT_7       0x0080
#define BIT_8       0x0100
#define BIT_9       0x0200
#define BIT_10      0x0400
#define BIT_11      0x0800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000

#define SUCCESS     0
#define FAILURE     -1


//ODM modify
#if 0
typedef struct {
    uint32      unit;
    uint32      port;
} HANDLE;
#endif


#ifndef bool
  #define bool          int
#endif

#define BOOLEAN         bool
#define BOOL            uint32
#define UINT32          uint32
#define UINT16          uint16
#define UINT8           uint8
//#define Sleep(_t)       osal_time_udelay(_t*1000)
#define IN
#define OUT


#define MMD_PMAPMD     1
#define MMD_PCS        3
#define MMD_AN         7
#define MMD_VEND1      30   /* Vendor specific 2 */
#define MMD_VEND2      31   /* Vendor specific 2 */


typedef struct
{
    UINT16 dev;
    UINT16 addr;
    UINT16 value;
} MMD_REG;


BOOLEAN
MmdPhyRead(
    IN  HANDLE hDevice,
    IN  UINT16 dev,
    IN  UINT16 addr,
    OUT UINT16 *data);

BOOLEAN
MmdPhyWrite(
    IN HANDLE hDevice,
    IN UINT16 dev,
    IN UINT16 addr,
    IN UINT16 data);




#endif /* __NIC_RTL8226_TYPEDEF_H__ */


