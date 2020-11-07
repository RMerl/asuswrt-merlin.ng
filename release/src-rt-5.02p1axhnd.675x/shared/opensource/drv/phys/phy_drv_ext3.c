/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

/*
 *  Created on: Jan 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Phy driver for external 1G/2.5G/5G/10G phys: BCM8486x, BCM8488x, BCM8489x, BCM5499x
 */

#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "bcm_gpio.h"
#ifndef _CFE_
#include "phy_macsec_api.h"
#endif

typedef struct firmware_s firmware_t;
typedef struct phy_desc_s phy_desc_t;

struct firmware_s {
    char *version;
    uint32_t *data;
    uint32_t size;
    int (*load)(firmware_t *firmware);
    uint32_t map;
    uint8_t macsec_capable;
};

struct phy_desc_s {
    uint16_t phyid1;
    uint16_t phyid2;
    char *name;
    firmware_t *firmware;
};

#if defined(_MAKO_A0_)
static int load_mako(firmware_t *firmware);
#endif
#if defined(_ORCA_A0_) || defined(_ORCA_B0_)
static int load_orca(firmware_t *firmware);
#endif
#if defined(_BLACKFIN_A0_) || defined(_BLACKFIN_B0_) || defined(_LONGFIN_A0_)
static int load_blackfin(firmware_t *firmware);
#endif

#ifdef _MAKO_A0_
#include "mako_a0_firmware.h"
firmware_t mako_a0 = { mako_a0_version, mako_a0_firmware, sizeof(mako_a0_firmware), &load_mako, 0, 0 };
#endif
#ifdef _ORCA_A0_
#include "orca_a0_firmware.h"
firmware_t orca_a0 = { orca_a0_version, orca_a0_firmware, sizeof(orca_a0_firmware), &load_orca, 0, 0 };
#endif
#ifdef _ORCA_B0_
#include "orca_b0_firmware.h"
firmware_t orca_b0 = { orca_b0_version, orca_b0_firmware, sizeof(orca_b0_firmware), &load_orca, 0, 0 };
#endif
#ifdef _BLACKFIN_A0_ 
#include "blackfin_a0_firmware.h"
firmware_t blackfin_a0 = { blackfin_a0_version, blackfin_a0_firmware, sizeof(blackfin_a0_firmware), &load_blackfin, 0, 0 };
#endif
#ifdef _BLACKFIN_B0_ 
#include "blackfin_b0_firmware.h"
firmware_t blackfin_b0 = { blackfin_b0_version, blackfin_b0_firmware, sizeof(blackfin_b0_firmware), &load_blackfin, 0, 0 };
#endif
#ifdef _LONGFIN_A0_
#include "longfin_a0_firmware.h"
firmware_t longfin_a0 = { longfin_a0_version, longfin_a0_firmware, sizeof(longfin_a0_firmware), &load_blackfin, 0, 1 };
#endif

static firmware_t *firmware_list[] = {
#ifdef _MAKO_A0_
    &mako_a0,
#endif
#ifdef _ORCA_A0_
    &orca_a0,
#endif
#ifdef _ORCA_B0_
    &orca_b0,
#endif
#ifdef _BLACKFIN_A0_ 
    &blackfin_a0,
#endif
#ifdef _BLACKFIN_B0_ 
    &blackfin_b0,
#endif
#ifdef _LONGFIN_A0_
    &longfin_a0,
#endif
};

static phy_desc_t phy_desc[] = {
#ifdef _MAKO_A0_
    { 0xae02, 0x5048, "84860    A0", &mako_a0 },
    { 0xae02, 0x5040, "84861    A0", &mako_a0 },
#endif
#ifdef _ORCA_A0_
    { 0xae02, 0x5158, "84880    A0", &orca_a0 },
    { 0xae02, 0x5150, "84881    A0", &orca_a0 },
    { 0xae02, 0x5148, "84884    A0", &orca_a0 },
    { 0xae02, 0x5168, "84884E   A0", &orca_a0 },
    { 0xae02, 0x5178, "84885    A0", &orca_a0 },
    { 0xae02, 0x5170, "84886    A0", &orca_a0 },
    { 0xae02, 0x5144, "84887    A0", &orca_a0 },
    { 0xae02, 0x5140, "84888    A0", &orca_a0 },
    { 0xae02, 0x5160, "84888E   A0", &orca_a0 },
    { 0xae02, 0x5174, "84888S   A0", &orca_a0 },
#endif
#ifdef _ORCA_B0_
    { 0xae02, 0x5159, "84880    B0", &orca_b0 },
    { 0xae02, 0x5151, "84881    B0", &orca_b0 },
    { 0xae02, 0x5149, "84884    B0", &orca_b0 },
    { 0xae02, 0x5169, "84884E   B0", &orca_b0 },
    { 0xae02, 0x5179, "84885    B0", &orca_b0 },
    { 0xae02, 0x5171, "84886    B0", &orca_b0 },
    { 0xae02, 0x5145, "84887    B0", &orca_b0 },
    { 0xae02, 0x5141, "84888    B0", &orca_b0 },
    { 0xae02, 0x5161, "84888E   B0", &orca_b0 },
    { 0xae02, 0x5175, "84888S   B0", &orca_b0 },
#endif
#ifdef _BLACKFIN_A0_
    { 0x3590, 0x5090, "84891    A0", &blackfin_a0 },
    { 0x3590, 0x5094, "54991    A0", &blackfin_a0 },
    { 0x3590, 0x5098, "54991E   A0", &blackfin_a0 },
    { 0x3590, 0x5080, "84891L   A0", &blackfin_a0 },
    { 0x3590, 0x5084, "54991L   A0", &blackfin_a0 },
    { 0x3590, 0x5088, "54991EL  A0", &blackfin_a0 },
    { 0x3590, 0x50a0, "84892    A0", &blackfin_a0 },
    { 0x3590, 0x50a4, "54992    A0", &blackfin_a0 },
    { 0x3590, 0x50a8, "54992E   A0", &blackfin_a0 },
    { 0x3590, 0x50b0, "84894    A0", &blackfin_a0 },
    { 0x3590, 0x50b4, "54994    A0", &blackfin_a0 },
    { 0x3590, 0x50b8, "54994E   A0", &blackfin_a0 },
    { 0x3590, 0x50d0, "54991H   A0", &blackfin_a0 },
    { 0x3590, 0x50f0, "54994H   A0", &blackfin_a0 },
#endif
#ifdef _BLACKFIN_B0_
    { 0x3590, 0x5091, "84891    B0", &blackfin_b0 },
    { 0x3590, 0x5095, "54991    B0", &blackfin_b0 },
    { 0x3590, 0x5099, "54991E   B0", &blackfin_b0 },
    { 0x3590, 0x5081, "84891L   B0", &blackfin_b0 },
    { 0x3590, 0x5085, "54991L   B0", &blackfin_b0 },
    { 0x3590, 0x5089, "54991EL  B0", &blackfin_b0 },
    { 0x3590, 0x50a1, "84892    B0", &blackfin_b0 },
    { 0x3590, 0x50a5, "54992    B0", &blackfin_b0 },
    { 0x3590, 0x50a9, "54992E   B0", &blackfin_b0 },
    { 0x3590, 0x50b1, "84894    B0", &blackfin_b0 },
    { 0x3590, 0x50b5, "54994    B0", &blackfin_b0 },
    { 0x3590, 0x50b9, "54994E   B0", &blackfin_b0 },
    { 0x3590, 0x50d1, "54991H   B0", &blackfin_b0 },
    { 0x3590, 0x50f1, "54994H   B0", &blackfin_b0 },
#endif
#ifdef _LONGFIN_A0_
    { 0x3590, 0x5180, "84891LM  A0", &longfin_a0 },
    { 0x3590, 0x5190, "84891M   A0", &longfin_a0 },
    { 0x3590, 0x5194, "54991M   A0", &longfin_a0 },
    { 0x3590, 0x5198, "54991EM  A0", &longfin_a0 },
    { 0x3590, 0x5188, "54991ELM A0", &longfin_a0 },
#endif
};

static uint32_t enabled_phys;

#define BUS_WRITE(a, b, c, d)       if ((ret = _bus_write(a, b, c, d))) goto Exit;
#define BUS_WRITE_ALL(a, b, c, d)   if ((ret = _bus_write_all(a, b, c, d))) goto Exit;
#define BUS_WRITE_AND_VERIFY_ALL(a, b, c, d, e)   if ((ret = _bus_write_and_verify_all(a, b, c, d, e))) goto Exit;

#define PHY_READ(a, b, c, d)        if ((ret = phy_bus_c45_read(a, b, c, d))) goto Exit;
#define PHY_WRITE(a, b, c, d)       if ((ret = phy_bus_c45_write(a, b, c, d))) goto Exit;

/* Command codes */
#define CMD_GET_PAIR_SWAP                           0x8000
#define CMD_SET_PAIR_SWAP                           0x8001
#define CMD_GET_1588_ENABLE                         0x8004
#define CMD_SET_1588_ENABLE                         0x8005
#define CMD_GET_SHORT_REACH_MODE_ENABLE             0x8006
#define CMD_SET_SHORT_REACH_MODE_ENABLE             0x8007
#define CMD_GET_EEE_MODE                            0x8008
#define CMD_SET_EEE_MODE                            0x8009
#define CMD_GET_EMI_MODE_ENABLE                     0x800A
#define CMD_SET_EMI_MODE_ENABLE                     0x800B
#define CMD_GET_SUB_LF_RF_STATUS                    0x800D
#define CMD_GET_SERDES_KR_MODE_ENABLE               0x800E
#define CMD_SET_SERDES_KR_MODE_ENABLE               0x800F
#define CMD_CLEAR_SUB_LF_RF                         0x8010
#define CMD_SET_SUB_LF_RF                           0x8011
#define CMD_READ_INDIRECT_GPHY_REG_BITS             0x8014
#define CMD_WRITE_INDIRECT_GPHY_REG_BITS            0x8015
#define CMD_GET_XFI_2P5G_5G_MODE                    0x8016
#define CMD_SET_XFI_2P5G_5G_MODE                    0x8017
#define CMD_GET_TWO_PAIR_1G_MODE                    0x8018
#define CMD_SET_TWO_PAIR_1G_MODE                    0x8019
#define CMD_SET_EEE_STATISTICS                      0x801A
#define CMD_GET_EEE_STATISTICS                      0x801B
#define CMD_SET_JUMBO_PACKET                        0x801C
#define CMD_GET_JUMBO_PACKET                        0x801D
#define CMD_GET_MSE                                 0x801E
#define CMD_GET_PAUSE_FRAME_MODE                    0x801F
#define CMD_SET_PAUSE_FRAME_MODE                    0x8020
#define CMD_GET_LED_TYPE                            0x8021
#define CMD_SET_LED_TYPE                            0x8022
#define CMD_GET_MGBASE_T_802_3BZ_TYPE               0x8023
#define CMD_SET_MGBASE_T_802_3BZ_TYPE               0x8024
#define CMD_GET_MSE_GPHY                            0x8025
#define CMD_SET_USXGMII                             0x8026
#define CMD_GET_USXGMII                             0x8027
#define CMD_GET_XL_MODE                             0x8029
#define CMD_SET_XL_MODE                             0x802A
#define CMD_GET_XFI_TX_FILTERS                      0x802B
#define CMD_SET_XFI_TX_FILTERS                      0x802C
#define CMD_GET_XFI_POLARITY                        0x802D
#define CMD_SET_XFI_POLARITY                        0x802E
#define CMD_GET_CURRENT_VOLTAGE                     0x802F
#define CMD_GET_SNR                                 0x8030
#define CMD_GET_CURRENT_TEMP                        0x8031
#define CMD_SET_UPPER_TEMP_WARNING_LEVEL            0x8032
#define CMD_GET_UPPER_TEMP_WARNING_LEVEL            0x8033
#define CMD_SET_LOWER_TEMP_WARNING_LEVEL            0x8034
#define CMD_GET_LOWER_TEMP_WARNING_LEVEL            0x8035
#define CMD_GET_HW_FR_EMI_MODE_ENABLE               0x803A
#define CMD_SET_HW_FR_EMI_MODE_ENABLE               0x803B
#define CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST    0x8040
#define CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST    0x8041
#define CMD_GET_DYNAMIC_PARTITION_SELECT            0x8042
#define CMD_SET_DYNAMIC_PARTITION_SELECT            0x8043
#define CMD_RESET_STAT_LOG                          0xC017

/* Command hanlder status codes */
#define CMD_RECEIVED                                0x0001
#define CMD_IN_PROGRESS                             0x0002
#define CMD_COMPLETE_PASS                           0x0004
#define CMD_COMPLETE_ERROR                          0x0008
#define CMD_SYSTEM_BUSY                             0xBBBB

/* Fixups for 5499x phys */
#define ID1_5499X                                   0x35900000
#define ID1_MASK                                    0xffff0000
#define SUPER_I_DEFAULT                             (1<<15)
#define SUPER_I_BLACKFIN                            (1<<8)
#define CHANGE_STRAP_STATUS                         (1<<1)

static int _wait_for_cmd_ready(phy_dev_t *phy_dev)
{ 
    int ret, i;
    uint16_t val;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_dev, 0x1e, 0x4037, &val);

        if (val != CMD_IN_PROGRESS && val != CMD_SYSTEM_BUSY)
            return 0;

        udelay(2000);
    }

    printk("Timed out waiting for command ready");

Exit:
    return -1;
}

static uint16_t _wait_for_cmd_complete(phy_dev_t *phy_dev)
{ 
    int ret, i;
    uint16_t val = 0;

    for (i = 0; i < 1000; i++)
    {
        /* Read status of command */
        PHY_READ(phy_dev, 0x1e, 0x4037, &val);

        if (val == CMD_COMPLETE_PASS || val == CMD_COMPLETE_ERROR)
            goto Exit;

        udelay(2000);
    }

    printk("Timed out waiting for command complete\n");

Exit:
    return val;
}

static int cmd_handler(phy_dev_t *phy_dev, uint16_t cmd_code, uint16_t *data1, uint16_t *data2, uint16_t *data3, uint16_t *data4, uint16_t *data5)
{
    int ret;
    uint16_t cmd_status = 0;

    /* Make sure command interface is open */
    if ((ret = _wait_for_cmd_ready(phy_dev)))
        goto Exit;

    switch (cmd_code)
    {
    case CMD_SET_PAIR_SWAP:
    case CMD_SET_1588_ENABLE:
    case CMD_SET_SHORT_REACH_MODE_ENABLE:
    case CMD_SET_EEE_MODE:
    case CMD_SET_EMI_MODE_ENABLE:
    case CMD_SET_SERDES_KR_MODE_ENABLE:
    case CMD_CLEAR_SUB_LF_RF:
    case CMD_SET_SUB_LF_RF:
    case CMD_WRITE_INDIRECT_GPHY_REG_BITS:
    case CMD_SET_XFI_2P5G_5G_MODE:
    case CMD_SET_TWO_PAIR_1G_MODE:
    case CMD_SET_PAUSE_FRAME_MODE:
    case CMD_SET_LED_TYPE:
    case CMD_SET_MGBASE_T_802_3BZ_TYPE:
    case CMD_SET_USXGMII:
    case CMD_SET_EEE_STATISTICS:
    case CMD_SET_JUMBO_PACKET:
    case CMD_SET_XL_MODE:
    case CMD_SET_XFI_TX_FILTERS:
    case CMD_SET_XFI_POLARITY:
    case CMD_SET_UPPER_TEMP_WARNING_LEVEL:
    case CMD_SET_LOWER_TEMP_WARNING_LEVEL:
    case CMD_SET_HW_FR_EMI_MODE_ENABLE:
    case CMD_SET_CUSTOMER_REQUESTED_TX_PWR_ADJUST:
    case CMD_SET_DYNAMIC_PARTITION_SELECT:
    {
        if (data1)
            PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);
        if (data2)
            PHY_WRITE(phy_dev, 0x1e, 0x4039, *data2);
        if (data3)
            PHY_WRITE(phy_dev, 0x1e, 0x403a, *data3);
        if (data4)
            PHY_WRITE(phy_dev, 0x1e, 0x403b, *data4);
        if (data5)
            PHY_WRITE(phy_dev, 0x1e, 0x403c, *data5);

        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        break;
    }
    case CMD_GET_PAIR_SWAP:
    case CMD_GET_1588_ENABLE:
    case CMD_GET_SHORT_REACH_MODE_ENABLE:
    case CMD_GET_EEE_MODE:
    case CMD_GET_EMI_MODE_ENABLE:
    case CMD_GET_SERDES_KR_MODE_ENABLE:
    case CMD_GET_SUB_LF_RF_STATUS:
    case CMD_GET_XFI_2P5G_5G_MODE:
    case CMD_GET_TWO_PAIR_1G_MODE:
    case CMD_GET_PAUSE_FRAME_MODE:
    case CMD_GET_LED_TYPE:
    case CMD_GET_MGBASE_T_802_3BZ_TYPE:
    case CMD_GET_MSE_GPHY:
    case CMD_GET_USXGMII:
    case CMD_GET_JUMBO_PACKET:
    case CMD_GET_MSE:
    case CMD_GET_XL_MODE:
    case CMD_GET_XFI_TX_FILTERS:
    case CMD_GET_XFI_POLARITY:
    case CMD_GET_CURRENT_VOLTAGE:
    case CMD_GET_SNR:
    case CMD_GET_CURRENT_TEMP:
    case CMD_GET_UPPER_TEMP_WARNING_LEVEL:
    case CMD_GET_LOWER_TEMP_WARNING_LEVEL:
    case CMD_GET_HW_FR_EMI_MODE_ENABLE:
    case CMD_GET_CUSTOMER_REQUESTED_TX_PWR_ADJUST:
    case CMD_GET_DYNAMIC_PARTITION_SELECT:
    case CMD_RESET_STAT_LOG:
    {
        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        if (data1)
            PHY_READ(phy_dev, 0x1e, 0x4038, data1);
        if (data2)
            PHY_READ(phy_dev, 0x1e, 0x4039, data2);
        if (data3)
            PHY_READ(phy_dev, 0x1e, 0x403a, data3);
        if (data4)
            PHY_READ(phy_dev, 0x1e, 0x403b, data4);
        if (data5)
            PHY_READ(phy_dev, 0x1e, 0x403c, data5);

        break;
    }
    case CMD_READ_INDIRECT_GPHY_REG_BITS:
    {
        if (data1)
            PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);
        if (data2)
            PHY_WRITE(phy_dev, 0x1e, 0x4039, *data2);
        if (data3)
            PHY_WRITE(phy_dev, 0x1e, 0x403a, *data3);
        if (data4)
            PHY_WRITE(phy_dev, 0x1e, 0x403b, *data4);

        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        if (data5)
            PHY_READ(phy_dev, 0x1e, 0x403c, data5);

        break;
    }
    case CMD_GET_EEE_STATISTICS:
    {
        if (data1)
            PHY_WRITE(phy_dev, 0x1e, 0x4038, *data1);

        PHY_WRITE(phy_dev, 0x1e, 0x4005, cmd_code);
        cmd_status = _wait_for_cmd_complete(phy_dev);

        if (data2)
            PHY_READ(phy_dev, 0x1e, 0x4039, data2);
        if (data3)
            PHY_READ(phy_dev, 0x1e, 0x403a, data3);
        if (data4)
            PHY_READ(phy_dev, 0x1e, 0x403b, data4);
        if (data5)
            PHY_READ(phy_dev, 0x1e, 0x403c, data5);

        break;
    }
    default:
        printk("Unsupported cmd code: 0x%x\n", cmd_code);
        break;
    }

    if (cmd_status != CMD_COMPLETE_PASS)
    {
        printk("Failed to execute cmd code: 0x%x\n", cmd_code);
        return -1;
    }

Exit:
    return ret;
}

static int _phy_power_get(phy_dev_t *phy_dev, int *enable)
{
    uint16_t val;
    int ret;

    PHY_READ(phy_dev, 0x01, 0x0000, &val);

    *enable = (val & (1 << 11)) ? 0 : 1;

Exit:
    return ret;
}

static int _phy_power_set(phy_dev_t *phy_dev, int enable)
{
    uint16_t val;
    int ret;

    PHY_READ(phy_dev, 0x01, 0x0000, &val);

    if (enable)
        val &= ~(1 << 11); /* Power up */
    else
        val |= (1 << 11); /* Power down */

    PHY_WRITE(phy_dev, 0x01, 0x0000, val);

Exit:
    return ret;
}

static int _phy_idle_stuffing_mode_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val = enable ? 0 : 1;
    uint16_t const_disabled = 1;

    /* Set idle stuffing mode in 2.5GBASE-T and 5GBASE-T modes */
    if ((ret = cmd_handler(phy_dev, CMD_SET_XFI_2P5G_5G_MODE, &const_disabled, &val, NULL, NULL, NULL)))
        goto Exit;

Exit:
    return ret;
}
static int _phy_led_control_mode_set(phy_dev_t *phy_dev, int user_control)
{
    int ret = 0;
    /* uint16_t val = user_control ? 1 : 0; */

    /* Set led control to user or firmware */
    /*if ((ret = cmd_handler(phy_dev, CMD_SET_LED_TYPE, &val, NULL, NULL, NULL, NULL)))
        goto Exit;

Exit:*/
    return ret;
}

static int _phy_force_auto_mdix_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_dev, 0x07, 0x902f, &val);
    
    if (enable)
        val |= (1 << 9); /* Auto-MDIX enabled */
    else
        val &= ~(1 << 9); /* Auto-MDIX disabled */

    PHY_WRITE(phy_dev, 0x07, 0x902f, val);

Exit:
    return ret;
}

static int _phy_force_auto_mdix_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_dev, 0x07, 0x902f, &val);
    
    *enable = val & (1 << 9) ? 1 : 0; /* Force Auto MDIX Enabled */

Exit:
    return ret;
}

static int _phy_eth_wirespeed_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_dev, 0x07, 0x902f, &val);

    if (enable)
        val |= (1 << 4); /* Ethernet@Wirespeed enabled */
    else
        val &= ~(1 << 4); /* Ethernet@Wirespeed disabled */

    PHY_WRITE(phy_dev, 0x07, 0x902f, val);

Exit:
    return ret;
}

static int _phy_pair_swap_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t data;

    if (enable)
        data = 0x1b;
    else
        data = 0xe4;

    if ((ret = cmd_handler(phy_dev, CMD_SET_PAIR_SWAP, NULL, &data, NULL, NULL, NULL)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_xfi_polarity_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t data = enable ? 1 : 0;
    uint16_t type = 0;

    if ((ret = cmd_handler(phy_dev, CMD_SET_XFI_POLARITY, &type, &data, &data, NULL, NULL)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_apd_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_dev, 0x07, 0x901a, &val);
    *enable = val & (1 << 5) ? 1 : 0; /* Auto power-down mode enabled */

Exit:
    return ret;
}

static int _phy_apd_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;

    /* Auto-Power Down */
    PHY_READ(phy_dev, 0x07, 0x901a, &val);

    if (enable)
    {
        val |= (1 << 5); /* Auto power-down mode enabled */
        val |= (1 << 8); /* Enable energy detect single link pulse */
    }
    else
    {
        val &= ~(1 << 5); /* Auto power-down mode disabled */
        val &= ~(1 << 8); /* Disable energy detect single link pulse */
    }

    PHY_WRITE(phy_dev, 0x07, 0x901a, val);

   /* Reserved Control 3 */
    PHY_READ(phy_dev, 0x07, 0x9015, &val);

    if (enable)
    {
        val |= (1 << 0); /* Disable CLK125 output */
        val &= ~(1 << 1); /* Enable powering down of dll during auto-power down */
    }
    else
    {
        val &= ~(1 << 0); /* Enable CLK125 output */
        val |= (1 << 1); /* Disable powering down of dll during auto-power down */
    }

    PHY_WRITE(phy_dev, 0x07, 0x9015, val);

Exit:
    return ret;
}

static int _phy_eee_mode_get(phy_dev_t *phy_dev, uint16_t *mode);

static int _phy_eee_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* local EEE Status */
    if ((ret = _phy_eee_mode_get(phy_dev, &val)))
        goto Exit;

    *enable = !!val;

Exit:
    return ret;
}

static int _phy_eee_resolution_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    /* EEE Resolution Status */
    PHY_READ(phy_dev, 7, 0x803e, &val);

    /* Check if the link partner auto-negotiated EEE capability */
    *enable = val & 0x3e ? 1 : 0;

Exit:
    return ret;
}

static int _phy_eee_mode_get(phy_dev_t *phy_dev, uint16_t *mode)
{
    int ret;
    uint16_t data1, data2, data3, data4;

    /* Get EEE mode */
    if ((ret = cmd_handler(phy_dev, CMD_GET_EEE_MODE, &data1, &data2, &data3, &data4, NULL)))
        goto Exit;

    *mode = data1;

Exit:
    return ret;
}

static int _phy_eee_mode_set(phy_dev_t *phy_dev, uint32_t caps)
{
    int ret;
    uint16_t val, data1, data2, data3, data4;
    uint8_t mode;

    val = 0;
    mode = 1; /* Native EEE */

    val |= ((caps & PHY_CAP_100_HALF) || (caps & PHY_CAP_100_FULL)) ? (mode << 2) : 0;
    val |= ((caps & PHY_CAP_1000_HALF) || (caps & PHY_CAP_1000_FULL)) ? (mode << 2) : 0;
    val |= ((caps & PHY_CAP_2500) ? (mode << 4) : 0);
    val |= ((caps & PHY_CAP_5000) ? (mode << 6) : 0);
    val |= ((caps & PHY_CAP_10000)) ? (mode << 0) : 0;

    data1 = val;
    data2 = 0;
    data3 = 0;
    data4 = 0;

    /* Set EEE mode */
    if ((ret = cmd_handler(phy_dev, CMD_SET_EEE_MODE, &data1, &data2, &data3, &data4, NULL)))
        goto Exit;

Exit:
    return ret;
}

int _phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *pcaps);

static int _phy_eee_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;
    uint32_t caps;

    if ((ret = _phy_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
        goto Exit;

    if ((ret = _phy_eee_mode_set(phy_dev, enable ? caps : 0)))
        goto Exit;

    /* Restart auto negotiation to kick off EEE settings */
    PHY_READ(phy_dev, 0x07, 0xffe0, &val);
    val |= (1 << 9);
    PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

Exit:
    return ret;
}

static int _phy_read_status(phy_dev_t *phy_dev)
{
    int ret, mode;
    uint16_t val;

    phy_dev->link = 0;
    phy_dev->speed = PHY_SPEED_UNKNOWN;
    phy_dev->duplex = PHY_DUPLEX_UNKNOWN;
    phy_dev->pause_rx = 0;
    phy_dev->pause_tx = 0;

    /* Read the status register */
    PHY_READ(phy_dev, 0x1e, 0x400d, &val);

    /* Copper link status */
    phy_dev->link = ((val >> 5) & 0x1);

    if (!phy_dev->link)
        goto Exit;

    /* Copper speed */
    mode = ((val >> 2) & 0x7);

    if (mode == 0)
        phy_dev->speed = PHY_SPEED_10;
    else if (mode == 1)
        phy_dev->speed = PHY_SPEED_2500;
    else if (mode == 2)
        phy_dev->speed = PHY_SPEED_100;
    else if (mode == 3)
        phy_dev->speed = PHY_SPEED_5000;
    else if (mode == 4)
        phy_dev->speed = PHY_SPEED_1000;
    else if (mode == 6)
        phy_dev->speed = PHY_SPEED_10000;
    else
        goto Exit;

    /* Read the 1000BASE-T auxiliary status summary register */
    PHY_READ(phy_dev, 0x07, 0xfff9, &val);

    if (phy_dev->speed > PHY_SPEED_1000)
    {
        phy_dev->duplex = PHY_DUPLEX_FULL;
    }
    else
    {
        mode = ((val >> 8) & 0x7);

        if (mode == 3 || mode == 6)
            phy_dev->duplex = PHY_DUPLEX_HALF;
        else if (mode == 5 || mode == 7)
            phy_dev->duplex = PHY_DUPLEX_FULL;
    }

    phy_dev->pause_rx = ((val >> 1) & 0x1);
    phy_dev->pause_tx = ((val >> 0) & 0x1);

Exit:
    return ret;
}

int _phy_caps_get(phy_dev_t *phy_dev, int caps_type, uint32_t *pcaps)
{
    int ret = 0;
    uint16_t val = 0;
    uint32_t caps = 0;

    if ((caps_type != CAPS_TYPE_ADVERTISE) 
        && (caps_type != CAPS_TYPE_SUPPORTED))
        goto Exit;

    if (caps_type == CAPS_TYPE_SUPPORTED)
    {
        caps |= PHY_CAP_AUTONEG | PHY_CAP_PAUSE | PHY_CAP_PAUSE_ASYM | PHY_CAP_REPEATER;
        caps |= PHY_CAP_100_HALF | PHY_CAP_100_FULL;
        caps |= PHY_CAP_1000_HALF | PHY_CAP_1000_FULL;
        caps |= PHY_CAP_2500 | PHY_CAP_5000  | PHY_CAP_10000;

        /* Don't advertise 5G/10G speeds when the mac in HSGMII mode */
        if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
            caps &= ~(PHY_CAP_5000 | PHY_CAP_10000);

        /* Don't advertise 2.5G/5G/10G speeds when the mac in SGMII mode */
        if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
            caps &= ~(PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000);

        if (phy_dev->disable_hd)
            caps &= ~(PHY_CAP_100_HALF | PHY_CAP_1000_HALF);

        *pcaps = caps;

        return 0;
    }

    /* 1000Base-T/100Base-TX MII control */
    PHY_READ(phy_dev, 0x07, 0xffe0, &val);

    if (val & (1 << 12))
        caps |= PHY_CAP_AUTONEG;

    /* Copper auto-negotiation advertisement */
    PHY_READ(phy_dev, 0x07, 0xffe4, &val);

    if (val & (1 << 10))
        caps |= PHY_CAP_PAUSE;

    if (val & (1 << 11))
        caps |= PHY_CAP_PAUSE_ASYM;

    if (val & (1 << 7))
        caps |= PHY_CAP_100_HALF;

    if (val & (1 << 8))
        caps |= PHY_CAP_100_FULL;

    /* 1000Base-T control */
    PHY_READ(phy_dev, 0x07, 0xffe9, &val);

    if (val & (1 << 8))
        caps |= PHY_CAP_1000_HALF;

    if (val & (1 << 9))
        caps |= PHY_CAP_1000_FULL;

    if (val & (1 << 10))
        caps |= PHY_CAP_REPEATER;

    /* 10GBase-T AN control */
    PHY_READ(phy_dev, 0x07, 0x0020, &val);

    if (val & (1 << 7))
        caps |= PHY_CAP_2500;

    if (val & (1 << 8))
        caps |= PHY_CAP_5000;

    if (val & (1 << 12))
        caps |= PHY_CAP_10000;

    *pcaps = caps;

Exit:
    return ret;
}

int _phy_caps_set(phy_dev_t *phy_dev, uint32_t caps)
{
    int ret;
    uint16_t val, mode;

    caps |= PHY_CAP_AUTONEG;

    /* Don't advertise 5G/10G speeds when the mac in HSGMII mode */
    if (phy_dev->mii_type == PHY_MII_TYPE_HSGMII)
        caps &= ~(PHY_CAP_5000 | PHY_CAP_10000);

    /* Don't advertise 2.5G/5G/10G speeds when the mac in SGMII mode */
    if (phy_dev->mii_type == PHY_MII_TYPE_SGMII)
        caps &= ~(PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000);

    if (phy_dev->disable_hd)
        caps &= ~(PHY_CAP_100_HALF | PHY_CAP_1000_HALF);

    /* Copper auto-negotiation advertisement */
    PHY_READ(phy_dev, 0x07, 0xffe4, &val);

    val &= ~((1 << 7) | (1 << 8) | (1 << 10));

    if (caps & PHY_CAP_100_HALF)
        val |= (1 << 7);

    if (caps & PHY_CAP_100_FULL)
        val |= (1 << 8);

    if (caps & PHY_CAP_PAUSE)
        val |= (1 << 10);

    if (caps & PHY_CAP_PAUSE_ASYM)
        val |= (1 << 11);

    PHY_WRITE(phy_dev, 0x07, 0xffe4, val);

    /* 1000Base-T control */
    PHY_READ(phy_dev, 0x07, 0xffe9, &val);

    val &= ~((1 << 8) | (1 << 9) | (1 << 10));

    if (caps & PHY_CAP_1000_HALF)
        val |= (1 << 8);

    if (caps & PHY_CAP_1000_FULL)
        val |= (1 << 9);

    if (caps & PHY_CAP_REPEATER)
        val |= (1 << 10);

    PHY_WRITE(phy_dev, 0x07, 0xffe9, val);

    /* 10GBase-T AN control */
    PHY_READ(phy_dev, 0x07, 0x0020, &val);

    val &= ~((1 << 7) | (1 << 8) | (1 << 12));

    if (caps & PHY_CAP_2500)
        val |= (1 << 7);

    if (caps & PHY_CAP_5000)
        val |= (1 << 8);

    if (caps & PHY_CAP_10000)
        val |= (1 << 12);

    PHY_WRITE(phy_dev, 0x07, 0x0020, val);

    /* 1000Base-T/100Base-TX MII control */
    PHY_READ(phy_dev, 0x07, 0xffe0, &val);

    val &= ~(1 << 12);

    if (caps & PHY_CAP_AUTONEG)
        val |= (1 << 12);

    PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

    if (!(caps & PHY_CAP_AUTONEG))
        goto Exit;

    /* Check if EEE mode is configured */
    if ((ret = _phy_eee_mode_get(phy_dev, &mode)))
        goto Exit;

    /* Reset the EEE mode according to the phy capabilites, if it was set before */
    if (mode && (ret = _phy_eee_mode_set(phy_dev, caps)))
        goto Exit;

    /* Restart auto negotiation */
    val |= (1 << 9);
    PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

Exit:
    return ret;
}

static int _phy_speed_set(phy_dev_t *phy_dev, phy_speed_t speed, phy_duplex_t duplex)
{
    int ret;
    uint32_t caps;

    if (speed == PHY_SPEED_UNKNOWN)
    {
        speed = PHY_SPEED_10000;
        duplex = PHY_DUPLEX_FULL;
    }

    if ((ret = _phy_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
        goto Exit;

    caps &= ~(PHY_CAP_100_HALF | PHY_CAP_100_FULL |
        PHY_CAP_1000_HALF | PHY_CAP_1000_FULL |
        PHY_CAP_2500 | PHY_CAP_5000 | PHY_CAP_10000);

    switch (speed)
    {
    case PHY_SPEED_10000:
        caps |= PHY_CAP_10000;
    case PHY_SPEED_5000:
        caps |= PHY_CAP_5000;
    case PHY_SPEED_2500:
        caps |= PHY_CAP_2500;
    case PHY_SPEED_1000:
        caps |= PHY_CAP_1000_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_1000_FULL : 0);
    case PHY_SPEED_100:
        caps |= PHY_CAP_100_HALF | ((duplex == PHY_DUPLEX_FULL) ? PHY_CAP_100_FULL : 0);
        break;
    default:
        printk("Ignoring unsupported speed\n");
        goto Exit;
        break;
    }

    if ((ret = _phy_caps_set(phy_dev, caps)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_phyid_get(phy_dev_t *phy_dev, uint32_t *phyid)
{
    int ret;
    uint16_t phyid1, phyid2;

    PHY_READ(phy_dev, 0x01, 0x0002, &phyid1);
    PHY_READ(phy_dev, 0x01, 0x0003, &phyid2);

    *phyid = phyid1 << 16 | phyid2;

Exit:
    return ret;
}

static int _phy_super_isolate_5499x(phy_dev_t *phy_dev, int isolate)
{
    int ret;
    uint16_t data;

    /* Read the status register */
    PHY_READ(phy_dev, 0x1e, 0x401c, &data);

    if (isolate)
        data |= SUPER_I_BLACKFIN;
    else
        data &= ~SUPER_I_BLACKFIN;

    PHY_WRITE(phy_dev, 0x1e, 0x401c, data);

    return 0;
Exit:
    return ret;
}

static int _phy_super_isolate_default(phy_dev_t *phy_dev, int isolate)
{
    int ret;
    uint16_t data;

    /* Read the status register */
    PHY_READ(phy_dev, 0x1e, 0x401a, &data);

    if (isolate)
        data |= SUPER_I_DEFAULT;
    else
        data &= ~SUPER_I_DEFAULT;

    PHY_WRITE(phy_dev, 0x1e, 0x401a, data);

    return 0;
Exit:
    return ret;
}

static int _phy_isolate(phy_dev_t *phy_dev, int isolate)
{
    uint16_t val;
    int ret;

    PHY_READ(phy_dev, 0x7, 0xffe0, &val);
    if (isolate) {
        val |= BMCR_ISOLATE;
    } else {
        val &= ~BMCR_ISOLATE;
    }
    PHY_WRITE(phy_dev, 0x7, 0xffe0, val);

Exit:
    return ret;
}

static int _phy_super_isolate(phy_dev_t *phy_dev, int isolate)
{
    int ret, i = 1000;
    uint16_t data;
    uint32_t phyid;

    _phy_phyid_get(phy_dev, &phyid);

    if ((phyid & ID1_MASK) == ID1_5499X)
        ret = _phy_super_isolate_5499x(phy_dev, isolate);
    else
        ret = _phy_super_isolate_default(phy_dev, isolate);

    if (ret)
        goto Exit;
    do {
        udelay(1000);
        PHY_READ(phy_dev, 0x1e, 0x400e, &data);
    } while (i-- && (data & CHANGE_STRAP_STATUS));

    if (data & CHANGE_STRAP_STATUS)
        printk("ERROR: PHY address: %d, hardware failed to clear SUPER_I boot strap status 0x%x\n", 
                phy_dev->addr, data);

    return 0;
Exit:
    return ret;
}

static int _phy_set_mode(phy_dev_t *phy_dev, int line_side)
{
    int ret;
    uint16_t val1, val2, val3;

    val1 = line_side ? 0x0001 : 0x2004;
    val2 = line_side ? 0x0001 : 0x2004;
    val3 = line_side ? 0x1002 : 0x2004;

    /* Set base-pointer mode */
    PHY_WRITE(phy_dev, 0x1e, 0x4110, val1);
    PHY_WRITE(phy_dev, 0x1e, 0x4111, val2);
    PHY_WRITE(phy_dev, 0x1e, 0x4113, val3);

Exit:
    return ret;
}

static int _phy_init(phy_dev_t *phy_dev)
{
    int ret, i;
    uint16_t val;
    phy_speed_t speed = PHY_SPEED_UNKNOWN;

    /* Reset the phy */
    PHY_WRITE(phy_dev, 0x01, 0x0000, 0x8000);

    /* Verify that the processor is running */
    i = 1000;
    do
    {
        udelay(2000);
        PHY_READ(phy_dev, 0x01, 0x0000, &val);
        ret = (val != 0x2040);
    } while (ret && i--);

    if (ret)
        goto Exit;

    /* Set base-pointer mode to line side to access PMD/PCS/AN for BASE-T */
    if ((ret = _phy_set_mode(phy_dev, 1)))
        goto Exit;

    /* Set idle stuffing mode */
    if ((ret = _phy_idle_stuffing_mode_set(phy_dev, phy_dev->idle_stuffing)))
        goto Exit;

    /* Enable Force Auto-MDIX mode */
    if ((ret = _phy_force_auto_mdix_set(phy_dev, 1)))
        goto Exit;

    /* Enable Ethernet@Wirespeed mode */
    if ((ret = _phy_eth_wirespeed_set(phy_dev, 1)))
        goto Exit;

    /* Set pair-swap mode */
    if ((ret = _phy_pair_swap_set(phy_dev, phy_dev->swap_pair)))
        goto Exit;

    /* Set XFI polarity */
    if ((ret = _phy_xfi_polarity_set(phy_dev, 0)))
        goto Exit;

    if ((ret = _phy_caps_set(phy_dev, PHY_CAP_PAUSE | PHY_CAP_REPEATER)))
        goto Exit;

    /* Set led control type */
    if ((ret = _phy_led_control_mode_set(phy_dev, 0)))
        goto Exit;

    
    switch (phy_dev->mii_type)
    {
    case PHY_MII_TYPE_SGMII:
        speed = PHY_SPEED_1000;
        break;
    case PHY_MII_TYPE_HSGMII:
        speed = PHY_SPEED_2500;
        break;
    case PHY_MII_TYPE_XFI:
        speed = PHY_SPEED_10000;
        break;
    default:
        printk("Unsupported MII type: %d\n", phy_dev->mii_type);
        break;
    }

    if ((ret = _phy_speed_set(phy_dev, speed, PHY_DUPLEX_FULL)))
        goto Exit;

#ifndef _CFE_
    {
        uint32_t phyid;
        _phy_phyid_get(phy_dev, &phyid);
        for (i = 0; i < sizeof(phy_desc)/sizeof(phy_desc[0]); i++)
        {
            if ((((phyid >> 16) & 0xffff) == phy_desc[i].phyid1) && ((phyid & 0xffff) == phy_desc[i].phyid2) && phy_desc[i].firmware->macsec_capable)
            {
                printk("phy %s is macsec capable, initializing macsec module\n", phy_desc[i].name);
                ret = phy_macsec_pu_init(phy_dev);
                break;
            }
        }
    }
#endif    
Exit:
    return ret;
}

extern phy_drv_t phy_drv_ext3;

static int inline _bus_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return phy_drv_ext3.bus_drv->c45_read(addr, dev, reg, val);
}

static int inline _bus_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return phy_drv_ext3.bus_drv->c45_write(addr, dev, reg, val);
}

static uint32_t _bus_read_all(uint32_t phy_map, uint16_t dev, uint16_t reg, uint16_t val, uint32_t mask)
{
    uint32_t i, ret = 0;
    uint16_t _val;

    for (i = 0; i < 32; i++)
    {
        if (!(phy_map & (1 << i)))
            continue;

        if ((_bus_read(i, dev, reg, &_val)))
            continue;

        if ((_val & mask) == val)
            ret |= (1 << i);
    }

    return ret;
}

static int _bus_write_all(uint32_t phy_map, uint16_t dev, uint16_t reg, uint16_t val)
{
    int ret;
    uint32_t i;

    for (i = 0; i < 32; i++)
    {
        if (!(phy_map & (1 << i)))
            continue;

        if ((ret = _bus_write(i, dev, reg, val)))
            return ret;
    }

    return 0;
}

static int _bus_write_and_verify_all(uint32_t phy_map, uint16_t dev, uint16_t reg, uint16_t val, uint32_t mask)
{
    int ret;
    uint32_t i, j;
    uint16_t _val;

    for (i = 0; i < 32; i++)
    {
        if (!(phy_map & (1 << i)))
            continue;

                j = 1000;
                do
                {
                    udelay(1000);
                    if ((ret = _bus_write(i, dev, reg, val)))
                        return ret;
                
                    if ((ret = _bus_read(i, dev, reg, &_val)))
                        return ret;
            } while (((_val & 0xffff) != val) && j--);
            
            if (j == 0)
            {
                printk("MDIO Bus Write and Verify is failed \n");
                return -1;
            }
    }

    return 0;
}

static int phy_count (uint32_t phy_map)
{
    int cnt;
    for (cnt = 0; phy_map != 0; phy_map &= (phy_map - 1))
        cnt++;

    return cnt;
}

static uint32_t get_base_phy_addr(uint32_t phy_map)
{
    int flag = 1;
    int cnt = 0;
    while ((flag & phy_map) == 0)
        {
            flag <<= 1;
            cnt ++;
        }
    
    return cnt;
}

static uint32_t bcast_phy_map(uint32_t phy_map)
{
    return (phy_map & ~(1 << get_base_phy_addr(phy_map)));
}

#if defined(_MAKO_A0_)
static int load_mako(firmware_t *firmware)
{
    int i, cnt, step, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, *firmware_data, firmware_size, b_phy_map;

    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);

    firmware_data = firmware->data;
    firmware_size = firmware->size;

    cnt = firmware_size / sizeof(uint32_t);
    step = cnt / 100;

    /* 1. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
    if (phy_cnt > 1)
    {
        printk("Turn on broadcast mode to accept write operations\n");
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
    }

    /* 2. Halt the BCM8486X processors operation */
    printk("Halt the PHYs processors operation\n");

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x017c);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0040);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xc300);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0010);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x1018);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe59f);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0004);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x1f11);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xee09);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0008);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe3a0);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x000c);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x1806);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe3a0);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0010);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0002);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe8a0);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0014);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0001);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xe150);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0018);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0xfffc);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x3aff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x001c);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0xfffe);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0xeaff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0020);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xffff);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0021);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x0004);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0000);

    /* 3. Upload the firmware into the on-chip memory of the devices */
    printk("Upload the firmware into the on-chip memory\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0038);

    for (i = 0; i < cnt; i++)
    {
        BUS_WRITE(base_phy_addr, 0x01, 0xa81c, firmware_data[i] >> 16); /* upper 16 bits */
        BUS_WRITE(base_phy_addr, 0x01, 0xa81b, firmware_data[i] & 0xffff); /* lower 16 bits */

        if (i == i / step * step)
            printk("\r%d%%", i / step);
    }
    printk("\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0xc300);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81b, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa81c, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0009);

    /* 4. Reset the processors to start execution of the code in the on-chip memory */
    printk("Reset the processors to start execution of the code in the on-chip memory\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa008, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x8004, 0x5555);
    BUS_WRITE(base_phy_addr, 0x01, 0x0000, 0x8000);

    /* 5. Verify that the processors are running */
    printk("Verify that the processors are running: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while ((ret != phy_map) && i--);

    printk("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    /* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    printk("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while ((ret != phy_map) && i--);

    printk("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    printk("Firmware loading completed successfully\n\n");

Exit:
    return ret;
}
#endif

#if defined(_ORCA_A0_) || defined(_ORCA_B0_)
static int load_orca(firmware_t *firmware)
{
    int i, cnt, step, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, *firmware_data, firmware_size, b_phy_map;

    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);

    firmware_data = firmware->data;
    firmware_size = firmware->size;

    cnt = firmware_size / sizeof(uint32_t);
    step = cnt / 100;

    /* 1. Halt the BCM8488X processors operation */
    printk("Halt the PHYs processors operation\n");

    BUS_WRITE_ALL(phy_map, 0x1e, 0x418c, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x017c);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);

    BUS_WRITE_AND_VERIFY_ALL(phy_map, 0x1e, 0x4181, 0x0040, 0xffff);

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4186, 0x8000);

    BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xc300);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0001);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4181, 0x0000);

    /* 2. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
    if (phy_cnt > 1)
    {
        printk("Turn on broadcast mode to accept write operations\n");
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
    }

    /* 3. Upload the firmware into the on-chip memory of the devices */
    printk("Upload the firmware into the on-chip memory\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0038);

    for (i = 0; i < cnt; i++)
    {
        BUS_WRITE(base_phy_addr, 0x01, 0xa81c, firmware_data[i] >> 16); /* upper 16 bits */
        BUS_WRITE(base_phy_addr, 0x01, 0xa81b, firmware_data[i] & 0xffff); /* lower 16 bits */

        if (i == i / step * step)
            printk("\r%d%%", i / step);
    }
    printk("\n");

    /* 4. Reset the processors to start execution of the code in the on-chip memory */
    printk("Reset the processors to start execution of the code in the on-chip memory\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa008, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x8004, 0x5555);
    BUS_WRITE(base_phy_addr, 0x1, 0x8003, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0x0000, 0x8000);

    /* 5. Verify that the processors are running */
    printk("Verify that the processors are running: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while ((ret != phy_map) && i--);

    printk("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    /* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    printk("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while ((ret != phy_map) && i--);

    printk("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    printk("Firmware loading completed successfully\n\n");

Exit:
    return ret;
}
#endif

#if defined(_BLACKFIN_A0_) || defined(_BLACKFIN_B0_) || defined(_LONGFIN_A0_)
static int load_blackfin(firmware_t *firmware)
{
    int i, cnt, step, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, *firmware_data, firmware_size, b_phy_map;

    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);

    firmware_data = firmware->data;
    firmware_size = firmware->size;

    cnt = firmware_size / sizeof(uint32_t);
    step = cnt / 100;

    /* 1. Halt the BCM5499X processors operation */
    printk("Halt the PHYs processors operation\n");

    BUS_WRITE_ALL(phy_map, 0x1e, 0x4110, 0x0001);

    BUS_WRITE_ALL(phy_map, 0x1e, 0x418c, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4188, 0x48f0);

    BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xf000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x3000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0121);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

    BUS_WRITE_ALL(phy_map, 0x1e, 0x80a6, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1, 0xa010, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1, 0x0000, 0x8000);

    udelay(1000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x4110, 0x0001);
    udelay(1000);

    /* 2. If more than one PHY, turn on broadcast mode to accept write operations for addr = base_phy_addr */
    if (phy_cnt > 1)
    {
        printk("Turn on broadcast mode to accept write operations\n");
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0401 | ((base_phy_addr & 0x1f) << 5));
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0xf001);
    }

    /* 3. Upload the firmware into the on-chip memory of the devices */
    printk("Upload the firmware into the on-chip memory\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0038);

    for (i = 0; i < cnt; i++)
    {
        BUS_WRITE(base_phy_addr, 0x01, 0xa81c, firmware_data[i] >> 16); /* upper 16 bits */
        BUS_WRITE(base_phy_addr, 0x01, 0xa81b, firmware_data[i] & 0xffff); /* lower 16 bits */

        if (i == i / step * step)
            printk("\r%d%%", i / step);
    }
    printk("\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0000);

    /* 4. Disable broadcast if phy cnt > 1 */
    if (phy_cnt > 1)
    {
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4107, 0x0000);
        BUS_WRITE_ALL(b_phy_map, 0x1e, 0x4117, 0x0000);
    }

    /* 5. Reset the processors to start execution of the code in the on-chip memory */
    printk("Reset the processors to start execution of the code in the on-chip memory\n");

    BUS_WRITE_ALL(phy_map, 0x01, 0xa81a, 0xf000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa819, 0x3000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81c, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa81b, 0x0020);
    BUS_WRITE_ALL(phy_map, 0x01, 0xa817, 0x0009);

    udelay(2000);
    
    /* 6. Verify that the processors are running */
    printk("Verify that the processors are running: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while ((ret != phy_map) && i--);

    printk("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    /* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    printk("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while ((ret != phy_map) && i--);

    printk("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    printk("Firmware loading completed successfully\n\n");

Exit:
    return ret;
}
#endif

static void phy_reset_lift(phy_dev_t *phy_dev)
{
    if (phy_dev->reset_gpio == -1)
        return;

    printk(" Lift PHY at address %d out of Reset by GPIO:%d Active %s\n", 
        phy_dev->addr, phy_dev->reset_gpio,
        phy_dev->reset_gpio_active_hi? "High": "Low");
    bcm_gpio_set_dir(phy_dev->reset_gpio, 1);
    bcm_gpio_set_data(phy_dev->reset_gpio, phy_dev->reset_gpio_active_hi);
    udelay(2000);
    bcm_gpio_set_data(phy_dev->reset_gpio, !phy_dev->reset_gpio_active_hi);
    udelay(2000);
} 

static void phys_reset_lift(uint32_t phy_map)
{
    int i;
    phy_dev_t *phy_dev;

    for (i = 0; i < 32; i++) {
        if (!(phy_map & (1 << i)))
            continue;
        phy_dev = phy_dev_get(PHY_TYPE_EXT3, i);
        phy_reset_lift(phy_dev);
    }
}

static int _phy_cfg(uint32_t enabled_phys)
{
    int i, j, k, ret = 0;
    uint32_t phy_map;

    if (!enabled_phys)
        return 0;

    printk("\nDetecting PHYs...0x%x\n", enabled_phys);

	phys_reset_lift(enabled_phys);

    for (i = 0; i < sizeof(firmware_list)/sizeof(firmware_list[0]); i++)
    {
        for (j = 0; j < sizeof(phy_desc)/sizeof(phy_desc[0]); j++)
        {
            if (phy_desc[j].firmware != firmware_list[i])
                continue;

            phy_map = (_bus_read_all(enabled_phys, 0x01, 0x0002, phy_desc[j].phyid1, 0xffff)) &
                (_bus_read_all(enabled_phys, 0x01, 0x0003, phy_desc[j].phyid2, 0xffff));

            firmware_list[i]->map |= phy_map;

            if (phy_map)
            {
                printk("%s %x:%x --> ", phy_desc[j].name, phy_desc[j].phyid1, phy_desc[j].phyid2);
                for (k = 0; k < 32 ; k++)
                {
                    if (!(phy_map & (1 << k)))
                        continue;

                    printk("0x%x ", k);
                }
                printk("\n");
            }
        }
    }

    printk("\nLoading firmware into detected PHYs...\n\n");

    for (i = 0; i < sizeof(firmware_list)/sizeof(firmware_list[0]); i++)
    {
        phy_map = firmware_list[i]->map;

        if (!phy_map)
            continue;

        printk("Firmware version: %s\n", firmware_list[i]->version);
        printk("Loading firmware into PHYs: map=0x%x count=%d\n", phy_map, phy_count(phy_map));

        firmware_list[i]->load(firmware_list[i]);
    }

    return ret;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    enabled_phys |= (1 << (phy_dev->addr));
    return 0;
}

static int _phy_dev_del(phy_dev_t *phy_dev)
{
    enabled_phys &= ~(1 << (phy_dev->addr));

    return 0;
}

static int _phy_drv_init(phy_drv_t *phy_drv)
{
    if (_phy_cfg(enabled_phys))
    {
        printk("Failed to initialize the driver\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

phy_drv_t phy_drv_ext3 =
{
    .phy_type = PHY_TYPE_EXT3,
    .name = "EXT3",
    .power_get = _phy_power_get,
    .power_set = _phy_power_set,
    .apd_get = _phy_apd_get,
    .apd_set = _phy_apd_set,
    .eee_get = _phy_eee_get,
    .eee_set = _phy_eee_set,
    .eee_resolution_get = _phy_eee_resolution_get,
    .read_status = _phy_read_status,
    .speed_set = _phy_speed_set,
    .caps_get = _phy_caps_get,
    .caps_set = _phy_caps_set,
    .phyid_get = _phy_phyid_get,
    .auto_mdix_set = _phy_force_auto_mdix_set,
    .auto_mdix_get = _phy_force_auto_mdix_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
    .pair_swap_set = _phy_pair_swap_set,
    .isolate_phy = _phy_isolate,
    .super_isolate_phy = _phy_super_isolate,
#ifndef _CFE_
    .macsec_oper = phy_macsec_oper,
#endif    
};
