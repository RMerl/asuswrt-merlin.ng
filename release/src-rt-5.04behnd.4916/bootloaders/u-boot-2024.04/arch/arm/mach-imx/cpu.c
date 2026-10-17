// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 */

#include <bootm.h>
#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <netdev.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/mach-imx/boot_mode.h>
#include <imx_thermal.h>
#include <ipu_pixfmt.h>
#include <thermal.h>
#include <sata.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

#ifdef CONFIG_FSL_ESDHC_IMX
#include <fsl_esdhc_imx.h>
#endif

static u32 reset_cause = -1;

u32 get_imx_reset_cause(void)
{
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

	if (reset_cause == -1) {
		reset_cause = readl(&src_regs->srsr);
/* preserve the value for U-Boot proper */
#if !defined(CONFIG_SPL_BUILD)
		writel(reset_cause, &src_regs->srsr);
#endif
	}

	return reset_cause;
}

#if defined(CONFIG_DISPLAY_CPUINFO) && !defined(CONFIG_SPL_BUILD)
static char *get_reset_cause(void)
{
	switch (get_imx_reset_cause()) {
	case 0x00001:
	case 0x00011:
		return "POR";
	case 0x00004:
		return "CSU";
	case 0x00008:
		return "IPP USER";
	case 0x00010:
#ifdef	CONFIG_MX7
		return "WDOG1";
#else
		return "WDOG";
#endif
	case 0x00020:
		return "JTAG HIGH-Z";
	case 0x00040:
		return "JTAG SW";
	case 0x00080:
		return "WDOG3";
#ifdef CONFIG_MX7
	case 0x00100:
		return "WDOG4";
	case 0x00200:
		return "TEMPSENSE";
#elif defined(CONFIG_IMX8M)
	case 0x00100:
		return "WDOG2";
	case 0x00200:
		return "TEMPSENSE";
#else
	case 0x00100:
		return "TEMPSENSE";
	case 0x10000:
		return "WARM BOOT";
#endif
	default:
		return "unknown reset";
	}
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO) && !defined(CONFIG_SPL_BUILD)

const char *get_imx_type(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_IMX8MP:
		return "8MP[8]";	/* Quad-core version of the imx8mp */
	case MXC_CPU_IMX8MPD:
		return "8MP Dual[3]";	/* Dual-core version of the imx8mp */
	case MXC_CPU_IMX8MPL:
		return "8MP Lite[4]";	/* Quad-core Lite version of the imx8mp */
	case MXC_CPU_IMX8MP6:
		return "8MP[6]";	/* Quad-core version of the imx8mp, NPU fused */
	case MXC_CPU_IMX8MPUL:
		return "8MP UltraLite";	/* Quad-core UltraLite version of the imx8mp */
	case MXC_CPU_IMX8MN:
		return "8MNano Quad"; /* Quad-core version */
	case MXC_CPU_IMX8MND:
		return "8MNano Dual"; /* Dual-core version */
	case MXC_CPU_IMX8MNS:
		return "8MNano Solo"; /* Single-core version */
	case MXC_CPU_IMX8MNL:
		return "8MNano QuadLite"; /* Quad-core Lite version */
	case MXC_CPU_IMX8MNDL:
		return "8MNano DualLite"; /* Dual-core Lite version */
	case MXC_CPU_IMX8MNSL:
		return "8MNano SoloLite";/* Single-core Lite version of the imx8mn */
	case MXC_CPU_IMX8MNUQ:
		return "8MNano UltraLite Quad";/* Quad-core UltraLite version of the imx8mn */
	case MXC_CPU_IMX8MNUD:
		return "8MNano UltraLite Dual";/* Dual-core UltraLite version of the imx8mn */
	case MXC_CPU_IMX8MNUS:
		return "8MNano UltraLite Solo";/* Single-core UltraLite version of the imx8mn */
	case MXC_CPU_IMX8MM:
		return "8MMQ";	/* Quad-core version of the imx8mm */
	case MXC_CPU_IMX8MML:
		return "8MMQL";	/* Quad-core Lite version of the imx8mm */
	case MXC_CPU_IMX8MMD:
		return "8MMD";	/* Dual-core version of the imx8mm */
	case MXC_CPU_IMX8MMDL:
		return "8MMDL";	/* Dual-core Lite version of the imx8mm */
	case MXC_CPU_IMX8MMS:
		return "8MMS";	/* Single-core version of the imx8mm */
	case MXC_CPU_IMX8MMSL:
		return "8MMSL";	/* Single-core Lite version of the imx8mm */
	case MXC_CPU_IMX8MQ:
		return "8MQ";	/* Quad-core version of the imx8mq */
	case MXC_CPU_IMX8MQL:
		return "8MQLite";	/* Quad-core Lite version of the imx8mq */
	case MXC_CPU_IMX8MD:
		return "8MD";	/* Dual-core version of the imx8mq */
	case MXC_CPU_MX7S:
		return "7S";	/* Single-core version of the mx7 */
	case MXC_CPU_MX7D:
		return "7D";	/* Dual-core version of the mx7 */
	case MXC_CPU_MX6QP:
		return "6QP";	/* Quad-Plus version of the mx6 */
	case MXC_CPU_MX6DP:
		return "6DP";	/* Dual-Plus version of the mx6 */
	case MXC_CPU_MX6Q:
		return "6Q";	/* Quad-core version of the mx6 */
	case MXC_CPU_MX6D:
		return "6D";	/* Dual-core version of the mx6 */
	case MXC_CPU_MX6DL:
		return "6DL";	/* Dual Lite version of the mx6 */
	case MXC_CPU_MX6SOLO:
		return "6SOLO";	/* Solo version of the mx6 */
	case MXC_CPU_MX6SL:
		return "6SL";	/* Solo-Lite version of the mx6 */
	case MXC_CPU_MX6SLL:
		return "6SLL";	/* SLL version of the mx6 */
	case MXC_CPU_MX6SX:
		return "6SX";   /* SoloX version of the mx6 */
	case MXC_CPU_MX6UL:
		return "6UL";   /* Ultra-Lite version of the mx6 */
	case MXC_CPU_MX6ULL:
		return "6ULL";	/* ULL version of the mx6 */
	case MXC_CPU_MX6ULZ:
		return "6ULZ";	/* ULZ version of the mx6 */
	case MXC_CPU_MX51:
		return "51";
	case MXC_CPU_MX53:
		return "53";
	default:
		return "??";
	}
}

int print_cpuinfo(void)
{
	u32 cpurev;
	__maybe_unused u32 max_freq;

	cpurev = get_cpu_rev();

#if defined(CONFIG_IMX_THERMAL) || defined(CONFIG_IMX_TMU)
	struct udevice *thermal_dev;
	int cpu_tmp, minc, maxc, ret;

	printf("CPU:   Freescale i.MX%s rev%d.%d",
	       get_imx_type((cpurev & 0x1FF000) >> 12),
	       (cpurev & 0x000F0) >> 4,
	       (cpurev & 0x0000F) >> 0);
	max_freq = get_cpu_speed_grade_hz();
	if (!max_freq || max_freq == mxc_get_clock(MXC_ARM_CLK)) {
		printf(" at %dMHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000000);
	} else {
		printf(" %d MHz (running at %d MHz)\n", max_freq / 1000000,
		       mxc_get_clock(MXC_ARM_CLK) / 1000000);
	}
#else
	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
		get_imx_type((cpurev & 0x1FF000) >> 12),
		(cpurev & 0x000F0) >> 4,
		(cpurev & 0x0000F) >> 0,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
#endif

#if defined(CONFIG_IMX_THERMAL) || defined(CONFIG_IMX_TMU)
	puts("CPU:   ");
	switch (get_cpu_temp_grade(&minc, &maxc)) {
	case TEMP_AUTOMOTIVE:
		puts("Automotive temperature grade ");
		break;
	case TEMP_INDUSTRIAL:
		puts("Industrial temperature grade ");
		break;
	case TEMP_EXTCOMMERCIAL:
		puts("Extended Commercial temperature grade ");
		break;
	default:
		puts("Commercial temperature grade ");
		break;
	}
	printf("(%dC to %dC)", minc, maxc);
	ret = uclass_get_device(UCLASS_THERMAL, 0, &thermal_dev);
	if (!ret) {
		ret = thermal_get_temp(thermal_dev, &cpu_tmp);

		if (!ret)
			printf(" at %dC", cpu_tmp);
		else
			debug(" - invalid sensor data\n");
	} else {
		debug(" - invalid sensor device\n");
	}
	puts("\n");
#endif

	printf("Reset cause: %s\n", get_reset_cause());
	return 0;
}
#endif

int cpu_eth_init(struct bd_info *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_FSL_ESDHC_IMX
/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(struct bd_info *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

#if !(defined(CONFIG_MX7) || defined(CONFIG_IMX8M))
u32 get_ahb_clk(void)
{
	struct mxc_ccm_reg *imx_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 reg, ahb_podf;

	reg = __raw_readl(&imx_ccm->cbcdr);
	reg &= MXC_CCM_CBCDR_AHB_PODF_MASK;
	ahb_podf = reg >> MXC_CCM_CBCDR_AHB_PODF_OFFSET;

	return get_periph_clk() / (ahb_podf + 1);
}
#endif

void arch_preboot_os(void)
{
#if defined(CONFIG_IMX_AHCI)
	struct udevice *dev;
	int rc;

	rc = uclass_find_device(UCLASS_AHCI, 0, &dev);
	if (!rc && dev) {
		rc = device_remove(dev, DM_REMOVE_NORMAL);
		if (rc)
			printf("Cannot remove SATA device '%s' (err=%d)\n",
				dev->name, rc);
	}
#endif

#if defined(CONFIG_SATA)
	if (!is_mx6sdl()) {
		sata_remove(0);
#if defined(CONFIG_MX6)
		disable_sata_clock();
#endif
	}
#endif
#if defined(CONFIG_VIDEO_IPUV3)
	/* disable video before launching O/S */
	ipuv3_fb_shutdown();
#endif
#if defined(CONFIG_VIDEO_MXS) && !defined(CONFIG_VIDEO)
	lcdif_power_down();
#endif
}

#ifndef CONFIG_IMX8M
void set_chipselect_size(int const cs_size)
{
	unsigned int reg;
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	reg = readl(&iomuxc_regs->gpr[1]);

	switch (cs_size) {
	case CS0_128:
		reg &= ~0x7;	/* CS0=128MB, CS1=0, CS2=0, CS3=0 */
		reg |= 0x5;
		break;
	case CS0_64M_CS1_64M:
		reg &= ~0x3F;	/* CS0=64MB, CS1=64MB, CS2=0, CS3=0 */
		reg |= 0x1B;
		break;
	case CS0_64M_CS1_32M_CS2_32M:
		reg &= ~0x1FF;	/* CS0=64MB, CS1=32MB, CS2=32MB, CS3=0 */
		reg |= 0x4B;
		break;
	case CS0_32M_CS1_32M_CS2_32M_CS3_32M:
		reg &= ~0xFFF;  /* CS0=32MB, CS1=32MB, CS2=32MB, CS3=32MB */
		reg |= 0x249;
		break;
	default:
		printf("Unknown chip select size: %d\n", cs_size);
		break;
	}

	writel(reg, &iomuxc_regs->gpr[1]);
}
#endif

#if defined(CONFIG_MX7) || defined(CONFIG_IMX8M)
/*
 * OCOTP_TESTER3[9:8] (see Fusemap Description Table offset 0x440)
 * defines a 2-bit SPEED_GRADING
 */
#define OCOTP_TESTER3_SPEED_SHIFT	8
enum cpu_speed {
	OCOTP_TESTER3_SPEED_GRADE0,
	OCOTP_TESTER3_SPEED_GRADE1,
	OCOTP_TESTER3_SPEED_GRADE2,
	OCOTP_TESTER3_SPEED_GRADE3,
	OCOTP_TESTER3_SPEED_GRADE4,
};

u32 get_cpu_speed_grade_hz(void)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	uint32_t val;

	val = readl(&fuse->tester3);
	val >>= OCOTP_TESTER3_SPEED_SHIFT;

	if (is_imx8mn() || is_imx8mp()) {
		val &= 0xf;
		return 2300000000 - val * 100000000;
	}

	if (is_imx8mm())
		val &= 0x7;
	else
		val &= 0x3;

	switch(val) {
	case OCOTP_TESTER3_SPEED_GRADE0:
		return 800000000;
	case OCOTP_TESTER3_SPEED_GRADE1:
		return (is_mx7() ? 500000000 : (is_imx8mq() ? 1000000000 : 1200000000));
	case OCOTP_TESTER3_SPEED_GRADE2:
		return (is_mx7() ? 1000000000 : (is_imx8mq() ? 1300000000 : 1600000000));
	case OCOTP_TESTER3_SPEED_GRADE3:
		return (is_mx7() ? 1200000000 : (is_imx8mq() ? 1500000000 : 1800000000));
	case OCOTP_TESTER3_SPEED_GRADE4:
		return 2000000000;
	}

	return 0;
}

/*
 * OCOTP_TESTER3[7:6] (see Fusemap Description Table offset 0x440)
 * defines a 2-bit SPEED_GRADING
 */
#define OCOTP_TESTER3_TEMP_SHIFT	6

/* iMX8MP uses OCOTP_TESTER3[6:5] for Market segment */
#define IMX8MP_OCOTP_TESTER3_TEMP_SHIFT	5

u32 get_cpu_temp_grade(int *minc, int *maxc)
{
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[1];
	struct fuse_bank1_regs *fuse =
		(struct fuse_bank1_regs *)bank->fuse_regs;
	uint32_t val;

	val = readl(&fuse->tester3);
	if (is_imx8mp())
		val >>= IMX8MP_OCOTP_TESTER3_TEMP_SHIFT;
	else
		val >>= OCOTP_TESTER3_TEMP_SHIFT;
	val &= 0x3;

	if (minc && maxc) {
		if (val == TEMP_AUTOMOTIVE) {
			*minc = -40;
			*maxc = 125;
		} else if (val == TEMP_INDUSTRIAL) {
			*minc = -40;
			*maxc = 105;
		} else if (val == TEMP_EXTCOMMERCIAL) {
			*minc = -20;
			*maxc = 105;
		} else {
			*minc = 0;
			*maxc = 95;
		}
	}
	return val;
}
#endif

#if defined(CONFIG_MX7) || defined(CONFIG_IMX8MQ) || defined(CONFIG_IMX8MM)
enum boot_device get_boot_device(void)
{
	struct bootrom_sw_info **p =
		(struct bootrom_sw_info **)(ulong)ROM_SW_INFO_ADDR;

	enum boot_device boot_dev = SD1_BOOT;
	u8 boot_type = (*p)->boot_dev_type;
	u8 boot_instance = (*p)->boot_dev_instance;

	switch (boot_type) {
	case BOOT_TYPE_SD:
		boot_dev = boot_instance + SD1_BOOT;
		break;
	case BOOT_TYPE_MMC:
		boot_dev = boot_instance + MMC1_BOOT;
		break;
	case BOOT_TYPE_NAND:
		boot_dev = NAND_BOOT;
		break;
	case BOOT_TYPE_QSPI:
		boot_dev = QSPI_BOOT;
		break;
	case BOOT_TYPE_WEIM:
		boot_dev = WEIM_NOR_BOOT;
		break;
	case BOOT_TYPE_SPINOR:
		boot_dev = SPI_NOR_BOOT;
		break;
	case BOOT_TYPE_USB:
		boot_dev = USB_BOOT;
		break;
	default:
#ifdef CONFIG_IMX8M
		if (((readl(SRC_BASE_ADDR + 0x58) & 0x00007FFF) >> 12) == 0x4)
			boot_dev = QSPI_BOOT;
#endif
		break;
	}

	return boot_dev;
}
#endif

#ifdef CONFIG_NXP_BOARD_REVISION
int nxp_board_rev(void)
{
	/*
	 * Get Board ID information from OCOTP_GP1[15:8]
	 * RevA: 0x1
	 * RevB: 0x2
	 * RevC: 0x3
	 */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
			(struct fuse_bank4_regs *)bank->fuse_regs;

	return (readl(&fuse->gp1) >> 8 & 0x0F);
}

char nxp_board_rev_string(void)
{
	const char *rev = "A";

	return (*rev + nxp_board_rev() - 1);
}
#endif

__weak void reset_cpu(void)
{
}
