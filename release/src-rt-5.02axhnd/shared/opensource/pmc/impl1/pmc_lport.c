/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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
#ifndef _CFE_
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#else
#include "lib_printf.h"
#endif
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

#ifndef _CFE_
#include <asm/delay.h>
#define PRINTK  printk
#define UDELAY udelay
#else
#define PRINTK  xprintf
extern void cfe_usleep(int); 
#define UDELAY(x)  cfe_usleep(x)
#endif

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
        PRINTK("UNIPLL PowerOnDevice failed\n");
        return status;
    }

    status =  pll_ch_freq_vco_set(PMB_ADDR_UNIPLL, 2, UNIPLL_CHAN23_MDIV, 0);
    if (status)
    {
        PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
        return status;
    }

    //Configure LPORT Block BPCM
    status = PowerOnDevice(PMB_ADDR_LPORT);
    if (status)
    {
        PRINTK("LPORT PowerOnDevice failed\n");
        return status;
    }

#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    // register ubus port
    ubus_register_port(UCB_NODE_ID_SLV_LPORT);
#endif

    z0_ctl.z0_ubus_dev_clk_en = 1;
    z0_ctl.z0_qegphy_clk_en = 1;
    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_CONTROL, *(uint32*)&z0_ctl);
    if (status)
    {
        PRINTK("LPORT  failed BPCM LPORT_BPCM_Z0_CONTROL\n");
        return status;
    }

    z1_ctl.z1_cclk_clk_en = 1;
    z1_ctl.z1_clk_250_clk_en = 1;
    z1_ctl.z1_data_path_cclk_clk_en = 1;
    z1_ctl.z1_tsc_clk_en = 1;
    z1_ctl.z1_tsc_clk_gated_clk_en = 1;
    z1_ctl.z1_tsclk_clk_en = 1;

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z1_CONTROL, *(uint32*)&z1_ctl);
    if (status)
    {
        PRINTK("LPORT  failed BPCM LPORT_BPCM_Z1_CONTROL\n");
        return status;
    }

    z2_ctl.z2_cclk_clk_en = 1;
    z2_ctl.z2_clk_250_clk_en = 1;
    z2_ctl.z2_tsc_clk_en = 1;
    z2_ctl.z2_data_path_cclk_clk_en = 1;
    z2_ctl.z2_tsc_clk_gated_clk_en = 1;
    z2_ctl.z2_tsclk_clk_en = 1;

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z2_CONTROL, *(uint32*)&z2_ctl);
    if (status)
    {
        PRINTK("LPORT unreset failed BPCM LPORT_BPCM_Z2_CONTROL\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES0_CTRL, 6);
    if (status)
    {
        PRINTK("LPORT unreset failed BPCM LPORT_BPCM_Z0_DSERDES0_CTRL\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES1_CTRL, 6);
    if (status)
    {
        PRINTK("LPORT unreset failed BPCM LPORT_BPCM_Z1_DSERDES0_CTRL\n");
        return status;
    }

    UDELAY(1000);

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES0_CTRL, 0);
    if (status)
    {
        PRINTK("LPORT unreset failed BPCM LPORT_BPCM_Z0_DSERDES0_CTRL\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_LPORT, LPORT_BPCM_Z0_DSERDES1_CTRL, 0);
    if (status)
    {
        PRINTK("LPORT unreset failed BPCM LPORT_BPCM_Z1_DSERDES0_CTRL\n");
        return status;
    }

    return status;
}

int pmc_lport_shutdown(void)
{
#if defined (CONFIG_BCM96858) || defined(_BCM96858_)
    ubus_deregister_port(UCB_NODE_ID_SLV_LPORT);
#endif

    /* shut down all zones */
     return PowerOffDevice(PMB_ADDR_LPORT, 0);
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_lport_init);
postcore_initcall(pmc_lport_init);
#endif


