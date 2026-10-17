// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * IPD Support.
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

cvmx_ipd_config_t cvmx_ipd_cfg = {
	.first_mbuf_skip = 184,
	.ipd_enable = 1,
	.cache_mode = CVMX_IPD_OPC_MODE_STT,
	.packet_pool = { 0, 2048, 0 },
	.wqe_pool = { 1, 128, 0 },
	.port_config = { CVMX_PIP_PORT_CFG_MODE_SKIPL2,
			 CVMX_POW_TAG_TYPE_ORDERED, CVMX_PIP_TAG_MODE_TUPLE,
			 .tag_fields = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } }
};

#define IPD_RED_AVG_DLY 1000
#define IPD_RED_PRB_DLY 1000

void cvmx_ipd_config(u64 mbuff_size, u64 first_mbuff_skip,
		     u64 not_first_mbuff_skip, u64 first_back, u64 second_back,
		     u64 wqe_fpa_pool, cvmx_ipd_mode_t cache_mode,
		     u64 back_pres_enable_flag)
{
	cvmx_ipd_1st_mbuff_skip_t first_skip;
	cvmx_ipd_mbuff_not_first_skip_t not_first_skip;
	cvmx_ipd_packet_mbuff_size_t size;
	cvmx_ipd_1st_next_ptr_back_t first_back_struct;
	cvmx_ipd_second_next_ptr_back_t second_back_struct;
	cvmx_ipd_wqe_fpa_queue_t wqe_pool;
	cvmx_ipd_ctl_status_t ipd_ctl_reg;

	/* Enforce 1st skip minimum if WQE shares the buffer with packet */
	if (octeon_has_feature(OCTEON_FEATURE_NO_WPTR)) {
		union cvmx_ipd_ctl_status ctl_status;

		ctl_status.u64 = csr_rd(CVMX_IPD_CTL_STATUS);
		if (ctl_status.s.no_wptr != 0 && first_mbuff_skip < 16)
			first_mbuff_skip = 16;
	}

	first_skip.u64 = 0;
	first_skip.s.skip_sz = first_mbuff_skip;
	csr_wr(CVMX_IPD_1ST_MBUFF_SKIP, first_skip.u64);

	not_first_skip.u64 = 0;
	not_first_skip.s.skip_sz = not_first_mbuff_skip;
	csr_wr(CVMX_IPD_NOT_1ST_MBUFF_SKIP, not_first_skip.u64);

	size.u64 = 0;
	size.s.mb_size = mbuff_size;
	csr_wr(CVMX_IPD_PACKET_MBUFF_SIZE, size.u64);

	first_back_struct.u64 = 0;
	first_back_struct.s.back = first_back;
	csr_wr(CVMX_IPD_1st_NEXT_PTR_BACK, first_back_struct.u64);

	second_back_struct.u64 = 0;
	second_back_struct.s.back = second_back;
	csr_wr(CVMX_IPD_2nd_NEXT_PTR_BACK, second_back_struct.u64);

	wqe_pool.u64 = 0;
	wqe_pool.s.wqe_pool = wqe_fpa_pool;
	csr_wr(CVMX_IPD_WQE_FPA_QUEUE, wqe_pool.u64);

	ipd_ctl_reg.u64 = csr_rd(CVMX_IPD_CTL_STATUS);
	ipd_ctl_reg.s.opc_mode = cache_mode;
	ipd_ctl_reg.s.pbp_en = back_pres_enable_flag;
	csr_wr(CVMX_IPD_CTL_STATUS, ipd_ctl_reg.u64);

	/* Note: the example RED code is below */
}

/**
 * Enable IPD
 */
void cvmx_ipd_enable(void)
{
	cvmx_ipd_ctl_status_t ipd_reg;

	ipd_reg.u64 = csr_rd(CVMX_IPD_CTL_STATUS);

	/*
	 * busy-waiting for rst_done in o68
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		while (ipd_reg.s.rst_done != 0)
			ipd_reg.u64 = csr_rd(CVMX_IPD_CTL_STATUS);

	if (ipd_reg.s.ipd_en)
		debug("Warning: Enabling IPD when IPD already enabled.\n");

	ipd_reg.s.ipd_en = 1;

	if (cvmx_ipd_cfg.enable_len_M8_fix)
		ipd_reg.s.len_m8 = 1;

	csr_wr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);
}
