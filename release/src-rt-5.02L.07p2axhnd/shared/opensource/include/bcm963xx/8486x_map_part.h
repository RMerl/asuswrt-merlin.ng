/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef __BCM8486X_MAP_PART_H
#define __BCM8486X_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************
 * 8486x PHY Register Definitions                           *
 ************************************************************/
enum {
    CL45_CMD_RECEIVED       = 0x0,
    CL45_CMD_IN_PROGRESS    = 0x2,
    CL45_CMD_COMPLETE_PASS  = 0x4,
    CL45_CMD_COMPLETE_ERROR = 0x8,
    CL45_CMD_SYSTEM_BUSY    = 0xbbbb,
};

/* PHY Command */
#define CL45_CMD_GET_PAIR_SWAP      0x8000
#define CL45_CMD_SET_PAIR_SWAP      0x8001
#define CL45_CMD_GET_1588_ENABLE    0x8004
#define CL45_CMD_SET_1588_ENABLE    0x8005
#define CL45_CMD_GET_SHORT_REACH_MODE_ENABLE    0x8006
#define CL45_CMD_SET_SHORT_REACH_MODE_ENABLE    0x8007
#define CL45_CMD_GET_EEE_MODE       0x8008
#define CL45_CMD_SET_EEE_MODE       0x8009
#define CL45_CMD_GET_EMI_MODE_ENABLE    0x800a
#define CL45_CMD_SET_EMI_MODE_ENABLE    0x800b

#define CL45_CMD_GET_SUB_LF_RF_STATUS   0x800d
#define CL45_CMD_GET_KR_MODE_ENABLE     0x800e
#define CL45_CMD_SET_KR_MODE_ENABLE     0x800f
#define CL45_CMD_CLEAR_SUB_LF_RF        0x8010
#define CL45_CMD_SET_SUB_LF_RF          0x8011
#define CL45_CMD_READ_INDIRECT_GPHY_REG_BITS        0x8014
#define CL45_CMD_WRITE_INDIRECT_GPHY_REG_BITS       0x8015
#define CL45_CMD_GET_XFI_TX_FILTERS     0x802b
#define CL45_CMD_SET_XFI_TX_FILTERS     0x802c
#define CL45_CMD_GET_XFI_POLARITY       0x802d
#define CL45_CMD_SET_XFI_POLARITY       0x802e
#define CL45_CMD_GET_CURRENT_VOLTAGE    0x802f

#define CL45_CMD_GET_SNR                0x8030
#define CL45_CMD_GET_CURRENT_TEMP       0x8031
#define CL45_CMD_SET_UPPER_TEMP_WARNING_LEVEL   0x8032
#define CL45_CMD_GET_UPPER_TEMP_WARNING_LEVEL   0x8033
#define CL45_CMD_SET_LOWER_TEMP_WARNING_LEVEL   0x8034
#define CL45_CMD_GET_LOWER_TEMP_WARNING_LEVEL   0x8035
    
#define CL45_CMD_GET_HW_FR_EMI_MODE_ENABLE      0x803a
#define CL45_CMD_SET_HW_FR_EMI_MODE_ENABLE      0x803a
#define CL45_CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST   0x8040
#define CL45_CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST   0x8041
#define CL45_CMD_GET_XFI_2P5G_MODE      0x8016
#define CL45_CMD_SET_XFI_2P5G_MODE      0x8017
#define CL45_CMD_GET_AGGREGATE_MODE     0x8018
#define CL45_CMD_SET_AGGREGATE_MODE     0x8019
#define CL45_CMD_GET_EEE_STATICS        0x801a
#define CL45_CMD_SET_EEE_STATICS        0x801b
#define CL45_CMD_GET_JUMBO_PACKET       0x801c
#define CL45_CMD_SET_JUMBO_PACKET       0x801d

#define CL45_REG_DNLD_PROC_CTRL     0x1a817
#define CL45_REG_DNLD_STATUS        0x1a818
#define CL45_REG_DNLD_ADDR_LO       0x1a819
#define CL45_REG_DNLD_ADDR_HI       0x1a81a
#define CL45_REG_DNLD_DATA_LO       0x1a81b
#define CL45_REG_DNLD_DATA_HI       0x1a81c

#define CL45_REG_PRIV_STATUS        0x1e400d
    #define CL45_REG_STATUS_CRC_CHK_SHFT    14
    #define CL45_REG_STATUS_CRC_CHK_MASK    (0x3 << CL45_REG_STATUS_CRC_CHK_SHFT)
    #define CL45_REG_STATUS_CRC_CHK_CHKING  (0 << CL45_REG_STATUS_CRC_CHK_SHFT)
    #define CL45_REG_STATUS_CRC_CHK_GOOD    (1 << CL45_REG_STATUS_CRC_CHK_SHFT)
    #define CL45_REG_STATUS_CRC_CHK_BAD     (2 << CL45_REG_STATUS_CRC_CHK_SHFT)

#define CL45_REG_DNLD_CTRL      0x1a817
    #define CL45_DNLD_WRT_DWORD     0x9
    #define CL45_DNLD_READ_DWORD    0xa
    #define CL45_DNLD_DNLD_DWORD    0x38
#define CL45_REG_DNLD_STATUS    0x1a818
    #define CL45_BIT_DNLD_STATUS    0x1
#define CL45_REG_DNLD_ADDR_LO   0x1a819
#define CL45_REG_DNLD_ADDR_HI   0x1a81a
#define CL45_REG_DNLD_DATA_LO   0x1a81b
#define CL45_REG_DNLD_DATA_HI   0x1a81c

/* Device 1 definitions */
#define CL45_REG_PMA_PMD_EXT_ABL    0x1000b
    #define CL45_REG_CAP_2P5G_5G    (1<<14)
    #define CL45_REG_CAP_100MB_T    (1<<7)
    #define CL45_REG_CAP_1GB_T     (1<<5)
    #define CL45_REG_CAP_10GB_T     (1<<2)

#define CL45_GET_DEVID(devReg)  (devReg>>16)
#define CL45_GET_REGADDR(devReg)  ((devReg)&0xffff)

/* Device 7 definitoins */
#define CL45_REG_1G100M_CTL     0x7ffe0
    #define CL45_REG_INTNL_LOOPBACK     (1<<14)
    #define CL45_REG_SPEED_MASK         ((1<<6)|(1<<13))
    #define CL45_REG_1000M_SPEED        (1<<6)
    #define CL45_REG_100M_SPEED         (1<<13)
    #define CL45_REG_1G100M_AN_ENABLED  (1<<12)
    #define CL45_REG_1G100M_AN_RESTART  (1<<9)
    #define CL45_REG_DUPLEX_MODE        (1<<8)

#define CL45_REG_COP_AN     0x7ffe4
    #define CL45_REG_COP_AN_100M_ADV_MASK   (3<<7)
    #define CL45_REG_COP_AN_100M_FD_ADV     (2<<7)
    #define CL45_REG_COP_AN_100M_HD_ADV     (1<<7)
    #define CL45_REG_COP_AN_100M_FHD_ADV    (3<<7)
    #define CL45_REG_COP_PAUSE              (1<<10)
    #define CL45_REG_COP_PAUSE_ASYM         (1<<11)

#define CL45_REG_1G_CTL     0x7ffe9
    #define CL45_REG_1G_CTL_1G_ADV_MASK     (3<<8)
    #define CL45_REG_1G_CTL_1G_FD_ADV       (2<<8)
    #define CL45_REG_1G_CTL_1G_HD_ADV       (1<<8)
    #define CL45_REG_1G_CTL_1G_FHD_ADV      (3<<8)
    #define CL45_REG_CAP_REPEATER           (1<<10)

#define CL45_REG_1G100M_AUX_STATUS  0x7fff9
    #define CL45_AN_STATUS_MASK     (7<<8)
    #define CL45_AN_STATUS_1GFD     (7<<8)
    #define CL45_AN_STATUS_1GHD     (6<<8)
    #define CL45_AN_STATUS_100MFD   (5<<8)
    #define CL45_AN_STATUS_100MT4   (4<<8)
    #define CL45_AN_STATUS_100MHD   (3<<8)
    #define CL45_AN_STATUS_RX_PAUSE (1<<1)
    #define CL45_AN_STATUS_TX_PAUSE (1<<0)

#define CL45_REG_1G_TEST_REG        0x7fffe
    #define CL45_FORCE_LINK_UP      (1<<12)

/* Device 7 10G Register definitions */
/* 10GBase-T AN Control Register */
#define CL45REG_10GBASET_AN_CTL 0x70000
    #define CL45_REG_10G_AN_ENABLE      (1<<12)
    #define CL45_REG_10G_AN_RESTART     (1<<9)

/* Device 7 10G AN definitions */
#define CL45REG_10GBASET_AN_DEF_CTL 0x70020
    #define CL45_10GAN_10G_ABL      (1<<12)
    #define CL45_10GAN_5G_ABL       (1<<8)
    #define CL45_10GAN_2P5G_ABL     (1<<7)

/* Device 30 definitions */
#define CL45_REG_MGB_AN_CTL     0x1e0000
    #define CL45_REG_MGB_AN_5G_PCS_LOOPBACK     (1<<6)
    #define CL45_REG_MGB_AN_2P5G_PCS_LOOPBACK   (1<<5)
    #define CL45_REG_MGB_AN_5G_EEE_ADV          (1<<4)
    #define CL45_REG_MGB_AN_2P5G_EEE_ADV        (1<<3)
    #define CL45_REG_MGB_AN_5G_ADV              (1<<2)
    #define CL45_REG_MGB_AN_2P5G_ADV            (1<<1)
    #define CL45_REG_MGB_ENABLE                 (1<<0)

#define CL45_REG_USER_REQ_1_STATUS 0x1e400e
    #define CL45_REG_USER_DONT_CHANGE_STRAP     (1<<1)

#define CL45_REG_FMWR_REV       0x1e400f
    #define CL45_REG_FW_VER_BLD_SHFT    12
    #define CL45_REG_FW_VER_BLD_MASK    (0xf << CL45_REG_FW_VER_BLD_SHFT)
    #define CL45_REG_FW_VER_MAIN_SHFT    7
    #define CL45_REG_FW_VER_MAIN_MASK    (0x1f << CL45_REG_FW_VER_MAIN_SHFT)
    #define CL45_REG_FW_VER_BRCH_MASK    0x7f

#define CL45_REG_FMWR_DATE      0x1e4010
    #define CL45_REG_FMWR_MNTH_SHFT     9
    #define CL45_REG_FMWR_MNTH_MASK     (0xf << CL45_REG_FMWR_MNTH_SHFT)
    #define CL45_REG_FMWR_DAY_SHFT      4
    #define CL45_REG_FMWR_DAY_MASK      (0x1f << CL45_REG_FMWR_DAY_SHFT)
    #define CL45_REG_FMWR_YEAR_MASK     0xf

#define CL45_REG_UDEF_STATUS    0x1e400d
    #define CL45_UDEF_STATUS_MAC_LINK       (1<<13)
    #define CL45_UDEF_STATUS_COPPER_LINK    (1<<5)
    #define CL45_UDEF_STATUS_COPPER_SPD_S   2
        #define CL45_UDEF_STATUS_COPPER_SPD_M       (7<<CL45_UDEF_STATUS_COPPER_SPD_S)
        #define CL45_UDEF_STATUS_COPPER_SPD_100M    (2<<CL45_UDEF_STATUS_COPPER_SPD_S)
        #define CL45_UDEF_STATUS_COPPER_SPD_1G      (4<<CL45_UDEF_STATUS_COPPER_SPD_S)
        #define CL45_UDEF_STATUS_COPPER_SPD_10G     (6<<CL45_UDEF_STATUS_COPPER_SPD_S)
        #define CL45_UDEF_STATUS_COPPER_SPD_2P5G    (1<<CL45_UDEF_STATUS_COPPER_SPD_S)
        #define CL45_UDEF_STATUS_COPPER_SPD_5G      (3<<CL45_UDEF_STATUS_COPPER_SPD_S)

#define CL45_REG_CONF_STRAP_PIN 0x1e401a
    #define CL45_REG_STRAP_SUPER_ISOLATE    (1<<15)

#define CL45_REG_PCS_CONTROL_1  0x30000
    #define CL45_PCS_LOOPBACK       (0x1<<14)
    #define CL45_SPEED_SEL_MASK     (0xf<<2)
    #define CL45_SPEED_SEL_SHIFT    2
        #define CL45_SPEED_SEL_5G   (0x8<<2)
        #define CL45_SPEED_SEL_2P5G (0x7<<2)
        #define CL45_SPEED_SEL_10G  (0x0<<2)

#ifdef __cplusplus
}
#endif

#endif

