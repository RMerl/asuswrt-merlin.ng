/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifndef _IHS_MDIO_H_
#define _IHS_MDIO_H_

struct ihs_mdio_info {
#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
	u32 fpga;
#else
	struct udevice *fpga;
	int base;
#endif
	char *name;
};

int ihs_mdio_init(struct ihs_mdio_info *info);

#endif
