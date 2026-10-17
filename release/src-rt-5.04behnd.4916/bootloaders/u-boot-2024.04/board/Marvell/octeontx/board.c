// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <dm.h>
#include <malloc.h>
#include <errno.h>
#include <env.h>
#include <init.h>
#include <log.h>
#include <netdev.h>
#include <pci_ids.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/smc.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

void octeontx_cleanup_ethaddr(void)
{
	char ename[32];

	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if (env_get(ename))
			env_set(ename, NULL);
	}
}

int octeontx_board_has_pmp(void)
{
	return (otx_is_board("sff8104") || otx_is_board("nas8104"));
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_NET_OCTEONTX))
		fdt_parse_phy_info();

	return 0;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = smc_dram_size(0);
	gd->ram_size -= CFG_SYS_SDRAM_BASE;
	mem_map_fill();

	return 0;
}

void board_late_probe_devices(void)
{
	struct udevice *dev;
	int err, bgx_cnt, i;

	/* Probe MAC(BGX) and NIC PF devices before Network stack init */
	bgx_cnt = otx_is_soc(CN81XX) ? 2 : 4;
	for (i = 0; i < bgx_cnt; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
					 PCI_DEVICE_ID_CAVIUM_BGX, i, &dev);
		if (err)
			debug("%s BGX%d device not found\n", __func__, i);
	}
	if (otx_is_soc(CN81XX)) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
					 PCI_DEVICE_ID_CAVIUM_RGX, 0, &dev);
		if (err)
			debug("%s RGX device not found\n", __func__);
	}
	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
				 PCI_DEVICE_ID_CAVIUM_NIC, 0, &dev);
	if (err)
		debug("NIC PF device not found\n");
}

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[32];
	char boardserial[150], boardrev[150];
	bool save_env = false;
	const char *str;

	/*
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of network interfaces can change.
	 */
	octeontx_cleanup_ethaddr();

	snprintf(boardname, sizeof(boardname), "%s> ", fdt_get_board_model());
	env_set("prompt", boardname);

	set_working_fdt_addr(env_get_hex("fdtcontroladdr", fdt_base_addr));

	str = fdt_get_board_revision();
	if (str) {
		snprintf(boardrev, sizeof(boardrev), "%s", str);
		if (env_get("boardrev") &&
		    strcmp(boardrev, env_get("boardrev")))
			save_env = true;
		env_set("boardrev", boardrev);
	}

	str = fdt_get_board_serial();
	if (str) {
		snprintf(boardserial, sizeof(boardserial), "%s", str);
		if (env_get("serial#") &&
		    strcmp(boardserial, env_get("serial#")))
			save_env = true;
		env_set("serial#", boardserial);
	}

	if (IS_ENABLED(CONFIG_NET_OCTEONTX))
		board_late_probe_devices();

	if (save_env)
		env_save();

	return 0;
}

/*
 * Invoked before relocation, so limit to stack variables.
 */
int checkboard(void)
{
	printf("Board: %s\n", fdt_get_board_model());

	return 0;
}
