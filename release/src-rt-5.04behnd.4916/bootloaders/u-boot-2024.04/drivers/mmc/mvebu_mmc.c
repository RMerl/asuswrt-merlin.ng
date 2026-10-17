// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell MMC/SD/SDIO driver
 *
 * (C) Copyright 2012-2014
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Maen Suleiman, Gerald Kerma
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <dm.h>
#include <fdtdec.h>
#include <part.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <mvebu_mmc.h>
#include <dm/device_compat.h>

#define MVEBU_TARGET_DRAM 0

#define TIMEOUT_DELAY	5*CONFIG_SYS_HZ		/* wait 5 seconds */

static inline void *get_regbase(const struct mmc *mmc)
{
	struct mvebu_mmc_plat *pdata = mmc->priv;

	return pdata->iobase;
}

static void mvebu_mmc_write(const struct mmc *mmc, u32 offs, u32 val)
{
	writel(val, get_regbase(mmc) + (offs));
}

static u32 mvebu_mmc_read(const struct mmc *mmc, u32 offs)
{
	return readl(get_regbase(mmc) + (offs));
}

static int mvebu_mmc_setup_data(struct udevice *dev, struct mmc_data *data)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;
	u32 ctrl_reg;

	dev_dbg(dev, "data %s : blocks=%d blksz=%d\n",
		(data->flags & MMC_DATA_READ) ? "read" : "write",
		data->blocks, data->blocksize);

	/* default to maximum timeout */
	ctrl_reg = mvebu_mmc_read(mmc, SDIO_HOST_CTRL);
	ctrl_reg |= SDIO_HOST_CTRL_TMOUT(SDIO_HOST_CTRL_TMOUT_MAX);
	mvebu_mmc_write(mmc, SDIO_HOST_CTRL, ctrl_reg);

	if (data->flags & MMC_DATA_READ) {
		mvebu_mmc_write(mmc, SDIO_SYS_ADDR_LOW, (u32)data->dest & 0xffff);
		mvebu_mmc_write(mmc, SDIO_SYS_ADDR_HI, (u32)data->dest >> 16);
	} else {
		mvebu_mmc_write(mmc, SDIO_SYS_ADDR_LOW, (u32)data->src & 0xffff);
		mvebu_mmc_write(mmc, SDIO_SYS_ADDR_HI, (u32)data->src >> 16);
	}

	mvebu_mmc_write(mmc, SDIO_BLK_COUNT, data->blocks);
	mvebu_mmc_write(mmc, SDIO_BLK_SIZE, data->blocksize);

	return 0;
}

static int mvebu_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	ulong start;
	ushort waittype = 0;
	ushort resptype = 0;
	ushort xfertype = 0;
	ushort resp_indx = 0;
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;

	dev_dbg(dev, "cmdidx [0x%x] resp_type[0x%x] cmdarg[0x%x]\n",
		cmd->cmdidx, cmd->resp_type, cmd->cmdarg);

	dev_dbg(dev, "cmd %d (hw state 0x%04x)\n",
		cmd->cmdidx, mvebu_mmc_read(mmc, SDIO_HW_STATE));

	/*
	 * Hardware weirdness.  The FIFO_EMPTY bit of the HW_STATE
	 * register is sometimes not set before a while when some
	 * "unusual" data block sizes are used (such as with the SWITCH
	 * command), even despite the fact that the XFER_DONE interrupt
	 * was raised.  And if another data transfer starts before
	 * this bit comes to good sense (which eventually happens by
	 * itself) then the new transfer simply fails with a timeout.
	 */
	if (!(mvebu_mmc_read(mmc, SDIO_HW_STATE) & CMD_FIFO_EMPTY)) {
		ushort hw_state, count = 0;

		start = get_timer(0);
		do {
			hw_state = mvebu_mmc_read(mmc, SDIO_HW_STATE);
			if ((get_timer(0) - start) > TIMEOUT_DELAY) {
				printf("%s : FIFO_EMPTY bit missing\n",
				       dev->name);
				break;
			}
			count++;
		} while (!(hw_state & CMD_FIFO_EMPTY));
		dev_dbg(dev, "*** wait for FIFO_EMPTY bit (hw=0x%04x, count=%d, jiffies=%ld)\n",
			hw_state, count, (get_timer(0) - (start)));
	}

	/* Clear status */
	mvebu_mmc_write(mmc, SDIO_NOR_INTR_STATUS, SDIO_POLL_MASK);
	mvebu_mmc_write(mmc, SDIO_ERR_INTR_STATUS, SDIO_POLL_MASK);

	resptype = SDIO_CMD_INDEX(cmd->cmdidx);

	/* Analyzing resptype/xfertype/waittype for the command */
	if (cmd->resp_type & MMC_RSP_BUSY)
		resptype |= SDIO_CMD_RSP_48BUSY;
	else if (cmd->resp_type & MMC_RSP_136)
		resptype |= SDIO_CMD_RSP_136;
	else if (cmd->resp_type & MMC_RSP_PRESENT)
		resptype |= SDIO_CMD_RSP_48;
	else
		resptype |= SDIO_CMD_RSP_NONE;

	if (cmd->resp_type & MMC_RSP_CRC)
		resptype |= SDIO_CMD_CHECK_CMDCRC;

	if (cmd->resp_type & MMC_RSP_OPCODE)
		resptype |= SDIO_CMD_INDX_CHECK;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		resptype |= SDIO_UNEXPECTED_RESP;
		waittype |= SDIO_NOR_UNEXP_RSP;
	}

	if (data) {
		int err = mvebu_mmc_setup_data(dev, data);

		if (err) {
			dev_dbg(dev, "command DATA error :%x\n", err);
			return err;
		}

		resptype |= SDIO_CMD_DATA_PRESENT | SDIO_CMD_CHECK_DATACRC16;
		xfertype |= SDIO_XFER_MODE_HW_WR_DATA_EN;
		if (data->flags & MMC_DATA_READ) {
			xfertype |= SDIO_XFER_MODE_TO_HOST;
			waittype = SDIO_NOR_DMA_INI;
		} else {
			waittype |= SDIO_NOR_XFER_DONE;
		}
	} else {
		waittype |= SDIO_NOR_CMD_DONE;
	}

	/* Setting cmd arguments */
	mvebu_mmc_write(mmc, SDIO_ARG_LOW, cmd->cmdarg & 0xffff);
	mvebu_mmc_write(mmc, SDIO_ARG_HI, cmd->cmdarg >> 16);

	/* Setting Xfer mode */
	mvebu_mmc_write(mmc, SDIO_XFER_MODE, xfertype);

	/* Sending command */
	mvebu_mmc_write(mmc, SDIO_CMD, resptype);

	start = get_timer(0);

	while (!((mvebu_mmc_read(mmc, SDIO_NOR_INTR_STATUS)) & waittype)) {
		if (mvebu_mmc_read(mmc, SDIO_NOR_INTR_STATUS) & SDIO_NOR_ERROR) {
			dev_dbg(dev, "error! cmdidx : %d, err reg: %04x\n",
				cmd->cmdidx,
				mvebu_mmc_read(mmc, SDIO_ERR_INTR_STATUS));
			if (mvebu_mmc_read(mmc, SDIO_ERR_INTR_STATUS) &
			    (SDIO_ERR_CMD_TIMEOUT | SDIO_ERR_DATA_TIMEOUT)) {
				dev_dbg(dev, "command READ timed out\n");
				return -ETIMEDOUT;
			}
			dev_dbg(dev, "command READ error\n");
			return -ECOMM;
		}

		if ((get_timer(0) - start) > TIMEOUT_DELAY) {
			dev_dbg(dev, "command timed out\n");
			return -ETIMEDOUT;
		}
	}

	/* Handling response */
	if (cmd->resp_type & MMC_RSP_136) {
		uint response[8];

		for (resp_indx = 0; resp_indx < 8; resp_indx++)
			response[resp_indx] = mvebu_mmc_read(mmc, SDIO_RSP(resp_indx));

		cmd->response[0] =	((response[0] & 0x03ff) << 22) |
					((response[1] & 0xffff) << 6) |
					((response[2] & 0xfc00) >> 10);
		cmd->response[1] =	((response[2] & 0x03ff) << 22) |
					((response[3] & 0xffff) << 6) |
					((response[4] & 0xfc00) >> 10);
		cmd->response[2] =	((response[4] & 0x03ff) << 22) |
					((response[5] & 0xffff) << 6) |
					((response[6] & 0xfc00) >> 10);
		cmd->response[3] =	((response[6] & 0x03ff) << 22) |
					((response[7] & 0x3fff) << 8);
	} else if (cmd->resp_type & MMC_RSP_PRESENT) {
		uint response[3];

		for (resp_indx = 0; resp_indx < 3; resp_indx++)
			response[resp_indx] = mvebu_mmc_read(mmc, SDIO_RSP(resp_indx));

		cmd->response[0] =	((response[2] & 0x003f) << (8 - 8)) |
					((response[1] & 0xffff) << (14 - 8)) |
					((response[0] & 0x03ff) << (30 - 8));
		cmd->response[1] =	((response[0] & 0xfc00) >> 10);
		cmd->response[2] =	0;
		cmd->response[3] =	0;
	} else {
		cmd->response[0] =	0;
		cmd->response[1] =	0;
		cmd->response[2] =	0;
		cmd->response[3] =	0;
	}

	dev_dbg(dev, "resp[0x%x] ", cmd->resp_type);
	debug("[0x%x] ", cmd->response[0]);
	debug("[0x%x] ", cmd->response[1]);
	debug("[0x%x] ", cmd->response[2]);
	debug("[0x%x] ", cmd->response[3]);
	debug("\n");

	if (mvebu_mmc_read(mmc, SDIO_ERR_INTR_STATUS) &
		(SDIO_ERR_CMD_TIMEOUT | SDIO_ERR_DATA_TIMEOUT))
		return -ETIMEDOUT;

	return 0;
}

static void mvebu_mmc_power_up(struct udevice *dev)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;

	dev_dbg(dev, "power up\n");

	/* disable interrupts */
	mvebu_mmc_write(mmc, SDIO_NOR_INTR_EN, 0);
	mvebu_mmc_write(mmc, SDIO_ERR_INTR_EN, 0);

	/* SW reset */
	mvebu_mmc_write(mmc, SDIO_SW_RESET, SDIO_SW_RESET_NOW);

	mvebu_mmc_write(mmc, SDIO_XFER_MODE, 0);

	/* enable status */
	mvebu_mmc_write(mmc, SDIO_NOR_STATUS_EN, SDIO_POLL_MASK);
	mvebu_mmc_write(mmc, SDIO_ERR_STATUS_EN, SDIO_POLL_MASK);

	/* enable interrupts status */
	mvebu_mmc_write(mmc, SDIO_NOR_INTR_STATUS, SDIO_POLL_MASK);
	mvebu_mmc_write(mmc, SDIO_ERR_INTR_STATUS, SDIO_POLL_MASK);
}

static void mvebu_mmc_set_clk(struct udevice *dev, unsigned int clock)
{
	unsigned int m;
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;

	if (clock == 0) {
		dev_dbg(dev, "clock off\n");
		mvebu_mmc_write(mmc, SDIO_XFER_MODE, SDIO_XFER_MODE_STOP_CLK);
		mvebu_mmc_write(mmc, SDIO_CLK_DIV, MVEBU_MMC_BASE_DIV_MAX);
	} else {
		m = MVEBU_MMC_BASE_FAST_CLOCK/(2*clock) - 1;
		if (m > MVEBU_MMC_BASE_DIV_MAX)
			m = MVEBU_MMC_BASE_DIV_MAX;
		mvebu_mmc_write(mmc, SDIO_CLK_DIV, m & MVEBU_MMC_BASE_DIV_MAX);
		dev_dbg(dev, "clock (%d) div : %d\n", clock, m);
	}
}

static void mvebu_mmc_set_bus(struct udevice *dev, unsigned int bus)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;
	u32 ctrl_reg = 0;

	ctrl_reg = mvebu_mmc_read(mmc, SDIO_HOST_CTRL);
	ctrl_reg &= ~SDIO_HOST_CTRL_DATA_WIDTH_4_BITS;

	switch (bus) {
	case 4:
		ctrl_reg |= SDIO_HOST_CTRL_DATA_WIDTH_4_BITS;
		break;
	case 1:
	default:
		ctrl_reg |= SDIO_HOST_CTRL_DATA_WIDTH_1_BIT;
	}

	/* default transfer mode */
	ctrl_reg |= SDIO_HOST_CTRL_BIG_ENDIAN;
	ctrl_reg &= ~SDIO_HOST_CTRL_LSB_FIRST;

	/* default to maximum timeout */
	ctrl_reg |= SDIO_HOST_CTRL_TMOUT(SDIO_HOST_CTRL_TMOUT_MAX);
	ctrl_reg |= SDIO_HOST_CTRL_TMOUT_EN;

	ctrl_reg |= SDIO_HOST_CTRL_PUSH_PULL_EN;

	ctrl_reg |= SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY;

	dev_dbg(dev, "ctrl 0x%04x: %s %s %s\n", ctrl_reg,
		(ctrl_reg & SDIO_HOST_CTRL_PUSH_PULL_EN) ?
		"push-pull" : "open-drain",
		(ctrl_reg & SDIO_HOST_CTRL_DATA_WIDTH_4_BITS) ?
		"4bit-width" : "1bit-width",
		(ctrl_reg & SDIO_HOST_CTRL_HI_SPEED_EN) ?
		"high-speed" : "");

	mvebu_mmc_write(mmc, SDIO_HOST_CTRL, ctrl_reg);
}

static int mvebu_mmc_set_ios(struct udevice *dev)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;

	dev_dbg(dev, "bus[%d] clock[%d]\n",
		mmc->bus_width, mmc->clock);
	mvebu_mmc_set_bus(dev, mmc->bus_width);
	mvebu_mmc_set_clk(dev, mmc->clock);

	return 0;
}

/*
 * Set window register.
 */
static void mvebu_window_setup(const struct mmc *mmc)
{
	int i;

	for (i = 0; i < 4; i++) {
		mvebu_mmc_write(mmc, WINDOW_CTRL(i), 0);
		mvebu_mmc_write(mmc, WINDOW_BASE(i), 0);
	}
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		u32 size, base, attrib;

		/* Enable DRAM bank */
		switch (i) {
		case 0:
			attrib = KWCPU_ATTR_DRAM_CS0;
			break;
		case 1:
			attrib = KWCPU_ATTR_DRAM_CS1;
			break;
		case 2:
			attrib = KWCPU_ATTR_DRAM_CS2;
			break;
		case 3:
			attrib = KWCPU_ATTR_DRAM_CS3;
			break;
		default:
			/* invalide bank, disable access */
			attrib = 0;
			break;
		}

		size = gd->bd->bi_dram[i].size;
		base = gd->bd->bi_dram[i].start;
		if (size && attrib) {
			mvebu_mmc_write(mmc, WINDOW_CTRL(i),
					MVCPU_WIN_CTRL_DATA(size,
							    MVEBU_TARGET_DRAM,
							    attrib,
							    MVCPU_WIN_ENABLE));
		} else {
			mvebu_mmc_write(mmc, WINDOW_CTRL(i), MVCPU_WIN_DISABLE);
		}
		mvebu_mmc_write(mmc, WINDOW_BASE(i), base);
	}
}

static int mvebu_mmc_initialize(struct udevice *dev)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc *mmc = &pdata->mmc;

	dev_dbg(dev, "%s\n", __func__);

	/*
	 * Setting host parameters
	 * Initial Host Ctrl : Timeout : max , Normal Speed mode,
	 * 4-bit data mode, Big Endian, SD memory Card, Push_pull CMD Line
	 */
	mvebu_mmc_write(mmc, SDIO_HOST_CTRL,
			SDIO_HOST_CTRL_TMOUT(SDIO_HOST_CTRL_TMOUT_MAX) |
			SDIO_HOST_CTRL_DATA_WIDTH_4_BITS |
			SDIO_HOST_CTRL_BIG_ENDIAN |
			SDIO_HOST_CTRL_PUSH_PULL_EN |
			SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY);

	mvebu_mmc_write(mmc, SDIO_CLK_CTRL, 0);

	/* enable status */
	mvebu_mmc_write(mmc, SDIO_NOR_STATUS_EN, SDIO_POLL_MASK);
	mvebu_mmc_write(mmc, SDIO_ERR_STATUS_EN, SDIO_POLL_MASK);

	/* disable interrupts */
	mvebu_mmc_write(mmc, SDIO_NOR_INTR_EN, 0);
	mvebu_mmc_write(mmc, SDIO_ERR_INTR_EN, 0);

	mvebu_window_setup(mmc);

	/* SW reset */
	mvebu_mmc_write(mmc, SDIO_SW_RESET, SDIO_SW_RESET_NOW);

	return 0;
}

static int mvebu_mmc_of_to_plat(struct udevice *dev)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	pdata->iobase = (void *)addr;

	return 0;
}

static int mvebu_mmc_probe(struct udevice *dev)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mmc *mmc = &pdata->mmc;
	struct mmc_config *cfg = &pdata->cfg;

	cfg->name = dev->name;
	cfg->f_min = MVEBU_MMC_BASE_FAST_CLOCK / MVEBU_MMC_BASE_DIV_MAX;
	cfg->f_max = MVEBU_MMC_CLOCKRATE_MAX;
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_HS | MMC_MODE_HS_52MHz;
	cfg->part_type = PART_TYPE_DOS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	mmc->cfg = cfg;
	mmc->priv = pdata;
	mmc->dev = dev;
	upriv->mmc = mmc;

	mvebu_mmc_power_up(dev);
	mvebu_mmc_initialize(dev);

	return 0;
}

static const struct dm_mmc_ops mvebu_dm_mmc_ops = {
	.send_cmd = mvebu_mmc_send_cmd,
	.set_ios = mvebu_mmc_set_ios,
};

static int mvebu_mmc_bind(struct udevice *dev)
{
	struct mvebu_mmc_plat *pdata = dev_get_plat(dev);

	return mmc_bind(dev, &pdata->mmc, &pdata->cfg);
}

static const struct udevice_id mvebu_mmc_match[] = {
	{ .compatible = "marvell,orion-sdio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mvebu_mmc) = {
	.name = "mvebu_mmc",
	.id = UCLASS_MMC,
	.of_match = mvebu_mmc_match,
	.ops = &mvebu_dm_mmc_ops,
	.probe = mvebu_mmc_probe,
	.bind = mvebu_mmc_bind,
	.of_to_plat = mvebu_mmc_of_to_plat,
	.plat_auto = sizeof(struct mvebu_mmc_plat),
};
