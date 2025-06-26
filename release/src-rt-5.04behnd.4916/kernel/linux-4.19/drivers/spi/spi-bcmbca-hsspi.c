#if defined(CONFIG_BCM_KF_SPI)
/*
 * Broadcom BCMBCA High Speed SPI Controller driver
 *
 * Copyright 2000-2010 Broadcom Corporation
 * Copyright 2012-2013 Jonas Gorski <jogo@openwrt.org>
 *
 * Licensed under the GNU/GPL. See COPYING for details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

/* Broadcom Legacy SPI device driver flags */
#define SPIDEV_CONTROLLER_STATE_SET		BIT(31)
#define SPIDEV_CONTROLLER_STATE_GATE_CLK_SSOFF	BIT(29)

#define spidev_ctrl_data(spi)			\
	((u32)((uintptr_t)(spi)->controller_data))

#define HSSPI_GLOBAL_CTRL_REG			0x0
#define GLOBAL_CTRL_CS_POLARITY_SHIFT		0
#define GLOBAL_CTRL_CS_POLARITY_MASK		0x000000ff
#define GLOBAL_CTRL_PLL_CLK_CTRL_SHIFT		8
#define GLOBAL_CTRL_PLL_CLK_CTRL_MASK		0x0000ff00
#define GLOBAL_CTRL_CLK_GATE_SSOFF		BIT(16)
#define GLOBAL_CTRL_CLK_POLARITY		BIT(17)
#define GLOBAL_CTRL_MOSI_IDLE			BIT(18)

#define HSSPI_GLOBAL_EXT_TRIGGER_REG		0x4

#define HSSPI_INT_STATUS_REG			0x8
#define HSSPI_INT_STATUS_MASKED_REG		0xc
#define HSSPI_INT_MASK_REG			0x10

#define HSSPI_PINGx_CMD_DONE(i)			BIT((i * 8) + 0)
#define HSSPI_PINGx_RX_OVER(i)			BIT((i * 8) + 1)
#define HSSPI_PINGx_TX_UNDER(i)			BIT((i * 8) + 2)
#define HSSPI_PINGx_POLL_TIMEOUT(i)		BIT((i * 8) + 3)
#define HSSPI_PINGx_CTRL_INVAL(i)		BIT((i * 8) + 4)

#define HSSPI_INT_CLEAR_ALL			0xff001f1f

#define HSSPI_PINGPONG_COMMAND_REG(x)		(0x80 + (x) * 0x40)
#define PINGPONG_CMD_COMMAND_MASK		0xf
#define PINGPONG_COMMAND_NOOP			0
#define PINGPONG_COMMAND_START_NOW		1
#define PINGPONG_COMMAND_START_TRIGGER		2
#define PINGPONG_COMMAND_HALT			3
#define PINGPONG_COMMAND_FLUSH			4
#define PINGPONG_CMD_PROFILE_SHIFT		8
#define PINGPONG_CMD_SS_SHIFT			12

#define HSSPI_PINGPONG_STATUS_REG(x)		(0x84 + (x) * 0x40)
#define HSSPI_PINGPONG_STATUS_SRC_BUSY          BIT(1)

#define HSSPI_PROFILE_CLK_CTRL_REG(x)		(0x100 + (x) * 0x20)
#define CLK_CTRL_FREQ_CTRL_MASK			0x0000ffff
#define CLK_CTRL_SPI_CLK_2X_SEL			BIT(14)
#define CLK_CTRL_ACCUM_RST_ON_LOOP		BIT(15)
#define CLK_CTRL_CLK_POLARITY			BIT(16)

#define HSSPI_PROFILE_SIGNAL_CTRL_REG(x)	(0x104 + (x) * 0x20)
#define SIGNAL_CTRL_LATCH_RISING		BIT(12)
#define SIGNAL_CTRL_LAUNCH_RISING		BIT(13)
#define SIGNAL_CTRL_ASYNC_INPUT_PATH		BIT(16)

#define HSSPI_PROFILE_MODE_CTRL_REG(x)		(0x108 + (x) * 0x20)
#define MODE_CTRL_MULTIDATA_RD_STRT_SHIFT	8
#define MODE_CTRL_MULTIDATA_WR_STRT_SHIFT	12
#define MODE_CTRL_MULTIDATA_RD_SIZE_SHIFT	16
#define MODE_CTRL_MULTIDATA_WR_SIZE_SHIFT	18
#define MODE_CTRL_MODE_3WIRE			BIT(20)
#define MODE_CTRL_PREPENDBYTE_CNT_SHIFT		24

#define HSSPI_FIFO_REG(x)			(0x200 + (x) * 0x200)

#define HSSPI_OP_MULTIBIT			BIT(11)
#define HSSPI_OP_CODE_SHIFT			13
#define HSSPI_OP_SLEEP				(0 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_READ_WRITE			(1 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_WRITE				(2 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_READ				(3 << HSSPI_OP_CODE_SHIFT)
#define HSSPI_OP_SETIRQ				(4 << HSSPI_OP_CODE_SHIFT)

#define HSSPI_BUFFER_LEN			512
#define HSSPI_OPCODE_LEN			2

#define HSSPI_MAX_PREPEND_LEN			15

#define HSSPI_MAX_SYNC_CLOCK			30000000

#define HSSPI_SPI_MAX_CS			8
#define HSSPI_BUS_NUM				1	/* 0 is legacy SPI */

#define SPIM_CTRL_CS_OVERRIDE_SEL_SHIFT		0
#define SPIM_CTRL_CS_OVERRIDE_SEL_MASK		0xff
#define SPIM_CTRL_CS_OVERRIDE_VAL_SHIFT		8
#define SPIM_CTRL_CS_OVERRIDE_VAL_MASK		0xff

struct bcmbca_hsspi {
	struct completion done;
	struct mutex bus_mutex;
	struct platform_device *pdev;
	struct clk *clk;
	struct clk *pll_clk;
	void __iomem *regs;
	void __iomem *spim_ctrl;  
	u8 __iomem *fifo;
	u32 speed_hz;
	u8 cs_polarity;

	int polling;
};

#define BCMBCA_MUTEX_LOCK(m) do{if(!oops_in_progress)mutex_lock(m);}while(0)
#define BCMBCA_MUTEX_UNLOCK(m) do{if(!oops_in_progress)mutex_unlock(m);}while(0)

static void bcmbca_hsspi_set_clk(struct bcmbca_hsspi *bs,
				  struct spi_device *spi, int hz);

static inline int bcmbca_hsspi_dev_no_clk_gate(struct spi_device *spi)
{
	u32 value = 0;

	/* check spi device dn first */
	if (of_property_read_u32(spi->dev.of_node, "no_clk_gate", &value) == 0)
		return value;

	/* check spi dev controller data for legacy device support */
	value = spidev_ctrl_data(spi);
	return ((value & SPIDEV_CONTROLLER_STATE_SET) &&
		!(value & SPIDEV_CONTROLLER_STATE_GATE_CLK_SSOFF));
}

static void bcmbca_hsspi_set_clk_gate(struct bcmbca_hsspi *bs,
				       struct spi_device *spi)
{
	u32 reg = 0;

	if (bcmbca_hsspi_dev_no_clk_gate(spi)) {
		BCMBCA_MUTEX_LOCK(&bs->bus_mutex);
		reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
		reg |= GLOBAL_CTRL_CLK_GATE_SSOFF;
		__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
		BCMBCA_MUTEX_UNLOCK(&bs->bus_mutex);
	}
}

static void bcmbca_hsspi_set_cs(struct bcmbca_hsspi *bs, unsigned int cs,
				 bool active)
{
 	u32 reg;

	/* No cs orerriden needed for SS7 internal cs on pcm based voice dev */
	if (cs == 7)
		return;

	BCMBCA_MUTEX_LOCK(&bs->bus_mutex);

	if (active) {
		/* activate cs by setting the override bit and active value bit*/
		reg = __raw_readl(bs->spim_ctrl);
		reg |= BIT(cs+SPIM_CTRL_CS_OVERRIDE_SEL_SHIFT);
		reg &= ~BIT(cs+SPIM_CTRL_CS_OVERRIDE_VAL_SHIFT);
		if (bs->cs_polarity & BIT(cs))
			reg |= BIT(cs+SPIM_CTRL_CS_OVERRIDE_VAL_SHIFT);
		__raw_writel(reg, bs->spim_ctrl);
	} else {
		/* clear the cs override bit */	  
		reg = __raw_readl(bs->spim_ctrl);
		reg &= ~BIT(cs+SPIM_CTRL_CS_OVERRIDE_SEL_SHIFT);
		__raw_writel(reg, bs->spim_ctrl);	  
	}

	BCMBCA_MUTEX_UNLOCK(&bs->bus_mutex);
}

static void bcmbca_hsspi_set_clk(struct bcmbca_hsspi *bs,
				  struct spi_device *spi, int hz)
{
	unsigned int profile = spi->chip_select;
	u32 reg;

	reg = DIV_ROUND_UP(2048, DIV_ROUND_UP(bs->speed_hz, hz));
	__raw_writel(CLK_CTRL_ACCUM_RST_ON_LOOP | reg,
		     bs->regs + HSSPI_PROFILE_CLK_CTRL_REG(profile));

	reg = __raw_readl(bs->regs + HSSPI_PROFILE_SIGNAL_CTRL_REG(profile));
	if (hz > HSSPI_MAX_SYNC_CLOCK)
		reg |= SIGNAL_CTRL_ASYNC_INPUT_PATH;
	else
		reg &= ~SIGNAL_CTRL_ASYNC_INPUT_PATH;
	__raw_writel(reg, bs->regs + HSSPI_PROFILE_SIGNAL_CTRL_REG(profile));

	BCMBCA_MUTEX_LOCK(&bs->bus_mutex);
	/* setup clock polarity */
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
	reg &= ~GLOBAL_CTRL_CLK_POLARITY;
	if (spi->mode & SPI_CPOL)
		reg |= GLOBAL_CTRL_CLK_POLARITY;

	if (bcmbca_hsspi_dev_no_clk_gate(spi)) {
		reg &= ~GLOBAL_CTRL_CLK_GATE_SSOFF;
	}
	else {
		reg |= GLOBAL_CTRL_CLK_GATE_SSOFF;
	}

	__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);

	BCMBCA_MUTEX_UNLOCK(&bs->bus_mutex);
}

static int bcmbca_hsspi_do_txrx(struct spi_device *spi, struct spi_transfer *t,
								struct spi_message *msg)
{
	struct bcmbca_hsspi *bs = spi_master_get_devdata(spi->master);
	unsigned int chip_select = spi->chip_select;
	u16 opcode = 0;
	int pending = t->len;
	int step_size = HSSPI_BUFFER_LEN;
	const u8 *tx = t->tx_buf;
	u8 *rx = t->rx_buf;
	u32 reg = 0, cs_act = 0;
	bool keep_cs = false;

	bcmbca_hsspi_set_clk(bs, spi, t->speed_hz);
	
	if (tx && rx)
		opcode = HSSPI_OP_READ_WRITE;
	else if (tx)
		opcode = HSSPI_OP_WRITE;
	else if (rx)
		opcode = HSSPI_OP_READ;

	if (opcode != HSSPI_OP_READ)
		step_size -= HSSPI_OPCODE_LEN;

	if ((opcode == HSSPI_OP_READ && t->rx_nbits == SPI_NBITS_DUAL) ||
	    (opcode == HSSPI_OP_WRITE && t->tx_nbits == SPI_NBITS_DUAL)) {
		opcode |= HSSPI_OP_MULTIBIT;

		if (t->rx_nbits == SPI_NBITS_DUAL)
			reg |= 1 << MODE_CTRL_MULTIDATA_RD_SIZE_SHIFT;
		if (t->tx_nbits == SPI_NBITS_DUAL)
			reg |= 1 << MODE_CTRL_MULTIDATA_WR_SIZE_SHIFT;
	}

	__raw_writel(reg | 0xff,
		     bs->regs + HSSPI_PROFILE_MODE_CTRL_REG(chip_select));

	while (pending > 0) {
		int curr_step = min_t(int, step_size, pending);

		reinit_completion(&bs->done);
		if (tx) {
			memcpy_toio(bs->fifo + HSSPI_OPCODE_LEN, tx, curr_step);
			tx += curr_step;
		}
		__raw_writew(cpu_to_be16(opcode | curr_step), bs->fifo);

		/* enable interrupt */
		__raw_writel(HSSPI_PINGx_CMD_DONE(0),
			    bs->regs + HSSPI_INT_MASK_REG);

		if (!cs_act) {
			bcmbca_hsspi_set_cs(bs, chip_select, true);
			cs_act = 1;
		}
		reg = chip_select << PINGPONG_CMD_SS_SHIFT |
			    chip_select << PINGPONG_CMD_PROFILE_SHIFT |
			    PINGPONG_COMMAND_START_NOW;
		__raw_writel(reg, bs->regs + HSSPI_PINGPONG_COMMAND_REG(0));
		
		if (!oops_in_progress && !bs->polling) {
			if (wait_for_completion_timeout(&bs->done, HZ) == 0) {
				dev_err(&bs->pdev->dev, "transfer timed out!\n");
				bcmbca_hsspi_set_cs(bs, chip_select, false);				
				return -ETIMEDOUT;
			}
		} else {
			while (__raw_readl(bs->regs +
			       HSSPI_PINGPONG_STATUS_REG(0)) &
			       HSSPI_PINGPONG_STATUS_SRC_BUSY)
				cpu_relax();
		}

		pending -= curr_step;
		if (pending == 0) {
			if (list_is_last(&t->transfer_list, &msg->transfers)) {
		  		if (!t->cs_change) {
					bcmbca_hsspi_set_cs(bs, spi->chip_select, false);					  
				}
			} else {
			  	if (t->cs_change) {
					bcmbca_hsspi_set_cs(bs, spi->chip_select, false);
					udelay(10);
					bcmbca_hsspi_set_cs(bs, spi->chip_select, true);					
				}
			}
		}
		if (rx) {
			memcpy_fromio(rx, bs->fifo, curr_step);
			rx += curr_step;
		}
	}

	return 0;
}

static int bcmbca_hsspi_setup(struct spi_device *spi)
{
	struct bcmbca_hsspi *bs = spi_master_get_devdata(spi->master);
	u32 reg;

	reg = __raw_readl(bs->regs +
			  HSSPI_PROFILE_SIGNAL_CTRL_REG(spi->chip_select));
	reg &= ~(SIGNAL_CTRL_LAUNCH_RISING | SIGNAL_CTRL_LATCH_RISING);
	if (spi->mode & SPI_CPHA)
		reg |= SIGNAL_CTRL_LAUNCH_RISING;
	else
		reg |= SIGNAL_CTRL_LATCH_RISING;
	__raw_writel(reg, bs->regs +
		     HSSPI_PROFILE_SIGNAL_CTRL_REG(spi->chip_select));

	BCMBCA_MUTEX_LOCK(&bs->bus_mutex);
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);

	if (spi->mode & SPI_CS_HIGH)
		reg |= BIT(spi->chip_select);
	else
		reg &= ~BIT(spi->chip_select);
	__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);

	if (spi->mode & SPI_CS_HIGH)
		bs->cs_polarity |= BIT(spi->chip_select);
	else
		bs->cs_polarity &= ~BIT(spi->chip_select);

	BCMBCA_MUTEX_UNLOCK(&bs->bus_mutex);

	return 0;
}

static int bcmbca_hsspi_transfer_one(struct spi_master *master,
				      struct spi_message *msg)
{
	struct bcmbca_hsspi *bs = spi_master_get_devdata(master);
	struct spi_transfer *t;
	struct spi_device *spi = msg->spi;
	int status = -EINVAL;
	bool keep_cs = false;


	list_for_each_entry(t, &msg->transfers, transfer_list) {
		status = bcmbca_hsspi_do_txrx(spi, t, msg);
		if (status)
			break;

		msg->actual_length += t->len;

		if (t->delay_usecs) {
			u16 us = t->delay_usecs;
			if (us <= 10 || oops_in_progress)
				udelay(us);
			else
				usleep_range(us, us + DIV_ROUND_UP(us, 10));
		}		
	}

	/* restore the default clk gate setting in case some
	 * spidev turn it off
	 */
	bcmbca_hsspi_set_clk_gate(bs, spi);

	msg->status = status;
	spi_finalize_current_message(master);

	return 0;
}

static irqreturn_t bcmbca_hsspi_interrupt(int irq, void *dev_id)
{
	struct bcmbca_hsspi *bs = (struct bcmbca_hsspi *)dev_id;

	if (__raw_readl(bs->regs + HSSPI_INT_STATUS_MASKED_REG) == 0)
		return IRQ_NONE;

	__raw_writel(HSSPI_INT_CLEAR_ALL, bs->regs + HSSPI_INT_STATUS_REG);
	__raw_writel(0, bs->regs + HSSPI_INT_MASK_REG);

	complete(&bs->done);

	return IRQ_HANDLED;
}

static int bcmbca_hsspi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct bcmbca_hsspi *bs;
	struct resource *res_mem;
	void __iomem *spim_ctrl;	
	void __iomem *regs;
	struct device *dev = &pdev->dev;
	struct clk *clk, *pll_clk = NULL;
	int irq, ret;
	u32 reg, rate, num_cs = HSSPI_SPI_MAX_CS;
	int polling = 0;

	res_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "hsspi");
	if (!res_mem)
		return -EINVAL;
	regs = devm_ioremap_resource(dev, res_mem);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	res_mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "spim-ctrl");	
	if (!res_mem)
		return -EINVAL;	
	spim_ctrl = devm_ioremap_resource(dev, res_mem);
	if (IS_ERR(spim_ctrl))
		return PTR_ERR(spim_ctrl);
	
	irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (!irq) {
		polling = 1;
	}

	clk = devm_clk_get(dev, "hsspi");

	if (IS_ERR(clk))
		return PTR_ERR(clk);

	ret = clk_prepare_enable(clk);
	if (ret)
		return ret;

	rate = clk_get_rate(clk);
	if (!rate) {
		pll_clk = devm_clk_get(dev, "pll");

		if (IS_ERR(pll_clk)) {
			ret = PTR_ERR(pll_clk);
			goto out_disable_clk;
		}

		ret = clk_prepare_enable(pll_clk);
		if (ret)
			goto out_disable_clk;

		rate = clk_get_rate(pll_clk);
		if (!rate) {
			ret = -EINVAL;
			goto out_disable_pll_clk;
		}
	}

	master = spi_alloc_master(&pdev->dev, sizeof(*bs));
	if (!master) {
		ret = -ENOMEM;
		goto out_disable_pll_clk;
	}

	bs = spi_master_get_devdata(master);
	bs->pdev = pdev;
	bs->clk = clk;
	bs->pll_clk = pll_clk;
	bs->regs = regs;
	bs->spim_ctrl = spim_ctrl;	
	bs->speed_hz = rate;
	bs->fifo = (u8 __iomem *) (bs->regs + HSSPI_FIFO_REG(0));
	bs->polling = polling;

	mutex_init(&bs->bus_mutex);
	init_completion(&bs->done);

	master->dev.of_node = dev->of_node;
	if (!dev->of_node)
		master->bus_num = HSSPI_BUS_NUM;

	of_property_read_u32(dev->of_node, "num-cs", &num_cs);
	if (num_cs > 8) {
		dev_warn(dev, "unsupported number of cs (%i), reducing to 8\n",
			 num_cs);
		num_cs = HSSPI_SPI_MAX_CS;
	}
	master->num_chipselect = num_cs;
	master->setup = bcmbca_hsspi_setup;
	master->transfer_one_message = bcmbca_hsspi_transfer_one;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH |
	    SPI_RX_DUAL | SPI_TX_DUAL;
	master->bits_per_word_mask = SPI_BPW_MASK(8);
	master->auto_runtime_pm = true;

	platform_set_drvdata(pdev, master);

	/* Initialize the hardware */
	__raw_writel(0, bs->regs + HSSPI_INT_MASK_REG);

	/* clean up any pending interrupts */
	__raw_writel(HSSPI_INT_CLEAR_ALL, bs->regs + HSSPI_INT_STATUS_REG);

	/* read out default CS polarities */
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
	bs->cs_polarity = reg & GLOBAL_CTRL_CS_POLARITY_MASK;
	__raw_writel(reg | GLOBAL_CTRL_CLK_GATE_SSOFF,
		     bs->regs + HSSPI_GLOBAL_CTRL_REG);

	if (bs->polling == 0) {
		ret = devm_request_irq(dev, irq, bcmbca_hsspi_interrupt, IRQF_SHARED,
			       pdev->name, bs);
		if (ret)
			goto out_put_master;
	}
	/* register and we are done */
	ret = devm_spi_register_master(dev, master);
	if (ret)
		goto out_put_master;

	dev_info(dev, "Broadcom BCMBCA High Speed SPI Controller driver");
	if (bs->polling)
		dev_info(dev, "spi driver using polling mode");

	return 0;

out_put_master:
	spi_master_put(master);
out_disable_pll_clk:
	clk_disable_unprepare(pll_clk);
out_disable_clk:
	clk_disable_unprepare(clk);
	return ret;
}

static int bcmbca_hsspi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct bcmbca_hsspi *bs = spi_master_get_devdata(master);

	/* reset the hardware and block queue progress */
	__raw_writel(0, bs->regs + HSSPI_INT_MASK_REG);
	clk_disable_unprepare(bs->pll_clk);
	clk_disable_unprepare(bs->clk);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int bcmbca_hsspi_suspend(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	struct bcmbca_hsspi *bs = spi_master_get_devdata(master);

	spi_master_suspend(master);
	clk_disable_unprepare(bs->pll_clk);
	clk_disable_unprepare(bs->clk);

	return 0;
}

static int bcmbca_hsspi_resume(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	struct bcmbca_hsspi *bs = spi_master_get_devdata(master);
	int ret;

	ret = clk_prepare_enable(bs->clk);
	if (ret)
		return ret;

	if (bs->pll_clk) {
		ret = clk_prepare_enable(bs->pll_clk);
		if (ret)
			return ret;
	}

	spi_master_resume(master);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(bcmbca_hsspi_pm_ops, bcmbca_hsspi_suspend,
			 bcmbca_hsspi_resume);

static const struct of_device_id bcmbca_hsspi_of_match[] = {
	{.compatible = "brcm,bcmbca-hsspi",},
	{},
};

MODULE_DEVICE_TABLE(of, bcmbca_hsspi_of_match);

static struct platform_driver bcmbca_hsspi_driver = {
	.driver = {
		   .name = "bcmbca-hsspi",
		   .pm = &bcmbca_hsspi_pm_ops,
		   .of_match_table = bcmbca_hsspi_of_match,
		   },
	.probe = bcmbca_hsspi_probe,
	.remove = bcmbca_hsspi_remove,
};

module_platform_driver(bcmbca_hsspi_driver);

MODULE_ALIAS("platform:bcmbca_hsspi");
MODULE_DESCRIPTION("Broadcom BCMBCA High Speed SPI Controller driver");
MODULE_LICENSE("GPL");

#endif
