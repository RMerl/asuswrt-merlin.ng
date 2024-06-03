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

#if defined(PMC_ON_HOSTCPU) || defined(PMC_DIRECT_MODE_ONLY)
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
#elif IS_BCMCHIP(6837) || IS_BCMCHIP(68880)
int getAVSConfig(void)
{
    char *current = env_get(ENV_AVS_DISABLE);
    int result = 0;

    if (current)
    {
        if (!strcmp(current, "true"))
            result = 1;
    }

    return result;
}

static inline void setAVSConfig(int new)
{
	int old = getAVSConfig();

	if (old == new) return;

	if (!env_set(ENV_AVS_DISABLE, new ? "true" : NULL) && !env_save())
		printf("Reboot to use new " ENV_AVS_DISABLE "=%s\n", new ? "true":"false");
}
#define ENV_CPU_PROFILE "low_perf_profile"
#define ENV_AVS_TEST_MODE "avs_test_mode"
#define ENV_AVS_CORE_MARGIN_HI "avs_core_margin_hi"
#define ENV_AVS_CORE_MARGIN_LO "avs_core_margin_lo"
#define ENV_AVS_CPU_MARGIN_HI "avs_cpu_margin_hi"
#define ENV_AVS_CPU_MARGIN_LO "avs_cpu_margin_lo"
#define ENV_AVS_CORE_MIN_VOLTAGE "avs_core_min_voltage"
#define ENV_AVS_CORE_MAX_VOLTAGE "avs_core_max_voltage"
#define ENV_AVS_CPU_MIN_VOLTAGE "avs_cpu_min_voltage"
#define ENV_AVS_CPU_MAX_VOLTAGE "avs_cpu_max_voltage"

static int
do_pmc_sc_avsrestore(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
    env_set(ENV_AVS_DISABLE, NULL);
    env_set(ENV_CPU_PROFILE, NULL);
    env_set(ENV_AVS_TEST_MODE, NULL);
    env_set(ENV_AVS_CORE_MARGIN_HI, NULL);
    env_set(ENV_AVS_CORE_MARGIN_LO, NULL);
    env_set(ENV_AVS_CPU_MARGIN_HI, NULL);
    env_set(ENV_AVS_CPU_MARGIN_LO, NULL);
    env_set(ENV_AVS_CORE_MIN_VOLTAGE, NULL);
    env_set(ENV_AVS_CORE_MAX_VOLTAGE, NULL);
    env_set(ENV_AVS_CPU_MIN_VOLTAGE, NULL);
    env_set(ENV_AVS_CPU_MAX_VOLTAGE, NULL);
    env_save();
    return CMD_RET_SUCCESS;
}

int get_avstrack(void)
{
    char *current = env_get(ENV_AVS_TEST_MODE);
    int result = 1;

    if (current)
    {
        if (!strcmp(current, "true"))
            result = 0;
    }

    return result;
}

static void set_avstrack(int new)
{
	int old = get_avstrack();

	if (old == new) return;

	if (!env_set(ENV_AVS_TEST_MODE, new ? NULL : "true") && !env_save())
		printf("Reboot to use new " ENV_AVS_TEST_MODE "=%s\n", new ? "false":"true");
}

static int
do_pmc_sc_avstrack(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
	static const char *tracking_off[] = {"0", "off"};
	char *p = argc > 1 ? argv[1] : NULL;
	int status = 1;
	int i;

	if (!p || !strcmp(p, "show"))
	{
		status = get_avstrack();
		printf("AVS tracking is %s\n", status ? "on" : "off");
		return CMD_RET_SUCCESS;
	}

	for (i = 0; i < sizeof tracking_off / sizeof tracking_off[0]; i++)
	{
		if (!strcmp(argv[1], tracking_off[i])) 
		{
			status = 0;
			break;
		}
	}

	set_avstrack(status);
	return CMD_RET_SUCCESS;
}

int getCPUConfig(void)
{
    char *current = env_get(ENV_CPU_PROFILE);
    int result = 0;

    if (current)
    {
        if (!strcmp(current, "true"))
            result = 1;
    }

    return result;
}

static void setCPUConfig(int new)
{
	int old = getCPUConfig();

	if (old == new) return;

	if (!env_set(ENV_CPU_PROFILE, new ? "true" : NULL) && !env_save())
		printf("Reboot to use new " ENV_CPU_PROFILE "=%s\n", new ? "true":"false");
}

static int
do_pmc_sc_cpu_profile(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
static const char *low_profile[] = {"0", "low"};
int res, i;

if (argc < 2) {
    res = getCPUConfig();
    printf("CPU is set to %s performance profile by environment\n", res ? "low" : "high");
    return CMD_RET_SUCCESS;
}

if (!strcmp(argv[1], "show")) {
    res = getCPUConfig();
    printf("CPU is set to %s performance profile\n", res ? "low" : "high");
    return CMD_RET_SUCCESS;
}

res = 0;
for (i = 0; i < sizeof low_profile / sizeof low_profile[0]; i++)
    if (!strcmp(argv[1], low_profile[i])) {
        res = 1;
        break;
    }

setCPUConfig(res);
return CMD_RET_SUCCESS;
}

static int
do_pmc_sc_closeavs(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
char *core_margin_mv_lo = NULL;
char *core_margin_mv_hi = NULL;
char *cpu_margin_mv_lo = NULL;
char *cpu_margin_mv_hi = NULL;
char *core_min_voltage = NULL;
char *core_max_voltage = NULL;
char *cpu_min_voltage = NULL;
char *cpu_max_voltage = NULL;
char *test_mode = NULL;

if ((argc !=1) && (argc != 2) && (argc != 5) && (argc != 9))
{
    printf("invalid number of parameters\n");
    return CMD_RET_SUCCESS;
}

if (argc == 1)
{
    test_mode = env_get(ENV_AVS_TEST_MODE);
    core_margin_mv_lo = env_get(ENV_AVS_CORE_MARGIN_LO);
    core_margin_mv_hi = env_get(ENV_AVS_CORE_MARGIN_HI);
    core_max_voltage = env_get(ENV_AVS_CORE_MAX_VOLTAGE);
    core_min_voltage = env_get(ENV_AVS_CORE_MIN_VOLTAGE);

    cpu_margin_mv_lo = env_get(ENV_AVS_CPU_MARGIN_LO);
    cpu_margin_mv_hi = env_get(ENV_AVS_CPU_MARGIN_HI);
    cpu_max_voltage = env_get(ENV_AVS_CPU_MAX_VOLTAGE);
    cpu_min_voltage = env_get(ENV_AVS_CPU_MIN_VOLTAGE);

    printf("AVS running using the following settings\n");
    printf("Core Margins [%s / %s] Voltage [%smV / %smV]\n", 
        (core_margin_mv_lo == NULL) ? "default": (strcmp(core_margin_mv_lo, "-1") == 0) ? "default" : core_margin_mv_lo, 
        (core_margin_mv_hi == NULL) ? "default": (strcmp(core_margin_mv_hi, "-1") == 0) ? "default" : core_margin_mv_hi,
        (core_min_voltage  == NULL) ? "default": (strcmp(core_min_voltage , "-1") == 0) ? "default" : core_min_voltage, 
        (core_max_voltage  == NULL) ? "default": (strcmp(core_max_voltage , "-1") == 0) ? "default" : core_max_voltage);
    printf("CPU Margins [%s / %s] Voltage [%smV / %smV]\n", 
        (cpu_margin_mv_lo == NULL) ? "default": (strcmp(cpu_margin_mv_lo, "-1") == 0) ? "default" : cpu_margin_mv_lo, 
        (cpu_margin_mv_hi == NULL) ? "default": (strcmp(cpu_margin_mv_hi, "-1") == 0) ? "default" : cpu_margin_mv_hi,
        (cpu_min_voltage  == NULL) ? "default": (strcmp(cpu_min_voltage , "-1") == 0) ? "default" : cpu_min_voltage, 
        (cpu_max_voltage  == NULL) ? "default": (strcmp(cpu_max_voltage , "-1") == 0) ? "default" : cpu_max_voltage);
    printf("AVS Tracking is %s\n", (test_mode == NULL) ? "on": "off");

    return CMD_RET_SUCCESS;
}

if (argc >= 5)
{
    test_mode = "true";
    core_margin_mv_lo = argv[1];
    core_margin_mv_hi = argv[2];
    cpu_margin_mv_lo = argv[3];
    cpu_margin_mv_hi = argv[4];

    if (argc == 9)
    {
        core_min_voltage = argv[5];
        core_max_voltage = argv[6];
        cpu_min_voltage  = argv[7];
        cpu_max_voltage  = argv[8];
    }
    else
    {
        core_min_voltage = "-1";
        core_max_voltage = "-1";
        cpu_min_voltage  = "-1";
        cpu_max_voltage  = "-1";
    }
}

if (!env_set(ENV_AVS_CORE_MARGIN_LO, core_margin_mv_lo) &&
    !env_set(ENV_AVS_CORE_MARGIN_HI, core_margin_mv_hi) &&
    !env_set(ENV_AVS_CORE_MIN_VOLTAGE, core_min_voltage) &&
    !env_set(ENV_AVS_CORE_MAX_VOLTAGE, core_max_voltage) &&
    !env_set(ENV_AVS_CPU_MARGIN_LO, cpu_margin_mv_lo) &&
    !env_set(ENV_AVS_CPU_MARGIN_HI, cpu_margin_mv_hi) &&
    !env_set(ENV_AVS_CPU_MIN_VOLTAGE, cpu_min_voltage) &&
    !env_set(ENV_AVS_CPU_MAX_VOLTAGE, cpu_max_voltage) &&
    !env_set(ENV_AVS_TEST_MODE, test_mode) &&
    !env_save())
{
    if (!test_mode)
    {
        printf("Reboot to close AVS with defaults values\n");
    }
    else
    {
        printf("Reboot to close AVS with new margins Core [%s / %s] Voltage [%smV / %smV] "
            "CPU [%s / %s] Voltage [%smV / %smV]\n",
             core_margin_mv_lo, core_margin_mv_hi,
             (argc == 9) ? core_min_voltage : "default",
             (argc == 9) ? core_max_voltage : "default",
             cpu_margin_mv_lo, cpu_margin_mv_hi,
             (argc == 9) ? cpu_min_voltage  : "default",
             (argc == 9) ? cpu_max_voltage  : "default");
    }
}

return CMD_RET_SUCCESS;
}

__weak int do_pmc_debug_cmd(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
return CMD_RET_SUCCESS;
}

int do_pmc_smclog(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[]);

#else
// return 1 if the environment variable avs_disable=1
int getAVSConfig(void)
{
const char *disable = NULL;
int offset = fdt_path_offset(gd->fdt_blob, "/chosen");

if (offset < 0) return 0;
disable = fdt_getprop(gd->fdt_blob, offset, ENV_AVS_DISABLE, NULL);
if (!disable) return 0;

return *disable == '1';
}

static inline void setAVSConfig(int new)
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
#if IS_BCMCHIP(4908) || IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X) || \
IS_BCMCHIP(6855) || IS_BCMCHIP(6837) || IS_BCMCHIP(68880)
static const char *disables[] = {"0", "disable", "false", "off"};
int res, i;

if (argc < 2) {
    res = getAVSConfig();
    printf("AVS %sdisabled by environment\n", res ? "" : "not ");
    return CMD_RET_SUCCESS;
}

if (!strcmp(argv[1], "show")) {
#if defined(PMC_ON_HOSTCPU) || IS_BCMCHIP(6837) || IS_BCMCHIP(68880)
    res = getAVSConfig();
    printf("AVS is %s \n", res ? "disabled" : "enabled");
#else
{
int pmc[4];
    memset((void*)pmc, 0, sizeof(pmc));
    /* Try to know the AVS disable state */
    pmc[0] = 20; // cmdGetAvsDisableState defined in pmc/command.h
    res = PMCcmd(pmc);
    if (!res)
        printf("AVS %s\n", pmc[2] ? "Disabled" : "Enabled");
    else
        printf("failed to get AVS state, res=%d\n", res);
}
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

#if !IS_BCMCHIP(6837) && !IS_BCMCHIP(68880)
static int
do_pmc_sc_closeavs(cmd_tbl_t * cmdtp, int flag, int argc, char *const argv[])
{
#if IS_BCMCHIP(63138) || IS_BCMCHIP(63148) || IS_BCMCHIP(4908) || \
        IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X) || IS_BCMCHIP(6855)
int res;
uint32_t island = 0;
uint16_t margin_mv_slow=0;
uint16_t margin_mv_fast=0;
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
#ifdef PMC_LOG_IN_DTCM
pmc_show_live_log();
#endif // #ifdef PMC_LOG_IN_DTCM
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
#endif

static char pmc_usage[] =
"{avs | closeavs"
#if !IS_BCMCHIP(6837) && !IS_BCMCHIP(68880)
" | cmd | log | pvtmon | tracktemp"
#else
" | cpu | smclog | trackavs"
#endif
"}\n"
"    - avs [enable | disable | show]\n"
#if !IS_BCMCHIP(6837) && !IS_BCMCHIP(68880)
"    - closeavs [<island>] <margin_mv_slow> <margin_mv_fast>"
#if IS_BCMCHIP(63158) || defined(PMC_IMPL_3_X)
            " [<maximum_mv> <minimum_mv>]"
#endif
            "\n"
"    - cmd <cmdnum> [<param1> [<param2> [<param3>]]]\n"
"    - log [<logtype>]\n"
"    - pvtmon [<select> [<island>]]\n"
"    - tracktemp [status | on | off]\n"
#else
"    - closeavs default | <core_margin_mv_lo> <core_margin_mv_hi> <cpu_margin_mv_lo> <cpu_margin_mv_hi> "
"[<core_min_voltage> <core_max_voltage> <cpu_max_voltage> <cpu_min_voltage>]\n"
"    - cpu [high | low | show]\n"
"    - avstrack [on | off | show]\n"
"    - avsrestore_defaults\n"
"    - smclog\n"
#endif
;

U_BOOT_CMD_WITH_SUBCMDS(pmc, "Broadcom pmc commands", pmc_usage,
U_BOOT_SUBCMD_MKENT(avs, 2, 0, do_pmc_sc_avs),
#if !IS_BCMCHIP(6837) && !IS_BCMCHIP(68880)
U_BOOT_SUBCMD_MKENT(closeavs, 6, 0, do_pmc_sc_closeavs),
U_BOOT_SUBCMD_MKENT(cmd, 5, 0, do_pmc_sc_cmd),
U_BOOT_SUBCMD_MKENT(log, 2, 0, do_pmc_sc_log),
U_BOOT_SUBCMD_MKENT(pvtmon, 3, 0, do_pmc_sc_pvtmon),
U_BOOT_SUBCMD_MKENT(tracktemp, 2, 0, do_pmc_sc_tracktemp)
#else
U_BOOT_SUBCMD_MKENT(closeavs, 9, 0, do_pmc_sc_closeavs),
U_BOOT_SUBCMD_MKENT(cpu, 2, 0, do_pmc_sc_cpu_profile),
	U_BOOT_SUBCMD_MKENT(avstrack, 2, 0, do_pmc_sc_avstrack),
	U_BOOT_SUBCMD_MKENT(avsrestore_defaults, 2, 0, do_pmc_sc_avsrestore),
    U_BOOT_SUBCMD_MKENT(debug_cmd, 3, 0, do_pmc_debug_cmd),
	U_BOOT_SUBCMD_MKENT(smclog, 2, 0, do_pmc_smclog)
#endif
    );
