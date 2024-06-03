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

#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>
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

#ifdef CONFIG_BRCM_SMC_BASED

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
    struct device_node *node;
    unsigned long fdt_node;
};

static struct g_strap_t g_strap;

static struct of_device_id const bcm_strap_of_match[] = {
    { .compatible = "brcm,strap", },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_strap_of_match);

static const struct of_device_id* fdt_match_node(unsigned long node, const struct of_device_id * match_table);
static int __init fdt_get_memory_prop(unsigned long node, int index, uint64_t* base, uint64_t* size);


static int bcm_strap_drv_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;

    match = of_match_device(bcm_strap_of_match, dev);
    if (!match)
    {
        dev_err(dev, "Failed to match strap driver\n");
        return -ENODEV;
    }

    g_strap.node = dev->of_node;

    return 0;
}

static __init uint32_t bcm_early_strap_get_val(void)
{
    unsigned long strap_node;
    u64 base, size;
    void __iomem * reg;
    u32 strap;

    strap_node = fdt_node_offset_by_compatible(initial_boot_params, -1, "brcm,strap");
    if ((int)strap_node < 0) {
        printk("Failed to find strap dts node\n");
        return 0xFFFFFFFF;
    }

    if (!fdt_match_node(strap_node, bcm_strap_of_match))
        return 0xFFFFFFFF;
    base = of_flat_dt_translate_address(strap_node);
    if (base == OF_BAD_ADDR) {
        printk("Failed to find strap resources\n");
        return 0xFFFFFFFF;
    }
    if (fdt_get_memory_prop(strap_node, 0, NULL, &size))
        return 0xFFFFFFFF;

    reg = ioremap(base, size);
    if (IS_ERR_OR_NULL(reg))
        return 0xFFFFFFFF;
    strap = *(uint32_t *)reg;

    iounmap(reg);

    return strap;
}

uint32_t bcm_strap_get_val(void)
{
    uint32_t ret_val = 0xFFFFFFFF;
    
    if (g_strap.strap_reg)
        ret_val = g_strap.strap_val;
    else
    {
        printk("Error: Wrong strap value\n");
    }

    return ret_val;
}
EXPORT_SYMBOL(bcm_strap_get_val);

static int bcm_test_bit(uint32_t val, unsigned int shift, bool bit_val)
{
    if (shift > 31)
        return -EINVAL;

    if (((val >> shift) & 1) == bit_val)
        return 1;
    else
        return 0;
}

int __init of_fdt_property_read_u32_array(unsigned long node, const char *propname, u32 *out_val, size_t sz)
{
    const __be32 *reg;
    int regsize;
    int i;

    reg = of_get_flat_dt_prop(node, propname, &regsize);
    if (reg == NULL)
        return -ENODEV;

    if (regsize != sz * sizeof(u32))
        return -EINVAL;

    for (i = 0; i < sz; i++)
        out_val[i] = (u32)dt_mem_next_cell(1, &reg);

    return 0;
}

int bcm_strap_parse_and_test(struct device_node *np, const char* consumer_name)
{
    uint32_t params[2];
    unsigned int strap_en_bit = 0;
    bool strap_en = 0;
    int ret;
    struct device_node *node = np;

    if (!g_strap.strap_reg)
        return -EAGAIN;
    
    if(node == NULL)
    {
        node = g_strap.node;
    }
    
    if (node) 
    {
        if (of_property_read_u32_array(node, consumer_name, params, 2))
        {
            /* no strap defined skipping */
            return -ENODEV;
        }
    }
    else
    {
        return -ENODEV;
    }

    strap_en_bit = params[0];
    strap_en = (bool)params[1];

    ret = bcm_test_bit(g_strap.strap_val, strap_en_bit, strap_en);
    if (ret < 0)
    {
        printk("The %s property specifies wrong bit number %d\n", consumer_name, strap_en_bit);
    }

    return ret;
}
EXPORT_SYMBOL(bcm_strap_parse_and_test);

int __init early_bcm_strap_parse_and_test(struct device_node *np, const char* consumer_name)
{
    uint32_t params[2];
    unsigned int strap_en_bit = 0;
    bool strap_en = 0;
    int ret;

    if (of_fdt_property_read_u32_array(g_strap.fdt_node, consumer_name, params, 2))
    {
        /* no strap defined skipping */
        return -ENODEV;
    }

    strap_en_bit = params[0];
    strap_en = (bool)params[1];

    ret = bcm_test_bit(g_strap.strap_val, strap_en_bit, strap_en);
    if (ret < 0)
    {
        printk("The %s property specifies wrong bit number %d\n", consumer_name, strap_en_bit);
    }

    return ret;
}

uint32_t bcm_strap_get_field_val(const char* field_name)
{
    uint32_t params[2], strap, mask;
    uint32_t field_start_bit, field_num_bit;
    struct device_node *node  = g_strap.node;

    if (!g_strap.strap_reg)
        return -EAGAIN;
    
    if (node) 
    {
        if (of_property_read_u32_array(node, field_name, params, 2))
        {
            /* no strap defined skipping */
            return -ENODEV;
        }
    }
    else
    {
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

#ifdef CONFIG_BRCM_SMC_BASED

static int bootsel_to_boot_dev(uint32_t bootsel)
{
	switch(bootsel)
	{
	case BOOT_SEL_STRAP_NAND_2K_PAGE:
		printk(KERN_DEBUG "Boot Device: NAND 2K Page\n");
		return NAND_2K;
	case BOOT_SEL_STRAP_NAND_4K_PAGE:
		printk(KERN_DEBUG "Boot Device: NAND 4K Page\n");
		return NAND_4K;
	case BOOT_SEL_STRAP_NAND_8K_PAGE:
		printk(KERN_DEBUG "Boot Device: NAND 8K Page\n");
		return NAND_8K;
	case BOOT_SEL_STRAP_SPI_NOR_3BIT:
		printk(KERN_DEBUG "Boot Device: SPI NOR\n");
		return SPI_NOR;
    case BOOT_SEL_STRAP_SPI_NOR_4BIT:
		printk(KERN_DEBUG "Boot Device: SPI NOR\n");
		return SPI_NOR;
	case BOOT_SEL_STRAP_EMMC:
		printk(KERN_DEBUG "Boot Device: EMMC\n");
		return EMMC;
	case BOOT_SEL_STRAP_SPI_NAND_3BIT:
		printk(KERN_DEBUG "Boot Device: SPI NAND\n");
		return SPI_NAND;
    case BOOT_SEL_STRAP_SPI_NAND_4BIT:
		printk(KERN_DEBUG "Boot Device: SPI NAND\n");
		return SPI_NAND;
	default:
		printk("NOT SUPPORTED BOOT DEVICE!!!!! 0x%x\n", bootsel);
		return -EINVAL;
	}
}

#else

static boot_dev_t bootsel_to_boot_dev(uint32_t bootsel)
{
    switch(bootsel)
    {
    case BOOT_SEL_STRAP_NAND_2K_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 2K Page\n");
        return NAND_2K;
    case BOOT_SEL_STRAP_NAND_4K_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 4K Page\n");
        return NAND_4K;
    case BOOT_SEL_STRAP_NAND_8K_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 8K Page\n");
        return NAND_8K;
    case BOOT_SEL_STRAP_NAND_512B_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 512B Page\n");
        return NAND_512B;
    case BOOT_SEL_STRAP_SPI_NOR:
        printk(KERN_DEBUG "Boot Device: SPI NOR\n");
        return SPI_NOR;
    case BOOT_SEL_STRAP_EMMC:
        printk(KERN_DEBUG "Boot Device: EMMC\n");
        return EMMC;
    case BOOT_SEL_STRAP_SPI_NAND:
        printk(KERN_DEBUG "Boot Device: SPI NAND\n");
        return SPI_NAND;
    case BOOT_SEL_STRAP_ETH:
        printk(KERN_DEBUG "Boot Device: Network\n");
        return ETHERNET;
    default:
        printk("NOT SUPPORTED BOOT DEVICE!!!!! 0x%x\n", bootsel);
        BUG();
        return -EINVAL;
    }
}

#endif

static boot_dev_t bootsel_legacy_to_boot_dev(uint32_t bootsel)
{
    switch(bootsel)
    {
    case BOOT_SEL_LEGACY_STRAP_NAND_2K_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 2K Page\n");
        return NAND_2K;
    case BOOT_SEL_LEGACY_STRAP_NAND_4K_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 4K Page\n");
        return NAND_4K;
    case BOOT_SEL_LEGACY_STRAP_NAND_8K_PAGE:
        printk(KERN_DEBUG "Boot Device: NAND 8K Page\n");
        return NAND_8K;
    case BOOT_SEL_LEGACY_STRAP_SPI_NOR:
        printk(KERN_DEBUG "Boot Device: SPI NOR\n");
        return SPI_NOR;
    default:
        printk("NOT SUPPORTED BOOT DEVICE!!!!! 0x%x\n", bootsel);
        BUG();
        return -EINVAL;
    }
}

static uint32_t bootsel = 0;
static uint32_t bootsel5 = 0;
static uint32_t nand_ecc_bootsel = 0;

void __init init_boot_device(void)
{

    bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-3") << 3);
    bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-4") << 4);

#ifdef CONFIG_BRCM_SMC_BASED

    if( bootsel != BOOT_SEL_STRAP_NAND_2K_PAGE && 
        bootsel != BOOT_SEL_STRAP_NAND_4K_PAGE && 
        bootsel != BOOT_SEL_STRAP_NAND_8K_PAGE)
    {
        bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-0"));
        bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-1") << 1);
        bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-2") << 2);
    }

#endif
    bootsel5 = early_bcm_strap_parse_and_test(NULL, "boot-select-5");

    printk(KERN_DEBUG "init_boot_device %x %x\n", bootsel, bootsel5);
}


boot_dev_t bcm_get_boot_device(void)
{

    /* legacy device like 63148 does not have bootsel bit5 */
    if (bootsel5 == -ENODEV)
        return bootsel_legacy_to_boot_dev(bootsel);
    else {
        bootsel |= (bootsel5 << 5);
        return bootsel_to_boot_dev(bootsel);
    }
}
EXPORT_SYMBOL(bcm_get_boot_device);


void __init init_nand_ecc(void)
{
    nand_ecc_bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-0") << 0);
    nand_ecc_bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-1") << 1);
    nand_ecc_bootsel |= (early_bcm_strap_parse_and_test(NULL, "boot-select-2") << 2);
}

nand_ecc_t  bcm_get_nand_ecc(void)
{
    switch(nand_ecc_bootsel)
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
        BUG();
        return -EINVAL;
    }
}
EXPORT_SYMBOL(bcm_get_nand_ecc);

static const struct of_device_id* fdt_match_node(unsigned long node, const struct of_device_id * match_table)
{
    if (!match_table)
        return NULL;

    while (match_table->compatible)
    {
        if (of_flat_dt_is_compatible(node, match_table->compatible))
            return match_table;
        match_table++;
    }

    return NULL;
}

static int __init fdt_get_memory_prop(unsigned long node, int index, uint64_t* base, uint64_t* size)
{
    const __be32 *endp;
    const __be32 *reg;
    int regsize;
    int idx = 0;
    uint64_t value;

    reg = of_get_flat_dt_prop(node, "reg", &regsize);
    if (reg == NULL)
        return -ENODEV;
    endp = reg + (regsize / sizeof(__be32));
    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells))
    {
        value = dt_mem_next_cell(dt_root_addr_cells, &reg);
        if (base)
            *base = value;
        value = dt_mem_next_cell(dt_root_size_cells, &reg);
        if (size)
            *size = value;
        if (idx == index)
            return 0;

        idx++;
    }

    return -EINVAL;
}

int __init bcm_strap_early_scan_dt(unsigned long node, const char *uname, int depth, void *data)
{
    const struct of_device_id *match;
    int ret = 0;
    uint64_t base, size;

    if (strncmp(uname, "strap", 5) != 0)
        goto exit;

    match = fdt_match_node(node, bcm_strap_of_match);
    if (!match)
    {
        printk("%s not match Strap\n", uname);
        goto exit;
    }

    g_strap.fdt_node = node;
    base = of_flat_dt_translate_address(node);
    if (base == OF_BAD_ADDR)
    {
        printk("Failed to find strap resources\n");
        ret = -ENXIO;
        goto exit;
    }
    ret = fdt_get_memory_prop(node, 0, NULL, &size);
    if (ret)
    {
        printk("Failed to find strap resource\n");
        goto exit;
    }

    g_strap.strap_reg = ioremap(base, size);
    if (IS_ERR_OR_NULL(g_strap.strap_reg)) 
    {
        printk("Failed to map the strap resource\n");
        ret = -ENXIO;
    }
    
    /* The strap register could not be change in the run time.
     So there is no need to read it every time. */
    g_strap.strap_val = *(uint32_t *)(g_strap.strap_reg);

    init_boot_device();
    init_nand_ecc();

exit:
    return ret;
}

static struct platform_driver bcm_strap_driver = {
    .driver = {
        .name = "bcm-strap-drv",
        .of_match_table = bcm_strap_of_match,
    },
    .probe = bcm_strap_drv_probe,
};

int __init init_strap_register(void)
{

    return of_scan_flat_dt(bcm_strap_early_scan_dt, NULL);

}

static int __init bcm_strap_drv_reg(void)
{
    int ret;

    printk(KERN_DEBUG "Strap driver initcall\n");

    ret=init_strap_register();
    if (ret) 
        goto error;

    printk("Strap Register: 0x%0x\n", bcm_early_strap_get_val());
    return platform_driver_register(&bcm_strap_driver);

error:
    BUG();

    return ret;
}

core_initcall_sync(bcm_strap_drv_reg);

MODULE_DESCRIPTION("Broadcom BCA Strap Driver");
MODULE_LICENSE("GPL v2");
