/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

#ifndef _CFE_
#define PRINTK  printk
#else
#define PRINTK  xprintf
#endif
#include "pmc_drv.h"
#include "pmc_xrdp.h"
#include "BPCM.h"
#include "clk_rst.h"
#include "bcm_map_part.h"
#include "bcm_ubus4.h"

int pmc_xrdp_init(void)
{

    int status = 0;

#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
    uint32 reg = 0;

#if 0
	status =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 1, 2);
	if (status)
	{
		PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
		return status;
	}

	status =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 2, 2);
	if (status)
	{
	 PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
	 return status;
	}

	status =  pll_ch_freq_set(PMB_ADDR_RDPPLL, 3, 6);
	if (status)
	{
	 PRINTK("UNIPLL Channel 2 Mdiv=5 failed\n");
	 return status;
	}
#endif
    status =  PowerOnDevice(PMB_ADDR_XRDP);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_QM);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_QM\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD0);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD0\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD1);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD1\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD2);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD2\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC_QUAD3);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC_QUAD3\n");
        return status;
    }

    PRINTK("Toggle reset of XRDP core...\n");
    status = ReadBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), &reg);
    reg &= 0xFFFFFF00;
    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), reg);

    status |= ReadBPCMRegister(PMB_ADDR_XRDP_QM, BPCMRegOffset(sr_control), &reg);
    reg &= 0xFFFFFF00;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_QM, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD0, BPCMRegOffset(sr_control), &reg);
    reg &= 0xFFFFFF00;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD0, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD1, BPCMRegOffset(sr_control), &reg);
    reg &= 0xFFFFFF00;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD1, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD2, BPCMRegOffset(sr_control), &reg);
    reg &= 0xFFFFFF00;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD2, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD3, BPCMRegOffset(sr_control), &reg);
    reg &= 0xFFFFFF00;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD3, BPCMRegOffset(sr_control), reg);

    if(status)
    {
        PRINTK("failed Toggle reset of XRDP core to zero...\n");
        return status;
    }
    status |=  ReadBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), &reg);
    reg |= 0xff;
    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_QM, BPCMRegOffset(sr_control), &reg);
    reg |= 0xff;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_QM, BPCMRegOffset(sr_control), 0xff);


    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD0, BPCMRegOffset(sr_control), &reg);
    reg |= 0xff;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD0, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD1, BPCMRegOffset(sr_control), &reg);
    reg |= 0xff;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD1, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD2, BPCMRegOffset(sr_control), &reg);
    reg |= 0xff;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD2, BPCMRegOffset(sr_control), reg);

    status |=  ReadBPCMRegister(PMB_ADDR_XRDP_RC_QUAD3, BPCMRegOffset(sr_control), &reg);
    reg |= 0xff;
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC_QUAD3, BPCMRegOffset(sr_control), reg);

    if(status)
    {
        PRINTK("failed Toggle reset of XRDP core to 0xff...\n");
        return status;
    }

    apply_ubus_credit_each_master(UBUS_PORT_ID_QM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DQM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_NATC);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA1);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ1);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ2);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ3);

#if defined(_BCM96858_)
    ubus_master_rte_cfg();
#endif

#elif defined(_BCM96836_) || defined(CONFIG_BCM96836) || defined(_BCM963158_) || defined(CONFIG_BCM963158) ||\
      defined(_BCM96856_) || defined(CONFIG_BCM96856)

    status =  PowerOnDevice(PMB_ADDR_XRDP);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP status[%d]\n",status);
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC0);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC0\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC1);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC1\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC2);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC2\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC3);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC3\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC4);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC4\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC5);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC5\n");
        return status;
    }

#if defined(_BCM96856_) || defined(CONFIG_BCM96856)
    status = PowerOnDevice(PMB_ADDR_XRDP_RC6);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC4\n");
        return status;
    }
    status = PowerOnDevice(PMB_ADDR_XRDP_RC7);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice XRDP_RC5\n");
        return status;
    }
    PRINTK("Toggle reset of XRDP core...\n");
    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0x7);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC0, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC1, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC2, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC3, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC4, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC5, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC6, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC7, BPCMRegOffset(sr_control), 0); 
    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to zero...\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC0, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC1, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC2, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC3, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC4, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC5, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC6, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC7, BPCMRegOffset(sr_control), 0xfffffff); 
    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to 0xffffffff...\n");
        return status;
    }
#endif
    status = PowerOnDevice(PMB_ADDR_WAN);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_WAN\n");
        return status;
    }

    PRINTK("Toggle reset of XRDP core...\n");
    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0x7);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC0, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC1, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC2, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC3, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC4, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC5, BPCMRegOffset(sr_control), 0);

    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to zero...\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC0, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC1, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC2, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC3, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC4, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC5, BPCMRegOffset(sr_control), 0xfffffff);

    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to 0xffffffff...\n");
        return status;
    }
#elif defined(_BCM96846_) || defined(CONFIG_BCM96846)
    status = PowerOnDevice(PMB_ADDR_XRDP);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_XRDP\n");
        return status;
    }

    status = PowerOnDevice(PMB_ADDR_XRDP_RC0);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_XRDP_RC0\n");
        return status;
    }

    status = PowerOnDevice(PMB_ADDR_XRDP_RC1);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_XRDP_RC1\n");
        return status;
    }

    status = PowerOnDevice(PMB_ADDR_XRDP_RC2);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_XRDP_RC2\n");
        return status;
    }

    status = PowerOnDevice(PMB_ADDR_WAN);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_WAN\n");
        return status;
    }

    PRINTK("Toggle reset of XRDP core...\n");
    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0x7);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC0, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC1, BPCMRegOffset(sr_control), 0);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC2, BPCMRegOffset(sr_control), 0);

    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to zero...\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC0, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC1, BPCMRegOffset(sr_control), 0xfffffff);
    status |= WriteBPCMRegister(PMB_ADDR_XRDP_RC2, BPCMRegOffset(sr_control), 0xfffffff);

    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to 0xffffffff...\n");
        return status;
    }
#else
    PRINTK("%s is not implemented in platform yet!\n", __FUNCTION__);
#endif

#if defined(CONFIG_BCM963158)
    apply_ubus_credit_each_master(UBUS_PORT_ID_QM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DQM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_NATC);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_SWH);
#endif

#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(UBUS_PORT_ID_QM, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_DQM, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_NATC, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA0, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ0, 0);
#if defined(CONFIG_BCM963158)
    ubus_cong_threshold_wr(UBUS_PORT_ID_SWH, 0);
#elif defined(CONFIG_BCM96858)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ2, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ3, 0);
#elif defined(CONFIG_BCM96856)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ1, 0);
#endif
#endif
    return status;
}

int pmc_xrdp_shutdown(void)
{
    int status = 0;
#if defined(_BCM96858_) || defined(CONFIG_BCM96858)
    status =  PowerOffDevice(PMB_ADDR_XRDP, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_QM, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_QM\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD0, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD0\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD1, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD1\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD2, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD2\n");
       return status;
    }
    status = PowerOffDevice(PMB_ADDR_XRDP_RC_QUAD3, 0);
    if(status)
    {
       PRINTK("Failed to PowerOffDevice XRDP_RC_QUAD3\n");
       return status;
    }
#else
    PRINTK("%s is not implemented in platform yet!\n", __FUNCTION__);
#endif

    return status;
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_xrdp_init);
postcore_initcall(pmc_xrdp_init);
#endif
