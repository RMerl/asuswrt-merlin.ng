/*********************************************************************
 * bcm63xx-i2s.c  --  ALSA SoC Audio Layer - Broadcom I2S Controller driver
 *
 * Author: Kevin Li <kevin-ke.li@broadcom.com>
 * 
 * Copyright (c) 2018 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
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
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "bcm63xx-squamish.h"

#define DRV_NAME "brcm-pcm-audio"
#define BCM63XX_I2S_RATES  \
    SNDRV_PCM_RATE_8000  | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_22050 |\
    SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
    SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000 

struct regmap		*regmap_pcm;
static struct clk	*i2s_clk;

int bcm63xx_soc_platform_probe(struct platform_device *pdev);
int bcm63xx_soc_platform_remove(struct platform_device *pdev);

static bool brcm_pcm_wr_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
		case PCM_CTRL ... PCM_REG_MAX:
			return true;
		default:
			return false;
	}
}
static bool brcm_pcm_rd_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
		case PCM_CTRL ... PCM_REG_MAX:
			return true;
		default:
			return false;
	}
}

static bool brcm_pcm_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
		case PCM_CTRL ... PCM_REG_MAX:
			return true;
		default:
			return false;
	}
}

static const struct regmap_config brcm_pcm_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.max_register = PCM_REG_MAX,
	.writeable_reg= brcm_pcm_wr_reg,
	.readable_reg = brcm_pcm_rd_reg,
	.volatile_reg = brcm_pcm_volatile_reg,
// .precious_reg = brcm_i2s_precious_reg,
	.cache_type   = REGCACHE_FLAT,
};

static int bcm63xx_pcm_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
		case SND_SOC_DAIFMT_CBS_CFS:    // todo:
			break;
		default:
			return -EINVAL;
	}
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			regmap_update_bits(regmap_pcm, PCM_CTRL, 
				PCM_CLK_INV | PCM_FS_INV,
				0 );
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int bcm63xx_pcm_hw_params(struct snd_pcm_substream *substream,
			struct snd_pcm_hw_params *params,
			struct snd_soc_dai *dai)
{
	switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S32_LE:
			clk_set_rate(i2s_clk, params_rate(params));
			break;
		default:
			dev_err(dai->dev, "Format unsupported\n");
		return -EINVAL;
	}
	return 0;
}

static int bcm63xx_pcm_set_sysclk(struct snd_soc_dai *cpu_dai,
				int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int bcm63xx_pcm_probe(struct snd_soc_dai *dai)
{
	regmap_write(regmap_pcm, DMA_CTRL, 0);
	regmap_write(regmap_pcm, DMA_CTRL_GLB_IRQMASK, 0);
	regmap_write(regmap_pcm, DMA_RX_CH_CFG, 0);
	regmap_write(regmap_pcm, DMA_TX_CH_CFG, 0);
	regmap_write(regmap_pcm, DMA_RX_IRQ_MASK, 0);
	regmap_write(regmap_pcm, DMA_TX_IRQ_MASK, 0);
	regmap_write(regmap_pcm, DMA_RX_BURST, 1);
	regmap_write(regmap_pcm, DMA_TX_BURST, 1);
	regmap_write(regmap_pcm, PCM_TS_ALLOC1, 0);
	regmap_write(regmap_pcm, PCM_TS_ALLOC2, 0);
	regmap_write(regmap_pcm, PCM_TS_ALLOC3, 0);
	regmap_update_bits(regmap_pcm,PCM_TS_ALLOC0,
		PCM_TS_MASK(0) | PCM_TS_MASK(1),
		PCM_TS_EN(0) | PCM_TS_EN(1));

	return 0;
}
static int bcm63xx_pcm_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	if (!cpu_dai->active) {
		// todo
	}

	regmap_update_bits(regmap_pcm, PCM_CTRL, 
					PCM_SLAVE_MODE |
					PCM_CLK_INV |
					PCM_FS_INV|
					PCM_FS_LONG |
					/*PCM_FS_TRIG |*/
					PCM_DATA_OFF |
					PCM_SAMPLE_SZ_MASK |
					PCM_LSB_FIRST |
					PCM_LOOPBACK |
					PCM_CLK_DIV_MASK |
					PCM_FRAME_SIZE_MASK,
					PCM_FS_LONG |
					/*PCM_FS_TRIG |*/
					PCM_DATA_OFF |
					PCM_SAMPLE_SZ_32 |
					PCM_CLK_DIV_2 |
					PCM_FRAME_SIZE_2);

	return 0;
}

static void bcm63xx_pcm_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	return ;
}

static const struct snd_soc_dai_ops bcm63xx_pcm_dai_ops = {
	.startup = bcm63xx_pcm_startup,
	.shutdown = bcm63xx_pcm_shutdown,
	.hw_params = bcm63xx_pcm_hw_params,
	.set_fmt = bcm63xx_pcm_set_fmt,
	.set_sysclk = bcm63xx_pcm_set_sysclk,
};

static struct snd_soc_dai_driver bcm63xx_pcm_dai = {
	.name = "squamish-cpu-dai",
	.probe = bcm63xx_pcm_probe,
	.playback =  {
		.channels_min = 2,
		.channels_max = 2,
		.rates = BCM63XX_I2S_RATES,
		.formats      = SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = BCM63XX_I2S_RATES,
		.formats = SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops = &bcm63xx_pcm_dai_ops,
};

static const struct snd_soc_component_driver bcm63xx_pcm_component = {
	.name = "bcm63xx",
};

static int bcm63xx_pcm_dev_probe(struct platform_device *pdev)
{
	int ret = 0;
	void __iomem *regs;
	struct resource *r_mem, *region;

	i2s_clk = devm_clk_get(&pdev->dev, "pcmclk");
	if (IS_ERR(i2s_clk)) {
		dev_err(&pdev->dev, "%s: cannot get a brcm clock: %ld\n",
					__func__,PTR_ERR(i2s_clk));
		return PTR_ERR(i2s_clk);
	}

	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		dev_err(&pdev->dev, "Unable to get register resource.\n");
		return -ENODEV;
	}

	region = devm_request_mem_region(&pdev->dev, r_mem->start,
				resource_size(r_mem), DRV_NAME);
	if (!region) {
		dev_err(&pdev->dev, "Memory region already claimed\n");
		return -EBUSY;
	}

	regs = devm_ioremap_nocache(&pdev->dev,r_mem->start,
					resource_size(r_mem));
	regmap_pcm = devm_regmap_init_mmio(&pdev->dev,regs,
					&brcm_pcm_regmap_config);
	if (!regmap_pcm) {
		dev_err(&pdev->dev,
			"Failed to initialise managed register map\n");
	}

	ret = devm_snd_soc_register_component(&pdev->dev,
					&bcm63xx_pcm_component,
					&bcm63xx_pcm_dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "failed to register the dai\n");
		return ret;
	}

	ret = bcm63xx_soc_platform_probe(pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to register the pcm\n");
	}

	return ret;
}

static int bcm63xx_pcm_dev_remove(struct platform_device *pdev)
{
	bcm63xx_soc_platform_remove(pdev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id snd_soc_bcm_audio_match[] = 
{
	{ .compatible = "brcm,bcm63xx-pcm" },
	{ }
};
#endif

static struct platform_driver bcm63xx_pcm_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(snd_soc_bcm_audio_match),
	},
	.probe  = bcm63xx_pcm_dev_probe,
	.remove = bcm63xx_pcm_dev_remove,
};

module_platform_driver(bcm63xx_pcm_driver);

/* Module information */
MODULE_AUTHOR("Kevin,Li <kevin-ke.li@broadcom.com>");
MODULE_DESCRIPTION("Broadcom DSL XPON ASOC PCM Interface");
MODULE_LICENSE("GPL");
