// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <dm.h>
#include <fdt_support.h>
#include <ram.h>
#include <asm/gpio.h>

#include <mach/octeon_ddr.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-bgxx-defs.h>

#include "board_ddr.h"

#define MAX_MIX_ENV_VARS	4

#define EBB7304_DEF_DRAM_FREQ	800

static struct ddr_conf board_ddr_conf[] = {
	OCTEON_EBB7304_DDR_CONFIGURATION
};

static int no_phy[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

struct ddr_conf *octeon_ddr_conf_table_get(int *count, int *def_ddr_freq)
{
	*count = ARRAY_SIZE(board_ddr_conf);
	*def_ddr_freq = EBB7304_DEF_DRAM_FREQ;

	return board_ddr_conf;
}

/*
 * parse_env_var:	Parse the environment variable ("bgx_for_mix%d") to
 *			extract the lmac it is set to.
 *
 *  index:		Index of environment variable to parse.
 *			environment variable.
 *  env_bgx:		Updated with the bgx of the lmac in the environment
 *			variable.
 *  env_lmac:		Updated with the index of lmac in the environment
 *			variable.
 *
 *  returns:		Zero on success, error otherwise.
 */
static int parse_env_var(int index, int *env_bgx, int *env_lmac)
{
	char env_var[20];
	ulong xipd_port;

	sprintf(env_var, "bgx_for_mix%d", index);
	xipd_port = env_get_ulong(env_var, 0, 0xffff);
	if (xipd_port != 0xffff) {
		int xiface;
		struct cvmx_xiface xi;
		struct cvmx_xport xp;

		/*
		 * The environemt variable is set to the xipd port. Convert the
		 * xipd port to numa node, bgx, and lmac.
		 */
		xiface = cvmx_helper_get_interface_num(xipd_port);
		xi = cvmx_helper_xiface_to_node_interface(xiface);
		xp = cvmx_helper_ipd_port_to_xport(xipd_port);
		*env_bgx = xi.interface;
		*env_lmac = cvmx_helper_get_interface_index_num(xp.port);
		return 0;
	}

	return -1;
}

/*
 * get_lmac_fdt_node:	Search the device tree for the node corresponding to
 *			a given bgx lmac.
 *
 *  fdt:		Pointer to flat device tree
 *  search_node:	Numa node of the lmac to search for.
 *  search_bgx:		Bgx of the lmac to search for.
 *  search_lmac:	Lmac index to search for.
 *  compat:		Compatible string to search for.

 *  returns:		The device tree node of the lmac if found,
 *			or -1 otherwise.
 */
static int get_lmac_fdt_node(const void *fdt, int search_node, int search_bgx, int search_lmac,
			     const char *compat)
{
	int node;
	const fdt32_t *reg;
	u64 addr;
	int fdt_node = -1;
	int fdt_bgx = -1;
	int fdt_lmac = -1;
	int len;
	int parent;

	/* Iterate through all bgx ports */
	fdt_for_each_node_by_compatible(node, (void *)fdt, -1, compat) {
		/* Get the node and bgx from the physical address */
		parent = fdt_parent_offset(fdt, node);
		reg = fdt_getprop(fdt, parent, "reg", &len);
		if (parent < 0 || !reg)
			continue;

		addr = fdt_translate_address((void *)fdt, parent, reg);
		fdt_node = (addr >> 36) & 0x7;
		fdt_bgx = (addr >> 24) & 0xf;

		/* Get the lmac index from the reg property */
		reg = fdt_getprop(fdt, node, "reg", &len);
		if (reg)
			fdt_lmac = *reg;

		/* Check for a match */
		if (search_node == fdt_node && search_bgx == fdt_bgx &&
		    search_lmac == fdt_lmac)
			return node;
	}

	return -1;
}

/*
 * get_mix_fdt_node:	Search the device tree for the node corresponding to
 *			a given mix.
 *
 *  fdt:		Pointer to flat device tree
 *  search_node:	Mix numa node to search for.
 *  search_index:	Mix index to search for.
 *
 *  returns:		The device tree node of the lmac if found,
 *			or -1 otherwise.
 */
static int get_mix_fdt_node(const void *fdt, int search_node, int search_index)
{
	int node;

	/* Iterate through all the mix fdt nodes */
	fdt_for_each_node_by_compatible(node, (void *)fdt, -1,
					"cavium,octeon-7890-mix") {
		int parent;
		int len;
		const char *name;
		int mix_numa_node;
		const fdt32_t *reg;
		int mix_index = -1;
		u64 addr;

		/* Get the numa node of the mix from the parent node name */
		parent = fdt_parent_offset(fdt, node);
		if (parent < 0 ||
		    ((name = fdt_get_name(fdt, parent, &len)) == NULL) ||
		    ((name = strchr(name, '@')) == NULL))
			continue;

		name++;
		mix_numa_node = simple_strtol(name, NULL, 0) ? 1 : 0;

		/* Get the mix index from the reg property */
		reg = fdt_getprop(fdt, node, "reg", &len);
		if (reg) {
			addr = fdt_translate_address((void *)fdt, parent, reg);
			mix_index = (addr >> 11) & 1;
		}

		/* Check for a match */
		if (mix_numa_node == search_node && mix_index == search_index)
			return node;
	}

	return -1;
}

/*
 * fdt_fix_mix:		Fix the mix nodes in the device tree. Only the mix nodes
 *			configured by the user will be preserved. All other mix
 *			nodes will be trimmed.
 *
 *  fdt:		Pointer to flat device tree
 *
 *  returns:		Zero on success, error otherwise.
 */
static int fdt_fix_mix(const void *fdt)
{
	int node;
	int next_node;
	int len;
	int i;

	/* Parse all the mix port environment variables */
	for (i = 0; i < MAX_MIX_ENV_VARS; i++) {
		int env_node = 0;
		int env_bgx = -1;
		int env_lmac = -1;
		int lmac_fdt_node = -1;
		int mix_fdt_node = -1;
		unsigned int lmac_phandle;
		char *compat;

		/* Get the lmac for this environment variable */
		if (parse_env_var(i, &env_bgx, &env_lmac))
			continue;

		/* Get the fdt node for this lmac and add a phandle to it */
		compat = "cavium,octeon-7890-bgx-port";
		lmac_fdt_node = get_lmac_fdt_node(fdt, env_node, env_bgx,
						  env_lmac, compat);
		if (lmac_fdt_node < 0) {
			/* Must check for the xcv compatible string too */
			compat = "cavium,octeon-7360-xcv";
			lmac_fdt_node = get_lmac_fdt_node(fdt, env_node,
							  env_bgx, env_lmac,
							  compat);
			if (lmac_fdt_node < 0) {
				printf("WARNING: Failed to get lmac fdt node for %d%d%d\n",
				       env_node, env_bgx, env_lmac);
				continue;
			}
		}

		lmac_phandle = fdt_create_phandle((void *)fdt, lmac_fdt_node);

		/* Get the fdt mix node corresponding to this lmac */
		mix_fdt_node = get_mix_fdt_node(fdt, env_node, env_lmac);
		if (mix_fdt_node < 0)
			continue;

		/* Point the mix to the lmac */
		fdt_getprop(fdt, mix_fdt_node, "cavium,mac-handle", &len);
		fdt_setprop_inplace((void *)fdt, mix_fdt_node,
				    "cavium,mac-handle", &lmac_phandle, len);
	}

	/* Trim unused mix'es from the device tree */
	for (node = fdt_next_node(fdt, -1, NULL); node >= 0; node = next_node) {
		const char *compat;
		const fdt32_t *reg;

		next_node = fdt_next_node(fdt, node, NULL);

		compat = fdt_getprop(fdt, node, "compatible", &len);
		if (compat) {
			if (strcmp(compat, "cavium,octeon-7890-mix"))
				continue;

			reg = fdt_getprop(fdt, node, "cavium,mac-handle", &len);
			if (reg) {
				if (*reg == 0xffff)
					fdt_nop_node((void *)fdt, node);
			}
		}
	}

	return 0;
}

static void kill_fdt_phy(void *fdt, int offset, void *arg)
{
	int len, phy_offset;
	const fdt32_t *php;
	u32 phandle;

	php = fdt_getprop(fdt, offset, "phy-handle", &len);
	if (php && len == sizeof(*php)) {
		phandle = fdt32_to_cpu(*php);
		fdt_nop_property(fdt, offset, "phy-handle");
		phy_offset = fdt_node_offset_by_phandle(fdt, phandle);
		if (phy_offset > 0)
			fdt_nop_node(fdt, phy_offset);
	}
}

void __fixup_xcv(void)
{
	unsigned long bgx = env_get_ulong("bgx_for_rgmii", 10,
					  (unsigned long)-1);
	char fdt_key[16];
	int i;

	debug("%s: BGX %d\n", __func__, (int)bgx);

	for (i = 0; i < 3; i++) {
		snprintf(fdt_key, sizeof(fdt_key),
			 bgx == i ? "%d,xcv" : "%d,not-xcv", i);
		debug("%s: trimming bgx %lu with key %s\n",
		      __func__, bgx, fdt_key);

		octeon_fdt_patch_rename((void *)gd->fdt_blob, fdt_key,
					"cavium,xcv-trim", true, NULL, NULL);
	}
}

/* QLM0 - QLM6 */
void __fixup_fdt(void)
{
	int qlm;
	int speed = 0;

	for (qlm = 0; qlm < 7; qlm++) {
		enum cvmx_qlm_mode mode;
		char fdt_key[16];
		const char *type_str = "none";

		mode = cvmx_qlm_get_mode(qlm);
		switch (mode) {
		case CVMX_QLM_MODE_SGMII:
		case CVMX_QLM_MODE_RGMII_SGMII:
		case CVMX_QLM_MODE_RGMII_SGMII_1X1:
			type_str = "sgmii";
			break;
		case CVMX_QLM_MODE_XAUI:
		case CVMX_QLM_MODE_RGMII_XAUI:
			speed = (cvmx_qlm_get_gbaud_mhz(qlm) * 8 / 10) * 4;
			if (speed == 10000)
				type_str = "xaui";
			else
				type_str = "dxaui";
			break;
		case CVMX_QLM_MODE_RXAUI:
		case CVMX_QLM_MODE_RGMII_RXAUI:
			type_str = "rxaui";
			break;
		case CVMX_QLM_MODE_XLAUI:
		case CVMX_QLM_MODE_RGMII_XLAUI:
			type_str = "xlaui";
			break;
		case CVMX_QLM_MODE_XFI:
		case CVMX_QLM_MODE_RGMII_XFI:
		case CVMX_QLM_MODE_RGMII_XFI_1X1:
			type_str = "10gbase-r";
			break;
		case CVMX_QLM_MODE_10G_KR:
		case CVMX_QLM_MODE_RGMII_10G_KR:
			type_str = "10G_KR";
			break;
		case CVMX_QLM_MODE_40G_KR4:
		case CVMX_QLM_MODE_RGMII_40G_KR4:
			type_str = "40G_KR4";
			break;
		case CVMX_QLM_MODE_SATA_2X1:
			type_str = "sata";
			break;
		case CVMX_QLM_MODE_SGMII_2X1:
		case CVMX_QLM_MODE_XFI_1X2:
		case CVMX_QLM_MODE_10G_KR_1X2:
		case CVMX_QLM_MODE_RXAUI_1X2:
		case CVMX_QLM_MODE_MIXED: // special for DLM5 & DLM6
		{
			cvmx_bgxx_cmrx_config_t cmr_config;
			cvmx_bgxx_spux_br_pmd_control_t pmd_control;
			int mux = cvmx_qlm_mux_interface(2);

			if (mux == 2) { // only dlm6
				cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(2, 2));
				pmd_control.u64 =
					csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(2, 2));
			} else {
				if (qlm == 5) {
					cmr_config.u64 =
						csr_rd(CVMX_BGXX_CMRX_CONFIG(0, 2));
					pmd_control.u64 =
						csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(0, 2));
				} else {
					cmr_config.u64 =
						csr_rd(CVMX_BGXX_CMRX_CONFIG(2, 2));
					pmd_control.u64 =
						csr_rd(CVMX_BGXX_SPUX_BR_PMD_CONTROL(2, 2));
				}
			}
			switch (cmr_config.s.lmac_type) {
			case 0:
				type_str = "sgmii";
				break;
			case 1:
				type_str = "xaui";
				break;
			case 2:
				type_str = "rxaui";
				break;
			case 3:
				if (pmd_control.s.train_en)
					type_str = "10G_KR";
				else
					type_str = "10gbase-r";
				break;
			case 4:
				if (pmd_control.s.train_en)
					type_str = "40G_KR4";
				else
					type_str = "xlaui";
				break;
			default:
				type_str = "none";
				break;
			}
			break;
		}
		default:
			type_str = "none";
			break;
		}
		sprintf(fdt_key, "%d,%s", qlm, type_str);
		debug("Patching qlm %d for %s for mode %d%s\n", qlm, fdt_key, mode,
		      no_phy[qlm] ? ", removing PHY" : "");
		octeon_fdt_patch_rename((void *)gd->fdt_blob, fdt_key, NULL, true,
					no_phy[qlm] ? kill_fdt_phy : NULL, NULL);
	}
}

int board_fix_fdt(void)
{
	__fixup_fdt();
	__fixup_xcv();

	/* Fix the mix ports */
	fdt_fix_mix(gd->fdt_blob);

	return 0;
}

/*
 * Here is the description of the parameters that are passed to QLM
 * configuration:
 *
 *	param0 : The QLM to configure
 *	param1 : Speed to configure the QLM at
 *	param2 : Mode the QLM to configure
 *	param3 : 1 = RC, 0 = EP
 *	param4 : 0 = GEN1, 1 = GEN2, 2 = GEN3
 *	param5 : ref clock select, 0 = 100Mhz, 1 = 125MHz, 2 = 156MHz
 *	param6 : ref clock input to use:
 *		 0 - external reference (QLMx_REF_CLK)
 *		 1 = common clock 0 (QLMC_REF_CLK0)
 *		 2 = common_clock 1 (QLMC_REF_CLK1)
 */
static void board_configure_qlms(void)
{
	int speed[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int mode[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	int pcie_rc[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int pcie_gen[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int ref_clock_sel[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int ref_clock_input[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	struct gpio_desc desc;
	int rbgx, rqlm;
	char env_var[16];
	int qlm;
	int ret;

	/* RGMII PHY reset GPIO */
	ret = dm_gpio_lookup_name("gpio-controllerA27", &desc);
	if (ret)
		debug("gpio ret=%d\n", ret);
	ret = dm_gpio_request(&desc, "rgmii_phy_reset");
	if (ret)
		debug("gpio_request ret=%d\n", ret);
	ret = dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT);
	if (ret)
		debug("gpio dir ret=%d\n", ret);

	/* Put RGMII PHY in reset */
	dm_gpio_set_value(&desc, 0);

	octeon_init_qlm(0);

	rbgx = env_get_ulong("bgx_for_rgmii", 10, (unsigned long)-1);
	switch (rbgx) {
	case 0:
		rqlm = 2;
		break;
	case 1:
		rqlm = 3;
		break;
	case 2:
		rqlm = 5;
		break;
	default:
		rqlm = -1;
		break;
	}

	for (qlm = 0; qlm < 7; qlm++) {
		const char *mode_str;
		char spd_env[16];

		mode[qlm] = CVMX_QLM_MODE_DISABLED;
		sprintf(env_var, "qlm%d_mode", qlm);
		mode_str = env_get(env_var);
		if (!mode_str)
			continue;

		if (qlm == 4 && mode[4] != -1 &&
		    mode[4] != CVMX_QLM_MODE_SATA_2X1) {
			printf("Error: DLM 4 can only be configured for SATA\n");
			continue;
		}

		if (strstr(mode_str, ",no_phy"))
			no_phy[qlm] = 1;

		if (!strncmp(mode_str, "sgmii", 5)) {
			bool rgmii = false;

			speed[qlm] = 1250;
			if (rqlm == qlm && qlm < 5) {
				mode[qlm] = CVMX_QLM_MODE_RGMII_SGMII;
				rgmii = true;
			} else if (qlm == 6 || qlm == 5) {
				if (rqlm == qlm && qlm == 5) {
					mode[qlm] = CVMX_QLM_MODE_RGMII_SGMII_1X1;
					rgmii = true;
				} else if (rqlm == 5 && qlm == 6 &&
					   mode[5] != CVMX_QLM_MODE_RGMII_SGMII_1X1) {
					mode[qlm] = CVMX_QLM_MODE_RGMII_SGMII_2X1;
					rgmii = true;
				} else {
					mode[qlm] = CVMX_QLM_MODE_SGMII_2X1;
				}
			} else {
				mode[qlm] = CVMX_QLM_MODE_SGMII;
			}
			ref_clock_sel[qlm] = 2;

			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1

			if (no_phy[qlm]) {
				int i;
				int start = 0, stop = 2;

				rbgx = 0;
				switch (qlm) {
				case 3:
					rbgx = 1;
				case 2:
					for (i = 0; i < 4; i++) {
						printf("Ignoring PHY for interface: %d, port: %d\n",
						       rbgx, i);
						cvmx_helper_set_port_force_link_up(rbgx, i, true);
					}
					break;
				case 6:
					start = 2;
					stop = 4;
				case 5:
					for (i = start; i < stop; i++) {
						printf("Ignoring PHY for interface: %d, port: %d\n",
						       2, i);
						cvmx_helper_set_port_force_link_up(2, i, true);
					}
					break;
				default:
					printf("SGMII not supported for QLM/DLM %d\n",
					       qlm);
					break;
				}
			}
			printf("QLM %d: SGMII%s\n",
			       qlm, rgmii ? ", RGMII" : "");
		} else if (!strncmp(mode_str, "xaui", 4)) {
			speed[qlm] = 3125;
			mode[qlm] = CVMX_QLM_MODE_XAUI;
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			printf("QLM %d: XAUI\n", qlm);
		} else if (!strncmp(mode_str, "dxaui", 5)) {
			speed[qlm] = 6250;
			mode[qlm] = CVMX_QLM_MODE_XAUI;
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			printf("QLM %d: DXAUI\n", qlm);
		} else if (!strncmp(mode_str, "rxaui", 5)) {
			bool rgmii = false;

			speed[qlm] = 6250;
			if (qlm == 5 || qlm == 6) {
				if (rqlm == qlm && qlm == 5) {
					mode[qlm] = CVMX_QLM_MODE_RGMII_RXAUI;
					rgmii = true;
				} else {
					mode[qlm] = CVMX_QLM_MODE_RXAUI_1X2;
				}
			} else {
				mode[qlm] = CVMX_QLM_MODE_RXAUI;
			}
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			printf("QLM %d: RXAUI%s\n",
			       qlm, rgmii ? ", rgmii" : "");
		} else if (!strncmp(mode_str, "xlaui", 5)) {
			speed[qlm] = 103125;
			mode[qlm] = CVMX_QLM_MODE_XLAUI;
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			sprintf(spd_env, "qlm%d_speed", qlm);
			if (env_get(spd_env)) {
				int spd = env_get_ulong(spd_env, 0, 8);

				if (spd)
					speed[qlm] = spd;
				else
					speed[qlm] = 103125;
			}
			printf("QLM %d: XLAUI\n", qlm);
		} else if (!strncmp(mode_str, "10gbase-r", 3)) {
			bool rgmii = false;

			speed[qlm] = 103125;
			if (rqlm == qlm) {
				mode[qlm] = CVMX_QLM_MODE_RGMII_XFI;
				rgmii = true;
			} else if (qlm == 5 || qlm == 6) {
				mode[qlm] = CVMX_QLM_MODE_XFI_1X2;
			} else {
				mode[qlm] = CVMX_QLM_MODE_XFI;
			}
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			printf("QLM %d: XFI%s\n", qlm, rgmii ? ", RGMII" : "");
		} else if (!strncmp(mode_str, "10G_KR", 6)) {
			speed[qlm] = 103125;
			if (rqlm == qlm && qlm == 5)
				mode[qlm] = CVMX_QLM_MODE_RGMII_10G_KR;
			else if (qlm == 5 || qlm == 6)
				mode[qlm] = CVMX_QLM_MODE_10G_KR_1X2;
			else
				mode[qlm] = CVMX_QLM_MODE_10G_KR;
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			printf("QLM %d: 10G_KR\n", qlm);
		} else if (!strncmp(mode_str, "40G_KR4", 7)) {
			speed[qlm] = 103125;
			mode[qlm] = CVMX_QLM_MODE_40G_KR4;
			ref_clock_sel[qlm] = 2;
			if (qlm == 5 || qlm == 6)
				ref_clock_input[qlm] = 2; // use QLMC_REF_CLK1
			printf("QLM %d: 40G_KR4\n", qlm);
		} else if (!strcmp(mode_str, "pcie")) {
			char *pmode;
			int lanes = 0;

			sprintf(env_var, "pcie%d_mode", qlm);
			pmode = env_get(env_var);
			if (pmode && !strcmp(pmode, "ep"))
				pcie_rc[qlm] = 0;
			else
				pcie_rc[qlm] = 1;
			sprintf(env_var, "pcie%d_gen", qlm);
			pcie_gen[qlm] = env_get_ulong(env_var, 0, 3);
			sprintf(env_var, "pcie%d_lanes", qlm);
			lanes = env_get_ulong(env_var, 0, 8);
			if (lanes == 8) {
				mode[qlm] = CVMX_QLM_MODE_PCIE_1X8;
			} else if (qlm == 5 || qlm == 6) {
				if (lanes != 2) {
					printf("QLM%d: Invalid lanes selected, defaulting to 2 lanes\n",
					       qlm);
				}
				mode[qlm] = CVMX_QLM_MODE_PCIE_1X2;
				ref_clock_input[qlm] = 1; // use QLMC_REF_CLK0
			} else {
				mode[qlm] = CVMX_QLM_MODE_PCIE;
			}
			ref_clock_sel[qlm] = 0;
			printf("QLM %d: PCIe gen%d %s, x%d lanes\n",
			       qlm, pcie_gen[qlm] + 1,
			       pcie_rc[qlm] ? "root complex" : "endpoint",
			       lanes);
		} else if (!strcmp(mode_str, "sata")) {
			mode[qlm] = CVMX_QLM_MODE_SATA_2X1;
			ref_clock_sel[qlm] = 0;
			ref_clock_input[qlm] = 1;
			sprintf(spd_env, "qlm%d_speed", qlm);
			if (env_get(spd_env)) {
				int spd = env_get_ulong(spd_env, 0, 8);

				if (spd == 1500 || spd == 3000 || spd == 3000)
					speed[qlm] = spd;
				else
					speed[qlm] = 6000;
			} else {
				speed[qlm] = 6000;
			}
		} else {
			printf("QLM %d: disabled\n", qlm);
		}
	}

	for (qlm = 0; qlm < 7; qlm++) {
		int rc;

		if (mode[qlm] == -1)
			continue;

		debug("Configuring qlm%d with speed(%d), mode(%d), RC(%d), Gen(%d), REF_CLK(%d), CLK_SOURCE(%d)\n",
		      qlm, speed[qlm], mode[qlm], pcie_rc[qlm],
		      pcie_gen[qlm] + 1,
		      ref_clock_sel[qlm], ref_clock_input[qlm]);
		rc = octeon_configure_qlm(qlm, speed[qlm], mode[qlm],
					  pcie_rc[qlm], pcie_gen[qlm],
					  ref_clock_sel[qlm],
					  ref_clock_input[qlm]);

		if (speed[qlm] == 6250) {
			if (mode[qlm] == CVMX_QLM_MODE_RXAUI) {
				octeon_qlm_tune_v3(0, qlm, speed[qlm], 0x12,
						   0xa0, -1, -1);
			} else {
				octeon_qlm_tune_v3(0, qlm, speed[qlm], 0xa,
						   0xa0, -1, -1);
			}
		} else if (speed[qlm] == 103125) {
			octeon_qlm_tune_v3(0, qlm, speed[qlm], 0xd, 0xd0,
					   -1, -1);
		}

		if (qlm == 4 && rc != 0)
			/*
			 * There is a bug with SATA with 73xx.  Until it's
			 * fixed we need to strip it from the device tree.
			 */
			octeon_fdt_patch_rename((void *)gd->fdt_blob, "4,none",
						NULL, true, NULL, NULL);
	}

	dm_gpio_set_value(&desc, 0); /* Put RGMII PHY in reset */
	mdelay(10);
	dm_gpio_set_value(&desc, 1); /* Take RGMII PHY out of reset */
}

int board_late_init(void)
{
	board_configure_qlms();

	return 0;
}
