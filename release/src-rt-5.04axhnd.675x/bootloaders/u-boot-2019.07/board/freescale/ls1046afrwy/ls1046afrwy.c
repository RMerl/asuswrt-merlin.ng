// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <i2c.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <hwconfig.h>
#include <ahci.h>
#include <mmc.h>
#include <scsi.h>
#include <fm_eth.h>
#include <fsl_csu.h>
#include <fsl_esdhc.h>
#include <fsl_sec.h>
#include <fsl_dspi.h>

#define LS1046A_PORSR1_REG 0x1EE0000
#define BOOT_SRC_SD        0x20000000
#define BOOT_SRC_MASK	   0xFF800000
#define BOARD_REV_GPIO		13
#define USB2_SEL_MASK	   0x00000100

#define BYTE_SWAP_32(word)  ((((word) & 0xff000000) >> 24) |  \
(((word) & 0x00ff0000) >>  8) | \
(((word) & 0x0000ff00) <<  8) | \
(((word) & 0x000000ff) << 24))
#define SPI_MCR_REG	0x2100000

DECLARE_GLOBAL_DATA_PTR;

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

static inline void demux_select_usb2(void)
{
	u32 val;
	struct ccsr_gpio *pgpio = (void *)(GPIO3_BASE_ADDR);

	val = in_be32(&pgpio->gpdir);
	val |=  USB2_SEL_MASK;
	out_be32(&pgpio->gpdir, val);

	val = in_be32(&pgpio->gpdat);
	val |=  USB2_SEL_MASK;
	out_be32(&pgpio->gpdat, val);
}

static inline void set_spi_cs_signal_inactive(void)
{
	/* default: all CS signals inactive state is high */
	uint mcr_val;
	uint mcr_cfg_val = DSPI_MCR_MSTR | DSPI_MCR_PCSIS_MASK |
				DSPI_MCR_CRXF | DSPI_MCR_CTXF;

	mcr_val = in_be32(SPI_MCR_REG);
	mcr_val |= DSPI_MCR_HALT;
	out_be32(SPI_MCR_REG, mcr_val);
	out_be32(SPI_MCR_REG, mcr_cfg_val);
	mcr_val = in_be32(SPI_MCR_REG);
	mcr_val &= ~DSPI_MCR_HALT;
	out_be32(SPI_MCR_REG, mcr_val);
}

int board_early_init_f(void)
{
	fsl_lsch2_early_init_f();

	return 0;
}

static inline uint8_t get_board_version(void)
{
	u8 val;
	struct ccsr_gpio *pgpio = (void *)(GPIO2_BASE_ADDR);

	val = (in_le32(&pgpio->gpdat) >> BOARD_REV_GPIO) & 0x03;

	return val;
}

int checkboard(void)
{
	static const char *freq[2] = {"100.00MHZ", "100.00MHZ"};
	u32 boot_src;
	u8 rev;

	rev = get_board_version();
	switch (rev) {
	case 0x00:
		puts("Board: LS1046AFRWY, Rev: A, boot from ");
		break;
	case 0x01:
		puts("Board: LS1046AFRWY, Rev: B, boot from ");
		break;
	default:
		puts("Board: LS1046AFRWY, Rev: Unknown, boot from ");
		break;
	}
	boot_src = BYTE_SWAP_32(readl(LS1046A_PORSR1_REG));

	if ((boot_src & BOOT_SRC_MASK) == BOOT_SRC_SD)
		puts("SD\n");
	else
		puts("QSPI\n");
	printf("SD1_CLK1 = %s, SD1_CLK2 = %s\n", freq[0], freq[1]);

	return 0;
}

int board_init(void)
{
#ifdef CONFIG_SECURE_BOOT
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	 */
	u32 val;
val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif

#ifdef CONFIG_FSL_CAAM
	sec_init();
#endif

	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
	return 0;
}

int board_setup_core_volt(u32 vdd)
{
	return 0;
}

void config_board_mux(void)
{
#ifdef CONFIG_HAS_FSL_XHCI_USB
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 usb_pwrfault;
	/*
	 * USB2 is used, configure mux to USB2_DRVVBUS/USB2_PWRFAULT
	 * USB3 is not used, configure mux to IIC4_SCL/IIC4_SDA
	 */
	out_be32(&scfg->rcwpmuxcr0, 0x3300);
#ifdef CONFIG_HAS_FSL_IIC3
	/* IIC3 is used, configure mux to use IIC3_SCL/IIC3/SDA */
	out_be32(&scfg->rcwpmuxcr0, 0x0000);
#endif
	out_be32(&scfg->usbdrvvbus_selcr, SCFG_USBDRVVBUS_SELCR_USB1);
	usb_pwrfault = (SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB3_SHIFT) |
			(SCFG_USBPWRFAULT_DEDICATED <<
			SCFG_USBPWRFAULT_USB2_SHIFT) |
			(SCFG_USBPWRFAULT_SHARED <<
			SCFG_USBPWRFAULT_USB1_SHIFT);
	out_be32(&scfg->usbpwrfault_selcr, usb_pwrfault);
#ifndef CONFIG_HAS_FSL_IIC3
	/*
	 * LS1046A FRWY board has demultiplexer NX3DV42GU with GPIO3_23 as input
	 * to select I2C3_USB2_SEL_IO
	 * I2C3_USB2_SEL = 0: I2C3_SCL/SDA signals are routed to
	 * I2C3 header (default)
	 * I2C3_USB2_SEL = 1: USB2_DRVVBUS/PWRFAULT signals are routed to
	 * USB2 port
	 * programmed to select USB2 by setting GPIO3_23 output to one
	 */
	demux_select_usb2();
#endif
#endif
	set_spi_cs_signal_inactive();
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	config_board_mux();
	return 0;
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];

	/* fixup DT for the two DDR banks */
	base[0] = gd->bd->bi_dram[0].start;
	size[0] = gd->bd->bi_dram[0].size;
	base[1] = gd->bd->bi_dram[1].start;
	size[1] = gd->bd->bi_dram[1].size;

	fdt_fixup_memory_banks(blob, base, size, 2);
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
#endif

	fdt_fixup_icid(blob);

	return 0;
}
