/*
   Copyright (c) 2021 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :> 
*/

#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/io.h>

#define SNOOP_CTRL_ENABLE_SNOOP            0x1
#define STATUS_CHANGE_PENDING              0x1

#define CCB_ENABLE_COHERENCY               0x1

struct bcm_coherency_ops {
    int (*init)(void* param);
    void (*enable)(void* param);
};

struct bcm_coherency {
	char* name;
	struct bcm_coherency_ops *ops;
	void __iomem *base;
	int slave_intf;
	struct device_node *dn;
};

static int bcm_cci_coherency_init(void* param)
{
	struct bcm_coherency* coh = (struct bcm_coherency*)param;
	int ret;
	struct resource res;

	ret = of_address_to_resource(coh->dn, 0, &res);
	if (!ret)
		coh->base = ioremap(res.start, resource_size(&res));

	if (ret || IS_ERR_OR_NULL(coh->base)) {
		pr_err("%s fail to ioremap or find cci reg ret %d\n", coh->name, ret);
		return -ENXIO;
	}

	ret = of_property_read_u32(coh->dn, "slave-intf-cpu", &coh->slave_intf);
	if(ret) {
		pr_err("%s fail to find slave intf number ret %d\n", coh->name, ret);	  
        return EINVAL;
	}
	return 0;
}
  
static void bcm_cci_coherency_enable(void* param)
{
	struct bcm_coherency* coh = (struct bcm_coherency*)param;
	void __iomem *si_ctrl, *status;
	u32 val;

	si_ctrl = coh->base + (coh->slave_intf+1)*0x1000;
	status = coh->base + 0xc;
	
	val = readl(si_ctrl);
	writel(val|SNOOP_CTRL_ENABLE_SNOOP, si_ctrl);
	while (readl_relaxed(status) & STATUS_CHANGE_PENDING)
		;

	pr_info("%s hardware cache coherency enabled\n", coh->name);
}

static int bcm_ccb_coherency_init(void* param)
{
	struct bcm_coherency* coh = (struct bcm_coherency*)param;
	int ret;
	struct resource res;

	ret = of_address_to_resource(coh->dn, 0, &res);
	if (!ret)
		coh->base = ioremap(res.start, resource_size(&res));

	if (ret || IS_ERR_OR_NULL(coh->base)) {
		pr_err("%s fail to ioremap or find ccb reg ret %d\n", coh->name, ret);
		return -ENXIO;
	}

	return 0;
}
  

static void bcm_ccb_coherency_enable(void* param)
{
	struct bcm_coherency* coh = (struct bcm_coherency*)param;
	void __iomem *axi_config0;
	u32 val;

	axi_config0 = coh->base + 0x10;
	
	val = readl(axi_config0);
	if((val & CCB_ENABLE_COHERENCY) == 0)
		writel(val|CCB_ENABLE_COHERENCY, axi_config0);

	pr_info("%s hardware cache coherency enabled\n", coh->name);
}

static struct bcm_coherency_ops cci_ops = {
	.init = bcm_cci_coherency_init,
	.enable = bcm_cci_coherency_enable,
};

static struct bcm_coherency_ops ccb_ops = {
	.init = bcm_ccb_coherency_init,
	.enable = bcm_ccb_coherency_enable,
};

static struct bcm_coherency cci400_coh = {
	.name = "CCI-400",
	.ops = &cci_ops,
};

static struct bcm_coherency cci500_coh = {
	.name = "CCI-500",
	.ops = &cci_ops,
};

static struct bcm_coherency ccb_coh = {
	.name = "CCB",
	.ops = &ccb_ops,
};

static const struct of_device_id bcm_coherency_matches[] = {
	{.compatible = "arm,cci-400", .data = &cci400_coh },
	{.compatible = "arm,cci-500", .data = &cci500_coh},
	{.compatible = "brcm,ccb", .data = &ccb_coh},
	{},
};

static struct bcm_coherency *bcm_coherency_get(struct device_node *dn)
{
	struct bcm_coherency *coh = NULL;
	const struct of_device_id *id;

	if (dn) {
		id = of_match_node(bcm_coherency_matches, dn);
		if (id && id->data) {
			coh = (struct bcm_coherency *)id->data;
			coh->dn = dn;
		}
	}

	return coh;
}

void bcm_coherency_init(void)
{
	struct device_node *dn;
	struct bcm_coherency *coh = NULL;
	int ret = 0;

	dn = of_find_compatible_node(NULL, NULL, "arm,cci-500");
	if (!dn) {
		dn = of_find_compatible_node(NULL, NULL, "arm,cci-400");
		if (!dn)
			dn = of_find_compatible_node(NULL, NULL, "brcm,ccb");
	}

	if(dn) {
		coh = bcm_coherency_get(dn);
		if (coh) {
			ret = coh->ops->init(coh);
			if (ret == 0)
				coh->ops->enable(coh);
		}
	}

	return;
}
