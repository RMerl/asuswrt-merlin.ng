// SPDX-License-Identifier: GPL-2.0
/*
 * Sound for broadwell
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_SOUND

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2s.h>
#include <sound.h>

static int broadwell_sound_probe(struct udevice *dev)
{
	return sound_find_codec_i2s(dev);
}

static int broadwell_sound_setup(struct udevice *dev)
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

static int broadwell_sound_play(struct udevice *dev, void *data, uint data_size)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);

	return i2s_tx_data(uc_priv->i2s, data, data_size);
}

static const struct sound_ops broadwell_sound_ops = {
	.setup		= broadwell_sound_setup,
	.play		= broadwell_sound_play,
};

static const struct udevice_id broadwell_sound_ids[] = {
	{ .compatible = "google,samus-sound" },
	{ }
};

U_BOOT_DRIVER(broadwell_sound_drv) = {
	.name		= "broadwell_sound",
	.id		= UCLASS_SOUND,
	.of_match	= broadwell_sound_ids,
	.probe		= broadwell_sound_probe,
	.ops		= &broadwell_sound_ops,
};
