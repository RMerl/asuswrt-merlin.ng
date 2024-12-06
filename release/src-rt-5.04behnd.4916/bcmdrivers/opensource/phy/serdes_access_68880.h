/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

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
 *  Created on: Apr 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Merlin SerDes registers access for 68880
 */

#ifndef __SERDES_ACCESS_68880_H__
#define __SERDES_ACCESS_68880_H__

#define SHORTFIN_BASE_CORE          0
#define SHORTFIN_CORES              1

#define SHASTA_BASE_CORE            1
#define SHASTA_CORES                1

#ifdef __SERDES_ACCESS_C__

#define SERDES_0_INDIR_ACC_ADDR     0x0004
#define SERDES_0_INDIR_ACC_MASK     0x0008
#define SERDES_0_CONTROL            0x000c
#define SERDES_0_STATUS             0x0010
#define SERDES_0_AN_STATUS          0x0020
#define SERDES_0_STATUS_1           0x0024
#define SERDES_0_INDIR_ACC_CNTRL    0x00f0

#define SERDES_1_INDIR_ACC_ADDR     0x0104
#define SERDES_1_INDIR_ACC_MASK     0x0108
#define SERDES_1_CONTROL            0x010c
#define SERDES_1_STATUS             0x0110
#define SERDES_1_AN_STATUS          0x0120
#define SERDES_1_STATUS_1           0x0124
#define SERDES_1_INDIR_ACC_CNTRL    0x01f0

#define SERDES_CORES                2
#define DEV_TYPE_OFFSET             27
#define LANE_ADDRESS_OFFSET         16

static uintptr_t SERDES_CONTROL[SERDES_CORES] = {
    SERDES_0_CONTROL,
    SERDES_1_CONTROL,
};

static uintptr_t SERDES_STATUS[SERDES_CORES] = {
    SERDES_0_STATUS,
    SERDES_1_STATUS,
};

static uintptr_t SERDES_AN_STATUS[SERDES_CORES] = {
    SERDES_0_AN_STATUS,
    SERDES_1_AN_STATUS,
};

static uintptr_t SERDES_INDIR_ACC_CNTRL[SERDES_CORES] = {
    SERDES_0_INDIR_ACC_CNTRL,
    SERDES_1_INDIR_ACC_CNTRL,
};

static uintptr_t SERDES_INDIR_ACC_ADDR[SERDES_CORES] = {
    SERDES_0_INDIR_ACC_ADDR,
    SERDES_1_INDIR_ACC_ADDR,
};

static uintptr_t SERDES_INDIR_ACC_MASK[SERDES_CORES] = {
    SERDES_0_INDIR_ACC_MASK,
    SERDES_1_INDIR_ACC_MASK,
};

static uintptr_t SERDES_STATUS_1[SERDES_CORES] = {
    SERDES_0_STATUS_1,
    SERDES_1_STATUS_1,
};

#pragma pack(push,1)
typedef struct
{
    uint32_t iddq:1;
    uint32_t refclk_reset:1;
    uint32_t serdes_reset:1;
    uint32_t refsel:3;
    uint32_t serdes_prtad:5;
    uint32_t serdes_ln_offset:5;
    uint32_t pmd_refclk_term_sel:2;
    uint32_t pmd_refclk_div4:1;
    uint32_t pmd_refclk_div2:1;
    uint32_t serdes_test_en:1;
    uint32_t serdes_testsel:3;
    uint32_t ref_cmos_clk_hz:1;
    uint32_t pd_cml_refclk_chout:1;
    uint32_t pd_cml_lcrefout:1;
    uint32_t iso_enable:1;
    uint32_t comclk_en:1;
    uint32_t mdio_fast_mode:1;
    uint32_t reserved1:2;
} serdes_control_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t rx_sigdet:4;
    uint32_t cdr_lock:4;
    uint32_t link_status:4;
    uint32_t pll_lock:1;
    uint32_t reserved1:3;
    uint32_t ext_sig_det:4;
    uint32_t debounced_signal_detect:4;
    uint32_t serdes_mod_def0:4;
    uint32_t reserved2:4;
} serdes_status_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t an_link_status:4;
    uint32_t an_hcd_fulld:4;
    uint32_t an_hcd_rx_pause:4;
    uint32_t an_hcd_tx_pause:4;
    uint32_t an_hcd_eee_cap:4;
    uint32_t an_eee_clock_stop_cap:4;
    uint32_t reserved1:8;
} serdes_an_status_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reg_data:16;
    uint32_t r_w:1;
    uint32_t start_busy:1;
    uint32_t delayed_ack:1;
    uint32_t reserved1:13;
} serdes_indirect_access_control_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t speed_10m:4;
    uint32_t speed_100m:4;
    uint32_t speed_1g:4;
    uint32_t speed_2p5g:4;
    uint32_t speed_5g:4;
    uint32_t speed_10g:4;
    uint32_t xgmii_mode:4;
    uint32_t phy_clk_vld:1;
    uint32_t reserved1:3;
} serdes_status_1_t;
#pragma pack(pop)

static inline int serdes_indirect_access_control_status(serdes_indirect_access_control_t *siac)
{
    return 0;
}

static inline int serdes_indirect_access_control_init(serdes_indirect_access_control_t *siac, int is_read, uint16_t val)
{
    memset(siac, 0, sizeof(serdes_indirect_access_control_t));
    siac->delayed_ack = 1;
    siac->start_busy = 1;
    siac->r_w = is_read;
    siac->reg_data = val;

    return 0;
}

static inline int serdes_access_parse_speed(serdes_status_1_t *serdes_status_1, uint8_t lane_id, phy_speed_t *speed)
{
    uint8_t mask = (1 << lane_id);

    *speed = PHY_SPEED_UNKNOWN;

    if (serdes_status_1->speed_10m & mask)
        *speed = PHY_SPEED_10;
    if (serdes_status_1->speed_100m & mask)
        *speed = PHY_SPEED_100;
    if (serdes_status_1->speed_1g & mask)
        *speed = PHY_SPEED_1000;
    if (serdes_status_1->speed_2p5g & mask)
        *speed = PHY_SPEED_2500;
    if (serdes_status_1->speed_5g & mask)
        *speed = PHY_SPEED_5000;
    if (serdes_status_1->speed_10g & mask)
        *speed = PHY_SPEED_10000;

    return 0;
}

#endif

#endif
