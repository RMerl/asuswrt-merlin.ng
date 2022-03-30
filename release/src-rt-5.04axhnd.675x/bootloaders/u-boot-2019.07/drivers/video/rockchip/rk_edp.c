// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <edid.h>
#include <panel.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/edp_rk3288.h>
#include <asm/arch-rockchip/grf_rk3288.h>
#include <dt-bindings/clock/rk3288-cru.h>

#define MAX_CR_LOOP 5
#define MAX_EQ_LOOP 5
#define DP_LINK_STATUS_SIZE 6

static const char * const voltage_names[] = {
	"0.4V", "0.6V", "0.8V", "1.2V"
};
static const char * const pre_emph_names[] = {
	"0dB", "3.5dB", "6dB", "9.5dB"
};

#define DP_VOLTAGE_MAX         DP_TRAIN_VOLTAGE_SWING_1200
#define DP_PRE_EMPHASIS_MAX    DP_TRAIN_PRE_EMPHASIS_9_5

struct rk_edp_priv {
	struct rk3288_edp *regs;
	struct rk3288_grf *grf;
	struct udevice *panel;
	struct link_train link_train;
	u8 train_set[4];
};

static void rk_edp_init_refclk(struct rk3288_edp *regs)
{
	writel(SEL_24M, &regs->analog_ctl_2);
	writel(REF_CLK_24M, &regs->pll_reg_1);

	writel(LDO_OUTPUT_V_SEL_145 | KVCO_DEFALUT | CHG_PUMP_CUR_SEL_5US |
	       V2L_CUR_SEL_1MA, &regs->pll_reg_2);

	writel(LOCK_DET_CNT_SEL_256 | LOOP_FILTER_RESET | PALL_SSC_RESET |
	       LOCK_DET_BYPASS | PLL_LOCK_DET_MODE | PLL_LOCK_DET_FORCE,
	       &regs->pll_reg_3);

	writel(REGULATOR_V_SEL_950MV | STANDBY_CUR_SEL |
	       CHG_PUMP_INOUT_CTRL_1200MV | CHG_PUMP_INPUT_CTRL_OP,
	       &regs->pll_reg_5);

	writel(SSC_OFFSET | SSC_MODE | SSC_DEPTH, &regs->ssc_reg);

	writel(TX_SWING_PRE_EMP_MODE | PRE_DRIVER_PW_CTRL1 |
	       LP_MODE_CLK_REGULATOR | RESISTOR_MSB_CTRL | RESISTOR_CTRL,
	       &regs->tx_common);

	writel(DP_AUX_COMMON_MODE | DP_AUX_EN | AUX_TERM_50OHM,
	       &regs->dp_aux);

	writel(DP_BG_OUT_SEL | DP_DB_CUR_CTRL | DP_BG_SEL | DP_RESISTOR_TUNE_BG,
	       &regs->dp_bias);

	writel(CH1_CH3_SWING_EMP_CTRL | CH0_CH2_SWING_EMP_CTRL,
	       &regs->dp_reserv2);
}

static void rk_edp_init_interrupt(struct rk3288_edp *regs)
{
	/* Set interrupt pin assertion polarity as high */
	writel(INT_POL, &regs->int_ctl);

	/* Clear pending registers */
	writel(0xff, &regs->common_int_sta_1);
	writel(0x4f, &regs->common_int_sta_2);
	writel(0xff, &regs->common_int_sta_3);
	writel(0x27, &regs->common_int_sta_4);
	writel(0x7f, &regs->dp_int_sta);

	/* 0:mask,1: unmask */
	writel(0x00, &regs->common_int_mask_1);
	writel(0x00, &regs->common_int_mask_2);
	writel(0x00, &regs->common_int_mask_3);
	writel(0x00, &regs->common_int_mask_4);
	writel(0x00, &regs->int_sta_mask);
}

static void rk_edp_enable_sw_function(struct rk3288_edp *regs)
{
	clrbits_le32(&regs->func_en_1, SW_FUNC_EN_N);
}

static bool rk_edp_get_pll_locked(struct rk3288_edp *regs)
{
	u32 val;

	val = readl(&regs->dp_debug_ctl);

	return val & PLL_LOCK;
}

static int rk_edp_init_analog_func(struct rk3288_edp *regs)
{
	ulong start;

	writel(0x00, &regs->dp_pd);
	writel(PLL_LOCK_CHG, &regs->common_int_sta_1);

	clrbits_le32(&regs->dp_debug_ctl, F_PLL_LOCK | PLL_LOCK_CTRL);

	start = get_timer(0);
	while (!rk_edp_get_pll_locked(regs)) {
		if (get_timer(start) > PLL_LOCK_TIMEOUT) {
			printf("%s: PLL is not locked\n", __func__);
			return -ETIMEDOUT;
		}
	}

	/* Enable Serdes FIFO function and Link symbol clock domain module */
	clrbits_le32(&regs->func_en_2, SERDES_FIFO_FUNC_EN_N |
				       LS_CLK_DOMAIN_FUNC_EN_N | AUX_FUNC_EN_N |
				       SSC_FUNC_EN_N);

	return 0;
}

static void rk_edp_init_aux(struct rk3288_edp *regs)
{
	/* Clear inerrupts related to AUX channel */
	writel(AUX_FUNC_EN_N, &regs->dp_int_sta);

	/* Disable AUX channel module */
	setbits_le32(&regs->func_en_2, AUX_FUNC_EN_N);

	/* Receive AUX Channel DEFER commands equal to DEFFER_COUNT*64 */
	writel(DEFER_CTRL_EN | DEFER_COUNT(1), &regs->aux_ch_defer_dtl);

	/* Enable AUX channel module */
	clrbits_le32(&regs->func_en_2, AUX_FUNC_EN_N);
}

static int rk_edp_aux_enable(struct rk3288_edp *regs)
{
	ulong start;

	setbits_le32(&regs->aux_ch_ctl_2, AUX_EN);
	start = get_timer(0);
	do {
		if (!(readl(&regs->aux_ch_ctl_2) & AUX_EN))
			return 0;
	} while (get_timer(start) < 20);

	return -ETIMEDOUT;
}

static int rk_edp_is_aux_reply(struct rk3288_edp *regs)
{
	ulong start;

	start = get_timer(0);
	while (!(readl(&regs->dp_int_sta) & RPLY_RECEIV)) {
		if (get_timer(start) > 10)
			return -ETIMEDOUT;
	}

	writel(RPLY_RECEIV, &regs->dp_int_sta);

	return 0;
}

static int rk_edp_start_aux_transaction(struct rk3288_edp *regs)
{
	int val, ret;

	/* Enable AUX CH operation */
	ret = rk_edp_aux_enable(regs);
	if (ret) {
		debug("AUX CH enable timeout!\n");
		return ret;
	}

	/* Is AUX CH command reply received? */
	if (rk_edp_is_aux_reply(regs)) {
		debug("AUX CH command reply failed!\n");
		return ret;
	}

	/* Clear interrupt source for AUX CH access error */
	val = readl(&regs->dp_int_sta);
	if (val & AUX_ERR) {
		writel(AUX_ERR, &regs->dp_int_sta);
		return -EIO;
	}

	/* Check AUX CH error access status */
	val = readl(&regs->dp_int_sta);
	if (val & AUX_STATUS_MASK) {
		debug("AUX CH error happens: %d\n\n", val & AUX_STATUS_MASK);
		return -EIO;
	}

	return 0;
}

static int rk_edp_dpcd_transfer(struct rk3288_edp *regs,
				unsigned int val_addr, u8 *in_data,
				unsigned int length,
				enum dpcd_request request)
{
	int val;
	int i, try_times;
	u8 *data;
	int ret = 0;
	u32 len = 0;

	while (length) {
		len = min(length, 16U);
		for (try_times = 0; try_times < 10; try_times++) {
			data = in_data;
			/* Clear AUX CH data buffer */
			writel(BUF_CLR, &regs->buf_data_ctl);

			/* Select DPCD device address */
			writel(AUX_ADDR_7_0(val_addr), &regs->aux_addr_7_0);
			writel(AUX_ADDR_15_8(val_addr), &regs->aux_addr_15_8);
			writel(AUX_ADDR_19_16(val_addr), &regs->aux_addr_19_16);

			/*
			 * Set DisplayPort transaction and read 1 byte
			 * If bit 3 is 1, DisplayPort transaction.
			 * If Bit 3 is 0, I2C transaction.
			 */
			if (request == DPCD_WRITE) {
				val = AUX_LENGTH(len) |
					AUX_TX_COMM_DP_TRANSACTION |
					AUX_TX_COMM_WRITE;
				for (i = 0; i < len; i++)
					writel(*data++, &regs->buf_data[i]);
			} else
				val = AUX_LENGTH(len) |
					AUX_TX_COMM_DP_TRANSACTION |
					AUX_TX_COMM_READ;

			writel(val, &regs->aux_ch_ctl_1);

			/* Start AUX transaction */
			ret = rk_edp_start_aux_transaction(regs);
			if (ret == 0)
				break;
			else
				printf("read dpcd Aux Transaction fail!\n");
		}

		if (ret)
			return ret;

		if (request == DPCD_READ) {
			for (i = 0; i < len; i++)
				*data++ = (u8)readl(&regs->buf_data[i]);
		}

		length -= len;
		val_addr += len;
		in_data += len;
	}

	return 0;
}

static int rk_edp_dpcd_read(struct rk3288_edp *regs, u32 addr, u8 *values,
			    size_t size)
{
	return rk_edp_dpcd_transfer(regs, addr, values, size, DPCD_READ);
}

static int rk_edp_dpcd_write(struct rk3288_edp *regs, u32 addr, u8 *values,
			     size_t size)
{
	return rk_edp_dpcd_transfer(regs, addr, values, size, DPCD_WRITE);
}


static int rk_edp_link_power_up(struct rk_edp_priv *edp)
{
	u8 value;
	int ret;

	/* DP_SET_POWER register is only available on DPCD v1.1 and later */
	if (edp->link_train.revision < 0x11)
		return 0;

	ret = rk_edp_dpcd_read(edp->regs, DPCD_LINK_POWER_STATE, &value, 1);
	if (ret)
		return ret;

	value &= ~DP_SET_POWER_MASK;
	value |= DP_SET_POWER_D0;

	ret = rk_edp_dpcd_write(edp->regs, DPCD_LINK_POWER_STATE, &value, 1);
	if (ret)
		return ret;

	/*
	 * According to the DP 1.1 specification, a "Sink Device must exit the
	 * power saving state within 1 ms" (Section 2.5.3.1, Table 5-52, "Sink
	 * Control Field" (register 0x600).
	 */
	mdelay(1);

	return 0;
}

static int rk_edp_link_configure(struct rk_edp_priv *edp)
{
	u8 values[2];

	values[0] = edp->link_train.link_rate;
	values[1] = edp->link_train.lane_count;

	return rk_edp_dpcd_write(edp->regs, DPCD_LINK_BW_SET, values,
				 sizeof(values));
}

static void rk_edp_set_link_training(struct rk_edp_priv *edp,
				     const u8 *training_values)
{
	int i;

	for (i = 0; i < edp->link_train.lane_count; i++)
		writel(training_values[i], &edp->regs->ln_link_trn_ctl[i]);
}

static u8 edp_link_status(const u8 *link_status, int r)
{
	return link_status[r - DPCD_LANE0_1_STATUS];
}

static int rk_edp_dpcd_read_link_status(struct rk_edp_priv *edp,
					u8 *link_status)
{
	return rk_edp_dpcd_read(edp->regs, DPCD_LANE0_1_STATUS, link_status,
				DP_LINK_STATUS_SIZE);
}

static u8 edp_get_lane_status(const u8 *link_status, int lane)
{
	int i = DPCD_LANE0_1_STATUS + (lane >> 1);
	int s = (lane & 1) * 4;
	u8 l = edp_link_status(link_status, i);

	return (l >> s) & 0xf;
}

static int rk_edp_clock_recovery(const u8 *link_status, int lane_count)
{
	int lane;
	u8 lane_status;

	for (lane = 0; lane < lane_count; lane++) {
		lane_status = edp_get_lane_status(link_status, lane);
		if ((lane_status & DP_LANE_CR_DONE) == 0)
			return -EIO;
	}

	return 0;
}

static int rk_edp_channel_eq(const u8 *link_status, int lane_count)
{
	u8 lane_align;
	u8 lane_status;
	int lane;

	lane_align = edp_link_status(link_status,
				    DPCD_LANE_ALIGN_STATUS_UPDATED);
	if (!(lane_align & DP_INTERLANE_ALIGN_DONE))
		return -EIO;
	for (lane = 0; lane < lane_count; lane++) {
		lane_status = edp_get_lane_status(link_status, lane);
		if ((lane_status & DP_CHANNEL_EQ_BITS) != DP_CHANNEL_EQ_BITS)
			return -EIO;
	}

	return 0;
}

static uint rk_edp_get_adjust_request_voltage(const u8 *link_status, int lane)
{
	int i = DPCD_ADJUST_REQUEST_LANE0_1 + (lane >> 1);
	int s = ((lane & 1) ?
		 DP_ADJUST_VOLTAGE_SWING_LANE1_SHIFT :
		 DP_ADJUST_VOLTAGE_SWING_LANE0_SHIFT);
	u8 l = edp_link_status(link_status, i);

	return ((l >> s) & 0x3) << DP_TRAIN_VOLTAGE_SWING_SHIFT;
}

static uint rk_edp_get_adjust_request_pre_emphasis(const u8 *link_status,
						   int lane)
{
	int i = DPCD_ADJUST_REQUEST_LANE0_1 + (lane >> 1);
	int s = ((lane & 1) ?
		 DP_ADJUST_PRE_EMPHASIS_LANE1_SHIFT :
		 DP_ADJUST_PRE_EMPHASIS_LANE0_SHIFT);
	u8 l = edp_link_status(link_status, i);

	return ((l >> s) & 0x3) << DP_TRAIN_PRE_EMPHASIS_SHIFT;
}

static void edp_get_adjust_train(const u8 *link_status, int lane_count,
				 u8 train_set[])
{
	uint v = 0;
	uint p = 0;
	int lane;

	for (lane = 0; lane < lane_count; lane++) {
		uint this_v, this_p;

		this_v = rk_edp_get_adjust_request_voltage(link_status, lane);
		this_p = rk_edp_get_adjust_request_pre_emphasis(link_status,
								lane);

		debug("requested signal parameters: lane %d voltage %s pre_emph %s\n",
		      lane,
		      voltage_names[this_v >> DP_TRAIN_VOLTAGE_SWING_SHIFT],
		      pre_emph_names[this_p >> DP_TRAIN_PRE_EMPHASIS_SHIFT]);

		if (this_v > v)
			v = this_v;
		if (this_p > p)
			p = this_p;
	}

	if (v >= DP_VOLTAGE_MAX)
		v |= DP_TRAIN_MAX_SWING_REACHED;

	if (p >= DP_PRE_EMPHASIS_MAX)
		p |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

	debug("using signal parameters: voltage %s pre_emph %s\n",
	      voltage_names[(v & DP_TRAIN_VOLTAGE_SWING_MASK)
			>> DP_TRAIN_VOLTAGE_SWING_SHIFT],
	      pre_emph_names[(p & DP_TRAIN_PRE_EMPHASIS_MASK)
			>> DP_TRAIN_PRE_EMPHASIS_SHIFT]);

	for (lane = 0; lane < 4; lane++)
		train_set[lane] = v | p;
}

static int rk_edp_link_train_cr(struct rk_edp_priv *edp)
{
	struct rk3288_edp *regs = edp->regs;
	int clock_recovery;
	uint voltage, tries = 0;
	u8 status[DP_LINK_STATUS_SIZE];
	int i, ret;
	u8 value;

	value = DP_TRAINING_PATTERN_1;
	writel(value, &regs->dp_training_ptn_set);
	ret = rk_edp_dpcd_write(regs, DPCD_TRAINING_PATTERN_SET, &value, 1);
	if (ret)
		return ret;
	memset(edp->train_set, '\0', sizeof(edp->train_set));

	/* clock recovery loop */
	clock_recovery = 0;
	tries = 0;
	voltage = 0xff;

	while (1) {
		rk_edp_set_link_training(edp, edp->train_set);
		ret = rk_edp_dpcd_write(regs, DPCD_TRAINING_LANE0_SET,
					edp->train_set,
					edp->link_train.lane_count);
		if (ret)
			return ret;

		mdelay(1);

		ret = rk_edp_dpcd_read_link_status(edp, status);
		if (ret) {
			printf("displayport link status failed, ret=%d\n", ret);
			break;
		}

		clock_recovery = rk_edp_clock_recovery(status,
						edp->link_train.lane_count);
		if (!clock_recovery)
			break;

		for (i = 0; i < edp->link_train.lane_count; i++) {
			if ((edp->train_set[i] &
				DP_TRAIN_MAX_SWING_REACHED) == 0)
				break;
		}
		if (i == edp->link_train.lane_count) {
			printf("clock recovery reached max voltage\n");
			break;
		}

		if ((edp->train_set[0] & DP_TRAIN_VOLTAGE_SWING_MASK) ==
				voltage) {
			if (++tries == MAX_CR_LOOP) {
				printf("clock recovery tried 5 times\n");
				break;
			}
		} else {
			tries = 0;
		}

		voltage = edp->train_set[0] & DP_TRAIN_VOLTAGE_SWING_MASK;

		/* Compute new train_set as requested by sink */
		edp_get_adjust_train(status, edp->link_train.lane_count,
				     edp->train_set);
	}
	if (clock_recovery) {
		printf("clock recovery failed: %d\n", clock_recovery);
		return clock_recovery;
	} else {
		debug("clock recovery at voltage %d pre-emphasis %d\n",
		      edp->train_set[0] & DP_TRAIN_VOLTAGE_SWING_MASK,
		      (edp->train_set[0] & DP_TRAIN_PRE_EMPHASIS_MASK) >>
				DP_TRAIN_PRE_EMPHASIS_SHIFT);
		return 0;
	}
}

static int rk_edp_link_train_ce(struct rk_edp_priv *edp)
{
	struct rk3288_edp *regs = edp->regs;
	int channel_eq;
	u8 value;
	int tries;
	u8 status[DP_LINK_STATUS_SIZE];
	int ret;

	value = DP_TRAINING_PATTERN_2;
	writel(value, &regs->dp_training_ptn_set);
	ret = rk_edp_dpcd_write(regs, DPCD_TRAINING_PATTERN_SET, &value, 1);
	if (ret)
		return ret;

	/* channel equalization loop */
	channel_eq = 0;
	for (tries = 0; tries < 5; tries++) {
		rk_edp_set_link_training(edp, edp->train_set);
		udelay(400);

		if (rk_edp_dpcd_read_link_status(edp, status) < 0) {
			printf("displayport link status failed\n");
			return -1;
		}

		channel_eq = rk_edp_channel_eq(status,
					       edp->link_train.lane_count);
		if (!channel_eq)
			break;
		edp_get_adjust_train(status, edp->link_train.lane_count,
				     edp->train_set);
	}

	if (channel_eq) {
		printf("channel eq failed, ret=%d\n", channel_eq);
		return channel_eq;
	}

	debug("channel eq at voltage %d pre-emphasis %d\n",
	      edp->train_set[0] & DP_TRAIN_VOLTAGE_SWING_MASK,
	      (edp->train_set[0] & DP_TRAIN_PRE_EMPHASIS_MASK)
			>> DP_TRAIN_PRE_EMPHASIS_SHIFT);

	return 0;
}

static int rk_edp_init_training(struct rk_edp_priv *edp)
{
	u8 values[3];
	int ret;

	ret = rk_edp_dpcd_read(edp->regs, DPCD_DPCD_REV, values,
			       sizeof(values));
	if (ret < 0)
		return ret;

	edp->link_train.revision = values[0];
	edp->link_train.link_rate = values[1];
	edp->link_train.lane_count = values[2] & DP_MAX_LANE_COUNT_MASK;

	debug("max link rate:%d.%dGps max number of lanes:%d\n",
	      edp->link_train.link_rate * 27 / 100,
	      edp->link_train.link_rate * 27 % 100,
	      edp->link_train.lane_count);

	if ((edp->link_train.link_rate != LINK_RATE_1_62GBPS) &&
	    (edp->link_train.link_rate != LINK_RATE_2_70GBPS)) {
		debug("Rx Max Link Rate is abnormal :%x\n",
		      edp->link_train.link_rate);
		return -EPERM;
	}

	if (edp->link_train.lane_count == 0) {
		debug("Rx Max Lane count is abnormal :%x\n",
		      edp->link_train.lane_count);
		return -EPERM;
	}

	ret = rk_edp_link_power_up(edp);
	if (ret)
		return ret;

	return rk_edp_link_configure(edp);
}

static int rk_edp_hw_link_training(struct rk_edp_priv *edp)
{
	ulong start;
	u32 val;
	int ret;

	/* Set link rate and count as you want to establish */
	writel(edp->link_train.link_rate, &edp->regs->link_bw_set);
	writel(edp->link_train.lane_count, &edp->regs->lane_count_set);

	ret = rk_edp_link_train_cr(edp);
	if (ret)
		return ret;
	ret = rk_edp_link_train_ce(edp);
	if (ret)
		return ret;

	writel(HW_LT_EN, &edp->regs->dp_hw_link_training);
	start = get_timer(0);
	do {
		val = readl(&edp->regs->dp_hw_link_training);
		if (!(val & HW_LT_EN))
			break;
	} while (get_timer(start) < 10);

	if (val & HW_LT_ERR_CODE_MASK) {
		printf("edp hw link training error: %d\n",
		       val >> HW_LT_ERR_CODE_SHIFT);
		return -EIO;
	}

	return 0;
}

static int rk_edp_select_i2c_device(struct rk3288_edp *regs,
				    unsigned int device_addr,
				    unsigned int val_addr)
{
	int ret;

	/* Set EDID device address */
	writel(device_addr, &regs->aux_addr_7_0);
	writel(0x0, &regs->aux_addr_15_8);
	writel(0x0, &regs->aux_addr_19_16);

	/* Set offset from base address of EDID device */
	writel(val_addr, &regs->buf_data[0]);

	/*
	 * Set I2C transaction and write address
	 * If bit 3 is 1, DisplayPort transaction.
	 * If Bit 3 is 0, I2C transaction.
	 */
	writel(AUX_TX_COMM_I2C_TRANSACTION | AUX_TX_COMM_MOT |
	       AUX_TX_COMM_WRITE, &regs->aux_ch_ctl_1);

	/* Start AUX transaction */
	ret = rk_edp_start_aux_transaction(regs);
	if (ret != 0) {
		debug("select_i2c_device Aux Transaction fail!\n");
		return ret;
	}

	return 0;
}

static int rk_edp_i2c_read(struct rk3288_edp *regs, unsigned int device_addr,
			   unsigned int val_addr, unsigned int count, u8 edid[])
{
	u32 val;
	unsigned int i, j;
	unsigned int cur_data_idx;
	unsigned int defer = 0;
	int ret = 0;

	for (i = 0; i < count; i += 16) {
		for (j = 0; j < 10; j++) { /* try 10 times */
			/* Clear AUX CH data buffer */
			writel(BUF_CLR, &regs->buf_data_ctl);

			/* Set normal AUX CH command */
			clrbits_le32(&regs->aux_ch_ctl_2, ADDR_ONLY);

			/*
			 * If Rx sends defer, Tx sends only reads
			 * request without sending addres
			 */
			if (!defer) {
				ret = rk_edp_select_i2c_device(regs,
							       device_addr,
							       val_addr + i);
			} else {
				defer = 0;
			}

			/*
			 * Set I2C transaction and write data
			 * If bit 3 is 1, DisplayPort transaction.
			 * If Bit 3 is 0, I2C transaction.
			 */
			writel(AUX_LENGTH(16) | AUX_TX_COMM_I2C_TRANSACTION |
			       AUX_TX_COMM_READ, &regs->aux_ch_ctl_1);

			/* Start AUX transaction */
			ret = rk_edp_start_aux_transaction(regs);
			if (ret == 0) {
				break;
			} else {
				debug("Aux Transaction fail!\n");
				continue;
			}

			/* Check if Rx sends defer */
			val = readl(&regs->aux_rx_comm);
			if (val == AUX_RX_COMM_AUX_DEFER ||
			    val == AUX_RX_COMM_I2C_DEFER) {
				debug("Defer: %d\n\n", val);
				defer = 1;
			}
		}

		if (ret)
			return ret;

		for (cur_data_idx = 0; cur_data_idx < 16; cur_data_idx++) {
			val = readl(&regs->buf_data[cur_data_idx]);
			edid[i + cur_data_idx] = (u8)val;
		}
	}

	return 0;
}

static int rk_edp_set_link_train(struct rk_edp_priv *edp)
{
	int ret;

	ret = rk_edp_init_training(edp);
	if (ret) {
		printf("DP LT init failed!\n");
		return ret;
	}

	ret = rk_edp_hw_link_training(edp);
	if (ret)
		return ret;

	return 0;
}

static void rk_edp_init_video(struct rk3288_edp *regs)
{
	writel(VSYNC_DET | VID_FORMAT_CHG | VID_CLK_CHG,
	       &regs->common_int_sta_1);
	writel(CHA_CRI(4) | CHA_CTRL, &regs->sys_ctl_2);
	writel(VID_HRES_TH(2) | VID_VRES_TH(0), &regs->video_ctl_8);
}

static void rk_edp_config_video_slave_mode(struct rk3288_edp *regs)
{
	clrbits_le32(&regs->func_en_1, VID_FIFO_FUNC_EN_N | VID_CAP_FUNC_EN_N);
}

static void rk_edp_set_video_cr_mn(struct rk3288_edp *regs,
				   enum clock_recovery_m_value_type type,
				   u32 m_value,
				   u32 n_value)
{
	if (type == REGISTER_M) {
		setbits_le32(&regs->sys_ctl_4, FIX_M_VID);
		writel(m_value & 0xff, &regs->m_vid_0);
		writel((m_value >> 8) & 0xff, &regs->m_vid_1);
		writel((m_value >> 16) & 0xff, &regs->m_vid_2);

		writel(n_value & 0xf, &regs->n_vid_0);
		writel((n_value >> 8) & 0xff, &regs->n_vid_1);
		writel((n_value >> 16) & 0xff, &regs->n_vid_2);
	} else {
		clrbits_le32(&regs->sys_ctl_4, FIX_M_VID);

		writel(0x00, &regs->n_vid_0);
		writel(0x80, &regs->n_vid_1);
		writel(0x00, &regs->n_vid_2);
	}
}

static int rk_edp_is_video_stream_clock_on(struct rk3288_edp *regs)
{
	ulong start;
	u32 val;

	start = get_timer(0);
	do {
		val = readl(&regs->sys_ctl_1);

		/* must write value to update DET_STA bit status */
		writel(val, &regs->sys_ctl_1);
		val = readl(&regs->sys_ctl_1);
		if (!(val & DET_STA))
			continue;

		val = readl(&regs->sys_ctl_2);

		/* must write value to update CHA_STA bit status */
		writel(val, &regs->sys_ctl_2);
		val = readl(&regs->sys_ctl_2);
		if (!(val & CHA_STA))
			return 0;

	} while (get_timer(start) < 100);

	return -ETIMEDOUT;
}

static int rk_edp_is_video_stream_on(struct rk_edp_priv *edp)
{
	ulong start;
	u32 val;

	start = get_timer(0);
	do {
		val = readl(&edp->regs->sys_ctl_3);

		/* must write value to update STRM_VALID bit status */
		writel(val, &edp->regs->sys_ctl_3);

		val = readl(&edp->regs->sys_ctl_3);
		if (!(val & STRM_VALID))
			return 0;
	} while (get_timer(start) < 100);

	return -ETIMEDOUT;
}

static int rk_edp_config_video(struct rk_edp_priv *edp)
{
	int ret;

	rk_edp_config_video_slave_mode(edp->regs);

	if (!rk_edp_get_pll_locked(edp->regs)) {
		debug("PLL is not locked yet.\n");
		return -ETIMEDOUT;
	}

	ret = rk_edp_is_video_stream_clock_on(edp->regs);
	if (ret)
		return ret;

	/* Set to use the register calculated M/N video */
	rk_edp_set_video_cr_mn(edp->regs, CALCULATED_M, 0, 0);

	/* For video bist, Video timing must be generated by register */
	clrbits_le32(&edp->regs->video_ctl_10, F_SEL);

	/* Disable video mute */
	clrbits_le32(&edp->regs->video_ctl_1, VIDEO_MUTE);

	/* Enable video at next frame */
	setbits_le32(&edp->regs->video_ctl_1, VIDEO_EN);

	return rk_edp_is_video_stream_on(edp);
}

static void rockchip_edp_force_hpd(struct rk_edp_priv *edp)
{
	setbits_le32(&edp->regs->sys_ctl_3, F_HPD | HPD_CTRL);
}

static int rockchip_edp_get_plug_in_status(struct rk_edp_priv *edp)
{
	u32 val;

	val = readl(&edp->regs->sys_ctl_3);
	if (val & HPD_STATUS)
		return 1;

	return 0;
}

/*
 * support edp HPD function
 * some hardware version do not support edp hdp,
 * we use 200ms to try to get the hpd single now,
 * if we can not get edp hpd single, it will delay 200ms,
 * also meet the edp power timing request, to compatible
 * all of the hardware version
 */
static void rockchip_edp_wait_hpd(struct rk_edp_priv *edp)
{
	ulong start;

	start = get_timer(0);
	do {
		if (rockchip_edp_get_plug_in_status(edp))
			return;
		udelay(100);
	} while (get_timer(start) < 200);

	debug("do not get hpd single, force hpd\n");
	rockchip_edp_force_hpd(edp);
}

static int rk_edp_enable(struct udevice *dev, int panel_bpp,
			 const struct display_timing *edid)
{
	struct rk_edp_priv *priv = dev_get_priv(dev);
	int ret = 0;

	ret = rk_edp_set_link_train(priv);
	if (ret) {
		printf("link train failed!\n");
		return ret;
	}

	rk_edp_init_video(priv->regs);
	ret = rk_edp_config_video(priv);
	if (ret) {
		printf("config video failed\n");
		return ret;
	}
	ret = panel_enable_backlight(priv->panel);
	if (ret) {
		debug("%s: backlight error: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int rk_edp_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct rk_edp_priv *priv = dev_get_priv(dev);
	u32 edid_size = EDID_LENGTH;
	int ret;
	int i;

	for (i = 0; i < 3; i++) {
		ret = rk_edp_i2c_read(priv->regs, EDID_ADDR, EDID_HEADER,
				      EDID_LENGTH, &buf[EDID_HEADER]);
		if (ret) {
			debug("EDID read failed\n");
			continue;
		}

		/*
		 * check if the EDID has an extension flag, and read additional
		 * EDID data if needed
		 */
		if (buf[EDID_EXTENSION_FLAG]) {
			edid_size += EDID_LENGTH;
			ret = rk_edp_i2c_read(priv->regs, EDID_ADDR,
					      EDID_LENGTH, EDID_LENGTH,
					      &buf[EDID_LENGTH]);
			if (ret) {
				debug("EDID Read failed!\n");
				continue;
			}
		}
		goto done;
	}

	/* After 3 attempts, give up */
	return ret;

done:
	return edid_size;
}

static int rk_edp_ofdata_to_platdata(struct udevice *dev)
{
	struct rk_edp_priv *priv = dev_get_priv(dev);

	priv->regs = (struct rk3288_edp *)devfdt_get_addr(dev);
	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	return 0;
}

static int rk_edp_remove(struct udevice *dev)
{
	struct rk_edp_priv *priv = dev_get_priv(dev);
	struct rk3288_edp *regs = priv->regs;

	setbits_le32(&regs->video_ctl_1, VIDEO_MUTE);
	clrbits_le32(&regs->video_ctl_1, VIDEO_EN);
	clrbits_le32(&regs->sys_ctl_3, F_HPD | HPD_CTRL);
	setbits_le32(&regs->func_en_1, SW_FUNC_EN_N);

	return 0;
}

static int rk_edp_probe(struct udevice *dev)
{
	struct display_plat *uc_plat = dev_get_uclass_platdata(dev);
	struct rk_edp_priv *priv = dev_get_priv(dev);
	struct rk3288_edp *regs = priv->regs;
	struct clk clk;
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL, dev, "rockchip,panel",
					   &priv->panel);
	if (ret) {
		debug("%s: Cannot find panel for '%s' (ret=%d)\n", __func__,
		      dev->name, ret);
		return ret;
	}

	int vop_id = uc_plat->source_id;
	debug("%s, uc_plat=%p, vop_id=%u\n", __func__, uc_plat, vop_id);

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 0);
		clk_free(&clk);
	}
	if (ret) {
		debug("%s: Failed to set EDP clock: ret=%d\n", __func__, ret);
		return ret;
	}

	ret = clk_get_by_index(uc_plat->src_dev, 0, &clk);
	if (ret >= 0) {
		ret = clk_set_rate(&clk, 192000000);
		clk_free(&clk);
	}
	if (ret < 0) {
		debug("%s: Failed to set clock in source device '%s': ret=%d\n",
		      __func__, uc_plat->src_dev->name, ret);
		return ret;
	}

	/* grf_edp_ref_clk_sel: from internal 24MHz or 27MHz clock */
	rk_setreg(&priv->grf->soc_con12, 1 << 4);

	/* select epd signal from vop0 or vop1 */
	rk_setreg(&priv->grf->soc_con6, (vop_id == 1) ? (1 << 5) : (1 << 5));

	rockchip_edp_wait_hpd(priv);

	rk_edp_init_refclk(regs);
	rk_edp_init_interrupt(regs);
	rk_edp_enable_sw_function(regs);
	ret = rk_edp_init_analog_func(regs);
	if (ret)
		return ret;
	rk_edp_init_aux(regs);

	return 0;
}

static const struct dm_display_ops dp_rockchip_ops = {
	.read_edid = rk_edp_read_edid,
	.enable = rk_edp_enable,
};

static const struct udevice_id rockchip_dp_ids[] = {
	{ .compatible = "rockchip,rk3288-edp" },
	{ }
};

U_BOOT_DRIVER(dp_rockchip) = {
	.name	= "edp_rockchip",
	.id	= UCLASS_DISPLAY,
	.of_match = rockchip_dp_ids,
	.ops	= &dp_rockchip_ops,
	.ofdata_to_platdata	= rk_edp_ofdata_to_platdata,
	.probe	= rk_edp_probe,
	.remove	= rk_edp_remove,
	.priv_auto_alloc_size	= sizeof(struct rk_edp_priv),
};
