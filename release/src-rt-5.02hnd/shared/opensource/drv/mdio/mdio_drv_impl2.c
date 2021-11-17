/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :>
*/

#include "access_macros.h"
#include "bcm_map_part.h"
#include "mdio_drv_impl2.h"

#ifdef _CFE_
#define spin_lock_bh(x)
#define spin_unlock_bh(x)
#else
#include <linux/spinlock.h>
static DEFINE_SPINLOCK(mdio_access);
#endif

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

#pragma pack(push, 1)
typedef struct
{
    uint32_t reserved:2;
    uint32_t mdio_busy:1;
    uint32_t fail:1;
    uint32_t op_code:2;
    uint32_t phy_prt_addr:5;
    uint32_t reg_dec_addr:5;
    uint32_t data_addr:16;

} mdio_cmd_reg_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint32_t reserved:18;
    uint32_t supress_preamble:1;
    uint32_t reserved1:1;
    uint32_t mdio_clk_divider:7;
    uint32_t reserved0:4;
    uint32_t mdio_clause:1;

} mdio_cfg_reg_t;
#pragma pack(pop)

#define MDIO_CMD                    0xb08020c0
#define MDIO_CFG                    0xb08020c4

#define MDIO_BUSY_RETRY             2000
#define CHECK_MDIO_READY            if (!mdio_is_ready()) {spin_unlock_bh(&mdio_access); return MDIO_ERROR;}

static inline int32_t mdio_is_ready(void)
{
    uint32_t retry = MDIO_BUSY_RETRY;
    mdio_cmd_reg_t cmd;

    do {
        READ_32(MDIO_CMD, cmd);
        if (!cmd.mdio_busy)
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
        WRITE_32(MDIO_CMD, cmd);
        return 0;
    }

    return 1;
}

static inline void mdio_cfg_clause_mode(mdio_clause_t mdio_clause)
{
    mdio_cfg_reg_t cfg;

    READ_32(MDIO_CFG, cfg);
    cfg.mdio_clause = mdio_clause;
    WRITE_32(MDIO_CFG, cfg);
}

static inline void mdio_cfg_type(mdio_type_t type)
{
    uint32_t mdio_master_bit = (1 << CLKGEN3_MDIO_MASTER_SHIFT);

    if (type == MDIO_INT)
    {
        MISC->miscMDIOmasterSelect = 1;
        MISC_REG->ClkgenCtr3 &= ~mdio_master_bit;
    }
    else
    {
        MISC->miscMDIOmasterSelect = 0;
        MISC_REG->ClkgenCtr3 |= mdio_master_bit;
    }
}

int32_t mdio_read_c22_register(mdio_type_t type, uint32_t phy_addr, uint32_t reg_addr, uint16_t *data_read)
{
    mdio_cmd_reg_t cmd = {};

    spin_lock_bh(&mdio_access);

    mdio_cfg_type(type);
    mdio_cfg_clause_mode(MDIO_CLAUSE_22);

    CHECK_MDIO_READY;

    cmd.op_code = MDIO22_READ;
    cmd.phy_prt_addr = phy_addr;
    cmd.reg_dec_addr = reg_addr;
    cmd.mdio_busy = 1;
    WRITE_32(MDIO_CMD, cmd);

    CHECK_MDIO_READY;

    READ_32(MDIO_CMD, cmd);
    *data_read = cmd.data_addr;

    spin_unlock_bh(&mdio_access);

    return MDIO_OK;
}
EXPORT_SYMBOL(mdio_read_c22_register);

int32_t mdio_write_c22_register(mdio_type_t type, uint32_t phy_addr, uint32_t reg_addr, uint16_t data_write)
{
    mdio_cmd_reg_t cmd = {};

    spin_lock_bh(&mdio_access);

    mdio_cfg_type(type);
    mdio_cfg_clause_mode(MDIO_CLAUSE_22);

    CHECK_MDIO_READY;

    cmd.op_code = MDIO22_WRITE;
    cmd.phy_prt_addr = phy_addr;
    cmd.reg_dec_addr = reg_addr;
    cmd.data_addr = data_write;
    cmd.mdio_busy = 1;
    WRITE_32(MDIO_CMD, cmd);

    CHECK_MDIO_READY;

    spin_unlock_bh(&mdio_access);

    return MDIO_OK;
}
EXPORT_SYMBOL(mdio_write_c22_register);

int32_t mdio_read_c45_register(mdio_type_t type, uint32_t port_addr, uint32_t dev_addr, uint16_t dev_offset, uint16_t *data_read)
{
    mdio_cmd_reg_t cmd = {};

    spin_lock_bh(&mdio_access);

    mdio_cfg_type(type);
    mdio_cfg_clause_mode(MDIO_CLAUSE_45);

    CHECK_MDIO_READY;

    cmd.op_code = MDIO45_ADDRESS;
    cmd.phy_prt_addr = port_addr;
    cmd.reg_dec_addr = dev_addr;
    cmd.data_addr = dev_offset;
    cmd.mdio_busy = 1;
    WRITE_32(MDIO_CMD, cmd);

    CHECK_MDIO_READY;

    cmd.op_code = MDIO45_READ;
    cmd.mdio_busy = 1;
    WRITE_32(MDIO_CMD, cmd);

    CHECK_MDIO_READY;

    READ_32(MDIO_CMD, cmd);
    *data_read = cmd.data_addr;

    spin_unlock_bh(&mdio_access);

    return MDIO_OK;
}
EXPORT_SYMBOL(mdio_read_c45_register);

int32_t mdio_write_c45_register(mdio_type_t type, uint32_t port_addr, uint32_t dev_addr, uint16_t dev_offset, uint16_t data_write)
{
    mdio_cmd_reg_t cmd = {};

    spin_lock_bh(&mdio_access);

    mdio_cfg_type(type);
    mdio_cfg_clause_mode(MDIO_CLAUSE_45);

    CHECK_MDIO_READY;

    cmd.op_code = MDIO45_ADDRESS;
    cmd.phy_prt_addr = port_addr;
    cmd.reg_dec_addr = dev_addr;
    cmd.data_addr = dev_offset;
    cmd.mdio_busy = 1;
    WRITE_32(MDIO_CMD, cmd);

    CHECK_MDIO_READY;

    cmd.op_code = MDIO45_WRITE;
    cmd.data_addr = data_write;
    cmd.mdio_busy = 1;
    WRITE_32(MDIO_CMD, cmd);

    CHECK_MDIO_READY;

    spin_unlock_bh(&mdio_access);

    return MDIO_OK;
}
EXPORT_SYMBOL(mdio_write_c45_register);

