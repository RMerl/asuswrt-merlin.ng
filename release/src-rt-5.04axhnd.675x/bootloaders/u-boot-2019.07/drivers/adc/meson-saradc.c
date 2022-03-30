// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * Amlogic Meson Successive Approximation Register (SAR) A/D Converter
 */

#include <common.h>
#include <adc.h>
#include <clk.h>
#include <dm.h>
#include <regmap.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/math64.h>
#include <linux/bitfield.h>

#define MESON_SAR_ADC_REG0					0x00
	#define MESON_SAR_ADC_REG0_PANEL_DETECT			BIT(31)
	#define MESON_SAR_ADC_REG0_BUSY_MASK			GENMASK(30, 28)
	#define MESON_SAR_ADC_REG0_DELTA_BUSY			BIT(30)
	#define MESON_SAR_ADC_REG0_AVG_BUSY			BIT(29)
	#define MESON_SAR_ADC_REG0_SAMPLE_BUSY			BIT(28)
	#define MESON_SAR_ADC_REG0_FIFO_FULL			BIT(27)
	#define MESON_SAR_ADC_REG0_FIFO_EMPTY			BIT(26)
	#define MESON_SAR_ADC_REG0_FIFO_COUNT_MASK		GENMASK(25, 21)
	#define MESON_SAR_ADC_REG0_ADC_BIAS_CTRL_MASK		GENMASK(20, 19)
	#define MESON_SAR_ADC_REG0_CURR_CHAN_ID_MASK		GENMASK(18, 16)
	#define MESON_SAR_ADC_REG0_ADC_TEMP_SEN_SEL		BIT(15)
	#define MESON_SAR_ADC_REG0_SAMPLING_STOP		BIT(14)
	#define MESON_SAR_ADC_REG0_CHAN_DELTA_EN_MASK		GENMASK(13, 12)
	#define MESON_SAR_ADC_REG0_DETECT_IRQ_POL		BIT(10)
	#define MESON_SAR_ADC_REG0_DETECT_IRQ_EN		BIT(9)
	#define MESON_SAR_ADC_REG0_FIFO_CNT_IRQ_MASK		GENMASK(8, 4)
	#define MESON_SAR_ADC_REG0_FIFO_IRQ_EN			BIT(3)
	#define MESON_SAR_ADC_REG0_SAMPLING_START		BIT(2)
	#define MESON_SAR_ADC_REG0_CONTINUOUS_EN		BIT(1)
	#define MESON_SAR_ADC_REG0_SAMPLE_ENGINE_ENABLE		BIT(0)

#define MESON_SAR_ADC_CHAN_LIST					0x04
	#define MESON_SAR_ADC_CHAN_LIST_MAX_INDEX_MASK		GENMASK(26, 24)
	#define MESON_SAR_ADC_CHAN_LIST_ENTRY_MASK(_chan)	\
					(GENMASK(2, 0) << ((_chan) * 3))

#define MESON_SAR_ADC_AVG_CNTL					0x08
	#define MESON_SAR_ADC_AVG_CNTL_AVG_MODE_SHIFT(_chan)	\
					(16 + ((_chan) * 2))
	#define MESON_SAR_ADC_AVG_CNTL_AVG_MODE_MASK(_chan)	\
					(GENMASK(17, 16) << ((_chan) * 2))
	#define MESON_SAR_ADC_AVG_CNTL_NUM_SAMPLES_SHIFT(_chan)	\
					(0 + ((_chan) * 2))
	#define MESON_SAR_ADC_AVG_CNTL_NUM_SAMPLES_MASK(_chan)	\
					(GENMASK(1, 0) << ((_chan) * 2))

#define MESON_SAR_ADC_REG3					0x0c
	#define MESON_SAR_ADC_REG3_CNTL_USE_SC_DLY		BIT(31)
	#define MESON_SAR_ADC_REG3_CLK_EN			BIT(30)
	#define MESON_SAR_ADC_REG3_BL30_INITIALIZED		BIT(28)
	#define MESON_SAR_ADC_REG3_CTRL_CONT_RING_COUNTER_EN	BIT(27)
	#define MESON_SAR_ADC_REG3_CTRL_SAMPLING_CLOCK_PHASE	BIT(26)
	#define MESON_SAR_ADC_REG3_CTRL_CHAN7_MUX_SEL_MASK	GENMASK(25, 23)
	#define MESON_SAR_ADC_REG3_DETECT_EN			BIT(22)
	#define MESON_SAR_ADC_REG3_ADC_EN			BIT(21)
	#define MESON_SAR_ADC_REG3_PANEL_DETECT_COUNT_MASK	GENMASK(20, 18)
	#define MESON_SAR_ADC_REG3_PANEL_DETECT_FILTER_TB_MASK	GENMASK(17, 16)
	#define MESON_SAR_ADC_REG3_ADC_CLK_DIV_SHIFT		10
	#define MESON_SAR_ADC_REG3_ADC_CLK_DIV_WIDTH		5
	#define MESON_SAR_ADC_REG3_BLOCK_DLY_SEL_MASK		GENMASK(9, 8)
	#define MESON_SAR_ADC_REG3_BLOCK_DLY_MASK		GENMASK(7, 0)

#define MESON_SAR_ADC_DELAY					0x10
	#define MESON_SAR_ADC_DELAY_INPUT_DLY_SEL_MASK		GENMASK(25, 24)
	#define MESON_SAR_ADC_DELAY_BL30_BUSY			BIT(15)
	#define MESON_SAR_ADC_DELAY_KERNEL_BUSY			BIT(14)
	#define MESON_SAR_ADC_DELAY_INPUT_DLY_CNT_MASK		GENMASK(23, 16)
	#define MESON_SAR_ADC_DELAY_SAMPLE_DLY_SEL_MASK		GENMASK(9, 8)
	#define MESON_SAR_ADC_DELAY_SAMPLE_DLY_CNT_MASK		GENMASK(7, 0)

#define MESON_SAR_ADC_LAST_RD					0x14
	#define MESON_SAR_ADC_LAST_RD_LAST_CHANNEL1_MASK	GENMASK(23, 16)
	#define MESON_SAR_ADC_LAST_RD_LAST_CHANNEL0_MASK	GENMASK(9, 0)

#define MESON_SAR_ADC_FIFO_RD					0x18
	#define MESON_SAR_ADC_FIFO_RD_CHAN_ID_MASK		GENMASK(14, 12)
	#define MESON_SAR_ADC_FIFO_RD_SAMPLE_VALUE_MASK		GENMASK(11, 0)

#define MESON_SAR_ADC_AUX_SW					0x1c
	#define MESON_SAR_ADC_AUX_SW_MUX_SEL_CHAN_SHIFT(_chan)	\
					(8 + (((_chan) - 2) * 3))
	#define MESON_SAR_ADC_AUX_SW_VREF_P_MUX			BIT(6)
	#define MESON_SAR_ADC_AUX_SW_VREF_N_MUX			BIT(5)
	#define MESON_SAR_ADC_AUX_SW_MODE_SEL			BIT(4)
	#define MESON_SAR_ADC_AUX_SW_YP_DRIVE_SW		BIT(3)
	#define MESON_SAR_ADC_AUX_SW_XP_DRIVE_SW		BIT(2)
	#define MESON_SAR_ADC_AUX_SW_YM_DRIVE_SW		BIT(1)
	#define MESON_SAR_ADC_AUX_SW_XM_DRIVE_SW		BIT(0)

#define MESON_SAR_ADC_CHAN_10_SW				0x20
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_MUX_SEL_MASK	GENMASK(25, 23)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_VREF_P_MUX	BIT(22)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_VREF_N_MUX	BIT(21)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_MODE_SEL		BIT(20)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_YP_DRIVE_SW	BIT(19)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_XP_DRIVE_SW	BIT(18)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_YM_DRIVE_SW	BIT(17)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN1_XM_DRIVE_SW	BIT(16)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_MUX_SEL_MASK	GENMASK(9, 7)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_VREF_P_MUX	BIT(6)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_VREF_N_MUX	BIT(5)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_MODE_SEL		BIT(4)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_YP_DRIVE_SW	BIT(3)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_XP_DRIVE_SW	BIT(2)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_YM_DRIVE_SW	BIT(1)
	#define MESON_SAR_ADC_CHAN_10_SW_CHAN0_XM_DRIVE_SW	BIT(0)

#define MESON_SAR_ADC_DETECT_IDLE_SW				0x24
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_SW_EN	BIT(26)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_MUX_MASK	GENMASK(25, 23)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_VREF_P_MUX	BIT(22)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_VREF_N_MUX	BIT(21)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_MODE_SEL	BIT(20)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_YP_DRIVE_SW	BIT(19)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_XP_DRIVE_SW	BIT(18)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_YM_DRIVE_SW	BIT(17)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_XM_DRIVE_SW	BIT(16)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_MUX_SEL_MASK	GENMASK(9, 7)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_VREF_P_MUX	BIT(6)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_VREF_N_MUX	BIT(5)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_MODE_SEL	BIT(4)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_YP_DRIVE_SW	BIT(3)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_XP_DRIVE_SW	BIT(2)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_YM_DRIVE_SW	BIT(1)
	#define MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_XM_DRIVE_SW	BIT(0)

#define MESON_SAR_ADC_DELTA_10					0x28
	#define MESON_SAR_ADC_DELTA_10_TEMP_SEL			BIT(27)
	#define MESON_SAR_ADC_DELTA_10_TS_REVE1			BIT(26)
	#define MESON_SAR_ADC_DELTA_10_CHAN1_DELTA_VALUE_MASK	GENMASK(25, 16)
	#define MESON_SAR_ADC_DELTA_10_TS_REVE0			BIT(15)
	#define MESON_SAR_ADC_DELTA_10_TS_C_SHIFT		11
	#define MESON_SAR_ADC_DELTA_10_TS_C_MASK		GENMASK(14, 11)
	#define MESON_SAR_ADC_DELTA_10_TS_VBG_EN		BIT(10)
	#define MESON_SAR_ADC_DELTA_10_CHAN0_DELTA_VALUE_MASK	GENMASK(9, 0)

/*
 * NOTE: registers from here are undocumented (the vendor Linux kernel driver
 * and u-boot source served as reference). These only seem to be relevant on
 * GXBB and newer.
 */
#define MESON_SAR_ADC_REG11					0x2c
	#define MESON_SAR_ADC_REG11_BANDGAP_EN			BIT(13)

#define MESON_SAR_ADC_REG13					0x34
	#define MESON_SAR_ADC_REG13_12BIT_CALIBRATION_MASK	GENMASK(13, 8)

#define MESON_SAR_ADC_MAX_FIFO_SIZE				32
#define MESON_SAR_ADC_TIMEOUT					100 /* ms */

#define NUM_CHANNELS						8

#define MILLION							1000000

struct meson_saradc_data {
	int				num_bits;
};

struct meson_saradc_priv {
	const struct meson_saradc_data	*data;
	struct regmap			*regmap;
	struct clk			core_clk;
	struct clk			adc_clk;
	bool				initialized;
	int				active_channel;
	int				calibbias;
	int				calibscale;
};

static unsigned int
meson_saradc_get_fifo_count(struct meson_saradc_priv *priv)
{
	u32 regval;

	regmap_read(priv->regmap, MESON_SAR_ADC_REG0, &regval);

	return FIELD_GET(MESON_SAR_ADC_REG0_FIFO_COUNT_MASK, regval);
}

static int meson_saradc_lock(struct meson_saradc_priv *priv)
{
	uint val, timeout = 10000;

	/* prevent BL30 from using the SAR ADC while we are using it */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELAY,
			   MESON_SAR_ADC_DELAY_KERNEL_BUSY,
			   MESON_SAR_ADC_DELAY_KERNEL_BUSY);

	/*
	 * wait until BL30 releases it's lock (so we can use the SAR ADC)
	 */
	do {
		udelay(1);
		regmap_read(priv->regmap, MESON_SAR_ADC_DELAY, &val);
	} while (val & MESON_SAR_ADC_DELAY_BL30_BUSY && timeout--);

	if (timeout < 0) {
		printf("Timeout while waiting for BL30 unlock\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static void meson_saradc_unlock(struct meson_saradc_priv *priv)
{
	/* allow BL30 to use the SAR ADC again */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELAY,
			   MESON_SAR_ADC_DELAY_KERNEL_BUSY, 0);
}

static void meson_saradc_clear_fifo(struct meson_saradc_priv *priv)
{
	unsigned int count, tmp;

	for (count = 0; count < MESON_SAR_ADC_MAX_FIFO_SIZE; count++) {
		if (!meson_saradc_get_fifo_count(priv))
			break;

		regmap_read(priv->regmap, MESON_SAR_ADC_FIFO_RD, &tmp);
	}
}

static int meson_saradc_calib_val(struct meson_saradc_priv *priv, int val)
{
	int tmp;

	/* use val_calib = scale * val_raw + offset calibration function */
	tmp = div_s64((s64)val * priv->calibscale, MILLION) + priv->calibbias;

	return clamp(tmp, 0, (1 << priv->data->num_bits) - 1);
}

static int meson_saradc_wait_busy_clear(struct meson_saradc_priv *priv)
{
	uint regval, timeout = 10000;

	/*
	 * NOTE: we need a small delay before reading the status, otherwise
	 * the sample engine may not have started internally (which would
	 * seem to us that sampling is already finished).
	 */
	do {
		udelay(1);
		regmap_read(priv->regmap, MESON_SAR_ADC_REG0, &regval);
	} while (FIELD_GET(MESON_SAR_ADC_REG0_BUSY_MASK, regval) && timeout--);

	if (timeout < 0)
		return -ETIMEDOUT;

	return 0;
}

static int meson_saradc_read_raw_sample(struct meson_saradc_priv *priv,
					unsigned int channel, uint *val)
{
	uint regval, fifo_chan, fifo_val, count;
	int ret;

	ret = meson_saradc_wait_busy_clear(priv);
	if (ret)
		return ret;

	count = meson_saradc_get_fifo_count(priv);
	if (count != 1) {
		printf("ADC FIFO has %d element(s) instead of one\n", count);
		return -EINVAL;
	}

	regmap_read(priv->regmap, MESON_SAR_ADC_FIFO_RD, &regval);
	fifo_chan = FIELD_GET(MESON_SAR_ADC_FIFO_RD_CHAN_ID_MASK, regval);
	if (fifo_chan != channel) {
		printf("ADC FIFO entry belongs to channel %d instead of %d\n",
		       fifo_chan, channel);
		return -EINVAL;
	}

	fifo_val = FIELD_GET(MESON_SAR_ADC_FIFO_RD_SAMPLE_VALUE_MASK, regval);
	fifo_val &= GENMASK(priv->data->num_bits - 1, 0);
	*val = meson_saradc_calib_val(priv, fifo_val);

	return 0;
}

static void meson_saradc_start_sample_engine(struct meson_saradc_priv *priv)
{
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_FIFO_IRQ_EN,
			   MESON_SAR_ADC_REG0_FIFO_IRQ_EN);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_SAMPLE_ENGINE_ENABLE,
			   MESON_SAR_ADC_REG0_SAMPLE_ENGINE_ENABLE);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_SAMPLING_START,
			   MESON_SAR_ADC_REG0_SAMPLING_START);
}

static void meson_saradc_stop_sample_engine(struct meson_saradc_priv *priv)
{
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_FIFO_IRQ_EN, 0);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_SAMPLING_STOP,
			   MESON_SAR_ADC_REG0_SAMPLING_STOP);

	/* wait until all modules are stopped */
	meson_saradc_wait_busy_clear(priv);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_SAMPLE_ENGINE_ENABLE, 0);
}

enum meson_saradc_avg_mode {
	NO_AVERAGING = 0x0,
	MEAN_AVERAGING = 0x1,
	MEDIAN_AVERAGING = 0x2,
};

enum meson_saradc_num_samples {
	ONE_SAMPLE = 0x0,
	TWO_SAMPLES = 0x1,
	FOUR_SAMPLES = 0x2,
	EIGHT_SAMPLES = 0x3,
};

static void meson_saradc_set_averaging(struct meson_saradc_priv *priv,
				       unsigned int channel,
				       enum meson_saradc_avg_mode mode,
				       enum meson_saradc_num_samples samples)
{
	int val;

	val = samples << MESON_SAR_ADC_AVG_CNTL_NUM_SAMPLES_SHIFT(channel);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_AVG_CNTL,
			   MESON_SAR_ADC_AVG_CNTL_NUM_SAMPLES_MASK(channel),
			   val);

	val = mode << MESON_SAR_ADC_AVG_CNTL_AVG_MODE_SHIFT(channel);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_AVG_CNTL,
			   MESON_SAR_ADC_AVG_CNTL_AVG_MODE_MASK(channel), val);
}

static void meson_saradc_enable_channel(struct meson_saradc_priv *priv,
					unsigned int channel)
{
	uint regval;

	/*
	 * the SAR ADC engine allows sampling multiple channels at the same
	 * time. to keep it simple we're only working with one *internal*
	 * channel, which starts counting at index 0 (which means: count = 1).
	 */
	regval = FIELD_PREP(MESON_SAR_ADC_CHAN_LIST_MAX_INDEX_MASK, 0);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_CHAN_LIST,
			   MESON_SAR_ADC_CHAN_LIST_MAX_INDEX_MASK, regval);

	/* map channel index 0 to the channel which we want to read */
	regval = FIELD_PREP(MESON_SAR_ADC_CHAN_LIST_ENTRY_MASK(0), channel);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_CHAN_LIST,
			   MESON_SAR_ADC_CHAN_LIST_ENTRY_MASK(0), regval);

	regval = FIELD_PREP(MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_MUX_MASK,
			    channel);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DETECT_IDLE_SW,
			   MESON_SAR_ADC_DETECT_IDLE_SW_DETECT_MUX_MASK,
			   regval);

	regval = FIELD_PREP(MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_MUX_SEL_MASK,
			    channel);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DETECT_IDLE_SW,
			   MESON_SAR_ADC_DETECT_IDLE_SW_IDLE_MUX_SEL_MASK,
			   regval);

	if (channel == 6)
		regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELTA_10,
				   MESON_SAR_ADC_DELTA_10_TEMP_SEL, 0);
}

static int meson_saradc_get_sample(struct meson_saradc_priv *priv,
				   int chan, uint *val)
{
	int ret;

	ret = meson_saradc_lock(priv);
	if (ret)
		return ret;

	/* clear the FIFO to make sure we're not reading old values */
	meson_saradc_clear_fifo(priv);

	meson_saradc_set_averaging(priv, chan, MEAN_AVERAGING, EIGHT_SAMPLES);

	meson_saradc_enable_channel(priv, chan);

	meson_saradc_start_sample_engine(priv);
	ret = meson_saradc_read_raw_sample(priv, chan, val);
	meson_saradc_stop_sample_engine(priv);

	meson_saradc_unlock(priv);

	if (ret) {
		printf("failed to read sample for channel %d: %d\n",
		       chan, ret);
		return ret;
	}

	return 0;
}

static int meson_saradc_channel_data(struct udevice *dev, int channel,
				     unsigned int *data)
{
	struct meson_saradc_priv *priv = dev_get_priv(dev);

	if (channel != priv->active_channel) {
		pr_err("Requested channel is not active!");
		return -EINVAL;
	}

	return meson_saradc_get_sample(priv, channel, data);
}

enum meson_saradc_chan7_mux_sel {
	CHAN7_MUX_VSS = 0x0,
	CHAN7_MUX_VDD_DIV4 = 0x1,
	CHAN7_MUX_VDD_DIV2 = 0x2,
	CHAN7_MUX_VDD_MUL3_DIV4 = 0x3,
	CHAN7_MUX_VDD = 0x4,
	CHAN7_MUX_CH7_INPUT = 0x7,
};

static void meson_saradc_set_chan7_mux(struct meson_saradc_priv *priv,
				       enum meson_saradc_chan7_mux_sel sel)
{
	u32 regval;

	regval = FIELD_PREP(MESON_SAR_ADC_REG3_CTRL_CHAN7_MUX_SEL_MASK, sel);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG3,
			   MESON_SAR_ADC_REG3_CTRL_CHAN7_MUX_SEL_MASK, regval);

	udelay(20);
}

static int meson_saradc_calib(struct meson_saradc_priv *priv)
{
	uint nominal0, nominal1, value0, value1;
	int ret;

	/* use points 25% and 75% for calibration */
	nominal0 = (1 << priv->data->num_bits) / 4;
	nominal1 = (1 << priv->data->num_bits) * 3 / 4;

	meson_saradc_set_chan7_mux(priv, CHAN7_MUX_VDD_DIV4);
	udelay(20);
	ret = meson_saradc_get_sample(priv, 7, &value0);
	if (ret < 0)
		goto out;

	meson_saradc_set_chan7_mux(priv, CHAN7_MUX_VDD_MUL3_DIV4);
	udelay(20);
	ret = meson_saradc_get_sample(priv, 7, &value1);
	if (ret < 0)
		goto out;

	if (value1 <= value0) {
		ret = -EINVAL;
		goto out;
	}

	priv->calibscale = div_s64((nominal1 - nominal0) * (s64)MILLION,
				   value1 - value0);
	priv->calibbias = nominal0 - div_s64((s64)value0 * priv->calibscale,
					     MILLION);
	ret = 0;
out:
	meson_saradc_set_chan7_mux(priv, CHAN7_MUX_CH7_INPUT);

	return ret;
}

static int meson_saradc_init(struct meson_saradc_priv *priv)
{
	uint regval;
	int ret, i;

	priv->calibscale = MILLION;

	/*
	 * make sure we start at CH7 input since the other muxes are only used
	 * for internal calibration.
	 */
	meson_saradc_set_chan7_mux(priv, CHAN7_MUX_CH7_INPUT);

	/*
	 * leave sampling delay and the input clocks as configured by
	 * BL30 to make sure BL30 gets the values it expects when
	 * reading the temperature sensor.
	 */
	regmap_read(priv->regmap, MESON_SAR_ADC_REG3, &regval);
	if (regval & MESON_SAR_ADC_REG3_BL30_INITIALIZED)
		return 0;

	meson_saradc_stop_sample_engine(priv);

	/* update the channel 6 MUX to select the temperature sensor */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_ADC_TEMP_SEN_SEL,
			   MESON_SAR_ADC_REG0_ADC_TEMP_SEN_SEL);

	/* disable all channels by default */
	regmap_write(priv->regmap, MESON_SAR_ADC_CHAN_LIST, 0x0);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG3,
			   MESON_SAR_ADC_REG3_CTRL_SAMPLING_CLOCK_PHASE, 0);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG3,
			   MESON_SAR_ADC_REG3_CNTL_USE_SC_DLY,
			   MESON_SAR_ADC_REG3_CNTL_USE_SC_DLY);

	/* delay between two samples = (10+1) * 1uS */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELAY,
			   MESON_SAR_ADC_DELAY_INPUT_DLY_CNT_MASK,
			   FIELD_PREP(MESON_SAR_ADC_DELAY_SAMPLE_DLY_CNT_MASK,
				      10));
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELAY,
			   MESON_SAR_ADC_DELAY_SAMPLE_DLY_SEL_MASK,
			   FIELD_PREP(MESON_SAR_ADC_DELAY_SAMPLE_DLY_SEL_MASK,
				      0));

	/* delay between two samples = (10+1) * 1uS */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELAY,
			   MESON_SAR_ADC_DELAY_INPUT_DLY_CNT_MASK,
			   FIELD_PREP(MESON_SAR_ADC_DELAY_INPUT_DLY_CNT_MASK,
				      10));
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_DELAY,
			   MESON_SAR_ADC_DELAY_INPUT_DLY_SEL_MASK,
			   FIELD_PREP(MESON_SAR_ADC_DELAY_INPUT_DLY_SEL_MASK,
				      1));

	/*
	 * set up the input channel muxes in MESON_SAR_ADC_CHAN_10_SW
	 * (0 = SAR_ADC_CH0, 1 = SAR_ADC_CH1)
	 */
	regval = FIELD_PREP(MESON_SAR_ADC_CHAN_10_SW_CHAN0_MUX_SEL_MASK, 0);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_CHAN_10_SW,
			   MESON_SAR_ADC_CHAN_10_SW_CHAN0_MUX_SEL_MASK,
			   regval);
	regval = FIELD_PREP(MESON_SAR_ADC_CHAN_10_SW_CHAN1_MUX_SEL_MASK, 1);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_CHAN_10_SW,
			   MESON_SAR_ADC_CHAN_10_SW_CHAN1_MUX_SEL_MASK,
			   regval);

	/*
	 * set up the input channel muxes in MESON_SAR_ADC_AUX_SW
	 * (2 = SAR_ADC_CH2, 3 = SAR_ADC_CH3, ...) and enable
	 * MESON_SAR_ADC_AUX_SW_YP_DRIVE_SW and
	 * MESON_SAR_ADC_AUX_SW_XP_DRIVE_SW like the vendor driver.
	 */
	regval = 0;
	for (i = 2; i <= 7; i++)
		regval |= i << MESON_SAR_ADC_AUX_SW_MUX_SEL_CHAN_SHIFT(i);
	regval |= MESON_SAR_ADC_AUX_SW_YP_DRIVE_SW;
	regval |= MESON_SAR_ADC_AUX_SW_XP_DRIVE_SW;
	regmap_write(priv->regmap, MESON_SAR_ADC_AUX_SW, regval);

	ret = meson_saradc_lock(priv);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable(&priv->core_clk);
	if (ret)
		return ret;
#endif

	regval = FIELD_PREP(MESON_SAR_ADC_REG0_FIFO_CNT_IRQ_MASK, 1);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG0,
			   MESON_SAR_ADC_REG0_FIFO_CNT_IRQ_MASK, regval);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_BANDGAP_EN,
			   MESON_SAR_ADC_REG11_BANDGAP_EN);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG3,
			   MESON_SAR_ADC_REG3_ADC_EN,
			   MESON_SAR_ADC_REG3_ADC_EN);

	udelay(5);

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_enable(&priv->adc_clk);
	if (ret)
		return ret;
#endif

	meson_saradc_unlock(priv);

	ret = meson_saradc_calib(priv);
	if (ret) {
		printf("calibration failed\n");
		return -EIO;
	}

	return 0;
}

static int meson_saradc_start_channel(struct udevice *dev, int channel)
{
	struct meson_saradc_priv *priv = dev_get_priv(dev);

	if (channel < 0 || channel >= NUM_CHANNELS) {
		printf("Requested channel is invalid!");
		return -EINVAL;
	}

	if (!priv->initialized) {
		int ret;

		ret = meson_saradc_init(priv);
		if (ret)
			return ret;

		priv->initialized = true;
	}

	priv->active_channel = channel;

	return 0;
}

static int meson_saradc_stop(struct udevice *dev)
{
	struct meson_saradc_priv *priv = dev_get_priv(dev);

	priv->active_channel = -1;

	return 0;
}

static int meson_saradc_probe(struct udevice *dev)
{
	struct meson_saradc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

#if CONFIG_IS_ENABLED(CLK)
	ret = clk_get_by_name(dev, "core", &priv->core_clk);
	if (ret)
		return ret;

	ret = clk_get_by_name(dev, "adc_clk", &priv->adc_clk);
	if (ret)
		return ret;
#endif

	priv->active_channel = -1;

	return 0;
}

int meson_saradc_ofdata_to_platdata(struct udevice *dev)
{
	struct adc_uclass_platdata *uc_pdata = dev_get_uclass_platdata(dev);
	struct meson_saradc_priv *priv = dev_get_priv(dev);

	priv->data = (struct meson_saradc_data *)dev_get_driver_data(dev);

	uc_pdata->data_mask = GENMASK(priv->data->num_bits - 1, 0);
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = MESON_SAR_ADC_TIMEOUT * 1000;
	uc_pdata->channel_mask = GENMASK(NUM_CHANNELS - 1, 0);

	return 0;
}

static const struct adc_ops meson_saradc_ops = {
	.start_channel = meson_saradc_start_channel,
	.channel_data = meson_saradc_channel_data,
	.stop = meson_saradc_stop,
};

static const struct meson_saradc_data gxbb_saradc_data = {
	.num_bits = 10,
};

static const struct meson_saradc_data gxl_saradc_data = {
	.num_bits = 12,
};

static const struct udevice_id meson_saradc_ids[] = {
	{ .compatible = "amlogic,meson-gxbb-saradc",
	  .data = (ulong)&gxbb_saradc_data },
	{ .compatible = "amlogic,meson-gxl-saradc",
	  .data = (ulong)&gxl_saradc_data },
	{ .compatible = "amlogic,meson-gxm-saradc",
	  .data = (ulong)&gxl_saradc_data },
	{ }
};

U_BOOT_DRIVER(meson_saradc) = {
	.name		= "meson_saradc",
	.id		= UCLASS_ADC,
	.of_match	= meson_saradc_ids,
	.ops		= &meson_saradc_ops,
	.probe		= meson_saradc_probe,
	.ofdata_to_platdata = meson_saradc_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct meson_saradc_priv),
};
