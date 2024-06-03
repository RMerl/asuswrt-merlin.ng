/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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

#include <linux/interrupt.h>

#include <linux/of_device.h>
#include <linux/of_address.h>

static void __iomem *memc_int_clear_reg = NULL;
static void __iomem *memc_int_mask_clear_reg = NULL;

static irqreturn_t memc_isr(int irq, void *dev_id)
{
	pr_err("ALERT!! Memory access violation detected\n");

	/* Acknowledge interrupt */
	*(uint32_t*)memc_int_clear_reg = 0xF;

	return IRQ_HANDLED;
}

static void bcm_memc_install_isr (int irq)
{
	if (request_irq(irq, memc_isr, 0, "memc", NULL))
		pr_err("Failed to configure interrupt \n");
	else {
		/* Clear the pending interrupts */
		*(uint32_t*)memc_int_clear_reg = 0xF;
		/* Enable interrupt by clearing the mask */
		*(uint32_t*)memc_int_mask_clear_reg = 0xF;
	}
}

int bcm_memc_irq_init(struct platform_device *pdev)
{
	int memc_irq = 0;
	struct resource *res = NULL;

	/* Extract Interrupt Control Registers */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "memc-int-clear-reg");
	if ( !res ) {
		dev_err(&pdev->dev, "Platform resource memc-int-clear-reg is missing\n");
		goto out;
	}

	memc_int_clear_reg = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(memc_int_clear_reg)) {
		dev_err(&pdev->dev, "Ioremap failed for memc-int-clear-reg\n");
		goto out;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "memc-int-mask-clear-reg");
	if ( !res ) {
		dev_err(&pdev->dev, "Platform resource memc-int-mask-clear-reg is missing\n");
		goto out;
	}

	memc_int_mask_clear_reg = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(memc_int_mask_clear_reg)) {
		dev_err(&pdev->dev, "Ioremap failed for memc-int-mask-clear-reg\n");
		goto out;
	}

	/* Configure interrupt handler */
	memc_irq = platform_get_irq(pdev, 0);
	if ( memc_irq < 0 ) {
		dev_err(&pdev->dev, "Failed to get MEMC irq\n");
		goto out;
	}

	bcm_memc_install_isr(memc_irq);
	return 0;

out:
	if (memc_int_clear_reg)
		iounmap(memc_int_clear_reg);

	if (memc_int_mask_clear_reg)
		iounmap(memc_int_mask_clear_reg);

	return -EINVAL;
}


