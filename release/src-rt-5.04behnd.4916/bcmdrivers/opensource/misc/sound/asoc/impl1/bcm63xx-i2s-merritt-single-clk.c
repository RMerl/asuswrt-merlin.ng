/*********************************************************************
 * bcm63xx-i2s-merritt.c -- ALSA SoC Audio Layer - Broadcom I2S/TDM Controller driver
 * 
 * Copyright (c) 2020 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 **********************************************************************/

#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "bcm63xx-i2stdm.h"

#define DRV_NAME "brcm-i2s"

#define MIC_MUTE_LOOP_DELAY 50 /*ms*/

static struct task_struct *micMuteCtlTask;

int kthreadfunc_micmute(void *data)
{
	int mute_state=0,state_prev,state_toggle=0;
	struct bcm_i2s_priv *i2s_priv = (struct bcm_i2s_priv *)data;
	while (!kthread_should_stop()) {
		if (i2s_priv->mic_mute_detect_gpio) {
			state_prev = mute_state;
			/* get button status*/
			mute_state = gpiod_get_value(i2s_priv->mic_mute_detect_gpio);
			if (mute_state > state_prev && i2s_priv->mic_power_en_gpio)
				gpiod_direction_output(i2s_priv->mic_power_en_gpio,state_toggle++%2);
		}
		msleep(MIC_MUTE_LOOP_DELAY);
	}
	do_exit(0);
}

static bool brcm_i2s_wr_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case I2S_TX_CFG ... I2S_TX_DESC_IFF_LEN:
	case I2S_TX_CFG_2 ... I2S_RX_DESC_IFF_LEN:
	case I2S_RX_CFG_2 ... I2S_REG_MAX:
		return true;
	default:
		return false;
	}
}

static bool brcm_i2s_rd_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case I2S_TX_CFG ... I2S_REG_MAX:
		return true;
	default:
		return false;
	}
}

static bool brcm_i2s_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case I2S_TX_CFG:
	case I2S_TX_IRQ_CTL:
	case I2S_TX_DESC_IFF_ADDR:
	case I2S_TX_DESC_IFF_LEN:
	case I2S_TX_DESC_OFF_ADDR:
	case I2S_TX_DESC_OFF_LEN:
	case I2S_TX_CFG_2:
	case I2S_RX_CFG:
	case I2S_RX_IRQ_CTL:
	case I2S_RX_DESC_OFF_ADDR:
	case I2S_RX_DESC_OFF_LEN:
	case I2S_RX_DESC_IFF_LEN:
	case I2S_RX_DESC_IFF_ADDR:
	case I2S_RX_CFG_2:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config brcm_i2s_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = I2S_REG_MAX,
	.writeable_reg = brcm_i2s_wr_reg,
	.readable_reg = brcm_i2s_rd_reg,
	.volatile_reg = brcm_i2s_volatile_reg,
	.cache_type = REGCACHE_FLAT,
};

/* Set I2S DAI format */
static int bcm63xx_i2s_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	struct bcm_i2s_priv *i2s_priv = snd_soc_dai_get_drvdata(cpu_dai);
	struct regmap *regmap_i2s = i2s_priv->regmap_i2s;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) 
	{
		case SND_SOC_DAIFMT_CBS_CFS:
			regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
			                   I2S_TX_SLAVE_MODE_MASK,
			                   I2S_TX_SLAVE_MODE);
			regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
			                   I2S_RX_SLAVE_MODE_MASK,
			                   I2S_RX_SLAVE_MODE);
			break;
		case SND_SOC_DAIFMT_CBM_CFM:
			break;
		default:
			return -EINVAL;
	}
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) 
	{
		case SND_SOC_DAIFMT_DSP_A:
		regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
				   I2S_TX_TDM_MODE_MASK,
				   I2S_TX_TDM_MODE);
		regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
			           I2S_RX_TDM_MODE_MASK,
			           I2S_RX_TDM_MODE);
			break;
		case SND_SOC_DAIFMT_LEFT_J:
		regmap_update_bits(regmap_i2s, I2S_TX_CFG,
			           I2S_TX_JUSTIFY_MASK,
			           I2S_TX_JUSTIFY_LEFT);
		regmap_update_bits(regmap_i2s, I2S_RX_CFG,
			           I2S_RX_JUSTIFY_MASK,
			           I2S_RX_JUSTIFY_LEFT);
			break;
		case SND_SOC_DAIFMT_RIGHT_J:
		regmap_update_bits(regmap_i2s, I2S_TX_CFG,
			           I2S_TX_JUSTIFY_MASK,
			           I2S_TX_JUSTIFY_RIGHT);
		regmap_update_bits(regmap_i2s, I2S_RX_CFG,
			           I2S_RX_JUSTIFY_MASK,
			           I2S_RX_JUSTIFY_RIGHT);
		default:
			return -EINVAL;
	}
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) 
	{
		case SND_SOC_DAIFMT_NB_NF:
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int bcm63xx_i2s_set_tdm_slot(struct snd_soc_dai *cpu_dai,
unsigned int tx_mask, unsigned int rx_mask, int channels, int slot_width)
{
	unsigned int slots;
	unsigned int slotwidth;
	struct bcm_i2s_priv *i2s_priv = snd_soc_dai_get_drvdata(cpu_dai);
	struct regmap *regmap_i2s = i2s_priv->regmap_i2s;

	if (slot_width == 16)
		slotwidth = 1;
	else if (slot_width == 32)
		slotwidth = 0;
	else {
		dev_err(i2s_priv->dev,
		"Unsupported slot width(%d)\n", slot_width);
		return -EINVAL;
	}

	slots = channels/2%16;
	if (i2s_priv->dir == SNDRV_PCM_STREAM_PLAYBACK){
		regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,tx_mask,
	                           slots << I2S_TX_VALID_SLOT_SHIFT |
	                           slotwidth << I2S_BIT_PER_SLOT_SHIFT);
		i2s_priv->tx_chan = channels;
	} else {
		regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,rx_mask,
	                           slots << I2S_RX_VALID_SLOT_SHIFT |
	                           slotwidth << I2S_BIT_PER_SLOT_SHIFT);
		i2s_priv->rx_chan = channels;
	}

	return 0;
}

static int bcm63xx_i2s_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	int ret = 0;
	unsigned int channels,slot,sample_depth,mode;
	struct clk *i2s_clk;
	struct bcm_i2s_priv *i2s_priv = snd_soc_dai_get_drvdata(dai);
	struct regmap *regmap_i2s = i2s_priv->regmap_i2s;

	channels = max( i2s_priv->tx_chan, i2s_priv->rx_chan );

	regmap_update_bits(i2s_priv->regmap_i2s, I2S_TX_CFG,
			   I2S_TX_SCLK_1FS_DIV32_MASK,
			   channels << I2S_TX_SCLK_1FS_DIV32_SHIFT );
	regmap_update_bits(i2s_priv->regmap_i2s, I2S_RX_CFG,
			   I2S_RX_SCLK_1FS_DIV32_MASK,
			   channels << I2S_RX_SCLK_1FS_DIV32_SHIFT );

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		i2s_clk = i2s_priv->i2s_tx_clk;
		regmap_read(i2s_priv->regmap_i2s, I2S_TX_CFG, &sample_depth);
		sample_depth = (sample_depth & I2S_BITS_PER_SAMPLE_MASK) >>
				I2S_BITS_PER_SAMPLE_SHIFT;
		sample_depth = sample_depth ? sample_depth : 32;

		ret = clk_set_rate(i2s_clk, params_rate(params) * channels * 
			           sample_depth * 2);
		if (ret < 0) {
			dev_err(i2s_priv->dev,
				"Can't set tx clock rate, err: %d\n", ret);
			return ret;
		}

		regmap_read(regmap_i2s, I2S_TX_CFG_2, &mode);
		dev_info(i2s_priv->dev,
			 "Playback is working on %s mode.\n",
			 mode&I2S_TX_SLAVE_MODE_MASK? "slave":"master");

	} else {
		i2s_clk = i2s_priv->i2s_rx_clk;
		regmap_read(i2s_priv->regmap_i2s, I2S_RX_CFG, &sample_depth);
		sample_depth = (sample_depth & I2S_BITS_PER_SAMPLE_MASK) >> 
				I2S_BITS_PER_SAMPLE_SHIFT;
		sample_depth = sample_depth ? sample_depth : 32;

		ret = clk_set_rate(i2s_clk, params_rate(params) * channels * 
			           sample_depth * 2);
		if (ret < 0) {
			dev_err(i2s_priv->dev,
				"Can't set rx clock rate, err: %d\n", ret);
			return ret;
		}

		regmap_read(regmap_i2s, I2S_RX_CFG_2, &mode);
		dev_info(i2s_priv->dev,
			 "Capture stream is working on %s mode.\n",
			 mode&I2S_RX_SLAVE_MODE_MASK? "slave":"master");
	}

	return ret;
}

static int bcm63xx_i2s_startup(struct snd_pcm_substream *substream,
			       struct snd_soc_dai *dai)
{
	unsigned int slavemode,mode;
	struct bcm_i2s_priv *i2s_priv = snd_soc_dai_get_drvdata(dai);
	struct regmap *regmap_i2s = i2s_priv->regmap_i2s;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		regmap_update_bits(regmap_i2s, I2S_TX_CFG,
			   I2S_TX_DATA_ALIGNMENT,
			   I2S_TX_DATA_ALIGNMENT);

		regmap_update_bits(regmap_i2s, I2S_TX_CFG,
				   I2S_TX_DATA_ENABLE | I2S_TX_CLOCK_ENABLE,
				   I2S_TX_DATA_ENABLE | I2S_TX_CLOCK_ENABLE);

		regmap_read(regmap_i2s, I2S_RX_CFG_2, &slavemode);
		if (slavemode & I2S_RX_SLAVE_MODE_MASK)
			regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
				           I2S_TX_SLAVE_MODE_MASK,
				           I2S_TX_MASTER_MODE);
		else
			regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
				           I2S_TX_SLAVE_MODE_MASK,
				           I2S_TX_SLAVE_MODE);

		i2s_priv->dir = SNDRV_PCM_STREAM_PLAYBACK;

		regmap_write(regmap_i2s, I2S_TX_IRQ_CTL, 0);
		regmap_write(regmap_i2s, I2S_TX_IRQ_IFF_THLD, 0);
		regmap_write(regmap_i2s, I2S_TX_IRQ_OFF_THLD, 1);

		regmap_read(regmap_i2s, I2S_TX_CFG_2, &slavemode);
		dev_info(i2s_priv->dev,
			 "Playback is set to %s mode.\n",
			 slavemode&I2S_TX_SLAVE_MODE_MASK? "slave":"master");
	} else {
		regmap_update_bits(regmap_i2s, I2S_RX_CFG,
			   I2S_RX_DATA_ALIGNMENT,
			   I2S_RX_DATA_ALIGNMENT);

		regmap_update_bits(regmap_i2s, I2S_RX_CFG,
				   I2S_RX_CLOCK_ENABLE, I2S_RX_CLOCK_ENABLE);

		regmap_read(regmap_i2s, I2S_TX_CFG_2, &slavemode);
		if (slavemode & I2S_TX_SLAVE_MODE_MASK)
			regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
				           I2S_RX_SLAVE_MODE_MASK,
				           I2S_RX_MASTER_MODE);
		else
			regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
				           I2S_RX_SLAVE_MODE_MASK,
				           I2S_RX_SLAVE_MODE);

		i2s_priv->dir = SNDRV_PCM_STREAM_CAPTURE;

		regmap_write(regmap_i2s, I2S_RX_IRQ_CTL, 0);
		regmap_write(regmap_i2s, I2S_RX_IRQ_IFF_THLD, 0);
		regmap_write(regmap_i2s, I2S_RX_IRQ_OFF_THLD, 1);

		regmap_read(regmap_i2s, I2S_RX_CFG_2, &slavemode);
		dev_info(i2s_priv->dev,
			 "Capture stream is set to %s mode.\n",
			 slavemode&I2S_RX_SLAVE_MODE_MASK? "slave":"master");
	}
	return 0;
}

static void bcm63xx_i2s_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	unsigned int i,level,val;
	unsigned int enabled, slavemode;
	struct bcm_i2s_priv *i2s_priv = snd_soc_dai_get_drvdata(dai);
	struct regmap *regmap_i2s = i2s_priv->regmap_i2s;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		regmap_update_bits(regmap_i2s, I2S_TX_CFG,
				I2S_TX_OUT_R | I2S_TX_DATA_ALIGNMENT |
				I2S_TX_DATA_ENABLE | I2S_TX_CLOCK_ENABLE, 0);
		regmap_write(regmap_i2s, I2S_TX_IRQ_CTL, 1);
		regmap_write(regmap_i2s, I2S_TX_IRQ_IFF_THLD, 4);
		regmap_write(regmap_i2s, I2S_TX_IRQ_OFF_THLD, 4);

		i2s_priv->tx_chan = 0;

		regmap_read(regmap_i2s, I2S_TX_CFG_2, &slavemode);
		slavemode = slavemode & I2S_TX_SLAVE_MODE_MASK;
		if (!slavemode) {
			regmap_read(regmap_i2s, I2S_RX_CFG, &enabled);
			enabled = enabled & I2S_RX_ENABLE_MASK;
			if (enabled) {
				regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
							I2S_TX_SLAVE_MODE_MASK,
							I2S_TX_SLAVE_MODE);
				regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
						   I2S_RX_SLAVE_MODE_MASK,
						   I2S_RX_MASTER_MODE);
			}
		}
		regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
					I2S_TX_SLAVE_MODE_MASK,
					I2S_TX_SLAVE_MODE);
		regmap_read(regmap_i2s, I2S_TX_CFG_2, &slavemode);
		regmap_read(regmap_i2s, I2S_RX_CFG_2, &slavemode);
	
	} else {
		i = 0;
		/* loop until IFF empty */
		do{ 
			regmap_read(regmap_i2s, I2S_RX_IRQ_CTL, &level);
			level = level & I2S_RX_DESC_IFF_LEVEL_MASK;
			msleep(1);
			if (i++ > 1000)
				break;
		}while( level != 0);

		/* clean up OFF */
		regmap_read(regmap_i2s, I2S_RX_IRQ_CTL, &level);
		level = (level & I2S_RX_DESC_OFF_LEVEL_MASK) >>
			   I2S_RX_DESC_OFF_LEVEL_SHIFT;
		for (i=0; i<level; i++) {
			   regmap_read(regmap_i2s, I2S_RX_DESC_OFF_ADDR, &val);
			   regmap_read(regmap_i2s, I2S_RX_DESC_OFF_LEN, &val);
		}

		regmap_update_bits(regmap_i2s,
			   I2S_RX_IRQ_EN,
			   I2S_RX_DESC_OFF_INTR_EN_MSK,
			   0);
		regmap_update_bits(regmap_i2s,
			   I2S_RX_CFG,
			   I2S_RX_ENABLE_MASK,
			   0);

		regmap_update_bits(regmap_i2s, I2S_RX_CFG,
				   I2S_RX_CLOCK_ENABLE, 0);
		regmap_write(regmap_i2s, I2S_RX_IRQ_CTL, 1);
		regmap_write(regmap_i2s, I2S_RX_IRQ_IFF_THLD, 4);
		regmap_write(regmap_i2s, I2S_RX_IRQ_OFF_THLD, 4);

		i2s_priv->rx_chan = 0;

		regmap_read(regmap_i2s, I2S_RX_CFG_2, &slavemode);
		slavemode = slavemode & I2S_RX_SLAVE_MODE_MASK;
		if (!slavemode) {
			regmap_read(regmap_i2s, I2S_TX_CFG, &enabled);
			enabled = enabled & I2S_TX_ENABLE_MASK;
			if (enabled) {
				regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
						I2S_RX_SLAVE_MODE_MASK,
						I2S_RX_SLAVE_MODE);
				regmap_update_bits(regmap_i2s, I2S_TX_CFG_2,
						   I2S_TX_SLAVE_MODE_MASK,
						   I2S_TX_MASTER_MODE);
			}
		}
		regmap_update_bits(regmap_i2s, I2S_RX_CFG_2,
				   I2S_RX_SLAVE_MODE_MASK, I2S_RX_SLAVE_MODE);
		regmap_read(regmap_i2s, I2S_TX_CFG_2, &slavemode);
		regmap_read(regmap_i2s, I2S_RX_CFG_2, &slavemode);
	}
}

static const struct snd_soc_dai_ops bcm63xx_i2s_dai_ops = {
	.startup = bcm63xx_i2s_startup,
	.shutdown = bcm63xx_i2s_shutdown,
	.hw_params = bcm63xx_i2s_hw_params,
	.set_fmt = bcm63xx_i2s_set_fmt,
	.set_tdm_slot = bcm63xx_i2s_set_tdm_slot,
};

static struct snd_soc_dai_driver bcm63xx_i2s_dai = {
	.name = "merritt-cpu-dai",
	.playback = {
		.stream_name="playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.stream_name="capture",
		.channels_min = 2,
		.channels_max = 32,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &bcm63xx_i2s_dai_ops,
};

static const struct snd_soc_component_driver bcm63xx_i2s_component = {
	.name = "bcm63xx",
};

static int bcm63xx_i2s_dev_probe(struct platform_device *pdev)
{
	int ret;
	void __iomem *regs;
	struct resource *r_mem;
	struct bcm_i2s_priv *i2s_priv;
	struct regmap *regmap_i2s;
	struct clk *i2s_tx_clk,*i2s_rx_clk;

	i2s_priv = devm_kzalloc(&pdev->dev, sizeof(*i2s_priv), GFP_KERNEL);
	if (!i2s_priv)
		return -ENOMEM;
	memset(i2s_priv, 0, sizeof(*i2s_priv));

	i2s_tx_clk = devm_clk_get(&pdev->dev, "i2stxclk");
	if (IS_ERR(i2s_tx_clk)) {
		dev_err(&pdev->dev, "%s: cannot get a brcm clock: %ld\n",
					__func__, PTR_ERR(i2s_tx_clk));
		return PTR_ERR(i2s_tx_clk);
	}

	i2s_rx_clk = i2s_tx_clk;

	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		dev_err(&pdev->dev, "Unable to get register resource.\n");
		return -ENODEV;
	}

	regs = devm_ioremap_resource(&pdev->dev, r_mem);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	regmap_i2s = devm_regmap_init_mmio(&pdev->dev,
					regs, &brcm_i2s_regmap_config);
	if (IS_ERR(regmap_i2s))
		return PTR_ERR(regmap_i2s);

	ret = devm_snd_soc_register_component(&pdev->dev,
					      &bcm63xx_i2s_component,
					      &bcm63xx_i2s_dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register the dai.\n");
		return ret;
	}

	i2s_priv->dev = &pdev->dev;
	i2s_priv->i2s_tx_clk = i2s_tx_clk;
	i2s_priv->i2s_rx_clk = i2s_rx_clk;
	i2s_priv->regmap_i2s = regmap_i2s;

	i2s_priv->mic_sel_1_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_sel_1", 0);
	if (IS_ERR(i2s_priv->mic_sel_1_gpio)) {
		dev_err(&pdev->dev, "mic_sel_1 GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_sel_1_gpio));
		return PTR_ERR(i2s_priv->mic_sel_1_gpio);
	}

	i2s_priv->mic_sel_2_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_sel_2", 0);
	if (IS_ERR(i2s_priv->mic_sel_2_gpio)) {
		dev_err(&pdev->dev, "mic_sel_2 GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_sel_2_gpio));
		return PTR_ERR(i2s_priv->mic_sel_2_gpio);
	}

	i2s_priv->mic_array_sw1_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_array_sw1", 0);
	if (IS_ERR(i2s_priv->mic_array_sw1_gpio)) {
		dev_err(&pdev->dev, "mic_array_sw1 GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_array_sw1_gpio));
		return PTR_ERR(i2s_priv->mic_array_sw1_gpio);
	}

	i2s_priv->mic_array_sw2_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_array_sw2", 0);
	if (IS_ERR(i2s_priv->mic_array_sw2_gpio)) {
		dev_err(&pdev->dev, "mic_array_sw2 GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_array_sw2_gpio));
		return PTR_ERR(i2s_priv->mic_array_sw2_gpio);
	}

	i2s_priv->mic_array_sw3_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_array_sw3", 0);
	if (IS_ERR(i2s_priv->mic_array_sw3_gpio)) {
		dev_err(&pdev->dev, "mic_array_sw3 GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_array_sw3_gpio));
		return PTR_ERR(i2s_priv->mic_array_sw3_gpio);
	}

	i2s_priv->mic_power_en_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_power_en", 0);
	if (IS_ERR(i2s_priv->mic_power_en_gpio)) {
		dev_err(&pdev->dev, "mic_power_en GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_power_en_gpio));
		return PTR_ERR(i2s_priv->mic_power_en_gpio);
	}

	i2s_priv->mic_mute_detect_gpio = devm_gpiod_get_optional(&pdev->dev, "mic_mute_detect", GPIOD_IN);
	if (IS_ERR(i2s_priv->mic_mute_detect_gpio)) {
		dev_err(&pdev->dev, "mic_mute_detect GPIO request failed: %ld\n", PTR_ERR(i2s_priv->mic_mute_detect_gpio));
		return PTR_ERR(i2s_priv->mic_mute_detect_gpio);
	}

	i2s_priv->i2s_clk_select = devm_gpiod_get_optional(&pdev->dev, "i2s_clk_select", 0);
	if (IS_ERR(i2s_priv->i2s_clk_select)) {
		dev_err(&pdev->dev, "mic_mute_detect GPIO request failed: %ld\n", PTR_ERR(i2s_priv->i2s_clk_select));
		return PTR_ERR(i2s_priv->i2s_clk_select);
	}
	gpiod_direction_output(i2s_priv->i2s_clk_select, 0);/* select TX RX share one clk */

	/* pwr on mics when probed, green led on*/
	gpiod_direction_output(i2s_priv->mic_power_en_gpio, 0);

	/* select mic 1/2/5/6 located 0/180/90/270 degrees */
	gpiod_direction_output(i2s_priv->mic_sel_1_gpio, 1);
	gpiod_direction_output(i2s_priv->mic_sel_2_gpio, 1);
	gpiod_direction_output(i2s_priv->mic_array_sw1_gpio, 1);
	gpiod_direction_output(i2s_priv->mic_array_sw2_gpio, 1);

#if 0
	/* select mic 3/4/7/8 located 60/240/300/120 degrees */
	gpiod_direction_output(i2s_priv->mic_sel_1_gpio, 0);
	gpiod_direction_output(i2s_priv->mic_sel_2_gpio, 1);
	gpiod_direction_output(i2s_priv->mic_array_sw2_gpio, 1);

	/* select mic 11/12/15/16 located inner circle 0/240/center/120 degree */
	gpiod_direction_output(i2s_priv->mic_sel_1_gpio, 1);
	gpiod_direction_output(i2s_priv->mic_sel_2_gpio, 0);
	gpiod_direction_output(i2s_priv->mic_array_sw1_gpio, 0);
	gpiod_direction_output(i2s_priv->mic_array_sw3_gpio, 0);

	/* select mic 9/10/13/14 located 315/135/225/45 degree */
	gpiod_direction_output(i2s_priv->mic_sel_1_gpio, 0);
	gpiod_direction_output(i2s_priv->mic_sel_2_gpio, 0);
	gpiod_direction_output(i2s_priv->mic_array_sw2_gpio, 1);
	gpiod_direction_output(i2s_priv->mic_array_sw3_gpio, 1);
#endif

	dev_set_drvdata(&pdev->dev, i2s_priv);

	ret = bcm63xx_soc_platform_probe(pdev, i2s_priv);
	if (ret) {
		dev_err(&pdev->dev, "failed to register the pcm\n");
		return ret;
	}

	micMuteCtlTask = kthread_run(kthreadfunc_micmute, (void *)i2s_priv, "MIC_Mute_Control");
	if (IS_ERR(micMuteCtlTask)) {
		ret = PTR_ERR(micMuteCtlTask);
		micMuteCtlTask = NULL;
		dev_err(&pdev->dev, "Create Mic mute control thread failed.\n");
		return ret;
	}

	dev_notice(&pdev->dev, "%s successfully probed!\n",  DRV_NAME);

	return 0;
}

static int bcm63xx_i2s_dev_remove(struct platform_device *pdev)
{
	if(micMuteCtlTask){
		kthread_stop(micMuteCtlTask);
		micMuteCtlTask = NULL;
	}
	bcm63xx_soc_platform_remove(pdev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = {
	{.compatible = "brcm,bcm63xx-i2s"},
	{ }
};
#endif

static struct platform_driver bcm63xx_i2s_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
	},
	.probe = bcm63xx_i2s_dev_probe,
	.remove = bcm63xx_i2s_dev_remove,
};

module_platform_driver(bcm63xx_i2s_driver);

MODULE_AUTHOR("Kevin,Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("Broadcom DSL XPON ASOC I2S/TDN Interface");
MODULE_LICENSE("GPL v2");
