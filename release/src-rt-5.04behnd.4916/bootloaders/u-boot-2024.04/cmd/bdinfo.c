// SPDX-License-Identifier: GPL-2.0+
/*
 * Implements the 'bd' command to show board information
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <getopt.h>
#include <lmb.h>
#include <mapmem.h>
#include <net.h>
#include <serial.h>
#include <video.h>
#include <vsprintf.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <display_options.h>

DECLARE_GLOBAL_DATA_PTR;

void bdinfo_print_size(const char *name, uint64_t size)
{
	printf("%-12s= ", name);
	print_size(size, "\n");
}

void bdinfo_print_str(const char *name, const char *str)
{
	printf("%-12s= %s\n", name, str);
}

void bdinfo_print_num_l(const char *name, ulong value)
{
	printf("%-12s= 0x%0*lx\n", name, 2 * (int)sizeof(value), value);
}

void bdinfo_print_num_ll(const char *name, unsigned long long value)
{
	printf("%-12s= 0x%.*llx\n", name, 2 * (int)sizeof(ulong), value);
}

static void print_eth(void)
{
	const int idx = eth_get_dev_index();
	uchar enetaddr[6];
	char name[10];
	int ret;

	if (idx)
		sprintf(name, "eth%iaddr", idx);
	else
		strcpy(name, "ethaddr");

	ret = eth_env_get_enetaddr_by_index("eth", idx, enetaddr);

	printf("current eth = %s\n", eth_get_name());
	if (!ret)
		printf("%-12s= (not set)\n", name);
	else
		printf("%-12s= %pM\n", name, enetaddr);
	printf("IP addr     = %s\n", env_get("ipaddr"));
}

void bdinfo_print_mhz(const char *name, unsigned long hz)
{
	char buf[32];

	printf("%-12s= %6s MHz\n", name, strmhz(buf, hz));
}

static void print_bi_dram(const struct bd_info *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			bdinfo_print_num_l("DRAM bank",	i);
			bdinfo_print_num_ll("-> start",	bd->bi_dram[i].start);
			bdinfo_print_num_ll("-> size",	bd->bi_dram[i].size);
		}
	}
}

__weak void arch_print_bdinfo(void)
{
}

static void show_video_info(void)
{
	const struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_VIDEO, dev, uc) {
		printf("%-12s= %s %sactive\n", "Video", dev->name,
		       device_active(dev) ? "" : "in");
		if (device_active(dev)) {
			struct video_priv *upriv = dev_get_uclass_priv(dev);
			struct video_uc_plat *plat = dev_get_uclass_plat(dev);

			bdinfo_print_num_ll("FB base", (ulong)upriv->fb);
			if (upriv->copy_fb) {
				bdinfo_print_num_ll("FB copy",
						    (ulong)upriv->copy_fb);
				bdinfo_print_num_l(" copy size",
						   plat->copy_size);
			}
			printf("%-12s= %dx%dx%d\n", "FB size", upriv->xsize,
			       upriv->ysize, 1 << upriv->bpix);
		}
	}
}

static void print_serial(struct udevice *dev)
{
	struct serial_device_info info;
	int ret;

	if (!dev || !IS_ENABLED(CONFIG_DM_SERIAL))
		return;

	ret = serial_getinfo(dev, &info);
	if (ret)
		return;

	bdinfo_print_num_l("serial addr", info.addr);
	bdinfo_print_num_l(" width", info.reg_width);
	bdinfo_print_num_l(" shift", info.reg_shift);
	bdinfo_print_num_l(" offset", info.reg_offset);
	bdinfo_print_num_l(" clock", info.clock);
}

static int bdinfo_print_all(struct bd_info *bd)
{
#ifdef DEBUG
	bdinfo_print_num_l("bd address", (ulong)bd);
#endif
	bdinfo_print_num_l("boot_params", (ulong)bd->bi_boot_params);
	print_bi_dram(bd);
	if (IS_ENABLED(CONFIG_SYS_HAS_SRAM)) {
		bdinfo_print_num_l("sramstart", (ulong)bd->bi_sramstart);
		bdinfo_print_num_l("sramsize", (ulong)bd->bi_sramsize);
	}
	bdinfo_print_num_l("flashstart", (ulong)bd->bi_flashstart);
	bdinfo_print_num_l("flashsize", (ulong)bd->bi_flashsize);
	bdinfo_print_num_l("flashoffset", (ulong)bd->bi_flashoffset);
	printf("baudrate    = %u bps\n", gd->baudrate);
	bdinfo_print_num_l("relocaddr", gd->relocaddr);
	bdinfo_print_num_l("reloc off", gd->reloc_off);
	printf("%-12s= %u-bit\n", "Build", (uint)sizeof(void *) * 8);
	if (IS_ENABLED(CONFIG_CMD_NET))
		print_eth();
	bdinfo_print_num_l("fdt_blob", (ulong)map_to_sysmem(gd->fdt_blob));
	bdinfo_print_num_l("new_fdt", (ulong)map_to_sysmem(gd->new_fdt));
	bdinfo_print_num_l("fdt_size", (ulong)gd->fdt_size);
	if (IS_ENABLED(CONFIG_VIDEO))
		show_video_info();
#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
	bdinfo_print_num_l("multi_dtb_fit", (ulong)gd->multi_dtb_fit);
#endif
	if (IS_ENABLED(CONFIG_LMB) && gd->fdt_blob) {
		struct lmb lmb;

		lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);
		lmb_dump_all_force(&lmb);
		if (IS_ENABLED(CONFIG_OF_REAL))
			printf("devicetree  = %s\n", fdtdec_get_srcname());
	}
	print_serial(gd->cur_serial_dev);

	if (IS_ENABLED(CONFIG_CMD_BDINFO_EXTRA)) {
		bdinfo_print_num_ll("stack ptr", (ulong)&bd);
		bdinfo_print_num_ll("ram_top ptr", (ulong)gd->ram_top);
		bdinfo_print_num_l("malloc base", gd_malloc_start());
	}

	arch_print_bdinfo();

	return 0;
}

int do_bdinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct bd_info *bd = gd->bd;
	struct getopt_state gs;
	int opt;

	if (!CONFIG_IS_ENABLED(GETOPT) || argc == 1)
		return bdinfo_print_all(bd);

	getopt_init_state(&gs);
	while ((opt = getopt(&gs, argc, argv, "aem")) > 0) {
		switch (opt) {
		case 'a':
			return bdinfo_print_all(bd);
		case 'e':
			if (!IS_ENABLED(CONFIG_CMD_NET))
				return CMD_RET_USAGE;
			print_eth();
			return CMD_RET_SUCCESS;
		case 'm':
			print_bi_dram(bd);
			return CMD_RET_SUCCESS;
		default:
			return CMD_RET_USAGE;
		}
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	bdinfo,	2,	1,	do_bdinfo,
	"print Board Info structure",
	""
);
