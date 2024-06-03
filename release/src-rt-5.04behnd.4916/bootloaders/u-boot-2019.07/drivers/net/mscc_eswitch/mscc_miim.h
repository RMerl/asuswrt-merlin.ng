/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

struct mscc_miim_dev {
	void __iomem *regs;
	void __iomem *phy_regs;
};

int mscc_miim_read(struct mii_dev *bus, int addr, int devad, int reg);
int mscc_miim_write(struct mii_dev *bus, int addr, int devad, int reg, u16 val);
