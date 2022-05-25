/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
#define printk printk

#include "pmc_drv.h"
#include "pmc_xrdp.h"
#include "BPCM.h"
#include "clk_rst.h"
#include "bcm_ubus4.h"

int pmc_xrdp_init(void)
{
    int status = 0;
    uint32_t reg = 0;
    int i;

    printk("Powering on XRDP core...\n");
    for (i = 0; i < sizeof(xrdp_pmb)/sizeof(pmb_init_t);i++)
    {
        status =  PowerOnDevice(xrdp_pmb[i].pmb_addr);
        if(status)
        {
            printk("Failed to PowerOnDevice %s\n", xrdp_pmb[i].name);
            return status;
        }
    }

    printk("Toggle reset of XRDP core...\n");

    for (i = 0; i < sizeof(xrdp_pmb)/sizeof(pmb_init_t);i++)
    {
        if (xrdp_pmb[i].reset_value == PMB_NO_RESET)
            continue;

        status = ReadBPCMRegister(xrdp_pmb[i].pmb_addr, BPCMRegOffset(sr_control), &reg);
        reg &= ~(xrdp_pmb[i].reset_value);
        status = WriteBPCMRegister(xrdp_pmb[i].pmb_addr, BPCMRegOffset(sr_control), reg);

        if(status)
        {
            printk("failed Toggle reset of %s core to zero...\n",xrdp_pmb[i].name);
            return status;
        }

    }

    for (i = 0; i < sizeof(xrdp_pmb)/sizeof(pmb_init_t);i++)
    {
        if (xrdp_pmb[i].reset_value == PMB_NO_RESET)
            continue;

        status =  ReadBPCMRegister(xrdp_pmb[i].pmb_addr, BPCMRegOffset(sr_control), &reg);
        reg |= xrdp_pmb[i].reset_value;
        status = WriteBPCMRegister(xrdp_pmb[i].pmb_addr, BPCMRegOffset(sr_control), reg);

        if(status)
        {
            printk("failed Toggle reset of %s core to 0x%x...\n", xrdp_pmb[i].name, 
                xrdp_pmb[i].reset_value);
            return status;
        }
    }

#if IS_BCMCHIP(6858)
    apply_ubus_credit_each_master(UBUS_PORT_ID_QM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DQM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_NATC);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA1);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ1);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ2);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ3);

    ubus_master_rte_cfg();
#endif

#if IS_BCMCHIP(63158)
    apply_ubus_credit_each_master(UBUS_PORT_ID_QM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DQM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_NATC);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_SWH);
#endif

#ifdef CONFIG_BCM_UBUS4_DCM
    ubus_cong_threshold_wr(UBUS_PORT_ID_QM, 0);
#ifdef UBUS_PORT_ID_DQM
    ubus_cong_threshold_wr(UBUS_PORT_ID_DQM, 0);
#endif
#ifdef UBUS_PORT_ID_NATC
    ubus_cong_threshold_wr(UBUS_PORT_ID_NATC, 0);
#endif
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA0, 0);
#ifdef UBUS_PORT_ID_DMA1
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA1, 0);
#endif
#ifdef UBUS_PORT_ID_DMA2
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA2, 0);
#endif
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ0, 0);
#ifdef UBUS_PORT_ID_RQ1
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ1, 0);
#endif
#ifdef UBUS_PORT_ID_RQ2
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ2, 0);
#endif
#ifdef UBUS_PORT_ID_RQ3
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ3, 0);
#endif
#ifdef UBUS_PORT_ID_SWH
    ubus_cong_threshold_wr(UBUS_PORT_ID_SWH, 0);
#endif
#endif // #ifdef CONFIG_BCM_UBUS4_DCM

    return status;
}

int pmc_xrdp_shutdown(void)
{
    int status = 0;
    int i;

    for (i = 0; i <= sizeof(xrdp_pmb)/sizeof(pmb_init_t);i++)
    {
        status =  PowerOffDevice(xrdp_pmb[i].pmb_addr, 0);
        if(status)
        {
            printk("Failed to PowerOffDevice %s\n", xrdp_pmb[i].name);
            return status;
        }
    }

    return status;
}

EXPORT_SYMBOL(pmc_xrdp_init);
arch_initcall(pmc_xrdp_init);
