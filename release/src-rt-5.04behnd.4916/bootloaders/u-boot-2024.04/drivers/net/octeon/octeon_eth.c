// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <env.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <miiphy.h>
#include <misc.h>
#include <asm/io.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/octeon_fdt.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_eth.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-asxx-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-dbg-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-l2c-defs.h>
#include <mach/cvmx-npi-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pexp-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-smix-defs.h>
#include <mach/cvmx-sriox-defs.h>
#include <mach/cvmx-xcv-defs.h>
#include <mach/cvmx-pcsxx-defs.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-pko.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-config.h>
#include <mach/cvmx-mdio.h>

/** Maximum receive packet size (hardware default is 1536) */
#define CFG_OCTEON_NETWORK_MRU 1536

#define OCTEON_BOOTLOADER_NAMED_BLOCK_TMP_PREFIX "__tmp"

/**
 * Enables RX packet debugging if octeon_debug_rx_packets is set in the
 * environment.
 */
#define DEBUG_RX_PACKET

/**
 * Enables TX packet debugging if octeon_debug_tx_packets is set in the
 * environment.
 */
#define DEBUG_TX_PACKET

/* Global flag indicating common hw has been set up */
static int octeon_global_hw_inited;

#if defined(DEBUG_RX_PACKET) || defined(DEBUG_TX_PACKET)
static int packet_rx_debug;
static int packet_tx_debug;
#endif

/* Make sure that we have enough buffers to keep prefetching blocks happy.
 * Absolute minimum is probably about 200.
 */
#define NUM_PACKET_BUFFERS 1000

#define PKO_SHUTDOWN_TIMEOUT_VAL 100

/* Define the offsets from the base CSR */
#define GMX_PRT_CFG 0x10

#define GMX_RX_FRM_MAX 0x30
#define GMX_RX_JABBER  0x38

#define GMX_RX_ADR_CTL	  0x100
#define GMX_RX_ADR_CAM_EN 0x108
#define GMX_RX_ADR_CAM0	  0x180
#define GMX_RX_ADR_CAM1	  0x188
#define GMX_RX_ADR_CAM2	  0x190
#define GMX_RX_ADR_CAM3	  0x198
#define GMX_RX_ADR_CAM4	  0x1a0
#define GMX_RX_ADR_CAM5	  0x1a8
#define GMX_TX_OVR_BP	  0x4c8

/**
 * Set the hardware MAC address for a device
 *
 * @param interface    interface of port to set
 * @param index    index of port to set MAC address for
 * @param addr   Address structure to change it too.
 * @return Zero on success
 */
static int cvm_oct_set_mac_address(struct udevice *dev)
{
	struct octeon_eth_info *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	cvmx_gmxx_prtx_cfg_t gmx_cfg;
	cvmx_helper_interface_mode_t mode;
	cvmx_gmxx_rxx_adr_ctl_t control;
	u8 *ptr = (uint8_t *)pdata->enetaddr;
	int interface = priv->interface;
	int index = priv->index;
	u64 mac = 0;
	u64 gmx_reg;
	int xipd_port;
	int i;

	for (i = 0; i < 6; i++)
		mac = (mac << 8) | (u64)(ptr[i]);

	debug("%s(%s (%pM))\n", __func__, dev->name, ptr);
	mode = cvmx_helper_interface_get_mode(interface);

	/* It's rather expensive to change the MAC address for BGX so we only
	 * do this if it has changed or not been set previously.
	 */
	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		xipd_port = cvmx_helper_get_ipd_port(interface, index);
		if (priv->last_bgx_mac != mac || !priv->bgx_mac_set) {
			cvmx_helper_bgx_set_mac(xipd_port, 1, 2, mac);
			priv->last_bgx_mac = mac;
			priv->bgx_mac_set = 1;
		}
		return 0;
	}

	if (mode == CVMX_HELPER_INTERFACE_MODE_AGL) {
		gmx_reg = CVMX_AGL_GMX_RXX_INT_REG(0);
	} else {
		gmx_reg = CVMX_GMXX_RXX_INT_REG(index, interface);
		csr_wr(CVMX_GMXX_SMACX(index, interface), mac);
	}

	/* Disable interface */
	gmx_cfg.u64 = csr_rd(gmx_reg + GMX_PRT_CFG);
	csr_wr(gmx_reg + GMX_PRT_CFG, gmx_cfg.u64 & ~1ull);
	debug("%s: gmx reg: 0x%llx\n", __func__, gmx_reg);

	csr_wr(gmx_reg + GMX_RX_ADR_CAM0, ptr[0]);
	csr_wr(gmx_reg + GMX_RX_ADR_CAM1, ptr[1]);
	csr_wr(gmx_reg + GMX_RX_ADR_CAM2, ptr[2]);
	csr_wr(gmx_reg + GMX_RX_ADR_CAM3, ptr[3]);
	csr_wr(gmx_reg + GMX_RX_ADR_CAM4, ptr[4]);
	csr_wr(gmx_reg + GMX_RX_ADR_CAM5, ptr[5]);

	control.u64 = 0;
	control.s.bcst = 1;	/* Allow broadcast MAC addresses */
	control.s.mcst = 1;	/* Force reject multicast packets */
	control.s.cam_mode = 1; /* Filter packets based on the CAM */

	csr_wr(gmx_reg + GMX_RX_ADR_CTL, control.u64);

	csr_wr(gmx_reg + GMX_RX_ADR_CAM_EN, 1);

	/* Return interface to previous enable state */
	csr_wr(gmx_reg + GMX_PRT_CFG, gmx_cfg.u64);

	return 0;
}

static void cvm_oct_fill_hw_memory(u64 pool, u64 size, u64 elements)
{
	static int alloc_count;
	char tmp_name[64];
	int ret;

	debug("%s: pool: 0x%llx, size: 0xx%llx, count: 0x%llx\n",
	      __func__, pool, size, elements);
	sprintf(tmp_name, "%s_fpa_alloc_%d",
		OCTEON_BOOTLOADER_NAMED_BLOCK_TMP_PREFIX, alloc_count++);
	ret = cvmx_fpa_setup_pool(pool, tmp_name, NULL, size, elements);
}

/**
 * Configure common hardware for all interfaces
 */
static void cvm_oct_configure_common_hw(void)
{
	int mru = env_get_ulong("octeon_mru", 0, CFG_OCTEON_NETWORK_MRU);
	int packet_pool_size = CVMX_FPA_PACKET_POOL_SIZE;

	if (mru > packet_pool_size)
		packet_pool_size = (mru + CVMX_CACHE_LINE_SIZE - 1) &
				   ~(CVMX_CACHE_LINE_SIZE - 1);

	/* Setup the FPA */
	cvmx_fpa_enable();

	cvm_oct_fill_hw_memory(CVMX_FPA_WQE_POOL, CVMX_FPA_WQE_POOL_SIZE,
			       NUM_PACKET_BUFFERS);
#if CVMX_FPA_OUTPUT_BUFFER_POOL != CVMX_FPA_PACKET_POOL
	if (!octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvm_oct_fill_hw_memory(CVMX_FPA_OUTPUT_BUFFER_POOL,
				       CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE, 128);
	}
#endif
	cvm_oct_fill_hw_memory(CVMX_FPA_PACKET_POOL, packet_pool_size,
			       NUM_PACKET_BUFFERS);

	cvmx_helper_initialize_packet_io_global();
	cvmx_helper_initialize_packet_io_local();

	/* The MRU defaults to 1536 bytes by the hardware.  Setting
	 * CFG_OCTEON_NETWORK_MRU allows this to be overridden.
	 */
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		struct cvmx_pki_global_config gbl_cfg;
		int i;

		cvmx_pki_read_global_config(0, &gbl_cfg);
		for (i = 0; i < CVMX_PKI_NUM_FRAME_CHECK; i++)
			gbl_cfg.frm_len[i].maxlen = mru;
		cvmx_pki_write_global_config(0, &gbl_cfg);
	}

	/* Set POW get work timeout to maximum value */
	if (octeon_has_feature(OCTEON_FEATURE_CN68XX_WQE) ||
	    octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		csr_wr(CVMX_SSO_NW_TIM, 0x3ff);
	else
		csr_wr(CVMX_POW_NW_TIM, 0x3ff);
}

/**
 * Enables Ethernet devices to allow packets to be transmitted and received.
 * For example, this is activated when the DHCP command is issued.
 *
 * @param	dev	Ethernet device to initialize
 * @param	bis	board data structure, not used.
 *
 * @return	1 for success
 */
int octeon_eth_init(struct udevice *dev)
{
	struct octeon_eth_info *priv = dev_get_priv(dev);

	debug("%s(), dev_ptr: %p, dev: %s, port: %d\n", __func__, dev,
	      dev->name, priv->port);

	if (priv->initted_flag) {
		debug("%s already initialized\n", dev->name);
		return 1;
	}

	if (!octeon_global_hw_inited) {
		debug("Initializing common hardware\n");
		cvm_oct_configure_common_hw();
	}

	/* Ignore backpressure on RGMII ports */
	if (!octeon_has_feature(OCTEON_FEATURE_BGX))
		csr_wr(priv->gmx_base + GMX_TX_OVR_BP, 0xf << 8 | 0xf);

	debug("%s: Setting MAC address\n", __func__);
	cvm_oct_set_mac_address(dev);

	if (!octeon_global_hw_inited) {
		debug("Enabling packet input\n");
		cvmx_helper_ipd_and_packet_input_enable();
		octeon_global_hw_inited = 1;

		/* Connect, configure and start the PHY, if the device is
		 * connected to one. If not, then it's most likely an SPF
		 * enabled port, which does not have such PHY setup here.
		 */
		if (priv->mdio_dev) {
			priv->phy_dev = dm_eth_phy_connect(dev);
			phy_config(priv->phy_dev);
			phy_startup(priv->phy_dev);
		}
	}
	priv->enabled = 0;
	priv->initted_flag = 1;

	debug("%s exiting successfully\n", __func__);
	return 1;
}

/**
 * Initializes the specified interface and port
 *
 * @param	interface	interface to initialize
 * @param	index		port index on interface
 * @param	port		ipd port number
 * @param	if_mode		interface mode
 *
 * @return	0 for success, -1 if out of memory, 1 if port is invalid
 */
static int octeon_eth_initialize(struct udevice *dev, int interface,
				 int index, int port,
				 cvmx_helper_interface_mode_t if_mode)
{
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	int eth;

	eth = cvmx_helper_get_port_fdt_node_offset(interface, index);
	if (eth <= 0) {
		debug("ERROR: No fdt node for interface %d, index %d\n",
		      interface, index);
		return 1;
	}

	oct_eth_info->is_c45 = (if_mode == CVMX_HELPER_INTERFACE_MODE_XAUI) ||
			       (if_mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) ||
			       (if_mode == CVMX_HELPER_INTERFACE_MODE_XFI) ||
			       (if_mode == CVMX_HELPER_INTERFACE_MODE_XLAUI) ||
			       (if_mode == CVMX_HELPER_INTERFACE_MODE_10G_KR) ||
			       (if_mode == CVMX_HELPER_INTERFACE_MODE_10G_KR);
	oct_eth_info->port = port;
	oct_eth_info->index = index;
	oct_eth_info->interface = interface;
	oct_eth_info->initted_flag = 0;
	/* This is guaranteed to force the link state to be printed out */
	oct_eth_info->link_state = 0xffffffffffffffffULL;
	debug("Setting up port: %d, int: %d, index: %d, device: octeth%d\n",
	      oct_eth_info->port, oct_eth_info->interface, oct_eth_info->index,
	      dev_seq(dev));
	if (if_mode == CVMX_HELPER_INTERFACE_MODE_AGL) {
		oct_eth_info->gmx_base = CVMX_AGL_GMX_RXX_INT_REG(0);
	} else {
		if (!octeon_has_feature(OCTEON_FEATURE_BGX))
			oct_eth_info->gmx_base =
				CVMX_GMXX_RXX_INT_REG(index, interface);
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

static int octeon_nic_probe(struct udevice *dev)
{
	struct octeon_eth_info *info = dev_get_priv(dev);
	struct ofnode_phandle_args phandle;
	struct cvmx_xiface xi;
	ofnode node, mdio_node;
	int ipd_port;
	int intf;
	int ret;

	/* The empty stub is to keep cvmx_user_app_init() happy. */
	cvmx_npi_max_pknds = 1;
	__cvmx_helper_init_port_valid();

	xi = __cvmx_bgx_reg_addr_to_xiface(dev_read_addr(dev));
	intf = xi.interface;
	debug("%s: Found BGX node %d, interface %d\n", __func__, xi.node, intf);

	ipd_port = cvmx_helper_get_ipd_port(intf, xi.node);
	ret = octeon_eth_initialize(dev, intf, xi.node, ipd_port,
				    cvmx_helper_interface_get_mode(intf));

	/* Move to subnode, as this includes the "phy-handle" */
	node = dev_read_first_subnode(dev);

	/* Check if an SPF module is conneted, then no MDIO is probed */
	ret = ofnode_parse_phandle_with_args(node, "sfp-slot", NULL, 0, 0,
					     &phandle);
	if (!ret) {
		dev_dbg(dev, "sfp-slot found, not probing for MDIO\n");
		return 0;
	}

	/* Continue with MDIO probing */
	ret = ofnode_parse_phandle_with_args(node, "phy-handle", NULL, 0, 0,
					     &phandle);
	if (ret) {
		dev_err(dev, "phy-handle not found in subnode\n");
		return -ENODEV;
	}

	/* Get MDIO node */
	mdio_node = ofnode_get_parent(phandle.node);
	ret = uclass_get_device_by_ofnode(UCLASS_MDIO, mdio_node,
					  &info->mdio_dev);
	if (ret) {
		dev_err(dev, "mdio_dev not found\n");
		return -ENODEV;
	}

	return 0;
}

/**
 * Sets the hardware MAC address of the Ethernet device
 *
 * @param dev - Ethernet device
 *
 * @return 0 for success
 */
int octeon_eth_write_hwaddr(struct udevice *dev)
{
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);

	/* Skip if the interface isn't yet enabled */
	if (!oct_eth_info->enabled) {
		debug("%s: Interface not enabled, not setting MAC address\n",
		      __func__);
		return 0;
	}
	debug("%s: Setting %s address to %02x:%02x:%02x:%02x:%02x:%02x\n",
	      __func__, dev->name, pdata->enetaddr[0], pdata->enetaddr[1],
	      pdata->enetaddr[2], pdata->enetaddr[3], pdata->enetaddr[4],
	      pdata->enetaddr[5]);
	return cvm_oct_set_mac_address(dev);
}

/**
 * Enables and disables the XCV RGMII interface
 *
 * @param	interface	Interface number
 * @param	index		Port index (should be 0 for RGMII)
 * @param	enable		True to enable it, false to disable it
 */
static void octeon_bgx_xcv_rgmii_enable(int interface, int index, bool enable)
{
	union cvmx_xcv_reset xcv_reset;

	debug("%s(%d, %d, %sable)\n", __func__, interface, index,
	      enable ? "en" : "dis");
	xcv_reset.u64 = csr_rd(CVMX_XCV_RESET);
	xcv_reset.s.rx_pkt_rst_n = enable ? 1 : 0;
	csr_wr(CVMX_XCV_RESET, xcv_reset.u64);
}

/**
 * Enables a SGMII interface
 *
 * @param dev - Ethernet device to initialize
 */
void octeon_eth_sgmii_enable(struct udevice *dev)
{
	struct octeon_eth_info *oct_eth_info;
	cvmx_gmxx_prtx_cfg_t gmx_cfg;
	int index, interface;
	cvmx_helper_interface_mode_t if_mode;

	oct_eth_info = dev_get_priv(dev);
	interface = oct_eth_info->interface;
	index = oct_eth_info->index;

	debug("%s(%s) (%d.%d)\n", __func__, dev->name, interface, index);
	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		cvmx_bgxx_cmrx_config_t cmr_config;

		cmr_config.u64 =
			csr_rd(CVMX_BGXX_CMRX_CONFIG(index, interface));
		cmr_config.s.enable = 1;
		cmr_config.s.data_pkt_tx_en = 1;
		cmr_config.s.data_pkt_rx_en = 1;
		csr_wr(CVMX_BGXX_CMRX_CONFIG(index, interface), cmr_config.u64);
		mdelay(100);
		if (cvmx_helper_bgx_is_rgmii(interface, index))
			octeon_bgx_xcv_rgmii_enable(interface, index, true);
	} else {
		if_mode = cvmx_helper_interface_get_mode(interface);
		/* Normal operating mode. */

		if (if_mode == CVMX_HELPER_INTERFACE_MODE_SGMII ||
		    if_mode == CVMX_HELPER_INTERFACE_MODE_QSGMII) {
			cvmx_pcsx_miscx_ctl_reg_t pcsx_miscx_ctl_reg;

			debug("  if mode: (Q)SGMII\n");
			pcsx_miscx_ctl_reg.u64 = csr_rd(CVMX_PCSX_MISCX_CTL_REG(index, interface));
			pcsx_miscx_ctl_reg.s.gmxeno = 0;
			csr_wr(CVMX_PCSX_MISCX_CTL_REG(index, interface),
			       pcsx_miscx_ctl_reg.u64);
		} else if (if_mode != CVMX_HELPER_INTERFACE_MODE_AGL) {
			cvmx_pcsxx_misc_ctl_reg_t pcsxx_misc_ctl_reg;

			debug("  if mode: AGM\n");
			pcsxx_misc_ctl_reg.u64 =
				csr_rd(CVMX_PCSXX_MISC_CTL_REG(interface));
			pcsxx_misc_ctl_reg.s.gmxeno = 0;
			csr_wr(CVMX_PCSXX_MISC_CTL_REG(interface),
			       pcsxx_misc_ctl_reg.u64);
		}

		gmx_cfg.u64 = csr_rd(oct_eth_info->gmx_base + GMX_PRT_CFG);
		gmx_cfg.s.en = 1;
		csr_wr(oct_eth_info->gmx_base + GMX_PRT_CFG, gmx_cfg.u64);
		gmx_cfg.u64 = csr_rd(oct_eth_info->gmx_base + GMX_PRT_CFG);
	}
}

/**
 * Enables an Ethernet interface
 *
 * @param dev - Ethernet device to enable
 */
void octeon_eth_enable(struct udevice *dev)
{
	struct octeon_eth_info *oct_eth_info;
	u64 tmp;
	int interface;
	cvmx_helper_interface_mode_t if_mode;

	oct_eth_info = dev_get_priv(dev);
	interface = oct_eth_info->interface;
	if_mode = cvmx_helper_interface_get_mode(interface);

	switch (if_mode) {
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		debug("  rgmii/gmii mode\n");
		tmp = csr_rd(CVMX_ASXX_RX_PRT_EN(interface));
		tmp |= (1ull << (oct_eth_info->port & 0x3));
		csr_wr(CVMX_ASXX_RX_PRT_EN(interface), tmp);
		tmp = csr_rd(CVMX_ASXX_TX_PRT_EN(interface));
		tmp |= (1ull << (oct_eth_info->port & 0x3));
		csr_wr(CVMX_ASXX_TX_PRT_EN(interface), tmp);
		octeon_eth_write_hwaddr(dev);
		break;

	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
	case CVMX_HELPER_INTERFACE_MODE_MIXED:
	case CVMX_HELPER_INTERFACE_MODE_AGL:
		debug("  SGMII/XAUI/etc.\n");
		octeon_eth_sgmii_enable(dev);
		octeon_eth_write_hwaddr(dev);
		break;

	default:
		break;
	}
}

void octeon_phy_port_check(struct udevice *dev)
{
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	struct phy_device *phydev = oct_eth_info->phydev;

	if (oct_eth_info->phy_port_check)
		oct_eth_info->phy_port_check(phydev);
}

/**
 * Configure the RGMII port for the negotiated speed
 *
 * @param dev    Linux device for the RGMII port
 */
static void cvm_oct_configure_rgmii_speed(struct udevice *dev)
{
	struct octeon_eth_info *priv = dev_get_priv(dev);
	int port = priv->port;
	cvmx_helper_link_info_t link_state = cvmx_helper_link_get(port);

	/* If the port is down some PHYs we need to check modules, etc. */
	if (!link_state.s.link_up)
		octeon_phy_port_check(dev);

	if (link_state.u64 != priv->link_state) {
		cvmx_helper_interface_mode_t mode;

		octeon_phy_port_check(dev);

		debug("%s(%s): Link state changed\n", __func__, dev->name);
		printf("%s: ", dev->name);
		if (!link_state.s.link_up) {
			puts("Down ");
		} else {
			printf("Up %d Mbps ", link_state.s.speed);
			if (link_state.s.full_duplex)
				puts("Full duplex ");
			else
				puts("Half duplex ");
		}
		mode = cvmx_helper_interface_get_mode(priv->interface);
		printf("(port %2d) (%s)\n", port,
		       cvmx_helper_interface_mode_to_string(mode));
		debug("%s: Setting link state\n", __func__);
		cvmx_helper_link_set(priv->port, link_state);
		priv->link_state = link_state.u64;
	}
}

#if defined(DEBUG_TX_PACKET) || defined(DEBUG_RX_PACKET)
static void print_mac(const char *label, const uint8_t *mac_addr)
{
	printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", label, mac_addr[0],
	       mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

static void print_ip(const void *packet)
{
	u8 *p = (uint8_t *)packet;
	u16 length;
	u8 hdr_len;

	puts("IP Header:\n");
	if ((p[0] & 0xF0) != 0x40) {
		printf("Invalid IP version %d\n", *p >> 4);
		return;
	}
	hdr_len = *p & 0x0F;
	if (hdr_len < 5)
		printf("Invalid IP header length %d\n", hdr_len);
	printf("  Version: 4, Header length: %d\n", hdr_len);
	length = (p[2] << 8) | p[3];
	printf("  TOS: 0x%02x, length: %d\n", p[1], length);
	printf("  ID: %d, %s%s%s fragment offset: %d\n", (p[4] << 8) | p[5],
	       p[6] & 0x80 ? "congested, " : "", p[6] & 0x40 ? "DF, " : "",
	       p[6] & 0x20 ? "MF, " : "", ((p[6] & 0x1F) << 8) | p[7]);
	printf("  TTL: %d, Protocol: %d, Header Checksum: 0x%x\n", p[8], p[9],
	       (p[10] << 8) | p[11]);
	printf("  Source IP: %d.%d.%d.%d\n  Destination IP: %d.%d.%d.%d\n",
	       p[12], p[13], p[14], p[15], p[16], p[17], p[18], p[19]);
	if (p[9] == 17 || p[9] == 6)
		printf("  Source port: %u, Destination port: %u\n",
		       (p[20] << 8) | p[21], (p[22] << 8) | p[23]);
	puts("\n");
}

/**
 * Prints out a packet for debugging purposes
 *
 * @param[in]	packet - pointer to packet data
 * @param	length - length of packet in bytes
 */
static void print_packet(const void *packet, int length)
{
	int i, j;
	const unsigned char *up = packet;
	u16 type = (up[12] << 8 | up[13]);
	int start = 14;

	print_mac("DMAC", &up[0]);
	puts("    ");
	print_mac("SMAC", &up[6]);
	printf("    TYPE: %04x\n", type);

	if (type == 0x0800)
		print_ip(&up[start]);

	for (i = start; (i + 16) < length; i += 16) {
		printf("%04x ", i);
		for (j = 0; j < 16; ++j)
			printf("%02x ", up[i + j]);

		printf("    ");
		for (j = 0; j < 16; ++j)
			printf("%c",
			       ((up[i + j] >= ' ') && (up[i + j] <= '~')) ?
				       up[i + j] :
					     '.');
		printf("\n");
	}
	printf("%04x ", i);
	for (j = 0; i + j < length; ++j)
		printf("%02x ", up[i + j]);

	for (; j < 16; ++j)
		printf("   ");

	printf("    ");
	for (j = 0; i + j < length; ++j)
		printf("%c", ((up[i + j] >= ' ') && (up[i + j] <= '~')) ?
				     up[i + j] :
					   '.');

	printf("\n");
}
#endif

/**
 * String representation of error codes.
 */
static const char * const rx_error_codes[] = {
	"OK",
	"partial",
	"jabber",
	"overrun",
	"oversize",
	"alignment",
	"fragment",
	"fcs",
	"undersize",
	"extend",
	"length mismatch",
	"rgmii rx",
	"skip error",
	"nibble error (studder)",
	"(undefined)",
	"(undefined)",
	"SPI 4.2 FCS",
	"skip",
	"L2 malformed",
};

/**
 * Called to receive a packet
 *
 * @param dev - device to receive on
 *
 * @return - length of packet
 *
 * This function is used to poll packets.  In turn it calls NetReceive
 * to process the packets.
 */
static int nic_recv(struct udevice *dev, int flags, uchar **packetp)
{
	cvmx_wqe_t *work = cvmx_pow_work_request_sync(CVMX_POW_WAIT);
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	cvmx_buf_ptr_t buf_ptr;
	void *packet_data;
	int length;
	int error_code;

	if (!oct_eth_info->enabled) {
		oct_eth_info->enabled = 1;
		debug("%s: Enabling interface %s\n", __func__, dev->name);
		octeon_eth_enable(dev);
	}

	if (!work) {
		/*
		 * Somtimes the link is not up yet. Return here in this
		 * case, this function will be called again later.
		 */
		return 0;
	}

	error_code = cvmx_wqe_get_rcv_err(work);
	if (error_code) {
		/* Work has error, so drop */
		cvmx_helper_free_packet_data(work);
		cvmx_wqe_free(work);
		if (error_code < ARRAY_SIZE(rx_error_codes) &&
		    !octeon_has_feature(OCTEON_FEATURE_BGX))
			printf("Receive error (code %d: %s), dropping\n",
			       error_code, rx_error_codes[error_code]);
		else
			printf("Receive error (code %d (unknown), dropping\n",
			       error_code);
		return 0;
	}
	if (cvmx_wqe_get_bufs(work) != 1) {
		/* can only support single-buffer packets */
		printf("Abnormal packet received in %u bufs, dropping\n",
		       cvmx_wqe_get_bufs(work));
		length = cvmx_wqe_get_len(work);
		buf_ptr = cvmx_wqe_get_packet_ptr(work);
		packet_data = cvmx_phys_to_ptr(buf_ptr.s.addr);
		print_packet(packet_data, length);
		cvmx_helper_free_packet_data(work);
		cvmx_wqe_free(work);
		return 0;
	}

	buf_ptr = cvmx_wqe_get_packet_ptr(work);
	packet_data = cvmx_phys_to_ptr(buf_ptr.s.addr);
	length = cvmx_wqe_get_len(work);

	oct_eth_info->packets_received++;
	debug("############# got work: %p, len: %d, packet_ptr: %p\n", work,
	      length, packet_data);
#if defined(DEBUG_RX_PACKET)
	if (packet_rx_debug) {
		printf("\nRX packet: interface: %d, index: %d\n",
		       oct_eth_info->interface, oct_eth_info->index);
		print_packet(packet_data, length);
	}
#endif
	*packetp = (uchar *)packet_data;

	/* Save work for free_pkt() */
	oct_eth_info->work = work;

	/* Free WQE and packet data */
	return length;
}

static int nic_free_pkt(struct udevice *dev, uchar *pkt, int pkt_len)
{
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	cvmx_wqe_t *work = oct_eth_info->work;

	if (!work)
		return 0;

	cvmx_helper_free_packet_data(work);
	cvmx_wqe_free(work);
	oct_eth_info->work = NULL;

	return 0;
}

/**
 * Packet transmit
 *
 * @param skb    Packet to send
 * @param dev    Device info structure
 * @return Always returns zero
 */
static int cvm_oct_xmit(struct udevice *dev, void *packet, int len)
{
	struct octeon_eth_info *priv = dev_get_priv(dev);
	int queue = cvmx_pko_get_base_queue(priv->port);
	cvmx_pko_command_word0_t pko_command;
	cvmx_buf_ptr_t hw_buffer;
	int rv;

	debug("%s: addr: %p, len: %d\n", __func__, packet, len);

	hw_buffer.u64 = 0;
	hw_buffer.s.addr = cvmx_ptr_to_phys(packet);
	hw_buffer.s.pool = CVMX_FPA_PACKET_POOL;
	hw_buffer.s.size = len;
	hw_buffer.s.back = 0;

	/* Build the PKO command */
	pko_command.u64 = 0;
	pko_command.s.subone0 = 1;
	pko_command.s.dontfree = 0;
	pko_command.s.segs = 1;
	pko_command.s.total_bytes = len;
	/* Send the packet to the output queue */

	debug("%s: port: %d, queue: %d\n", __func__, priv->port, queue);
	cvmx_pko_send_packet_prepare(priv->port, queue, 0);
	rv = cvmx_pko_send_packet_finish(priv->port, queue, pko_command,
					 hw_buffer, 0);
	if (rv)
		printf("Failed to send the packet rv=%d\n", rv);

	return 0;
}

static int nic_xmit(struct udevice *dev, void *pkt, int pkt_len)
{
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	void *fpa_buf = cvmx_fpa_alloc(CVMX_FPA_PACKET_POOL);

	if (!oct_eth_info->enabled) {
		oct_eth_info->enabled = 1;
		octeon_eth_enable(dev);
	}

	/* We need to copy this to a FPA buffer, then give that to TX */

	if (oct_eth_info->packets_sent == 0 &&
	    !octeon_has_feature(OCTEON_FEATURE_BGX))
		cvm_oct_configure_rgmii_speed(dev);

	if (!fpa_buf) {
		printf("ERROR allocating buffer for packet!\n");
		return -1;
	}

	memcpy(fpa_buf, pkt, pkt_len);
#ifdef DEBUG_TX_PACKET
	if (packet_tx_debug) {
		printf("\nTX packet: interface: %d, index: %d\n",
		       oct_eth_info->interface, oct_eth_info->index);
		print_packet(pkt, pkt_len);
	}
#endif
	cvm_oct_xmit(dev, fpa_buf, pkt_len);
	oct_eth_info->packets_sent++;

	return 0;
}

int nic_open(struct udevice *dev)
{
	octeon_eth_init(dev);

	return 0;
}

static void octeon_eth_halt_bgx(struct udevice *dev,
				cvmx_helper_interface_mode_t mode)
{
	union cvmx_bgxx_cmrx_config cmr_config;
	union cvmx_bgxx_cmr_rx_adrx_cam cmr_cam;
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	int index = oct_eth_info->index;
	int xiface = oct_eth_info->interface;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	debug("%s(%s(%d.%d), %d)\n", __func__, dev->name, xiface, index, mode);

	/* For RGMII we need to properly shut down the XCV interface */
	if (cvmx_helper_bgx_is_rgmii(xiface, index)) {
		debug("  Shut down XCV RGMII\n");
		octeon_bgx_xcv_rgmii_enable(xi.interface, index, false);
	} else {
		cmr_config.u64 = csr_rd_node(xi.node,
					     CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		cmr_config.s.data_pkt_tx_en = 0;
		cmr_config.s.data_pkt_rx_en = 0;
		csr_wr_node(xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
			    cmr_config.u64);

		cmr_cam.u64 = csr_rd_node(xi.node,
					  CVMX_BGXX_CMR_RX_ADRX_CAM(index * 8, xi.interface));
		cmr_cam.s.en = 0;
		csr_wr_node(xi.node,
			    CVMX_BGXX_CMR_RX_ADRX_CAM(index * 8, xi.interface),
			    cmr_cam.u64);
		oct_eth_info->last_bgx_mac = 0;
		oct_eth_info->bgx_mac_set = 0;
	}
}

/**
 * Halts the specified Ethernet interface preventing it from receiving any more
 * packets.
 *
 * @param dev - Ethernet device to shut down.
 */
void octeon_eth_halt(struct udevice *dev)
{
	struct octeon_eth_info *oct_eth_info = dev_get_priv(dev);
	int index = oct_eth_info->index;
	int interface = oct_eth_info->interface;
	cvmx_helper_interface_mode_t mode;
	union cvmx_gmxx_rxx_adr_ctl adr_ctl;
	cvmx_gmxx_prtx_cfg_t gmx_cfg;
	u64 tmp;

	debug("%s(%s): Halting\n", __func__, dev->name);

	oct_eth_info->enabled = 0;

	mode = cvmx_helper_interface_get_mode(oct_eth_info->interface);
	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		octeon_eth_halt_bgx(dev, mode);
		return;
	}

	/* Stop SCC */
	/* Disable reception on this port at the GMX block */
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		debug("  RGMII/GMII\n");
		tmp = csr_rd(CVMX_ASXX_RX_PRT_EN(oct_eth_info->interface));
		tmp &= ~(1ull << index);
		/* Disable the RGMII RX ports */
		csr_wr(CVMX_ASXX_RX_PRT_EN(oct_eth_info->interface), tmp);
		tmp = csr_rd(CVMX_ASXX_TX_PRT_EN(oct_eth_info->interface));
		tmp &= ~(1ull << index);
		/* Disable the RGMII TX ports */
		csr_wr(CVMX_ASXX_TX_PRT_EN(oct_eth_info->interface), tmp);
		/* No break! */
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
	case CVMX_HELPER_INTERFACE_MODE_MIXED:
	case CVMX_HELPER_INTERFACE_MODE_AGL:
		/* Disable MAC filtering */
		gmx_cfg.u64 = csr_rd(oct_eth_info->gmx_base + GMX_PRT_CFG);
		csr_wr(oct_eth_info->gmx_base + GMX_PRT_CFG,
		       gmx_cfg.u64 & ~1ull);
		adr_ctl.u64 = 0;
		adr_ctl.s.bcst = 1; /* Reject broadcast */
		csr_wr(oct_eth_info->gmx_base + GMX_RX_ADR_CTL, adr_ctl.u64);
		csr_wr(oct_eth_info->gmx_base + GMX_RX_ADR_CAM_EN, 0);
		csr_wr(oct_eth_info->gmx_base + GMX_PRT_CFG, gmx_cfg.u64);
		break;
	default:
		printf("%s: Unknown mode %d for interface 0x%x:%d\n", __func__,
		       mode, interface, index);
		break;
	}
}

void nic_stop(struct udevice *dev)
{
	octeon_eth_halt(dev);
}

int nic_write_hwaddr(struct udevice *dev)
{
	cvm_oct_set_mac_address(dev);

	return 0;
}

static const struct eth_ops octeon_nic_ops = {
	.start = nic_open,
	.stop = nic_stop,
	.send = nic_xmit,
	.recv = nic_recv,
	.free_pkt = nic_free_pkt,
	.write_hwaddr = nic_write_hwaddr,
};

static const struct udevice_id octeon_nic_ids[] = {
	{ .compatible = "cavium,octeon-7890-bgx" },
	{}
};

U_BOOT_DRIVER(octeon_nic) = {
	.name = "octeon_nic",
	.id = UCLASS_ETH,
	.probe = octeon_nic_probe,
	.of_match = octeon_nic_ids,
	.ops = &octeon_nic_ops,
	.priv_auto = sizeof(struct octeon_eth_info),
};
