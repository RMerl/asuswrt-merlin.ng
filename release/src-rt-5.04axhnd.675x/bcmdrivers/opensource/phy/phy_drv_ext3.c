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

/*
 * PHY firmware versions:
 * MAKO A0          1.0.3
 * ORCA A0          1.1.2
 * ORCA B0          2.3.5
 * BLACKFIN A0      0.2.4
 * BLACKFIN B0      2.2.8
 * SHORTFIN B0      2.2.11
 * LONGFIN A0       2.2.8
 */

#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include "phy_drv.h"
#include "phy_drv_mii.h"
#include "xrdp_led_init.h"
#ifdef MACSEC_SUPPORT
#include "phy_macsec_api.h"
#endif

typedef struct firmware_s firmware_t;
typedef struct phy_desc_s phy_desc_t;

struct firmware_s {
    char *name;
    int (*load)(firmware_t *firmware);
    uint32_t map;
    uint8_t macsec_capable;
};

struct phy_desc_s {
    uint16_t phyid1;
    uint16_t phyid2;
    char *name;
    firmware_t *firmware;
    uint32_t inter_phy_types;
};

#if defined(CONFIG_BCM_PHY_MAKO_A0)
static int load_mako(firmware_t *firmware);
#endif
#if defined(CONFIG_BCM_PHY_ORCA_A0) || defined(CONFIG_BCM_PHY_ORCA_B0)
static int load_orca(firmware_t *firmware);
#endif
#if defined(CONFIG_BCM_PHY_BLACKFIN_A0) || defined(CONFIG_BCM_PHY_BLACKFIN_B0) || defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
static int load_blackfin(firmware_t *firmware);
#endif

#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
static int load_shortfin(firmware_t *firmware);
#endif

#ifdef CONFIG_BCM_PHY_MAKO_A0
firmware_t mako_a0 = { "mako_a0", load_mako, 0, 0 };
#endif
#ifdef CONFIG_BCM_PHY_ORCA_A0
firmware_t orca_a0 = { "orca_a0", load_orca, 0, 0 };
#endif
#ifdef CONFIG_BCM_PHY_ORCA_B0
firmware_t orca_b0 = { "orca_b0", load_orca, 0, 0 };
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
firmware_t blackfin_a0 = { "blackfin_a0", load_blackfin, 0, 0 };
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
firmware_t blackfin_b0 = { "blackfin_b0", load_blackfin, 0, 0 };
#endif
#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
firmware_t shortfin_b0 = { "shortfin_b0", load_shortfin, 0, 0 };
#endif
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
// longfin A0 and B0 use same firmware dated April 2020 or later
firmware_t longfin_a0_m = { "longfin_a0", load_blackfin, 0, 1 };
firmware_t longfin_a0 = { "longfin_a0", load_blackfin, 0, 0 };
#endif

static firmware_t *firmware_list[] = {
#ifdef CONFIG_BCM_PHY_MAKO_A0
    &mako_a0,
#endif
#ifdef CONFIG_BCM_PHY_ORCA_A0
    &orca_a0,
#endif
#ifdef CONFIG_BCM_PHY_ORCA_B0
    &orca_b0,
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
    &blackfin_a0,
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
    &blackfin_b0,
#endif
#if defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0)
    &longfin_a0_m,
    &longfin_a0,
#endif
#if defined(CONFIG_BCM_PHY_SHORTFIN_B0)
    &shortfin_b0,
#endif
};

static phy_desc_t phy_desc[] = {
#ifdef CONFIG_BCM_PHY_MAKO_A0
    { 0xae02, 0x5048, "84860    A0", &mako_a0, INTER_PHY_TYPES_S1K2KI5I_M },
    { 0xae02, 0x5040, "84861    A0", &mako_a0, INTER_PHY_TYPES_S1K2KI5I_M },
#endif
#ifdef CONFIG_BCM_PHY_ORCA_A0
    { 0xae02, 0x5158, "84880    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5150, "84881    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5148, "84884    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5168, "84884E   A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5178, "84885    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5170, "84886    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5144, "84887    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5140, "84888    A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5160, "84888E   A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5174, "84888S   A0", &orca_a0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
#endif
#ifdef CONFIG_BCM_PHY_ORCA_B0
    { 0xae02, 0x5159, "84880    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5151, "84881    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5149, "84884    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5169, "84884E   B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5179, "84885    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI_M },
    { 0xae02, 0x5171, "84886    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5145, "84887    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5141, "84888    B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5161, "84888E   B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
    { 0xae02, 0x5175, "84888S   B0", &orca_b0, INTER_PHY_TYPES_US1K2KI5KI10R_M },
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_A0
    { 0x3590, 0x5090, "84891    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5094, "54991    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x5098, "54991E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x5080, "84891L   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5084, "54991L   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x5088, "54991EL  A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50a0, "84892    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50a4, "54992    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x50a8, "54992E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50b0, "84894    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50b4, "54994    A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x50b8, "54994E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50d0, "54991H   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50f0, "54994H   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50c8, "50991EL  A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50f8, "50994E   A0", &blackfin_a0, INTER_PHY_TYPES_US1K2KIR_M },
#endif
#ifdef CONFIG_BCM_PHY_BLACKFIN_B0
    { 0x3590, 0x5091, "84891    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5095, "54991    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x5099, "54991E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x5081, "84891L   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5085, "54991L   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x5089, "54991EL  B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50c9, "50991EL  B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50a1, "84892    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50a5, "54992    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x50a9, "54992E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50b1, "84894    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50b5, "54994    B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x50b9, "54994E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50d1, "54991H   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50f1, "54994H   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x50f9, "50994E   B0", &blackfin_b0, INTER_PHY_TYPES_US1K2KIR_M },
#endif
#ifdef CONFIG_BCM_PHY_SHORTFIN_B0
    { 0x3590, 0x5001, "84898    B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x5005, "84896    B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x5009, "54998S   B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x500d, "54998ES  B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x5011, "54998    B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x5015, "54998E   B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x5019, "54994L   B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
    { 0x3590, 0x501d, "54994EL  B0", &shortfin_b0, INTER_PHY_TYPE_USXGMII_MP_M },
#endif
#ifdef CONFIG_BCM_PHY_LONGFIN_A0
    { 0x3590, 0x5180, "84891LM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5184, "54991LM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5190, "84891M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5194, "54991M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x5198, "54991EM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x5188, "54991ELM A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x51a0, "84892M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51a4, "54992M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x51a8, "54992EM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x51b0, "84894M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51b4, "54994M   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x51b8, "54994EM  A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x51d0, "54991H   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51f0, "54994H   A0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
#endif
#ifdef CONFIG_BCM_PHY_LONGFIN_B0
    { 0x3590, 0x5181, "84891LM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5185, "54991LM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5191, "84891M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x5195, "54991M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x5199, "54991EM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x5189, "54991ELM B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x518d, "50991ELM B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x51a1, "84892M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51a5, "54992M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x51a9, "54992EM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x51b1, "84894M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51b5, "54994M   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },
    { 0x3590, 0x51b9, "54994EM  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },
    { 0x3590, 0x50c1, "49418      ", &longfin_a0,   INTER_PHY_TYPES_US1K2KIR5KIR_M },	// 4912 integrated XGPHY
    { 0x3590, 0x51c1, "49418M     ", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR_M },	// 4912 integrated XGPHY
    { 0x3590, 0x50cd, "4912       ", &longfin_a0,   INTER_PHY_TYPES_US1K2KIR_M },	// 4912 integrated XGPHY
    { 0x3590, 0x51cd, "4912M      ", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR_M },	// 4912 integrated XGPHY
    { 0x3590, 0x51d1, "54991H   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51d5, "54991SK  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51f1, "54994H   B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },
    { 0x3590, 0x51f5, "54994SK  B0", &longfin_a0_m, INTER_PHY_TYPES_US1K2KIR5KIR10R_M },

#endif
};

static uint32_t enabled_phys;
static uint32_t macsec_phys;
bus_drv_t *bus_drv;

#define BUS_READ(a, b, c, d)       if ((ret = _bus_read(a, b, c, d))) goto Exit;
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
#define CMD_SET_MACSEC_ENABLE                       0x805E
#define CMD_GET_MACSEC_ENABLE                       0x805F
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

#define STEPS                                       10

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

int cmd_handler(phy_dev_t *phy_dev, uint16_t cmd_code, uint16_t *data1, uint16_t *data2, uint16_t *data3, uint16_t *data4, uint16_t *data5)
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
    case CMD_SET_MACSEC_ENABLE:
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
    case CMD_GET_MACSEC_ENABLE:
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

static phy_desc_t *_phy_get_ext3_desc_by_phyid(int phyid1, int phyid2)
{
    int j;

    for (j = 0; j < sizeof(phy_desc)/sizeof(phy_desc[0]); j++)
    {
        if (phy_desc[j].phyid1 != phyid1 || phy_desc[j].phyid2 != phyid2)
            continue;

        return &phy_desc[j];
    }
    return 0;
}

static phy_desc_t *_phy_get_ext3_desc(phy_dev_t *phy_dev)
{
    uint16_t phyid1, phyid2;
    int ret;

    PHY_READ(phy_dev, 0x01, 0x0002, &phyid1);
    PHY_READ(phy_dev, 0x01, 0x0003, &phyid2);

    return _phy_get_ext3_desc_by_phyid(phyid1, phyid2);
Exit:
    return 0;
}

static void serdes_register_read(phy_dev_t *phy_dev, int dev, int reg, uint16_t *val)
{
    int ret;
    PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x2004);
    PHY_READ(phy_dev, dev, reg, val);
    PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x0001);
Exit:
    return;
}

static void serdes_register_write(phy_dev_t *phy_dev, int dev, int reg, uint16_t val)
{
    int ret;
    PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x2004);
    PHY_WRITE(phy_dev, dev, reg, val);
    PHY_WRITE(phy_dev, 0x1e, 0x4110, 0x0001);
Exit:
    return;
}

void phy_shortfin_short_amble_workaround(phy_dev_t *phy_dev)
{
    uint16_t val;
    static uint16_t reg_save, reg_1c600h_default, reg_1c6e2h_default;

    return;

    if (!phy_dev->descriptor)
    {
        phy_dev->descriptor = _phy_get_ext3_desc(phy_dev);
        if (!phy_dev->descriptor)
            return;
    }

    if (((phy_desc_t *)phy_dev->descriptor)->firmware != &shortfin_b0)
        return; 

    if (!reg_save)
    {
        serdes_register_read(phy_dev, 0x01, 0xc600, &reg_1c600h_default);
        serdes_register_read(phy_dev, 0x01, 0xc6e2, &reg_1c6e2h_default);
        reg_save = 1;
    }

    if (phy_dev->link == 0)
        return;

    if (phy_dev->speed == PHY_SPEED_1000)
    {
        serdes_register_read(phy_dev, 0x01, 0xc600, &val);
        val &= ~0x000f;
        /* (en_1588_rxtx_phy.3) */
        val |= 0x0003;
        /* (sel_int_extclk phy,0) */
        val |= 0x0004;
        serdes_register_write(phy_dev, 0x01, 0xc600, val);

        serdes_register_read(phy_dev, 0x01, 0xc6e2, &val);

        /* (en_preabmle_fix phy ,1) */
        val &= ~0x0cc0;
        val |= ((0x0 & 0x3) << 6);
        val |= ((0x3 & 0x3) << 10);
        serdes_register_write(phy_dev, 0x01, 0xc6e2, val);
    }
    else
    {
        serdes_register_write(phy_dev, 0x01, 0xc600, reg_1c600h_default);
        serdes_register_write(phy_dev, 0x01, 0xc6e2, reg_1c6e2h_default);
    }
    return;
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

#define XFI_MODE_IDLE_STUFFING           0 /* Idle Stuffing mode over XFI interface */
#define XFI_MODE_BASE_X                  1 /* 2.5GBase-X or 5GBase-X */
#define XFI_MODE_BASE_R                  2 /* 2.5GBase-R or 5GBase-R */

#if !defined(DSL_DEVICES)
static int _phy_inter_phy_types_set(phy_dev_t *phy_dev, inter_phy_type_t inter_phy_types)
{
    int ret;
    uint16_t data1, data2, data3, data4, data5;

    data1 = XFI_MODE_BASE_X ; /* XFI mode in 2.5G speed */
    data2 = XFI_MODE_BASE_X ; /* XFI mode in 5G speed */

    if (inter_phy_types & INTER_PHY_TYPE_2P5GBASE_R_M)
        data1 = XFI_MODE_BASE_R;

    if (inter_phy_types & INTER_PHY_TYPE_2P5GIDLE_M)
        data1 = XFI_MODE_IDLE_STUFFING;

    if (inter_phy_types & INTER_PHY_TYPE_5GBASE_R_M)
        data2 = XFI_MODE_BASE_R;
    
    if (inter_phy_types & INTER_PHY_TYPE_5GIDLE_M)
        data2 = XFI_MODE_IDLE_STUFFING;

    /* Set XFI modes for 2.5G and 5G */
    if ((ret = cmd_handler(phy_dev, CMD_SET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL)))
        goto Exit;

    /* Configure USXGMII mode */
    data1 = inter_phy_types & (INTER_PHY_TYPE_USXGMII_M | INTER_PHY_TYPE_USXGMII_MP_M)? 1 : 0; /* Enable */
    data2 = 1; /* AN,*/
    data3 = 0x124; /* baud rate 10G, quad speed 2.5G */
    data4 = 0; /* 0 = Broadcom mode */
    data5 = 0; /* 0 = MAC/PHY frequency is locked without PPM offset */

    if ((ret = cmd_handler(phy_dev, CMD_SET_USXGMII, &data1, &data2, &data3, &data4, &data5)))
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
#endif

static int inter_phy_current_types_2p5g_5g_get(phy_dev_t *phy_dev, uint32_t *types)
{
    int rc = 0;
    uint16_t data1, data2;

    *types = 0;
    rc = cmd_handler(phy_dev, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);

    switch(data1)
    {
        case 0:
            *types |= INTER_PHY_TYPE_2P5GIDLE_M;
            break;
        case 1:
            *types |= INTER_PHY_TYPE_2500BASE_X_M;
            break;
        case 2:
            *types |= INTER_PHY_TYPE_2P5GBASE_R_M;
            break;
    }

    switch(data2)
    {
        case 0:
            *types |= INTER_PHY_TYPE_5GIDLE_M;
            break;
        case 1:
            *types |= INTER_PHY_TYPE_5000BASE_X_M;
            break;
        case 2:
            *types |= INTER_PHY_TYPE_5GBASE_R_M;
            break;
    }

    return 0;
}

static int inter_phy_type_usxgmii_get(phy_dev_t *phy_dev)
{
    uint16_t data1, data2, data3;

    cmd_handler(phy_dev, CMD_GET_USXGMII, &data1, &data2, &data3, NULL, NULL);

    return data1 > 0;
}

static int inter_type_max_speed(phy_dev_t *phy_dev);
static inter_phy_type_t _phy_current_inter_phy_type_get(phy_dev_t *phy_dev)
{
    uint32_t sw_types, hw_types;
    uint32_t types;

    phy_dev_configured_inter_phy_types_get(phy_dev, &sw_types);


    if (inter_phy_type_usxgmii_get(phy_dev))
    {
        phy_dev_inter_phy_types_get(phy_dev, INTER_PHY_TYPE_UP, &types);
        if (types & INTER_PHY_TYPE_USXGMII_MP_M)
            return INTER_PHY_TYPE_USXGMII_MP;
        else
            return INTER_PHY_TYPE_USXGMII;
    }

    if (inter_type_max_speed(phy_dev) <= PHY_SPEED_1000 &&
        sw_types & INTER_PHY_TYPE_SGMII_M)
        return INTER_PHY_TYPE_SGMII;

    if (!phy_dev->link)
        return INTER_PHY_TYPE_UNKNOWN;

    inter_phy_current_types_2p5g_5g_get(phy_dev, &hw_types);
    switch(phy_dev->speed)
    {
        case PHY_SPEED_100:
        case PHY_SPEED_1000:
            if (sw_types & INTER_PHY_TYPE_SGMII_M)
                return INTER_PHY_TYPE_SGMII;
            return INTER_PHY_TYPE_UNKNOWN;  /* Bug */

        case PHY_SPEED_2500:
            if (hw_types & INTER_PHY_TYPE_2P5GBASE_R_M)
                return INTER_PHY_TYPE_2P5GBASE_R;
            if (hw_types & INTER_PHY_TYPE_2500BASE_X_M)
                return INTER_PHY_TYPE_2500BASE_X;
            if (hw_types & INTER_PHY_TYPE_2P5GIDLE_M)
                return INTER_PHY_TYPE_2P5GIDLE;
            return INTER_PHY_TYPE_UNKNOWN;  /* Bug */

        case PHY_SPEED_5000:
            if (hw_types & INTER_PHY_TYPE_5GBASE_R_M)
                return INTER_PHY_TYPE_5GBASE_R;
            if (hw_types & INTER_PHY_TYPE_5000BASE_X_M)
                return INTER_PHY_TYPE_5000BASE_X;
            if (hw_types & INTER_PHY_TYPE_5GIDLE_M)
                return INTER_PHY_TYPE_5GIDLE;
            return INTER_PHY_TYPE_UNKNOWN;  /* Bug */

        case PHY_SPEED_10000:
            return INTER_PHY_TYPE_10GBASE_R;

        default:
            break;
    }

    return INTER_PHY_TYPE_UNKNOWN;  /* Bug */
}

static int inter_type_max_speed(phy_dev_t *phy_dev)
{
    uint32_t caps;

    phy_dev_caps_get(phy_dev, CAPS_TYPE_SUPPORTED, &caps);
    return phy_caps_to_max_speed(caps);
}

static int inter_phy_type_usxgmii_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, inter_phy_type_t type)
{
    int rc = 0;
    uint16_t data1, data2, data3, data4, data5;
    phy_speed_t speed;

    rc = cmd_handler(phy_dev, CMD_GET_USXGMII, &data1, &data2, &data3, NULL, NULL);
    data1 = type == INTER_PHY_TYPE_USXGMII; /* Enable/Disable bit */
    data2 = 1;  /* AN, only on */

    if (data1)
    {
        switch (phy_dev->usxgmii_m_type)
        {
            case USXGMII_S:
                speed = inter_type_max_speed(phy_dev);
                if (speed >= PHY_SPEED_10000)
                    data3 = 4;
                else if (speed >= PHY_SPEED_5000)
                    data3 = 2;
                else
                    data3 = 1;
                break;
                /* data sheet values */
                /*
            case USXGMII_M_10G_Q:
                data3 = (1<<8);
                break;
            case USXGMII_M_10G_D:
                data3 = (2<<8);
                break;
            case USXGMII_M_10G_S:
                data3 = (4<<8);
                break;
                */
            /* PLP script values */
            case USXGMII_M_10G_Q:
            case USXGMII_M_10G_D:
            case USXGMII_M_10G_S:
                data3 = 0x124;
                data4 = 0; /* 0 = Broadcom mode */
                data5 = 0; /* 0 = MAC/PHY frequency is locked without PPM offset */
                break;
            default:
                printk("No supported USXGMII mode: %d\n", phy_dev->usxgmii_m_type); 
                return -1;
        }
    }

    rc += cmd_handler(phy_dev, CMD_SET_USXGMII, &data1, &data2, &data3, NULL, NULL);
    return rc;
}

static int inter_phy_type_2P5G5G_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, inter_phy_type_t type)
{
    int rc = 0;
    uint16_t data1, data2;

    rc = cmd_handler(phy_dev, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);

    switch(type)
    {
        case INTER_PHY_TYPE_2P5GIDLE:
            data1 = XFI_MODE_IDLE_STUFFING;
            break;
        case INTER_PHY_TYPE_2500BASE_X:
            data1 = XFI_MODE_BASE_X;
            break;
        case INTER_PHY_TYPE_2P5GBASE_R:
            data1 = XFI_MODE_BASE_R;
            break;
        case INTER_PHY_TYPE_5GIDLE:
            data2 = XFI_MODE_IDLE_STUFFING;
            break;
        case INTER_PHY_TYPE_5000BASE_X:
            data2 = XFI_MODE_BASE_X;
            break;
        case INTER_PHY_TYPE_5GBASE_R:
            data2 = XFI_MODE_BASE_R;
            break;
        default:
            /* do nothing */
            break;
    }

    rc += cmd_handler(phy_dev, CMD_SET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL);

    return rc;
}

static int _phy_configured_inter_phy_types_set(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t types)
{
    int rc = 0;
    inter_phy_type_t best_type;
    int disable_usxgmii;

    best_type = phy_get_best_inter_phy_configure_type(phy_dev, types, PHY_SPEED_2500);

    if (best_type == INTER_PHY_TYPE_USXGMII || best_type == INTER_PHY_TYPE_USXGMII_MP)
    {
        inter_phy_type_usxgmii_set(phy_dev, if_dir, INTER_PHY_TYPE_USXGMII);
        phy_dev->current_inter_phy_type = best_type;
        return 0;
    }

    if (best_type != INTER_PHY_TYPE_UNKNOWN)
    {
        rc += inter_phy_type_2P5G5G_set(phy_dev, if_dir, best_type);
        disable_usxgmii = 1;
    }

    best_type = phy_get_best_inter_phy_configure_type(phy_dev, types, PHY_SPEED_5000);
    if (best_type != INTER_PHY_TYPE_UNKNOWN)
    {
        rc += inter_phy_type_2P5G5G_set(phy_dev, if_dir, best_type);
        disable_usxgmii = 1;
    }

    if (disable_usxgmii)
        rc += inter_phy_type_usxgmii_set(phy_dev, if_dir, best_type);

    return rc;
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

static int _phy_eth_wirespeed_get(phy_dev_t *phy_dev, int *enable)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_dev, 0x07, 0x902f, &val);

    *enable = val & (1 << 4) ? 1 : 0; /* Ethernet@Wirespeed Enabled */

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

#if !defined(DSL_DEVICES)
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
#endif

static int _phy_xfi_txrx_polarity_set(phy_dev_t *phy_dev, int inverse, int tx_dir)
{
    int ret;
    uint16_t data2, data3;
    uint16_t type;

    if ((ret = cmd_handler(phy_dev, CMD_GET_XFI_POLARITY, &type, &data2, &data3, NULL, NULL)))
        goto Exit;

    if (tx_dir)
        data2 = inverse > 0;
    else
        data3 = inverse > 0;

    if ((ret = cmd_handler(phy_dev, CMD_SET_XFI_POLARITY, &type, &data2, &data3, NULL, NULL)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_xfi_tx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    return _phy_xfi_txrx_polarity_set(phy_dev, inverse, 1);
}

static int _phy_xfi_rx_polarity_set(phy_dev_t *phy_dev, int inverse)
{
    return _phy_xfi_txrx_polarity_set(phy_dev, inverse, 0);
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

#define EEE_MODE_DISABLED               0 /* EEE Disabled*/
#define EEE_MODE_NATIVE_EEE             1 /* Native EEE */
#define EEE_MODE_AUTOGREEEN_FIXED       2 /* AutoGrEEEn Fixed Latency */
#define EEE_MODE_AUTOGREEEN_VARIABLE    3 /* AutoGrEEEn Variable Latency */

/* Note: AutoGrEEEn mode is not supported when idle stuffing is enabled on 2.5/5G rates */

static int _phy_eee_mode_set(phy_dev_t *phy_dev, uint32_t caps)
{
    int ret;
    uint16_t val = 0, data1, data2, data3, data4;
    uint8_t mode = EEE_MODE_AUTOGREEEN_FIXED;

    if ((ret = cmd_handler(phy_dev, CMD_GET_XFI_2P5G_5G_MODE, &data1, &data2, NULL, NULL, NULL)))
        goto Exit;

    /* 10G          bits 0:1 */
    /* 100M/1G      bits 2:3 */
    /* 2.5G         bits 4:5 */
    /* 5G           bits 6:7 */
    val |= ((caps & PHY_CAP_100_HALF) || (caps & PHY_CAP_100_FULL)) ? (mode << 2) : 0;
    val |= ((caps & PHY_CAP_1000_HALF) || (caps & PHY_CAP_1000_FULL)) ? (mode << 2) : 0;
    val |= ((caps & PHY_CAP_2500) ? ((data1 == XFI_MODE_IDLE_STUFFING ? EEE_MODE_NATIVE_EEE : mode) << 4) : 0);
    val |= ((caps & PHY_CAP_5000) ? ((data2 == XFI_MODE_IDLE_STUFFING ? EEE_MODE_NATIVE_EEE : mode) << 6) : 0);
    val |= ((caps & PHY_CAP_10000)) ? (mode << 0) : 0;

    data1 = val;    /* Bitmap of EEE modes per speed */
    data2 = 0;      /* AutoGrEEEn High Threshold */
    data3 = 0x7a12; /* AutoGrEEEn Low Threshold */
    data4 = 0x0480; /* AutoGrEEEn Latency */

    /* Set EEE mode */
    if ((ret = cmd_handler(phy_dev, CMD_SET_EEE_MODE, &data1, &data2, &data3, &data4, NULL)))
        goto Exit;

Exit:
    return ret;
}

static int _phy_an_restart(phy_dev_t *phy_dev)
{
    int ret;
    uint16_t val;

    PHY_READ(phy_dev, 0x07, 0xffe0, &val);
    val |= (1 << 9);
    PHY_WRITE(phy_dev, 0x07, 0xffe0, val);

Exit:
    return ret;
}

static int _phy_eee_set(phy_dev_t *phy_dev, int enable)
{
    int ret;
    uint16_t val;
    uint32_t caps;

    if ((ret = phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
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

    phy_shortfin_short_amble_workaround(phy_dev);

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

    if ((ret = phy_dev_caps_get(phy_dev, CAPS_TYPE_ADVERTISE, &caps)))
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

    if ((ret = phy_dev_caps_set(phy_dev, caps)))
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

#if !defined(DSL_DEVICES)
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
#endif

#if defined(DSL_DEVICES)
extern int dsl_runner_ext3_phy_init(phy_dev_t *phy_dev);
static int _phy_init(phy_dev_t *phy_dev)
{
    int ret = dsl_runner_ext3_phy_init(phy_dev);
    
    if (ret)
        goto Exit;

#ifdef MACSEC_SUPPORT
    if (macsec_phys & (1 << (phy_dev->addr)))
    {
        printk("phy 0x%x is macsec capable, initializing macsec module\n", phy_dev->addr);
        ret = phy_macsec_pu_init(phy_dev);
    }
#endif
Exit:
    return ret;
}
#else
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

    /* Set SerDes mode for 2.5G/5G speeds */
    if ((ret = _phy_inter_phy_types_set(phy_dev, phy_dev->inter_phy_types)))
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

    if (phy_dev->xfi_tx_polarity_inverse)
    {
        printk("Invert XFI Tx Polarity of PHY %s at %d\n", phy_dev_get_phy_name(phy_dev), phy_dev->addr);
        _phy_xfi_tx_polarity_set(phy_dev, phy_dev->xfi_tx_polarity_inverse);
    }

    if (phy_dev->xfi_rx_polarity_inverse)
    {
        printk("Invert XFI Rx Polarity of PHY %s at %d\n", phy_dev_get_phy_name(phy_dev), phy_dev->addr);
        _phy_xfi_rx_polarity_set(phy_dev, phy_dev->xfi_rx_polarity_inverse);
    }

    if ((ret = phy_dev_caps_set(phy_dev, PHY_CAP_PAUSE | PHY_CAP_REPEATER)))
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

#ifdef MACSEC_SUPPORT
    if (macsec_phys & (1 << (phy_dev->addr)))
    {
        printk("phy 0x%x is macsec capable, initializing macsec module\n", phy_dev->addr);
        ret = phy_macsec_pu_init(phy_dev);
    }
#endif

Exit:
    return ret;
}
#endif

static int inline _bus_read(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t *val)
{
    return bus_drv && bus_drv->c45_read(addr, dev, reg, val);
}

static int inline _bus_write(uint32_t addr, uint16_t dev, uint16_t reg, uint16_t val)
{
    return bus_drv && bus_drv->c45_write(addr, dev, reg, val);
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

static int get_file_size(const char *file_path)
{
    struct path p;
    struct kstat ks;

    if (kern_path(file_path, LOOKUP_FOLLOW, &p))
        return -1;

    vfs_getattr(&p, &ks, STATX_BASIC_STATS, AT_STATX_SYNC_AS_STAT);

    return ks.size;
}

static int load_firmware_file(int phy_addr, char *name)
{
    struct file *fp;
    int len = 0, ret = 0;
    unsigned char buf[1024];
    char fw_path[64];
    int fw_size, written = 0;

    snprintf(fw_path, sizeof(fw_path), "/rom/etc/fw/%s_firmware.bin", name);

    fw_size = get_file_size(fw_path);
    if (fw_size <= 0)
    {
        printk("Firmware file size could not be read\n");
        return -1;
    }

    fp = filp_open(fw_path, O_RDONLY, 0);
    if (fp < 0)
    {
        printk("Firmware file could not be opened\n");
        return -1;
    }

    fp->f_pos = 0;

    printk("Upload the firmware into the on-chip memory: file=%s size=%d\n", fw_path, fw_size);

    while (1)
    {
        int i;
        uint32_t data;

        len = kernel_read(fp, buf, sizeof(buf), &fp->f_pos);
        if (len <= 0)
            break;

        for (i = 0; i < len; i += sizeof(uint32_t))
        {
            data = *(uint32_t *)(buf + i);

            BUS_WRITE(phy_addr, 0x01, 0xa81c, data >> 16); /* upper 16 bits */
            BUS_WRITE(phy_addr, 0x01, 0xa81b, data & 0xffff); /* lower 16 bits */
        }

        written += len;
        printk("%d%%\e[1A\n", 100 * written / fw_size);
    }
    printk("\n");

Exit:
    filp_close(fp, NULL);

    if (len < 0)
        printk("Failed to read firmware file \n");
    if (ret)
        printk("Failed to load firmware file \n");

    return ret;
}

static int print_firmware_version(int phy_addr)
{
    int ret;
    uint16_t fw_version, fw_build, fw_main, fw_branch;

    BUS_READ(phy_addr, 0x1e, 0x400f, &fw_version);
    fw_branch = (fw_version >> 0) & 0x7f;
    fw_main = (fw_version >> 7) & 0x1f;
    fw_build = (fw_version >> 12) & 0xf;

    printk("Firmware loading completed successfully: v%d.%d.%d\n\n", fw_branch, fw_main, fw_build);

Exit:
    return ret;
}

#if defined(CONFIG_BCM_PHY_MAKO_A0)
static int load_mako(firmware_t *firmware)
{
    int i, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, b_phy_map;

    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);

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

    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0038);

    ret = load_firmware_file(base_phy_addr, firmware->name);
    if (ret)
        goto Exit;

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
    pr_cont("Verify that the processors are running: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while ((ret != phy_map) && i--);

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    /* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    pr_cont("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while ((ret != phy_map) && i--);

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    print_firmware_version(base_phy_addr);

Exit:
    return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_ORCA_A0) || defined(CONFIG_BCM_PHY_ORCA_B0)
static int load_orca(firmware_t *firmware)
{
    int i, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, b_phy_map;

    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);

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
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0038);

    ret = load_firmware_file(base_phy_addr, firmware->name);
    if (ret)
        goto Exit;

    /* 4. Reset the processors to start execution of the code in the on-chip memory */
    printk("Reset the processors to start execution of the code in the on-chip memory\n");

    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa008, 0x0000);
    BUS_WRITE_ALL(phy_map, 0x1e, 0x8004, 0x5555);
    BUS_WRITE(base_phy_addr, 0x1, 0x8003, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0x0000, 0x8000);

    /* 5. Verify that the processors are running */
    pr_cont("Verify that the processors are running: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while ((ret != phy_map) && i--);

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    /* 6. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    pr_cont("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while ((ret != phy_map) && i--);

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    print_firmware_version(base_phy_addr);

Exit:
    return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_SHORTFIN_B0)
static int _bus_write_reg32(uint32_t addr, uint32_t reg, uint32_t val)
{
    _bus_write(addr, 0x01, 0xa819, reg & 0xffff);
    _bus_write(addr, 0x01, 0xa81a, (reg>>16) & 0xffff);
    _bus_write(addr, 0x01, 0xa81b, val & 0xffff);
    _bus_write(addr, 0x01, 0xa81c, (val>>16) & 0xffff);
    _bus_write(addr, 0x01, 0xa817, 0x0009);
    return 0;
}

static int load_shortfin(firmware_t *firmware)
{
    int i, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, b_phy_map;


    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);
    
    if (phy_cnt > 1)
    {
        printk("Turn on broadcast mode to accept write operations\n");
        _bus_write(base_phy_addr, 0x1e, 0x8114, 0xffff);
    }

    /* 
        0. Set BUSY_IN pin to 0; 
       This is a work around for floating BUSY_IN pin on 54994EL daugter card,
       but it is also a correct configuration for any correct designed EXT3 PHY board with BUSY_IN grounded.
    */
    _bus_write(base_phy_addr, 0x1e, 0x80b5, 0x0001);
    udelay(2000);


    /* 1. Halt the BCM5499X processors operation */
    printk("Halt the PHYs processors operation\n");

    /* Stop Watchdog timer */
    _bus_write(base_phy_addr, 0x1e, 0x418c, 0x0000);

    /* Setup Clock */
    _bus_write(base_phy_addr, 0x1e, 0x4188, 0x48f0);

    _bus_write_reg32(base_phy_addr, 0xf4003000, 0x00000121);

    /* ' IOPAD_CFG_SPARE_REG5, clear PLL done status */
    _bus_write(base_phy_addr, 0x1e, 0x80a6, 0x0000);

    /* 'COMMON_MAILBOX_9, clear serdes loader done status */
    _bus_write(base_phy_addr, 0x01, 0xa819, 0x4020);
    _bus_write(base_phy_addr, 0x01, 0xa81a, 0xf603);
    _bus_write(base_phy_addr, 0x01, 0xa81b, 0x0000);
    _bus_write(base_phy_addr, 0x01, 0xa81c, 0x0000);
    _bus_write(base_phy_addr, 0x01, 0xa817, 0x0005);

    /* Issue soft reset on all ports */
    _bus_write(base_phy_addr, 0x01, 0xa819, 0x0000); 
    _bus_write(base_phy_addr, 0x01, 0xa81a, 0xf602);
    _bus_write(base_phy_addr, 0x01, 0xa81b, 0x8000);
    _bus_write(base_phy_addr, 0x01, 0xa81c, 0x0000);
    _bus_write(base_phy_addr, 0x01, 0xa817, 0x0005);

    /* 3. Upload the firmware into the on-chip memory of the devices */
    _bus_write(base_phy_addr, 0x01, 0xa81a, 0xf790);
    _bus_write(base_phy_addr, 0x01, 0xa819, 0x0000);
    _bus_write(base_phy_addr, 0x01, 0xa817, 0x0038);

    ret = load_firmware_file(base_phy_addr, firmware->name);
    if (ret)
        goto Exit;

    _bus_write(base_phy_addr, 0x01, 0xa817, 0x0000);

    /* 4. Reset the processors to start execution of the code in the on-chip memory */
    printk("Reset the processors to start execution of the code in the on-chip memory\n");
    _bus_write(base_phy_addr, 0x01, 0xa008, 0x0000);

    _bus_write_reg32(base_phy_addr, 0xf0003000, 0x00000020);

    /* 5. Disable broadcast if phy cnt > 1 */
    if (phy_cnt > 1)
        BUS_WRITE_ALL(base_phy_addr, 0x1e, 0x8114, 0x0000);
  
    /* 6. Verify that the processors are running */
    pr_cont("Verify that the processors are running: ");
    for (i = 1000, ret=0; i && ret != phy_map; i--)
    {
        udelay(1000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    }

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");
    if (ret != phy_map) BUG();
    
    /* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    pr_cont("Verify that the firmware has been loaded with good CRC: ");
    for (i = 1000, ret=0; i && ret != phy_map; i--)
    {
        udelay(1000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    }
    if (ret != phy_map) BUG();

    print_firmware_version(base_phy_addr);
    ret = 0;

Exit:
    return ret;
}
#endif

#if defined(CONFIG_BCM_PHY_BLACKFIN_A0) || defined(CONFIG_BCM_PHY_BLACKFIN_B0) || defined(CONFIG_BCM_PHY_LONGFIN_A0) || defined(CONFIG_BCM_PHY_LONGFIN_B0) || defined(CONFIG_BCM_PHY_68880_XPHY)
static int load_blackfin(firmware_t *firmware)
{
    int i, ret, base_phy_addr, phy_cnt;
    uint32_t phy_map, b_phy_map;

    phy_map = firmware->map;
    base_phy_addr = get_base_phy_addr(phy_map); /* select the min PHY address for broadcast operation */
    b_phy_map = bcast_phy_map(phy_map); /* phy map for broadcast configuration - exclude the min PHY address */
    phy_cnt = phy_count(phy_map);

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
    BUS_WRITE(base_phy_addr, 0x01, 0xa81a, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa819, 0x0000);
    BUS_WRITE(base_phy_addr, 0x01, 0xa817, 0x0038);

    ret = load_firmware_file(base_phy_addr, firmware->name);
    if (ret)
        goto Exit;

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
    pr_cont("Verify that the processors are running: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x01, 0x0000, 0x2040, 0xffff);
    } while ((ret != phy_map) && i--);

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    /* 7. Verify that the firmware has been loaded into the on-chip memory with a good CRC */
    pr_cont("Verify that the firmware has been loaded with good CRC: ");

    i = 1000;
    do
    {
        udelay(2000);
        ret = _bus_read_all(phy_map, 0x1e, 0x400d, 0x4000, 0xc000);
    } while ((ret != phy_map) && i--);

    pr_cont("%s\n", ret != phy_map ? "Failed" : "OK");

    ret = ret != phy_map;
    if (ret)
        goto Exit;

    print_firmware_version(base_phy_addr);

Exit:
    return ret;
}
#endif

static void phy_reset_lift(phy_dev_t *phy_dev)
{
    if (!dt_gpio_exists(phy_dev->gpiod_phy_reset))
        return;

    dt_gpio_set_direction_output(phy_dev->gpiod_phy_reset, 1);
    printk(" Lift PHY at address %d out of Reset\n", phy_dev->addr);
    dt_gpio_set_value(phy_dev->gpiod_phy_reset, 1);
    udelay(2000);
    dt_gpio_set_value(phy_dev->gpiod_phy_reset, 0);
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

    printk("\nDetecting PHYs...\n");

	phys_reset_lift(enabled_phys);
    {
        uint16_t rd_phyid1[32], rd_phyid2[32];

        for (i = 0; i < 32; i++)
        {
            rd_phyid1[i] = rd_phyid2[i] = 0;
            if (!(enabled_phys & (1 << i)))
                continue;
            if ((_bus_read(i, 1, 2, &rd_phyid1[i])))
                continue;

            if ((_bus_read(i, 1, 3, &rd_phyid2[i])))
                continue;
        }

        for (i = 0; i < sizeof(firmware_list)/sizeof(firmware_list[0]); i++)
        {
            for (j = 0; j < sizeof(phy_desc)/sizeof(phy_desc[0]); j++)
            {
                if (phy_desc[j].firmware != firmware_list[i])
                    continue;

                for (k = 0, phy_map = 0; k < 32; k++)
                {
                    if (!(enabled_phys & (1 << k)))
                        continue;
                    if (phy_desc[j].phyid1 == rd_phyid1[k] && phy_desc[j].phyid2 == rd_phyid2[k])
                        phy_map |= (1 << k);
                }

                firmware_list[i]->map |= phy_map;

                if (phy_map)
                {
                    pr_cont("%s %x:%x --> ", phy_desc[j].name, phy_desc[j].phyid1, phy_desc[j].phyid2);
                    for (k = 0; k < 32 ; k++)
                    {
                        if (!(phy_map & (1 << k)))
                            continue;

                        pr_cont("0x%x ", k);
                    }
                    printk("\n");
                }
            }
        }
    }

    for (i = 0; i < sizeof(firmware_list)/sizeof(firmware_list[0]); i++)
    {
        phy_map = firmware_list[i]->map;

        if (!phy_map)
            continue;

        if (firmware_list[i]->macsec_capable)
            macsec_phys |= phy_map;

        printk("Loading firmware into detected PHYs: map=0x%x count=%d\n", phy_map, phy_count(phy_map));

        firmware_list[i]->load(firmware_list[i]);
    }

    return ret;
}

static int _phy_dev_add(phy_dev_t *phy_dev)
{
    bus_drv = phy_dev->bus_drv;
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
#if defined(DSL_DEVICES)
    extern void phy_bus_probe(bus_drv_t *bus_drv);
    phy_bus_probe(bus_drv);
#endif
    if (_phy_cfg(enabled_phys))
    {
        printk("Failed to initialize the driver\n");
        return -1;
    }

    phy_drv->initialized = 1;

    return 0;
}

static int _phy_inter_phy_types_get(phy_dev_t *phy_dev, inter_phy_types_dir_t if_dir, uint32_t *inter_phy_types)
{
    uint16_t phyid1, phyid2;
    int j, ret;

    *inter_phy_types = 0;
    PHY_READ(phy_dev, 0x01, 0x0002, &phyid1);
    PHY_READ(phy_dev, 0x01, 0x0003, &phyid2);

    for (j = 0; j < sizeof(phy_desc)/sizeof(phy_desc[0]); j++)
    {
        if (phy_desc[j].phyid1 != phyid1 || phy_desc[j].phyid2 != phyid2)
            continue;
        *inter_phy_types = phy_desc[j].inter_phy_types;
#ifdef MACSEC_SUPPORT   /* MacSec not working with USXGMII mode in EXT3 PHY for now */
        *inter_phy_types &= ~(INTER_PHY_TYPE_USXGMII_M);
#endif
        phy_dev->inter_phy_types = *inter_phy_types;
        return 0;
    }

    /* Should not happen */
    printk("No EXT3 device found for %d\n", phy_dev->addr);
Exit:
    return -1;
}

static char *_phy_get_phy_name(phy_dev_t *phy_dev)
{
    if (!phy_dev->descriptor)
    {
        phy_dev->descriptor = _phy_get_ext3_desc(phy_dev);
        if (!phy_dev->descriptor)
            return "No Descrpter Found."; // bug
    }

    return ((phy_desc_t *)phy_dev->descriptor)->name;
}

#define EXT3_ECD_CTRL_STATUS_D  0x1e
#define EXT3_ECD_CTRL_STATUS_R  0x4006
    #define EXT3_ECDRUN_IMMEDIATE   (1<<15)
    #define EXT3_ECDBREAK_LINK      (1<<12)
    #define EXT3_ECDDIAG_IN_PROG    (1<<11)

#define EXT3_ECD_RESULTS_D      0x1
#define EXT3_ECD_RESULTS_R      0xa896
    #define EXT3_PACD_CODE_SHIFT            4
    #define EXT3_PACD_CODE_MASK             0xf
    #define EXT3_PACD_CODE_INVALID          0x0
    #define EXT3_PACD_CODE_PAIR_OK          0x1
    #define EXT3_PACD_CODE_PAIR_OPEN        0x2
    #define EXT3_PACD_CODE_PAIR_INTRA_SHORT 0x3
    #define EXT3_PACD_CODE_PAIR_INTER_SHORT 0x4
    #define EXT3_PACD_CODE_PAIR_GET(v, p)          (((v)>>((p)*4))&0xf)
    #define EXT3_PACD_CODE_PAIR_SET(v, p)          (((v)&0xf)<<((p)*4))
    #define EXT3_PACD_CODE_PAIR_ALL_OK      0x1111
    #define EXT3_PACD_CODE_PAIR_ALL_OPEN    0x2222

#define EXT3_ECD_CABLE_LEN_D 0x1
#define EXT3_ECD_CABLE_LEN_R 0xa897

/*
    Work around some hardware inconsistency
    Pick up the most popular length from 4 pairs
*/
static void cable_length_pick_link_up(int *pair_len, int excluded_pair)
{
    int len[4]={0};
    int i, j, k, m;

    for (i=0, k=0; i<4; i++) {
        if (excluded_pair & (1<<i))  /* Exclude failed CD pair */
            continue;

        for(j=0; j<k; j++)
            if (pair_len[j] == pair_len[i])
                break;

        if (j==k)
            k++;
        len[j]++;
    }

    for (i=0, j=0, m=0; i<k; i++) {
        if (len[i] == 0)    /* If result is zero, exclude the pair from picking */
            continue;

        if(len[i]>j) {
            j=len[i];
            m=i;
        }
    }

    m = pair_len[m];
    for (i=0; i<4; i++)
        pair_len[i] = m;
}

int _phy_enhanced_cable_diag_run(phy_dev_t *phy_dev, int *result, int *pair_len)
{
    uint16_t v16, ctl_reg;
    int i, j, ret = 0, excluded_pair = 0;
    int apd_enabled, phy_link;
    unsigned long jiffie;
    int retries = 0;
#define EXT3_ECDCHECK_SECS 3
#define EXT3_ECDMAX_RETRIES 3

    phy_dev_apd_get(phy_dev, &apd_enabled);
    if (apd_enabled)
        phy_dev_apd_set(phy_dev, 0);

    v16 = EXT3_ECDRUN_IMMEDIATE;
    if ((phy_link = phy_dev->link)) {  /* Save initial PHY link status */
        PHY_READ(phy_dev, 0x07, 0xffe0, &ctl_reg);
    }

    if (phy_link) { /* If link is up, Write RUN first and wait until link goes down */
        PHY_WRITE(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, v16);
        for(;;) {
            phy_dev_read_status(phy_dev);
            if (!phy_dev->link) break;
        }
    }

TryAgain:
    if (retries) for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(2*1000)););
    if (++retries > EXT3_ECDMAX_RETRIES)  /* If we did retry more than certain time, declares it as faiure */
        goto end;

    PHY_WRITE(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, v16);
    ret = phy_bus_c45_read(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, &v16);
    for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(EXT3_ECDCHECK_SECS*1000)); ) {
        PHY_READ(phy_dev, EXT3_ECD_CTRL_STATUS_D, EXT3_ECD_CTRL_STATUS_R, &v16);
        if (!(v16 & EXT3_ECDDIAG_IN_PROG)) {
            ret = 0;
            i = 1;
            break;
        }
    }

    if (!i) {    /* If CD is still in progress after certain time, retry it */
        *result = EXT3_PACD_CODE_INVALID;
        ret = -1;
        goto TryAgain;
    }

    for(i=0, jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(EXT3_ECDCHECK_SECS*1000)); ) {
        PHY_READ(phy_dev, EXT3_ECD_RESULTS_D, EXT3_ECD_RESULTS_R, &v16);
        *result = v16;
        excluded_pair = 0;
        for(j=0; j<4; j++) { /* Check if all four pairs of diags are done */
            if( EXT3_PACD_CODE_PAIR_GET(*result, j) > EXT3_PACD_CODE_PAIR_INTER_SHORT)
                break;

            /* If link is up, excluded failed measuring result */
            if( phy_link && ( EXT3_PACD_CODE_PAIR_GET(*result, j) != EXT3_PACD_CODE_PAIR_OK))
                excluded_pair |= (1<<j);
        }

        /* If all pair of diags finish, check the results */
        if (j==4) {
            /* If in link up, all pair diag failed, try again */
            if (*result == EXT3_PACD_CODE_INVALID || excluded_pair == 0xf )
                goto TryAgain;
            /* Otherwise, we are done with CD */
            i=1;
            break;
        }
    }

    if (phy_link)
        *result = EXT3_PACD_CODE_PAIR_ALL_OK;

    if (*result == EXT3_PACD_CODE_INVALID || !i) {  /* If CD ends with INVALID result, retry it */
        *result = EXT3_PACD_CODE_INVALID;
        ret = -1;
        goto TryAgain;
    }

#define EXT3_CABLE_LEN_OFFSET_LINK_DOWN 200
    for(i=0; i<4; i++) {
        PHY_READ(phy_dev, EXT3_ECD_CABLE_LEN_D, (EXT3_ECD_CABLE_LEN_R + i), &v16);
        if (*result == EXT3_PACD_CODE_PAIR_ALL_OPEN)
            pair_len[i] = (v16> EXT3_CABLE_LEN_OFFSET_LINK_DOWN ? v16 - EXT3_CABLE_LEN_OFFSET_LINK_DOWN : 0); /* To guarrantee no cable result correct based on testing */
        else if (*result == EXT3_PACD_CODE_PAIR_ALL_OK)
            pair_len[i] = v16 + EXT3_CABLE_LEN_OFFSET_LINK_DOWN;
        else
            pair_len[i] = v16;
    }

    /* If link is up, but alll pair length is zero, try again */
    if (phy_link && (pair_len[0] + pair_len[1] + pair_len[2] + pair_len[3] == 0))
        goto TryAgain;

end:
    /* Reset PHY after CD */
    ctl_reg |= (1<<15);
    if (ctl_reg & (1<<12))
        ctl_reg |= (1<<9);  /* If AN is on, set AN restart bit */
    PHY_WRITE(phy_dev, 0x07, 0xffe0, ctl_reg);

    /* Wait the link come back up */
    if (phy_link) {
        for(jiffie = jiffies; jiffies < (jiffie + msecs_to_jiffies(3*EXT3_ECDCHECK_SECS*1000)); ) {
            phy_dev_read_status(phy_dev);
            if (phy_dev->link) break;
        }
    }

    if (phy_link)
        cable_length_pick_link_up(pair_len, excluded_pair);

    if (apd_enabled)
        phy_dev_apd_set(phy_dev, apd_enabled);

    return ret;
Exit:
    return -1;
}
#if defined (MAC_LPORT)
extern int lport_led_init(void *leds_info);
#endif

static int _phy_leds_init(phy_dev_t *phy_dev, void *leds_info)
{
#if defined (MAC_LPORT)
    return lport_led_init(leds_info);
#else
    return xrdp_leds_init(leds_info);
#endif
}

static int _phy_read(phy_dev_t *phy_dev, uint16_t reg, uint16_t *val)
{
    return -EOPNOTSUPP;
}

static int _phy_write(phy_dev_t *phy_dev, uint16_t reg, uint16_t val)
{
    return -EOPNOTSUPP;
}

phy_drv_t phy_drv_ext3 =
{
    .phy_type = PHY_TYPE_EXT3,
    .name = "EXT3",
    .read = _phy_read,
    .write = _phy_write,
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
    .wirespeed_set = _phy_eth_wirespeed_set,
    .wirespeed_get = _phy_eth_wirespeed_get,
    .init = _phy_init,
    .dev_add = _phy_dev_add,
    .dev_del = _phy_dev_del,
    .drv_init = _phy_drv_init,
    .pair_swap_set = _phy_pair_swap_set,
    .isolate_phy = _phy_isolate,
    .super_isolate_phy = _phy_super_isolate,
#ifdef MACSEC_SUPPORT
    .macsec_oper = phy_macsec_oper,
#endif
    .inter_phy_types_get = _phy_inter_phy_types_get,
    .configured_inter_phy_types_set = _phy_configured_inter_phy_types_set,
    .get_phy_name = _phy_get_phy_name,
    .cable_diag_run = _phy_enhanced_cable_diag_run,
    .current_inter_phy_type_get = _phy_current_inter_phy_type_get,
    .leds_init = _phy_leds_init,
    .an_restart = _phy_an_restart,
};
