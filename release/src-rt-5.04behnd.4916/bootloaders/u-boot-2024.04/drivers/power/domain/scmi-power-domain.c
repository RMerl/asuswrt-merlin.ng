// SPDX-License-Identifier: GPL-2.0+
/*
 * SCMI Power domain driver
 *
 * Copyright (C) 2023 Linaro Limited
 *              author: AKASHI Takahiro <takahiro.akashi@linaro.org>
 */

#include <dm.h>
#include <malloc.h>
#include <power-domain.h>
#include <power-domain-uclass.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <dm/device_compat.h>

/**
 * struct scmi_pwd_properties
 * @attributes:	Power domain attributes
 * @name:	Name of the domain
 */
struct scmi_pwd_properties {
	u32 attributes;
	u8 *name; /* not used now */
};

/**
 * struct scmi_power_domain_priv
 * @num_pwdoms:	Number of power domains
 * @prop:	Pointer to domain's properties
 * @stats_addr:	Address of statistics memory region
 * @stats_len:	Length of statistics memory region
 */
struct scmi_power_domain_priv {
	int num_pwdoms;
	struct scmi_pwd_properties *prop;
	u64 stats_addr;
	size_t stats_len;
};

/**
 * async_is_supported - check asynchronous transition
 * @attributes:	Power domain attributes
 *
 * Determine if the power transition can be done asynchronously.
 *
 * Return: true if supported, false if not
 */
static bool async_is_supported(u32 attributes)
{
	if (attributes & SCMI_PWD_ATTR_PSTATE_ASYNC)
		return true;

	/* TODO: check attributes && SCMI_PWD_ATTR_PSTATE_SYNC */
	return false;
}

/**
 * scmi_power_domain_on - Enable the power domain
 * @power_domain:	Power domain
 *
 * Turn on the power domain.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_power_domain_on(struct power_domain *power_domain)
{
	struct scmi_power_domain_priv *priv = dev_get_priv(power_domain->dev);
	u32 flags, pstate;
	int ret;

	if (power_domain->id > priv->num_pwdoms)
		return -EINVAL;

	if (async_is_supported(priv->prop[power_domain->id].attributes))
		flags = SCMI_PWD_SET_FLAGS_ASYNC;
	else
		flags = 0;

	/* ON */
	pstate = 0;

	ret = scmi_pwd_state_set(power_domain->dev, flags, power_domain->id,
				 pstate);
	if (ret) {
		dev_err(power_domain->dev, "failed to set the state on (%d)\n",
			ret);
		return ret;
	}

	return 0;
}

/**
 * scmi_power_domain_off - Disable the power domain
 * @power_domain:	Power domain
 *
 * Turn off the power domain.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_power_domain_off(struct power_domain *power_domain)
{
	struct scmi_power_domain_priv *priv = dev_get_priv(power_domain->dev);
	u32 flags, pstate;
	int ret;

	if (power_domain->id > priv->num_pwdoms)
		return -EINVAL;

	if (async_is_supported(priv->prop[power_domain->id].attributes))
		flags = SCMI_PWD_SET_FLAGS_ASYNC;
	else
		flags = 0;

	/* OFF */
	pstate = SCMI_PWD_PSTATE_TYPE_LOST;

	ret = scmi_pwd_state_set(power_domain->dev, flags, power_domain->id,
				 pstate);
	if (ret) {
		dev_err(power_domain->dev, "failed to set the state off (%d)\n",
			ret);
		return ret;
	}

	return 0;
}

/**
 * scmi_power_domain_probe - Probe the power domain
 * @dev:	Power domain device
 *
 * Probe the power domain and initialize the properties.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_power_domain_probe(struct udevice *dev)
{
	struct scmi_power_domain_priv *priv = dev_get_priv(dev);
	u32 version;
	int i, ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret) {
		dev_err(dev, "failed to get channel (%d)\n", ret);
		return ret;
	}

	ret = scmi_generic_protocol_version(dev, SCMI_PROTOCOL_ID_POWER_DOMAIN,
					    &version);

	ret = scmi_pwd_protocol_attrs(dev, &priv->num_pwdoms, &priv->stats_addr,
				      &priv->stats_len);
	if (ret) {
		dev_err(dev, "failed to get protocol attributes (%d)\n", ret);
		return ret;
	}

	priv->prop = calloc(sizeof(*priv->prop), priv->num_pwdoms);
	if (!priv->prop)
		return -ENOMEM;

	for (i = 0; i < priv->num_pwdoms; i++) {
		ret = scmi_pwd_attrs(dev, i, &priv->prop[i].attributes,
				     &priv->prop[i].name);
		if (ret) {
			dev_err(dev, "failed to get attributes pwd:%d (%d)\n",
				i, ret);
			for (i--; i >= 0; i--)
				free(priv->prop[i].name);
			free(priv->prop);

			return ret;
		}
	}

	return 0;
}

struct power_domain_ops scmi_power_domain_ops = {
	.on = scmi_power_domain_on,
	.off = scmi_power_domain_off,
};

U_BOOT_DRIVER(scmi_power_domain) = {
	.name = "scmi_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &scmi_power_domain_ops,
	.probe = scmi_power_domain_probe,
	.priv_auto = sizeof(struct scmi_power_domain_priv),
};
