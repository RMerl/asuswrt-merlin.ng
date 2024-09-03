/****************************************************************************
 *
 * <:copyright-BRCM:2020:DUAL/GPL:standard
 * 
 *    Copyright (c) 2020 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 *
 ***************************************************************************/

#include "pmc_drv.h"
#include "BPCM.h"
#include <linux/delay.h>

unsigned int read_ssbm_reg(unsigned int addr) 
{
    volatile Procmon *procmon = (volatile Procmon *)g_pmc->procmon_base;
    procmon->SSBMaster.control = PMC_SSBM_CONTROL_SSB_ADPRE | 
                                (PMC_SSBM_CONTROL_SSB_CMD_READ << PMC_SSBM_CONTROL_SSB_CMD_SHIFT ) 
                                | (PMC_SSBM_CONTROL_SSB_ADDR_MASK & 
                                   (addr << PMC_SSBM_CONTROL_SSB_ADDR_SHIFT) );
    procmon->SSBMaster.control |= PMC_SSBM_CONTROL_SSB_EN;
    procmon->SSBMaster.control |= PMC_SSBM_CONTROL_SSB_START;
    while (procmon->SSBMaster.control & PMC_SSBM_CONTROL_SSB_START);
    return(procmon->SSBMaster.rd_data);
}

void write_ssbm_reg(unsigned int addr, unsigned int data, int force) 
{
    volatile Procmon *procmon = (volatile Procmon *)g_pmc->procmon_base;
    procmon->SSBMaster.wr_data = data;
    procmon->SSBMaster.control = (PMC_SSBM_CONTROL_SSB_CMD_WRITE << PMC_SSBM_CONTROL_SSB_CMD_SHIFT ) 
                                | (PMC_SSBM_CONTROL_SSB_ADDR_MASK & 
                                   (addr << PMC_SSBM_CONTROL_SSB_ADDR_SHIFT) );
    procmon->SSBMaster.control |= PMC_SSBM_CONTROL_SSB_EN;
    procmon->SSBMaster.control |= PMC_SSBM_CONTROL_SSB_START;
    while (procmon->SSBMaster.control & PMC_SSBM_CONTROL_SSB_START);
    if (force != 0) {
        unsigned int reg0;
        reg0 = read_ssbm_reg(addr&0xfff0);
        write_ssbm_reg(addr&0xfff0, reg0|2, 0);
        write_ssbm_reg(addr&0xfff0, reg0&(~2), 0);
    }
    return;
}

void enable_over_current_watchdog(void)
{
    volatile Procmon *procmon = (volatile Procmon *)g_pmc->procmon_base;
    procmon->Misc.gp_out |= (0x1<<31);
}
