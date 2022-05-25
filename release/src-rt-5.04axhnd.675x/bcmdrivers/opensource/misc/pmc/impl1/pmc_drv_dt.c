/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
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
 *      Code for PMC Linux device tree parsing
 *****************************************************************************/

#include "pmc_drv.h"
#include "BPCM.h"
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>

struct g_pmc_t *g_pmc = NULL;

static int __init fdt_get_memory_prop(unsigned long node, int index, uint64_t* base, uint64_t* size)
{
    const __be32 *endp;
    const __be32 *reg;
    int regsize;
    int idx = 0;

    reg = of_get_flat_dt_prop(node, "reg", &regsize);
    if (reg == NULL)
        return 0;
    endp = reg + (regsize / sizeof(__be32));
    while ((endp - reg) >= (dt_root_addr_cells + dt_root_size_cells))
    {
        *base = dt_mem_next_cell(dt_root_addr_cells, &reg);
        *size = dt_mem_next_cell(dt_root_size_cells, &reg);
        if (idx == index)
            return 1;

        idx++;
    }

    return 0;
}

static int __init fdt_get_resource_byname(unsigned long node, const char *name, uint64_t *base, uint64_t *size)
{
    const char *resource_name;
    int ret_size = 0;
    int idx = 0;
    int found = 0;

    resource_name = of_get_flat_dt_prop(node, "reg-names", &ret_size);
    while (ret_size > 0)
    {
        if (strncmp(resource_name, name, ret_size) == 0)
        {
            found = 1;
            break;
        }
        idx++;
        resource_name += (strlen(resource_name) + 1);
        ret_size -= (strlen(resource_name) + 1);
    }
    if (!found)
        return -ENODEV;

    found = fdt_get_memory_prop(node, idx, base, size);
    if (!found)
        return -EINVAL;

    return 0;
}

static void pmc_1_x_unmap(struct g_pmc_t *g_pmc)
{
    if (!IS_ERR_OR_NULL(g_pmc->pmc_base))
        iounmap(g_pmc->pmc_base);

    if (!IS_ERR_OR_NULL(g_pmc->procmon_base))
        iounmap(g_pmc->procmon_base);

    if (!IS_ERR_OR_NULL(g_pmc->strap))
        iounmap(g_pmc->strap);
}

static int dt_parse_1_x(unsigned long node, struct g_pmc_t *g_pmc)
{
    int ret;
    uint64_t base, size;

    g_pmc->unmap = pmc_1_x_unmap;

    ret = fdt_get_resource_byname(node, "pmc", &base, &size);
    if (ret)
    {
        printk("Failed to find pmc resource\n");
        return ret;
    }

    g_pmc->pmc_base = ioremap(base, size);
    if (IS_ERR(g_pmc->pmc_base)) 
    {
        printk("Failed to map the pmc resource\n");
        return -ENXIO;
    }

    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->pmc_base, size);
    ret = fdt_get_resource_byname(node, "procmon", &base, &size);
    if (ret)
    {
        printk("Failed to find procmon resource\n");
        return ret;
    }

    g_pmc->procmon_base = ioremap(base, size);
    if (IS_ERR(g_pmc->procmon_base)) 
    {
        printk("Failed to map the procmon resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->procmon_base, size);

#if defined(MISC_STRAP_BUS_PMC_ROM_BOOT)
    ret = fdt_get_resource_byname(node, "strap", &base, &size);
    if (ret)
    {
        printk("Failed to find starp resource\n");
        return ret;
    }

    g_pmc->strap = ioremap(base, size);
    if (IS_ERR(g_pmc->strap)) 
    {
        printk("Failed to map the strap resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->strap, size);
#endif
    return 0;
}

static void pmc_3_x_unmap(struct g_pmc_t *g_pmc)
{
    if (!IS_ERR_OR_NULL(g_pmc->pmc_base))
        iounmap(g_pmc->pmc_base);

    if (!IS_ERR_OR_NULL(g_pmc->procmon_base))
        iounmap(g_pmc->procmon_base);

    if (!IS_ERR_OR_NULL(g_pmc->maestro_base))
        iounmap(g_pmc->maestro_base);

#if defined(PMC_LOG_IN_DTCM)
    if (!IS_ERR(g_pmc->dtcm_base))
        iounmap(g_pmc->dtcm_base);
#endif
}

static int dt_parse_3_x(unsigned long node, struct g_pmc_t *g_pmc)
{
    int ret;
    uint64_t base, size;

    g_pmc->unmap = pmc_3_x_unmap;

    ret = fdt_get_resource_byname(node, "pmc", &base, &size);
    if (ret)
    {
        printk("Failed to find pmc resource\n");
        return ret;
    }

    g_pmc->pmc_base = ioremap(base, size);
    if (IS_ERR(g_pmc->pmc_base)) 
    {
        printk("Failed to map the pmc resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->pmc_base, size);

    ret = fdt_get_resource_byname(node, "procmon", &base, &size);
    if (ret)
    {
        printk("Failed to find procmon resource\n");
        return ret;
    }

    g_pmc->procmon_base = ioremap(base, size);
    if (IS_ERR(g_pmc->procmon_base)) 
    {
        printk("Failed to map the procmon resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->procmon_base, size);

    ret = fdt_get_resource_byname(node, "maestro", &base, &size);
    if (ret)
    {
        printk("Failed to find maestro resource\n");
        return ret;
    }

    g_pmc->maestro_base = ioremap(base, size);
    if (IS_ERR(g_pmc->maestro_base))
    {
        printk("Failed to map the maestro resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->maestro_base, size);

#if defined(PMC_LOG_IN_DTCM)
    ret = fdt_get_resource_byname(node, "dtcm", &base, &size);
    if (ret)
    {
        printk("Failed to find dtcm resource\n");
        return ret;
    }

    g_pmc->dtcm_base = ioremap(base, size);
    if (IS_ERR(g_pmc->dtcm_base)) 
    {
        printk("Failed to map the dtcm resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->dtcm_base, size);
#endif
    return 0;
}

static void pmc_lite_unmap(struct g_pmc_t *g_pmc)
{
    if (!IS_ERR_OR_NULL(g_pmc->pmc_base))
        iounmap(g_pmc->pmc_base);
}

static int dt_parse_lite(unsigned long node, struct g_pmc_t *g_pmc)
{
    int ret;
    uint64_t base, size;

    g_pmc->unmap = pmc_lite_unmap;

    ret = fdt_get_resource_byname(node, "pmc", &base, &size);
    if (ret)
    {
        printk("Failed to find pmc resource\n");
        return ret;
    }

    g_pmc->pmc_base = ioremap(base, size);
    if (IS_ERR(g_pmc->pmc_base)) 
    {
        printk("Failed to map the pmc resource\n");
        return -ENXIO;
    }
    printk("     0x%016llx 0x%016llx 0x%08llx\n", base, (uint64_t)(uintptr_t)g_pmc->pmc_base, size);

    return 0;
}

static struct of_device_id const bcm_pmc_of_match[] = {
    { .compatible = "brcm,bca-pmc-1-x" , .data = (void*)dt_parse_1_x, },
    { .compatible = "brcm,bca-pmc-3-1", .data = (void*)dt_parse_3_x, },
    { .compatible = "brcm,bca-pmc-3-2", .data = (void*)dt_parse_3_x, },
    { .compatible = "brcm,bca-pmc-lite", .data = (void*)dt_parse_lite, },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_pmc_of_match);

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

int __init bcm_pmc_early_scan_dt(unsigned long node, const char *uname, int depth, void *data)
{
    const struct of_device_id *match;
    int(*parse_func)(unsigned long, struct g_pmc_t *);

    if (strncmp(uname, "pmc", 3) != 0)
        return 0;

    match = fdt_match_node(node, bcm_pmc_of_match);
    if (!match)
    {
        printk("%s not match PMC\n", uname);
        return 0;
    }
    else 
    {
        printk("%s match PMC %s\n", uname, match->compatible);
    }

    printk("     Remapping PMC IO memories...\n");
    printk("             phys              virt          size\n");
    parse_func = (int(*)(unsigned long, struct g_pmc_t *))match->data;
    return parse_func(node, g_pmc);
}

static int __init bcm_pmc_drv_reg(void)
{
    int ret;

    printk("PMC driver initcall\n");
    g_pmc = kzalloc(sizeof(*g_pmc), GFP_KERNEL);
    if (!g_pmc)
    {
        printk("PMC driver: Failed to allocate memory \n");
        ret = -ENOMEM;
        goto error;
    }

    printk("PMC driver scanning DT\n");
    ret = of_scan_flat_dt(bcm_pmc_early_scan_dt, NULL);
    if (ret) 
        goto error;

#if defined(PMC_CPUTEMP_SUPPORT)
    g_pmc->bac_cpu_base = ioremap(BAC_CPU_REG_OFFSET, BAC_CPU_REG_SIZE);
    if (IS_ERR(g_pmc->bac_cpu_base)) 
    {
        printk("Failed to map the BIU BAC resource\n");
        ret = IS_ERR(g_pmc->bac_cpu_base);
        g_pmc->bac_cpu_base = NULL;
        goto error; 
    }
#endif

    pmc_init();

    return 0;

error:

    BUG();

    if (g_pmc)
    {
        if (g_pmc->unmap)
            g_pmc->unmap(g_pmc);
#if defined(PMC_CPUTEMP_SUPPORT)
        if (g_pmc->bac_cpu_base)
            iounmap(g_pmc->bac_cpu_base); 
#endif
        kfree(g_pmc);
    }

    return ret;
}

postcore_initcall(bcm_pmc_drv_reg);

MODULE_DESCRIPTION("Broadcom BCA PMC Driver");
MODULE_LICENSE("GPL v2");
