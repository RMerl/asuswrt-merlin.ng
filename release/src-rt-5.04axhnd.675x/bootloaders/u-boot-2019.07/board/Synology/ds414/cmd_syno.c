// SPDX-License-Identifier: GPL-2.0+
/*
 * Commands to deal with Synology specifics.
 *
 * Copyright (C) 2015  Phil Sutter <phil@nwl.cc>
 */

#include <common.h>
#include <div64.h>
#include <spi.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>

#include <asm/io.h>
#include "../drivers/ddr/marvell/axp/ddr3_init.h"

#define ETHADDR_MAX		4
#define SYNO_SN_TAG		"SN="
#define SYNO_CHKSUM_TAG		"CHK="


static int do_syno_populate(int argc, char * const argv[])
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct spi_flash *flash;
	unsigned long addr = 0x80000; /* XXX: parameterize this? */
	loff_t offset = 0x007d0000;
	loff_t len = 0x00010000;
	char *buf, *bufp;
	char var[128];
	char val[128];
	int ret, n;

	/* XXX: arg parsing to select flash here? */

	flash = spi_flash_probe(bus, cs, speed, mode);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	buf = map_physmem(addr, len, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	ret = spi_flash_read(flash, offset, len, buf);
	if (ret) {
		puts("Failed to read from SPI flash\n");
		goto out_unmap;
	}

	for (n = 0; n < ETHADDR_MAX; n++) {
		char ethaddr[ETH_ALEN];
		int i, sum = 0;
		unsigned char csum = 0;

		for (i = 0, bufp = buf + n * 7; i < ETH_ALEN; i++) {
			sum += bufp[i];
			csum += bufp[i];
			ethaddr[i] = bufp[i];
		}
		if (!sum)		/* MAC address empty */
			continue;
		if (csum != bufp[i]) {	/* seventh byte is checksum value */
			printf("Invalid MAC address for interface %d!\n", n);
			continue;
		}
		if (n == 0)
			sprintf(var, "ethaddr");
		else
			sprintf(var, "eth%daddr", n);
		snprintf(val, sizeof(val) - 1,
		         "%02x:%02x:%02x:%02x:%02x:%02x",
		         ethaddr[0], ethaddr[1], ethaddr[2],
			 ethaddr[3], ethaddr[4], ethaddr[5]);
		printf("parsed %s = %s\n", var, val);
		env_set(var, val);
	}
	if (!strncmp(buf + 32, SYNO_SN_TAG, strlen(SYNO_SN_TAG))) {
		char *snp, *csump;
		int csum = 0;
		unsigned long c;

		snp = bufp = buf + 32 + strlen(SYNO_SN_TAG);
		for (n = 0; bufp[n] && bufp[n] != ','; n++)
			csum += bufp[n];
		bufp[n] = '\0';

		/* should come right after, but you never know */
		bufp = strstr(bufp + n + 1, SYNO_CHKSUM_TAG);
		if (!bufp) {
			printf("Serial number checksum tag missing!\n");
			goto out_unmap;
		}

		csump = bufp += strlen(SYNO_CHKSUM_TAG);
		for (n = 0; bufp[n] && bufp[n] != ','; n++)
			;
		bufp[n] = '\0';

		if (strict_strtoul(csump, 10, &c) || c != csum) {
			puts("Invalid serial number found!\n");
			ret = 1;
			goto out_unmap;
		}
		printf("parsed SN = %s\n", snp);
		env_set("SN", snp);
	} else {	/* old style format */
		unsigned char csum = 0;

		for (n = 0, bufp = buf + 32; n < 10; n++)
			csum += bufp[n];

		if (csum != bufp[n]) {
			puts("Invalid serial number found!\n");
			ret = 1;
			goto out_unmap;
		}
		bufp[n] = '\0';
		printf("parsed SN = %s\n", buf + 32);
		env_set("SN", buf + 32);
	}
out_unmap:
	unmap_physmem(buf, len);
	return ret;
}

/* map bit position to function in POWER_MNG_CTRL_REG */
static const char * const pwr_mng_bit_func[] = {
	"audio",
	"ge3", "ge2", "ge1", "ge0",
	"pcie00", "pcie01", "pcie02", "pcie03",
	"pcie10", "pcie11", "pcie12", "pcie13",
	"bp",
	"sata0_link", "sata0_core",
	"lcd",
	"sdio",
	"usb0", "usb1", "usb2",
	"idma", "xor0", "crypto",
	NULL,
	"tdm",
	"pcie20", "pcie30",
	"xor1",
	"sata1_link", "sata1_core",
	NULL,
};

static int do_syno_clk_gate(int argc, char * const argv[])
{
	u32 pwr_mng_ctrl_reg = reg_read(POWER_MNG_CTRL_REG);
	const char *func, *state;
	int i, val;

	if (argc < 2)
		return -1;

	if (!strcmp(argv[1], "get")) {
		puts("Clock Gating:\n");
		for (i = 0; i < 32; i++) {
			func = pwr_mng_bit_func[i];
			if (!func)
				continue;
			state = pwr_mng_ctrl_reg & (1 << i) ?  "ON" : "OFF";
			printf("%s:\t\t%s\n", func, state);
		}
		return 0;
	}
	if (argc < 4)
		return -1;
	if (!strcmp(argv[1], "set")) {
		func = argv[2];
		state = argv[3];
		for (i = 0; i < 32; i++) {
			if (!pwr_mng_bit_func[i])
				continue;
			if (!strcmp(func, pwr_mng_bit_func[i]))
				break;
		}
		if (i == 32) {
			printf("Error: name '%s' not known\n", func);
			return -1;
		}
		val = state[0] != '0';
		pwr_mng_ctrl_reg |= (val << i);
		pwr_mng_ctrl_reg &= ~(!val << i);
		reg_write(POWER_MNG_CTRL_REG, pwr_mng_ctrl_reg);
	}
	return 0;
}

static int do_syno(cmd_tbl_t *cmdtp, int flag,
                   int argc, char * const argv[])
{
	const char *cmd;
	int ret = 0;

	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (!strcmp(cmd, "populate_env"))
		ret = do_syno_populate(argc, argv);
	else if (!strcmp(cmd, "clk_gate"))
		ret = do_syno_clk_gate(argc, argv);

	if (ret != -1)
		return ret;
usage:
	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	syno, 5, 1, do_syno,
	"Synology specific commands",
	"populate_env                 - Read vendor data from SPI flash into environment\n"
	"clk_gate (get|set name 1|0)  - Manage clock gating\n"
);
