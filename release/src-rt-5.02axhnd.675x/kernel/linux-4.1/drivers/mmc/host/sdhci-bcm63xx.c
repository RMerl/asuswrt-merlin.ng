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
 *
 ************************************************************/
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mmc/host.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <flash_api.h>
#include <bcm_intr.h>
#include <boardparms.h>

#include "sdhci.h"
#include "sdhci-pltfm.h"
   
#define SDHCI_BCM63XX_FORCE_LOW_PERFORMANCE 0
#define SDHCI_BCM63XX_FAKE_TRIGGER_NATIVE_CD 0
#if defined(CONFIG_BCM96858)
#define SDHCI_BCM63XX_FORCE_PIO_MODE 1
#else
#define SDHCI_BCM63XX_FORCE_PIO_MODE 0
#endif

static unsigned short ext_intr;
static struct sdhci_pltfm_data sdhci_bcm63xx_pdata = {
	/* Quirks and ops defined here will be passed to sdhci_host structure */
	.quirks = 0
#if SDHCI_BCM63XX_FORCE_PIO_MODE
        | SDHCI_QUIRK_BROKEN_ADMA
        | SDHCI_QUIRK_BROKEN_DMA
#endif
#if SDHCI_BCM63XX_FORCE_LOW_PERFORMANCE	
		| SDHCI_QUIRK_FORCE_1_BIT_DATA
		| SDHCI_QUIRK_BROKEN_ADMA
		| SDHCI_QUIRK_NO_MULTIBLOCK
#endif		
		,
	.quirks2 = 0
	        | SDHCI_QUIRK2_STOP_WITH_TC
#if SDHCI_BCM63XX_FORCE_LOW_PERFORMANCE
		| SDHCI_QUIRK2_NO_1_8_V
		| SDHCI_QUIRK2_BROKEN_DDR50
#endif		
		,
};

static const struct of_device_id sdhci_bcm63xx_of_match[] = {
	{ .compatible = "brcm,bcm63xx-sdhci"},
	{}
};
MODULE_DEVICE_TABLE(of, sdhci_bcm63xx_of_match);

#if defined(CONFIG_BCM963138)
#include <bcm_intr.h>
static struct resource bcm63138_emmc_resources[] = {
	[0] = 	{
			.start = EMMC_HOSTIF_PHYS_BASE,
			.end = EMMC_HOSTIF_PHYS_BASE + SZ_256 - 1,  /* we only need this area */
			/* the memory map actually makes SZ_4K available  */
			.flags = IORESOURCE_MEM,
		},
	[1] =	{
			.start = INTERRUPT_ID_EMMC,
			.end = INTERRUPT_ID_EMMC,
			.flags = IORESOURCE_IRQ,
		},
};
#endif

#if SDHCI_BCM63XX_FAKE_TRIGGER_NATIVE_CD
static int sdhci_bcm63xx_trigger_native_card_detection_int(void)
{
	/* Placeholder to fake-trigger native card detection interrupt - if required */
	return 0;
}
#endif

static irqreturn_t sdhci_bcm63xx_card_detection_isr(int irq, void *dev_id)
{
	/* Schedule a card detection after a debounce timeout */
	struct mmc_host *host = dev_id;

#if SDHCI_BCM63XX_FAKE_TRIGGER_NATIVE_CD
	sdhci_bcm63xx_trigger_native_card_detection_int();
#else	
	host->trigger_card_event = true;
	mmc_detect_change(host, msecs_to_jiffies(200));
#endif
		
	BcmHalExternalIrqClear(irq);
	return IRQ_HANDLED;
}

static int sdhci_bcm63xx_init_card_detection_irq(struct mmc_host *host)
{
	int ret = -1;
	if( BpGetSDCardDetectExtIntr(&ext_intr) == BP_SUCCESS ) 
	{
		ret = ext_irq_connect(ext_intr, (void *)host, (FN_HANDLER)sdhci_bcm63xx_card_detection_isr);
	}
	
	return ret;
}

static int sdhci_bcm63xx_probe(struct platform_device *pdev)
{
	int res = 0;
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;

#if defined(CONFIG_BCM963138)
	/* Get flash type detected by flash API */
	int flash_type = flash_get_flash_type();

	/* 63138 does not support concurrent NAND and eMMC accesses, therefore
	 * donot init the emmc driver when we are strapped to boot from NAND
	 */
	if (flash_type == FLASH_IFC_NAND)
		return res;
#endif

	/* Force straps to enable emmc signals - must prevent platform device register if not using emmc */
	AHBSS_CTRL->ahbss_ctrl_cfg |= FORCE_EMMC_BOOT_STRAP;
	EMMC_TOP_CFG->emmc_top_cfg_sd_pin_sel = 1;
	
	/* Check if we are in normal mode, if not then force us in normal mode */
	while( EMMC_BOOT->emmc_boot_status & EMMC_BOOT_MODE_MASK )
	{
		EMMC_BOOT->emmc_boot_main_ctl &= ~EMMC_BOOT_ENABLE;		
	}
	
#if defined(CONFIG_BCM963138)
	/* For 63138 the interrupt controller is not listed in the device tree
	 * therefore all interrupt related bindings will fail. We therefore have 
	 * to manually insert the IRQs by redefining the driver resources
	 */
	pdev->resource = bcm63138_emmc_resources;
	pdev->num_resources = ARRAY_SIZE(bcm63138_emmc_resources);
#endif	

	host = sdhci_pltfm_init(pdev, &sdhci_bcm63xx_pdata, sizeof(&sdhci_bcm63xx_pdata));
	if (IS_ERR(host))
		return PTR_ERR(host);
	
	/* Get pltfm host */
	pltfm_host = sdhci_priv(host);

	res = mmc_of_parse(host->mmc);
	if (res)
		goto err_pltfm_free;

	sdhci_bcm63xx_init_card_detection_irq(host->mmc);

	sdhci_get_of_property(pdev);

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
