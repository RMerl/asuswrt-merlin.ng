// SPDX-License-Identifier: GPL-2.0
/*
 * Stout board CPLD access support
 *
 * Copyright (C) 2015 Renesas Electronics Europe GmbH
 * Copyright (C) 2015 Renesas Electronics Corporation
 * Copyright (C) 2015 Cogent Embedded, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include "cpld.h"

#define SCLK			(92 + 24)
#define SSTBZ			(92 + 25)
#define MOSI			(92 + 26)
#define MISO			(92 + 27)

#define CPLD_ADDR_MODE		0x00 /* RW */
#define CPLD_ADDR_MUX		0x01 /* RW */
#define CPLD_ADDR_HDMI		0x02 /* RW */
#define CPLD_ADDR_DIPSW		0x08 /* R */
#define CPLD_ADDR_RESET		0x80 /* RW */
#define CPLD_ADDR_VERSION	0xFF /* R */

static u32 cpld_read(u8 addr)
{
	int i;
	u32 data = 0;

	for (i = 0; i < 8; i++) {
		gpio_set_value(MOSI, addr & 0x80); /* MSB first */
		gpio_set_value(SCLK, 1);
		addr <<= 1;
		gpio_set_value(SCLK, 0);
	}

	gpio_set_value(MOSI, 0); /* READ */
	gpio_set_value(SSTBZ, 0);
	gpio_set_value(SCLK, 1);
	gpio_set_value(SCLK, 0);
	gpio_set_value(SSTBZ, 1);

	for (i = 0; i < 32; i++) {
		gpio_set_value(SCLK, 1);
		data <<= 1;
		data |= gpio_get_value(MISO); /* MSB first */
		gpio_set_value(SCLK, 0);
	}

	return data;
}

static void cpld_write(u8 addr, u32 data)
{
	int i;

	for (i = 0; i < 32; i++) {
		gpio_set_value(MOSI, data & (1 << 31)); /* MSB first */
		gpio_set_value(SCLK, 1);
		data <<= 1;
		gpio_set_value(SCLK, 0);
	}

	for (i = 0; i < 8; i++) {
		gpio_set_value(MOSI, addr & 0x80); /* MSB first */
		gpio_set_value(SCLK, 1);
		addr <<= 1;
		gpio_set_value(SCLK, 0);
	}

	gpio_set_value(MOSI, 1); /* WRITE */
	gpio_set_value(SSTBZ, 0);
	gpio_set_value(SCLK, 1);
	gpio_set_value(SCLK, 0);
	gpio_set_value(SSTBZ, 1);
}

/* LSI pin pull-up control */
#define PUPR3		0xe606010C
#define PUPR3_SD3_DAT1	(1 << 27)

void cpld_init(void)
{
	u32 val;

	/* PULL-UP on MISO line */
	val = readl(PUPR3);
	val |= PUPR3_SD3_DAT1;
	writel(val, PUPR3);

	gpio_request(SCLK, "SCLK");
	gpio_request(SSTBZ, "SSTBZ");
	gpio_request(MOSI, "MOSI");
	gpio_request(MISO, "MISO");

	gpio_direction_output(SCLK, 0);
	gpio_direction_output(SSTBZ, 1);
	gpio_direction_output(MOSI, 0);
	gpio_direction_input(MISO);

	/* dummy read */
	cpld_read(CPLD_ADDR_VERSION);

	printf("CPLD version:              0x%08x\n",
	       cpld_read(CPLD_ADDR_VERSION));
	printf("H2 Mode setting (MD0..28): 0x%08x\n",
	       cpld_read(CPLD_ADDR_MODE));
	printf("Multiplexer settings:      0x%08x\n",
	       cpld_read(CPLD_ADDR_MUX));
	printf("HDMI setting:              0x%08x\n",
	       cpld_read(CPLD_ADDR_HDMI));
	printf("DIPSW (SW3):               0x%08x\n",
	       cpld_read(CPLD_ADDR_DIPSW));

#ifdef CONFIG_SH_SDHI
	/* switch MUX to SD0 */
	val = cpld_read(CPLD_ADDR_MUX);
	val &= ~MUX_MSK_SD0;
	val |= MUX_VAL_SD0;
	cpld_write(CPLD_ADDR_MUX, val);
#endif
}

static int do_cpld(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 addr, val;

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[2], NULL, 16);
	if (!(addr == CPLD_ADDR_VERSION || addr == CPLD_ADDR_MODE ||
	      addr == CPLD_ADDR_MUX || addr == CPLD_ADDR_HDMI ||
	      addr == CPLD_ADDR_DIPSW || addr == CPLD_ADDR_RESET)) {
		printf("cpld invalid addr\n");
		return CMD_RET_USAGE;
	}

	if (argc == 3 && strcmp(argv[1], "read") == 0) {
		printf("0x%x\n", cpld_read(addr));
	} else if (argc == 4 && strcmp(argv[1], "write") == 0) {
		val = simple_strtoul(argv[3], NULL, 16);
		if (addr == CPLD_ADDR_MUX) {
			/* never mask SCIFA0 console */
			val &= ~MUX_MSK_SCIFA0_USB;
			val |= MUX_VAL_SCIFA0_USB;
		}
		cpld_write(addr, val);
	}

	return 0;
}

U_BOOT_CMD(
	cpld, 4, 1, do_cpld,
	"CPLD access",
	"read addr\n"
	"cpld write addr val\n"
);

void reset_cpu(ulong addr)
{
	cpld_write(CPLD_ADDR_RESET, 1);
}
