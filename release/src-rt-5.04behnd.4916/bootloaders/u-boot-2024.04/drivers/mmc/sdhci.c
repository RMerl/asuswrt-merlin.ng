// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011, Marvell Semiconductor Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <mmc.h>
#include <sdhci.h>
#include <asm/cache.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/printk.h>
#include <phys2bus.h>
#include <power/regulator.h>

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
	unsigned long timeout;
	u16 clk = 0;

	if(!(mask & SDHCI_RESET_ALL)) {
		/* Workaround for clk gated CMD/DAT resets in 5.1 controller. Internal 
		 * Clk enable must be set for reset logic to clear reset bits
		 */
		clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
		if(!(clk & SDHCI_CLOCK_INT_EN))
			sdhci_writew(host, clk|SDHCI_CLOCK_INT_EN, SDHCI_CLOCK_CONTROL);
	}

	/* Wait max 100 ms */
	timeout = 100;
	sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
	while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			printf("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			goto sdhci_reset_return;
		}
		timeout--;
		udelay(1000);
	}

sdhci_reset_return:
	if(!(mask & SDHCI_RESET_ALL)) {
		/* Restore clock settings if overridden and not doing a total core reset */
		if(!(clk & SDHCI_CLOCK_INT_EN))
			sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
	}
}

static void sdhci_cmd_done(struct sdhci_host *host, struct mmc_cmd *cmd)
{
	int i;
	if (cmd->resp_type & MMC_RSP_136) {
		/* CRC is stripped so we need to do some shifting. */
		for (i = 0; i < 4; i++) {
			cmd->response[i] = sdhci_readl(host,
					SDHCI_RESPONSE + (3-i)*4) << 8;
			if (i != 3)
				cmd->response[i] |= sdhci_readb(host,
						SDHCI_RESPONSE + (3-i)*4-1);
		}
	} else {
		cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
	}
}

static void sdhci_transfer_pio(struct sdhci_host *host, struct mmc_data *data)
{
	int i;
	char *offs;
	for (i = 0; i < data->blocksize; i += 4) {
		offs = data->dest + i;
		if (data->flags == MMC_DATA_READ)
			*(u32 *)offs = sdhci_readl(host, SDHCI_BUFFER);
		else
			sdhci_writel(host, *(u32 *)offs, SDHCI_BUFFER);
	}
}

#if (CONFIG_IS_ENABLED(MMC_SDHCI_SDMA) || CONFIG_IS_ENABLED(MMC_SDHCI_ADMA))
static void sdhci_prepare_dma(struct sdhci_host *host, struct mmc_data *data,
			      int *is_aligned, int trans_bytes)
{
	dma_addr_t dma_addr;
	unsigned char ctrl;
	void *buf;

	if (data->flags == MMC_DATA_READ)
		buf = data->dest;
	else
		buf = (void *)data->src;

	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	ctrl &= ~SDHCI_CTRL_DMA_MASK;
	if (host->flags & USE_ADMA64)
		ctrl |= SDHCI_CTRL_ADMA64;
	else if (host->flags & USE_ADMA)
		ctrl |= SDHCI_CTRL_ADMA32;
	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

	if (host->flags & USE_SDMA &&
	    (host->force_align_buffer ||
	     (host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR &&
	      ((unsigned long)buf & 0x7) != 0x0))) {
		*is_aligned = 0;
		if (data->flags != MMC_DATA_READ)
			memcpy(host->align_buffer, buf, trans_bytes);
		buf = host->align_buffer;
	}

	host->start_addr = dma_map_single(buf, trans_bytes,
					  mmc_get_dma_dir(data));

	if (host->flags & USE_SDMA) {
		dma_addr = dev_phys_to_bus(mmc_to_dev(host->mmc), host->start_addr);
		sdhci_writel(host, dma_addr, SDHCI_DMA_ADDRESS);
	}
#if CONFIG_IS_ENABLED(MMC_SDHCI_ADMA)
	else if (host->flags & (USE_ADMA | USE_ADMA64)) {
		sdhci_prepare_adma_table(host->adma_desc_table, data,
					 host->start_addr);

		sdhci_writel(host, lower_32_bits(host->adma_addr),
			     SDHCI_ADMA_ADDRESS);
		if (host->flags & USE_ADMA64)
			sdhci_writel(host, upper_32_bits(host->adma_addr),
				     SDHCI_ADMA_ADDRESS_HI);
	}
#endif
}
#else
static void sdhci_prepare_dma(struct sdhci_host *host, struct mmc_data *data,
			      int *is_aligned, int trans_bytes)
{}
#endif
static int sdhci_transfer_data(struct sdhci_host *host, struct mmc_data *data)
{
	dma_addr_t start_addr = host->start_addr;
	unsigned int stat, rdy, mask, timeout, block = 0;
	bool transfer_done = false;

	timeout = 1000000;
	rdy = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
	mask = SDHCI_DATA_AVAILABLE | SDHCI_SPACE_AVAILABLE;
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR) {
			pr_debug("%s: Error detected in status(0x%X)!\n",
				 __func__, stat);
			return -EIO;
		}
		if (!transfer_done && (stat & rdy)) {
			if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & mask))
				continue;
			sdhci_writel(host, rdy, SDHCI_INT_STATUS);
			sdhci_transfer_pio(host, data);
			data->dest += data->blocksize;
			if (++block >= data->blocks) {
				/* Keep looping until the SDHCI_INT_DATA_END is
				 * cleared, even if we finished sending all the
				 * blocks.
				 */
				transfer_done = true;
				continue;
			}
		}
		if ((host->flags & USE_DMA) && !transfer_done &&
		    (stat & SDHCI_INT_DMA_END)) {
			sdhci_writel(host, SDHCI_INT_DMA_END, SDHCI_INT_STATUS);
			if (host->flags & USE_SDMA) {
				start_addr &=
				~(SDHCI_DEFAULT_BOUNDARY_SIZE - 1);
				start_addr += SDHCI_DEFAULT_BOUNDARY_SIZE;
				start_addr = dev_phys_to_bus(mmc_to_dev(host->mmc),
							     start_addr);
				sdhci_writel(host, start_addr, SDHCI_DMA_ADDRESS);
			}
		}
		if (timeout-- > 0)
			udelay(10);
		else {
			printf("%s: Transfer data timeout\n", __func__);
			return -ETIMEDOUT;
		}
	} while (!(stat & SDHCI_INT_DATA_END));

#if (CONFIG_IS_ENABLED(MMC_SDHCI_SDMA) || CONFIG_IS_ENABLED(MMC_SDHCI_ADMA))
	dma_unmap_single(host->start_addr, data->blocks * data->blocksize,
			 mmc_get_dma_dir(data));
#endif

	return 0;
}

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value.
 */
#define SDHCI_CMD_MAX_TIMEOUT			3200
#define SDHCI_CMD_DEFAULT_TIMEOUT		100
#define SDHCI_READ_STATUS_TIMEOUT		1000

#ifdef CONFIG_DM_MMC
static int sdhci_send_command(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

#else
static int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
#endif
	struct sdhci_host *host = mmc->priv;
	unsigned int stat = 0;
	int ret = 0;
	int trans_bytes = 0, is_aligned = 1;
	u32 mask, flags, mode = 0;
	unsigned int time = 0;
	int mmc_dev = mmc_get_blk_desc(mmc)->devnum;
	ulong start = get_timer(0);

	host->start_addr = 0;
	/* Timeout unit - ms */
	static unsigned int cmd_timeout = SDHCI_CMD_DEFAULT_TIMEOUT;

	mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

	/* We shouldn't wait for data inihibit for stop commands, even
	   though they might use busy signaling */
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION ||
	    ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK ||
	      cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) && !data))
		mask &= ~SDHCI_DATA_INHIBIT;

	while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask) {
		if (time >= cmd_timeout) {
			printf("%s: MMC: %d busy ", __func__, mmc_dev);
			if (2 * cmd_timeout <= SDHCI_CMD_MAX_TIMEOUT) {
				cmd_timeout += cmd_timeout;
				printf("timeout increasing to: %u ms.\n",
				       cmd_timeout);
			} else {
				puts("timeout.\n");
				return -ECOMM;
			}
		}
		time++;
		udelay(1000);
	}

	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);

	mask = SDHCI_INT_RESPONSE;
	if ((cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK ||
	     cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200) && !data)
		mask = SDHCI_INT_DATA_AVAIL;

	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = SDHCI_CMD_RESP_NONE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = SDHCI_CMD_RESP_LONG;
	else if (cmd->resp_type & MMC_RSP_BUSY) {
		flags = SDHCI_CMD_RESP_SHORT_BUSY;
		mask |= SDHCI_INT_DATA_END;
	} else
		flags = SDHCI_CMD_RESP_SHORT;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= SDHCI_CMD_CRC;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= SDHCI_CMD_INDEX;
	if (data || cmd->cmdidx ==  MMC_CMD_SEND_TUNING_BLOCK ||
	    cmd->cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200)
		flags |= SDHCI_CMD_DATA;

	/* Set Transfer mode regarding to data flag */
	if (data) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);

		if (!(host->quirks & SDHCI_QUIRK_SUPPORT_SINGLE))
			mode = SDHCI_TRNS_BLK_CNT_EN;
		trans_bytes = data->blocks * data->blocksize;
		if (data->blocks > 1)
			mode |= SDHCI_TRNS_MULTI | SDHCI_TRNS_BLK_CNT_EN;

		if (data->flags == MMC_DATA_READ)
			mode |= SDHCI_TRNS_READ;

		if (host->flags & USE_DMA) {
			mode |= SDHCI_TRNS_DMA;
			sdhci_prepare_dma(host, data, &is_aligned, trans_bytes);
		}

		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
				data->blocksize),
				SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
	} else if (cmd->resp_type & MMC_RSP_BUSY) {
		sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
	}

	sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
	sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
	start = get_timer(0);
	do {
		stat = sdhci_readl(host, SDHCI_INT_STATUS);
		if (stat & SDHCI_INT_ERROR)
			break;

		if (host->quirks & SDHCI_QUIRK_BROKEN_R1B &&
		    cmd->resp_type & MMC_RSP_BUSY && !data) {
			unsigned int state =
				sdhci_readl(host, SDHCI_PRESENT_STATE);

			if (!(state & SDHCI_DAT_ACTIVE))
				return 0;
		}

		if (get_timer(start) >= SDHCI_READ_STATUS_TIMEOUT) {
			printf("%s: Timeout for status update: %08x %08x\n",
			       __func__, stat, mask);
			return -ETIMEDOUT;
		}
	} while ((stat & mask) != mask);

	if ((stat & (SDHCI_INT_ERROR | mask)) == mask) {
		sdhci_cmd_done(host, cmd);
		sdhci_writel(host, mask, SDHCI_INT_STATUS);
	} else
		ret = -1;

	if (!ret && data)
		ret = sdhci_transfer_data(host, data);

	if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
		udelay(1000);

	stat = sdhci_readl(host, SDHCI_INT_STATUS);
	sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
	if (!ret) {
		if ((host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) &&
				!is_aligned && (data->flags == MMC_DATA_READ))
			memcpy(data->dest, host->align_buffer, trans_bytes);
		return 0;
	}

	sdhci_reset(host, SDHCI_RESET_CMD);
	sdhci_reset(host, SDHCI_RESET_DATA);
	if (stat & SDHCI_INT_TIMEOUT)
		return -ETIMEDOUT;
	else
		return -ECOMM;
}

#if defined(CONFIG_DM_MMC) && defined(MMC_SUPPORTS_TUNING)
static int sdhci_execute_tuning(struct udevice *dev, uint opcode)
{
	int err;
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct sdhci_host *host = mmc->priv;

	debug("%s\n", __func__);

	if (host->ops && host->ops->platform_execute_tuning) {
		err = host->ops->platform_execute_tuning(mmc, opcode);
		if (err)
			return err;
		return 0;
	}
	return 0;
}
#endif
int sdhci_set_clock(struct mmc *mmc, unsigned int clock)
{
	struct sdhci_host *host = mmc->priv;
	unsigned int div, clk = 0, timeout;
	int ret;

	/* Wait max 20 ms */
	timeout = 200;
	while (sdhci_readl(host, SDHCI_PRESENT_STATE) &
			   (SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT)) {
		if (timeout == 0) {
			printf("%s: Timeout to wait cmd & data inhibit\n",
			       __func__);
			return -EBUSY;
		}

		timeout--;
		udelay(100);
	}

	sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

	if (clock == 0)
		return 0;

	if (host->ops && host->ops->set_delay) {
		ret = host->ops->set_delay(host);
		if (ret) {
			printf("%s: Error while setting tap delay\n", __func__);
			return ret;
		}
	}

	if (host->ops && host->ops->config_dll) {
		ret = host->ops->config_dll(host, clock, false);
		if (ret) {
			printf("%s: Error while configuring dll\n", __func__);
			return ret;
		}
	}

	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		/*
		 * Check if the Host Controller supports Programmable Clock
		 * Mode.
		 */
		if (host->clk_mul) {
			for (div = 1; div <= 1024; div++) {
				if ((host->max_clk / div) <= clock)
					break;
			}

			/*
			 * Set Programmable Clock Mode in the Clock
			 * Control register.
			 */
			clk = SDHCI_PROG_CLOCK_MODE;
			div--;
		} else {
			/* Version 3.00 divisors must be a multiple of 2. */
			if (host->max_clk <= clock) {
				div = 1;
			} else {
				for (div = 2;
				     div < SDHCI_MAX_DIV_SPEC_300;
				     div += 2) {
					if ((host->max_clk / div) <= clock)
						break;
				}
			}
			div >>= 1;
		}
	} else {
		/* Version 2.00 divisors must be a power of 2. */
		for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2) {
			if ((host->max_clk / div) <= clock)
				break;
		}
		div >>= 1;
	}

	if (host->ops && host->ops->set_clock)
		host->ops->set_clock(host, div);

	if (host->ops && host->ops->config_dll) {
		ret = host->ops->config_dll(host, clock, true);
		if (ret) {
			printf("%s: Error while configuring dll\n", __func__);
			return ret;
		}
	}

	clk |= (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
	clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
		<< SDHCI_DIVIDER_HI_SHIFT;
	clk |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Wait max 20 ms */
	timeout = 20;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
		& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			printf("%s: Internal clock never stabilised.\n",
			       __func__);
			return -EBUSY;
		}
		timeout--;
		udelay(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
	return 0;
}

static void sdhci_set_power(struct sdhci_host *host, unsigned short power)
{
	u8 pwr = 0;

	if (power != (unsigned short)-1) {
		switch (1 << power) {
		case MMC_VDD_165_195:
			pwr = SDHCI_POWER_180;
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = SDHCI_POWER_300;
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = SDHCI_POWER_330;
			break;
		}
	}

	if (pwr == 0) {
		sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
		return;
	}

	pwr |= SDHCI_POWER_ON;

	sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
}

void sdhci_set_uhs_timing(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 reg;

	reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	reg &= ~SDHCI_CTRL_UHS_MASK;

	switch (mmc->selected_mode) {
	case UHS_SDR25:
	case MMC_HS:
		reg |= SDHCI_CTRL_UHS_SDR25;
		break;
	case UHS_SDR50:
	case MMC_HS_52:
		reg |= SDHCI_CTRL_UHS_SDR50;
		break;
	case UHS_DDR50:
	case MMC_DDR_52:
		reg |= SDHCI_CTRL_UHS_DDR50;
		break;
	case UHS_SDR104:
	case MMC_HS_200:
		reg |= SDHCI_CTRL_UHS_SDR104;
		break;
	case MMC_HS_400:
	case MMC_HS_400_ES:
		reg |= SDHCI_CTRL_HS400;
		break;
	default:
		reg |= SDHCI_CTRL_UHS_SDR12;
	}

	sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
}

static void sdhci_set_voltage(struct sdhci_host *host)
{
	if (IS_ENABLED(CONFIG_MMC_IO_VOLTAGE)) {
		struct mmc *mmc = (struct mmc *)host->mmc;
		u32 ctrl;

		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

		switch (mmc->signal_voltage) {
		case MMC_SIGNAL_VOLTAGE_330:
#if CONFIG_IS_ENABLED(DM_REGULATOR)
			if (mmc->vqmmc_supply) {
				if (regulator_set_enable_if_allowed(mmc->vqmmc_supply, false)) {
					pr_err("failed to disable vqmmc-supply\n");
					return;
				}

				if (regulator_set_value(mmc->vqmmc_supply, 3300000)) {
					pr_err("failed to set vqmmc-voltage to 3.3V\n");
					return;
				}

				if (regulator_set_enable_if_allowed(mmc->vqmmc_supply, true)) {
					pr_err("failed to enable vqmmc-supply\n");
					return;
				}
			}
#endif
			if (IS_SD(mmc)) {
				ctrl &= ~SDHCI_CTRL_VDD_180;
				sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
			}

			/* Wait for 5ms */
			mdelay(5);

			/* 3.3V regulator output should be stable within 5 ms */
			if (IS_SD(mmc)) {
				if (ctrl & SDHCI_CTRL_VDD_180) {
					pr_err("3.3V regulator output did not become stable\n");
					return;
				}
			}

			break;
		case MMC_SIGNAL_VOLTAGE_180:
#if CONFIG_IS_ENABLED(DM_REGULATOR)
			if (mmc->vqmmc_supply) {
				if (regulator_set_enable_if_allowed(mmc->vqmmc_supply, false)) {
					pr_err("failed to disable vqmmc-supply\n");
					return;
				}

				if (regulator_set_value(mmc->vqmmc_supply, 1800000)) {
					pr_err("failed to set vqmmc-voltage to 1.8V\n");
					return;
				}

				if (regulator_set_enable_if_allowed(mmc->vqmmc_supply, true)) {
					pr_err("failed to enable vqmmc-supply\n");
					return;
				}
			}
#endif
			if (IS_SD(mmc)) {
				ctrl |= SDHCI_CTRL_VDD_180;
				sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
			}

			/* Wait for 5 ms */
			mdelay(5);

			/* 1.8V regulator output has to be stable within 5 ms */
			if (IS_SD(mmc)) {
				if (!(ctrl & SDHCI_CTRL_VDD_180)) {
					pr_err("1.8V regulator output did not become stable\n");
					return;
				}
			}

			break;
		default:
			/* No signal voltage switch required */
			return;
		}
	}
}

void sdhci_set_control_reg(struct sdhci_host *host)
{
	sdhci_set_voltage(host);
	sdhci_set_uhs_timing(host);
}

#ifdef CONFIG_DM_MMC
static int sdhci_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
#else
static int sdhci_set_ios(struct mmc *mmc)
{
#endif
	u32 ctrl;
	struct sdhci_host *host = mmc->priv;
	bool no_hispd_bit = false;

	if (host->ops && host->ops->set_control_reg)
		host->ops->set_control_reg(host);

	if (mmc->clock != host->clock)
		sdhci_set_clock(mmc, mmc->clock);

	if (mmc->clk_disable)
		sdhci_set_clock(mmc, 0);

	/* Set bus width */
	ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
	if (mmc->bus_width == 8) {
		ctrl &= ~SDHCI_CTRL_4BITBUS;
		if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
				(host->quirks & SDHCI_QUIRK_USE_WIDE8))
			ctrl |= SDHCI_CTRL_8BITBUS;
	} else {
		if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
				(host->quirks & SDHCI_QUIRK_USE_WIDE8))
			ctrl &= ~SDHCI_CTRL_8BITBUS;
		if (mmc->bus_width == 4)
			ctrl |= SDHCI_CTRL_4BITBUS;
		else
			ctrl &= ~SDHCI_CTRL_4BITBUS;
	}

	if ((host->quirks & SDHCI_QUIRK_NO_HISPD_BIT) ||
	    (host->quirks & SDHCI_QUIRK_BROKEN_HISPD_MODE)) {
		ctrl &= ~SDHCI_CTRL_HISPD;
		no_hispd_bit = true;
	}

	if (!no_hispd_bit) {
		if (mmc->selected_mode == MMC_HS ||
		    mmc->selected_mode == SD_HS ||
		    mmc->selected_mode == MMC_HS_52 ||
		    mmc->selected_mode == MMC_DDR_52 ||
		    mmc->selected_mode == MMC_HS_200 ||
		    mmc->selected_mode == MMC_HS_400 ||
		    mmc->selected_mode == MMC_HS_400_ES ||
		    mmc->selected_mode == UHS_SDR25 ||
		    mmc->selected_mode == UHS_SDR50 ||
		    mmc->selected_mode == UHS_SDR104 ||
		    mmc->selected_mode == UHS_DDR50)
			ctrl |= SDHCI_CTRL_HISPD;
		else
			ctrl &= ~SDHCI_CTRL_HISPD;
	}

	sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

	/* If available, call the driver specific "post" set_ios() function */
	if (host->ops && host->ops->set_ios_post)
		return host->ops->set_ios_post(host);

	return 0;
}

static int sdhci_init(struct mmc *mmc)
{
	struct sdhci_host *host = mmc->priv;
#if CONFIG_IS_ENABLED(DM_MMC) && CONFIG_IS_ENABLED(DM_GPIO)
	struct udevice *dev = mmc->dev;

	gpio_request_by_name(dev, "cd-gpios", 0,
			     &host->cd_gpio, GPIOD_IS_IN);
#endif

	sdhci_reset(host, SDHCI_RESET_ALL);

#if defined(CONFIG_FIXED_SDHCI_ALIGNED_BUFFER)
	host->align_buffer = (void *)CONFIG_FIXED_SDHCI_ALIGNED_BUFFER;
	/*
	 * Always use this bounce-buffer when CONFIG_FIXED_SDHCI_ALIGNED_BUFFER
	 * is defined.
	 */
	host->force_align_buffer = true;
#else
	if (host->quirks & SDHCI_QUIRK_32BIT_DMA_ADDR) {
		host->align_buffer = memalign(8, 512 * 1024);
		if (!host->align_buffer) {
			printf("%s: Aligned buffer alloc failed!!!\n",
			       __func__);
			return -ENOMEM;
		}
	}
#endif

	sdhci_set_power(host, fls(mmc->cfg->voltages) - 1);

	if (host->ops && host->ops->get_cd)
		host->ops->get_cd(host);

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

#ifdef CONFIG_DM_MMC
int sdhci_probe(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return sdhci_init(mmc);
}

static int sdhci_deferred_probe(struct udevice *dev)
{
	int err;
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct sdhci_host *host = mmc->priv;

	if (host->ops && host->ops->deferred_probe) {
		err = host->ops->deferred_probe(host);
		if (err)
			return err;
	}
	return 0;
}

static int sdhci_get_cd(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct sdhci_host *host = mmc->priv;
	int value;

	/* If nonremovable, assume that the card is always present. */
	if (mmc->cfg->host_caps & MMC_CAP_NONREMOVABLE)
		return 1;
	/* If polling, assume that the card is always present. */
	if (mmc->cfg->host_caps & MMC_CAP_NEEDS_POLL)
		return 1;

#if CONFIG_IS_ENABLED(DM_GPIO)
	value = dm_gpio_get_value(&host->cd_gpio);
	if (value >= 0) {
		if (mmc->cfg->host_caps & MMC_CAP_CD_ACTIVE_HIGH)
			return !value;
		else
			return value;
	}
#endif
	value = !!(sdhci_readl(host, SDHCI_PRESENT_STATE) &
		   SDHCI_CARD_PRESENT);
	if (mmc->cfg->host_caps & MMC_CAP_CD_ACTIVE_HIGH)
		return !value;
	else
		return value;
}

static int sdhci_wait_dat0(struct udevice *dev, int state,
			   int timeout_us)
{
	int tmp;
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct sdhci_host *host = mmc->priv;
	unsigned long timeout = timer_get_us() + timeout_us;

	// readx_poll_timeout is unsuitable because sdhci_readl accepts
	// two arguments
	do {
		tmp = sdhci_readl(host, SDHCI_PRESENT_STATE);
		if (!!(tmp & SDHCI_DATA_0_LVL_MASK) == !!state)
			return 0;
	} while (!timeout_us || !time_after(timer_get_us(), timeout));

	return -ETIMEDOUT;
}

#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
static int sdhci_set_enhanced_strobe(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct sdhci_host *host = mmc->priv;

	if (host->ops && host->ops->set_enhanced_strobe)
		return host->ops->set_enhanced_strobe(host);

	return -ENOTSUPP;
}
#endif

const struct dm_mmc_ops sdhci_ops = {
	.send_cmd	= sdhci_send_command,
	.set_ios	= sdhci_set_ios,
	.get_cd		= sdhci_get_cd,
	.deferred_probe	= sdhci_deferred_probe,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning	= sdhci_execute_tuning,
#endif
	.wait_dat0	= sdhci_wait_dat0,
#if CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT)
	.set_enhanced_strobe = sdhci_set_enhanced_strobe,
#endif
};
#else
static const struct mmc_ops sdhci_ops = {
	.send_cmd	= sdhci_send_command,
	.set_ios	= sdhci_set_ios,
	.init		= sdhci_init,
};
#endif

int sdhci_setup_cfg(struct mmc_config *cfg, struct sdhci_host *host,
		u32 f_max, u32 f_min)
{
	u32 caps, caps_1 = 0;
#if CONFIG_IS_ENABLED(DM_MMC)
	u64 dt_caps, dt_caps_mask;

	dt_caps_mask = dev_read_u64_default(host->mmc->dev,
					    "sdhci-caps-mask", 0);
	dt_caps = dev_read_u64_default(host->mmc->dev,
				       "sdhci-caps", 0);
	caps = ~lower_32_bits(dt_caps_mask) &
	       sdhci_readl(host, SDHCI_CAPABILITIES);
	caps |= lower_32_bits(dt_caps);
#else
	caps = sdhci_readl(host, SDHCI_CAPABILITIES);
#endif
	debug("%s, caps: 0x%x\n", __func__, caps);

#if CONFIG_IS_ENABLED(MMC_SDHCI_SDMA)
	if ((caps & SDHCI_CAN_DO_SDMA)) {
		host->flags |= USE_SDMA;
	} else {
		debug("%s: Your controller doesn't support SDMA!!\n",
		      __func__);
	}
#endif
#if CONFIG_IS_ENABLED(MMC_SDHCI_ADMA)
	if (!(caps & SDHCI_CAN_DO_ADMA2)) {
		printf("%s: Your controller doesn't support ADMA!!\n",
		       __func__);
		return -EINVAL;
	}
	host->adma_desc_table = sdhci_adma_init();
	host->adma_addr = (dma_addr_t)host->adma_desc_table;

#ifdef CONFIG_DMA_ADDR_T_64BIT
	host->flags |= USE_ADMA64;
#else
	host->flags |= USE_ADMA;
#endif
#endif
	if (host->quirks & SDHCI_QUIRK_REG32_RW)
		host->version =
			sdhci_readl(host, SDHCI_HOST_VERSION - 2) >> 16;
	else
		host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

	cfg->name = host->name;
#ifndef CONFIG_DM_MMC
	cfg->ops = &sdhci_ops;
#endif

	/* Check whether the clock multiplier is supported or not */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
#if CONFIG_IS_ENABLED(DM_MMC)
		caps_1 = ~upper_32_bits(dt_caps_mask) &
			 sdhci_readl(host, SDHCI_CAPABILITIES_1);
		caps_1 |= upper_32_bits(dt_caps);
#else
		caps_1 = sdhci_readl(host, SDHCI_CAPABILITIES_1);
#endif
		debug("%s, caps_1: 0x%x\n", __func__, caps_1);
		host->clk_mul = (caps_1 & SDHCI_CLOCK_MUL_MASK) >>
				SDHCI_CLOCK_MUL_SHIFT;
	}

	if (host->max_clk == 0) {
		if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
			host->max_clk = (caps & SDHCI_CLOCK_V3_BASE_MASK) >>
				SDHCI_CLOCK_BASE_SHIFT;
		else
			host->max_clk = (caps & SDHCI_CLOCK_BASE_MASK) >>
				SDHCI_CLOCK_BASE_SHIFT;
		host->max_clk *= 1000000;
		if (host->clk_mul)
			host->max_clk *= host->clk_mul;
	}
	if (host->max_clk == 0) {
		printf("%s: Hardware doesn't specify base clock frequency\n",
		       __func__);
		return -EINVAL;
	}
	if (f_max && (f_max < host->max_clk))
		cfg->f_max = f_max;
	else
		cfg->f_max = host->max_clk;
	if (f_min)
		cfg->f_min = f_min;
	else {
		if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
			cfg->f_min = cfg->f_max / SDHCI_MAX_DIV_SPEC_300;
		else
			cfg->f_min = cfg->f_max / SDHCI_MAX_DIV_SPEC_200;
	}
	cfg->voltages = 0;
	if (caps & SDHCI_CAN_VDD_330)
		cfg->voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;
	if (caps & SDHCI_CAN_VDD_300)
		cfg->voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
	if (caps & SDHCI_CAN_VDD_180)
		cfg->voltages |= MMC_VDD_165_195;

	if (host->quirks & SDHCI_QUIRK_BROKEN_VOLTAGE)
		cfg->voltages |= host->voltages;

	if (caps & SDHCI_CAN_DO_HISPD)
		cfg->host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz;

	cfg->host_caps |= MMC_MODE_4BIT;

	/* Since Host Controller Version3.0 */
	if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) {
		if (!(caps & SDHCI_CAN_DO_8BIT))
			cfg->host_caps &= ~MMC_MODE_8BIT;
	}

	if (host->quirks & SDHCI_QUIRK_BROKEN_HISPD_MODE) {
		cfg->host_caps &= ~MMC_MODE_HS;
		cfg->host_caps &= ~MMC_MODE_HS_52MHz;
	}

	if (!(cfg->voltages & MMC_VDD_165_195) ||
	    (host->quirks & SDHCI_QUIRK_NO_1_8_V))
		caps_1 &= ~(SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 |
			    SDHCI_SUPPORT_DDR50);

	if (caps_1 & (SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 |
		      SDHCI_SUPPORT_DDR50))
		cfg->host_caps |= MMC_CAP(UHS_SDR12) | MMC_CAP(UHS_SDR25);

	if (caps_1 & SDHCI_SUPPORT_SDR104) {
		cfg->host_caps |= MMC_CAP(UHS_SDR104) | MMC_CAP(UHS_SDR50);
		/*
		 * SD3.0: SDR104 is supported so (for eMMC) the caps2
		 * field can be promoted to support HS200.
		 */
		cfg->host_caps |= MMC_CAP(MMC_HS_200);
	} else if (caps_1 & SDHCI_SUPPORT_SDR50) {
		cfg->host_caps |= MMC_CAP(UHS_SDR50);
	}

	if ((host->quirks & SDHCI_QUIRK_CAPS_BIT63_FOR_HS400) &&
	    (caps_1 & SDHCI_SUPPORT_HS400))
		cfg->host_caps |= MMC_CAP(MMC_HS_400);

	if (caps_1 & SDHCI_SUPPORT_DDR50)
		cfg->host_caps |= MMC_CAP(UHS_DDR50);

	if (host->host_caps)
		cfg->host_caps |= host->host_caps;

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	return 0;
}

#ifdef CONFIG_BLK
int sdhci_bind(struct udevice *dev, struct mmc *mmc, struct mmc_config *cfg)
{
	return mmc_bind(dev, mmc, cfg);
}
#else
int add_sdhci(struct sdhci_host *host, u32 f_max, u32 f_min)
{
	int ret;

	ret = sdhci_setup_cfg(&host->cfg, host, f_max, f_min);
	if (ret)
		return ret;

	host->mmc = mmc_create(&host->cfg, host);
	if (host->mmc == NULL) {
		printf("%s: mmc create fail!\n", __func__);
		return -ENOMEM;
	}

	return 0;
}
#endif
