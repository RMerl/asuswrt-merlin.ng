// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019
 * Broadcom Corp
 */

#include <command.h>
#include <common.h>
#include <environment.h>
#include "spl_env.h"
#include "pmc_drv.h"

#ifdef PMC_ON_HOSTCPU
#define PMCcmd(...) -1
#else
extern int PMCcmd(int arg[4]);
#endif

DECLARE_GLOBAL_DATA_PTR;

enum {
	PMCFW_CODE,
	PMCFW_DATA,
	PMCFW_TYPES
};

static int is_pmcfw_loaded(int type)
{
	const char *path[PMCFW_TYPES] = {
		"/fit-images/pmcfw_code",
		"/fit-images/pmcfw_data"
	};
	int offset = fdt_path_offset(gd->fdt_blob, path[type]);
	const uint32_t *load_addr, *size;

	if (offset < 0) return 0;
	load_addr = fdt_getprop(gd->fdt_blob, offset, "load-addr", NULL);
	if (!load_addr) return 0;
	size = fdt_getprop(gd->fdt_blob, offset, "size", NULL);
	if (!size) return 0;
	printf("pmc firmware %s has been loaded to 0x%x, size=%uB\n",
			(type ? "data" : "code"),
			be32_to_cpu(*load_addr), be32_to_cpu(*size));

	return 1;
}

int is_pmcfw_code_loaded(void)
{
	return is_pmcfw_loaded(PMCFW_CODE);
}

int is_pmcfw_data_loaded(void)
{
	return is_pmcfw_loaded(PMCFW_DATA);
}
#if IS_BCMCHIP(6855)

typedef union 
{
    struct
    {
        uint32_t slow_marg : 16; //[00:15]
        uint32_t fast_marg : 16; //[16:31]
    } Bits;
    uint32_t Reg32;
} MARGINS_TEST;
#define MARG ((volatile MARGINS_TEST *)0xff802628)
#define MARG_CMD ((volatile uint32_t *)0xff80262c)

int getAVSConfig(void)
{
    if (*MARG_CMD == 0x50f77e57)
        return 1;
    else 
        return 0;
}

static void setAVSConfig(int new)
{
    if (new)
    {
        *MARG_CMD = 0x50f77e57;
        printf("Reboot to run with disabled AVS\n");
    }
    else
        *MARG_CMD = 0x0;
}
#else
// return 1 if the environment variable avs_disable=1
int getAVSConfig(void)
{
	char *disable = NULL;
	int offset = fdt_path_offset(gd->fdt_blob, "/chosen");

	if (offset < 0) return 0;
	disable = fdt_getprop(gd->fdt_blob, offset, ENV_AVS_DISABLE, NULL);
	if (!disable) return 0;

	return *disable == '1';
}

static void setAVSConfig(int new)
{
	int old = getAVSConfig();

	if (old == new) return;

	if (!env_set(ENV_AVS_DISABLE, new ? "1" : NULL) && !env_save())
		printf("Reboot to use new " ENV_AVS_DISABLE "=%d\n", new);
}
#endif
static int
do_pmc_sc_avs(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
#if IS_BCMCHIP(4908) || IS_BCMCHIP(63158) || IS_BCMCHIP(63178) || \
	IS_BCMCHIP(47622) || IS_BCMCHIP(63146) || IS_BCMCHIP(4912) || \
	IS_BCMCHIP(6756) || IS_BCMCHIP(6855)
	int pmc[4];
	static const char *disables[] = {"0", "disable", "false", "off"};
	int res, i;

	if (argc < 2) {
		res = getAVSConfig();
		printf("AVS %sdisabled by environment\n", res ? "" : "not ");
		return CMD_RET_SUCCESS;
	}

	if (!strcmp(argv[1], "show")) {
#if PMC_ON_HOSTCPU
		res = getAVSConfig();
		printf("AVS is %s \n", res ? "disabled" : "enabled");
#else
		memset((void*)pmc, 0, sizeof(pmc));
		/* Try to know the AVS disable state */
		pmc[0] = 20; // cmdGetAvsDisableState defined in pmc/command.h
		res = PMCcmd(pmc);
		if (!res)
			printf("AVS %s\n", pmc[2] ? "Disabled" : "Enabled");
		else
			printf("failed to get AVS state, res=%d\n", res);
#endif
		return CMD_RET_SUCCESS;
	}

	res = 0;
	for (i = 0; i < sizeof disables / sizeof disables[0]; i++)
		if (!strcmp(argv[1], disables[i])) {
			res = 1;
			break;
		}

	setAVSConfig(res);
#else
	printf("not supported\n");
#endif
	return CMD_RET_SUCCESS;
}

static int
do_pmc_sc_closeavs(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148) || IS_BCMCHIP(4908) || \
			IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X) || IS_BCMCHIP(6855)
	int res;
	uint32_t island = 0;
	uint16_t margin_mv_slow;
	uint16_t margin_mv_fast;
	uint16_t maximum_mv = 0;
	uint16_t minimum_mv = 0;

	if (!(argc == 4 || argc == 3
#if IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X)
		|| argc == 6 || argc == 5
#endif
		)) {
		printf("invalid number of parameters\n");
		return CMD_RET_SUCCESS;
	}

	switch (argc) {
	case 3:
	case 5:
		margin_mv_slow = (uint16_t)simple_strtol(argv[1], NULL, 0);
		margin_mv_fast = (uint16_t)simple_strtol(argv[2], NULL, 0);
		if (argc == 3)
			break;
		maximum_mv = (uint16_t)simple_strtol(argv[3], NULL, 0);
		minimum_mv = (uint16_t)simple_strtol(argv[4], NULL, 0);
		break;
	case 4:
	case 6:
		island = (uint32_t)simple_strtol(argv[1], NULL, 0);
		margin_mv_slow = (uint16_t)simple_strtol(argv[2], NULL, 0);
		margin_mv_fast = (uint16_t)simple_strtol(argv[3], NULL, 0);
		if (argc == 4)
			break;
		maximum_mv = (uint16_t)simple_strtol(argv[4], NULL, 0);
		minimum_mv = (uint16_t)simple_strtol(argv[5], NULL, 0);
		break;
	}

	if (minimum_mv && maximum_mv && (minimum_mv > maximum_mv)) {
		printf("invalid parameters: min %d > max %d "
				"and not firmware default\n",
				minimum_mv, maximum_mv);
		return CMD_RET_SUCCESS;
	}

	printf("closeavs with [island %d] margin_mv slow %d fast %d"
#if IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X)
		"\n  [max_mv %d min_mv %d (0 means using firmware default)]"
#endif
		" ...\n", island, (short) margin_mv_slow, (short) margin_mv_fast
#if IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X)
		, maximum_mv, minimum_mv
#endif
		);

	res = CloseAVS(island, margin_mv_slow, margin_mv_fast, maximum_mv, minimum_mv);
	printf("%sed, res=%d\n", res ? "fail" : "succeed", res);
#else
	printf("not supported\n");
#endif
	return CMD_RET_SUCCESS;
}

static int
do_pmc_sc_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	int i, res, cmd[4] = { 0 };

	for (i = 0; (i < sizeof(cmd)/sizeof(cmd[0])) && (i+1 < argc); i++)
		cmd[i] = simple_strtol(argv[i+1], NULL, 0);
	
	res = PMCcmd(cmd);
	printf("res=%d rsp=[%08x %08x %08x %08x]\n",
			res, cmd[0], cmd[1], cmd[2], cmd[3]);

	return CMD_RET_SUCCESS;
}

static int
do_pmc_sc_log(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
#if defined(PMC_RAM_BOOT) && (defined(PMC_SHARED_MEMORY) || defined(PMC_IMPL_3_X))
	int log_type = 0;

	if (argc > 1)
		log_type = simple_strtol(argv[1], NULL, 0);
	pmc_log(log_type);
#endif
	return CMD_RET_SUCCESS;
}

static int get_pvtmon_result(int select, int island, int * val)
{
	int res, adc = -1;

	res = GetPVT(select, island, &adc);
	if (!res)
		*val = pmc_convert_pvtmon(select, adc);
	return res;
}

static int
do_pmc_sc_pvtmon(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	int select = 0;
	int island = 0;
	int res, val;

	if (argc > 1)
		select = simple_strtol(argv[1], NULL, 0);

	if (select < 0 || select > 7) {
		printf("invalid value for select, must be in 0 ~ 7\n");
		return CMD_RET_SUCCESS;
	}

	if (argc > 2)
		island = simple_strtol(argv[2], NULL, 0);

	printf("get pvtmon select %d island %d ... ", select, island);
	res = get_pvtmon_result(select, island, &val);
	if (res)
		printf("failed, res=%d\n", res);
	else
		printf("value %d.%03d %c\n",
			val/1000, (val > 0 ? val : -val)%1000,
			select ? 'V' : 'C');
	return CMD_RET_SUCCESS;
}

static int
do_pmc_sc_tracktemp(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	char *p = argc > 1 ? argv[1] : NULL;
	int res, status = 1;

	if (!p || !strcmp(p, "status")) {
		res = pmc_get_tracktemp(&status);
		
		if (res)
			printf("failed to get tracktemp status, res=%d\n", res);
		else
			printf("tracktemp is %s\n", status ? "on" : "off");

		return CMD_RET_SUCCESS;
	}
	
	if (!strcmp(p, "off")) status = 0;

	res = pmc_set_tracktemp(status);
	if (res)
		printf("failed to set tracktemp %s, res=%d\n",
				status ? "on" : "off", res);
	return CMD_RET_SUCCESS;
}

static char pmc_usage[] =
"{avs | closeavs | cmd | log | pvtmon | tracktemp}\n"
"    - avs [enable | disable | show]\n"
"    - closeavs [<island>] <margin_mv_slow> <margin_mv_fast>"
#if IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X)
				" [<maximum_mv> <minimum_mv>]"
#endif
				"\n"
"    - cmd <cmdnum> [<param1> [<param2> [<param3>]]]\n"
"    - log [<logtype>]\n"
"    - pvtmon [<select> [<island>]]\n"
"    - tracktemp [status | on | off]\n"
;

U_BOOT_CMD_WITH_SUBCMDS(pmc, "Broadcom pmc commands", pmc_usage,
	U_BOOT_SUBCMD_MKENT(avs, 2, 0, do_pmc_sc_avs),
	U_BOOT_SUBCMD_MKENT(closeavs, 6, 0, do_pmc_sc_closeavs),
	U_BOOT_SUBCMD_MKENT(cmd, 5, 0, do_pmc_sc_cmd),
	U_BOOT_SUBCMD_MKENT(log, 2, 0, do_pmc_sc_log),
	U_BOOT_SUBCMD_MKENT(pvtmon, 3, 0, do_pmc_sc_pvtmon),
	U_BOOT_SUBCMD_MKENT(tracktemp, 2, 0, do_pmc_sc_tracktemp));
