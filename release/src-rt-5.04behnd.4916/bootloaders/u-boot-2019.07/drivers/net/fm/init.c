// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011-2015 Freescale Semiconductor, Inc.
 */
#include <errno.h>
#include <common.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#ifdef CONFIG_FSL_LAYERSCAPE
#include <asm/arch/fsl_serdes.h>
#else
#include <asm/fsl_serdes.h>
#endif

#include "fm.h"

struct fm_eth_info fm_info[] = {
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 1)
	FM_DTSEC_INFO_INITIALIZER(1, 1),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 2)
	FM_DTSEC_INFO_INITIALIZER(1, 2),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 3)
	FM_DTSEC_INFO_INITIALIZER(1, 3),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 4)
	FM_DTSEC_INFO_INITIALIZER(1, 4),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 5)
	FM_DTSEC_INFO_INITIALIZER(1, 5),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 6)
	FM_DTSEC_INFO_INITIALIZER(1, 6),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 7)
	FM_DTSEC_INFO_INITIALIZER(1, 9),
#endif
#if (CONFIG_SYS_NUM_FM1_DTSEC >= 8)
	FM_DTSEC_INFO_INITIALIZER(1, 10),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 1)
	FM_DTSEC_INFO_INITIALIZER(2, 1),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 2)
	FM_DTSEC_INFO_INITIALIZER(2, 2),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 3)
	FM_DTSEC_INFO_INITIALIZER(2, 3),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 4)
	FM_DTSEC_INFO_INITIALIZER(2, 4),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 5)
	FM_DTSEC_INFO_INITIALIZER(2, 5),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 6)
	FM_DTSEC_INFO_INITIALIZER(2, 6),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 7)
	FM_DTSEC_INFO_INITIALIZER(2, 9),
#endif
#if (CONFIG_SYS_NUM_FM2_DTSEC >= 8)
	FM_DTSEC_INFO_INITIALIZER(2, 10),
#endif
#if (CONFIG_SYS_NUM_FM1_10GEC >= 1)
	FM_TGEC_INFO_INITIALIZER(1, 1),
#endif
#if (CONFIG_SYS_NUM_FM1_10GEC >= 2)
	FM_TGEC_INFO_INITIALIZER(1, 2),
#endif
#if (CONFIG_SYS_NUM_FM1_10GEC >= 3)
	FM_TGEC_INFO_INITIALIZER2(1, 3),
#endif
#if (CONFIG_SYS_NUM_FM1_10GEC >= 4)
	FM_TGEC_INFO_INITIALIZER2(1, 4),
#endif
#if (CONFIG_SYS_NUM_FM2_10GEC >= 1)
	FM_TGEC_INFO_INITIALIZER(2, 1),
#endif
#if (CONFIG_SYS_NUM_FM2_10GEC >= 2)
	FM_TGEC_INFO_INITIALIZER(2, 2),
#endif
};

int fm_standard_init(bd_t *bis)
{
	int i;
	struct ccsr_fman *reg;

	reg = (void *)CONFIG_SYS_FSL_FM1_ADDR;
	if (fm_init_common(0, reg))
		return 0;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if ((fm_info[i].enabled) && (fm_info[i].index == 1))
			fm_eth_initialize(reg, &fm_info[i]);
	}

#if (CONFIG_SYS_NUM_FMAN == 2)
	reg = (void *)CONFIG_SYS_FSL_FM2_ADDR;
	if (fm_init_common(1, reg))
		return 0;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if ((fm_info[i].enabled) && (fm_info[i].index == 2))
			fm_eth_initialize(reg, &fm_info[i]);
	}
#endif

	return 1;
}

/* simple linear search to map from port to array index */
static int fm_port_to_index(enum fm_port port)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		if (fm_info[i].port == port)
			return i;
	}

	return -1;
}

/*
 * Determine if an interface is actually active based on HW config
 * we expect fman_port_enet_if() to report PHY_INTERFACE_MODE_NONE if
 * the interface is not active based on HW cfg of the SoC
 */
void fman_enet_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		phy_interface_t enet_if;

		enet_if = fman_port_enet_if(fm_info[i].port);
		if (enet_if != PHY_INTERFACE_MODE_NONE) {
			fm_info[i].enabled = 1;
			fm_info[i].enet_if = enet_if;
		} else {
			fm_info[i].enabled = 0;
		}
	}

	return ;
}

void fm_disable_port(enum fm_port port)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return;

	fm_info[i].enabled = 0;
#ifndef CONFIG_SYS_FMAN_V3
	fman_disable_port(port);
#endif
}

void fm_enable_port(enum fm_port port)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return;

	fm_info[i].enabled = 1;
	fman_enable_port(port);
}

void fm_info_set_mdio(enum fm_port port, struct mii_dev *bus)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return;

	fm_info[i].bus = bus;
}

void fm_info_set_phy_address(enum fm_port port, int address)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return;

	fm_info[i].phy_addr = address;
}

/*
 * Returns the PHY address for a given Fman port
 *
 * The port must be set via a prior call to fm_info_set_phy_address().
 * A negative error code is returned if the port is invalid.
 */
int fm_info_get_phy_address(enum fm_port port)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return -1;

	return fm_info[i].phy_addr;
}

/*
 * Returns the type of the data interface between the given MAC and its PHY.
 * This is typically determined by the RCW.
 */
phy_interface_t fm_info_get_enet_if(enum fm_port port)
{
	int i = fm_port_to_index(port);

	if (i == -1)
		return PHY_INTERFACE_MODE_NONE;

	if (fm_info[i].enabled)
		return fm_info[i].enet_if;

	return PHY_INTERFACE_MODE_NONE;
}

static void
__def_board_ft_fman_fixup_port(void *blob, char * prop, phys_addr_t pa,
				enum fm_port port, int offset)
{
	return ;
}

void board_ft_fman_fixup_port(void *blob, char * prop, phys_addr_t pa,
				enum fm_port port, int offset)
	 __attribute__((weak, alias("__def_board_ft_fman_fixup_port")));

int ft_fixup_port(void *blob, struct fm_eth_info *info, char *prop)
{
	int off;
	uint32_t ph;
	phys_addr_t paddr = CONFIG_SYS_CCSRBAR_PHYS + info->compat_offset;
#ifndef CONFIG_SYS_FMAN_V3
	u64 dtsec1_addr = (u64)CONFIG_SYS_CCSRBAR_PHYS +
				CONFIG_SYS_FSL_FM1_DTSEC1_OFFSET;
#endif

	off = fdt_node_offset_by_compat_reg(blob, prop, paddr);
	if (off == -FDT_ERR_NOTFOUND)
		return -EINVAL;

	if (info->enabled) {
		fdt_fixup_phy_connection(blob, off, info->enet_if);
		board_ft_fman_fixup_port(blob, prop, paddr, info->port, off);
		return 0;
	}

#ifdef CONFIG_SYS_FMAN_V3
#ifndef CONFIG_FSL_FM_10GEC_REGULAR_NOTATION
	/*
	 * On T2/T4 SoCs, physically FM1_DTSEC9 and FM1_10GEC1 use the same
	 * dual-role MAC, when FM1_10GEC1 is enabled and  FM1_DTSEC9
	 * is disabled, ensure that the dual-role MAC is not disabled,
	 * ditto for other dual-role MACs.
	 */
	if (((info->port == FM1_DTSEC9) && (PORT_IS_ENABLED(FM1_10GEC1)))  ||
	    ((info->port == FM1_DTSEC10) && (PORT_IS_ENABLED(FM1_10GEC2))) ||
	    ((info->port == FM1_DTSEC1) && (PORT_IS_ENABLED(FM1_10GEC3)))  ||
	    ((info->port == FM1_DTSEC2) && (PORT_IS_ENABLED(FM1_10GEC4)))  ||
	    ((info->port == FM1_10GEC1) && (PORT_IS_ENABLED(FM1_DTSEC9)))  ||
	    ((info->port == FM1_10GEC2) && (PORT_IS_ENABLED(FM1_DTSEC10))) ||
	    ((info->port == FM1_10GEC3) && (PORT_IS_ENABLED(FM1_DTSEC1)))  ||
	    ((info->port == FM1_10GEC4) && (PORT_IS_ENABLED(FM1_DTSEC2)))
#if (CONFIG_SYS_NUM_FMAN == 2)
										||
	    ((info->port == FM2_DTSEC9) && (PORT_IS_ENABLED(FM2_10GEC1)))	||
	    ((info->port == FM2_DTSEC10) && (PORT_IS_ENABLED(FM2_10GEC2)))	||
	    ((info->port == FM2_10GEC1) && (PORT_IS_ENABLED(FM2_DTSEC9)))	||
	    ((info->port == FM2_10GEC2) && (PORT_IS_ENABLED(FM2_DTSEC10)))
#endif
#else
	/* FM1_DTSECx and FM1_10GECx use the same dual-role MAC */
	if (((info->port == FM1_DTSEC1) && (PORT_IS_ENABLED(FM1_10GEC1)))  ||
	    ((info->port == FM1_DTSEC2) && (PORT_IS_ENABLED(FM1_10GEC2)))  ||
	    ((info->port == FM1_DTSEC3) && (PORT_IS_ENABLED(FM1_10GEC3)))  ||
	    ((info->port == FM1_DTSEC4) && (PORT_IS_ENABLED(FM1_10GEC4)))  ||
	    ((info->port == FM1_10GEC1) && (PORT_IS_ENABLED(FM1_DTSEC1)))  ||
	    ((info->port == FM1_10GEC2) && (PORT_IS_ENABLED(FM1_DTSEC2)))  ||
	    ((info->port == FM1_10GEC3) && (PORT_IS_ENABLED(FM1_DTSEC3)))  ||
	    ((info->port == FM1_10GEC4) && (PORT_IS_ENABLED(FM1_DTSEC4)))
#endif
	)
		return 0;
#endif
	/* board code might have caused offset to change */
	off = fdt_node_offset_by_compat_reg(blob, prop, paddr);

#ifndef CONFIG_SYS_FMAN_V3
	/* Don't disable FM1-DTSEC1 MAC as its used for MDIO */
	if (paddr != dtsec1_addr)
#endif
		fdt_status_disabled(blob, off); /* disable the MAC node */

	/* disable the fsl,dpa-ethernet node that points to the MAC */
	ph = fdt_get_phandle(blob, off);
	do_fixup_by_prop(blob, "fsl,fman-mac", &ph, sizeof(ph),
		"status", "disabled", strlen("disabled") + 1, 1);

	return 0;
}

void fdt_fixup_fman_ethernet(void *blob)
{
	int i;

#ifdef CONFIG_SYS_FMAN_V3
	for (i = 0; i < ARRAY_SIZE(fm_info); i++)
		ft_fixup_port(blob, &fm_info[i], "fsl,fman-memac");
#else
	for (i = 0; i < ARRAY_SIZE(fm_info); i++) {
		/* Try the new compatible first.
		 * If the node is missing, try the old.
		 */
		if (fm_info[i].type == FM_ETH_1G_E) {
			if (ft_fixup_port(blob, &fm_info[i], "fsl,fman-dtsec"))
				ft_fixup_port(blob, &fm_info[i],
					      "fsl,fman-1g-mac");
		} else {
			if (ft_fixup_port(blob, &fm_info[i], "fsl,fman-xgec") &&
			    ft_fixup_port(blob, &fm_info[i], "fsl,fman-tgec"))
				ft_fixup_port(blob, &fm_info[i],
					      "fsl,fman-10g-mac");
		}
	}
#endif
}

/*QSGMII Riser Card can work in SGMII mode, but the PHY address is different.
 *This function scans which Riser Card being used(QSGMII or SGMII Riser Card),
 *then set the correct PHY address
 */
void set_sgmii_phy(struct mii_dev *bus, enum fm_port base_port,
		unsigned int port_num, int phy_base_addr)
{
	unsigned int regnum = 0;
	int qsgmii;
	int i;
	int phy_real_addr;

	qsgmii = is_qsgmii_riser_card(bus, phy_base_addr, port_num, regnum);

	if (!qsgmii)
		return;

	for (i = base_port; i < base_port + port_num; i++) {
		if (fm_info_get_enet_if(i) == PHY_INTERFACE_MODE_SGMII) {
			phy_real_addr = phy_base_addr + i - base_port;
			fm_info_set_phy_address(i, phy_real_addr);
		}
	}
}

/*to check whether qsgmii riser card is used*/
int is_qsgmii_riser_card(struct mii_dev *bus, int phy_base_addr,
		unsigned int port_num, unsigned regnum)
{
	int i;
	int val;

	if (!bus)
		return 0;

	for (i = phy_base_addr; i < phy_base_addr + port_num; i++) {
		val = bus->read(bus, i, MDIO_DEVAD_NONE, regnum);
		if (val != MIIM_TIMEOUT)
			return 1;
	}

	return 0;
}
