/*
<:copyright-BRCM:2021:DUAL/GPL:standard 

   Copyright (c) 2021 Broadcom 
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
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/ahci_platform.h>
#include <linux/libata.h>
#include <linux/delay.h>

#include "ahci.h"
#include "bcm_otp.h"
#include "bcm_strap_drv.h"
#include "pmc_sata.h"

#define DRV_NAME           "bcm-bca-ahci"

/* SATA_CTRL regsiters */
#define SATA_TOP_CTRL      0x0040
#define SATA_PORT0_PCB     0x0100

#define SATA_TOP_CTRL_BUS_CTRL      (SATA_TOP_CTRL+0x04)
/* Phy reg */
#define PORT0_SATA3_PCB_REG0        (SATA_PORT0_PCB+0x0200)
#define PORT0_SATA3_PCB_REG1        (SATA_PORT0_PCB+0x0204)
#define PORT0_SATA3_PCB_REG2        (SATA_PORT0_PCB+0x0208)
#define PORT0_SATA3_PCB_REG3        (SATA_PORT0_PCB+0x020c)
#define PORT0_SATA3_PCB_REG4        (SATA_PORT0_PCB+0x0210)
#define PORT0_SATA3_PCB_REG5        (SATA_PORT0_PCB+0x0214)
#define PORT0_SATA3_PCB_REG6        (SATA_PORT0_PCB+0x0218)
#define PORT0_SATA3_PCB_REG7        (SATA_PORT0_PCB+0x021c)
#define PORT0_SATA3_PCB_REG8        (SATA_PORT0_PCB+0x0220)
#define PORT0_SATA3_PCB_BLOCK_ADDR  (SATA_PORT0_PCB+0x023C)
#define PCB_REG(x)                  (PORT0_SATA3_PCB_REG0 + x*4)

/* AHCI GHC regs */
#define GHC_HBA_CAP                 0x00 /* host capabilities */
#define GHC_GLOBAL_HBA_CONTROL      0x04 /* global host control */
#define GHC_INTERRUPT_STATUS        0x08 /* interrupt status */
#define GHC_PORTS_IMPLEMENTED       0x0c /* bitmap of implemented ports */
#define GHC_HOST_VERSION            0x10 /* AHCI spec. version compliancy */

/* SATA_PORT0_AHCI_S1 registers */
#define SATA_PORT0_AHCI_S1_PXIS     0x10
#define SATA_PORT0_AHCI_S1_PXIE     0x14
#define SATA_PORT0_AHCI_S1_PXCMD    0x18


#define SATA3_TXPMD_REG_BANK        0x01a0

#define BDEV_CTRL_RD(x)      readl((void __iomem *)(ahci_priv->ctrl_base+(x)))
#define BDEV_CTRL_WR(x, y)   writel((y), (void __iomem *)(ahci_priv->ctrl_base+(x)))
#define BDEV_AHCI_RD(x)      readl((void __iomem *)(ahci_priv->ahci_base+(x)))
#define BDEV_AHCI_WR(x, y)   writel((y), (void __iomem *)(ahci_priv->ahci_base+(x)))
#define BDEV_SS_RD(x)        readl((void __iomem *)(ahci_priv->ss_base+(x)))
#define BDEV_SS_WR(x, y)     writel((y), (void __iomem *)(ahci_priv->ss_base+(x)))

struct bcm_bca_ahci_priv {
	struct device *dev;
	struct device_node *of_node;
	void __iomem *ahci_base;
	void __iomem *ctrl_base;
	void __iomem *ss_base;
};

struct bcm_bca_ahci_priv *ahci_priv = NULL;

static int is_sata_port_enabled(void)
{
	u32 otp_value = 0;

	if (bcm_strap_parse_and_test(NULL, "sata-strap-enable-bit") == 0)
		return 0;

	if (bcm_otp_is_sata_disabled(&otp_value) == 0 && otp_value)
		return 0;

	return 1;
}

static void write_to_pcb_block(u32 reg_addr, u32 value, u32 pcb_block)
{
	BDEV_CTRL_WR(PORT0_SATA3_PCB_BLOCK_ADDR, pcb_block);
	BDEV_CTRL_WR(reg_addr, value);
}

static u32 read_from_pcb_block(u32 reg_addr, u32 pcb_block)
{
	u32 value;
	BDEV_CTRL_WR(PORT0_SATA3_PCB_BLOCK_ADDR, pcb_block);
	value = BDEV_CTRL_RD(reg_addr);
	return value;
}

static __init void get_freq_lock(struct bcm_bca_ahci_priv *priv)
{
	u32 regData;
	int i = 10;

	//printk("writing PORT0_SATA3_PCB_BLOCK_ADDR\n");

	write_to_pcb_block(PORT0_SATA3_PCB_REG7, 0x873, 0x60);

	write_to_pcb_block(PORT0_SATA3_PCB_REG6, 0xc000, 0x60);

	write_to_pcb_block(PORT0_SATA3_PCB_REG1, 0x3089, 0x50);
	udelay(100);
	write_to_pcb_block(PORT0_SATA3_PCB_REG1, 0x3088, 0x50);
	udelay(1000);
	//// Done with PLL ratio change and re-tunning

	write_to_pcb_block(PORT0_SATA3_PCB_REG2, 0x3000, 0xE0);
	write_to_pcb_block(PORT0_SATA3_PCB_REG6, 0x3000, 0xE0);

	udelay(1000);
	write_to_pcb_block(PORT0_SATA3_PCB_REG3, 0x32, 0x50);

	write_to_pcb_block(PORT0_SATA3_PCB_REG4, 0xA, 0x50);

	write_to_pcb_block(PORT0_SATA3_PCB_REG6, 0x64, 0x50);

	udelay(1000);
	BDEV_CTRL_WR(PORT0_SATA3_PCB_BLOCK_ADDR, 0x00);
	wmb();

	regData = BDEV_CTRL_RD(PORT0_SATA3_PCB_REG1);

	while (i && ((regData & 0x1000) == 0)) {
		regData = BDEV_CTRL_RD(PORT0_SATA3_PCB_REG1);
		udelay(1000);
		i--;
	}

	dev_info(priv->dev, "PLL lock for port0 detected 0x%x...\n", regData);
}

static void sata_sim_init(struct bcm_bca_ahci_priv *priv)
{
	BDEV_AHCI_WR(GHC_GLOBAL_HBA_CONTROL, 0x80000001);
	mdelay(1);
	BDEV_AHCI_WR(GHC_GLOBAL_HBA_CONTROL, 0x80000000);
	mdelay(10);

	BDEV_SS_WR(SATA_PORT0_AHCI_S1_PXIS, 0x7fffffff);
	BDEV_AHCI_WR(GHC_INTERRUPT_STATUS, 0x7fffffff);
	BDEV_SS_WR(SATA_PORT0_AHCI_S1_PXIE, 0x7fffffff);

	BDEV_SS_WR(SATA_PORT0_AHCI_S1_PXCMD, 0x00000010);
	/* setup endianess */
	BDEV_CTRL_WR(SATA_TOP_CTRL_BUS_CTRL, 0x00000000);
}

static void sata_enable_ssc(struct bcm_bca_ahci_priv *priv)
{
	u32 rvalue;

	rvalue = read_from_pcb_block(PCB_REG(1), SATA3_TXPMD_REG_BANK);
	rvalue |= 0x3;
	write_to_pcb_block(PCB_REG(1), rvalue, SATA3_TXPMD_REG_BANK);
}

static void bcm_bca_sata_init(struct bcm_bca_ahci_priv *priv)
{
	dev_info(priv->dev, "Initializing SATA block...\n");

	pmc_sata_power_up();
	mdelay(1);

	get_freq_lock(priv);
	mdelay(1);

	sata_sim_init(priv);
	mdelay(1);

	sata_enable_ssc(priv);
}

static void bcm_bca_sata_deinit(struct bcm_bca_ahci_priv *priv)
{
	pmc_sata_power_down();
	mdelay(1);
}

static void bcm_bca_ahci_host_stop(struct ata_host *host)
{
	struct ahci_host_priv *hpriv = host->private_data;
	struct bcm_bca_ahci_priv *priv = hpriv->plat_data;

	ahci_platform_disable_resources(hpriv);

	bcm_bca_sata_deinit(priv);
}

static struct ata_port_operations ahci_bcm_bca_platform_ops = {
	.inherits	= &ahci_platform_ops,
	.host_stop	= bcm_bca_ahci_host_stop,
};

static const struct ata_port_info ahci_bcm_bca_port_info = {
	.flags		= AHCI_FLAG_COMMON,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &ahci_bcm_bca_platform_ops,
};

static struct scsi_host_template ahci_platform_sht = {
	AHCI_SHT(DRV_NAME),
};

static const struct of_device_id bcm_bca_ahci_of_match[] = {
	{.compatible = "brcm,bcmbca-ahci",},
	{},
};
MODULE_DEVICE_TABLE(of, bcm_bca_ahci_of_match);

static int bcm_bca_ahci_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bcm_bca_ahci_priv *priv;
	struct ahci_host_priv *hpriv;
	struct resource *res;
	int ret;

	if (!is_sata_port_enabled())
	{
		dev_err(dev,"++++ No SATA Hardware Detected\n");
		return -ENODEV;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sata-ctrl");
	priv->ctrl_base = devm_ioremap_resource(dev, res);
	if (IS_ERR_OR_NULL(priv->ctrl_base))
		return PTR_ERR(priv->ctrl_base);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ahci-ss");
	priv->ss_base = devm_ioremap_resource(dev, res);
	if (IS_ERR_OR_NULL(priv->ss_base))
		return PTR_ERR(priv->ss_base);
	
	priv->dev = dev;
	priv->of_node = pdev->dev.of_node;
	ahci_priv = priv;

	hpriv = ahci_platform_get_resources(pdev, 0);
	if (IS_ERR(hpriv))
		return PTR_ERR(hpriv);

	hpriv->plat_data = priv;
	priv->ahci_base = hpriv->mmio;

	bcm_bca_sata_init(priv);

	ret = ahci_platform_enable_resources(hpriv);
	if (ret)
		return ret;

	ret = ahci_platform_init_host(pdev, hpriv, &ahci_bcm_bca_port_info,
				  &ahci_platform_sht);
	if (ret) {
		ahci_platform_disable_resources(hpriv);
		return ret;
	}

	dev_info(dev, "Broadcom AHCI SATA3 registered\n");

	return 0;
}

static int bcm_bca_ahci_remove(struct platform_device *pdev)
{
	return ata_platform_remove_one(pdev);
}

static SIMPLE_DEV_PM_OPS(ahci_pm_ops, ahci_platform_suspend,
			 ahci_platform_resume);

static struct platform_driver bcm_bca_ahci_driver = {
	.probe = bcm_bca_ahci_probe,
	.remove = bcm_bca_ahci_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = bcm_bca_ahci_of_match,
		.pm = &ahci_pm_ops,
	},
};
module_platform_driver(bcm_bca_ahci_driver);

MODULE_DESCRIPTION("Broadcom BCA SATA3 AHCI Controller Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm-bca-ahci");
