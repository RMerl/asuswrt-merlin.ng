// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2020 CS Group
 * Charles Frey <charles.frey@c-s.fr>
 * Florent Trinh Thai <florent.trinh-thai@c-s.fr>
 * Christophe Leroy <christophe.leroy@c-s.fr>
 *
 * Common specific routines for the CS Group boards
 */

#include <dm.h>
#include <env.h>
#include <fdt_support.h>
#include <hang.h>
#include <spi.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "common.h"

#define ADDR_FPGA_R_BASE		((unsigned char  __iomem *)CONFIG_FPGA_BASE)

#define FPGA_R_ACQ_AL_FAV	0x04

#define TYPE_MCR			0x22
#define TYPE_MIAE			0x23

#define FAR_CASRSA     2
#define FAR_VGOIP      4
#define FAV_CLA        7
#define FAV_SRSA       8

#define SPI_EEPROM_READ	0x03

static int fdt_set_node_and_value(void *blob, char *node, const char *prop,
				  void *var, int size)
{
	int ret, off;

	off = fdt_path_offset(blob, node);

	if (off < 0) {
		printf("Cannot find %s node err:%s\n", node, fdt_strerror(off));

		return off;
	}

	ret = fdt_setprop(blob, off, prop, var, size);

	if (ret < 0)
		printf("Cannot set %s/%s prop err: %s\n", node, prop, fdt_strerror(ret));

	return ret;
}

/* Checks front/rear id and remove unneeded nodes from the blob */
static void ft_cleanup(void *blob, unsigned long id, const char *prop, const char *compatible)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, compatible);

	while (off != -FDT_ERR_NOTFOUND) {
		const struct fdt_property *ids;
		int nb_ids, idx;
		int tmp = -1;

		ids = fdt_get_property(blob, off, prop, &nb_ids);

		for (idx = 0; idx < nb_ids; idx += 4) {
			if (*((uint32_t *)&ids->data[idx]) == id)
				break;
		}

		if (idx >= nb_ids)
			fdt_del_node(blob, off);
		else
			tmp = off;

		off = fdt_node_offset_by_compatible(blob, tmp, compatible);
	}

	fdt_set_node_and_value(blob, "/", prop, &id, sizeof(uint32_t));
}

int read_eeprom(u8 *din, int len)
{
	struct udevice *eeprom;
	struct spi_slave *slave;
	uchar dout[3] = {SPI_EEPROM_READ, 0, 0};
	int ret;

	ret = uclass_get_device(UCLASS_SPI, 0, &eeprom);
	if (ret)
		return ret;

	ret = _spi_get_bus_and_cs(0, 0, 1000000, 0, "spi_generic_drv",
				  "generic_0:0", &eeprom, &slave);
	if (ret)
		return ret;

	ret = spi_claim_bus(slave);

	ret = spi_xfer(slave, sizeof(dout) << 3, dout, NULL, SPI_XFER_BEGIN);
	if (ret)
		return ret;

	ret = spi_xfer(slave, len << 3, NULL, din, SPI_XFER_END);
	if (ret)
		return ret;

	spi_release_bus(slave);

	return 0;
}

int ft_board_setup_common(void *blob)
{
	u8 far_id, fav_id;

	if (in_8(ADDR_FPGA_R_BASE) != TYPE_MIAE)
		return 0;

	far_id = in_8(ADDR_FPGA_R_BASE + 0x43) >> 5;
	ft_cleanup(blob, far_id, "far-id", "cs,mia-far");

	fav_id = in_8(ADDR_FPGA_R_BASE + 0x44) >> 5;

	if (far_id == FAR_CASRSA && fav_id == FAV_CLA)
		fav_id = FAV_SRSA;

	ft_cleanup(blob, fav_id, "fav-id", "cs,mia-fav");

	if (far_id == FAR_CASRSA)
		ft_board_setup_phy3();

	return 0;
}

int checkboard_common(void)
{
	switch (in_8(ADDR_FPGA_R_BASE)) {
		int far_id;
	case TYPE_MCR:
		printf("MCR3000_2G (CS GROUP)\n");
		break;
	case TYPE_MIAE:
		far_id = in_8(ADDR_FPGA_R_BASE + 0x43) >> 5;

		if (far_id == FAR_VGOIP)
			printf("VGoIP (CS GROUP)\n");
		else
			printf("MIAE (CS GROUP)\n");

		break;
	default:
		printf("Unknown\n");
		for (;;)
			;
		break;
	}
	return 0;
}

void misc_init_r_common(void)
{
	u8 tmp, far_id;
	int count = 3;

	switch (in_8(ADDR_FPGA_R_BASE)) {
	case TYPE_MCR:
		/* if at boot alarm button is pressed, delay boot */
		if ((in_8(ADDR_FPGA_R_BASE + 0x31) & FPGA_R_ACQ_AL_FAV) == 0)
			env_set("bootdelay", "60");

		env_set("config", CFG_BOARD_MCR3000_2G);
		env_set("hostname", CFG_BOARD_MCR3000_2G);
		break;

	case TYPE_MIAE:
		do {
			tmp = in_8(ADDR_FPGA_R_BASE + 0x41);
			count--;
			mdelay(10); /* 10msec wait */
		} while (count && tmp != in_8(ADDR_FPGA_R_BASE + 0x41));

		if (!count) {
			printf("Cannot read the reset factory switch position\n");
			hang();
		}

		if (tmp & 0x1)
			env_set_default("Factory settings switch ON", 0);

		env_set("config", CFG_BOARD_MIAE);
		far_id = in_8(ADDR_FPGA_R_BASE + 0x43) >> 5;

		if (far_id == FAR_VGOIP)
			env_set("hostname", CFG_BOARD_VGOIP);
		else
			env_set("hostname", CFG_BOARD_MIAE);
		break;

	default:
		env_set("config", CFG_BOARD_CMPCXXX);
		env_set("hostname", CFG_BOARD_CMPCXXX);
		break;
	}
}

void iop_setup_common(void)
{
	u8 type = in_8(ADDR_FPGA_R_BASE);

	if (type == TYPE_MCR)
		iop_setup_mcr();
	else if (type == TYPE_MIAE)
		iop_setup_miae();
}
