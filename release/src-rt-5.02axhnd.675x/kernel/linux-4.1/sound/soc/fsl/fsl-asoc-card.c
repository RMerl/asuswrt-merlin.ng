/*
 * Freescale Generic ASoC Sound Card driver with ASRC
 *
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * Author: Nicolin Chen <nicoleotsuka@gmail.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "fsl_esai.h"
#include "fsl_sai.h"
#include "imx-audmux.h"

#include "../codecs/sgtl5000.h"
#include "../codecs/wm8962.h"

#define RX 0
#define TX 1

/* Default DAI format without Master and Slave flag */
#define DAI_FMT_BASE (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF)

/**
 * CODEC private data
 *
 * @mclk_freq: Clock rate of MCLK
 * @mclk_id: MCLK (or main clock) id for set_sysclk()
 * @fll_id: FLL (or secordary clock) id for set_sysclk()
 * @pll_id: PLL id for set_pll()
 */
struct codec_priv {
	unsigned long mclk_freq;
	u32 mclk_id;
	u32 fll_id;
	u32 pll_id;
};

/**
 * CPU private data
 *
 * @sysclk_freq[2]: SYSCLK rates for set_sysclk()
 * @sysclk_dir[2]: SYSCLK directions for set_sysclk()
 * @sysclk_id[2]: SYSCLK ids for set_sysclk()
 * @slot_width: Slot width of each frame
 *
 * Note: [1] for tx and [0] for rx
 */
struct cpu_priv {
	unsigned long sysclk_freq[2];
	u32 sysclk_dir[2];
	u32 sysclk_id[2];
	u32 slot_width;
};

/**
 * Freescale Generic ASOC card private data
 *
 * @dai_link[3]: DAI link structure including normal one and DPCM link
 * @pdev: platform device pointer
 * @codec_priv: CODEC private data
 * @cpu_priv: CPU private data
 * @card: ASoC card structure
 * @sample_rate: Current sample rate
 * @sample_format: Current sample format
 * @asrc_rate: ASRC sample rate used by Back-Ends
 * @asrc_format: ASRC sample format used by Back-Ends
 * @dai_fmt: DAI format between CPU and CODEC
 * @name: Card name
 */

struct fsl_asoc_card_priv {
	struct snd_soc_dai_link dai_link[3];
	struct platform_device *pdev;
	struct codec_priv codec_priv;
	struct cpu_priv cpu_priv;
	struct snd_soc_card card;
	u32 sample_rate;
	u32 sample_format;
	u32 asrc_rate;
	u32 asrc_format;
	u32 dai_fmt;
	char name[32];
};

/**
 * This dapm route map exsits for DPCM link only.
 * The other routes shall go through Device Tree.
 */
static const struct snd_soc_dapm_route audio_map[] = {
	{"CPU-Playback",  NULL, "ASRC-Playback"},
	{"Playback",  NULL, "CPU-Playback"},
	{"ASRC-Capture",  NULL, "CPU-Capture"},
	{"CPU-Capture",  NULL, "Capture"},
};

/* Add all possible widgets into here without being redundant */
static const struct snd_soc_dapm_widget fsl_asoc_card_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("Line Out Jack", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_MIC("AMIC", NULL),
	SND_SOC_DAPM_MIC("DMIC", NULL),
};

static int fsl_asoc_card_hw_params(struct snd_pcm_substream *substream,
				   struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct fsl_asoc_card_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	bool tx = substream->stream == SNDRV_PCM_STREAM_PLAYBACK;
	struct cpu_priv *cpu_priv = &priv->cpu_priv;
	struct device *dev = rtd->card->dev;
	int ret;

	priv->sample_rate = params_rate(params);
	priv->sample_format = params_format(params);

	/*
	 * If codec-dai is DAI Master and all configurations are already in the
	 * set_bias_level(), bypass the remaining settings in hw_params().
	 * Note: (dai_fmt & CBM_CFM) includes CBM_CFM and CBM_CFS.
	 */
	if (priv->card.set_bias_level && priv->dai_fmt & SND_SOC_DAIFMT_CBM_CFM)
		return 0;

	/* Specific configurations of DAIs starts from here */
	ret = snd_soc_dai_set_sysclk(rtd->cpu_dai, cpu_priv->sysclk_id[tx],
				     cpu_priv->sysclk_freq[tx],
				     cpu_priv->sysclk_dir[tx]);
	if (ret) {
		dev_err(dev, "failed to set sysclk for cpu dai\n");
		return ret;
	}

	if (cpu_priv->slot_width) {
		ret = snd_soc_dai_set_tdm_slot(rtd->cpu_dai, 0x3, 0x3, 2,
					       cpu_priv->slot_width);
		if (ret) {
			dev_err(dev, "failed to set TDM slot for cpu dai\n");
			return ret;
		}
	}

	return 0;
}

static struct snd_soc_ops fsl_asoc_card_ops = {
	.hw_params = fsl_asoc_card_hw_params,
};

static int be_hw_params_fixup(struct snd_soc_pcm_runtime *rtd,
			      struct snd_pcm_hw_params *params)
{
	struct fsl_asoc_card_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	struct snd_interval *rate;
	struct snd_mask *mask;

	rate = hw_param_interval(params, SNDRV_PCM_HW_PARAM_RATE);
	rate->max = rate->min = priv->asrc_rate;

	mask = hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);
	snd_mask_none(mask);
	snd_mask_set(mask, priv->asrc_format);

	return 0;
}

static struct snd_soc_dai_link fsl_asoc_card_dai[] = {
	/* Default ASoC DAI Link*/
	{
		.name = "HiFi",
		.stream_name = "HiFi",
		.ops = &fsl_asoc_card_ops,
	},
	/* DPCM Link between Front-End and Back-End (Optional) */
	{
		.name = "HiFi-ASRC-FE",
		.stream_name = "HiFi-ASRC-FE",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.dynamic = 1,
	},
	{
		.name = "HiFi-ASRC-BE",
		.stream_name = "HiFi-ASRC-BE",
		.platform_name = "snd-soc-dummy",
		.be_hw_params_fixup = be_hw_params_fixup,
		.ops = &fsl_asoc_card_ops,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.no_pcm = 1,
	},
};

static int fsl_asoc_card_set_bias_level(struct snd_soc_card *card,
					struct snd_soc_dapm_context *dapm,
					enum snd_soc_bias_level level)
{
	struct fsl_asoc_card_priv *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct codec_priv *codec_priv = &priv->codec_priv;
	struct device *dev = card->dev;
	unsigned int pll_out;
	int ret;

	if (dapm->dev != codec_dai->dev)
		return 0;

	switch (level) {
	case SND_SOC_BIAS_PREPARE:
		if (dapm->bias_level != SND_SOC_BIAS_STANDBY)
			break;

		if (priv->sample_format == SNDRV_PCM_FORMAT_S24_LE)
			pll_out = priv->sample_rate * 384;
		else
			pll_out = priv->sample_rate * 256;

		ret = snd_soc_dai_set_pll(codec_dai, codec_priv->pll_id,
					  codec_priv->mclk_id,
					  codec_priv->mclk_freq, pll_out);
		if (ret) {
			dev_err(dev, "failed to start FLL: %d\n", ret);
			return ret;
		}

		ret = snd_soc_dai_set_sysclk(codec_dai, codec_priv->fll_id,
					     pll_out, SND_SOC_CLOCK_IN);
		if (ret) {
			dev_err(dev, "failed to set SYSCLK: %d\n", ret);
			return ret;
		}
		break;

	case SND_SOC_BIAS_STANDBY:
		if (dapm->bias_level != SND_SOC_BIAS_PREPARE)
			break;

		ret = snd_soc_dai_set_sysclk(codec_dai, codec_priv->mclk_id,
					     codec_priv->mclk_freq,
					     SND_SOC_CLOCK_IN);
		if (ret) {
			dev_err(dev, "failed to switch away from FLL: %d\n", ret);
			return ret;
		}

		ret = snd_soc_dai_set_pll(codec_dai, codec_priv->pll_id, 0, 0, 0);
		if (ret) {
			dev_err(dev, "failed to stop FLL: %d\n", ret);
			return ret;
		}
		break;

	default:
		break;
	}

	return 0;
}

static int fsl_asoc_card_audmux_init(struct device_node *np,
				     struct fsl_asoc_card_priv *priv)
{
	struct device *dev = &priv->pdev->dev;
	u32 int_ptcr = 0, ext_ptcr = 0;
	int int_port, ext_port;
	int ret;

	ret = of_property_read_u32(np, "mux-int-port", &int_port);
	if (ret) {
		dev_err(dev, "mux-int-port missing or invalid\n");
		return ret;
	}
	ret = of_property_read_u32(np, "mux-ext-port", &ext_port);
	if (ret) {
		dev_err(dev, "mux-ext-port missing or invalid\n");
		return ret;
	}

	/*
	 * The port numbering in the hardware manual starts at 1, while
	 * the AUDMUX API expects it starts at 0.
	 */
	int_port--;
	ext_port--;

	/*
	 * Use asynchronous mode (6 wires) for all cases.
	 * If only 4 wires are needed, just set SSI into
	 * synchronous mode and enable 4 PADs in IOMUX.
	 */
	switch (priv->dai_fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		int_ptcr = IMX_AUDMUX_V2_PTCR_RFSEL(8 | ext_port) |
			   IMX_AUDMUX_V2_PTCR_RCSEL(8 | ext_port) |
			   IMX_AUDMUX_V2_PTCR_TFSEL(ext_port) |
			   IMX_AUDMUX_V2_PTCR_TCSEL(ext_port) |
			   IMX_AUDMUX_V2_PTCR_RFSDIR |
			   IMX_AUDMUX_V2_PTCR_RCLKDIR |
			   IMX_AUDMUX_V2_PTCR_TFSDIR |
			   IMX_AUDMUX_V2_PTCR_TCLKDIR;
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
		int_ptcr = IMX_AUDMUX_V2_PTCR_RCSEL(8 | ext_port) |
			   IMX_AUDMUX_V2_PTCR_TCSEL(ext_port) |
			   IMX_AUDMUX_V2_PTCR_RCLKDIR |
			   IMX_AUDMUX_V2_PTCR_TCLKDIR;
		ext_ptcr = IMX_AUDMUX_V2_PTCR_RFSEL(8 | int_port) |
			   IMX_AUDMUX_V2_PTCR_TFSEL(int_port) |
			   IMX_AUDMUX_V2_PTCR_RFSDIR |
			   IMX_AUDMUX_V2_PTCR_TFSDIR;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
		int_ptcr = IMX_AUDMUX_V2_PTCR_RFSEL(8 | ext_port) |
			   IMX_AUDMUX_V2_PTCR_TFSEL(ext_port) |
			   IMX_AUDMUX_V2_PTCR_RFSDIR |
			   IMX_AUDMUX_V2_PTCR_TFSDIR;
		ext_ptcr = IMX_AUDMUX_V2_PTCR_RCSEL(8 | int_port) |
			   IMX_AUDMUX_V2_PTCR_TCSEL(int_port) |
			   IMX_AUDMUX_V2_PTCR_RCLKDIR |
			   IMX_AUDMUX_V2_PTCR_TCLKDIR;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		ext_ptcr = IMX_AUDMUX_V2_PTCR_RFSEL(8 | int_port) |
			   IMX_AUDMUX_V2_PTCR_RCSEL(8 | int_port) |
			   IMX_AUDMUX_V2_PTCR_TFSEL(int_port) |
			   IMX_AUDMUX_V2_PTCR_TCSEL(int_port) |
			   IMX_AUDMUX_V2_PTCR_RFSDIR |
			   IMX_AUDMUX_V2_PTCR_RCLKDIR |
			   IMX_AUDMUX_V2_PTCR_TFSDIR |
			   IMX_AUDMUX_V2_PTCR_TCLKDIR;
		break;
	default:
		return -EINVAL;
	}

	/* Asynchronous mode can not be set along with RCLKDIR */
	ret = imx_audmux_v2_configure_port(int_port, 0,
					   IMX_AUDMUX_V2_PDCR_RXDSEL(ext_port));
	if (ret) {
		dev_err(dev, "audmux internal port setup failed\n");
		return ret;
	}

	ret = imx_audmux_v2_configure_port(int_port, int_ptcr,
					   IMX_AUDMUX_V2_PDCR_RXDSEL(ext_port));
	if (ret) {
		dev_err(dev, "audmux internal port setup failed\n");
		return ret;
	}

	ret = imx_audmux_v2_configure_port(ext_port, 0,
					   IMX_AUDMUX_V2_PDCR_RXDSEL(int_port));
	if (ret) {
		dev_err(dev, "audmux external port setup failed\n");
		return ret;
	}

	ret = imx_audmux_v2_configure_port(ext_port, ext_ptcr,
					   IMX_AUDMUX_V2_PDCR_RXDSEL(int_port));
	if (ret) {
		dev_err(dev, "audmux external port setup failed\n");
		return ret;
	}

	return 0;
}

static int fsl_asoc_card_late_probe(struct snd_soc_card *card)
{
	struct fsl_asoc_card_priv *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct codec_priv *codec_priv = &priv->codec_priv;
	struct device *dev = card->dev;
	int ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, codec_priv->mclk_id,
				     codec_priv->mclk_freq, SND_SOC_CLOCK_IN);
	if (ret) {
		dev_err(dev, "failed to set sysclk in %s\n", __func__);
		return ret;
	}

	return 0;
}

static int fsl_asoc_card_probe(struct platform_device *pdev)
{
	struct device_node *cpu_np, *codec_np, *asrc_np;
	struct device_node *np = pdev->dev.of_node;
	struct platform_device *asrc_pdev = NULL;
	struct platform_device *cpu_pdev;
	struct fsl_asoc_card_priv *priv;
	struct i2c_client *codec_dev;
	struct clk *codec_clk;
	u32 width;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	cpu_np = of_parse_phandle(np, "audio-cpu", 0);
	/* Give a chance to old DT binding */
	if (!cpu_np)
		cpu_np = of_parse_phandle(np, "ssi-controller", 0);
	codec_np = of_parse_phandle(np, "audio-codec", 0);
	if (!cpu_np || !codec_np) {
		dev_err(&pdev->dev, "phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	cpu_pdev = of_find_device_by_node(cpu_np);
	if (!cpu_pdev) {
		dev_err(&pdev->dev, "failed to find CPU DAI device\n");
		ret = -EINVAL;
		goto fail;
	}

	codec_dev = of_find_i2c_device_by_node(codec_np);
	if (!codec_dev) {
		dev_err(&pdev->dev, "failed to find codec platform device\n");
		ret = -EINVAL;
		goto fail;
	}

	asrc_np = of_parse_phandle(np, "audio-asrc", 0);
	if (asrc_np)
		asrc_pdev = of_find_device_by_node(asrc_np);

	/* Get the MCLK rate only, and leave it controlled by CODEC drivers */
	codec_clk = clk_get(&codec_dev->dev, NULL);
	if (!IS_ERR(codec_clk)) {
		priv->codec_priv.mclk_freq = clk_get_rate(codec_clk);
		clk_put(codec_clk);
	}

	/* Default sample rate and format, will be updated in hw_params() */
	priv->sample_rate = 44100;
	priv->sample_format = SNDRV_PCM_FORMAT_S16_LE;

	/* Assign a default DAI format, and allow each card to overwrite it */
	priv->dai_fmt = DAI_FMT_BASE;

	/* Diversify the card configurations */
	if (of_device_is_compatible(np, "fsl,imx-audio-cs42888")) {
		priv->card.set_bias_level = NULL;
		priv->cpu_priv.sysclk_freq[TX] = priv->codec_priv.mclk_freq;
		priv->cpu_priv.sysclk_freq[RX] = priv->codec_priv.mclk_freq;
		priv->cpu_priv.sysclk_dir[TX] = SND_SOC_CLOCK_OUT;
		priv->cpu_priv.sysclk_dir[RX] = SND_SOC_CLOCK_OUT;
		priv->cpu_priv.slot_width = 32;
		priv->dai_fmt |= SND_SOC_DAIFMT_CBS_CFS;
	} else if (of_device_is_compatible(np, "fsl,imx-audio-sgtl5000")) {
		priv->codec_priv.mclk_id = SGTL5000_SYSCLK;
		priv->dai_fmt |= SND_SOC_DAIFMT_CBM_CFM;
	} else if (of_device_is_compatible(np, "fsl,imx-audio-wm8962")) {
		priv->card.set_bias_level = fsl_asoc_card_set_bias_level;
		priv->codec_priv.mclk_id = WM8962_SYSCLK_MCLK;
		priv->codec_priv.fll_id = WM8962_SYSCLK_FLL;
		priv->codec_priv.pll_id = WM8962_FLL;
		priv->dai_fmt |= SND_SOC_DAIFMT_CBM_CFM;
	} else {
		dev_err(&pdev->dev, "unknown Device Tree compatible\n");
		return -EINVAL;
	}

	/* Common settings for corresponding Freescale CPU DAI driver */
	if (strstr(cpu_np->name, "ssi")) {
		/* Only SSI needs to configure AUDMUX */
		ret = fsl_asoc_card_audmux_init(np, priv);
		if (ret) {
			dev_err(&pdev->dev, "failed to init audmux\n");
			goto asrc_fail;
		}
	} else if (strstr(cpu_np->name, "esai")) {
		priv->cpu_priv.sysclk_id[1] = ESAI_HCKT_EXTAL;
		priv->cpu_priv.sysclk_id[0] = ESAI_HCKR_EXTAL;
	} else if (strstr(cpu_np->name, "sai")) {
		priv->cpu_priv.sysclk_id[1] = FSL_SAI_CLK_MAST1;
		priv->cpu_priv.sysclk_id[0] = FSL_SAI_CLK_MAST1;
	}

	sprintf(priv->name, "%s-audio", codec_dev->name);

	/* Initialize sound card */
	priv->pdev = pdev;
	priv->card.dev = &pdev->dev;
	priv->card.name = priv->name;
	priv->card.dai_link = priv->dai_link;
	priv->card.dapm_routes = audio_map;
	priv->card.late_probe = fsl_asoc_card_late_probe;
	priv->card.num_dapm_routes = ARRAY_SIZE(audio_map);
	priv->card.dapm_widgets = fsl_asoc_card_dapm_widgets;
	priv->card.num_dapm_widgets = ARRAY_SIZE(fsl_asoc_card_dapm_widgets);

	memcpy(priv->dai_link, fsl_asoc_card_dai,
	       sizeof(struct snd_soc_dai_link) * ARRAY_SIZE(priv->dai_link));

	ret = snd_soc_of_parse_audio_routing(&priv->card, "audio-routing");
	if (ret) {
		dev_err(&pdev->dev, "failed to parse audio-routing: %d\n", ret);
		goto asrc_fail;
	}

	/* Normal DAI Link */
	priv->dai_link[0].cpu_of_node = cpu_np;
	priv->dai_link[0].codec_of_node = codec_np;
	priv->dai_link[0].codec_dai_name = codec_dev->name;
	priv->dai_link[0].platform_of_node = cpu_np;
	priv->dai_link[0].dai_fmt = priv->dai_fmt;
	priv->card.num_links = 1;

	if (asrc_pdev) {
		/* DPCM DAI Links only if ASRC exsits */
		priv->dai_link[1].cpu_of_node = asrc_np;
		priv->dai_link[1].platform_of_node = asrc_np;
		priv->dai_link[2].codec_dai_name = codec_dev->name;
		priv->dai_link[2].codec_of_node = codec_np;
		priv->dai_link[2].cpu_of_node = cpu_np;
		priv->dai_link[2].dai_fmt = priv->dai_fmt;
		priv->card.num_links = 3;

		ret = of_property_read_u32(asrc_np, "fsl,asrc-rate",
					   &priv->asrc_rate);
		if (ret) {
			dev_err(&pdev->dev, "failed to get output rate\n");
			ret = -EINVAL;
			goto asrc_fail;
		}

		ret = of_property_read_u32(asrc_np, "fsl,asrc-width", &width);
		if (ret) {
			dev_err(&pdev->dev, "failed to get output rate\n");
			ret = -EINVAL;
			goto asrc_fail;
		}

		if (width == 24)
			priv->asrc_format = SNDRV_PCM_FORMAT_S24_LE;
		else
			priv->asrc_format = SNDRV_PCM_FORMAT_S16_LE;
	}

	/* Finish card registering */
	platform_set_drvdata(pdev, priv);
	snd_soc_card_set_drvdata(&priv->card, priv);

	ret = devm_snd_soc_register_card(&pdev->dev, &priv->card);
	if (ret)
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", ret);

asrc_fail:
	of_node_put(asrc_np);
fail:
	of_node_put(codec_np);
	of_node_put(cpu_np);

	return ret;
}

static const struct of_device_id fsl_asoc_card_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-cs42888", },
	{ .compatible = "fsl,imx-audio-sgtl5000", },
	{ .compatible = "fsl,imx-audio-wm8962", },
	{}
};

static struct platform_driver fsl_asoc_card_driver = {
	.probe = fsl_asoc_card_probe,
	.driver = {
		.name = "fsl-asoc-card",
		.pm = &snd_soc_pm_ops,
		.of_match_table = fsl_asoc_card_dt_ids,
	},
};
module_platform_driver(fsl_asoc_card_driver);

MODULE_DESCRIPTION("Freescale Generic ASoC Sound Card driver with ASRC");
MODULE_AUTHOR("Nicolin Chen <nicoleotsuka@gmail.com>");
MODULE_ALIAS("platform:fsl-asoc-card");
MODULE_LICENSE("GPL");
