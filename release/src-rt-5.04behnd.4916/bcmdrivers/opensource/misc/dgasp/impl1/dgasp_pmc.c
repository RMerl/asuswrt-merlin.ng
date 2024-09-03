/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
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
*/

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <bcm_dgasp.h>
#include "pmc_dgasp.h"
#include "bcm_dgasphw.h"
#include <bcm_ioremap_shared.h>


#define DGASP_PMC_DBG   0
#define DG_EN_SHIFT     3 
#define DGASP_IRQMASK_RES   "dg-irq-mask-reg"
#define DGASP_HWREINIT_FLAG "reinit-hw-irqenable"

static volatile uint32_t* dgasp_irq_mask_reg = NULL;
static int reinit_on_irqenable = 0;

void kerSysDisableDyingGaspOverride(void)
{
#if defined(CONFIG_BCM_PMC)
    pmc_dgasp_override_disable();
    msleep(5);
#endif
}
EXPORT_SYMBOL(kerSysDisableDyingGaspOverride);

void kerSysEnableDyingGaspOverride(void)
{
#if defined(CONFIG_BCM_PMC)
    pmc_dgasp_override_enable();
    msleep(5);
#endif
}
EXPORT_SYMBOL(kerSysEnableDyingGaspOverride);

void kerSysGetDyingGaspConfig( unsigned int * afe_reg0, unsigned int * bg_bias0)
{
#if defined(CONFIG_BCM_PMC)
    pmc_dgasp_get_config( afe_reg0, bg_bias0 );
#endif
}
EXPORT_SYMBOL(kerSysGetDyingGaspConfig);

int dgasp_hw_init(struct platform_device *pdev)
{
    struct resource *res1 = NULL;
    void __iomem *reg_base = NULL;

    res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, DGASP_IRQMASK_RES );
    if ( !res1 ) 
    {
        dev_err(&pdev->dev, "Platform resource %s is missing\n", DGASP_IRQMASK_RES);
        return -EINVAL;
    }
    else
    {
        reg_base = devm_ioremap_shared_resource(&pdev->dev, res1);
#if DGASP_PMC_DBG        
        printk("%s: 0x%px<-->0x%px:0x%08x\n", DGASP_IRQMASK_RES, (void *)res1->start, (void *)reg_base, *(uint32_t*)reg_base);
#endif        

        if (IS_ERR(reg_base)) 
        {
            dev_err(&pdev->dev, "Ioremap failed for %s\n", DGASP_IRQMASK_RES);
            return -EINVAL;
        }
        else
            dgasp_irq_mask_reg = reg_base;
    }

    /* get reinit property */
    of_property_read_u32(pdev->dev.of_node, DGASP_HWREINIT_FLAG, &reinit_on_irqenable);
#if DGASP_PMC_DBG        
    printk("%s: reinit:0x%08x\n", DGASP_HWREINIT_FLAG, reinit_on_irqenable);
#endif        

#if defined(CONFIG_BCM_PMC)
    pmc_dgasp_init();
    msleep(5);
#endif

    return 0;
}

int dgasp_hw_disable_irq(void)
{
    /* Clear local interrupt mask for DG */
    if( dgasp_irq_mask_reg )
        *dgasp_irq_mask_reg &= ~(1 << DG_EN_SHIFT);

    return 0;
}

int dgasp_hw_enable_irq( int bIrqMapped )
{
    /* If we need to re-init the dgasp hw on every irq enable */
    if( reinit_on_irqenable )
    {
        if( bIrqMapped )
        {
#if defined(CONFIG_BCM_PMC)
            pmc_dgasp_init();
            msleep(5);
#endif
        }
    }

    /* Set local interrupt mask for DG */
    if( dgasp_irq_mask_reg )
        *dgasp_irq_mask_reg |= (1 << DG_EN_SHIFT);

    return 0;
}
