// SPDX-License-Identifier: GPL-2.0+
/*
 * Power Domain test commands
 *
 * Copyright (C) 2020 Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <k3-dev.h>

static const struct udevice_id ti_pd_of_match[] = {
	{ .compatible = "ti,sci-pm-domain" },
	{ /* sentinel */ }
};

static struct ti_k3_pd_platdata *ti_pd_find_data(void)
{
	struct udevice *dev;
	int i = 0;

	while (1) {
		uclass_get_device(UCLASS_POWER_DOMAIN, i++, &dev);
		if (!dev)
			return NULL;

		if (device_is_compatible(dev,
					 ti_pd_of_match[0].compatible))
			return  dev_get_priv(dev);
	}

	return NULL;
}

static void dump_lpsc(struct ti_k3_pd_platdata *data, struct ti_pd *pd)
{
	int i;
	struct ti_lpsc *lpsc;
	u8 state;
	static const char * const lpsc_states[] = {
		"swrstdis", "syncrst", "disable", "enable", "autosleep",
		"autowake", "unknown",
	};

	for (i = 0; i < data->num_lpsc; i++) {
		lpsc = &data->lpsc[i];
		if (lpsc->pd != pd)
			continue;
		state = lpsc_get_state(lpsc);
		if (state > ARRAY_SIZE(lpsc_states))
			state = ARRAY_SIZE(lpsc_states) - 1;
		printf("    LPSC%d: state=%s, usecount=%d\n",
		       lpsc->id, lpsc_states[state], lpsc->usecount);
	}
}

static void dump_pd(struct ti_k3_pd_platdata *data, struct ti_psc *psc)
{
	int i;
	struct ti_pd *pd;
	u8 state;
	static const char * const pd_states[] = {
		"off", "on", "unknown"
	};

	for (i = 0; i < data->num_pd; i++) {
		pd = &data->pd[i];
		if (pd->psc != psc)
			continue;
		state = ti_pd_state(pd);
		if (state > ARRAY_SIZE(pd_states))
			state = ARRAY_SIZE(pd_states) - 1;
		printf("  PD%d: state=%s, usecount=%d:\n",
		       pd->id, pd_states[state], pd->usecount);
		dump_lpsc(data, pd);
	}
}

static void dump_psc(struct ti_k3_pd_platdata *data)
{
	int i;
	struct ti_psc *psc;

	for (i = 0; i < data->num_psc; i++) {
		psc = &data->psc[i];
		printf("PSC%d [%p]:\n", psc->id, psc->base);
		dump_pd(data, psc);
	}
}

static int do_pd_dump(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct ti_k3_pd_platdata *data;

	data = ti_pd_find_data();
	if (!data)
		return CMD_RET_FAILURE;

	dump_psc(data);

	return 0;
}

static int do_pd_endis(int argc, char *const argv[], u8 state)
{
	u32 psc_id;
	u32 lpsc_id;
	int i;
	struct ti_k3_pd_platdata *data;
	struct ti_lpsc *lpsc;
	int ret;

	if (argc < 3)
		return CMD_RET_FAILURE;

	data = ti_pd_find_data();
	if (!data)
		return CMD_RET_FAILURE;

	psc_id = dectoul(argv[1], NULL);
	lpsc_id = dectoul(argv[2], NULL);

	for (i = 0; i < data->num_lpsc; i++) {
		lpsc = &data->lpsc[i];
		if (lpsc->pd->psc->id != psc_id)
			continue;
		if (lpsc->id != lpsc_id)
			continue;
		printf("%s pd [PSC:%d,LPSC:%d]...\n",
		       state == MDSTAT_STATE_ENABLE ? "Enabling" : "Disabling",
		       psc_id, lpsc_id);
		ret = ti_lpsc_transition(lpsc, state);
		if (ret)
			return CMD_RET_FAILURE;
		else
			return 0;
	}

	printf("No matching psc/lpsc found.\n");

	return CMD_RET_FAILURE;
}

static int do_pd_enable(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return do_pd_endis(argc, argv, MDSTAT_STATE_ENABLE);
}

static int do_pd_disable(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	return do_pd_endis(argc, argv, MDSTAT_STATE_SWRSTDISABLE);
}

static struct cmd_tbl cmd_pd[] = {
	U_BOOT_CMD_MKENT(dump, 1, 0, do_pd_dump, "", ""),
	U_BOOT_CMD_MKENT(enable, 3, 0, do_pd_enable, "", ""),
	U_BOOT_CMD_MKENT(disable, 3, 0, do_pd_disable, "", ""),
};

static int ti_do_pd(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct cmd_tbl *c;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_pd, ARRAY_SIZE(cmd_pd));
	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(pd,
	   "dump                 - show power domain status\n"
	   "enable [psc] [lpsc]  - enable power domain\n"
	   "disable [psc] [lpsc] - disable power domain\n");

U_BOOT_CMD(pd, 4, 1, ti_do_pd,
	   "TI power domain control", pd_help_text
);
