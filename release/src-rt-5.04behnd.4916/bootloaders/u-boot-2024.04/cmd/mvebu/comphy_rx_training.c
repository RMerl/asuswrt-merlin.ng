// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <fdtdec.h>
#include <dm/device-internal.h>
#include <mvebu/comphy.h>

int mvebu_comphy_rx_training_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
				 char * const argv[])
{
	struct udevice *dev;
	struct uclass *uc;
	int ret, cp_index, comphy_index, i = 0;

	if (argc != 3) {
		printf("missing arguments\n");
		return CMD_RET_USAGE;
	}

	cp_index = hextoul(argv[1], NULL);
	comphy_index = hextoul(argv[2], NULL);

	ret = uclass_get(UCLASS_MISC, &uc);
	if (ret) {
		printf("Couldn't find UCLASS_MISC\n");
		return ret;
	}

	uclass_foreach_dev(dev, uc) {
		if (!(memcmp(dev->name, "comphy", 5))) {
			if (i == cp_index) {
				comphy_rx_training(dev, comphy_index);
				return 0;
			}

			i++;
		}
	}

	printf("Coudn't find comphy %d\n", cp_index);

	return 0;
}

U_BOOT_CMD(
	mvebu_comphy_rx_training, 3, 0, mvebu_comphy_rx_training_cmd,
	"mvebu_comphy_rx_training <cp id> <comphy id>\n",
	"\n\tRun COMPHY RX training sequence, the user must state CP index (0/1) and comphy ID (0/5)"
);
