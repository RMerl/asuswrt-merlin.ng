// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <i2s.h>
#include <sound.h>

#define SOUND_BITS_IN_BYTE 8

int sound_setup(struct udevice *dev)
{
	struct sound_ops *ops = sound_get_ops(dev);

	if (!ops->setup)
		return 0;

	return ops->setup(dev);
}

int sound_play(struct udevice *dev, void *data, uint data_size)
{
	struct sound_ops *ops = sound_get_ops(dev);

	if (!ops->play)
		return -ENOSYS;

	return ops->play(dev, data, data_size);
}

int sound_start_beep(struct udevice *dev, int frequency_hz)
{
	struct sound_ops *ops = sound_get_ops(dev);

	if (!ops->start_beep)
		return -ENOSYS;

	return ops->start_beep(dev, frequency_hz);
}

int sound_stop_beep(struct udevice *dev)
{
	struct sound_ops *ops = sound_get_ops(dev);

	if (!ops->stop_beep)
		return -ENOSYS;

	return ops->stop_beep(dev);
}

int sound_beep(struct udevice *dev, int msecs, int frequency_hz)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct i2s_uc_priv *i2s_uc_priv;
	unsigned short *data;
	uint data_size;
	int ret;

	ret = sound_setup(dev);
	if (ret && ret != -EALREADY)
		return ret;

	/* Try using the beep interface if available */
	ret = sound_start_beep(dev, frequency_hz);
	if (ret != -ENOSYS) {
		if (ret)
			return ret;
		mdelay(msecs);
		ret = sound_stop_beep(dev);

		return ret;
	}

	/* Buffer length computation */
	i2s_uc_priv = dev_get_uclass_priv(uc_priv->i2s);
	data_size = i2s_uc_priv->samplingrate * i2s_uc_priv->channels;
	data_size *= (i2s_uc_priv->bitspersample / SOUND_BITS_IN_BYTE);
	data = malloc(data_size);
	if (!data) {
		debug("%s: malloc failed\n", __func__);
		return -ENOMEM;
	}

	sound_create_square_wave(i2s_uc_priv->samplingrate, data, data_size,
				 frequency_hz, i2s_uc_priv->channels);

	while (msecs >= 1000) {
		ret = sound_play(dev, data, data_size);
		msecs -= 1000;
	}
	if (msecs) {
		unsigned long size =
			(data_size * msecs) / (sizeof(int) * 1000);

		ret = sound_play(dev, data, size);
	}

	free(data);

	return ret;
}

int sound_find_codec_i2s(struct udevice *dev)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args;
	ofnode node;
	int ret;

	/* First the codec */
	node = ofnode_find_subnode(dev_ofnode(dev), "codec");
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
	ret = uclass_get_device_by_ofnode(UCLASS_AUDIO_CODEC, args.node,
					  &uc_priv->codec);
	if (ret) {
		debug("Cannot find codec: %d\n", ret);
		return ret;
	}

	/* Now the i2s */
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

	return 0;
}

UCLASS_DRIVER(sound) = {
	.id		= UCLASS_SOUND,
	.name		= "sound",
	.per_device_auto_alloc_size	= sizeof(struct sound_uc_priv),
};
