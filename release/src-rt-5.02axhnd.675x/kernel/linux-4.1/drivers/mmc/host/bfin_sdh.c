/*
 * bfin_sdh.c - Analog Devices Blackfin SDH Controller
 *
 * Copyright (C) 2007-2009 Analog Device Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#define DRIVER_NAME	"bfin-sdh"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/proc_fs.h>
#include <linux/gfp.h>

#include <asm/cacheflush.h>
#include <asm/dma.h>
#include <asm/portmux.h>
#include <asm/bfin_sdh.h>

#if defined(CONFIG_BF51x) || defined(__ADSPBF60x__)
#define bfin_read_SDH_CLK_CTL		bfin_read_RSI_CLK_CTL
#define bfin_write_SDH_CLK_CTL		bfin_write_RSI_CLK_CTL
#define bfin_write_SDH_ARGUMENT		bfin_write_RSI_ARGUMENT
#define bfin_write_SDH_COMMAND		bfin_write_RSI_COMMAND
#define bfin_write_SDH_DATA_TIMER	bfin_write_RSI_DATA_TIMER
#define bfin_read_SDH_RESPONSE0		bfin_read_RSI_RESPONSE0
#define bfin_read_SDH_RESPONSE1		bfin_read_RSI_RESPONSE1
#define bfin_read_SDH_RESPONSE2		bfin_read_RSI_RESPONSE2
#define bfin_read_SDH_RESPONSE3		bfin_read_RSI_RESPONSE3
#define bfin_write_SDH_DATA_LGTH	bfin_write_RSI_DATA_LGTH
#define bfin_read_SDH_DATA_CTL		bfin_read_RSI_DATA_CTL
#define bfin_write_SDH_DATA_CTL		bfin_write_RSI_DATA_CTL
#define bfin_read_SDH_DATA_CNT		bfin_read_RSI_DATA_CNT
#define bfin_write_SDH_STATUS_CLR	bfin_write_RSI_STATUS_CLR
#define bfin_read_SDH_E_STATUS		bfin_read_RSI_E_STATUS
#define bfin_write_SDH_E_STATUS		bfin_write_RSI_E_STATUS
#define bfin_read_SDH_STATUS		bfin_read_RSI_STATUS
#define bfin_write_SDH_MASK0		bfin_write_RSI_MASK0
#define bfin_write_SDH_E_MASK		bfin_write_RSI_E_MASK
#define bfin_read_SDH_CFG		bfin_read_RSI_CFG
#define bfin_write_SDH_CFG		bfin_write_RSI_CFG
# if defined(__ADSPBF60x__)
#  define bfin_read_SDH_BLK_SIZE	bfin_read_RSI_BLKSZ
#  define bfin_write_SDH_BLK_SIZE	bfin_write_RSI_BLKSZ
# else
#  define bfin_read_SDH_PWR_CTL		bfin_read_RSI_PWR_CTL
#  define bfin_write_SDH_PWR_CTL	bfin_write_RSI_PWR_CTL
# endif
#endif

struct sdh_host {
	struct mmc_host		*mmc;
	spinlock_t		lock;
	struct resource		*res;
	void __iomem		*base;
	int			irq;
	int			stat_irq;
	int			dma_ch;
	int			dma_dir;
	struct dma_desc_array	*sg_cpu;
	dma_addr_t		sg_dma;
	int			dma_len;

	unsigned long		sclk;
	unsigned int		imask;
	unsigned int		power_mode;
	unsigned int		clk_div;

	struct mmc_request	*mrq;
	struct mmc_command	*cmd;
	struct mmc_data		*data;
};

static struct bfin_sd_host *get_sdh_data(struct platform_device *pdev)
{
	return pdev->dev.platform_data;
}

static void sdh_stop_clock(struct sdh_host *host)
{
	bfin_write_SDH_CLK_CTL(bfin_read_SDH_CLK_CTL() & ~CLK_E);
	SSYNC();
}

static void sdh_enable_stat_irq(struct sdh_host *host, unsigned int mask)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->imask |= mask;
	bfin_write_SDH_MASK0(mask);
	SSYNC();
	spin_unlock_irqrestore(&host->lock, flags);
}

static void sdh_disable_stat_irq(struct sdh_host *host, unsigned int mask)
{
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);
	host->imask &= ~mask;
	bfin_write_SDH_MASK0(host->imask);
	SSYNC();
	spin_unlock_irqrestore(&host->lock, flags);
}

static int sdh_setup_data(struct sdh_host *host, struct mmc_data *data)
{
	unsigned int length;
	unsigned int data_ctl;
	unsigned int dma_cfg;
	unsigned int cycle_ns, timeout;

	dev_dbg(mmc_dev(host->mmc), "%s enter flags: 0x%x\n", __func__, data->flags);
	host->data = data;
	data_ctl = 0;
	dma_cfg = 0;

	length = data->blksz * data->blocks;
	bfin_write_SDH_DATA_LGTH(length);

	if (data->flags & MMC_DATA_STREAM)
		data_ctl |= DTX_MODE;

	if (data->flags & MMC_DATA_READ)
		data_ctl |= DTX_DIR;
	/* Only supports power-of-2 block size */
	if (data->blksz & (data->blksz - 1))
		return -EINVAL;
#ifndef RSI_BLKSZ
	data_ctl |= ((ffs(data->blksz) - 1) << 4);
#else
        bfin_write_SDH_BLK_SIZE(data->blksz);
#endif

	bfin_write_SDH_DATA_CTL(data_ctl);
	/* the time of a host clock period in ns */
	cycle_ns = 1000000000 / (host->sclk / (2 * (host->clk_div + 1)));
	timeout = data->timeout_ns / cycle_ns;
	timeout += data->timeout_clks;
	bfin_write_SDH_DATA_TIMER(timeout);
	SSYNC();

	if (data->flags & MMC_DATA_READ) {
		host->dma_dir = DMA_FROM_DEVICE;
		dma_cfg |= WNR;
	} else
		host->dma_dir = DMA_TO_DEVICE;

	sdh_enable_stat_irq(host, (DAT_CRC_FAIL | DAT_TIME_OUT | DAT_END));
	host->dma_len = dma_map_sg(mmc_dev(host->mmc), data->sg, data->sg_len, host->dma_dir);
#if defined(CONFIG_BF54x) || defined(CONFIG_BF60x)
	dma_cfg |= DMAFLOW_ARRAY | RESTART | WDSIZE_32 | DMAEN;
# ifdef RSI_BLKSZ
	dma_cfg |= PSIZE_32 | NDSIZE_3;
# else
	dma_cfg |= NDSIZE_5;
# endif
	{
		struct scatterlist *sg;
		int i;
		for_each_sg(data->sg, sg, host->dma_len, i) {
			host->sg_cpu[i].start_addr = sg_dma_address(sg);
			host->sg_cpu[i].cfg = dma_cfg;
			host->sg_cpu[i].x_count = sg_dma_len(sg) / 4;
			host->sg_cpu[i].x_modify = 4;
			dev_dbg(mmc_dev(host->mmc), "%d: start_addr:0x%lx, "
				"cfg:0x%lx, x_count:0x%lx, x_modify:0x%lx\n",
				i, host->sg_cpu[i].start_addr,
				host->sg_cpu[i].cfg, host->sg_cpu[i].x_count,
				host->sg_cpu[i].x_modify);
		}
	}
	flush_dcache_range((unsigned int)host->sg_cpu,
		(unsigned int)host->sg_cpu +
			host->dma_len * sizeof(struct dma_desc_array));
	/* Set the last descriptor to stop mode */
	host->sg_cpu[host->dma_len - 1].cfg &= ~(DMAFLOW | NDSIZE);
	host->sg_cpu[host->dma_len - 1].cfg |= DI_EN;

	set_dma_curr_desc_addr(host->dma_ch, (unsigned long *)host->sg_dma);
	set_dma_x_count(host->dma_ch, 0);
	set_dma_x_modify(host->dma_ch, 0);
	SSYNC();
	set_dma_config(host->dma_ch, dma_cfg);
#elif defined(CONFIG_BF51x)
	/* RSI DMA doesn't work in array mode */
	dma_cfg |= WDSIZE_32 | DMAEN;
	set_dma_start_addr(host->dma_ch, sg_dma_address(&data->sg[0]));
	set_dma_x_count(host->dma_ch, length / 4);
	set_dma_x_modify(host->dma_ch, 4);
	SSYNC();
	set_dma_config(host->dma_ch, dma_cfg);
#endif
	bfin_write_SDH_DATA_CTL(bfin_read_SDH_DATA_CTL() | DTX_DMA_E | DTX_E);

	SSYNC();

	dev_dbg(mmc_dev(host->mmc), "%s exit\n", __func__);
	return 0;
}

static void sdh_start_cmd(struct sdh_host *host, struct mmc_command *cmd)
{
	unsigned int sdh_cmd;
	unsigned int stat_mask;

	dev_dbg(mmc_dev(host->mmc), "%s enter cmd: 0x%p\n", __func__, cmd);
	WARN_ON(host->cmd != NULL);
	host->cmd = cmd;

	sdh_cmd = 0;
	stat_mask = 0;

	sdh_cmd |= cmd->opcode;

	if (cmd->flags & MMC_RSP_PRESENT) {
		sdh_cmd |= CMD_RSP;
		stat_mask |= CMD_RESP_END;
	} else {
		stat_mask |= CMD_SENT;
	}

	if (cmd->flags & MMC_RSP_136)
		sdh_cmd |= CMD_L_RSP;

	stat_mask |= CMD_CRC_FAIL | CMD_TIME_OUT;

	sdh_enable_stat_irq(host, stat_mask);

	bfin_write_SDH_ARGUMENT(cmd->arg);
	bfin_write_SDH_COMMAND(sdh_cmd | CMD_E);
	bfin_write_SDH_CLK_CTL(bfin_read_SDH_CLK_CTL() | CLK_E);
	SSYNC();
}

static void sdh_finish_request(struct sdh_host *host, struct mmc_request *mrq)
{
	dev_dbg(mmc_dev(host->mmc), "%s enter\n", __func__);
	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;
	mmc_request_done(host->mmc, mrq);
}

static int sdh_cmd_done(struct sdh_host *host, unsigned int stat)
{
	struct mmc_command *cmd = host->cmd;
	int ret = 0;

	dev_dbg(mmc_dev(host->mmc), "%s enter cmd: %p\n", __func__, cmd);
	if (!cmd)
		return 0;

	host->cmd = NULL;

	if (cmd->flags & MMC_RSP_PRESENT) {
		cmd->resp[0] = bfin_read_SDH_RESPONSE0();
		if (cmd->flags & MMC_RSP_136) {
			cmd->resp[1] = bfin_read_SDH_RESPONSE1();
			cmd->resp[2] = bfin_read_SDH_RESPONSE2();
			cmd->resp[3] = bfin_read_SDH_RESPONSE3();
		}
	}
	if (stat & CMD_TIME_OUT)
		cmd->error = -ETIMEDOUT;
	else if (stat & CMD_CRC_FAIL && cmd->flags & MMC_RSP_CRC)
		cmd->error = -EILSEQ;

	sdh_disable_stat_irq(host, (CMD_SENT | CMD_RESP_END | CMD_TIME_OUT | CMD_CRC_FAIL));

	if (host->data && !cmd->error) {
		if (host->data->flags & MMC_DATA_WRITE) {
			ret = sdh_setup_data(host, host->data);
			if (ret)
				return 0;
		}

		sdh_enable_stat_irq(host, DAT_END | RX_OVERRUN | TX_UNDERRUN | DAT_TIME_OUT);
	} else
		sdh_finish_request(host, host->mrq);

	return 1;
}

static int sdh_data_done(struct sdh_host *host, unsigned int stat)
{
	struct mmc_data *data = host->data;

	dev_dbg(mmc_dev(host->mmc), "%s enter stat: 0x%x\n", __func__, stat);
	if (!data)
		return 0;

	disable_dma(host->dma_ch);
	dma_unmap_sg(mmc_dev(host->mmc), data->sg, data->sg_len,
		     host->dma_dir);

	if (stat & DAT_TIME_OUT)
		data->error = -ETIMEDOUT;
	else if (stat & DAT_CRC_FAIL)
		data->error = -EILSEQ;
	else if (stat & (RX_OVERRUN | TX_UNDERRUN))
		data->error = -EIO;

	if (!data->error)
		data->bytes_xfered = data->blocks * data->blksz;
	else
		data->bytes_xfered = 0;

	bfin_write_SDH_STATUS_CLR(DAT_END_STAT | DAT_TIMEOUT_STAT | \
			DAT_CRC_FAIL_STAT | DAT_BLK_END_STAT | RX_OVERRUN | TX_UNDERRUN);
	bfin_write_SDH_DATA_CTL(0);
	SSYNC();

	host->data = NULL;
	if (host->mrq->stop) {
		sdh_stop_clock(host);
		sdh_start_cmd(host, host->mrq->stop);
	} else {
		sdh_finish_request(host, host->mrq);
	}

	return 1;
}

static void sdh_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct sdh_host *host = mmc_priv(mmc);
	int ret = 0;

	dev_dbg(mmc_dev(host->mmc), "%s enter, mrp:%p, cmd:%p\n", __func__, mrq, mrq->cmd);
	WARN_ON(host->mrq != NULL);

	spin_lock(&host->lock);
	host->mrq = mrq;
	host->data = mrq->data;

	if (mrq->data && mrq->data->flags & MMC_DATA_READ) {
		ret = sdh_setup_data(host, mrq->data);
		if (ret)
			goto data_err;
	}

	sdh_start_cmd(host, mrq->cmd);
data_err:
	spin_unlock(&host->lock);
}

static void sdh_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct sdh_host *host;
	u16 clk_ctl = 0;
#ifndef RSI_BLKSZ
	u16 pwr_ctl = 0;
#endif
	u16 cfg;
	host = mmc_priv(mmc);

	spin_lock(&host->lock);

	cfg = bfin_read_SDH_CFG();
	cfg |= MWE;
	switch (ios->bus_width) {
	case MMC_BUS_WIDTH_4:
#ifndef RSI_BLKSZ
		cfg &= ~PD_SDDAT3;
#endif
		cfg |= PUP_SDDAT3;
		/* Enable 4 bit SDIO */
		cfg |= SD4E;
		clk_ctl |= WIDE_BUS_4;
		break;
	case MMC_BUS_WIDTH_8:
#ifndef RSI_BLKSZ
		cfg &= ~PD_SDDAT3;
#endif
		cfg |= PUP_SDDAT3;
		/* Disable 4 bit SDIO */
		cfg &= ~SD4E;
		clk_ctl |= BYTE_BUS_8;
		break;
	default:
		cfg &= ~PUP_SDDAT3;
		/* Disable 4 bit SDIO */
		cfg &= ~SD4E;
	}
	bfin_write_SDH_CFG(cfg);

	host->power_mode = ios->power_mode;
#ifndef RSI_BLKSZ
	if (ios->bus_mode == MMC_BUSMODE_OPENDRAIN) {
		pwr_ctl |= ROD_CTL;
# ifndef CONFIG_SDH_BFIN_MISSING_CMD_PULLUP_WORKAROUND
		pwr_ctl |= SD_CMD_OD;
# endif
	}

	if (ios->power_mode != MMC_POWER_OFF)
		pwr_ctl |= PWR_ON;
	else
		pwr_ctl &= ~PWR_ON;

	bfin_write_SDH_PWR_CTL(pwr_ctl);
#else
# ifndef CONFIG_SDH_BFIN_MISSING_CMD_PULLUP_WORKAROUND
	if (ios->bus_mode == MMC_BUSMODE_OPENDRAIN)
		cfg |= SD_CMD_OD;
	else
		cfg &= ~SD_CMD_OD;
# endif

	if (ios->power_mode != MMC_POWER_OFF)
		cfg |= PWR_ON;
	else
		cfg &= ~PWR_ON;

	bfin_write_SDH_CFG(cfg);
#endif
	SSYNC();

	if (ios->power_mode == MMC_POWER_ON && ios->clock) {
		unsigned char clk_div;
		clk_div = (get_sclk() / ios->clock - 1) / 2;
		clk_div = min_t(unsigned char, clk_div, 0xFF);
		clk_ctl |= clk_div;
		clk_ctl |= CLK_E;
		host->clk_div = clk_div;
		bfin_write_SDH_CLK_CTL(clk_ctl);
	} else
		sdh_stop_clock(host);

	/* set up sdh interrupt mask*/
	if (ios->power_mode == MMC_POWER_ON)
		bfin_write_SDH_MASK0(DAT_END | DAT_TIME_OUT | DAT_CRC_FAIL |
			RX_OVERRUN | TX_UNDERRUN | CMD_SENT | CMD_RESP_END |
			CMD_TIME_OUT | CMD_CRC_FAIL);
	else
		bfin_write_SDH_MASK0(0);
	SSYNC();

	spin_unlock(&host->lock);

	dev_dbg(mmc_dev(host->mmc), "SDH: clk_div = 0x%x actual clock:%ld expected clock:%d\n",
		host->clk_div,
		host->clk_div ? get_sclk() / (2 * (host->clk_div + 1)) : 0,
		ios->clock);
}

static const struct mmc_host_ops sdh_ops = {
	.request	= sdh_request,
	.set_ios	= sdh_set_ios,
};

static irqreturn_t sdh_dma_irq(int irq, void *devid)
{
	struct sdh_host *host = devid;

	dev_dbg(mmc_dev(host->mmc), "%s enter, irq_stat: 0x%04lx\n", __func__,
		get_dma_curr_irqstat(host->dma_ch));
	clear_dma_irqstat(host->dma_ch);
	SSYNC();

	return IRQ_HANDLED;
}

static irqreturn_t sdh_stat_irq(int irq, void *devid)
{
	struct sdh_host *host = devid;
	unsigned int status;
	int handled = 0;

	dev_dbg(mmc_dev(host->mmc), "%s enter\n", __func__);

	spin_lock(&host->lock);

	status = bfin_read_SDH_E_STATUS();
	if (status & SD_CARD_DET) {
		mmc_detect_change(host->mmc, 0);
		bfin_write_SDH_E_STATUS(SD_CARD_DET);
	}
	status = bfin_read_SDH_STATUS();
	if (status & (CMD_SENT | CMD_RESP_END | CMD_TIME_OUT | CMD_CRC_FAIL)) {
		handled |= sdh_cmd_done(host, status);
		bfin_write_SDH_STATUS_CLR(CMD_SENT_STAT | CMD_RESP_END_STAT | \
				CMD_TIMEOUT_STAT | CMD_CRC_FAIL_STAT);
		SSYNC();
	}

	status = bfin_read_SDH_STATUS();
	if (status & (DAT_END | DAT_TIME_OUT | DAT_CRC_FAIL | RX_OVERRUN | TX_UNDERRUN))
		handled |= sdh_data_done(host, status);

	spin_unlock(&host->lock);

	dev_dbg(mmc_dev(host->mmc), "%s exit\n\n", __func__);

	return IRQ_RETVAL(handled);
}

static void sdh_reset(void)
{
#if defined(CONFIG_BF54x)
	/* Secure Digital Host shares DMA with Nand controller */
	bfin_write_DMAC1_PERIMUX(bfin_read_DMAC1_PERIMUX() | 0x1);
#endif

	bfin_write_SDH_CFG(bfin_read_SDH_CFG() | CLKS_EN);
	SSYNC();

	/* Disable card inserting detection pin. set MMC_CAP_NEEDS_POLL, and
	 * mmc stack will do the detection.
	 */
	bfin_write_SDH_CFG((bfin_read_SDH_CFG() & 0x1F) | (PUP_SDDAT | PUP_SDDAT3));
	SSYNC();
}

static int sdh_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct sdh_host *host;
	struct bfin_sd_host *drv_data = get_sdh_data(pdev);
	int ret;

	if (!drv_data) {
		dev_err(&pdev->dev, "missing platform driver data\n");
		ret = -EINVAL;
		goto out;
	}

	mmc = mmc_alloc_host(sizeof(struct sdh_host), &pdev->dev);
	if (!mmc) {
		ret = -ENOMEM;
		goto out;
	}

	mmc->ops = &sdh_ops;
#if defined(CONFIG_BF51x)
	mmc->max_segs = 1;
#else
	mmc->max_segs = PAGE_SIZE / sizeof(struct dma_desc_array);
#endif
#ifdef RSI_BLKSZ
	mmc->max_seg_size = -1;
#else
	mmc->max_seg_size = 1 << 16;
#endif
	mmc->max_blk_size = 1 << 11;
	mmc->max_blk_count = 1 << 11;
	mmc->max_req_size = PAGE_SIZE;
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->f_max = get_sclk();
	mmc->f_min = mmc->f_max >> 9;
	mmc->caps = MMC_CAP_4_BIT_DATA | MMC_CAP_NEEDS_POLL;
	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->sclk = get_sclk();

	spin_lock_init(&host->lock);
	host->irq = drv_data->irq_int0;
	host->dma_ch = drv_data->dma_chan;

	ret = request_dma(host->dma_ch, DRIVER_NAME "DMA");
	if (ret) {
		dev_err(&pdev->dev, "unable to request DMA channel\n");
		goto out1;
	}

	ret = set_dma_callback(host->dma_ch, sdh_dma_irq, host);
	if (ret) {
		dev_err(&pdev->dev, "unable to request DMA irq\n");
		goto out2;
	}

	host->sg_cpu = dma_alloc_coherent(&pdev->dev, PAGE_SIZE, &host->sg_dma, GFP_KERNEL);
	if (host->sg_cpu == NULL) {
		ret = -ENOMEM;
		goto out2;
	}

	platform_set_drvdata(pdev, mmc);

	ret = request_irq(host->irq, sdh_stat_irq, 0, "SDH Status IRQ", host);
	if (ret) {
		dev_err(&pdev->dev, "unable to request status irq\n");
		goto out3;
	}

	ret = peripheral_request_list(drv_data->pin_req, DRIVER_NAME);
	if (ret) {
		dev_err(&pdev->dev, "unable to request peripheral pins\n");
		goto out4;
	}

	sdh_reset();

	mmc_add_host(mmc);
	return 0;

out4:
	free_irq(host->irq, host);
out3:
	mmc_remove_host(mmc);
	dma_free_coherent(&pdev->dev, PAGE_SIZE, host->sg_cpu, host->sg_dma);
out2:
	free_dma(host->dma_ch);
out1:
	mmc_free_host(mmc);
 out:
	return ret;
}

static int sdh_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);

	if (mmc) {
		struct sdh_host *host = mmc_priv(mmc);

		mmc_remove_host(mmc);

		sdh_stop_clock(host);
		free_irq(host->irq, host);
		free_dma(host->dma_ch);
		dma_free_coherent(&pdev->dev, PAGE_SIZE, host->sg_cpu, host->sg_dma);

		mmc_free_host(mmc);
	}

	return 0;
}

#ifdef CONFIG_PM
static int sdh_suspend(struct platform_device *dev, pm_message_t state)
{
	struct bfin_sd_host *drv_data = get_sdh_data(dev);

	peripheral_free_list(drv_data->pin_req);

	return 0;
}

static int sdh_resume(struct platform_device *dev)
{
	struct bfin_sd_host *drv_data = get_sdh_data(dev);
	int ret = 0;

	ret = peripheral_request_list(drv_data->pin_req, DRIVER_NAME);
	if (ret) {
		dev_err(&dev->dev, "unable to request peripheral pins\n");
		return ret;
	}

	sdh_reset();
	return ret;
}
#else
# define sdh_suspend NULL
# define sdh_resume  NULL
#endif

static struct platform_driver sdh_driver = {
	.probe   = sdh_probe,
	.remove  = sdh_remove,
	.suspend = sdh_suspend,
	.resume  = sdh_resume,
	.driver  = {
		.name = DRIVER_NAME,
	},
};

module_platform_driver(sdh_driver);

MODULE_DESCRIPTION("Blackfin Secure Digital Host Driver");
MODULE_AUTHOR("Cliff Cai, Roy Huang");
MODULE_LICENSE("GPL");
