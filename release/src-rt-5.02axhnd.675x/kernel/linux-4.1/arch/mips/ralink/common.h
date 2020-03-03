/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#ifndef _RALINK_COMMON_H__
#define _RALINK_COMMON_H__

#define RAMIPS_SYS_TYPE_LEN	32

struct ralink_soc_info {
	unsigned char sys_type[RAMIPS_SYS_TYPE_LEN];
	unsigned char *compatible;

	unsigned long mem_base;
	unsigned long mem_size;
	unsigned long mem_size_min;
	unsigned long mem_size_max;
};
extern struct ralink_soc_info soc_info;

extern void ralink_of_remap(void);

extern void ralink_clk_init(void);
extern void ralink_clk_add(const char *dev, unsigned long rate);

extern void ralink_rst_init(void);

extern void prom_soc_init(struct ralink_soc_info *soc_info);

__iomem void *plat_of_remap_node(const char *node);

#endif /* _RALINK_COMMON_H__ */
