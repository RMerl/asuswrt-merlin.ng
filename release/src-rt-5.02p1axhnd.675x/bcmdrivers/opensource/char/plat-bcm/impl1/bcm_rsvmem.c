/*
<:copyright-BRCM:2019:GPL/GPL:standard

   Copyright (c) 2019 Broadcom 
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
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_address.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <linux/dma-direct.h>
#else
#include <linux/dma-mapping.h>
#endif
#include "bcm_hwdefs.h"
#include "bcm_rsvmem.h"

static int bcm_rsvmem_is_valid_name(const char* node_name)
{
	int ret = 0, i = 0;

	while( dt_scan_mem_str[i] ) {
		if ( strncmp(node_name, dt_scan_mem_str[i], MAX_RESREVE_MEM_NAME_SIZE) == 0 ) {
			ret = 1;
			break;
		}
		i++;
	}

	return ret;
}

static int bcm_rsvmem_entry_exist(char* rsv_name, int* index)
{
        int ret = 0, i;

	for ( i = 0; i < TOTAL_RESERVE_MEM_NUM; i++ ) {
		if ( strncmp(rsv_name, reserve_mem[i].name, MAX_RESREVE_MEM_NAME_SIZE) == 0 ) {
			*index = i;
			ret = 1;
			break;
		}
	}

	return ret;
}

static int bcm_rsvmem_next_entry(void)
{
        int i;

	for ( i = 0; i < TOTAL_RESERVE_MEM_NUM; i++ ) {
		if (reserve_mem[i].name[0] == 0x0 )
			return i;
	}
	pr_err("no free rsv memory entry available current cnt %d total cnt %d\n", 
		rsvd_mem_cnt, TOTAL_RESERVE_MEM_NUM);

	return -1;
}

static int bcm_rsvmem_checkname(const char* node_name, char* rsv_name, int length)
{
	int ret = -1;

	if (strncmp(node_name, DT_RSVD_PREFIX_STR, strlen(DT_RSVD_PREFIX_STR)) == 0) {
		node_name += strlen(DT_RSVD_PREFIX_STR);
		if (bcm_rsvmem_is_valid_name(node_name) && strlen(node_name) <= length) {
			strncpy(rsv_name, node_name, length-1);
			ret = 0;
		}
		else
			pr_err("invalid rsv node name %s\n", node_name);
	}

	return ret; 
}

static int bcm_rsvmem_addmem(char* rsv_name, void* virt_addr, phys_addr_t phys_addr, uint32_t rsv_size, int cached)
{
	int ret = -1, i, index;

	if (rsvd_mem_cnt >= TOTAL_RESERVE_MEM_NUM) {
		pr_err("reserved memory count %d reached the total memory reserve count %d!!!", rsvd_mem_cnt, TOTAL_RESERVE_MEM_NUM);
		return ret;
	}

	if (!bcm_rsvmem_entry_exist(rsv_name, &index)) {
		i =  bcm_rsvmem_next_entry();
		if (i >= 0 && i < TOTAL_RESERVE_MEM_NUM) {
			strncpy(reserve_mem[i].name, rsv_name, MAX_RESREVE_MEM_NAME_SIZE);
	                reserve_mem[i].virt_addr = virt_addr;
        	        reserve_mem[i].phys_addr = phys_addr;
                	reserve_mem[i].size = rsv_size;
	                reserve_mem[i].mapped = cached;
        	        is_memory_reserved = 1;
                	reserved_mem_total += rsv_size;
	                rsvd_mem_cnt++;
			ret = 0;
			pr_info("reserved CMA memory %s virt addr %px phys addr %pa size 0x%x cached=%d\n", 
				rsv_name, virt_addr, &phys_addr, rsv_size, cached);
		}
	} else
		pr_err("Add reserved memory node %s already exist!!!", rsv_name);

	return ret;
}

static int bcm_rsvmem_removemem(char* rsv_name, reserve_mem_t* rsv_mem)
{
	int ret = -1, index;

	if (rsvd_mem_cnt > 0 && bcm_rsvmem_entry_exist(rsv_name, &index)) {
		*rsv_mem = reserve_mem[index];
		memset(&reserve_mem[index], 0x0, sizeof(reserve_mem_t));
               	reserved_mem_total -= rsv_mem->size;
                rsvd_mem_cnt--;
		if (rsvd_mem_cnt == 0)
        	        is_memory_reserved = 0;
		ret = 0;
	} else
		pr_err("Remove reserved memory node %s does not exist cur count %d!!!", rsv_name, rsvd_mem_cnt);

	return ret;

}

static int bcm_rsvmem_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct device_node *rsv_node;
	void* virt_addr;
	char rsv_name[16];
	dma_addr_t dma_addr;
	uint32_t rsv_size;
	uint32_t dsl_rsv_size = 0;
	static uint32_t total_alloc_size = 0;
	int pad_size;
	int cached = of_dma_is_coherent(dn);
	int ret;

	ret = of_reserved_mem_device_init(&pdev->dev);
	if (ret && ret != -ENODEV) {
		dev_err(&pdev->dev, "Couldn't assign reserve memory to device ret = %d\n", ret);
        	return ret;	
	}

	for_each_available_child_of_node(dn, rsv_node) {
		virt_addr = NULL;
		rsv_size  = 0;
		memset(rsv_name, 0x0, 16);

		if (bcm_rsvmem_checkname(rsv_node->name, rsv_name, 16)) {
			dev_err(&pdev->dev, "invalid reserve memory node name %s to add!!\n", rsv_node->name);
			continue;
		}

		if (of_property_read_u32(rsv_node, "rsvd-size", &rsv_size)) {
			dev_err(&pdev->dev, "invalid reserve memory node name %s does not specify the rsvd-size!!\n", rsv_node->name);

			continue;
		}

		if (rsv_size) {
			/* must allocate the DSL to end of the CMA region, skip here */
			if( strcmp(rsv_name, ADSL_BASE_ADDR_STR) == 0 ) {
				dsl_rsv_size = rsv_size;
				continue;
			}
			virt_addr = dma_alloc_coherent(&pdev->dev, rsv_size, &dma_addr, GFP_KERNEL);
			if (virt_addr == NULL)
				dev_err(&pdev->dev,"Failed to allocated cma memory of 0x%x bytes!!\n", rsv_size);
			else {
				bcm_rsvmem_addmem(rsv_name, virt_addr, dma_to_phys(&pdev->dev, dma_addr), rsv_size, cached);
				total_alloc_size += rsv_size;
			}
		}
	}

	if (dsl_rsv_size) {
		/* due to hardware requirement, dsl memory must be at the end of reserved memory i.e. last 3MB of 
		   the reserved area. check if there any extra space due to alginment requirement */
		pad_size = cma_size - total_alloc_size - dsl_rsv_size;
		BUG_ON(pad_size < 0);
		if (pad_size > 0) {
			virt_addr = dma_alloc_coherent(&pdev->dev, pad_size, &dma_addr, GFP_KERNEL);
			if (virt_addr)
				bcm_rsvmem_addmem(CMA_PAD_BASE_ADDR_STR, virt_addr, dma_to_phys(&pdev->dev, dma_addr), pad_size, cached);
		}

		virt_addr = dma_alloc_coherent(&pdev->dev, dsl_rsv_size, &dma_addr, GFP_KERNEL);
		if (virt_addr == NULL)
			dev_err(&pdev->dev,"Failed to allocated cma memory of 0x%x bytes!!\n", dsl_rsv_size);
		else
			bcm_rsvmem_addmem(ADSL_BASE_ADDR_STR, virt_addr, dma_to_phys(&pdev->dev, dma_addr), dsl_rsv_size, cached);
	}
	return ret;
}

static int bcm_rsvmem_remove(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct device_node *rsv_node;
	reserve_mem_t rsv_mem;
	char rsv_name[16];
	
	for_each_available_child_of_node(dn, rsv_node) {
		memset(rsv_name, 0x0, 16);
		if (bcm_rsvmem_checkname(rsv_node->name, rsv_name, 16)) {
			dev_err(&pdev->dev, "invalid reserve memory node name %s to remove!!\n", rsv_node->name);
			continue;
		}

		if (bcm_rsvmem_removemem(rsv_name, &rsv_mem) == 0 ) {
			dma_free_coherent(&pdev->dev, rsv_mem.size, rsv_mem.virt_addr, phys_to_dma(&pdev->dev, rsv_mem.phys_addr));
		}
	}

	return 0;
}

static const struct of_device_id bcm_rsvmem_of_match[] = {
	{.compatible = "brcm,plat-rsvmem"},
	{}
};
MODULE_DEVICE_TABLE(of, bcm_rsvmem_of_match);

static struct platform_driver bcm_rsvmem_driver = {
	.probe  = bcm_rsvmem_probe,
	.remove = bcm_rsvmem_remove,
	.driver = {
		.name = "bcm_rsvmem",
		.of_match_table = bcm_rsvmem_of_match
	}
};

static int bcm_rsvmem_init(void)
{
	platform_driver_register(&bcm_rsvmem_driver);
	return 0;
}
arch_initcall(bcm_rsvmem_init);
