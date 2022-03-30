// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2011-2013, NVIDIA Corporation.
 * Copyright 2014 Google Inc.
 */

#include <common.h>
#include <display.h>
#include <dm.h>
#include <div64.h>
#include <errno.h>
#include <video_bridge.h>
#include <asm/io.h>
#include <asm/arch-tegra/dc.h>
#include "display.h"
#include "edid.h"
#include "sor.h"
#include "displayport.h"

#define DO_FAST_LINK_TRAINING		1

struct tegra_dp_plat {
	ulong base;
};

/**
 * struct tegra_dp_priv - private displayport driver info
 *
 * @dc_dev:	Display controller device that is sending the video feed
 */
struct tegra_dp_priv {
	struct udevice *sor;
	struct udevice *dc_dev;
	struct dpaux_ctlr *regs;
	u8 revision;
	int enabled;
};

struct tegra_dp_priv dp_data;

static inline u32 tegra_dpaux_readl(struct tegra_dp_priv *dp, u32 reg)
{
	return readl((u32 *)dp->regs + reg);
}

static inline void tegra_dpaux_writel(struct tegra_dp_priv *dp, u32 reg,
				      u32 val)
{
	writel(val, (u32 *)dp->regs + reg);
}

static inline u32 tegra_dc_dpaux_poll_register(struct tegra_dp_priv *dp,
					   u32 reg, u32 mask, u32 exp_val,
					   u32 poll_interval_us,
					   u32 timeout_us)
{
	u32 reg_val = 0;
	u32 temp = timeout_us;

	do {
		udelay(poll_interval_us);
		reg_val = tegra_dpaux_readl(dp, reg);
		if (timeout_us > poll_interval_us)
			timeout_us -= poll_interval_us;
		else
			break;
	} while ((reg_val & mask) != exp_val);

	if ((reg_val & mask) == exp_val)
		return 0;	/* success */
	debug("dpaux_poll_register 0x%x: timeout: (reg_val)0x%08x & (mask)0x%08x != (exp_val)0x%08x\n",
	      reg, reg_val, mask, exp_val);
	return temp;
}

static inline int tegra_dpaux_wait_transaction(struct tegra_dp_priv *dp)
{
	/* According to DP spec, each aux transaction needs to finish
	   within 40ms. */
	if (tegra_dc_dpaux_poll_register(dp, DPAUX_DP_AUXCTL,
					 DPAUX_DP_AUXCTL_TRANSACTREQ_MASK,
					 DPAUX_DP_AUXCTL_TRANSACTREQ_DONE,
					 100, DP_AUX_TIMEOUT_MS * 1000) != 0) {
		debug("dp: DPAUX transaction timeout\n");
		return -1;
	}
	return 0;
}

static int tegra_dc_dpaux_write_chunk(struct tegra_dp_priv *dp, u32 cmd,
					  u32 addr, u8 *data, u32 *size,
					  u32 *aux_stat)
{
	int i;
	u32 reg_val;
	u32 timeout_retries = DP_AUX_TIMEOUT_MAX_TRIES;
	u32 defer_retries = DP_AUX_DEFER_MAX_TRIES;
	u32 temp_data;

	if (*size > DP_AUX_MAX_BYTES)
		return -1;	/* only write one chunk of data */

	/* Make sure the command is write command */
	switch (cmd) {
	case DPAUX_DP_AUXCTL_CMD_I2CWR:
	case DPAUX_DP_AUXCTL_CMD_MOTWR:
	case DPAUX_DP_AUXCTL_CMD_AUXWR:
		break;
	default:
		debug("dp: aux write cmd 0x%x is invalid\n", cmd);
		return -EINVAL;
	}

	tegra_dpaux_writel(dp, DPAUX_DP_AUXADDR, addr);
	for (i = 0; i < DP_AUX_MAX_BYTES / 4; ++i) {
		memcpy(&temp_data, data, 4);
		tegra_dpaux_writel(dp, DPAUX_DP_AUXDATA_WRITE_W(i), temp_data);
		data += 4;
	}

	reg_val = tegra_dpaux_readl(dp, DPAUX_DP_AUXCTL);
	reg_val &= ~DPAUX_DP_AUXCTL_CMD_MASK;
	reg_val |= cmd;
	reg_val &= ~DPAUX_DP_AUXCTL_CMDLEN_FIELD;
	reg_val |= ((*size - 1) << DPAUX_DP_AUXCTL_CMDLEN_SHIFT);

	while ((timeout_retries > 0) && (defer_retries > 0)) {
		if ((timeout_retries != DP_AUX_TIMEOUT_MAX_TRIES) ||
		    (defer_retries != DP_AUX_DEFER_MAX_TRIES))
			udelay(1);

		reg_val |= DPAUX_DP_AUXCTL_TRANSACTREQ_PENDING;
		tegra_dpaux_writel(dp, DPAUX_DP_AUXCTL, reg_val);

		if (tegra_dpaux_wait_transaction(dp))
			debug("dp: aux write transaction timeout\n");

		*aux_stat = tegra_dpaux_readl(dp, DPAUX_DP_AUXSTAT);

		if ((*aux_stat & DPAUX_DP_AUXSTAT_TIMEOUT_ERROR_PENDING) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_RX_ERROR_PENDING) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_SINKSTAT_ERROR_PENDING) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_NO_STOP_ERROR_PENDING)) {
			if (timeout_retries-- > 0) {
				debug("dp: aux write retry (0x%x) -- %d\n",
				      *aux_stat, timeout_retries);
				/* clear the error bits */
				tegra_dpaux_writel(dp, DPAUX_DP_AUXSTAT,
						   *aux_stat);
				continue;
			} else {
				debug("dp: aux write got error (0x%x)\n",
				      *aux_stat);
				return -ETIMEDOUT;
			}
		}

		if ((*aux_stat & DPAUX_DP_AUXSTAT_REPLYTYPE_I2CDEFER) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_REPLYTYPE_DEFER)) {
			if (defer_retries-- > 0) {
				debug("dp: aux write defer (0x%x) -- %d\n",
				      *aux_stat, defer_retries);
				/* clear the error bits */
				tegra_dpaux_writel(dp, DPAUX_DP_AUXSTAT,
						   *aux_stat);
				continue;
			} else {
				debug("dp: aux write defer exceeds max retries (0x%x)\n",
				      *aux_stat);
				return -ETIMEDOUT;
			}
		}

		if ((*aux_stat & DPAUX_DP_AUXSTAT_REPLYTYPE_MASK) ==
			DPAUX_DP_AUXSTAT_REPLYTYPE_ACK) {
			*size = ((*aux_stat) & DPAUX_DP_AUXSTAT_REPLY_M_MASK);
			return 0;
		} else {
			debug("dp: aux write failed (0x%x)\n", *aux_stat);
			return -EIO;
		}
	}
	/* Should never come to here */
	return -EIO;
}

static int tegra_dc_dpaux_read_chunk(struct tegra_dp_priv *dp, u32 cmd,
					 u32 addr, u8 *data, u32 *size,
					 u32 *aux_stat)
{
	u32 reg_val;
	u32 timeout_retries = DP_AUX_TIMEOUT_MAX_TRIES;
	u32 defer_retries = DP_AUX_DEFER_MAX_TRIES;

	if (*size > DP_AUX_MAX_BYTES) {
		debug("only read one chunk\n");
		return -EIO;	/* only read one chunk */
	}

	/* Check to make sure the command is read command */
	switch (cmd) {
	case DPAUX_DP_AUXCTL_CMD_I2CRD:
	case DPAUX_DP_AUXCTL_CMD_I2CREQWSTAT:
	case DPAUX_DP_AUXCTL_CMD_MOTRD:
	case DPAUX_DP_AUXCTL_CMD_AUXRD:
		break;
	default:
		debug("dp: aux read cmd 0x%x is invalid\n", cmd);
		return -EIO;
	}

	*aux_stat = tegra_dpaux_readl(dp, DPAUX_DP_AUXSTAT);
	if (!(*aux_stat & DPAUX_DP_AUXSTAT_HPD_STATUS_PLUGGED)) {
		debug("dp: HPD is not detected\n");
		return -EIO;
	}

	tegra_dpaux_writel(dp, DPAUX_DP_AUXADDR, addr);

	reg_val = tegra_dpaux_readl(dp, DPAUX_DP_AUXCTL);
	reg_val &= ~DPAUX_DP_AUXCTL_CMD_MASK;
	reg_val |= cmd;
	reg_val &= ~DPAUX_DP_AUXCTL_CMDLEN_FIELD;
	reg_val |= ((*size - 1) << DPAUX_DP_AUXCTL_CMDLEN_SHIFT);
	while ((timeout_retries > 0) && (defer_retries > 0)) {
		if ((timeout_retries != DP_AUX_TIMEOUT_MAX_TRIES) ||
		    (defer_retries != DP_AUX_DEFER_MAX_TRIES))
			udelay(DP_DPCP_RETRY_SLEEP_NS * 2);

		reg_val |= DPAUX_DP_AUXCTL_TRANSACTREQ_PENDING;
		tegra_dpaux_writel(dp, DPAUX_DP_AUXCTL, reg_val);

		if (tegra_dpaux_wait_transaction(dp))
			debug("dp: aux read transaction timeout\n");

		*aux_stat = tegra_dpaux_readl(dp, DPAUX_DP_AUXSTAT);

		if ((*aux_stat & DPAUX_DP_AUXSTAT_TIMEOUT_ERROR_PENDING) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_RX_ERROR_PENDING) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_SINKSTAT_ERROR_PENDING) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_NO_STOP_ERROR_PENDING)) {
			if (timeout_retries-- > 0) {
				debug("dp: aux read retry (0x%x) -- %d\n",
				      *aux_stat, timeout_retries);
				/* clear the error bits */
				tegra_dpaux_writel(dp, DPAUX_DP_AUXSTAT,
						   *aux_stat);
				continue;	/* retry */
			} else {
				debug("dp: aux read got error (0x%x)\n",
				      *aux_stat);
				return -ETIMEDOUT;
			}
		}

		if ((*aux_stat & DPAUX_DP_AUXSTAT_REPLYTYPE_I2CDEFER) ||
		    (*aux_stat & DPAUX_DP_AUXSTAT_REPLYTYPE_DEFER)) {
			if (defer_retries-- > 0) {
				debug("dp: aux read defer (0x%x) -- %d\n",
				      *aux_stat, defer_retries);
				/* clear the error bits */
				tegra_dpaux_writel(dp, DPAUX_DP_AUXSTAT,
						   *aux_stat);
				continue;
			} else {
				debug("dp: aux read defer exceeds max retries (0x%x)\n",
				      *aux_stat);
				return -ETIMEDOUT;
			}
		}

		if ((*aux_stat & DPAUX_DP_AUXSTAT_REPLYTYPE_MASK) ==
			DPAUX_DP_AUXSTAT_REPLYTYPE_ACK) {
			int i;
			u32 temp_data[4];

			for (i = 0; i < DP_AUX_MAX_BYTES / 4; ++i)
				temp_data[i] = tegra_dpaux_readl(dp,
						DPAUX_DP_AUXDATA_READ_W(i));

			*size = ((*aux_stat) & DPAUX_DP_AUXSTAT_REPLY_M_MASK);
			memcpy(data, temp_data, *size);

			return 0;
		} else {
			debug("dp: aux read failed (0x%x\n", *aux_stat);
			return -EIO;
		}
	}
	/* Should never come to here */
	debug("%s: can't\n", __func__);

	return -EIO;
}

static int tegra_dc_dpaux_read(struct tegra_dp_priv *dp, u32 cmd, u32 addr,
			u8 *data, u32 *size, u32 *aux_stat)
{
	u32 finished = 0;
	u32 cur_size;
	int ret = 0;

	do {
		cur_size = *size - finished;
		if (cur_size > DP_AUX_MAX_BYTES)
			cur_size = DP_AUX_MAX_BYTES;

		ret = tegra_dc_dpaux_read_chunk(dp, cmd, addr,
						data, &cur_size, aux_stat);
		if (ret)
			break;

		/* cur_size should be the real size returned */
		addr += cur_size;
		data += cur_size;
		finished += cur_size;

	} while (*size > finished);
	*size = finished;

	return ret;
}

static int tegra_dc_dp_dpcd_read(struct tegra_dp_priv *dp, u32 cmd,
				 u8 *data_ptr)
{
	u32 size = 1;
	u32 status = 0;
	int ret;

	ret = tegra_dc_dpaux_read_chunk(dp, DPAUX_DP_AUXCTL_CMD_AUXRD,
					cmd, data_ptr, &size, &status);
	if (ret) {
		debug("dp: Failed to read DPCD data. CMD 0x%x, Status 0x%x\n",
		      cmd, status);
	}

	return ret;
}

static int tegra_dc_dp_dpcd_write(struct tegra_dp_priv *dp, u32 cmd,
				u8 data)
{
	u32 size = 1;
	u32 status = 0;
	int ret;

	ret = tegra_dc_dpaux_write_chunk(dp, DPAUX_DP_AUXCTL_CMD_AUXWR,
					cmd, &data, &size, &status);
	if (ret) {
		debug("dp: Failed to write DPCD data. CMD 0x%x, Status 0x%x\n",
		      cmd, status);
	}

	return ret;
}

static int tegra_dc_i2c_aux_read(struct tegra_dp_priv *dp, u32 i2c_addr,
				 u8 addr, u8 *data, u32 size, u32 *aux_stat)
{
	u32 finished = 0;
	int ret = 0;

	do {
		u32 cur_size = min((u32)DP_AUX_MAX_BYTES, size - finished);

		u32 len = 1;
		ret = tegra_dc_dpaux_write_chunk(
				dp, DPAUX_DP_AUXCTL_CMD_MOTWR, i2c_addr,
				&addr, &len, aux_stat);
		if (ret) {
			debug("%s: error sending address to read.\n",
			      __func__);
			return ret;
		}

		ret = tegra_dc_dpaux_read_chunk(
				dp, DPAUX_DP_AUXCTL_CMD_I2CRD, i2c_addr,
				data, &cur_size, aux_stat);
		if (ret) {
			debug("%s: error reading data.\n", __func__);
			return ret;
		}

		/* cur_size should be the real size returned */
		addr += cur_size;
		data += cur_size;
		finished += cur_size;
	} while (size > finished);

	return finished;
}

static void tegra_dc_dpaux_enable(struct tegra_dp_priv *dp)
{
	/* clear interrupt */
	tegra_dpaux_writel(dp, DPAUX_INTR_AUX, 0xffffffff);
	/* do not enable interrupt for now. Enable them when Isr in place */
	tegra_dpaux_writel(dp, DPAUX_INTR_EN_AUX, 0x0);

	tegra_dpaux_writel(dp, DPAUX_HYBRID_PADCTL,
			   DPAUX_HYBRID_PADCTL_AUX_DRVZ_OHM_50 |
			   DPAUX_HYBRID_PADCTL_AUX_CMH_V0_70 |
			   0x18 << DPAUX_HYBRID_PADCTL_AUX_DRVI_SHIFT |
			   DPAUX_HYBRID_PADCTL_AUX_INPUT_RCV_ENABLE);

	tegra_dpaux_writel(dp, DPAUX_HYBRID_SPARE,
			   DPAUX_HYBRID_SPARE_PAD_PWR_POWERUP);
}

#ifdef DEBUG
static void tegra_dc_dp_dump_link_cfg(struct tegra_dp_priv *dp,
	const struct tegra_dp_link_config *link_cfg)
{
	debug("DP config: cfg_name               cfg_value\n");
	debug("           Lane Count             %d\n",
	      link_cfg->max_lane_count);
	debug("           SupportEnhancedFraming %s\n",
	      link_cfg->support_enhanced_framing ? "Y" : "N");
	debug("           Bandwidth              %d\n",
	      link_cfg->max_link_bw);
	debug("           bpp                    %d\n",
	      link_cfg->bits_per_pixel);
	debug("           EnhancedFraming        %s\n",
	      link_cfg->enhanced_framing ? "Y" : "N");
	debug("           Scramble_enabled       %s\n",
	      link_cfg->scramble_ena ? "Y" : "N");
	debug("           LinkBW                 %d\n",
	      link_cfg->link_bw);
	debug("           lane_count             %d\n",
	      link_cfg->lane_count);
	debug("           activespolarity        %d\n",
	      link_cfg->activepolarity);
	debug("           active_count           %d\n",
	      link_cfg->active_count);
	debug("           tu_size                %d\n",
	      link_cfg->tu_size);
	debug("           active_frac            %d\n",
	      link_cfg->active_frac);
	debug("           watermark              %d\n",
	      link_cfg->watermark);
	debug("           hblank_sym             %d\n",
	      link_cfg->hblank_sym);
	debug("           vblank_sym             %d\n",
	      link_cfg->vblank_sym);
}
#endif

static int _tegra_dp_lower_link_config(struct tegra_dp_priv *dp,
				       struct tegra_dp_link_config *cfg)
{
	switch (cfg->link_bw) {
	case SOR_LINK_SPEED_G1_62:
		if (cfg->max_link_bw > SOR_LINK_SPEED_G1_62)
			cfg->link_bw = SOR_LINK_SPEED_G2_7;
		cfg->lane_count /= 2;
		break;
	case SOR_LINK_SPEED_G2_7:
		cfg->link_bw = SOR_LINK_SPEED_G1_62;
		break;
	case SOR_LINK_SPEED_G5_4:
		if (cfg->lane_count == 1) {
			cfg->link_bw = SOR_LINK_SPEED_G2_7;
			cfg->lane_count = cfg->max_lane_count;
		} else {
			cfg->lane_count /= 2;
		}
		break;
	default:
		debug("dp: Error link rate %d\n", cfg->link_bw);
		return -ENOLINK;
	}

	return (cfg->lane_count > 0) ? 0 : -ENOLINK;
}

/*
 * Calcuate if given cfg can meet the mode request.
 * Return 0 if mode is possible, -1 otherwise
 */
static int tegra_dc_dp_calc_config(struct tegra_dp_priv *dp,
				   const struct display_timing *timing,
				   struct tegra_dp_link_config *link_cfg)
{
	const u32	link_rate = 27 * link_cfg->link_bw * 1000 * 1000;
	const u64	f	  = 100000;	/* precision factor */
	u32	num_linkclk_line; /* Number of link clocks per line */
	u64	ratio_f; /* Ratio of incoming to outgoing data rate */
	u64	frac_f;
	u64	activesym_f;	/* Activesym per TU */
	u64	activecount_f;
	u32	activecount;
	u32	activepolarity;
	u64	approx_value_f;
	u32	activefrac		  = 0;
	u64	accumulated_error_f	  = 0;
	u32	lowest_neg_activecount	  = 0;
	u32	lowest_neg_activepolarity = 0;
	u32	lowest_neg_tusize	  = 64;
	u32	num_symbols_per_line;
	u64	lowest_neg_activefrac	  = 0;
	u64	lowest_neg_error_f	  = 64 * f;
	u64	watermark_f;
	int	i;
	int	neg;

	if (!link_rate || !link_cfg->lane_count || !timing->pixelclock.typ ||
	    !link_cfg->bits_per_pixel)
		return -1;

	if ((u64)timing->pixelclock.typ * link_cfg->bits_per_pixel >=
		(u64)link_rate * 8 * link_cfg->lane_count)
		return -1;

	num_linkclk_line = (u32)(lldiv(link_rate * timing->hactive.typ,
				       timing->pixelclock.typ));

	ratio_f = (u64)timing->pixelclock.typ * link_cfg->bits_per_pixel * f;
	ratio_f /= 8;
	do_div(ratio_f, link_rate * link_cfg->lane_count);

	for (i = 64; i >= 32; --i) {
		activesym_f	= ratio_f * i;
		activecount_f	= lldiv(activesym_f, (u32)f) * f;
		frac_f		= activesym_f - activecount_f;
		activecount	= (u32)(lldiv(activecount_f, (u32)f));

		if (frac_f < (lldiv(f, 2))) /* fraction < 0.5 */
			activepolarity = 0;
		else {
			activepolarity = 1;
			frac_f = f - frac_f;
		}

		if (frac_f != 0) {
			/* warning: frac_f should be 64-bit */
			frac_f = lldiv(f * f, frac_f); /* 1 / fraction */
			if (frac_f > (15 * f))
				activefrac = activepolarity ? 1 : 15;
			else
				activefrac = activepolarity ?
					(u32)lldiv(frac_f, (u32)f) + 1 :
					(u32)lldiv(frac_f, (u32)f);
		}

		if (activefrac == 1)
			activepolarity = 0;

		if (activepolarity == 1)
			approx_value_f = activefrac ? lldiv(
				(activecount_f + (activefrac * f - f) * f),
				(activefrac * f)) :
				activecount_f + f;
		else
			approx_value_f = activefrac ?
				activecount_f + lldiv(f, activefrac) :
				activecount_f;

		if (activesym_f < approx_value_f) {
			accumulated_error_f = num_linkclk_line *
				lldiv(approx_value_f - activesym_f, i);
			neg = 1;
		} else {
			accumulated_error_f = num_linkclk_line *
				lldiv(activesym_f - approx_value_f, i);
			neg = 0;
		}

		if ((neg && (lowest_neg_error_f > accumulated_error_f)) ||
		    (accumulated_error_f == 0)) {
			lowest_neg_error_f = accumulated_error_f;
			lowest_neg_tusize = i;
			lowest_neg_activecount = activecount;
			lowest_neg_activepolarity = activepolarity;
			lowest_neg_activefrac = activefrac;

			if (accumulated_error_f == 0)
				break;
		}
	}

	if (lowest_neg_activefrac == 0) {
		link_cfg->activepolarity = 0;
		link_cfg->active_count   = lowest_neg_activepolarity ?
			lowest_neg_activecount : lowest_neg_activecount - 1;
		link_cfg->tu_size	      = lowest_neg_tusize;
		link_cfg->active_frac    = 1;
	} else {
		link_cfg->activepolarity = lowest_neg_activepolarity;
		link_cfg->active_count   = (u32)lowest_neg_activecount;
		link_cfg->tu_size	      = lowest_neg_tusize;
		link_cfg->active_frac    = (u32)lowest_neg_activefrac;
	}

	watermark_f = lldiv(ratio_f * link_cfg->tu_size * (f - ratio_f), f);
	link_cfg->watermark = (u32)(lldiv(watermark_f + lowest_neg_error_f,
		f)) + link_cfg->bits_per_pixel / 4 - 1;
	num_symbols_per_line = (timing->hactive.typ *
				link_cfg->bits_per_pixel) /
			       (8 * link_cfg->lane_count);

	if (link_cfg->watermark > 30) {
		debug("dp: sor setting: unable to get a good tusize, force watermark to 30\n");
		link_cfg->watermark = 30;
		return -1;
	} else if (link_cfg->watermark > num_symbols_per_line) {
		debug("dp: sor setting: force watermark to the number of symbols in the line\n");
		link_cfg->watermark = num_symbols_per_line;
		return -1;
	}

	/*
	 * Refer to dev_disp.ref for more information.
	 * # symbols/hblank = ((SetRasterBlankEnd.X + SetRasterSize.Width -
	 *                      SetRasterBlankStart.X - 7) * link_clk / pclk)
	 *                      - 3 * enhanced_framing - Y
	 * where Y = (# lanes == 4) 3 : (# lanes == 2) ? 6 : 12
	 */
	link_cfg->hblank_sym = (int)lldiv(((uint64_t)timing->hback_porch.typ +
			timing->hfront_porch.typ + timing->hsync_len.typ - 7) *
			link_rate, timing->pixelclock.typ) -
			3 * link_cfg->enhanced_framing -
			(12 / link_cfg->lane_count);

	if (link_cfg->hblank_sym < 0)
		link_cfg->hblank_sym = 0;


	/*
	 * Refer to dev_disp.ref for more information.
	 * # symbols/vblank = ((SetRasterBlankStart.X -
	 *                      SetRasterBlankEen.X - 25) * link_clk / pclk)
	 *                      - Y - 1;
	 * where Y = (# lanes == 4) 12 : (# lanes == 2) ? 21 : 39
	 */
	link_cfg->vblank_sym = (int)lldiv(((uint64_t)timing->hactive.typ - 25)
			* link_rate, timing->pixelclock.typ) - (36 /
			link_cfg->lane_count) - 4;

	if (link_cfg->vblank_sym < 0)
		link_cfg->vblank_sym = 0;

	link_cfg->is_valid = 1;
#ifdef DEBUG
	tegra_dc_dp_dump_link_cfg(dp, link_cfg);
#endif

	return 0;
}

static int tegra_dc_dp_init_max_link_cfg(
			const struct display_timing *timing,
			struct tegra_dp_priv *dp,
			struct tegra_dp_link_config *link_cfg)
{
	const int drive_current = 0x40404040;
	const int preemphasis = 0x0f0f0f0f;
	const int postcursor = 0;
	u8 dpcd_data;
	int ret;

	ret = tegra_dc_dp_dpcd_read(dp, DP_MAX_LANE_COUNT, &dpcd_data);
	if (ret)
		return ret;
	link_cfg->max_lane_count = dpcd_data & DP_MAX_LANE_COUNT_MASK;
	link_cfg->tps3_supported = (dpcd_data &
			DP_MAX_LANE_COUNT_TPS3_SUPPORTED_YES) ? 1 : 0;

	link_cfg->support_enhanced_framing =
		(dpcd_data & DP_MAX_LANE_COUNT_ENHANCED_FRAMING_YES) ?
		1 : 0;

	ret = tegra_dc_dp_dpcd_read(dp, DP_MAX_DOWNSPREAD, &dpcd_data);
	if (ret)
		return ret;
	link_cfg->downspread = (dpcd_data & DP_MAX_DOWNSPREAD_VAL_0_5_PCT) ?
				1 : 0;

	ret = tegra_dc_dp_dpcd_read(dp, NV_DPCD_TRAINING_AUX_RD_INTERVAL,
				    &link_cfg->aux_rd_interval);
	if (ret)
		return ret;
	ret = tegra_dc_dp_dpcd_read(dp, DP_MAX_LINK_RATE,
				    &link_cfg->max_link_bw);
	if (ret)
		return ret;

	/*
	 * Set to a high value for link training and attach.
	 * Will be re-programmed when dp is enabled.
	 */
	link_cfg->drive_current = drive_current;
	link_cfg->preemphasis = preemphasis;
	link_cfg->postcursor = postcursor;

	ret = tegra_dc_dp_dpcd_read(dp, DP_EDP_CONFIGURATION_CAP, &dpcd_data);
	if (ret)
		return ret;

	link_cfg->alt_scramber_reset_cap =
		(dpcd_data & DP_EDP_CONFIGURATION_CAP_ASC_RESET_YES) ?
		1 : 0;
	link_cfg->only_enhanced_framing =
		(dpcd_data & DP_EDP_CONFIGURATION_CAP_FRAMING_CHANGE_YES) ?
		1 : 0;

	link_cfg->lane_count = link_cfg->max_lane_count;
	link_cfg->link_bw = link_cfg->max_link_bw;
	link_cfg->enhanced_framing = link_cfg->support_enhanced_framing;
	link_cfg->frame_in_ms = (1000 / 60) + 1;

	tegra_dc_dp_calc_config(dp, timing, link_cfg);
	return 0;
}

static int tegra_dc_dp_set_assr(struct tegra_dp_priv *priv,
				struct udevice *sor, int ena)
{
	int ret;

	u8 dpcd_data = ena ?
		DP_MAIN_LINK_CHANNEL_CODING_SET_ASC_RESET_ENABLE :
		DP_MAIN_LINK_CHANNEL_CODING_SET_ASC_RESET_DISABLE;

	ret = tegra_dc_dp_dpcd_write(priv, DP_EDP_CONFIGURATION_SET,
				     dpcd_data);
	if (ret)
		return ret;

	/* Also reset the scrambler to 0xfffe */
	tegra_dc_sor_set_internal_panel(sor, ena);
	return 0;
}

static int tegra_dp_set_link_bandwidth(struct tegra_dp_priv *dp,
				       struct udevice *sor,
				       u8 link_bw)
{
	tegra_dc_sor_set_link_bandwidth(sor, link_bw);

	/* Sink side */
	return tegra_dc_dp_dpcd_write(dp, DP_LINK_BW_SET, link_bw);
}

static int tegra_dp_set_lane_count(struct tegra_dp_priv *dp,
		const struct tegra_dp_link_config *link_cfg,
		struct udevice *sor)
{
	u8	dpcd_data;
	int	ret;

	/* check if panel support enhanched_framing */
	dpcd_data = link_cfg->lane_count;
	if (link_cfg->enhanced_framing)
		dpcd_data |= DP_LANE_COUNT_SET_ENHANCEDFRAMING_T;
	ret = tegra_dc_dp_dpcd_write(dp, DP_LANE_COUNT_SET, dpcd_data);
	if (ret)
		return ret;

	tegra_dc_sor_set_lane_count(sor, link_cfg->lane_count);

	/* Also power down lanes that will not be used */
	return 0;
}

static int tegra_dc_dp_link_trained(struct tegra_dp_priv *dp,
				    const struct tegra_dp_link_config *cfg)
{
	u32 lane;
	u8 mask;
	u8 data;
	int ret;

	for (lane = 0; lane < cfg->lane_count; ++lane) {
		ret = tegra_dc_dp_dpcd_read(dp, (lane / 2) ?
				DP_LANE2_3_STATUS : DP_LANE0_1_STATUS,
				&data);
		if (ret)
			return ret;
		mask = (lane & 1) ?
			NV_DPCD_STATUS_LANEXPLUS1_CR_DONE_YES |
			NV_DPCD_STATUS_LANEXPLUS1_CHN_EQ_DONE_YES |
			NV_DPCD_STATUS_LANEXPLUS1_SYMBOL_LOCKED_YES :
			DP_LANE_CR_DONE |
			DP_LANE_CHANNEL_EQ_DONE |
			DP_LANE_SYMBOL_LOCKED;
		if ((data & mask) != mask)
			return -1;
	}
	return 0;
}

static int tegra_dp_channel_eq_status(struct tegra_dp_priv *dp,
				      const struct tegra_dp_link_config *cfg)
{
	u32 cnt;
	u32 n_lanes = cfg->lane_count;
	u8 data;
	u8 ce_done = 1;
	int ret;

	for (cnt = 0; cnt < n_lanes / 2; cnt++) {
		ret = tegra_dc_dp_dpcd_read(dp, DP_LANE0_1_STATUS + cnt, &data);
		if (ret)
			return ret;

		if (n_lanes == 1) {
			ce_done = (data & (0x1 <<
			NV_DPCD_STATUS_LANEX_CHN_EQ_DONE_SHIFT)) &&
			(data & (0x1 <<
			NV_DPCD_STATUS_LANEX_SYMBOL_LOCKED_SHFIT));
			break;
		} else if (!(data & (0x1 <<
				NV_DPCD_STATUS_LANEX_CHN_EQ_DONE_SHIFT)) ||
			   !(data & (0x1 <<
				NV_DPCD_STATUS_LANEX_SYMBOL_LOCKED_SHFIT)) ||
			   !(data & (0x1 <<
				NV_DPCD_STATUS_LANEXPLUS1_CHN_EQ_DONE_SHIFT)) ||
			   !(data & (0x1 <<
				NV_DPCD_STATUS_LANEXPLUS1_SYMBOL_LOCKED_SHIFT)))
			return -EIO;
	}

	if (ce_done) {
		ret = tegra_dc_dp_dpcd_read(dp,
					    DP_LANE_ALIGN_STATUS_UPDATED,
					    &data);
		if (ret)
			return ret;
		if (!(data & NV_DPCD_LANE_ALIGN_STATUS_UPDATED_DONE_YES))
			ce_done = 0;
	}

	return ce_done ? 0 : -EIO;
}

static int tegra_dp_clock_recovery_status(struct tegra_dp_priv *dp,
					 const struct tegra_dp_link_config *cfg)
{
	u32 cnt;
	u32 n_lanes = cfg->lane_count;
	u8 data_ptr;
	int ret;

	for (cnt = 0; cnt < n_lanes / 2; cnt++) {
		ret = tegra_dc_dp_dpcd_read(dp, (DP_LANE0_1_STATUS + cnt),
					    &data_ptr);
		if (ret)
			return ret;

		if (n_lanes == 1)
			return (data_ptr & NV_DPCD_STATUS_LANEX_CR_DONE_YES) ?
				1 : 0;
		else if (!(data_ptr & NV_DPCD_STATUS_LANEX_CR_DONE_YES) ||
			 !(data_ptr & (NV_DPCD_STATUS_LANEXPLUS1_CR_DONE_YES)))
			return 0;
	}

	return 1;
}

static int tegra_dp_lt_adjust(struct tegra_dp_priv *dp, u32 pe[4], u32 vs[4],
			      u32 pc[4], u8 pc_supported,
			      const struct tegra_dp_link_config *cfg)
{
	size_t cnt;
	u8 data_ptr;
	u32 n_lanes = cfg->lane_count;
	int ret;

	for (cnt = 0; cnt < n_lanes / 2; cnt++) {
		ret = tegra_dc_dp_dpcd_read(dp, DP_ADJUST_REQUEST_LANE0_1 + cnt,
					    &data_ptr);
		if (ret)
			return ret;
		pe[2 * cnt] = (data_ptr & NV_DPCD_ADJUST_REQ_LANEX_PE_MASK) >>
					NV_DPCD_ADJUST_REQ_LANEX_PE_SHIFT;
		vs[2 * cnt] = (data_ptr & NV_DPCD_ADJUST_REQ_LANEX_DC_MASK) >>
					NV_DPCD_ADJUST_REQ_LANEX_DC_SHIFT;
		pe[1 + 2 * cnt] =
			(data_ptr & NV_DPCD_ADJUST_REQ_LANEXPLUS1_PE_MASK) >>
					NV_DPCD_ADJUST_REQ_LANEXPLUS1_PE_SHIFT;
		vs[1 + 2 * cnt] =
			(data_ptr & NV_DPCD_ADJUST_REQ_LANEXPLUS1_DC_MASK) >>
					NV_DPCD_ADJUST_REQ_LANEXPLUS1_DC_SHIFT;
	}
	if (pc_supported) {
		ret = tegra_dc_dp_dpcd_read(dp, NV_DPCD_ADJUST_REQ_POST_CURSOR2,
					    &data_ptr);
		if (ret)
			return ret;
		for (cnt = 0; cnt < n_lanes; cnt++) {
			pc[cnt] = (data_ptr >>
			NV_DPCD_ADJUST_REQ_POST_CURSOR2_LANE_SHIFT(cnt)) &
			NV_DPCD_ADJUST_REQ_POST_CURSOR2_LANE_MASK;
		}
	}

	return 0;
}

static void tegra_dp_wait_aux_training(struct tegra_dp_priv *dp,
					bool is_clk_recovery,
					const struct tegra_dp_link_config *cfg)
{
	if (!cfg->aux_rd_interval)
		udelay(is_clk_recovery ? 200 : 500);
	else
		mdelay(cfg->aux_rd_interval * 4);
}

static void tegra_dp_tpg(struct tegra_dp_priv *dp, u32 tp, u32 n_lanes,
			 const struct tegra_dp_link_config *cfg)
{
	u8 data = (tp == training_pattern_disabled)
		? (tp | NV_DPCD_TRAINING_PATTERN_SET_SC_DISABLED_F)
		: (tp | NV_DPCD_TRAINING_PATTERN_SET_SC_DISABLED_T);

	tegra_dc_sor_set_dp_linkctl(dp->sor, 1, tp, cfg);
	tegra_dc_dp_dpcd_write(dp, DP_TRAINING_PATTERN_SET, data);
}

static int tegra_dp_link_config(struct tegra_dp_priv *dp,
				const struct tegra_dp_link_config *link_cfg)
{
	u8 dpcd_data;
	u32 retry;
	int ret;

	if (link_cfg->lane_count == 0) {
		debug("dp: error: lane count is 0. Can not set link config.\n");
		return -ENOLINK;
	}

	/* Set power state if it is not in normal level */
	ret = tegra_dc_dp_dpcd_read(dp, DP_SET_POWER, &dpcd_data);
	if (ret)
		return ret;

	if (dpcd_data == DP_SET_POWER_D3) {
		dpcd_data = DP_SET_POWER_D0;

		/* DP spec requires 3 retries */
		for (retry = 3; retry > 0; --retry) {
			ret = tegra_dc_dp_dpcd_write(dp, DP_SET_POWER,
						     dpcd_data);
			if (!ret)
				break;
			if (retry == 1) {
				debug("dp: Failed to set DP panel power\n");
				return ret;
			}
		}
	}

	/* Enable ASSR if possible */
	if (link_cfg->alt_scramber_reset_cap) {
		ret = tegra_dc_dp_set_assr(dp, dp->sor, 1);
		if (ret)
			return ret;
	}

	ret = tegra_dp_set_link_bandwidth(dp, dp->sor, link_cfg->link_bw);
	if (ret) {
		debug("dp: Failed to set link bandwidth\n");
		return ret;
	}
	ret = tegra_dp_set_lane_count(dp, link_cfg, dp->sor);
	if (ret) {
		debug("dp: Failed to set lane count\n");
		return ret;
	}
	tegra_dc_sor_set_dp_linkctl(dp->sor, 1, training_pattern_none,
				    link_cfg);

	return 0;
}

static int tegra_dp_lower_link_config(struct tegra_dp_priv *dp,
				      const struct display_timing *timing,
				      struct tegra_dp_link_config *cfg)
{
	struct tegra_dp_link_config tmp_cfg;
	int ret;

	tmp_cfg = *cfg;
	cfg->is_valid = 0;

	ret = _tegra_dp_lower_link_config(dp, cfg);
	if (!ret)
		ret = tegra_dc_dp_calc_config(dp, timing, cfg);
	if (!ret)
		ret = tegra_dp_link_config(dp, cfg);
	if (ret)
		goto fail;

	return 0;

fail:
	*cfg = tmp_cfg;
	tegra_dp_link_config(dp, &tmp_cfg);
	return ret;
}

static int tegra_dp_lt_config(struct tegra_dp_priv *dp, u32 pe[4], u32 vs[4],
			      u32 pc[4], const struct tegra_dp_link_config *cfg)
{
	struct udevice *sor = dp->sor;
	u32 n_lanes = cfg->lane_count;
	u8 pc_supported = cfg->tps3_supported;
	u32 cnt;
	u32 val;

	for (cnt = 0; cnt < n_lanes; cnt++) {
		u32 mask = 0;
		u32 pe_reg, vs_reg, pc_reg;
		u32 shift = 0;

		switch (cnt) {
		case 0:
			mask = PR_LANE2_DP_LANE0_MASK;
			shift = PR_LANE2_DP_LANE0_SHIFT;
			break;
		case 1:
			mask = PR_LANE1_DP_LANE1_MASK;
			shift = PR_LANE1_DP_LANE1_SHIFT;
			break;
		case 2:
			mask = PR_LANE0_DP_LANE2_MASK;
			shift = PR_LANE0_DP_LANE2_SHIFT;
			break;
		case 3:
			mask = PR_LANE3_DP_LANE3_MASK;
			shift = PR_LANE3_DP_LANE3_SHIFT;
			break;
		default:
			debug("dp: incorrect lane cnt\n");
			return -EINVAL;
		}

		pe_reg = tegra_dp_pe_regs[pc[cnt]][vs[cnt]][pe[cnt]];
		vs_reg = tegra_dp_vs_regs[pc[cnt]][vs[cnt]][pe[cnt]];
		pc_reg = tegra_dp_pc_regs[pc[cnt]][vs[cnt]][pe[cnt]];

		tegra_dp_set_pe_vs_pc(sor, mask, pe_reg << shift,
				      vs_reg << shift, pc_reg << shift,
				      pc_supported);
	}

	tegra_dp_disable_tx_pu(dp->sor);
	udelay(20);

	for (cnt = 0; cnt < n_lanes; cnt++) {
		u32 max_vs_flag = tegra_dp_is_max_vs(pe[cnt], vs[cnt]);
		u32 max_pe_flag = tegra_dp_is_max_pe(pe[cnt], vs[cnt]);

		val = (vs[cnt] << NV_DPCD_TRAINING_LANEX_SET_DC_SHIFT) |
			(max_vs_flag ?
			NV_DPCD_TRAINING_LANEX_SET_DC_MAX_REACHED_T :
			NV_DPCD_TRAINING_LANEX_SET_DC_MAX_REACHED_F) |
			(pe[cnt] << NV_DPCD_TRAINING_LANEX_SET_PE_SHIFT) |
			(max_pe_flag ?
			NV_DPCD_TRAINING_LANEX_SET_PE_MAX_REACHED_T :
			NV_DPCD_TRAINING_LANEX_SET_PE_MAX_REACHED_F);
		tegra_dc_dp_dpcd_write(dp, (DP_TRAINING_LANE0_SET + cnt), val);
	}

	if (pc_supported) {
		for (cnt = 0; cnt < n_lanes / 2; cnt++) {
			u32 max_pc_flag0 = tegra_dp_is_max_pc(pc[cnt]);
			u32 max_pc_flag1 = tegra_dp_is_max_pc(pc[cnt + 1]);
			val = (pc[cnt] << NV_DPCD_LANEX_SET2_PC2_SHIFT) |
				(max_pc_flag0 ?
				NV_DPCD_LANEX_SET2_PC2_MAX_REACHED_T :
				NV_DPCD_LANEX_SET2_PC2_MAX_REACHED_F) |
				(pc[cnt + 1] <<
				NV_DPCD_LANEXPLUS1_SET2_PC2_SHIFT) |
				(max_pc_flag1 ?
				NV_DPCD_LANEXPLUS1_SET2_PC2_MAX_REACHED_T :
				NV_DPCD_LANEXPLUS1_SET2_PC2_MAX_REACHED_F);
			tegra_dc_dp_dpcd_write(dp,
					       NV_DPCD_TRAINING_LANE0_1_SET2 +
					       cnt, val);
		}
	}

	return 0;
}

static int _tegra_dp_channel_eq(struct tegra_dp_priv *dp, u32 pe[4],
				u32 vs[4], u32 pc[4], u8 pc_supported,
				u32 n_lanes,
				const struct tegra_dp_link_config *cfg)
{
	u32 retry_cnt;

	for (retry_cnt = 0; retry_cnt < 4; retry_cnt++) {
		int ret;

		if (retry_cnt) {
			ret = tegra_dp_lt_adjust(dp, pe, vs, pc, pc_supported,
						 cfg);
			if (ret)
				return ret;
			tegra_dp_lt_config(dp, pe, vs, pc, cfg);
		}

		tegra_dp_wait_aux_training(dp, false, cfg);

		if (!tegra_dp_clock_recovery_status(dp, cfg)) {
			debug("dp: CR failed in channel EQ sequence!\n");
			break;
		}

		if (!tegra_dp_channel_eq_status(dp, cfg))
			return 0;
	}

	return -EIO;
}

static int tegra_dp_channel_eq(struct tegra_dp_priv *dp, u32 pe[4], u32 vs[4],
			       u32 pc[4],
			       const struct tegra_dp_link_config *cfg)
{
	u32 n_lanes = cfg->lane_count;
	u8 pc_supported = cfg->tps3_supported;
	int ret;
	u32 tp_src = training_pattern_2;

	if (pc_supported)
		tp_src = training_pattern_3;

	tegra_dp_tpg(dp, tp_src, n_lanes, cfg);

	ret = _tegra_dp_channel_eq(dp, pe, vs, pc, pc_supported, n_lanes, cfg);

	tegra_dp_tpg(dp, training_pattern_disabled, n_lanes, cfg);

	return ret;
}

static int _tegra_dp_clk_recovery(struct tegra_dp_priv *dp, u32 pe[4],
				  u32 vs[4], u32 pc[4], u8 pc_supported,
				  u32 n_lanes,
				  const struct tegra_dp_link_config *cfg)
{
	u32 vs_temp[4];
	u32 retry_cnt = 0;

	do {
		tegra_dp_lt_config(dp, pe, vs, pc, cfg);
		tegra_dp_wait_aux_training(dp, true, cfg);

		if (tegra_dp_clock_recovery_status(dp, cfg))
			return 0;

		memcpy(vs_temp, vs, sizeof(vs_temp));
		tegra_dp_lt_adjust(dp, pe, vs, pc, pc_supported, cfg);

		if (memcmp(vs_temp, vs, sizeof(vs_temp)))
			retry_cnt = 0;
		else
			++retry_cnt;
	} while (retry_cnt < 5);

	return -EIO;
}

static int tegra_dp_clk_recovery(struct tegra_dp_priv *dp, u32 pe[4],
				 u32 vs[4], u32 pc[4],
				 const struct tegra_dp_link_config *cfg)
{
	u32 n_lanes = cfg->lane_count;
	u8 pc_supported = cfg->tps3_supported;
	int err;

	tegra_dp_tpg(dp, training_pattern_1, n_lanes, cfg);

	err = _tegra_dp_clk_recovery(dp, pe, vs, pc, pc_supported, n_lanes,
				     cfg);
	if (err < 0)
		tegra_dp_tpg(dp, training_pattern_disabled, n_lanes, cfg);

	return err;
}

static int tegra_dc_dp_full_link_training(struct tegra_dp_priv *dp,
					  const struct display_timing *timing,
					  struct tegra_dp_link_config *cfg)
{
	struct udevice *sor = dp->sor;
	int err;
	u32 pe[4], vs[4], pc[4];

	tegra_sor_precharge_lanes(sor, cfg);

retry_cr:
	memset(pe, PREEMPHASIS_DISABLED, sizeof(pe));
	memset(vs, DRIVECURRENT_LEVEL0, sizeof(vs));
	memset(pc, POSTCURSOR2_LEVEL0, sizeof(pc));

	err = tegra_dp_clk_recovery(dp, pe, vs, pc, cfg);
	if (err) {
		if (!tegra_dp_lower_link_config(dp, timing, cfg))
			goto retry_cr;

		debug("dp: clk recovery failed\n");
		goto fail;
	}

	err = tegra_dp_channel_eq(dp, pe, vs, pc, cfg);
	if (err) {
		if (!tegra_dp_lower_link_config(dp, timing, cfg))
			goto retry_cr;

		debug("dp: channel equalization failed\n");
		goto fail;
	}
#ifdef DEBUG
	tegra_dc_dp_dump_link_cfg(dp, cfg);
#endif
	return 0;

fail:
	return err;
}

/*
 * All link training functions are ported from kernel dc driver.
 * See more details at drivers/video/tegra/dc/dp.c
 */
static int tegra_dc_dp_fast_link_training(struct tegra_dp_priv *dp,
		const struct tegra_dp_link_config *link_cfg,
		struct udevice *sor)
{
	u8	link_bw;
	u8	lane_count;
	u16	data16;
	u32	data32;
	u32	size;
	u32	status;
	int	j;
	u32	mask = 0xffff >> ((4 - link_cfg->lane_count) * 4);

	tegra_dc_sor_set_lane_parm(sor, link_cfg);
	tegra_dc_dp_dpcd_write(dp, DP_MAIN_LINK_CHANNEL_CODING_SET,
			       DP_SET_ANSI_8B10B);

	/* Send TP1 */
	tegra_dc_sor_set_dp_linkctl(sor, 1, training_pattern_1, link_cfg);
	tegra_dc_dp_dpcd_write(dp, DP_TRAINING_PATTERN_SET,
			       DP_TRAINING_PATTERN_1);

	for (j = 0; j < link_cfg->lane_count; ++j)
		tegra_dc_dp_dpcd_write(dp, DP_TRAINING_LANE0_SET + j, 0x24);
	udelay(520);

	size = sizeof(data16);
	tegra_dc_dpaux_read(dp, DPAUX_DP_AUXCTL_CMD_AUXRD,
			    DP_LANE0_1_STATUS, (u8 *)&data16, &size, &status);
	status = mask & 0x1111;
	if ((data16 & status) != status) {
		debug("dp: Link training error for TP1 (%#x, status %#x)\n",
		      data16, status);
		return -EFAULT;
	}

	/* enable ASSR */
	tegra_dc_dp_set_assr(dp, sor, link_cfg->scramble_ena);
	tegra_dc_sor_set_dp_linkctl(sor, 1, training_pattern_3, link_cfg);

	tegra_dc_dp_dpcd_write(dp, DP_TRAINING_PATTERN_SET,
			       link_cfg->link_bw == 20 ? 0x23 : 0x22);
	for (j = 0; j < link_cfg->lane_count; ++j)
		tegra_dc_dp_dpcd_write(dp, DP_TRAINING_LANE0_SET + j, 0x24);
	udelay(520);

	size = sizeof(data32);
	tegra_dc_dpaux_read(dp, DPAUX_DP_AUXCTL_CMD_AUXRD, DP_LANE0_1_STATUS,
			    (u8 *)&data32, &size, &status);
	if ((data32 & mask) != (0x7777 & mask)) {
		debug("dp: Link training error for TP2/3 (0x%x)\n", data32);
		return -EFAULT;
	}

	tegra_dc_sor_set_dp_linkctl(sor, 1, training_pattern_disabled,
				    link_cfg);
	tegra_dc_dp_dpcd_write(dp, DP_TRAINING_PATTERN_SET, 0);

	if (tegra_dc_dp_link_trained(dp, link_cfg)) {
		tegra_dc_sor_read_link_config(sor, &link_bw, &lane_count);
		debug("Fast link training failed, link bw %d, lane # %d\n",
		      link_bw, lane_count);
		return -EFAULT;
	}

	debug("Fast link training succeeded, link bw %d, lane %d\n",
	      link_cfg->link_bw, link_cfg->lane_count);

	return 0;
}

static int tegra_dp_do_link_training(struct tegra_dp_priv *dp,
		struct tegra_dp_link_config *link_cfg,
		const struct display_timing *timing,
		struct udevice *sor)
{
	u8	link_bw;
	u8	lane_count;
	int	ret;

	if (DO_FAST_LINK_TRAINING) {
		ret = tegra_dc_dp_fast_link_training(dp, link_cfg, sor);
		if (ret) {
			debug("dp: fast link training failed\n");
		} else {
			/*
			* set to a known-good drive setting if fast link
			* succeeded. Ignore any error.
			*/
			ret = tegra_dc_sor_set_voltage_swing(dp->sor, link_cfg);
			if (ret)
				debug("Failed to set voltage swing\n");
		}
	} else {
		ret = -ENOSYS;
	}
	if (ret) {
		/* Try full link training then */
		ret = tegra_dc_dp_full_link_training(dp, timing, link_cfg);
		if (ret) {
			debug("dp: full link training failed\n");
			return ret;
		}
	}

	/* Everything is good; double check the link config */
	tegra_dc_sor_read_link_config(sor, &link_bw, &lane_count);

	if ((link_cfg->link_bw == link_bw) &&
	    (link_cfg->lane_count == lane_count))
		return 0;
	else
		return -EFAULT;
}

static int tegra_dc_dp_explore_link_cfg(struct tegra_dp_priv *dp,
			struct tegra_dp_link_config *link_cfg,
			struct udevice *sor,
			const struct display_timing *timing)
{
	struct tegra_dp_link_config temp_cfg;

	if (!timing->pixelclock.typ || !timing->hactive.typ ||
	    !timing->vactive.typ) {
		debug("dp: error mode configuration");
		return -EINVAL;
	}
	if (!link_cfg->max_link_bw || !link_cfg->max_lane_count) {
		debug("dp: error link configuration");
		return -EINVAL;
	}

	link_cfg->is_valid = 0;

	memcpy(&temp_cfg, link_cfg, sizeof(temp_cfg));

	temp_cfg.link_bw = temp_cfg.max_link_bw;
	temp_cfg.lane_count = temp_cfg.max_lane_count;

	/*
	 * set to max link config
	 */
	if ((!tegra_dc_dp_calc_config(dp, timing, &temp_cfg)) &&
	    (!tegra_dp_link_config(dp, &temp_cfg)) &&
		(!tegra_dp_do_link_training(dp, &temp_cfg, timing, sor)))
		/* the max link cfg is doable */
		memcpy(link_cfg, &temp_cfg, sizeof(temp_cfg));

	return link_cfg->is_valid ? 0 : -EFAULT;
}

static int tegra_dp_hpd_plug(struct tegra_dp_priv *dp)
{
	const int vdd_to_hpd_delay_ms = 200;
	u32 val;
	ulong start;

	start = get_timer(0);
	do {
		val = tegra_dpaux_readl(dp, DPAUX_DP_AUXSTAT);
		if (val & DPAUX_DP_AUXSTAT_HPD_STATUS_PLUGGED)
			return 0;
		udelay(100);
	} while (get_timer(start) < vdd_to_hpd_delay_ms);

	return -EIO;
}

static int tegra_dc_dp_sink_out_of_sync(struct tegra_dp_priv *dp, u32 delay_ms)
{
	u8 dpcd_data;
	int out_of_sync;
	int ret;

	debug("%s: delay=%d\n", __func__, delay_ms);
	mdelay(delay_ms);
	ret = tegra_dc_dp_dpcd_read(dp, DP_SINK_STATUS, &dpcd_data);
	if (ret)
		return ret;

	out_of_sync = !(dpcd_data & DP_SINK_STATUS_PORT0_IN_SYNC);
	if (out_of_sync)
		debug("SINK receive port 0 out of sync, data=%x\n", dpcd_data);
	else
		debug("SINK is in synchronization\n");

	return out_of_sync;
}

static int tegra_dc_dp_check_sink(struct tegra_dp_priv *dp,
				  struct tegra_dp_link_config *link_cfg,
				  const struct display_timing *timing)
{
	const int max_retry = 5;
	int delay_frame;
	int retries;

	/*
	 * DP TCON may skip some main stream frames, thus we need to wait
	 * some delay before reading the DPCD SINK STATUS register, starting
	 * from 5
	 */
	delay_frame = 5;

	retries = max_retry;
	do {
		int ret;

		if (!tegra_dc_dp_sink_out_of_sync(dp, link_cfg->frame_in_ms *
						  delay_frame))
			return 0;

		debug("%s: retries left %d\n", __func__, retries);
		if (!retries--) {
			printf("DP: Out of sync after %d retries\n", max_retry);
			return -EIO;
		}
		ret = tegra_dc_sor_detach(dp->dc_dev, dp->sor);
		if (ret)
			return ret;
		if (tegra_dc_dp_explore_link_cfg(dp, link_cfg, dp->sor,
						 timing)) {
			debug("dp: %s: error to configure link\n", __func__);
			continue;
		}

		tegra_dc_sor_set_power_state(dp->sor, 1);
		tegra_dc_sor_attach(dp->dc_dev, dp->sor, link_cfg, timing);

		/* Increase delay_frame for next try in case the sink is
		   skipping more frames */
		delay_frame += 10;
	} while (1);
}

int tegra_dp_enable(struct udevice *dev, int panel_bpp,
		    const struct display_timing *timing)
{
	struct tegra_dp_priv *priv = dev_get_priv(dev);
	struct tegra_dp_link_config slink_cfg, *link_cfg = &slink_cfg;
	struct udevice *sor;
	int data;
	int retry;
	int ret;

	memset(link_cfg, '\0', sizeof(*link_cfg));
	link_cfg->is_valid = 0;
	link_cfg->scramble_ena = 1;

	tegra_dc_dpaux_enable(priv);

	if (tegra_dp_hpd_plug(priv) < 0) {
		debug("dp: hpd plug failed\n");
		return -EIO;
	}

	link_cfg->bits_per_pixel = panel_bpp;
	if (tegra_dc_dp_init_max_link_cfg(timing, priv, link_cfg)) {
		debug("dp: failed to init link configuration\n");
		return -ENOLINK;
	}

	ret = uclass_first_device(UCLASS_VIDEO_BRIDGE, &sor);
	if (ret || !sor) {
		debug("dp: failed to find SOR device: ret=%d\n", ret);
		return ret;
	}
	priv->sor = sor;
	ret = tegra_dc_sor_enable_dp(sor, link_cfg);
	if (ret)
		return ret;

	tegra_dc_sor_set_panel_power(sor, 1);

	/* Write power on to DPCD */
	data = DP_SET_POWER_D0;
	retry = 0;
	do {
		ret = tegra_dc_dp_dpcd_write(priv, DP_SET_POWER, data);
	} while ((retry++ < DP_POWER_ON_MAX_TRIES) && ret);

	if (ret || retry >= DP_POWER_ON_MAX_TRIES) {
		debug("dp: failed to power on panel (0x%x)\n", ret);
		return -ENETUNREACH;
		goto error_enable;
	}

	/* Confirm DP plugging status */
	if (!(tegra_dpaux_readl(priv, DPAUX_DP_AUXSTAT) &
			DPAUX_DP_AUXSTAT_HPD_STATUS_PLUGGED)) {
		debug("dp: could not detect HPD\n");
		return -ENXIO;
	}

	/* Check DP version */
	if (tegra_dc_dp_dpcd_read(priv, DP_DPCD_REV, &priv->revision)) {
		debug("dp: failed to read the revision number from sink\n");
		return -EIO;
	}

	if (tegra_dc_dp_explore_link_cfg(priv, link_cfg, sor, timing)) {
		debug("dp: error configuring link\n");
		return -ENOMEDIUM;
	}

	tegra_dc_sor_set_power_state(sor, 1);
	ret = tegra_dc_sor_attach(priv->dc_dev, sor, link_cfg, timing);
	if (ret && ret != -EEXIST)
		return ret;

	/*
	 * This takes a long time, but can apparently resolve a failure to
	 * bring up the display correctly.
	 */
	if (0) {
		ret = tegra_dc_dp_check_sink(priv, link_cfg, timing);
		if (ret)
			return ret;
	}

	/* Power down the unused lanes to save power - a few hundred mW */
	tegra_dc_sor_power_down_unused_lanes(sor, link_cfg);

	ret = video_bridge_set_backlight(sor, 80);
	if (ret) {
		debug("dp: failed to set backlight\n");
		return ret;
	}

	priv->enabled = true;
error_enable:
	return 0;
}

static int tegra_dp_ofdata_to_platdata(struct udevice *dev)
{
	struct tegra_dp_plat *plat = dev_get_platdata(dev);

	plat->base = dev_read_addr(dev);

	return 0;
}

static int tegra_dp_read_edid(struct udevice *dev, u8 *buf, int buf_size)
{
	struct tegra_dp_priv *priv = dev_get_priv(dev);
	const int tegra_edid_i2c_address = 0x50;
	u32 aux_stat = 0;

	tegra_dc_dpaux_enable(priv);

	return tegra_dc_i2c_aux_read(priv, tegra_edid_i2c_address, 0, buf,
				     buf_size, &aux_stat);
}

static const struct dm_display_ops dp_tegra_ops = {
	.read_edid = tegra_dp_read_edid,
	.enable = tegra_dp_enable,
};

static int dp_tegra_probe(struct udevice *dev)
{
	struct tegra_dp_plat *plat = dev_get_platdata(dev);
	struct tegra_dp_priv *priv = dev_get_priv(dev);
	struct display_plat *disp_uc_plat = dev_get_uclass_platdata(dev);

	priv->regs = (struct dpaux_ctlr *)plat->base;
	priv->enabled = false;

	/* Remember the display controller that is sending us video */
	priv->dc_dev = disp_uc_plat->src_dev;

	return 0;
}

static const struct udevice_id tegra_dp_ids[] = {
	{ .compatible = "nvidia,tegra124-dpaux" },
	{ }
};

U_BOOT_DRIVER(dp_tegra) = {
	.name	= "dpaux_tegra",
	.id	= UCLASS_DISPLAY,
	.of_match = tegra_dp_ids,
	.ofdata_to_platdata = tegra_dp_ofdata_to_platdata,
	.probe	= dp_tegra_probe,
	.ops	= &dp_tegra_ops,
	.priv_auto_alloc_size = sizeof(struct tegra_dp_priv),
	.platdata_auto_alloc_size = sizeof(struct tegra_dp_plat),
};
