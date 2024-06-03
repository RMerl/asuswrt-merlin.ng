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
#include <malloc.h>
#include <linux/types.h>
#include <linux/list.h>
#include <power/pmic.h>

static LIST_HEAD(pmic_list);

int check_reg(struct pmic *p, u32 reg)
{
	if (reg >= p->number_of_regs) {
		printf("<reg num> = %d is invalid. Should be less than %d\n",
		       reg, p->number_of_regs);
		return -EINVAL;
	}

	return 0;
}

int pmic_set_output(struct pmic *p, u32 reg, int out, int on)
{
	u32 val;

	if (pmic_reg_read(p, reg, &val))
		return -ENOTSUPP;

	if (on)
		val |= out;
	else
		val &= ~out;

	if (pmic_reg_write(p, reg, val))
		return -ENOTSUPP;

	return 0;
}

struct pmic *pmic_alloc(void)
{
	struct pmic *p;

	p = calloc(sizeof(*p), 1);
	if (!p) {
		printf("%s: No available memory for allocation!\n", __func__);
		return NULL;
	}

	list_add_tail(&p->list, &pmic_list);

	debug("%s: new pmic struct: 0x%p\n", __func__, p);

	return p;
}

struct pmic *pmic_get(const char *s)
{
	struct pmic *p;

	list_for_each_entry(p, &pmic_list, list) {
		if (strcmp(p->name, s) == 0) {
			debug("%s: pmic %s -> 0x%p\n", __func__, p->name, p);
			return p;
		}
	}

	return NULL;
}

#ifndef CONFIG_SPL_BUILD
static int pmic_dump(struct pmic *p)
{
	int i, ret;
	u32 val;

	if (!p) {
		puts("Wrong PMIC name!\n");
		return -ENODEV;
	}

	printf("PMIC: %s\n", p->name);
	for (i = 0; i < p->number_of_regs; i++) {
		ret = pmic_reg_read(p, i, &val);
		if (ret)
			puts("PMIC: Registers dump failed\n");

		if (!(i % 8))
			printf("\n0x%02x: ", i);

		printf("%08x ", val);
	}
	puts("\n");
	return 0;
}

static const char *power_get_interface(int interface)
{
	const char *power_interface[] = {"I2C", "SPI", "|+|-|"};
	return power_interface[interface];
}

static void pmic_list_names(void)
{
	struct pmic *p;

	puts("PMIC devices:\n");
	list_for_each_entry(p, &pmic_list, list) {
		printf("name: %s bus: %s_%d\n", p->name,
		       power_get_interface(p->interface), p->bus);
	}
}

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 ret, reg, val;
	char *cmd, *name;
	struct pmic *p;

	/* at least two arguments please */
	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "list") == 0) {
		pmic_list_names();
		return CMD_RET_SUCCESS;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	name = argv[1];
	cmd = argv[2];

	debug("%s: name: %s cmd: %s\n", __func__, name, cmd);
	p = pmic_get(name);
	if (!p)
		return CMD_RET_FAILURE;

	if (strcmp(cmd, "dump") == 0) {
		if (pmic_dump(p))
			return CMD_RET_FAILURE;
		return CMD_RET_SUCCESS;
	}

	if (strcmp(cmd, "read") == 0) {
		if (argc < 4)
			return CMD_RET_USAGE;

		reg = simple_strtoul(argv[3], NULL, 16);
		ret = pmic_reg_read(p, reg, &val);

		if (ret)
			puts("PMIC: Register read failed\n");

		printf("\n0x%02x: 0x%08x\n", reg, val);

		return CMD_RET_SUCCESS;
	}

	if (strcmp(cmd, "write") == 0) {
		if (argc < 5)
			return CMD_RET_USAGE;

		reg = simple_strtoul(argv[3], NULL, 16);
		val = simple_strtoul(argv[4], NULL, 16);
		pmic_reg_write(p, reg, val);

		return CMD_RET_SUCCESS;
	}

	if (strcmp(cmd, "bat") == 0) {
		if (argc < 4)
			return CMD_RET_USAGE;

		if (!p->pbat) {
			printf("%s is not a battery\n", p->name);
			return CMD_RET_FAILURE;
		}

		if (strcmp(argv[3], "state") == 0)
			p->fg->fg_battery_check(p->pbat->fg, p);

		if (strcmp(argv[3], "charge") == 0) {
			printf("BAT: %s charging (ctrl+c to break)\n",
			       p->name);
			if (p->low_power_mode)
				p->low_power_mode();
			if (p->pbat->battery_charge)
				p->pbat->battery_charge(p);
		}

		return CMD_RET_SUCCESS;
	}

	/* No subcommand found */
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	pmic,	CONFIG_SYS_MAXARGS, 1, do_pmic,
	"PMIC",
	"list - list available PMICs\n"
	"pmic name dump - dump named PMIC registers\n"
	"pmic name read <reg> - read register\n"
	"pmic name write <reg> <value> - write register\n"
	"pmic name bat state - write register\n"
	"pmic name bat charge - write register\n"
);
#endif
