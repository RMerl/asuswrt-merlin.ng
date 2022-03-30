// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx ZynqMP SoC Tap Delay Programming
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>

#define SD_DLL_CTRL			0xFF180358
#define SD_ITAP_DLY			0xFF180314
#define SD_OTAP_DLY			0xFF180318
#define SD0_DLL_RST_MASK		0x00000004
#define SD0_DLL_RST			0x00000004
#define SD1_DLL_RST_MASK		0x00040000
#define SD1_DLL_RST			0x00040000
#define SD0_ITAPCHGWIN_MASK		0x00000200
#define SD0_ITAPCHGWIN			0x00000200
#define SD1_ITAPCHGWIN_MASK		0x02000000
#define SD1_ITAPCHGWIN			0x02000000
#define SD0_ITAPDLYENA_MASK		0x00000100
#define SD0_ITAPDLYENA			0x00000100
#define SD1_ITAPDLYENA_MASK		0x01000000
#define SD1_ITAPDLYENA			0x01000000
#define SD0_ITAPDLYSEL_MASK		0x000000FF
#define SD0_ITAPDLYSEL_HSD		0x00000015
#define SD0_ITAPDLYSEL_SD_DDR50		0x0000003D
#define SD0_ITAPDLYSEL_MMC_DDR50	0x00000012

#define SD1_ITAPDLYSEL_MASK		0x00FF0000
#define SD1_ITAPDLYSEL_HSD		0x00150000
#define SD1_ITAPDLYSEL_SD_DDR50		0x003D0000
#define SD1_ITAPDLYSEL_MMC_DDR50	0x00120000

#define SD0_OTAPDLYSEL_MASK		0x0000003F
#define SD0_OTAPDLYSEL_MMC_HSD		0x00000006
#define SD0_OTAPDLYSEL_SD_HSD		0x00000005
#define SD0_OTAPDLYSEL_SDR50		0x00000003
#define SD0_OTAPDLYSEL_SDR104_B0	0x00000003
#define SD0_OTAPDLYSEL_SDR104_B2	0x00000002
#define SD0_OTAPDLYSEL_SD_DDR50		0x00000004
#define SD0_OTAPDLYSEL_MMC_DDR50	0x00000006

#define SD1_OTAPDLYSEL_MASK		0x003F0000
#define SD1_OTAPDLYSEL_MMC_HSD		0x00060000
#define SD1_OTAPDLYSEL_SD_HSD		0x00050000
#define SD1_OTAPDLYSEL_SDR50		0x00030000
#define SD1_OTAPDLYSEL_SDR104_B0	0x00030000
#define SD1_OTAPDLYSEL_SDR104_B2	0x00020000
#define SD1_OTAPDLYSEL_SD_DDR50		0x00040000
#define SD1_OTAPDLYSEL_MMC_DDR50	0x00060000

#define MMC_BANK2		0x2

#define MMC_TIMING_UHS_SDR25		1
#define MMC_TIMING_UHS_SDR50		2
#define MMC_TIMING_UHS_SDR104		3
#define MMC_TIMING_UHS_DDR50		4
#define MMC_TIMING_MMC_HS200		5
#define MMC_TIMING_SD_HS		6
#define MMC_TIMING_MMC_DDR52		7
#define MMC_TIMING_MMC_HS		8

void zynqmp_dll_reset(u8 deviceid)
{
	/* Issue DLL Reset */
	if (deviceid == 0)
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK,
				  SD0_DLL_RST);
	else
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK,
				  SD1_DLL_RST);

	mdelay(1);

	/* Release DLL Reset */
	if (deviceid == 0)
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK, 0x0);
	else
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK, 0x0);
}

static void arasan_zynqmp_tap_sdr104(u8 deviceid, u8 timing, u8 bank)
{
	if (deviceid == 0) {
		/* Program OTAP */
		if (bank == MMC_BANK2)
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  SD0_OTAPDLYSEL_SDR104_B2);
		else
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  SD0_OTAPDLYSEL_SDR104_B0);
	} else {
		/* Program OTAP */
		if (bank == MMC_BANK2)
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  SD1_OTAPDLYSEL_SDR104_B2);
		else
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  SD1_OTAPDLYSEL_SDR104_B0);
	}
}

static void arasan_zynqmp_tap_hs(u8 deviceid, u8 timing, u8 bank)
{
	if (deviceid == 0) {
		/* Program ITAP */
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN_MASK,
				  SD0_ITAPCHGWIN);
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYENA_MASK,
				  SD0_ITAPDLYENA);
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYSEL_MASK,
				  SD0_ITAPDLYSEL_HSD);
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN_MASK, 0x0);
		/* Program OTAP */
		if (timing == MMC_TIMING_MMC_HS)
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  SD0_OTAPDLYSEL_MMC_HSD);
		else
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  SD0_OTAPDLYSEL_SD_HSD);
	} else {
		/* Program ITAP */
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN_MASK,
				  SD1_ITAPCHGWIN);
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYENA_MASK,
				  SD1_ITAPDLYENA);
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYSEL_MASK,
				  SD1_ITAPDLYSEL_HSD);
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN_MASK, 0x0);
		/* Program OTAP */
		if (timing == MMC_TIMING_MMC_HS)
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  SD1_OTAPDLYSEL_MMC_HSD);
		else
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  SD1_OTAPDLYSEL_SD_HSD);
	}
}

static void arasan_zynqmp_tap_ddr50(u8 deviceid, u8 timing, u8 bank)
{
	if (deviceid == 0) {
		/* Program ITAP */
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN_MASK,
				  SD0_ITAPCHGWIN);
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYENA_MASK,
				  SD0_ITAPDLYENA);
		if (timing == MMC_TIMING_UHS_DDR50)
			zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYSEL_MASK,
					  SD0_ITAPDLYSEL_SD_DDR50);
		else
			zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPDLYSEL_MASK,
					  SD0_ITAPDLYSEL_MMC_DDR50);
		zynqmp_mmio_write(SD_ITAP_DLY, SD0_ITAPCHGWIN_MASK, 0x0);
		/* Program OTAP */
		if (timing == MMC_TIMING_UHS_DDR50)
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  SD0_OTAPDLYSEL_SD_DDR50);
		else
			zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
					  SD0_OTAPDLYSEL_MMC_DDR50);
	} else {
		/* Program ITAP */
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN_MASK,
				  SD1_ITAPCHGWIN);
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYENA_MASK,
				  SD1_ITAPDLYENA);
		if (timing == MMC_TIMING_UHS_DDR50)
			zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYSEL_MASK,
					  SD1_ITAPDLYSEL_SD_DDR50);
		else
			zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPDLYSEL_MASK,
					  SD1_ITAPDLYSEL_MMC_DDR50);
		zynqmp_mmio_write(SD_ITAP_DLY, SD1_ITAPCHGWIN_MASK, 0x0);
		/* Program OTAP */
		if (timing == MMC_TIMING_UHS_DDR50)
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  SD1_OTAPDLYSEL_SD_DDR50);
		else
			zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
					  SD1_OTAPDLYSEL_MMC_DDR50);
	}
}

static void arasan_zynqmp_tap_sdr50(u8 deviceid, u8 timing, u8 bank)
{
	if (deviceid == 0) {
		/* Program OTAP */
		zynqmp_mmio_write(SD_OTAP_DLY, SD0_OTAPDLYSEL_MASK,
				  SD0_OTAPDLYSEL_SDR50);
	} else {
		/* Program OTAP */
		zynqmp_mmio_write(SD_OTAP_DLY, SD1_OTAPDLYSEL_MASK,
				  SD1_OTAPDLYSEL_SDR50);
	}
}

void arasan_zynqmp_set_tapdelay(u8 deviceid, u8 timing, u8 bank)
{
	if (deviceid == 0)
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK,
				  SD0_DLL_RST);
	else
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK,
				  SD1_DLL_RST);

	switch (timing) {
	case MMC_TIMING_UHS_SDR25:
		arasan_zynqmp_tap_hs(deviceid, timing, bank);
		break;
	case MMC_TIMING_UHS_SDR50:
		arasan_zynqmp_tap_sdr50(deviceid, timing, bank);
		break;
	case MMC_TIMING_UHS_SDR104:
	case MMC_TIMING_MMC_HS200:
		arasan_zynqmp_tap_sdr104(deviceid, timing, bank);
		break;
	case MMC_TIMING_UHS_DDR50:
		arasan_zynqmp_tap_ddr50(deviceid, timing, bank);
		break;
	}

	if (deviceid == 0)
		zynqmp_mmio_write(SD_DLL_CTRL, SD0_DLL_RST_MASK, 0x0);
	else
		zynqmp_mmio_write(SD_DLL_CTRL, SD1_DLL_RST_MASK, 0x0);
}
