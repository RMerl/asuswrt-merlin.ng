// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * IPD helper functions.
 */

#include <errno.h>
#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr.h>
#include <mach/cvmx-bootmem.h>
#include <mach/octeon-model.h>
#include <mach/cvmx-fuse.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-qlm.h>
#include <mach/octeon_qlm.h>
#include <mach/cvmx-pcie.h>
#include <mach/cvmx-coremask.h>
#include <mach/cvmx-range.h>
#include <mach/cvmx-global-resources.h>

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-ipd.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pko3.h>
#include <mach/cvmx-pko3-queue.h>
#include <mach/cvmx-pko3-resources.h>
#include <mach/cvmx-pip.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

/** It allocate pools for packet and wqe pools
 * and sets up the FPA hardware
 */
int __cvmx_helper_ipd_setup_fpa_pools(void)
{
	cvmx_fpa_global_initialize();
	if (cvmx_ipd_cfg.packet_pool.buffer_count == 0)
		return 0;
	__cvmx_helper_initialize_fpa_pool(cvmx_ipd_cfg.packet_pool.pool_num,
					  cvmx_ipd_cfg.packet_pool.buffer_size,
					  cvmx_ipd_cfg.packet_pool.buffer_count,
					  "Packet Buffers");
	if (cvmx_ipd_cfg.wqe_pool.buffer_count == 0)
		return 0;
	__cvmx_helper_initialize_fpa_pool(cvmx_ipd_cfg.wqe_pool.pool_num,
					  cvmx_ipd_cfg.wqe_pool.buffer_size,
					  cvmx_ipd_cfg.wqe_pool.buffer_count,
					  "WQE Buffers");
	return 0;
}

/**
 * @INTERNAL
 * Setup global setting for IPD/PIP not related to a specific
 * interface or port. This must be called before IPD is enabled.
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_ipd_global_setup(void)
{
	/* Setup the packet and wqe pools*/
	__cvmx_helper_ipd_setup_fpa_pools();
	/* Setup the global packet input options */
	cvmx_ipd_config(cvmx_ipd_cfg.packet_pool.buffer_size / 8,
			cvmx_ipd_cfg.first_mbuf_skip / 8,
			cvmx_ipd_cfg.not_first_mbuf_skip / 8,
			/* The +8 is to account for the next ptr */
			(cvmx_ipd_cfg.first_mbuf_skip + 8) / 128,
			/* The +8 is to account for the next ptr */
			(cvmx_ipd_cfg.not_first_mbuf_skip + 8) / 128,
			cvmx_ipd_cfg.wqe_pool.pool_num,
			(cvmx_ipd_mode_t)(cvmx_ipd_cfg.cache_mode), 1);
	return 0;
}

/**
 * Enable or disable FCS stripping for all the ports on an interface.
 *
 * @param xiface
 * @param nports number of ports
 * @param has_fcs 0 for disable and !0 for enable
 */
static int cvmx_helper_fcs_op(int xiface, int nports, int has_fcs)
{
	u64 port_bit;
	int index;
	int pknd;
	union cvmx_pip_sub_pkind_fcsx pkind_fcsx;
	union cvmx_pip_prt_cfgx port_cfg;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!octeon_has_feature(OCTEON_FEATURE_PKND))
		return 0;
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		cvmx_helper_pki_set_fcs_op(xi.node, xi.interface, nports,
					   has_fcs);
		return 0;
	}

	port_bit = 0;
	for (index = 0; index < nports; index++)
		port_bit |= ((u64)1 << cvmx_helper_get_pknd(xiface, index));

	pkind_fcsx.u64 = csr_rd(CVMX_PIP_SUB_PKIND_FCSX(0));
	if (has_fcs)
		pkind_fcsx.s.port_bit |= port_bit;
	else
		pkind_fcsx.s.port_bit &= ~port_bit;
	csr_wr(CVMX_PIP_SUB_PKIND_FCSX(0), pkind_fcsx.u64);

	for (pknd = 0; pknd < 64; pknd++) {
		if ((1ull << pknd) & port_bit) {
			port_cfg.u64 = csr_rd(CVMX_PIP_PRT_CFGX(pknd));
			port_cfg.s.crc_en = (has_fcs) ? 1 : 0;
			csr_wr(CVMX_PIP_PRT_CFGX(pknd), port_cfg.u64);
		}
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure the IPD/PIP tagging and QoS options for a specific
 * port. This function determines the POW work queue entry
 * contents for a port. The setup performed here is controlled by
 * the defines in executive-config.h.
 *
 * @param ipd_port Port/Port kind to configure. This follows the IPD numbering,
 *                 not the per interface numbering
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_ipd_port_setup(int ipd_port)
{
	union cvmx_pip_prt_cfgx port_config;
	union cvmx_pip_prt_tagx tag_config;

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int xiface, index, pknd;
		union cvmx_pip_prt_cfgbx prt_cfgbx;

		xiface = cvmx_helper_get_interface_num(ipd_port);
		index = cvmx_helper_get_interface_index_num(ipd_port);
		pknd = cvmx_helper_get_pknd(xiface, index);

		port_config.u64 = csr_rd(CVMX_PIP_PRT_CFGX(pknd));
		tag_config.u64 = csr_rd(CVMX_PIP_PRT_TAGX(pknd));

		port_config.s.qos = pknd & 0x7;

		/* Default BPID to use for packets on this port-kind */
		prt_cfgbx.u64 = csr_rd(CVMX_PIP_PRT_CFGBX(pknd));
		prt_cfgbx.s.bpid = pknd;
		csr_wr(CVMX_PIP_PRT_CFGBX(pknd), prt_cfgbx.u64);
	} else {
		port_config.u64 = csr_rd(CVMX_PIP_PRT_CFGX(ipd_port));
		tag_config.u64 = csr_rd(CVMX_PIP_PRT_TAGX(ipd_port));

		/* Have each port go to a different POW queue */
		port_config.s.qos = ipd_port & 0x7;
	}

	/* Process the headers and place the IP header in the work queue */
	port_config.s.mode =
		(cvmx_pip_port_parse_mode_t)cvmx_ipd_cfg.port_config.parse_mode;

	tag_config.s.ip6_src_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv6_src_ip;
	tag_config.s.ip6_dst_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv6_dst_ip;
	tag_config.s.ip6_sprt_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv6_src_port;
	tag_config.s.ip6_dprt_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv6_dst_port;
	tag_config.s.ip6_nxth_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv6_next_header;
	tag_config.s.ip4_src_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv4_src_ip;
	tag_config.s.ip4_dst_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv4_dst_ip;
	tag_config.s.ip4_sprt_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv4_src_port;
	tag_config.s.ip4_dprt_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv4_dst_port;
	tag_config.s.ip4_pctl_flag =
		cvmx_ipd_cfg.port_config.tag_fields.ipv4_protocol;
	tag_config.s.inc_prt_flag =
		cvmx_ipd_cfg.port_config.tag_fields.input_port;
	tag_config.s.tcp6_tag_type =
		(cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.tcp4_tag_type =
		(cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.ip6_tag_type =
		(cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.ip4_tag_type =
		(cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.non_tag_type =
		(cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;

	/* Put all packets in group 0. Other groups can be used by the app */
	tag_config.s.grp = 0;

	cvmx_pip_config_port(ipd_port, port_config, tag_config);

	/* Give the user a chance to override our setting for each port */
	if (cvmx_override_ipd_port_setup)
		cvmx_override_ipd_port_setup(ipd_port);

	return 0;
}

/**
 * @INTERNAL
 * Setup the IPD/PIP for the ports on an interface. Packet
 * classification and tagging are set for every port on the
 * interface. The number of ports on the interface must already
 * have been probed.
 *
 * @param xiface to setup IPD/PIP for
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_ipd_setup_interface(int xiface)
{
	cvmx_helper_interface_mode_t mode;
	int num_ports = cvmx_helper_ports_on_interface(xiface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int ipd_port = cvmx_helper_get_ipd_port(xiface, 0);
	int delta;

	if (num_ports == CVMX_HELPER_CFG_INVALID_VALUE)
		return 0;

	delta = 1;
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		if (xi.interface < CVMX_HELPER_MAX_GMX)
			delta = 16;
	}

	while (num_ports--) {
		if (!cvmx_helper_is_port_valid(xiface, num_ports))
			continue;
		if (octeon_has_feature(OCTEON_FEATURE_PKI))
			__cvmx_helper_pki_port_setup(xi.node, ipd_port);
		else
			__cvmx_helper_ipd_port_setup(ipd_port);
		ipd_port += delta;
	}
	/* FCS settings */
	cvmx_helper_fcs_op(xiface, cvmx_helper_ports_on_interface(xiface),
			   __cvmx_helper_get_has_fcs(xiface));

	mode = cvmx_helper_interface_get_mode(xiface);

	if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP)
		__cvmx_helper_loop_enable(xiface);

	return 0;
}
