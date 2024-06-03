// SPDX-License-Identifier: GPL-2.0+
/*
 * ARM PrimeCell MultiMedia Card Interface - PL180
 *
 * Copyright (C) ST-Ericsson SA 2010
 *
 * Author: Ulf Hansson <ulf.hansson@stericsson.com>
 * Author: Martin Lundholm <martin.xa.lundholm@stericsson.com>
 * Ported to drivers/mmc/ by: Matt Waddel <matt.waddel@linaro.org>
 */

/* #define DEBUG */

#include "common.h"
#include <clk.h>
#include <errno.h>
#include <malloc.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm-generic/gpio.h>

#include "arm_pl180_mmci.h"

#ifdef CONFIG_DM_MMC
#include <dm.h>
#define MMC_CLOCK_MAX	48000000
#define MMC_CLOCK_MIN	400000

struct arm_pl180_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};
#endif

static int wait_for_command_end(struct mmc *dev, struct mmc_cmd *cmd)
{
	u32 hoststatus, statusmask;
	struct pl180_mmc_host *host = dev->priv;

	statusmask = SDI_STA_CTIMEOUT | SDI_STA_CCRCFAIL;
	if ((cmd->resp_type & MMC_RSP_PRESENT))
		statusmask |= SDI_STA_CMDREND;
	else
		statusmask |= SDI_STA_CMDSENT;

	do
		hoststatus = readl(&host->base->status) & statusmask;
	while (!hoststatus);

	writel(statusmask, &host->base->status_clear);
	if (hoststatus & SDI_STA_CTIMEOUT) {
		debug("CMD%d time out\n", cmd->cmdidx);
		return -ETIMEDOUT;
	} else if ((hoststatus & SDI_STA_CCRCFAIL) &&
		   (cmd->resp_type & MMC_RSP_CRC)) {
		printf("CMD%d CRC error\n", cmd->cmdidx);
		return -EILSEQ;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		cmd->response[0] = readl(&host->base->response0);
		cmd->response[1] = readl(&host->base->response1);
		cmd->response[2] = readl(&host->base->response2);
		cmd->response[3] = readl(&host->base->response3);
		debug("CMD%d response[0]:0x%08X, response[1]:0x%08X, "
			"response[2]:0x%08X, response[3]:0x%08X\n",
			cmd->cmdidx, cmd->response[0], cmd->response[1],
			cmd->response[2], cmd->response[3]);
	}

	return 0;
}

/* send command to the mmc card and wait for results */
static int do_command(struct mmc *dev, struct mmc_cmd *cmd)
{
	int result;
	u32 sdi_cmd = 0;
	struct pl180_mmc_host *host = dev->priv;

	sdi_cmd = ((cmd->cmdidx & SDI_CMD_CMDINDEX_MASK) | SDI_CMD_CPSMEN);

	if (cmd->resp_type) {
		sdi_cmd |= SDI_CMD_WAITRESP;
		if (cmd->resp_type & MMC_RSP_136)
			sdi_cmd |= SDI_CMD_LONGRESP;
	}

	writel((u32)cmd->cmdarg, &host->base->argument);
	udelay(COMMAND_REG_DELAY);
	writel(sdi_cmd, &host->base->command);
	result = wait_for_command_end(dev, cmd);

	/* After CMD2 set RCA to a none zero value. */
	if ((result == 0) && (cmd->cmdidx == MMC_CMD_ALL_SEND_CID))
		dev->rca = 10;

	/* After CMD3 open drain is switched off and push pull is used. */
	if ((result == 0) && (cmd->cmdidx == MMC_CMD_SET_RELATIVE_ADDR)) {
		u32 sdi_pwr = readl(&host->base->power) & ~SDI_PWR_OPD;
		writel(sdi_pwr, &host->base->power);
	}

	return result;
}

static int read_bytes(struct mmc *dev, u32 *dest, u32 blkcount, u32 blksize)
{
	u32 *tempbuff = dest;
	u64 xfercount = blkcount * blksize;
	struct pl180_mmc_host *host = dev->priv;
	u32 status, status_err;

	debug("read_bytes: blkcount=%u blksize=%u\n", blkcount, blksize);

	status = readl(&host->base->status);
	status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT |
			       SDI_STA_RXOVERR);
	while ((!status_err) && (xfercount >= sizeof(u32))) {
		if (status & SDI_STA_RXDAVL) {
			*(tempbuff) = readl(&host->base->fifo);
			tempbuff++;
			xfercount -= sizeof(u32);
		}
		status = readl(&host->base->status);
		status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT |
				       SDI_STA_RXOVERR);
	}

	status_err = status &
		(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND |
		 SDI_STA_RXOVERR);
	while (!status_err) {
		status = readl(&host->base->status);
		status_err = status &
			(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND |
			 SDI_STA_RXOVERR);
	}

	if (status & SDI_STA_DTIMEOUT) {
		printf("Read data timed out, xfercount: %llu, status: 0x%08X\n",
			xfercount, status);
		return -ETIMEDOUT;
	} else if (status & SDI_STA_DCRCFAIL) {
		printf("Read data bytes CRC error: 0x%x\n", status);
		return -EILSEQ;
	} else if (status & SDI_STA_RXOVERR) {
		printf("Read data RX overflow error\n");
		return -EIO;
	}

	writel(SDI_ICR_MASK, &host->base->status_clear);

	if (xfercount) {
		printf("Read data error, xfercount: %llu\n", xfercount);
		return -ENOBUFS;
	}

	return 0;
}

static int write_bytes(struct mmc *dev, u32 *src, u32 blkcount, u32 blksize)
{
	u32 *tempbuff = src;
	int i;
	u64 xfercount = blkcount * blksize;
	struct pl180_mmc_host *host = dev->priv;
	u32 status, status_err;

	debug("write_bytes: blkcount=%u blksize=%u\n", blkcount, blksize);

	status = readl(&host->base->status);
	status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT);
	while (!status_err && xfercount) {
		if (status & SDI_STA_TXFIFOBW) {
			if (xfercount >= SDI_FIFO_BURST_SIZE * sizeof(u32)) {
				for (i = 0; i < SDI_FIFO_BURST_SIZE; i++)
					writel(*(tempbuff + i),
						&host->base->fifo);
				tempbuff += SDI_FIFO_BURST_SIZE;
				xfercount -= SDI_FIFO_BURST_SIZE * sizeof(u32);
			} else {
				while (xfercount >= sizeof(u32)) {
					writel(*(tempbuff), &host->base->fifo);
					tempbuff++;
					xfercount -= sizeof(u32);
				}
			}
		}
		status = readl(&host->base->status);
		status_err = status & (SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT);
	}

	status_err = status &
		(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND);
	while (!status_err) {
		status = readl(&host->base->status);
		status_err = status &
			(SDI_STA_DCRCFAIL | SDI_STA_DTIMEOUT | SDI_STA_DBCKEND);
	}

	if (status & SDI_STA_DTIMEOUT) {
		printf("Write data timed out, xfercount:%llu,status:0x%08X\n",
		       xfercount, status);
		return -ETIMEDOUT;
	} else if (status & SDI_STA_DCRCFAIL) {
		printf("Write data CRC error\n");
		return -EILSEQ;
	}

	writel(SDI_ICR_MASK, &host->base->status_clear);

	if (xfercount) {
		printf("Write data error, xfercount:%llu", xfercount);
		return -ENOBUFS;
	}

	return 0;
}

static int do_data_transfer(struct mmc *dev,
			    struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	int error = -ETIMEDOUT;
	struct pl180_mmc_host *host = dev->priv;
	u32 blksz = 0;
	u32 data_ctrl = 0;
	u32 data_len = (u32) (data->blocks * data->blocksize);

	if (!host->version2) {
		blksz = (ffs(data->blocksize) - 1);
		data_ctrl |= ((blksz << 4) & SDI_DCTRL_DBLKSIZE_MASK);
	} else {
		blksz = data->blocksize;
		data_ctrl |= (blksz << SDI_DCTRL_DBLOCKSIZE_V2_SHIFT);
	}
	data_ctrl |= SDI_DCTRL_DTEN | SDI_DCTRL_BUSYMODE;

	writel(SDI_DTIMER_DEFAULT, &host->base->datatimer);
	writel(data_len, &host->base->datalength);
	udelay(DATA_REG_DELAY);

	if (data->flags & MMC_DATA_READ) {
		data_ctrl |= SDI_DCTRL_DTDIR_IN;
		writel(data_ctrl, &host->base->datactrl);

		error = do_command(dev, cmd);
		if (error)
			return error;

		error = read_bytes(dev, (u32 *)data->dest, (u32)data->blocks,
				   (u32)data->blocksize);
	} else if (data->flags & MMC_DATA_WRITE) {
		error = do_command(dev, cmd);
		if (error)
			return error;

		writel(data_ctrl, &host->base->datactrl);
		error = write_bytes(dev, (u32 *)data->src, (u32)data->blocks,
							(u32)data->blocksize);
	}

	return error;
}

static int host_request(struct mmc *dev,
			struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	int result;

	if (data)
		result = do_data_transfer(dev, cmd, data);
	else
		result = do_command(dev, cmd);

	return result;
}

static int  host_set_ios(struct mmc *dev)
{
	struct pl180_mmc_host *host = dev->priv;
	u32 sdi_clkcr;

	sdi_clkcr = readl(&host->base->clock);

	/* Ramp up the clock rate */
	if (dev->clock) {
		u32 clkdiv = 0;
		u32 tmp_clock;

		if (dev->clock >= dev->cfg->f_max) {
			clkdiv = 0;
			dev->clock = dev->cfg->f_max;
		} else {
			clkdiv = (host->clock_in / dev->clock) - 2;
		}

		tmp_clock = host->clock_in / (clkdiv + 2);
		while (tmp_clock > dev->clock) {
			clkdiv++;
			tmp_clock = host->clock_in / (clkdiv + 2);
		}

		if (clkdiv > SDI_CLKCR_CLKDIV_MASK)
			clkdiv = SDI_CLKCR_CLKDIV_MASK;

		tmp_clock = host->clock_in / (clkdiv + 2);
		dev->clock = tmp_clock;
		sdi_clkcr &= ~(SDI_CLKCR_CLKDIV_MASK);
		sdi_clkcr |= clkdiv;
	}

	/* Set the bus width */
	if (dev->bus_width) {
		u32 buswidth = 0;

		switch (dev->bus_width) {
		case 1:
			buswidth |= SDI_CLKCR_WIDBUS_1;
			break;
		case 4:
			buswidth |= SDI_CLKCR_WIDBUS_4;
			break;
		case 8:
			buswidth |= SDI_CLKCR_WIDBUS_8;
			break;
		default:
			printf("Invalid bus width: %d\n", dev->bus_width);
			break;
		}
		sdi_clkcr &= ~(SDI_CLKCR_WIDBUS_MASK);
		sdi_clkcr |= buswidth;
	}

	writel(sdi_clkcr, &host->base->clock);
	udelay(CLK_CHANGE_DELAY);

	return 0;
}

#ifndef CONFIG_DM_MMC
/* MMC uses open drain drivers in the enumeration phase */
static int mmc_host_reset(struct mmc *dev)
{
	struct pl180_mmc_host *host = dev->priv;

	writel(host->pwr_init, &host->base->power);

	return 0;
}

static const struct mmc_ops arm_pl180_mmci_ops = {
	.send_cmd = host_request,
	.set_ios = host_set_ios,
	.init = mmc_host_reset,
};

/*
 * mmc_host_init - initialize the mmc controller.
 * Set initial clock and power for mmc slot.
 * Initialize mmc struct and register with mmc framework.
 */

int arm_pl180_mmci_init(struct pl180_mmc_host *host, struct mmc **mmc)
{
	u32 sdi_u32;

	writel(host->pwr_init, &host->base->power);
	writel(host->clkdiv_init, &host->base->clock);
	udelay(CLK_CHANGE_DELAY);

	/* Disable mmc interrupts */
	sdi_u32 = readl(&host->base->mask0) & ~SDI_MASK0_MASK;
	writel(sdi_u32, &host->base->mask0);

	host->cfg.name = host->name;
	host->cfg.ops = &arm_pl180_mmci_ops;

	/* TODO remove the duplicates */
	host->cfg.host_caps = host->caps;
	host->cfg.voltages = host->voltages;
	host->cfg.f_min = host->clock_min;
	host->cfg.f_max = host->clock_max;
	if (host->b_max != 0)
		host->cfg.b_max = host->b_max;
	else
		host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	*mmc = mmc_create(&host->cfg, host);
	if (!*mmc)
		return -1;
	debug("registered mmc interface number is:%d\n",
	      (*mmc)->block_dev.devnum);

	return 0;
}
#endif

#ifdef CONFIG_DM_MMC
static void arm_pl180_mmc_init(struct pl180_mmc_host *host)
{
	u32 sdi_u32;

	writel(host->pwr_init, &host->base->power);
	writel(host->clkdiv_init, &host->base->clock);
	udelay(CLK_CHANGE_DELAY);

	/* Disable mmc interrupts */
	sdi_u32 = readl(&host->base->mask0) & ~SDI_MASK0_MASK;
	writel(sdi_u32, &host->base->mask0);
}

static int arm_pl180_mmc_probe(struct udevice *dev)
{
	struct arm_pl180_mmc_plat *pdata = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = &pdata->mmc;
	struct pl180_mmc_host *host = dev->priv;
	struct mmc_config *cfg = &pdata->cfg;
	struct clk clk;
	u32 bus_width;
	u32 periphid;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		clk_free(&clk);
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	host->pwr_init = INIT_PWR;
	host->clkdiv_init = SDI_CLKCR_CLKDIV_INIT_V1 | SDI_CLKCR_CLKEN |
			    SDI_CLKCR_HWFC_EN;
	host->clock_in = clk_get_rate(&clk);

	periphid = dev_read_u32_default(dev, "arm,primecell-periphid", 0);
	switch (periphid) {
	case STM32_MMCI_ID: /* stm32 variant */
		host->version2 = false;
		break;
	default:
		host->version2 = true;
	}

	cfg->name = dev->name;
	cfg->voltages = VOLTAGE_WINDOW_SD;
	cfg->host_caps = 0;
	cfg->f_min = host->clock_in / (2 * (SDI_CLKCR_CLKDIV_INIT_V1 + 1));
	cfg->f_max = dev_read_u32_default(dev, "max-frequency", MMC_CLOCK_MAX);
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	gpio_request_by_name(dev, "cd-gpios", 0, &host->cd_gpio, GPIOD_IS_IN);

	bus_width = dev_read_u32_default(dev, "bus-width", 1);
	switch (bus_width) {
	case 8:
		cfg->host_caps |= MMC_MODE_8BIT;
		/* Hosts capable of 8-bit transfers can also do 4 bits */
	case 4:
		cfg->host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		dev_err(dev, "Invalid bus-width value %u\n", bus_width);
	}

	arm_pl180_mmc_init(host);
	mmc->priv = host;
	mmc->dev = dev;
	upriv->mmc = mmc;

	return 0;
}

int arm_pl180_mmc_bind(struct udevice *dev)
{
	struct arm_pl180_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static int dm_host_request(struct udevice *dev, struct mmc_cmd *cmd,
			   struct mmc_data *data)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return host_request(mmc, cmd, data);
}

static int dm_host_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return host_set_ios(mmc);
}

static int dm_mmc_getcd(struct udevice *dev)
{
	struct pl180_mmc_host *host = dev->priv;
	int value = 1;

	if (dm_gpio_is_valid(&host->cd_gpio))
		value = dm_gpio_get_value(&host->cd_gpio);

	return value;
}

static const struct dm_mmc_ops arm_pl180_dm_mmc_ops = {
	.send_cmd = dm_host_request,
	.set_ios = dm_host_set_ios,
	.get_cd = dm_mmc_getcd,
};

static int arm_pl180_mmc_ofdata_to_platdata(struct udevice *dev)
{
	struct pl180_mmc_host *host = dev->priv;
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	host->base = (void *)addr;

	return 0;
}

static const struct udevice_id arm_pl180_mmc_match[] = {
	{ .compatible = "arm,pl180" },
	{ .compatible = "arm,primecell" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(arm_pl180_mmc) = {
	.name = "arm_pl180_mmc",
	.id = UCLASS_MMC,
	.of_match = arm_pl180_mmc_match,
	.ops = &arm_pl180_dm_mmc_ops,
	.probe = arm_pl180_mmc_probe,
	.ofdata_to_platdata = arm_pl180_mmc_ofdata_to_platdata,
	.bind = arm_pl180_mmc_bind,
	.priv_auto_alloc_size = sizeof(struct pl180_mmc_host),
	.platdata_auto_alloc_size = sizeof(struct arm_pl180_mmc_plat),
};
#endif
