// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2011 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 */

#include <common.h>
#include <spi.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <errno.h>

#if defined(CONFIG_POWER_FSL_MC13892)
#define FSL_PMIC_I2C_LENGTH	3
#elif defined(CONFIG_POWER_FSL_MC34704)
#define FSL_PMIC_I2C_LENGTH	1
#endif

#if defined(CONFIG_POWER_SPI)
static u32 pmic_spi_prepare_tx(u32 reg, u32 *val, u32 write)
{
	return (write << 31) | (reg << 25) | (*val & 0x00FFFFFF);
}
#endif

int pmic_init(unsigned char bus)
{
	static const char name[] = "FSL_PMIC";
	struct pmic *p = pmic_alloc();

	if (!p) {
		printf("%s: POWER allocation error!\n", __func__);
		return -ENOMEM;
	}

	p->name = name;
	p->number_of_regs = PMIC_NUM_OF_REGS;
	p->bus = bus;

#if defined(CONFIG_POWER_SPI)
	p->interface = PMIC_SPI;
	p->hw.spi.cs = CONFIG_FSL_PMIC_CS;
	p->hw.spi.clk = CONFIG_FSL_PMIC_CLK;
	p->hw.spi.mode = CONFIG_FSL_PMIC_MODE;
	p->hw.spi.bitlen = CONFIG_FSL_PMIC_BITLEN;
	p->hw.spi.flags = SPI_XFER_BEGIN | SPI_XFER_END;
	p->hw.spi.prepare_tx = pmic_spi_prepare_tx;
#elif defined(CONFIG_POWER_I2C)
	p->interface = PMIC_I2C;
	p->hw.i2c.addr = CONFIG_SYS_FSL_PMIC_I2C_ADDR;
	p->hw.i2c.tx_num = FSL_PMIC_I2C_LENGTH;
#else
#error "You must select CONFIG_POWER_SPI or CONFIG_POWER_I2C"
#endif

	return 0;
}
