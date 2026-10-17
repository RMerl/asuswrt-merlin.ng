// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020-2022 Marvell International Ltd.
 *
 * FDT Helper functions similar to those provided to U-Boot.
 */

#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <linux/delay.h>
#include <asm-generic/gpio.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-fdt.h>

/**
 * Local allocator to handle both SE and U-Boot that also zeroes out memory
 *
 * @param	size	number of bytes to allocate
 *
 * Return:	pointer to allocated memory or NULL if out of memory.
 *		Alignment is set to 8-bytes.
 */
static void *cvmx_fdt_alloc(size_t size)
{
	return calloc(size, 1);
}

int cvmx_ofnode_lookup_phandles(ofnode node, const char *prop_name, int *lenp,
				ofnode *nodes)
{
	const u32 *phandles;
	int count;
	int i;

	phandles = ofnode_get_property(node, prop_name, &count);
	if (!phandles || count < 0)
		return -FDT_ERR_NOTFOUND;

	count /= 4;
	if (count > *lenp)
		count = *lenp;

	for (i = 0; i < count; i++)
		nodes[i] = ofnode_get_by_phandle(fdt32_to_cpu(phandles[i]));

	*lenp = count;
	return 0;
}

/**
 * Given a FDT node return the CPU node number
 *
 * @param[in]	fdt_addr	Address of FDT
 * @param	node		FDT node number
 *
 * Return:	CPU node number or error if negative
 */
int cvmx_fdt_get_cpu_node(const void *fdt_addr, int node)
{
	int parent = node;
	const u32 *ranges;
	int len = 0;

	while (fdt_node_check_compatible(fdt_addr, parent, "simple-bus") != 0) {
		parent = fdt_parent_offset(fdt_addr, parent);
		if (parent < 0)
			return parent;
	}
	ranges = fdt_getprop(fdt_addr, parent, "ranges", &len);
	if (!ranges)
		return len;

	if (len == 0)
		return 0;

	if (len < 24)
		return -FDT_ERR_TRUNCATED;

	return fdt32_to_cpu(ranges[2]) / 0x10;
}

/**
 * Parses all of the channels assigned to a VSC7224 device
 *
 * @param[in]		fdt_addr	Address of flat device tree
 * @param		of_offset	Offset of vsc7224 node
 * @param[in,out]	vsc7224		Data structure to hold the data
 *
 * Return:	0 for success, -1 on error
 */
static int cvmx_fdt_parse_vsc7224_channels(ofnode node,
					   struct cvmx_vsc7224 *vsc7224)
{
	struct ofnode_phandle_args phandle;
	int err = 0;
	int reg;
	int num_chan = 0;
	struct cvmx_vsc7224_chan *channel;
	struct cvmx_fdt_sfp_info *sfp_info;
	int len;
	int num_taps;
	int i;
	const u32 *tap_values;
	int of_mac;
	int xiface, index;
	bool is_tx;
	bool is_qsfp;
	const char *mac_str;
	ofnode node_chan;

	debug("%s(%x, %s)\n", __func__, ofnode_to_offset(node), vsc7224->name);
	ofnode_for_each_compatible_node(node_chan, "vitesse,vsc7224-channel") {
		if (!ofnode_valid(node_chan)) {
			debug("%s: Error parsing FDT node %s\n",
			      __func__, ofnode_get_name(node));
			break;
		}

		if (ofnode_to_offset(ofnode_get_parent(node_chan)) !=
		    ofnode_to_offset(node))
			break;

		reg = ofnode_get_addr(node_chan);
		if (reg < 0 || reg > 3) {
			debug("%s: channel reg is either not present or out of range\n",
			      __func__);
			err = -1;
			break;
		}
		is_tx = ofnode_read_bool(node_chan, "direction-tx");

		debug("%s(%s): Adding %cx channel %d\n",
		      __func__, vsc7224->name, is_tx ? 't' : 'r',
		      reg);
		tap_values = ofnode_get_property(node_chan, "taps", &len);
		if (!tap_values) {
			debug("%s: Error: no taps defined for vsc7224 channel %d\n",
			      __func__, reg);
			err = -1;
			break;
		}

		if (vsc7224->channel[reg]) {
			debug("%s: Error: channel %d already assigned at %p\n",
			      __func__, reg,
			      vsc7224->channel[reg]);
			err = -1;
			break;
		}
		if (len % 16) {
			debug("%s: Error: tap format error for channel %d\n",
			      __func__, reg);
			err = -1;
			break;
		}
		num_taps = len / 16;
		debug("%s: Adding %d taps\n", __func__, num_taps);

		channel = cvmx_fdt_alloc(sizeof(*channel) +
					 num_taps * sizeof(struct cvmx_vsc7224_tap));
		if (!channel) {
			debug("%s: Out of memory\n", __func__);
			err = -1;
			break;
		}
		vsc7224->channel[reg] = channel;
		channel->num_taps = num_taps;
		channel->lane = reg;
		channel->of_offset = ofnode_to_offset(node_chan);
		channel->is_tx = is_tx;
		channel->pretap_disable = ofnode_read_bool(node_chan,
							   "pretap-disable");
		channel->posttap_disable = ofnode_read_bool(node_chan,
							    "posttap-disable");
		channel->vsc7224 = vsc7224;
		/* Read all the tap values */
		for (i = 0; i < num_taps; i++) {
			channel->taps[i].len = fdt32_to_cpu(tap_values[i * 4 + 0]);
			channel->taps[i].main_tap = fdt32_to_cpu(tap_values[i * 4 + 1]);
			channel->taps[i].pre_tap = fdt32_to_cpu(tap_values[i * 4 + 2]);
			channel->taps[i].post_tap = fdt32_to_cpu(tap_values[i * 4 + 3]);
			debug("%s: tap %d: len: %d, main_tap: 0x%x, pre_tap: 0x%x, post_tap: 0x%x\n",
			      __func__, i, channel->taps[i].len, channel->taps[i].main_tap,
			      channel->taps[i].pre_tap, channel->taps[i].post_tap);
		}
		/* Now find out which interface it's mapped to */
		channel->ipd_port = -1;

		mac_str = "sfp-mac";
		if (ofnode_get_property(node_chan, mac_str, NULL)) {
			is_qsfp = false;
		} else if (ofnode_get_property(node_chan, "qsfp-mac", NULL)) {
			is_qsfp = true;
			mac_str = "qsfp-mac";
		} else {
			debug("%s: Error: MAC not found for %s channel %d\n", __func__,
			      vsc7224->name, reg);
			return -1;
		}

		err = ofnode_parse_phandle_with_args(node_chan, mac_str, NULL,
						     0, 0, &phandle);
		if (err) {
			debug("%s: Error %d with MAC %s phandle for %s\n", __func__, of_mac,
			      mac_str, vsc7224->name);
			return -1;
		}

		debug("%s: Found mac at %s\n", __func__,
		      ofnode_get_name(phandle.node));

		xiface = (ofnode_get_addr(ofnode_get_parent(phandle.node))
			  >> 24) & 0x0f;
		index = ofnode_get_addr(phandle.node);
		channel->xiface = xiface;
		channel->index = index;
		channel->ipd_port = cvmx_helper_get_ipd_port(xiface, index);

		debug("%s: Found MAC, xiface: 0x%x, index: %d, ipd port: %d\n", __func__,
		      xiface, index, channel->ipd_port);
		if (channel->ipd_port >= 0) {
			cvmx_helper_cfg_set_vsc7224_chan_info(xiface, index, channel);
			debug("%s: Storing config channel for xiface 0x%x, index %d\n",
			      __func__, xiface, index);
		}
		sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
		if (!sfp_info) {
			debug("%s: Warning: no (Q)SFP+ slot found for xinterface 0x%x, index %d for channel %d\n",
			      __func__, xiface, index, channel->lane);
			continue;
		}

		/* Link it */
		channel->next = sfp_info->vsc7224_chan;
		if (sfp_info->vsc7224_chan)
			sfp_info->vsc7224_chan->prev = channel;
		sfp_info->vsc7224_chan = channel;
		sfp_info->is_vsc7224 = true;
		debug("%s: Registering VSC7224 %s channel %d with SFP %s\n", __func__,
		      vsc7224->name, channel->lane, sfp_info->name);
		if (!sfp_info->mod_abs_changed) {
			debug("%s: Registering cvmx_sfp_vsc7224_mod_abs_changed at %p for xinterface 0x%x, index %d\n",
			      __func__, &cvmx_sfp_vsc7224_mod_abs_changed, xiface, index);
			cvmx_sfp_register_mod_abs_changed(
				sfp_info,
				&cvmx_sfp_vsc7224_mod_abs_changed,
				NULL);
		}

		if (num_chan >= 4)
			break;
	}

	return err;
}

/**
 * @INTERNAL
 * Parses all instances of the Vitesse VSC7224 reclocking chip
 *
 * @param[in]	fdt_addr	Address of flat device tree
 *
 * Return:	0 for success, error otherwise
 */
int __cvmx_fdt_parse_vsc7224(const void *fdt_addr)
{
	struct cvmx_vsc7224 *vsc7224 = NULL;
	ofnode node;
	int err = 0;
	static bool parsed;
	const int *init_array;
	struct udevice *dev;
	u16 value;
	int reg;
	int len;
	int ret;
	int i;

	debug("%s(%p)\n", __func__, fdt_addr);

	if (parsed) {
		debug("%s: Already parsed\n", __func__);
		return 0;
	}

	ofnode_for_each_compatible_node(node, "vitesse,vsc7224") {
		if (!ofnode_valid(node)) {
			debug("%s: Error parsing FDT node %s\n",
			      __func__, ofnode_get_name(node));
			break;
		}

		vsc7224 = cvmx_fdt_alloc(sizeof(*vsc7224));
		if (!vsc7224) {
			debug("%s: Out of memory!\n", __func__);
			return -1;
		}

		vsc7224->of_offset = ofnode_to_offset(node);
		vsc7224->i2c_addr = ofnode_get_addr(node);
		vsc7224->i2c_bus = cvmx_ofnode_get_i2c_bus(ofnode_get_parent(node));
		if (vsc7224->i2c_addr < 0) {
			debug("%s: Error: reg field missing\n", __func__);
			err = -1;
			break;
		}
		if (!vsc7224->i2c_bus) {
			debug("%s: Error getting i2c bus\n", __func__);
			err = -1;
			break;
		}
		vsc7224->name = ofnode_get_name(node);
		debug("%s: Adding %s\n", __func__, vsc7224->name);

		err = gpio_request_by_name_nodev(node, "reset", 0,
						 &vsc7224->reset_gpio,
						 GPIOD_IS_OUT);
		if (err) {
			printf("%s: reset GPIO not found in DT!\n", __func__);
			return -ENODEV;
		}

		err = gpio_request_by_name_nodev(node, "los", 0,
						 &vsc7224->los_gpio,
						 GPIOD_IS_IN);
		if (err) {
			printf("%s: los GPIO not found in DT!\n", __func__);
			return -ENODEV;
		}

		/*
		 * This code was taken from the NIC23 board specific code
		 * but should be better placed here in the common code
		 */
		debug("%s: Putting device in reset\n", __func__);
		dm_gpio_set_value(&vsc7224->reset_gpio, 1);
		mdelay(10);
		debug("%s: Taking device out of reset\n", __func__);
		dm_gpio_set_value(&vsc7224->reset_gpio, 0);
		mdelay(50);

		init_array = ofnode_get_property(node, "vitesse,reg-init",
						 &len);
		if (!init_array) {
			debug("%s: No initialization array\n", __func__);
			continue;
		}
		if ((len % 8) != 0) {
			printf("%s: Error: register init string should be an array of reg number followed by value\n",
			       __func__);
			return -1;
		}

		ret = i2c_get_chip(vsc7224->i2c_bus->i2c_bus,
				   vsc7224->i2c_addr, 1, &dev);
		if (ret) {
			debug("Cannot find I2C device: %d\n", ret);
			return -1;
		}

		for (i = 0; i < len / sizeof(int); i += 2) {
			u8 buffer[2];

			reg = fdt32_to_cpu(init_array[i]);
			value = fdt32_to_cpu(init_array[i + 1]);
			buffer[0] = value >> 8;
			buffer[1] = value & 0xff;
			ret = dm_i2c_write(dev, reg, buffer, 2);
			if (ret) {
				debug("Cannot write I2C device: %d\n", ret);
				return -1;
			}

			debug("  Wrote 0x%02x <= 0x%02x%02x\n", reg,
			      buffer[0], buffer[1]);
		}

		debug("%s: Parsing channels\n", __func__);
		err = cvmx_fdt_parse_vsc7224_channels(node, vsc7224);
		if (err) {
			debug("%s: Error parsing VSC7224 channels\n", __func__);
			break;
		}
	}

	if (err) {
		debug("%s(): Error\n", __func__);
		if (vsc7224) {
			dm_gpio_free(vsc7224->reset_gpio.dev,
				     &vsc7224->reset_gpio);
			dm_gpio_free(vsc7224->los_gpio.dev,
				     &vsc7224->los_gpio);
			if (vsc7224->i2c_bus)
				cvmx_fdt_free_i2c_bus(vsc7224->i2c_bus);
			free(vsc7224);
		}
	}
	if (!err)
		parsed = true;

	return err;
}

/**
 * Given the parent offset of an i2c device build up a list describing the bus
 * which can contain i2c muxes and switches.
 *
 * @param[in]	node		ofnode of the parent node of a GPIO device in
 *				the device tree.
 *
 * @return	pointer to list of i2c devices starting from the root which
 *		can include i2c muxes and switches or NULL if error.  Note that
 *		all entries are allocated on the heap.
 *
 * @see cvmx_fdt_free_i2c_bus()
 */
struct cvmx_fdt_i2c_bus_info *cvmx_ofnode_get_i2c_bus(ofnode node)
{
	struct cvmx_fdt_i2c_bus_info *businfo = NULL;
	struct udevice *bus;
	int ret;

	businfo = cvmx_fdt_alloc(sizeof(*businfo));
	if (!businfo) {
		debug("Out of memory\n");
		return NULL;
	}

	debug("%s: Found node %s\n", __func__, ofnode_get_name(node));
	businfo->of_offset = ofnode_to_offset(node);

	/*
	 * Get I2C bus and probe it automatically - needed for later use
	 */
	ret = device_get_global_by_ofnode(node, &bus);
	if (!bus || ret) {
		printf("Cannot find a I2C bus\n");
		return NULL;
	}

	businfo->i2c_bus = bus;

	return businfo;
}

/**
 * Return the Octeon bus number for a bus descriptor
 *
 * @param[in]	bus	bus descriptor
 *
 * @return	Octeon twsi bus number or -1 on error
 */
int cvmx_fdt_i2c_get_root_bus(const struct cvmx_fdt_i2c_bus_info *bus)
{
	if (bus->type != CVMX_I2C_BUS_OCTEON)
		return -1;
	return bus->channel;
}

/**
 * Frees all entries for an i2c bus descriptor
 *
 * @param	bus	bus to free
 *
 * @return	0
 */
int cvmx_fdt_free_i2c_bus(struct cvmx_fdt_i2c_bus_info *bus)
{
	struct cvmx_fdt_i2c_bus_info *last;

	while (bus) {
		last = bus;
		bus = bus->child;
		free(last);
	}
	return 0;
}
