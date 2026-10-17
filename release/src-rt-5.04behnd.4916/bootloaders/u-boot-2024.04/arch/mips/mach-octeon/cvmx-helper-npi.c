// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Functions for NPI initialization, configuration,
 * and monitoring.
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

static int cvmx_npi_num_pipes = -1;

/**
 * @INTERNAL
 * Probe a NPI interface and determine the number of ports
 * connected to it. The NPI interface should still be down
 * after this call.
 *
 * @param interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_npi_probe(int interface)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 32;
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 128;
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 64;

	return 0;
}

/**
 * @INTERNAL
 * Bringup and enable a NPI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_npi_enable(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int port;
	int num_ports = cvmx_helper_ports_on_interface(interface);

	/*
	 * On CN50XX, CN52XX, and CN56XX we need to disable length
	 * checking so packet < 64 bytes and jumbo frames don't get
	 * errors.
	 */
	for (port = 0; port < num_ports; port++) {
		union cvmx_pip_prt_cfgx port_cfg;
		int ipd_port =
			(octeon_has_feature(OCTEON_FEATURE_PKND)) ?
				cvmx_helper_get_pknd(interface, port) :
				      cvmx_helper_get_ipd_port(interface, port);
		if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
			unsigned int node = cvmx_get_node_num();

			cvmx_pki_endis_l2_errs(node, ipd_port, 0, 0, 0);

		} else {
			port_cfg.u64 = csr_rd(CVMX_PIP_PRT_CFGX(ipd_port));
			port_cfg.s.lenerr_en = 0;
			port_cfg.s.maxerr_en = 0;
			port_cfg.s.minerr_en = 0;
			csr_wr(CVMX_PIP_PRT_CFGX(ipd_port), port_cfg.u64);
		}
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			/* Set up pknd and bpid */
			union cvmx_sli_portx_pkind config;

			config.u64 = csr_rd(CVMX_PEXP_SLI_PORTX_PKIND(port));
			config.s.bpkind = cvmx_helper_get_bpid(interface, port);
			config.s.pkind = cvmx_helper_get_pknd(interface, port);
			csr_wr(CVMX_PEXP_SLI_PORTX_PKIND(port), config.u64);
		}
	}

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/*
		 * Set up pko pipes.
		 */
		union cvmx_sli_tx_pipe config;

		config.u64 = csr_rd(CVMX_PEXP_SLI_TX_PIPE);
		config.s.base = __cvmx_pko_get_pipe(interface, 0);
		config.s.nump =
			cvmx_npi_num_pipes < 0 ? num_ports : cvmx_npi_num_pipes;
		csr_wr(CVMX_PEXP_SLI_TX_PIPE, config.u64);
	}

	/* Enables are controlled by the remote host, so nothing to do here */
	return 0;
}
