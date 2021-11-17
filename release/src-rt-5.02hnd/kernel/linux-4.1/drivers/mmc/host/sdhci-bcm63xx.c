#if defined(CONFIG_BCM_KF_EMMC)
/**************************************************************
 * sdhci-bcm63xx.c Support for SDHCI on Broadcom DSL/PON CPE SoC's
 *
 * Author: Farhan Ali <fali@broadcom.com>
 * Based on sdhci-brcmstb.c
 *
 * Copyright (c) 2014 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2014:DUAL/GPL:standard
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
 *
 ************************************************************/
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/module.h>
#include <bcm_map_part.h>

#include "sdhci.h"
#include "sdhci-pltfm.h"
   
static struct sdhci_pltfm_data sdhci_bcm63xx_pdata = {
	/* Quirks and ops defined here will be passed to sdhci_host structure */
};

static const struct of_device_id sdhci_bcm63xx_of_match[] = {
	{ .compatible = "brcm,bcm63xx-sdhci"},
	{}
};
MODULE_DEVICE_TABLE(of, sdhci_bcm63xx_of_match);

static int sdhci_bcm63xx_probe(struct platform_device *pdev)
{
	int res = 0;
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;

	/* Force straps to enable emmc signals - must prevent platform device register if not using emmc */
	AHBSS_CTRL->ahbss_ctrl_cfg |= FORCE_EMMC_BOOT_STRAP;
	
	/* Check if we are in normal mode, if not then force us in normal mode */
	while( EMMC_BOOT->emmc_boot_status & EMMC_BOOT_MODE_MASK )
	{
		EMMC_BOOT->emmc_boot_main_ctl &= ~EMMC_BOOT_ENABLE;		
	}
	
	host = sdhci_pltfm_init(pdev, &sdhci_bcm63xx_pdata, sizeof(&sdhci_bcm63xx_pdata));
	if (IS_ERR(host))
		return PTR_ERR(host);
	
	/* Get pltfm host */
	pltfm_host = sdhci_priv(host);

	res = mmc_of_parse(host->mmc);
	if (res)
		goto err_pltfm_free;

	res = sdhci_add_host(host);

err_pltfm_free:	
	if (res)
		sdhci_pltfm_free(pdev);
	
	return res;
}

static int sdhci_bcm63xx_remove(struct platform_device *pdev)
{
	int res;
	res = sdhci_pltfm_unregister(pdev);
	return res;
}

static struct platform_driver sdhci_bcm63xx_driver = {
	.driver		= {
		.name	= "sdhci-bcm63xx",
		.owner	= THIS_MODULE,
		.of_match_table = sdhci_bcm63xx_of_match,
	},
	.probe		= sdhci_bcm63xx_probe,
	.remove		= sdhci_bcm63xx_remove,
};

module_platform_driver(sdhci_bcm63xx_driver);

MODULE_DESCRIPTION("SDHCI driver for Broadcom DSL/PON CPE devices");
MODULE_AUTHOR("Farhan Ali <fali@broadcom.com>");
MODULE_LICENSE("GPL v2");
#endif /* CONFIG_BCM_KF_EMMC */
