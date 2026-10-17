// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <env.h>
#include <log.h>
#include <i2c.h>
#include <net.h>
#include <dm/device.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/octeon_eth.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-gpio.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <asm/gpio.h>

#ifdef CONFIG_PCA953X
#include <pca953x.h>
#endif
#ifdef CONFIG_PCF857X
#include <pcf857x.h>
#endif
#ifdef CONFIG_PCA9554
#include <pca9554.h>
#endif
#ifdef CONFIG_PCA9555
#include <pca9555.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_PCA9554
static const char * const pca9554_gpio_list[] = {
	"pca9554",
	"nxp,pca9554",
	"ti,pca9554",
	NULL,
};
#endif

#ifdef CONFIG_PCA9555
static const char * const pca9555_gpio_list[] = {
	"pca9535",    "nxp,pca9535", "pca9539", "nxp,pca9539", "pca9555",
	"nxp,pca9555", "ti,pca9555", "max7312", "maxim,max7312", "max7313",
	"maxim,max7313", "tca6416", "tca9539",    NULL,
};
#endif

#ifdef CONFIG_PCA953X
/** List of compatible strings supported by pca953x driver */
static const char * const pca953x_gpio_list[] = {
	"nxp,pca9534", "nxp,pca9535", "nxp,pca9536", "nxp,pca9537", "nxp,pca9538", "nxp,pca9539",
	"nxp,pca953x", "nxp,pca9554", "nxp,pca9555", "nxp,pca9556", "nxp,pca9557", "nxp,pca6107",
	"pca9534",     "pca9535",     "pca9536",     "pca9537",	    "pca9538",	   "pca9539",
	"pca953x",     "pca9554",     "pca9555",     "pca9556",	    "pca9557",	   "max7310",
	"max7312",     "max7313",     "max7315",     "pca6107",	    "tca6408",	   "tca6416",
	"tca9555",     NULL
};
#endif

#ifdef CONFIG_PHY_VITESSE
static const char * const vitesse_vsc8488_gpio_list[] = {
	"vitesse,vsc8486",   "microsemi,vsc8486", "vitesse,vsc8488",
	"microsemi,vsc8488", "vitesse,vsc8489",	  "microsemi,vsc8489",
	"vitesse,vsc8490",   "microsemi,vsc8490", NULL
};
#endif

/** List of compatible strings supported by Octeon driver */
static const char * const octeon_gpio_list[] = {
	"cavium,octeon-7890-gpio",
	"cavium,octeon-3860-gpio",
	NULL
};

/**
 * Trims nodes from the flat device tree.
 *
 * @param fdt - pointer to working FDT, usually in gd->fdt_blob
 * @param fdt_key - key to preserve.  All non-matching keys are removed
 * @param trim_name - name of property to look for.  If NULL use
 *		      'cavium,qlm-trim'
 *
 * The key should look something like device #, type where device # is a
 * number from 0-9 and type is a string describing the type.  For QLM
 * operations this would typically contain the QLM number followed by
 * the type in the device tree, like "0,xaui", "0,sgmii", etc.  This function
 * will trim all items in the device tree which match the device number but
 * have a type which does not match.  For example, if a QLM has a xaui module
 * installed on QLM 0 and "0,xaui" is passed as a key, then all FDT nodes that
 * have "0,xaui" will be preserved but all others, i.e. "0,sgmii" will be
 * removed.
 *
 * Note that the trim_name must also match.  If trim_name is NULL then it
 * looks for the property "cavium,qlm-trim".
 *
 * Also, when the trim_name is "cavium,qlm-trim" or NULL that the interfaces
 * will also be renamed based on their register values.
 *
 * For example, if a PIP interface is named "interface@W" and has the property
 * reg = <0> then the interface will be renamed after this function to
 * interface@0.
 *
 * Return: 0 for success.
 */
int __octeon_fdt_patch(void *fdt, const char *fdt_key, const char *trim_name)
{
	bool rename = !trim_name || !strcmp(trim_name, "cavium,qlm-trim");

	return octeon_fdt_patch_rename(fdt, fdt_key, trim_name, rename, NULL, NULL);
}

int octeon_fdt_patch(void *fdt, const char *fdt_key, const char *trim_name)
	__attribute__((weak, alias("__octeon_fdt_patch")));

/**
 * Trims nodes from the flat device tree.
 *
 * @param fdt - pointer to working FDT, usually in gd->fdt_blob
 * @param fdt_key - key to preserve.  All non-matching keys are removed
 * @param trim_name - name of property to look for.  If NULL use
 *		      'cavium,qlm-trim'
 * @param rename - set to TRUE to rename interfaces.
 * @param callback - function to call on matched nodes.
 * @param cbarg - passed to callback.
 *
 * The key should look something like device #, type where device # is a
 * number from 0-9 and type is a string describing the type.  For QLM
 * operations this would typically contain the QLM number followed by
 * the type in the device tree, like "0,xaui", "0,sgmii", etc.  This function
 * will trim all items in the device tree which match the device number but
 * have a type which does not match.  For example, if a QLM has a xaui module
 * installed on QLM 0 and "0,xaui" is passed as a key, then all FDT nodes that
 * have "0,xaui" will be preserved but all others, i.e. "0,sgmii" will be
 * removed.
 *
 * Note that the trim_name must also match.  If trim_name is NULL then it
 * looks for the property "cavium,qlm-trim".
 *
 * Also, when the trim_name is "cavium,qlm-trim" or NULL that the interfaces
 * will also be renamed based on their register values.
 *
 * For example, if a PIP interface is named "interface@W" and has the property
 * reg = <0> then the interface will be renamed after this function to
 * interface@0.
 *
 * Return: 0 for success.
 */
int octeon_fdt_patch_rename(void *fdt, const char *fdt_key,
			    const char *trim_name, bool rename,
			    void (*callback)(void *fdt, int offset, void *arg),
			    void *cbarg)
	__attribute__((weak, alias("__octeon_fdt_patch_rename")));

int __octeon_fdt_patch_rename(void *fdt, const char *fdt_key,
			      const char *trim_name, bool rename,
			      void (*callback)(void *fdt, int offset, void *arg),
			      void *cbarg)
{
	int fdt_key_len;
	int offset, next_offset;
	int aliases;
	const void *aprop;
	char qlm[32];
	char *mode;
	int qlm_key_len;
	int rc;
	int cpu_node;

	if (!trim_name)
		trim_name = "cavium,qlm-trim";

	strncpy(qlm, fdt_key, sizeof(qlm));
	mode = qlm;
	strsep(&mode, ",");
	qlm_key_len = strlen(qlm);

	debug("In %s: Patching FDT header at 0x%p with key \"%s\"\n", __func__, fdt, fdt_key);
	if (!fdt || fdt_check_header(fdt) != 0) {
		printf("%s: Invalid device tree\n", __func__);
		return -1;
	}

	fdt_key_len = strlen(fdt_key) + 1;

	/* Prune out the unwanted parts based on the QLM mode.  */
	offset = 0;
	for (offset = fdt_next_node(fdt, offset, NULL); offset >= 0; offset = next_offset) {
		int len;
		const char *val;
		const char *val_comma;

		next_offset = fdt_next_node(fdt, offset, NULL);

		val = fdt_getprop(fdt, offset, trim_name, &len);
		if (!val)
			continue;

		debug("fdt found trim name %s, comparing key \"%s\"(%d) with \"%s\"(%d)\n",
		      trim_name, fdt_key, fdt_key_len, val, len);
		val_comma = strchr(val, ',');
		if (!val_comma || (val_comma - val) != qlm_key_len)
			continue;
		if (strncmp(val, qlm, qlm_key_len) != 0)
			continue; /* Not this QLM. */

		debug("fdt key number \"%s\" matches\n", val);
		if (!fdt_stringlist_contains(val, len, fdt_key)) {
			debug("Key \"%s\" does not match \"%s\"\n", val, fdt_key);
			/* This QLM, but wrong mode.  Delete it. */
			/* See if there's an alias that needs deleting */
			val = fdt_getprop(fdt, offset, "cavium,qlm-trim-alias", NULL);
			if (val) {
				debug("Trimming alias \"%s\"\n", val);
				aliases = fdt_path_offset(fdt, "/aliases");
				if (aliases) {
					aprop = fdt_getprop(fdt, aliases, val, NULL);
					if (aprop) {
						rc = fdt_nop_property(fdt, aliases, val);
						if (rc) {
							printf("Error: Could not NOP alias %s in fdt\n",
							       val);
						}
					} else {
						printf("Error: could not find /aliases/%s in device tree\n",
						       val);
					}
				} else {
					puts("Error: could not find /aliases in device tree\n");
				}
			}
			debug("fdt trimming matching key %s\n", fdt_key);
			next_offset = fdt_parent_offset(fdt, offset);
			rc = fdt_nop_node(fdt, offset);
			if (rc)
				printf("Error %d noping node in device tree\n", rc);
		}
	}

	debug("%s: Starting pass 2 for key %s\n", __func__, fdt_key);
	/* Second pass: Rewrite names and remove key properties.  */
	offset = -1;
	for (offset = fdt_next_node(fdt, offset, NULL); offset >= 0; offset = next_offset) {
		int len;
		const char *val = fdt_getprop(fdt, offset, trim_name, &len);

		next_offset = fdt_next_node(fdt, offset, NULL);

		if (!val)
			continue;
		debug("Searching stringlist %s for %s\n", val, fdt_key);
		if (fdt_stringlist_contains(val, len, fdt_key)) {
			char new_name[64];
			const char *name;
			const char *at;
			int reg;

			debug("Found key %s at offset 0x%x\n", fdt_key, offset);
			fdt_nop_property(fdt, offset, trim_name);

			if (rename) {
				name = fdt_get_name(fdt, offset, NULL);
				debug("  name: %s\n", name);
				if (!name)
					continue;
				at = strchr(name, '@');
				if (!at)
					continue;

				reg = fdtdec_get_int(fdt, offset, "reg", -1);
				if (reg == -1)
					continue;

				debug("  reg: %d\n", reg);
				len = at - name + 1;
				debug("  len: %d\n", len);
				if (len + 9 >= sizeof(new_name))
					continue;

				memcpy(new_name, name, len);
				cpu_node = cvmx_fdt_get_cpu_node(fdt, offset);
				if (cpu_node > 0)
					snprintf(new_name + len, sizeof(new_name) - len, "%x_%x",
						 cpu_node, reg);
				else
					sprintf(new_name + len, "%x", reg);
				debug("Renaming cpu node %d %s to %s\n", cpu_node, name, new_name);
				fdt_set_name(fdt, offset, new_name);
			}
			if (callback)
				callback(fdt, offset, cbarg);

			/* Structure may have changed, start at the beginning. */
			next_offset = 0;
		}
	}

	return 0;
}

#ifdef CONFIG_CMD_NET
static void octeon_set_one_fdt_mac(int node, uint64_t *mac)
{
	u8 mac_addr[6];
	int r;

	mac_addr[5] = *mac & 0xff;
	mac_addr[4] = (*mac >> 8) & 0xff;
	mac_addr[3] = (*mac >> 16) & 0xff;
	mac_addr[2] = (*mac >> 24) & 0xff;
	mac_addr[1] = (*mac >> 32) & 0xff;
	mac_addr[0] = (*mac >> 40) & 0xff;

	r = fdt_setprop_inplace(working_fdt, node, "local-mac-address", mac_addr, 6);
	if (r == 0)
		*mac = *mac + 1;
}

static uint64_t convert_mac(const u8 mac_addr[6])
{
	int i;
	u64 mac = 0;

	for (i = 0; i < 6; i++)
		mac = (mac << 8) | mac_addr[i];
	return mac;
}

/**
 * Fix up the MAC address in the flat device tree based on the MAC address
 * stored in ethaddr or in the board descriptor.
 *
 * NOTE: This function is weak and an alias for __octeon_fixup_fdt_mac_addr.
 */
void octeon_fixup_fdt_mac_addr(void) __attribute__((weak, alias("__octeon_fixup_fdt_mac_addr")));

void __octeon_fixup_fdt_mac_addr(void)
{
	int node, pip, interface, ethernet;
	int i, e;
	u64 mac = 0;
	uchar mac_addr[6];
	char name[20];
	bool env_mac_addr_valid;
	const char *p;

	debug("%s: env ethaddr: %s\n", __func__, (p = env_get("ethaddr")) ? p : "not set");
	if (eth_env_get_enetaddr("ethaddr", mac_addr)) {
		mac = convert_mac(mac_addr);
		env_mac_addr_valid = true;
	} else {
		mac = convert_mac((uint8_t *)gd->arch.mac_desc.mac_addr_base);
		env_mac_addr_valid = false;
	}

	debug("%s: mac_addr: %pM, board mac: %pM, env valid: %s\n", __func__, mac_addr,
	      gd->arch.mac_desc.mac_addr_base, env_mac_addr_valid ? "true" : "false");

	if (env_mac_addr_valid && memcmp(mac_addr, (void *)gd->arch.mac_desc.mac_addr_base, 6))
		printf("Warning: the environment variable ethaddr is set to %pM\n"
		       "which does not match the board descriptor MAC address %pM.\n"
		       "Please clear the ethaddr environment variable with the command\n"
		       "\"setenv -f ethaddr; saveenv\" or change the board MAC address with the command\n"
		       "\"tlv_eeprom set mac %pM\" to change the board MAC address so that it matches\n"
		       "the environment address.\n"
		       "Note: the correct MAC address is usually the one stored in the tlv EEPROM.\n",
		       mac_addr, gd->arch.mac_desc.mac_addr_base, mac_addr);

	for (i = 0; i < 2; i++) {
		sprintf(name, "mix%x", i);
		p = fdt_get_alias(working_fdt, name);
		if (p) {
			node = fdt_path_offset(working_fdt, p);
			if (node > 0)
				octeon_set_one_fdt_mac(node, &mac);
		}
	}

	for (i = 0; i < 2; i++) {
		sprintf(name, "rgmii%x", i);
		p = fdt_get_alias(working_fdt, name);
		if (p) {
			node = fdt_path_offset(working_fdt, p);
			if (node > 0)
				octeon_set_one_fdt_mac(node, &mac);
		}
	}

	pip = fdt_node_offset_by_compatible(working_fdt, -1, "cavium,octeon-3860-pip");

	if (pip > 0)
		for (i = 0; i < 8; i++) {
			sprintf(name, "interface@%d", i);
			interface = fdt_subnode_offset(working_fdt, pip, name);
			if (interface <= 0)
				continue;
			for (e = 0; e < 16; e++) {
				sprintf(name, "ethernet@%d", e);
				ethernet = fdt_subnode_offset(working_fdt, interface, name);
				if (ethernet <= 0)
					continue;
				octeon_set_one_fdt_mac(ethernet, &mac);
			}
		}

	/* Assign 78XX addresses in the order they appear in the device tree. */
	fdt_for_each_node_by_compatible(node, working_fdt, -1, "cavium,octeon-7890-bgx-port")
		octeon_set_one_fdt_mac(node, &mac);
}
#endif

/**
 * This function fixes the clock-frequency in the flat device tree for the UART.
 *
 * NOTE: This function is weak and an alias for __octeon_fixup_fdt_uart.
 */
void octeon_fixup_fdt_uart(void) __attribute__((weak, alias("__octeon_fixup_fdt_uart")));

void __octeon_fixup_fdt_uart(void)
{
	u32 clk;
	int node;

	clk = gd->bus_clk;

	/* Device trees already have good values for fast simulator
	 * output, real boards need the correct value.
	 */
	fdt_for_each_node_by_compatible(node, working_fdt, -1, "cavium,octeon-3860-uart")
		fdt_setprop_inplace_cell(working_fdt, node, "clock-frequency", clk);
}

/**
 * This function fills in the /memory portion of the flat device tree.
 *
 * NOTE: This function is weak and aliased to __octeon_fixup_fdt_memory.
 */
void octeon_fixup_fdt_memory(void) __attribute__((weak, alias("__octeon_fixup_fdt_memory")));

void __octeon_fixup_fdt_memory(void)
{
	u64 sizes[3], addresses[3];
	u64 size_left = gd->ram_size;
	int num_addresses = 0;
	int rc;
	int node;

	size_left = gd->ram_size;
	sizes[num_addresses] = min_t(u64, size_left, 256 * 1024 * 1024);
	size_left -= sizes[num_addresses];
	addresses[num_addresses] = 0;
	num_addresses++;

	if (size_left > 0) {
		sizes[num_addresses] = size_left;
		addresses[num_addresses] = 0x20000000ULL;
		num_addresses++;
	}

	node = fdt_path_offset(working_fdt, "/memory");
	if (node < 0)
		node = fdt_add_subnode(working_fdt, fdt_path_offset(working_fdt, "/"), "memory");
	if (node < 0) {
		printf("Could not add memory section to fdt: %s\n", fdt_strerror(node));
		return;
	}
	rc = fdt_fixup_memory_banks(working_fdt, addresses, sizes, num_addresses);
	if (rc != 0)
		printf("%s: fdt_fixup_memory_banks returned %d when adding %d addresses\n",
		       __func__, rc, num_addresses);
}

void octeon_fixup_fdt(void) __attribute__((weak, alias("__octeon_fixup_fdt")));

void __octeon_fixup_fdt(void)
{
	if (!working_fdt)
		return;

#ifdef CONFIG_CMD_NET
	octeon_fixup_fdt_mac_addr();
#endif /* CONFIG_CMD_NET */

#if !CONFIG_OCTEON_SIM_SPEED
	octeon_fixup_fdt_uart();
#endif

	octeon_fixup_fdt_memory();
}

int __board_fixup_fdt(void)
{
	/*
	 * Nothing to do in this dummy implementation
	 */
	return 0;
}

int board_fixup_fdt(void) __attribute__((weak, alias("__board_fixup_fdt")));

/**
 * This is a helper function to find the offset of a PHY device given
 * an Ethernet device.
 *
 * @param[in] eth - Ethernet device to search for PHY offset
 *
 * @returns offset of phy info in device tree or -1 if not found
 */
//int octeon_fdt_find_phy(const struct eth_device *eth)
int octeon_fdt_find_phy(const struct udevice *eth)
{
	int aliases;
	const void *fdt = gd->fdt_blob;
	const char *pip_path;
	int pip;
	char buffer[64];
#if 0
	struct octeon_eth_info *oct_eth_info =
				 (struct octeon_eth_info *)eth->priv;
#else
	struct octeon_eth_info *oct_eth_info = dev_get_priv(eth);
#endif
	int interface, index;
	int phandle;
	int phy;
	u32 *phy_handle;

	aliases = fdt_path_offset(fdt, "/aliases");
	if (aliases < 0) {
		puts("/aliases not found in device tree!\n");
		return -1;
	}
	pip_path = fdt_getprop(fdt, aliases, "pip", NULL);
	if (!pip_path) {
		puts("pip not found in aliases in device tree\n");
		return -1;
	}
	pip = fdt_path_offset(fdt, pip_path);
	if (pip < 0) {
		puts("pip not found in device tree\n");
		return -1;
	}
	snprintf(buffer, sizeof(buffer), "interface@%d", oct_eth_info->interface);
	interface = fdt_subnode_offset(fdt, pip, buffer);
	if (interface < 0) {
		printf("%s: interface@%d not found in device tree for %s\n", __func__,
		       oct_eth_info->interface, eth->name);
		return -1;
	}
	snprintf(buffer, sizeof(buffer), "ethernet@%x", oct_eth_info->index);
	index = fdt_subnode_offset(fdt, interface, buffer);
	if (index < 0) {
		printf("%s: ethernet@%x not found in device tree for %s\n", __func__,
		       oct_eth_info->index, eth->name);
		return -1;
	}
	phy_handle = (uint32_t *)fdt_getprop(fdt, index, "phy-handle", NULL);
	if (phy_handle < 0) {
		printf("%s: phy-handle not found for %s\n", __func__, eth->name);
		return -1;
	}
	phandle = fdt32_to_cpu(*phy_handle);
	phy = fdt_node_offset_by_phandle(fdt, phandle);
	if (phy < 0) {
		printf("%s: phy not found for %s\n", __func__, eth->name);
		return -1;
	}

	return phy;
}

/**
 * This helper function returns if a node contains the specified vendor name.
 *
 * @param[in]	fdt		pointer to device tree blob
 * @param	nodeoffset	offset of the tree node
 * @param[in]	vendor		name of vendor to check
 *
 * returns:
 *	0, if the node has a compatible vendor string property
 *	1, if the node does not contain the vendor string property
 *	-FDT_ERR_NOTFOUND, if the given node has no 'compatible' property
 *	-FDT_ERR_BADOFFSET, if nodeoffset does not refer to a BEGIN_NODE tag
 *	-FDT_ERR_BADMAGIC,
 *	-FDT_ERR_BADVERSION,
 *	-FDT_BADSTATE,
 *	-FDT_ERR_BADSTRUCTURE, standard meanings
 */
int octeon_fdt_compat_vendor(const void *fdt, int nodeoffset, const char *vendor)
{
	const char *strlist;
	const char *p;
	int len;
	int listlen;

	strlist = fdt_getprop(fdt, nodeoffset, "compatible", &listlen);
	if (!strlist)
		return listlen;

	len = strlen(vendor);

	debug("%s(%p, %d, %s (%p)) strlist: %s (%p), len: %d\n", __func__, fdt, nodeoffset, vendor,
	      vendor, strlist, strlist, len);
	while (listlen >= len) {
		debug("  Comparing %d bytes of %s and %s\n", len, vendor, strlist);
		if ((memcmp(vendor, strlist, len) == 0) &&
		    ((strlist[len] == ',') || (strlist[len] == '\0')))
			return 0;
		p = memchr(strlist, '\0', listlen);
		if (!p)
			return 1; /* malformed strlist.. */
		listlen -= (p - strlist) + 1;
		strlist = p + 1;
	}
	return 1;
}

/**
 * Given a node in the device tree get the OCTEON OCX node number
 *
 * @param fdt		pointer to flat device tree
 * @param nodeoffset	node offset to get OCX node for
 *
 * Return: the Octeon OCX node number
 */
int octeon_fdt_get_soc_node(const void *fdt, int nodeoffset)
{
	return 0;
}

/**
 * Given a FDT node, check if it is compatible with a list of devices
 *
 * @param[in]	fdt		Flat device tree pointer
 * @param	node_offset	Node offset in device tree
 * @param[in]	strlist		Array of FDT devices to check, end must be NULL
 *
 * Return:	0 if at least one device is compatible, 1 if not compatible.
 */
int octeon_fdt_node_check_compatible(const void *fdt, int node_offset,
				     const char *const *strlist)
{
	while (*strlist && **strlist) {
		debug("%s: Checking %s\n", __func__, *strlist);
		if (!fdt_node_check_compatible(fdt, node_offset, *strlist)) {
			debug("%s: match found\n", __func__);
			return 0;
		}
		strlist++;
	}
	debug("%s: No match found\n", __func__);
	return 1;
}

/**
 * Given a node offset, find the i2c bus number for that node
 *
 * @param[in]	fdt	Pointer to flat device tree
 * @param	node_offset	Node offset in device tree
 *
 * Return:	i2c bus number or -1 if error
 */
int octeon_fdt_i2c_get_bus(const void *fdt, int node_offset)
{
	const char *compat;
	const u64 addresses[] = { 0x1180000001000, 0x1180000001200 };
	u64 reg;
	int i;
	int bus = -1;
	bool found = false;

	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		compat = "cavium,octeon-7890-twsi";
	else
		compat = "cavium,octeon-3860-twsi";

	while (node_offset > 0 &&
	       !(found = !fdt_node_check_compatible(fdt, node_offset, compat))) {
		node_offset = fdt_parent_offset(fdt, node_offset);
#ifdef CONFIG_OCTEON_I2C_FDT
		bus = i2c_get_bus_num_fdt(node_offset);
		if (bus >= 0) {
			debug("%s: Found bus 0x%x\n", __func__, bus);
			return bus;
		}
#endif
	}
	if (!found) {
		printf("Error: node %d in device tree is not a child of the I2C bus\n",
		       node_offset);
		return -1;
	}

	reg = fdtdec_get_addr(fdt, node_offset, "reg");
	if (reg == FDT_ADDR_T_NONE) {
		printf("%s: Error: invalid reg address for TWSI bus\n", __func__);
		return -1;
	}

	for (i = 0; i < ARRAY_SIZE(addresses); i++)
		if (reg == addresses[i]) {
			bus = i;
			break;
		}

	debug("%s: bus 0x%x\n", __func__, bus);
	return bus;
}

/**
 * Given an offset into the fdt, output the i2c bus and address of the device
 *
 * @param[in]	fdt	fdt blob pointer
 * @param	node	offset in FDT of device
 * @param[out]	bus	i2c bus number of device
 * @param[out]	addr	address of device on i2c bus
 *
 * Return:	0 for success, -1 on error
 */
int octeon_fdt_get_i2c_bus_addr(const void *fdt, int node, int *bus, int *addr)
{
	*bus = octeon_fdt_i2c_get_bus(fdt, fdt_parent_offset(fdt, node));
	if (*bus < 0) {
		printf("%s: Could not get parent i2c bus\n", __func__);
		return -1;
	}
	*addr = fdtdec_get_int(fdt, node, "reg", -1);
	if (*addr < 0)
		return -1;
	return 0;
}

/**
 * Reads a GPIO pin given the node of the GPIO device in the device tree and
 * the pin number.
 *
 * @param[in]	fdt	fdt blob pointer
 * @param	phandle	phandle of GPIO node
 * @param	pin	pin number to read
 *
 * Return:	0 = pin is low, 1 = pin is high, -1 = error
 */
int octeon_fdt_read_gpio(const void *fdt, int phandle, int pin)
{
	enum cvmx_gpio_type type;
	__maybe_unused int node;
	__maybe_unused int addr;
	__maybe_unused int bus;
	__maybe_unused int old_bus;
	int num_pins;
	int value;

	type = cvmx_fdt_get_gpio_type(fdt, phandle, &num_pins);
	if ((pin & 0xff) >= num_pins) {
		debug("%s: pin number %d out of range\n", __func__, pin);
		return -1;
	}
	switch (type) {
#ifdef CONFIG_PCA953X
	case CVMX_GPIO_PIN_PCA953X:
		node = fdt_node_offset_by_phandle(fdt, phandle);
		if (octeon_fdt_get_i2c_bus_addr(fdt, node, &bus, &addr)) {
			printf("%s: Could not get gpio bus and/or address\n", __func__);
			return -1;
		}
		value = pca953x_get_val(bus, addr);
		if (value < 0) {
			printf("%s: Error reading PCA953X GPIO at 0x%x:0x%x\n", __func__, bus,
			       addr);
			return -1;
		}
		value = (value >> pin) & 1;
		break;
#endif
#ifdef CONFIG_PCF857X
	case CVMX_GPIO_PIN_PCF857X:
		node = fdt_node_offset_by_phandle(fdt, phandle);
		if (octeon_fdt_get_i2c_bus_addr(fdt, node, &bus, &addr)) {
			printf("%s: Could not get gpio bus and/or address\n", __func__);
			return -1;
		}
		value = pcf857x_get_val(bus, addr);
		if (value < 0) {
			printf("%s: Error reading PCF857X GPIO at 0x%x:0x%x\n", __func__, bus,
			       addr);
			return -1;
		}
		value = (value >> pin) & 1;
		break;
#endif
	case CVMX_GPIO_PIN_OCTEON:
		value = gpio_get_value(pin);
		break;
	default:
		printf("%s: Unknown GPIO type %d\n", __func__, type);
		return -1;
	}
	return value;
}

/**
 * Reads a GPIO pin given the node of the GPIO device in the device tree and
 * the pin number.
 *
 * @param[in]	fdt	fdt blob pointer
 * @param	phandle	phandle of GPIO node
 * @param	pin	pin number to read
 * @param	val	value to write (1 = high, 0 = low)
 *
 * Return:	0 = success, -1 = error
 */
int octeon_fdt_set_gpio(const void *fdt, int phandle, int pin, int val)
{
	enum cvmx_gpio_type type;
	int node;
	int num_pins;
	__maybe_unused int addr;
	__maybe_unused int bus;
	__maybe_unused int old_bus;
	__maybe_unused int rc;

	node = fdt_node_offset_by_phandle(fdt, phandle);
	if (node < 0) {
		printf("%s: Invalid phandle\n", __func__);
		return -1;
	}

	type = cvmx_fdt_get_gpio_type(fdt, phandle, &num_pins);
	if ((pin & 0xff) >= num_pins) {
		debug("%s: pin number %d out of range\n", __func__, pin);
		return -1;
	}
	switch (type) {
#ifdef CONFIG_PCA953X
	case CVMX_GPIO_PIN_PCA953X:
		if (octeon_fdt_get_i2c_bus_addr(fdt, node, &bus, &addr)) {
			printf("%s: Could not get gpio bus and/or address\n", __func__);
			return -1;
		}

		return pca953x_set_val(bus, addr, 1 << pin, val << pin);
#endif
#ifdef CONFIG_PCF857X
	case CVMX_GPIO_PIN_PCF857X:
		if (octeon_fdt_get_i2c_bus_addr(fdt, node, &bus, &addr)) {
			printf("%s: Could not get gpio bus and/or address\n", __func__);
			return -1;
		}
		return pcf957x_set_val(bus, addr, 1 << pin, val << pin);
#endif
	case CVMX_GPIO_PIN_OCTEON:
		return gpio_set_value(pin, val);
	default:
		printf("%s: Unknown GPIO type %d\n", __func__, type);
		return -1;
	}
}

/**
 * Given the node of a GPIO entry output the GPIO type, i2c bus and i2c
 * address.
 *
 * @param	fdt_node	node of GPIO in device tree, generally
 *				derived from a phandle.
 * @param[out]	type		Type of GPIO detected
 * @param[out]	i2c_bus		For i2c GPIO expanders, the i2c bus number
 * @param[out]	i2c_addr	For i2c GPIO expanders, the i2c address
 *
 * Return:	0 for success, -1 for errors
 *
 * NOTE: It is up to the caller to determine the pin number.
 */
int octeon_fdt_get_gpio_info(int fdt_node, enum octeon_gpio_type *type,
			     int *i2c_bus, int *i2c_addr)
{
	const void *fdt = gd->fdt_blob;

	int i2c_bus_node __attribute__((unused));

	*type = GPIO_TYPE_UNKNOWN;

	if (!octeon_fdt_node_check_compatible(fdt, fdt_node, octeon_gpio_list)) {
		debug("%s: Found Octeon compatible GPIO\n", __func__);
		*type = GPIO_TYPE_OCTEON;
		if (i2c_bus)
			*i2c_bus = -1;
		if (i2c_addr)
			*i2c_addr = -1;
		return 0;
	}
#ifdef CONFIG_PCA9555
	if (!octeon_fdt_node_check_compatible(fdt, fdt_node, pca9555_gpio_list)) {
		debug("%s: Found PCA9555 type compatible GPIO\n", __func__);
		*type = GPIO_TYPE_PCA9555;
	}
#endif
#ifdef CONFIG_PCA9554
	if (!octeon_fdt_node_check_compatible(fdt, fdt_node, pca9554_gpio_list)) {
		debug("%s: Found PCA9555 type compatible GPIO\n", __func__);
		*type = GPIO_TYPE_PCA9554;
	}
#endif
#ifdef CONFIG_PCA953X
	if (!octeon_fdt_node_check_compatible(fdt, fdt_node, pca953x_gpio_list)) {
		debug("%s: Found PCA953x compatible GPIO", __func__);
		*type = GPIO_TYPE_PCA953X;
	}
#endif
#if defined(CONFIG_PCA953X) || \
	defined(CONFIG_PCA9555) || defined(CONFIG_PCA9554)
	if (!i2c_addr || !i2c_bus) {
		printf("%s: Error: i2c_addr or i2c_bus is NULL\n", __func__);
		return -1;
	}

	*i2c_addr = fdtdec_get_int(fdt, fdt_node, "reg", -1);
	i2c_bus_node = fdt_parent_offset(fdt, fdt_node);
	if (i2c_bus_node < 0) {
		printf("%s: Invalid parent\n", __func__);
		return -1;
	}
	*i2c_bus = i2c_get_bus_num_fdt(i2c_bus_node);
#endif
	return (*type != GPIO_TYPE_UNKNOWN) ? 0 : -1;
}

#ifdef CONFIG_PHY_VITESSE
/**
 * Given a node in the flat device tree, return the matching PHY device
 *
 * @param	fdt_node	FDT node in device tree
 *
 * Return:	pointer to PHY device or NULL if none found.
 */
static struct phy_device *octeon_fdt_get_phy_device_from_node(int fdt_node)
{
	struct eth_device *dev;
	int i = 0;
	struct octeon_eth_info *ethinfo = NULL;

	do {
		dev = eth_get_dev_by_index(i++);
		if (!dev)
			return NULL;
		ethinfo = dev->priv;
		if (ethinfo->phy_offset == fdt_node)
			return ethinfo->phydev;
	} while (dev);
	return NULL;
}
#endif

/**
 * Get the PHY data structure for the specified FDT node and output the type
 *
 * @param	fdt_node	FDT node of phy
 * @param[out]	type		Type of GPIO
 *
 * Return:	pointer to phy device or NULL if no match found.
 */
struct phy_device *octeon_fdt_get_phy_gpio_info(int fdt_node, enum octeon_gpio_type *type)
{
#ifdef CONFIG_PHY_VITESSE
	struct phy_device *phydev;

	if (!octeon_fdt_node_check_compatible(gd->fdt_blob, fdt_node,
					      vitesse_vsc8488_gpio_list)) {
		phydev = octeon_fdt_get_phy_device_from_node(fdt_node);
		if (phydev) {
			debug("%s: Found Vitesse VSC848X compatible GPIO\n", __func__);
			*type = GPIO_TYPE_VSC8488;
			return phydev;
		}

		debug("%s: Error: phy device not found!\n", __func__);
		return NULL;
	}

	debug("%s: No compatible Vitesse PHY type found\n", __func__);
#endif
	return NULL;
}
