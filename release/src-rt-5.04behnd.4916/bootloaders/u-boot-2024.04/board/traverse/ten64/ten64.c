// SPDX-License-Identifier: GPL-2.0+
/*
 * Traverse Ten64 Family board
 * Copyright 2017-2018 NXP
 * Copyright 2019-2021 Traverse Technologies
 */
#include <common.h>
#include <display_options.h>
#include <dm/uclass.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <log.h>
#include <malloc.h>
#include <errno.h>
#include <misc.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fsl_ddr.h>
#include <fsl_sec.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <fsl-mc/fsl_mc.h>
#include <env_internal.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <hwconfig.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <nvme.h>

#include <fsl_immap.h>

#include "../common/ten64-controller.h"

#define I2C_RETIMER_ADDR		0x27

DECLARE_GLOBAL_DATA_PTR;

static int ten64_read_board_info(struct t64uc_board_info *);
static void ten64_set_macaddrs_from_board_info(struct t64uc_board_info *);
static void ten64_board_retimer_ds110df410_init(void);

enum {
	TEN64_BOARD_REV_A = 0xFF,
	TEN64_BOARD_REV_B = 0xFE,
	TEN64_BOARD_REV_C = 0xFD,
	TEN64_BOARD_REV_D = 0xFC,
	TEN64_BOARD_MAX
};

#define RESV_MEM_IN_BANK(b)	(gd->arch.resv_ram >= base[b] && \
				 gd->arch.resv_ram < base[b] + size[b])

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();
	return 0;
}

static u32 ten64_get_board_rev(void)
{
	struct ccsr_gur *dcfg = (void *)CFG_SYS_FSL_GUTS_ADDR;
	u32 board_rev_in = in_le32(&dcfg->gpporcr1);
	return board_rev_in;
}

int checkboard(void)
{
	enum boot_src src = get_boot_src();
	char boardmodel[32];
	struct t64uc_board_info boardinfo;
	u32 board_rev = ten64_get_board_rev();

	switch (board_rev) {
	case TEN64_BOARD_REV_A:
		snprintf(boardmodel, 32, "A (Alpha)");
		break;
	case TEN64_BOARD_REV_B:
		snprintf(boardmodel, 32, "B (Beta)");
		break;
	case TEN64_BOARD_REV_C:
		snprintf(boardmodel, 32, "C");
		break;
	case TEN64_BOARD_REV_D:
		snprintf(boardmodel, 32, "D");
		break;
	default:
		snprintf(boardmodel, 32, " Revision %X", (0xFF - board_rev));
		break;
	}

	printf("Board: 1064-0201%s, boot from ", boardmodel);

	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD card\n");
	else if (src == BOOT_SOURCE_QSPI_NOR)
		puts("QSPI\n");
	else
		printf("Unknown boot source %d\n", src);

	puts("Controller: ");
	if (IS_ENABLED(CONFIG_TEN64_CONTROLLER)) {
		/* Driver not compatible with alpha/beta board MCU firmware */
		if (board_rev <= TEN64_BOARD_REV_C) {
			if (ten64_read_board_info(&boardinfo)) {
				puts("ERROR: unable to communicate\n");
			} else {
				printf("firmware %d.%d.%d\n",
				       boardinfo.fwversion_major,
				       boardinfo.fwversion_minor,
				       boardinfo.fwversion_patch);
				ten64_set_macaddrs_from_board_info(&boardinfo);
			}
		} else {
			puts("not supported on this board revision\n");
		}
	} else {
		puts("driver not enabled (no MAC addresses or other information will be read)\n");
	}

	return 0;
}

int board_init(void)
{
	init_final_memctl_regs();

	if (IS_ENABLED(CONFIG_FSL_CAAM))
		sec_init();

	return 0;
}

int fsl_initdram(void)
{
	gd->ram_size = tfa_get_dram_size();

	if (!gd->ram_size)
		gd->ram_size = fsl_ddr_sdram_size();

	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
}

void board_quiesce_devices(void)
{
	if (IS_ENABLED(CONFIG_FSL_MC_ENET))
		fsl_mc_ldpaa_exit(gd->bd);
}

void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	if (offset < 0) {
		printf("%s: ERROR: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	/* In the U-Boot FDT, a 'simple-mfd' compatible is added.
	 * Remove this as FreeBSD will only match "fsl,qoriq-mc"
	 * exactly on the DPAA2 bus/MC node.
	 */
	fdt_setprop(fdt, offset, "compatible", "fsl,qoriq-mc", 12);

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0))
		fdt_status_okay(fdt, offset);
	else
		fdt_status_fail(fdt, offset);
}

/* Called after SoC board_late_init in fsl-layerscape/soc.c */
int fsl_board_late_init(void)
{
	ten64_board_retimer_ds110df410_init();

	/* Ensure nvme storage devices are available to bootflow */
	if (IS_ENABLED(CONFIG_NVME))
		nvme_scan_namespace();

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int i;
	u16 mc_memory_bank = 0;

	u64 *base;
	u64 *size;
	u64 mc_memory_base = 0;
	u64 mc_memory_size = 0;
	u16 total_memory_banks;

	debug("%s blob=0x%p\n", __func__, blob);

	ft_cpu_setup(blob, bd);

	fdt_fixup_mc_ddr(&mc_memory_base, &mc_memory_size);

	if (mc_memory_base != 0)
		mc_memory_bank++;

	total_memory_banks = CONFIG_NR_DRAM_BANKS + mc_memory_bank;

	base = calloc(total_memory_banks, sizeof(u64));
	size = calloc(total_memory_banks, sizeof(u64));

	/* fixup DT for the two GPP DDR banks */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
		/* reduce size if reserved memory is within this bank */
		if (IS_ENABLED(CONFIG_RESV_RAM) && RESV_MEM_IN_BANK(i))
			size[i] = gd->arch.resv_ram - base[i];
	}

	if (mc_memory_base != 0) {
		for (i = 0; i <= total_memory_banks; i++) {
			if (base[i] == 0 && size[i] == 0) {
				base[i] = mc_memory_base;
				size[i] = mc_memory_size;
				break;
			}
		}
	}

	fdt_fixup_memory_banks(blob, base, size, total_memory_banks);

	fdt_fsl_mc_fixup_iommu_map_entry(blob);

	if (IS_ENABLED(CONFIG_FSL_MC_ENET))
		fdt_fixup_board_enet(blob);

	fdt_fixup_icid(blob);

	return 0;
}

#define MACADDRBITS(a, b) (u8)(((a) >> (b)) & 0xFF)

/** Probe and return a udevice for the Ten64 board microcontroller.
 * Optionally, return the I2C bus the microcontroller resides on
 * @i2c_bus_out: return I2C bus device handle in this pointer
 */
static int ten64_get_micro_udevice(struct udevice **ucdev, struct udevice **i2c_bus_out)
{
	int ret;
	struct udevice *i2cbus;

	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &i2cbus);
	if (ret) {
		printf("%s: Could not get I2C UCLASS", __func__);
		return ret;
	}
	if (i2c_bus_out)
		*i2c_bus_out = i2cbus;

	ret = dm_i2c_probe(i2cbus, 0x7E, DM_I2C_CHIP_RD_ADDRESS, ucdev);
	if (ret) {
		printf("%s: Could not get microcontroller device\n", __func__);
		return ret;
	}
	return ret;
}

static int ten64_read_board_info(struct t64uc_board_info *boardinfo)
{
	struct udevice *ucdev;
	int ret;

	ret = ten64_get_micro_udevice(&ucdev, NULL);
	if (ret)
		return ret;

	ret = misc_call(ucdev, TEN64_CNTRL_GET_BOARD_INFO, NULL, 0, (void *)boardinfo, 0);
	if (ret)
		return ret;

	return 0;
}

static void ten64_set_macaddrs_from_board_info(struct t64uc_board_info *boardinfo)
{
	char ethaddr[18];
	char enetvar[10];
	char serial[18];
	u8 intfidx, this_dpmac_num;
	u64 macaddr = 0;
	/* We will copy the MAC address returned from the
	 * uC (48 bits) into the u64 macaddr
	 */
	u8 *macaddr_bytes = (u8 *)&macaddr + 2;

	/** MAC addresses are allocated in order of the physical port numbers,
	 * DPMAC7->10 is "eth0" through "eth3"
	 * DPMAC3->6 is "eth4" through "eth7"
	 * DPMAC2 and 1 are "eth8" and "eth9" respectively
	 */
	int allocation_order[10] = {7, 8, 9, 10, 3, 4, 5, 6, 2, 1};

	memcpy(macaddr_bytes, boardinfo->mac, 6);
	/* MAC address bytes from uC are in big endian,
	 * convert to CPU
	 */
	macaddr = __be64_to_cpu(macaddr);

	/* Set serial# to GE0/DPMAC7 MAC address
	 * (Matches the labels on the board and appliance)
	 */
	snprintf(serial, 18, "%02X%02X%02X%02X%02X%02X",
		 MACADDRBITS(macaddr, 40),
		 MACADDRBITS(macaddr, 32),
		 MACADDRBITS(macaddr, 24),
		 MACADDRBITS(macaddr, 16),
		 MACADDRBITS(macaddr, 8),
		 MACADDRBITS(macaddr, 0));
	if (!env_get("serial#"))
		env_set("serial#", serial);

	for (intfidx = 0; intfidx < 10; intfidx++) {
		snprintf(ethaddr, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
			 MACADDRBITS(macaddr, 40),
			 MACADDRBITS(macaddr, 32),
			 MACADDRBITS(macaddr, 24),
			 MACADDRBITS(macaddr, 16),
			 MACADDRBITS(macaddr, 8),
			 MACADDRBITS(macaddr, 0));

		this_dpmac_num = allocation_order[intfidx];
		printf("DPMAC%d: %s\n", this_dpmac_num, ethaddr);
		snprintf(enetvar, 10,
			 (intfidx != 0) ? "eth%daddr" : "ethaddr",
			 intfidx);
		macaddr++;

		if (!env_get(enetvar))
			env_set(enetvar, ethaddr);
	}
}

/* The retimer (DS110DF410) is one of the devices without
 * a RESET line, but a power switch is on the board
 * allowing it to be reset via uC command
 */
static int board_cycle_retimer(struct udevice **retim_dev)
{
	int ret;
	u8 loop;
	struct udevice *uc_dev;
	struct udevice *i2cbus;
	u32 board_rev = ten64_get_board_rev();

	ret = ten64_get_micro_udevice(&uc_dev, &i2cbus);
	if (ret)
		return ret;

	/* Retimer power cycle not implemented on early board
	 * revisions/controller firmwares
	 */
	if (IS_ENABLED(CONFIG_TEN64_CONTROLLER) &&
	    board_rev <= TEN64_BOARD_REV_C) {
		ret = dm_i2c_probe(i2cbus, I2C_RETIMER_ADDR, 0, retim_dev);
		if (ret == 0) {
			puts("(retimer on, resetting...) ");

			ret = misc_call(uc_dev, TEN64_CNTRL_10G_OFF, NULL, 0, NULL, 0);
			if (ret)
				return ret;
			mdelay(1000);
		}

		/* Turn on the retimer */
		ret = misc_call(uc_dev, TEN64_CNTRL_10G_ON, NULL, 0, NULL, 0);
		if (ret)
			return ret;
	}

	// Wait for retimer to come back
	for (loop = 0; loop < 5; loop++) {
		ret = dm_i2c_probe(i2cbus, I2C_RETIMER_ADDR, 0, retim_dev);
		if (ret == 0)
			return 0;
		mdelay(500);
	}

	return -ENOSYS;
}

/* ten64_board_retimer_ds110df410_init() - Configure the 10G retimer
 * Adopted from the t102xqds board file
 */
static void ten64_board_retimer_ds110df410_init(void)
{
	u8 reg;
	int ret;
	struct udevice *retim_dev;

	puts("Retimer: ");

	ret = board_cycle_retimer(&retim_dev);
	if (ret) {
		puts("Retimer power on failed\n");
		return;
	}

	/* Access to Control/Shared register */
	reg = 0x0;

	ret = dm_i2c_write(retim_dev, 0xff, &reg, 1);
	if (ret) {
		printf("Error writing to retimer register (error %d)\n", ret);
		return;
	}

	/* Read device revision and ID */
	dm_i2c_read(retim_dev, 1, &reg, 1);
	if (reg == 0xF0)
		puts("DS110DF410 found\n");
	else
		printf("Unknown retimer 0x%xn\n", reg);

	/* Enable Broadcast */
	reg = 0x0c;
	dm_i2c_write(retim_dev, 0xff, &reg, 1);

	/* Perform a full reset (state, channel and clock)
	 * for all channels
	 * as the DS110DF410 does not have a RESET line
	 */
	dm_i2c_read(retim_dev, 0, &reg, 1);
	reg |= 0x7;
	dm_i2c_write(retim_dev, 0, &reg, 1);

	/* Set rate/subrate = 0 */
	reg = 0x6;
	dm_i2c_write(retim_dev, 0x2F, &reg, 1);

	/* Set data rate as 10.3125 Gbps */
	reg = 0x0;
	dm_i2c_write(retim_dev, 0x60, &reg, 1);
	reg = 0xb2;
	dm_i2c_write(retim_dev, 0x61, &reg, 1);
	reg = 0x90;
	dm_i2c_write(retim_dev, 0x62, &reg, 1);
	reg = 0xb3;
	dm_i2c_write(retim_dev, 0x63, &reg, 1);
	reg = 0xff;
	dm_i2c_write(retim_dev, 0x64, &reg, 1);

	/* Invert channel 2 (Lower SFP TX to CPU) due to the SFP being inverted */
	reg = 0x05;
	dm_i2c_write(retim_dev, 0xFF, &reg, 1);
	dm_i2c_read(retim_dev, 0x1F, &reg, 1);
	reg |= 0x80;
	dm_i2c_write(retim_dev, 0x1F, &reg, 1);

	puts("OK\n");
}

/* Opt out of the fsl_setenv_bootcmd
 * in arch/arm/cpu/armv8/fsl-layerscape/soc.c
 * which is invoked by board_late_init.
 */
int fsl_setenv_bootcmd(void)
{
	return 0;
}

