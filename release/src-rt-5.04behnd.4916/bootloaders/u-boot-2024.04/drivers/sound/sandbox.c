// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#define LOG_CATEGORY UCLASS_SOUND

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <i2s.h>
#include <log.h>
#include <sound.h>
#include <asm/sdl.h>

struct sandbox_codec_priv {
	int interface;
	int rate;
	int mclk_freq;
	int bits_per_sample;
	uint channels;
};

struct sandbox_i2s_priv {
	int sum;	/* Use to sum the provided audio data */
	bool silent;	/* Sound is silent, don't use SDL */
};

struct sandbox_sound_priv {
	int setup_called;	/* Incremented when setup() method is called */
	bool active;		/* TX data is being sent */
	int count;		/* Use to count the provided audio data */
	int sum;		/* Use to sum the provided audio data */
	bool allow_beep;	/* true to allow the start_beep() interface */
	int frequency_hz;	/* Beep frequency if active, else 0 */
};

void sandbox_get_codec_params(struct udevice *dev, int *interfacep, int *ratep,
			      int *mclk_freqp, int *bits_per_samplep,
			      uint *channelsp)
{
	struct sandbox_codec_priv *priv = dev_get_priv(dev);

	*interfacep = priv->interface;
	*ratep = priv->rate;
	*mclk_freqp = priv->mclk_freq;
	*bits_per_samplep = priv->bits_per_sample;
	*channelsp = priv->channels;
}

int sandbox_get_i2s_sum(struct udevice *dev)
{
	struct sandbox_i2s_priv *priv = dev_get_priv(dev);

	return priv->sum;
}

int sandbox_get_setup_called(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	return priv->setup_called;
}

int sandbox_get_sound_active(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	return priv->active;
}

int sandbox_get_sound_count(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	return priv->count;
}

int sandbox_get_sound_sum(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	return priv->sum;
}

void sandbox_set_allow_beep(struct udevice *dev, bool allow)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	priv->allow_beep = allow;
}

int sandbox_get_beep_frequency(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	return priv->frequency_hz;
}

static int sandbox_codec_set_params(struct udevice *dev, int interface,
				    int rate, int mclk_freq,
				    int bits_per_sample, uint channels)
{
	struct sandbox_codec_priv *priv = dev_get_priv(dev);

	priv->interface = interface;
	priv->rate = rate;
	priv->mclk_freq = mclk_freq;
	priv->bits_per_sample = bits_per_sample;
	priv->channels = channels;

	return 0;
}

static int sandbox_i2s_tx_data(struct udevice *dev, void *data,
			       uint data_size)
{
	struct sandbox_i2s_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < data_size; i++)
		priv->sum += ((uint8_t *)data)[i];

	if (!priv->silent) {
		int ret;

		ret = sandbox_sdl_sound_play(data, data_size);
		if (ret)
			return ret;
	}

	return 0;
}

static int sandbox_i2s_probe(struct udevice *dev)
{
	struct i2s_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sandbox_i2s_priv *priv = dev_get_priv(dev);

	/* Use hard-coded values here */
	uc_priv->rfs = 256;
	uc_priv->bfs = 32;
	uc_priv->audio_pll_clk = 192000000;
	uc_priv->samplingrate = 48000;
	uc_priv->bitspersample = 16;
	uc_priv->channels = 2;
	uc_priv->id = 1;

	priv->silent = dev_read_bool(dev, "sandbox,silent");

	if (priv->silent) {
		log_warning("Sound is silenced\n");
	} else if (sandbox_sdl_sound_init(uc_priv->samplingrate,
					  uc_priv->channels)) {
		/* Ignore any error here - we'll just have no sound */
		priv->silent = true;
	}

	return 0;
}

static int sandbox_sound_setup(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	priv->setup_called++;

	return 0;
}

static int sandbox_sound_play(struct udevice *dev, void *data, uint data_size)
{
	struct sound_uc_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sandbox_sound_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < data_size; i++)
		priv->sum += ((uint8_t *)data)[i];
	priv->count += data_size;

	return i2s_tx_data(uc_priv->i2s, data, data_size);
}

static int sandbox_sound_stop_play(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	sandbox_sdl_sound_stop();
	priv->active = false;

	return 0;
}

int sandbox_sound_start_beep(struct udevice *dev, int frequency_hz)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	if (!priv->allow_beep)
		return -ENOSYS;
	priv->frequency_hz = frequency_hz;

	return 0;
}

int sandbox_sound_stop_beep(struct udevice *dev)
{
	struct sandbox_sound_priv *priv = dev_get_priv(dev);

	if (!priv->allow_beep)
		return -ENOSYS;
	priv->frequency_hz = 0;

	return 0;
}

static int sandbox_sound_probe(struct udevice *dev)
{
	return sound_find_codec_i2s(dev);
}

static const struct audio_codec_ops sandbox_codec_ops = {
	.set_params	= sandbox_codec_set_params,
};

static const struct udevice_id sandbox_codec_ids[] = {
	{ .compatible = "sandbox,audio-codec" },
	{ }
};

U_BOOT_DRIVER(sandbox_codec) = {
	.name		= "sandbox_codec",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= sandbox_codec_ids,
	.ops		= &sandbox_codec_ops,
	.priv_auto	= sizeof(struct sandbox_codec_priv),
};

static const struct i2s_ops sandbox_i2s_ops = {
	.tx_data	= sandbox_i2s_tx_data,
};

static const struct udevice_id sandbox_i2s_ids[] = {
	{ .compatible = "sandbox,i2s" },
	{ }
};

U_BOOT_DRIVER(sandbox_i2s) = {
	.name		= "sandbox_i2s",
	.id		= UCLASS_I2S,
	.of_match	= sandbox_i2s_ids,
	.ops		= &sandbox_i2s_ops,
	.probe		= sandbox_i2s_probe,
	.priv_auto	= sizeof(struct sandbox_i2s_priv),
};

static const struct sound_ops sandbox_sound_ops = {
	.setup		= sandbox_sound_setup,
	.play		= sandbox_sound_play,
	.stop_play	= sandbox_sound_stop_play,
	.start_beep	= sandbox_sound_start_beep,
	.stop_beep	= sandbox_sound_stop_beep,
};

static const struct udevice_id sandbox_sound_ids[] = {
	{ .compatible = "sandbox,sound" },
	{ }
};

U_BOOT_DRIVER(sandbox_sound) = {
	.name		= "sandbox_sound",
	.id		= UCLASS_SOUND,
	.of_match	= sandbox_sound_ids,
	.ops		= &sandbox_sound_ops,
	.priv_auto	= sizeof(struct sandbox_sound_priv),
	.probe		= sandbox_sound_probe,
};
