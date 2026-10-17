// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Nelson Integration, LLC
 * Author: Eric Nelson <eric@nelint.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/iomux.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

static iomux_v3_cfg_t const uart_pads[] = {
#ifdef CONFIG_UART2_EIM_D26_27
	IOMUX_PADS(PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
#elif defined(CONFIG_UART1_CSI0_DAT10_11)
	IOMUX_PADS(PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
#elif defined(CONFIG_UART1_SD3_DAT6_7)
	IOMUX_PADS(PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
#elif defined(CONFIG_UART1_UART1)
	MX6_PAD_UART1_TXD__UART1_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RXD__UART1_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
#else
#error select UART console pads
#endif
};

#ifdef CONFIG_DDR3
#define GRP_DDRTYPE	0x000C0000
#else
#define GRP_DDRTYPE	0x00080000
#endif

/* all existing designs have this disabled */
#define DDR_PKE		0

/* use Kconfig for ODT and DRIVE_STRENGTH */
#define DDR_ODT	\
	(CONFIG_DDR_ODT << 8)
#define DRAM_DRIVE_STRENGTH \
	(CONFIG_DRAM_DRIVE_STRENGTH << 3)

/* configure MX6Q/DUAL mmdc DDR io registers */
static struct mx6dq_iomux_ddr_regs const mx6dq_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_sdclk_1 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_cas = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_ras = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_reset = DDR_ODT + DRAM_DRIVE_STRENGTH,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00003000 + DRAM_DRIVE_STRENGTH,
	.dram_sdodt1 = 0x00003000 + DRAM_DRIVE_STRENGTH,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs1 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs2 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs3 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs4 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs5 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs6 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs7 = DRAM_DRIVE_STRENGTH,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm1 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm2 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm3 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm4 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm5 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm6 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm7 = DDR_ODT + DRAM_DRIVE_STRENGTH,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
static struct mx6dq_iomux_grp_regs const mx6dq_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type = GRP_DDRTYPE,
	.grp_ddrmode_ctl = DDR_ODT,
	/* disable DDR pullups */
	.grp_ddrpke = DDR_PKE,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = DRAM_DRIVE_STRENGTH,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = DRAM_DRIVE_STRENGTH,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = DDR_ODT,
	.grp_b0ds = DRAM_DRIVE_STRENGTH,
	.grp_b1ds = DRAM_DRIVE_STRENGTH,
	.grp_b2ds = DRAM_DRIVE_STRENGTH,
	.grp_b3ds = DRAM_DRIVE_STRENGTH,
	.grp_b4ds = DRAM_DRIVE_STRENGTH,
	.grp_b5ds = DRAM_DRIVE_STRENGTH,
	.grp_b6ds = DRAM_DRIVE_STRENGTH,
	.grp_b7ds = DRAM_DRIVE_STRENGTH,
};

static struct mx6sdl_iomux_ddr_regs const mx6sdl_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_sdclk_1 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_cas = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_ras = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_reset = DDR_ODT + DRAM_DRIVE_STRENGTH,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00003000 + DRAM_DRIVE_STRENGTH,
	.dram_sdodt1 = 0x00003000 + DRAM_DRIVE_STRENGTH,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs1 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs2 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs3 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs4 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs5 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs6 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs7 = DRAM_DRIVE_STRENGTH,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm1 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm2 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm3 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm4 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm5 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm6 = DDR_ODT + DRAM_DRIVE_STRENGTH,
	.dram_dqm7 = DDR_ODT + DRAM_DRIVE_STRENGTH,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
static struct mx6sdl_iomux_grp_regs const mx6sdl_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type = GRP_DDRTYPE,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.grp_ddrmode_ctl = DDR_ODT,
	/* disable DDR pullups */
	.grp_ddrpke = DDR_PKE,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = DRAM_DRIVE_STRENGTH,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = DRAM_DRIVE_STRENGTH,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = DDR_ODT,
	.grp_b0ds = DRAM_DRIVE_STRENGTH,
	.grp_b1ds = DRAM_DRIVE_STRENGTH,
	.grp_b2ds = DRAM_DRIVE_STRENGTH,
	.grp_b3ds = DRAM_DRIVE_STRENGTH,
	.grp_b4ds = DRAM_DRIVE_STRENGTH,
	.grp_b5ds = DRAM_DRIVE_STRENGTH,
	.grp_b6ds = DRAM_DRIVE_STRENGTH,
	.grp_b7ds = DRAM_DRIVE_STRENGTH,
};

const struct mx6sl_iomux_ddr_regs mx6sl_ddr_ioregs = {
	.dram_sdqs0 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs1 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs2 = DRAM_DRIVE_STRENGTH,
	.dram_sdqs3 = DRAM_DRIVE_STRENGTH,
	.dram_dqm0 = DRAM_DRIVE_STRENGTH,
	.dram_dqm1 = DRAM_DRIVE_STRENGTH,
	.dram_dqm2 = DRAM_DRIVE_STRENGTH,
	.dram_dqm3 = DRAM_DRIVE_STRENGTH,
	.dram_cas  = DRAM_DRIVE_STRENGTH,
	.dram_ras  = DRAM_DRIVE_STRENGTH,
	.dram_sdclk_0 = DRAM_DRIVE_STRENGTH,
	.dram_reset = DRAM_DRIVE_STRENGTH,
	.dram_sdba2 = 0x00020000,
	.dram_odt0 = 0x00030000 + DRAM_DRIVE_STRENGTH,
	.dram_odt1 = 0x00030000 + DRAM_DRIVE_STRENGTH,
};

const struct mx6sl_iomux_grp_regs mx6sl_grp_ioregs = {
	.grp_b0ds = DRAM_DRIVE_STRENGTH,
	.grp_b1ds = DRAM_DRIVE_STRENGTH,
	.grp_b2ds = DRAM_DRIVE_STRENGTH,
	.grp_b3ds = DRAM_DRIVE_STRENGTH,
	.grp_addds = DRAM_DRIVE_STRENGTH,
	.grp_ctlds = DRAM_DRIVE_STRENGTH,
	.grp_ddrmode_ctl = DDR_ODT,
	.grp_ddrpke = DDR_PKE,
	.grp_ddrmode = DDR_ODT,
	.grp_ddr_type = GRP_DDRTYPE,
};

static struct mx6_ddr_sysinfo const sysinfo = {
	/* width of data bus:0=16,1=32,2=64 */
#if CONFIG_DDRWIDTH == 32
	.dsize = 1,
#elif CONFIG_DDRWIDTH == 64
	.dsize = 2,
#else
#error missing CONFIG_DDRWIDTH
#endif
	/* config for full 4GB range so that get_mem_size() works */
	.cs_density = 32, /* 32Gb per CS */

	/* # of chip selects */
	.ncs = CONFIG_DDRCS,
	.cs1_mirror = 0,
	.bi_on = 1,	/* Bank interleaving enabled */
	.rtt_nom = CONFIG_RTT_NOM,
	.rtt_wr = CONFIG_RTT_WR,
	.ralat = CONFIG_RALAT,	/* Read additional latency */
	.walat = CONFIG_WALAT,	/* Write additional latency */
	.mif3_mode = 3,	/* Command prediction working mode */
#ifdef CONFIG_DDR3
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.sde_to_rst = 0x10,	/* JEDEC value for LPDDR2 - 200us */
	.pd_fast_exit = 0,	/* immaterial for calibration */
	.ddr_type = DDR_TYPE_DDR3,
#else
	.rst_to_cke = 0x10,	/* JEDEC value for LPDDR2: 200us */
	.sde_to_rst = 0,	/* LPDDR2 does not need this field */
	.pd_fast_exit = 0,	/* immaterial for calibration */
	.ddr_type = DDR_TYPE_LPDDR2,
#endif
	.refsel = CONFIG_REFSEL,
	.refr = CONFIG_REFR,
};

#ifdef CONFIG_MT41K512M16TNA
/* Micron MT41K512M16TNA-125 */
static struct mx6_ddr3_cfg const ddrtype = {
	.mem_speed = 1600,
	.density = 8,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 1,
	.trcd = 1375,
	.trcmin = 5062,
	.trasmin = 3750,
};
#elif defined(CONFIG_MT41K128M16JT)
/* Micron MT41K128M16JT-125 */
static struct mx6_ddr3_cfg const ddrtype = {
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
};
#elif defined(CONFIG_H5TQ4G63AFR)
/* Hynix H5TQ4G63AFR */
static struct mx6_ddr3_cfg const ddrtype = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};
#elif defined CONFIG_H5TQ2G63DFR
/* Hynix H5TQ2G63DFR */
static struct mx6_ddr3_cfg const ddrtype = {
	.mem_speed = 1333,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1350,
	.trcmin = 4950,
	.trasmin = 3600,
};
#elif defined(CONFIG_MT42L256M32D2LG)
/* Micron MT42L256M32D2LG */
static struct mx6_lpddr2_cfg ddrtype = {
	.mem_speed = 800,
	.density = 4,
	.width = 32,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.trcd_lp = 2000,
	.trppb_lp = 2000,
	.trpab_lp = 2250,
	.trasmin = 4200,
};
#elif defined(CONFIG_MT29PZZZ4D4BKESK)
/* Micron MT29PZZZ4D4BKESK */
static struct mx6_lpddr2_cfg ddrtype = {
	.mem_speed = 800,
	.density = 4,
	.width = 32,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.trcd_lp = 2000,
	.trppb_lp = 2000,
	.trpab_lp = 2250,
	.trasmin = 4200,
};
#else
#error please select DDR type using menuconfig
#endif

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* FIXME: these should probably be checked, especially
	 * for i.MX6SL, UL, ULL
	 */
	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void display_calibration(struct mx6_mmdc_calibration *calib)
{
	printf(".p0_mpdgctrl0\t= 0x%08X\n", calib->p0_mpdgctrl0);
	printf(".p0_mpdgctrl1\t= 0x%08X\n", calib->p0_mpdgctrl1);
	printf(".p0_mprddlctl\t= 0x%08X\n", calib->p0_mprddlctl);
	printf(".p0_mpwrdlctl\t= 0x%08X\n", calib->p0_mpwrdlctl);
	printf(".p0_mpwldectrl0\t= 0x%08X\n", calib->p0_mpwldectrl0);
	printf(".p0_mpwldectrl1\t= 0x%08X\n", calib->p0_mpwldectrl1);
	if (sysinfo.dsize == 2) {
		printf(".p1_mpdgctrl0\t= 0x%08X\n", calib->p1_mpdgctrl0);
		printf(".p1_mpdgctrl1\t= 0x%08X\n", calib->p1_mpdgctrl1);
		printf(".p1_mprddlctl\t= 0x%08X\n", calib->p1_mprddlctl);
		printf(".p1_mpwrdlctl\t= 0x%08X\n", calib->p1_mpwrdlctl);
		printf(".p1_mpwldectrl0\t= 0x%08X\n", calib->p1_mpwldectrl0);
		printf(".p1_mpwldectrl1\t= 0x%08X\n", calib->p1_mpwldectrl1);
	}
#ifdef CONFIG_IMXIMAGE_OUTPUT
	printf("DATA 4 MX6_MMDC_P0_MPDGCTRL0\t= 0x%08X\n", calib->p0_mpdgctrl0);
	printf("DATA 4 MX6_MMDC_P0_MPDGCTRL1\t= 0x%08X\n", calib->p0_mpdgctrl1);
	printf("DATA 4 MX6_MMDC_P0_MPRDDLCTL\t= 0x%08X\n", calib->p0_mprddlctl);
	printf("DATA 4 MX6_MMDC_P0_MPWRDLCTL\t= 0x%08X\n", calib->p0_mpwrdlctl);
	printf("DATA 4 MX6_MMDC_P0_MPWLDECTRL0\t= 0x%08X\n",
	       calib->p0_mpwldectrl0);
	printf("DATA 4 MX6_MMDC_P0_MPWLDECTRL1\t= 0x%08X\n",
	       calib->p0_mpwldectrl1);
	if (sysinfo.dsize == 2) {
		printf("DATA 4 MX6_MMDC_P1_MPDGCTRL0\t= 0x%08X\n",
		       calib->p1_mpdgctrl0);
		printf("DATA 4 MX6_MMDC_P1_MPDGCTRL1\t= 0x%08X\n",
		       calib->p1_mpdgctrl1);
		printf("DATA 4 MX6_MMDC_P1_MPRDDLCTL\t= 0x%08X\n",
		       calib->p1_mprddlctl);
		printf("DATA 4 MX6_MMDC_P1_MPWRDLCTL\t= 0x%08X\n",
		       calib->p1_mpwrdlctl);
		printf("DATA 4 MX6_MMDC_P1_MPWLDECTRL0\t= 0x%08X\n",
		       calib->p1_mpwldectrl0);
		printf("DATA 4 MX6_MMDC_P1_MPWLDECTRL1\t= 0x%08X\n",
		       calib->p1_mpwldectrl1);
	}
#endif
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	int errs;
	struct mx6_mmdc_calibration calibration = {0};

	memset((void *)gd, 0, sizeof(struct global_data));

	/* write leveling calibration defaults */
	calibration.p0_mpwrdlctl = 0x40404040;
	calibration.p1_mpwrdlctl = 0x40404040;

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();

	SETUP_IOMUX_PADS(uart_pads);

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	if (sysinfo.dsize != 1) {
		if (is_cpu_type(MXC_CPU_MX6SX) ||
		    is_cpu_type(MXC_CPU_MX6UL) ||
		    is_cpu_type(MXC_CPU_MX6ULL) ||
		    is_cpu_type(MXC_CPU_MX6SL)) {
			printf("cpu type 0x%x doesn't support 64-bit bus\n",
			       get_cpu_type());
			reset_cpu();
		}
	}
#ifdef CONFIG_MX6SL
	mx6sl_dram_iocfg(CONFIG_DDRWIDTH, &mx6sl_ddr_ioregs,
			 &mx6sl_grp_ioregs);
#else
	if (is_cpu_type(MXC_CPU_MX6Q)) {
		mx6dq_dram_iocfg(CONFIG_DDRWIDTH, &mx6dq_ddr_ioregs,
				 &mx6dq_grp_ioregs);
	} else {
		mx6sdl_dram_iocfg(CONFIG_DDRWIDTH, &mx6sdl_ddr_ioregs,
				  &mx6sdl_grp_ioregs);
	}
#endif
	mx6_dram_cfg(&sysinfo, &calibration, &ddrtype);

	errs = mmdc_do_write_level_calibration(&sysinfo);
	if (errs) {
		printf("error %d from write level calibration\n", errs);
	} else {
		errs = mmdc_do_dqs_calibration(&sysinfo);
		if (errs) {
			printf("error %d from dqs calibration\n", errs);
		} else {
			printf("completed successfully\n");
			mmdc_read_calibration(&sysinfo, &calibration);
			display_calibration(&calibration);
		}
	}
}
