// SPDX-License-Identifier: GPL-2.0+
/*
 * ACPI driver for DA7219 codec
 *
 * Copyright 2019 Google LLC
 * Parts taken from coreboot
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <irq.h>
#include <log.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_dp.h>
#ifdef CONFIG_X86
#include <asm/acpi_nhlt.h>
#endif
#include <asm-generic/gpio.h>
#include <dt-bindings/sound/nhlt.h>
#include <dm/acpi.h>

#define DA7219_ACPI_HID		"DLGS7219"

__maybe_unused
static int da7219_acpi_fill_ssdt(const struct udevice *dev,
				 struct acpi_ctx *ctx)
{
	char scope[ACPI_PATH_MAX];
	char name[ACPI_NAME_MAX];
	struct acpi_dp *dsd, *aad;
	ofnode node;
	u32 val;
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
	acpigen_write_name_string(ctx, "_HID", DA7219_ACPI_HID);
	acpigen_write_name_integer(ctx, "_UID", 1);
	acpigen_write_name_string(ctx, "_DDN",
				  dev_read_string(dev, "acpi,ddn"));
	acpigen_write_name_integer(ctx, "_S0W", 4);
	acpigen_write_sta(ctx, acpi_device_status(dev));

	/* Resources */
	acpigen_write_name(ctx, "_CRS");
	acpigen_write_resourcetemplate_header(ctx);
	ret = acpi_device_write_i2c_dev(ctx, dev);
	if (ret < 0)
		return log_msg_ret("i2c", ret);

	/* Use either Interrupt() or GpioInt() */
	ret = acpi_device_write_interrupt_or_gpio(ctx, (struct udevice *)dev,
						  "req-gpios");
	if (ret < 0)
		return log_msg_ret("irq_gpio", ret);
	acpigen_write_resourcetemplate_footer(ctx);

	/* AAD Child Device Properties */
	aad = acpi_dp_new_table("DAAD");
	if (!aad)
		return log_msg_ret("aad", -ENOMEM);

	node = ofnode_find_subnode(dev_ofnode(dev), "da7219_aad");
	if (!ofnode_valid(node))
		return log_msg_ret("da7219_aad", -EINVAL);
	acpi_dp_ofnode_copy_int(node, aad, "dlg,btn-cfg");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,mic-det-thr");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,jack-ins-deb");
	acpi_dp_ofnode_copy_str(node, aad, "dlg,jack-det-rate");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,jack-rem-deb");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,a-d-btn-thr");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,d-b-btn-thr");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,b-c-btn-thr");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,c-mic-btn-thr");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,btn-avg");
	acpi_dp_ofnode_copy_int(node, aad, "dlg,adc-1bit-rpt");
	if (!ofnode_read_u32(node, "dlg,micbias-pulse-lvl", &val)) {
		acpi_dp_ofnode_copy_int(node, aad, "dlg,micbias-pulse-lvl");
		acpi_dp_ofnode_copy_int(node, aad, "dlg,micbias-pulse-time");
	}

	/* DA7219 Properties */
	dsd = acpi_dp_new_table("_DSD");
	if (!dsd)
		return log_msg_ret("dsd", -ENOMEM);
	acpi_dp_dev_copy_int(dev, dsd, "dlg,micbias-lvl");
	acpi_dp_dev_copy_str(dev, dsd, "dlg,mic-amp-in-sel");
	acpi_dp_dev_copy_str(dev, dsd, "dlg,mclk-name");
	acpi_dp_add_child(dsd, "da7219_aad", aad);

	/* Write Device Property Hierarchy */
	acpi_dp_write(ctx, dsd);

	acpigen_pop_len(ctx); /* Device */
	acpigen_pop_len(ctx); /* Scope */

	return 0;
}

/* For now only X86 boards support NHLT */
#ifdef CONFIG_X86
static const struct nhlt_format_config da7219_formats[] = {
	/* 48 KHz 24-bits per sample. */
	{
		.num_channels = 2,
		.sample_freq_khz = 48,
		.container_bits_per_sample = 32,
		.valid_bits_per_sample = 24,
		.settings_file = "dialog-2ch-48khz-24b.dat",
	},
};

static const struct nhlt_tdm_config tdm_config = {
	.virtual_slot = 0,
	.config_type = NHLT_TDM_BASIC,
};

static const struct nhlt_endp_descriptor da7219_descriptors[] = {
	/* Render Endpoint */
	{
		.link = NHLT_LINK_SSP,
		.device = NHLT_SSP_DEV_I2S,
		.direction = NHLT_DIR_RENDER,
		.vid = NHLT_VID,
		.did = NHLT_DID_SSP,
		.cfg = &tdm_config,
		.cfg_size = sizeof(tdm_config),
		.formats = da7219_formats,
		.num_formats = ARRAY_SIZE(da7219_formats),
	},
	/* Capture Endpoint */
	{
		.link = NHLT_LINK_SSP,
		.device = NHLT_SSP_DEV_I2S,
		.direction = NHLT_DIR_CAPTURE,
		.vid = NHLT_VID,
		.did = NHLT_DID_SSP,
		.cfg = &tdm_config,
		.cfg_size = sizeof(tdm_config),
		.formats = da7219_formats,
		.num_formats = ARRAY_SIZE(da7219_formats),
	},
};

static int da7219_acpi_setup_nhlt(const struct udevice *dev,
				  struct acpi_ctx *ctx)
{
	u32 hwlink;
	int ret;

	if (dev_read_u32(dev, "acpi,audio-link", &hwlink))
		return log_msg_ret("link", -EINVAL);

	/* Virtual bus id of SSP links are the hardware port ids proper. */
	ret = nhlt_add_ssp_endpoints(ctx->nhlt, hwlink, da7219_descriptors,
				     ARRAY_SIZE(da7219_descriptors));
	if (ret)
		return log_msg_ret("add", ret);

	return 0;
}
#endif

struct acpi_ops da7219_acpi_ops = {
#ifdef CONFIG_ACPIGEN
	.fill_ssdt	= da7219_acpi_fill_ssdt,
#ifdef CONFIG_X86
	.setup_nhlt	= da7219_acpi_setup_nhlt,
#endif
#endif
};

static const struct udevice_id da7219_ids[] = {
	{ .compatible = "dlg,da7219" },
	{ }
};

U_BOOT_DRIVER(da7219) = {
	.name		= "da7219",
	.id		= UCLASS_MISC,
	.of_match	= da7219_ids,
	ACPI_OPS_PTR(&da7219_acpi_ops)
};
