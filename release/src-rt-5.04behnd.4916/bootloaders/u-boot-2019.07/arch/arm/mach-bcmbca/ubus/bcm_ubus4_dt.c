/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright (c) 2021 Broadcom Ltd.
 */
/*
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm/device-internal.h>

#include "bcm_ubus4.h"
#include "bcm_ubus_internal.h"

ubus_t *ubus_sys = NULL;
ubus_t *ubus_xrdp = NULL;
#ifdef CONFIG_BCMBCA_UBUS4_DCM
ubus_dcm_t *ubus_dcm_sys = NULL;
ubus_dcm_t *ubus_dcm_xrdp = NULL;

void bcm_ubus4_dcm_clk_bypass(int enable)
{
    volatile Ubus4ClkCtrlCfgRegs* clk;

    if(ubus_dcm_sys&&ubus_dcm_sys->clk) {
        clk = (volatile Ubus4ClkCtrlCfgRegs*)ubus_dcm_sys->clk;
        if(enable)
            clk->ClockCtrl = UBUS4_CLK_BYPASS_MASK;
        else
            clk->ClockCtrl = 0;
    }
}
#endif
  
static int __init bcm_ubus_parse_resource(struct udevice *dev, ubus_t *ubus)
{
    int ret;
    struct resource res;

    ret = dev_read_resource_byname(dev, "ubus_systop", &res);
    if (ret) {
        printk("Failed to find ubus_systop resource\n");
        return ret;
    }
    ubus->systop = ioremap(res.start, resource_size(&res));
    if (IS_ERR(ubus->systop)) {
        printk("Failed to map the ubus_systop resource\n");
        return -ENXIO;
    }
	//printk("ubus systop  \t\t addr 0x%px\n", ubus->systop);

    ret = dev_read_resource_byname(dev, "ubus_registration", &res);
    if (ret) {
        printk("Failed to find ubus_registration resource\n");
        return ret;
    }
    ubus->registration = ioremap(res.start, resource_size(&res));
    if (IS_ERR(ubus->registration)) {
        printk("Failed to map the ubus_registration resource\n");
        return -ENXIO;
    }
	//printk("ubus registration  \t addr 0x%px\n", ubus->registration);

	/* only the main ubus port has this resource */
	ret = dev_read_resource_byname(dev, "ubus_coherency_port", &res);
    if (!ret) {
	    ubus->cohport = ioremap(res.start, resource_size(&res));
        if (IS_ERR(ubus->cohport)) {
            printk("Failed to map the ubus_coherency_port resource\n");
            return -ENXIO;
        }
		//printk("ubus coherency \t\t addr 0x%px\n", ubus->cohport);
    }

    if (ret == -ENODATA)
        ret = 0;
    return ret;
}

static int __init bcm_ubus_parse_mst_node(struct udevice *dev)
{
    ofnode mst_node;
    int i = 0, ret = 0;
    ub_mst_addr_map_t* mst;
    struct resource res;

    mst_node = ofnode_find_subnode(dev_ofnode(dev), "ubus_mst");
    if (!ofnode_valid(mst_node)) {
        printk("missing ubus_mst node under ubus\n");
        return -ENODEV;
    }

    while (ub_mst_addr_map_tbl[i].port_id != -1) {
        mst = &ub_mst_addr_map_tbl[i];
        ret = ofnode_read_resource_byname(mst_node, mst->str, &res);
        if (!ret) {
            mst->phys_base = (phys_addr_t)res.start;
            mst->base = ioremap(res.start, resource_size(&res));
            if (IS_ERR(mst->base)) {
                printk("Failed to map the mst node %s resource\n", mst->str);
                mst->base = NULL;
                return -ENXIO;
            }
			//printk("ubus mst node %s\t addr 0x%px\n", mst->str, mst->base);
        }
        i++;
    }

    return 0;
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

static int bcm_ubus4_probe(struct udevice *dev)
{
	const char* name = ofnode_get_name(dev_ofnode(dev));
	ubus_t *ubus;
	int ret = 0;
	u32 flags;
									   
	if (strncmp(name, "ubus_sys", 8) != 0 && strncmp(name, "ubus_xrdp", 9) != 0)
		return -EINVAL;

	ubus = kzalloc(sizeof(*ubus), GFP_KERNEL);
	if (!ubus)
		return -ENOMEM;

	if (dev_read_u32(dev, "flags", &flags) == 0 && (flags&UBUS_FLAGS_MODULE_XRDP))
		ubus_xrdp = ubus;
 	else
		ubus_sys = ubus;

	if ((ret = bcm_ubus_parse_resource(dev, ubus)) != 0) {
		bcu_ubus_free_resource(ubus);
		return ret;
	}

	if ((ret = bcm_ubus_parse_mst_node(dev)) != 0) {
		bcu_ubus_free_resource(ubus);
	}

	printk("BCM UBUS4 driver [%s] registered\n", name);	
    return ret;
}

#ifdef CONFIG_BCMBCA_UBUS4_DCM
static int bcm_ubus4_dcm_probe(struct udevice *dev)
{
	ubus_dcm_t *ubus_dcm;
	int ret = 0;
	u32 flags;
    struct resource res;

	ubus_dcm = kzalloc(sizeof(*ubus_dcm), GFP_KERNEL);
	if (!ubus_dcm)
		return -ENOMEM;

	if (dev_read_u32(dev, "flags", &flags) == 0 && (flags&UBUS_FLAGS_MODULE_XRDP)) {
		ubus_dcm_xrdp = ubus_dcm;
        ubus_dcm->xrdp_module = 1;
	} else {
		ubus_dcm_sys = ubus_dcm;
        ubus_dcm->xrdp_module = 0;
	}

    ret = dev_read_resource_byname(dev, "ubus_dcm_clk", &res);
    if (ret) {
        printk("Failed to find ubus_systop resource\n");
		kfree(ubus_dcm);
        return ret;
    }
    ubus_dcm->clk = ioremap(res.start, resource_size(&res));
    if (IS_ERR(ubus_dcm->clk)) {
        printk("Failed to map the ubus_systop resource\n");
		ubus_dcm->clk = NULL;
		kfree(ubus_dcm);		
        return -ENXIO;
    }
	//printk("ubus dcm clk  \t\t addr 0x%px\n", ubus_dcm->clk);

	printk("BCM UBUS4 DCM clk driver [%s] registered\n",  ofnode_get_name(dev_ofnode(dev)));
    return ret;
}
#endif

int bcm_ubus_drv_init(void)
{
	struct udevice *dev = NULL;
	struct uclass *uc = NULL;
	int ret;

	ret = uclass_get(UCLASS_NOP, &uc);
	if (ret)
		return ret;
	  
	uclass_foreach_dev(dev, uc) {
		if(dev->driver == DM_GET_DRIVER(bcm_ubus4_drv)) {
			ret = device_probe(dev);
		}
	}
#ifdef CONFIG_BCMBCA_UBUS4_DCM
	uclass_foreach_dev(dev, uc) {
		if(dev->driver == DM_GET_DRIVER(bcm_ubus4_dcm_drv)) {
			ret = device_probe(dev);
		}
	}
#endif
	return ret;
}

static const struct udevice_id bcm_ubus4_of_match[] = {
	{ .compatible = "brcm,bca-ubus4"},
	{}
};

U_BOOT_DRIVER(bcm_ubus4_drv) = {
	.name = "bcm_ubus4",
	.id = UCLASS_NOP,
	.of_match = bcm_ubus4_of_match,
	.probe = bcm_ubus4_probe,
};

#ifdef CONFIG_BCMBCA_UBUS4_DCM
static const struct udevice_id bcm_ubus4_dcm_of_match[] = {
	{ .compatible = "brcm,bca-ubus4-dcm"},
	{}
};

U_BOOT_DRIVER(bcm_ubus4_dcm_drv) = {
	.name = "bcm_ubus4_dcm",
	.id = UCLASS_NOP,
	.of_match = bcm_ubus4_dcm_of_match,
	.probe = bcm_ubus4_dcm_probe,
};
#endif
