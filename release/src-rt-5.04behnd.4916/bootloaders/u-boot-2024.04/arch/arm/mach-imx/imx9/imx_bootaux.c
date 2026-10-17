// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <imx_sip.h>
#include <linux/arm-smccc.h>

int arch_auxiliary_core_check_up(u32 core_id)
{
	struct arm_smccc_res res;

	arm_smccc_smc(IMX_SIP_SRC, IMX_SIP_SRC_MCU_STARTED, 0, 0,
		      0, 0, 0, 0, &res);

	return res.a0;
}

int arch_auxiliary_core_down(u32 core_id)
{
	struct arm_smccc_res res;

	printf("## Stopping auxiliary core\n");

	arm_smccc_smc(IMX_SIP_SRC, IMX_SIP_SRC_MCU_STOP, 0, 0,
		      0, 0, 0, 0, &res);

	return 0;
}

int arch_auxiliary_core_up(u32 core_id, ulong addr)
{
	struct arm_smccc_res res;

	if (!addr)
		return -EINVAL;

	printf("## Starting auxiliary core addr = 0x%08lX...\n", addr);

	arm_smccc_smc(IMX_SIP_SRC, IMX_SIP_SRC_MCU_START, addr, 0,
		      0, 0, 0, 0, &res);

	return 0;
}

/*
 * To i.MX6SX and i.MX7D, the image supported by bootaux needs
 * the reset vector at the head for the image, with SP and PC
 * as the first two words.
 *
 * Per the cortex-M reference manual, the reset vector of M4/M7 needs
 * to exist at 0x0 (TCMUL/IDTCM). The PC and SP are the first two addresses
 * of that vector.  So to boot M4/M7, the A core must build the M4/M7's reset
 * vector with getting the PC and SP from image and filling them to
 * TCMUL/IDTCM. When M4/M7 is kicked, it will load the PC and SP by itself.
 * The TCMUL/IDTCM is mapped to (MCU_BOOTROM_BASE_ADDR) at A core side for
 * accessing the M4/M7 TCMUL/IDTCM.
 */
static int do_bootaux(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	ulong addr;
	int ret, up;
	u32 core = 0;
	u32 stop = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc > 2)
		core = simple_strtoul(argv[2], NULL, 10);

	if (argc > 3)
		stop = simple_strtoul(argv[3], NULL, 10);

	up = arch_auxiliary_core_check_up(core);
	if (up) {
		printf("## Auxiliary core is already up\n");
		return CMD_RET_SUCCESS;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	if (!addr)
		return CMD_RET_FAILURE;

	ret = arch_auxiliary_core_up(core, addr);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_stopaux(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int ret, up;

	up = arch_auxiliary_core_check_up(0);
	if (!up) {
		printf("## Auxiliary core is already down\n");
		return CMD_RET_SUCCESS;
	}

	ret = arch_auxiliary_core_down(0);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	stopaux, CONFIG_SYS_MAXARGS, 1,	do_stopaux,
	"Stop auxiliary core",
	"<address> [<core>]\n"
	"   - start auxiliary core [<core>] (default 0),\n"
	"     at address <address>\n"
);

U_BOOT_CMD(
	bootaux, CONFIG_SYS_MAXARGS, 1,	do_bootaux,
	"Start auxiliary core",
	"<address> [<core>]\n"
	"   - start auxiliary core [<core>] (default 0),\n"
	"     at address <address> of auxiliary core view\n"
);
