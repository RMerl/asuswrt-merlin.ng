/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/delay.h>

#include "pmc_drv.h"
#include "pmc_lport.h"
#include "BPCM.h"
#include "clk_rst.h"
#include "bcm_ubus4.h"

/* pmc_lport.c
 *
 *  Created on: 21 Nov 2015
 *      Author: yonatani
 */

#define UNIPLL_CHAN23_MDIV 5

int pmc_lport_init(void)
{
    int status;
    LPORT_BPCM_Z0_CONTROL_REG z0_ctl =
    { };
    LPORT_BPCM_Z1_CONTROL_REG z1_ctl =
    { };
    LPORT_BPCM_Z2_CONTROL_REG z2_ctl =
    { };

    //Configur PLL
    status = PowerOnDevice(PMB_ADDR_UNIPLL);
    if (status)
    {
        printk("UNIPLL PowerOnDevice failed\n");
        return status;
    }

    status =  pll_ch_freq_vco_set(PMB_ADDR_UNIPLL, 2, UNIPLL_CHAN23_MDIV, 0);
    if (status)
    {
        printk("UNIPLL Channel 2 Mdiv=5 failed\n");
        return status;
    }

    //Configure LPORT Block BPCM
    status = PowerOnDevice(PMB_ADDR_LPORT);
    if (status)
    {
        printk("LPORT PowerOnDevice failed\n");
        return status;
    }

    z0_ctl.z0_ubus_dev_clk_en = 1;
    z0_ctl.z0_qegphy_clk_en = 1;
    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_CONTROL, *(uint32_t *)&z0_ctl);
    if (status)
    {
        printk("LPORT  failed BPCM LPORT_BPCM_Z0_CONTROL\n");
        return status;
    }

    z1_ctl.z1_cclk_clk_en = 1;
    z1_ctl.z1_clk_250_clk_en = 1;
    z1_ctl.z1_data_path_cclk_clk_en = 1;
    z1_ctl.z1_tsc_clk_en = 1;
    z1_ctl.z1_tsc_clk_gated_clk_en = 1;
    z1_ctl.z1_tsclk_clk_en = 1;

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z1_CONTROL, *(uint32_t *)&z1_ctl);
    if (status)
    {
        printk("LPORT  failed BPCM LPORT_BPCM_Z1_CONTROL\n");
        return status;
    }

    z2_ctl.z2_cclk_clk_en = 1;
    z2_ctl.z2_clk_250_clk_en = 1;
    z2_ctl.z2_tsc_clk_en = 1;
    z2_ctl.z2_data_path_cclk_clk_en = 1;
    z2_ctl.z2_tsc_clk_gated_clk_en = 1;
    z2_ctl.z2_tsclk_clk_en = 1;

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z2_CONTROL, *(uint32_t *)&z2_ctl);
    if (status)
    {
        printk("LPORT unreset failed BPCM LPORT_BPCM_Z2_CONTROL\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES0_CTRL, 6);
    if (status)
    {
        printk("LPORT unreset failed BPCM LPORT_BPCM_Z0_DSERDES0_CTRL\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES1_CTRL, 6);
    if (status)
    {
        printk("LPORT unreset failed BPCM LPORT_BPCM_Z1_DSERDES0_CTRL\n");
        return status;
    }

    udelay(1000);

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES0_CTRL, 0);
    if (status)
    {
        printk("LPORT unreset failed BPCM LPORT_BPCM_Z0_DSERDES0_CTRL\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES1_CTRL, 0);
    if (status)
    {
        printk("LPORT unreset failed BPCM LPORT_BPCM_Z1_DSERDES0_CTRL\n");
        return status;
    }

    return status;
}

int pmc_lport_shutdown(void)
{
    /* shut down all zones */
    return PowerOffDevice(PMB_ADDR_LPORT, 0);
}

EXPORT_SYMBOL(pmc_lport_init);
arch_initcall(pmc_lport_init);


