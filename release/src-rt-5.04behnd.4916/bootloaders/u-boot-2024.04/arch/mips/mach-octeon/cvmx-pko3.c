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

static const int debug;

#define CVMX_DUMP_REGX(reg)						\
	if (debug)							\
		debug("%s=%#llx\n", #reg, (long long)csr_rd_node(node, reg))

static int cvmx_pko_setup_macs(int node);

/*
 * PKO descriptor queue operation error string
 *
 * @param dqstatus is the enumeration returned from hardware,
 *	  PKO_QUERY_RTN_S[DQSTATUS].
 *
 * @return static constant string error description
 */
const char *pko_dqstatus_error(pko_query_dqstatus_t dqstatus)
{
	char *str = "PKO Undefined error";

	switch (dqstatus) {
	case PKO_DQSTATUS_PASS:
		str = "No error";
		break;
	case PKO_DQSTATUS_BADSTATE:
		str = "PKO queue not ready";
		break;
	case PKO_DQSTATUS_NOFPABUF:
		str = "PKO failed to allocate buffer from FPA";
		break;
	case PKO_DQSTATUS_NOPKOBUF:
		str = "PKO out of buffers";
		break;
	case PKO_DQSTATUS_FAILRTNPTR:
		str = "PKO failed to return buffer to FPA";
		break;
	case PKO_DQSTATUS_ALREADY:
		str = "PKO queue already opened";
		break;
	case PKO_DQSTATUS_NOTCREATED:
		str = "PKO queue has not been created";
		break;
	case PKO_DQSTATUS_NOTEMPTY:
		str = "PKO queue is not empty";
		break;
	case PKO_DQSTATUS_SENDPKTDROP:
		str = "Illegal PKO command construct";
		break;
	}
	return str;
}

/*
 * PKO global initialization for 78XX.
 *
 * @param node is the node on which PKO block is initialized.
 * @return none.
 */
int cvmx_pko3_hw_init_global(int node, uint16_t aura)
{
	cvmx_pko_dpfi_flush_t pko_flush;
	cvmx_pko_dpfi_fpa_aura_t pko_aura;
	cvmx_pko_dpfi_ena_t dpfi_enable;
	cvmx_pko_ptf_iobp_cfg_t ptf_iobp_cfg;
	cvmx_pko_pdm_cfg_t pko_pdm_cfg;
	cvmx_pko_enable_t pko_enable;
	cvmx_pko_dpfi_status_t dpfi_status;
	cvmx_pko_status_t pko_status;
	cvmx_pko_shaper_cfg_t shaper_cfg;
	u64 cycles;
	const unsigned int timeout = 100; /* 100 milliseconds */

	if (node != (aura >> 10))
		cvmx_printf("WARNING: AURA vs PKO node mismatch\n");

	pko_enable.u64 = csr_rd_node(node, CVMX_PKO_ENABLE);
	if (pko_enable.s.enable) {
		cvmx_printf("WARNING: %s: PKO already enabled on node %u\n",
			    __func__, node);
		return 0;
	}
	/* Enable color awareness. */
	shaper_cfg.u64 = csr_rd_node(node, CVMX_PKO_SHAPER_CFG);
	shaper_cfg.s.color_aware = 1;
	csr_wr_node(node, CVMX_PKO_SHAPER_CFG, shaper_cfg.u64);

	/* Clear FLUSH command to be sure */
	pko_flush.u64 = 0;
	pko_flush.s.flush_en = 0;
	csr_wr_node(node, CVMX_PKO_DPFI_FLUSH, pko_flush.u64);

	/* set the aura number in pko, use aura node from parameter */
	pko_aura.u64 = 0;
	pko_aura.s.node = aura >> 10;
	pko_aura.s.laura = aura;
	csr_wr_node(node, CVMX_PKO_DPFI_FPA_AURA, pko_aura.u64);

	CVMX_DUMP_REGX(CVMX_PKO_DPFI_FPA_AURA);

	dpfi_enable.u64 = 0;
	dpfi_enable.s.enable = 1;
	csr_wr_node(node, CVMX_PKO_DPFI_ENA, dpfi_enable.u64);

	/* Prepare timeout */
	cycles = get_timer(0);

	/* Wait until all pointers have been returned */
	do {
		pko_status.u64 = csr_rd_node(node, CVMX_PKO_STATUS);
		if (get_timer(cycles) > timeout)
			break;
	} while (!pko_status.s.pko_rdy);

	if (!pko_status.s.pko_rdy) {
		dpfi_status.u64 = csr_rd_node(node, CVMX_PKO_DPFI_STATUS);
		cvmx_printf("ERROR: %s: PKO DFPI failed, PKO_STATUS=%#llx DPFI_STATUS=%#llx\n",
			    __func__, (unsigned long long)pko_status.u64,
			    (unsigned long long)dpfi_status.u64);
		return -1;
	}

	/* Set max outstanding requests in IOBP for any FIFO.*/
	ptf_iobp_cfg.u64 = csr_rd_node(node, CVMX_PKO_PTF_IOBP_CFG);
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		ptf_iobp_cfg.s.max_read_size = 0x10; /* Recommended by HRM.*/
	else
		/* Reduce the value from recommended 0x10 to avoid
		 * getting "underflow" condition in the BGX TX FIFO.
		 */
		ptf_iobp_cfg.s.max_read_size = 3;
	csr_wr_node(node, CVMX_PKO_PTF_IOBP_CFG, ptf_iobp_cfg.u64);

	/* Set minimum packet size per Ethernet standard */
	pko_pdm_cfg.u64 = 0;
	pko_pdm_cfg.s.pko_pad_minlen = 0x3c; /* 60 bytes before FCS */
	csr_wr_node(node, CVMX_PKO_PDM_CFG, pko_pdm_cfg.u64);

	/* Initialize MACs and FIFOs */
	cvmx_pko_setup_macs(node);

	/* enable PKO, although interfaces and queues are not up yet */
	pko_enable.u64 = 0;
	pko_enable.s.enable = 1;
	csr_wr_node(node, CVMX_PKO_ENABLE, pko_enable.u64);

	/* PKO_RDY set indicates successful initialization */
	pko_status.u64 = csr_rd_node(node, CVMX_PKO_STATUS);
	if (pko_status.s.pko_rdy)
		return 0;

	cvmx_printf("ERROR: %s: failed, PKO_STATUS=%#llx\n", __func__,
		    (unsigned long long)pko_status.u64);
	return -1;
}

/*
 * Configure Channel credit level in PKO.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param level specifies the level at which pko channel queues will be configured,
 * @return returns 0 if successful and -1 on failure.
 */
int cvmx_pko3_channel_credit_level(int node, enum cvmx_pko3_level_e level)
{
	union cvmx_pko_channel_level channel_level;

	channel_level.u64 = 0;

	if (level == CVMX_PKO_L2_QUEUES)
		channel_level.s.cc_level = 0;
	else if (level == CVMX_PKO_L3_QUEUES)
		channel_level.s.cc_level = 1;
	else
		return -1;

	csr_wr_node(node, CVMX_PKO_CHANNEL_LEVEL, channel_level.u64);

	return 0;
}

/** Open configured descriptor queues before queueing packets into them.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns 0 on success or -1 on failure.
 */
int cvmx_pko_dq_open(int node, int dq)
{
	cvmx_pko_query_rtn_t pko_status;
	pko_query_dqstatus_t dqstatus;
	cvmx_pko3_dq_params_t *p_param;

	if (debug)
		debug("%s: DEBUG: dq %u\n", __func__, dq);

	__cvmx_pko3_dq_param_setup(node);

	pko_status = __cvmx_pko3_do_dma(node, dq, NULL, 0, CVMX_PKO_DQ_OPEN);

	dqstatus = pko_status.s.dqstatus;

	if (dqstatus == PKO_DQSTATUS_ALREADY)
		return 0;
	if (dqstatus != PKO_DQSTATUS_PASS) {
		cvmx_printf("%s: ERROR: Failed to open dq :%u: %s\n", __func__,
			    dq, pko_dqstatus_error(dqstatus));
		return -1;
	}

	/* Setup the descriptor queue software parameters */
	p_param = cvmx_pko3_dq_parameters(node, dq);
	if (p_param) {
		p_param->depth = pko_status.s.depth;
		if (p_param->limit == 0)
			p_param->limit = 1024; /* last-resort default */
	}

	return 0;
}

/*
 * PKO initialization of MACs and FIFOs
 *
 * All MACs are configured and assigned a specific FIFO,
 * and each FIFO is configured with size for a best utilization
 * of available FIFO resources.
 *
 * @param node is to specify which node's pko block for this setup.
 * @return returns 0 if successful and -1 on failure.
 *
 * Note: This function contains model-specific code.
 */
static int cvmx_pko_setup_macs(int node)
{
	unsigned int interface;
	unsigned int port, num_ports;
	unsigned int mac_num, fifo, pri, cnt;
	cvmx_helper_interface_mode_t mode;
	const unsigned int num_interfaces =
		cvmx_helper_get_number_of_interfaces();
	u8 fifo_group_cfg[8];
	u8 fifo_group_spd[8];
	unsigned int fifo_count = 0;
	unsigned int max_fifos = 0, fifo_groups = 0;
	struct {
		u8 fifo_cnt;
		u8 fifo_id;
		u8 pri;
		u8 spd;
		u8 mac_fifo_cnt;
	} cvmx_pko3_mac_table[32];

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		max_fifos = 28;	 /* exclusive of NULL FIFO */
		fifo_groups = 8; /* inclusive of NULL PTGF */
	}
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		max_fifos = 16;
		fifo_groups = 5;
	}

	/* Initialize FIFO allocation table */
	memset(&fifo_group_cfg, 0, sizeof(fifo_group_cfg));
	memset(&fifo_group_spd, 0, sizeof(fifo_group_spd));
	memset(cvmx_pko3_mac_table, 0, sizeof(cvmx_pko3_mac_table));

	/* Initialize all MACs as disabled */
	for (mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko3_mac_table[mac_num].pri = 0;
		cvmx_pko3_mac_table[mac_num].fifo_cnt = 0;
		cvmx_pko3_mac_table[mac_num].fifo_id = 0x1f;
	}

	for (interface = 0; interface < num_interfaces; interface++) {
		int xiface =
			cvmx_helper_node_interface_to_xiface(node, interface);
		/* Interface type for ALL interfaces */
		mode = cvmx_helper_interface_get_mode(xiface);
		num_ports = cvmx_helper_interface_enumerate(xiface);

		if (mode == CVMX_HELPER_INTERFACE_MODE_DISABLED)
			continue;
		/*
		 * Non-BGX interfaces:
		 * Each of these interfaces has a single MAC really.
		 */
		if (mode == CVMX_HELPER_INTERFACE_MODE_ILK ||
		    mode == CVMX_HELPER_INTERFACE_MODE_NPI ||
		    mode == CVMX_HELPER_INTERFACE_MODE_LOOP)
			num_ports = 1;

		for (port = 0; port < num_ports; port++) {
			int i;

			/* Get the per-port mode for BGX-interfaces */
			if (interface < CVMX_HELPER_MAX_GMX)
				mode = cvmx_helper_bgx_get_mode(xiface, port);
			/* In MIXED mode, LMACs can run different protocols */

			/* convert interface/port to mac number */
			i = __cvmx_pko3_get_mac_num(xiface, port);
			if (i < 0 || i >= (int)__cvmx_pko3_num_macs()) {
				cvmx_printf("%s: ERROR: interface %d:%u port %d has no MAC %d/%d\n",
					    __func__, node, interface, port, i,
					    __cvmx_pko3_num_macs());
				continue;
			}

			if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) {
				unsigned int bgx_fifo_size =
					__cvmx_helper_bgx_fifo_size(xiface,
								    port);

				cvmx_pko3_mac_table[i].mac_fifo_cnt =
					bgx_fifo_size /
					(CVMX_BGX_TX_FIFO_SIZE / 4);
				cvmx_pko3_mac_table[i].pri = 2;
				cvmx_pko3_mac_table[i].spd = 10;
				cvmx_pko3_mac_table[i].fifo_cnt = 2;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI) {
				unsigned int bgx_fifo_size =
					__cvmx_helper_bgx_fifo_size(xiface,
								    port);

				cvmx_pko3_mac_table[i].mac_fifo_cnt =
					bgx_fifo_size /
					(CVMX_BGX_TX_FIFO_SIZE / 4);
				cvmx_pko3_mac_table[i].pri = 4;
				cvmx_pko3_mac_table[i].spd = 40;
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI) {
				unsigned int bgx_fifo_size =
					__cvmx_helper_bgx_fifo_size(xiface,
								    port);

				cvmx_pko3_mac_table[i].mac_fifo_cnt =
					bgx_fifo_size /
					(CVMX_BGX_TX_FIFO_SIZE / 4);
				cvmx_pko3_mac_table[i].pri = 3;
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				/* DXAUI at 20G, or XAU at 10G */
				cvmx_pko3_mac_table[i].spd = 20;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XFI) {
				unsigned int bgx_fifo_size =
					__cvmx_helper_bgx_fifo_size(xiface,
								    port);

				cvmx_pko3_mac_table[i].mac_fifo_cnt =
					bgx_fifo_size /
					(CVMX_BGX_TX_FIFO_SIZE / 4);
				cvmx_pko3_mac_table[i].pri = 3;
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				cvmx_pko3_mac_table[i].spd = 10;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
				cvmx_pko3_mac_table[i].fifo_cnt = 1;
				cvmx_pko3_mac_table[i].pri = 1;
				cvmx_pko3_mac_table[i].spd = 1;
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 1;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_ILK ||
				   mode == CVMX_HELPER_INTERFACE_MODE_SRIO) {
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				cvmx_pko3_mac_table[i].pri = 3;
				/* ILK/SRIO: speed depends on lane count */
				cvmx_pko3_mac_table[i].spd = 40;
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 4;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				cvmx_pko3_mac_table[i].pri = 2;
				/* Actual speed depends on PCIe lanes/mode */
				cvmx_pko3_mac_table[i].spd = 50;
				/* SLI Tx FIFO size to be revisitted */
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 1;
			} else {
				/* Other BGX interface modes: SGMII/RGMII */
				unsigned int bgx_fifo_size =
					__cvmx_helper_bgx_fifo_size(xiface,
								    port);

				cvmx_pko3_mac_table[i].mac_fifo_cnt =
					bgx_fifo_size /
					(CVMX_BGX_TX_FIFO_SIZE / 4);
				cvmx_pko3_mac_table[i].fifo_cnt = 1;
				cvmx_pko3_mac_table[i].pri = 1;
				cvmx_pko3_mac_table[i].spd = 1;
			}

			if (debug)
				debug("%s: intf %d:%u port %u %s mac %02u cnt %u macfifo %uk spd %u\n",
				      __func__, node, interface, port,
				      cvmx_helper_interface_mode_to_string(mode),
				      i, cvmx_pko3_mac_table[i].fifo_cnt,
				      cvmx_pko3_mac_table[i].mac_fifo_cnt * 8,
				      cvmx_pko3_mac_table[i].spd);

		} /* for port */
	}	  /* for interface */

	/* Count the number of requested FIFOs */
	for (fifo_count = mac_num = 0; mac_num < __cvmx_pko3_num_macs();
	     mac_num++)
		fifo_count += cvmx_pko3_mac_table[mac_num].fifo_cnt;

	if (debug)
		debug("%s: initially requested FIFO count %u\n", __func__,
		      fifo_count);

	/* Heuristically trim FIFO count to fit in available number */
	pri = 1;
	cnt = 4;
	while (fifo_count > max_fifos) {
		for (mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
			if (cvmx_pko3_mac_table[mac_num].fifo_cnt == cnt &&
			    cvmx_pko3_mac_table[mac_num].pri <= pri) {
				cvmx_pko3_mac_table[mac_num].fifo_cnt >>= 1;
				fifo_count -=
					cvmx_pko3_mac_table[mac_num].fifo_cnt;
			}
			if (fifo_count <= max_fifos)
				break;
		}
		if (pri >= 4) {
			pri = 1;
			cnt >>= 1;
		} else {
			pri++;
		}
		if (cnt == 0)
			break;
	}

	if (debug)
		debug("%s: adjusted FIFO count %u\n", __func__, fifo_count);

	/* Special case for NULL Virtual FIFO */
	fifo_group_cfg[fifo_groups - 1] = 0;
	/* there is no MAC connected to NULL FIFO */

	/* Configure MAC units, and attach a FIFO to each */
	for (fifo = 0, cnt = 4; cnt > 0; cnt >>= 1) {
		unsigned int g;

		for (mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
			if (cvmx_pko3_mac_table[mac_num].fifo_cnt < cnt ||
			    cvmx_pko3_mac_table[mac_num].fifo_id != 0x1f)
				continue;

			/* Attach FIFO to MAC */
			cvmx_pko3_mac_table[mac_num].fifo_id = fifo;
			g = fifo >> 2;
			/* Sum speed for FIFO group */
			fifo_group_spd[g] += cvmx_pko3_mac_table[mac_num].spd;

			if (cnt == 4)
				fifo_group_cfg[g] = 4; /* 10k,0,0,0 */
			else if (cnt == 2 && (fifo & 0x3) == 0)
				fifo_group_cfg[g] = 3; /* 5k,0,5k,0 */
			else if (cnt == 2 && fifo_group_cfg[g] == 3)
				/* no change */;
			else if (cnt == 1 && (fifo & 0x2) &&
				 fifo_group_cfg[g] == 3)
				fifo_group_cfg[g] = 1; /* 5k,0,2.5k 2.5k*/
			else if (cnt == 1 && (fifo & 0x3) == 0x3)
				/* no change */;
			else if (cnt == 1)
				fifo_group_cfg[g] = 0; /* 2.5k x 4 */
			else
				cvmx_printf("ERROR: %s: internal error\n",
					    __func__);

			fifo += cnt;
		}
	}

	/* Check if there was no error in FIFO allocation */
	if (fifo > max_fifos) {
		cvmx_printf("ERROR: %s: Internal error FIFO %u\n", __func__,
			    fifo);
		return -1;
	}

	if (debug)
		debug("%s: used %u of FIFOs\n", __func__, fifo);

	/* Now configure all FIFO groups */
	for (fifo = 0; fifo < fifo_groups; fifo++) {
		cvmx_pko_ptgfx_cfg_t pko_ptgfx_cfg;

		pko_ptgfx_cfg.u64 = csr_rd_node(node, CVMX_PKO_PTGFX_CFG(fifo));
		if (pko_ptgfx_cfg.s.size != fifo_group_cfg[fifo])
			pko_ptgfx_cfg.s.reset = 1;
		pko_ptgfx_cfg.s.size = fifo_group_cfg[fifo];
		if (fifo_group_spd[fifo] >= 40)
			if (pko_ptgfx_cfg.s.size >= 3)
				pko_ptgfx_cfg.s.rate = 3; /* 50 Gbps */
			else
				pko_ptgfx_cfg.s.rate = 2; /* 25 Gbps */
		else if (fifo_group_spd[fifo] >= 20)
			pko_ptgfx_cfg.s.rate = 2; /* 25 Gbps */
		else if (fifo_group_spd[fifo] >= 10)
			pko_ptgfx_cfg.s.rate = 1; /* 12.5 Gbps */
		else
			pko_ptgfx_cfg.s.rate = 0; /* 6.25 Gbps */

		if (debug)
			debug("%s: FIFO %#x-%#x size=%u speed=%d rate=%d\n",
			      __func__, fifo * 4, fifo * 4 + 3,
			      pko_ptgfx_cfg.s.size, fifo_group_spd[fifo],
			      pko_ptgfx_cfg.s.rate);

		csr_wr_node(node, CVMX_PKO_PTGFX_CFG(fifo), pko_ptgfx_cfg.u64);
		pko_ptgfx_cfg.s.reset = 0;
		csr_wr_node(node, CVMX_PKO_PTGFX_CFG(fifo), pko_ptgfx_cfg.u64);
	}

	/* Configure all MACs assigned FIFO number */
	for (mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko_macx_cfg_t pko_mac_cfg;

		if (debug)
			debug("%s: mac#%02u: fifo=%#x cnt=%u speed=%d\n",
			      __func__, mac_num,
			      cvmx_pko3_mac_table[mac_num].fifo_id,
			      cvmx_pko3_mac_table[mac_num].fifo_cnt,
			      cvmx_pko3_mac_table[mac_num].spd);

		pko_mac_cfg.u64 = csr_rd_node(node, CVMX_PKO_MACX_CFG(mac_num));
		pko_mac_cfg.s.fifo_num = cvmx_pko3_mac_table[mac_num].fifo_id;
		csr_wr_node(node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);
	}

	/* Setup PKO MCI0/MCI1/SKID credits */
	for (mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko_mci0_max_credx_t pko_mci0_max_cred;
		cvmx_pko_mci1_max_credx_t pko_mci1_max_cred;
		cvmx_pko_macx_cfg_t pko_mac_cfg;
		unsigned int fifo_credit, mac_credit, skid_credit;
		unsigned int pko_fifo_cnt, fifo_size;
		unsigned int mac_fifo_cnt;
		unsigned int tmp;
		int saved_fifo_num;

		pko_fifo_cnt = cvmx_pko3_mac_table[mac_num].fifo_cnt;
		mac_fifo_cnt = cvmx_pko3_mac_table[mac_num].mac_fifo_cnt;

		/* Skip unused MACs */
		if (pko_fifo_cnt == 0)
			continue;

		/* Check for sanity */
		if (pko_fifo_cnt > 4)
			pko_fifo_cnt = 1;

		fifo_size = (2 * 1024) + (1024 / 2); /* 2.5KiB */
		fifo_credit = pko_fifo_cnt * fifo_size;

		if (mac_num == 0) {
			/* loopback */
			mac_credit = 4096; /* From HRM Sec 13.0 */
			skid_credit = 0;
		} else if (mac_num == 1) {
			/* DPI */
			mac_credit = 2 * 1024;
			skid_credit = 0;
		} else if (octeon_has_feature(OCTEON_FEATURE_ILK) &&
			   (mac_num & 0xfe) == 2) {
			/* ILK0, ILK1: MAC 2,3 */
			mac_credit = 4 * 1024; /* 4KB fifo */
			skid_credit = 0;
		} else if (octeon_has_feature(OCTEON_FEATURE_SRIO) &&
			   (mac_num >= 6) && (mac_num <= 9)) {
			/* SRIO0, SRIO1: MAC 6..9 */
			mac_credit = 1024 / 2;
			skid_credit = 0;
		} else {
			/* BGX */
			mac_credit = mac_fifo_cnt * 8 * 1024;
			skid_credit = mac_fifo_cnt * 256;
		}

		if (debug)
			debug("%s: mac %u pko_fifo_credit=%u mac_credit=%u\n",
			      __func__, mac_num, fifo_credit, mac_credit);

		tmp = (fifo_credit + mac_credit) / 16;
		pko_mci0_max_cred.u64 = 0;
		pko_mci0_max_cred.s.max_cred_lim = tmp;

		/* Check for overflow */
		if (pko_mci0_max_cred.s.max_cred_lim != tmp) {
			cvmx_printf("WARNING: %s: MCI0 credit overflow\n",
				    __func__);
			pko_mci0_max_cred.s.max_cred_lim = 0xfff;
		}

		/* Pass 2 PKO hardware does not use the MCI0 credits */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			csr_wr_node(node, CVMX_PKO_MCI0_MAX_CREDX(mac_num),
				    pko_mci0_max_cred.u64);

		/* The original CSR formula is the correct one after all */
		tmp = (mac_credit) / 16;
		pko_mci1_max_cred.u64 = 0;
		pko_mci1_max_cred.s.max_cred_lim = tmp;

		/* Check for overflow */
		if (pko_mci1_max_cred.s.max_cred_lim != tmp) {
			cvmx_printf("WARNING: %s: MCI1 credit overflow\n",
				    __func__);
			pko_mci1_max_cred.s.max_cred_lim = 0xfff;
		}

		csr_wr_node(node, CVMX_PKO_MCI1_MAX_CREDX(mac_num),
			    pko_mci1_max_cred.u64);

		tmp = (skid_credit / 256) >> 1; /* valid 0,1,2 */
		pko_mac_cfg.u64 = csr_rd_node(node, CVMX_PKO_MACX_CFG(mac_num));

		/* The PKO_MACX_CFG bits cannot be changed unless FIFO_MUM=0x1f (unused fifo) */
		saved_fifo_num = pko_mac_cfg.s.fifo_num;
		pko_mac_cfg.s.fifo_num = 0x1f;
		pko_mac_cfg.s.skid_max_cnt = tmp;
		csr_wr_node(node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);

		pko_mac_cfg.u64 = csr_rd_node(node, CVMX_PKO_MACX_CFG(mac_num));
		pko_mac_cfg.s.fifo_num = saved_fifo_num;
		csr_wr_node(node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);

		if (debug) {
			pko_mci0_max_cred.u64 =
				csr_rd_node(node, CVMX_PKO_MCI0_MAX_CREDX(mac_num));
			pko_mci1_max_cred.u64 =
				csr_rd_node(node, CVMX_PKO_MCI1_MAX_CREDX(mac_num));
			pko_mac_cfg.u64 =
				csr_rd_node(node, CVMX_PKO_MACX_CFG(mac_num));
			debug("%s: mac %u PKO_MCI0_MAX_CREDX=%u PKO_MCI1_MAX_CREDX=%u PKO_MACX_CFG[SKID_MAX_CNT]=%u\n",
			      __func__, mac_num,
			      pko_mci0_max_cred.s.max_cred_lim,
			      pko_mci1_max_cred.s.max_cred_lim,
			      pko_mac_cfg.s.skid_max_cnt);
		}
	} /* for mac_num */

	return 0;
}

/** Set MAC options
 *
 * The options supported are the parameters below:
 *
 * @param xiface The physical interface number
 * @param index The physical sub-interface port
 * @param fcs_enable Enable FCS generation
 * @param pad_enable Enable padding to minimum packet size
 * @param fcs_sop_off Number of bytes at start of packet to exclude from FCS
 *
 * The typical use for `fcs_sop_off` is when the interface is configured
 * to use a header such as HighGig to precede every Ethernet packet,
 * such a header usually does not partake in the CRC32 computation stream,
 * and its size must be set with this parameter.
 *
 * @return Returns 0 on success, -1 if interface/port is invalid.
 */
int cvmx_pko3_interface_options(int xiface, int index, bool fcs_enable,
				bool pad_enable, unsigned int fcs_sop_off)
{
	int mac_num;
	cvmx_pko_macx_cfg_t pko_mac_cfg;
	unsigned int fifo_num;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (debug)
		debug("%s: intf %u:%u/%u fcs=%d pad=%d\n", __func__, xi.node,
		      xi.interface, index, fcs_enable, pad_enable);

	mac_num = __cvmx_pko3_get_mac_num(xiface, index);
	if (mac_num < 0) {
		cvmx_printf("ERROR: %s: invalid interface %u:%u/%u\n", __func__,
			    xi.node, xi.interface, index);
		return -1;
	}

	pko_mac_cfg.u64 = csr_rd_node(xi.node, CVMX_PKO_MACX_CFG(mac_num));

	/* If MAC is not assigned, return an error */
	if (pko_mac_cfg.s.fifo_num == 0x1f) {
		cvmx_printf("ERROR: %s: unused interface %u:%u/%u\n", __func__,
			    xi.node, xi.interface, index);
		return -1;
	}

	if (pko_mac_cfg.s.min_pad_ena == pad_enable &&
	    pko_mac_cfg.s.fcs_ena == fcs_enable) {
		if (debug)
			debug("%s: mac %#x unchanged\n", __func__, mac_num);
		return 0;
	}

	/* WORKAROUND: Pass1 won't allow change any bits unless FIFO_NUM=0x1f */
	fifo_num = pko_mac_cfg.s.fifo_num;
	pko_mac_cfg.s.fifo_num = 0x1f;

	pko_mac_cfg.s.min_pad_ena = pad_enable;
	pko_mac_cfg.s.fcs_ena = fcs_enable;
	pko_mac_cfg.s.fcs_sop_off = fcs_sop_off;

	csr_wr_node(xi.node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);

	pko_mac_cfg.s.fifo_num = fifo_num;
	csr_wr_node(xi.node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);

	if (debug)
		debug("%s: PKO_MAC[%u]CFG=%#llx\n", __func__, mac_num,
		      (unsigned long long)csr_rd_node(xi.node, CVMX_PKO_MACX_CFG(mac_num)));

	return 0;
}

/** Set Descriptor Queue options
 *
 * The `min_pad` parameter must be in agreement with the interface-level
 * padding option for all descriptor queues assigned to that particular
 * interface/port.
 *
 * @param node on which to operate
 * @param dq descriptor queue to set
 * @param min_pad minimum padding to set for dq
 */
void cvmx_pko3_dq_options(unsigned int node, unsigned int dq, bool min_pad)
{
	cvmx_pko_pdm_dqx_minpad_t reg;

	dq &= (1 << 10) - 1;
	reg.u64 = csr_rd_node(node, CVMX_PKO_PDM_DQX_MINPAD(dq));
	reg.s.minpad = min_pad;
	csr_wr_node(node, CVMX_PKO_PDM_DQX_MINPAD(dq), reg.u64);
}
