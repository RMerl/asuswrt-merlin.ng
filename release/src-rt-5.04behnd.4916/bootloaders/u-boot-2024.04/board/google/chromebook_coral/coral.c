// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY	UCLASS_SYSINFO

#include <common.h>
#include <bloblist.h>
#include <command.h>
#include <cros_ec.h>
#include <dm.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <sysinfo.h>
#include <acpi/acpigen.h>
#include <asm-generic/gpio.h>
#include <asm/acpi_nhlt.h>
#include <asm/cb_sysinfo.h>
#include <asm/intel_gnvs.h>
#include <asm/intel_pinctrl.h>
#include <dm/acpi.h>
#include <linux/delay.h>
#include "variant_gpio.h"

DECLARE_GLOBAL_DATA_PTR;

struct cros_gpio_info {
	const char *linux_name;
	enum cros_gpio_t type;
	int gpio_num;
	int flags;
};

static int coral_check_ll_boot(void)
{
	if (!ll_boot_init()) {
		printf("Running as secondary loader");
		if (CONFIG_IS_ENABLED(COREBOOT_SYSINFO) &&
		    gd->arch.coreboot_table) {
			int ret;

			printf(" (found coreboot table at %lx)",
			       gd->arch.coreboot_table);

			ret = get_coreboot_info(&lib_sysinfo);
			if (ret) {
				printf("\nFailed to parse coreboot tables (err=%d)\n",
				       ret);
				return ret;
			}
		}

		printf("\n");
	}

	return 0;
}
EVENT_SPY_SIMPLE(EVT_MISC_INIT_F, coral_check_ll_boot);

int arch_misc_init(void)
{
	return 0;
}

static int get_memconfig(struct udevice *dev)
{
	struct gpio_desc gpios[4];
	int cfg;
	int ret;

	ret = gpio_request_list_by_name(dev, "memconfig-gpios", gpios,
					ARRAY_SIZE(gpios),
					GPIOD_IS_IN | GPIOD_PULL_UP);
	if (ret < 0) {
		log_debug("Cannot get GPIO list '%s' (%d)\n", dev->name, ret);
		return ret;
	}

	/* Give the lines time to settle */
	udelay(10);

	ret = dm_gpio_get_values_as_int(gpios, ARRAY_SIZE(gpios));
	if (ret < 0)
		return log_msg_ret("get", ret);
	cfg = ret;

	ret = gpio_free_list(dev, gpios, ARRAY_SIZE(gpios));
	if (ret)
		return log_msg_ret("free", ret);

	return cfg;
}

/**
 * get_skuconfig() - Get the SKU number either from pins or the EC
 *
 * Two options are supported:
 *     skuconfig-gpios - two pins in the device tree (tried first)
 *     EC              - reading from the EC (backup)
 *
 * @dev: sysinfo device to use
 * Return: SKU ID, or -ve error if not found
 */
static int get_skuconfig(struct udevice *dev)
{
	struct gpio_desc gpios[2];
	int cfg;
	int ret;

	ret = gpio_request_list_by_name(dev, "skuconfig-gpios", gpios,
					ARRAY_SIZE(gpios),
					GPIOD_IS_IN);
	if (ret != ARRAY_SIZE(gpios)) {
		struct udevice *cros_ec;

		log_debug("Cannot get GPIO list '%s' (%d)\n", dev->name, ret);

		/* Try the EC */
		ret = uclass_first_device_err(UCLASS_CROS_EC, &cros_ec);
		if (ret < 0) {
			log_err("Cannot find EC for SKU details\n");
			return log_msg_ret("sku", ret);
		}
		ret = cros_ec_get_sku_id(cros_ec);
		if (ret < 0) {
			log_err("Cannot read SKU details\n");
			return log_msg_ret("sku", ret);
		}

		return ret;
	}

	ret = dm_gpio_get_values_as_int_base3(gpios, ARRAY_SIZE(gpios));
	if (ret < 0)
		return log_msg_ret("get", ret);
	cfg = ret;

	ret = gpio_free_list(dev, gpios, ARRAY_SIZE(gpios));
	if (ret)
		return log_msg_ret("free", ret);

	return cfg;
}

static int coral_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	int ret;

	if (IS_ENABLED(CONFIG_SPL_BUILD))
		return -ENOSYS;

	switch (id) {
	case SYSINFO_ID_SMBIOS_SYSTEM_VERSION:
	case SYSINFO_ID_SMBIOS_BASEBOARD_VERSION: {
		ret = get_skuconfig(dev);

		if (ret < 0)
			return ret;
		if (size < 15)
			return -ENOSPC;
		sprintf(val, "rev%d", ret);
		break;
	}
	case SYSINFO_ID_BOARD_MODEL: {
		int mem_config, sku_config;
		const char *model;

		ret = get_memconfig(dev);
		if (ret < 0)
			log_warning("Unable to read memconfig (err=%d)\n", ret);
		mem_config = ret;
		ret = get_skuconfig(dev);
		if (ret < 0)
			log_warning("Unable to read skuconfig (err=%d)\n", ret);
		sku_config = ret;
		model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
		snprintf(val, size, "%s (memconfig %d, SKU %d)", model,
			 mem_config, sku_config);
		break;
	}
	default:
		return -ENOENT;
	}

	return 0;
}

int chromeos_get_gpio(const struct udevice *dev, const char *prop,
		      enum cros_gpio_t type, struct cros_gpio_info *info)
{
	struct udevice *pinctrl;
	struct gpio_desc desc;
	int ret;

	ret = gpio_request_by_name((struct udevice *)dev, prop, 0, &desc, 0);
	if (ret == -ENOTBLK) {
		info->gpio_num = CROS_GPIO_VIRTUAL;
		log_debug("GPIO '%s' is virtual\n", prop);
	} else if (ret) {
		return log_msg_ret("gpio", ret);
	} else {
		info->gpio_num = desc.offset;
		dm_gpio_free((struct udevice *)dev, &desc);
	}
	info->linux_name = dev_read_string(desc.dev, "linux-name");
	if (!info->linux_name)
		return log_msg_ret("linux-name", -ENOENT);
	info->type = type;
	/* Get ACPI pin from GPIO library if available */
	if (info->gpio_num != CROS_GPIO_VIRTUAL) {
		pinctrl = dev_get_parent(desc.dev);
		info->gpio_num = intel_pinctrl_get_acpi_pin(pinctrl,
							    info->gpio_num);
	}
	info->flags = desc.flags & GPIOD_ACTIVE_LOW ? CROS_GPIO_ACTIVE_LOW :
		CROS_GPIO_ACTIVE_HIGH;
	if (!ret)
		dm_gpio_free(desc.dev, &desc);

	return 0;
}

static int chromeos_acpi_gpio_generate(const struct udevice *dev,
				       struct acpi_ctx *ctx)
{
	struct cros_gpio_info info[3];
	int count, i;
	int ret;

	count = 3;
	ret = chromeos_get_gpio(dev, "recovery-gpios", CROS_GPIO_REC, &info[0]);
	if (ret)
		return log_msg_ret("rec", ret);
	ret = chromeos_get_gpio(dev, "write-protect-gpios", CROS_GPIO_WP,
				&info[1]);
	if (ret)
		return log_msg_ret("wp", ret);
	ret = chromeos_get_gpio(dev, "phase-enforce-gpios", CROS_GPIO_PE,
				&info[2]);
	if (ret)
		return log_msg_ret("phase", ret);
	acpigen_write_scope(ctx, "\\");
	acpigen_write_name(ctx, "OIPG");
	acpigen_write_package(ctx, count);
	for (i = 0; i < count; i++) {
		acpigen_write_package(ctx, 4);
		acpigen_write_integer(ctx, info[i].type);
		acpigen_write_integer(ctx, info[i].flags);
		acpigen_write_integer(ctx, info[i].gpio_num);
		acpigen_write_string(ctx, info[i].linux_name);
		acpigen_pop_len(ctx);
	}

	acpigen_pop_len(ctx);
	acpigen_pop_len(ctx);

	return 0;
}

static int coral_write_acpi_tables(const struct udevice *dev,
				   struct acpi_ctx *ctx)
{
	struct acpi_global_nvs *gnvs;
	struct nhlt *nhlt;
	const char *oem_id = "coral";
	const char *oem_table_id = "coral";
	u32 oem_revision = 3;
	int ret;

	gnvs = bloblist_find(BLOBLISTT_ACPI_GNVS, sizeof(*gnvs));
	if (!gnvs)
		return log_msg_ret("bloblist", -ENOENT);

	nhlt = nhlt_init();
	if (!nhlt)
		return -ENOMEM;

	log_debug("Setting up NHLT\n");
	ret = acpi_setup_nhlt(ctx, nhlt);
	if (ret)
		return log_msg_ret("setup", ret);

	/* Update NHLT GNVS Data */
	gnvs->nhla = (uintptr_t)ctx->current;
	gnvs->nhll = nhlt_current_size(nhlt);

	ret = nhlt_serialise_oem_overrides(ctx, nhlt, oem_id, oem_table_id,
					   oem_revision);
	if (ret)
		return log_msg_ret("serialise", ret);

	return 0;
}

struct acpi_ops coral_acpi_ops = {
	.write_tables	= coral_write_acpi_tables,
	.inject_dsdt	= chromeos_acpi_gpio_generate,
};

struct sysinfo_ops coral_sysinfo_ops = {
	.get_str	= coral_get_str,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id coral_ids[] = {
	{ .compatible = "google,coral" },
	{ }
};
#endif

U_BOOT_DRIVER(coral_drv) = {
	.name		= "coral",
	.id		= UCLASS_SYSINFO,
	.of_match	= of_match_ptr(coral_ids),
	.ops		= &coral_sysinfo_ops,
	ACPI_OPS_PTR(&coral_acpi_ops)
};
