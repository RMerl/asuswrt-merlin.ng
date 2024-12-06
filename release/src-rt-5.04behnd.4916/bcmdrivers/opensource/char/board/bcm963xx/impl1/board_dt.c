/*
* <:copyright-BRCM:2021:DUAL/GPL:standard
* 
*    Copyright (c) 2021 Broadcom 
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
