// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2s.h>
#include <sound.h>
#include <asm/gpio.h>
#include <asm/arch/power.h>

static int samsung_sound_setup(struct udevice *dev)
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

static int samsung_sound_play(struct udevice *dev, void *data, uint data_size)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);

	return i2s_tx_data(uc_priv->i2s, data, data_size);
}

static int samsung_sound_probe(struct udevice *dev)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args;
	struct gpio_desc en_gpio;
	ofnode node;
	int ret;

	ret = gpio_request_by_name(dev, "codec-enable-gpio", 0, &en_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	/* Turn on the GPIO which connects to the codec's "enable" line. */
	if (!ret)
		gpio_set_pull(gpio_get_number(&en_gpio), S5P_GPIO_PULL_NONE);

	ret = uclass_get_device_by_phandle(UCLASS_AUDIO_CODEC, dev,
					   "samsung,audio-codec",
					   &uc_priv->codec);
	if (ret) {
		debug("Failed to probe audio codec\n");
		return ret;
	}
	node = ofnode_find_subnode(dev_ofnode(dev), "cpu");
	if (!ofnode_valid(node)) {
		debug("Failed to find /cpu subnode\n");
		return -EINVAL;
	}
	ret = ofnode_parse_phandle_with_args(node, "sound-dai",
					     "#sound-dai-cells", 0, 0, &args);
	if (ret) {
		debug("Cannot find phandle: %d\n", ret);
		return ret;
	}
	ret = uclass_get_device_by_ofnode(UCLASS_I2S, args.node, &uc_priv->i2s);
	if (ret) {
		debug("Cannot find i2s: %d\n", ret);
		return ret;
	}
	debug("Probed sound '%s' with codec '%s' and i2s '%s'\n", dev->name,
	      uc_priv->codec->name, uc_priv->i2s->name);

	/* Enable codec clock */
	set_xclkout();

	return 0;
}

static const struct sound_ops samsung_sound_ops = {
	.setup	= samsung_sound_setup,
	.play	= samsung_sound_play,
};

static const struct udevice_id samsung_sound_ids[] = {
	{ .compatible = "google,snow-audio-max98095" },
	{ .compatible = "google,spring-audio-max98088" },
	{ .compatible = "samsung,smdk5420-audio-wm8994" },
	{ .compatible = "google,peach-audio-max98090" },
	{ }
};

U_BOOT_DRIVER(samsung_sound) = {
	.name		= "samsung_sound",
	.id		= UCLASS_SOUND,
	.of_match	= samsung_sound_ids,
	.probe		= samsung_sound_probe,
	.ops		= &samsung_sound_ops,
};
