// SPDX-License-Identifier: GPL-2.0+
/*
 * Davinci MMC Controller Driver
 *
 * Copyright (C) 2010 Texas Instruments Incorporated
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mmc.h>
#include <command.h>
#include <part.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/sdmmc_defs.h>
#include <asm-generic/gpio.h>

#define DAVINCI_MAX_BLOCKS	(32)
#define WATCHDOG_COUNT		(100000)

#define get_val(addr)		REG(addr)
#define set_val(addr, val)	REG(addr) = (val)
#define set_bit(addr, val)	set_val((addr), (get_val(addr) | (val)))
#define clear_bit(addr, val)	set_val((addr), (get_val(addr) & ~(val)))

#ifdef CONFIG_DM_MMC
struct davinci_of_data {
	const char *name;
	u8 version;
};

/* Davinci MMC board definitions */
struct davinci_mmc_priv {
	struct davinci_mmc_regs *reg_base;	/* Register base address */
	uint input_clk;		/* Input clock to MMC controller */
	uint version;		/* MMC Controller version */
	struct gpio_desc cd_gpio;       /* Card Detect GPIO */
	struct gpio_desc wp_gpio;       /* Write Protect GPIO */
};

struct davinci_mmc_plat
{
	struct mmc_config cfg;
	struct mmc mmc;
};
#endif

/* Set davinci clock prescalar value based on the required clock in HZ */
#if !CONFIG_IS_ENABLED(DM_MMC)
static void dmmc_set_clock(struct mmc *mmc, uint clock)
{
	struct davinci_mmc *host = mmc->priv;
#else

static void davinci_mmc_set_clock(struct udevice *dev, uint clock)
{
	struct davinci_mmc_priv *host = dev_get_priv(dev);
        struct mmc *mmc = mmc_get_mmc_dev(dev);
#endif
	struct davinci_mmc_regs *regs = host->reg_base;
	uint clkrt, sysclk2, act_clock;

	if (clock < mmc->cfg->f_min)
		clock = mmc->cfg->f_min;
	if (clock > mmc->cfg->f_max)
		clock = mmc->cfg->f_max;

	set_val(&regs->mmcclk, 0);
	sysclk2 = host->input_clk;
	clkrt = (sysclk2 / (2 * clock)) - 1;

	/* Calculate the actual clock for the divider used */
	act_clock = (sysclk2 / (2 * (clkrt + 1)));

	/* Adjust divider if actual clock exceeds the required clock */
	if (act_clock > clock)
		clkrt++;

	/* check clock divider boundary and correct it */
	if (clkrt > 0xFF)
		clkrt = 0xFF;

	set_val(&regs->mmcclk, (clkrt | MMCCLK_CLKEN));
}

/* Status bit wait loop for MMCST1 */
static int
dmmc_wait_fifo_status(volatile struct davinci_mmc_regs *regs, uint status)
{
	uint wdog = WATCHDOG_COUNT;

	while (--wdog && ((get_val(&regs->mmcst1) & status) != status))
		udelay(10);

	if (!(get_val(&regs->mmcctl) & MMCCTL_WIDTH_4_BIT))
		udelay(100);

	if (wdog == 0)
		return -ECOMM;

	return 0;
}

/* Busy bit wait loop for MMCST1 */
static int dmmc_busy_wait(volatile struct davinci_mmc_regs *regs)
{
	uint wdog = WATCHDOG_COUNT;

	while (--wdog && (get_val(&regs->mmcst1) & MMCST1_BUSY))
		udelay(10);

	if (wdog == 0)
		return -ECOMM;

	return 0;
}

/* Status bit wait loop for MMCST0 - Checks for error bits as well */
static int dmmc_check_status(volatile struct davinci_mmc_regs *regs,
		uint *cur_st, uint st_ready, uint st_error)
{
	uint wdog = WATCHDOG_COUNT;
	uint mmcstatus = *cur_st;

	while (wdog--) {
		if (mmcstatus & st_ready) {
			*cur_st = mmcstatus;
			mmcstatus = get_val(&regs->mmcst1);
			return 0;
		} else if (mmcstatus & st_error) {
			if (mmcstatus & MMCST0_TOUTRS)
				return -ETIMEDOUT;
			printf("[ ST0 ERROR %x]\n", mmcstatus);
			/*
			 * Ignore CRC errors as some MMC cards fail to
			 * initialize on DM365-EVM on the SD1 slot
			 */
			if (mmcstatus & MMCST0_CRCRS)
				return 0;
			return -ECOMM;
		}
		udelay(10);

		mmcstatus = get_val(&regs->mmcst0);
	}

	printf("Status %x Timeout ST0:%x ST1:%x\n", st_ready, mmcstatus,
			get_val(&regs->mmcst1));
	return -ECOMM;
}

/*
 * Sends a command out on the bus.  Takes the device pointer,
 * a command pointer, and an optional data pointer.
 */
#if !CONFIG_IS_ENABLED(DM_MMC)
static int dmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct davinci_mmc *host = mmc->priv;
#else
static int
davinci_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct davinci_mmc_priv *host = dev_get_priv(dev);
#endif
	volatile struct davinci_mmc_regs *regs = host->reg_base;
	uint mmcstatus, status_rdy, status_err;
	uint i, cmddata, bytes_left = 0;
	int fifo_words, fifo_bytes, err;
	char *data_buf = NULL;

	/* Clear status registers */
	mmcstatus = get_val(&regs->mmcst0);
	fifo_words = (host->version == MMC_CTLR_VERSION_2) ? 16 : 8;
	fifo_bytes = fifo_words << 2;

	/* Wait for any previous busy signal to be cleared */
	dmmc_busy_wait(regs);

	cmddata = cmd->cmdidx;
	cmddata |= MMCCMD_PPLEN;

	/* Send init clock for CMD0 */
	if (cmd->cmdidx == MMC_CMD_GO_IDLE_STATE)
		cmddata |= MMCCMD_INITCK;

	switch (cmd->resp_type) {
	case MMC_RSP_R1b:
		cmddata |= MMCCMD_BSYEXP;
		/* Fall-through */
	case MMC_RSP_R1:    /* R1, R1b, R5, R6, R7 */
		cmddata |= MMCCMD_RSPFMT_R1567;
		break;
	case MMC_RSP_R2:
		cmddata |= MMCCMD_RSPFMT_R2;
		break;
	case MMC_RSP_R3: /* R3, R4 */
		cmddata |= MMCCMD_RSPFMT_R3;
		break;
	}

	set_val(&regs->mmcim, 0);

	if (data) {
		/* clear previous data transfer if any and set new one */
		bytes_left = (data->blocksize * data->blocks);

		/* Reset FIFO - Always use 32 byte fifo threshold */
		set_val(&regs->mmcfifoctl,
				(MMCFIFOCTL_FIFOLEV | MMCFIFOCTL_FIFORST));

		if (host->version == MMC_CTLR_VERSION_2)
			cmddata |= MMCCMD_DMATRIG;

		cmddata |= MMCCMD_WDATX;
		if (data->flags == MMC_DATA_READ) {
			set_val(&regs->mmcfifoctl, MMCFIFOCTL_FIFOLEV);
		} else if (data->flags == MMC_DATA_WRITE) {
			set_val(&regs->mmcfifoctl,
					(MMCFIFOCTL_FIFOLEV |
					 MMCFIFOCTL_FIFODIR));
			cmddata |= MMCCMD_DTRW;
		}

		set_val(&regs->mmctod, 0xFFFF);
		set_val(&regs->mmcnblk, (data->blocks & MMCNBLK_NBLK_MASK));
		set_val(&regs->mmcblen, (data->blocksize & MMCBLEN_BLEN_MASK));

		if (data->flags == MMC_DATA_WRITE) {
			uint val;
			data_buf = (char *)data->src;
			/* For write, fill FIFO with data before issue of CMD */
			for (i = 0; (i < fifo_words) && bytes_left; i++) {
				memcpy((char *)&val, data_buf, 4);
				set_val(&regs->mmcdxr, val);
				data_buf += 4;
				bytes_left -= 4;
			}
		}
	} else {
		set_val(&regs->mmcblen, 0);
		set_val(&regs->mmcnblk, 0);
	}

	set_val(&regs->mmctor, 0x1FFF);

	/* Send the command */
	set_val(&regs->mmcarghl, cmd->cmdarg);
	set_val(&regs->mmccmd, cmddata);

	status_rdy = MMCST0_RSPDNE;
	status_err = (MMCST0_TOUTRS | MMCST0_TOUTRD |
			MMCST0_CRCWR | MMCST0_CRCRD);
	if (cmd->resp_type & MMC_RSP_CRC)
		status_err |= MMCST0_CRCRS;

	mmcstatus = get_val(&regs->mmcst0);
	err = dmmc_check_status(regs, &mmcstatus, status_rdy, status_err);
	if (err)
		return err;

	/* For R1b wait for busy done */
	if (cmd->resp_type == MMC_RSP_R1b)
		dmmc_busy_wait(regs);

	/* Collect response from controller for specific commands */
	if (mmcstatus & MMCST0_RSPDNE) {
		/* Copy the response to the response buffer */
		if (cmd->resp_type & MMC_RSP_136) {
			cmd->response[0] = get_val(&regs->mmcrsp67);
			cmd->response[1] = get_val(&regs->mmcrsp45);
			cmd->response[2] = get_val(&regs->mmcrsp23);
			cmd->response[3] = get_val(&regs->mmcrsp01);
		} else if (cmd->resp_type & MMC_RSP_PRESENT) {
			cmd->response[0] = get_val(&regs->mmcrsp67);
		}
	}

	if (data == NULL)
		return 0;

	if (data->flags == MMC_DATA_READ) {
		/* check for DATDNE along with DRRDY as the controller might
		 * set the DATDNE without DRRDY for smaller transfers with
		 * less than FIFO threshold bytes
		 */
		status_rdy = MMCST0_DRRDY | MMCST0_DATDNE;
		status_err = MMCST0_TOUTRD | MMCST0_CRCRD;
		data_buf = data->dest;
	} else {
		status_rdy = MMCST0_DXRDY | MMCST0_DATDNE;
		status_err = MMCST0_CRCWR;
	}

	/* Wait until all of the blocks are transferred */
	while (bytes_left) {
		err = dmmc_check_status(regs, &mmcstatus, status_rdy,
				status_err);
		if (err)
			return err;

		if (data->flags == MMC_DATA_READ) {
			/*
			 * MMC controller sets the Data receive ready bit
			 * (DRRDY) in MMCST0 even before the entire FIFO is
			 * full. This results in erratic behavior if we start
			 * reading the FIFO soon after DRRDY.  Wait for the
			 * FIFO full bit in MMCST1 for proper FIFO clearing.
			 */
			if (bytes_left > fifo_bytes)
				dmmc_wait_fifo_status(regs, 0x4a);
			else if (bytes_left == fifo_bytes) {
				dmmc_wait_fifo_status(regs, 0x40);
				if (cmd->cmdidx == MMC_CMD_SEND_EXT_CSD)
					udelay(600);
			}

			for (i = 0; bytes_left && (i < fifo_words); i++) {
				cmddata = get_val(&regs->mmcdrr);
				memcpy(data_buf, (char *)&cmddata, 4);
				data_buf += 4;
				bytes_left -= 4;
			}
		} else {
			/*
			 * MMC controller sets the Data transmit ready bit
			 * (DXRDY) in MMCST0 even before the entire FIFO is
			 * empty. This results in erratic behavior if we start
			 * writing the FIFO soon after DXRDY.  Wait for the
			 * FIFO empty bit in MMCST1 for proper FIFO clearing.
			 */
			dmmc_wait_fifo_status(regs, MMCST1_FIFOEMP);
			for (i = 0; bytes_left && (i < fifo_words); i++) {
				memcpy((char *)&cmddata, data_buf, 4);
				set_val(&regs->mmcdxr, cmddata);
				data_buf += 4;
				bytes_left -= 4;
			}
			dmmc_busy_wait(regs);
		}
	}

	err = dmmc_check_status(regs, &mmcstatus, MMCST0_DATDNE, status_err);
	if (err)
		return err;

	return 0;
}

/* Initialize Davinci MMC controller */
#if !CONFIG_IS_ENABLED(DM_MMC)
static int dmmc_init(struct mmc *mmc)
{
	struct davinci_mmc *host = mmc->priv;
#else
static int davinci_dm_mmc_init(struct udevice *dev)
{
	struct davinci_mmc_priv *host = dev_get_priv(dev);
#endif
	struct davinci_mmc_regs *regs = host->reg_base;

	/* Clear status registers explicitly - soft reset doesn't clear it
	 * If Uboot is invoked from UBL with SDMMC Support, the status
	 * registers can have uncleared bits
	 */
	get_val(&regs->mmcst0);
	get_val(&regs->mmcst1);

	/* Hold software reset */
	set_bit(&regs->mmcctl, MMCCTL_DATRST);
	set_bit(&regs->mmcctl, MMCCTL_CMDRST);
	udelay(10);

	set_val(&regs->mmcclk, 0x0);
	set_val(&regs->mmctor, 0x1FFF);
	set_val(&regs->mmctod, 0xFFFF);

	/* Clear software reset */
	clear_bit(&regs->mmcctl, MMCCTL_DATRST);
	clear_bit(&regs->mmcctl, MMCCTL_CMDRST);

	udelay(10);

	/* Reset FIFO - Always use the maximum fifo threshold */
	set_val(&regs->mmcfifoctl, (MMCFIFOCTL_FIFOLEV | MMCFIFOCTL_FIFORST));
	set_val(&regs->mmcfifoctl, MMCFIFOCTL_FIFOLEV);

	return 0;
}

/* Set buswidth or clock as indicated by the MMC framework */
#if !CONFIG_IS_ENABLED(DM_MMC)
static int dmmc_set_ios(struct mmc *mmc)
{
	struct davinci_mmc *host = mmc->priv;
	struct davinci_mmc_regs *regs = host->reg_base;
#else
static int davinci_mmc_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	struct davinci_mmc_priv *host = dev_get_priv(dev);
	struct davinci_mmc_regs *regs = host->reg_base;
#endif
	/* Set the bus width */
	if (mmc->bus_width == 4)
		set_bit(&regs->mmcctl, MMCCTL_WIDTH_4_BIT);
	else
		clear_bit(&regs->mmcctl, MMCCTL_WIDTH_4_BIT);

	/* Set clock speed */
	if (mmc->clock) {
#if !CONFIG_IS_ENABLED(DM_MMC)
		dmmc_set_clock(mmc, mmc->clock);
#else
		davinci_mmc_set_clock(dev, mmc->clock);
#endif
	}
	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static const struct mmc_ops dmmc_ops = {
       .send_cmd       = dmmc_send_cmd,
       .set_ios        = dmmc_set_ios,
       .init           = dmmc_init,
};
#else

static int davinci_mmc_getcd(struct udevice *dev)
{
	int value = -1;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct davinci_mmc_priv *priv = dev_get_priv(dev);
	value = dm_gpio_get_value(&priv->cd_gpio);
#endif
	/* if no CD return as 1 */
	if (value < 0)
		return 1;

	return value;
}

static int davinci_mmc_getwp(struct udevice *dev)
{
	int value = -1;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct davinci_mmc_priv *priv = dev_get_priv(dev);

	value = dm_gpio_get_value(&priv->wp_gpio);
#endif
	/* if no WP return as 0 */
	if (value < 0)
		return 0;

	return value;
}

static const struct dm_mmc_ops davinci_mmc_ops = {
	.send_cmd	= davinci_mmc_send_cmd,
	.set_ios	= davinci_mmc_set_ios,
	.get_cd		= davinci_mmc_getcd,
	.get_wp		= davinci_mmc_getwp,
};
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
/* Called from board_mmc_init during startup. Can be called multiple times
* depending on the number of slots available on board and controller
*/
int davinci_mmc_init(bd_t *bis, struct davinci_mmc *host)
{
	host->cfg.name = "davinci";
	host->cfg.ops = &dmmc_ops;
	host->cfg.f_min = 200000;
	host->cfg.f_max = 25000000;
	host->cfg.voltages = host->voltages;
	host->cfg.host_caps = host->host_caps;

	host->cfg.b_max = DAVINCI_MAX_BLOCKS;

	mmc_create(&host->cfg, host);

	return 0;
}
#else


static int davinci_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct davinci_mmc_plat *plat = dev_get_platdata(dev);
	struct davinci_mmc_priv *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct davinci_of_data *data =
			(struct davinci_of_data *)dev_get_driver_data(dev);
	cfg->f_min = 200000;
	cfg->f_max = 25000000;
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34,
	cfg->host_caps = MMC_MODE_4BIT, /* DA850 supports only 4-bit SD/MMC */
	cfg->b_max = DAVINCI_MAX_BLOCKS;

	if (data) {
		cfg->name = data->name;
		priv->version = data->version;
	}

	priv->reg_base = (struct davinci_mmc_regs *)dev_read_addr(dev);
	priv->input_clk = clk_get(DAVINCI_MMCSD_CLKID);

#if CONFIG_IS_ENABLED(DM_GPIO)
	/* These GPIOs are optional */
	gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio, GPIOD_IS_IN);
	gpio_request_by_name(dev, "wp-gpios", 0, &priv->wp_gpio, GPIOD_IS_IN);
#endif

	upriv->mmc = &plat->mmc;

	return davinci_dm_mmc_init(dev);
}

static int davinci_mmc_bind(struct udevice *dev)
{
	struct davinci_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}


const struct davinci_of_data davinci_mmc_host_info[] = {
	{
		.name	= "dm6441-mmc",
		.version = MMC_CTLR_VERSION_1,
	},
	{
		.name	= "da830-mmc",
		.version = MMC_CTLR_VERSION_2,
	},
	{},
};

static const struct udevice_id davinci_mmc_ids[] = {
	{
		.compatible = "ti,dm6441-mmc",
		.data = (ulong) &davinci_mmc_host_info[MMC_CTLR_VERSION_1]
	},
	{
		.compatible = "ti,da830-mmc",
		.data = (ulong) &davinci_mmc_host_info[MMC_CTLR_VERSION_2]
	},
	{},
};

U_BOOT_DRIVER(davinci_mmc_drv) = {
	.name = "davinci_mmc",
	.id		= UCLASS_MMC,
	.of_match	= davinci_mmc_ids,
#if CONFIG_BLK
	.bind		= davinci_mmc_bind,
#endif
	.probe = davinci_mmc_probe,
	.ops = &davinci_mmc_ops,
	.platdata_auto_alloc_size = sizeof(struct davinci_mmc_plat),
	.priv_auto_alloc_size = sizeof(struct davinci_mmc_priv),
};
#endif
