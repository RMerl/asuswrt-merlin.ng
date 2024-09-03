/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/
#include <linux/of.h>

#include "board_dt.h"

void *bcm_get_ioreg(const char *compat, const char *name)
{
	struct device_node *node;
	size_t size;
	phys_addr_t paddr;
	void *vaddr = NULL;
	struct property *prop;
	
	node = of_find_compatible_node(NULL, NULL, compat);
	if(node)
	{
		prop = of_find_property(node, name, NULL);
		if(prop)
		{
			paddr = __swab(((phys_addr_t *)prop->value)[0]);
			size = __swab(((phys_addr_t *)prop->value)[1]);
			vaddr = ioremap(paddr, size);
		}
		
		of_node_put(node);
	}
	
	return vaddr;
}
EXPORT_SYMBOL(bcm_get_ioreg);

bool bcm_get_prop32(const char *compat, const char *name, uint32_t *value)
{
	struct device_node *node;
	struct property *prop;
	bool ret = false;
	
	node = of_find_compatible_node(NULL, NULL, compat);
	if(node)
	{
		prop = of_find_property(node, name, NULL);
		if(prop)
		{
			*value = __swab32(*(uint32_t *)prop->value);
			ret = true;
		}
		
		of_node_put(node);
	}
	
	return ret;
}
EXPORT_SYMBOL(bcm_get_prop32);
