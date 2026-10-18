/*
 * Copyright © 2023 Broadcom
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

#ifndef _BRCMSTB_HPA_H
#define _BRCMSTB_HPA_H

#define BHPA_ORDER	(21 - PAGE_SHIFT)
/*  size of single HUGE page, in bytes */
#define BHPA_SIZE	(PAGE_SIZE << BHPA_ORDER)

#if pageblock_order > BHPA_ORDER
#define BHPA_ALIGN	(1 << (pageblock_order + PAGE_SHIFT))
#else
#define BHPA_ALIGN	BHPA_SIZE
#endif

#define MAX_BHPA_REGIONS	8

struct bhpa_region {
	phys_addr_t		addr;
	phys_addr_t		size;
	int			memc;
	unsigned		mt;
};

#define B_LOG_WRN(fmt, args...) pr_warn(fmt "\n", ## args)
#define B_LOG_MSG(fmt, args...) pr_info(fmt "\n", ## args)
#define B_LOG_DBG(fmt, args...) pr_debug(fmt "\n", ## args)
#define B_LOG_TRACE(fmt, args...) do { if (0) pr_debug(fmt "\n", ## args); } while (0)

#endif /* _BRCMSTB_HPA_H */
