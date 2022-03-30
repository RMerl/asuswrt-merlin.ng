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
#include <linux/of.h>
#include <linux/mmc/host.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <flash_api.h>
#include <bcm_intr.h>
#include <pmc/pmc_sdhci.h>
#include <bcm_bca_extintr.h>

#include "sdhci.h"
#include "sdhci-pltfm.h"
   
#define SDHCI_BCM63XX_TOPCFG_CAP_REG0_OFFSET		0x10
#define SDHCI_BCM63XX_TOPCFG_CAP_REG_OVERRIDE_OFFSET	0x0c
#define SDHCI_BCM63XX_TOPCFG_CAP_REG_OVERRIDE_VAL	0x01
#define SDHCI_BCM63XX_TOPCFG_SD_PIN_SEL_OFFSET		0x54
#define SDHCI_BCM63XX_TOPCFG_SD_PIN_SEL_VAL		0x01

#define SDHCI_BCM63XX_BOOT_MAIN_CTL_OFFSET	0x00
#define SDHCI_BCM63XX_BOOT_EMMC_BOOT_ENABLE_VAL	0x01
#define SDHCI_BCM63XX_BOOT_STATUS_OFFSET	0x04
#define SDHCI_BCM63XX_BOOT_EMMC_BOOT_MODE_MASK	0x01

#define SDHCI_BCM63XX_AHBSS_CTRL_CFG_OFFSET			0x00	
#define SDHCI_BCM63XX_AHBSS_CTRL_FORCE_EMMC_BOOTSTRAP_VAL	0x01

#define SDHCI_BCM63XX_FORCE_LOW_PERFORMANCE 0
#define SDHCI_BCM63XX_FAKE_TRIGGER_NATIVE_CD 0
#if defined(CONFIG_BCM96858)
#define SDHCI_BCM63XX_FORCE_PIO_MODE 1
#else
#define SDHCI_BCM63XX_FORCE_PIO_MODE 0
#endif

int card_irq = -1;

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
	bcm_bca_extintr_clear(irq);
	return IRQ_HANDLED;
}

static int sdhci_bcm63xx_setup_baseclk(struct platform_device *pdev, void __iomem *top_cfg_base)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
        struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	unsigned int base_clk_freq;
        struct resource *res;
	pltfm_host = sdhci_priv(host);

	/* read capabilities registers */
	sdhci_read_caps(host);
	host->read_caps = false;

	/* determing max base clock frequency */
	base_clk_freq = (host->caps & SDHCI_CLOCK_V3_BASE_MASK)
			>> SDHCI_CLOCK_BASE_SHIFT;
	base_clk_freq *= 1000000;

	dev_info(&pdev->dev, "'clock-frequency' %uMhz, base_clk_freq %uMhz\n", 
		pltfm_host->clock, base_clk_freq ); 

	/* If requested clock is greater than max clock, update base_clock */
	if( pltfm_host->clock != (base_clk_freq) )
	{
		/* Enable new base clock pll */
		if( pmc_sdhci_set_base_clk(pltfm_host->clock) )
		{
	        	dev_err(&pdev->dev, "Cannot set platfrom clock frequency, using defaults!\n");
		        return -EINVAL;
		}

		/* Option 1: override baseclock in override register */
		/* Write new base clock frequency */
		host->caps &= ~(SDHCI_CLOCK_V3_BASE_MASK);
		host->caps |= (pltfm_host->clock / 1000000) 
			<< SDHCI_CLOCK_BASE_SHIFT;
		writel(host->caps, top_cfg_base 
			+ SDHCI_BCM63XX_TOPCFG_CAP_REG0_OFFSET);

		/* enable capability override */
		writel(SDHCI_BCM63XX_TOPCFG_CAP_REG_OVERRIDE_VAL, top_cfg_base 
			+ SDHCI_BCM63XX_TOPCFG_CAP_REG_OVERRIDE_OFFSET);

		/* Indicate presets are not valid anymore */
		host->quirks2 |= SDHCI_QUIRK2_PRESET_VALUE_BROKEN;
	}

	/* clear caps read flag */
	return 0;
}

static int sdhci_bcm63xx_init_card_detection_irq(struct device *dev,
						 struct device_node *np,
						 struct mmc_host *host)
{
	if (of_find_property(np, "card_detect", NULL))
		card_irq = bcm_bca_extintr_request(dev, np, "card_detect",
					   sdhci_bcm63xx_card_detection_isr,
					   (void*)host, "SD card detection", NULL);
	return 0;
}

static int sdhci_bcm63xx_probe(struct platform_device *pdev)
{
	int res = 0;
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;	

	void __iomem *reg_base;
	void __iomem *reg_top_cfg_base;
        struct resource *res1;
	u32 reg_val = 0;

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
 	res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sdhci-topcfg" );
        if ( !res1 ) 
	{
        	dev_err(dev, "Platform resource sdhci-topcfg is missing\n");
	        return -EINVAL;
        }
	reg_top_cfg_base = devm_ioremap_resource(dev, res1);
        if (IS_ERR(reg_top_cfg_base)) 
	{
        	dev_err(&pdev->dev, "Ioremap failed for sdhci-topcfg\n");
	        return -EINVAL;
	}
	writel(SDHCI_BCM63XX_TOPCFG_SD_PIN_SEL_VAL, reg_top_cfg_base 
		+ SDHCI_BCM63XX_TOPCFG_SD_PIN_SEL_OFFSET);

	/* Set interface to eMMC */
 	res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sdhci-ahbss-ctrl" );
        if ( !res1 ) 
	{
        	dev_err(dev, "Platform resource sdhci-ahbss-ctrl is missing\n");
	        return -EINVAL;
        }
	reg_base = devm_ioremap_resource(dev, res1);
        if (IS_ERR(reg_base)) 
	{
        	dev_err(dev, "Ioremap failed for sdhci-ahbss-ctrl\n");
	        return -EINVAL;
	}
	reg_val = readl(reg_base + SDHCI_BCM63XX_AHBSS_CTRL_CFG_OFFSET);
	reg_val |= SDHCI_BCM63XX_AHBSS_CTRL_FORCE_EMMC_BOOTSTRAP_VAL;
	writel(reg_val, reg_base + SDHCI_BCM63XX_AHBSS_CTRL_CFG_OFFSET);
	devm_iounmap(dev, reg_base);
	
	/* Check if we are in normal mode, if not then force us in normal mode */
 	res1 = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sdhci-boot" );
        if ( !res1 ) 
	{
        	dev_err(dev, "Platform resource sdhci-boot is missing\n");
	        return -EINVAL;
        }
	reg_base = devm_ioremap_resource(dev, res1);
        if (IS_ERR(reg_base)) 
	{
        	dev_err(&pdev->dev, "Ioremap failed for sdhci-boot\n");
	        return -EINVAL;
	}        
	reg_val = readl(reg_base + SDHCI_BCM63XX_BOOT_STATUS_OFFSET);
	while( reg_val & SDHCI_BCM63XX_BOOT_EMMC_BOOT_MODE_MASK)
	{
		/* Get us out of boot mode */
		reg_val = readl(reg_base + SDHCI_BCM63XX_BOOT_MAIN_CTL_OFFSET);	
		reg_val &= ~SDHCI_BCM63XX_BOOT_EMMC_BOOT_ENABLE_VAL;
		writel(reg_val, reg_base + SDHCI_BCM63XX_BOOT_MAIN_CTL_OFFSET);

		/* Read status */
		reg_val = readl(reg_base + SDHCI_BCM63XX_BOOT_STATUS_OFFSET);
	}
	devm_iounmap(dev, reg_base);
	
	/* Initialize platform driver */
	host = sdhci_pltfm_init(pdev, &sdhci_bcm63xx_pdata, sizeof(&sdhci_bcm63xx_pdata));
	if (IS_ERR(host))
		return PTR_ERR(host);
	
	/* Get pltfm host */
	pltfm_host = sdhci_priv(host);

	/* Clear f_max and platform base clock */
	host->mmc->f_max = 0;
	pltfm_host->clock = 0;

	/* Parse mmc params from dtb */
	res = mmc_of_parse(host->mmc);
	if (res)
		goto err_pltfm_free;

	/* Initialize card detection ext irq */
	sdhci_bcm63xx_init_card_detection_irq(dev, np, host->mmc);

	/* Parse platform params from dtb */
	sdhci_get_of_property(pdev);

	/* Modify base clock frequency if required */
	if( pltfm_host->clock )
		if( sdhci_bcm63xx_setup_baseclk(pdev, reg_top_cfg_base) )
	        	dev_warn(dev, "Proceeding with default clock configuration!\n");

	/* Unmap top_cfg */
	devm_iounmap(dev, reg_top_cfg_base);

	/* Add host */
	res = sdhci_add_host(host);

err_pltfm_free:	
	if (res)
		sdhci_pltfm_free(pdev);

	return res;
}

static int sdhci_bcm63xx_remove(struct platform_device *pdev)
{
	int res;

	if (card_irq > 0)
		bcm_bca_extintr_free(&pdev->dev, card_irq, 0);	
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
