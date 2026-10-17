/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef _MSCC_MIIM_H_
#define _MSCC_MIIM_H_

struct mscc_miim_dev {
	void __iomem *regs;
	phys_addr_t miim_base;
	unsigned long miim_size;
	struct mii_dev *bus;
};

int mscc_miim_read(struct mii_dev *bus, int addr, int devad, int reg);
int mscc_miim_write(struct mii_dev *bus, int addr, int devad, int reg, u16 val);

struct mii_dev *mscc_mdiobus_init(struct mscc_miim_dev *miim, int *miim_count,
				  phys_addr_t miim_base,
				  unsigned long miim_size);


#endif /* _MSCC_MIIM_H_ */
