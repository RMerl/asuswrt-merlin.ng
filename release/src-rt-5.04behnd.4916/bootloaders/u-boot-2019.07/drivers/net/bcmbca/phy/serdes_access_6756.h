// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2016 Broadcom Corporation
   All Rights Reserved

    
*/


/*
 *  Created on: May 2021
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * Merlin SerDes registers access for 6756
 */

#ifndef __SERDES_ACCESS_6756_H__
#define __SERDES_ACCESS_6756_H__

#define MPTWO_CORES                 1
#define MPTWO_BASE_CORE             0

#ifdef __SERDES_ACCESS_C__

#define SERDES_0_INDIR_ACC_ADDR_0   0x0004
#define SERDES_0_INDIR_ACC_MASK_0   0x0008
#define SERDES_0_INDIR_ACC_ADDR_1   0x000c
#define SERDES_0_INDIR_ACC_MASK_1   0x0010
#define SERDES_0_CONTROL            0x0014
#define SERDES_0_STATUS             0x0018
#define SERDES_0_INDIR_ACC_CNTRL_0  0x0100
#define SERDES_0_INDIR_ACC_CNTRL_1  0x0104
#define SERDES_0_AN_STATUS          0x0000
#define SERDES_0_STATUS_1           0x0000

#define SERDES_CORES                1
#define DEV_TYPE_OFFSET             27
#define LANE_ADDRESS_OFFSET         16

static uintptr_t SERDES_CONTROL[SERDES_CORES] = {
    SERDES_0_CONTROL,
};

static uintptr_t SERDES_STATUS[SERDES_CORES] = {
    SERDES_0_STATUS,
};

static uintptr_t SERDES_INDIR_ACC_CNTRL[SERDES_CORES] = {
    SERDES_0_INDIR_ACC_CNTRL_0,
};

static uintptr_t SERDES_INDIR_ACC_ADDR[SERDES_CORES] = {
    SERDES_0_INDIR_ACC_ADDR_0,
};

static uintptr_t SERDES_INDIR_ACC_MASK[SERDES_CORES] = {
    SERDES_0_INDIR_ACC_MASK_0,
};

static uintptr_t SERDES_STATUS_1[SERDES_CORES] = {
    SERDES_0_STATUS_1,
};

static uintptr_t SERDES_AN_STATUS[SERDES_CORES] = {
    SERDES_0_AN_STATUS,
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
    uint32_t serdes_test_en:1;
    uint32_t serdes_sd_sel:1;
    uint32_t serdes_inv_sd:1;
    uint32_t serdes_sd_intr_src_sel:1;
    uint32_t serdes_testsel:3;
    uint32_t delayed_ack:1;
    uint32_t reserved2:8;
} serdes_control_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t rx_sigdet:2;
    uint32_t cdr_lock:2;
    uint32_t link_status:2;
    uint32_t pll_lock:1;
    uint32_t ext_sig_det:2;
    uint32_t serdes_mod_def0:2;
    uint32_t reserved1:21;
} serdes_status_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t an_link_status:2; /* dummy */
    uint32_t reserved1:30;
} serdes_an_status_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reg_data:16;
    uint32_t r_w:1;
    uint32_t start_busy:1;
    uint32_t err:1;
    uint32_t reserved1:13;
} serdes_indirect_access_control_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
    uint32_t reserved1:32;
} serdes_status_1_t;
#pragma pack(pop)

static inline int serdes_indirect_access_control_status(serdes_indirect_access_control_t *siac)
{
    return (siac->err | siac->start_busy);
}

static inline int serdes_indirect_access_control_init(serdes_indirect_access_control_t *siac, int is_read, uint16_t val)
{
    memset(siac, 0, sizeof(serdes_indirect_access_control_t));
    siac->err = 0;
    siac->start_busy = 1;
    siac->r_w = is_read;
    siac->reg_data = val;

    return 0;
}

static inline int serdes_access_parse_speed(serdes_status_1_t *serdes_status_1, uint8_t lane_id, phy_speed_t *speed)
{
    *speed = PHY_SPEED_UNKNOWN;

    return 0;
}

#endif

#endif
