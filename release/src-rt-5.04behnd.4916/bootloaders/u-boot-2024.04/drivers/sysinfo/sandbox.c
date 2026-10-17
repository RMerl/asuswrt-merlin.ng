// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <sysinfo.h>

#include "sandbox.h"

struct sysinfo_sandbox_priv {
	bool called_detect;
	int test_i1;
	int test_i2;
};

char vacation_spots[][64] = {"R'lyeh", "Dreamlands", "Plateau of Leng",
			     "Carcosa", "Yuggoth", "The Nameless City"};

int sysinfo_sandbox_detect(struct udevice *dev)
{
	struct sysinfo_sandbox_priv *priv = dev_get_priv(dev);

	priv->called_detect = true;
	priv->test_i2 = 100;

	return 0;
}

int sysinfo_sandbox_get_bool(struct udevice *dev, int id, bool *val)
{
	struct sysinfo_sandbox_priv *priv = dev_get_priv(dev);

	switch (id) {
	case BOOL_CALLED_DETECT:
		/* Checks if the dectect method has been called */
		*val = priv->called_detect;
		return 0;
	}

	return -ENOENT;
}

int sysinfo_sandbox_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_sandbox_priv *priv = dev_get_priv(dev);

	switch (id) {
	case INT_TEST1:
		*val = priv->test_i1;
		/* Increments with every call */
		priv->test_i1++;
		return 0;
	case INT_TEST2:
		*val = priv->test_i2;
		/* Decrements with every call */
		priv->test_i2--;
		return 0;
	}

	return -ENOENT;
}

int sysinfo_sandbox_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct sysinfo_sandbox_priv *priv = dev_get_priv(dev);
	int i1 = priv->test_i1;
	int i2 = priv->test_i2;
	int index = (i1 * i2) % ARRAY_SIZE(vacation_spots);

	switch (id) {
	case STR_VACATIONSPOT:
		/* Picks a vacation spot depending on i1 and i2 */
		snprintf(val, size, vacation_spots[index]);
		return 0;
	}

	return -ENOENT;
}

static const struct udevice_id sysinfo_sandbox_ids[] = {
	{ .compatible = "sandbox,sysinfo-sandbox" },
	{ /* sentinel */ }
};

static const struct sysinfo_ops sysinfo_sandbox_ops = {
	.detect = sysinfo_sandbox_detect,
	.get_bool = sysinfo_sandbox_get_bool,
	.get_int = sysinfo_sandbox_get_int,
	.get_str = sysinfo_sandbox_get_str,
};

int sysinfo_sandbox_probe(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(sysinfo_sandbox) = {
	.name           = "sysinfo_sandbox",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_sandbox_ids,
	.ops		= &sysinfo_sandbox_ops,
	.priv_auto	= sizeof(struct sysinfo_sandbox_priv),
	.probe          = sysinfo_sandbox_probe,
};
