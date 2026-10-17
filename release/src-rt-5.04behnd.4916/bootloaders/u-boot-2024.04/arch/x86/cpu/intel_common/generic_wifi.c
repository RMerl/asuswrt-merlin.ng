// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic WiFi ACPI info
 *
 * Copyright 2019 Google LLC
 * Modified from coreboot src/drivers/wifi/generic.c
 */

#include <common.h>
#include <log.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <dm.h>
#include <dm/acpi.h>

/* WRDS Spec Revision */
#define WRDS_REVISION 0x0

/* EWRD Spec Revision */
#define EWRD_REVISION 0x0

/* WRDS Domain type */
#define WRDS_DOMAIN_TYPE_WIFI 0x7

/* EWRD Domain type */
#define EWRD_DOMAIN_TYPE_WIFI 0x7

/* WGDS Domain type */
#define WGDS_DOMAIN_TYPE_WIFI 0x7

/*
 * WIFI ACPI NAME = "WF" + hex value of last 8 bits of dev_path_encode + '\0'
 * The above representation returns unique and consistent name every time
 * generate_wifi_acpi_name is invoked. The last 8 bits of dev_path_encode is
 * chosen since it contains the bus address of the device.
 */
#define WIFI_ACPI_NAME_MAX_LEN 5

/**
 * struct generic_wifi_config - Data structure to contain common wifi config
 * @wake: Wake pin for ACPI _PRW
 * @maxsleep: Maximum sleep state to wake from
 */
struct generic_wifi_config {
	unsigned int wake;
	unsigned int maxsleep;
};

static int generic_wifi_fill_ssdt(struct acpi_ctx *ctx,
				  const struct udevice *dev,
				  const struct generic_wifi_config *config)
{
	char name[ACPI_NAME_MAX];
	char path[ACPI_PATH_MAX];
	pci_dev_t bdf;
	u32 address;
	int ret;

	ret = acpi_device_path(dev_get_parent(dev), path, sizeof(path));
	if (ret)
		return log_msg_ret("path", ret);
	ret = acpi_get_name(dev, name);
	if (ret)
		return log_msg_ret("name", ret);

	/* Device */
	acpigen_write_scope(ctx, path);
	acpigen_write_device(ctx, name);
	acpigen_write_name_integer(ctx, "_UID", 0);
	acpigen_write_name_string(ctx, "_DDN",
				  dev_read_string(dev, "acpi,ddn"));

	/* Address */
	bdf = dm_pci_get_bdf(dev);
	address = (PCI_DEV(bdf) << 16) | PCI_FUNC(bdf);
	acpigen_write_name_dword(ctx, "_ADR", address);

	/* Wake capabilities */
	if (config)
		acpigen_write_prw(ctx, config->wake, config->maxsleep);

	acpigen_pop_len(ctx); /* Device */
	acpigen_pop_len(ctx); /* Scope */

	return 0;
}

static int intel_wifi_acpi_fill_ssdt(const struct udevice *dev,
				     struct acpi_ctx *ctx)
{
	struct generic_wifi_config config;
	bool have_config;
	int ret;

	ret = dev_read_u32(dev, "acpi,wake", &config.wake);
	have_config = !ret;
	/* By default, all intel wifi chips wake from S3 */
	config.maxsleep = 3;
	ret = generic_wifi_fill_ssdt(ctx, dev, have_config ? &config : NULL);
	if (ret)
		return log_msg_ret("wifi", ret);

	return 0;
}

struct acpi_ops wifi_acpi_ops = {
	.fill_ssdt	= intel_wifi_acpi_fill_ssdt,
};

static const struct udevice_id intel_wifi_ids[] = {
	{ .compatible = "intel,generic-wifi" },
	{ }
};

U_BOOT_DRIVER(intel_wifi) = {
	.name		= "intel_wifi",
	.id		= UCLASS_MISC,
	.of_match	= intel_wifi_ids,
	ACPI_OPS_PTR(&wifi_acpi_ops)
};
