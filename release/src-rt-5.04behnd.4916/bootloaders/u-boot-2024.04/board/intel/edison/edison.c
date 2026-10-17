// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 */
#include <common.h>
#include <env.h>
#include <init.h>
#include <mmc.h>
#include <u-boot/md5.h>

#include <asm/cache.h>
#include <asm/pmu.h>
#include <asm/scu.h>
#include <asm/u-boot-x86.h>

/* List of Intel Tangier LSSs */
#define PMU_LSS_TANGIER_SDIO0_01	1

int board_early_init_r(void)
{
	pmu_turn_power(PMU_LSS_TANGIER_SDIO0_01, true);
	return 0;
}

static void assign_serial(void)
{
	struct mmc *mmc = find_mmc_device(0);
	unsigned char ssn[16];
	char usb0addr[18];
	char serial[33];
	int i;

	if (!mmc)
		return;

	md5((unsigned char *)mmc->cid, sizeof(mmc->cid), ssn);

	snprintf(usb0addr, sizeof(usb0addr), "02:00:86:%02x:%02x:%02x",
		 ssn[13], ssn[14], ssn[15]);
	env_set("usb0addr", usb0addr);

	for (i = 0; i < 16; i++)
		snprintf(&serial[2 * i], 3, "%02x", ssn[i]);
	env_set("serial#", serial);

#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_ENV_IS_NOWHERE)
	env_save();
#endif
}

static void assign_hardware_id(void)
{
	struct ipc_ifwi_version v;
	char hardware_id[4];
	int ret;

	ret = scu_ipc_command(IPCMSG_GET_FW_REVISION, 1, NULL, 0, (u32 *)&v, 4);
	if (ret < 0)
		printf("Can't retrieve hardware revision\n");

	snprintf(hardware_id, sizeof(hardware_id), "%02X", v.hardware_id);
	env_set("hardware_id", hardware_id);

#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_ENV_IS_NOWHERE)
	env_save();
#endif
}

int board_late_init(void)
{
	if (!env_get("serial#"))
		assign_serial();

	if (!env_get("hardware_id"))
		assign_hardware_id();

	return 0;
}
