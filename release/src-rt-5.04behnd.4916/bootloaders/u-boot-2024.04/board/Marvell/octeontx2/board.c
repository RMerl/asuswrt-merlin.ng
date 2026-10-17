// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <command.h>
#include <console.h>
#include <cpu_func.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/uclass-internal.h>
#include <env.h>
#include <event.h>
#include <init.h>
#include <malloc.h>
#include <net.h>
#include <pci_ids.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/arch/smc.h>
#include <asm/arch/soc.h>
#include <asm/arch/board.h>
#include <dm/util.h>

DECLARE_GLOBAL_DATA_PTR;

void cleanup_env_ethaddr(void)
{
	char ename[32];

	for (int i = 0; i < 20; i++) {
		sprintf(ename, i ? "eth%daddr" : "ethaddr", i);
		if (env_get(ename))
			env_set(ename, NULL);
	}
}

void octeontx2_board_get_mac_addr(u8 index, u8 *mac_addr)
{
	u64 tmp_mac, board_mac_addr = fdt_get_board_mac_addr();
	static int board_mac_num;

	board_mac_num = fdt_get_board_mac_cnt();
	if ((!is_zero_ethaddr((u8 *)&board_mac_addr)) && board_mac_num) {
		tmp_mac = board_mac_addr;
		tmp_mac += index;
		tmp_mac = swab64(tmp_mac) >> 16;
		memcpy(mac_addr, (u8 *)&tmp_mac, ARP_HLEN);
		board_mac_num--;
	} else {
		memset(mac_addr, 0, ARP_HLEN);
	}
	debug("%s mac %pM\n", __func__, mac_addr);
}

void board_quiesce_devices(void)
{
	struct uclass *uc_dev;
	int ret;

	/* Removes all RVU PF devices */
	ret = uclass_get(UCLASS_ETH, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove rvu pf devices\n");

	if (IS_ENABLED(CONFIG_OCTEONTX2_CGX_INTF)) {
		/* Bring down all cgx lmac links */
		cgx_intf_shutdown();
	}

	/* Removes all CGX and RVU AF devices */
	ret = uclass_get(UCLASS_MISC, &uc_dev);
	if (uc_dev)
		ret = uclass_destroy(uc_dev);
	if (ret)
		printf("couldn't remove misc (cgx/rvu_af) devices\n");

	/* SMC call - removes all LF<->PF mappings */
	smc_disable_rvu_lfs(0);
}

int board_early_init_r(void)
{
	pci_init();
	return 0;
}

int board_init(void)
{
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
	int err, cgx_cnt = 3, i;

	/* Probe MAC(CGX) and NIC AF devices before Network stack init */
	for (i = 0; i < cgx_cnt; i++) {
		err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
					 PCI_DEVICE_ID_CAVIUM_CGX, i, &dev);
		if (err)
			debug("%s CGX%d device not found\n", __func__, i);
	}
	err = dm_pci_find_device(PCI_VENDOR_ID_CAVIUM,
				 PCI_DEVICE_ID_CAVIUM_RVU_AF, 0, &dev);
	if (err)
		debug("NIC AF device not found\n");
}

/**
 * Board late initialization routine.
 */
int board_late_init(void)
{
	char boardname[32];
	char boardserial[150], boardrev[150];
	long val;
	bool save_env = false;
	const char *str;

	debug("%s()\n", __func__);

	/*
	 * Now that pci_init initializes env device.
	 * Try to cleanup ethaddr env variables, this is needed
	 * as with each boot, configuration of QLM can change.
	 */
	cleanup_env_ethaddr();

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

	val = env_get_hex("disable_ooo", 0);
	smc_configure_ooo(val);

	if (IS_ENABLED(CONFIG_NET_OCTEONTX2))
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

void board_acquire_flash_arb(bool acquire)
{
	union cpc_boot_ownerx ownerx;

	if (!acquire) {
		ownerx.u = readl(CPC_BOOT_OWNERX(3));
		ownerx.s.boot_req = 0;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
	} else {
		ownerx.u = 0;
		ownerx.s.boot_req = 1;
		writel(ownerx.u, CPC_BOOT_OWNERX(3));
		udelay(1);
		do {
			ownerx.u = readl(CPC_BOOT_OWNERX(3));
		} while (ownerx.s.boot_wait);
	}
}

static int last_stage_init(void)
{
	(void)smc_flsf_fw_booted();
	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

static int do_go_uboot(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	typedef void __noreturn (*uboot_entry_t)(ulong, void *);
	uboot_entry_t entry;
	ulong addr;
	void *fdt;
	int err;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);
	fdt = board_fdt_blob_setup(&err);
	entry = (uboot_entry_t)addr;
	flush_cache((ulong)addr, 1 << 20);	/* 1MiB should be enough */
	dcache_disable();

	printf("## Starting U-Boot at %p (FDT at %p)...\n", entry, fdt);

	entry(0, fdt);

	return 0;
}

U_BOOT_CMD(go_uboot, 2, 0, do_go_uboot,
	   "Start U-Boot from RAM (pass FDT via x1 register)",
	   "");
