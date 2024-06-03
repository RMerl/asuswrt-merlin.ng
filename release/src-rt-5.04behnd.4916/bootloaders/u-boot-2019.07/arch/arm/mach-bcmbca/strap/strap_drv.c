/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2021 Broadcom Ltd.
 */
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

/*****************************************************************************
 *  Description:
 *      Strap Register DT based driver.
 *****************************************************************************/
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <bcm_strap_drv.h>

/* boot select bits 0-2 */
#define BOOT_NAND_ECC_DISABLE                   0x0
#define BOOT_NAND_ECC_1_BIT                     0x1
#define BOOT_NAND_ECC_4_BIT                     0x2
#define BOOT_NAND_ECC_8_BIT                     0x3
#define BOOT_NAND_ECC_12_BIT                    0x4
#define BOOT_NAND_ECC_24_BIT                    0x5
#define BOOT_NAND_ECC_40_BIT                    0x6
#define BOOT_NAND_ECC_60_BIT                    0x7

/* boot select bits 3-5 */
#define BOOT_SEL_STRAP_NAND_2K_PAGE             0x00
#define BOOT_SEL_STRAP_NAND_4K_PAGE             0x08
#define BOOT_SEL_STRAP_NAND_8K_PAGE             0x10
#define BOOT_SEL_STRAP_NAND_512B_PAGE           0x18

#ifdef CONFIG_SMC_BASED

#define BOOT_SEL_STRAP_SPI_NOR_3BIT             0x18
#define BOOT_SEL_STRAP_SPI_NOR_4BIT             0x1f
#define BOOT_SEL_STRAP_EMMC                     0x1e
#define BOOT_SEL_STRAP_SPI_NAND_3BIT            0x38
#define BOOT_SEL_STRAP_SPI_NAND_4BIT            0x3f

#else

#define BOOT_SEL_STRAP_SPI_NOR                  0x38
#define BOOT_SEL_STRAP_EMMC                     0x30
#define BOOT_SEL_STRAP_SPI_NAND                 0x28
#define BOOT_SEL_STRAP_ETH                      0x20

#endif


/* legacy boot select bits 3-4 */
#define BOOT_SEL_LEGACY_STRAP_NAND_2K_PAGE      0x00
#define BOOT_SEL_LEGACY_STRAP_NAND_4K_PAGE      0x08
#define BOOT_SEL_LEGACY_STRAP_NAND_8K_PAGE      0x10
#define BOOT_SEL_LEGACY_STRAP_SPI_NOR           0x18


struct strap_prop_t {
	const char *name;
	unsigned int bit_num;
	bool bit_en;
};

struct g_strap_t {
	void __iomem *strap_reg;
	uint32_t strap_val;
	ofnode node;
};

static struct g_strap_t g_strap = {
	.strap_reg = NULL,
	.strap_val = 0xFFFFFFFF,
	.node.of_offset = -1,
};

static int bcm_strap_drv_probe(struct udevice *dev)
{
	struct resource res;
	int ret = 0;

	ret = dev_read_resource(dev, 0, &res);
	if (ret) {
		debug("bcm_strap_drv_probe failed to read strap reg resource\n");
		return ret;
	}
	g_strap.strap_reg = ioremap(res.start, resource_size(&res));
	if (IS_ERR(g_strap.strap_reg)) {
		debug("bcm_strap_drv_probe failed to map reg resource\n");
		return -ENXIO;
	}

	g_strap.node = dev_ofnode(dev);
	g_strap.strap_val = readl(g_strap.strap_reg);

	return 0;
}

static ofnode bcm_early_find_strap_node(void)
{
	return ofnode_by_compatible(ofnode_null(), "brcm,strap");
}

static uint32_t bcm_early_strap_get_val(void)
{
	ofnode strap_node;
	void __iomem * reg;
	struct resource res;
	u32 strap;

	strap_node = bcm_early_find_strap_node();
	if (!ofnode_valid(strap_node))
		return 0xFFFFFFFF;
	if (ofnode_read_resource(strap_node, 0, &res))
		return 0xFFFFFFFF;

	reg = ioremap(res.start, resource_size(&res));
	if (IS_ERR_OR_NULL(reg))
		return 0xFFFFFFFF;
	strap = readl(reg);

	iounmap(reg);

	return strap;
}

uint32_t bcm_strap_get_val(void)
{
	if (g_strap.strap_reg)
		return g_strap.strap_val;
	else
		return bcm_early_strap_get_val();
}
EXPORT_SYMBOL(bcm_strap_get_val);

uint32_t bcm_strap_get_field_val(const char* field_name)
{
	uint32_t params[2], strap, mask;
	uint32_t field_start_bit, field_num_bit;
	ofnode node = g_strap.node;

	if (!ofnode_valid(node))
		node = bcm_early_find_strap_node();

	if (ofnode_read_u32_array(node, field_name, params, 2)) {
		/* no strap defined skipping */
		return -ENODEV;
	}

	field_start_bit = params[0];
	field_num_bit = params[1];
	
	if ((field_start_bit + field_num_bit) > 32) {
		printk("The %s property specifies wrong field bits %d %d\n",
			field_name, field_start_bit, field_num_bit);
		return -EINVAL;
	}

	strap = bcm_strap_get_val();
	if (field_num_bit == 32)
		return strap;

	mask = ((1<<field_num_bit) - 1) << field_start_bit;

	return (strap & mask) >> field_start_bit;
	
}
EXPORT_SYMBOL(bcm_strap_get_field_val);

static int bcm_test_bit(uint32_t val, unsigned int shift, bool bit_val)
{
	if (shift > 31)
		return -EINVAL;

	if (((val >> shift) & 1) == bit_val)
		return 1;
	else
		return 0;
}

int bcm_strap_parse_and_test(ofnode np, const char* consumer_name)
{
	uint32_t params[2];
	unsigned int strap_en_bit = 0;
	bool strap_en = 0;
	int ret;
	ofnode node = np;

	if (!ofnode_valid(node))
		node = g_strap.node;

	if (!ofnode_valid(node))
		node = bcm_early_find_strap_node();

	if (ofnode_read_u32_array(node, consumer_name, params, 2)) {
		/* no strap defined skipping */
		return -ENODEV;
	}

	strap_en_bit = params[0];
	strap_en = (bool)params[1];

	ret = bcm_test_bit(bcm_strap_get_val(), strap_en_bit, strap_en);
	if (ret < 0)
	{
		printk("The %s property specifies wrong bit number %d\n", consumer_name, strap_en_bit);
	}

	return ret;
}
EXPORT_SYMBOL(bcm_strap_parse_and_test);


#ifdef CONFIG_SMC_BASED

static int bootsel_to_boot_dev(uint32_t bootsel)
{
	switch(bootsel)
	{
	case BOOT_SEL_STRAP_NAND_2K_PAGE:
	case BOOT_SEL_STRAP_NAND_4K_PAGE:
	case BOOT_SEL_STRAP_NAND_8K_PAGE:
		printk("Boot Device: NAND\n");	  
		return BOOT_DEVICE_NAND;
	case BOOT_SEL_STRAP_SPI_NOR_3BIT:
		printk("Boot Device: SPI NOR\n");
		return BOOT_DEVICE_NOR;
    case BOOT_SEL_STRAP_SPI_NOR_4BIT:
		printk("Boot Device: SPI NOR\n");
		return BOOT_DEVICE_NOR;
	case BOOT_SEL_STRAP_EMMC:
		printk("Boot Device: EMMC\n");
		return BOOT_DEVICE_MMC1;
	case BOOT_SEL_STRAP_SPI_NAND_3BIT:
		printk("Boot Device: SPI NAND\n");
		return BOOT_DEVICE_SPI;
    case BOOT_SEL_STRAP_SPI_NAND_4BIT:
		printk("Boot Device: SPI NAND\n");
		return BOOT_DEVICE_SPI;
	default:
		printk("NOT SUPPORTED BOOT DEVICE!!!!! 0x%x\n", bootsel);
		return -EINVAL;
	}
}

#else

static int bootsel_to_boot_dev(uint32_t bootsel)
{
	switch(bootsel)
	{
	case BOOT_SEL_STRAP_NAND_2K_PAGE:
	case BOOT_SEL_STRAP_NAND_4K_PAGE:
	case BOOT_SEL_STRAP_NAND_8K_PAGE:
	case BOOT_SEL_STRAP_NAND_512B_PAGE:
		debug("Boot Device: NAND\n");	  
		return BOOT_DEVICE_NAND;
	case BOOT_SEL_STRAP_SPI_NOR:
		debug("Boot Device: SPI NOR\n");
		return BOOT_DEVICE_NOR;
	case BOOT_SEL_STRAP_EMMC:
		debug("Boot Device: EMMC\n");
		return BOOT_DEVICE_MMC1;
	case BOOT_SEL_STRAP_SPI_NAND:
		debug("Boot Device: SPI NAND\n");
		return BOOT_DEVICE_SPI;
	case BOOT_SEL_STRAP_ETH:
		debug("Boot Device: Network\n");
		return BOOT_DEVICE_ETH;
	default:
		printk("NOT SUPPORTED BOOT DEVICE!!!!! 0x%x\n", bootsel);
		return -EINVAL;
	}
}

#endif

static int bootsel_legacy_to_boot_dev(uint32_t bootsel)
{
	switch(bootsel)
	{
	case BOOT_SEL_LEGACY_STRAP_NAND_2K_PAGE:
	case BOOT_SEL_LEGACY_STRAP_NAND_4K_PAGE:
	case BOOT_SEL_LEGACY_STRAP_NAND_8K_PAGE:
		debug("Boot Device: NAND\n");	  
		return BOOT_DEVICE_NAND;
	case BOOT_SEL_LEGACY_STRAP_SPI_NOR:
		debug("Boot Device: SPI NOR\n");
		return BOOT_DEVICE_NOR;
	default:
		printk("NOT SUPPORTED BOOT DEVICE!!!!! 0x%x\n", bootsel);
		return -EINVAL;
	}
}

int bcm_get_boot_device(void)
{
	uint32_t bootsel = 0;
	uint32_t bootsel5 = 0;

	bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-3") << 3);
	bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-4") << 4);

#ifdef CONFIG_SMC_BASED
    if( bootsel != BOOT_SEL_STRAP_NAND_2K_PAGE && 
        bootsel != BOOT_SEL_STRAP_NAND_4K_PAGE && 
        bootsel != BOOT_SEL_STRAP_NAND_8K_PAGE)
    {
        bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-0"));
        bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-1") << 1);
        bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-2") << 2);
    }
#endif

	/* legacy device like 63148 does not have bootsel bit5 */
	bootsel5 = bcm_strap_parse_and_test(ofnode_null(), "boot-select-5");
	if (bootsel5 == -ENODEV)
		return bootsel_legacy_to_boot_dev(bootsel);
	else {
		bootsel |= (bootsel5 << 5);
		return bootsel_to_boot_dev(bootsel);
	}
}
EXPORT_SYMBOL(bcm_get_boot_device);

nand_ecc_t bcm_get_nand_ecc(void)
{
	uint32_t bootsel = 0;

	bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-0") << 0);
	bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-1") << 1);
	bootsel |= (bcm_strap_parse_and_test(ofnode_null(), "boot-select-2") << 2);

	switch(bootsel)
	{
	case BOOT_NAND_ECC_DISABLE:
		return ECC_NONE;
	case BOOT_NAND_ECC_1_BIT:
		return ECC_1_BIT;
	case BOOT_NAND_ECC_4_BIT:
		return ECC_4_BIT;
	case BOOT_NAND_ECC_8_BIT:
		return ECC_8_BIT;
	case BOOT_NAND_ECC_12_BIT:
		return ECC_12_BIT;
	case BOOT_NAND_ECC_24_BIT:
		return ECC_24_BIT;
	case BOOT_NAND_ECC_40_BIT:
		return ECC_40_BIT;
	case BOOT_NAND_ECC_60_BIT:
		return ECC_60_BIT;
	default:
		printk("NOT SUPPORTED ECC TYPE!!!!!\n");
		return -EINVAL;
	}
}
EXPORT_SYMBOL(bcm_get_nand_ecc);


int bcm_strap_drv_reg(void)
{
	int ret;
	struct udevice *dev;

	ret = uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bcm_strap_drv), &dev);
	printk("Strap Register: 0x%0x\n", bcm_strap_get_val());

	return ret;
}

static const struct udevice_id bcm_strap_of_match[] = {
	{ .compatible = "brcm,strap"},
	{}
};

U_BOOT_DRIVER(bcm_strap_drv) = {
	.name = "bcm_strap",
	.id = UCLASS_NOP,
	.of_match = bcm_strap_of_match,
	.probe = bcm_strap_drv_probe,
};
