/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * CPSW MDIO generic driver API for TI AMxx/K2x/EMAC devices.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef CPSW_MDIO_H_
#define CPSW_MDIO_H_

struct cpsw_mdio;

struct mii_dev *cpsw_mdio_init(const char *name, u32 mdio_base,
			       u32 bus_freq, int fck_freq);
void cpsw_mdio_free(struct mii_dev *bus);
u32 cpsw_mdio_get_alive(struct mii_dev *bus);

#endif /* CPSW_MDIO_H_ */
