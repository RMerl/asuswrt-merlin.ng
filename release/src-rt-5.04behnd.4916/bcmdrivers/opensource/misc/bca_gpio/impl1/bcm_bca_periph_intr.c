/*
 * <:copyright-BRCM:2024:DUAL/GPL:standard
 * 
 *    Copyright (c) 2024 Broadcom 
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
#if defined(PERIPH_INTR)
#include <linux/io.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/version.h>

#define MAX_INT_SETS         4

#define INT_SET_IRQ_SENSE_OFFSET       0x0
#define INT_SET_IRQ_MASK0_OFFSET       0x10
#define INT_SET_IRQ_STATUS0_OFFSET     0x50
#define INT_SET_IRQ_MASKX_OFFSET(x)    (INT_SET_IRQ_MASK0_OFFSET + (4*(sizeof(uint32_t))*(x)))
#define INT_SET_IRQ_STATUSX_OFFSET(x)  (INT_SET_IRQ_STATUS0_OFFSET + (4*(sizeof(uint32_t))*(x)))
#define INT_SET_IRQ_INTERNAL_OFFSET(x) (x * sizeof(uint32_t))

struct bcm_bca_periph_intr {
    void __iomem *intset_base[MAX_INT_SETS];
};

static struct bcm_bca_periph_intr *bca_ext;

int bcm_bca_periph_intr_init(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct resource *res;
    char resname[16];
    int ret = 0, i;

    bca_ext = devm_kzalloc(dev, sizeof(*bca_ext), GFP_KERNEL);
    if (!bca_ext) {
        ret = -ENOMEM;
        goto error;
    }

    for (i = 0; i < MAX_INT_SETS; i++) {
        snprintf(resname, 16, "intset%d", i);
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM, resname);
        if (!res)
            break;

        bca_ext->intset_base[i] = devm_ioremap_resource(dev, res);
        if (IS_ERR(bca_ext->intset_base[i])) {
            ret = -ENXIO;
            break;
        }

        dev_info(dev, "%s %pr va 0x%px\n",
            res->name, res, bca_ext->intset_base[i]);
    }

error:
    return ret;
}

int periph_intr_sense_read(unsigned int hw_irq)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_SENSE_OFFSET + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    if (data & (1 << int_shift))
        return 1;
    else
        return 0;
}

int periph_intr_sense_set(unsigned int hw_irq, bool is_active_high)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_SENSE_OFFSET + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    data |= (1 << int_shift);
    if (is_active_high)
        data &= ~(1 << int_shift);

    writel(data, reg + INT_SET_IRQ_SENSE_OFFSET + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    return 0;
}

int periph_intr_mask_read(unsigned int mask_num, unsigned int hw_irq)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_MASKX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    if (data & (1 << int_shift))
        return 1;
    else
        return 0;
}

int periph_intr_mask_set(unsigned int mask_num, unsigned int hw_irq, bool do_set)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_MASKX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    data &= ~(1 << int_shift);
    if (do_set)
        data |= (1 << int_shift);

    writel(data, reg + INT_SET_IRQ_MASKX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    return 0;
}

int periph_intr_status_read(unsigned int mask_num, unsigned int hw_irq)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_STATUSX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    if (data & (1 << int_shift))
        return 1;
    else
        return 0;
}

int periph_intr_status_set(unsigned int mask_num, unsigned int hw_irq, bool do_set)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_STATUSX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    data &= ~(1 << int_shift);
    if (do_set)
        data |= (1 << int_shift);

    writel(data, reg + INT_SET_IRQ_STATUSX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    return 0;
}

EXPORT_SYMBOL(periph_intr_sense_read);
EXPORT_SYMBOL(periph_intr_sense_set);
EXPORT_SYMBOL(periph_intr_mask_read);
EXPORT_SYMBOL(periph_intr_mask_set);
EXPORT_SYMBOL(periph_intr_status_read);
EXPORT_SYMBOL(periph_intr_status_set);
#endif