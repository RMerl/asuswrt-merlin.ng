// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
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

union interface_port {
	struct {
		int port;
		int interface;
	} s;
	u64 u64;
};

static int dbg;

static int port_range_init;

int __cvmx_pko_internal_ports_range_init(void)
{
	int rv = 0;

	if (port_range_init)
		return 0;
	port_range_init = 1;
	rv = cvmx_create_global_resource_range(CVMX_GR_TAG_PKO_IPORTS,
					       CVMX_HELPER_CFG_MAX_PKO_QUEUES);
	if (rv != 0)
		debug("ERROR : Failed to initialize pko internal port range\n");
	return rv;
}

int cvmx_pko_internal_ports_alloc(int xiface, int port, u64 count)
{
	int ret_val = -1;
	union interface_port inf_port;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	__cvmx_pko_internal_ports_range_init();
	inf_port.s.interface = xi.interface;
	inf_port.s.port = port;
	ret_val = cvmx_allocate_global_resource_range(CVMX_GR_TAG_PKO_IPORTS,
						      inf_port.u64, count, 1);
	if (dbg)
		debug("internal port alloc : port=%02d base=%02d count=%02d\n",
		      (int)port, ret_val, (int)count);
	if (ret_val == -1)
		return ret_val;
	cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_port_base = ret_val;
	cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_num_ports = count;
	return 0;
}
