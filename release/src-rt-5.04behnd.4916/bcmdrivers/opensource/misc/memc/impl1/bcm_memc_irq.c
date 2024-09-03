/*
<:copyright-BRCM:2019:DUAL/GPL:standard 

   Copyright (c) 2019 Broadcom 
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


