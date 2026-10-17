// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_SOUND

#include <common.h>
#include <audio_codec.h>
#include <clk.h>
#include <dm.h>
#include <i2s.h>
#include <log.h>
#include <misc.h>
#include <sound.h>
#include <asm/arch-rockchip/periph.h>
#include <dm/pinctrl.h>

static int rockchip_sound_setup(struct udevice *dev)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct i2s_uc_priv *i2c_priv = dev_get_uclass_priv(uc_priv->i2s);
	int ret;

	if (uc_priv->setup_done)
		return -EALREADY;
	ret = audio_codec_set_params(uc_priv->codec, i2c_priv->id,
				     i2c_priv->samplingrate,
				     i2c_priv->samplingrate * i2c_priv->rfs,
				     i2c_priv->bitspersample,
				     i2c_priv->channels);
	if (ret)
		return ret;
	uc_priv->setup_done = true;

	return 0;
}

static int rockchip_sound_play(struct udevice *dev, void *data, uint data_size)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);

	return i2s_tx_data(uc_priv->i2s, data, data_size);
}

static int rockchip_sound_probe(struct udevice *dev)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args;
	struct udevice *pinctrl;
	struct clk clk;
	ofnode node;
	int ret;

	node = ofnode_find_subnode(dev_ofnode(dev), "cpu");
	if (!ofnode_valid(node)) {
		log_debug("Failed to find /cpu subnode\n");
		return -EINVAL;
	}
	ret = ofnode_parse_phandle_with_args(node, "sound-dai",
					     "#sound-dai-cells", 0, 0, &args);
	if (ret) {
		log_debug("Cannot find i2s phandle: %d\n", ret);
		return ret;
	}
	ret = uclass_get_device_by_ofnode(UCLASS_I2S, args.node, &uc_priv->i2s);
	if (ret) {
		log_debug("Cannot find i2s: %d\n", ret);
		return ret;
	}

	node = ofnode_find_subnode(dev_ofnode(dev), "codec");
	if (!ofnode_valid(node)) {
		log_debug("Failed to find /codec subnode\n");
		return -EINVAL;
	}
	ret = ofnode_parse_phandle_with_args(node, "sound-dai",
					     "#sound-dai-cells", 0, 0, &args);
	if (ret) {
		log_debug("Cannot find codec phandle: %d\n", ret);
		return ret;
	}
	ret = uclass_get_device_by_ofnode(UCLASS_AUDIO_CODEC, args.node,
					  &uc_priv->codec);
	if (ret) {
		log_debug("Cannot find audio codec: %d\n", ret);
		return ret;
	}
	ret = clk_get_by_index(uc_priv->i2s, 1, &clk);
	if (ret) {
		log_debug("Cannot find clock: %d\n", ret);
		return ret;
	}
	ret = clk_set_rate(&clk, 12288000);
	if (ret < 0) {
		log_debug("Cannot find clock: %d\n", ret);
		return ret;
	}
	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		return ret;
	}
	ret = pinctrl_request(pinctrl, PERIPH_ID_I2S, 0);
	if (ret) {
		debug("%s: Cannot select I2C pinctrl\n", __func__);
		return ret;
	}

	log_debug("Probed sound '%s' with codec '%s' and i2s '%s'\n", dev->name,
		  uc_priv->codec->name, uc_priv->i2s->name);

	return 0;
}

static const struct sound_ops rockchip_sound_ops = {
	.setup	= rockchip_sound_setup,
	.play	= rockchip_sound_play,
};

static const struct udevice_id rockchip_sound_ids[] = {
	{ .compatible = "rockchip,audio-max98090-jerry" },
	{ }
};

U_BOOT_DRIVER(rockchip_sound) = {
	.name		= "rockchip_sound",
	.id		= UCLASS_SOUND,
	.of_match	= rockchip_sound_ids,
	.probe		= rockchip_sound_probe,
	.ops		= &rockchip_sound_ops,
};
