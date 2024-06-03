/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (c) 2019 Broadcom Ltd.
 */
/*
 *
 */

/*****************************************************************************
 *  Description:
 *      Code for PMC Linux device tree parsing
 *****************************************************************************/

#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "asm/arch/misc.h"
#include <asm/u-boot.h>
#include <asm/global_data.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm.h>

struct g_pmc_t pmc;

struct g_pmc_t *g_pmc = &pmc;
#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD) || defined(CONFIG_BCMBCA_IKOS)
#define pmc_printk(...) do {} while (0)
#else
#define pmc_printk printk
#endif

static void pmc_1_x_unmap(struct g_pmc_t *g_pmc)
{
    if (!IS_ERR_OR_NULL(g_pmc->pmc_base))
        iounmap(g_pmc->pmc_base);

    if (!IS_ERR_OR_NULL(g_pmc->procmon_base))
        iounmap(g_pmc->procmon_base);
}

static int dt_parse_1_x(struct udevice *dev)
{
    int ret;
    struct resource res;

    g_pmc->unmap = pmc_1_x_unmap;

    ret = dev_read_resource_byname(dev, "pmc", &res);
    if (ret)
    {
        pmc_printk("Failed to find pmc resource\n");
        return ret;
    }

    g_pmc->pmc_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->pmc_base)) 
    {
        pmc_printk("Failed to map the pmc resource\n");
        return -ENXIO;
    }

    pmc_printk("PMC  0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->pmc_base, (uint32_t)resource_size(&res));
    ret = dev_read_resource_byname(dev, "procmon", &res);
    if (ret)
    {
        pmc_printk("Failed to find procmon resource\n");
        return ret;
    }

    g_pmc->procmon_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->procmon_base)) 
    {
        pmc_printk("Failed to map the procmon resource\n");
        return -ENXIO;
    }
    pmc_printk("PRCM 0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->procmon_base, (uint32_t)resource_size(&res));

    g_pmc->strap = MISC->miscStrapBus;
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
#if defined(PMC_FW_IN_ITCM)
    if (!IS_ERR(g_pmc->itcm_base))
        iounmap(g_pmc->itcm_base);
#endif
}

static int dt_parse_3_x(struct udevice *dev)
{
    int ret;
    struct resource res;

    g_pmc->unmap = pmc_3_x_unmap;

    ret = dev_read_resource_byname(dev, "pmc", &res);
    if (ret)
    {
        pmc_printk("Failed to find pmc resource\n");
        return ret;
    }

    g_pmc->pmc_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->pmc_base)) 
    {
        pmc_printk("Failed to map the pmc resource\n");
        return -ENXIO;
    }
    pmc_printk("PMC  0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->pmc_base, (uint32_t)resource_size(&res));

    ret = dev_read_resource_byname(dev, "procmon", &res);
    if (ret)
    {
        pmc_printk("Failed to find procmon resource\n");
        return ret;
    }

    g_pmc->procmon_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->procmon_base)) 
    {
        pmc_printk("Failed to map the procmon resource\n");
        return -ENXIO;
    }
    pmc_printk("PRCM 0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->procmon_base, (uint32_t)resource_size(&res));

    ret = dev_read_resource_byname(dev, "maestro", &res);
    if (ret)
    {
        pmc_printk("Failed to find maestro resource\n");
        return ret;
    }

    g_pmc->maestro_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->maestro_base))
    {
        pmc_printk("Failed to map the maestro resource\n");
        return -ENXIO;
    }
    pmc_printk("MAES 0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->maestro_base, (uint32_t)resource_size(&res));

#if defined(PMC_LOG_IN_DTCM)
    ret = dev_read_resource_byname(dev, "dtcm", &res);
    if (ret)
    {
        pmc_printk("Failed to find dtcm resource\n");
        return ret;
    }

    g_pmc->dtcm_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->dtcm_base)) 
    {
        pmc_printk("Failed to map the dtcm resource\n");
        return -ENXIO;
    }
    pmc_printk("DTCM 0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->dtcm_base, (uint32_t)resource_size(&res));
#endif
#if defined(PMC_FW_IN_ITCM)
    ret = dev_read_resource_byname(dev, "itcm", &res);
    if (ret)
    {
        pmc_printk("Failed to find dtcm resource\n");
        return ret;
    }

    g_pmc->itcm_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->itcm_base)) 
    {
        pmc_printk("Failed to map the itcm resource\n");
        return -ENXIO;
    }

    g_pmc->itcm_size = resource_size(&res);
    pmc_printk("ITCM 0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->itcm_base, (uint32_t)resource_size(&res));
#endif

    return 0;
}

static void pmc_lite_unmap(struct g_pmc_t *g_pmc)
{
    if (!IS_ERR_OR_NULL(g_pmc->pmc_base))
        iounmap(g_pmc->pmc_base);
}

static int dt_parse_lite(struct udevice *dev)
{
    int ret;
    struct resource res;

    g_pmc->unmap = pmc_lite_unmap;

    ret = dev_read_resource_byname(dev, "pmc", &res);
    if (ret)
    {
        pmc_printk("Failed to find pmc resource\n");
        return ret;
    }

    g_pmc->pmc_base = ioremap(res.start, resource_size(&res));
    if (IS_ERR(g_pmc->pmc_base)) 
    {
        pmc_printk("Failed to map the pmc resource\n");
        return -ENXIO;
    }
    pmc_printk("PMC  0x%x 0x%p 0x%x\n", (uint32_t)res.start, g_pmc->pmc_base, (uint32_t)resource_size(&res));

    return 0;
}

static const struct udevice_id bcm_pmc_1_x_of_match[] = {
    { .compatible = "brcm,bca-pmc-1-x"},
    {}
};

static const struct udevice_id bcm_pmc_3_1_of_match[] = {
    { .compatible = "brcm,bca-pmc-3-1"},
    {}
};

static const struct udevice_id bcm_pmc_3_2_of_match[] = {
    { .compatible = "brcm,bca-pmc-3-2"},
    {}
};

static const struct udevice_id bcm_pmc_lite_of_match[] = {
    { .compatible = "brcm,bca-pmc-lite"},
    {}
};

U_BOOT_DRIVER(bcm_pmc_1_x_drv) = {
        .name = "bcm_pmc_1_x",
        .id = UCLASS_NOP,
        .of_match = bcm_pmc_1_x_of_match,
        .probe = dt_parse_1_x,
};

U_BOOT_DRIVER(bcm_pmc_3_1_drv) = {
        .name = "bcm_pmc_3_1",
        .id = UCLASS_NOP,
        .of_match = bcm_pmc_3_1_of_match,
        .probe = dt_parse_3_x,
};

U_BOOT_DRIVER(bcm_pmc_3_2_drv) = {
        .name = "bcm_pmc_3_2",
        .id = UCLASS_NOP,
        .of_match = bcm_pmc_3_2_of_match,
        .probe = dt_parse_3_x,
};

U_BOOT_DRIVER(bcm_pmc_lite_drv) = {
        .name = "bcm_pmc_lite",
        .id = UCLASS_NOP,
        .of_match = bcm_pmc_lite_of_match,
        .probe = dt_parse_lite,
};

void pmc_init(void);

int bcm_pmc_drv_reg(void)
{
    int ret;
    struct udevice *dev;

    printk("PMC driver initcall\n");

    pmc_printk("PMC driver scanning DT\n");

    pmc_printk("     Remapping PMC IO memories...\n");
    pmc_printk("name             phys              virt          size\n");
    ret = uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bcm_pmc_1_x_drv), &dev);
    ret = (ret == 0) ? ret : uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bcm_pmc_3_1_drv), &dev);
    ret = (ret == 0) ? ret : uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bcm_pmc_3_2_drv), &dev);
    ret = (ret == 0) ? ret : uclass_get_device_by_driver(UCLASS_NOP, DM_GET_DRIVER(bcm_pmc_lite_drv), &dev);

    if (ret)
        goto error;

    return 0;

error:

    BUG();

    if (g_pmc->unmap)
        g_pmc->unmap(g_pmc);

    return ret;
}

