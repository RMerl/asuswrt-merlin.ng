// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Samsung Electronics
 */

#include <common.h>
#include <config.h>

#include <asm/arch/clock.h>
#include <asm/arch/clk.h>
#include <asm/arch/dmc.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/spl.h>
#include <asm/arch/spi.h>

#include "common_setup.h"
#include "clock_init.h"

DECLARE_GLOBAL_DATA_PTR;

/* Index into irom ptr table */
enum index {
	MMC_INDEX,
	EMMC44_INDEX,
	EMMC44_END_INDEX,
	SPI_INDEX,
	USB_INDEX,
};

/* IROM Function Pointers Table */
u32 irom_ptr_table[] = {
	[MMC_INDEX] = 0x02020030,	/* iROM Function Pointer-SDMMC boot */
	[EMMC44_INDEX] = 0x02020044,	/* iROM Function Pointer-EMMC4.4 boot*/
	[EMMC44_END_INDEX] = 0x02020048,/* iROM Function Pointer
						-EMMC4.4 end boot operation */
	[SPI_INDEX] = 0x02020058,	/* iROM Function Pointer-SPI boot */
	[USB_INDEX] = 0x02020070,	/* iROM Function Pointer-USB boot*/
	};

void *get_irom_func(int index)
{
	return (void *)*(u32 *)irom_ptr_table[index];
}

#ifdef CONFIG_USB_BOOTING
/*
 * Set/clear program flow prediction and return the previous state.
 */
static int config_branch_prediction(int set_cr_z)
{
	unsigned int cr;

	/* System Control Register: 11th bit Z Branch prediction enable */
	cr = get_cr();
	set_cr(set_cr_z ? cr | CR_Z : cr & ~CR_Z);

	return cr & CR_Z;
}
#endif

#ifdef CONFIG_SPI_BOOTING
static void spi_rx_tx(struct exynos_spi *regs, int todo,
			void *dinp, void const *doutp, int i)
{
	uint *rxp = (uint *)(dinp + (i * (32 * 1024)));
	int rx_lvl, tx_lvl;
	uint out_bytes, in_bytes;

	out_bytes = todo;
	in_bytes = todo;
	setbits_le32(&regs->ch_cfg, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_CH_RST);
	writel(((todo * 8) / 32) | SPI_PACKET_CNT_EN, &regs->pkt_cnt);

	while (in_bytes) {
		uint32_t spi_sts;
		int temp;

		spi_sts = readl(&regs->spi_sts);
		rx_lvl = ((spi_sts >> 15) & 0x7f);
		tx_lvl = ((spi_sts >> 6) & 0x7f);
		while (tx_lvl < 32 && out_bytes) {
			temp = 0xffffffff;
			writel(temp, &regs->tx_data);
			out_bytes -= 4;
			tx_lvl += 4;
		}
		while (rx_lvl >= 4 && in_bytes) {
			temp = readl(&regs->rx_data);
			if (rxp)
				*rxp++ = temp;
			in_bytes -= 4;
			rx_lvl -= 4;
		}
	}
}

/*
 * Copy uboot from spi flash to RAM
 *
 * @parma uboot_size	size of u-boot to copy
 * @param uboot_addr	address in u-boot to copy
 */
static void exynos_spi_copy(unsigned int uboot_size, unsigned int uboot_addr)
{
	int upto, todo;
	int i, timeout = 100;
	struct exynos_spi *regs = (struct exynos_spi *)CONFIG_SYS_SPI_BASE;

	set_spi_clk(PERIPH_ID_SPI1, 50000000); /* set spi clock to 50Mhz */
	/* set the spi1 GPIO */
	exynos_pinmux_config(PERIPH_ID_SPI1, PINMUX_FLAG_NONE);

	/* set pktcnt and enable it */
	writel(4 | SPI_PACKET_CNT_EN, &regs->pkt_cnt);
	/* set FB_CLK_SEL */
	writel(SPI_FB_DELAY_180, &regs->fb_clk);
	/* set CH_WIDTH and BUS_WIDTH as word */
	setbits_le32(&regs->mode_cfg, SPI_MODE_CH_WIDTH_WORD |
					SPI_MODE_BUS_WIDTH_WORD);
	clrbits_le32(&regs->ch_cfg, SPI_CH_CPOL_L); /* CPOL: active high */

	/* clear rx and tx channel if set priveously */
	clrbits_le32(&regs->ch_cfg, SPI_RX_CH_ON | SPI_TX_CH_ON);

	setbits_le32(&regs->swap_cfg, SPI_RX_SWAP_EN |
			SPI_RX_BYTE_SWAP |
			SPI_RX_HWORD_SWAP);

	/* do a soft reset */
	setbits_le32(&regs->ch_cfg, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_CH_RST);

	/* now set rx and tx channel ON */
	setbits_le32(&regs->ch_cfg, SPI_RX_CH_ON | SPI_TX_CH_ON | SPI_CH_HS_EN);
	clrbits_le32(&regs->cs_reg, SPI_SLAVE_SIG_INACT); /* CS low */

	/* Send read instruction (0x3h) followed by a 24 bit addr */
	writel((SF_READ_DATA_CMD << 24) | SPI_FLASH_UBOOT_POS, &regs->tx_data);

	/* waiting for TX done */
	while (!(readl(&regs->spi_sts) & SPI_ST_TX_DONE)) {
		if (!timeout) {
			debug("SPI TIMEOUT\n");
			break;
		}
		timeout--;
	}

	for (upto = 0, i = 0; upto < uboot_size; upto += todo, i++) {
		todo = min(uboot_size - upto, (unsigned int)(1 << 15));
		spi_rx_tx(regs, todo, (void *)(uboot_addr),
			  (void *)(SPI_FLASH_UBOOT_POS), i);
	}

	setbits_le32(&regs->cs_reg, SPI_SLAVE_SIG_INACT);/* make the CS high */

	/*
	 * Let put controller mode to BYTE as
	 * SPI driver does not support WORD mode yet
	 */
	clrbits_le32(&regs->mode_cfg, SPI_MODE_CH_WIDTH_WORD |
					SPI_MODE_BUS_WIDTH_WORD);
	writel(0, &regs->swap_cfg);

	/*
	 * Flush spi tx, rx fifos and reset the SPI controller
	 * and clear rx/tx channel
	 */
	clrsetbits_le32(&regs->ch_cfg, SPI_CH_HS_EN, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_TX_CH_ON | SPI_RX_CH_ON);
}
#endif

/*
* Copy U-Boot from mmc to RAM:
* COPY_BL2_FNPTR_ADDR: Address in iRAM, which Contains
* Pointer to API (Data transfer from mmc to ram)
*/
void copy_uboot_to_ram(void)
{
	unsigned int bootmode = BOOT_MODE_OM;

	u32 (*copy_bl2)(u32 offset, u32 nblock, u32 dst) = NULL;
	u32 offset = 0, size = 0;
#ifdef CONFIG_SPI_BOOTING
	struct spl_machine_param *param = spl_get_machine_params();
#endif
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	u32 (*copy_bl2_from_emmc)(u32 nblock, u32 dst);
	void (*end_bootop_from_emmc)(void);
#endif
#ifdef CONFIG_USB_BOOTING
	int is_cr_z_set;
	unsigned int sec_boot_check;

	/*
	 * Note that older hardware (before Exynos5800) does not expect any
	 * arguments, but it does not hurt to pass them, so a common function
	 * prototype is used.
	 */
	u32 (*usb_copy)(u32 num_of_block, u32 *dst);

	/* Read iRAM location to check for secondary USB boot mode */
	sec_boot_check = readl(EXYNOS_IRAM_SECONDARY_BASE);
	if (sec_boot_check == EXYNOS_USB_SECONDARY_BOOT)
		bootmode = BOOT_MODE_USB;
#endif

	if (bootmode == BOOT_MODE_OM)
		bootmode = get_boot_mode();

	switch (bootmode) {
#ifdef CONFIG_SPI_BOOTING
	case BOOT_MODE_SERIAL:
		/* Customised function to copy u-boot from SF */
		exynos_spi_copy(param->uboot_size, CONFIG_SYS_TEXT_BASE);
		break;
#endif
	case BOOT_MODE_SD:
		offset = BL2_START_OFFSET;
		size = BL2_SIZE_BLOC_COUNT;
		copy_bl2 = get_irom_func(MMC_INDEX);
		break;
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	case BOOT_MODE_EMMC:
		/* Set the FSYS1 clock divisor value for EMMC boot */
		emmc_boot_clk_div_set();

		copy_bl2_from_emmc = get_irom_func(EMMC44_INDEX);
		end_bootop_from_emmc = get_irom_func(EMMC44_END_INDEX);

		copy_bl2_from_emmc(BL2_SIZE_BLOC_COUNT, CONFIG_SYS_TEXT_BASE);
		end_bootop_from_emmc();
		break;
#endif
#ifdef CONFIG_USB_BOOTING
	case BOOT_MODE_USB:
		/*
		 * iROM needs program flow prediction to be disabled
		 * before copy from USB device to RAM
		 */
		is_cr_z_set = config_branch_prediction(0);
		usb_copy = get_irom_func(USB_INDEX);
		usb_copy(0, (u32 *)CONFIG_SYS_TEXT_BASE);
		config_branch_prediction(is_cr_z_set);
		break;
#endif
	default:
		break;
	}

	if (copy_bl2)
		copy_bl2(offset, size, CONFIG_SYS_TEXT_BASE);
}

void memzero(void *s, size_t n)
{
	char *ptr = s;
	size_t i;

	for (i = 0; i < n; i++)
		*ptr++ = '\0';
}

/**
 * Set up the U-Boot global_data pointer
 *
 * This sets the address of the global data, and sets up basic values.
 *
 * @param gdp   Value to give to gd
 */
static void setup_global_data(gd_t *gdp)
{
	gd = gdp;
	memzero((void *)gd, sizeof(gd_t));
	gd->flags |= GD_FLG_RELOC;
	gd->baudrate = CONFIG_BAUDRATE;
	gd->have_console = 1;
}

void board_init_f(unsigned long bootflag)
{
	__aligned(8) gd_t local_gd;
	__attribute__((noreturn)) void (*uboot)(void);

	setup_global_data(&local_gd);

	if (do_lowlevel_init())
		power_exit_wakeup();

	copy_uboot_to_ram();

	/* Jump to U-Boot image */
	uboot = (void *)CONFIG_SYS_TEXT_BASE;
	(*uboot)();
	/* Never returns Here */
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/* Function attribute is no-return */
	/* This Function never executes */
	while (1)
		;
}
