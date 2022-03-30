// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2018 NXP
 */
#include <common.h>
#include <i2c.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fsl_ddr.h>
#include <fsl_sec.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <fsl-mc/fsl_mc.h>
#include <environment.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch/ppa.h>
#include <hwconfig.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>

#include "../common/qixis.h"
#include "ls1088a_qixis.h"
#include "../common/vid.h"
#include <fsl_immap.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_TARGET_LS1088AQDS
#ifdef CONFIG_TFABOOT
struct ifc_regs ifc_cfg_ifc_nor_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nor0",
		CONFIG_SYS_NOR0_CSPR_EARLY,
		CONFIG_SYS_NOR0_CSPR_EXT,
		CONFIG_SYS_NOR_AMASK,
		CONFIG_SYS_NOR_CSOR,
		{
			CONFIG_SYS_NOR_FTIM0,
			CONFIG_SYS_NOR_FTIM1,
			CONFIG_SYS_NOR_FTIM2,
			CONFIG_SYS_NOR_FTIM3
		},
		0,
		CONFIG_SYS_NOR0_CSPR,
		0,
	},
	{
		"nor1",
		CONFIG_SYS_NOR1_CSPR_EARLY,
		CONFIG_SYS_NOR0_CSPR_EXT,
		CONFIG_SYS_NOR_AMASK_EARLY,
		CONFIG_SYS_NOR_CSOR,
		{
			CONFIG_SYS_NOR_FTIM0,
			CONFIG_SYS_NOR_FTIM1,
			CONFIG_SYS_NOR_FTIM2,
			CONFIG_SYS_NOR_FTIM3
		},
		0,
		CONFIG_SYS_NOR1_CSPR,
		CONFIG_SYS_NOR_AMASK,
	},
	{
		"nand",
		CONFIG_SYS_NAND_CSPR,
		CONFIG_SYS_NAND_CSPR_EXT,
		CONFIG_SYS_NAND_AMASK,
		CONFIG_SYS_NAND_CSOR,
		{
			CONFIG_SYS_NAND_FTIM0,
			CONFIG_SYS_NAND_FTIM1,
			CONFIG_SYS_NAND_FTIM2,
			CONFIG_SYS_NAND_FTIM3
		},
	},
	{
		"fpga",
		CONFIG_SYS_FPGA_CSPR,
		CONFIG_SYS_FPGA_CSPR_EXT,
		SYS_FPGA_AMASK,
		CONFIG_SYS_FPGA_CSOR,
		{
			SYS_FPGA_CS_FTIM0,
			SYS_FPGA_CS_FTIM1,
			SYS_FPGA_CS_FTIM2,
			SYS_FPGA_CS_FTIM3
		},
		0,
		SYS_FPGA_CSPR_FINAL,
		0,
	}
};

struct ifc_regs ifc_cfg_qspi_nor_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nand",
		CONFIG_SYS_NAND_CSPR,
		CONFIG_SYS_NAND_CSPR_EXT,
		CONFIG_SYS_NAND_AMASK,
		CONFIG_SYS_NAND_CSOR,
		{
			CONFIG_SYS_NAND_FTIM0,
			CONFIG_SYS_NAND_FTIM1,
			CONFIG_SYS_NAND_FTIM2,
			CONFIG_SYS_NAND_FTIM3
		},
	},
	{
		"reserved",
	},
	{
		"fpga",
		CONFIG_SYS_FPGA_CSPR,
		CONFIG_SYS_FPGA_CSPR_EXT,
		SYS_FPGA_AMASK,
		CONFIG_SYS_FPGA_CSOR,
		{
			SYS_FPGA_CS_FTIM0,
			SYS_FPGA_CS_FTIM1,
			SYS_FPGA_CS_FTIM2,
			SYS_FPGA_CS_FTIM3
		},
		0,
		SYS_FPGA_CSPR_FINAL,
		0,
	}
};

void ifc_cfg_boot_info(struct ifc_regs_info *regs_info)
{
	enum boot_src src = get_boot_src();

	if (src == BOOT_SOURCE_QSPI_NOR)
		regs_info->regs = ifc_cfg_qspi_nor_boot;
	else
		regs_info->regs = ifc_cfg_ifc_nor_boot;

	regs_info->cs_size = CONFIG_SYS_FSL_IFC_BANK_COUNT;
}
#endif /* CONFIG_TFABOOT */
#endif /* CONFIG_TARGET_LS1088AQDS */

int board_early_init_f(void)
{
#if defined(CONFIG_SYS_I2C_EARLY_INIT) && defined(CONFIG_TARGET_LS1088AQDS)
	i2c_early_init_f();
#endif
	fsl_lsch3_early_init_f();
	return 0;
}

#ifdef CONFIG_FSL_QIXIS
unsigned long long get_qixis_addr(void)
{
	unsigned long long addr;

	if (gd->flags & GD_FLG_RELOC)
		addr = QIXIS_BASE_PHYS;
	else
		addr = QIXIS_BASE_PHYS_EARLY;

	/*
	 * IFC address under 256MB is mapped to 0x30000000, any address above
	 * is mapped to 0x5_10000000 up to 4GB.
	 */
	addr = addr  > 0x10000000 ? addr + 0x500000000ULL : addr + 0x30000000;

	return addr;
}
#endif

#if defined(CONFIG_VID)
int init_func_vid(void)
{
	if (adjust_vdd(0) < 0)
		printf("core voltage not adjusted\n");

	return 0;
}
#endif

int is_pb_board(void)
{
	u8 board_id;

	board_id = QIXIS_READ(id);
	if (board_id == LS1088ARDB_PB_BOARD)
		return 1;
	else
		return 0;
}

int fixup_ls1088ardb_pb_banner(void *fdt)
{
	fdt_setprop_string(fdt, 0, "model", "LS1088ARDB-PB Board");

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
int checkboard(void)
{
#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
#endif
	char buf[64];
	u8 sw;
	static const char *const freq[] = {"100", "125", "156.25",
					    "100 separate SSCG"};
	int clock;

#ifdef CONFIG_TARGET_LS1088AQDS
	printf("Board: LS1088A-QDS, ");
#else
	if (is_pb_board())
		printf("Board: LS1088ARDB-PB, ");
	else
		printf("Board: LS1088A-RDB, ");
#endif

	sw = QIXIS_READ(arch);
	printf("Board Arch: V%d, ", sw >> 4);

#ifdef CONFIG_TARGET_LS1088AQDS
	printf("Board version: %c, boot from ", (sw & 0xf) + 'A' - 1);
#else
	printf("Board version: %c, boot from ", (sw & 0xf) + 'A');
#endif

	memset((u8 *)buf, 0x00, ARRAY_SIZE(buf));

	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

#ifdef CONFIG_TFABOOT
	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD card\n");
#else
#ifdef CONFIG_SD_BOOT
	puts("SD card\n");
#endif
#endif /* CONFIG_TFABOOT */
	switch (sw) {
#ifdef CONFIG_TARGET_LS1088AQDS
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		printf("vBank: %d\n", sw);
		break;
	case 8:
		puts("PromJet\n");
		break;
	case 15:
		puts("IFCCard\n");
		break;
	case 14:
#else
	case 0:
#endif
		puts("QSPI:");
		sw = QIXIS_READ(brdcfg[0]);
		sw = (sw & QIXIS_QMAP_MASK) >> QIXIS_QMAP_SHIFT;
		if (sw == 0 || sw == 4)
			puts("0\n");
		else if (sw == 1)
			puts("1\n");
		else
			puts("EMU\n");
		break;

	default:
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);
		break;
	}

#ifdef CONFIG_TARGET_LS1088AQDS
	printf("FPGA: v%d (%s), build %d",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());
	/* the timestamp string contains "\n" at the end */
	printf(" on %s", qixis_read_time(buf));
#else
	printf("CPLD: v%d.%d\n", QIXIS_READ(scver), QIXIS_READ(tagdata));
#endif

	/*
	 * Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	puts("SERDES1 Reference : ");
	sw = QIXIS_READ(brdcfg[2]);
	clock = (sw >> 6) & 3;
	printf("Clock1 = %sMHz ", freq[clock]);
	clock = (sw >> 4) & 3;
	printf("Clock2 = %sMHz", freq[clock]);

	puts("\nSERDES2 Reference : ");
	clock = (sw >> 2) & 3;
	printf("Clock1 = %sMHz ", freq[clock]);
	clock = (sw >> 0) & 3;
	printf("Clock2 = %sMHz\n", freq[clock]);

	return 0;
}
#endif

bool if_board_diff_clk(void)
{
#ifdef CONFIG_TARGET_LS1088AQDS
	u8 diff_conf = QIXIS_READ(brdcfg[11]);
	return diff_conf & 0x40;
#else
	u8 diff_conf = QIXIS_READ(dutcfg[11]);
	return diff_conf & 0x80;
#endif
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0f) {
	case QIXIS_SYSCLK_83:
		return 83333333;
	case QIXIS_SYSCLK_100:
		return 100000000;
	case QIXIS_SYSCLK_125:
		return 125000000;
	case QIXIS_SYSCLK_133:
		return 133333333;
	case QIXIS_SYSCLK_150:
		return 150000000;
	case QIXIS_SYSCLK_160:
		return 160000000;
	case QIXIS_SYSCLK_166:
		return 166666666;
	}

	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	if (if_board_diff_clk())
		return get_board_sys_clk();
	switch ((ddrclk_conf & 0x30) >> 4) {
	case QIXIS_DDRCLK_100:
		return 100000000;
	case QIXIS_DDRCLK_125:
		return 125000000;
	case QIXIS_DDRCLK_133:
		return 133333333;
	}

	return 66666666;
}

int select_i2c_ch_pca9547(u8 ch)
{
	int ret;

	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
void board_retimer_init(void)
{
	u8 reg;

	/* Retimer is connected to I2C1_CH5 */
	select_i2c_ch_pca9547(I2C_MUX_CH5);

	/* Access to Control/Shared register */
	reg = 0x0;
	i2c_write(I2C_RETIMER_ADDR, 0xff, 1, &reg, 1);

	/* Read device revision and ID */
	i2c_read(I2C_RETIMER_ADDR, 1, 1, &reg, 1);
	debug("Retimer version id = 0x%x\n", reg);

	/* Enable Broadcast. All writes target all channel register sets */
	reg = 0x0c;
	i2c_write(I2C_RETIMER_ADDR, 0xff, 1, &reg, 1);

	/* Reset Channel Registers */
	i2c_read(I2C_RETIMER_ADDR, 0, 1, &reg, 1);
	reg |= 0x4;
	i2c_write(I2C_RETIMER_ADDR, 0, 1, &reg, 1);

	/* Set data rate as 10.3125 Gbps */
	reg = 0x90;
	i2c_write(I2C_RETIMER_ADDR, 0x60, 1, &reg, 1);
	reg = 0xb3;
	i2c_write(I2C_RETIMER_ADDR, 0x61, 1, &reg, 1);
	reg = 0x90;
	i2c_write(I2C_RETIMER_ADDR, 0x62, 1, &reg, 1);
	reg = 0xb3;
	i2c_write(I2C_RETIMER_ADDR, 0x63, 1, &reg, 1);
	reg = 0xcd;
	i2c_write(I2C_RETIMER_ADDR, 0x64, 1, &reg, 1);

	/* Select VCO Divider to full rate (000) */
	i2c_read(I2C_RETIMER_ADDR, 0x2F, 1, &reg, 1);
	reg &= 0x0f;
	reg |= 0x70;
	i2c_write(I2C_RETIMER_ADDR, 0x2F, 1, &reg, 1);

#ifdef	CONFIG_TARGET_LS1088AQDS
	/* Retimer is connected to I2C1_CH5 */
	select_i2c_ch_pca9547(I2C_MUX_CH5);

	/* Access to Control/Shared register */
	reg = 0x0;
	i2c_write(I2C_RETIMER_ADDR2, 0xff, 1, &reg, 1);

	/* Read device revision and ID */
	i2c_read(I2C_RETIMER_ADDR2, 1, 1, &reg, 1);
	debug("Retimer version id = 0x%x\n", reg);

	/* Enable Broadcast. All writes target all channel register sets */
	reg = 0x0c;
	i2c_write(I2C_RETIMER_ADDR2, 0xff, 1, &reg, 1);

	/* Reset Channel Registers */
	i2c_read(I2C_RETIMER_ADDR2, 0, 1, &reg, 1);
	reg |= 0x4;
	i2c_write(I2C_RETIMER_ADDR2, 0, 1, &reg, 1);

	/* Set data rate as 10.3125 Gbps */
	reg = 0x90;
	i2c_write(I2C_RETIMER_ADDR2, 0x60, 1, &reg, 1);
	reg = 0xb3;
	i2c_write(I2C_RETIMER_ADDR2, 0x61, 1, &reg, 1);
	reg = 0x90;
	i2c_write(I2C_RETIMER_ADDR2, 0x62, 1, &reg, 1);
	reg = 0xb3;
	i2c_write(I2C_RETIMER_ADDR2, 0x63, 1, &reg, 1);
	reg = 0xcd;
	i2c_write(I2C_RETIMER_ADDR2, 0x64, 1, &reg, 1);

	/* Select VCO Divider to full rate (000) */
	i2c_read(I2C_RETIMER_ADDR2, 0x2F, 1, &reg, 1);
	reg &= 0x0f;
	reg |= 0x70;
	i2c_write(I2C_RETIMER_ADDR2, 0x2F, 1, &reg, 1);
#endif
	/*return the default channel*/
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_TARGET_LS1088ARDB
	u8 brdcfg5;

	if (hwconfig("esdhc-force-sd")) {
		brdcfg5 = QIXIS_READ(brdcfg[5]);
		brdcfg5 &= ~BRDCFG5_SPISDHC_MASK;
		brdcfg5 |= BRDCFG5_FORCE_SD;
		QIXIS_WRITE(brdcfg[5], brdcfg5);
	}
#endif
	return 0;
}
#endif
#endif

int i2c_multiplexer_select_vid_channel(u8 channel)
{
	return select_i2c_ch_pca9547(channel);
}

#ifdef CONFIG_TARGET_LS1088AQDS
/* read the current value(SVDD) of the LTM Regulator Voltage */
int get_serdes_volt(void)
{
	int  ret, vcode = 0;
	u8 chan = PWM_CHANNEL0;

	/* Select the PAGE 0 using PMBus commands PAGE for VDD */
	ret = i2c_write(I2C_SVDD_MONITOR_ADDR,
			PMBUS_CMD_PAGE, 1, &chan, 1);
	if (ret) {
		printf("VID: failed to select VDD Page 0\n");
		return ret;
	}

	/* Read the output voltage using PMBus command READ_VOUT */
	ret = i2c_read(I2C_SVDD_MONITOR_ADDR,
		       PMBUS_CMD_READ_VOUT, 1, (void *)&vcode, 2);
	if (ret) {
		printf("VID: failed to read the volatge\n");
		return ret;
	}

	return vcode;
}

int set_serdes_volt(int svdd)
{
	int ret, vdd_last;
	u8 buff[5] = {0x04, PWM_CHANNEL0, PMBUS_CMD_VOUT_COMMAND,
			svdd & 0xFF, (svdd & 0xFF00) >> 8};

	/* Write the desired voltage code to the SVDD regulator */
	ret = i2c_write(I2C_SVDD_MONITOR_ADDR,
			PMBUS_CMD_PAGE_PLUS_WRITE, 1, (void *)&buff, 5);
	if (ret) {
		printf("VID: I2C failed to write to the volatge regulator\n");
		return -1;
	}

	/* Wait for the volatge to get to the desired value */
	do {
		vdd_last = get_serdes_volt();
		if (vdd_last < 0) {
			printf("VID: Couldn't read sensor abort VID adjust\n");
			return -1;
		}
	} while (vdd_last != svdd);

	return 1;
}
#else
int get_serdes_volt(void)
{
	return 0;
}

int set_serdes_volt(int svdd)
{
	int ret;
	u8 brdcfg4;

	printf("SVDD changing of RDB\n");

	/* Read the BRDCFG54 via CLPD */
	ret = i2c_read(CONFIG_SYS_I2C_FPGA_ADDR,
		       QIXIS_BRDCFG4_OFFSET, 1, (void *)&brdcfg4, 1);
	if (ret) {
		printf("VID: I2C failed to read the CPLD BRDCFG4\n");
		return -1;
	}

	brdcfg4 = brdcfg4 | 0x08;

	/* Write to the BRDCFG4 */
	ret = i2c_write(CONFIG_SYS_I2C_FPGA_ADDR,
			QIXIS_BRDCFG4_OFFSET, 1, (void *)&brdcfg4, 1);
	if (ret) {
		debug("VID: I2C failed to set the SVDD CPLD BRDCFG4\n");
		return -1;
	}

	/* Wait for the volatge to get to the desired value */
	udelay(10000);

	return 1;
}
#endif

/* this function disables the SERDES, changes the SVDD Voltage and enables it*/
int board_adjust_vdd(int vdd)
{
	int ret = 0;

	debug("%s: vdd = %d\n", __func__, vdd);

	/* Special settings to be performed when voltage is 900mV */
	if (vdd == 900) {
		ret = setup_serdes_volt(vdd);
		if (ret < 0) {
			ret = -1;
			goto exit;
		}
	}
exit:
	return ret;
}

#if !defined(CONFIG_SPL_BUILD)
int board_init(void)
{
	init_final_memctl_regs();
#if defined(CONFIG_TARGET_LS1088ARDB) && defined(CONFIG_FSL_MC_ENET)
	u32 __iomem *irq_ccsr = (u32 __iomem *)ISC_BASE;
#endif

	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
	board_retimer_init();

#ifdef CONFIG_ENV_IS_NOWHERE
	gd->env_addr = (ulong)&default_environment[0];
#endif

#if defined(CONFIG_TARGET_LS1088ARDB) && defined(CONFIG_FSL_MC_ENET)
	/* invert AQR105 IRQ pins polarity */
	out_le32(irq_ccsr + IRQCR_OFFSET / 4, AQR105_IRQ_MASK);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif
#ifdef CONFIG_FSL_LS_PPA
	ppa_init();
#endif
	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
}

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FSL_MC_ENET
void board_quiesce_devices(void)
{
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

	if (get_mc_boot_status() == 0 &&
	    (is_lazy_dpl_addr_valid() || get_dpl_apply_status() == 0))
		fdt_status_okay(fdt, offset);
	else
		fdt_status_fail(fdt, offset);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
void fsl_fdt_fixup_flash(void *fdt)
{
	int offset;
#ifdef CONFIG_TFABOOT
	u32 __iomem *dcfg_ccsr = (u32 __iomem *)DCFG_BASE;
	u32 val;
#endif

/*
 * IFC-NOR and QSPI are muxed on SoC.
 * So disable IFC node in dts if QSPI is enabled or
 * disable QSPI node in dts in case QSPI is not enabled.
 */

#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
	bool disable_ifc = false;

	switch (src) {
	case BOOT_SOURCE_IFC_NOR:
		disable_ifc = false;
		break;
	case BOOT_SOURCE_QSPI_NOR:
		disable_ifc = true;
		break;
	default:
		val = in_le32(dcfg_ccsr + DCFG_RCWSR15 / 4);
		if (DCFG_RCWSR15_IFCGRPABASE_QSPI == (val & (u32)0x3))
			disable_ifc = true;
		break;
	}

	if (disable_ifc) {
		offset = fdt_path_offset(fdt, "/soc/ifc/nor");

		if (offset < 0)
			offset = fdt_path_offset(fdt, "/ifc/nor");
	} else {
		offset = fdt_path_offset(fdt, "/soc/quadspi");

		if (offset < 0)
			offset = fdt_path_offset(fdt, "/quadspi");
	}

#else
#ifdef CONFIG_FSL_QSPI
	offset = fdt_path_offset(fdt, "/soc/ifc/nor");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/ifc/nor");
#else
	offset = fdt_path_offset(fdt, "/soc/quadspi");

	if (offset < 0)
		offset = fdt_path_offset(fdt, "/quadspi");
#endif
#endif
	if (offset < 0)
		return;

	fdt_status_disabled(fdt, offset);
}

int ft_board_setup(void *blob, bd_t *bd)
{
	int i;
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
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
#endif

	fdt_fixup_memory_banks(blob, base, size, CONFIG_NR_DRAM_BANKS);

	fdt_fsl_mc_fixup_iommu_map_entry(blob);

	fsl_fdt_fixup_flash(blob);

#ifdef CONFIG_FSL_MC_ENET
	fdt_fixup_board_enet(blob);
#endif
	if (is_pb_board())
		fixup_ls1088ardb_pb_banner(blob);

	return 0;
}
#endif
#endif /* defined(CONFIG_SPL_BUILD) */

#ifdef CONFIG_TFABOOT
#ifdef CONFIG_MTD_NOR_FLASH
int is_flash_available(void)
{
	char *env_hwconfig = env_get("hwconfig");
	enum boot_src src = get_boot_src();
	int is_nor_flash_available = 1;

	switch (src) {
	case BOOT_SOURCE_IFC_NOR:
		is_nor_flash_available = 1;
		break;
	case BOOT_SOURCE_QSPI_NOR:
		is_nor_flash_available = 0;
		break;
	/*
	 * In Case of SD boot,if qspi is defined in env_hwconfig
	 * disable nor flash probe.
	 */
	default:
		if (hwconfig_f("qspi", env_hwconfig))
			is_nor_flash_available = 0;
		break;
	}
	return is_nor_flash_available;
}
#endif

void *env_sf_get_env_addr(void)
{
	return (void *)(CONFIG_SYS_FSL_QSPI_BASE + CONFIG_ENV_OFFSET);
}
#endif
