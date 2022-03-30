// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/types.h>
#include <power/pmic.h>
#include <spi.h>

static struct spi_slave *slave;

static u32 pmic_reg(struct pmic *p, u32 reg, u32 *val, u32 write)
{
	u32 pmic_tx, pmic_rx;
	u32 tmp;

	if (!slave) {
		slave = spi_setup_slave(p->bus, p->hw.spi.cs, p->hw.spi.clk,
					p->hw.spi.mode);

		if (!slave)
			return -ENODEV;
	}

	if (check_reg(p, reg))
		return -EINVAL;

	if (spi_claim_bus(slave))
		return -EBUSY;

	pmic_tx = p->hw.spi.prepare_tx(reg, val, write);

	tmp = cpu_to_be32(pmic_tx);

	if (spi_xfer(slave, pmic_spi_bitlen, &tmp, &pmic_rx,
			pmic_spi_flags))
		goto err;

	if (write) {
		pmic_tx = p->hw.spi.prepare_tx(reg, val, 0);
		tmp = cpu_to_be32(pmic_tx);
		if (spi_xfer(slave, pmic_spi_bitlen, &tmp, &pmic_rx,
			pmic_spi_flags))
			goto err;
	}

	spi_release_bus(slave);
	*val = cpu_to_be32(pmic_rx);

	return 0;

err:
	spi_release_bus(slave);
	return -ENOTSUPP;
}

int pmic_reg_write(struct pmic *p, u32 reg, u32 val)
{
	return pmic_reg(p, reg, &val, 1);
}

int pmic_reg_read(struct pmic *p, u32 reg, u32 *val)
{
	return pmic_reg(p, reg, val, 0);
}
