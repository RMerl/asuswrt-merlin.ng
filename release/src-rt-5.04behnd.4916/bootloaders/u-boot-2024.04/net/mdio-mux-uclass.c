// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <miiphy.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <dm/lists.h>

#define MDIO_MUX_CHILD_DRV_NAME	"mdio-mux-bus-drv"

/**
 * struct mdio_mux_perdev_priv - Per-device class data for MDIO MUX DM
 *
 * @parent_mdio: Parent DM MDIO device, this is called for actual MDIO I/O after
 *               setting up the mux.  Typically this is a real MDIO device,
 *               unless there are cascaded muxes.
 * @selected:    Current child bus selection.  Defaults to -1
 */
struct mdio_mux_perdev_priv {
	struct udevice *mdio_parent;
	int selected;
};

/*
 * This source file uses three types of devices, as follows:
 * - mux is the hardware MDIO MUX which selects between the existing child MDIO
 * buses, this is the device relevant for MDIO MUX class of drivers.
 * - ch is a child MDIO bus, this is just a representation of a mux selection,
 * not a real piece of hardware.
 * - mdio_parent is the actual MDIO bus called to perform reads/writes after
 * the MUX is configured.  Typically this is a real MDIO device, unless there
 * are cascaded muxes.
 */

/**
 * struct mdio_mux_ch_data - Per-device data for child MDIOs
 *
 * @sel: Selection value used by the MDIO MUX to access this child MDIO bus
 */
struct mdio_mux_ch_data {
	int sel;
};

static struct udevice *mmux_get_parent_mdio(struct udevice *mux)
{
	struct mdio_mux_perdev_priv *pdata = dev_get_uclass_priv(mux);

	return pdata->mdio_parent;
}

/* call driver select function before performing MDIO r/w */
static int mmux_change_sel(struct udevice *ch, bool sel)
{
	struct udevice *mux = ch->parent;
	struct mdio_mux_perdev_priv *priv = dev_get_uclass_priv(mux);
	struct mdio_mux_ops *ops = mdio_mux_get_ops(mux);
	struct mdio_mux_ch_data *ch_data = dev_get_parent_plat(ch);
	int err = 0;

	if (sel) {
		err = ops->select(mux, priv->selected, ch_data->sel);
		if (err)
			return err;

		priv->selected = ch_data->sel;
	} else {
		if (ops->deselect) {
			ops->deselect(mux, ch_data->sel);
			priv->selected = MDIO_MUX_SELECT_NONE;
		}
	}

	return 0;
}

/* Read wrapper, sets up the mux before issuing a read on parent MDIO bus */
static int mmux_read(struct udevice *ch, int addr, int devad,
		     int reg)
{
	struct udevice *mux = ch->parent;
	struct udevice *parent_mdio = mmux_get_parent_mdio(mux);
	int err;

	err = mmux_change_sel(ch, true);
	if (err)
		return err;

	err = dm_mdio_read(parent_mdio, addr, devad, reg);
	mmux_change_sel(ch, false);

	return err;
}

/* Write wrapper, sets up the mux before issuing a write on parent MDIO bus */
static int mmux_write(struct udevice *ch, int addr, int devad,
		      int reg, u16 val)
{
	struct udevice *mux = ch->parent;
	struct udevice *parent_mdio = mmux_get_parent_mdio(mux);
	int err;

	err = mmux_change_sel(ch, true);
	if (err)
		return err;

	err = dm_mdio_write(parent_mdio, addr, devad, reg, val);
	mmux_change_sel(ch, false);

	return err;
}

/* Reset wrapper, sets up the mux before issuing a reset on parent MDIO bus */
static int mmux_reset(struct udevice *ch)
{
	struct udevice *mux = ch->parent;
	struct udevice *parent_mdio = mmux_get_parent_mdio(mux);
	int err;

	/* reset is optional, if it's not implemented just exit */
	if (!mdio_get_ops(parent_mdio)->reset)
		return 0;

	err = mmux_change_sel(ch, true);
	if (err)
		return err;

	err = dm_mdio_reset(parent_mdio);
	mmux_change_sel(ch, false);

	return err;
}

/* Picks up the mux selection value for each child */
static int dm_mdio_mux_child_post_bind(struct udevice *ch)
{
	struct mdio_mux_ch_data *ch_data = dev_get_parent_plat(ch);

	ch_data->sel = dev_read_u32_default(ch, "reg", MDIO_MUX_SELECT_NONE);

	if (ch_data->sel == MDIO_MUX_SELECT_NONE)
		return -EINVAL;

	return 0;
}

/* Explicitly bind child MDIOs after binding the mux */
static int dm_mdio_mux_post_bind(struct udevice *mux)
{
	ofnode ch_node;
	int err, first_err = 0;

	if (!dev_has_ofnode(mux)) {
		debug("%s: no mux node found, no child MDIO busses set up\n",
		      __func__);
		return 0;
	}

	/*
	 * we're going by Linux bindings so the child nodes do not have
	 * compatible strings.  We're going through them here and binding to
	 * them.
	 */
	dev_for_each_subnode(ch_node, mux) {
		struct udevice *ch_dev;
		const char *ch_name;

		ch_name = ofnode_get_name(ch_node);

		err = device_bind_driver_to_node(mux, MDIO_MUX_CHILD_DRV_NAME,
						 ch_name, ch_node, &ch_dev);
		/* try to bind all, but keep 1st error */
		if (err && !first_err)
			first_err = err;
	}

	return first_err;
}

/* Get a reference to the parent MDIO bus, it should be bound by now */
static int dm_mdio_mux_post_probe(struct udevice *mux)
{
	struct mdio_mux_perdev_priv *priv = dev_get_uclass_priv(mux);
	int err;

	priv->selected = MDIO_MUX_SELECT_NONE;

	/* pick up mdio parent from device tree */
	err = uclass_get_device_by_phandle(UCLASS_MDIO, mux, "mdio-parent-bus",
					   &priv->mdio_parent);
	if (err) {
		debug("%s: didn't find mdio-parent-bus\n", __func__);
		return err;
	}

	return 0;
}

const struct mdio_ops mmux_child_mdio_ops = {
	.read = mmux_read,
	.write = mmux_write,
	.reset = mmux_reset,
};

/* MDIO class driver used for MUX child MDIO buses */
U_BOOT_DRIVER(mdio_mux_child) = {
	.name		= MDIO_MUX_CHILD_DRV_NAME,
	.id		= UCLASS_MDIO,
	.ops		= &mmux_child_mdio_ops,
};

UCLASS_DRIVER(mdio_mux) = {
	.id = UCLASS_MDIO_MUX,
	.name = "mdio-mux",
	.child_post_bind = dm_mdio_mux_child_post_bind,
	.post_bind  = dm_mdio_mux_post_bind,
	.post_probe = dm_mdio_mux_post_probe,
	.per_device_auto	= sizeof(struct mdio_mux_perdev_priv),
	.per_child_plat_auto	= sizeof(struct mdio_mux_ch_data),
};
