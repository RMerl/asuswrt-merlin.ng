// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc
 * Copyright 2019 NXP Semiconductors
 * Andy Fleming
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <clk.h>
#include <errno.h>
#include <hwconfig.h>
#include <mmc.h>
#include <part.h>
#include <power/regulator.h>
#include <malloc.h>
#include <fsl_esdhc.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <dm.h>
#include <asm-generic/gpio.h>
#include <dm/pinctrl.h>

#if !CONFIG_IS_ENABLED(BLK)
#include "mmc_private.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#define SDHCI_IRQ_EN_BITS		(IRQSTATEN_CC | IRQSTATEN_TC | \
				IRQSTATEN_CINT | \
				IRQSTATEN_CTOE | IRQSTATEN_CCE | IRQSTATEN_CEBE | \
				IRQSTATEN_CIE | IRQSTATEN_DTOE | IRQSTATEN_DCE | \
				IRQSTATEN_DEBE | IRQSTATEN_BRR | IRQSTATEN_BWR | \
				IRQSTATEN_DINT)
#define MAX_TUNING_LOOP 40
#define ESDHC_DRIVER_STAGE_VALUE 0xffffffff

struct fsl_esdhc {
	uint    dsaddr;		/* SDMA system address register */
	uint    blkattr;	/* Block attributes register */
	uint    cmdarg;		/* Command argument register */
	uint    xfertyp;	/* Transfer type register */
	uint    cmdrsp0;	/* Command response 0 register */
	uint    cmdrsp1;	/* Command response 1 register */
	uint    cmdrsp2;	/* Command response 2 register */
	uint    cmdrsp3;	/* Command response 3 register */
	uint    datport;	/* Buffer data port register */
	uint    prsstat;	/* Present state register */
	uint    proctl;		/* Protocol control register */
	uint    sysctl;		/* System Control Register */
	uint    irqstat;	/* Interrupt status register */
	uint    irqstaten;	/* Interrupt status enable register */
	uint    irqsigen;	/* Interrupt signal enable register */
	uint    autoc12err;	/* Auto CMD error status register */
	uint    hostcapblt;	/* Host controller capabilities register */
	uint    wml;		/* Watermark level register */
	uint    mixctrl;	/* For USDHC */
	char    reserved1[4];	/* reserved */
	uint    fevt;		/* Force event register */
	uint    admaes;		/* ADMA error status register */
	uint    adsaddr;	/* ADMA system address register */
	char    reserved2[4];
	uint    dllctrl;
	uint    dllstat;
	uint    clktunectrlstatus;
	char    reserved3[4];
	uint	strobe_dllctrl;
	uint	strobe_dllstat;
	char    reserved4[72];
	uint    vendorspec;
	uint    mmcboot;
	uint    vendorspec2;
	uint    tuning_ctrl;	/* on i.MX6/7/8 */
	char	reserved5[44];
	uint    hostver;	/* Host controller version register */
	char    reserved6[4];	/* reserved */
	uint    dmaerraddr;	/* DMA error address register */
	char    reserved7[4];	/* reserved */
	uint    dmaerrattr;	/* DMA error attribute register */
	char    reserved8[4];	/* reserved */
	uint    hostcapblt2;	/* Host controller capabilities register 2 */
	char    reserved9[8];	/* reserved */
	uint    tcr;		/* Tuning control register */
	char    reserved10[28];	/* reserved */
	uint    sddirctl;	/* SD direction control register */
	char    reserved11[712];/* reserved */
	uint    scr;		/* eSDHC control register */
};

struct fsl_esdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct esdhc_soc_data {
	u32 flags;
	u32 caps;
};

/**
 * struct fsl_esdhc_priv
 *
 * @esdhc_regs: registers of the sdhc controller
 * @sdhc_clk: Current clk of the sdhc controller
 * @bus_width: bus width, 1bit, 4bit or 8bit
 * @cfg: mmc config
 * @mmc: mmc
 * Following is used when Driver Model is enabled for MMC
 * @dev: pointer for the device
 * @non_removable: 0: removable; 1: non-removable
 * @wp_enable: 1: enable checking wp; 0: no check
 * @vs18_enable: 1: use 1.8V voltage; 0: use 3.3V
 * @flags: ESDHC_FLAG_xx in include/fsl_esdhc.h
 * @caps: controller capabilities
 * @tuning_step: tuning step setting in tuning_ctrl register
 * @start_tuning_tap: the start point for tuning in tuning_ctrl register
 * @strobe_dll_delay_target: settings in strobe_dllctrl
 * @signal_voltage: indicating the current voltage
 * @cd_gpio: gpio for card detection
 * @wp_gpio: gpio for write protection
 */
struct fsl_esdhc_priv {
	struct fsl_esdhc *esdhc_regs;
	unsigned int sdhc_clk;
	struct clk per_clk;
	unsigned int clock;
	unsigned int mode;
	unsigned int bus_width;
#if !CONFIG_IS_ENABLED(BLK)
	struct mmc *mmc;
#endif
	struct udevice *dev;
	int non_removable;
	int wp_enable;
	int vs18_enable;
	u32 flags;
	u32 caps;
	u32 tuning_step;
	u32 tuning_start_tap;
	u32 strobe_dll_delay_target;
	u32 signal_voltage;
#if IS_ENABLED(CONFIG_DM_REGULATOR)
	struct udevice *vqmmc_dev;
	struct udevice *vmmc_dev;
#endif
#ifdef CONFIG_DM_GPIO
	struct gpio_desc cd_gpio;
	struct gpio_desc wp_gpio;
#endif
};

/* Return the XFERTYP flags for a given command and data packet */
static uint esdhc_xfertyp(struct mmc_cmd *cmd, struct mmc_data *data)
{
	uint xfertyp = 0;

	if (data) {
		xfertyp |= XFERTYP_DPSEL;
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
		xfertyp |= XFERTYP_DMAEN;
#endif
		if (data->blocks > 1) {
			xfertyp |= XFERTYP_MSBSEL;
			xfertyp |= XFERTYP_BCEN;
#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC111
			xfertyp |= XFERTYP_AC12EN;
#endif
		}

		if (data->flags & MMC_DATA_READ)
			xfertyp |= XFERTYP_DTDSEL;
	}

	if (cmd->resp_type & MMC_RSP_CRC)
		xfertyp |= XFERTYP_CCCEN;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		xfertyp |= XFERTYP_CICEN;
	if (cmd->resp_type & MMC_RSP_136)
		xfertyp |= XFERTYP_RSPTYP_136;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		xfertyp |= XFERTYP_RSPTYP_48_BUSY;
	else if (cmd->resp_type & MMC_RSP_PRESENT)
		xfertyp |= XFERTYP_RSPTYP_48;

	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		xfertyp |= XFERTYP_CMDTYP_ABORT;

	return XFERTYP_CMD(cmd->cmdidx) | xfertyp;
}

#ifdef CONFIG_SYS_FSL_ESDHC_USE_PIO
/*
 * PIO Read/Write Mode reduce the performace as DMA is not used in this mode.
 */
static void esdhc_pio_read_write(struct fsl_esdhc_priv *priv,
				 struct mmc_data *data)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	uint blocks;
	char *buffer;
	uint databuf;
	uint size;
	uint irqstat;
	ulong start;

	if (data->flags & MMC_DATA_READ) {
		blocks = data->blocks;
		buffer = data->dest;
		while (blocks) {
			start = get_timer(0);
			size = data->blocksize;
			irqstat = esdhc_read32(&regs->irqstat);
			while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_BREN)) {
				if (get_timer(start) > PIO_TIMEOUT) {
					printf("\nData Read Failed in PIO Mode.");
					return;
				}
			}
			while (size && (!(irqstat & IRQSTAT_TC))) {
				udelay(100); /* Wait before last byte transfer complete */
				irqstat = esdhc_read32(&regs->irqstat);
				databuf = in_le32(&regs->datport);
				*((uint *)buffer) = databuf;
				buffer += 4;
				size -= 4;
			}
			blocks--;
		}
	} else {
		blocks = data->blocks;
		buffer = (char *)data->src;
		while (blocks) {
			start = get_timer(0);
			size = data->blocksize;
			irqstat = esdhc_read32(&regs->irqstat);
			while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_BWEN)) {
				if (get_timer(start) > PIO_TIMEOUT) {
					printf("\nData Write Failed in PIO Mode.");
					return;
				}
			}
			while (size && (!(irqstat & IRQSTAT_TC))) {
				udelay(100); /* Wait before last byte transfer complete */
				databuf = *((uint *)buffer);
				buffer += 4;
				size -= 4;
				irqstat = esdhc_read32(&regs->irqstat);
				out_le32(&regs->datport, databuf);
			}
			blocks--;
		}
	}
}
#endif

static int esdhc_setup_data(struct fsl_esdhc_priv *priv, struct mmc *mmc,
			    struct mmc_data *data)
{
	int timeout;
	struct fsl_esdhc *regs = priv->esdhc_regs;
#if defined(CONFIG_FSL_LAYERSCAPE) || defined(CONFIG_S32V234) || \
	defined(CONFIG_IMX8) || defined(CONFIG_IMX8M)
	dma_addr_t addr;
#endif
	uint wml_value;

	wml_value = data->blocksize/4;

	if (data->flags & MMC_DATA_READ) {
		if (wml_value > WML_RD_WML_MAX)
			wml_value = WML_RD_WML_MAX_VAL;

		esdhc_clrsetbits32(&regs->wml, WML_RD_WML_MASK, wml_value);
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
#if defined(CONFIG_FSL_LAYERSCAPE) || defined(CONFIG_S32V234) || \
	defined(CONFIG_IMX8) || defined(CONFIG_IMX8M)
		addr = virt_to_phys((void *)(data->dest));
		if (upper_32_bits(addr))
			printf("Error found for upper 32 bits\n");
		else
			esdhc_write32(&regs->dsaddr, lower_32_bits(addr));
#else
		esdhc_write32(&regs->dsaddr, (u32)data->dest);
#endif
#endif
	} else {
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
		flush_dcache_range((ulong)data->src,
				   (ulong)data->src+data->blocks
					 *data->blocksize);
#endif
		if (wml_value > WML_WR_WML_MAX)
			wml_value = WML_WR_WML_MAX_VAL;
		if (priv->wp_enable) {
			if ((esdhc_read32(&regs->prsstat) &
			    PRSSTAT_WPSPL) == 0) {
				printf("\nThe SD card is locked. Can not write to a locked card.\n\n");
				return -ETIMEDOUT;
			}
		} else {
#ifdef CONFIG_DM_GPIO
			if (dm_gpio_is_valid(&priv->wp_gpio) && dm_gpio_get_value(&priv->wp_gpio)) {
				printf("\nThe SD card is locked. Can not write to a locked card.\n\n");
				return -ETIMEDOUT;
			}
#endif
		}

		esdhc_clrsetbits32(&regs->wml, WML_WR_WML_MASK,
					wml_value << 16);
#ifndef CONFIG_SYS_FSL_ESDHC_USE_PIO
#if defined(CONFIG_FSL_LAYERSCAPE) || defined(CONFIG_S32V234) || \
	defined(CONFIG_IMX8) || defined(CONFIG_IMX8M)
		addr = virt_to_phys((void *)(data->src));
		if (upper_32_bits(addr))
			printf("Error found for upper 32 bits\n");
		else
			esdhc_write32(&regs->dsaddr, lower_32_bits(addr));
#else
		esdhc_write32(&regs->dsaddr, (u32)data->src);
#endif
#endif
	}

	esdhc_write32(&regs->blkattr, data->blocks << 16 | data->blocksize);

	/* Calculate the timeout period for data transactions */
	/*
	 * 1)Timeout period = (2^(timeout+13)) SD Clock cycles
	 * 2)Timeout period should be minimum 0.250sec as per SD Card spec
	 *  So, Number of SD Clock cycles for 0.25sec should be minimum
	 *		(SD Clock/sec * 0.25 sec) SD Clock cycles
	 *		= (mmc->clock * 1/4) SD Clock cycles
	 * As 1) >=  2)
	 * => (2^(timeout+13)) >= mmc->clock * 1/4
	 * Taking log2 both the sides
	 * => timeout + 13 >= log2(mmc->clock/4)
	 * Rounding up to next power of 2
	 * => timeout + 13 = log2(mmc->clock/4) + 1
	 * => timeout + 13 = fls(mmc->clock/4)
	 *
	 * However, the MMC spec "It is strongly recommended for hosts to
	 * implement more than 500ms timeout value even if the card
	 * indicates the 250ms maximum busy length."  Even the previous
	 * value of 300ms is known to be insufficient for some cards.
	 * So, we use
	 * => timeout + 13 = fls(mmc->clock/2)
	 */
	timeout = fls(mmc->clock/2);
	timeout -= 13;

	if (timeout > 14)
		timeout = 14;

	if (timeout < 0)
		timeout = 0;

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC_A001
	if ((timeout == 4) || (timeout == 8) || (timeout == 12))
		timeout++;
#endif

#ifdef ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE
	timeout = 0xE;
#endif
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, timeout << 16);

	return 0;
}

static void check_and_invalidate_dcache_range
	(struct mmc_cmd *cmd,
	 struct mmc_data *data) {
	unsigned start = 0;
	unsigned end = 0;
	unsigned size = roundup(ARCH_DMA_MINALIGN,
				data->blocks*data->blocksize);
#if defined(CONFIG_FSL_LAYERSCAPE) || defined(CONFIG_S32V234) || \
	defined(CONFIG_IMX8) || defined(CONFIG_IMX8M)
	dma_addr_t addr;

	addr = virt_to_phys((void *)(data->dest));
	if (upper_32_bits(addr))
		printf("Error found for upper 32 bits\n");
	else
		start = lower_32_bits(addr);
#else
	start = (unsigned)data->dest;
#endif
	end = start + size;
	invalidate_dcache_range(start, end);
}

#ifdef CONFIG_MCF5441x
/*
 * Swaps 32-bit words to little-endian byte order.
 */
static inline void sd_swap_dma_buff(struct mmc_data *data)
{
	int i, size = data->blocksize >> 2;
	u32 *buffer = (u32 *)data->dest;
	u32 sw;

	while (data->blocks--) {
		for (i = 0; i < size; i++) {
			sw = __sw32(*buffer);
			*buffer++ = sw;
		}
	}
}
#endif

/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int esdhc_send_cmd_common(struct fsl_esdhc_priv *priv, struct mmc *mmc,
				 struct mmc_cmd *cmd, struct mmc_data *data)
{
	int	err = 0;
	uint	xfertyp;
	uint	irqstat;
	u32	flags = IRQSTAT_CC | IRQSTAT_CTOE;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	unsigned long start;

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC111
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		return 0;
#endif

	esdhc_write32(&regs->irqstat, -1);

	sync();

	/* Wait for the bus to be idle */
	while ((esdhc_read32(&regs->prsstat) & PRSSTAT_CICHB) ||
			(esdhc_read32(&regs->prsstat) & PRSSTAT_CIDHB))
		;

	while (esdhc_read32(&regs->prsstat) & PRSSTAT_DLA)
		;

	/* Wait at least 8 SD clock cycles before the next command */
	/*
	 * Note: This is way more than 8 cycles, but 1ms seems to
	 * resolve timing issues with some cards
	 */
	udelay(1000);

	/* Set up for a data transfer if we have one */
	if (data) {
		err = esdhc_setup_data(priv, mmc, data);
		if(err)
			return err;

		if (data->flags & MMC_DATA_READ)
			check_and_invalidate_dcache_range(cmd, data);
	}

	/* Figure out the transfer arguments */
	xfertyp = esdhc_xfertyp(cmd, data);

	/* Mask all irqs */
	esdhc_write32(&regs->irqsigen, 0);

	/* Send the command */
	esdhc_write32(&regs->cmdarg, cmd->cmdarg);
#if defined(CONFIG_FSL_USDHC)
	esdhc_write32(&regs->mixctrl,
	(esdhc_read32(&regs->mixctrl) & 0xFFFFFF80) | (xfertyp & 0x7F)
			| (mmc->ddr_mode ? XFERTYP_DDREN : 0));
	esdhc_write32(&regs->xfertyp, xfertyp & 0xFFFF0000);
#else
	esdhc_write32(&regs->xfertyp, xfertyp);
#endif

	if ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK) ||
	    (cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200))
		flags = IRQSTAT_BRR;

	/* Wait for the command to complete */
	start = get_timer(0);
	while (!(esdhc_read32(&regs->irqstat) & flags)) {
		if (get_timer(start) > 1000) {
			err = -ETIMEDOUT;
			goto out;
		}
	}

	irqstat = esdhc_read32(&regs->irqstat);

	if (irqstat & CMD_ERR) {
		err = -ECOMM;
		goto out;
	}

	if (irqstat & IRQSTAT_CTOE) {
		err = -ETIMEDOUT;
		goto out;
	}

	/* Switch voltage to 1.8V if CMD11 succeeded */
	if (cmd->cmdidx == SD_CMD_SWITCH_UHS18V) {
		esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);

		printf("Run CMD11 1.8V switch\n");
		/* Sleep for 5 ms - max time for card to switch to 1.8V */
		udelay(5000);
	}

	/* Workaround for ESDHC errata ENGcm03648 */
	if (!data && (cmd->resp_type & MMC_RSP_BUSY)) {
		int timeout = 6000;

		/* Poll on DATA0 line for cmd with busy signal for 600 ms */
		while (timeout > 0 && !(esdhc_read32(&regs->prsstat) &
					PRSSTAT_DAT0)) {
			udelay(100);
			timeout--;
		}

		if (timeout <= 0) {
			printf("Timeout waiting for DAT0 to go high!\n");
			err = -ETIMEDOUT;
			goto out;
		}
	}

	/* Copy the response to the response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		u32 cmdrsp3, cmdrsp2, cmdrsp1, cmdrsp0;

		cmdrsp3 = esdhc_read32(&regs->cmdrsp3);
		cmdrsp2 = esdhc_read32(&regs->cmdrsp2);
		cmdrsp1 = esdhc_read32(&regs->cmdrsp1);
		cmdrsp0 = esdhc_read32(&regs->cmdrsp0);
		cmd->response[0] = (cmdrsp3 << 8) | (cmdrsp2 >> 24);
		cmd->response[1] = (cmdrsp2 << 8) | (cmdrsp1 >> 24);
		cmd->response[2] = (cmdrsp1 << 8) | (cmdrsp0 >> 24);
		cmd->response[3] = (cmdrsp0 << 8);
	} else
		cmd->response[0] = esdhc_read32(&regs->cmdrsp0);

	/* Wait until all of the blocks are transferred */
	if (data) {
#ifdef CONFIG_SYS_FSL_ESDHC_USE_PIO
		esdhc_pio_read_write(priv, data);
#else
		flags = DATA_COMPLETE;
		if ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK) ||
		    (cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200)) {
			flags = IRQSTAT_BRR;
		}

		do {
			irqstat = esdhc_read32(&regs->irqstat);

			if (irqstat & IRQSTAT_DTOE) {
				err = -ETIMEDOUT;
				goto out;
			}

			if (irqstat & DATA_ERR) {
				err = -ECOMM;
				goto out;
			}
		} while ((irqstat & flags) != flags);

		/*
		 * Need invalidate the dcache here again to avoid any
		 * cache-fill during the DMA operations such as the
		 * speculative pre-fetching etc.
		 */
		if (data->flags & MMC_DATA_READ) {
			check_and_invalidate_dcache_range(cmd, data);
#ifdef CONFIG_MCF5441x
			sd_swap_dma_buff(data);
#endif
		}
#endif
	}

out:
	/* Reset CMD and DATA portions on error */
	if (err) {
		esdhc_write32(&regs->sysctl, esdhc_read32(&regs->sysctl) |
			      SYSCTL_RSTC);
		while (esdhc_read32(&regs->sysctl) & SYSCTL_RSTC)
			;

		if (data) {
			esdhc_write32(&regs->sysctl,
				      esdhc_read32(&regs->sysctl) |
				      SYSCTL_RSTD);
			while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTD))
				;
		}

		/* If this was CMD11, then notify that power cycle is needed */
		if (cmd->cmdidx == SD_CMD_SWITCH_UHS18V)
			printf("CMD11 to switch to 1.8V mode failed, card requires power cycle.\n");
	}

	esdhc_write32(&regs->irqstat, -1);

	return err;
}

static void set_sysctl(struct fsl_esdhc_priv *priv, struct mmc *mmc, uint clock)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int div = 1;
#ifdef ARCH_MXC
#ifdef CONFIG_MX53
	/* For i.MX53 eSDHCv3, SYSCTL.SDCLKFS may not be set to 0. */
	int pre_div = (regs == (struct fsl_esdhc *)MMC_SDHC3_BASE_ADDR) ? 2 : 1;
#else
	int pre_div = 1;
#endif
#else
	int pre_div = 2;
#endif
	int ddr_pre_div = mmc->ddr_mode ? 2 : 1;
	int sdhc_clk = priv->sdhc_clk;
	uint clk;

	if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;

	while (sdhc_clk / (16 * pre_div * ddr_pre_div) > clock && pre_div < 256)
		pre_div *= 2;

	while (sdhc_clk / (div * pre_div * ddr_pre_div) > clock && div < 16)
		div++;

	pre_div >>= 1;
	div -= 1;

	clk = (pre_div << 8) | (div << 4);

#ifdef CONFIG_FSL_USDHC
	esdhc_clrbits32(&regs->vendorspec, VENDORSPEC_CKEN);
#else
	esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);
#endif

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_CLOCK_MASK, clk);

	udelay(10000);

#ifdef CONFIG_FSL_USDHC
	esdhc_setbits32(&regs->vendorspec, VENDORSPEC_PEREN | VENDORSPEC_CKEN);
#else
	esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_CKEN);
#endif

	priv->clock = clock;
}

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
static void esdhc_clock_control(struct fsl_esdhc_priv *priv, bool enable)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	u32 value;
	u32 time_out;

	value = esdhc_read32(&regs->sysctl);

	if (enable)
		value |= SYSCTL_CKEN;
	else
		value &= ~SYSCTL_CKEN;

	esdhc_write32(&regs->sysctl, value);

	time_out = 20;
	value = PRSSTAT_SDSTB;
	while (!(esdhc_read32(&regs->prsstat) & value)) {
		if (time_out == 0) {
			printf("fsl_esdhc: Internal clock never stabilised.\n");
			break;
		}
		time_out--;
		mdelay(1);
	}
}
#endif

#ifdef MMC_SUPPORTS_TUNING
static int esdhc_change_pinstate(struct udevice *dev)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	int ret;

	switch (priv->mode) {
	case UHS_SDR50:
	case UHS_DDR50:
		ret = pinctrl_select_state(dev, "state_100mhz");
		break;
	case UHS_SDR104:
	case MMC_HS_200:
	case MMC_HS_400:
		ret = pinctrl_select_state(dev, "state_200mhz");
		break;
	default:
		ret = pinctrl_select_state(dev, "default");
		break;
	}

	if (ret)
		printf("%s %d error\n", __func__, priv->mode);

	return ret;
}

static void esdhc_reset_tuning(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(mmc->dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;

	if (priv->flags & ESDHC_FLAG_USDHC) {
		if (priv->flags & ESDHC_FLAG_STD_TUNING) {
			esdhc_clrbits32(&regs->autoc12err,
					MIX_CTRL_SMPCLK_SEL |
					MIX_CTRL_EXE_TUNE);
		}
	}
}

static void esdhc_set_strobe_dll(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(mmc->dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;
	u32 val;

	if (priv->clock > ESDHC_STROBE_DLL_CLK_FREQ) {
		writel(ESDHC_STROBE_DLL_CTRL_RESET, &regs->strobe_dllctrl);

		/*
		 * enable strobe dll ctrl and adjust the delay target
		 * for the uSDHC loopback read clock
		 */
		val = ESDHC_STROBE_DLL_CTRL_ENABLE |
			(priv->strobe_dll_delay_target <<
			 ESDHC_STROBE_DLL_CTRL_SLV_DLY_TARGET_SHIFT);
		writel(val, &regs->strobe_dllctrl);
		/* wait 1us to make sure strobe dll status register stable */
		mdelay(1);
		val = readl(&regs->strobe_dllstat);
		if (!(val & ESDHC_STROBE_DLL_STS_REF_LOCK))
			pr_warn("HS400 strobe DLL status REF not lock!\n");
		if (!(val & ESDHC_STROBE_DLL_STS_SLV_LOCK))
			pr_warn("HS400 strobe DLL status SLV not lock!\n");
	}
}

static int esdhc_set_timing(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(mmc->dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;
	u32 mixctrl;

	mixctrl = readl(&regs->mixctrl);
	mixctrl &= ~(MIX_CTRL_DDREN | MIX_CTRL_HS400_EN);

	switch (mmc->selected_mode) {
	case MMC_LEGACY:
	case SD_LEGACY:
		esdhc_reset_tuning(mmc);
		writel(mixctrl, &regs->mixctrl);
		break;
	case MMC_HS_400:
		mixctrl |= MIX_CTRL_DDREN | MIX_CTRL_HS400_EN;
		writel(mixctrl, &regs->mixctrl);
		esdhc_set_strobe_dll(mmc);
		break;
	case MMC_HS:
	case MMC_HS_52:
	case MMC_HS_200:
	case SD_HS:
	case UHS_SDR12:
	case UHS_SDR25:
	case UHS_SDR50:
	case UHS_SDR104:
		writel(mixctrl, &regs->mixctrl);
		break;
	case UHS_DDR50:
	case MMC_DDR_52:
		mixctrl |= MIX_CTRL_DDREN;
		writel(mixctrl, &regs->mixctrl);
		break;
	default:
		printf("Not supported %d\n", mmc->selected_mode);
		return -EINVAL;
	}

	priv->mode = mmc->selected_mode;

	return esdhc_change_pinstate(mmc->dev);
}

static int esdhc_set_voltage(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(mmc->dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int ret;

	priv->signal_voltage = mmc->signal_voltage;
	switch (mmc->signal_voltage) {
	case MMC_SIGNAL_VOLTAGE_330:
		if (priv->vs18_enable)
			return -EIO;
#if CONFIG_IS_ENABLED(DM_REGULATOR)
		if (!IS_ERR_OR_NULL(priv->vqmmc_dev)) {
			ret = regulator_set_value(priv->vqmmc_dev, 3300000);
			if (ret) {
				printf("Setting to 3.3V error");
				return -EIO;
			}
			/* Wait for 5ms */
			mdelay(5);
		}
#endif

		esdhc_clrbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);
		if (!(esdhc_read32(&regs->vendorspec) &
		    ESDHC_VENDORSPEC_VSELECT))
			return 0;

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_180:
#if CONFIG_IS_ENABLED(DM_REGULATOR)
		if (!IS_ERR_OR_NULL(priv->vqmmc_dev)) {
			ret = regulator_set_value(priv->vqmmc_dev, 1800000);
			if (ret) {
				printf("Setting to 1.8V error");
				return -EIO;
			}
		}
#endif
		esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);
		if (esdhc_read32(&regs->vendorspec) & ESDHC_VENDORSPEC_VSELECT)
			return 0;

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_120:
		return -ENOTSUPP;
	default:
		return 0;
	}
}

static void esdhc_stop_tuning(struct mmc *mmc)
{
	struct mmc_cmd cmd;

	cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1b;

	dm_mmc_send_cmd(mmc->dev, &cmd, NULL);
}

static int fsl_esdhc_execute_tuning(struct udevice *dev, uint32_t opcode)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;
	struct mmc *mmc = &plat->mmc;
	u32 irqstaten = readl(&regs->irqstaten);
	u32 irqsigen = readl(&regs->irqsigen);
	int i, ret = -ETIMEDOUT;
	u32 val, mixctrl;

	/* clock tuning is not needed for upto 52MHz */
	if (mmc->clock <= 52000000)
		return 0;

	/* This is readw/writew SDHCI_HOST_CONTROL2 when tuning */
	if (priv->flags & ESDHC_FLAG_STD_TUNING) {
		val = readl(&regs->autoc12err);
		mixctrl = readl(&regs->mixctrl);
		val &= ~MIX_CTRL_SMPCLK_SEL;
		mixctrl &= ~(MIX_CTRL_FBCLK_SEL | MIX_CTRL_AUTO_TUNE_EN);

		val |= MIX_CTRL_EXE_TUNE;
		mixctrl |= MIX_CTRL_FBCLK_SEL | MIX_CTRL_AUTO_TUNE_EN;

		writel(val, &regs->autoc12err);
		writel(mixctrl, &regs->mixctrl);
	}

	/* sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE); */
	mixctrl = readl(&regs->mixctrl);
	mixctrl = MIX_CTRL_DTDSEL_READ | (mixctrl & ~MIX_CTRL_SDHCI_MASK);
	writel(mixctrl, &regs->mixctrl);

	writel(IRQSTATEN_BRR, &regs->irqstaten);
	writel(IRQSTATEN_BRR, &regs->irqsigen);

	/*
	 * Issue opcode repeatedly till Execute Tuning is set to 0 or the number
	 * of loops reaches 40 times.
	 */
	for (i = 0; i < MAX_TUNING_LOOP; i++) {
		u32 ctrl;

		if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200) {
			if (mmc->bus_width == 8)
				writel(0x7080, &regs->blkattr);
			else if (mmc->bus_width == 4)
				writel(0x7040, &regs->blkattr);
		} else {
			writel(0x7040, &regs->blkattr);
		}

		/* sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE) */
		val = readl(&regs->mixctrl);
		val = MIX_CTRL_DTDSEL_READ | (val & ~MIX_CTRL_SDHCI_MASK);
		writel(val, &regs->mixctrl);

		/* We are using STD tuning, no need to check return value */
		mmc_send_tuning(mmc, opcode, NULL);

		ctrl = readl(&regs->autoc12err);
		if ((!(ctrl & MIX_CTRL_EXE_TUNE)) &&
		    (ctrl & MIX_CTRL_SMPCLK_SEL)) {
			/*
			 * need to wait some time, make sure sd/mmc fininsh
			 * send out tuning data, otherwise, the sd/mmc can't
			 * response to any command when the card still out
			 * put the tuning data.
			 */
			mdelay(1);
			ret = 0;
			break;
		}

		/* Add 1ms delay for SD and eMMC */
		mdelay(1);
	}

	writel(irqstaten, &regs->irqstaten);
	writel(irqsigen, &regs->irqsigen);

	esdhc_stop_tuning(mmc);

	return ret;
}
#endif

static int esdhc_set_ios_common(struct fsl_esdhc_priv *priv, struct mmc *mmc)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int ret __maybe_unused;

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
	/* Select to use peripheral clock */
	esdhc_clock_control(priv, false);
	esdhc_setbits32(&regs->scr, ESDHCCTL_PCS);
	esdhc_clock_control(priv, true);
#endif
	/* Set the clock speed */
	if (priv->clock != mmc->clock)
		set_sysctl(priv, mmc, mmc->clock);

#ifdef MMC_SUPPORTS_TUNING
	if (mmc->clk_disable) {
#ifdef CONFIG_FSL_USDHC
		esdhc_clrbits32(&regs->vendorspec, VENDORSPEC_CKEN);
#else
		esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);
#endif
	} else {
#ifdef CONFIG_FSL_USDHC
		esdhc_setbits32(&regs->vendorspec, VENDORSPEC_PEREN |
				VENDORSPEC_CKEN);
#else
		esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_CKEN);
#endif
	}

	if (priv->mode != mmc->selected_mode) {
		ret = esdhc_set_timing(mmc);
		if (ret) {
			printf("esdhc_set_timing error %d\n", ret);
			return ret;
		}
	}

	if (priv->signal_voltage != mmc->signal_voltage) {
		ret = esdhc_set_voltage(mmc);
		if (ret) {
			printf("esdhc_set_voltage error %d\n", ret);
			return ret;
		}
	}
#endif

	/* Set the bus width */
	esdhc_clrbits32(&regs->proctl, PROCTL_DTW_4 | PROCTL_DTW_8);

	if (mmc->bus_width == 4)
		esdhc_setbits32(&regs->proctl, PROCTL_DTW_4);
	else if (mmc->bus_width == 8)
		esdhc_setbits32(&regs->proctl, PROCTL_DTW_8);

	return 0;
}

static int esdhc_init_common(struct fsl_esdhc_priv *priv, struct mmc *mmc)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	ulong start;

	/* Reset the entire host controller */
	esdhc_setbits32(&regs->sysctl, SYSCTL_RSTA);

	/* Wait until the controller is available */
	start = get_timer(0);
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA)) {
		if (get_timer(start) > 1000)
			return -ETIMEDOUT;
	}

#if defined(CONFIG_FSL_USDHC)
	/* RSTA doesn't reset MMC_BOOT register, so manually reset it */
	esdhc_write32(&regs->mmcboot, 0x0);
	/* Reset MIX_CTRL and CLK_TUNE_CTRL_STATUS regs to 0 */
	esdhc_write32(&regs->mixctrl, 0x0);
	esdhc_write32(&regs->clktunectrlstatus, 0x0);

	/* Put VEND_SPEC to default value */
	if (priv->vs18_enable)
		esdhc_write32(&regs->vendorspec, (VENDORSPEC_INIT |
			      ESDHC_VENDORSPEC_VSELECT));
	else
		esdhc_write32(&regs->vendorspec, VENDORSPEC_INIT);

	/* Disable DLL_CTRL delay line */
	esdhc_write32(&regs->dllctrl, 0x0);
#endif

#ifndef ARCH_MXC
	/* Enable cache snooping */
	esdhc_write32(&regs->scr, 0x00000040);
#endif

#ifndef CONFIG_FSL_USDHC
	esdhc_setbits32(&regs->sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);
#else
	esdhc_setbits32(&regs->vendorspec, VENDORSPEC_HCKEN | VENDORSPEC_IPGEN);
#endif

	/* Set the initial clock speed */
	mmc_set_clock(mmc, 400000, MMC_CLK_ENABLE);

	/* Disable the BRR and BWR bits in IRQSTAT */
	esdhc_clrbits32(&regs->irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

#ifdef CONFIG_MCF5441x
	esdhc_write32(&regs->proctl, PROCTL_INIT | PROCTL_D3CD);
#else
	/* Put the PROCTL reg back to the default */
	esdhc_write32(&regs->proctl, PROCTL_INIT);
#endif

	/* Set timout to the maximum value */
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

	return 0;
}

static int esdhc_getcd_common(struct fsl_esdhc_priv *priv)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int timeout = 1000;

#ifdef CONFIG_ESDHC_DETECT_QUIRK
	if (CONFIG_ESDHC_DETECT_QUIRK)
		return 1;
#endif

#if CONFIG_IS_ENABLED(DM_MMC)
	if (priv->non_removable)
		return 1;
#ifdef CONFIG_DM_GPIO
	if (dm_gpio_is_valid(&priv->cd_gpio))
		return dm_gpio_get_value(&priv->cd_gpio);
#endif
#endif

	while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_CINS) && --timeout)
		udelay(1000);

	return timeout > 0;
}

static int esdhc_reset(struct fsl_esdhc *regs)
{
	ulong start;

	/* reset the controller */
	esdhc_setbits32(&regs->sysctl, SYSCTL_RSTA);

	/* hardware clears the bit when it is done */
	start = get_timer(0);
	while ((esdhc_read32(&regs->sysctl) & SYSCTL_RSTA)) {
		if (get_timer(start) > 100) {
			printf("MMC/SD: Reset never completed.\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int esdhc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_getcd_common(priv);
}

static int esdhc_init(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_init_common(priv, mmc);
}

static int esdhc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			  struct mmc_data *data)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_send_cmd_common(priv, mmc, cmd, data);
}

static int esdhc_set_ios(struct mmc *mmc)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_set_ios_common(priv, mmc);
}

static const struct mmc_ops esdhc_ops = {
	.getcd		= esdhc_getcd,
	.init		= esdhc_init,
	.send_cmd	= esdhc_send_cmd,
	.set_ios	= esdhc_set_ios,
};
#endif

static int fsl_esdhc_init(struct fsl_esdhc_priv *priv,
			  struct fsl_esdhc_plat *plat)
{
	struct mmc_config *cfg;
	struct fsl_esdhc *regs;
	u32 caps, voltage_caps;
	int ret;

	if (!priv)
		return -EINVAL;

	regs = priv->esdhc_regs;

	/* First reset the eSDHC controller */
	ret = esdhc_reset(regs);
	if (ret)
		return ret;

#ifdef CONFIG_MCF5441x
	/* ColdFire, using SDHC_DATA[3] for card detection */
	esdhc_write32(&regs->proctl, PROCTL_INIT | PROCTL_D3CD);
#endif

#ifndef CONFIG_FSL_USDHC
	esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_HCKEN
				| SYSCTL_IPGEN | SYSCTL_CKEN);
	/* Clearing tuning bits in case ROM has set it already */
	esdhc_write32(&regs->mixctrl, 0);
	esdhc_write32(&regs->autoc12err, 0);
	esdhc_write32(&regs->clktunectrlstatus, 0);
#else
	esdhc_setbits32(&regs->vendorspec, VENDORSPEC_PEREN |
			VENDORSPEC_HCKEN | VENDORSPEC_IPGEN | VENDORSPEC_CKEN);
#endif

	if (priv->vs18_enable)
		esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);

	writel(SDHCI_IRQ_EN_BITS, &regs->irqstaten);
	cfg = &plat->cfg;
#ifndef CONFIG_DM_MMC
	memset(cfg, '\0', sizeof(*cfg));
#endif

	voltage_caps = 0;
	caps = esdhc_read32(&regs->hostcapblt);

#ifdef CONFIG_MCF5441x
	/*
	 * MCF5441x RM declares in more points that sdhc clock speed must
	 * never exceed 25 Mhz. From this, the HS bit needs to be disabled
	 * from host capabilities.
	 */
	caps &= ~ESDHC_HOSTCAPBLT_HSS;
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_ESDHC135
	caps = caps & ~(ESDHC_HOSTCAPBLT_SRS |
			ESDHC_HOSTCAPBLT_VS18 | ESDHC_HOSTCAPBLT_VS30);
#endif

/* T4240 host controller capabilities register should have VS33 bit */
#ifdef CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33
	caps = caps | ESDHC_HOSTCAPBLT_VS33;
#endif

	if (caps & ESDHC_HOSTCAPBLT_VS18)
		voltage_caps |= MMC_VDD_165_195;
	if (caps & ESDHC_HOSTCAPBLT_VS30)
		voltage_caps |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & ESDHC_HOSTCAPBLT_VS33)
		voltage_caps |= MMC_VDD_32_33 | MMC_VDD_33_34;

	cfg->name = "FSL_SDHC";
#if !CONFIG_IS_ENABLED(DM_MMC)
	cfg->ops = &esdhc_ops;
#endif
#ifdef CONFIG_SYS_SD_VOLTAGE
	cfg->voltages = CONFIG_SYS_SD_VOLTAGE;
#else
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
#endif
	if ((cfg->voltages & voltage_caps) == 0) {
		printf("voltage not supported by controller\n");
		return -1;
	}

	if (priv->bus_width == 8)
		cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;
	else if (priv->bus_width == 4)
		cfg->host_caps = MMC_MODE_4BIT;

	cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT;
#ifdef CONFIG_SYS_FSL_ESDHC_HAS_DDR_MODE
	cfg->host_caps |= MMC_MODE_DDR_52MHz;
#endif

	if (priv->bus_width > 0) {
		if (priv->bus_width < 8)
			cfg->host_caps &= ~MMC_MODE_8BIT;
		if (priv->bus_width < 4)
			cfg->host_caps &= ~MMC_MODE_4BIT;
	}

	if (caps & ESDHC_HOSTCAPBLT_HSS)
		cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

#ifdef CONFIG_ESDHC_DETECT_8_BIT_QUIRK
	if (CONFIG_ESDHC_DETECT_8_BIT_QUIRK)
		cfg->host_caps &= ~MMC_MODE_8BIT;
#endif

	cfg->host_caps |= priv->caps;

	cfg->f_min = 400000;
	cfg->f_max = min(priv->sdhc_clk, (u32)200000000);

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	writel(0, &regs->dllctrl);
	if (priv->flags & ESDHC_FLAG_USDHC) {
		if (priv->flags & ESDHC_FLAG_STD_TUNING) {
			u32 val = readl(&regs->tuning_ctrl);

			val |= ESDHC_STD_TUNING_EN;
			val &= ~ESDHC_TUNING_START_TAP_MASK;
			val |= priv->tuning_start_tap;
			val &= ~ESDHC_TUNING_STEP_MASK;
			val |= (priv->tuning_step) << ESDHC_TUNING_STEP_SHIFT;
			writel(val, &regs->tuning_ctrl);
		}
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int fsl_esdhc_cfg_to_priv(struct fsl_esdhc_cfg *cfg,
				 struct fsl_esdhc_priv *priv)
{
	if (!cfg || !priv)
		return -EINVAL;

	priv->esdhc_regs = (struct fsl_esdhc *)(unsigned long)(cfg->esdhc_base);
	priv->bus_width = cfg->max_bus_width;
	priv->sdhc_clk = cfg->sdhc_clk;
	priv->wp_enable  = cfg->wp_enable;
	priv->vs18_enable  = cfg->vs18_enable;

	return 0;
};

int fsl_esdhc_initialize(bd_t *bis, struct fsl_esdhc_cfg *cfg)
{
	struct fsl_esdhc_plat *plat;
	struct fsl_esdhc_priv *priv;
	struct mmc *mmc;
	int ret;

	if (!cfg)
		return -EINVAL;

	priv = calloc(sizeof(struct fsl_esdhc_priv), 1);
	if (!priv)
		return -ENOMEM;
	plat = calloc(sizeof(struct fsl_esdhc_plat), 1);
	if (!plat) {
		free(priv);
		return -ENOMEM;
	}

	ret = fsl_esdhc_cfg_to_priv(cfg, priv);
	if (ret) {
		debug("%s xlate failure\n", __func__);
		free(plat);
		free(priv);
		return ret;
	}

	ret = fsl_esdhc_init(priv, plat);
	if (ret) {
		debug("%s init failure\n", __func__);
		free(plat);
		free(priv);
		return ret;
	}

	mmc = mmc_create(&plat->cfg, priv);
	if (!mmc)
		return -EIO;

	priv->mmc = mmc;

	return 0;
}

int fsl_esdhc_mmc_init(bd_t *bis)
{
	struct fsl_esdhc_cfg *cfg;

	cfg = calloc(sizeof(struct fsl_esdhc_cfg), 1);
	cfg->esdhc_base = CONFIG_SYS_FSL_ESDHC_ADDR;
	cfg->sdhc_clk = gd->arch.sdhc_clk;
	return fsl_esdhc_initialize(bis, cfg);
}
#endif

#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
void mmc_adapter_card_type_ident(void)
{
	u8 card_id;
	u8 value;

	card_id = QIXIS_READ(present) & QIXIS_SDID_MASK;
	gd->arch.sdhc_adapter = card_id;

	switch (card_id) {
	case QIXIS_ESDHC_ADAPTER_TYPE_EMMC45:
		value = QIXIS_READ(brdcfg[5]);
		value |= (QIXIS_DAT4 | QIXIS_DAT5_6_7);
		QIXIS_WRITE(brdcfg[5], value);
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_SDMMC_LEGACY:
		value = QIXIS_READ(pwr_ctl[1]);
		value |= QIXIS_EVDD_BY_SDHC_VS;
		QIXIS_WRITE(pwr_ctl[1], value);
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_EMMC44:
		value = QIXIS_READ(brdcfg[5]);
		value |= (QIXIS_SDCLKIN | QIXIS_SDCLKOUT);
		QIXIS_WRITE(brdcfg[5], value);
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_RSV:
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_MMC:
		break;
	case QIXIS_ESDHC_ADAPTER_TYPE_SD:
		break;
	case QIXIS_ESDHC_NO_ADAPTER:
		break;
	default:
		break;
	}
}
#endif

#ifdef CONFIG_OF_LIBFDT
__weak int esdhc_status_fixup(void *blob, const char *compat)
{
#ifdef CONFIG_FSL_ESDHC_PIN_MUX
	if (!hwconfig("esdhc")) {
		do_fixup_by_compat(blob, compat, "status", "disabled",
				sizeof("disabled"), 1);
		return 1;
	}
#endif
	return 0;
}

void fdt_fixup_esdhc(void *blob, bd_t *bd)
{
	const char *compat = "fsl,esdhc";

	if (esdhc_status_fixup(blob, compat))
		return;

#ifdef CONFIG_FSL_ESDHC_USE_PERIPHERAL_CLK
	do_fixup_by_compat_u32(blob, compat, "peripheral-frequency",
			       gd->arch.sdhc_clk, 1);
#else
	do_fixup_by_compat_u32(blob, compat, "clock-frequency",
			       gd->arch.sdhc_clk, 1);
#endif
#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
	do_fixup_by_compat_u32(blob, compat, "adapter-type",
			       (u32)(gd->arch.sdhc_adapter), 1);
#endif
}
#endif

#if CONFIG_IS_ENABLED(DM_MMC)
#ifndef CONFIG_PPC
#include <asm/arch/clock.h>
#endif
__weak void init_clk_usdhc(u32 index)
{
}

static int fsl_esdhc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	struct esdhc_soc_data *data =
		(struct esdhc_soc_data *)dev_get_driver_data(dev);
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	struct udevice *vqmmc_dev;
#endif
	fdt_addr_t addr;
	unsigned int val;
	struct mmc *mmc;
#if !CONFIG_IS_ENABLED(BLK)
	struct blk_desc *bdesc;
#endif
	int ret;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
#ifdef CONFIG_PPC
	priv->esdhc_regs = (struct fsl_esdhc *)lower_32_bits(addr);
#else
	priv->esdhc_regs = (struct fsl_esdhc *)addr;
#endif
	priv->dev = dev;
	priv->mode = -1;
	if (data) {
		priv->flags = data->flags;
		priv->caps = data->caps;
	}

	val = dev_read_u32_default(dev, "bus-width", -1);
	if (val == 8)
		priv->bus_width = 8;
	else if (val == 4)
		priv->bus_width = 4;
	else
		priv->bus_width = 1;

	val = fdtdec_get_int(fdt, node, "fsl,tuning-step", 1);
	priv->tuning_step = val;
	val = fdtdec_get_int(fdt, node, "fsl,tuning-start-tap",
			     ESDHC_TUNING_START_TAP_DEFAULT);
	priv->tuning_start_tap = val;
	val = fdtdec_get_int(fdt, node, "fsl,strobe-dll-delay-target",
			     ESDHC_STROBE_DLL_CTRL_SLV_DLY_TARGET_DEFAULT);
	priv->strobe_dll_delay_target = val;

	if (dev_read_bool(dev, "non-removable")) {
		priv->non_removable = 1;
	 } else {
		priv->non_removable = 0;
#ifdef CONFIG_DM_GPIO
		gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio,
				     GPIOD_IS_IN);
#endif
	}

	if (dev_read_prop(dev, "fsl,wp-controller", NULL)) {
		priv->wp_enable = 1;
	} else {
		priv->wp_enable = 0;
#ifdef CONFIG_DM_GPIO
		gpio_request_by_name(dev, "wp-gpios", 0, &priv->wp_gpio,
				   GPIOD_IS_IN);
#endif
	}

	priv->vs18_enable = 0;

#if CONFIG_IS_ENABLED(DM_REGULATOR)
	/*
	 * If emmc I/O has a fixed voltage at 1.8V, this must be provided,
	 * otherwise, emmc will work abnormally.
	 */
	ret = device_get_supply_regulator(dev, "vqmmc-supply", &vqmmc_dev);
	if (ret) {
		dev_dbg(dev, "no vqmmc-supply\n");
	} else {
		ret = regulator_set_enable(vqmmc_dev, true);
		if (ret) {
			dev_err(dev, "fail to enable vqmmc-supply\n");
			return ret;
		}

		if (regulator_get_value(vqmmc_dev) == 1800000)
			priv->vs18_enable = 1;
	}
#endif

	if (fdt_get_property(fdt, node, "no-1-8-v", NULL))
		priv->caps &= ~(UHS_CAPS | MMC_MODE_HS200 | MMC_MODE_HS400);

	/*
	 * TODO:
	 * Because lack of clk driver, if SDHC clk is not enabled,
	 * need to enable it first before this driver is invoked.
	 *
	 * we use MXC_ESDHC_CLK to get clk freq.
	 * If one would like to make this function work,
	 * the aliases should be provided in dts as this:
	 *
	 *  aliases {
	 *	mmc0 = &usdhc1;
	 *	mmc1 = &usdhc2;
	 *	mmc2 = &usdhc3;
	 *	mmc3 = &usdhc4;
	 *	};
	 * Then if your board only supports mmc2 and mmc3, but we can
	 * correctly get the seq as 2 and 3, then let mxc_get_clock
	 * work as expected.
	 */

	init_clk_usdhc(dev->seq);

	if (IS_ENABLED(CONFIG_CLK)) {
		/* Assigned clock already set clock */
		ret = clk_get_by_name(dev, "per", &priv->per_clk);
		if (ret) {
			printf("Failed to get per_clk\n");
			return ret;
		}
		ret = clk_enable(&priv->per_clk);
		if (ret) {
			printf("Failed to enable per_clk\n");
			return ret;
		}

		priv->sdhc_clk = clk_get_rate(&priv->per_clk);
	} else {
#ifndef CONFIG_PPC
		priv->sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK + dev->seq);
#else
		priv->sdhc_clk = gd->arch.sdhc_clk;
#endif
		if (priv->sdhc_clk <= 0) {
			dev_err(dev, "Unable to get clk for %s\n", dev->name);
			return -EINVAL;
		}
	}

	ret = fsl_esdhc_init(priv, plat);
	if (ret) {
		dev_err(dev, "fsl_esdhc_init failure\n");
		return ret;
	}

	mmc = &plat->mmc;
	mmc->cfg = &plat->cfg;
	mmc->dev = dev;
#if !CONFIG_IS_ENABLED(BLK)
	mmc->priv = priv;

	/* Setup dsr related values */
	mmc->dsr_imp = 0;
	mmc->dsr = ESDHC_DRIVER_STAGE_VALUE;
	/* Setup the universal parts of the block interface just once */
	bdesc = mmc_get_blk_desc(mmc);
	bdesc->if_type = IF_TYPE_MMC;
	bdesc->removable = 1;
	bdesc->devnum = mmc_get_next_devnum();
	bdesc->block_read = mmc_bread;
	bdesc->block_write = mmc_bwrite;
	bdesc->block_erase = mmc_berase;

	/* setup initial part type */
	bdesc->part_type = mmc->cfg->part_type;
	mmc_list_add(mmc);
#endif

	upriv->mmc = mmc;

	return esdhc_init_common(priv, mmc);
}

#if CONFIG_IS_ENABLED(DM_MMC)
static int fsl_esdhc_get_cd(struct udevice *dev)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_getcd_common(priv);
}

static int fsl_esdhc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_send_cmd_common(priv, &plat->mmc, cmd, data);
}

static int fsl_esdhc_set_ios(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_set_ios_common(priv, &plat->mmc);
}

static const struct dm_mmc_ops fsl_esdhc_ops = {
	.get_cd		= fsl_esdhc_get_cd,
	.send_cmd	= fsl_esdhc_send_cmd,
	.set_ios	= fsl_esdhc_set_ios,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning	= fsl_esdhc_execute_tuning,
#endif
};
#endif

static struct esdhc_soc_data usdhc_imx7d_data = {
	.flags = ESDHC_FLAG_USDHC | ESDHC_FLAG_STD_TUNING
			| ESDHC_FLAG_HAVE_CAP1 | ESDHC_FLAG_HS200
			| ESDHC_FLAG_HS400,
	.caps = UHS_CAPS | MMC_MODE_HS200 | MMC_MODE_DDR_52MHz |
		MMC_MODE_HS_52MHz | MMC_MODE_HS,
};

static const struct udevice_id fsl_esdhc_ids[] = {
	{ .compatible = "fsl,imx53-esdhc", },
	{ .compatible = "fsl,imx6ul-usdhc", },
	{ .compatible = "fsl,imx6sx-usdhc", },
	{ .compatible = "fsl,imx6sl-usdhc", },
	{ .compatible = "fsl,imx6q-usdhc", },
	{ .compatible = "fsl,imx7d-usdhc", .data = (ulong)&usdhc_imx7d_data,},
	{ .compatible = "fsl,imx7ulp-usdhc", },
	{ .compatible = "fsl,esdhc", },
	{ /* sentinel */ }
};

#if CONFIG_IS_ENABLED(BLK)
static int fsl_esdhc_bind(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}
#endif

U_BOOT_DRIVER(fsl_esdhc) = {
	.name	= "fsl-esdhc-mmc",
	.id	= UCLASS_MMC,
	.of_match = fsl_esdhc_ids,
	.ops	= &fsl_esdhc_ops,
#if CONFIG_IS_ENABLED(BLK)
	.bind	= fsl_esdhc_bind,
#endif
	.probe	= fsl_esdhc_probe,
	.platdata_auto_alloc_size = sizeof(struct fsl_esdhc_plat),
	.priv_auto_alloc_size = sizeof(struct fsl_esdhc_priv),
};
#endif
