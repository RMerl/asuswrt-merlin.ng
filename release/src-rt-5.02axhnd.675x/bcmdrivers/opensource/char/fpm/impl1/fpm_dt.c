/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
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

/*****************************************************************************
 *
 * Copyright (c) 2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Tim Ross <tross@broadcom.com>
 *****************************************************************************/
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "fpm_dev.h"
#include "fpm_priv.h"
#include "fpm_dt.h"

int fpm_parse_dt_prop_u32(struct device_node *of_node,
			  const char *propname, u32 *dst)
{
	int status = 0;

	if (!of_property_read_u32(of_node, propname, dst)) {
		pr_debug("%s = %xh\n", propname, *dst);
	} else {
		pr_debug("Missing %s property!\n", propname);
		status = -EFAULT;
	}

	return status;
}

int fpm_parse_dt_prop_u32_array(struct device_node *of_node,
			        const char *propname, int n,
			        u32 *dst)
{
	int status = 0;
	int i;

	if (!of_property_read_u32_array(of_node, propname, dst, n)) {
		pr_debug("%s =", propname);
		for (i = 0; i < n; i++)
			pr_debug(" %xh", dst[i]);
		pr_debug("\n");
	} else {
		pr_debug("Missing %s property!\n", propname);
		status = -EINVAL;
	}

	return status;
}

int fpm_parse_dt_node(struct platform_device *pdev)
{
	int status = 0;
	struct fpmdev *fdev = pdev->dev.platform_data;
	struct device_node *of_node = pdev->dev.of_node;
	struct resource *mem;
	u32 tmp;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem) {
		pr_err("Unable to retrieve reg base.\n");
		status = -EFAULT;
		goto done;
	}
	fdev->reg_pbase = mem->start;
	fdev->reg_vbase = ioremap(mem->start, mem->end - mem->start);
	if (!fdev->reg_vbase) {
		pr_err("Unable to ioremap reg base.\n");
		status = -EFAULT;
		goto done;
	}

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		pr_err("Unable to retrieve memory base.\n");
		status = -EFAULT;
		goto err_unmap_reg;
	}
	fdev->pool_pbase[0] = (phys_addr_t)mem->start;
	fdev->pool_size[0] = (u32)resource_size(mem);

#if (CONFIG_BCM_FPM_POOL_NUM > 1)
	fdev->pool_pbase[1] = 0;
	fdev->pool_size[1] = 0;
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (mem) {
		fdev->pool_pbase[1] = (phys_addr_t)mem->start;
		fdev->pool_size[1] = (u32)resource_size(mem);
	}
#endif
#if STOP_UBUS_CAPTURE
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if (!mem) {
		pr_err("Unable to retrieve UBUS capture memory base 0.\n");
		status = -EFAULT;
		goto err_unmap_reg;
	}
	/* TBD... assume we can cast to 32 */
	fdev->cap_pbase[0] = (u32)mem->start;
	fdev->cap_size[0] = (u32)resource_size(mem);
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 4);
	if (!mem) {
		pr_err("Unable to retrieve UBUS capture memory base 1.\n");
		status = -EFAULT;
		goto err_unmap_reg;
	}
	/* TBD... assume we can cast to 32 */
	fdev->cap_pbase[1] = (u32)mem->start;
	fdev->cap_size[1] = (u32)resource_size(mem);
#endif

	fdev->irq = platform_get_irq(pdev, 0);

	status = fpm_parse_dt_prop_u32(of_node, "init", &tmp);
	if (status)
		goto err_unmap_reg;
	fdev->init = tmp == 1;

	tmp = 0;
	fpm_parse_dt_prop_u32(of_node, "track-tokens", &tmp);
	fdev->track_tokens = tmp == 1;

	tmp = 0;
	fpm_parse_dt_prop_u32(of_node, "track-on-err", &tmp);
	fdev->track_on_err = tmp == 1;

	memset(fdev->pool_alloc_weight, 0, sizeof(fdev->pool_alloc_weight));
	memset(fdev->pool_free_weight, 0, sizeof(fdev->pool_alloc_weight));
	fpm_parse_dt_prop_u32_array(of_node, "pool-alloc-weight", 2,
				    fdev->pool_alloc_weight);
	fpm_parse_dt_prop_u32_array(of_node, "pool-free-weight", 2,
				    fdev->pool_free_weight);

	goto done;

err_unmap_reg:
	iounmap(fdev->reg_vbase);

done:
	return status;
}
