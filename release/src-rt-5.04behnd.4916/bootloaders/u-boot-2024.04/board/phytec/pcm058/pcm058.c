// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefano Babic <sbabic@denx.de>
 */

/*
 * Please note: there are two version of the board
 * one with NAND and the other with eMMC.
 * Both NAND and eMMC cannot be set because they share the
 * same pins (SD4)
 */
#include <common.h>
#include <init.h>
#include <net.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>
#include <dm.h>

DECLARE_GLOBAL_DATA_PTR;

#define IMX6Q_DRIVE_STRENGTH	0x30

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno - 1;
}

#ifdef CONFIG_CMD_NAND
static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* gate ENFC_CLK_ROOT clock first,before clk source switch */
	clrbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable ENFC_CLK_ROOT clock */
	setbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_CMD_NAND
	setup_gpmi_nand();
#endif
	return 0;
}

#ifdef CONFIG_CMD_BMODE
/*
 * BOOT_CFG1, BOOT_CFG2, BOOT_CFG3, BOOT_CFG4
 * see Table 8-11 and Table 5-9
 *  BOOT_CFG1[7] = 1 (boot from NAND)
 *  BOOT_CFG1[5] = 0 - raw NAND
 *  BOOT_CFG1[4] = 0 - default pad settings
 *  BOOT_CFG1[3:2] = 00 - devices = 1
 *  BOOT_CFG1[1:0] = 00 - Row Address Cycles = 3
 *  BOOT_CFG2[4:3] = 00 - Boot Search Count = 2
 *  BOOT_CFG2[2:1] = 01 - Pages In Block = 64
 *  BOOT_CFG2[0] = 0 - Reset time 12ms
 */
static const struct boot_mode board_boot_modes[] = {
	/* NAND: 64pages per block, 3 row addr cycles, 2 copies of FCB/DBBT */
	{"nand", MAKE_CFGVAL(0x80, 0x02, 0x00, 0x00)},
	{"mmc0",  MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL, 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#include <linux/libfdt.h>

static const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000030,
	.dram_sdclk_1 = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_reset = 0x00000030,
	.dram_sdcke0 = 0x00000030,
	.dram_sdcke1 = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_sdodt0 = 0x00000030,
	.dram_sdodt1 = 0x00000030,
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_sdqs2 = 0x00000030,
	.dram_sdqs3 = 0x00000030,
	.dram_sdqs4 = 0x00000030,
	.dram_sdqs5 = 0x00000030,
	.dram_sdqs6 = 0x00000030,
	.dram_sdqs7 = 0x00000030,
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_dqm2 = 0x00000030,
	.dram_dqm3 = 0x00000030,
	.dram_dqm4 = 0x00000030,
	.dram_dqm5 = 0x00000030,
	.dram_dqm6 = 0x00000030,
	.dram_dqm7 = 0x00000030,
};

static const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds = IMX6Q_DRIVE_STRENGTH,
	.grp_ctlds = IMX6Q_DRIVE_STRENGTH,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b1ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b2ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b3ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b4ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b5ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b6ds = IMX6Q_DRIVE_STRENGTH,
	.grp_b7ds = IMX6Q_DRIVE_STRENGTH,
};

static const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00140014,
	.p0_mpwldectrl1 =  0x000A0015,
	.p1_mpwldectrl0 =  0x000A001E,
	.p1_mpwldectrl1 =  0x000A0015,
	.p0_mpdgctrl0 =  0x43080314,
	.p0_mpdgctrl1 =  0x02680300,
	.p1_mpdgctrl0 =  0x430C0318,
	.p1_mpdgctrl1 =  0x03000254,
	.p0_mprddlctl =  0x3A323234,
	.p1_mprddlctl =  0x3E3C3242,
	.p0_mpwrdlctl =  0x2A2E3632,
	.p1_mpwrdlctl =  0x3C323E34,
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT       = 1,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = 2,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.ddr_type = DDR_TYPE_DDR3,
		.refsel = 1,	/* Refresh cycles at 32KHz */
		.refr = 7,	/* 8 refresh commands per refresh cycle */
	};

	mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
	printf("Boot device %x\n", spl_boot_list[0]);
	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_SPI:
		spl_boot_list[1] = BOOT_DEVICE_UART;
		break;
	case BOOT_DEVICE_MMC1:
		spl_boot_list[1] = BOOT_DEVICE_SPI;
		spl_boot_list[2] = BOOT_DEVICE_UART;
		break;
	default:
		printf("Boot device %x\n", spl_boot_list[0]);
	}
}

void board_init_f(ulong dummy)
{
	/* setup clock gating */
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup AXI */
	gpr_init();

	/* setup GP timer */
	timer_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* Enable device tree and early DM support*/
	spl_early_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
}

/*
 *  Manually probe the SPI bus devices, as this does not happen when the
 *  SPI Flash is probed, which then fails to find the bus.
 */
void spl_board_init(void)
{
	struct udevice *udev;
	int ret = uclass_get_device_by_name(UCLASS_SPI, "spi@2008000", &udev);

	if (ret) {
		printf("SPI bus probe failed, err = %d\n", ret);
	};
}
#endif
