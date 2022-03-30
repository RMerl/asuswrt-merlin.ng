/*
 * Broadcom BCM63XX High Speed SPI Controller driver
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
#if defined(CONFIG_BCM_KF_SPI)
#include <linux/of_address.h>
#include <linux/of_irq.h>

/* Broadcom Legacy SPI device driver flags */
#define SPIDEV_CONTROLLER_STATE_SET		BIT(31)
#define SPIDEV_CONTROLLER_STATE_GATE_CLK_SSOFF	BIT(29)

#define spidev_ctrl_data(spi)			\
	((u32)((uintptr_t)(spi)->controller_data))
#endif

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
#ifdef CONFIG_BCM_KF_SPI
#define HSSPI_PINGPONG_STATUS_SRC_BUSY          BIT(1)
#endif

#define HSSPI_PROFILE_CLK_CTRL_REG(x)		(0x100 + (x) * 0x20)
#define CLK_CTRL_FREQ_CTRL_MASK			0x0000ffff
#define CLK_CTRL_SPI_CLK_2X_SEL			BIT(14)
#define CLK_CTRL_ACCUM_RST_ON_LOOP		BIT(15)

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

struct bcm63xx_hsspi {
	struct completion done;
	struct mutex bus_mutex;

	struct platform_device *pdev;
	struct clk *clk;
	struct clk *pll_clk;
	void __iomem *regs;
	u8 __iomem *fifo;

	u32 speed_hz;
	u8 cs_polarity;
#if defined(CONFIG_BCM_KF_SPI)
	int use_cswar;
	int polling;
	u32 prepend_cnt;
	u8 *prepend_buf;
#endif
};

#if defined(CONFIG_BCM_KF_SPI)

static void bcm63xx_hsspi_set_clk(struct bcm63xx_hsspi *bs,
				  struct spi_device *spi, int hz);

static inline int bcm63xx_hsspi_dev_no_clk_gate(struct spi_device *spi)
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

static size_t bcm63xx_hsspi_max_message_size(struct spi_device *spi)
{
	return (HSSPI_BUFFER_LEN - HSSPI_OPCODE_LEN);
}

static void bcm63xx_hsspi_set_clk_gate(struct bcm63xx_hsspi *bs,
				       struct spi_device *spi)
{
	u32 reg = 0;

	if (bcm63xx_hsspi_dev_no_clk_gate(spi)) {
		mutex_lock(&bs->bus_mutex);
		reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
		reg |= GLOBAL_CTRL_CLK_GATE_SSOFF;
		__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
		mutex_unlock(&bs->bus_mutex);
	}
}

static bool bcm63xx_check_msg_prependable(struct spi_master *master,
					  struct spi_message *msg,
					  struct spi_transfer *t_prepend)
{

	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);
	bool prepend = false, tx_only = false;
	struct spi_transfer *t;

	/* If cs dummy workaround used, no need to prepend message */
	if (bs->use_cswar)
		goto check_done;

	/*
	 * message must only contain n half duplex write transfer + optional
	 * full duplex read/write at the end. There must be no udelay between
	 * transfers and no cs_change request
	 */
	bs->prepend_cnt = 0;
	list_for_each_entry(t, &msg->transfers, transfer_list) {
		if (t->delay_usecs || t->cs_change) {
			dev_warn(&bs->pdev->dev,
				 "prepend does not support delay or cs change between transfers!\n");
			break;
		}

		tx_only = false;
		if (t->tx_buf && !t->rx_buf) {
			tx_only = true;
			if (bs->prepend_cnt + t->len >
			    (HSSPI_BUFFER_LEN - HSSPI_OPCODE_LEN)) {
				dev_warn(&bs->pdev->dev,
					 "exceed max prepend count abort prepending transfers!\n");
				break;
			}
			memcpy(bs->prepend_buf + bs->prepend_cnt, t->tx_buf,
			       t->len);
			bs->prepend_cnt += t->len;
		} else {
			if (!list_is_last(&t->transfer_list, &msg->transfers)) {
				dev_warn(&bs->pdev->dev,
					 "can not prepend message when rx/tx_rx transfer is not the last transfer!\n");
				break;
			}
		}

		if (list_is_last(&t->transfer_list, &msg->transfers)) {
			memcpy(t_prepend, t, sizeof(struct spi_transfer));
			/* if the last is also a tx only transfer, merge all
			 * them into one single tx transfer
			 */
			if (tx_only) {
				t_prepend->len = bs->prepend_cnt;
				t_prepend->tx_buf = bs->prepend_buf;
				bs->prepend_cnt = 0;
			}
			prepend = true;
		}
	}

check_done:
	if (!bs->use_cswar && !prepend)
		dev_warn(&bs->pdev->dev,
			 "SPI message not prependable and cs workaround not used. SPI transfer may fail!\n");
	return prepend;
}

static int bcm63xx_hsspi_do_prepend_txrx(struct spi_device *spi,
					 struct spi_transfer *t)
{
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(spi->master);
	unsigned int chip_select = spi->chip_select;
	u16 opcode = 0;
	const u8 *tx = t->tx_buf;
	u8 *rx = t->rx_buf;
	u32 reg = 0;

	/* shouldnt happen as we set the max_message_size in the probe.
	 * but check it again in case some driver does not honor the max size
	 */
	if (t->len + bs->prepend_cnt > (HSSPI_BUFFER_LEN - HSSPI_OPCODE_LEN)) {
		dev_warn(&bs->pdev->dev,
			 "Prepend message large than fifo size len %d prepend %d\n",
			 t->len, bs->prepend_cnt);
		return -EINVAL;
	}

	bcm63xx_hsspi_set_clk(bs, spi, t->speed_hz);

	if (tx && rx)
		opcode = HSSPI_OP_READ_WRITE;
	else if (tx)
		opcode = HSSPI_OP_WRITE;
	else if (rx)
		opcode = HSSPI_OP_READ;

	if ((opcode == HSSPI_OP_READ && t->rx_nbits == SPI_NBITS_DUAL) ||
	    (opcode == HSSPI_OP_WRITE && t->tx_nbits == SPI_NBITS_DUAL)) {
		opcode |= HSSPI_OP_MULTIBIT;

		if (t->rx_nbits == SPI_NBITS_DUAL)
			reg |= 1 << MODE_CTRL_MULTIDATA_RD_SIZE_SHIFT;
		if (t->tx_nbits == SPI_NBITS_DUAL)
			reg |= 1 << MODE_CTRL_MULTIDATA_WR_SIZE_SHIFT;
	}

	reg |= bs->prepend_cnt << MODE_CTRL_PREPENDBYTE_CNT_SHIFT;
	__raw_writel(reg | 0xff,
		     bs->regs + HSSPI_PROFILE_MODE_CTRL_REG(chip_select));

	reinit_completion(&bs->done);
	if (bs->prepend_cnt)
		memcpy_toio(bs->fifo + HSSPI_OPCODE_LEN, bs->prepend_buf,
			    bs->prepend_cnt);
	if (tx)
		memcpy_toio(bs->fifo + HSSPI_OPCODE_LEN + bs->prepend_cnt, tx,
			    t->len);

	__raw_writew(cpu_to_be16(opcode | t->len), bs->fifo);
	/* enable interrupt */
	__raw_writel(HSSPI_PINGx_CMD_DONE(0), bs->regs + HSSPI_INT_MASK_REG);

	/* start the transfer */
	reg = chip_select << PINGPONG_CMD_SS_SHIFT |
	    chip_select << PINGPONG_CMD_PROFILE_SHIFT |
	    PINGPONG_COMMAND_START_NOW;
	__raw_writel(reg, bs->regs + HSSPI_PINGPONG_COMMAND_REG(0));

	if (!oops_in_progress && !bs->polling) {
		if (wait_for_completion_timeout(&bs->done, HZ) == 0) {
			dev_err(&bs->pdev->dev, "transfer timed out!\n");
			return -ETIMEDOUT;
		}
	} else {
		while (__raw_readl(bs->regs + HSSPI_PINGPONG_STATUS_REG(0)) &
		       HSSPI_PINGPONG_STATUS_SRC_BUSY)
			cpu_relax();
	}

	if (rx)
		memcpy_fromio(rx, bs->fifo, t->len);

	return 0;

}
#endif

static void bcm63xx_hsspi_set_cs(struct bcm63xx_hsspi *bs, unsigned int cs,
				 bool active)
{
	u32 reg;

	mutex_lock(&bs->bus_mutex);
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);

	reg &= ~BIT(cs);
	if (active == !(bs->cs_polarity & BIT(cs)))
		reg |= BIT(cs);

	__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
	mutex_unlock(&bs->bus_mutex);
}

static void bcm63xx_hsspi_set_clk(struct bcm63xx_hsspi *bs,
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

	mutex_lock(&bs->bus_mutex);
	/* setup clock polarity */
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
	reg &= ~GLOBAL_CTRL_CLK_POLARITY;
	if (spi->mode & SPI_CPOL)
		reg |= GLOBAL_CTRL_CLK_POLARITY;

#if defined(CONFIG_BCM_KF_SPI)
	if (bcm63xx_hsspi_dev_no_clk_gate(spi))
		reg &= ~GLOBAL_CTRL_CLK_GATE_SSOFF;
	else
		reg |= GLOBAL_CTRL_CLK_GATE_SSOFF;
#endif
	__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
	mutex_unlock(&bs->bus_mutex);
}

static int bcm63xx_hsspi_do_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(spi->master);
	unsigned int chip_select = spi->chip_select;
	u16 opcode = 0;
	int pending = t->len;
	int step_size = HSSPI_BUFFER_LEN;
	const u8 *tx = t->tx_buf;
	u8 *rx = t->rx_buf;
#if defined(CONFIG_BCM_KF_SPI)
	u32 reg = 0;
#endif

	bcm63xx_hsspi_set_clk(bs, spi, t->speed_hz);

#if defined(CONFIG_BCM_KF_SPI)
	if (bs->use_cswar)
#endif
		bcm63xx_hsspi_set_cs(bs, spi->chip_select, true);

	if (tx && rx)
		opcode = HSSPI_OP_READ_WRITE;
	else if (tx)
		opcode = HSSPI_OP_WRITE;
	else if (rx)
		opcode = HSSPI_OP_READ;

	if (opcode != HSSPI_OP_READ)
		step_size -= HSSPI_OPCODE_LEN;

#if defined(CONFIG_BCM_KF_SPI)
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
#else
	if ((opcode == HSSPI_OP_READ && t->rx_nbits == SPI_NBITS_DUAL) ||
	    (opcode == HSSPI_OP_WRITE && t->tx_nbits == SPI_NBITS_DUAL))
		opcode |= HSSPI_OP_MULTIBIT;

	__raw_writel(1 << MODE_CTRL_MULTIDATA_WR_SIZE_SHIFT |
		     1 << MODE_CTRL_MULTIDATA_RD_SIZE_SHIFT | 0xff,
		     bs->regs + HSSPI_PROFILE_MODE_CTRL_REG(chip_select));
#endif

	while (pending > 0) {
		int curr_step = min_t(int, step_size, pending);

		reinit_completion(&bs->done);
		if (tx) {
			memcpy_toio(bs->fifo + HSSPI_OPCODE_LEN, tx, curr_step);
			tx += curr_step;
		}
#if defined(CONFIG_BCM_KF_SPI)
		__raw_writew(cpu_to_be16(opcode | curr_step), bs->fifo);
#else
		__raw_writew(opcode | curr_step, bs->fifo);
#endif

		/* enable interrupt */
		__raw_writel(HSSPI_PINGx_CMD_DONE(0),
			     bs->regs + HSSPI_INT_MASK_REG);
#if defined(CONFIG_BCM_KF_SPI)
		/* start the transfer */
		if (!bs->use_cswar)
			reg = chip_select << PINGPONG_CMD_SS_SHIFT |
			    chip_select << PINGPONG_CMD_PROFILE_SHIFT |
			    PINGPONG_COMMAND_START_NOW;
		else
			reg = !chip_select << PINGPONG_CMD_SS_SHIFT |
			    chip_select << PINGPONG_CMD_PROFILE_SHIFT |
			    PINGPONG_COMMAND_START_NOW;
		__raw_writel(reg, bs->regs + HSSPI_PINGPONG_COMMAND_REG(0));
#else
		/* start the transfer */
		__raw_writel(!chip_select << PINGPONG_CMD_SS_SHIFT |
			     chip_select << PINGPONG_CMD_PROFILE_SHIFT |
			     PINGPONG_COMMAND_START_NOW,
			     bs->regs + HSSPI_PINGPONG_COMMAND_REG(0));
#endif

#ifdef CONFIG_BCM_KF_SPI
		if (!oops_in_progress && !bs->polling) {
#endif
			if (wait_for_completion_timeout(&bs->done, HZ) == 0) {
				dev_err(&bs->pdev->dev, "transfer timed out!\n");
				return -ETIMEDOUT;
			}
#ifdef CONFIG_BCM_KF_SPI
		} else {
			while (__raw_readl(bs->regs +
			       HSSPI_PINGPONG_STATUS_REG(0)) &
			       HSSPI_PINGPONG_STATUS_SRC_BUSY)
				cpu_relax();
		}
#endif
		if (rx) {
			memcpy_fromio(rx, bs->fifo, curr_step);
			rx += curr_step;
		}

		pending -= curr_step;
	}

	return 0;
}

static int bcm63xx_hsspi_setup(struct spi_device *spi)
{
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(spi->master);
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

	mutex_lock(&bs->bus_mutex);
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);

	/* only change actual polarities if there is no transfer */
	if ((reg & GLOBAL_CTRL_CS_POLARITY_MASK) == bs->cs_polarity) {
		if (spi->mode & SPI_CS_HIGH)
			reg |= BIT(spi->chip_select);
		else
			reg &= ~BIT(spi->chip_select);
		__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
	}

	if (spi->mode & SPI_CS_HIGH)
		bs->cs_polarity |= BIT(spi->chip_select);
	else
		bs->cs_polarity &= ~BIT(spi->chip_select);

	mutex_unlock(&bs->bus_mutex);

	return 0;
}

static int bcm63xx_hsspi_transfer_one(struct spi_master *master,
				      struct spi_message *msg)
{
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);
	struct spi_transfer *t;
	struct spi_device *spi = msg->spi;
	int status = -EINVAL;
	int dummy_cs;
	u32 reg;
#if defined(CONFIG_BCM_KF_SPI)
	bool restore_polarity = true;
	bool prepend = false;
	struct spi_transfer t_prepend;
#endif

#if defined(CONFIG_BCM_KF_SPI)
	prepend = bcm63xx_check_msg_prependable(master, msg, &t_prepend);
	if (prepend) {
		status = bcm63xx_hsspi_do_prepend_txrx(spi, &t_prepend);
		msg->actual_length += (t_prepend.len + bs->prepend_cnt);
		bcm63xx_hsspi_set_clk_gate(bs, spi);
		goto msg_done;
	}
#endif

	/* This controller does not support keeping CS active during idle.
	 * To work around this, we use the following ugly hack:
	 *
	 * a. Invert the target chip select's polarity so it will be active.
	 * b. Select a "dummy" chip select to use as the hardware target.
	 * c. Invert the dummy chip select's polarity so it will be inactive
	 *    during the actual transfers.
	 * d. Tell the hardware to send to the dummy chip select. Thanks to
	 *    the multiplexed nature of SPI the actual target will receive
	 *    the transfer and we see its response.
	 *
	 * e. At the end restore the polarities again to their default values.
	 */

#if defined(CONFIG_BCM_KF_SPI)
	if (bs->use_cswar) {
#endif
		dummy_cs = !spi->chip_select;
		bcm63xx_hsspi_set_cs(bs, dummy_cs, true);
#if defined(CONFIG_BCM_KF_SPI)
	}
#endif

	list_for_each_entry(t, &msg->transfers, transfer_list) {
		status = bcm63xx_hsspi_do_txrx(spi, t);
		if (status)
			break;

		msg->actual_length += t->len;

		if (t->delay_usecs)
			udelay(t->delay_usecs);

#if defined(CONFIG_BCM_KF_SPI)
		/*
		 * cs_change rules:
		 * (1) cs_change = 0 && last_xfer = 0:
		 *      Do not touch the CS. On to the next xfer.
		 * (2) cs_change = 1 && last_xfer = 0:
		 *      Set cs = false before the next xfer.
		 * (3) cs_change = 0 && last_xfer = 1:
		 *      We want CS to be deactivated. So do NOT set cs = false,
		 *      instead just restore the original polarity. This has the
		 *      same effect of deactivating the CS.
		 * (4) cs_change = 1 && last_xfer = 1:
		 *      We want to keep CS active. So do NOT set cs = false, and
		 *      make sure we do NOT reverse polarity.
		 */
		if (t->cs_change
		    && !list_is_last(&t->transfer_list, &msg->transfers))
#else
		if (t->cs_change)
#endif
			bcm63xx_hsspi_set_cs(bs, spi->chip_select, false);
#if defined(CONFIG_BCM_KF_SPI)

		restore_polarity = !t->cs_change;
#endif
	}

#if defined(CONFIG_BCM_KF_SPI)
	if (restore_polarity && bs->use_cswar) {
		mutex_lock(&bs->bus_mutex);
		reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
		reg &= ~GLOBAL_CTRL_CS_POLARITY_MASK;
		reg |= bs->cs_polarity;
		__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
		mutex_unlock(&bs->bus_mutex);
	}

	/* restore the default clk gate setting in case some
	 * spidev turn it off
	 */
	bcm63xx_hsspi_set_clk_gate(bs, spi);

msg_done:
#else
	mutex_lock(&bs->bus_mutex);
	reg = __raw_readl(bs->regs + HSSPI_GLOBAL_CTRL_REG);
	reg &= ~GLOBAL_CTRL_CS_POLARITY_MASK;
	reg |= bs->cs_polarity;
	__raw_writel(reg, bs->regs + HSSPI_GLOBAL_CTRL_REG);
	mutex_unlock(&bs->bus_mutex);
#endif

	msg->status = status;
	spi_finalize_current_message(master);

	return 0;
}

static irqreturn_t bcm63xx_hsspi_interrupt(int irq, void *dev_id)
{
	struct bcm63xx_hsspi *bs = (struct bcm63xx_hsspi *)dev_id;

	if (__raw_readl(bs->regs + HSSPI_INT_STATUS_MASKED_REG) == 0)
		return IRQ_NONE;

	__raw_writel(HSSPI_INT_CLEAR_ALL, bs->regs + HSSPI_INT_STATUS_REG);
	__raw_writel(0, bs->regs + HSSPI_INT_MASK_REG);

	complete(&bs->done);

	return IRQ_HANDLED;
}

static int bcm63xx_hsspi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct bcm63xx_hsspi *bs;
#if defined(CONFIG_BCM_KF_SPI)
	struct resource res_mem;
#else
	struct resource *res_mem;
#endif
	void __iomem *regs;
	struct device *dev = &pdev->dev;
	struct clk *clk, *pll_clk = NULL;
	int irq, ret;
	u32 reg, rate, num_cs = HSSPI_SPI_MAX_CS;
#if defined(CONFIG_BCM_KF_SPI)
	int polling = 0;

	irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
#else
	irq = platform_get_irq(pdev, 0);
#endif

#if defined(CONFIG_BCM_KF_SPI)
	if (!irq) {
		/*
		 * switch to polling if intr is not defined and
		 * for better throughput as well
		 */
		dev_err(dev, "spi driver using polling mode\n");
		polling = 1;
	}
#else
	if (irq < 0) {
		dev_err(dev, "no irq: %d\n", irq);
		return irq;
	}
#endif

#if defined(CONFIG_BCM_KF_SPI)
	ret = of_address_to_resource(pdev->dev.of_node, 0, &res_mem);
	if (ret)
		return -EINVAL;

	regs = devm_ioremap_resource(dev, &res_mem);
#else
	res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	regs = devm_ioremap_resource(dev, res_mem);
#endif
	if (IS_ERR(regs))
		return PTR_ERR(regs);

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
	bs->speed_hz = rate;
	bs->fifo = (u8 __iomem *) (bs->regs + HSSPI_FIFO_REG(0));
#if defined(CONFIG_BCM_KF_SPI)
	bs->polling = polling;
	bs->prepend_buf = kmalloc(HSSPI_BUFFER_LEN, GFP_KERNEL);
	if (!bs->prepend_buf) {
		ret = -ENOMEM;
		goto out_put_master;
	}

	/* check if dummy cs workaround is needed */
	if (of_property_read_u32
	    (dev->of_node, "use_cs_workaround", &bs->use_cswar))
		bs->use_cswar = 0;
#endif

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
	master->setup = bcm63xx_hsspi_setup;
	master->transfer_one_message = bcm63xx_hsspi_transfer_one;
#if defined(CONFIG_BCM_KF_SPI)
	if (bs->use_cswar == 0) {
		master->max_transfer_size = bcm63xx_hsspi_max_message_size;
		master->max_message_size = bcm63xx_hsspi_max_message_size;
	}
#endif
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

#if defined(CONFIG_BCM_KF_SPI)
	if (bs->polling == 0) {
#endif
	ret = devm_request_irq(dev, irq, bcm63xx_hsspi_interrupt, IRQF_SHARED,
			       pdev->name, bs);

	if (ret)
		goto out_put_master;
#if defined(CONFIG_BCM_KF_SPI)
	}
#endif
	/* register and we are done */
	ret = devm_spi_register_master(dev, master);
	if (ret)
		goto out_put_master;

	return 0;

out_put_master:
	spi_master_put(master);
out_disable_pll_clk:
	clk_disable_unprepare(pll_clk);
out_disable_clk:
	clk_disable_unprepare(clk);
	return ret;
}

static int bcm63xx_hsspi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);

	/* reset the hardware and block queue progress */
	__raw_writel(0, bs->regs + HSSPI_INT_MASK_REG);
	clk_disable_unprepare(bs->pll_clk);
	clk_disable_unprepare(bs->clk);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int bcm63xx_hsspi_suspend(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);

	spi_master_suspend(master);
	clk_disable_unprepare(bs->pll_clk);
	clk_disable_unprepare(bs->clk);

	return 0;
}

static int bcm63xx_hsspi_resume(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	struct bcm63xx_hsspi *bs = spi_master_get_devdata(master);
	int ret;

	ret = clk_prepare_enable(bs->clk);
	if (ret)
		return ret;

	if (bs->pll_clk) {
		ret = clk_prepare_enable(bs->pll_clk);
		if (ret) {
			clk_disable_unprepare(bs->clk);
			return ret;
		}
	}

	spi_master_resume(master);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(bcm63xx_hsspi_pm_ops, bcm63xx_hsspi_suspend,
			 bcm63xx_hsspi_resume);

static const struct of_device_id bcm63xx_hsspi_of_match[] = {
	{.compatible = "brcm,bcm6328-hsspi",},
	{},
};

MODULE_DEVICE_TABLE(of, bcm63xx_hsspi_of_match);

static struct platform_driver bcm63xx_hsspi_driver = {
	.driver = {
		   .name = "bcm63xx-hsspi",
		   .pm = &bcm63xx_hsspi_pm_ops,
		   .of_match_table = bcm63xx_hsspi_of_match,
		   },
	.probe = bcm63xx_hsspi_probe,
	.remove = bcm63xx_hsspi_remove,
};

module_platform_driver(bcm63xx_hsspi_driver);

MODULE_ALIAS("platform:bcm63xx_hsspi");
MODULE_DESCRIPTION("Broadcom BCM63xx High Speed SPI Controller driver");
MODULE_AUTHOR("Jonas Gorski <jogo@openwrt.org>");
MODULE_LICENSE("GPL");
