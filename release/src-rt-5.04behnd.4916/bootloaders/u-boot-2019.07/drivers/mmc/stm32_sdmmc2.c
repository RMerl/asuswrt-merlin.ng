// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <mmc.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/iopoll.h>

struct stm32_sdmmc2_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct stm32_sdmmc2_priv {
	fdt_addr_t base;
	struct clk clk;
	struct reset_ctl reset_ctl;
	struct gpio_desc cd_gpio;
	u32 clk_reg_msk;
	u32 pwr_reg_msk;
};

struct stm32_sdmmc2_ctx {
	u32 cache_start;
	u32 cache_end;
	u32 data_length;
	bool dpsm_abort;
};

/* SDMMC REGISTERS OFFSET */
#define SDMMC_POWER		0x00	/* SDMMC power control             */
#define SDMMC_CLKCR		0x04	/* SDMMC clock control             */
#define SDMMC_ARG		0x08	/* SDMMC argument                  */
#define SDMMC_CMD		0x0C	/* SDMMC command                   */
#define SDMMC_RESP1		0x14	/* SDMMC response 1                */
#define SDMMC_RESP2		0x18	/* SDMMC response 2                */
#define SDMMC_RESP3		0x1C	/* SDMMC response 3                */
#define SDMMC_RESP4		0x20	/* SDMMC response 4                */
#define SDMMC_DTIMER		0x24	/* SDMMC data timer                */
#define SDMMC_DLEN		0x28	/* SDMMC data length               */
#define SDMMC_DCTRL		0x2C	/* SDMMC data control              */
#define SDMMC_DCOUNT		0x30	/* SDMMC data counter              */
#define SDMMC_STA		0x34	/* SDMMC status                    */
#define SDMMC_ICR		0x38	/* SDMMC interrupt clear           */
#define SDMMC_MASK		0x3C	/* SDMMC mask                      */
#define SDMMC_IDMACTRL		0x50	/* SDMMC DMA control               */
#define SDMMC_IDMABASE0		0x58	/* SDMMC DMA buffer 0 base address */

/* SDMMC_POWER register */
#define SDMMC_POWER_PWRCTRL_MASK	GENMASK(1, 0)
#define SDMMC_POWER_PWRCTRL_OFF		0
#define SDMMC_POWER_PWRCTRL_CYCLE	2
#define SDMMC_POWER_PWRCTRL_ON		3
#define SDMMC_POWER_VSWITCH		BIT(2)
#define SDMMC_POWER_VSWITCHEN		BIT(3)
#define SDMMC_POWER_DIRPOL		BIT(4)

/* SDMMC_CLKCR register */
#define SDMMC_CLKCR_CLKDIV		GENMASK(9, 0)
#define SDMMC_CLKCR_CLKDIV_MAX		SDMMC_CLKCR_CLKDIV
#define SDMMC_CLKCR_PWRSAV		BIT(12)
#define SDMMC_CLKCR_WIDBUS_4		BIT(14)
#define SDMMC_CLKCR_WIDBUS_8		BIT(15)
#define SDMMC_CLKCR_NEGEDGE		BIT(16)
#define SDMMC_CLKCR_HWFC_EN		BIT(17)
#define SDMMC_CLKCR_DDR			BIT(18)
#define SDMMC_CLKCR_BUSSPEED		BIT(19)
#define SDMMC_CLKCR_SELCLKRX_MASK	GENMASK(21, 20)
#define SDMMC_CLKCR_SELCLKRX_CK		0
#define SDMMC_CLKCR_SELCLKRX_CKIN	BIT(20)
#define SDMMC_CLKCR_SELCLKRX_FBCK	BIT(21)

/* SDMMC_CMD register */
#define SDMMC_CMD_CMDINDEX		GENMASK(5, 0)
#define SDMMC_CMD_CMDTRANS		BIT(6)
#define SDMMC_CMD_CMDSTOP		BIT(7)
#define SDMMC_CMD_WAITRESP		GENMASK(9, 8)
#define SDMMC_CMD_WAITRESP_0		BIT(8)
#define SDMMC_CMD_WAITRESP_1		BIT(9)
#define SDMMC_CMD_WAITINT		BIT(10)
#define SDMMC_CMD_WAITPEND		BIT(11)
#define SDMMC_CMD_CPSMEN		BIT(12)
#define SDMMC_CMD_DTHOLD		BIT(13)
#define SDMMC_CMD_BOOTMODE		BIT(14)
#define SDMMC_CMD_BOOTEN		BIT(15)
#define SDMMC_CMD_CMDSUSPEND		BIT(16)

/* SDMMC_DCTRL register */
#define SDMMC_DCTRL_DTEN		BIT(0)
#define SDMMC_DCTRL_DTDIR		BIT(1)
#define SDMMC_DCTRL_DTMODE		GENMASK(3, 2)
#define SDMMC_DCTRL_DBLOCKSIZE		GENMASK(7, 4)
#define SDMMC_DCTRL_DBLOCKSIZE_SHIFT	4
#define SDMMC_DCTRL_RWSTART		BIT(8)
#define SDMMC_DCTRL_RWSTOP		BIT(9)
#define SDMMC_DCTRL_RWMOD		BIT(10)
#define SDMMC_DCTRL_SDMMCEN		BIT(11)
#define SDMMC_DCTRL_BOOTACKEN		BIT(12)
#define SDMMC_DCTRL_FIFORST		BIT(13)

/* SDMMC_STA register */
#define SDMMC_STA_CCRCFAIL		BIT(0)
#define SDMMC_STA_DCRCFAIL		BIT(1)
#define SDMMC_STA_CTIMEOUT		BIT(2)
#define SDMMC_STA_DTIMEOUT		BIT(3)
#define SDMMC_STA_TXUNDERR		BIT(4)
#define SDMMC_STA_RXOVERR		BIT(5)
#define SDMMC_STA_CMDREND		BIT(6)
#define SDMMC_STA_CMDSENT		BIT(7)
#define SDMMC_STA_DATAEND		BIT(8)
#define SDMMC_STA_DHOLD			BIT(9)
#define SDMMC_STA_DBCKEND		BIT(10)
#define SDMMC_STA_DABORT		BIT(11)
#define SDMMC_STA_DPSMACT		BIT(12)
#define SDMMC_STA_CPSMACT		BIT(13)
#define SDMMC_STA_TXFIFOHE		BIT(14)
#define SDMMC_STA_RXFIFOHF		BIT(15)
#define SDMMC_STA_TXFIFOF		BIT(16)
#define SDMMC_STA_RXFIFOF		BIT(17)
#define SDMMC_STA_TXFIFOE		BIT(18)
#define SDMMC_STA_RXFIFOE		BIT(19)
#define SDMMC_STA_BUSYD0		BIT(20)
#define SDMMC_STA_BUSYD0END		BIT(21)
#define SDMMC_STA_SDMMCIT		BIT(22)
#define SDMMC_STA_ACKFAIL		BIT(23)
#define SDMMC_STA_ACKTIMEOUT		BIT(24)
#define SDMMC_STA_VSWEND		BIT(25)
#define SDMMC_STA_CKSTOP		BIT(26)
#define SDMMC_STA_IDMATE		BIT(27)
#define SDMMC_STA_IDMABTC		BIT(28)

/* SDMMC_ICR register */
#define SDMMC_ICR_CCRCFAILC		BIT(0)
#define SDMMC_ICR_DCRCFAILC		BIT(1)
#define SDMMC_ICR_CTIMEOUTC		BIT(2)
#define SDMMC_ICR_DTIMEOUTC		BIT(3)
#define SDMMC_ICR_TXUNDERRC		BIT(4)
#define SDMMC_ICR_RXOVERRC		BIT(5)
#define SDMMC_ICR_CMDRENDC		BIT(6)
#define SDMMC_ICR_CMDSENTC		BIT(7)
#define SDMMC_ICR_DATAENDC		BIT(8)
#define SDMMC_ICR_DHOLDC		BIT(9)
#define SDMMC_ICR_DBCKENDC		BIT(10)
#define SDMMC_ICR_DABORTC		BIT(11)
#define SDMMC_ICR_BUSYD0ENDC		BIT(21)
#define SDMMC_ICR_SDMMCITC		BIT(22)
#define SDMMC_ICR_ACKFAILC		BIT(23)
#define SDMMC_ICR_ACKTIMEOUTC		BIT(24)
#define SDMMC_ICR_VSWENDC		BIT(25)
#define SDMMC_ICR_CKSTOPC		BIT(26)
#define SDMMC_ICR_IDMATEC		BIT(27)
#define SDMMC_ICR_IDMABTCC		BIT(28)
#define SDMMC_ICR_STATIC_FLAGS		((GENMASK(28, 21)) | (GENMASK(11, 0)))

/* SDMMC_MASK register */
#define SDMMC_MASK_CCRCFAILIE		BIT(0)
#define SDMMC_MASK_DCRCFAILIE		BIT(1)
#define SDMMC_MASK_CTIMEOUTIE		BIT(2)
#define SDMMC_MASK_DTIMEOUTIE		BIT(3)
#define SDMMC_MASK_TXUNDERRIE		BIT(4)
#define SDMMC_MASK_RXOVERRIE		BIT(5)
#define SDMMC_MASK_CMDRENDIE		BIT(6)
#define SDMMC_MASK_CMDSENTIE		BIT(7)
#define SDMMC_MASK_DATAENDIE		BIT(8)
#define SDMMC_MASK_DHOLDIE		BIT(9)
#define SDMMC_MASK_DBCKENDIE		BIT(10)
#define SDMMC_MASK_DABORTIE		BIT(11)
#define SDMMC_MASK_TXFIFOHEIE		BIT(14)
#define SDMMC_MASK_RXFIFOHFIE		BIT(15)
#define SDMMC_MASK_RXFIFOFIE		BIT(17)
#define SDMMC_MASK_TXFIFOEIE		BIT(18)
#define SDMMC_MASK_BUSYD0ENDIE		BIT(21)
#define SDMMC_MASK_SDMMCITIE		BIT(22)
#define SDMMC_MASK_ACKFAILIE		BIT(23)
#define SDMMC_MASK_ACKTIMEOUTIE		BIT(24)
#define SDMMC_MASK_VSWENDIE		BIT(25)
#define SDMMC_MASK_CKSTOPIE		BIT(26)
#define SDMMC_MASK_IDMABTCIE		BIT(28)

/* SDMMC_IDMACTRL register */
#define SDMMC_IDMACTRL_IDMAEN		BIT(0)

#define SDMMC_CMD_TIMEOUT		0xFFFFFFFF
#define SDMMC_BUSYD0END_TIMEOUT_US	1000000

static void stm32_sdmmc2_start_data(struct stm32_sdmmc2_priv *priv,
				    struct mmc_data *data,
				    struct stm32_sdmmc2_ctx *ctx)
{
	u32 data_ctrl, idmabase0;

	/* Configure the SDMMC DPSM (Data Path State Machine) */
	data_ctrl = (__ilog2(data->blocksize) <<
		     SDMMC_DCTRL_DBLOCKSIZE_SHIFT) &
		    SDMMC_DCTRL_DBLOCKSIZE;

	if (data->flags & MMC_DATA_READ) {
		data_ctrl |= SDMMC_DCTRL_DTDIR;
		idmabase0 = (u32)data->dest;
	} else {
		idmabase0 = (u32)data->src;
	}

	/* Set the SDMMC DataLength value */
	writel(ctx->data_length, priv->base + SDMMC_DLEN);

	/* Write to SDMMC DCTRL */
	writel(data_ctrl, priv->base + SDMMC_DCTRL);

	/* Cache align */
	ctx->cache_start = rounddown(idmabase0, ARCH_DMA_MINALIGN);
	ctx->cache_end = roundup(idmabase0 + ctx->data_length,
				 ARCH_DMA_MINALIGN);

	/*
	 * Flush data cache before DMA start (clean and invalidate)
	 * Clean also needed for read
	 * Avoid issue on buffer not cached-aligned
	 */
	flush_dcache_range(ctx->cache_start, ctx->cache_end);

	/* Enable internal DMA */
	writel(idmabase0, priv->base + SDMMC_IDMABASE0);
	writel(SDMMC_IDMACTRL_IDMAEN, priv->base + SDMMC_IDMACTRL);
}

static void stm32_sdmmc2_start_cmd(struct stm32_sdmmc2_priv *priv,
				   struct mmc_cmd *cmd, u32 cmd_param,
				   struct stm32_sdmmc2_ctx *ctx)
{
	u32 timeout = 0;

	if (readl(priv->base + SDMMC_CMD) & SDMMC_CMD_CPSMEN)
		writel(0, priv->base + SDMMC_CMD);

	cmd_param |= cmd->cmdidx | SDMMC_CMD_CPSMEN;
	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136)
			cmd_param |= SDMMC_CMD_WAITRESP;
		else if (cmd->resp_type & MMC_RSP_CRC)
			cmd_param |= SDMMC_CMD_WAITRESP_0;
		else
			cmd_param |= SDMMC_CMD_WAITRESP_1;
	}

	/*
	 * SDMMC_DTIME must be set in two case:
	 * - on data transfert.
	 * - on busy request.
	 * If not done or too short, the dtimeout flag occurs and DPSM stays
	 * enabled/busy and waits for abort (stop transmission cmd).
	 * Next data command is not possible whereas DPSM is activated.
	 */
	if (ctx->data_length) {
		timeout = SDMMC_CMD_TIMEOUT;
	} else {
		writel(0, priv->base + SDMMC_DCTRL);

		if (cmd->resp_type & MMC_RSP_BUSY)
			timeout = SDMMC_CMD_TIMEOUT;
	}

	/* Set the SDMMC Data TimeOut value */
	writel(timeout, priv->base + SDMMC_DTIMER);

	/* Clear flags */
	writel(SDMMC_ICR_STATIC_FLAGS, priv->base + SDMMC_ICR);

	/* Set SDMMC argument value */
	writel(cmd->cmdarg, priv->base + SDMMC_ARG);

	/* Set SDMMC command parameters */
	writel(cmd_param, priv->base + SDMMC_CMD);
}

static int stm32_sdmmc2_end_cmd(struct stm32_sdmmc2_priv *priv,
				struct mmc_cmd *cmd,
				struct stm32_sdmmc2_ctx *ctx)
{
	u32 mask = SDMMC_STA_CTIMEOUT;
	u32 status;
	int ret;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		mask |= SDMMC_STA_CMDREND;
		if (cmd->resp_type & MMC_RSP_CRC)
			mask |= SDMMC_STA_CCRCFAIL;
	} else {
		mask |= SDMMC_STA_CMDSENT;
	}

	/* Polling status register */
	ret = readl_poll_timeout(priv->base + SDMMC_STA, status, status & mask,
				 10000);

	if (ret < 0) {
		debug("%s: timeout reading SDMMC_STA register\n", __func__);
		ctx->dpsm_abort = true;
		return ret;
	}

	/* Check status */
	if (status & SDMMC_STA_CTIMEOUT) {
		debug("%s: error SDMMC_STA_CTIMEOUT (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		ctx->dpsm_abort = true;
		return -ETIMEDOUT;
	}

	if (status & SDMMC_STA_CCRCFAIL && cmd->resp_type & MMC_RSP_CRC) {
		debug("%s: error SDMMC_STA_CCRCFAIL (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		ctx->dpsm_abort = true;
		return -EILSEQ;
	}

	if (status & SDMMC_STA_CMDREND && cmd->resp_type & MMC_RSP_PRESENT) {
		cmd->response[0] = readl(priv->base + SDMMC_RESP1);
		if (cmd->resp_type & MMC_RSP_136) {
			cmd->response[1] = readl(priv->base + SDMMC_RESP2);
			cmd->response[2] = readl(priv->base + SDMMC_RESP3);
			cmd->response[3] = readl(priv->base + SDMMC_RESP4);
		}

		/* Wait for BUSYD0END flag if busy status is detected */
		if (cmd->resp_type & MMC_RSP_BUSY &&
		    status & SDMMC_STA_BUSYD0) {
			mask = SDMMC_STA_DTIMEOUT | SDMMC_STA_BUSYD0END;

			/* Polling status register */
			ret = readl_poll_timeout(priv->base + SDMMC_STA,
						 status, status & mask,
						 SDMMC_BUSYD0END_TIMEOUT_US);

			if (ret < 0) {
				debug("%s: timeout reading SDMMC_STA\n",
				      __func__);
				ctx->dpsm_abort = true;
				return ret;
			}

			if (status & SDMMC_STA_DTIMEOUT) {
				debug("%s: error SDMMC_STA_DTIMEOUT (0x%x)\n",
				      __func__, status);
				ctx->dpsm_abort = true;
				return -ETIMEDOUT;
			}
		}
	}

	return 0;
}

static int stm32_sdmmc2_end_data(struct stm32_sdmmc2_priv *priv,
				 struct mmc_cmd *cmd,
				 struct mmc_data *data,
				 struct stm32_sdmmc2_ctx *ctx)
{
	u32 mask = SDMMC_STA_DCRCFAIL | SDMMC_STA_DTIMEOUT |
		   SDMMC_STA_IDMATE | SDMMC_STA_DATAEND;
	u32 status;

	if (data->flags & MMC_DATA_READ)
		mask |= SDMMC_STA_RXOVERR;
	else
		mask |= SDMMC_STA_TXUNDERR;

	status = readl(priv->base + SDMMC_STA);
	while (!(status & mask))
		status = readl(priv->base + SDMMC_STA);

	/*
	 * Need invalidate the dcache again to avoid any
	 * cache-refill during the DMA operations (pre-fetching)
	 */
	if (data->flags & MMC_DATA_READ)
		invalidate_dcache_range(ctx->cache_start, ctx->cache_end);

	if (status & SDMMC_STA_DCRCFAIL) {
		debug("%s: error SDMMC_STA_DCRCFAIL (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		if (readl(priv->base + SDMMC_DCOUNT))
			ctx->dpsm_abort = true;
		return -EILSEQ;
	}

	if (status & SDMMC_STA_DTIMEOUT) {
		debug("%s: error SDMMC_STA_DTIMEOUT (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		ctx->dpsm_abort = true;
		return -ETIMEDOUT;
	}

	if (status & SDMMC_STA_TXUNDERR) {
		debug("%s: error SDMMC_STA_TXUNDERR (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		ctx->dpsm_abort = true;
		return -EIO;
	}

	if (status & SDMMC_STA_RXOVERR) {
		debug("%s: error SDMMC_STA_RXOVERR (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		ctx->dpsm_abort = true;
		return -EIO;
	}

	if (status & SDMMC_STA_IDMATE) {
		debug("%s: error SDMMC_STA_IDMATE (0x%x) for cmd %d\n",
		      __func__, status, cmd->cmdidx);
		ctx->dpsm_abort = true;
		return -EIO;
	}

	return 0;
}

static int stm32_sdmmc2_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
				 struct mmc_data *data)
{
	struct stm32_sdmmc2_priv *priv = dev_get_priv(dev);
	struct stm32_sdmmc2_ctx ctx;
	u32 cmdat = data ? SDMMC_CMD_CMDTRANS : 0;
	int ret, retry = 3;

retry_cmd:
	ctx.data_length = 0;
	ctx.dpsm_abort = false;

	if (data) {
		ctx.data_length = data->blocks * data->blocksize;
		stm32_sdmmc2_start_data(priv, data, &ctx);
	}

	stm32_sdmmc2_start_cmd(priv, cmd, cmdat, &ctx);

	debug("%s: send cmd %d data: 0x%x @ 0x%x\n",
	      __func__, cmd->cmdidx,
	      data ? ctx.data_length : 0, (unsigned int)data);

	ret = stm32_sdmmc2_end_cmd(priv, cmd, &ctx);

	if (data && !ret)
		ret = stm32_sdmmc2_end_data(priv, cmd, data, &ctx);

	/* Clear flags */
	writel(SDMMC_ICR_STATIC_FLAGS, priv->base + SDMMC_ICR);
	if (data)
		writel(0x0, priv->base + SDMMC_IDMACTRL);

	/*
	 * To stop Data Path State Machine, a stop_transmission command
	 * shall be send on cmd or data errors.
	 */
	if (ctx.dpsm_abort && (cmd->cmdidx != MMC_CMD_STOP_TRANSMISSION)) {
		struct mmc_cmd stop_cmd;

		stop_cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		stop_cmd.cmdarg = 0;
		stop_cmd.resp_type = MMC_RSP_R1b;

		debug("%s: send STOP command to abort dpsm treatments\n",
		      __func__);

		ctx.data_length = 0;

		stm32_sdmmc2_start_cmd(priv, &stop_cmd,
				       SDMMC_CMD_CMDSTOP, &ctx);
		stm32_sdmmc2_end_cmd(priv, &stop_cmd, &ctx);

		writel(SDMMC_ICR_STATIC_FLAGS, priv->base + SDMMC_ICR);
	}

	if ((ret != -ETIMEDOUT) && (ret != 0) && retry) {
		printf("%s: cmd %d failed, retrying ...\n",
		       __func__, cmd->cmdidx);
		retry--;
		goto retry_cmd;
	}

	debug("%s: end for CMD %d, ret = %d\n", __func__, cmd->cmdidx, ret);

	return ret;
}

/*
 * Reset the SDMMC with the RCC.SDMMCxRST register bit.
 * This will reset the SDMMC to the reset state and the CPSM and DPSM
 * to the Idle state. SDMMC is disabled, Signals Hiz.
 */
static void stm32_sdmmc2_reset(struct stm32_sdmmc2_priv *priv)
{
	/* Reset */
	reset_assert(&priv->reset_ctl);
	udelay(2);
	reset_deassert(&priv->reset_ctl);

	/* init the needed SDMMC register after reset */
	writel(priv->pwr_reg_msk, priv->base + SDMMC_POWER);
}

/*
 * Set the SDMMC in power-cycle state.
 * This will make that the SDMMC_D[7:0],
 * SDMMC_CMD and SDMMC_CK are driven low, to prevent the card from being
 * supplied through the signal lines.
 */
static void stm32_sdmmc2_pwrcycle(struct stm32_sdmmc2_priv *priv)
{
	if ((readl(priv->base + SDMMC_POWER) & SDMMC_POWER_PWRCTRL_MASK) ==
	    SDMMC_POWER_PWRCTRL_CYCLE)
		return;

	stm32_sdmmc2_reset(priv);
	writel(SDMMC_POWER_PWRCTRL_CYCLE | priv->pwr_reg_msk,
	       priv->base + SDMMC_POWER);
}

/*
 * set the SDMMC state Power-on: the card is clocked
 * manage the SDMMC state control:
 * Reset => Power-Cycle => Power-Off => Power
 *    PWRCTRL=10     PWCTRL=00    PWCTRL=11
 */
static void stm32_sdmmc2_pwron(struct stm32_sdmmc2_priv *priv)
{
	u32 pwrctrl =
		readl(priv->base + SDMMC_POWER) &  SDMMC_POWER_PWRCTRL_MASK;

	if (pwrctrl == SDMMC_POWER_PWRCTRL_ON)
		return;

	/* warning: same PWRCTRL value after reset and for power-off state
	 * it is the reset state here = the only managed by the driver
	 */
	if (pwrctrl == SDMMC_POWER_PWRCTRL_OFF) {
		writel(SDMMC_POWER_PWRCTRL_CYCLE | priv->pwr_reg_msk,
		       priv->base + SDMMC_POWER);
	}

	/*
	 * the remaining case is SDMMC_POWER_PWRCTRL_CYCLE
	 * switch to Power-Off state: SDMCC disable, signals drive 1
	 */
	writel(SDMMC_POWER_PWRCTRL_OFF | priv->pwr_reg_msk,
	       priv->base + SDMMC_POWER);

	/* After the 1ms delay set the SDMMC to power-on */
	mdelay(1);
	writel(SDMMC_POWER_PWRCTRL_ON | priv->pwr_reg_msk,
	       priv->base + SDMMC_POWER);

	/* during the first 74 SDMMC_CK cycles the SDMMC is still disabled. */
}

#define IS_RISING_EDGE(reg) (reg & SDMMC_CLKCR_NEGEDGE ? 0 : 1)
static int stm32_sdmmc2_set_ios(struct udevice *dev)
{
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	struct stm32_sdmmc2_priv *priv = dev_get_priv(dev);
	u32 desired = mmc->clock;
	u32 sys_clock = clk_get_rate(&priv->clk);
	u32 clk = 0;

	debug("%s: bus_with = %d, clock = %d\n", __func__,
	      mmc->bus_width, mmc->clock);

	if (mmc->clk_disable)
		stm32_sdmmc2_pwrcycle(priv);
	else
		stm32_sdmmc2_pwron(priv);

	/*
	 * clk_div = 0 => command and data generated on SDMMCCLK falling edge
	 * clk_div > 0 and NEGEDGE = 0 => command and data generated on
	 * SDMMCCLK rising edge
	 * clk_div > 0 and NEGEDGE = 1 => command and data generated on
	 * SDMMCCLK falling edge
	 */
	if (desired && ((sys_clock > desired) ||
			IS_RISING_EDGE(priv->clk_reg_msk))) {
		clk = DIV_ROUND_UP(sys_clock, 2 * desired);
		if (clk > SDMMC_CLKCR_CLKDIV_MAX)
			clk = SDMMC_CLKCR_CLKDIV_MAX;
	}

	if (mmc->bus_width == 4)
		clk |= SDMMC_CLKCR_WIDBUS_4;
	if (mmc->bus_width == 8)
		clk |= SDMMC_CLKCR_WIDBUS_8;

	writel(clk | priv->clk_reg_msk | SDMMC_CLKCR_HWFC_EN,
	       priv->base + SDMMC_CLKCR);

	return 0;
}

static int stm32_sdmmc2_getcd(struct udevice *dev)
{
	struct stm32_sdmmc2_priv *priv = dev_get_priv(dev);

	debug("stm32_sdmmc2_getcd called\n");

	if (dm_gpio_is_valid(&priv->cd_gpio))
		return dm_gpio_get_value(&priv->cd_gpio);

	return 1;
}

static const struct dm_mmc_ops stm32_sdmmc2_ops = {
	.send_cmd = stm32_sdmmc2_send_cmd,
	.set_ios = stm32_sdmmc2_set_ios,
	.get_cd = stm32_sdmmc2_getcd,
};

static int stm32_sdmmc2_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct stm32_sdmmc2_plat *plat = dev_get_platdata(dev);
	struct stm32_sdmmc2_priv *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	if (dev_read_bool(dev, "st,neg-edge"))
		priv->clk_reg_msk |= SDMMC_CLKCR_NEGEDGE;
	if (dev_read_bool(dev, "st,sig-dir"))
		priv->pwr_reg_msk |= SDMMC_POWER_DIRPOL;
	if (dev_read_bool(dev, "st,use-ckin"))
		priv->clk_reg_msk |= SDMMC_CLKCR_SELCLKRX_CKIN;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		goto clk_free;

	ret = reset_get_by_index(dev, 0, &priv->reset_ctl);
	if (ret)
		goto clk_disable;

	gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio,
			     GPIOD_IS_IN);

	cfg->f_min = 400000;
	cfg->f_max = dev_read_u32_default(dev, "max-frequency", 52000000);
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
	cfg->name = "STM32 SDMMC2";

	cfg->host_caps = 0;
	if (cfg->f_max > 25000000)
		cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	switch (dev_read_u32_default(dev, "bus-width", 1)) {
	case 8:
		cfg->host_caps |= MMC_MODE_8BIT;
	case 4:
		cfg->host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		pr_err("invalid \"bus-width\" property, force to 1\n");
	}

	upriv->mmc = &plat->mmc;

	/* SDMMC init */
	stm32_sdmmc2_reset(priv);
	return 0;

clk_disable:
	clk_disable(&priv->clk);
clk_free:
	clk_free(&priv->clk);

	return ret;
}

int stm32_sdmmc_bind(struct udevice *dev)
{
	struct stm32_sdmmc2_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id stm32_sdmmc2_ids[] = {
	{ .compatible = "st,stm32-sdmmc2" },
	{ }
};

U_BOOT_DRIVER(stm32_sdmmc2) = {
	.name = "stm32_sdmmc2",
	.id = UCLASS_MMC,
	.of_match = stm32_sdmmc2_ids,
	.ops = &stm32_sdmmc2_ops,
	.probe = stm32_sdmmc2_probe,
	.bind = stm32_sdmmc_bind,
	.priv_auto_alloc_size = sizeof(struct stm32_sdmmc2_priv),
	.platdata_auto_alloc_size = sizeof(struct stm32_sdmmc2_plat),
};
