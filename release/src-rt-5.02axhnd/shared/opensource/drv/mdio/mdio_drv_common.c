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
 *  Created on: Aug 2016
 *      Author: yuval.raviv@broadcom.com
 */

/*
 * MDIO common functions
 */

#include "mdio_drv_common.h"
#if defined(DSL_RUNNER_DEVICE)
/* TODO_DSL: SF2 based is little endian, no need to byte swap, but need to figure out how to use access_macros.h instead */
#define WRITE_32(a, r)			( *(volatile uint32_t*)(a) = *(uint32_t*)&(r) )
#define READ_32(a, r)			( *(volatile uint32_t*)&(r) = *(volatile uint32_t*) (a) )
#else
#include "access_macros.h"
#endif
#include "bcm_map_part.h"

#define MDIO_BUSY_RETRY         2000
#define CHECK_MDIO_READY(x)     if (!is_mdio_ready(x)) {ret = MDIO_ERROR; goto Exit;}

typedef enum
{
    MDIO_CLAUSE_45 = 0,
    MDIO_CLAUSE_22 = 1,
} mdio_clause_t;

typedef enum
{
    MDIO22_WRITE = 1,
    MDIO22_READ = 2,
} mdio22_op_t;

typedef enum
{
    MDIO45_ADDRESS = 0,
    MDIO45_WRITE = 1,
    MDIO45_READINC = 2,
    MDIO45_READ = 3,
} mdio45_op_t;

#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
#pragma pack(push, 1)
typedef struct
{
    uint32_t data_addr:16;
    uint32_t reg_dev_addr:5;
    uint32_t phy_prt_addr:5;
    uint32_t op_code:2;
    uint32_t fail:1;
    uint32_t busy:1;
    uint32_t reserved:2;
} mdio_cmd_reg_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint32_t mdio_clause:1;
    uint32_t unused1:3;
    uint32_t mdio_clk_divider:8;
    uint32_t unused2:1;
    uint32_t free_run_clk_enable:1;
    uint32_t unused3:18;
} mdio_cfg_reg_t;
#pragma pack(pop)
#else
#pragma pack(push, 1)
typedef struct
{
    uint32_t reserved:2;
    uint32_t busy:1;
    uint32_t fail:1;
    uint32_t op_code:2;
    uint32_t phy_prt_addr:5;
    uint32_t reg_dev_addr:5;
    uint32_t data_addr:16;
} mdio_cmd_reg_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint32_t unused3:18;
    uint32_t free_run_clk_enable:1;
    uint32_t unused2:1;
    uint32_t mdio_clk_divider:8;
    uint32_t unused1:3;
    uint32_t mdio_clause:1;
} mdio_cfg_reg_t;
#pragma pack(pop)
#endif

static int32_t is_mdio_ready(uint32_t *p)
{
    uint32_t retry = MDIO_BUSY_RETRY;
    mdio_cmd_reg_t cmd;

    do {
        READ_32(p, cmd);
        if (!cmd.busy)
            break;
        udelay(10);
    } while (retry--);

    if (!retry)
    {
        printk("MDIO Error: mdio_is_ready() reached maximum retries of %d\n", MDIO_BUSY_RETRY);
        return 0;
    }

    if (cmd.fail)
    {
        printk("MDIO Error: MDIO got failure status on phy %d\n", cmd.phy_prt_addr);
        memset(&cmd, 0, sizeof(cmd));
        WRITE_32(p, cmd);
        return 0;
    }

    return 1;
}

static void mdio_cfg_clause_mode(uint32_t *p, mdio_clause_t mdio_clause)
{
    mdio_cfg_reg_t cfg;

    READ_32(p, cfg);
#if defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
    cfg.free_run_clk_enable = 1;
    cfg.mdio_clk_divider = 12;
#endif
    cfg.mdio_clause = mdio_clause;
    WRITE_32(p, cfg);
}

int32_t mdio_cmd_read_22(uint32_t *p, uint32_t addr, uint32_t reg, uint16_t *val)
{
    int ret = MDIO_OK;
    mdio_cmd_reg_t cmd = {};

    mdio_cfg_clause_mode(p + 1, MDIO_CLAUSE_22);

    cmd.op_code = MDIO22_READ;
    cmd.phy_prt_addr = addr;
    cmd.reg_dev_addr = reg;
    cmd.busy = 1;
    WRITE_32(p, cmd);

    CHECK_MDIO_READY(p);

    READ_32(p, cmd);
    *val = cmd.data_addr;

Exit:
    return ret;
}

int32_t mdio_cmd_write_22(uint32_t *p, uint32_t addr, uint32_t reg, uint16_t val)
{
    int ret = MDIO_OK;
    mdio_cmd_reg_t cmd = {};

    mdio_cfg_clause_mode(p + 1, MDIO_CLAUSE_22);

    cmd.op_code = MDIO22_WRITE;
    cmd.phy_prt_addr = addr;
    cmd.reg_dev_addr = reg;
    cmd.data_addr = val;
    cmd.busy = 1;
    WRITE_32(p, cmd);

    CHECK_MDIO_READY(p);

Exit:
    return ret;
}

int32_t mdio_cmd_read_45(uint32_t *p, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t *val)
{
    int ret = MDIO_OK;
    mdio_cmd_reg_t cmd = {};

    mdio_cfg_clause_mode(p + 1, MDIO_CLAUSE_45);

    cmd.op_code = MDIO45_ADDRESS;
    cmd.phy_prt_addr = addr;
    cmd.reg_dev_addr = dev;
    cmd.data_addr = reg;
    cmd.busy = 1;
    WRITE_32(p, cmd);

    CHECK_MDIO_READY(p);

    cmd.op_code = MDIO45_READ;
    cmd.busy = 1;
    WRITE_32(p, cmd);

    CHECK_MDIO_READY(p);

    READ_32(p, cmd);
    *val = cmd.data_addr;

Exit:
    return ret;
}

int32_t mdio_cmd_write_45(uint32_t *p, uint32_t addr, uint32_t dev, uint16_t reg, uint16_t val)
{
    int ret = MDIO_OK;
    mdio_cmd_reg_t cmd = {};

    mdio_cfg_clause_mode(p + 1, MDIO_CLAUSE_45);

    cmd.op_code = MDIO45_ADDRESS;
    cmd.phy_prt_addr = addr;
    cmd.reg_dev_addr = dev;
    cmd.data_addr = reg;
    cmd.busy = 1;
    WRITE_32(p, cmd);

    CHECK_MDIO_READY(p);

    cmd.op_code = MDIO45_WRITE;
    cmd.data_addr = val;
    cmd.busy = 1;
    WRITE_32(p, cmd);

    CHECK_MDIO_READY(p);

Exit:
    return ret;
}
