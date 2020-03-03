/*
 * AT86RF230/RF231 driver
 *
 * Copyright (C) 2009-2012 Siemens AG
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Written by:
 * Dmitry Eremin-Solenikov <dbaryshkov@gmail.com>
 * Alexander Smirnov <alex.bluesman.smirnov@gmail.com>
 * Alexander Aring <aar@pengutronix.de>
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/at86rf230.h>
#include <linux/regmap.h>
#include <linux/skbuff.h>
#include <linux/of_gpio.h>
#include <linux/ieee802154.h>

#include <net/mac802154.h>
#include <net/cfg802154.h>

struct at86rf230_local;
/* at86rf2xx chip depend data.
 * All timings are in us.
 */
struct at86rf2xx_chip_data {
	u16 t_sleep_cycle;
	u16 t_channel_switch;
	u16 t_reset_to_off;
	u16 t_off_to_aack;
	u16 t_off_to_tx_on;
	u16 t_frame;
	u16 t_p_ack;
	int rssi_base_val;

	int (*set_channel)(struct at86rf230_local *, u8, u8);
	int (*get_desense_steps)(struct at86rf230_local *, s32);
};

#define AT86RF2XX_MAX_BUF		(127 + 3)
/* tx retries to access the TX_ON state
 * if it's above then force change will be started.
 *
 * We assume the max_frame_retries (7) value of 802.15.4 here.
 */
#define AT86RF2XX_MAX_TX_RETRIES	7
/* We use the recommended 5 minutes timeout to recalibrate */
#define AT86RF2XX_CAL_LOOP_TIMEOUT	(5 * 60 * HZ)

struct at86rf230_state_change {
	struct at86rf230_local *lp;
	int irq;

	struct hrtimer timer;
	struct spi_message msg;
	struct spi_transfer trx;
	u8 buf[AT86RF2XX_MAX_BUF];

	void (*complete)(void *context);
	u8 from_state;
	u8 to_state;

	bool irq_enable;
};

struct at86rf230_local {
	struct spi_device *spi;

	struct ieee802154_hw *hw;
	struct at86rf2xx_chip_data *data;
	struct regmap *regmap;
	int slp_tr;

	struct completion state_complete;
	struct at86rf230_state_change state;

	struct at86rf230_state_change irq;

	bool tx_aret;
	unsigned long cal_timeout;
	s8 max_frame_retries;
	bool is_tx;
	bool is_tx_from_off;
	u8 tx_retry;
	struct sk_buff *tx_skb;
	struct at86rf230_state_change tx;
};

#define RG_TRX_STATUS	(0x01)
#define SR_TRX_STATUS		0x01, 0x1f, 0
#define SR_RESERVED_01_3	0x01, 0x20, 5
#define SR_CCA_STATUS		0x01, 0x40, 6
#define SR_CCA_DONE		0x01, 0x80, 7
#define RG_TRX_STATE	(0x02)
#define SR_TRX_CMD		0x02, 0x1f, 0
#define SR_TRAC_STATUS		0x02, 0xe0, 5
#define RG_TRX_CTRL_0	(0x03)
#define SR_CLKM_CTRL		0x03, 0x07, 0
#define SR_CLKM_SHA_SEL		0x03, 0x08, 3
#define SR_PAD_IO_CLKM		0x03, 0x30, 4
#define SR_PAD_IO		0x03, 0xc0, 6
#define RG_TRX_CTRL_1	(0x04)
#define SR_IRQ_POLARITY		0x04, 0x01, 0
#define SR_IRQ_MASK_MODE	0x04, 0x02, 1
#define SR_SPI_CMD_MODE		0x04, 0x0c, 2
#define SR_RX_BL_CTRL		0x04, 0x10, 4
#define SR_TX_AUTO_CRC_ON	0x04, 0x20, 5
#define SR_IRQ_2_EXT_EN		0x04, 0x40, 6
#define SR_PA_EXT_EN		0x04, 0x80, 7
#define RG_PHY_TX_PWR	(0x05)
#define SR_TX_PWR		0x05, 0x0f, 0
#define SR_PA_LT		0x05, 0x30, 4
#define SR_PA_BUF_LT		0x05, 0xc0, 6
#define RG_PHY_RSSI	(0x06)
#define SR_RSSI			0x06, 0x1f, 0
#define SR_RND_VALUE		0x06, 0x60, 5
#define SR_RX_CRC_VALID		0x06, 0x80, 7
#define RG_PHY_ED_LEVEL	(0x07)
#define SR_ED_LEVEL		0x07, 0xff, 0
#define RG_PHY_CC_CCA	(0x08)
#define SR_CHANNEL		0x08, 0x1f, 0
#define SR_CCA_MODE		0x08, 0x60, 5
#define SR_CCA_REQUEST		0x08, 0x80, 7
#define RG_CCA_THRES	(0x09)
#define SR_CCA_ED_THRES		0x09, 0x0f, 0
#define SR_RESERVED_09_1	0x09, 0xf0, 4
#define RG_RX_CTRL	(0x0a)
#define SR_PDT_THRES		0x0a, 0x0f, 0
#define SR_RESERVED_0a_1	0x0a, 0xf0, 4
#define RG_SFD_VALUE	(0x0b)
#define SR_SFD_VALUE		0x0b, 0xff, 0
#define RG_TRX_CTRL_2	(0x0c)
#define SR_OQPSK_DATA_RATE	0x0c, 0x03, 0
#define SR_SUB_MODE		0x0c, 0x04, 2
#define SR_BPSK_QPSK		0x0c, 0x08, 3
#define SR_OQPSK_SUB1_RC_EN	0x0c, 0x10, 4
#define SR_RESERVED_0c_5	0x0c, 0x60, 5
#define SR_RX_SAFE_MODE		0x0c, 0x80, 7
#define RG_ANT_DIV	(0x0d)
#define SR_ANT_CTRL		0x0d, 0x03, 0
#define SR_ANT_EXT_SW_EN	0x0d, 0x04, 2
#define SR_ANT_DIV_EN		0x0d, 0x08, 3
#define SR_RESERVED_0d_2	0x0d, 0x70, 4
#define SR_ANT_SEL		0x0d, 0x80, 7
#define RG_IRQ_MASK	(0x0e)
#define SR_IRQ_MASK		0x0e, 0xff, 0
#define RG_IRQ_STATUS	(0x0f)
#define SR_IRQ_0_PLL_LOCK	0x0f, 0x01, 0
#define SR_IRQ_1_PLL_UNLOCK	0x0f, 0x02, 1
#define SR_IRQ_2_RX_START	0x0f, 0x04, 2
#define SR_IRQ_3_TRX_END	0x0f, 0x08, 3
#define SR_IRQ_4_CCA_ED_DONE	0x0f, 0x10, 4
#define SR_IRQ_5_AMI		0x0f, 0x20, 5
#define SR_IRQ_6_TRX_UR		0x0f, 0x40, 6
#define SR_IRQ_7_BAT_LOW	0x0f, 0x80, 7
#define RG_VREG_CTRL	(0x10)
#define SR_RESERVED_10_6	0x10, 0x03, 0
#define SR_DVDD_OK		0x10, 0x04, 2
#define SR_DVREG_EXT		0x10, 0x08, 3
#define SR_RESERVED_10_3	0x10, 0x30, 4
#define SR_AVDD_OK		0x10, 0x40, 6
#define SR_AVREG_EXT		0x10, 0x80, 7
#define RG_BATMON	(0x11)
#define SR_BATMON_VTH		0x11, 0x0f, 0
#define SR_BATMON_HR		0x11, 0x10, 4
#define SR_BATMON_OK		0x11, 0x20, 5
#define SR_RESERVED_11_1	0x11, 0xc0, 6
#define RG_XOSC_CTRL	(0x12)
#define SR_XTAL_TRIM		0x12, 0x0f, 0
#define SR_XTAL_MODE		0x12, 0xf0, 4
#define RG_RX_SYN	(0x15)
#define SR_RX_PDT_LEVEL		0x15, 0x0f, 0
#define SR_RESERVED_15_2	0x15, 0x70, 4
#define SR_RX_PDT_DIS		0x15, 0x80, 7
#define RG_XAH_CTRL_1	(0x17)
#define SR_RESERVED_17_8	0x17, 0x01, 0
#define SR_AACK_PROM_MODE	0x17, 0x02, 1
#define SR_AACK_ACK_TIME	0x17, 0x04, 2
#define SR_RESERVED_17_5	0x17, 0x08, 3
#define SR_AACK_UPLD_RES_FT	0x17, 0x10, 4
#define SR_AACK_FLTR_RES_FT	0x17, 0x20, 5
#define SR_CSMA_LBT_MODE	0x17, 0x40, 6
#define SR_RESERVED_17_1	0x17, 0x80, 7
#define RG_FTN_CTRL	(0x18)
#define SR_RESERVED_18_2	0x18, 0x7f, 0
#define SR_FTN_START		0x18, 0x80, 7
#define RG_PLL_CF	(0x1a)
#define SR_RESERVED_1a_2	0x1a, 0x7f, 0
#define SR_PLL_CF_START		0x1a, 0x80, 7
#define RG_PLL_DCU	(0x1b)
#define SR_RESERVED_1b_3	0x1b, 0x3f, 0
#define SR_RESERVED_1b_2	0x1b, 0x40, 6
#define SR_PLL_DCU_START	0x1b, 0x80, 7
#define RG_PART_NUM	(0x1c)
#define SR_PART_NUM		0x1c, 0xff, 0
#define RG_VERSION_NUM	(0x1d)
#define SR_VERSION_NUM		0x1d, 0xff, 0
#define RG_MAN_ID_0	(0x1e)
#define SR_MAN_ID_0		0x1e, 0xff, 0
#define RG_MAN_ID_1	(0x1f)
#define SR_MAN_ID_1		0x1f, 0xff, 0
#define RG_SHORT_ADDR_0	(0x20)
#define SR_SHORT_ADDR_0		0x20, 0xff, 0
#define RG_SHORT_ADDR_1	(0x21)
#define SR_SHORT_ADDR_1		0x21, 0xff, 0
#define RG_PAN_ID_0	(0x22)
#define SR_PAN_ID_0		0x22, 0xff, 0
#define RG_PAN_ID_1	(0x23)
#define SR_PAN_ID_1		0x23, 0xff, 0
#define RG_IEEE_ADDR_0	(0x24)
#define SR_IEEE_ADDR_0		0x24, 0xff, 0
#define RG_IEEE_ADDR_1	(0x25)
#define SR_IEEE_ADDR_1		0x25, 0xff, 0
#define RG_IEEE_ADDR_2	(0x26)
#define SR_IEEE_ADDR_2		0x26, 0xff, 0
#define RG_IEEE_ADDR_3	(0x27)
#define SR_IEEE_ADDR_3		0x27, 0xff, 0
#define RG_IEEE_ADDR_4	(0x28)
#define SR_IEEE_ADDR_4		0x28, 0xff, 0
#define RG_IEEE_ADDR_5	(0x29)
#define SR_IEEE_ADDR_5		0x29, 0xff, 0
#define RG_IEEE_ADDR_6	(0x2a)
#define SR_IEEE_ADDR_6		0x2a, 0xff, 0
#define RG_IEEE_ADDR_7	(0x2b)
#define SR_IEEE_ADDR_7		0x2b, 0xff, 0
#define RG_XAH_CTRL_0	(0x2c)
#define SR_SLOTTED_OPERATION	0x2c, 0x01, 0
#define SR_MAX_CSMA_RETRIES	0x2c, 0x0e, 1
#define SR_MAX_FRAME_RETRIES	0x2c, 0xf0, 4
#define RG_CSMA_SEED_0	(0x2d)
#define SR_CSMA_SEED_0		0x2d, 0xff, 0
#define RG_CSMA_SEED_1	(0x2e)
#define SR_CSMA_SEED_1		0x2e, 0x07, 0
#define SR_AACK_I_AM_COORD	0x2e, 0x08, 3
#define SR_AACK_DIS_ACK		0x2e, 0x10, 4
#define SR_AACK_SET_PD		0x2e, 0x20, 5
#define SR_AACK_FVN_MODE	0x2e, 0xc0, 6
#define RG_CSMA_BE	(0x2f)
#define SR_MIN_BE		0x2f, 0x0f, 0
#define SR_MAX_BE		0x2f, 0xf0, 4

#define CMD_REG		0x80
#define CMD_REG_MASK	0x3f
#define CMD_WRITE	0x40
#define CMD_FB		0x20

#define IRQ_BAT_LOW	(1 << 7)
#define IRQ_TRX_UR	(1 << 6)
#define IRQ_AMI		(1 << 5)
#define IRQ_CCA_ED	(1 << 4)
#define IRQ_TRX_END	(1 << 3)
#define IRQ_RX_START	(1 << 2)
#define IRQ_PLL_UNL	(1 << 1)
#define IRQ_PLL_LOCK	(1 << 0)

#define IRQ_ACTIVE_HIGH	0
#define IRQ_ACTIVE_LOW	1

#define STATE_P_ON		0x00	/* BUSY */
#define STATE_BUSY_RX		0x01
#define STATE_BUSY_TX		0x02
#define STATE_FORCE_TRX_OFF	0x03
#define STATE_FORCE_TX_ON	0x04	/* IDLE */
/* 0x05 */				/* INVALID_PARAMETER */
#define STATE_RX_ON		0x06
/* 0x07 */				/* SUCCESS */
#define STATE_TRX_OFF		0x08
#define STATE_TX_ON		0x09
/* 0x0a - 0x0e */			/* 0x0a - UNSUPPORTED_ATTRIBUTE */
#define STATE_SLEEP		0x0F
#define STATE_PREP_DEEP_SLEEP	0x10
#define STATE_BUSY_RX_AACK	0x11
#define STATE_BUSY_TX_ARET	0x12
#define STATE_RX_AACK_ON	0x16
#define STATE_TX_ARET_ON	0x19
#define STATE_RX_ON_NOCLK	0x1C
#define STATE_RX_AACK_ON_NOCLK	0x1D
#define STATE_BUSY_RX_AACK_NOCLK 0x1E
#define STATE_TRANSITION_IN_PROGRESS 0x1F

#define TRX_STATE_MASK		(0x1F)

#define AT86RF2XX_NUMREGS 0x3F

static void
at86rf230_async_state_change(struct at86rf230_local *lp,
			     struct at86rf230_state_change *ctx,
			     const u8 state, void (*complete)(void *context),
			     const bool irq_enable);

static inline int
__at86rf230_write(struct at86rf230_local *lp,
		  unsigned int addr, unsigned int data)
{
	return regmap_write(lp->regmap, addr, data);
}

static inline int
__at86rf230_read(struct at86rf230_local *lp,
		 unsigned int addr, unsigned int *data)
{
	return regmap_read(lp->regmap, addr, data);
}

static inline int
at86rf230_read_subreg(struct at86rf230_local *lp,
		      unsigned int addr, unsigned int mask,
		      unsigned int shift, unsigned int *data)
{
	int rc;

	rc = __at86rf230_read(lp, addr, data);
	if (!rc)
		*data = (*data & mask) >> shift;

	return rc;
}

static inline int
at86rf230_write_subreg(struct at86rf230_local *lp,
		       unsigned int addr, unsigned int mask,
		       unsigned int shift, unsigned int data)
{
	return regmap_update_bits(lp->regmap, addr, mask, data << shift);
}

static inline void
at86rf230_slp_tr_rising_edge(struct at86rf230_local *lp)
{
	gpio_set_value(lp->slp_tr, 1);
	udelay(1);
	gpio_set_value(lp->slp_tr, 0);
}

static bool
at86rf230_reg_writeable(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case RG_TRX_STATE:
	case RG_TRX_CTRL_0:
	case RG_TRX_CTRL_1:
	case RG_PHY_TX_PWR:
	case RG_PHY_ED_LEVEL:
	case RG_PHY_CC_CCA:
	case RG_CCA_THRES:
	case RG_RX_CTRL:
	case RG_SFD_VALUE:
	case RG_TRX_CTRL_2:
	case RG_ANT_DIV:
	case RG_IRQ_MASK:
	case RG_VREG_CTRL:
	case RG_BATMON:
	case RG_XOSC_CTRL:
	case RG_RX_SYN:
	case RG_XAH_CTRL_1:
	case RG_FTN_CTRL:
	case RG_PLL_CF:
	case RG_PLL_DCU:
	case RG_SHORT_ADDR_0:
	case RG_SHORT_ADDR_1:
	case RG_PAN_ID_0:
	case RG_PAN_ID_1:
	case RG_IEEE_ADDR_0:
	case RG_IEEE_ADDR_1:
	case RG_IEEE_ADDR_2:
	case RG_IEEE_ADDR_3:
	case RG_IEEE_ADDR_4:
	case RG_IEEE_ADDR_5:
	case RG_IEEE_ADDR_6:
	case RG_IEEE_ADDR_7:
	case RG_XAH_CTRL_0:
	case RG_CSMA_SEED_0:
	case RG_CSMA_SEED_1:
	case RG_CSMA_BE:
		return true;
	default:
		return false;
	}
}

static bool
at86rf230_reg_readable(struct device *dev, unsigned int reg)
{
	bool rc;

	/* all writeable are also readable */
	rc = at86rf230_reg_writeable(dev, reg);
	if (rc)
		return rc;

	/* readonly regs */
	switch (reg) {
	case RG_TRX_STATUS:
	case RG_PHY_RSSI:
	case RG_IRQ_STATUS:
	case RG_PART_NUM:
	case RG_VERSION_NUM:
	case RG_MAN_ID_1:
	case RG_MAN_ID_0:
		return true;
	default:
		return false;
	}
}

static bool
at86rf230_reg_volatile(struct device *dev, unsigned int reg)
{
	/* can be changed during runtime */
	switch (reg) {
	case RG_TRX_STATUS:
	case RG_TRX_STATE:
	case RG_PHY_RSSI:
	case RG_PHY_ED_LEVEL:
	case RG_IRQ_STATUS:
	case RG_VREG_CTRL:
	case RG_PLL_CF:
	case RG_PLL_DCU:
		return true;
	default:
		return false;
	}
}

static bool
at86rf230_reg_precious(struct device *dev, unsigned int reg)
{
	/* don't clear irq line on read */
	switch (reg) {
	case RG_IRQ_STATUS:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config at86rf230_regmap_spi_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.write_flag_mask = CMD_REG | CMD_WRITE,
	.read_flag_mask = CMD_REG,
	.cache_type = REGCACHE_RBTREE,
	.max_register = AT86RF2XX_NUMREGS,
	.writeable_reg = at86rf230_reg_writeable,
	.readable_reg = at86rf230_reg_readable,
	.volatile_reg = at86rf230_reg_volatile,
	.precious_reg = at86rf230_reg_precious,
};

static void
at86rf230_async_error_recover(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	lp->is_tx = 0;
	at86rf230_async_state_change(lp, ctx, STATE_RX_AACK_ON, NULL, false);
	ieee802154_wake_queue(lp->hw);
}

static inline void
at86rf230_async_error(struct at86rf230_local *lp,
		      struct at86rf230_state_change *ctx, int rc)
{
	dev_err(&lp->spi->dev, "spi_async error %d\n", rc);

	at86rf230_async_state_change(lp, ctx, STATE_FORCE_TRX_OFF,
				     at86rf230_async_error_recover, false);
}

/* Generic function to get some register value in async mode */
static void
at86rf230_async_read_reg(struct at86rf230_local *lp, const u8 reg,
			 struct at86rf230_state_change *ctx,
			 void (*complete)(void *context),
			 const bool irq_enable)
{
	int rc;

	u8 *tx_buf = ctx->buf;

	tx_buf[0] = (reg & CMD_REG_MASK) | CMD_REG;
	ctx->msg.complete = complete;
	ctx->irq_enable = irq_enable;
	rc = spi_async(lp->spi, &ctx->msg);
	if (rc) {
		if (irq_enable)
			enable_irq(ctx->irq);

		at86rf230_async_error(lp, ctx, rc);
	}
}

static inline u8 at86rf230_state_to_force(u8 state)
{
	if (state == STATE_TX_ON)
		return STATE_FORCE_TX_ON;
	else
		return STATE_FORCE_TRX_OFF;
}

static void
at86rf230_async_state_assert(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	const u8 *buf = ctx->buf;
	const u8 trx_state = buf[1] & TRX_STATE_MASK;

	/* Assert state change */
	if (trx_state != ctx->to_state) {
		/* Special handling if transceiver state is in
		 * STATE_BUSY_RX_AACK and a SHR was detected.
		 */
		if  (trx_state == STATE_BUSY_RX_AACK) {
			/* Undocumented race condition. If we send a state
			 * change to STATE_RX_AACK_ON the transceiver could
			 * change his state automatically to STATE_BUSY_RX_AACK
			 * if a SHR was detected. This is not an error, but we
			 * can't assert this.
			 */
			if (ctx->to_state == STATE_RX_AACK_ON)
				goto done;

			/* If we change to STATE_TX_ON without forcing and
			 * transceiver state is STATE_BUSY_RX_AACK, we wait
			 * 'tFrame + tPAck' receiving time. In this time the
			 * PDU should be received. If the transceiver is still
			 * in STATE_BUSY_RX_AACK, we run a force state change
			 * to STATE_TX_ON. This is a timeout handling, if the
			 * transceiver stucks in STATE_BUSY_RX_AACK.
			 *
			 * Additional we do several retries to try to get into
			 * TX_ON state without forcing. If the retries are
			 * higher or equal than AT86RF2XX_MAX_TX_RETRIES we
			 * will do a force change.
			 */
			if (ctx->to_state == STATE_TX_ON ||
			    ctx->to_state == STATE_TRX_OFF) {
				u8 state = ctx->to_state;

				if (lp->tx_retry >= AT86RF2XX_MAX_TX_RETRIES)
					state = at86rf230_state_to_force(state);
				lp->tx_retry++;

				at86rf230_async_state_change(lp, ctx, state,
							     ctx->complete,
							     ctx->irq_enable);
				return;
			}
		}

		dev_warn(&lp->spi->dev, "unexcept state change from 0x%02x to 0x%02x. Actual state: 0x%02x\n",
			 ctx->from_state, ctx->to_state, trx_state);
	}

done:
	if (ctx->complete)
		ctx->complete(context);
}

static enum hrtimer_restart at86rf230_async_state_timer(struct hrtimer *timer)
{
	struct at86rf230_state_change *ctx =
		container_of(timer, struct at86rf230_state_change, timer);
	struct at86rf230_local *lp = ctx->lp;

	at86rf230_async_read_reg(lp, RG_TRX_STATUS, ctx,
				 at86rf230_async_state_assert,
				 ctx->irq_enable);

	return HRTIMER_NORESTART;
}

/* Do state change timing delay. */
static void
at86rf230_async_state_delay(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	struct at86rf2xx_chip_data *c = lp->data;
	bool force = false;
	ktime_t tim;

	/* The force state changes are will show as normal states in the
	 * state status subregister. We change the to_state to the
	 * corresponding one and remember if it was a force change, this
	 * differs if we do a state change from STATE_BUSY_RX_AACK.
	 */
	switch (ctx->to_state) {
	case STATE_FORCE_TX_ON:
		ctx->to_state = STATE_TX_ON;
		force = true;
		break;
	case STATE_FORCE_TRX_OFF:
		ctx->to_state = STATE_TRX_OFF;
		force = true;
		break;
	default:
		break;
	}

	switch (ctx->from_state) {
	case STATE_TRX_OFF:
		switch (ctx->to_state) {
		case STATE_RX_AACK_ON:
			tim = ktime_set(0, c->t_off_to_aack * NSEC_PER_USEC);
			/* state change from TRX_OFF to RX_AACK_ON to do a
			 * calibration, we need to reset the timeout for the
			 * next one.
			 */
			lp->cal_timeout = jiffies + AT86RF2XX_CAL_LOOP_TIMEOUT;
			goto change;
		case STATE_TX_ARET_ON:
		case STATE_TX_ON:
			tim = ktime_set(0, c->t_off_to_tx_on * NSEC_PER_USEC);
			/* state change from TRX_OFF to TX_ON or ARET_ON to do
			 * a calibration, we need to reset the timeout for the
			 * next one.
			 */
			lp->cal_timeout = jiffies + AT86RF2XX_CAL_LOOP_TIMEOUT;
			goto change;
		default:
			break;
		}
		break;
	case STATE_BUSY_RX_AACK:
		switch (ctx->to_state) {
		case STATE_TRX_OFF:
		case STATE_TX_ON:
			/* Wait for worst case receiving time if we
			 * didn't make a force change from BUSY_RX_AACK
			 * to TX_ON or TRX_OFF.
			 */
			if (!force) {
				tim = ktime_set(0, (c->t_frame + c->t_p_ack) *
						   NSEC_PER_USEC);
				goto change;
			}
			break;
		default:
			break;
		}
		break;
	/* Default value, means RESET state */
	case STATE_P_ON:
		switch (ctx->to_state) {
		case STATE_TRX_OFF:
			tim = ktime_set(0, c->t_reset_to_off * NSEC_PER_USEC);
			goto change;
		default:
			break;
		}
		break;
	default:
		break;
	}

	/* Default delay is 1us in the most cases */
	tim = ktime_set(0, NSEC_PER_USEC);

change:
	hrtimer_start(&ctx->timer, tim, HRTIMER_MODE_REL);
}

static void
at86rf230_async_state_change_start(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	u8 *buf = ctx->buf;
	const u8 trx_state = buf[1] & TRX_STATE_MASK;
	int rc;

	/* Check for "possible" STATE_TRANSITION_IN_PROGRESS */
	if (trx_state == STATE_TRANSITION_IN_PROGRESS) {
		udelay(1);
		at86rf230_async_read_reg(lp, RG_TRX_STATUS, ctx,
					 at86rf230_async_state_change_start,
					 ctx->irq_enable);
		return;
	}

	/* Check if we already are in the state which we change in */
	if (trx_state == ctx->to_state) {
		if (ctx->complete)
			ctx->complete(context);
		return;
	}

	/* Set current state to the context of state change */
	ctx->from_state = trx_state;

	/* Going into the next step for a state change which do a timing
	 * relevant delay.
	 */
	buf[0] = (RG_TRX_STATE & CMD_REG_MASK) | CMD_REG | CMD_WRITE;
	buf[1] = ctx->to_state;
	ctx->msg.complete = at86rf230_async_state_delay;
	rc = spi_async(lp->spi, &ctx->msg);
	if (rc) {
		if (ctx->irq_enable)
			enable_irq(ctx->irq);

		at86rf230_async_error(lp, ctx, rc);
	}
}

static void
at86rf230_async_state_change(struct at86rf230_local *lp,
			     struct at86rf230_state_change *ctx,
			     const u8 state, void (*complete)(void *context),
			     const bool irq_enable)
{
	/* Initialization for the state change context */
	ctx->to_state = state;
	ctx->complete = complete;
	ctx->irq_enable = irq_enable;
	at86rf230_async_read_reg(lp, RG_TRX_STATUS, ctx,
				 at86rf230_async_state_change_start,
				 irq_enable);
}

static void
at86rf230_sync_state_change_complete(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	complete(&lp->state_complete);
}

/* This function do a sync framework above the async state change.
 * Some callbacks of the IEEE 802.15.4 driver interface need to be
 * handled synchronously.
 */
static int
at86rf230_sync_state_change(struct at86rf230_local *lp, unsigned int state)
{
	unsigned long rc;

	at86rf230_async_state_change(lp, &lp->state, state,
				     at86rf230_sync_state_change_complete,
				     false);

	rc = wait_for_completion_timeout(&lp->state_complete,
					 msecs_to_jiffies(100));
	if (!rc) {
		at86rf230_async_error(lp, &lp->state, -ETIMEDOUT);
		return -ETIMEDOUT;
	}

	return 0;
}

static void
at86rf230_tx_complete(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	enable_irq(ctx->irq);

	ieee802154_xmit_complete(lp->hw, lp->tx_skb, !lp->tx_aret);
}

static void
at86rf230_tx_on(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	at86rf230_async_state_change(lp, ctx, STATE_RX_AACK_ON,
				     at86rf230_tx_complete, true);
}

static void
at86rf230_tx_trac_check(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	const u8 *buf = ctx->buf;
	const u8 trac = (buf[1] & 0xe0) >> 5;

	/* If trac status is different than zero we need to do a state change
	 * to STATE_FORCE_TRX_OFF then STATE_RX_AACK_ON to recover the
	 * transceiver.
	 */
	if (trac)
		at86rf230_async_state_change(lp, ctx, STATE_FORCE_TRX_OFF,
					     at86rf230_tx_on, true);
	else
		at86rf230_tx_on(context);
}

static void
at86rf230_tx_trac_status(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	at86rf230_async_read_reg(lp, RG_TRX_STATE, ctx,
				 at86rf230_tx_trac_check, true);
}

static void
at86rf230_rx_read_frame_complete(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	u8 rx_local_buf[AT86RF2XX_MAX_BUF];
	const u8 *buf = ctx->buf;
	struct sk_buff *skb;
	u8 len, lqi;

	len = buf[1];
	if (!ieee802154_is_valid_psdu_len(len)) {
		dev_vdbg(&lp->spi->dev, "corrupted frame received\n");
		len = IEEE802154_MTU;
	}
	lqi = buf[2 + len];

	memcpy(rx_local_buf, buf + 2, len);
	ctx->trx.len = 2;
	enable_irq(ctx->irq);

	skb = dev_alloc_skb(IEEE802154_MTU);
	if (!skb) {
		dev_vdbg(&lp->spi->dev, "failed to allocate sk_buff\n");
		return;
	}

	memcpy(skb_put(skb, len), rx_local_buf, len);
	ieee802154_rx_irqsafe(lp->hw, skb, lqi);
}

static void
at86rf230_rx_read_frame(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	u8 *buf = ctx->buf;
	int rc;

	buf[0] = CMD_FB;
	ctx->trx.len = AT86RF2XX_MAX_BUF;
	ctx->msg.complete = at86rf230_rx_read_frame_complete;
	rc = spi_async(lp->spi, &ctx->msg);
	if (rc) {
		ctx->trx.len = 2;
		enable_irq(ctx->irq);
		at86rf230_async_error(lp, ctx, rc);
	}
}

static void
at86rf230_rx_trac_check(void *context)
{
	/* Possible check on trac status here. This could be useful to make
	 * some stats why receive is failed. Not used at the moment, but it's
	 * maybe timing relevant. Datasheet doesn't say anything about this.
	 * The programming guide say do it so.
	 */

	at86rf230_rx_read_frame(context);
}

static void
at86rf230_irq_trx_end(struct at86rf230_local *lp)
{
	if (lp->is_tx) {
		lp->is_tx = 0;

		if (lp->tx_aret)
			at86rf230_async_state_change(lp, &lp->irq,
						     STATE_FORCE_TX_ON,
						     at86rf230_tx_trac_status,
						     true);
		else
			at86rf230_async_state_change(lp, &lp->irq,
						     STATE_RX_AACK_ON,
						     at86rf230_tx_complete,
						     true);
	} else {
		at86rf230_async_read_reg(lp, RG_TRX_STATE, &lp->irq,
					 at86rf230_rx_trac_check, true);
	}
}

static void
at86rf230_irq_status(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	const u8 *buf = ctx->buf;
	const u8 irq = buf[1];

	if (irq & IRQ_TRX_END) {
		at86rf230_irq_trx_end(lp);
	} else {
		enable_irq(ctx->irq);
		dev_err(&lp->spi->dev, "not supported irq %02x received\n",
			irq);
	}
}

static irqreturn_t at86rf230_isr(int irq, void *data)
{
	struct at86rf230_local *lp = data;
	struct at86rf230_state_change *ctx = &lp->irq;
	u8 *buf = ctx->buf;
	int rc;

	disable_irq_nosync(irq);

	buf[0] = (RG_IRQ_STATUS & CMD_REG_MASK) | CMD_REG;
	ctx->msg.complete = at86rf230_irq_status;
	rc = spi_async(lp->spi, &ctx->msg);
	if (rc) {
		enable_irq(irq);
		at86rf230_async_error(lp, ctx, rc);
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

static void
at86rf230_write_frame_complete(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	u8 *buf = ctx->buf;
	int rc;

	ctx->trx.len = 2;

	if (gpio_is_valid(lp->slp_tr)) {
		at86rf230_slp_tr_rising_edge(lp);
	} else {
		buf[0] = (RG_TRX_STATE & CMD_REG_MASK) | CMD_REG | CMD_WRITE;
		buf[1] = STATE_BUSY_TX;
		ctx->msg.complete = NULL;
		rc = spi_async(lp->spi, &ctx->msg);
		if (rc)
			at86rf230_async_error(lp, ctx, rc);
	}
}

static void
at86rf230_write_frame(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;
	struct sk_buff *skb = lp->tx_skb;
	u8 *buf = ctx->buf;
	int rc;

	lp->is_tx = 1;

	buf[0] = CMD_FB | CMD_WRITE;
	buf[1] = skb->len + 2;
	memcpy(buf + 2, skb->data, skb->len);
	ctx->trx.len = skb->len + 2;
	ctx->msg.complete = at86rf230_write_frame_complete;
	rc = spi_async(lp->spi, &ctx->msg);
	if (rc) {
		ctx->trx.len = 2;
		at86rf230_async_error(lp, ctx, rc);
	}
}

static void
at86rf230_xmit_tx_on(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	at86rf230_async_state_change(lp, ctx, STATE_TX_ARET_ON,
				     at86rf230_write_frame, false);
}

static void
at86rf230_xmit_start(void *context)
{
	struct at86rf230_state_change *ctx = context;
	struct at86rf230_local *lp = ctx->lp;

	/* In ARET mode we need to go into STATE_TX_ARET_ON after we
	 * are in STATE_TX_ON. The pfad differs here, so we change
	 * the complete handler.
	 */
	if (lp->tx_aret) {
		if (lp->is_tx_from_off) {
			lp->is_tx_from_off = false;
			at86rf230_async_state_change(lp, ctx, STATE_TX_ARET_ON,
						     at86rf230_xmit_tx_on,
						     false);
		} else {
			at86rf230_async_state_change(lp, ctx, STATE_TX_ON,
						     at86rf230_xmit_tx_on,
						     false);
		}
	} else {
		at86rf230_async_state_change(lp, ctx, STATE_TX_ON,
					     at86rf230_write_frame, false);
	}
}

static int
at86rf230_xmit(struct ieee802154_hw *hw, struct sk_buff *skb)
{
	struct at86rf230_local *lp = hw->priv;
	struct at86rf230_state_change *ctx = &lp->tx;

	lp->tx_skb = skb;
	lp->tx_retry = 0;

	/* After 5 minutes in PLL and the same frequency we run again the
	 * calibration loops which is recommended by at86rf2xx datasheets.
	 *
	 * The calibration is initiate by a state change from TRX_OFF
	 * to TX_ON, the lp->cal_timeout should be reinit by state_delay
	 * function then to start in the next 5 minutes.
	 */
	if (time_is_before_jiffies(lp->cal_timeout)) {
		lp->is_tx_from_off = true;
		at86rf230_async_state_change(lp, ctx, STATE_TRX_OFF,
					     at86rf230_xmit_start, false);
	} else {
		at86rf230_xmit_start(ctx);
	}

	return 0;
}

static int
at86rf230_ed(struct ieee802154_hw *hw, u8 *level)
{
	BUG_ON(!level);
	*level = 0xbe;
	return 0;
}

static int
at86rf230_start(struct ieee802154_hw *hw)
{
	return at86rf230_sync_state_change(hw->priv, STATE_RX_AACK_ON);
}

static void
at86rf230_stop(struct ieee802154_hw *hw)
{
	at86rf230_sync_state_change(hw->priv, STATE_FORCE_TRX_OFF);
}

static int
at86rf23x_set_channel(struct at86rf230_local *lp, u8 page, u8 channel)
{
	return at86rf230_write_subreg(lp, SR_CHANNEL, channel);
}

static int
at86rf212_set_channel(struct at86rf230_local *lp, u8 page, u8 channel)
{
	int rc;

	if (channel == 0)
		rc = at86rf230_write_subreg(lp, SR_SUB_MODE, 0);
	else
		rc = at86rf230_write_subreg(lp, SR_SUB_MODE, 1);
	if (rc < 0)
		return rc;

	if (page == 0) {
		rc = at86rf230_write_subreg(lp, SR_BPSK_QPSK, 0);
		lp->data->rssi_base_val = -100;
	} else {
		rc = at86rf230_write_subreg(lp, SR_BPSK_QPSK, 1);
		lp->data->rssi_base_val = -98;
	}
	if (rc < 0)
		return rc;

	/* This sets the symbol_duration according frequency on the 212.
	 * TODO move this handling while set channel and page in cfg802154.
	 * We can do that, this timings are according 802.15.4 standard.
	 * If we do that in cfg802154, this is a more generic calculation.
	 *
	 * This should also protected from ifs_timer. Means cancel timer and
	 * init with a new value. For now, this is okay.
	 */
	if (channel == 0) {
		if (page == 0) {
			/* SUB:0 and BPSK:0 -> BPSK-20 */
			lp->hw->phy->symbol_duration = 50;
		} else {
			/* SUB:1 and BPSK:0 -> BPSK-40 */
			lp->hw->phy->symbol_duration = 25;
		}
	} else {
		if (page == 0)
			/* SUB:0 and BPSK:1 -> OQPSK-100/200/400 */
			lp->hw->phy->symbol_duration = 40;
		else
			/* SUB:1 and BPSK:1 -> OQPSK-250/500/1000 */
			lp->hw->phy->symbol_duration = 16;
	}

	lp->hw->phy->lifs_period = IEEE802154_LIFS_PERIOD *
				   lp->hw->phy->symbol_duration;
	lp->hw->phy->sifs_period = IEEE802154_SIFS_PERIOD *
				   lp->hw->phy->symbol_duration;

	return at86rf230_write_subreg(lp, SR_CHANNEL, channel);
}

static int
at86rf230_channel(struct ieee802154_hw *hw, u8 page, u8 channel)
{
	struct at86rf230_local *lp = hw->priv;
	int rc;

	rc = lp->data->set_channel(lp, page, channel);
	/* Wait for PLL */
	usleep_range(lp->data->t_channel_switch,
		     lp->data->t_channel_switch + 10);

	lp->cal_timeout = jiffies + AT86RF2XX_CAL_LOOP_TIMEOUT;
	return rc;
}

static int
at86rf230_set_hw_addr_filt(struct ieee802154_hw *hw,
			   struct ieee802154_hw_addr_filt *filt,
			   unsigned long changed)
{
	struct at86rf230_local *lp = hw->priv;

	if (changed & IEEE802154_AFILT_SADDR_CHANGED) {
		u16 addr = le16_to_cpu(filt->short_addr);

		dev_vdbg(&lp->spi->dev,
			 "at86rf230_set_hw_addr_filt called for saddr\n");
		__at86rf230_write(lp, RG_SHORT_ADDR_0, addr);
		__at86rf230_write(lp, RG_SHORT_ADDR_1, addr >> 8);
	}

	if (changed & IEEE802154_AFILT_PANID_CHANGED) {
		u16 pan = le16_to_cpu(filt->pan_id);

		dev_vdbg(&lp->spi->dev,
			 "at86rf230_set_hw_addr_filt called for pan id\n");
		__at86rf230_write(lp, RG_PAN_ID_0, pan);
		__at86rf230_write(lp, RG_PAN_ID_1, pan >> 8);
	}

	if (changed & IEEE802154_AFILT_IEEEADDR_CHANGED) {
		u8 i, addr[8];

		memcpy(addr, &filt->ieee_addr, 8);
		dev_vdbg(&lp->spi->dev,
			 "at86rf230_set_hw_addr_filt called for IEEE addr\n");
		for (i = 0; i < 8; i++)
			__at86rf230_write(lp, RG_IEEE_ADDR_0 + i, addr[i]);
	}

	if (changed & IEEE802154_AFILT_PANC_CHANGED) {
		dev_vdbg(&lp->spi->dev,
			 "at86rf230_set_hw_addr_filt called for panc change\n");
		if (filt->pan_coord)
			at86rf230_write_subreg(lp, SR_AACK_I_AM_COORD, 1);
		else
			at86rf230_write_subreg(lp, SR_AACK_I_AM_COORD, 0);
	}

	return 0;
}

static int
at86rf230_set_txpower(struct ieee802154_hw *hw, s8 db)
{
	struct at86rf230_local *lp = hw->priv;

	/* typical maximum output is 5dBm with RG_PHY_TX_PWR 0x60, lower five
	 * bits decrease power in 1dB steps. 0x60 represents extra PA gain of
	 * 0dB.
	 * thus, supported values for db range from -26 to 5, for 31dB of
	 * reduction to 0dB of reduction.
	 */
	if (db > 5 || db < -26)
		return -EINVAL;

	db = -(db - 5);

	return __at86rf230_write(lp, RG_PHY_TX_PWR, 0x60 | db);
}

static int
at86rf230_set_lbt(struct ieee802154_hw *hw, bool on)
{
	struct at86rf230_local *lp = hw->priv;

	return at86rf230_write_subreg(lp, SR_CSMA_LBT_MODE, on);
}

static int
at86rf230_set_cca_mode(struct ieee802154_hw *hw,
		       const struct wpan_phy_cca *cca)
{
	struct at86rf230_local *lp = hw->priv;
	u8 val;

	/* mapping 802.15.4 to driver spec */
	switch (cca->mode) {
	case NL802154_CCA_ENERGY:
		val = 1;
		break;
	case NL802154_CCA_CARRIER:
		val = 2;
		break;
	case NL802154_CCA_ENERGY_CARRIER:
		switch (cca->opt) {
		case NL802154_CCA_OPT_ENERGY_CARRIER_AND:
			val = 3;
			break;
		case NL802154_CCA_OPT_ENERGY_CARRIER_OR:
			val = 0;
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	return at86rf230_write_subreg(lp, SR_CCA_MODE, val);
}

static int
at86rf212_get_desens_steps(struct at86rf230_local *lp, s32 level)
{
	return (level - lp->data->rssi_base_val) * 100 / 207;
}

static int
at86rf23x_get_desens_steps(struct at86rf230_local *lp, s32 level)
{
	return (level - lp->data->rssi_base_val) / 2;
}

static int
at86rf230_set_cca_ed_level(struct ieee802154_hw *hw, s32 level)
{
	struct at86rf230_local *lp = hw->priv;

	if (level < lp->data->rssi_base_val || level > 30)
		return -EINVAL;

	return at86rf230_write_subreg(lp, SR_CCA_ED_THRES,
				      lp->data->get_desense_steps(lp, level));
}

static int
at86rf230_set_csma_params(struct ieee802154_hw *hw, u8 min_be, u8 max_be,
			  u8 retries)
{
	struct at86rf230_local *lp = hw->priv;
	int rc;

	rc = at86rf230_write_subreg(lp, SR_MIN_BE, min_be);
	if (rc)
		return rc;

	rc = at86rf230_write_subreg(lp, SR_MAX_BE, max_be);
	if (rc)
		return rc;

	return at86rf230_write_subreg(lp, SR_MAX_CSMA_RETRIES, retries);
}

static int
at86rf230_set_frame_retries(struct ieee802154_hw *hw, s8 retries)
{
	struct at86rf230_local *lp = hw->priv;
	int rc = 0;

	lp->tx_aret = retries >= 0;
	lp->max_frame_retries = retries;

	if (retries >= 0)
		rc = at86rf230_write_subreg(lp, SR_MAX_FRAME_RETRIES, retries);

	return rc;
}

static int
at86rf230_set_promiscuous_mode(struct ieee802154_hw *hw, const bool on)
{
	struct at86rf230_local *lp = hw->priv;
	int rc;

	if (on) {
		rc = at86rf230_write_subreg(lp, SR_AACK_DIS_ACK, 1);
		if (rc < 0)
			return rc;

		rc = at86rf230_write_subreg(lp, SR_AACK_PROM_MODE, 1);
		if (rc < 0)
			return rc;
	} else {
		rc = at86rf230_write_subreg(lp, SR_AACK_PROM_MODE, 0);
		if (rc < 0)
			return rc;

		rc = at86rf230_write_subreg(lp, SR_AACK_DIS_ACK, 0);
		if (rc < 0)
			return rc;
	}

	return 0;
}

static const struct ieee802154_ops at86rf230_ops = {
	.owner = THIS_MODULE,
	.xmit_async = at86rf230_xmit,
	.ed = at86rf230_ed,
	.set_channel = at86rf230_channel,
	.start = at86rf230_start,
	.stop = at86rf230_stop,
	.set_hw_addr_filt = at86rf230_set_hw_addr_filt,
	.set_txpower = at86rf230_set_txpower,
	.set_lbt = at86rf230_set_lbt,
	.set_cca_mode = at86rf230_set_cca_mode,
	.set_cca_ed_level = at86rf230_set_cca_ed_level,
	.set_csma_params = at86rf230_set_csma_params,
	.set_frame_retries = at86rf230_set_frame_retries,
	.set_promiscuous_mode = at86rf230_set_promiscuous_mode,
};

static struct at86rf2xx_chip_data at86rf233_data = {
	.t_sleep_cycle = 330,
	.t_channel_switch = 11,
	.t_reset_to_off = 26,
	.t_off_to_aack = 80,
	.t_off_to_tx_on = 80,
	.t_frame = 4096,
	.t_p_ack = 545,
	.rssi_base_val = -91,
	.set_channel = at86rf23x_set_channel,
	.get_desense_steps = at86rf23x_get_desens_steps
};

static struct at86rf2xx_chip_data at86rf231_data = {
	.t_sleep_cycle = 330,
	.t_channel_switch = 24,
	.t_reset_to_off = 37,
	.t_off_to_aack = 110,
	.t_off_to_tx_on = 110,
	.t_frame = 4096,
	.t_p_ack = 545,
	.rssi_base_val = -91,
	.set_channel = at86rf23x_set_channel,
	.get_desense_steps = at86rf23x_get_desens_steps
};

static struct at86rf2xx_chip_data at86rf212_data = {
	.t_sleep_cycle = 330,
	.t_channel_switch = 11,
	.t_reset_to_off = 26,
	.t_off_to_aack = 200,
	.t_off_to_tx_on = 200,
	.t_frame = 4096,
	.t_p_ack = 545,
	.rssi_base_val = -100,
	.set_channel = at86rf212_set_channel,
	.get_desense_steps = at86rf212_get_desens_steps
};

static int at86rf230_hw_init(struct at86rf230_local *lp, u8 xtal_trim)
{
	int rc, irq_type, irq_pol = IRQ_ACTIVE_HIGH;
	unsigned int dvdd;
	u8 csma_seed[2];

	rc = at86rf230_sync_state_change(lp, STATE_FORCE_TRX_OFF);
	if (rc)
		return rc;

	irq_type = irq_get_trigger_type(lp->spi->irq);
	if (irq_type == IRQ_TYPE_EDGE_RISING ||
	    irq_type == IRQ_TYPE_EDGE_FALLING)
		dev_warn(&lp->spi->dev,
			 "Using edge triggered irq's are not recommended!\n");
	if (irq_type == IRQ_TYPE_EDGE_FALLING ||
	    irq_type == IRQ_TYPE_LEVEL_LOW)
		irq_pol = IRQ_ACTIVE_LOW;

	rc = at86rf230_write_subreg(lp, SR_IRQ_POLARITY, irq_pol);
	if (rc)
		return rc;

	rc = at86rf230_write_subreg(lp, SR_RX_SAFE_MODE, 1);
	if (rc)
		return rc;

	rc = at86rf230_write_subreg(lp, SR_IRQ_MASK, IRQ_TRX_END);
	if (rc)
		return rc;

	/* reset values differs in at86rf231 and at86rf233 */
	rc = at86rf230_write_subreg(lp, SR_IRQ_MASK_MODE, 0);
	if (rc)
		return rc;

	get_random_bytes(csma_seed, ARRAY_SIZE(csma_seed));
	rc = at86rf230_write_subreg(lp, SR_CSMA_SEED_0, csma_seed[0]);
	if (rc)
		return rc;
	rc = at86rf230_write_subreg(lp, SR_CSMA_SEED_1, csma_seed[1]);
	if (rc)
		return rc;

	/* CLKM changes are applied immediately */
	rc = at86rf230_write_subreg(lp, SR_CLKM_SHA_SEL, 0x00);
	if (rc)
		return rc;

	/* Turn CLKM Off */
	rc = at86rf230_write_subreg(lp, SR_CLKM_CTRL, 0x00);
	if (rc)
		return rc;
	/* Wait the next SLEEP cycle */
	usleep_range(lp->data->t_sleep_cycle,
		     lp->data->t_sleep_cycle + 100);

	/* xtal_trim value is calculated by:
	 * CL = 0.5 * (CX + CTRIM + CPAR)
	 *
	 * whereas:
	 * CL = capacitor of used crystal
	 * CX = connected capacitors at xtal pins
	 * CPAR = in all at86rf2xx datasheets this is a constant value 3 pF,
	 *	  but this is different on each board setup. You need to fine
	 *	  tuning this value via CTRIM.
	 * CTRIM = variable capacitor setting. Resolution is 0.3 pF range is
	 *	   0 pF upto 4.5 pF.
	 *
	 * Examples:
	 * atben transceiver:
	 *
	 * CL = 8 pF
	 * CX = 12 pF
	 * CPAR = 3 pF (We assume the magic constant from datasheet)
	 * CTRIM = 0.9 pF
	 *
	 * (12+0.9+3)/2 = 7.95 which is nearly at 8 pF
	 *
	 * xtal_trim = 0x3
	 *
	 * openlabs transceiver:
	 *
	 * CL = 16 pF
	 * CX = 22 pF
	 * CPAR = 3 pF (We assume the magic constant from datasheet)
	 * CTRIM = 4.5 pF
	 *
	 * (22+4.5+3)/2 = 14.75 which is the nearest value to 16 pF
	 *
	 * xtal_trim = 0xf
	 */
	rc = at86rf230_write_subreg(lp, SR_XTAL_TRIM, xtal_trim);
	if (rc)
		return rc;

	rc = at86rf230_read_subreg(lp, SR_DVDD_OK, &dvdd);
	if (rc)
		return rc;
	if (!dvdd) {
		dev_err(&lp->spi->dev, "DVDD error\n");
		return -EINVAL;
	}

	/* Force setting slotted operation bit to 0. Sometimes the atben
	 * sets this bit and I don't know why. We set this always force
	 * to zero while probing.
	 */
	return at86rf230_write_subreg(lp, SR_SLOTTED_OPERATION, 0);
}

static int
at86rf230_get_pdata(struct spi_device *spi, int *rstn, int *slp_tr,
		    u8 *xtal_trim)
{
	struct at86rf230_platform_data *pdata = spi->dev.platform_data;
	int ret;

	if (!IS_ENABLED(CONFIG_OF) || !spi->dev.of_node) {
		if (!pdata)
			return -ENOENT;

		*rstn = pdata->rstn;
		*slp_tr = pdata->slp_tr;
		*xtal_trim = pdata->xtal_trim;
		return 0;
	}

	*rstn = of_get_named_gpio(spi->dev.of_node, "reset-gpio", 0);
	*slp_tr = of_get_named_gpio(spi->dev.of_node, "sleep-gpio", 0);
	ret = of_property_read_u8(spi->dev.of_node, "xtal-trim", xtal_trim);
	if (ret < 0 && ret != -EINVAL)
		return ret;

	return 0;
}

static int
at86rf230_detect_device(struct at86rf230_local *lp)
{
	unsigned int part, version, val;
	u16 man_id = 0;
	const char *chip;
	int rc;

	rc = __at86rf230_read(lp, RG_MAN_ID_0, &val);
	if (rc)
		return rc;
	man_id |= val;

	rc = __at86rf230_read(lp, RG_MAN_ID_1, &val);
	if (rc)
		return rc;
	man_id |= (val << 8);

	rc = __at86rf230_read(lp, RG_PART_NUM, &part);
	if (rc)
		return rc;

	rc = __at86rf230_read(lp, RG_VERSION_NUM, &version);
	if (rc)
		return rc;

	if (man_id != 0x001f) {
		dev_err(&lp->spi->dev, "Non-Atmel dev found (MAN_ID %02x %02x)\n",
			man_id >> 8, man_id & 0xFF);
		return -EINVAL;
	}

	lp->hw->flags = IEEE802154_HW_TX_OMIT_CKSUM | IEEE802154_HW_AACK |
			IEEE802154_HW_TXPOWER | IEEE802154_HW_ARET |
			IEEE802154_HW_AFILT | IEEE802154_HW_PROMISCUOUS;

	lp->hw->phy->cca.mode = NL802154_CCA_ENERGY;

	switch (part) {
	case 2:
		chip = "at86rf230";
		rc = -ENOTSUPP;
		break;
	case 3:
		chip = "at86rf231";
		lp->data = &at86rf231_data;
		lp->hw->phy->channels_supported[0] = 0x7FFF800;
		lp->hw->phy->current_channel = 11;
		lp->hw->phy->symbol_duration = 16;
		break;
	case 7:
		chip = "at86rf212";
		lp->data = &at86rf212_data;
		lp->hw->flags |= IEEE802154_HW_LBT;
		lp->hw->phy->channels_supported[0] = 0x00007FF;
		lp->hw->phy->channels_supported[2] = 0x00007FF;
		lp->hw->phy->current_channel = 5;
		lp->hw->phy->symbol_duration = 25;
		break;
	case 11:
		chip = "at86rf233";
		lp->data = &at86rf233_data;
		lp->hw->phy->channels_supported[0] = 0x7FFF800;
		lp->hw->phy->current_channel = 13;
		lp->hw->phy->symbol_duration = 16;
		break;
	default:
		chip = "unknown";
		rc = -ENOTSUPP;
		break;
	}

	dev_info(&lp->spi->dev, "Detected %s chip version %d\n", chip, version);

	return rc;
}

static void
at86rf230_setup_spi_messages(struct at86rf230_local *lp)
{
	lp->state.lp = lp;
	lp->state.irq = lp->spi->irq;
	spi_message_init(&lp->state.msg);
	lp->state.msg.context = &lp->state;
	lp->state.trx.len = 2;
	lp->state.trx.tx_buf = lp->state.buf;
	lp->state.trx.rx_buf = lp->state.buf;
	spi_message_add_tail(&lp->state.trx, &lp->state.msg);
	hrtimer_init(&lp->state.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lp->state.timer.function = at86rf230_async_state_timer;

	lp->irq.lp = lp;
	lp->irq.irq = lp->spi->irq;
	spi_message_init(&lp->irq.msg);
	lp->irq.msg.context = &lp->irq;
	lp->irq.trx.len = 2;
	lp->irq.trx.tx_buf = lp->irq.buf;
	lp->irq.trx.rx_buf = lp->irq.buf;
	spi_message_add_tail(&lp->irq.trx, &lp->irq.msg);
	hrtimer_init(&lp->irq.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lp->irq.timer.function = at86rf230_async_state_timer;

	lp->tx.lp = lp;
	lp->tx.irq = lp->spi->irq;
	spi_message_init(&lp->tx.msg);
	lp->tx.msg.context = &lp->tx;
	lp->tx.trx.len = 2;
	lp->tx.trx.tx_buf = lp->tx.buf;
	lp->tx.trx.rx_buf = lp->tx.buf;
	spi_message_add_tail(&lp->tx.trx, &lp->tx.msg);
	hrtimer_init(&lp->tx.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lp->tx.timer.function = at86rf230_async_state_timer;
}

static int at86rf230_probe(struct spi_device *spi)
{
	struct ieee802154_hw *hw;
	struct at86rf230_local *lp;
	unsigned int status;
	int rc, irq_type, rstn, slp_tr;
	u8 xtal_trim = 0;

	if (!spi->irq) {
		dev_err(&spi->dev, "no IRQ specified\n");
		return -EINVAL;
	}

	rc = at86rf230_get_pdata(spi, &rstn, &slp_tr, &xtal_trim);
	if (rc < 0) {
		dev_err(&spi->dev, "failed to parse platform_data: %d\n", rc);
		return rc;
	}

	if (gpio_is_valid(rstn)) {
		rc = devm_gpio_request_one(&spi->dev, rstn,
					   GPIOF_OUT_INIT_HIGH, "rstn");
		if (rc)
			return rc;
	}

	if (gpio_is_valid(slp_tr)) {
		rc = devm_gpio_request_one(&spi->dev, slp_tr,
					   GPIOF_OUT_INIT_LOW, "slp_tr");
		if (rc)
			return rc;
	}

	/* Reset */
	if (gpio_is_valid(rstn)) {
		udelay(1);
		gpio_set_value(rstn, 0);
		udelay(1);
		gpio_set_value(rstn, 1);
		usleep_range(120, 240);
	}

	hw = ieee802154_alloc_hw(sizeof(*lp), &at86rf230_ops);
	if (!hw)
		return -ENOMEM;

	lp = hw->priv;
	lp->hw = hw;
	lp->spi = spi;
	lp->slp_tr = slp_tr;
	hw->parent = &spi->dev;
	hw->vif_data_size = sizeof(*lp);
	ieee802154_random_extended_addr(&hw->phy->perm_extended_addr);

	lp->regmap = devm_regmap_init_spi(spi, &at86rf230_regmap_spi_config);
	if (IS_ERR(lp->regmap)) {
		rc = PTR_ERR(lp->regmap);
		dev_err(&spi->dev, "Failed to allocate register map: %d\n",
			rc);
		goto free_dev;
	}

	at86rf230_setup_spi_messages(lp);

	rc = at86rf230_detect_device(lp);
	if (rc < 0)
		goto free_dev;

	init_completion(&lp->state_complete);

	spi_set_drvdata(spi, lp);

	rc = at86rf230_hw_init(lp, xtal_trim);
	if (rc)
		goto free_dev;

	/* Read irq status register to reset irq line */
	rc = at86rf230_read_subreg(lp, RG_IRQ_STATUS, 0xff, 0, &status);
	if (rc)
		goto free_dev;

	irq_type = irq_get_trigger_type(spi->irq);
	if (!irq_type)
		irq_type = IRQF_TRIGGER_RISING;

	rc = devm_request_irq(&spi->dev, spi->irq, at86rf230_isr,
			      IRQF_SHARED | irq_type, dev_name(&spi->dev), lp);
	if (rc)
		goto free_dev;

	rc = ieee802154_register_hw(lp->hw);
	if (rc)
		goto free_dev;

	return rc;

free_dev:
	ieee802154_free_hw(lp->hw);

	return rc;
}

static int at86rf230_remove(struct spi_device *spi)
{
	struct at86rf230_local *lp = spi_get_drvdata(spi);

	/* mask all at86rf230 irq's */
	at86rf230_write_subreg(lp, SR_IRQ_MASK, 0);
	ieee802154_unregister_hw(lp->hw);
	ieee802154_free_hw(lp->hw);
	dev_dbg(&spi->dev, "unregistered at86rf230\n");

	return 0;
}

static const struct of_device_id at86rf230_of_match[] = {
	{ .compatible = "atmel,at86rf230", },
	{ .compatible = "atmel,at86rf231", },
	{ .compatible = "atmel,at86rf233", },
	{ .compatible = "atmel,at86rf212", },
	{ },
};
MODULE_DEVICE_TABLE(of, at86rf230_of_match);

static const struct spi_device_id at86rf230_device_id[] = {
	{ .name = "at86rf230", },
	{ .name = "at86rf231", },
	{ .name = "at86rf233", },
	{ .name = "at86rf212", },
	{ },
};
MODULE_DEVICE_TABLE(spi, at86rf230_device_id);

static struct spi_driver at86rf230_driver = {
	.id_table = at86rf230_device_id,
	.driver = {
		.of_match_table = of_match_ptr(at86rf230_of_match),
		.name	= "at86rf230",
		.owner	= THIS_MODULE,
	},
	.probe      = at86rf230_probe,
	.remove     = at86rf230_remove,
};

module_spi_driver(at86rf230_driver);

MODULE_DESCRIPTION("AT86RF230 Transceiver Driver");
MODULE_LICENSE("GPL v2");
