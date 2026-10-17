// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020-2022 Marvell International Ltd.
 *
 * Helper Functions for the Configuration Framework
 */

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
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pki-defs.h>

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-fdt.h>
#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

#include <mach/cvmx-global-resources.h>
#include <mach/cvmx-pko-internal-ports-range.h>
#include <mach/cvmx-ilk.h>
#include <mach/cvmx-pip.h>

DECLARE_GLOBAL_DATA_PTR;

int cvmx_npi_max_pknds;
static bool port_cfg_data_initialized;

struct cvmx_cfg_port_param cvmx_cfg_port[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE]
					[CVMX_HELPER_CFG_MAX_PORT_PER_IFACE];
/*
 * Indexed by the pko_port number
 */
static int __cvmx_cfg_pko_highest_queue;
struct cvmx_cfg_pko_port_param
cvmx_pko_queue_table[CVMX_HELPER_CFG_MAX_PKO_PORT] = {
	[0 ... CVMX_HELPER_CFG_MAX_PKO_PORT - 1] = {
		CVMX_HELPER_CFG_INVALID_VALUE,
		CVMX_HELPER_CFG_INVALID_VALUE
	}
};

cvmx_user_static_pko_queue_config_t
__cvmx_pko_queue_static_config[CVMX_MAX_NODES];

struct cvmx_cfg_pko_port_map
cvmx_cfg_pko_port_map[CVMX_HELPER_CFG_MAX_PKO_PORT] = {
	[0 ... CVMX_HELPER_CFG_MAX_PKO_PORT - 1] = {
		CVMX_HELPER_CFG_INVALID_VALUE,
		CVMX_HELPER_CFG_INVALID_VALUE,
		CVMX_HELPER_CFG_INVALID_VALUE
	}
};

/*
 * This array assists translation from ipd_port to pko_port.
 * The ``16'' is the rounded value for the 3rd 4-bit value of
 * ipd_port, used to differentiate ``interfaces.''
 */
static struct cvmx_cfg_pko_port_pair
ipd2pko_port_cache[16][CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
	[0 ... 15] = {
		[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] = {
			CVMX_HELPER_CFG_INVALID_VALUE,
			CVMX_HELPER_CFG_INVALID_VALUE
		}
	}
};

/*
 * Options
 *
 * Each array-elem's initial value is also the option's default value.
 */
static u64 cvmx_cfg_opts[CVMX_HELPER_CFG_OPT_MAX] = {
	[0 ... CVMX_HELPER_CFG_OPT_MAX - 1] = 1
};

/*
 * MISC
 */

static int cvmx_cfg_max_pko_engines; /* # of PKO DMA engines allocated */
static int cvmx_pko_queue_alloc(u64 port, int count);
static void cvmx_init_port_cfg(void);

int __cvmx_helper_cfg_pknd(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int pkind;

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();

	/*
	 * Only 8 PKNDs are assigned to ILK channels. The channels are wrapped
	 * if more than 8 channels are configured, fix the index accordingly.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if (cvmx_helper_interface_get_mode(xiface) ==
		    CVMX_HELPER_INTERFACE_MODE_ILK)
			index %= 8;
	}

	pkind = cvmx_cfg_port[xi.node][xi.interface][index].ccpp_pknd;
	return pkind;
}

int __cvmx_helper_cfg_bpid(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();

	/*
	 * Only 8 BIDs are assigned to ILK channels. The channels are wrapped
	 * if more than 8 channels are configured, fix the index accordingly.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if (cvmx_helper_interface_get_mode(xiface) ==
		    CVMX_HELPER_INTERFACE_MODE_ILK)
			index %= 8;
	}

	return cvmx_cfg_port[xi.node][xi.interface][index].ccpp_bpid;
}

int __cvmx_helper_cfg_pko_port_base(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();

	return cvmx_cfg_port[xi.node][xi.interface][index].ccpp_pko_port_base;
}

int __cvmx_helper_cfg_pko_port_num(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();

	return cvmx_cfg_port[xi.node][xi.interface][index].ccpp_pko_num_ports;
}

int __cvmx_helper_cfg_pko_queue_num(int pko_port)
{
	return cvmx_pko_queue_table[pko_port].ccppp_num_queues;
}

int __cvmx_helper_cfg_pko_queue_base(int pko_port)
{
	return cvmx_pko_queue_table[pko_port].ccppp_queue_base;
}

int __cvmx_helper_cfg_pko_max_queue(void)
{
	return __cvmx_cfg_pko_highest_queue;
}

int __cvmx_helper_cfg_pko_max_engine(void)
{
	return cvmx_cfg_max_pko_engines;
}

uint64_t cvmx_helper_cfg_opt_get(cvmx_helper_cfg_option_t opt)
{
	if (opt >= CVMX_HELPER_CFG_OPT_MAX)
		return (uint64_t)CVMX_HELPER_CFG_INVALID_VALUE;

	return cvmx_cfg_opts[opt];
}

/*
 * initialize the queue allocation list. the existing static allocation result
 * is used as a starting point to ensure backward compatibility.
 *
 * Return:  0 on success
 *         -1 on failure
 */
int cvmx_pko_queue_grp_alloc(u64 start, uint64_t end, uint64_t count)
{
	u64 port;
	int ret_val;

	for (port = start; port < end; port++) {
		ret_val = cvmx_pko_queue_alloc(port, count);
		if (ret_val == -1) {
			printf("ERROR: %sL Failed to allocate queue for port=%d count=%d\n",
			       __func__, (int)port, (int)count);
			return ret_val;
		}
	}
	return 0;
}

int cvmx_pko_queue_init_from_cvmx_config_non_pknd(void)
{
	int ret_val = -1;
	u64 count, start, end;

	start = 0;
	end = __cvmx_pko_queue_static_config[0].non_pknd.pko_ports_per_interface[0];
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_interface[0];
	cvmx_pko_queue_grp_alloc(start, end, count);

	start = 16;
	end = start + __cvmx_pko_queue_static_config[0].non_pknd.pko_ports_per_interface[1];
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_interface[1];
	ret_val = cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		/* Interface 4: AGL, PKO port 24 only, DPI 32-35 */
		start = 24;
		end = start + 1;
		count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_interface[4];
		ret_val = cvmx_pko_queue_grp_alloc(start, end, count);

		if (ret_val != 0)
			return -1;
		end = 32; /* DPI first PKO poty */
	}

	start = end;
	end = 36;
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_pci;
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end;
	end = 40;
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_loop;
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end;
	end = 42;
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_srio[0];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end;
	end = 44;
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_srio[1];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end;
	end = 46;
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_srio[2];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end;
	end = 48;
	count = __cvmx_pko_queue_static_config[0].non_pknd.pko_queues_per_port_srio[3];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;
	return 0;
}

static int queue_range_init;

int init_cvmx_pko_que_range(void)
{
	int rv = 0;

	if (queue_range_init)
		return 0;
	queue_range_init = 1;
	rv = cvmx_create_global_resource_range(CVMX_GR_TAG_PKO_QUEUES,
					       CVMX_HELPER_CFG_MAX_PKO_QUEUES);
	if (rv != 0)
		printf("ERROR: %s: Failed to initialize pko queues range\n", __func__);

	return rv;
}

/*
 * get a block of "count" queues for "port"
 *
 * @param  port   the port for which the queues are requested
 * @param  count  the number of queues requested
 *
 * Return:  0 on success
 *         -1 on failure
 */
static int cvmx_pko_queue_alloc(u64 port, int count)
{
	int ret_val = -1;
	int highest_queue;

	init_cvmx_pko_que_range();

	if (cvmx_pko_queue_table[port].ccppp_num_queues == count)
		return cvmx_pko_queue_table[port].ccppp_queue_base;

	if (cvmx_pko_queue_table[port].ccppp_num_queues > 0) {
		printf("WARNING: %s port=%d already %d queues\n",
		       __func__, (int)port,
		       (int)cvmx_pko_queue_table[port].ccppp_num_queues);
		return -1;
	}

	if (port >= CVMX_HELPER_CFG_MAX_PKO_QUEUES) {
		printf("ERROR: %s port=%d > %d\n", __func__, (int)port,
		       CVMX_HELPER_CFG_MAX_PKO_QUEUES);
		return -1;
	}

	ret_val = cvmx_allocate_global_resource_range(CVMX_GR_TAG_PKO_QUEUES,
						      port, count, 1);

	debug("%s: pko_e_port=%i q_base=%i q_count=%i\n",
	      __func__, (int)port, ret_val, (int)count);

	if (ret_val == -1)
		return ret_val;
	cvmx_pko_queue_table[port].ccppp_queue_base = ret_val;
	cvmx_pko_queue_table[port].ccppp_num_queues = count;

	highest_queue = ret_val + count - 1;
	if (highest_queue > __cvmx_cfg_pko_highest_queue)
		__cvmx_cfg_pko_highest_queue = highest_queue;
	return 0;
}

/*
 * initialize cvmx_cfg_pko_port_map
 */
void cvmx_helper_cfg_init_pko_port_map(void)
{
	int i, j, k;
	int pko_eid;
	int pko_port_base, pko_port_max;
	cvmx_helper_interface_mode_t mode;

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	/*
	 * one pko_eid is allocated to each port except for ILK, NPI, and
	 * LOOP. Each of the three has one eid.
	 */
	pko_eid = 0;
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		mode = cvmx_helper_interface_get_mode(i);
		for (j = 0; j < cvmx_helper_interface_enumerate(i); j++) {
			pko_port_base = cvmx_cfg_port[0][i][j].ccpp_pko_port_base;
			pko_port_max = pko_port_base + cvmx_cfg_port[0][i][j].ccpp_pko_num_ports;
			if (!octeon_has_feature(OCTEON_FEATURE_PKO3)) {
				cvmx_helper_cfg_assert(pko_port_base !=
						       CVMX_HELPER_CFG_INVALID_VALUE);
				cvmx_helper_cfg_assert(pko_port_max >= pko_port_base);
			}
			for (k = pko_port_base; k < pko_port_max; k++) {
				cvmx_cfg_pko_port_map[k].ccppl_interface = i;
				cvmx_cfg_pko_port_map[k].ccppl_index = j;
				cvmx_cfg_pko_port_map[k].ccppl_eid = pko_eid;
			}

			if (!(mode == CVMX_HELPER_INTERFACE_MODE_NPI ||
			      mode == CVMX_HELPER_INTERFACE_MODE_LOOP ||
			      mode == CVMX_HELPER_INTERFACE_MODE_ILK))
				pko_eid++;
		}

		if (mode == CVMX_HELPER_INTERFACE_MODE_NPI ||
		    mode == CVMX_HELPER_INTERFACE_MODE_LOOP ||
		    mode == CVMX_HELPER_INTERFACE_MODE_ILK)
			pko_eid++;
	}

	/*
	 * Legal pko_eids [0, 0x13] should not be exhausted.
	 */
	if (!octeon_has_feature(OCTEON_FEATURE_PKO3))
		cvmx_helper_cfg_assert(pko_eid <= 0x14);

	cvmx_cfg_max_pko_engines = pko_eid;
}

int __cvmx_helper_cfg_pko_port_interface(int pko_port)
{
	return cvmx_cfg_pko_port_map[pko_port].ccppl_interface;
}

int __cvmx_helper_cfg_pko_port_index(int pko_port)
{
	return cvmx_cfg_pko_port_map[pko_port].ccppl_index;
}

int __cvmx_helper_cfg_pko_port_eid(int pko_port)
{
	return cvmx_cfg_pko_port_map[pko_port].ccppl_eid;
}

#define IPD2PKO_CACHE_Y(ipd_port) (ipd_port) >> 8
#define IPD2PKO_CACHE_X(ipd_port) (ipd_port) & 0xff

static inline int __cvmx_helper_cfg_ipd2pko_cachex(int ipd_port)
{
	int ipd_x = IPD2PKO_CACHE_X(ipd_port);

	if (ipd_port & 0x800)
		ipd_x = (ipd_x >> 4) & 3;
	return ipd_x;
}

/*
 * ipd_port to pko_port translation cache
 */
int __cvmx_helper_cfg_init_ipd2pko_cache(void)
{
	int i, j, n;
	int ipd_y, ipd_x, ipd_port;

	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		n = cvmx_helper_interface_enumerate(i);

		for (j = 0; j < n; j++) {
			ipd_port = cvmx_helper_get_ipd_port(i, j);
			ipd_y = IPD2PKO_CACHE_Y(ipd_port);
			ipd_x = __cvmx_helper_cfg_ipd2pko_cachex(ipd_port);
			ipd2pko_port_cache[ipd_y][ipd_x] = (struct cvmx_cfg_pko_port_pair){
				__cvmx_helper_cfg_pko_port_base(i, j),
				__cvmx_helper_cfg_pko_port_num(i, j)
			};
		}
	}

	return 0;
}

int cvmx_helper_cfg_ipd2pko_port_base(int ipd_port)
{
	int ipd_y, ipd_x;

	/* Internal PKO ports are not present in PKO3 */
	if (octeon_has_feature(OCTEON_FEATURE_PKI))
		return ipd_port;

	ipd_y = IPD2PKO_CACHE_Y(ipd_port);
	ipd_x = __cvmx_helper_cfg_ipd2pko_cachex(ipd_port);

	return ipd2pko_port_cache[ipd_y][ipd_x].ccppp_base_port;
}

/**
 * Return the number of queues to be assigned to this pko_port
 *
 * @param pko_port
 * Return: the number of queues for this pko_port
 *
 */
static int cvmx_helper_cfg_dft_nqueues(int pko_port)
{
	cvmx_helper_interface_mode_t mode;
	int interface;
	int n;
	int ret;

	interface = __cvmx_helper_cfg_pko_port_interface(pko_port);
	mode = cvmx_helper_interface_get_mode(interface);

	n = NUM_ELEMENTS(__cvmx_pko_queue_static_config[0].pknd.pko_cfg_iface);

	if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
		ret = __cvmx_pko_queue_static_config[0].pknd.pko_cfg_loop.queues_per_port;
	} else if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
		ret = __cvmx_pko_queue_static_config[0].pknd.pko_cfg_npi.queues_per_port;
	}

	else if ((interface >= 0) && (interface < n)) {
		ret = __cvmx_pko_queue_static_config[0].pknd.pko_cfg_iface[interface].queues_per_port;
	} else {
		/* Should never be called */
		ret = 1;
	}
	/* Override for sanity in case of empty static config table */
	if (ret == 0)
		ret = 1;
	return ret;
}

static int cvmx_helper_cfg_init_pko_iports_and_queues_using_static_config(void)
{
	int pko_port_base = 0;
	int cvmx_cfg_default_pko_nports = 1;
	int i, j, n, k;
	int rv = 0;

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();

	/* When not using config file, each port is assigned one internal pko port*/
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		n = cvmx_helper_interface_enumerate(i);
		for (j = 0; j < n; j++) {
			cvmx_cfg_port[0][i][j].ccpp_pko_port_base = pko_port_base;
			cvmx_cfg_port[0][i][j].ccpp_pko_num_ports = cvmx_cfg_default_pko_nports;
			/*
			 * Initialize interface early here so that the
			 * cvmx_helper_cfg_dft_nqueues() below
			 * can get the interface number corresponding to the
			 * pko port
			 */
			for (k = pko_port_base; k < pko_port_base + cvmx_cfg_default_pko_nports;
			     k++) {
				cvmx_cfg_pko_port_map[k].ccppl_interface = i;
			}
			pko_port_base += cvmx_cfg_default_pko_nports;
		}
	}
	cvmx_helper_cfg_assert(pko_port_base <= CVMX_HELPER_CFG_MAX_PKO_PORT);

	/* Assigning queues per pko */
	for (i = 0; i < pko_port_base; i++) {
		int base;

		n = cvmx_helper_cfg_dft_nqueues(i);
		base = cvmx_pko_queue_alloc(i, n);
		if (base == -1) {
			printf("ERROR: %s: failed to alloc %d queues for pko port=%d\n", __func__,
			       n, i);
			rv = -1;
		}
	}
	return rv;
}

/**
 * Returns if port is valid for a given interface
 *
 * @param xiface  interface to check
 * @param index      port index in the interface
 *
 * Return: status of the port present or not.
 */
int cvmx_helper_is_port_valid(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].valid;
}

void cvmx_helper_set_port_valid(int xiface, int index, bool valid)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].valid = valid;
}

void cvmx_helper_set_mac_phy_mode(int xiface, int index, bool valid)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].sgmii_phy_mode = valid;
}

bool cvmx_helper_get_mac_phy_mode(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].sgmii_phy_mode;
}

void cvmx_helper_set_1000x_mode(int xiface, int index, bool valid)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].sgmii_1000x_mode = valid;
}

bool cvmx_helper_get_1000x_mode(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].sgmii_1000x_mode;
}

void cvmx_helper_set_agl_rx_clock_delay_bypass(int xiface, int index, bool valid)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].agl_rx_clk_delay_bypass = valid;
}

bool cvmx_helper_get_agl_rx_clock_delay_bypass(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].agl_rx_clk_delay_bypass;
}

void cvmx_helper_set_agl_rx_clock_skew(int xiface, int index, uint8_t value)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].agl_rx_clk_skew = value;
}

uint8_t cvmx_helper_get_agl_rx_clock_skew(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].agl_rx_clk_skew;
}

void cvmx_helper_set_agl_refclk_sel(int xiface, int index, uint8_t value)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].agl_refclk_sel = value;
}

uint8_t cvmx_helper_get_agl_refclk_sel(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].agl_refclk_sel;
}

void cvmx_helper_set_port_force_link_up(int xiface, int index, bool value)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].force_link_up = value;
}

bool cvmx_helper_get_port_force_link_up(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].force_link_up;
}

void cvmx_helper_set_port_phy_present(int xiface, int index, bool value)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].phy_present = value;
}

bool cvmx_helper_get_port_phy_present(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].phy_present;
}

int __cvmx_helper_init_port_valid(void)
{
	int i, j, node;
	bool valid;
	static void *fdt_addr;
	int rc;
	struct cvmx_coremask pcm;

	octeon_get_available_coremask(&pcm);

	if (fdt_addr == 0)
		fdt_addr = __cvmx_phys_addr_to_ptr((u64)gd->fdt_blob, 128 * 1024);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		rc = __cvmx_helper_parse_bgx_dt(fdt_addr);
		if (!rc)
			rc = __cvmx_fdt_parse_vsc7224(fdt_addr);
		if (!rc && octeon_has_feature(OCTEON_FEATURE_BGX_XCV))
			rc = __cvmx_helper_parse_bgx_rgmii_dt(fdt_addr);

		/* Some ports are not in sequence, the device tree does not
		 * clear them.
		 *
		 * Also clear any ports that are not defined in the device tree.
		 * Apply this to each node.
		 */
		for (node = 0; node < CVMX_MAX_NODES; node++) {
			if (!cvmx_coremask_get64_node(&pcm, node))
				continue;
			for (i = 0; i < CVMX_HELPER_MAX_GMX; i++) {
				int j;
				int xiface = cvmx_helper_node_interface_to_xiface(node, i);

				for (j = 0; j < cvmx_helper_interface_enumerate(i); j++) {
					cvmx_bgxx_cmrx_config_t cmr_config;

					cmr_config.u64 =
						csr_rd_node(node, CVMX_BGXX_CMRX_CONFIG(j, i));
					if ((cmr_config.s.lane_to_sds == 0xe4 &&
					     cmr_config.s.lmac_type != 4 &&
					     cmr_config.s.lmac_type != 1 &&
					     cmr_config.s.lmac_type != 5) ||
					    ((cvmx_helper_get_port_fdt_node_offset(xiface, j) ==
					      CVMX_HELPER_CFG_INVALID_VALUE)))
						cvmx_helper_set_port_valid(xiface, j, false);
				}
			}
		}
		return rc;
	}

	/* TODO: Update this to behave more like 78XX */
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		int n = cvmx_helper_interface_enumerate(i);

		for (j = 0; j < n; j++) {
			int ipd_port = cvmx_helper_get_ipd_port(i, j);

			valid = (__cvmx_helper_board_get_port_from_dt(fdt_addr, ipd_port) == 1);
			cvmx_helper_set_port_valid(i, j, valid);
		}
	}
	return 0;
}

/*
 * This call is made from Linux octeon_ethernet driver
 * to setup the PKO with a specific queue count and
 * internal port count configuration.
 */
int cvmx_pko_alloc_iport_and_queues(int interface, int port, int port_cnt, int queue_cnt)
{
	int rv, p, port_start, cnt;

	debug("%s: intf %d/%d pcnt %d qcnt %d\n", __func__, interface, port, port_cnt,
	      queue_cnt);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		rv = cvmx_pko_internal_ports_alloc(interface, port, port_cnt);
		if (rv < 0) {
			printf("ERROR: %s: failed to allocate internal ports forinterface=%d port=%d cnt=%d\n",
			       __func__, interface, port, port_cnt);
			return -1;
		}
		port_start = __cvmx_helper_cfg_pko_port_base(interface, port);
		cnt = __cvmx_helper_cfg_pko_port_num(interface, port);
	} else {
		port_start = cvmx_helper_get_ipd_port(interface, port);
		cnt = 1;
	}

	for (p = port_start; p < port_start + cnt; p++) {
		rv = cvmx_pko_queue_alloc(p, queue_cnt);
		if (rv < 0) {
			printf("ERROR: %s: failed to allocate queues for port=%d cnt=%d\n",
			       __func__, p, queue_cnt);
			return -1;
		}
	}
	return 0;
}

static void cvmx_init_port_cfg(void)
{
	int node, i, j;

	if (port_cfg_data_initialized)
		return;

	for (node = 0; node < CVMX_MAX_NODES; node++) {
		for (i = 0; i < CVMX_HELPER_MAX_IFACE; i++) {
			for (j = 0; j < CVMX_HELPER_CFG_MAX_PORT_PER_IFACE; j++) {
				struct cvmx_cfg_port_param *pcfg;
				struct cvmx_srio_port_param *sr;

				pcfg = &cvmx_cfg_port[node][i][j];

				memset(pcfg, 0, sizeof(*pcfg));

				pcfg->port_fdt_node = CVMX_HELPER_CFG_INVALID_VALUE;
				pcfg->phy_fdt_node = CVMX_HELPER_CFG_INVALID_VALUE;
				pcfg->phy_info = NULL;
				pcfg->ccpp_pknd = CVMX_HELPER_CFG_INVALID_VALUE;
				pcfg->ccpp_bpid = CVMX_HELPER_CFG_INVALID_VALUE;
				pcfg->ccpp_pko_port_base = CVMX_HELPER_CFG_INVALID_VALUE;
				pcfg->ccpp_pko_num_ports = CVMX_HELPER_CFG_INVALID_VALUE;
				pcfg->agl_rx_clk_skew = 0;
				pcfg->valid = true;
				pcfg->sgmii_phy_mode = false;
				pcfg->sgmii_1000x_mode = false;
				pcfg->agl_rx_clk_delay_bypass = false;
				pcfg->force_link_up = false;
				pcfg->disable_an = false;
				pcfg->link_down_pwr_dn = false;
				pcfg->phy_present = false;
				pcfg->tx_clk_delay_bypass = false;
				pcfg->rgmii_tx_clk_delay = 0;
				pcfg->enable_fec = false;
				sr = &pcfg->srio_short;
				sr->srio_rx_ctle_agc_override = false;
				sr->srio_rx_ctle_zero = 0x6;
				sr->srio_rx_agc_pre_ctle = 0x5;
				sr->srio_rx_agc_post_ctle = 0x4;
				sr->srio_tx_swing_override = false;
				sr->srio_tx_swing = 0x7;
				sr->srio_tx_premptap_override = false;
				sr->srio_tx_premptap_pre = 0;
				sr->srio_tx_premptap_post = 0xF;
				sr->srio_tx_gain_override = false;
				sr->srio_tx_gain = 0x3;
				sr->srio_tx_vboost_override = 0;
				sr->srio_tx_vboost = true;
				sr = &pcfg->srio_long;
				sr->srio_rx_ctle_agc_override = false;
				sr->srio_rx_ctle_zero = 0x6;
				sr->srio_rx_agc_pre_ctle = 0x5;
				sr->srio_rx_agc_post_ctle = 0x4;
				sr->srio_tx_swing_override = false;
				sr->srio_tx_swing = 0x7;
				sr->srio_tx_premptap_override = false;
				sr->srio_tx_premptap_pre = 0;
				sr->srio_tx_premptap_post = 0xF;
				sr->srio_tx_gain_override = false;
				sr->srio_tx_gain = 0x3;
				sr->srio_tx_vboost_override = 0;
				sr->srio_tx_vboost = true;
				pcfg->agl_refclk_sel = 0;
				pcfg->sfp_of_offset = -1;
				pcfg->vsc7224_chan = NULL;
			}
		}
	}
	port_cfg_data_initialized = true;
}

int __cvmx_helper_init_port_config_data(int node)
{
	int rv = 0;
	int i, j, n;
	int num_interfaces, interface;
	int pknd = 0, bpid = 0;
	const int use_static_config = 1;

	debug("%s:\n", __func__);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		/* PKO3: only needs BPID, PKND to be setup,
		 * while the rest of PKO3 init is done in cvmx-helper-pko3.c
		 */
		pknd = 0;
		bpid = 0;
		for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
			int xiface = cvmx_helper_node_interface_to_xiface(node, i);

			n = cvmx_helper_interface_enumerate(xiface);
			/*
			 * Assign 8 pknds to ILK interface, these pknds will be
			 * distributed among the channels configured
			 */
			if (cvmx_helper_interface_get_mode(xiface) ==
			    CVMX_HELPER_INTERFACE_MODE_ILK) {
				if (n > 8)
					n = 8;
			}
			if (cvmx_helper_interface_get_mode(xiface) !=
			    CVMX_HELPER_INTERFACE_MODE_NPI) {
				for (j = 0; j < n; j++) {
					struct cvmx_cfg_port_param *pcfg;

					pcfg = &cvmx_cfg_port[node][i][j];
					pcfg->ccpp_pknd = pknd++;
					pcfg->ccpp_bpid = bpid++;
				}
			} else {
				for (j = 0; j < n; j++) {
					if (j == n / cvmx_npi_max_pknds) {
						pknd++;
						bpid++;
					}
					cvmx_cfg_port[node][i][j].ccpp_pknd = pknd;
					cvmx_cfg_port[node][i][j].ccpp_bpid = bpid;
				}
				pknd++;
				bpid++;
			}
		} /* for i=0 */
		cvmx_helper_cfg_assert(pknd <= CVMX_HELPER_CFG_MAX_PIP_PKND);
		cvmx_helper_cfg_assert(bpid <= CVMX_HELPER_CFG_MAX_PIP_BPID);
	} else if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		if (use_static_config)
			cvmx_helper_cfg_init_pko_iports_and_queues_using_static_config();

		/* Initialize pknd and bpid */
		for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
			n = cvmx_helper_interface_enumerate(i);
			for (j = 0; j < n; j++) {
				cvmx_cfg_port[0][i][j].ccpp_pknd = pknd++;
				cvmx_cfg_port[0][i][j].ccpp_bpid = bpid++;
			}
		}
		cvmx_helper_cfg_assert(pknd <= CVMX_HELPER_CFG_MAX_PIP_PKND);
		cvmx_helper_cfg_assert(bpid <= CVMX_HELPER_CFG_MAX_PIP_BPID);
	} else {
		if (use_static_config)
			cvmx_pko_queue_init_from_cvmx_config_non_pknd();
	}

	/* Remainder not used for PKO3 */
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
		return 0;

	/* init ports, queues which are not initialized */
	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		int num_ports = __cvmx_helper_early_ports_on_interface(interface);
		int port, port_base, queue;

		for (port = 0; port < num_ports; port++) {
			bool init_req = false;

			if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
				port_base = __cvmx_helper_cfg_pko_port_base(interface, port);
				if (port_base == CVMX_HELPER_CFG_INVALID_VALUE)
					init_req = true;
			} else {
				port_base = cvmx_helper_get_ipd_port(interface, port);
				queue = __cvmx_helper_cfg_pko_queue_base(port_base);
				if (queue == CVMX_HELPER_CFG_INVALID_VALUE)
					init_req = true;
			}

			if (init_req) {
				rv = cvmx_pko_alloc_iport_and_queues(interface, port, 1, 1);
				if (rv < 0) {
					debug("cvm_pko_alloc_iport_and_queues failed.\n");
					return rv;
				}
			}
		}
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		cvmx_helper_cfg_init_pko_port_map();
		__cvmx_helper_cfg_init_ipd2pko_cache();
	}

#ifdef DEBUG
	cvmx_helper_cfg_show_cfg();
	cvmx_pko_queue_show();
#endif

	return rv;
}

/**
 * @INTERNAL
 * Store the FDT node offset in the device tree of a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param node_offset	node offset to store
 */
void cvmx_helper_set_port_fdt_node_offset(int xiface, int index, int node_offset)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].port_fdt_node = node_offset;
}

/**
 * @INTERNAL
 * Return the FDT node offset in the device tree of a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * Return:		node offset of port or -1 if invalid
 */
int cvmx_helper_get_port_fdt_node_offset(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].port_fdt_node;
}

/**
 * @INTERNAL
 * Store the FDT node offset in the device tree of a phy
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param node_offset	node offset to store
 */
void cvmx_helper_set_phy_fdt_node_offset(int xiface, int index, int node_offset)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].phy_fdt_node = node_offset;
}

/**
 * @INTERNAL
 * Return the FDT node offset in the device tree of a phy
 *
 * @param xiface	node and interface
 * @param index		port index
 * Return:		node offset of phy or -1 if invalid
 */
int cvmx_helper_get_phy_fdt_node_offset(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].phy_fdt_node;
}

/**
 * @INTERNAL
 * Override default autonegotiation for a port
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param enable	true to enable autonegotiation, false to force full
 *			duplex, full speed.
 */
void cvmx_helper_set_port_autonegotiation(int xiface, int index, bool enable)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].disable_an = !enable;
}

/**
 * @INTERNAL
 * Returns if autonegotiation is enabled or not.
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: 0 if autonegotiation is disabled, 1 if enabled.
 */
bool cvmx_helper_get_port_autonegotiation(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return !cvmx_cfg_port[xi.node][xi.interface][index].disable_an;
}

/**
 * @INTERNAL
 * Returns if forward error correction is enabled or not.
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: false if fec is disabled, true if enabled.
 */
bool cvmx_helper_get_port_fec(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].enable_fec;
}

/**
 * @INTERNAL
 * Sets the PHY info data structure
 *
 * @param xiface	node and interface
 * @param index		port index
 * @param[in] phy_info	phy information data structure pointer
 */
void cvmx_helper_set_port_phy_info(int xiface, int index, struct cvmx_phy_info *phy_info)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].phy_info = phy_info;
}

/**
 * @INTERNAL
 * Returns the PHY information data structure for a port
 *
 * @param xiface	node and interface
 * @param index		port index
 *
 * Return: pointer to PHY information data structure or NULL if not set
 */
struct cvmx_phy_info *cvmx_helper_get_port_phy_info(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].phy_info;
}

/**
 * @INTERNAL
 * Returns a pointer to the PHY LED configuration (if local GPIOs drive them)
 *
 * @param xiface	node and interface
 * @param index		portindex
 *
 * Return: pointer to the PHY LED information data structure or NULL if not
 *	   present
 */
struct cvmx_phy_gpio_leds *cvmx_helper_get_port_phy_leds(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].gpio_leds;
}

/**
 * @INTERNAL
 * Disables RGMII TX clock bypass and sets delay value
 *
 * @param xiface	node and interface
 * @param index		portindex
 * @param bypass	Set true to enable the clock bypass and false
 *			to sync clock and data synchronously.
 *			Default is false.
 * @param clk_delay	Delay value to skew TXC from TXD
 */
void cvmx_helper_cfg_set_rgmii_tx_clk_delay(int xiface, int index, bool bypass, int clk_delay)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].tx_clk_delay_bypass = bypass;
	cvmx_cfg_port[xi.node][xi.interface][index].rgmii_tx_clk_delay = clk_delay;
}

/**
 * @INTERNAL
 * Gets RGMII TX clock bypass and delay value
 *
 * @param xiface	node and interface
 * @param index		portindex
 * @param bypass	Set true to enable the clock bypass and false
 *			to sync clock and data synchronously.
 *			Default is false.
 * @param clk_delay	Delay value to skew TXC from TXD, default is 0.
 */
void cvmx_helper_cfg_get_rgmii_tx_clk_delay(int xiface, int index, bool *bypass, int *clk_delay)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	*bypass = cvmx_cfg_port[xi.node][xi.interface][index].tx_clk_delay_bypass;

	*clk_delay = cvmx_cfg_port[xi.node][xi.interface][index].rgmii_tx_clk_delay;
}

/**
 * Sets the Microsemi VSC7224 channel info data structure
 *
 * @param	xiface	node and interface
 * @param	index	port index
 * @param[in]	vsc7224_info	Microsemi VSC7224 data structure
 */
void cvmx_helper_cfg_set_vsc7224_chan_info(int xiface, int index,
					   struct cvmx_vsc7224_chan *vsc7224_chan_info)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].vsc7224_chan = vsc7224_chan_info;
}

/**
 * Gets the SFP data associated with a port
 *
 * @param	xiface	node and interface
 * @param	index	port index
 *
 * Return:	pointer to SFP data structure or NULL if none
 */
struct cvmx_fdt_sfp_info *cvmx_helper_cfg_get_sfp_info(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	return cvmx_cfg_port[xi.node][xi.interface][index].sfp_info;
}

/**
 * Sets the SFP data associated with a port
 *
 * @param	xiface		node and interface
 * @param	index		port index
 * @param[in]	sfp_info	port SFP data or NULL for none
 */
void cvmx_helper_cfg_set_sfp_info(int xiface, int index, struct cvmx_fdt_sfp_info *sfp_info)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!port_cfg_data_initialized)
		cvmx_init_port_cfg();
	cvmx_cfg_port[xi.node][xi.interface][index].sfp_info = sfp_info;
}
