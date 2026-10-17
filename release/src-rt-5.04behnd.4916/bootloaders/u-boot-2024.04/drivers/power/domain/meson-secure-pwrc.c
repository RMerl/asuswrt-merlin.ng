// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 SberDevices, Inc.
 * Author: Alexey Romanov <avromanov@sberdevices.ru>
 */

#include <dm.h>
#include <asm/arch/sm.h>
#include <power-domain.h>
#include <power-domain-uclass.h>
#include <dt-bindings/power/meson-a1-power.h>

struct meson_secure_pwrc_domain_desc {
	char *name;
	size_t index;
};

struct meson_secure_pwrc_domain_data {
	unsigned int count;
	struct meson_secure_pwrc_domain_desc *domains;
};

struct meson_secure_pwrc_priv {
	const struct meson_secure_pwrc_domain_data *data;
};

static int meson_secure_pwrc_on(struct power_domain *power_domain)
{
	struct meson_secure_pwrc_priv *priv = dev_get_priv(power_domain->dev);
	struct meson_secure_pwrc_domain_desc *pwrc_domain;
	int err;

	pwrc_domain = &priv->data->domains[power_domain->id];

	err = meson_sm_pwrdm_on(pwrc_domain->index);
	if (err) {
		pr_err("meson_sm_pwrdm_on() failed (%d)\n", err);
		return err;
	}

	pr_debug("enable %s power domain\n", pwrc_domain->name);

	return 0;
}

static int meson_secure_pwrc_off(struct power_domain *power_domain)
{
	struct meson_secure_pwrc_priv *priv = dev_get_priv(power_domain->dev);
	struct meson_secure_pwrc_domain_desc *pwrc_domain;
	int err;

	pwrc_domain = &priv->data->domains[power_domain->id];

	err = meson_sm_pwrdm_off(pwrc_domain->index);
	if (err) {
		pr_err("meson_sm_pwrdm_off() failed (%d)\n", err);
		return err;
	}

	pr_debug("disable %s power domain\n", pwrc_domain->name);

	return 0;
}

static int meson_secure_pwrc_of_xlate(struct power_domain *power_domain,
				  struct ofnode_phandle_args *args)
{
	struct meson_secure_pwrc_priv *priv = dev_get_priv(power_domain->dev);
	struct meson_secure_pwrc_domain_desc *pwrc_domain;

	if (args->args_count < 1) {
		pr_err("invalid args count: %d\n", args->args_count);
		return -EINVAL;
	}

	power_domain->id = args->args[0];

	if (power_domain->id >= priv->data->count) {
		pr_err("domain with ID=%lu is invalid\n", power_domain->id);
		return -EINVAL;
	}

	pwrc_domain = &priv->data->domains[power_domain->id];

	if (!pwrc_domain->name) {
		pr_err("domain with ID=%lu is invalid\n", power_domain->id);
		return -EINVAL;
	}

	return 0;
}

#define SEC_PD(__name)			\
[PWRC_##__name##_ID] =			\
{					\
	.name = #__name,		\
	.index = PWRC_##__name##_ID,	\
}

static struct meson_secure_pwrc_domain_desc a1_pwrc_domains[] = {
	SEC_PD(DSPA),
	SEC_PD(DSPB),
	SEC_PD(UART),
	SEC_PD(DMC),
	SEC_PD(I2C),
	SEC_PD(PSRAM),
	SEC_PD(ACODEC),
	SEC_PD(AUDIO),
	SEC_PD(OTP),
	SEC_PD(DMA),
	SEC_PD(SD_EMMC),
	SEC_PD(RAMA),
	SEC_PD(RAMB),
	SEC_PD(IR),
	SEC_PD(SPICC),
	SEC_PD(SPIFC),
	SEC_PD(USB),
	SEC_PD(NIC),
	SEC_PD(PDMIN),
	SEC_PD(RSA),
};

struct power_domain_ops meson_secure_pwrc_ops = {
	.on = meson_secure_pwrc_on,
	.off = meson_secure_pwrc_off,
	.of_xlate = meson_secure_pwrc_of_xlate,
};

static struct meson_secure_pwrc_domain_data meson_secure_a1_pwrc_data = {
	.count = ARRAY_SIZE(a1_pwrc_domains),
	.domains = a1_pwrc_domains,
};

static const struct udevice_id meson_secure_pwrc_ids[] = {
	{
		.compatible = "amlogic,meson-a1-pwrc",
		.data = (unsigned long)&meson_secure_a1_pwrc_data,
	},
	{ }
};

static int meson_secure_pwrc_probe(struct udevice *dev)
{
	struct meson_secure_pwrc_priv *priv = dev_get_priv(dev);

	priv->data = (void *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(meson_secure_pwrc) = {
	.name = "meson_secure_pwrc",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = meson_secure_pwrc_ids,
	.probe = meson_secure_pwrc_probe,
	.ops = &meson_secure_pwrc_ops,
	.priv_auto = sizeof(struct meson_secure_pwrc_priv),
};
