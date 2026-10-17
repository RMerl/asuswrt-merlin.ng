// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007, 2010-2011 Freescale Semiconductor, Inc
 * Copyright 2019, 2021 NXP
 * Andy Fleming
 * Yangbo Lu <yangbo.lu@nxp.com>
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <clk.h>
#include <cpu_func.h>
#include <errno.h>
#include <hwconfig.h>
#include <log.h>
#include <mmc.h>
#include <part.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/printk.h>
#include <power/regulator.h>
#include <malloc.h>
#include <fsl_esdhc_imx.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <dm.h>
#include <asm-generic/gpio.h>
#include <dm/pinctrl.h>
#include <dt-structs.h>
#include <mapmem.h>
#include <dm/ofnode.h>
#include <linux/iopoll.h>
#include <linux/dma-mapping.h>

#ifndef ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE
#ifdef CONFIG_FSL_USDHC
#define ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE	1
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

#define SDHCI_IRQ_EN_BITS		(IRQSTATEN_CC | IRQSTATEN_TC | \
				IRQSTATEN_CINT | \
				IRQSTATEN_CTOE | IRQSTATEN_CCE | IRQSTATEN_CEBE | \
				IRQSTATEN_CIE | IRQSTATEN_DTOE | IRQSTATEN_DCE | \
				IRQSTATEN_DEBE | IRQSTATEN_BRR | IRQSTATEN_BWR | \
				IRQSTATEN_DINT)
#define MAX_TUNING_LOOP 40

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
	uint    tuning_ctrl;	/* on i.MX6/7/8/RT */
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
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	/* Put this first since driver model will copy the data here */
	struct dtd_fsl_esdhc dtplat;
#endif

	struct mmc_config cfg;
	struct mmc mmc;
};

struct esdhc_soc_data {
	u32 flags;
};

/**
 * struct fsl_esdhc_priv
 *
 * @esdhc_regs: registers of the sdhc controller
 * @sdhc_clk: Current clk of the sdhc controller
 * @cfg: mmc config
 * @mmc: mmc
 * Following is used when Driver Model is enabled for MMC
 * @dev: pointer for the device
 * @broken_cd: 0: use GPIO for card detect; 1: Do not use GPIO for card detect
 * @wp_enable: 1: enable checking wp; 0: no check
 * @vs18_enable: 1: use 1.8V voltage; 0: use 3.3V
 * @flags: ESDHC_FLAG_xx in include/fsl_esdhc_imx.h
 * @caps: controller capabilities
 * @tuning_step: tuning step setting in tuning_ctrl register
 * @start_tuning_tap: the start point for tuning in tuning_ctrl register
 * @strobe_dll_delay_target: settings in strobe_dllctrl
 * @signal_voltage: indicating the current voltage
 * @signal_voltage_switch_extra_delay_ms: extra delay for IO voltage switch
 * @cd_gpio: gpio for card detection
 * @wp_gpio: gpio for write protection
 */
struct fsl_esdhc_priv {
	struct fsl_esdhc *esdhc_regs;
	unsigned int sdhc_clk;
	struct clk per_clk;
	unsigned int clock;
	unsigned int mode;
#if !CONFIG_IS_ENABLED(DM_MMC)
	struct mmc *mmc;
#endif
	struct udevice *dev;
	int broken_cd;
	int wp_enable;
	int vs18_enable;
	u32 flags;
	u32 caps;
	u32 tuning_step;
	u32 tuning_start_tap;
	u32 strobe_dll_delay_target;
	u32 signal_voltage;
	u32 signal_voltage_switch_extra_delay_ms;
	struct udevice *vqmmc_dev;
	struct udevice *vmmc_dev;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct gpio_desc cd_gpio;
	struct gpio_desc wp_gpio;
#endif
	dma_addr_t dma_addr;
};

/* Return the XFERTYP flags for a given command and data packet */
static uint esdhc_xfertyp(struct mmc_cmd *cmd, struct mmc_data *data)
{
	uint xfertyp = 0;

	if (data) {
		xfertyp |= XFERTYP_DPSEL;
		if (!IS_ENABLED(CONFIG_SYS_FSL_ESDHC_USE_PIO) &&
		    cmd->cmdidx != MMC_CMD_SEND_TUNING_BLOCK &&
		    cmd->cmdidx != MMC_CMD_SEND_TUNING_BLOCK_HS200)
			xfertyp |= XFERTYP_DMAEN;
		if (data->blocks > 1) {
			xfertyp |= XFERTYP_MSBSEL;
			xfertyp |= XFERTYP_BCEN;
			if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_ESDHC111))
				xfertyp |= XFERTYP_AC12EN;
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

static void esdhc_setup_watermark_level(struct fsl_esdhc_priv *priv,
					struct mmc_data *data)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	uint wml_value = data->blocksize / 4;

	if (data->flags & MMC_DATA_READ) {
		if (wml_value > WML_RD_WML_MAX)
			wml_value = WML_RD_WML_MAX_VAL;

		esdhc_clrsetbits32(&regs->wml, WML_RD_WML_MASK, wml_value);
	} else {
		if (wml_value > WML_WR_WML_MAX)
			wml_value = WML_WR_WML_MAX_VAL;

		esdhc_clrsetbits32(&regs->wml, WML_WR_WML_MASK,
				   wml_value << 16);
	}
}

static void esdhc_setup_dma(struct fsl_esdhc_priv *priv, struct mmc_data *data)
{
	uint trans_bytes = data->blocksize * data->blocks;
	struct fsl_esdhc *regs = priv->esdhc_regs;
	void *buf;

	if (data->flags & MMC_DATA_WRITE)
		buf = (void *)data->src;
	else
		buf = data->dest;

	priv->dma_addr = dma_map_single(buf, trans_bytes,
					mmc_get_dma_dir(data));
	if (upper_32_bits(priv->dma_addr))
		printf("Cannot use 64 bit addresses with SDMA\n");
	esdhc_write32(&regs->dsaddr, lower_32_bits(priv->dma_addr));
	esdhc_write32(&regs->blkattr, data->blocks << 16 | data->blocksize);
}

static int esdhc_setup_data(struct fsl_esdhc_priv *priv, struct mmc *mmc,
			    struct mmc_data *data)
{
	int timeout;
	bool is_write = data->flags & MMC_DATA_WRITE;
	struct fsl_esdhc *regs = priv->esdhc_regs;

	if (is_write) {
		if (priv->wp_enable && !(esdhc_read32(&regs->prsstat) & PRSSTAT_WPSPL)) {
			printf("Cannot write to locked SD card.\n");
			return -EINVAL;
		} else {
#if CONFIG_IS_ENABLED(DM_GPIO)
			if (dm_gpio_is_valid(&priv->wp_gpio) &&
			    dm_gpio_get_value(&priv->wp_gpio)) {
				printf("Cannot write to locked SD card.\n");
				return -EINVAL;
			}
#endif
		}
	}

	esdhc_setup_watermark_level(priv, data);
	if (!IS_ENABLED(CONFIG_SYS_FSL_ESDHC_USE_PIO))
		esdhc_setup_dma(priv, data);

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

	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_ESDHC_A001) &&
	    (timeout == 4 || timeout == 8 || timeout == 12))
		timeout++;

	if (IS_ENABLED(ESDHCI_QUIRK_BROKEN_TIMEOUT_VALUE))
		timeout = 0xE;

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, timeout << 16);

	return 0;
}

#if IS_ENABLED(CONFIG_MCF5441x)
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
#else
static inline void sd_swap_dma_buff(struct mmc_data *data)
{
	return;
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

	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_ESDHC111) &&
	    cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		return 0;

	esdhc_write32(&regs->irqstat, -1);

	sync();

	/* Wait for the bus to be idle */
	while ((esdhc_read32(&regs->prsstat) & PRSSTAT_CICHB) ||
			(esdhc_read32(&regs->prsstat) & PRSSTAT_CIDHB))
		;

	while (esdhc_read32(&regs->prsstat) & PRSSTAT_DLA)
		;

	/* Set up for a data transfer if we have one */
	if (data) {
		err = esdhc_setup_data(priv, mmc, data);
		if(err)
			return err;
	}

	/* Figure out the transfer arguments */
	xfertyp = esdhc_xfertyp(cmd, data);

	/* Mask all irqs */
	esdhc_write32(&regs->irqsigen, 0);

	/* Send the command */
	esdhc_write32(&regs->cmdarg, cmd->cmdarg);
	if (IS_ENABLED(CONFIG_FSL_USDHC)) {
		u32 mixctrl = esdhc_read32(&regs->mixctrl);

		esdhc_write32(&regs->mixctrl,
			      (mixctrl & 0xFFFFFF80) | (xfertyp & 0x7F)
			      | (mmc->ddr_mode ? XFERTYP_DDREN : 0));
		esdhc_write32(&regs->xfertyp, xfertyp & 0xFFFF0000);
	} else {
		esdhc_write32(&regs->xfertyp, xfertyp);
	}

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

	/* Workaround for ESDHC errata ENGcm03648 */
	if (!data && (cmd->resp_type & MMC_RSP_BUSY)) {
		int timeout = 50000;

		/* Poll on DATA0 line for cmd with busy signal for 5000 ms */
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
		if (IS_ENABLED(CONFIG_SYS_FSL_ESDHC_USE_PIO)) {
			esdhc_pio_read_write(priv, data);
		} else {
			flags = DATA_COMPLETE;
			if (cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK ||
			    cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200)
				flags = IRQSTAT_BRR;

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
			dma_unmap_single(priv->dma_addr,
					 data->blocks * data->blocksize,
					 mmc_get_dma_dir(data));
			if (IS_ENABLED(CONFIG_MCF5441x) &&
			    (data->flags & MMC_DATA_READ))
				sd_swap_dma_buff(data);
		}
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
	u32 tmp;
	int ret, pre_div;
	int ddr_pre_div = mmc->ddr_mode ? 2 : 1;
	int sdhc_clk = priv->sdhc_clk;
	uint clk;

#if IS_ENABLED(CONFIG_MX53)
	/* For i.MX53 eSDHCv3, SYSCTL.SDCLKFS may not be set to 0. */
	pre_div = (regs == (struct fsl_esdhc *)MMC_SDHC3_BASE_ADDR) ? 2 : 1;
#else
	pre_div = 1;
#endif

	while (sdhc_clk / (16 * pre_div * ddr_pre_div) > clock && pre_div < 256)
		pre_div *= 2;

	while (sdhc_clk / (div * pre_div * ddr_pre_div) > clock && div < 16)
		div++;

	mmc->clock = sdhc_clk / pre_div / div / ddr_pre_div;

	pre_div >>= 1;
	div -= 1;

	clk = (pre_div << 8) | (div << 4);

	if (IS_ENABLED(CONFIG_FSL_USDHC))
		esdhc_clrbits32(&regs->vendorspec, VENDORSPEC_CKEN);
	else
		esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);

	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_CLOCK_MASK, clk);

	ret = readx_poll_timeout(esdhc_read32, &regs->prsstat, tmp, tmp & PRSSTAT_SDSTB, 100);
	if (ret)
		pr_warn("fsl_esdhc_imx: Internal clock never stabilised.\n");

	if (IS_ENABLED(CONFIG_FSL_USDHC))
		esdhc_setbits32(&regs->vendorspec, VENDORSPEC_PEREN | VENDORSPEC_CKEN);
	else
		esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_CKEN);

	priv->clock = clock;
}

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
	case MMC_HS_400_ES:
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
		esdhc_write32(&regs->strobe_dllctrl, ESDHC_STROBE_DLL_CTRL_RESET);
		/* clear the reset bit on strobe dll before any setting */
		esdhc_write32(&regs->strobe_dllctrl, 0);

		/*
		 * enable strobe dll ctrl and adjust the delay target
		 * for the uSDHC loopback read clock
		 */
		val = ESDHC_STROBE_DLL_CTRL_ENABLE |
			ESDHC_STROBE_DLL_CTRL_SLV_UPDATE_INT_DEFAULT |
			(priv->strobe_dll_delay_target <<
			 ESDHC_STROBE_DLL_CTRL_SLV_DLY_TARGET_SHIFT);
		esdhc_write32(&regs->strobe_dllctrl, val);
		/* wait 5us to make sure strobe dll status register stable */
		mdelay(5);
		val = esdhc_read32(&regs->strobe_dllstat);
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

	mixctrl = esdhc_read32(&regs->mixctrl);
	mixctrl &= ~(MIX_CTRL_DDREN | MIX_CTRL_HS400_EN);

	switch (mmc->selected_mode) {
	case MMC_LEGACY:
		esdhc_reset_tuning(mmc);
		esdhc_write32(&regs->mixctrl, mixctrl);
		break;
	case MMC_HS_400:
	case MMC_HS_400_ES:
		mixctrl |= MIX_CTRL_DDREN | MIX_CTRL_HS400_EN;
		esdhc_write32(&regs->mixctrl, mixctrl);
		break;
	case MMC_HS:
	case MMC_HS_52:
	case MMC_HS_200:
	case SD_HS:
	case UHS_SDR12:
	case UHS_SDR25:
	case UHS_SDR50:
	case UHS_SDR104:
		esdhc_write32(&regs->mixctrl, mixctrl);
		break;
	case UHS_DDR50:
	case MMC_DDR_52:
		mixctrl |= MIX_CTRL_DDREN;
		esdhc_write32(&regs->mixctrl, mixctrl);
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
			return -ENOTSUPP;
		if (CONFIG_IS_ENABLED(DM_REGULATOR) &&
		    !IS_ERR_OR_NULL(priv->vqmmc_dev)) {
			ret = regulator_set_value(priv->vqmmc_dev,
						  3300000);
			if (ret) {
				printf("Setting to 3.3V error");
				return -EIO;
			}
			mdelay(5);
		}

		esdhc_clrbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);
		if (!(esdhc_read32(&regs->vendorspec) &
		    ESDHC_VENDORSPEC_VSELECT))
			return 0;

		return -EAGAIN;
	case MMC_SIGNAL_VOLTAGE_180:
		if (CONFIG_IS_ENABLED(DM_REGULATOR) &&
		    !IS_ERR_OR_NULL(priv->vqmmc_dev)) {
			ret = regulator_set_value(priv->vqmmc_dev,
						  1800000);
			if (ret) {
				printf("Setting to 1.8V error");
				return -EIO;
			}
		}
		esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);
		/*
		 * some board like imx8mm-evk need about 18ms to switch
		 * the IO voltage from 3.3v to 1.8v, common code only
		 * delay 10ms, so need to delay extra time to make sure
		 * the IO voltage change to 1.8v.
		 */
		if (priv->signal_voltage_switch_extra_delay_ms)
			mdelay(priv->signal_voltage_switch_extra_delay_ms);
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

	mmc_send_cmd(mmc, &cmd, NULL);
}

static int fsl_esdhc_execute_tuning(struct udevice *dev, uint32_t opcode)
{
	struct fsl_esdhc_plat *plat = dev_get_plat(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;
	struct mmc *mmc = &plat->mmc;
	u32 irqstaten = esdhc_read32(&regs->irqstaten);
	u32 irqsigen = esdhc_read32(&regs->irqsigen);
	int i, err, ret = -ETIMEDOUT;
	u32 val, mixctrl, tmp;

	/* clock tuning is not needed for upto 52MHz */
	if (mmc->clock <= 52000000)
		return 0;

	/* make sure the card clock keep on */
	esdhc_setbits32(&regs->vendorspec, VENDORSPEC_FRC_SDCLK_ON);

	/* This is readw/writew SDHCI_HOST_CONTROL2 when tuning */
	if (priv->flags & ESDHC_FLAG_STD_TUNING) {
		val = esdhc_read32(&regs->autoc12err);
		mixctrl = esdhc_read32(&regs->mixctrl);
		val &= ~MIX_CTRL_SMPCLK_SEL;
		mixctrl &= ~(MIX_CTRL_FBCLK_SEL | MIX_CTRL_AUTO_TUNE_EN);

		val |= MIX_CTRL_EXE_TUNE;
		mixctrl |= MIX_CTRL_FBCLK_SEL | MIX_CTRL_AUTO_TUNE_EN;

		esdhc_write32(&regs->autoc12err, val);
		esdhc_write32(&regs->mixctrl, mixctrl);
	}

	/* sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE); */
	mixctrl = esdhc_read32(&regs->mixctrl);
	mixctrl = MIX_CTRL_DTDSEL_READ | (mixctrl & ~MIX_CTRL_SDHCI_MASK);
	esdhc_write32(&regs->mixctrl, mixctrl);

	esdhc_write32(&regs->irqstaten, IRQSTATEN_BRR);
	esdhc_write32(&regs->irqsigen, IRQSTATEN_BRR);

	/*
	 * Issue opcode repeatedly till Execute Tuning is set to 0 or the number
	 * of loops reaches 40 times.
	 */
	for (i = 0; i < MAX_TUNING_LOOP; i++) {
		u32 ctrl;

		if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200) {
			if (mmc->bus_width == 8)
				esdhc_write32(&regs->blkattr, 0x7080);
			else if (mmc->bus_width == 4)
				esdhc_write32(&regs->blkattr, 0x7040);
		} else {
			esdhc_write32(&regs->blkattr, 0x7040);
		}

		/* sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE) */
		val = esdhc_read32(&regs->mixctrl);
		val = MIX_CTRL_DTDSEL_READ | (val & ~MIX_CTRL_SDHCI_MASK);
		esdhc_write32(&regs->mixctrl, val);

		/* We are using STD tuning, no need to check return value */
		mmc_send_tuning(mmc, opcode, NULL);

		ctrl = esdhc_read32(&regs->autoc12err);
		if ((!(ctrl & MIX_CTRL_EXE_TUNE)) &&
		    (ctrl & MIX_CTRL_SMPCLK_SEL)) {
			ret = 0;
			break;
		}
	}

	esdhc_write32(&regs->irqstaten, irqstaten);
	esdhc_write32(&regs->irqsigen, irqsigen);

	esdhc_stop_tuning(mmc);

	/* change to default setting, let host control the card clock */
	esdhc_clrbits32(&regs->vendorspec, VENDORSPEC_FRC_SDCLK_ON);
	err = readx_poll_timeout(esdhc_read32, &regs->prsstat, tmp, tmp & PRSSTAT_SDOFF, 100);
	if (err)
		dev_warn(dev, "card clock not gate off as expect.\n");

	return ret;
}
#endif

static int esdhc_set_ios_common(struct fsl_esdhc_priv *priv, struct mmc *mmc)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int ret __maybe_unused;
	u32 clock;

#ifdef MMC_SUPPORTS_TUNING
	/*
	 * call esdhc_set_timing() before update the clock rate,
	 * This is because current we support DDR and SDR mode,
	 * Once the DDR_EN bit is set, the card clock will be
	 * divide by 2 automatically. So need to do this before
	 * setting clock rate.
	 */
	if (priv->mode != mmc->selected_mode) {
		ret = esdhc_set_timing(mmc);
		if (ret) {
			printf("esdhc_set_timing error %d\n", ret);
			return ret;
		}
	}
#endif

	/* Set the clock speed */
	clock = mmc->clock;
	if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;

	if (priv->clock != clock)
		set_sysctl(priv, mmc, clock);

	if (mmc->clk_disable) {
		if (IS_ENABLED(CONFIG_FSL_USDHC))
			esdhc_clrbits32(&regs->vendorspec, VENDORSPEC_CKEN);
		else
			esdhc_clrbits32(&regs->sysctl, SYSCTL_CKEN);
	} else {
		if (IS_ENABLED(CONFIG_FSL_USDHC))
			esdhc_setbits32(&regs->vendorspec, VENDORSPEC_PEREN |
					VENDORSPEC_CKEN);
		else
			esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_CKEN);
	}

#ifdef MMC_SUPPORTS_TUNING
	/*
	 * For HS400/HS400ES mode, make sure set the strobe dll in the
	 * target clock rate. So call esdhc_set_strobe_dll() after the
	 * clock updated.
	 */
	if (mmc->selected_mode == MMC_HS_400 || mmc->selected_mode == MMC_HS_400_ES)
		esdhc_set_strobe_dll(mmc);

	if (priv->signal_voltage != mmc->signal_voltage) {
		ret = esdhc_set_voltage(mmc);
		if (ret) {
			if (ret != -ENOTSUPP)
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

	if (IS_ENABLED(CONFIG_FSL_USDHC)) {
		/* RSTA doesn't reset MMC_BOOT register, so manually reset it */
		esdhc_write32(&regs->mmcboot, 0x0);
		/* Reset MIX_CTRL and CLK_TUNE_CTRL_STATUS regs to 0 */
		esdhc_write32(&regs->mixctrl, 0x0);
		esdhc_write32(&regs->clktunectrlstatus, 0x0);

		/* Put VEND_SPEC to default value */
		if (priv->vs18_enable)
			esdhc_write32(&regs->vendorspec, VENDORSPEC_INIT |
				      ESDHC_VENDORSPEC_VSELECT);
		else
			esdhc_write32(&regs->vendorspec, VENDORSPEC_INIT);

		/* Disable DLL_CTRL delay line */
		esdhc_write32(&regs->dllctrl, 0x0);
	}

	if (IS_ENABLED(CONFIG_FSL_USDHC))
		esdhc_setbits32(&regs->vendorspec,
				VENDORSPEC_HCKEN | VENDORSPEC_IPGEN);
	else
		esdhc_setbits32(&regs->sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);

	/* Set the initial clock speed */
	set_sysctl(priv, mmc, 400000);

	/* Disable the BRR and BWR bits in IRQSTAT */
	esdhc_clrbits32(&regs->irqstaten, IRQSTATEN_BRR | IRQSTATEN_BWR);

	/* Put the PROCTL reg back to the default */
	if (IS_ENABLED(CONFIG_MCF5441x))
		esdhc_write32(&regs->proctl, PROCTL_INIT | PROCTL_D3CD);
	else
		esdhc_write32(&regs->proctl, PROCTL_INIT);

	/* Set timout to the maximum value */
	esdhc_clrsetbits32(&regs->sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

	return 0;
}

static int esdhc_getcd_common(struct fsl_esdhc_priv *priv)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int timeout = 1000;

	if (IS_ENABLED(CONFIG_ESDHC_DETECT_QUIRK))
		return 1;

	if (CONFIG_IS_ENABLED(DM_MMC)) {
		if (priv->broken_cd)
			return 1;
#if CONFIG_IS_ENABLED(DM_GPIO)
		if (dm_gpio_is_valid(&priv->cd_gpio))
			return dm_gpio_get_value(&priv->cd_gpio);
#endif
	}

	while (!(esdhc_read32(&regs->prsstat) & PRSSTAT_CINS) && --timeout)
		udelay(1000);

	return timeout > 0;
}

static int esdhc_wait_dat0_common(struct fsl_esdhc_priv *priv, int state,
				  int timeout_us)
{
	struct fsl_esdhc *regs = priv->esdhc_regs;
	int ret, err;
	u32 tmp;

	/* make sure the card clock keep on */
	esdhc_setbits32(&regs->vendorspec, VENDORSPEC_FRC_SDCLK_ON);

	ret = readx_poll_timeout(esdhc_read32, &regs->prsstat, tmp,
				!!(tmp & PRSSTAT_DAT0) == !!state,
				timeout_us);

	/* change to default setting, let host control the card clock */
	esdhc_clrbits32(&regs->vendorspec, VENDORSPEC_FRC_SDCLK_ON);

	err = readx_poll_timeout(esdhc_read32, &regs->prsstat, tmp, tmp & PRSSTAT_SDOFF, 100);
	if (err)
		pr_warn("card clock not gate off as expect.\n");

	return ret;
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

static int esdhc_wait_dat0(struct mmc *mmc, int state, int timeout_us)
{
	struct fsl_esdhc_priv *priv = mmc->priv;

	return esdhc_wait_dat0_common(priv, state, timeout_us);
}

static const struct mmc_ops esdhc_ops = {
	.getcd		= esdhc_getcd,
	.init		= esdhc_init,
	.send_cmd	= esdhc_send_cmd,
	.set_ios	= esdhc_set_ios,
	.wait_dat0	= esdhc_wait_dat0,
};
#endif

static int fsl_esdhc_init(struct fsl_esdhc_priv *priv,
			  struct fsl_esdhc_plat *plat)
{
	struct mmc_config *cfg;
	struct fsl_esdhc *regs;
	u32 caps;
	int ret;

	if (!priv)
		return -EINVAL;

	regs = priv->esdhc_regs;

	/* First reset the eSDHC controller */
	ret = esdhc_reset(regs);
	if (ret)
		return ret;

	/* ColdFire, using SDHC_DATA[3] for card detection */
	if (IS_ENABLED(CONFIG_MCF5441x))
		esdhc_write32(&regs->proctl, PROCTL_INIT | PROCTL_D3CD);

	if (IS_ENABLED(CONFIG_FSL_USDHC)) {
		esdhc_setbits32(&regs->vendorspec, VENDORSPEC_PEREN |
				VENDORSPEC_HCKEN | VENDORSPEC_IPGEN | VENDORSPEC_CKEN);
	} else {
		esdhc_setbits32(&regs->sysctl, SYSCTL_PEREN | SYSCTL_HCKEN
					| SYSCTL_IPGEN | SYSCTL_CKEN);
		/* Clearing tuning bits in case ROM has set it already */
		esdhc_write32(&regs->mixctrl, 0);
		esdhc_write32(&regs->autoc12err, 0);
		esdhc_write32(&regs->clktunectrlstatus, 0);
	}

	if (priv->vs18_enable)
		esdhc_setbits32(&regs->vendorspec, ESDHC_VENDORSPEC_VSELECT);

	esdhc_write32(&regs->irqstaten, SDHCI_IRQ_EN_BITS);
	cfg = &plat->cfg;
	if (!CONFIG_IS_ENABLED(DM_MMC))
		memset(cfg, '\0', sizeof(*cfg));

	caps = esdhc_read32(&regs->hostcapblt);

	/*
	 * MCF5441x RM declares in more points that sdhc clock speed must
	 * never exceed 25 Mhz. From this, the HS bit needs to be disabled
	 * from host capabilities.
	 */
	if (IS_ENABLED(CONFIG_MCF5441x))
		caps &= ~HOSTCAPBLT_HSS;

	if (IS_ENABLED(CONFIG_SYS_FSL_ERRATUM_ESDHC135))
		caps &= ~(HOSTCAPBLT_SRS | HOSTCAPBLT_VS18 | HOSTCAPBLT_VS30);

	if (IS_ENABLED(CONFIG_SYS_FSL_MMC_HAS_CAPBLT_VS33))
		caps |= HOSTCAPBLT_VS33;

	if (caps & HOSTCAPBLT_VS18)
		cfg->voltages |= MMC_VDD_165_195;
	if (caps & HOSTCAPBLT_VS30)
		cfg->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & HOSTCAPBLT_VS33)
		cfg->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;

	cfg->name = "FSL_SDHC";

#if !CONFIG_IS_ENABLED(DM_MMC)
	cfg->ops = &esdhc_ops;
#endif

	if (IS_ENABLED(CONFIG_SYS_FSL_ESDHC_HAS_DDR_MODE))
		cfg->host_caps |= MMC_MODE_DDR_52MHz;

	if (caps & HOSTCAPBLT_HSS)
		cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	cfg->host_caps |= priv->caps;

	cfg->f_min = 400000;
	cfg->f_max = min(priv->sdhc_clk, (u32)200000000);

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	esdhc_write32(&regs->dllctrl, 0);
	if (priv->flags & ESDHC_FLAG_USDHC) {
		if (priv->flags & ESDHC_FLAG_STD_TUNING) {
			u32 val = esdhc_read32(&regs->tuning_ctrl);

			val |= ESDHC_STD_TUNING_EN;
			val &= ~ESDHC_TUNING_START_TAP_MASK;
			val |= priv->tuning_start_tap;
			val &= ~ESDHC_TUNING_STEP_MASK;
			val |= (priv->tuning_step) << ESDHC_TUNING_STEP_SHIFT;

			/* Disable the CMD CRC check for tuning, if not, need to
			 * add some delay after every tuning command, because
			 * hardware standard tuning logic will directly go to next
			 * step once it detect the CMD CRC error, will not wait for
			 * the card side to finally send out the tuning data, trigger
			 * the buffer read ready interrupt immediately. If usdhc send
			 * the next tuning command some eMMC card will stuck, can't
			 * response, block the tuning procedure or the first command
			 * after the whole tuning procedure always can't get any response.
			 */
			val |= ESDHC_TUNING_CMD_CRC_CHECK_DISABLE;
			esdhc_write32(&regs->tuning_ctrl, val);
		}

		/*
		 * UHS doesn't have explicit ESDHC flags, so if it's
		 * not supported, disable it in config.
		 */
		if (CONFIG_IS_ENABLED(MMC_UHS_SUPPORT))
			cfg->host_caps |= UHS_CAPS;

		if (CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)) {
			if (priv->flags & ESDHC_FLAG_HS200)
				cfg->host_caps |= MMC_CAP(MMC_HS_200);
		}

		if (CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)) {
			if (priv->flags & ESDHC_FLAG_HS400)
				cfg->host_caps |= MMC_CAP(MMC_HS_400);
		}

		if (CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)) {
			if (priv->flags & ESDHC_FLAG_HS400_ES)
				cfg->host_caps |= MMC_CAP(MMC_HS_400_ES);
		}
	}
	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
int fsl_esdhc_initialize(struct bd_info *bis, struct fsl_esdhc_cfg *cfg)
{
	struct fsl_esdhc_plat *plat;
	struct fsl_esdhc_priv *priv;
	struct mmc_config *mmc_cfg;
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

	priv->esdhc_regs = (struct fsl_esdhc *)(unsigned long)(cfg->esdhc_base);
	priv->sdhc_clk = cfg->sdhc_clk;
	priv->wp_enable  = cfg->wp_enable;

	mmc_cfg = &plat->cfg;

	switch (cfg->max_bus_width) {
	case 0: /* Not set in config; assume everything is supported */
	case 8:
		mmc_cfg->host_caps |= MMC_MODE_8BIT;
		fallthrough;
	case 4:
		mmc_cfg->host_caps |= MMC_MODE_4BIT;
		fallthrough;
	case 1:
		mmc_cfg->host_caps |= MMC_MODE_1BIT;
		break;
	default:
		printf("invalid max bus width %u\n", cfg->max_bus_width);
		return -EINVAL;
	}

	if (IS_ENABLED(CONFIG_ESDHC_DETECT_8_BIT_QUIRK))
		mmc_cfg->host_caps &= ~MMC_MODE_8BIT;

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

int fsl_esdhc_mmc_init(struct bd_info *bis)
{
	struct fsl_esdhc_cfg *cfg;

	cfg = calloc(sizeof(struct fsl_esdhc_cfg), 1);
	cfg->esdhc_base = CFG_SYS_FSL_ESDHC_ADDR;
	cfg->sdhc_clk = gd->arch.sdhc_clk;
	return fsl_esdhc_initialize(bis, cfg);
}
#endif

#if CONFIG_IS_ENABLED(OF_LIBFDT)
__weak int esdhc_status_fixup(void *blob, const char *compat)
{
	if (IS_ENABLED(CONFIG_FSL_ESDHC_PIN_MUX) && !hwconfig("esdhc")) {
		do_fixup_by_compat(blob, compat, "status", "disabled",
				sizeof("disabled"), 1);
		return 1;
	}
	return 0;
}

void fdt_fixup_esdhc(void *blob, struct bd_info *bd)
{
	const char *compat = "fsl,esdhc";

	if (esdhc_status_fixup(blob, compat))
		return;

	do_fixup_by_compat_u32(blob, compat, "clock-frequency",
			       gd->arch.sdhc_clk, 1);
}
#endif

#if CONFIG_IS_ENABLED(DM_MMC)
#include <asm/arch/clock.h>
__weak void init_clk_usdhc(u32 index)
{
}

static int fsl_esdhc_of_to_plat(struct udevice *dev)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	struct udevice *vqmmc_dev;
	int ret;

	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	fdt_addr_t addr;
	unsigned int val;

	if (!CONFIG_IS_ENABLED(OF_REAL))
		return 0;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	priv->esdhc_regs = (struct fsl_esdhc *)addr;
	priv->dev = dev;
	priv->mode = -1;

	val = fdtdec_get_int(fdt, node, "fsl,tuning-step", 1);
	priv->tuning_step = val;
	val = fdtdec_get_int(fdt, node, "fsl,tuning-start-tap",
			     ESDHC_TUNING_START_TAP_DEFAULT);
	priv->tuning_start_tap = val;
	val = fdtdec_get_int(fdt, node, "fsl,strobe-dll-delay-target",
			     ESDHC_STROBE_DLL_CTRL_SLV_DLY_TARGET_DEFAULT);
	priv->strobe_dll_delay_target = val;
	val = fdtdec_get_int(fdt, node, "fsl,signal-voltage-switch-extra-delay-ms", 0);
	priv->signal_voltage_switch_extra_delay_ms = val;

	if (dev_read_bool(dev, "broken-cd"))
		priv->broken_cd = 1;

	if (dev_read_prop(dev, "fsl,wp-controller", NULL)) {
		priv->wp_enable = 1;
	} else {
		priv->wp_enable = 0;
	}

#if CONFIG_IS_ENABLED(DM_GPIO)
	gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio,
			     GPIOD_IS_IN);
	gpio_request_by_name(dev, "wp-gpios", 0, &priv->wp_gpio,
			     GPIOD_IS_IN);
#endif

	priv->vs18_enable = 0;

	if (!CONFIG_IS_ENABLED(DM_REGULATOR))
		return 0;

	/*
	 * If emmc I/O has a fixed voltage at 1.8V, this must be provided,
	 * otherwise, emmc will work abnormally.
	 */
	ret = device_get_supply_regulator(dev, "vqmmc-supply", &vqmmc_dev);
	if (ret) {
		dev_dbg(dev, "no vqmmc-supply\n");
	} else {
		priv->vqmmc_dev = vqmmc_dev;
		ret = regulator_set_enable(vqmmc_dev, true);
		if (ret) {
			dev_err(dev, "fail to enable vqmmc-supply\n");
			return ret;
		}

		if (regulator_get_value(vqmmc_dev) == 1800000)
			priv->vs18_enable = 1;
	}
	return 0;
}

static int fsl_esdhc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct fsl_esdhc_plat *plat = dev_get_plat(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	struct esdhc_soc_data *data =
		(struct esdhc_soc_data *)dev_get_driver_data(dev);
	struct mmc *mmc;
	int ret;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_esdhc *dtplat = &plat->dtplat;

	priv->esdhc_regs = map_sysmem(dtplat->reg[0], dtplat->reg[1]);

	if (dtplat->non_removable)
		plat->cfg.host_caps |= MMC_CAP_NONREMOVABLE;
	else
		plat->cfg.host_caps &= ~MMC_CAP_NONREMOVABLE;

	if (CONFIG_IS_ENABLED(DM_GPIO) && !dtplat->non_removable) {
		struct udevice *gpiodev;

		ret = device_get_by_ofplat_idx(dtplat->cd_gpios->idx, &gpiodev);
		if (ret)
			return ret;

		ret = gpio_dev_request_index(gpiodev, gpiodev->name, "cd-gpios",
					     dtplat->cd_gpios->arg[0], GPIOD_IS_IN,
					     dtplat->cd_gpios->arg[1], &priv->cd_gpio);

		if (ret)
			return ret;
	}
#endif

	if (data)
		priv->flags = data->flags;

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

#if CONFIG_IS_ENABLED(CLK)
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
#else
	init_clk_usdhc(dev_seq(dev));

	priv->sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK + dev_seq(dev));
	if (priv->sdhc_clk <= 0) {
		dev_err(dev, "Unable to get clk for %s\n", dev->name);
		return -EINVAL;
	}
#endif

	ret = fsl_esdhc_init(priv, plat);
	if (ret) {
		dev_err(dev, "fsl_esdhc_init failure\n");
		return ret;
	}

	if (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = mmc_of_parse(dev, &plat->cfg);
		if (ret)
			return ret;
	}

	mmc = &plat->mmc;
	mmc->cfg = &plat->cfg;
	mmc->dev = dev;

	upriv->mmc = mmc;

	return esdhc_init_common(priv, mmc);
}

static int fsl_esdhc_get_cd(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_plat(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	if (plat->cfg.host_caps & MMC_CAP_NONREMOVABLE)
		return 1;

	return esdhc_getcd_common(priv);
}

static int fsl_esdhc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct fsl_esdhc_plat *plat = dev_get_plat(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_send_cmd_common(priv, &plat->mmc, cmd, data);
}

static int fsl_esdhc_set_ios(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_plat(dev);
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_set_ios_common(priv, &plat->mmc);
}

static int __maybe_unused fsl_esdhc_set_enhanced_strobe(struct udevice *dev)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);
	struct fsl_esdhc *regs = priv->esdhc_regs;
	u32 m;

	m = esdhc_read32(&regs->mixctrl);
	m |= MIX_CTRL_HS400_ES;
	esdhc_write32(&regs->mixctrl, m);

	return 0;
}

static int fsl_esdhc_wait_dat0(struct udevice *dev, int state,
				int timeout_us)
{
	struct fsl_esdhc_priv *priv = dev_get_priv(dev);

	return esdhc_wait_dat0_common(priv, state, timeout_us);
}

static const struct dm_mmc_ops fsl_esdhc_ops = {
	.get_cd		= fsl_esdhc_get_cd,
	.send_cmd	= fsl_esdhc_send_cmd,
	.set_ios	= fsl_esdhc_set_ios,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning	= fsl_esdhc_execute_tuning,
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
	.set_enhanced_strobe = fsl_esdhc_set_enhanced_strobe,
#endif
	.wait_dat0 = fsl_esdhc_wait_dat0,
};

static struct esdhc_soc_data usdhc_imx7d_data = {
	.flags = ESDHC_FLAG_USDHC | ESDHC_FLAG_STD_TUNING
			| ESDHC_FLAG_HAVE_CAP1 | ESDHC_FLAG_HS200
			| ESDHC_FLAG_HS400,
};

static struct esdhc_soc_data usdhc_imx7ulp_data = {
	.flags = ESDHC_FLAG_USDHC | ESDHC_FLAG_STD_TUNING
			| ESDHC_FLAG_HAVE_CAP1 | ESDHC_FLAG_HS200
			| ESDHC_FLAG_HS400,
};

static struct esdhc_soc_data usdhc_imx8qm_data = {
	.flags = ESDHC_FLAG_USDHC | ESDHC_FLAG_STD_TUNING |
		ESDHC_FLAG_HAVE_CAP1 | ESDHC_FLAG_HS200 |
		ESDHC_FLAG_HS400 | ESDHC_FLAG_HS400_ES,
};

static const struct udevice_id fsl_esdhc_ids[] = {
	{ .compatible = "fsl,imx51-esdhc", },
	{ .compatible = "fsl,imx53-esdhc", },
	{ .compatible = "fsl,imx6ul-usdhc", },
	{ .compatible = "fsl,imx6sx-usdhc", },
	{ .compatible = "fsl,imx6sl-usdhc", },
	{ .compatible = "fsl,imx6q-usdhc", },
	{ .compatible = "fsl,imx7d-usdhc", .data = (ulong)&usdhc_imx7d_data,},
	{ .compatible = "fsl,imx7ulp-usdhc", .data = (ulong)&usdhc_imx7ulp_data,},
	{ .compatible = "fsl,imx8qm-usdhc", .data = (ulong)&usdhc_imx8qm_data,},
	{ .compatible = "fsl,imx8mm-usdhc", .data = (ulong)&usdhc_imx8qm_data,},
	{ .compatible = "fsl,imx8mn-usdhc", .data = (ulong)&usdhc_imx8qm_data,},
	{ .compatible = "fsl,imx8mp-usdhc", .data = (ulong)&usdhc_imx8qm_data,},
	{ .compatible = "fsl,imx8mq-usdhc", .data = (ulong)&usdhc_imx8qm_data,},
	{ .compatible = "fsl,imxrt-usdhc", },
	{ .compatible = "fsl,esdhc", },
	{ /* sentinel */ }
};

static int fsl_esdhc_bind(struct udevice *dev)
{
	struct fsl_esdhc_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

U_BOOT_DRIVER(fsl_esdhc) = {
	.name	= "fsl_esdhc",
	.id	= UCLASS_MMC,
	.of_match = fsl_esdhc_ids,
	.of_to_plat = fsl_esdhc_of_to_plat,
	.ops	= &fsl_esdhc_ops,
	.bind	= fsl_esdhc_bind,
	.probe	= fsl_esdhc_probe,
	.plat_auto	= sizeof(struct fsl_esdhc_plat),
	.priv_auto	= sizeof(struct fsl_esdhc_priv),
};

DM_DRIVER_ALIAS(fsl_esdhc, fsl_imx6q_usdhc)
#endif
