/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2021 Broadcom Ltd.
 */

#include <pmc_drv.h>
#include <linux/io.h>
#include <linux/err.h>

#if !defined(PMC_DIRECT_MODE_ONLY)
#define SWR_READ_CMD_P 0xB800
#define SWR_WR_CMD_P   0xB400
#define SWR_EN         0x1000
#define SET_ADDR(ps, reg)  (((ps) << 5 | ((reg) & 0x1f)) & 0x2ff)
#define SW_COMP_TIMEOUT 1000

static void sw_complete_wait(int err_code, int timeout_count)
{
    volatile SSBMaster *ssb_master = ioremap(SSBMASTER_BASE, sizeof(SSBMaster)); 

    for(;(((ssb_master->control) & 0x8000) && (timeout_count > 0)) ; timeout_count--)
        ;

    if(!timeout_count) 
        printk("sw_complete_wait: Error code %d timeout !!!", err_code);
}

void swr_write(unsigned int ps, unsigned int reg, unsigned int val)
{
    unsigned int cmd = 0;
    unsigned int cmd1 = 0;
    unsigned int reg0 = 0;
    volatile SSBMaster *ssb_master = ioremap(SSBMASTER_BASE, sizeof(SSBMaster)); 

    ssb_master->control = SWR_EN;

    if (reg == 0) {
        /* no need read reg0 in case that we write to it , we know wal :) */
        reg0 = val;
    } else {
        /* read reg0 */
        cmd1 = SWR_READ_CMD_P | SET_ADDR(ps, 0);
        ssb_master->control = cmd1;
        sw_complete_wait(1, SW_COMP_TIMEOUT);
        reg0 = ssb_master->rd_data;
    }
    /* write reg */
    ssb_master->wr_data = val;
    cmd = SWR_WR_CMD_P | SET_ADDR(ps, reg);
    ssb_master->control = cmd;
    sw_complete_wait(2, SW_COMP_TIMEOUT);
    /*toggele bit 1 reg0 this load the new regs value */
    cmd1 = SWR_WR_CMD_P | SET_ADDR(ps, 0);
    ssb_master->wr_data = reg0 & ~0x2;
    ssb_master->control = cmd1;
    sw_complete_wait(3, SW_COMP_TIMEOUT);
    ssb_master->wr_data = reg0 | 0x2;
    ssb_master->control = cmd1;
    sw_complete_wait(4, SW_COMP_TIMEOUT);
    ssb_master->wr_data = reg0 & ~0x2;
    ssb_master->control = cmd1;
    sw_complete_wait(5, SW_COMP_TIMEOUT);
}
#endif

