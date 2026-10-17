// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Helper functions to abstract board specific data about
 * network ports from the rest of the cvmx-helper files.
 */

#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-gpio.h>

#include <mach/cvmx-smix-defs.h>
#include <mach/cvmx-mdio.h>
#include <mach/cvmx-qlm.h>

DECLARE_GLOBAL_DATA_PTR;

static bool sfp_parsed;

static int __cvmx_helper_78xx_parse_phy(struct cvmx_phy_info *phy_info,
					int ipd_port);
static int __get_phy_info_from_dt(cvmx_phy_info_t *phy_info, int ipd_port);

/**
 * Writes to a Microsemi VSC7224 16-bit register
 *
 * @param[in]	i2c_bus	i2c bus data structure (must be enabled)
 * @param	addr	Address of VSC7224 on the i2c bus
 * @param	reg	8-bit register number to write to
 * @param	val	16-bit value to write
 *
 * @return	0 for success
 */
static int cvmx_write_vsc7224_reg(const struct cvmx_fdt_i2c_bus_info *i2c_bus,
				  u8 addr, u8 reg, u16 val)
{
	struct udevice *dev;
	u8 buffer[2];
	int ret;

	ret = i2c_get_chip(i2c_bus->i2c_bus, addr, 1, &dev);
	if (ret) {
		debug("Cannot find I2C device: %d\n", ret);
		return -1;
	}

	ret = dm_i2c_write(dev, reg, buffer, 2);
	if (ret) {
		debug("Cannot write I2C device: %d\n", ret);
		return -1;
	}

	return 0;
}

/**
 * Writes to a Microsemi VSC7224 16-bit register
 *
 * @param[in]	i2c_bus	i2c bus data structure (must be enabled)
 * @param	addr	Address of VSC7224 on the i2c bus
 * @param	reg	8-bit register number to write to
 *
 * @return	16-bit value or error if < 0
 */
static int cvmx_read_vsc7224_reg(const struct cvmx_fdt_i2c_bus_info *i2c_bus,
				 u8 addr, u8 reg)
{
	struct udevice *dev;
	u8 buffer[2];
	int ret;

	ret = i2c_get_chip(i2c_bus->i2c_bus, addr, 1, &dev);
	if (ret) {
		debug("Cannot find I2C device: %d\n", ret);
		return -1;
	}

	ret = dm_i2c_read(dev, reg, buffer, 2);
	if (ret) {
		debug("Cannot read I2C device: %d\n", ret);
		return -1;
	}

	return (buffer[0] << 8) | buffer[1];
}

/**
 * Function called whenever mod_abs/mod_prs has changed for Microsemi VSC7224
 *
 * @param	sfp	pointer to SFP data structure
 * @param	val	1 if absent, 0 if present, otherwise not set
 * @param	data	user-defined data
 *
 * @return	0 for success, -1 on error
 */
int cvmx_sfp_vsc7224_mod_abs_changed(struct cvmx_fdt_sfp_info *sfp, int val,
				     void *data)
{
	int err;
	struct cvmx_sfp_mod_info *mod_info;
	int length;
	struct cvmx_vsc7224 *vsc7224;
	struct cvmx_vsc7224_chan *vsc7224_chan;
	struct cvmx_vsc7224_tap *taps, *match = NULL;
	int i;

	debug("%s(%s, %d, %p): Module %s\n", __func__, sfp->name, val, data,
	      val ? "absent" : "present");
	if (val)
		return 0;

	/* We're here if we detect that the module is now present */
	err = cvmx_sfp_read_i2c_eeprom(sfp);
	if (err) {
		debug("%s: Error reading the SFP module eeprom for %s\n",
		      __func__, sfp->name);
		return err;
	}
	mod_info = &sfp->sfp_info;

	if (!mod_info->valid || !sfp->valid) {
		debug("%s: Module data is invalid\n", __func__);
		return -1;
	}

	vsc7224_chan = sfp->vsc7224_chan;
	while (vsc7224_chan) {
		/* We don't do any rx tuning */
		if (!vsc7224_chan->is_tx) {
			vsc7224_chan = vsc7224_chan->next;
			continue;
		}

		/* Walk through all the channels */
		taps = vsc7224_chan->taps;
		if (mod_info->limiting)
			length = 0;
		else
			length = mod_info->max_copper_cable_len;
		debug("%s: limiting: %d, length: %d\n", __func__,
		      mod_info->limiting, length);

		/* Find a matching length in the taps table */
		for (i = 0; i < vsc7224_chan->num_taps; i++) {
			if (length >= taps->len)
				match = taps;
			taps++;
		}
		if (!match) {
			debug("%s(%s, %d, %p): Error: no matching tap for length %d\n",
			      __func__, sfp->name, val, data, length);
			return -1;
		}
		debug("%s(%s): Applying %cx taps to vsc7224 %s:%d for cable length %d+\n",
		      __func__, sfp->name, vsc7224_chan->is_tx ? 't' : 'r',
		      vsc7224_chan->vsc7224->name, vsc7224_chan->lane,
		      match->len);
		/* Program the taps */
		vsc7224 = vsc7224_chan->vsc7224;
		cvmx_write_vsc7224_reg(vsc7224->i2c_bus, vsc7224->i2c_addr,
				       0x7f, vsc7224_chan->lane);
		if (!vsc7224_chan->maintap_disable)
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr, 0x99,
					       match->main_tap);
		if (!vsc7224_chan->pretap_disable)
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr, 0x9a,
					       match->pre_tap);
		if (!vsc7224_chan->posttap_disable)
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr, 0x9b,
					       match->post_tap);

		/* Re-use val and disable taps if needed */
		if (vsc7224_chan->maintap_disable ||
		    vsc7224_chan->pretap_disable ||
		    vsc7224_chan->posttap_disable) {
			val = cvmx_read_vsc7224_reg(vsc7224->i2c_bus,
						    vsc7224->i2c_addr, 0x97);
			if (vsc7224_chan->maintap_disable)
				val |= 0x800;
			if (vsc7224_chan->pretap_disable)
				val |= 0x1000;
			if (vsc7224_chan->posttap_disable)
				val |= 0x400;
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr, 0x97, val);
		}
		vsc7224_chan = vsc7224_chan->next;
	}

	return err;
}

/**
 * Update the mod_abs and error LED
 *
 * @param	ipd_port	ipd port number
 * @param	link		link information
 */
static void __cvmx_helper_update_sfp(int ipd_port,
				     struct cvmx_fdt_sfp_info *sfp_info,
				     cvmx_helper_link_info_t link)
{
	debug("%s(%d): checking mod_abs\n", __func__, ipd_port);

	cvmx_sfp_check_mod_abs(sfp_info, sfp_info->mod_abs_data);
}

static void cvmx_sfp_update_link(struct cvmx_fdt_sfp_info *sfp,
				 cvmx_helper_link_info_t link)
{
	while (sfp) {
		debug("%s(%s): checking mod_abs\n", __func__, sfp->name);
		if (link.s.link_up && sfp->last_mod_abs)
			cvmx_sfp_check_mod_abs(sfp, sfp->mod_abs_data);
		sfp = sfp->next_iface_sfp;
	}
}

/**
 * @INTERNAL
 * This function is used ethernet ports link speed. This functions uses the
 * device tree information to determine the phy address and type of PHY.
 * The only supproted PHYs are Marvell and Broadcom.
 *
 * @param ipd_port IPD input port associated with the port we want to get link
 *                 status for.
 *
 * @return The ports link status. If the link isn't fully resolved, this must
 *         return zero.
 */
cvmx_helper_link_info_t __cvmx_helper_board_link_get_from_dt(int ipd_port)
{
	cvmx_helper_link_info_t result;
	cvmx_phy_info_t *phy_info = NULL;
	cvmx_phy_info_t local_phy_info;
	int xiface = 0, index = 0;
	bool use_inband = false;
	struct cvmx_fdt_sfp_info *sfp_info;
	const void *fdt_addr = CASTPTR(const void *, gd->fdt_blob);

	result.u64 = 0;

	if (ipd_port >= 0) {
		int mode;

		xiface = cvmx_helper_get_interface_num(ipd_port);
		index = cvmx_helper_get_interface_index_num(ipd_port);
		mode = cvmx_helper_interface_get_mode(xiface);
		if (!cvmx_helper_get_port_autonegotiation(xiface, index)) {
			result.s.link_up = 1;
			result.s.full_duplex = 1;
			switch (mode) {
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			case CVMX_HELPER_INTERFACE_MODE_AGL:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
				if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
					struct cvmx_xiface xi =
						cvmx_helper_xiface_to_node_interface(
							xiface);
					u64 gbaud = cvmx_qlm_get_gbaud_mhz(0);

					result.s.speed = gbaud * 8 / 10;
					if (cvmx_qlm_get_dlm_mode(
						    0, xi.interface) ==
					    CVMX_QLM_MODE_SGMII)
						result.s.speed >>= 1;
					else
						result.s.speed >>= 2;
				} else {
					result.s.speed = 1000;
				}
				break;
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_10G_KR:
			case CVMX_HELPER_INTERFACE_MODE_XFI:
				result.s.speed = 10000;
				break;
			case CVMX_HELPER_INTERFACE_MODE_XLAUI:
			case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
				result.s.speed = 40000;
				break;
			default:
				break;
			}

			sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
			/* Initialize the SFP info if it hasn't already been
			 * done.
			 */
			if (!sfp_info && !sfp_parsed) {
				cvmx_sfp_parse_device_tree(fdt_addr);
				sfp_parsed = true;
				cvmx_sfp_read_all_modules();
				sfp_info = cvmx_helper_cfg_get_sfp_info(xiface,
									index);
			}
			/* If the link is down or the link is up but we still
			 * register the module as being absent, re-check
			 * mod_abs.
			 */
			cvmx_sfp_update_link(sfp_info, result);

			cvmx_helper_update_link_led(xiface, index, result);

			return result;
		}
		phy_info = cvmx_helper_get_port_phy_info(xiface, index);
		if (!phy_info) {
			debug("%s: phy info not saved in config, allocating for 0x%x:%d\n",
			      __func__, xiface, index);

			phy_info = (cvmx_phy_info_t *)cvmx_bootmem_alloc(
				sizeof(*phy_info), 0);
			if (!phy_info) {
				debug("%s: Out of memory\n", __func__);
				return result;
			}
			memset(phy_info, 0, sizeof(*phy_info));
			phy_info->phy_addr = -1;
			debug("%s: Setting phy info for 0x%x:%d to %p\n",
			      __func__, xiface, index, phy_info);
			cvmx_helper_set_port_phy_info(xiface, index, phy_info);
		}
	} else {
		/* For management ports we don't store the PHY information
		 * so we use a local copy instead.
		 */
		phy_info = &local_phy_info;
		memset(phy_info, 0, sizeof(*phy_info));
		phy_info->phy_addr = -1;
	}

	if (phy_info->phy_addr == -1) {
		if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
			if (__cvmx_helper_78xx_parse_phy(phy_info, ipd_port)) {
				phy_info->phy_addr = -1;
				use_inband = true;
			}
		} else if (__get_phy_info_from_dt(phy_info, ipd_port) < 0) {
			phy_info->phy_addr = -1;
			use_inband = true;
		}
	}

	/* If we can't get the PHY info from the device tree then try
	 * the inband state.
	 */
	if (use_inband) {
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		result.s.speed = 1000;
		return result;
	}

	if (phy_info->phy_addr < 0)
		return result;

	if (phy_info->link_function)
		result = phy_info->link_function(phy_info);
	else
		result = cvmx_helper_link_get(ipd_port);

	sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
	while (sfp_info) {
		/* If the link is down or the link is up but we still register
		 * the module as being absent, re-check mod_abs.
		 */
		if (!result.s.link_up ||
		    (result.s.link_up && sfp_info->last_mod_abs))
			__cvmx_helper_update_sfp(ipd_port, sfp_info, result);
		sfp_info = sfp_info->next_iface_sfp;
	}

	return result;
}

cvmx_helper_link_info_t __cvmx_helper_board_link_get(int ipd_port)
{
	cvmx_helper_link_info_t result;

	/* Unless we fix it later, all links are defaulted to down */
	result.u64 = 0;

	return __cvmx_helper_board_link_get_from_dt(ipd_port);
}

void cvmx_helper_update_link_led(int xiface, int index,
				 cvmx_helper_link_info_t result)
{
}

void cvmx_helper_leds_show_error(struct cvmx_phy_gpio_leds *leds, bool error)
{
}

int __cvmx_helper_board_interface_probe(int interface, int supported_ports)
{
	return supported_ports;
}

/**
 * Returns the Ethernet node offset in the device tree
 *
 * @param     fdt_addr - pointer to flat device tree in memory
 * @param     aliases    - offset of alias in device tree
 * @param     ipd_port - ipd port number to look up
 *
 * @returns   offset of Ethernet node if >= 0, error if -1
 */
int __pip_eth_node(const void *fdt_addr, int aliases, int ipd_port)
{
	char name_buffer[20];
	const char *pip_path;
	int pip, iface, eth;
	int interface_num = cvmx_helper_get_interface_num(ipd_port);
	int interface_index = cvmx_helper_get_interface_index_num(ipd_port);
	cvmx_helper_interface_mode_t interface_mode =
		cvmx_helper_interface_get_mode(interface_num);

	/* The following are not found in the device tree */
	switch (interface_mode) {
	case CVMX_HELPER_INTERFACE_MODE_ILK:
	case CVMX_HELPER_INTERFACE_MODE_LOOP:
	case CVMX_HELPER_INTERFACE_MODE_SRIO:
		debug("ERROR: No node expected for interface: %d, port: %d, mode: %s\n",
		      interface_index, ipd_port,
		      cvmx_helper_interface_mode_to_string(interface_mode));
		return -1;
	default:
		break;
	}
	pip_path = (const char *)fdt_getprop(fdt_addr, aliases, "pip", NULL);
	if (!pip_path) {
		debug("ERROR: pip path not found in device tree\n");
		return -1;
	}
	pip = fdt_path_offset(fdt_addr, pip_path);
	debug("ipdd_port=%d pip_path=%s pip=%d ", ipd_port, pip_path, pip);
	if (pip < 0) {
		debug("ERROR: pip not found in device tree\n");
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "interface@%d",
		 interface_num);
	iface = fdt_subnode_offset(fdt_addr, pip, name_buffer);
	debug("iface=%d ", iface);
	if (iface < 0) {
		debug("ERROR : pip intf %d not found in device tree\n",
		      interface_num);
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "ethernet@%x",
		 interface_index);
	eth = fdt_subnode_offset(fdt_addr, iface, name_buffer);
	debug("eth=%d\n", eth);
	if (eth < 0) {
		debug("ERROR : pip interface@%d ethernet@%d not found in device tree\n",
		      interface_num, interface_index);
		return -1;
	}
	return eth;
}

int __mix_eth_node(const void *fdt_addr, int aliases, int interface_index)
{
	char name_buffer[20];
	const char *mix_path;
	int mix;

	snprintf(name_buffer, sizeof(name_buffer), "mix%d", interface_index);
	mix_path =
		(const char *)fdt_getprop(fdt_addr, aliases, name_buffer, NULL);
	if (!mix_path) {
		debug("ERROR: mix%d path not found in device tree\n",
		      interface_index);
	}
	mix = fdt_path_offset(fdt_addr, mix_path);
	if (mix < 0) {
		debug("ERROR: %s not found in device tree\n", mix_path);
		return -1;
	}
	return mix;
}

static int __mdiobus_addr_to_unit(u32 addr)
{
	int unit = (addr >> 7) & 3;

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX) && !OCTEON_IS_MODEL(OCTEON_CN78XX))
		unit >>= 1;
	return unit;
}

/**
 * Parse the muxed MDIO interface information from the device tree
 *
 * @param phy_info - pointer to phy info data structure to update
 * @param mdio_offset - offset of MDIO bus
 * @param mux_offset - offset of MUX, parent of mdio_offset
 *
 * @return 0 for success or -1
 */
static int __get_muxed_mdio_info_from_dt(cvmx_phy_info_t *phy_info,
					 int mdio_offset, int mux_offset)
{
	const void *fdt_addr = CASTPTR(const void *, gd->fdt_blob);
	int phandle;
	int smi_offset;
	int gpio_offset;
	u64 smi_addr = 0;
	int len;
	u32 *pgpio_handle;
	int gpio_count = 0;
	u32 *prop_val;
	int offset;
	const char *prop_name;

	debug("%s(%p, 0x%x, 0x%x)\n", __func__, phy_info, mdio_offset,
	      mux_offset);

	/* Get register value to put onto the GPIO lines to select */
	phy_info->gpio_value =
		cvmx_fdt_get_int(fdt_addr, mdio_offset, "reg", -1);
	if (phy_info->gpio_value < 0) {
		debug("Could not get register value for muxed MDIO bus from DT\n");
		return -1;
	}

	smi_offset = cvmx_fdt_lookup_phandle(fdt_addr, mux_offset,
					     "mdio-parent-bus");
	if (smi_offset < 0) {
		debug("Invalid SMI offset for muxed MDIO interface in device tree\n");
		return -1;
	}
	smi_addr = cvmx_fdt_get_uint64(fdt_addr, smi_offset, "reg", 0);

	/* Convert SMI address to a MDIO interface */
	switch (smi_addr) {
	case 0x1180000001800:
	case 0x1180000003800: /* 68XX address */
		phy_info->mdio_unit = 0;
		break;
	case 0x1180000001900:
	case 0x1180000003880:
		phy_info->mdio_unit = 1;
		break;
	case 0x1180000003900:
		phy_info->mdio_unit = 2;
		break;
	case 0x1180000003980:
		phy_info->mdio_unit = 3;
		break;
	default:
		phy_info->mdio_unit = 1;
		break;
	}
	/* Find the GPIO MUX controller */
	pgpio_handle =
		(u32 *)fdt_getprop(fdt_addr, mux_offset, "gpios", &len);
	if (!pgpio_handle || len < 12 || (len % 12) != 0 ||
	    len > CVMX_PHY_MUX_MAX_GPIO * 12) {
		debug("Invalid GPIO for muxed MDIO controller in DT\n");
		return -1;
	}

	for (gpio_count = 0; gpio_count < len / 12; gpio_count++) {
		phandle = fdt32_to_cpu(pgpio_handle[gpio_count * 3]);
		phy_info->gpio[gpio_count] =
			fdt32_to_cpu(pgpio_handle[gpio_count * 3 + 1]);
		gpio_offset = fdt_node_offset_by_phandle(fdt_addr, phandle);
		if (gpio_offset < 0) {
			debug("Cannot access parent GPIO node in DT\n");
			return -1;
		}
		if (!fdt_node_check_compatible(fdt_addr, gpio_offset,
					       "cavium,octeon-3860-gpio")) {
			phy_info->gpio_type[gpio_count] = GPIO_OCTEON;
		} else if (!fdt_node_check_compatible(fdt_addr, gpio_offset,
						      "nxp,pca8574")) {
			/* GPIO is a TWSI GPIO unit which might sit behind
			 * another mux.
			 */
			phy_info->gpio_type[gpio_count] = GPIO_PCA8574;
			prop_val = (u32 *)fdt_getprop(
				fdt_addr, gpio_offset, "reg", NULL);
			if (!prop_val) {
				debug("Could not find TWSI address of npx pca8574 GPIO from DT\n");
				return -1;
			}
			/* Get the TWSI address of the GPIO unit */
			phy_info->cvmx_gpio_twsi[gpio_count] =
				fdt32_to_cpu(*prop_val);
			/* Get the selector on the GPIO mux if present */
			offset = fdt_parent_offset(fdt_addr, gpio_offset);
			prop_val = (u32 *)fdt_getprop(fdt_addr, offset,
							   "reg", NULL);
			if (prop_val) {
				phy_info->gpio_parent_mux_select =
					fdt32_to_cpu(*prop_val);
				/* Go up another level */
				offset = fdt_parent_offset(fdt_addr, offset);
				if (!fdt_node_check_compatible(fdt_addr, offset,
							       "nxp,pca9548")) {
					prop_val = (u32 *)fdt_getprop(
						fdt_addr, offset, "reg", NULL);
					if (!prop_val) {
						debug("Could not read MUX TWSI address from DT\n");
						return -1;
					}
					phy_info->gpio_parent_mux_twsi =
						fdt32_to_cpu(*prop_val);
				}
			}
		} else {
			prop_name = (char *)fdt_getprop(fdt_addr, gpio_offset,
							"compatible", NULL);
			debug("Unknown GPIO type %s\n", prop_name);
			return -1;
		}
	}
	return 0;
}

/**
 * @INTERNAL
 * Converts a BGX address to the node, interface and port number
 *
 * @param bgx_addr	Address of CSR register
 *
 * @return node, interface and port number, will be -1 for invalid address.
 */
static struct cvmx_xiface __cvmx_bgx_reg_addr_to_xiface(u64 bgx_addr)
{
	struct cvmx_xiface xi = { -1, -1 };

	xi.node = cvmx_csr_addr_to_node(bgx_addr);
	bgx_addr = cvmx_csr_addr_strip_node(bgx_addr);
	if ((bgx_addr & 0xFFFFFFFFF0000000) != 0x00011800E0000000) {
		debug("%s: Invalid BGX address 0x%llx\n", __func__,
		      (unsigned long long)bgx_addr);
		xi.node = -1;
		return xi;
	}
	xi.interface = (bgx_addr >> 24) & 0x0F;

	return xi;
}

static cvmx_helper_link_info_t
__get_marvell_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	int phy_status;
	u32 phy_addr = phy_info->phy_addr;

	result.u64 = 0;
	/* Set to page 0 */
	cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, 22, 0);
	/* All the speed information can be read from register 17 in one go. */
	phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 17);

	/* If the resolve bit 11 isn't set, see if autoneg is turned off
	 * (bit 12, reg 0). The resolve bit doesn't get set properly when
	 * autoneg is off, so force it
	 */
	if ((phy_status & (1 << 11)) == 0) {
		int auto_status =
			cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0);
		if ((auto_status & (1 << 12)) == 0)
			phy_status |= 1 << 11;
	}

	/* Link is up = Speed/Duplex Resolved + RT-Link Up + G-Link Up. */
	if ((phy_status & 0x0c08) == 0x0c08) {
		result.s.link_up = 1;
		result.s.full_duplex = ((phy_status >> 13) & 1);
		switch ((phy_status >> 14) & 3) {
		case 0: /* 10 Mbps */
			result.s.speed = 10;
			break;
		case 1: /* 100 Mbps */
			result.s.speed = 100;
			break;
		case 2: /* 1 Gbps */
			result.s.speed = 1000;
			break;
		case 3: /* Illegal */
			result.u64 = 0;
			break;
		}
	}
	return result;
}

/**
 * @INTERNAL
 * Get link state of broadcom PHY
 *
 * @param phy_info	PHY information
 */
static cvmx_helper_link_info_t
__get_broadcom_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	u32 phy_addr = phy_info->phy_addr;
	int phy_status;

	result.u64 = 0;
	/* Below we are going to read SMI/MDIO register 0x19 which works
	 * on Broadcom parts
	 */
	phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x19);
	switch ((phy_status >> 8) & 0x7) {
	case 0:
		result.u64 = 0;
		break;
	case 1:
		result.s.link_up = 1;
		result.s.full_duplex = 0;
		result.s.speed = 10;
		break;
	case 2:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 10;
		break;
	case 3:
		result.s.link_up = 1;
		result.s.full_duplex = 0;
		result.s.speed = 100;
		break;
	case 4:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 100;
		break;
	case 5:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 100;
		break;
	case 6:
		result.s.link_up = 1;
		result.s.full_duplex = 0;
		result.s.speed = 1000;
		break;
	case 7:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 1000;
		break;
	}
	return result;
}

/**
 * @INTERNAL
 * Get link state of generic gigabit PHY
 *
 * @param phy_info - information about the PHY
 *
 * @returns link status of the PHY
 */
static cvmx_helper_link_info_t
__cvmx_get_generic_8023_c22_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	u32 phy_addr = phy_info->phy_addr;
	int phy_basic_control;	 /* Register 0x0 */
	int phy_basic_status;	 /* Register 0x1 */
	int phy_anog_adv;	 /* Register 0x4 */
	int phy_link_part_avail; /* Register 0x5 */
	int phy_control;	 /* Register 0x9 */
	int phy_status;		 /* Register 0xA */

	result.u64 = 0;

	phy_basic_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 1);
	if (!(phy_basic_status & 0x4)) /* Check if link is up */
		return result;	       /* Link is down, return link down */

	result.s.link_up = 1;
	phy_basic_control = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0);
	/* Check if autonegotiation is enabled and completed */
	if ((phy_basic_control & (1 << 12)) && (phy_basic_status & (1 << 5))) {
		phy_status =
			cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0xA);
		phy_control =
			cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x9);

		phy_status &= phy_control << 2;
		phy_link_part_avail =
			cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x5);
		phy_anog_adv =
			cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x4);
		phy_link_part_avail &= phy_anog_adv;

		if (phy_status & 0xC00) { /* Gigabit full or half */
			result.s.speed = 1000;
			result.s.full_duplex = !!(phy_status & 0x800);
		} else if (phy_link_part_avail &
			   0x0180) { /* 100 full or half */
			result.s.speed = 100;
			result.s.full_duplex = !!(phy_link_part_avail & 0x100);
		} else if (phy_link_part_avail & 0x0060) {
			result.s.speed = 10;
			result.s.full_duplex = !!(phy_link_part_avail & 0x0040);
		}
	} else {
		/* Not autonegotiated */
		result.s.full_duplex = !!(phy_basic_control & (1 << 8));

		if (phy_basic_control & (1 << 6))
			result.s.speed = 1000;
		else if (phy_basic_control & (1 << 13))
			result.s.speed = 100;
		else
			result.s.speed = 10;
	}
	return result;
}

static cvmx_helper_link_info_t
__cvmx_get_qualcomm_s17_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	u32 phy_addr = phy_info->phy_addr;
	int phy_status;
	int auto_status;

	result.u64 = 0;

	phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 17);

	/* If bit 11 isn't set see if autonegotiation is turned off
	 * (bit 12, reg 0).  The resolved bit doesn't get set properly when
	 * autonegotiation is off, so force it.
	 */
	if ((phy_status & (1 << 11)) == 0) {
		auto_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0);
		if ((auto_status & (1 << 12)) == 0)
			phy_status |= 1 << 11;
	}
	/* Only return a link if the PHY has finished autonegotiation and set
	 * the resolved bit (bit 11).
	 */
	if (phy_status & (1 << 11)) {
		result.s.link_up = 1;
		result.s.full_duplex = !!(phy_status & (1 << 13));
		switch ((phy_status >> 14) & 3) {
		case 0: /* 10Mbps */
			result.s.speed = 10;
			break;
		case 1: /* 100Mbps */
			result.s.speed = 100;
			break;
		case 2: /* 1Gbps */
			result.s.speed = 1000;
			break;
		default: /* Illegal */
			result.u64 = 0;
			break;
		}
	}
	debug("   link: %s, duplex: %s, speed: %lu\n",
	      result.s.link_up ? "up" : "down",
	      result.s.full_duplex ? "full" : "half",
	      (unsigned long)result.s.speed);
	return result;
}

static cvmx_helper_link_info_t
__get_generic_8023_c45_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	int phy_status;
	int pma_ctrl1;
	u32 phy_addr = phy_info->phy_addr;

	result.u64 = 0;
	pma_ctrl1 = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 1, 0);
	if ((pma_ctrl1 & 0x207c) == 0x2040)
		result.s.speed = 10000;
	/* PMA Status 1 (1x0001) */
	phy_status = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 1, 0xa);
	if (phy_status < 0)
		return result;

	result.s.full_duplex = 1;
	if ((phy_status & 1) == 0)
		return result;
	phy_status = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 4, 0x18);
	if (phy_status < 0)
		return result;
	result.s.link_up = (phy_status & 0x1000) ? 1 : 0;

	return result;
}

static cvmx_helper_link_info_t
__cvmx_get_cortina_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;

	result.s.link_up = 1;
	result.s.full_duplex = 1;
	result.s.speed = 1000;
	return result;
}

static cvmx_helper_link_info_t
__get_vitesse_vsc8490_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;

	result.s.link_up = 1;
	result.s.full_duplex = 1;
	result.s.speed = 1000;
	return result;
}

static cvmx_helper_link_info_t
__get_aquantia_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;

	result.s.link_up = 1;
	result.s.full_duplex = 1;
	result.s.speed = 1000;
	return result;
}

static int __cvmx_helper_78xx_parse_phy(struct cvmx_phy_info *phy_info,
					int ipd_port)
{
	const void *fdt_addr = CASTPTR(const void *, gd->fdt_blob);
	const char *compat;
	int phy;
	int parent;
	u64 mdio_base;
	int node, bus;
	int phy_addr;
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	int compat_len = 0;

	debug("%s(0x%p, %d) ENTER\n", __func__, phy_info, ipd_port);

	phy = cvmx_helper_get_phy_fdt_node_offset(xiface, index);
	debug("%s: xiface: 0x%x, index: %d, ipd_port: %d, phy fdt offset: %d\n",
	      __func__, xiface, index, ipd_port, phy);
	if (phy < 0) {
		/* If this is the first time through we need to first parse the
		 * device tree to get the node offsets.
		 */
		debug("No config present, calling __cvmx_helper_parse_bgx_dt\n");
		if (__cvmx_helper_parse_bgx_dt(fdt_addr)) {
			printf("Error: could not parse BGX device tree\n");
			return -1;
		}
		if (__cvmx_fdt_parse_vsc7224(fdt_addr)) {
			debug("Error: could not parse Microsemi VSC7224 in DT\n");
			return -1;
		}
		if (octeon_has_feature(OCTEON_FEATURE_BGX_XCV) &&
		    __cvmx_helper_parse_bgx_rgmii_dt(fdt_addr)) {
			printf("Error: could not parse BGX XCV device tree\n");
			return -1;
		}
		phy = cvmx_helper_get_phy_fdt_node_offset(xiface, index);
		if (phy < 0) {
			debug("%s: Could not get PHY node offset for IPD port 0x%x, xiface: 0x%x, index: %d\n",
			      __func__, ipd_port, xiface, index);
			return -1;
		}
		debug("%s: phy: %d (%s)\n", __func__, phy,
		      fdt_get_name(fdt_addr, phy, NULL));
	}

	compat = (const char *)fdt_getprop(fdt_addr, phy, "compatible",
					   &compat_len);
	if (!compat) {
		printf("ERROR: %d:%d:no compatible prop in phy\n", xiface,
		       index);
		return -1;
	}

	debug("  compatible: %s\n", compat);

	phy_info->fdt_offset = phy;
	phy_addr = cvmx_fdt_get_int(fdt_addr, phy, "reg", -1);
	if (phy_addr == -1) {
		printf("Error: %d:%d:could not get PHY address\n", xiface,
		       index);
		return -1;
	}
	debug("  PHY address: %d, compat: %s\n", phy_addr, compat);

	if (!memcmp("marvell", compat, strlen("marvell"))) {
		phy_info->phy_type = MARVELL_GENERIC_PHY;
		phy_info->link_function = __get_marvell_phy_link_state;
	} else if (!memcmp("broadcom", compat, strlen("broadcom"))) {
		phy_info->phy_type = BROADCOM_GENERIC_PHY;
		phy_info->link_function = __get_broadcom_phy_link_state;
	} else if (!memcmp("cortina", compat, strlen("cortina"))) {
		phy_info->phy_type = CORTINA_PHY;
		phy_info->link_function = __cvmx_get_cortina_phy_link_state;
	} else if (!strcmp("vitesse,vsc8490", compat)) {
		phy_info->phy_type = VITESSE_VSC8490_PHY;
		phy_info->link_function = __get_vitesse_vsc8490_phy_link_state;
	} else if (fdt_stringlist_contains(compat, compat_len,
					   "ethernet-phy-ieee802.3-c22")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
		phy_info->link_function =
			__cvmx_get_generic_8023_c22_phy_link_state;
	} else if (fdt_stringlist_contains(compat, compat_len,
					   "ethernet-phy-ieee802.3-c45")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
		phy_info->link_function = __get_generic_8023_c45_phy_link_state;
	}

	phy_info->ipd_port = ipd_port;
	phy_info->phy_sub_addr = 0;
	phy_info->direct_connect = 1;

	parent = fdt_parent_offset(fdt_addr, phy);
	if (!fdt_node_check_compatible(fdt_addr, parent,
				       "ethernet-phy-nexus")) {
		debug("  nexus PHY found\n");
		if (phy_info->phy_type == CORTINA_PHY) {
			/* The Cortina CS422X uses the same PHY device for
			 * multiple ports for XFI.  In this case we use a
			 * nexus and each PHY address is the slice or
			 * sub-address and the actual PHY address is the
			 * nexus address.
			 */
			phy_info->phy_sub_addr = phy_addr;
			phy_addr =
				cvmx_fdt_get_int(fdt_addr, parent, "reg", -1);
			debug("  Cortina PHY real address: 0x%x\n", phy_addr);
		}
		parent = fdt_parent_offset(fdt_addr, parent);
	}

	debug("  Parent: %s\n", fdt_get_name(fdt_addr, parent, NULL));
	if (!fdt_node_check_compatible(fdt_addr, parent,
				       "cavium,octeon-3860-mdio")) {
		debug("  Found Octeon MDIO\n");
		mdio_base = cvmx_fdt_get_uint64(fdt_addr, parent, "reg",
						FDT_ADDR_T_NONE);
		debug("  MDIO address: 0x%llx\n",
		      (unsigned long long)mdio_base);

		mdio_base = cvmx_fdt_translate_address(fdt_addr, parent,
						       (u32 *)&mdio_base);
		debug("  Translated: 0x%llx\n", (unsigned long long)mdio_base);
		if (mdio_base == FDT_ADDR_T_NONE) {
			printf("Could not get MDIO base address from reg field\n");
			return -1;
		}
		__cvmx_mdio_addr_to_node_bus(mdio_base, &node, &bus);
		if (bus < 0) {
			printf("Invalid MDIO address 0x%llx, could not detect bus and node\n",
			       (unsigned long long)mdio_base);
			return -1;
		}
		debug("  MDIO node: %d, bus: %d\n", node, bus);
		phy_info->mdio_unit = (node << 2) | (bus & 3);
		phy_info->phy_addr = phy_addr | (phy_info->mdio_unit << 8);
	} else {
		printf("%s: Error: incompatible MDIO bus %s for IPD port %d\n",
		       __func__,
		       (const char *)fdt_get_name(fdt_addr, parent, NULL),
		       ipd_port);
		return -1;
	}

	debug("%s: EXIT 0\n", __func__);

	return 0;
}

/**
 * Return the MII PHY address associated with the given IPD
 * port. The phy address is obtained from the device tree.
 *
 * @param[out] phy_info - PHY information data structure updated
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * @return MII PHY address and bus number, -1 on error, -2 if PHY info missing (OK).
 */
static int __get_phy_info_from_dt(cvmx_phy_info_t *phy_info, int ipd_port)
{
	const void *fdt_addr = CASTPTR(const void *, gd->fdt_blob);
	int aliases, eth, phy, phy_parent, ret, i;
	int mdio_parent;
	const char *phy_compatible_str;
	const char *host_mode_str = NULL;
	int interface;
	int phy_addr_offset = 0;

	debug("%s(%p, %d)\n", __func__, phy_info, ipd_port);

	if (octeon_has_feature(OCTEON_FEATURE_BGX))
		return __cvmx_helper_78xx_parse_phy(phy_info, ipd_port);

	phy_info->phy_addr = -1;
	phy_info->phy_sub_addr = 0;
	phy_info->ipd_port = ipd_port;
	phy_info->direct_connect = -1;
	phy_info->phy_type = (cvmx_phy_type_t)-1;
	for (i = 0; i < CVMX_PHY_MUX_MAX_GPIO; i++)
		phy_info->gpio[i] = -1;
	phy_info->mdio_unit = -1;
	phy_info->gpio_value = -1;
	phy_info->gpio_parent_mux_twsi = -1;
	phy_info->gpio_parent_mux_select = -1;
	phy_info->link_function = NULL;
	phy_info->fdt_offset = -1;
	if (!fdt_addr) {
		debug("No device tree found.\n");
		return -1;
	}

	aliases = fdt_path_offset(fdt_addr, "/aliases");
	if (aliases < 0) {
		debug("Error: No /aliases node in device tree.\n");
		return -1;
	}
	if (ipd_port < 0) {
		int interface_index =
			ipd_port - CVMX_HELPER_BOARD_MGMT_IPD_PORT;
		eth = __mix_eth_node(fdt_addr, aliases, interface_index);
	} else {
		eth = __pip_eth_node(fdt_addr, aliases, ipd_port);
	}
	if (eth < 0) {
		debug("ERROR : cannot find interface for ipd_port=%d\n",
		      ipd_port);
		return -1;
	}

	interface = cvmx_helper_get_interface_num(ipd_port);
	/* Get handle to phy */
	phy = cvmx_fdt_lookup_phandle(fdt_addr, eth, "phy-handle");
	if (phy < 0) {
		cvmx_helper_interface_mode_t if_mode;

		/* Note that it's OK for RXAUI and ILK to not have a PHY
		 * connected (i.e. EBB boards in loopback).
		 */
		debug("Cannot get phy-handle for ipd_port: %d\n", ipd_port);
		if_mode = cvmx_helper_interface_get_mode(interface);
		if (if_mode != CVMX_HELPER_INTERFACE_MODE_RXAUI &&
		    if_mode != CVMX_HELPER_INTERFACE_MODE_ILK) {
			debug("ERROR : phy handle not found in device tree ipd_port=%d\n",
			      ipd_port);
			return -1;
		} else {
			return -2;
		}
	}

	phy_compatible_str =
		(const char *)fdt_getprop(fdt_addr, phy, "compatible", NULL);
	if (!phy_compatible_str) {
		debug("ERROR: no compatible prop in phy\n");
		return -1;
	}
	debug("Checking compatible string \"%s\" for ipd port %d\n",
	      phy_compatible_str, ipd_port);
	phy_info->fdt_offset = phy;
	if (!memcmp("marvell", phy_compatible_str, strlen("marvell"))) {
		debug("Marvell PHY detected for ipd_port %d\n", ipd_port);
		phy_info->phy_type = MARVELL_GENERIC_PHY;
		phy_info->link_function = __get_marvell_phy_link_state;
	} else if (!memcmp("broadcom", phy_compatible_str,
			   strlen("broadcom"))) {
		phy_info->phy_type = BROADCOM_GENERIC_PHY;
		phy_info->link_function = __get_broadcom_phy_link_state;
		debug("Broadcom PHY detected for ipd_port %d\n", ipd_port);
	} else if (!memcmp("vitesse", phy_compatible_str, strlen("vitesse"))) {
		debug("Vitesse PHY detected for ipd_port %d\n", ipd_port);
		if (!fdt_node_check_compatible(fdt_addr, phy,
					       "vitesse,vsc8490")) {
			phy_info->phy_type = VITESSE_VSC8490_PHY;
			debug("Vitesse VSC8490 detected\n");
			phy_info->link_function =
				__get_vitesse_vsc8490_phy_link_state;
		} else if (!fdt_node_check_compatible(
				   fdt_addr, phy,
				   "ethernet-phy-ieee802.3-c22")) {
			phy_info->phy_type = GENERIC_8023_C22_PHY;
			phy_info->link_function =
				__cvmx_get_generic_8023_c22_phy_link_state;
			debug("Vitesse 802.3 c22 detected\n");
		} else {
			phy_info->phy_type = GENERIC_8023_C45_PHY;
			phy_info->link_function =
				__get_generic_8023_c45_phy_link_state;
			debug("Vitesse 802.3 c45 detected\n");
		}
	} else if (!memcmp("aquantia", phy_compatible_str,
			   strlen("aquantia"))) {
		phy_info->phy_type = AQUANTIA_PHY;
		phy_info->link_function = __get_aquantia_phy_link_state;
		debug("Aquantia c45 PHY detected\n");
	} else if (!memcmp("cortina", phy_compatible_str, strlen("cortina"))) {
		phy_info->phy_type = CORTINA_PHY;
		phy_info->link_function = __cvmx_get_cortina_phy_link_state;
		host_mode_str = (const char *)fdt_getprop(
			fdt_addr, phy, "cortina,host-mode", NULL);
		debug("Cortina PHY detected for ipd_port %d\n", ipd_port);
	} else if (!memcmp("ti", phy_compatible_str, strlen("ti"))) {
		phy_info->phy_type = GENERIC_8023_C45_PHY;
		phy_info->link_function = __get_generic_8023_c45_phy_link_state;
		debug("TI PHY detected for ipd_port %d\n", ipd_port);
	} else if (!fdt_node_check_compatible(fdt_addr, phy,
					      "atheros,ar8334") ||
		   !fdt_node_check_compatible(fdt_addr, phy,
					      "qualcomm,qca8334") ||
		   !fdt_node_check_compatible(fdt_addr, phy,
					      "atheros,ar8337") ||
		   !fdt_node_check_compatible(fdt_addr, phy,
					      "qualcomm,qca8337")) {
		phy_info->phy_type = QUALCOMM_S17;
		phy_info->link_function =
			__cvmx_get_qualcomm_s17_phy_link_state;
		debug("Qualcomm QCA833X switch detected\n");
	} else if (!fdt_node_check_compatible(fdt_addr, phy,
					      "ethernet-phy-ieee802.3-c22")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
		phy_info->link_function =
			__cvmx_get_generic_8023_c22_phy_link_state;
		debug("Generic 802.3 c22 PHY detected\n");
	} else if (!fdt_node_check_compatible(fdt_addr, phy,
					      "ethernet-phy-ieee802.3-c45")) {
		phy_info->phy_type = GENERIC_8023_C45_PHY;
		phy_info->link_function = __get_generic_8023_c45_phy_link_state;
		debug("Generic 802.3 c45 PHY detected\n");
	} else {
		debug("Unknown PHY compatibility\n");
		phy_info->phy_type = (cvmx_phy_type_t)-1;
		phy_info->link_function = NULL;
	}

	phy_info->host_mode = CVMX_PHY_HOST_MODE_UNKNOWN;
	if (host_mode_str) {
		if (strcmp(host_mode_str, "rxaui") == 0)
			phy_info->host_mode = CVMX_PHY_HOST_MODE_RXAUI;
		else if (strcmp(host_mode_str, "xaui") == 0)
			phy_info->host_mode = CVMX_PHY_HOST_MODE_XAUI;
		else if (strcmp(host_mode_str, "sgmii") == 0)
			phy_info->host_mode = CVMX_PHY_HOST_MODE_SGMII;
		else if (strcmp(host_mode_str, "qsgmii") == 0)
			phy_info->host_mode = CVMX_PHY_HOST_MODE_QSGMII;
		else
			debug("Unknown PHY host mode\n");
	}

	/* Check if PHY parent is the octeon MDIO bus. Some boards are connected
	 * though a MUX and for them direct_connect_to_phy will be 0
	 */
	phy_parent = fdt_parent_offset(fdt_addr, phy);
	if (phy_parent < 0) {
		debug("ERROR : cannot find phy parent for ipd_port=%d ret=%d\n",
		      ipd_port, phy_parent);
		return -1;
	}
	/* For multi-phy devices and devices on a MUX, go to the parent */
	ret = fdt_node_check_compatible(fdt_addr, phy_parent,
					"ethernet-phy-nexus");
	if (ret == 0) {
		/* It's a nexus so check the grandparent. */
		phy_addr_offset =
			cvmx_fdt_get_int(fdt_addr, phy_parent, "reg", 0);
		phy_parent = fdt_parent_offset(fdt_addr, phy_parent);
	}

	/* Check for a muxed MDIO interface */
	mdio_parent = fdt_parent_offset(fdt_addr, phy_parent);
	ret = fdt_node_check_compatible(fdt_addr, mdio_parent,
					"cavium,mdio-mux");
	if (ret == 0) {
		ret = __get_muxed_mdio_info_from_dt(phy_info, phy_parent,
						    mdio_parent);
		if (ret) {
			printf("Error reading mdio mux information for ipd port %d\n",
			       ipd_port);
			return -1;
		}
	}
	ret = fdt_node_check_compatible(fdt_addr, phy_parent,
					"cavium,octeon-3860-mdio");
	if (ret == 0) {
		u32 *mdio_reg_base =
			(u32 *)fdt_getprop(fdt_addr, phy_parent, "reg", 0);
		phy_info->direct_connect = 1;
		if (mdio_reg_base == 0) {
			debug("ERROR : unable to get reg property in phy mdio\n");
			return -1;
		}
		phy_info->mdio_unit =
			__mdiobus_addr_to_unit(fdt32_to_cpu(mdio_reg_base[1]));
		debug("phy parent=%s reg_base=%08x mdio_unit=%d\n",
		      fdt_get_name(fdt_addr, phy_parent, NULL),
		      (int)mdio_reg_base[1], phy_info->mdio_unit);
	} else {
		phy_info->direct_connect = 0;
		/* The PHY is not directly connected to the Octeon MDIO bus.
		 * SE doesn't  have abstractions for MDIO MUX or MDIO MUX
		 * drivers and hence for the non direct cases code will be
		 * needed which is board specific.
		 * For now the MDIO Unit is defaulted to 1.
		 */
		debug("%s PHY at address: %d is not directly connected\n",
		      __func__, phy_info->phy_addr);
	}

	phy_info->phy_addr = cvmx_fdt_get_int(fdt_addr, phy, "reg", -1);
	if (phy_info->phy_addr < 0) {
		debug("ERROR: Could not read phy address from reg in DT\n");
		return -1;
	}
	phy_info->phy_addr += phy_addr_offset;
	phy_info->phy_addr |= phy_info->mdio_unit << 8;
	debug("%s(%p, %d) => 0x%x\n", __func__, phy_info, ipd_port,
	      phy_info->phy_addr);
	return phy_info->phy_addr;
}

/**
 * @INTERNAL
 * Parse the device tree and set whether a port is valid or not.
 *
 * @param fdt_addr	Pointer to device tree
 *
 * @return 0 for success, -1 on error.
 */
int __cvmx_helper_parse_bgx_dt(const void *fdt_addr)
{
	int port_index;
	struct cvmx_xiface xi;
	int fdt_port_node = -1;
	int fdt_interface_node;
	int fdt_phy_node;
	u64 reg_addr;
	int xiface;
	struct cvmx_phy_info *phy_info;
	static bool parsed;
	int err;
	int ipd_port;

	if (parsed) {
		debug("%s: Already parsed\n", __func__);
		return 0;
	}
	while ((fdt_port_node = fdt_node_offset_by_compatible(
			fdt_addr, fdt_port_node,
			"cavium,octeon-7890-bgx-port")) >= 0) {
		/* Get the port number */
		port_index =
			cvmx_fdt_get_int(fdt_addr, fdt_port_node, "reg", -1);
		if (port_index < 0) {
			debug("Error: missing reg field for bgx port in device tree\n");
			return -1;
		}
		debug("%s: Parsing BGX port %d\n", __func__, port_index);
		/* Get the interface number */
		fdt_interface_node = fdt_parent_offset(fdt_addr, fdt_port_node);
		if (fdt_interface_node < 0) {
			debug("Error: device tree corrupt!\n");
			return -1;
		}
		if (fdt_node_check_compatible(fdt_addr, fdt_interface_node,
					      "cavium,octeon-7890-bgx")) {
			debug("Error: incompatible Ethernet MAC Nexus in device tree!\n");
			return -1;
		}
		reg_addr =
			cvmx_fdt_get_addr(fdt_addr, fdt_interface_node, "reg");
		debug("%s: BGX interface address: 0x%llx\n", __func__,
		      (unsigned long long)reg_addr);
		if (reg_addr == FDT_ADDR_T_NONE) {
			debug("Device tree BGX node has invalid address 0x%llx\n",
			      (unsigned long long)reg_addr);
			return -1;
		}
		reg_addr = cvmx_fdt_translate_address(fdt_addr,
						      fdt_interface_node,
						      (u32 *)&reg_addr);
		xi = __cvmx_bgx_reg_addr_to_xiface(reg_addr);
		if (xi.node < 0) {
			debug("Device tree BGX node has invalid address 0x%llx\n",
			      (unsigned long long)reg_addr);
			return -1;
		}
		debug("%s: Found BGX node %d, interface %d\n", __func__,
		      xi.node, xi.interface);
		xiface = cvmx_helper_node_interface_to_xiface(xi.node,
							      xi.interface);
		cvmx_helper_set_port_fdt_node_offset(xiface, port_index,
						     fdt_port_node);
		cvmx_helper_set_port_valid(xiface, port_index, true);

		cvmx_helper_set_port_fdt_node_offset(xiface, port_index,
						     fdt_port_node);
		if (fdt_getprop(fdt_addr, fdt_port_node,
				"cavium,sgmii-mac-phy-mode", NULL))
			cvmx_helper_set_mac_phy_mode(xiface, port_index, true);
		else
			cvmx_helper_set_mac_phy_mode(xiface, port_index, false);

		if (fdt_getprop(fdt_addr, fdt_port_node, "cavium,force-link-up",
				NULL))
			cvmx_helper_set_port_force_link_up(xiface, port_index,
							   true);
		else
			cvmx_helper_set_port_force_link_up(xiface, port_index,
							   false);

		if (fdt_getprop(fdt_addr, fdt_port_node,
				"cavium,sgmii-mac-1000x-mode", NULL))
			cvmx_helper_set_1000x_mode(xiface, port_index, true);
		else
			cvmx_helper_set_1000x_mode(xiface, port_index, false);

		if (fdt_getprop(fdt_addr, fdt_port_node,
				"cavium,disable-autonegotiation", NULL))
			cvmx_helper_set_port_autonegotiation(xiface, port_index,
							     false);
		else
			cvmx_helper_set_port_autonegotiation(xiface, port_index,
							     true);

		fdt_phy_node = cvmx_fdt_lookup_phandle(fdt_addr, fdt_port_node,
						       "phy-handle");
		if (fdt_phy_node >= 0) {
			cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
							    fdt_phy_node);
			debug("%s: Setting PHY fdt node offset for interface 0x%x, port %d to %d\n",
			      __func__, xiface, port_index, fdt_phy_node);
			debug("%s: PHY node name: %s\n", __func__,
			      fdt_get_name(fdt_addr, fdt_phy_node, NULL));
			cvmx_helper_set_port_phy_present(xiface, port_index,
							 true);
			ipd_port = cvmx_helper_get_ipd_port(xiface, port_index);
			if (ipd_port >= 0) {
				debug("%s: Allocating phy info for 0x%x:%d\n",
				      __func__, xiface, port_index);
				phy_info =
					(cvmx_phy_info_t *)cvmx_bootmem_alloc(
						sizeof(*phy_info), 0);
				if (!phy_info) {
					debug("%s: Out of memory\n", __func__);
					return -1;
				}
				memset(phy_info, 0, sizeof(*phy_info));
				phy_info->phy_addr = -1;
				err = __get_phy_info_from_dt(phy_info,
							     ipd_port);
				if (err) {
					debug("%s: Error parsing phy info for ipd port %d\n",
					      __func__, ipd_port);
					return -1;
				}
				cvmx_helper_set_port_phy_info(
					xiface, port_index, phy_info);
				debug("%s: Saved phy info\n", __func__);
			}
		} else {
			cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
							    -1);
			debug("%s: No PHY fdt node offset for interface 0x%x, port %d to %d\n",
			      __func__, xiface, port_index, fdt_phy_node);
			cvmx_helper_set_port_phy_present(xiface, port_index,
							 false);
		}
	}
	if (!sfp_parsed)
		if (cvmx_sfp_parse_device_tree(fdt_addr))
			debug("%s: Error parsing SFP device tree\n", __func__);
	parsed = true;
	return 0;
}

int __cvmx_helper_parse_bgx_rgmii_dt(const void *fdt_addr)
{
	u64 reg_addr;
	struct cvmx_xiface xi;
	int fdt_port_node = -1;
	int fdt_interface_node;
	int fdt_phy_node;
	int port_index;
	int xiface;

	/* There's only one xcv (RGMII) interface, so just search for the one
	 * that's part of a BGX entry.
	 */
	while ((fdt_port_node = fdt_node_offset_by_compatible(
			fdt_addr, fdt_port_node, "cavium,octeon-7360-xcv")) >=
	       0) {
		fdt_interface_node = fdt_parent_offset(fdt_addr, fdt_port_node);
		if (fdt_interface_node < 0) {
			printf("Error: device tree corrupt!\n");
			return -1;
		}
		debug("%s: XCV parent node compatible: %s\n", __func__,
		      (char *)fdt_getprop(fdt_addr, fdt_interface_node,
					  "compatible", NULL));
		if (!fdt_node_check_compatible(fdt_addr, fdt_interface_node,
					       "cavium,octeon-7890-bgx"))
			break;
	}
	if (fdt_port_node == -FDT_ERR_NOTFOUND) {
		debug("No XCV/RGMII interface found in device tree\n");
		return 0;
	} else if (fdt_port_node < 0) {
		debug("%s: Error %d parsing device tree\n", __func__,
		      fdt_port_node);
		return -1;
	}
	port_index = cvmx_fdt_get_int(fdt_addr, fdt_port_node, "reg", -1);
	if (port_index != 0) {
		printf("%s: Error: port index (reg) must be 0, not %d.\n",
		       __func__, port_index);
		return -1;
	}
	reg_addr = cvmx_fdt_get_addr(fdt_addr, fdt_interface_node, "reg");
	if (reg_addr == FDT_ADDR_T_NONE) {
		printf("%s: Error: could not get BGX interface address\n",
		       __func__);
		return -1;
	}
	/* We don't have to bother translating since only 78xx supports OCX and
	 * doesn't support RGMII.
	 */
	xi = __cvmx_bgx_reg_addr_to_xiface(reg_addr);
	debug("%s: xi.node: %d, xi.interface: 0x%x, addr: 0x%llx\n", __func__,
	      xi.node, xi.interface, (unsigned long long)reg_addr);
	if (xi.node < 0) {
		printf("%s: Device tree BGX node has invalid address 0x%llx\n",
		       __func__, (unsigned long long)reg_addr);
		return -1;
	}
	debug("%s: Found XCV (RGMII) interface on interface %d\n", __func__,
	      xi.interface);
	debug("  phy handle: 0x%x\n",
	      cvmx_fdt_get_int(fdt_addr, fdt_port_node, "phy-handle", -1));
	fdt_phy_node =
		cvmx_fdt_lookup_phandle(fdt_addr, fdt_port_node, "phy-handle");
	debug("%s: phy-handle node: 0x%x\n", __func__, fdt_phy_node);
	xiface = cvmx_helper_node_interface_to_xiface(xi.node, xi.interface);

	cvmx_helper_set_port_fdt_node_offset(xiface, port_index, fdt_port_node);
	if (fdt_phy_node >= 0) {
		debug("%s: Setting PHY fdt node offset for interface 0x%x, port %d to %d\n",
		      __func__, xiface, port_index, fdt_phy_node);
		debug("%s: PHY node name: %s\n", __func__,
		      fdt_get_name(fdt_addr, fdt_phy_node, NULL));
		cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
						    fdt_phy_node);
		cvmx_helper_set_port_phy_present(xiface, port_index, true);
	} else {
		cvmx_helper_set_phy_fdt_node_offset(xiface, port_index, -1);
		debug("%s: No PHY fdt node offset for interface 0x%x, port %d to %d\n",
		      __func__, xiface, port_index, fdt_phy_node);
		cvmx_helper_set_port_phy_present(xiface, port_index, false);
	}

	return 0;
}

/**
 * Returns if a port is present on an interface
 *
 * @param fdt_addr - address fo flat device tree
 * @param ipd_port - IPD port number
 *
 * @return 1 if port is present, 0 if not present, -1 if error
 */
int __cvmx_helper_board_get_port_from_dt(void *fdt_addr, int ipd_port)
{
	int port_index;
	int aliases;
	const char *pip_path;
	char name_buffer[24];
	int pip, iface, eth;
	cvmx_helper_interface_mode_t mode;
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	u32 val;
	int phy_node_offset;
	int parse_bgx_dt_err;
	int parse_vsc7224_err;

	debug("%s(%p, %d)\n", __func__, fdt_addr, ipd_port);
	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		static int fdt_ports_initialized;

		port_index = cvmx_helper_get_interface_index_num(ipd_port);

		if (!fdt_ports_initialized) {
			if (octeon_has_feature(OCTEON_FEATURE_BGX_XCV)) {
				if (!__cvmx_helper_parse_bgx_rgmii_dt(fdt_addr))
					fdt_ports_initialized = 1;
				parse_bgx_dt_err =
					__cvmx_helper_parse_bgx_dt(fdt_addr);
				parse_vsc7224_err =
					__cvmx_fdt_parse_vsc7224(fdt_addr);
				if (!parse_bgx_dt_err && !parse_vsc7224_err)
					fdt_ports_initialized = 1;
			} else {
				debug("%s: Error parsing FDT\n", __func__);
				return -1;
			}
		}

		return cvmx_helper_is_port_valid(xiface, port_index);
	}

	mode = cvmx_helper_interface_get_mode(xiface);

	switch (mode) {
	/* Device tree has information about the following mode types. */
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
	case CVMX_HELPER_INTERFACE_MODE_SPI:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_AGL:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
		aliases = 1;
		break;
	default:
		aliases = 0;
		break;
	}

	/* The device tree information is present on interfaces that have phy */
	if (!aliases)
		return 1;

	port_index = cvmx_helper_get_interface_index_num(ipd_port);

	aliases = fdt_path_offset(fdt_addr, "/aliases");
	if (aliases < 0) {
		debug("%s: ERROR: /aliases not found in device tree fdt_addr=%p\n",
		      __func__, fdt_addr);
		return -1;
	}

	pip_path = (const char *)fdt_getprop(fdt_addr, aliases, "pip", NULL);
	if (!pip_path) {
		debug("%s: ERROR: interface %x pip path not found in device tree\n",
		      __func__, xiface);
		return -1;
	}
	pip = fdt_path_offset(fdt_addr, pip_path);
	if (pip < 0) {
		debug("%s: ERROR: interface %x pip not found in device tree\n",
		      __func__, xiface);
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "interface@%d",
		 xi.interface);
	iface = fdt_subnode_offset(fdt_addr, pip, name_buffer);
	if (iface < 0)
		return 0;
	snprintf(name_buffer, sizeof(name_buffer), "ethernet@%x", port_index);
	eth = fdt_subnode_offset(fdt_addr, iface, name_buffer);
	debug("%s: eth subnode offset %d from %s\n", __func__, eth,
	      name_buffer);

	if (eth < 0)
		return -1;

	cvmx_helper_set_port_fdt_node_offset(xiface, port_index, eth);

	phy_node_offset = cvmx_fdt_get_int(fdt_addr, eth, "phy", -1);
	cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
					    phy_node_offset);

	if (fdt_getprop(fdt_addr, eth, "cavium,sgmii-mac-phy-mode", NULL))
		cvmx_helper_set_mac_phy_mode(xiface, port_index, true);
	else
		cvmx_helper_set_mac_phy_mode(xiface, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,force-link-up", NULL))
		cvmx_helper_set_port_force_link_up(xiface, port_index, true);
	else
		cvmx_helper_set_port_force_link_up(xiface, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,sgmii-mac-1000x-mode", NULL))
		cvmx_helper_set_1000x_mode(xiface, port_index, true);
	else
		cvmx_helper_set_1000x_mode(xiface, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,disable-autonegotiation", NULL))
		cvmx_helper_set_port_autonegotiation(xiface, port_index, false);
	else
		cvmx_helper_set_port_autonegotiation(xiface, port_index, true);

	if (mode == CVMX_HELPER_INTERFACE_MODE_AGL) {
		bool tx_bypass = false;

		if (fdt_getprop(fdt_addr, eth, "cavium,rx-clk-delay-bypass",
				NULL))
			cvmx_helper_set_agl_rx_clock_delay_bypass(
				xiface, port_index, true);
		else
			cvmx_helper_set_agl_rx_clock_delay_bypass(
				xiface, port_index, false);

		val = cvmx_fdt_get_int(fdt_addr, eth, "cavium,rx-clk-skew", 0);
		cvmx_helper_set_agl_rx_clock_skew(xiface, port_index, val);

		if (fdt_getprop(fdt_addr, eth, "cavium,tx-clk-delay-bypass",
				NULL))
			tx_bypass = true;

		val = cvmx_fdt_get_int(fdt_addr, eth, "tx-clk-delay", 0);
		cvmx_helper_cfg_set_rgmii_tx_clk_delay(xiface, port_index,
						       tx_bypass, val);

		val = cvmx_fdt_get_int(fdt_addr, eth, "cavium,refclk-sel", 0);
		cvmx_helper_set_agl_refclk_sel(xiface, port_index, val);
	}

	return (eth >= 0);
}

/**
 * Given the address of the MDIO registers, output the CPU node and MDIO bus
 *
 * @param	addr	64-bit address of MDIO registers (from device tree)
 * @param[out]	node	CPU node number (78xx)
 * @param[out]	bus	MDIO bus number
 */
void __cvmx_mdio_addr_to_node_bus(u64 addr, int *node, int *bus)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if (node)
			*node = cvmx_csr_addr_to_node(addr);
		addr = cvmx_csr_addr_strip_node(addr);
	} else {
		if (node)
			*node = 0;
	}
	if (OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		switch (addr) {
		case 0x0001180000003800:
			*bus = 0;
			break;
		case 0x0001180000003880:
			*bus = 1;
			break;
		case 0x0001180000003900:
			*bus = 2;
			break;
		case 0x0001180000003980:
			*bus = 3;
			break;
		default:
			*bus = -1;
			printf("%s: Invalid SMI bus address 0x%llx\n", __func__,
			       (unsigned long long)addr);
			break;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		   OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		switch (addr) {
		case 0x0001180000003800:
			*bus = 0;
			break;
		case 0x0001180000003880:
			*bus = 1;
			break;
		default:
			*bus = -1;
			printf("%s: Invalid SMI bus address 0x%llx\n", __func__,
			       (unsigned long long)addr);
			break;
		}
	} else {
		switch (addr) {
		case 0x0001180000001800:
			*bus = 0;
			break;
		case 0x0001180000001900:
			*bus = 1;
			break;
		default:
			*bus = -1;
			printf("%s: Invalid SMI bus address 0x%llx\n", __func__,
			       (unsigned long long)addr);
			break;
		}
	}
}
