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
#ifndef __NIC_RTL8226_H__
#define __NIC_RTL8226_H__

// #include <hal/phy/nic_rtl8226/rtl8226_typedef.h>
#include "bcmtypes.h"
#include "rtl8226_typedef.h"

#define NO_LINK 0
#define LINK_SPEED_10M 10
#define LINK_SPEED_100M 100
#define LINK_SPEED_500M 500
#define LINK_SPEED_1G 1000
#define LINK_SPEED_2P5G 2500

typedef enum
{
    PHY_CROSSPVER_MODE_AUTO = 0,
    PHY_CROSSPVER_MODE_MDI,
    PHY_CROSSPVER_MODE_MDIX,
    PHY_CROSSPVER_MODE_END
} PHY_CROSSPVER_MODE;

typedef enum
{
    PHY_CROSSPVER_STATUS_MDI = 0,
    PHY_CROSSPVER_STATUS_MDIX,
    PHY_CROSSPVER_STATUS_END
} PHY_CROSSPVER_STATUS;

typedef enum
{
    PHY_AUTO_MODE = 0,
    PHY_SLAVE_MODE,
    PHY_MASTER_MODE,
    PHY_MASTER_SLAVE_END
} PHY_MASTERSLAVE_MODE;

typedef struct
{
    UINT32 Half_10:1;
    UINT32 Full_10:1;

    UINT32 Half_100:1;
    UINT32 Full_100:1;

    UINT32 Full_1000:1;

    UINT32 adv_2_5G:1;

    UINT32 FC:1;
    UINT32 AsyFC:1;
} PHY_LINK_ABILITY;

typedef struct
{
    UINT8 EEE_100:1;
    UINT8 EEE_1000:1;
    UINT8 EEE_2_5G:1;
} PHY_EEE_ENABLE;

typedef struct
{
    UINT8 TX_SWAP:1;
    UINT8 RX_SWAP:1;
} PHY_SERDES_POLARITY_SWAP;

typedef enum
{
	TESTMODE_CHANNEL_NONE = 0,
	TESTMODE_CHANNEL_A,
	TESTMODE_CHANNEL_B,
	TESTMODE_CHANNEL_C,
	TESTMODE_CHANNEL_D,
	TESTMODE_CHANNEL_END
} PHY_TESTMODE_CHANNEL;

typedef struct
{
    UINT32 TM1:1;
    UINT32 TM2:1;
    UINT32 TM3:1;
    UINT32 TM4:1;
    UINT32 TM5:1;
    UINT32 TM6:1;

    UINT32 TONE1:1;
    UINT32 TONE2:1;
    UINT32 TONE3:1;
    UINT32 TONE4:1;
    UINT32 TONE5:1;

    UINT32 TMFINISH:1;

	PHY_TESTMODE_CHANNEL channel:3;

} PHY_IEEE_TEST_MODE;

typedef enum
{
    MIS_MATCH_OPEN = 1, // Mis-Match_Open, larger_than_130ohm
    MIS_MATCH_SHORT = 2, // Mis-Match_short, less_than_77ohm
} PHY_RTCT_STATUS_MISMATCH;

typedef struct
{
    BOOL Open;
    BOOL Short;
    PHY_RTCT_STATUS_MISMATCH Mismatch;
} PHY_RTCT_STATUS;

typedef struct
{
    UINT32 rxLen;
    UINT32 txLen;

    UINT32 channelALen;
    UINT32 channelBLen;
    UINT32 channelCLen;
    UINT32 channelDLen;

    PHY_RTCT_STATUS channelAStatus;
    PHY_RTCT_STATUS channelBStatus;
    PHY_RTCT_STATUS channelCStatus;
    PHY_RTCT_STATUS channelDStatus;
} PHY_RTCT_RESULT;

typedef enum
{
    PHY_SERDES_MODE_OTHER = 0,
    PHY_SERDES_MODE_SGMII,
    PHY_SERDES_MODE_HiSGMII,
    PHY_SERDES_MODE_2500BASEX,
    PHY_SERDES_MODE_USXGMII,
    PHY_SERDES_MODE_NO_SDS,
    PHY_SERDES_MODE_END
} PHY_SERDES_MODE;



BOOLEAN
Rtl8226_phy_reset(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8226_autoNegoEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_autoNegoEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_autoNegoAbility_get(
    IN  HANDLE hDevice,
    OUT PHY_LINK_ABILITY *pPhyAbility
    );

BOOLEAN
Rtl8226_autoNegoAbility_set(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pPhyAbility
    );

BOOLEAN
Rtl8226_duplex_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_duplex_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_is_link(
    IN  HANDLE hDevice,
    OUT BOOL *plinkok
    );

BOOLEAN
Rtl8226_speed_get(
    IN  HANDLE hDevice,
    OUT UINT16 *pSpeed
    );

BOOLEAN
Rtl8226_force_speed_set(
    IN HANDLE hDevice,
    IN UINT16 Speed
    );

BOOLEAN
Rtl8226_enable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_greenEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_greenEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_eeeEnable_get(
    IN  HANDLE hDevice,
    OUT PHY_EEE_ENABLE *pEeeEnable
    );

BOOLEAN
Rtl8226_eeeEnable_set(
    IN HANDLE hDevice,
    IN PHY_EEE_ENABLE *pEeeEnable
    );

BOOLEAN
Rtl8226_crossOverMode_get(
    IN  HANDLE hDevice,
    OUT PHY_CROSSPVER_MODE *CrossOverMode
    );

BOOLEAN
Rtl8226_crossOverMode_set(
    IN HANDLE hDevice,
    IN PHY_CROSSPVER_MODE CrossOverMode
    );

BOOLEAN
Rtl8226_crossOverStatus_get(
    IN  HANDLE hDevice,
    OUT PHY_CROSSPVER_STATUS *pCrossOverStatus
    );

BOOLEAN
Rtl8226_masterSlave_get(
    IN  HANDLE hDevice,
    OUT PHY_MASTERSLAVE_MODE *MasterSlaveMode
    );

BOOLEAN
Rtl8226_masterSlave_set(
    IN HANDLE hDevice,
    IN PHY_MASTERSLAVE_MODE MasterSlaveMode
    );

BOOLEAN
Rtl8226_loopback_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_loopback_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_downSpeedEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_downSpeedEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_gigaLiteEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_gigaLiteEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_mdiSwapEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_mdiSwapEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_rtct_start(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8226_rtctResult_get(
    IN HANDLE hDevice,
    OUT PHY_RTCT_RESULT *pRtctResult
    );

BOOLEAN
Rtl8226_linkDownPowerSavingEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_linkDownPowerSavingEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_2p5gLiteEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_2p5gLiteEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_ThermalSensorEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    );

BOOLEAN
Rtl8226_ThermalSensorEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

BOOLEAN
Rtl8226_ieeeTestMode_set(
    IN HANDLE hDevice,
    IN UINT16 Speed,
    IN PHY_IEEE_TEST_MODE *pIEEEtestmode
    );

BOOLEAN
Rtl8226_serdes_rst(
    IN HANDLE hDevice
    );

BOOLEAN
Rtl8226_serdes_link_get(
    IN  HANDLE hDevice,
    OUT BOOL *perdesLink,
    OUT PHY_SERDES_MODE *SerdesMode
    );

BOOLEAN
Rtl8226_serdes_option_set(
    IN HANDLE hDevice,
    IN UINT8 functioninput
    );

BOOLEAN
Rtl8226_serdes_polarity_swap(
    IN HANDLE hDevice,
    IN PHY_SERDES_POLARITY_SWAP *ppolarityswap
    );

BOOLEAN
Rtl8226_serdes_autoNego_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    );

#endif /* __NIC_RTL8226_H__ */

