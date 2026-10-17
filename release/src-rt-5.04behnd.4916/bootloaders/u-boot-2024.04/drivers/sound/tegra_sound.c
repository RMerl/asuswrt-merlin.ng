// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_I2S

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2s.h>
#include <log.h>
#include <misc.h>
#include <sound.h>
#include <asm/gpio.h>
#include "tegra_i2s_priv.h"

static int tegra_sound_setup(struct udevice *dev)
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

static int tegra_sound_play(struct udevice *dev, void *data, uint data_size)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);

	return i2s_tx_data(uc_priv->i2s, data, data_size);
}

static int tegra_sound_probe(struct udevice *dev)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct gpio_desc en_gpio;
	struct udevice *ahub;
	int ret;

	ret = gpio_request_by_name(dev, "codec-enable-gpio", 0, &en_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	ret = uclass_get_device_by_phandle(UCLASS_AUDIO_CODEC, dev,
					   "nvidia,audio-codec",
					   &uc_priv->codec);
	if (ret) {
		log_debug("Failed to probe audio codec\n");
		return ret;
	}
	ret = uclass_get_device_by_phandle(UCLASS_I2S, dev,
					   "nvidia,i2s-controller",
					   &uc_priv->i2s);
	if (ret) {
		log_debug("Cannot find i2s: %d\n", ret);
		return ret;
	}

	/* Set up the audio hub, telling it the currect i2s to use */
	ahub = dev_get_parent(uc_priv->i2s);
	ret = misc_ioctl(ahub, AHUB_MISCOP_SET_I2S, &uc_priv->i2s);
	if (ret) {
		log_debug("Cannot set i2c: %d\n", ret);
		return ret;
	}

	log_debug("Probed sound '%s' with codec '%s' and i2s '%s'\n", dev->name,
		  uc_priv->codec->name, uc_priv->i2s->name);

	return 0;
}

static const struct sound_ops tegra_sound_ops = {
	.setup	= tegra_sound_setup,
	.play	= tegra_sound_play,
};

static const struct udevice_id tegra_sound_ids[] = {
	{ .compatible = "nvidia,tegra-audio-max98090-nyan-big" },
	{ }
};

U_BOOT_DRIVER(tegra_sound) = {
	.name		= "tegra_sound",
	.id		= UCLASS_SOUND,
	.of_match	= tegra_sound_ids,
	.probe		= tegra_sound_probe,
	.ops		= &tegra_sound_ops,
};
