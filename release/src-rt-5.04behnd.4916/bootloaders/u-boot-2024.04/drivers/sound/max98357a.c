// SPDX-License-Identifier: GPL-2.0
/*
 * max98357a.c -- MAX98357A Audio driver
 *
 * Copyright 2019 Google LLC
 * Parts taken from coreboot
 */

#include <common.h>
#include <audio_codec.h>
#include <dm.h>
#include <log.h>
#include <sound.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_dp.h>
#include <asm-generic/gpio.h>
#ifdef CONFIG_X86
#include <asm/acpi_nhlt.h>
#endif
#include <dt-bindings/sound/nhlt.h>
#include <dm/acpi.h>

struct max98357a_priv {
	struct gpio_desc sdmode_gpio;
};

static int max98357a_of_to_plat(struct udevice *dev)
{
	struct max98357a_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_request_by_name(dev, "sdmode-gpios", 0, &priv->sdmode_gpio,
				   GPIOD_IS_IN);
	if (ret)
		return log_msg_ret("gpio", ret);

	return 0;
}

__maybe_unused
static int max98357a_acpi_fill_ssdt(const struct udevice *dev,
				    struct acpi_ctx *ctx)
{
	struct max98357a_priv *priv = dev_get_priv(dev);
	char scope[ACPI_PATH_MAX];
	char name[ACPI_NAME_MAX];
	char path[ACPI_PATH_MAX];
	struct acpi_dp *dp;
	int ret;

	ret = acpi_device_scope(dev, scope, sizeof(scope));
	if (ret)
		return log_msg_ret("scope", ret);
	ret = acpi_get_name(dev, name);
	if (ret)
		return log_msg_ret("name", ret);

	/* Device */
	acpigen_write_scope(ctx, scope);
	acpigen_write_device(ctx, name);
	acpigen_write_name_string(ctx, "_HID",
				  dev_read_string(dev, "acpi,hid"));
	acpigen_write_name_integer(ctx, "_UID", 0);
	acpigen_write_name_string(ctx, "_DDN",
				  dev_read_string(dev, "acpi,ddn"));
	acpigen_write_sta(ctx, acpi_device_status(dev));

	/* Resources */
	acpigen_write_name(ctx, "_CRS");
	acpigen_write_resourcetemplate_header(ctx);
	ret = acpi_device_write_gpio_desc(ctx, &priv->sdmode_gpio);
	if (ret < 0)
		return log_msg_ret("gpio", ret);
	acpigen_write_resourcetemplate_footer(ctx);

	/* _DSD for devicetree properties */
	/* This points to the first pin in the first gpio entry in _CRS */
	ret = acpi_device_path(dev, path, sizeof(path));
	if (ret)
		return log_msg_ret("path", ret);
	dp = acpi_dp_new_table("_DSD");
	acpi_dp_add_gpio(dp, "sdmode-gpio", path, 0, 0,
			 priv->sdmode_gpio.flags & GPIOD_ACTIVE_LOW ?
			 ACPI_GPIO_ACTIVE_LOW : ACPI_GPIO_ACTIVE_HIGH);
	acpi_dp_add_integer(dp, "sdmode-delay",
			    dev_read_u32_default(dev, "sdmode-delay", 0));
	acpi_dp_write(ctx, dp);

	acpigen_pop_len(ctx); /* Device */
	acpigen_pop_len(ctx); /* Scope */

	return 0;
}

/* For now only X86 boards support NHLT */
#ifdef CONFIG_X86
static const struct nhlt_format_config max98357a_formats[] = {
	/* 48 KHz 24-bits per sample. */
	{
		.num_channels = 2,
		.sample_freq_khz = 48,
		.container_bits_per_sample = 32,
		.valid_bits_per_sample = 24,
		.settings_file = "max98357-render-2ch-48khz-24b.dat",
	},
};

static const struct nhlt_endp_descriptor max98357a_descriptors[] = {
	{
		.link = NHLT_LINK_SSP,
		.device = NHLT_SSP_DEV_I2S,
		.direction = NHLT_DIR_RENDER,
		.vid = NHLT_VID,
		.did = NHLT_DID_SSP,
		.formats = max98357a_formats,
		.num_formats = ARRAY_SIZE(max98357a_formats),
	},
};

static int max98357a_acpi_setup_nhlt(const struct udevice *dev,
				     struct acpi_ctx *ctx)
{
	u32 hwlink;
	int ret;

	if (dev_read_u32(dev, "acpi,audio-link", &hwlink))
		return log_msg_ret("link", -EINVAL);

	/* Virtual bus id of SSP links are the hardware port ids proper. */
	ret = nhlt_add_ssp_endpoints(ctx->nhlt, hwlink, max98357a_descriptors,
				     ARRAY_SIZE(max98357a_descriptors));
	if (ret)
		return log_msg_ret("add", ret);

	return 0;
}
#endif

struct acpi_ops max98357a_acpi_ops = {
#ifdef CONFIG_ACPIGEN
	.fill_ssdt	= max98357a_acpi_fill_ssdt,
#ifdef CONFIG_X86
	.setup_nhlt	= max98357a_acpi_setup_nhlt,
#endif
#endif
};

static const struct audio_codec_ops max98357a_ops = {
};

static const struct udevice_id max98357a_ids[] = {
	{ .compatible = "maxim,max98357a" },
	{ }
};

U_BOOT_DRIVER(max98357a) = {
	.name		= "max98357a",
	.id		= UCLASS_AUDIO_CODEC,
	.of_match	= max98357a_ids,
	.of_to_plat	= max98357a_of_to_plat,
	.ops		= &max98357a_ops,
	ACPI_OPS_PTR(&max98357a_acpi_ops)
};
