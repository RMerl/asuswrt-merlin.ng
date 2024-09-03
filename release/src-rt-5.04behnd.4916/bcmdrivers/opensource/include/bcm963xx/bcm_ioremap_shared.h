/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

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

#ifndef __BCM_IOREMAP_SHARED_H
#define __BCM_IOREMAP_SHARED_H

#include <linux/io.h>

/**
 * devm_ioremap_resource() - check, request region, and ioremap resource
 * @dev: generic device to handle the resource for
 * @res: resource to be handled
 *
 * Based on kernel patch proposition [4/5] devres: Add devm_ioremap_shared_resource() (https://patchwork.kernel.org/patch/10177127/)
 * Should be dropped when mentioned patch will be incorporated in kernel
 *
 * Checks that a resource is a valid memory region and ioremaps it. All operations are managed and will be undone
 * on driver detach.
 *
 * Returns a pointer to the remapped memory or an ERR_PTR() encoded error code
 * on failure. Usage example:
 *
 *	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
 *	base = devm_ioremap_resource(&pdev->dev, res);
 *	if (IS_ERR(base))
 *		return PTR_ERR(base);
 */
static void __iomem *devm_ioremap_shared_resource(struct device *dev,
				    const struct resource *res)
{
	resource_size_t size;
	void __iomem *dest_ptr;

	BUG_ON(!dev);

	if (!res || resource_type(res) != IORESOURCE_MEM) {
		dev_err(dev, "invalid resource\n");
		return IOMEM_ERR_PTR(-EINVAL);
	}

	size = resource_size(res);

	dest_ptr = devm_ioremap(dev, res->start, size);
	if (!dest_ptr) {
		dev_err(dev, "ioremap failed for resource %pR\n", res);
		dest_ptr = IOMEM_ERR_PTR(-ENOMEM);
	}

	return dest_ptr;
}

#endif
