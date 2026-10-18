/*
 * Copyright © 2015 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#ifndef _LINUX_BRCMSTB_BMEM_H
#define _LINUX_BRCMSTB_BMEM_H

int bmem_find_region(phys_addr_t addr, phys_addr_t size);
int bmem_get_page(struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long start, struct page **page);
int bmem_region_info(int idx, phys_addr_t *addr, phys_addr_t *size);

/* Below functions are for calling during initialization and may need stubs */

#ifdef CONFIG_BRCMSTB_BMEM
void __init bmem_reserve(void (*setup)(phys_addr_t addr, phys_addr_t size));
#endif

#endif /* _LINUX_BRCMSTB_BMEM_H */
