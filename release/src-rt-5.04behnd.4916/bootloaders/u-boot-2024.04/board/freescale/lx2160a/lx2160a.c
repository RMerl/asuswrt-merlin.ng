// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2021 NXP
 */

#include <common.h>
#include <clock_legacy.h>
#include <display_options.h>
#include <dm.h>
#include <init.h>
#include <asm/global_data.h>
#include <dm/platform_data/serial_pl01x.h>
#include <i2c.h>
#include <malloc.h>
#include <errno.h>
#include <event.h>
#include <netdev.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <linux/bitops.h>
#include <linux/libfdt.h>
#include <linux/delay.h>
#include <fsl-mc/fsl_mc.h>
#include <env_internal.h>
#include <efi_loader.h>
#include <asm/arch/mmu.h>
#include <hwconfig.h>
#include <asm/arch/clock.h>
#include <asm/arch/config.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include "../common/i2c_mux.h"

#include "../common/qixis.h"
#include "../common/vid.h"
#include <fsl_immap.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include "lx2160a.h"

#ifdef CONFIG_EMC2305
#include "../common/emc2305.h"
#endif

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
#define CFG_MUX_I2C_SDHC(reg, value)		((reg & 0x3f) | value)
#define SET_CFG_MUX1_SDHC1_SDHC(reg)		(reg & 0x3f)
#define SET_CFG_MUX2_SDHC1_SPI(reg, value)	((reg & 0xcf) | value)
#define SET_CFG_MUX3_SDHC1_SPI(reg, value)	((reg & 0xf8) | value)
#define SET_CFG_MUX_SDHC2_DSPI(reg, value)	((reg & 0xf8) | value)
#define SET_CFG_MUX1_SDHC1_DSPI(reg, value)	((reg & 0x3f) | value)
#define SDHC1_BASE_PMUX_DSPI			2
#define SDHC2_BASE_PMUX_DSPI			2
#define IIC5_PMUX_SPI3				3
#endif /* CONFIG_TARGET_LX2160AQDS or CONFIG_TARGET_LX2162AQDS */

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
#if defined(CONFIG_SYS_I2C_EARLY_INIT) && defined(CONFIG_SPL_BUILD)
	i2c_early_init_f();
#endif

#ifdef CONFIG_EMC2305
	select_i2c_ch_pca9547(I2C_MUX_CH_EMC2305, 0);
	emc2305_init(I2C_EMC2305_ADDR);
	set_fan_speed(I2C_EMC2305_PWM, I2C_EMC2305_ADDR);
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);
#endif

	fsl_lsch3_early_init_f();
	return 0;
}

#ifdef CONFIG_OF_BOARD_FIXUP
int board_fix_fdt(void *fdt)
{
	char *reg_names, *reg_name;
	int names_len, old_name_len, new_name_len, remaining_names_len;
	struct str_map {
		char *old_str;
		char *new_str;
	} reg_names_map[] = {
		{ "ccsr", "dbi" },
		{ "pf_ctrl", "ctrl" }
	};
	int off = -1, i = 0;

	if (IS_SVR_REV(get_svr(), 1, 0))
		return 0;

	fdt_for_each_node_by_compatible(off, fdt, -1, "fsl,lx2160a-pcie") {
		fdt_setprop(fdt, off, "compatible", "fsl,ls-pcie",
			    strlen("fsl,ls-pcie") + 1);

		reg_names = (char *)fdt_getprop(fdt, off, "reg-names",
						&names_len);
		if (!reg_names)
			continue;

		reg_name = reg_names;
		remaining_names_len = names_len - (reg_name - reg_names);
		i = 0;
		while ((i < ARRAY_SIZE(reg_names_map)) && remaining_names_len) {
			old_name_len = strlen(reg_names_map[i].old_str);
			new_name_len = strlen(reg_names_map[i].new_str);
			if (memcmp(reg_name, reg_names_map[i].old_str,
				   old_name_len) == 0) {
				/* first only leave required bytes for new_str
				 * and copy rest of the string after it
				 */
				memcpy(reg_name + new_name_len,
				       reg_name + old_name_len,
				       remaining_names_len - old_name_len);
				/* Now copy new_str */
				memcpy(reg_name, reg_names_map[i].new_str,
				       new_name_len);
				names_len -= old_name_len;
				names_len += new_name_len;
				i++;
			}

			reg_name = memchr(reg_name, '\0', remaining_names_len);
			if (!reg_name)
				break;

			reg_name += 1;

			remaining_names_len = names_len -
					      (reg_name - reg_names);
		}

		fdt_setprop(fdt, off, "reg-names", reg_names, names_len);
	}

	/* Fixup u-boot's DTS in case this is a revC board and
	 * we're using DM_ETH.
	 */
	if (IS_ENABLED(CONFIG_TARGET_LX2160ARDB) && IS_ENABLED(CONFIG_DM_ETH))
		fdt_fixup_board_phy_revc(fdt);
	return 0;
}
#endif

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
void esdhc_dspi_status_fixup(void *blob)
{
	const char esdhc0_path[] = "/soc/esdhc@2140000";
	const char esdhc1_path[] = "/soc/esdhc@2150000";
	const char dspi0_path[] = "/soc/spi@2100000";
	const char dspi1_path[] = "/soc/spi@2110000";
	const char dspi2_path[] = "/soc/spi@2120000";

	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 sdhc1_base_pmux;
	u32 sdhc2_base_pmux;
	u32 iic5_pmux;

	/* Check RCW field sdhc1_base_pmux to enable/disable
	 * esdhc0/dspi0 DT node
	 */
	sdhc1_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_SDHC1_BASE_PMUX_MASK;
	sdhc1_base_pmux >>= FSL_CHASSIS3_SDHC1_BASE_PMUX_SHIFT;

	if (sdhc1_base_pmux == SDHC1_BASE_PMUX_DSPI) {
		do_fixup_by_path(blob, dspi0_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, esdhc0_path, "status", "disabled",
				 sizeof("disabled"), 1);
	} else {
		do_fixup_by_path(blob, esdhc0_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, dspi0_path, "status", "disabled",
				 sizeof("disabled"), 1);
	}

	/* Check RCW field sdhc2_base_pmux to enable/disable
	 * esdhc1/dspi1 DT node
	 */
	sdhc2_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR13_REGSR - 1])
		& FSL_CHASSIS3_SDHC2_BASE_PMUX_MASK;
	sdhc2_base_pmux >>= FSL_CHASSIS3_SDHC2_BASE_PMUX_SHIFT;

	if (sdhc2_base_pmux == SDHC2_BASE_PMUX_DSPI) {
		do_fixup_by_path(blob, dspi1_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, esdhc1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	} else {
		do_fixup_by_path(blob, esdhc1_path, "status", "okay",
				 sizeof("okay"), 1);
		do_fixup_by_path(blob, dspi1_path, "status", "disabled",
				 sizeof("disabled"), 1);
	}

	/* Check RCW field IIC5 to enable dspi2 DT node */
	iic5_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_IIC5_PMUX_MASK;
	iic5_pmux >>= FSL_CHASSIS3_IIC5_PMUX_SHIFT;

	if (iic5_pmux == IIC5_PMUX_SPI3)
		do_fixup_by_path(blob, dspi2_path, "status", "okay",
				 sizeof("okay"), 1);
	else
		do_fixup_by_path(blob, dspi2_path, "status", "disabled",
				 sizeof("disabled"), 1);
}
#endif

int esdhc_status_fixup(void *blob, const char *compat)
{
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	/* Enable esdhc and dspi DT nodes based on RCW fields */
	esdhc_dspi_status_fixup(blob);
#else
	/* Enable both esdhc DT nodes for LX2160ARDB */
	do_fixup_by_compat(blob, compat, "status", "okay",
			   sizeof("okay"), 1);
#endif
	return 0;
}

#if defined(CONFIG_VID)
int i2c_multiplexer_select_vid_channel(u8 channel)
{
	return select_i2c_ch_pca9547(channel, 0);
}

int init_func_vid(void)
{
	int set_vid;

	if (IS_SVR_REV(get_svr(), 1, 0))
		set_vid = adjust_vdd(800);
	else
		set_vid = adjust_vdd(0);

	if (set_vid < 0)
		printf("core voltage not adjusted\n");

	return 0;
}
#endif
EVENT_SPY_SIMPLE(EVT_MISC_INIT_F, init_func_vid);

int checkboard(void)
{
	enum boot_src src = get_boot_src();
	char buf[64];
	u8 sw;
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	int clock;
	static const char *const freq[] = {"100", "125", "156.25",
					   "161.13", "322.26", "", "", "",
					   "", "", "", "", "", "", "",
					   "100 separate SSCG"};
#endif

	cpu_name(buf);
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	printf("Board: %s-QDS, ", buf);
#else
	printf("Board: %s-RDB, ", buf);
#endif

	sw = QIXIS_READ(arch);
	printf("Board version: %c, boot from ", (sw & 0xf) - 1 + 'A');

	if (src == BOOT_SOURCE_SD_MMC) {
		puts("SD\n");
	} else if (src == BOOT_SOURCE_SD_MMC2) {
		puts("eMMC\n");
	} else {
		sw = QIXIS_READ(brdcfg[0]);
		sw = (sw >> QIXIS_XMAP_SHIFT) & QIXIS_XMAP_MASK;
		switch (sw) {
		case 0:
		case 4:
			puts("FlexSPI DEV#0\n");
			break;
		case 1:
			puts("FlexSPI DEV#1\n");
			break;
		case 2:
		case 3:
			puts("FlexSPI EMU\n");
			break;
		default:
			printf("invalid setting, xmap: %d\n", sw);
			break;
		}
	}
#if defined(CONFIG_TARGET_LX2160ARDB)
	printf("FPGA: v%d.%d\n", QIXIS_READ(scver), QIXIS_READ(tagdata));

	puts("SERDES1 Reference: Clock1 = 161.13MHz Clock2 = 161.13MHz\n");
	puts("SERDES2 Reference: Clock1 = 100MHz Clock2 = 100MHz\n");
	puts("SERDES3 Reference: Clock1 = 100MHz Clock2 = 100MHz\n");
#else
	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));

	puts("SERDES1 Reference : ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = sw >> 4;
	printf("Clock1 = %sMHz ", freq[clock]);
#if defined(CONFIG_TARGET_LX2160AQDS)
	clock = sw & 0x0f;
	printf("Clock2 = %sMHz", freq[clock]);
#endif
	sw = QIXIS_READ(brdcfg[3]);
	puts("\nSERDES2 Reference : ");
	clock = sw >> 4;
	printf("Clock1 = %sMHz ", freq[clock]);
	clock = sw & 0x0f;
	printf("Clock2 = %sMHz\n", freq[clock]);
#if defined(CONFIG_TARGET_LX2160AQDS)
	sw = QIXIS_READ(brdcfg[12]);
	puts("SERDES3 Reference : ");
	clock = sw >> 4;
	printf("Clock1 = %sMHz Clock2 = %sMHz\n", freq[clock], freq[clock]);
#endif
#endif
	return 0;
}

#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
static void esdhc_adapter_card_ident(void)
{
	u8 card_id, val;

	val = QIXIS_READ(sdhc1);
	card_id = val & QIXIS_SDID_MASK;

	switch (card_id) {
	case QIXIS_ESDHC_ADAPTER_TYPE_SD:
		/* Power cycle to card */
		val &= ~QIXIS_SDHC1_S1V3;
		QIXIS_WRITE(sdhc1, val);
		mdelay(1);
		val |= QIXIS_SDHC1_S1V3;
		QIXIS_WRITE(sdhc1, val);
		/* Route to SDHC1_VS */
		val = QIXIS_READ(brdcfg[11]);
		val |= QIXIS_SDHC1_VS;
		QIXIS_WRITE(brdcfg[11], val);
		break;
	default:
		break;
	}
}

int config_board_mux(void)
{
	u8 reg11, reg5, reg13;
	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 sdhc1_base_pmux;
	u32 sdhc2_base_pmux;
	u32 iic5_pmux;

	/* Routes {I2C2_SCL, I2C2_SDA} to SDHC1 as {SDHC1_CD_B, SDHC1_WP}.
	 * Routes {I2C3_SCL, I2C3_SDA} to CAN transceiver as {CAN1_TX,CAN1_RX}.
	 * Routes {I2C4_SCL, I2C4_SDA} to CAN transceiver as {CAN2_TX,CAN2_RX}.
	 * Qixis and remote systems are isolated from the I2C1 bus.
	 * Processor connections are still available.
	 * SPI2 CS2_B controls EN25S64 SPI memory device.
	 * SPI3 CS2_B controls EN25S64 SPI memory device.
	 * EC2 connects to PHY #2 using RGMII protocol.
	 * CLK_OUT connects to FPGA for clock measurement.
	 */

	reg5 = QIXIS_READ(brdcfg[5]);
	reg5 = CFG_MUX_I2C_SDHC(reg5, 0x40);
	QIXIS_WRITE(brdcfg[5], reg5);

	/* Check RCW field sdhc1_base_pmux
	 * esdhc0 : sdhc1_base_pmux = 0
	 * dspi0  : sdhc1_base_pmux = 2
	 */
	sdhc1_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_SDHC1_BASE_PMUX_MASK;
	sdhc1_base_pmux >>= FSL_CHASSIS3_SDHC1_BASE_PMUX_SHIFT;

	if (sdhc1_base_pmux == SDHC1_BASE_PMUX_DSPI) {
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX1_SDHC1_DSPI(reg11, 0x40);
		QIXIS_WRITE(brdcfg[11], reg11);
	} else {
		/* - Routes {SDHC1_CMD, SDHC1_CLK } to SDHC1 adapter slot.
		 *          {SDHC1_DAT3, SDHC1_DAT2} to SDHC1 adapter slot.
		 *          {SDHC1_DAT1, SDHC1_DAT0} to SDHC1 adapter slot.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX1_SDHC1_SDHC(reg11);
		QIXIS_WRITE(brdcfg[11], reg11);
	}

	/* Check RCW field sdhc2_base_pmux
	 * esdhc1 : sdhc2_base_pmux = 0 (default)
	 * dspi1  : sdhc2_base_pmux = 2
	 */
	sdhc2_base_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR13_REGSR - 1])
		& FSL_CHASSIS3_SDHC2_BASE_PMUX_MASK;
	sdhc2_base_pmux >>= FSL_CHASSIS3_SDHC2_BASE_PMUX_SHIFT;

	if (sdhc2_base_pmux == SDHC2_BASE_PMUX_DSPI) {
		reg13 = QIXIS_READ(brdcfg[13]);
		reg13 = SET_CFG_MUX_SDHC2_DSPI(reg13, 0x01);
		QIXIS_WRITE(brdcfg[13], reg13);
	} else {
		reg13 = QIXIS_READ(brdcfg[13]);
		reg13 = SET_CFG_MUX_SDHC2_DSPI(reg13, 0x00);
		QIXIS_WRITE(brdcfg[13], reg13);
	}

	/* Check RCW field IIC5 to enable dspi2 DT nodei
	 * dspi2: IIC5 = 3
	 */
	iic5_pmux = gur_in32(&gur->rcwsr[FSL_CHASSIS3_RCWSR12_REGSR - 1])
		& FSL_CHASSIS3_IIC5_PMUX_MASK;
	iic5_pmux >>= FSL_CHASSIS3_IIC5_PMUX_SHIFT;

	if (iic5_pmux == IIC5_PMUX_SPI3) {
		/* - Routes {SDHC1_DAT4} to SPI3 devices as {SPI3_M_CS0_B}. */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX2_SDHC1_SPI(reg11, 0x10);
		QIXIS_WRITE(brdcfg[11], reg11);

		/* - Routes {SDHC1_DAT5, SDHC1_DAT6} nowhere.
		 * {SDHC1_DAT7, SDHC1_DS } to {nothing, SPI3_M0_CLK }.
		 * {I2C5_SCL, I2C5_SDA } to {SPI3_M0_MOSI, SPI3_M0_MISO}.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX3_SDHC1_SPI(reg11, 0x01);
		QIXIS_WRITE(brdcfg[11], reg11);
	} else {
		/*
		 * If {SDHC1_DAT4} has been configured to route to SDHC1_VS,
		 * do not change it.
		 * Otherwise route {SDHC1_DAT4} to SDHC1 adapter slot.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		if ((reg11 & 0x30) != 0x30) {
			reg11 = SET_CFG_MUX2_SDHC1_SPI(reg11, 0x00);
			QIXIS_WRITE(brdcfg[11], reg11);
		}

		/* - Routes {SDHC1_DAT5, SDHC1_DAT6} to SDHC1 adapter slot.
		 * {SDHC1_DAT7, SDHC1_DS } to SDHC1 adapter slot.
		 * {I2C5_SCL, I2C5_SDA } to SDHC1 adapter slot.
		 */
		reg11 = QIXIS_READ(brdcfg[11]);
		reg11 = SET_CFG_MUX3_SDHC1_SPI(reg11, 0x00);
		QIXIS_WRITE(brdcfg[11], reg11);
	}

	return 0;
}

int board_early_init_r(void)
{
	esdhc_adapter_card_ident();
	return 0;
}
#elif defined(CONFIG_TARGET_LX2160ARDB)
int config_board_mux(void)
{
	u8 brdcfg;

	brdcfg = QIXIS_READ(brdcfg[4]);
	/* The BRDCFG4 register controls general board configuration.
	 *|-------------------------------------------|
	 *|Field  | Function                          |
	 *|-------------------------------------------|
	 *|5      | CAN I/O Enable (net CFG_CAN_EN_B):|
	 *|CAN_EN | 0= CAN transceivers are disabled. |
	 *|       | 1= CAN transceivers are enabled.  |
	 *|-------------------------------------------|
	 */
	brdcfg |= BIT_MASK(5);
	QIXIS_WRITE(brdcfg[4], brdcfg);

	return 0;
}
#else
int config_board_mux(void)
{
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_TARGET_LX2160ARDB)
u8 get_board_rev(void)
{
	u8 board_rev = (QIXIS_READ(arch) & 0xf) - 1 + 'A';

	return board_rev;
}
#endif

unsigned long get_board_sys_clk(void)
{
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x03) {
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	}
	return 100000000;
#else
	return 100000000;
#endif
}

unsigned long get_board_ddr_clk(void)
{
#if defined(CONFIG_TARGET_LX2160AQDS) || defined(CONFIG_TARGET_LX2162AQDS)
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}
	return 100000000;
#else
	return 100000000;
#endif
}

int board_init(void)
{
#if defined(CONFIG_FSL_MC_ENET) && defined(CONFIG_TARGET_LX2160ARDB)
	u32 __iomem *irq_ccsr = (u32 __iomem *)ISC_BASE;
#endif

	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);

#if defined(CONFIG_FSL_MC_ENET) && defined(CONFIG_TARGET_LX2160ARDB)
	/* invert AQR107 IRQ pins polarity */
	out_le32(irq_ccsr + IRQCR_OFFSET / 4, AQR107_IRQ_MASK);
#endif

#if !defined(CONFIG_SYS_EARLY_PCI_INIT)
	pci_init();
#endif
	return 0;
}

void detail_board_ddr_info(void)
{
	int i;
	u64 ddr_size = 0;

	puts("\nDDR    ");
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		ddr_size += gd->bd->bi_dram[i].size;
	print_size(ddr_size, "");
	print_ddr_info(0);
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();

	return 0;
}
#endif

#ifdef CONFIG_VID
u16 soc_get_fuse_vid(int vid_index)
{
	static const u16 vdd[32] = {
		8250,
		7875,
		7750,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		8000,
		8125,
		8250,
		0,      /* reserved */
		8500,
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
		0,      /* reserved */
	};

	return vdd[vid_index];
};
#endif

#ifdef CONFIG_FSL_MC_ENET

void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/soc/fsl-mc");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/fsl-mc");

	if (offset < 0) {
		printf("%s: fsl-mc node not found in device tree (error %d)\n",
		       __func__, offset);
		return;
	}

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0)) {
		fdt_status_okay(fdt, offset);
		if (IS_ENABLED(CONFIG_TARGET_LX2160ARDB))
			fdt_fixup_board_phy_revc(fdt);
	} else {
		fdt_status_fail(fdt, offset);
	}
}

void board_quiesce_devices(void)
{
	fsl_mc_ldpaa_exit(gd->bd);
}
#endif

#if IS_ENABLED(CONFIG_TARGET_LX2160ARDB)
int fdt_fixup_add_thermal(void *blob, int mux_node, int channel, int reg)
{
	int err;
	int noff;
	int offset;
	char channel_node_name[50];
	char thermal_node_name[50];
	u32 phandle;

	snprintf(channel_node_name, sizeof(channel_node_name),
		 "i2c@%x", channel);
	debug("channel_node_name = %s\n", channel_node_name);

	snprintf(thermal_node_name, sizeof(thermal_node_name),
		 "temperature-sensor@%x", reg);
	debug("thermal_node_name = %s\n", thermal_node_name);

	err = fdt_increase_size(blob, 200);
	if (err) {
		printf("fdt_increase_size: err=%s\n", fdt_strerror(err));
		return err;
	}

	noff = fdt_subnode_offset(blob, mux_node, (const char *)
				  channel_node_name);
	if (noff < 0) {
		/* channel node not found - create it */
		noff = fdt_add_subnode(blob, mux_node, channel_node_name);
		if (noff < 0) {
			printf("fdt_add_subnode: err=%s\n", fdt_strerror(err));
			return err;
		}
		fdt_setprop_u32 (blob, noff, "#address-cells", 1);
		fdt_setprop_u32 (blob, noff, "#size-cells", 0);
		fdt_setprop_u32 (blob, noff, "reg", channel);
	}

	/* Create thermal node*/
	offset = fdt_add_subnode(blob, noff, thermal_node_name);
	fdt_setprop(blob, offset, "compatible", "nxp,sa56004",
		    strlen("nxp,sa56004") + 1);
	fdt_setprop_u32 (blob, offset, "reg", reg);

	/* fixup phandle*/
	noff = fdt_node_offset_by_compatible(blob, -1, "regulator-fixed");
	if (noff < 0) {
		printf("%s : failed to get phandle\n", __func__);
		return noff;
	}
	phandle = fdt_get_phandle(blob, noff);
	fdt_setprop_u32 (blob, offset, "vcc-supply", phandle);

	return 0;
}

void fdt_fixup_delete_thermal(void *blob, int mux_node, int channel, int reg)
{
	int node;
	int value;
	int err;
	int subnode;

	fdt_for_each_subnode(subnode, blob, mux_node) {
		value = fdtdec_get_uint(blob, subnode, "reg", -1);
		if (value == channel) {
			/* delete thermal node */
			fdt_for_each_subnode(node, blob, subnode) {
				value = fdtdec_get_uint(blob, node, "reg", -1);
				err = fdt_node_check_compatible(blob, node,
								"nxp,sa56004");
				if (!err && value == reg) {
					fdt_del_node(blob, node);
					break;
				}
			}
		}
	}
}

void fdt_fixup_i2c_thermal_node(void *blob)
{
	int i2coffset;
	int mux_node;
	int reg;
	int err;

	i2coffset = fdt_node_offset_by_compat_reg(blob, "fsl,vf610-i2c",
						  0x2000000);
	if (i2coffset != -FDT_ERR_NOTFOUND) {
		fdt_for_each_subnode(mux_node, blob, i2coffset) {
			reg = fdtdec_get_uint(blob, mux_node, "reg", -1);
			err = fdt_node_check_compatible(blob, mux_node,
							"nxp,pca9547");
			if (!err && reg == 0x77) {
				fdt_fixup_delete_thermal(blob, mux_node,
							 0x3, 0x4d);
				err = fdt_fixup_add_thermal(blob, mux_node,
							    0x3, 0x48);
				if (err)
					printf("%s: Add thermal node failed\n",
					       __func__);
			}
		}
	} else {
		printf("%s: i2c node not found\n", __func__);
	}
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int i;
	u16 mc_memory_bank = 0;

	u64 *base;
	u64 *size;
	u64 mc_memory_base = 0;
	u64 mc_memory_size = 0;
	u16 total_memory_banks;
	int err;

	err = fdt_increase_size(blob, 512);
	if (err) {
		printf("%s fdt_increase_size: err=%s\n", __func__,
		       fdt_strerror(err));
		return err;
	}

	ft_cpu_setup(blob, bd);

	fdt_fixup_mc_ddr(&mc_memory_base, &mc_memory_size);

	if (mc_memory_base != 0)
		mc_memory_bank++;

	total_memory_banks = CONFIG_NR_DRAM_BANKS + mc_memory_bank;

	base = calloc(total_memory_banks, sizeof(u64));
	size = calloc(total_memory_banks, sizeof(u64));

	/* fixup DT for the three GPP DDR banks */
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

#ifdef CONFIG_RESV_RAM
	/* reduce size if reserved memory is within this bank */
	if (gd->arch.resv_ram >= base[0] &&
	    gd->arch.resv_ram < base[0] + size[0])
		size[0] = gd->arch.resv_ram - base[0];
	else if (gd->arch.resv_ram >= base[1] &&
		 gd->arch.resv_ram < base[1] + size[1])
		size[1] = gd->arch.resv_ram - base[1];
	else if (gd->arch.resv_ram >= base[2] &&
		 gd->arch.resv_ram < base[2] + size[2])
		size[2] = gd->arch.resv_ram - base[2];
#endif

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

#ifdef CONFIG_USB_HOST
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_FSL_MC_ENET
	fdt_fsl_mc_fixup_iommu_map_entry(blob);
	fdt_fixup_board_enet(blob);
	fdt_reserve_mc_mem(blob, 0x4000);
#endif
	fdt_fixup_icid(blob);

#if IS_ENABLED(CONFIG_TARGET_LX2160ARDB)
	if (get_board_rev() == 'C')
		fdt_fixup_i2c_thermal_node(blob);
#endif

	return 0;
}
#endif

void qixis_dump_switch(void)
{
	int i, nr_of_cfgsw;

	QIXIS_WRITE(cms[0], 0x00);
	nr_of_cfgsw = QIXIS_READ(cms[1]);

	puts("DIP switch settings dump:\n");
	for (i = 1; i <= nr_of_cfgsw; i++) {
		QIXIS_WRITE(cms[0], i);
		printf("SW%d = (0x%02x)\n", i, QIXIS_READ(cms[1]));
	}
}
