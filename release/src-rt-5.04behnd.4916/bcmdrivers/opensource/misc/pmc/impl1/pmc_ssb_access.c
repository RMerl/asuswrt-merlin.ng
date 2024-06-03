/****************************************************************************
 *
 * <:copyright-BRCM:2020:DUAL/GPL:standard
 * 
 *    Copyright (c) 2020 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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
