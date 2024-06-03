/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
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
*/

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <bcm_dgasp.h>
#include <bcm_ioremap_shared.h>
#include "pmc_dsl.h"
#include "bcm_dgasphw.h"

#define BGBIAS_MASK 0x0000FFFF
#define AFE_MASK    0x0000FFFF

#define DGASP_AFE_DBG   0

static volatile uint32_t* bgBiasReg_0 = NULL;
static volatile uint32_t* afeReg_0 = NULL;
static uint32_t bgbias_val = 0;
static uint32_t afe_val = 0;

static uint32_t* get_afe_regs( struct platform_device *pdev, char * res_name)
{
    struct resource *res1 = NULL;
    void __iomem *reg_base = NULL;

    res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, res_name );
    if ( !res1 ) 
    {
        dev_err(&pdev->dev, "Platform resource %s is missing\n", res_name);
        return NULL;
    }
    else
    {
        reg_base = devm_ioremap_shared_resource(&pdev->dev, res1);
#if DGASP_AFE_DBG        
        printk("%s: 0x%08x<-->0x%08x:0x%08x\n", res_name, (unsigned int)res1->start, (unsigned int)reg_base, *(uint32_t*)reg_base);
#endif        
        if (IS_ERR(reg_base)) 
        {
            dev_err(&pdev->dev, "Ioremap failed for %s\n", res_name);
            return NULL;
        }
    }
    return reg_base;
}


int dgasp_hw_init(struct platform_device *pdev)
{
    /* Get bgbias regs and val */
    bgBiasReg_0 = get_afe_regs(pdev, "dg-bgbias-reg");
    if(of_property_read_u32(pdev->dev.of_node, "bgbias-reg-val", &bgbias_val))
        bgBiasReg_0 = NULL;

    /* Get afe regs and val */
    afeReg_0 = get_afe_regs(pdev, "dg-afe-reg");
    if(of_property_read_u32(pdev->dev.of_node, "afe-reg-val", &afe_val))
        afeReg_0 = NULL;

#if DGASP_AFE_DBG        
    printk("Values 0x%08x 0x%08x\n", bgbias_val, afe_val);
#endif
    if( !bgBiasReg_0 || !afeReg_0 )
        return -EINVAL;
        
    pmc_dsl_power_up();
    pmc_dsl_core_reset();
    msleep(5);
    return 0;
}

int dgasp_hw_disable_irq(void)
{
    return 0;
}

int dgasp_hw_enable_irq( int bIrqMapped )
{
#if DGASP_AFE_DBG        
    printk("0x%08x:0x%08x 0x%08x:0x%08x\n", (unsigned int)bgBiasReg_0, (unsigned int)afeReg_0, *bgBiasReg_0, *afeReg_0);
#endif    

    if( bgBiasReg_0 )
        *bgBiasReg_0 = (*bgBiasReg_0 & ~BGBIAS_MASK) | (bgbias_val & BGBIAS_MASK);

    if( afeReg_0 )
        *afeReg_0 = (*afeReg_0 & ~AFE_MASK) | (afe_val & AFE_MASK);

#if DGASP_AFE_DBG        
    printk("0x%08x:0x%08x 0x%08x:0x%08x\n", (unsigned int)bgBiasReg_0, (unsigned int)afeReg_0, *bgBiasReg_0, *afeReg_0);
#endif    
    msleep(5);
    return 0;
}
