// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Valentin Lontgchamp, Keymile AG, valentin.longchamp@keymile.com
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <miiphy.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <mv88e6352.h>

#define SMI_HDR		((0x8 | 0x1) << 12)
#define SMI_BUSY_MASK	(0x8000)
#define SMIRD_OP	(0x2 << 10)
#define SMIWR_OP	(0x1 << 10)
#define SMI_MASK	0x1f
#define PORT_SHIFT	5

#define COMMAND_REG	0
#define DATA_REG	1

/* global registers */
#define GLOBAL		0x1b

#define GLOBAL_STATUS	0x00
#define PPU_STATE	0x8000

#define GLOBAL_CTRL	0x04
#define SW_RESET	0x8000
#define PPU_ENABLE	0x4000

static int sw_wait_rdy(const char *devname, u8 phy_addr)
{
	u16 command;
	u32 timeout = 100;
	int ret;

	/* wait till the SMI is not busy */
	do {
		/* read command register */
		ret = miiphy_read(devname, phy_addr, COMMAND_REG, &command);
		if (ret < 0) {
			printf("%s: Error reading command register\n",
				__func__);
			return ret;
		}
		if (timeout-- == 0) {
			printf("Err..(%s) SMI busy timeout\n", __func__);
			return -EFAULT;
		}
	} while (command & SMI_BUSY_MASK);

	return 0;
}

static int sw_reg_read(const char *devname, u8 phy_addr, u8 port,
	u8 reg, u16 *data)
{
	int ret;
	u16 command;

	ret = sw_wait_rdy(devname, phy_addr);
	if (ret)
		return ret;

	command = SMI_HDR | SMIRD_OP | ((port&SMI_MASK) << PORT_SHIFT) |
			(reg & SMI_MASK);
	debug("%s: write to command: %#x\n", __func__, command);
	ret = miiphy_write(devname, phy_addr, COMMAND_REG, command);
	if (ret)
		return ret;

	ret = sw_wait_rdy(devname, phy_addr);
	if (ret)
		return ret;

	ret = miiphy_read(devname, phy_addr, DATA_REG, data);

	return ret;
}

static int sw_reg_write(const char *devname, u8 phy_addr, u8 port,
	u8 reg, u16 data)
{
	int ret;
	u16 value;

	ret = sw_wait_rdy(devname, phy_addr);
	if (ret)
		return ret;

	debug("%s: write to data: %#x\n", __func__, data);
	ret = miiphy_write(devname, phy_addr, DATA_REG, data);
	if (ret)
		return ret;

	value = SMI_HDR | SMIWR_OP | ((port & SMI_MASK) << PORT_SHIFT) |
			(reg & SMI_MASK);
	debug("%s: write to command: %#x\n", __func__, value);
	ret = miiphy_write(devname, phy_addr, COMMAND_REG, value);
	if (ret)
		return ret;

	ret = sw_wait_rdy(devname, phy_addr);
	if (ret)
		return ret;

	return 0;
}

static int ppu_enable(const char *devname, u8 phy_addr)
{
	int i, ret = 0;
	u16 reg;

	ret = sw_reg_read(devname, phy_addr, GLOBAL, GLOBAL_CTRL, &reg);
	if (ret) {
		printf("%s: Error reading global ctrl reg\n", __func__);
		return ret;
	}

	reg |= PPU_ENABLE;

	ret = sw_reg_write(devname, phy_addr, GLOBAL, GLOBAL_CTRL, reg);
	if (ret) {
		printf("%s: Error writing global ctrl reg\n", __func__);
		return ret;
	}

	for (i = 0; i < 1000; i++) {
		sw_reg_read(devname, phy_addr, GLOBAL, GLOBAL_STATUS,
			&reg);
		if ((reg & 0xc000) == 0xc000)
			return 0;
		udelay(1000);
	}

	return -ETIMEDOUT;
}

static int ppu_disable(const char *devname, u8 phy_addr)
{
	int i, ret = 0;
	u16 reg;

	ret = sw_reg_read(devname, phy_addr, GLOBAL, GLOBAL_CTRL, &reg);
	if (ret) {
		printf("%s: Error reading global ctrl reg\n", __func__);
		return ret;
	}

	reg &= ~PPU_ENABLE;

	ret = sw_reg_write(devname, phy_addr, GLOBAL, GLOBAL_CTRL, reg);
	if (ret) {
		printf("%s: Error writing global ctrl reg\n", __func__);
		return ret;
	}

	for (i = 0; i < 1000; i++) {
		sw_reg_read(devname, phy_addr, GLOBAL, GLOBAL_STATUS,
			&reg);
		if ((reg & 0xc000) != 0xc000)
			return 0;
		udelay(1000);
	}

	return -ETIMEDOUT;
}

int mv88e_sw_program(const char *devname, u8 phy_addr,
	struct mv88e_sw_reg *regs, int regs_nb)
{
	int i, ret = 0;

	/* first we need to disable the PPU */
	ret = ppu_disable(devname, phy_addr);
	if (ret) {
		printf("%s: Error disabling PPU\n", __func__);
		return ret;
	}

	for (i = 0; i < regs_nb; i++) {
		ret = sw_reg_write(devname, phy_addr, regs[i].port,
			regs[i].reg, regs[i].value);
		if (ret) {
			printf("%s: Error configuring switch\n", __func__);
			ppu_enable(devname, phy_addr);
			return ret;
		}
	}

	/* re-enable the PPU */
	ret = ppu_enable(devname, phy_addr);
	if (ret) {
		printf("%s: Error enabling PPU\n", __func__);
		return ret;
	}

	return 0;
}

int mv88e_sw_reset(const char *devname, u8 phy_addr)
{
	int i, ret = 0;
	u16 reg;

	ret = sw_reg_read(devname, phy_addr, GLOBAL, GLOBAL_CTRL, &reg);
	if (ret) {
		printf("%s: Error reading global ctrl reg\n", __func__);
		return ret;
	}

	reg = SW_RESET | PPU_ENABLE | 0x0400;

	ret = sw_reg_write(devname, phy_addr, GLOBAL, GLOBAL_CTRL, reg);
	if (ret) {
		printf("%s: Error writing global ctrl reg\n", __func__);
		return ret;
	}

	for (i = 0; i < 1000; i++) {
		sw_reg_read(devname, phy_addr, GLOBAL, GLOBAL_STATUS,
			&reg);
		if ((reg & 0xc800) != 0xc800)
			return 0;
		udelay(1000);
	}

	return -ETIMEDOUT;
}

int do_mvsw_reg_read(const char *name, int argc, char *const argv[])
{
	u16 value = 0, phyaddr, reg, port;
	int ret;

	phyaddr = dectoul(argv[1], NULL);
	port = dectoul(argv[2], NULL);
	reg = dectoul(argv[3], NULL);

	ret = sw_reg_read(name, phyaddr, port, reg, &value);
	printf("%#x\n", value);

	return ret;
}

int do_mvsw_reg_write(const char *name, int argc, char *const argv[])
{
	u16 value = 0, phyaddr, reg, port;
	int ret;

	phyaddr = dectoul(argv[1], NULL);
	port = dectoul(argv[2], NULL);
	reg = dectoul(argv[3], NULL);
	value = hextoul(argv[4], NULL);

	ret = sw_reg_write(name, phyaddr, port, reg, value);

	return ret;
}


int do_mvsw_reg(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	const char *cmd, *ethname;

	if (argc < 2)
		return cmd_usage(cmdtp);

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "read") == 0) {
		if (argc < 5)
			return cmd_usage(cmdtp);
		ethname = argv[1];
		--argc;
		++argv;
		ret = do_mvsw_reg_read(ethname, argc, argv);
	} else if (strcmp(cmd, "write") == 0) {
		if (argc < 6)
			return cmd_usage(cmdtp);
		ethname = argv[1];
		--argc;
		++argv;
		ret = do_mvsw_reg_write(ethname, argc, argv);
	} else
		return cmd_usage(cmdtp);

	return ret;
}

U_BOOT_CMD(
	mvsw_reg,	7,	1,	do_mvsw_reg,
	"marvell 88e6352 switch register access",
	"write ethname phyaddr port reg value\n"
	"mvsw_reg read  ethname phyaddr port reg\n"
	);
