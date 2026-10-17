/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Functions to configure the BGX MAC.
 */

#ifndef __CVMX_HELPER_BGX_H__
#define __CVMX_HELPER_BGX_H__

#define CVMX_BGX_RX_FIFO_SIZE (64 * 1024)
#define CVMX_BGX_TX_FIFO_SIZE (32 * 1024)

int __cvmx_helper_bgx_enumerate(int xiface);

/**
 * @INTERNAL
 * Disable the BGX port
 *
 * @param xipd_port IPD port of the BGX interface to disable
 */
void cvmx_helper_bgx_disable(int xipd_port);

/**
 * @INTERNAL
 * Probe a SGMII interface and determine the number of ports
 * connected to it. The SGMII/XAUI interface should still be down after
 * this call. This is used by interfaces using the bgx mac.
 *
 * @param xiface Interface to probe
 *
 * Return: Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_bgx_probe(int xiface);

/**
 * @INTERNAL
 * Bringup and enable a SGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using the
 * bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_enable(int xiface);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t __cvmx_helper_bgx_sgmii_link_get(int xipd_port);

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_link_set(int xipd_port, cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * Return: Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_sgmii_configure_loopback(int xipd_port, int enable_internal,
					       int enable_external);

/**
 * @INTERNAL
 * Bringup and enable a XAUI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using the
 * bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_bgx_xaui_enable(int xiface);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * Return: Link state
 */
cvmx_helper_link_info_t __cvmx_helper_bgx_xaui_link_get(int xipd_port);

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_bgx_xaui_link_set(int xipd_port, cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * Return: Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_xaui_configure_loopback(int xipd_port, int enable_internal,
					      int enable_external);

int __cvmx_helper_bgx_mixed_enable(int xiface);

cvmx_helper_link_info_t __cvmx_helper_bgx_mixed_link_get(int xipd_port);

int __cvmx_helper_bgx_mixed_link_set(int xipd_port, cvmx_helper_link_info_t link_info);

int __cvmx_helper_bgx_mixed_configure_loopback(int xipd_port, int enable_internal,
					       int enable_external);

cvmx_helper_interface_mode_t cvmx_helper_bgx_get_mode(int xiface, int index);

/**
 * @INTERNAL
 * Configure Priority-Based Flow Control (a.k.a. PFC/CBFC)
 * on a specific BGX interface/port.
 */
void __cvmx_helper_bgx_xaui_config_pfc(unsigned int node, unsigned int interface, unsigned int port,
				       bool pfc_enable);

/**
 * This function control how the hardware handles incoming PAUSE
 * packets. The most common modes of operation:
 * ctl_bck = 1, ctl_drp = 1: hardware handles everything
 * ctl_bck = 0, ctl_drp = 0: software sees all PAUSE frames
 * ctl_bck = 0, ctl_drp = 1: all PAUSE frames are completely ignored
 * @param node		node number.
 * @param interface	interface number
 * @param port		port number
 * @param ctl_bck	1: Forward PAUSE information to TX block
 * @param ctl_drp	1: Drop control PAUSE frames.
 */
void cvmx_helper_bgx_rx_pause_ctl(unsigned int node, unsigned int interface, unsigned int port,
				  unsigned int ctl_bck, unsigned int ctl_drp);

/**
 * This function configures the receive action taken for multicast, broadcast
 * and dmac filter match packets.
 * @param node		node number.
 * @param interface	interface number
 * @param port		port number
 * @param cam_accept	0: reject packets on dmac filter match
 *                      1: accept packet on dmac filter match
 * @param mcast_mode	0x0 = Force reject all multicast packets
 *                      0x1 = Force accept all multicast packets
 *                      0x2 = Use the address filter CAM
 * @param bcast_accept  0 = Reject all broadcast packets
 *                      1 = Accept all broadcast packets
 */
void cvmx_helper_bgx_rx_adr_ctl(unsigned int node, unsigned int interface, unsigned int port,
				unsigned int cam_accept, unsigned int mcast_mode,
				unsigned int bcast_accept);

/**
 * Function to control the generation of FCS, padding by the BGX
 *
 */
void cvmx_helper_bgx_tx_options(unsigned int node, unsigned int interface, unsigned int index,
				bool fcs_enable, bool pad_enable);

/**
 * Set mac for the ipd_port
 *
 * @param xipd_port ipd_port to set the mac
 * @param bcst      If set, accept all broadcast packets
 * @param mcst      Multicast mode
 *		    0 = Force reject all multicast packets
 *		    1 = Force accept all multicast packets
 *		    2 = use the address filter CAM.
 * @param mac       mac address for the ipd_port
 */
void cvmx_helper_bgx_set_mac(int xipd_port, int bcst, int mcst, u64 mac);

int __cvmx_helper_bgx_port_init(int xipd_port, int phy_pres);
void cvmx_helper_bgx_set_jabber(int xiface, unsigned int index, unsigned int size);
int cvmx_helper_bgx_shutdown_port(int xiface, int index);
int cvmx_bgx_set_backpressure_override(int xiface, unsigned int port_mask);
int __cvmx_helper_bgx_fifo_size(int xiface, unsigned int lmac);

/**
 * Returns if an interface is RGMII or not
 *
 * @param xiface	xinterface to check
 * @param index		port index (must be 0 for rgmii)
 *
 * Return:	true if RGMII, false otherwise
 */
static inline bool cvmx_helper_bgx_is_rgmii(int xiface, int index)
{
	union cvmx_bgxx_cmrx_config cmr_config;

	if (!OCTEON_IS_MODEL(OCTEON_CN73XX) || index != 0)
		return false;
	cmr_config.u64 = csr_rd(CVMX_BGXX_CMRX_CONFIG(index, xiface));
	return cmr_config.s.lmac_type == 5;
}

/**
 * Probes the BGX Super Path (SMU/SPU) mode
 *
 * @param xiface	global interface number
 * @param index		interface index
 *
 * Return:	true, if Super-MAC/PCS mode, false -- otherwise
 */
bool cvmx_helper_bgx_is_smu(int xiface, int index);

/**
 * @INTERNAL
 * Configure parameters of PAUSE packet.
 *
 * @param xipd_port		Global IPD port (node + IPD port).
 * @param smac			Source MAC address.
 * @param dmac			Destination MAC address.
 * @param type			PAUSE packet type.
 * @param time			Pause time for PAUSE packets (number of 512 bit-times).
 * @param interval		Interval between PAUSE packets (number of 512 bit-times).
 * Return: Zero on success, negative on failure.
 */
int cvmx_bgx_set_pause_pkt_param(int xipd_port, u64 smac, u64 dmac, unsigned int type,
				 unsigned int time, unsigned int interval);

/**
 * @INTERNAL
 * Setup the BGX flow-control mode.
 *
 * @param xipd_port		Global IPD port (node + IPD port).
 * @param type			Flow-control type/protocol.
 * @param mode			Flow-control mode.
 * Return: Zero on success, negative on failure.
 */
int cvmx_bgx_set_flowctl_mode(int xipd_port, cvmx_qos_proto_t qos, cvmx_qos_pkt_mode_t mode);

/**
 * Enables or disables autonegotiation for an interface.
 *
 * @param	xiface	interface to set autonegotiation
 * @param	index	port index
 * @param	enable	true to enable autonegotiation, false to disable it
 *
 * Return:	0 for success, -1 on error.
 */
int cvmx_helper_set_autonegotiation(int xiface, int index, bool enable);

/**
 * Enables or disables forward error correction
 *
 * @param	xiface	interface
 * @param	index	port index
 * @param	enable	set to true to enable FEC, false to disable
 *
 * Return:	0 for success, -1 on error
 *
 * @NOTE:	If autonegotiation is enabled then autonegotiation will be
 *		restarted for negotiating FEC.
 */
int cvmx_helper_set_fec(int xiface, int index, bool enable);

#ifdef CVMX_DUMP_BGX
/**
 * Dump BGX configuration for node 0
 */
int cvmx_dump_bgx_config(unsigned int bgx);
/**
 * Dump BGX status for node 0
 */
int cvmx_dump_bgx_status(unsigned int bgx);
/**
 * Dump BGX configuration
 */
int cvmx_dump_bgx_config_node(unsigned int node, unsigned int bgx);
/**
 * Dump BGX status
 */
int cvmx_dump_bgx_status_node(unsigned int node, unsigned int bgx);

#endif

#endif
