// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*

*/

#define PRINTK printf

#include "pmc_drv.h"
#include "pmc_xrdp.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
#include "bcm_ubus4.h"

int pmc_xrdp_init(void)
{
    int status = 0;

#if IS_BCMCHIP(6858)
    uint32_t reg = 0;

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

    ubus_master_rte_cfg();

#elif IS_BCMCHIP(63158) || IS_BCMCHIP(6856)

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

#if IS_BCMCHIP(6856)
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
#elif IS_BCMCHIP(6846)
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
#elif IS_BCMCHIP(6878) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6855)
    status = PowerOnDevice(PMB_ADDR_XRDP);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_XRDP\n");
        return status;
    }

#if IS_BCMCHIP(6878) || IS_BCMCHIP(6855)
    status = PowerOnDevice(PMB_ADDR_WAN);
    if(status)
    {
        PRINTK("Failed to PowerOnDevice PMB_ADDR_WAN\n");
        return status;
    }
#endif

    PRINTK("Toggle reset of XRDP core...\n");
    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0x7);
    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to zero...\n");
        return status;
    }

    status = WriteBPCMRegister(PMB_ADDR_XRDP, BPCMRegOffset(sr_control), 0xfffffff);
    if (status)
    {
        PRINTK("failed Toggle reset of XRDP core to 0xffffffff...\n");
        return status;
    }
#else
    PRINTK("%s is not implemented in platform yet!\n", __FUNCTION__);
#endif

#if IS_BCMCHIP(63158)
    apply_ubus_credit_each_master(UBUS_PORT_ID_QM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DQM);
    apply_ubus_credit_each_master(UBUS_PORT_ID_NATC);
    apply_ubus_credit_each_master(UBUS_PORT_ID_DMA0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_RQ0);
    apply_ubus_credit_each_master(UBUS_PORT_ID_SWH);
#endif
#if defined(CONFIG_BCM_UBUS4_DCM)
    ubus_cong_threshold_wr(UBUS_PORT_ID_QM, 0);
#if !IS_BCMCHIP(6855)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DQM, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_NATC, 0);
#endif
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA0, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ0, 0);
#if IS_BCMCHIP(63158)
    ubus_cong_threshold_wr(UBUS_PORT_ID_SWH, 0);
#elif IS_BCMCHIP(6858)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ2, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ3, 0);
#elif IS_BCMCHIP(6856)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ1, 0);
#elif IS_BCMCHIP(6855)
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA1, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_DMA2, 0);
    ubus_cong_threshold_wr(UBUS_PORT_ID_RQ1, 0);
#endif
#endif

    return status;
}

int pmc_xrdp_shutdown(void)
{
    int status = 0;
#if IS_BCMCHIP(6858)
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

#if !defined(_CFE_) && !defined(__UBOOT__)
EXPORT_SYMBOL(pmc_xrdp_init);
postcore_initcall(pmc_xrdp_init);
#endif
