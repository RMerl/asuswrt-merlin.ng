#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
<:copyright-BRCM:2016:GPL/GPL:standard

   Copyright (c) 2016 Broadcom 
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

#ifndef __BRCMNAND_H__
#define __BRCMNAND_H__

#include <linux/types.h>
#include <linux/io.h>
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

extern int setup_mtd_parts(struct mtd_info* mtd);
#endif

struct platform_device;
struct dev_pm_ops;

struct brcmnand_soc {
	struct platform_device *pdev;
	void *priv;
	bool (*ctlrdy_ack)(struct brcmnand_soc *soc);
	void (*ctlrdy_set_enabled)(struct brcmnand_soc *soc, bool en);
	void (*prepare_data_bus)(struct brcmnand_soc *soc, bool prepare);
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	int (*check_dying_gasp)(struct brcmnand_soc *soc);
#endif
};

static inline void brcmnand_soc_data_bus_prepare(struct brcmnand_soc *soc)
{
	if (soc && soc->prepare_data_bus)
		soc->prepare_data_bus(soc, true);
}

static inline void brcmnand_soc_data_bus_unprepare(struct brcmnand_soc *soc)
{
	if (soc && soc->prepare_data_bus)
		soc->prepare_data_bus(soc, false);
}

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/* Check if system is losing power.  Abort any write or erase request if system 
 * is shutting down to avoid any partial write or erase to the NAND media. Otherwise
 * the impacted block or page may be unstable
 */
static inline int brcmnand_check_dying_gasp(struct brcmnand_soc *soc)
{
	int ret = 0;
	if (soc && soc->check_dying_gasp)
		ret = soc->check_dying_gasp(soc);
	return ret;
}
#endif

static inline u32 brcmnand_readl(void __iomem *addr)
{
	/*
	 * MIPS endianness is configured by boot strap, which also reverses all
	 * bus endianness (i.e., big-endian CPU + big endian bus ==> native
	 * endian I/O).
	 *
	 * Other architectures (e.g., ARM) either do not support big endian, or
	 * else leave I/O in little endian mode.
	 */
	if (IS_ENABLED(CONFIG_MIPS) && IS_ENABLED(__BIG_ENDIAN))
		return __raw_readl(addr);
	else
		return readl_relaxed(addr);
}

static inline void brcmnand_writel(u32 val, void __iomem *addr)
{
	/* See brcmnand_readl() comments */
	if (IS_ENABLED(CONFIG_MIPS) && IS_ENABLED(__BIG_ENDIAN))
		__raw_writel(val, addr);
	else
		writel_relaxed(val, addr);
}

int brcmnand_probe(struct platform_device *pdev, struct brcmnand_soc *soc);
int brcmnand_remove(struct platform_device *pdev);

extern const struct dev_pm_ops brcmnand_pm_ops;

#endif /* __BRCMNAND_H__ */

#endif
