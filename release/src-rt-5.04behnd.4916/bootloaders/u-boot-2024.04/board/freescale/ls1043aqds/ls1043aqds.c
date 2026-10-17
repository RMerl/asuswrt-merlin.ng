// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2019-2020 NXP
 */

#include <common.h>
#include <clock_legacy.h>
#include <i2c.h>
#include <fdt_support.h>
#include <fsl_ddr_sdram.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/fdt.h>
#include <asm/arch/mmu.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <ahci.h>
#include <hwconfig.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_esdhc.h>
#include <fsl_ifc.h>
#include <spl.h>
#include "../common/i2c_mux.h"

#include "../common/qixis.h"
#include "ls1043aqds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

enum {
	MUX_TYPE_GPIO,
};

/* LS1043AQDS serdes mux */
#define CFG_SD_MUX1_SLOT2	0x0 /* SLOT2 TX/RX0 */
#define CFG_SD_MUX1_SLOT1	0x1 /* SLOT1 TX/RX1 */
#define CFG_SD_MUX2_SLOT3	0x0 /* SLOT3 TX/RX0 */
#define CFG_SD_MUX2_SLOT1	0x1 /* SLOT1 TX/RX2 */
#define CFG_SD_MUX3_SLOT4	0x0 /* SLOT4 TX/RX0 */
#define CFG_SD_MUX3_MUX4	0x1 /* MUX4 */
#define CFG_SD_MUX4_SLOT3	0x0 /* SLOT3 TX/RX1 */
#define CFG_SD_MUX4_SLOT1	0x1 /* SLOT1 TX/RX3 */
#define CFG_UART_MUX_MASK	0x6
#define CFG_UART_MUX_SHIFT	1
#define CFG_LPUART_EN		0x1

#ifdef CONFIG_TFABOOT
struct ifc_regs ifc_cfg_nor_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nor0",
		CFG_SYS_NOR0_CSPR,
		CFG_SYS_NOR0_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},

	},
	{
		"nor1",
		CFG_SYS_NOR1_CSPR,
		CFG_SYS_NOR1_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},
	},
	{
		"nand",
		CFG_SYS_NAND_CSPR,
		CFG_SYS_NAND_CSPR_EXT,
		CFG_SYS_NAND_AMASK,
		CFG_SYS_NAND_CSOR,
		{
			CFG_SYS_NAND_FTIM0,
			CFG_SYS_NAND_FTIM1,
			CFG_SYS_NAND_FTIM2,
			CFG_SYS_NAND_FTIM3
		},
	},
	{
		"fpga",
		CFG_SYS_FPGA_CSPR,
		CFG_SYS_FPGA_CSPR_EXT,
		CFG_SYS_FPGA_AMASK,
		CFG_SYS_FPGA_CSOR,
		{
			CFG_SYS_FPGA_FTIM0,
			CFG_SYS_FPGA_FTIM1,
			CFG_SYS_FPGA_FTIM2,
			CFG_SYS_FPGA_FTIM3
		},
	}
};

struct ifc_regs ifc_cfg_nand_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"nand",
		CFG_SYS_NAND_CSPR,
		CFG_SYS_NAND_CSPR_EXT,
		CFG_SYS_NAND_AMASK,
		CFG_SYS_NAND_CSOR,
		{
			CFG_SYS_NAND_FTIM0,
			CFG_SYS_NAND_FTIM1,
			CFG_SYS_NAND_FTIM2,
			CFG_SYS_NAND_FTIM3
		},
	},
	{
		"nor0",
		CFG_SYS_NOR0_CSPR,
		CFG_SYS_NOR0_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},
	},
	{
		"nor1",
		CFG_SYS_NOR1_CSPR,
		CFG_SYS_NOR1_CSPR_EXT,
		CFG_SYS_NOR_AMASK,
		CFG_SYS_NOR_CSOR,
		{
			CFG_SYS_NOR_FTIM0,
			CFG_SYS_NOR_FTIM1,
			CFG_SYS_NOR_FTIM2,
			CFG_SYS_NOR_FTIM3
		},
	},
	{
		"fpga",
		CFG_SYS_FPGA_CSPR,
		CFG_SYS_FPGA_CSPR_EXT,
		CFG_SYS_FPGA_AMASK,
		CFG_SYS_FPGA_CSOR,
		{
			CFG_SYS_FPGA_FTIM0,
			CFG_SYS_FPGA_FTIM1,
			CFG_SYS_FPGA_FTIM2,
			CFG_SYS_FPGA_FTIM3
		},
	}
};

void ifc_cfg_boot_info(struct ifc_regs_info *regs_info)
{
	enum boot_src src = get_boot_src();

	if (src == BOOT_SOURCE_IFC_NAND)
		regs_info->regs = ifc_cfg_nand_boot;
	else
		regs_info->regs = ifc_cfg_nor_boot;
	regs_info->cs_size = CONFIG_SYS_FSL_IFC_BANK_COUNT;
}
#endif

int checkboard(void)
{
#ifdef CONFIG_TFABOOT
	enum boot_src src = get_boot_src();
#endif
	char buf[64];
#ifndef CONFIG_SD_BOOT
	u8 sw;
#endif

	puts("Board: LS1043AQDS, boot from ");

#ifdef CONFIG_TFABOOT
	if (src == BOOT_SOURCE_SD_MMC)
		puts("SD\n");
	else {
#endif

#ifdef CONFIG_SD_BOOT
	puts("SD\n");
#else
	sw = QIXIS_READ(brdcfg[0]);
	sw = (sw & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	if (sw < 0x8)
		printf("vBank: %d\n", sw);
	else if (sw == 0x8)
		puts("PromJet\n");
	else if (sw == 0x9)
		puts("NAND\n");
	else if (sw == 0xF)
		printf("QSPI\n");
	else
		printf("invalid setting of SW%u\n", QIXIS_LBMAP_SWITCH);
#endif

#ifdef CONFIG_TFABOOT
	}
#endif
	printf("Sys ID: 0x%02x, Sys Ver: 0x%02x\n",
	       QIXIS_READ(id), QIXIS_READ(arch));

	printf("FPGA:  v%d (%s), build %d\n",
	       (int)QIXIS_READ(scver), qixis_read_tag(buf),
	       (int)qixis_read_minor());

	return 0;
}

bool if_board_diff_clk(void)
{
	u8 diff_conf = QIXIS_READ(brdcfg[11]);

	return diff_conf & 0x40;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch (sysclk_conf & 0x0f) {
	case QIXIS_SYSCLK_64:
		return 64000000;
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

int dram_init(void)
{
	/*
	 * When resuming from deep sleep, the I2C channel may not be
	 * in the default channel. So, switch to the default channel
	 * before accessing DDR SPD.
	 *
	 * PCA9547 mount on I2C1 bus
	 */
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);
	fsl_initdram();
#if (!defined(CONFIG_SPL) && !defined(CONFIG_TFABOOT)) || \
	defined(CONFIG_SPL_BUILD)
	/* This will break-before-make MMU for DDR */
	update_early_mmu_table();
#endif

	return 0;
}

int i2c_multiplexer_select_vid_channel(u8 channel)
{
	return select_i2c_ch_pca9547(channel, 0);
}

void board_retimer_init(void)
{
	u8 reg;
	int bus_num = 0;

	/* Retimer is connected to I2C1_CH7_CH5 */
	select_i2c_ch_pca9547(I2C_MUX_CH7, bus_num);
	reg = I2C_MUX_CH5;
#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(bus_num, I2C_MUX_PCA_ADDR_SEC,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return;
	}
	dm_i2c_write(dev, 0, &reg, 1);

	/* Access to Control/Shared register */
	ret = i2c_get_chip_for_busnum(bus_num, I2C_RETIMER_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return;
	}

	reg = 0x0;
	dm_i2c_write(dev, 0xff, &reg, 1);

	/* Read device revision and ID */
	dm_i2c_read(dev, 1, &reg, 1);
	debug("Retimer version id = 0x%x\n", reg);

	/* Enable Broadcast. All writes target all channel register sets */
	reg = 0x0c;
	dm_i2c_write(dev, 0xff, &reg, 1);

	/* Reset Channel Registers */
	dm_i2c_read(dev, 0, &reg, 1);
	reg |= 0x4;
	dm_i2c_write(dev, 0, &reg, 1);

	/* Enable override divider select and Enable Override Output Mux */
	dm_i2c_read(dev, 9, &reg, 1);
	reg |= 0x24;
	dm_i2c_write(dev, 9, &reg, 1);

	/* Select VCO Divider to full rate (000) */
	dm_i2c_read(dev, 0x18, &reg, 1);
	reg &= 0x8f;
	dm_i2c_write(dev, 0x18, &reg, 1);

	/* Selects active PFD MUX Input as Re-timed Data (001) */
	dm_i2c_read(dev, 0x1e, &reg, 1);
	reg &= 0x3f;
	reg |= 0x20;
	dm_i2c_write(dev, 0x1e, &reg, 1);

	/* Set data rate as 10.3125 Gbps */
	reg = 0x0;
	dm_i2c_write(dev, 0x60, &reg, 1);
	reg = 0xb2;
	dm_i2c_write(dev, 0x61, &reg, 1);
	reg = 0x90;
	dm_i2c_write(dev, 0x62, &reg, 1);
	reg = 0xb3;
	dm_i2c_write(dev, 0x63, &reg, 1);
	reg = 0xcd;
	dm_i2c_write(dev, 0x64, &reg, 1);
#else
	i2c_write(I2C_MUX_PCA_ADDR_SEC, 0, 1, &reg, 1);

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

	/* Enable override divider select and Enable Override Output Mux */
	i2c_read(I2C_RETIMER_ADDR, 9, 1, &reg, 1);
	reg |= 0x24;
	i2c_write(I2C_RETIMER_ADDR, 9, 1, &reg, 1);

	/* Select VCO Divider to full rate (000) */
	i2c_read(I2C_RETIMER_ADDR, 0x18, 1, &reg, 1);
	reg &= 0x8f;
	i2c_write(I2C_RETIMER_ADDR, 0x18, 1, &reg, 1);

	/* Selects active PFD MUX Input as Re-timed Data (001) */
	i2c_read(I2C_RETIMER_ADDR, 0x1e, 1, &reg, 1);
	reg &= 0x3f;
	reg |= 0x20;
	i2c_write(I2C_RETIMER_ADDR, 0x1e, 1, &reg, 1);

	/* Set data rate as 10.3125 Gbps */
	reg = 0x0;
	i2c_write(I2C_RETIMER_ADDR, 0x60, 1, &reg, 1);
	reg = 0xb2;
	i2c_write(I2C_RETIMER_ADDR, 0x61, 1, &reg, 1);
	reg = 0x90;
	i2c_write(I2C_RETIMER_ADDR, 0x62, 1, &reg, 1);
	reg = 0xb3;
	i2c_write(I2C_RETIMER_ADDR, 0x63, 1, &reg, 1);
	reg = 0xcd;
	i2c_write(I2C_RETIMER_ADDR, 0x64, 1, &reg, 1);
#endif

	/* Return the default channel */
	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, bus_num);
}

int board_early_init_f(void)
{
	u32 __iomem *cntcr = (u32 *)CFG_SYS_FSL_TIMER_ADDR;
#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CFG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;
#endif
#ifdef CONFIG_LPUART
	u8 uart;
#endif

	/*
	 * Enable secure system counter for timer
	 */
	out_le32(cntcr, 0x1);

#if defined(CONFIG_SYS_I2C_EARLY_INIT)
	i2c_early_init_f();
#endif
	fsl_lsch2_early_init_f();

#ifdef CONFIG_HAS_FSL_XHCI_USB
	out_be32(&scfg->rcwpmuxcr0, 0x3333);
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
	usb_pwrfault =
		(SCFG_USBPWRFAULT_DEDICATED << SCFG_USBPWRFAULT_USB3_SHIFT) |
		(SCFG_USBPWRFAULT_DEDICATED << SCFG_USBPWRFAULT_USB2_SHIFT) |
		(SCFG_USBPWRFAULT_SHARED << SCFG_USBPWRFAULT_USB1_SHIFT);
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#endif

#ifdef CONFIG_LPUART
	/* We use lpuart0 as system console */
	uart = QIXIS_READ(brdcfg[14]);
	uart &= ~CFG_UART_MUX_MASK;
	uart |= CFG_LPUART_EN << CFG_UART_MUX_SHIFT;
	QIXIS_WRITE(brdcfg[14], uart);
#endif

	return 0;
}

#ifdef CONFIG_FSL_DEEP_SLEEP
/* determine if it is a warm boot */
bool is_warm_boot(void)
{
#define DCFG_CCSR_CRSTSR_WDRFR	(1 << 3)
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;

	if (in_be32(&gur->crstsr) & DCFG_CCSR_CRSTSR_WDRFR)
		return 1;

	return 0;
}
#endif

int config_board_mux(int ctrl_type)
{
	u8 reg14;

	reg14 = QIXIS_READ(brdcfg[14]);

	switch (ctrl_type) {
	case MUX_TYPE_GPIO:
		reg14 = (reg14 & (~0x30)) | 0x20;
		break;
	default:
		puts("Unsupported mux interface type\n");
		return -1;
	}

	QIXIS_WRITE(brdcfg[14], reg14);

	return 0;
}

int config_serdes_mux(void)
{
	return 0;
}


#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	if (hwconfig("gpio"))
		config_board_mux(MUX_TYPE_GPIO);

	return 0;
}
#endif

int board_init(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
	erratum_a010315();
#endif

	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT, 0);
	board_retimer_init();

#ifdef CFG_SYS_FSL_SERDES
	config_serdes_mux();
#endif

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	u8 reg;

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

	fdt_fixup_memory_banks(blob, base, size, 2);
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_FMAN_ENET
	fdt_fixup_board_enet(blob);
#endif

	fdt_fixup_icid(blob);

	reg = QIXIS_READ(brdcfg[0]);
	reg = (reg & QIXIS_LBMAP_MASK) >> QIXIS_LBMAP_SHIFT;

	/* Disable IFC if QSPI is enabled */
	if (reg == 0xF)
		do_fixup_by_compat(blob, "fsl,ifc",
				   "status", "disabled", 8 + 1, 1);

	return 0;
}
#endif

u8 flash_read8(void *addr)
{
	return __raw_readb(addr + 1);
}

void flash_write16(u16 val, void *addr)
{
	u16 shftval = (((val >> 8) & 0xff) | ((val << 8) & 0xff00));

	__raw_writew(shftval, addr);
}

u16 flash_read16(void *addr)
{
	u16 val = __raw_readw(addr);

	return (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00);
}

#if defined(CONFIG_TFABOOT) && defined(CONFIG_ENV_IS_IN_SPI_FLASH)
void *env_sf_get_env_addr(void)
{
	return (void *)(CFG_SYS_FSL_QSPI_BASE + CONFIG_ENV_OFFSET);
}
#endif
