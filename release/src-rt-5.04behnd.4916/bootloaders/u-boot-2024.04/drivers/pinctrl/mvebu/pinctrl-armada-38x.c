// SPDX-License-Identifier: GPL-2.0-or-later
// (C) 2022 Pali Rohár <pali@kernel.org>

#include <common.h>
#include <config.h>
#include <dm.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <errno.h>
#include <asm/io.h>

struct mvebu_mpp_ctrl_setting {
	const char *name;
	const char *subname;
	u8 val;
	u8 variant;
};

struct mvebu_mpp_mode {
	const char *name;
	size_t nsettings;
	struct mvebu_mpp_ctrl_setting *settings;
};

#define MPP_MODE(_name, ...)					\
	{							\
		.name = _name,					\
		.nsettings = ARRAY_SIZE((			\
			(struct mvebu_mpp_ctrl_setting[])	\
			 { __VA_ARGS__ })),			\
		.settings = (struct mvebu_mpp_ctrl_setting[]){	\
			__VA_ARGS__ },				\
	}

#define MPP_VAR_FUNCTION(_val, _name, _subname, _mask)		\
	{							\
		.val = _val,					\
		.name = _name,					\
		.subname = _subname,				\
		.variant = _mask,				\
	}

#define MVEBU_MPPS_PER_REG	8
#define MVEBU_MPP_BITS		4
#define MVEBU_MPP_MASK		0xf

enum {
	V_88F6810 = BIT(0),
	V_88F6820 = BIT(1),
	V_88F6828 = BIT(2),
	V_88F6810_PLUS = (V_88F6810 | V_88F6820 | V_88F6828),
	V_88F6820_PLUS = (V_88F6820 | V_88F6828),
};

static struct mvebu_mpp_mode armada_38x_mpp_modes[] = {
	MPP_MODE("mpp0",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ua0",   "rxd",        V_88F6810_PLUS)),
	MPP_MODE("mpp1",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ua0",   "txd",        V_88F6810_PLUS)),
	MPP_MODE("mpp2",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "i2c0",  "sck",        V_88F6810_PLUS)),
	MPP_MODE("mpp3",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "i2c0",  "sda",        V_88F6810_PLUS)),
	MPP_MODE("mpp4",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge",    "mdc",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ua1",   "txd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua0",   "rts",        V_88F6810_PLUS)),
	MPP_MODE("mpp5",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge",    "mdio",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ua1",   "rxd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua0",   "cts",        V_88F6810_PLUS)),
	MPP_MODE("mpp6",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txclkout",   V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge0",   "crs",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "cs3",        V_88F6810_PLUS)),
	MPP_MODE("mpp7",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txd0",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad9",        V_88F6810_PLUS)),
	MPP_MODE("mpp8",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txd1",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad10",       V_88F6810_PLUS)),
	MPP_MODE("mpp9",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txd2",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad11",       V_88F6810_PLUS)),
	MPP_MODE("mpp10",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txd3",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad12",       V_88F6810_PLUS)),
	MPP_MODE("mpp11",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txctl",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad13",       V_88F6810_PLUS)),
	MPP_MODE("mpp12",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxd0",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "cs1",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad14",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "pcie3", "clkreq",     V_88F6810_PLUS)),
	MPP_MODE("mpp13",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxd1",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "pcie0", "clkreq",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "pcie1", "clkreq",     V_88F6820_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "cs2",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad15",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "pcie2", "clkreq",     V_88F6810_PLUS)),
	MPP_MODE("mpp14",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxd2",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ptp",   "clk",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "dram",  "vttctrl",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "cs3",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "we1",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "pcie3", "clkreq",     V_88F6810_PLUS)),
	MPP_MODE("mpp15",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxd3",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge",    "mdc slave",  V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "mosi",       V_88F6810_PLUS)),
	MPP_MODE("mpp16",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxctl",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge",    "mdio slave", V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "dram",  "deccerr",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "miso",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "pcie0", "clkreq",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "pcie1", "clkreq",     V_88F6820_PLUS)),
	MPP_MODE("mpp17",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxclk",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ptp",   "clk",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua1",   "rxd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "sck",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sata1", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "sata0", "prsnt",      V_88F6810_PLUS)),
	MPP_MODE("mpp18",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "rxerr",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ptp",   "trig",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua1",   "txd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi0",  "cs0",        V_88F6810_PLUS)),
	MPP_MODE("mpp19",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "col",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ptp",   "evreq",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ge0",   "txerr",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sata1", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "ua0",   "cts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "rxd",        V_88F6810_PLUS)),
	MPP_MODE("mpp20",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ge0",   "txclk",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ptp",   "clk",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "ua0",   "rts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "txd",        V_88F6810_PLUS)),
	MPP_MODE("mpp21",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "cs1",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "rxd0",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "cmd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "bootcs",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "sata1", "prsnt",      V_88F6810_PLUS)),
	MPP_MODE("mpp22",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "mosi",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad0",        V_88F6810_PLUS)),
	MPP_MODE("mpp23",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "sck",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad2",        V_88F6810_PLUS)),
	MPP_MODE("mpp24",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "miso",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ua0",   "cts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua1",   "rxd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d4",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ready",      V_88F6810_PLUS)),
	MPP_MODE("mpp25",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "cs0",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ua0",   "rts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua1",   "txd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d5",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "cs0",        V_88F6810_PLUS)),
	MPP_MODE("mpp26",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "cs2",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "i2c1",  "sck",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d6",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "cs1",        V_88F6810_PLUS)),
	MPP_MODE("mpp27",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "spi0",  "cs3",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "txclkout",   V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "i2c1",  "sda",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d7",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "cs2",        V_88F6810_PLUS)),
	MPP_MODE("mpp28",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "txd0",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "clk",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad5",        V_88F6810_PLUS)),
	MPP_MODE("mpp29",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "txd1",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ale0",       V_88F6810_PLUS)),
	MPP_MODE("mpp30",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "txd2",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "oe",         V_88F6810_PLUS)),
	MPP_MODE("mpp31",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "txd3",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ale1",       V_88F6810_PLUS)),
	MPP_MODE("mpp32",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "txctl",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "we0",        V_88F6810_PLUS)),
	MPP_MODE("mpp33",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "dram",  "deccerr",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad3",        V_88F6810_PLUS)),
	MPP_MODE("mpp34",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad1",        V_88F6810_PLUS)),
	MPP_MODE("mpp35",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ref",   "clk_out1",   V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "a1",         V_88F6810_PLUS)),
	MPP_MODE("mpp36",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ptp",   "trig",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "a0",         V_88F6810_PLUS)),
	MPP_MODE("mpp37",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ptp",   "clk",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "rxclk",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d3",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad8",        V_88F6810_PLUS)),
	MPP_MODE("mpp38",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ptp",   "evreq",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "rxd1",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ref",   "clk_out0",   V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d0",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad4",        V_88F6810_PLUS)),
	MPP_MODE("mpp39",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "i2c1",  "sck",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "rxd2",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua0",   "cts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d1",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "a2",         V_88F6810_PLUS)),
	MPP_MODE("mpp40",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "i2c1",  "sda",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "rxd3",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua0",   "rts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "sd0",   "d2",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad6",        V_88F6810_PLUS)),
	MPP_MODE("mpp41",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ua1",   "rxd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge1",   "rxctl",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua0",   "cts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "cs3",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "burst/last", V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "nand",  "rb0",        V_88F6810_PLUS)),
	MPP_MODE("mpp42",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ua1",   "txd",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "ua0",   "rts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "ad7",        V_88F6810_PLUS)),
	MPP_MODE("mpp43",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "pcie0", "clkreq",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "dram",  "vttctrl",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "dram",  "deccerr",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "cs2",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dev",   "clkout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "nand",  "rb1",        V_88F6810_PLUS)),
	MPP_MODE("mpp44",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "sata1", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "sata2", "prsnt",      V_88F6828),
		 MPP_VAR_FUNCTION(4, "sata3", "prsnt",      V_88F6828)),
	MPP_MODE("mpp45",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ref",   "clk_out0",   V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "rxd",        V_88F6810_PLUS)),
	MPP_MODE("mpp46",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ref",   "clk_out1",   V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "txd",        V_88F6810_PLUS)),
	MPP_MODE("mpp47",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "sata1", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "sata2", "prsnt",      V_88F6828),
		 MPP_VAR_FUNCTION(5, "sata3", "prsnt",      V_88F6828)),
	MPP_MODE("mpp48",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "dram",  "vttctrl",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "tdm",   "pclk",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "audio", "mclk",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d4",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "pcie0", "clkreq",     V_88F6810_PLUS)),
	MPP_MODE("mpp49",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "sata2", "prsnt",      V_88F6828),
		 MPP_VAR_FUNCTION(2, "sata3", "prsnt",      V_88F6828),
		 MPP_VAR_FUNCTION(3, "tdm",   "fsync",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "audio", "lrclk",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d5",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "pcie1", "clkreq",     V_88F6820_PLUS)),
	MPP_MODE("mpp50",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "tdm",   "drx",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "audio", "extclk",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "cmd",        V_88F6810_PLUS)),
	MPP_MODE("mpp51",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "tdm",   "dtx",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "audio", "sdo",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "dram",  "deccerr",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ptp",   "trig",       V_88F6810_PLUS)),
	MPP_MODE("mpp52",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "tdm",   "int",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "audio", "sdi",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d6",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ptp",   "clk",        V_88F6810_PLUS)),
	MPP_MODE("mpp53",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "sata1", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "tdm",   "rst",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "audio", "bclk",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d7",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ptp",   "evreq",      V_88F6810_PLUS)),
	MPP_MODE("mpp54",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "sata0", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "sata1", "prsnt",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "ge0",   "txerr",      V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d3",         V_88F6810_PLUS)),
	MPP_MODE("mpp55",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ua1",   "cts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge",    "mdio",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "pcie1", "clkreq",     V_88F6820_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "cs1",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d0",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "rxd",        V_88F6810_PLUS)),
	MPP_MODE("mpp56",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "ua1",   "rts",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "ge",    "mdc",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "dram",  "deccerr",    V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "mosi",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "txd",        V_88F6810_PLUS)),
	MPP_MODE("mpp57",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "sck",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "clk",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "txd",        V_88F6810_PLUS)),
	MPP_MODE("mpp58",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "pcie1", "clkreq",     V_88F6820_PLUS),
		 MPP_VAR_FUNCTION(2, "i2c1",  "sck",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(3, "pcie2", "clkreq",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "miso",       V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d1",         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(6, "ua1",   "rxd",        V_88F6810_PLUS)),
	MPP_MODE("mpp59",
		 MPP_VAR_FUNCTION(0, "gpio",  NULL,         V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(1, "pcie0", "rstout",     V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(2, "i2c1",  "sda",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(4, "spi1",  "cs0",        V_88F6810_PLUS),
		 MPP_VAR_FUNCTION(5, "sd0",   "d2",         V_88F6810_PLUS)),
};

static const char * const armada_38x_mpp_function_names[] = {
	"gpio", /* make gpio always as function 0 */

	"audio",
	"dev",
	"dram",
	"ge",
	"ge0",
	"ge1",
	"i2c0",
	"i2c1",
	"nand",
	"pcie0",
	"pcie1",
	"pcie2",
	"pcie3",
	"ptp",
	"ref",
	"sata0",
	"sata1",
	"sata2",
	"sata3",
	"sd0",
	"spi0",
	"spi1",
	"tdm",
	"ua0",
	"ua1",
};

struct armada_38x_pinctrl {
	void __iomem *base;
	u8 variant;
};

static int armada_38x_pinctrl_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(armada_38x_mpp_modes);
}

static const char *armada_38x_pinctrl_get_pin_name(struct udevice *dev, unsigned int selector)
{
	return armada_38x_mpp_modes[selector].name;
}

static int armada_38x_pinctrl_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(armada_38x_mpp_function_names);
}

static const char *armada_38x_pinctrl_get_function_name(struct udevice *dev, unsigned int selector)
{
	return armada_38x_mpp_function_names[selector];
}

static int armada_38x_pinctrl_get_pin_muxing(struct udevice *dev, unsigned int selector,
					     char *buf, int size)
{
	struct armada_38x_pinctrl *info = dev_get_priv(dev);
	unsigned int off = (selector / MVEBU_MPPS_PER_REG) * MVEBU_MPP_BITS;
	unsigned int shift = (selector % MVEBU_MPPS_PER_REG) * MVEBU_MPP_BITS;
	const char *func_name = NULL;
	const char *sub_name = NULL;
	unsigned long config;
	int i;

	config = (readl(info->base + off) >> shift) & MVEBU_MPP_MASK;

	for (i = 0; i < armada_38x_mpp_modes[selector].nsettings; i++) {
		if (armada_38x_mpp_modes[selector].settings[i].val == config)
			break;
	}

	if (i < armada_38x_mpp_modes[selector].nsettings) {
		func_name = armada_38x_mpp_modes[selector].settings[i].name;
		sub_name = armada_38x_mpp_modes[selector].settings[i].subname;
	}

	snprintf(buf, size, "%s%s%s",
		 func_name ? func_name : "unknown",
		 sub_name ? "_" : "",
		 sub_name ? sub_name : "");
	return 0;
}

static int armada_38x_pinctrl_pinmux_set(struct udevice *dev, unsigned int pin_selector,
					 unsigned int func_selector)
{
	struct armada_38x_pinctrl *info = dev_get_priv(dev);
	unsigned int off = (pin_selector / MVEBU_MPPS_PER_REG) * MVEBU_MPP_BITS;
	unsigned int shift = (pin_selector % MVEBU_MPPS_PER_REG) * MVEBU_MPP_BITS;
	const char *func_name = armada_38x_mpp_function_names[func_selector];
	unsigned long config, reg;
	int i;

	for (i = 0; i < armada_38x_mpp_modes[pin_selector].nsettings; i++) {
		if (strcmp(armada_38x_mpp_modes[pin_selector].settings[i].name, func_name) == 0)
			break;
	}

	if (i >= armada_38x_mpp_modes[pin_selector].nsettings)
		return -EINVAL;

	if (!(info->variant & armada_38x_mpp_modes[pin_selector].settings[i].variant))
		return -EINVAL;

	reg = readl(info->base + off) & ~(MVEBU_MPP_MASK << shift);
	config = armada_38x_mpp_modes[pin_selector].settings[i].val;
	writel(reg | (config << shift), info->base + off);

	return 0;
}

static int armada_38x_pinctrl_gpio_request_enable(struct udevice *dev, unsigned int selector)
{
	char buf[20];

	armada_38x_pinctrl_get_pin_muxing(dev, selector, buf, sizeof(buf));
	if (strcmp(buf, "gpio") != 0)
		printf("Warning: Changing mpp%u function from %s to gpio...\n", selector, buf);

	return armada_38x_pinctrl_pinmux_set(dev, selector, 0); /* gpio is always function 0 */
}

static int armada_38x_pinctrl_gpio_disable_free(struct udevice *dev, unsigned int selector)
{
	/* nothing to do */
	return 0;
}

static int armada_38x_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	return pinctrl_generic_set_state_prefix(dev, config, "marvell,");
}

static int armada_38x_pinctrl_probe(struct udevice *dev)
{
	struct armada_38x_pinctrl *info = dev_get_priv(dev);

	info->variant = (u8)dev_get_driver_data(dev);
	info->base = dev_read_addr_ptr(dev);

	if (!info->base)
		return -EINVAL;

	return 0;
}

struct pinctrl_ops armada_37xx_pinctrl_ops = {
	.get_pins_count = armada_38x_pinctrl_get_pins_count,
	.get_pin_name = armada_38x_pinctrl_get_pin_name,
	.get_functions_count = armada_38x_pinctrl_get_functions_count,
	.get_function_name = armada_38x_pinctrl_get_function_name,
	.get_pin_muxing = armada_38x_pinctrl_get_pin_muxing,
	.pinmux_set = armada_38x_pinctrl_pinmux_set,
	.gpio_request_enable = armada_38x_pinctrl_gpio_request_enable,
	.gpio_disable_free = armada_38x_pinctrl_gpio_disable_free,
	.set_state = armada_38x_pinctrl_set_state,
};

static const struct udevice_id armada_38x_pinctrl_of_match[] = {
	{
		.compatible = "marvell,mv88f6810-pinctrl",
		.data       = V_88F6810,
	},
	{
		.compatible = "marvell,mv88f6820-pinctrl",
		.data       = V_88F6820,
	},
	{
		.compatible = "marvell,mv88f6828-pinctrl",
		.data       = V_88F6828,
	},
	{ },
};

U_BOOT_DRIVER(armada_38x_pinctrl) = {
	.name = "armada-38x-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(armada_38x_pinctrl_of_match),
	.probe = armada_38x_pinctrl_probe,
	.priv_auto = sizeof(struct armada_38x_pinctrl),
	.ops = &armada_37xx_pinctrl_ops,
};
