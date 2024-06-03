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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>
#include "bcm_ubus4.h"
#include "bcm_ubus_internal.h"

#ifdef  CONFIG_BRCM_IKOS
#define UBUS_RESOURCE_PRINT(fmt, ...)
#else
#define UBUS_RESOURCE_PRINT(fmt, ...) printk(fmt, ##__VA_ARGS__)
#endif

ubus_t *ubus_sys = NULL;
ubus_t *ubus_xrdp = NULL;

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

static int __init bcm_ubus_parse_resource(unsigned long node, ubus_t *ubus)
{
    int ret;
    uint64_t base, size;

    ret = fdt_get_resource_byname(node, "ubus_systop", &base, &size);
    if (ret) {
        printk("Failed to find ubus_systop resource\n");
        return ret;
    }
    ubus->systop = ioremap(base, size);
    if (IS_ERR(ubus->systop)) {
        printk("Failed to map the ubus_systop resource\n");
        return -ENXIO;
    }
    UBUS_RESOURCE_PRINT("ubus systop  \t\t phys addr %pap virt addr 0x%px\n", (phys_addr_t*)&base, ubus->systop);

    ret = fdt_get_resource_byname(node, "ubus_registration", &base, &size);
    if (ret) {
        printk("Failed to find ubus_registration resource\n");
        return ret;
    }
    ubus->registration = ioremap(base, size);
    if (IS_ERR(ubus->registration)) {
        printk("Failed to map the ubus_registration resource\n");
        return -ENXIO;
    }
    UBUS_RESOURCE_PRINT("ubus registration  \t phys addr %pap virt addr 0x%px\n", (phys_addr_t*)&base, ubus->registration);

    /* only the main ubus port has this resource */
    ret = fdt_get_resource_byname(node, "ubus_coherency_port", &base, &size);
    if (!ret) {
        ubus->cohport = ioremap(base, size);
        if (IS_ERR(ubus->cohport)) {
            printk("Failed to map the ubus_coherency_port resource\n");
            return -ENXIO;
        }
        UBUS_RESOURCE_PRINT("ubus coherency \t\t phys addr %pap virt addr 0x%px\n", (phys_addr_t*)&base, ubus->cohport);
    }

    if (ret == -ENODEV)
        ret = 0;
    return ret;
}

static int __init bcm_ubus_parse_mst_node(unsigned long node)
{
    unsigned long mst_node;
    int i = 0, ret = 0;
    uint64_t base, size;
    ub_mst_addr_map_t* mst;

    mst_node = of_get_flat_dt_subnode_by_name(node, "ubus_mst");
    if (mst_node == -FDT_ERR_NOTFOUND) {
        printk("missing ubus_mst node under ubus\n");
        return -ENODEV;
    }

    while (ub_mst_addr_map_tbl[i].port_id != -1) {
        mst = &ub_mst_addr_map_tbl[i];
        ret = fdt_get_resource_byname(mst_node, mst->str, &base, &size);
        if (!ret) {
            mst->phys_base = (phys_addr_t)base;
            mst->base = ioremap(base, size);
            if (IS_ERR(mst->base)) {
                printk("Failed to map the mst node %s resource\n", mst->str);
                mst->base = NULL;
                return -ENXIO;
            }
            UBUS_RESOURCE_PRINT("ubus mst node %s\t phys addr %pap virt addr 0x%px\n",
                   mst->str, &mst->phys_base, mst->base);
        }
        i++;
    }

    return ret;
}

static void bcm_ubus_free_mst_node(void)
{
    int i = 0;

    while (ub_mst_addr_map_tbl[i].port_id != -1) {
        if (!IS_ERR_OR_NULL(ub_mst_addr_map_tbl[i].base))
            iounmap(ub_mst_addr_map_tbl[i].base);
    }
}

static void bcu_ubus_free_resource(ubus_t *ubus)
{
    if (ubus) {
        if (!IS_ERR_OR_NULL(ubus->systop))
            iounmap(ubus->systop);
        if (!IS_ERR_OR_NULL(ubus->registration))
            iounmap(ubus->registration);
        if (!IS_ERR_OR_NULL(ubus->cohport))
            iounmap(ubus->cohport);
        kfree(ubus);
    }
}


int __init bcm_ubus_early_scan_dt(unsigned long node, const char *uname, int depth, void *data)
{
    ubus_t *ubus;
    int ret = 0;
    const __be32 *flags;

    if (strncmp(uname, "ubus_sys", 8) != 0 && strncmp(uname, "ubus_xrdp", 9) != 0)
        return 0;

    if (!of_flat_dt_is_compatible(node, "brcm,bca-ubus4")) {
        printk("%s not match ubus compat id\n", uname);
        return 0;
    }

    flags = of_get_flat_dt_prop(node, "flags", NULL);
    if (flags && (be32_to_cpu(*flags)&UBUS_FLAGS_MODULE_XRDP)) {
        /* optional xrdp ubus node exists, allocate the structure first */
        ubus_xrdp = kzalloc(sizeof(*ubus_xrdp), GFP_KERNEL);
        if (!ubus_xrdp) {
            printk("UBUS failed to allocate memory \n");
            return -ENOMEM;
        } else
            ubus = ubus_xrdp;
    } else
        ubus = ubus_sys;

    if ((ret = bcm_ubus_parse_resource(node, ubus)) != 0)
        return ret;

    bcm_ubus_parse_mst_node(node);

    return ret;
}

static int __init bcm_ubus_drv_init(void)
{
    int ret;

    printk("BCM UBUS Driver initcall\n");

    ubus_sys = kzalloc(sizeof(*ubus_sys), GFP_KERNEL);
    if (!ubus_sys) {
        printk("UBUS failed to allocate memory \n");
        ret = -ENOMEM;
        goto error;
    }

    ret = of_scan_flat_dt(bcm_ubus_early_scan_dt, NULL);
    if (ret)
        goto error;

    bcm_ubus_config();

    return 0;

error:
    bcm_ubus_free_mst_node();
    bcu_ubus_free_resource(ubus_sys);
    bcu_ubus_free_resource(ubus_xrdp);
    return ret;
}

postcore_initcall(bcm_ubus_drv_init);

MODULE_DESCRIPTION("Broadcom BCA UBUS Driver");
MODULE_LICENSE("GPL v2");
