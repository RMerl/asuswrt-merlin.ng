// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Helper Functions for the PKO
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

#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-board.h>
#include <mach/cvmx-helper-cfg.h>

#include <mach/cvmx-helper-bgx.h>
#include <mach/cvmx-helper-cfg.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

static s64 pko_fpa_config_pool = -1;
static u64 pko_fpa_config_size = 1024;

/**
 * cvmx_override_pko_queue_priority(int pko_port, u64
 * priorities[16]) is a function pointer. It is meant to allow
 * customization of the PKO queue priorities based on the port
 * number. Users should set this pointer to a function before
 * calling any cvmx-helper operations.
 */
void (*cvmx_override_pko_queue_priority)(int ipd_port,
					 uint8_t *priorities) = NULL;

int64_t cvmx_fpa_get_pko_pool(void)
{
	return pko_fpa_config_pool;
}

/**
 * Gets the buffer size of pko pool
 */
u64 cvmx_fpa_get_pko_pool_block_size(void)
{
	return pko_fpa_config_size;
}

/**
 * Initialize PKO command queue buffer pool
 */
static int cvmx_helper_pko_pool_init(void)
{
	u8 pool;
	unsigned int buf_count;
	unsigned int pkt_buf_count;
	int rc;

	/* Reserve pool */
	pool = cvmx_fpa_get_pko_pool();

	/* Avoid redundant pool creation */
	if (cvmx_fpa_get_block_size(pool) > 0) {
#ifdef DEBUG
		debug("WARNING: %s: pool %d already initialized\n", __func__,
		      pool);
#endif
		/* It is up to the app to have sufficient buffer count */
		return pool;
	}

	/* Calculate buffer count: one per queue + 3-word-cmds * max_pkts */
	pkt_buf_count = cvmx_fpa_get_packet_pool_buffer_count();
	buf_count = CVMX_PKO_MAX_OUTPUT_QUEUES + (pkt_buf_count * 3) / 8;

	/* Allocate pools for pko command queues */
	rc = __cvmx_helper_initialize_fpa_pool(pool,
					       cvmx_fpa_get_pko_pool_block_size(),
					       buf_count, "PKO Cmd-bufs");

	if (rc < 0)
		debug("%s: ERROR: in PKO buffer pool\n", __func__);

	pool = rc;
	return pool;
}

/**
 * Initialize the PKO
 *
 */
int cvmx_helper_pko_init(void)
{
	int rc;

	rc = cvmx_helper_pko_pool_init();
	if (rc < 0)
		return rc;

	__cvmx_helper_init_port_config_data(0);

	cvmx_pko_hw_init(cvmx_fpa_get_pko_pool(),
			 cvmx_fpa_get_pko_pool_block_size());
	return 0;
}

/**
 * @INTERNAL
 * Setup the PKO for the ports on an interface. The number of
 * queues per port and the priority of each PKO output queue
 * is set here. PKO must be disabled when this function is called.
 *
 * @param interface to setup PKO for
 *
 * @return Zero on success, negative on failure
 *
 * @note This is for PKO1/PKO2, and is not used for PKO3.
 */
int __cvmx_helper_interface_setup_pko(int interface)
{
	/*
	 * Each packet output queue has an associated priority. The
	 * higher the priority, the more often it can send a packet. A
	 * priority of 8 means it can send in all 8 rounds of
	 * contention. We're going to make each queue one less than
	 * the last.  The vector of priorities has been extended to
	 * support CN5xxx CPUs, where up to 16 queues can be
	 * associated to a port.  To keep backward compatibility we
	 * don't change the initial 8 priorities and replicate them in
	 * the second half.  With per-core PKO queues (PKO lockless
	 * operation) all queues have the same priority.
	 */
	/* uint8_t priorities[16] = {8,7,6,5,4,3,2,1,8,7,6,5,4,3,2,1}; */
	u8 priorities[16] = { [0 ... 15] = 8 };

	/*
	 * Setup the IPD/PIP and PKO for the ports discovered
	 * above. Here packet classification, tagging and output
	 * priorities are set.
	 */
	int num_ports = cvmx_helper_ports_on_interface(interface);

	while (num_ports--) {
		int ipd_port;

		if (!cvmx_helper_is_port_valid(interface, num_ports))
			continue;

		ipd_port = cvmx_helper_get_ipd_port(interface, num_ports);
		/*
		 * Give the user a chance to override the per queue
		 * priorities.
		 */
		if (cvmx_override_pko_queue_priority)
			cvmx_override_pko_queue_priority(ipd_port, priorities);

		cvmx_pko_config_port(ipd_port,
				     cvmx_pko_get_base_queue(ipd_port),
				     cvmx_pko_get_num_queues(ipd_port),
				     priorities);
		ipd_port++;
	}
	return 0;
	/* NOTE:
	 * Now this function is called for all chips including 68xx,
	 * but on the 68xx it does not enable multiple pko_iports per
	 * eport, while before it was doing 3 pko_iport per eport
	 * buf the reason for that is not clear.
	 */
}
