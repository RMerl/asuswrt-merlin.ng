// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * PKI helper functions.
 */

#include <time.h>
#include <log.h>
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

#include <mach/cvmx-agl-defs.h>
#include <mach/cvmx-bgxx-defs.h>
#include <mach/cvmx-ciu-defs.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-gserx-defs.h>
#include <mach/cvmx-ilk-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pexp-defs.h>
#include <mach/cvmx-pcsx-defs.h>
#include <mach/cvmx-pcsxx-defs.h>
#include <mach/cvmx-pki-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-sli-defs.h>
#include <mach/cvmx-xcv-defs.h>

#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pki.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-pki.h>

#include <mach/cvmx-global-resources.h>
#include <mach/cvmx-pko-internal-ports-range.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pip.h>

static int pki_helper_debug;

bool cvmx_pki_dflt_init[CVMX_MAX_NODES] = { [0 ... CVMX_MAX_NODES - 1] = 1 };

static bool cvmx_pki_dflt_bp_en[CVMX_MAX_NODES] = { [0 ... CVMX_MAX_NODES - 1] =
							    true };
static struct cvmx_pki_cluster_grp_config pki_dflt_clgrp[CVMX_MAX_NODES] = {
	{ 0, 0xf },
	{ 0, 0xf }
};

struct cvmx_pki_pool_config pki_dflt_pool[CVMX_MAX_NODES] = {
	[0 ... CVMX_MAX_NODES -
	 1] = { .pool_num = -1, .buffer_size = 2048, .buffer_count = 0 }
};

struct cvmx_pki_aura_config pki_dflt_aura[CVMX_MAX_NODES] = {
	[0 ... CVMX_MAX_NODES -
	 1] = { .aura_num = 0, .pool_num = -1, .buffer_count = 0 }
};

struct cvmx_pki_style_config pki_dflt_style[CVMX_MAX_NODES] = {
	[0 ... CVMX_MAX_NODES - 1] = { .parm_cfg = { .lenerr_en = 1,
						     .maxerr_en = 1,
						     .minerr_en = 1,
						     .fcs_strip = 1,
						     .fcs_chk = 1,
						     .first_skip = 40,
						     .mbuff_size = 2048 } }
};

struct cvmx_pki_sso_grp_config pki_dflt_sso_grp[CVMX_MAX_NODES];
struct cvmx_pki_qpg_config pki_dflt_qpg[CVMX_MAX_NODES];
struct cvmx_pki_pkind_config pki_dflt_pkind[CVMX_MAX_NODES];
u64 pkind_style_map[CVMX_MAX_NODES][CVMX_PKI_NUM_PKIND] = {
	[0 ... CVMX_MAX_NODES -
	 1] = { 0,  1,	2,  3,	4,  5,	6,  7,	8,  9,	10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 }
};

/* To store the qos watcher values before they are written to pcam when watcher
 * is enabled. There is no cvmx-pip.c file exist so it ended up here
 */
struct cvmx_pki_legacy_qos_watcher qos_watcher[8];

/** @INTERNAL
 * This function setsup default ltype map
 * @param node    node number
 */
void __cvmx_helper_pki_set_dflt_ltype_map(int node)
{
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_NONE,
				 CVMX_PKI_BELTYPE_NONE);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_ENET,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_VLAN,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_SNAP_PAYLD,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_ARP,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_RARP,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IP4,
				 CVMX_PKI_BELTYPE_IP4);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IP4_OPT,
				 CVMX_PKI_BELTYPE_IP4);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IP6,
				 CVMX_PKI_BELTYPE_IP6);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IP6_OPT,
				 CVMX_PKI_BELTYPE_IP6);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IPSEC_ESP,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IPFRAG,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_IPCOMP,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_TCP,
				 CVMX_PKI_BELTYPE_TCP);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_UDP,
				 CVMX_PKI_BELTYPE_UDP);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_SCTP,
				 CVMX_PKI_BELTYPE_SCTP);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_UDP_VXLAN,
				 CVMX_PKI_BELTYPE_UDP);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_GRE,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_NVGRE,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_GTP,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_SW28,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_SW29,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_SW30,
				 CVMX_PKI_BELTYPE_MISC);
	cvmx_pki_write_ltype_map(node, CVMX_PKI_LTYPE_E_SW31,
				 CVMX_PKI_BELTYPE_MISC);
}

/** @INTERNAL
 * This function installs the default VLAN entries to identify
 * the VLAN and set WQE[vv], WQE[vs] if VLAN is found. In 78XX
 * hardware (PKI) is not hardwired to recognize any 802.1Q VLAN
 * Ethertypes
 *
 * @param node    node number
 */
int __cvmx_helper_pki_install_dflt_vlan(int node)
{
	struct cvmx_pki_pcam_input pcam_input;
	struct cvmx_pki_pcam_action pcam_action;
	enum cvmx_pki_term field;
	int index;
	int bank;
	u64 cl_mask = CVMX_PKI_CLUSTER_ALL;

	memset(&pcam_input, 0, sizeof(pcam_input));
	memset(&pcam_action, 0, sizeof(pcam_action));

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		/* PKI-20858 */
		int i;

		for (i = 0; i < 4; i++) {
			union cvmx_pki_clx_ecc_ctl ecc_ctl;

			ecc_ctl.u64 =
				csr_rd_node(node, CVMX_PKI_CLX_ECC_CTL(i));
			ecc_ctl.s.pcam_en = 0;
			ecc_ctl.s.pcam0_cdis = 1;
			ecc_ctl.s.pcam1_cdis = 1;
			csr_wr_node(node, CVMX_PKI_CLX_ECC_CTL(i), ecc_ctl.u64);
		}
	}

	for (field = CVMX_PKI_PCAM_TERM_ETHTYPE0;
	     field < CVMX_PKI_PCAM_TERM_ETHTYPE2; field++) {
		bank = field & 0x01;

		index = cvmx_pki_pcam_entry_alloc(
			node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			debug("ERROR: Allocating pcam entry node=%d bank=%d\n",
			      node, bank);
			return -1;
		}
		pcam_input.style = 0;
		pcam_input.style_mask = 0;
		pcam_input.field = field;
		pcam_input.field_mask = 0xfd;
		pcam_input.data = 0x81000000;
		pcam_input.data_mask = 0xffff0000;
		pcam_action.parse_mode_chg = CVMX_PKI_PARSE_NO_CHG;
		pcam_action.layer_type_set = CVMX_PKI_LTYPE_E_VLAN;
		pcam_action.style_add = 0;
		pcam_action.pointer_advance = 4;
		cvmx_pki_pcam_write_entry(
			node, index, cl_mask, pcam_input,
			pcam_action); /*cluster_mask in pass2*/

		index = cvmx_pki_pcam_entry_alloc(
			node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			debug("ERROR: Allocating pcam entry node=%d bank=%d\n",
			      node, bank);
			return -1;
		}
		pcam_input.data = 0x88a80000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input,
					  pcam_action);

		index = cvmx_pki_pcam_entry_alloc(
			node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			debug("ERROR: Allocating pcam entry node=%d bank=%d\n",
			      node, bank);
			return -1;
		}
		pcam_input.data = 0x92000000;
		cvmx_pki_pcam_write_entry(
			node, index, cl_mask, pcam_input,
			pcam_action); /* cluster_mask in pass2*/

		index = cvmx_pki_pcam_entry_alloc(
			node, CVMX_PKI_FIND_AVAL_ENTRY, bank, cl_mask);
		if (index < 0) {
			debug("ERROR: Allocating pcam entry node=%d bank=%d\n",
			      node, bank);
			return -1;
		}
		pcam_input.data = 0x91000000;
		cvmx_pki_pcam_write_entry(node, index, cl_mask, pcam_input,
					  pcam_action);
	}
	return 0;
}

static int __cvmx_helper_setup_pki_cluster_groups(int node)
{
	u64 cl_mask;
	int cl_group;

	cl_group =
		cvmx_pki_cluster_grp_alloc(node, pki_dflt_clgrp[node].grp_num);
	if (cl_group == CVMX_RESOURCE_ALLOC_FAILED)
		return -1;
	else if (cl_group == CVMX_RESOURCE_ALREADY_RESERVED) {
		if (pki_dflt_clgrp[node].grp_num == -1)
			return -1;
		else
			return 0; /* cluster already configured, share it */
	}
	cl_mask = pki_dflt_clgrp[node].cluster_mask;
	if (pki_helper_debug)
		debug("pki-helper: setup pki cluster grp %d with cl_mask 0x%llx\n",
		      (int)cl_group, (unsigned long long)cl_mask);
	cvmx_pki_attach_cluster_to_group(node, cl_group, cl_mask);
	return 0;
}

/**
 * This function sets up pools/auras to be used by PKI
 * @param node    node number
 */
static int __cvmx_helper_pki_setup_fpa_pools(int node)
{
	u64 buffer_count;
	u64 buffer_size;

	if (__cvmx_fpa3_aura_valid(pki_dflt_aura[node].aura))
		return 0; /* aura already configured, share it */

	buffer_count = pki_dflt_pool[node].buffer_count;
	buffer_size = pki_dflt_pool[node].buffer_size;

	if (buffer_count != 0) {
		pki_dflt_pool[node].pool = cvmx_fpa3_setup_fill_pool(
			node, pki_dflt_pool[node].pool_num, "PKI POOL DFLT",
			buffer_size, buffer_count, NULL);
		if (!__cvmx_fpa3_pool_valid(pki_dflt_pool[node].pool)) {
			cvmx_printf("ERROR: %s: Failed to allocate pool %d\n",
				    __func__, pki_dflt_pool[node].pool_num);
			return -1;
		}
		pki_dflt_pool[node].pool_num = pki_dflt_pool[node].pool.lpool;

		if (pki_helper_debug)
			debug("%s pool %d with buffer size %d cnt %d\n",
			      __func__, pki_dflt_pool[node].pool_num,
			      (int)buffer_size, (int)buffer_count);

		pki_dflt_aura[node].pool_num = pki_dflt_pool[node].pool_num;
		pki_dflt_aura[node].pool = pki_dflt_pool[node].pool;
	}

	buffer_count = pki_dflt_aura[node].buffer_count;

	if (buffer_count != 0) {
		pki_dflt_aura[node].aura = cvmx_fpa3_set_aura_for_pool(
			pki_dflt_aura[node].pool, pki_dflt_aura[node].aura_num,
			"PKI DFLT AURA", buffer_size, buffer_count);

		if (!__cvmx_fpa3_aura_valid(pki_dflt_aura[node].aura)) {
			debug("ERROR: %sL Failed to allocate aura %d\n",
			      __func__, pki_dflt_aura[node].aura_num);
			return -1;
		}
	}
	return 0;
}

static int __cvmx_helper_setup_pki_qpg_table(int node)
{
	int offset;

	offset = cvmx_pki_qpg_entry_alloc(node, pki_dflt_qpg[node].qpg_base, 1);
	if (offset == CVMX_RESOURCE_ALLOC_FAILED)
		return -1;
	else if (offset == CVMX_RESOURCE_ALREADY_RESERVED)
		return 0; /* share the qpg table entry */
	if (pki_helper_debug)
		debug("pki-helper: set qpg entry at offset %d with port add %d aura %d grp_ok %d grp_bad %d\n",
		      offset, pki_dflt_qpg[node].port_add,
		      pki_dflt_qpg[node].aura_num, pki_dflt_qpg[node].grp_ok,
		      pki_dflt_qpg[node].grp_bad);
	cvmx_pki_write_qpg_entry(node, offset, &pki_dflt_qpg[node]);
	return 0;
}

int __cvmx_helper_pki_port_setup(int node, int ipd_port)
{
	int xiface, index;
	int pknd, style_num;
	int rs;
	struct cvmx_pki_pkind_config pkind_cfg;

	if (!cvmx_pki_dflt_init[node])
		return 0;
	xiface = cvmx_helper_get_interface_num(ipd_port);
	index = cvmx_helper_get_interface_index_num(ipd_port);

	pknd = cvmx_helper_get_pknd(xiface, index);
	style_num = pkind_style_map[node][pknd];

	/* try to reserve the style, if it is not configured already, reserve
	and configure it */
	rs = cvmx_pki_style_alloc(node, style_num);
	if (rs < 0) {
		if (rs == CVMX_RESOURCE_ALLOC_FAILED)
			return -1;
	} else {
		if (pki_helper_debug)
			debug("pki-helper: set style %d with default parameters\n",
			      style_num);
		pkind_style_map[node][pknd] = style_num;
		/* configure style with default parameters */
		cvmx_pki_write_style_config(node, style_num,
					    CVMX_PKI_CLUSTER_ALL,
					    &pki_dflt_style[node]);
	}
	if (pki_helper_debug)
		debug("pki-helper: set pkind %d with initial style %d\n", pknd,
		      style_num);
	/* write pkind configuration */
	pkind_cfg = pki_dflt_pkind[node];
	pkind_cfg.initial_style = style_num;
	cvmx_pki_write_pkind_config(node, pknd, &pkind_cfg);
	return 0;
}

int __cvmx_helper_pki_global_setup(int node)
{
	__cvmx_helper_pki_set_dflt_ltype_map(node);
	if (!cvmx_pki_dflt_init[node])
		return 0;
	/* Setup the packet pools*/
	__cvmx_helper_pki_setup_fpa_pools(node);
	/*set up default cluster*/
	__cvmx_helper_setup_pki_cluster_groups(node);
	//__cvmx_helper_pki_setup_sso_groups(node);
	__cvmx_helper_setup_pki_qpg_table(node);
	/*
	 * errata PKI-19103 backward compat has only 1 aura
	 * no head line blocking
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		cvmx_pki_buf_ctl_t buf_ctl;

		buf_ctl.u64 = csr_rd_node(node, CVMX_PKI_BUF_CTL);
		buf_ctl.s.fpa_wait = 1;
		csr_wr_node(node, CVMX_PKI_BUF_CTL, buf_ctl.u64);
	}
	return 0;
}

/**
 * This function Enabled the PKI hardware to
 * start accepting/processing packets.
 *
 * @param node    node number
 */
void cvmx_helper_pki_enable(int node)
{
	if (pki_helper_debug)
		debug("enable PKI on node %d\n", node);
	__cvmx_helper_pki_install_dflt_vlan(node);
	cvmx_pki_setup_clusters(node);
	if (cvmx_pki_dflt_bp_en[node])
		cvmx_pki_enable_backpressure(node);
	cvmx_pki_parse_enable(node, 0);
	cvmx_pki_enable(node);
}

/**
 * This function setups the qos table by allocating qpg entry and writing
 * the provided parameters to that entry (offset).
 * @param node	  node number.
 * @param qpg_cfg       pointer to struct containing qpg configuration
 */
int cvmx_helper_pki_set_qpg_entry(int node, struct cvmx_pki_qpg_config *qpg_cfg)
{
	int offset;

	offset = cvmx_pki_qpg_entry_alloc(node, qpg_cfg->qpg_base, 1);
	if (pki_helper_debug)
		debug("pki-helper:set qpg entry at offset %d\n", offset);
	if (offset == CVMX_RESOURCE_ALREADY_RESERVED) {
		debug("INFO:setup_qpg_table: offset %d already reserved\n",
		      qpg_cfg->qpg_base);
		return CVMX_RESOURCE_ALREADY_RESERVED;
	} else if (offset == CVMX_RESOURCE_ALLOC_FAILED) {
		debug("ERROR:setup_qpg_table: no more entries available\n");
		return CVMX_RESOURCE_ALLOC_FAILED;
	}
	qpg_cfg->qpg_base = offset;
	cvmx_pki_write_qpg_entry(node, offset, qpg_cfg);
	return offset;
}

/**
 * This function gets all the PKI parameters related to that
 * particular port from hardware.
 * @param xipd_port	xipd_port port number with node to get parameter of
 * @param port_cfg	pointer to structure where to store read parameters
 */
void cvmx_pki_get_port_config(int xipd_port,
			      struct cvmx_pki_port_config *port_cfg)
{
	int xiface, index, pknd;
	int style, cl_mask;
	cvmx_pki_icgx_cfg_t pki_cl_msk;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);

	/* get the pkind used by this ipd port */
	xiface = cvmx_helper_get_interface_num(xipd_port);
	index = cvmx_helper_get_interface_index_num(xipd_port);
	pknd = cvmx_helper_get_pknd(xiface, index);

	cvmx_pki_read_pkind_config(xp.node, pknd, &port_cfg->pkind_cfg);
	style = port_cfg->pkind_cfg.initial_style;
	pki_cl_msk.u64 = csr_rd_node(
		xp.node, CVMX_PKI_ICGX_CFG(port_cfg->pkind_cfg.cluster_grp));
	cl_mask = pki_cl_msk.s.clusters;
	cvmx_pki_read_style_config(xp.node, style, cl_mask,
				   &port_cfg->style_cfg);
}

/**
 * This function sets all the PKI parameters related to that
 * particular port in hardware.
 * @param xipd_port	ipd port number with node to get parameter of
 * @param port_cfg	pointer to structure containing port parameters
 */
void cvmx_pki_set_port_config(int xipd_port,
			      struct cvmx_pki_port_config *port_cfg)
{
	int xiface, index, pknd;
	int style, cl_mask;
	cvmx_pki_icgx_cfg_t pki_cl_msk;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);

	/* get the pkind used by this ipd port */
	xiface = cvmx_helper_get_interface_num(xipd_port);
	index = cvmx_helper_get_interface_index_num(xipd_port);
	pknd = cvmx_helper_get_pknd(xiface, index);

	if (cvmx_pki_write_pkind_config(xp.node, pknd, &port_cfg->pkind_cfg))
		return;
	style = port_cfg->pkind_cfg.initial_style;
	pki_cl_msk.u64 = csr_rd_node(
		xp.node, CVMX_PKI_ICGX_CFG(port_cfg->pkind_cfg.cluster_grp));
	cl_mask = pki_cl_msk.s.clusters;
	cvmx_pki_write_style_config(xp.node, style, cl_mask,
				    &port_cfg->style_cfg);
}

/**
 * This function sets up all th eports of particular interface
 * for chosen fcs mode. (only use for backward compatibility).
 * New application can control it via init_interface calls.
 * @param node		node number.
 * @param interface	interface number.
 * @param nports	number of ports
 * @param has_fcs	1 -- enable fcs check and fcs strip.
 *			0 -- disable fcs check.
 */
void cvmx_helper_pki_set_fcs_op(int node, int interface, int nports,
				int has_fcs)
{
	int xiface, index;
	int pknd;
	unsigned int cluster = 0;
	cvmx_pki_clx_pkindx_cfg_t pkind_cfg;

	xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	for (index = 0; index < nports; index++) {
		pknd = cvmx_helper_get_pknd(xiface, index);
		while (cluster < CVMX_PKI_NUM_CLUSTER) {
			/*find the cluster in use pass2*/
			pkind_cfg.u64 = csr_rd_node(
				node, CVMX_PKI_CLX_PKINDX_CFG(pknd, cluster));
			pkind_cfg.s.fcs_pres = has_fcs;
			csr_wr_node(node,
				    CVMX_PKI_CLX_PKINDX_CFG(pknd, cluster),
				    pkind_cfg.u64);
			cluster++;
		}
		/* make sure fcs_strip and fcs_check is also enable/disable
		 * for the style used by that port
		 */
		cvmx_pki_endis_fcs_check(node, pknd, has_fcs, has_fcs);
		cluster = 0;
	}
}
