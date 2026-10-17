// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Small helper utilities.
 */

#include <log.h>
#include <time.h>
#include <linux/delay.h>

#include <mach/cvmx-regs.h>
#include <mach/cvmx-csr-enums.h>
#include <mach/octeon-model.h>
#include <mach/octeon-feature.h>
#include <mach/cvmx-gmxx-defs.h>
#include <mach/cvmx-ipd-defs.h>
#include <mach/cvmx-pko-defs.h>
#include <mach/cvmx-ipd.h>
#include <mach/cvmx-hwpko.h>
#include <mach/cvmx-pki.h>
#include <mach/cvmx-pip.h>
#include <mach/cvmx-helper.h>
#include <mach/cvmx-helper-util.h>
#include <mach/cvmx-helper-pki.h>

/**
 * @INTERNAL
 * These are the interface types needed to convert interface numbers to ipd
 * ports.
 *
 * @param GMII
 *	This type is used for sgmii, rgmii, xaui and rxaui interfaces.
 * @param ILK
 *	This type is used for ilk interfaces.
 * @param SRIO
 *	This type is used for serial-RapidIo interfaces.
 * @param NPI
 *	This type is used for npi interfaces.
 * @param LB
 *	This type is used for loopback interfaces.
 * @param INVALID_IF_TYPE
 *	This type indicates the interface hasn't been configured.
 */
enum port_map_if_type { INVALID_IF_TYPE = 0, GMII, ILK, SRIO, NPI, LB };

/**
 * @INTERNAL
 * This structure is used to map interface numbers to ipd ports.
 *
 * @param type
 *	Interface type
 * @param first_ipd_port
 *	First IPD port number assigned to this interface.
 * @param last_ipd_port
 *	Last IPD port number assigned to this interface.
 * @param ipd_port_adj
 *	Different octeon chips require different ipd ports for the
 *	same interface port/mode configuration. This value is used
 *	to account for that difference.
 */
struct ipd_port_map {
	enum port_map_if_type type;
	int first_ipd_port;
	int last_ipd_port;
	int ipd_port_adj;
};

/**
 * @INTERNAL
 * Interface number to ipd port map for the octeon 68xx.
 */
static const struct ipd_port_map ipd_port_map_68xx[CVMX_HELPER_MAX_IFACE] = {
	{ GMII, 0x800, 0x8ff, 0x40 }, /* Interface 0 */
	{ GMII, 0x900, 0x9ff, 0x40 }, /* Interface 1 */
	{ GMII, 0xa00, 0xaff, 0x40 }, /* Interface 2 */
	{ GMII, 0xb00, 0xbff, 0x40 }, /* Interface 3 */
	{ GMII, 0xc00, 0xcff, 0x40 }, /* Interface 4 */
	{ ILK, 0x400, 0x4ff, 0x00 },  /* Interface 5 */
	{ ILK, 0x500, 0x5ff, 0x00 },  /* Interface 6 */
	{ NPI, 0x100, 0x120, 0x00 },  /* Interface 7 */
	{ LB, 0x000, 0x008, 0x00 },   /* Interface 8 */
};

/**
 * @INTERNAL
 * Interface number to ipd port map for the octeon 78xx.
 *
 * This mapping corresponds to WQE(CHAN) enumeration in
 * HRM Sections 11.15, PKI_CHAN_E, Section 11.6
 *
 */
static const struct ipd_port_map ipd_port_map_78xx[CVMX_HELPER_MAX_IFACE] = {
	{ GMII, 0x800, 0x83f, 0x00 }, /* Interface 0 - BGX0 */
	{ GMII, 0x900, 0x93f, 0x00 }, /* Interface 1  -BGX1 */
	{ GMII, 0xa00, 0xa3f, 0x00 }, /* Interface 2  -BGX2 */
	{ GMII, 0xb00, 0xb3f, 0x00 }, /* Interface 3 - BGX3 */
	{ GMII, 0xc00, 0xc3f, 0x00 }, /* Interface 4 - BGX4 */
	{ GMII, 0xd00, 0xd3f, 0x00 }, /* Interface 5 - BGX5 */
	{ ILK, 0x400, 0x4ff, 0x00 },  /* Interface 6 - ILK0 */
	{ ILK, 0x500, 0x5ff, 0x00 },  /* Interface 7 - ILK1 */
	{ NPI, 0x100, 0x13f, 0x00 },  /* Interface 8 - DPI */
	{ LB, 0x000, 0x03f, 0x00 },   /* Interface 9 - LOOPBACK */
};

/**
 * @INTERNAL
 * Interface number to ipd port map for the octeon 73xx.
 */
static const struct ipd_port_map ipd_port_map_73xx[CVMX_HELPER_MAX_IFACE] = {
	{ GMII, 0x800, 0x83f, 0x00 }, /* Interface 0 - BGX(0,0-3) */
	{ GMII, 0x900, 0x93f, 0x00 }, /* Interface 1  -BGX(1,0-3) */
	{ GMII, 0xa00, 0xa3f, 0x00 }, /* Interface 2  -BGX(2,0-3) */
	{ NPI, 0x100, 0x17f, 0x00 },  /* Interface 3 - DPI */
	{ LB, 0x000, 0x03f, 0x00 },   /* Interface 4 - LOOPBACK */
};

/**
 * @INTERNAL
 * Interface number to ipd port map for the octeon 75xx.
 */
static const struct ipd_port_map ipd_port_map_75xx[CVMX_HELPER_MAX_IFACE] = {
	{ GMII, 0x800, 0x83f, 0x00 }, /* Interface 0 - BGX0 */
	{ SRIO, 0x240, 0x241, 0x00 }, /* Interface 1 - SRIO 0 */
	{ SRIO, 0x242, 0x243, 0x00 }, /* Interface 2 - SRIO 1 */
	{ NPI, 0x100, 0x13f, 0x00 },  /* Interface 3 - DPI */
	{ LB, 0x000, 0x03f, 0x00 },   /* Interface 4 - LOOPBACK */
};

/**
 * Convert a interface mode into a human readable string
 *
 * @param mode   Mode to convert
 *
 * Return: String
 */
const char *cvmx_helper_interface_mode_to_string(cvmx_helper_interface_mode_t mode)
{
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_DISABLED:
		return "DISABLED";
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
		return "RGMII";
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		return "GMII";
	case CVMX_HELPER_INTERFACE_MODE_SPI:
		return "SPI";
	case CVMX_HELPER_INTERFACE_MODE_PCIE:
		return "PCIE";
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		return "XAUI";
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		return "RXAUI";
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		return "SGMII";
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		return "QSGMII";
	case CVMX_HELPER_INTERFACE_MODE_PICMG:
		return "PICMG";
	case CVMX_HELPER_INTERFACE_MODE_NPI:
		return "NPI";
	case CVMX_HELPER_INTERFACE_MODE_LOOP:
		return "LOOP";
	case CVMX_HELPER_INTERFACE_MODE_SRIO:
		return "SRIO";
	case CVMX_HELPER_INTERFACE_MODE_ILK:
		return "ILK";
	case CVMX_HELPER_INTERFACE_MODE_AGL:
		return "AGL";
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
		return "XLAUI";
	case CVMX_HELPER_INTERFACE_MODE_XFI:
		return "XFI";
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
		return "40G_KR4";
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
		return "10G_KR";
	case CVMX_HELPER_INTERFACE_MODE_MIXED:
		return "MIXED";
	}
	return "UNKNOWN";
}

/**
 * @INTERNAL
 *
 * Extract NO_WPTR mode from PIP/IPD register
 */
static int __cvmx_ipd_mode_no_wptr(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_NO_WPTR)) {
		cvmx_ipd_ctl_status_t ipd_ctl_status;

		ipd_ctl_status.u64 = csr_rd(CVMX_IPD_CTL_STATUS);
		return ipd_ctl_status.s.no_wptr;
	}
	return 0;
}

static cvmx_buf_ptr_t __cvmx_packet_short_ptr[4];
static int8_t __cvmx_wqe_pool = -1;

/**
 * @INTERNAL
 * Prepare packet pointer templace for dynamic short
 * packets.
 */
static void cvmx_packet_short_ptr_calculate(void)
{
	unsigned int i, off;
	union cvmx_pip_gbl_cfg pip_gbl_cfg;
	union cvmx_pip_ip_offset pip_ip_offset;

	/* Fill in the common values for all cases */
	for (i = 0; i < 4; i++) {
		if (__cvmx_ipd_mode_no_wptr())
			/* packet pool, set to 0 in hardware */
			__cvmx_wqe_pool = 0;
		else
			/* WQE pool as configured */
			__cvmx_wqe_pool = csr_rd(CVMX_IPD_WQE_FPA_QUEUE) & 7;

		__cvmx_packet_short_ptr[i].s.pool = __cvmx_wqe_pool;
		__cvmx_packet_short_ptr[i].s.size = cvmx_fpa_get_block_size(__cvmx_wqe_pool);
		__cvmx_packet_short_ptr[i].s.size -= 32;
		__cvmx_packet_short_ptr[i].s.addr = 32;
	}

	pip_gbl_cfg.u64 = csr_rd(CVMX_PIP_GBL_CFG);
	pip_ip_offset.u64 = csr_rd(CVMX_PIP_IP_OFFSET);

	/* RAW_FULL: index = 0 */
	i = 0;
	off = pip_gbl_cfg.s.raw_shf;
	__cvmx_packet_short_ptr[i].s.addr += off;
	__cvmx_packet_short_ptr[i].s.size -= off;
	__cvmx_packet_short_ptr[i].s.back += off >> 7;

	/* NON-IP: index = 1 */
	i = 1;
	off = pip_gbl_cfg.s.nip_shf;
	__cvmx_packet_short_ptr[i].s.addr += off;
	__cvmx_packet_short_ptr[i].s.size -= off;
	__cvmx_packet_short_ptr[i].s.back += off >> 7;

	/* IPv4: index = 2 */
	i = 2;
	off = (pip_ip_offset.s.offset << 3) + 4;
	__cvmx_packet_short_ptr[i].s.addr += off;
	__cvmx_packet_short_ptr[i].s.size -= off;
	__cvmx_packet_short_ptr[i].s.back += off >> 7;

	/* IPv6: index = 3 */
	i = 3;
	off = (pip_ip_offset.s.offset << 3) + 0;
	__cvmx_packet_short_ptr[i].s.addr += off;
	__cvmx_packet_short_ptr[i].s.size -= off;
	__cvmx_packet_short_ptr[i].s.back += off >> 7;

	/* For IPv4/IPv6: subtract work->word2.s.ip_offset
	 * to addr, if it is smaller than IP_OFFSET[OFFSET]*8
	 * which is stored in __cvmx_packet_short_ptr[3].s.addr
	 */
}

/**
 * Extract packet data buffer pointer from work queue entry.
 *
 * Returns the legacy (Octeon1/Octeon2) buffer pointer structure
 * for the linked buffer list.
 * On CN78XX, the native buffer pointer structure is converted into
 * the legacy format.
 * The legacy buf_ptr is then stored in the WQE, and word0 reserved
 * field is set to indicate that the buffer pointers were translated.
 * If the packet data is only found inside the work queue entry,
 * a standard buffer pointer structure is created for it.
 */
cvmx_buf_ptr_t cvmx_wqe_get_packet_ptr(cvmx_wqe_t *work)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (void *)work;
		cvmx_buf_ptr_t optr, lptr;
		cvmx_buf_ptr_pki_t nptr;
		unsigned int pool, bufs;
		int node = cvmx_get_node_num();

		/* In case of repeated calls of this function */
		if (wqe->pki_wqe_translated || wqe->word2.software) {
			optr.u64 = wqe->packet_ptr.u64;
			return optr;
		}

		bufs = wqe->word0.bufs;
		pool = wqe->word0.aura;
		nptr.u64 = wqe->packet_ptr.u64;

		optr.u64 = 0;
		optr.s.pool = pool;
		optr.s.addr = nptr.addr;
		if (bufs == 1) {
			optr.s.size = pki_dflt_pool[node].buffer_size -
				      pki_dflt_style[node].parm_cfg.first_skip - 8 -
				      wqe->word0.apad;
		} else {
			optr.s.size = nptr.size;
		}

		/* Calculate the "back" offset */
		if (!nptr.packet_outside_wqe) {
			optr.s.back = (nptr.addr -
				       cvmx_ptr_to_phys(wqe)) >> 7;
		} else {
			optr.s.back =
				(pki_dflt_style[node].parm_cfg.first_skip +
				 8 + wqe->word0.apad) >> 7;
		}
		lptr = optr;

		/* Follow pointer and convert all linked pointers */
		while (bufs > 1) {
			void *vptr;

			vptr = cvmx_phys_to_ptr(lptr.s.addr);

			memcpy(&nptr, vptr - 8, 8);
			/*
			 * Errata (PKI-20776) PKI_BUFLINK_S's are endian-swapped
			 * CN78XX pass 1.x has a bug where the packet pointer
			 * in each segment is written in the opposite
			 * endianness of the configured mode. Fix these here
			 */
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				nptr.u64 = __builtin_bswap64(nptr.u64);
			lptr.u64 = 0;
			lptr.s.pool = pool;
			lptr.s.addr = nptr.addr;
			lptr.s.size = nptr.size;
			lptr.s.back = (pki_dflt_style[0].parm_cfg.later_skip + 8) >>
				      7; /* TBD: not guaranteed !! */

			memcpy(vptr - 8, &lptr, 8);
			bufs--;
		}
		/* Store translated bufptr in WQE, and set indicator */
		wqe->pki_wqe_translated = 1;
		wqe->packet_ptr.u64 = optr.u64;
		return optr;

	} else {
		unsigned int i;
		unsigned int off = 0;
		cvmx_buf_ptr_t bptr;

		if (cvmx_likely(work->word2.s.bufs > 0))
			return work->packet_ptr;

		if (cvmx_unlikely(work->word2.s.software))
			return work->packet_ptr;

		/* first packet, precalculate packet_ptr templaces */
		if (cvmx_unlikely(__cvmx_packet_short_ptr[0].u64 == 0))
			cvmx_packet_short_ptr_calculate();

		/* calculate templace index */
		i = work->word2.s_cn38xx.not_IP | work->word2.s_cn38xx.rcv_error;
		i = 2 ^ (i << 1);

		/* IPv4/IPv6: Adjust IP offset */
		if (cvmx_likely(i & 2)) {
			i |= work->word2.s.is_v6;
			off = work->word2.s.ip_offset;
		} else {
			/* RAWFULL/RAWSCHED should be handled here */
			i = 1; /* not-IP */
			off = 0;
		}

		/* Get the right templace */
		bptr = __cvmx_packet_short_ptr[i];
		bptr.s.addr -= off;
		bptr.s.back = bptr.s.addr >> 7;

		/* Add actual WQE paddr to the templace offset */
		bptr.s.addr += cvmx_ptr_to_phys(work);

		/* Adjust word2.bufs so that _free_data() handles it
		 * in the same way as PKO
		 */
		work->word2.s.bufs = 1;

		/* Store the new buffer pointer back into WQE */
		work->packet_ptr = bptr;

		/* Returned the synthetic buffer_pointer */
		return bptr;
	}
}

void cvmx_wqe_free(cvmx_wqe_t *work)
{
	unsigned int bufs, ncl = 1;
	u64 paddr, paddr1;

	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_wqe_78xx_t *wqe = (void *)work;
		cvmx_fpa3_gaura_t aura;
		cvmx_buf_ptr_pki_t bptr;

		bufs = wqe->word0.bufs;

		if (!wqe->pki_wqe_translated && bufs != 0) {
			/* Handle cn78xx native untralsated WQE */

			bptr = wqe->packet_ptr;

			/* Do nothing - first packet buffer shares WQE buffer */
			if (!bptr.packet_outside_wqe)
				return;
		} else if (cvmx_likely(bufs != 0)) {
			/* Handle translated 78XX WQE */
			paddr = (work->packet_ptr.s.addr & (~0x7full)) -
				(work->packet_ptr.s.back << 7);
			paddr1 = cvmx_ptr_to_phys(work);

			/* do not free WQE if contains first data buffer */
			if (paddr == paddr1)
				return;
		}

		/* WQE is separate from packet buffer, free it */
		aura = __cvmx_fpa3_gaura(wqe->word0.aura >> 10, wqe->word0.aura & 0x3ff);

		cvmx_fpa3_free(work, aura, ncl);
	} else {
		/* handle legacy WQE */
		bufs = work->word2.s_cn38xx.bufs;

		if (cvmx_likely(bufs != 0)) {
			/* Check if the first data buffer is inside WQE */
			paddr = (work->packet_ptr.s.addr & (~0x7full)) -
				(work->packet_ptr.s.back << 7);
			paddr1 = cvmx_ptr_to_phys(work);

			/* do not free WQE if contains first data buffer */
			if (paddr == paddr1)
				return;
		}

		/* precalculate packet_ptr, WQE pool number */
		if (cvmx_unlikely(__cvmx_wqe_pool < 0))
			cvmx_packet_short_ptr_calculate();
		cvmx_fpa1_free(work, __cvmx_wqe_pool, ncl);
	}
}

/**
 * Free the packet buffers contained in a work queue entry.
 * The work queue entry is also freed if it contains packet data.
 * If however the packet starts outside the WQE, the WQE will
 * not be freed. The application should call cvmx_wqe_free()
 * to free the WQE buffer that contains no packet data.
 *
 * @param work   Work queue entry with packet to free
 */
void cvmx_helper_free_packet_data(cvmx_wqe_t *work)
{
	u64 number_buffers;
	u64 start_of_buffer;
	u64 next_buffer_ptr;
	cvmx_fpa3_gaura_t aura;
	unsigned int ncl;
	cvmx_buf_ptr_t buffer_ptr;
	cvmx_buf_ptr_pki_t bptr;
	cvmx_wqe_78xx_t *wqe = (void *)work;
	int o3_pki_wqe = 0;

	number_buffers = cvmx_wqe_get_bufs(work);

	buffer_ptr.u64 = work->packet_ptr.u64;

	/* Zero-out WQE WORD3 so that the WQE is freed by cvmx_wqe_free() */
	work->packet_ptr.u64 = 0;

	if (number_buffers == 0)
		return;

	/* Interpret PKI-style bufptr unless it has been translated */
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE) &&
	    !wqe->pki_wqe_translated) {
		o3_pki_wqe = 1;
		cvmx_wqe_pki_errata_20776(work);
		aura = __cvmx_fpa3_gaura(wqe->word0.aura >> 10,
					 wqe->word0.aura & 0x3ff);
	} else {
		start_of_buffer = ((buffer_ptr.s.addr >> 7) -
				   buffer_ptr.s.back) << 7;
		next_buffer_ptr =
			*(uint64_t *)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
		/*
		 * Since the number of buffers is not zero, we know this is not
		 * a dynamic short packet. We need to check if it is a packet
		 * received with IPD_CTL_STATUS[NO_WPTR]. If this is true,
		 * we need to free all buffers except for the first one.
		 * The caller doesn't expect their WQE pointer to be freed
		 */
		if (cvmx_ptr_to_phys(work) == start_of_buffer) {
			buffer_ptr.u64 = next_buffer_ptr;
			number_buffers--;
		}
	}
	while (number_buffers--) {
		if (o3_pki_wqe) {
			bptr.u64 = buffer_ptr.u64;

			ncl = (bptr.size + CVMX_CACHE_LINE_SIZE - 1) /
				CVMX_CACHE_LINE_SIZE;

			/* XXX- assumes the buffer is cache-line aligned */
			start_of_buffer = (bptr.addr >> 7) << 7;

			/*
			 * Read pointer to next buffer before we free the
			 * current buffer.
			 */
			next_buffer_ptr = *(uint64_t *)cvmx_phys_to_ptr(bptr.addr - 8);
			/* FPA AURA comes from WQE, includes node */
			cvmx_fpa3_free(cvmx_phys_to_ptr(start_of_buffer),
				       aura, ncl);
		} else {
			ncl = (buffer_ptr.s.size + CVMX_CACHE_LINE_SIZE - 1) /
				      CVMX_CACHE_LINE_SIZE +
			      buffer_ptr.s.back;
			/*
			 * Calculate buffer start using "back" offset,
			 * Remember the back pointer is in cache lines,
			 * not 64bit words
			 */
			start_of_buffer = ((buffer_ptr.s.addr >> 7) -
					   buffer_ptr.s.back) << 7;
			/*
			 * Read pointer to next buffer before we free
			 * the current buffer.
			 */
			next_buffer_ptr =
				*(uint64_t *)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
			/* FPA pool comes from buf_ptr itself */
			if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
				aura = cvmx_fpa1_pool_to_fpa3_aura(buffer_ptr.s.pool);
				cvmx_fpa3_free(cvmx_phys_to_ptr(start_of_buffer),
					       aura, ncl);
			} else {
				cvmx_fpa1_free(cvmx_phys_to_ptr(start_of_buffer),
					       buffer_ptr.s.pool, ncl);
			}
		}
		buffer_ptr.u64 = next_buffer_ptr;
	}
}

/**
 * @INTERNAL
 * Setup the common GMX settings that determine the number of
 * ports. These setting apply to almost all configurations of all
 * chips.
 *
 * @param xiface Interface to configure
 * @param num_ports Number of ports on the interface
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_setup_gmx(int xiface, int num_ports)
{
	union cvmx_gmxx_tx_prts gmx_tx_prts;
	union cvmx_gmxx_rx_prts gmx_rx_prts;
	union cvmx_pko_reg_gmx_port_mode pko_mode;
	union cvmx_gmxx_txx_thresh gmx_tx_thresh;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int index;

	/*
	 * The common BGX settings are already done in the appropriate
	 * enable functions, nothing to do here.
	 */
	if (octeon_has_feature(OCTEON_FEATURE_BGX))
		return 0;

	/* Tell GMX the number of TX ports on this interface */
	gmx_tx_prts.u64 = csr_rd(CVMX_GMXX_TX_PRTS(xi.interface));
	gmx_tx_prts.s.prts = num_ports;
	csr_wr(CVMX_GMXX_TX_PRTS(xi.interface), gmx_tx_prts.u64);

	/*
	 * Tell GMX the number of RX ports on this interface.  This only applies
	 * to *GMII and XAUI ports.
	 */
	switch (cvmx_helper_interface_get_mode(xiface)) {
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		if (num_ports > 4) {
			debug("%s: Illegal num_ports\n", __func__);
			return -1;
		}

		gmx_rx_prts.u64 = csr_rd(CVMX_GMXX_RX_PRTS(xi.interface));
		gmx_rx_prts.s.prts = num_ports;
		csr_wr(CVMX_GMXX_RX_PRTS(xi.interface), gmx_rx_prts.u64);
		break;

	default:
		break;
	}

	/*
	 * Skip setting CVMX_PKO_REG_GMX_PORT_MODE on 30XX, 31XX, 50XX,
	 * and 68XX.
	 */
	if (!OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/* Tell PKO the number of ports on this interface */
		pko_mode.u64 = csr_rd(CVMX_PKO_REG_GMX_PORT_MODE);
		if (xi.interface == 0) {
			if (num_ports == 1)
				pko_mode.s.mode0 = 4;
			else if (num_ports == 2)
				pko_mode.s.mode0 = 3;
			else if (num_ports <= 4)
				pko_mode.s.mode0 = 2;
			else if (num_ports <= 8)
				pko_mode.s.mode0 = 1;
			else
				pko_mode.s.mode0 = 0;
		} else {
			if (num_ports == 1)
				pko_mode.s.mode1 = 4;
			else if (num_ports == 2)
				pko_mode.s.mode1 = 3;
			else if (num_ports <= 4)
				pko_mode.s.mode1 = 2;
			else if (num_ports <= 8)
				pko_mode.s.mode1 = 1;
			else
				pko_mode.s.mode1 = 0;
		}
		csr_wr(CVMX_PKO_REG_GMX_PORT_MODE, pko_mode.u64);
	}

	/*
	 * Set GMX to buffer as much data as possible before starting
	 * transmit. This reduces the chances that we have a TX under run
	 * due to memory contention. Any packet that fits entirely in the
	 * GMX FIFO can never have an under run regardless of memory load.
	 */
	gmx_tx_thresh.u64 = csr_rd(CVMX_GMXX_TXX_THRESH(0, xi.interface));
	/* ccn - common cnt numberator */
	int ccn = 0x100;

	/* Choose the max value for the number of ports */
	if (num_ports <= 1)
		gmx_tx_thresh.s.cnt = ccn / 1;
	else if (num_ports == 2)
		gmx_tx_thresh.s.cnt = ccn / 2;
	else
		gmx_tx_thresh.s.cnt = ccn / 4;

	/*
	 * SPI and XAUI can have lots of ports but the GMX hardware
	 * only ever has a max of 4
	 */
	if (num_ports > 4)
		num_ports = 4;
	for (index = 0; index < num_ports; index++)
		csr_wr(CVMX_GMXX_TXX_THRESH(index, xi.interface), gmx_tx_thresh.u64);

	/*
	 * For o68, we need to setup the pipes
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX) && xi.interface < CVMX_HELPER_MAX_GMX) {
		union cvmx_gmxx_txx_pipe config;

		for (index = 0; index < num_ports; index++) {
			config.u64 = 0;

			if (__cvmx_helper_cfg_pko_port_base(xiface, index) >= 0) {
				config.u64 = csr_rd(CVMX_GMXX_TXX_PIPE(index,
								       xi.interface));
				config.s.nump = __cvmx_helper_cfg_pko_port_num(xiface,
									       index);
				config.s.base = __cvmx_helper_cfg_pko_port_base(xiface,
										index);
				csr_wr(CVMX_GMXX_TXX_PIPE(index, xi.interface),
				       config.u64);
			}
		}
	}

	return 0;
}

int cvmx_helper_get_pko_port(int interface, int port)
{
	return cvmx_pko_get_base_pko_port(interface, port);
}

int cvmx_helper_get_ipd_port(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		const struct ipd_port_map *port_map;
		int ipd_port;

		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			port_map = ipd_port_map_68xx;
			ipd_port = 0;
		} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			port_map = ipd_port_map_78xx;
			ipd_port = cvmx_helper_node_to_ipd_port(xi.node, 0);
		} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
			port_map = ipd_port_map_73xx;
			ipd_port = 0;
		} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
			port_map = ipd_port_map_75xx;
			ipd_port = 0;
		} else {
			return -1;
		}

		ipd_port += port_map[xi.interface].first_ipd_port;
		if (port_map[xi.interface].type == GMII) {
			cvmx_helper_interface_mode_t mode;

			mode = cvmx_helper_interface_get_mode(xiface);
			if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI ||
			    (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI &&
			     OCTEON_IS_MODEL(OCTEON_CN68XX))) {
				ipd_port += port_map[xi.interface].ipd_port_adj;
				return ipd_port;
			} else {
				return ipd_port + (index * 16);
			}
		} else if (port_map[xi.interface].type == ILK) {
			return ipd_port + index;
		} else if (port_map[xi.interface].type == NPI) {
			return ipd_port + index;
		} else if (port_map[xi.interface].type == SRIO) {
			return ipd_port + index;
		} else if (port_map[xi.interface].type == LB) {
			return ipd_port + index;
		}

		debug("ERROR: %s: interface %u:%u bad mode\n",
		      __func__, xi.node, xi.interface);
		return -1;
	} else if (cvmx_helper_interface_get_mode(xiface) ==
		   CVMX_HELPER_INTERFACE_MODE_AGL) {
		return 24;
	}

	switch (xi.interface) {
	case 0:
		return index;
	case 1:
		return index + 16;
	case 2:
		return index + 32;
	case 3:
		return index + 36;
	case 4:
		return index + 40;
	case 5:
		return index + 42;
	case 6:
		return index + 44;
	case 7:
		return index + 46;
	}
	return -1;
}

int cvmx_helper_get_pknd(int xiface, int index)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_pknd(xiface, index);

	return CVMX_INVALID_PKND;
}

int cvmx_helper_get_bpid(int interface, int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_bpid(interface, port);

	return CVMX_INVALID_BPID;
}

/**
 * Returns the interface number for an IPD/PKO port number.
 *
 * @param ipd_port IPD/PKO port number
 *
 * Return: Interface number
 */
int cvmx_helper_get_interface_num(int ipd_port)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		const struct ipd_port_map *port_map;
		int i;
		struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

		port_map = ipd_port_map_68xx;
		for (i = 0; i < CVMX_HELPER_MAX_IFACE; i++) {
			if (xp.port >= port_map[i].first_ipd_port &&
			    xp.port <= port_map[i].last_ipd_port)
				return i;
		}
		return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		const struct ipd_port_map *port_map;
		int i;
		struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

		port_map = ipd_port_map_78xx;
		for (i = 0; i < CVMX_HELPER_MAX_IFACE; i++) {
			if (xp.port >= port_map[i].first_ipd_port &&
			    xp.port <= port_map[i].last_ipd_port)
				return cvmx_helper_node_interface_to_xiface(xp.node, i);
		}
		return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		const struct ipd_port_map *port_map;
		int i;
		struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

		port_map = ipd_port_map_73xx;
		for (i = 0; i < CVMX_HELPER_MAX_IFACE; i++) {
			if (xp.port >= port_map[i].first_ipd_port &&
			    xp.port <= port_map[i].last_ipd_port)
				return i;
		}
		return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		const struct ipd_port_map *port_map;
		int i;
		struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

		port_map = ipd_port_map_75xx;
		for (i = 0; i < CVMX_HELPER_MAX_IFACE; i++) {
			if (xp.port >= port_map[i].first_ipd_port &&
			    xp.port <= port_map[i].last_ipd_port)
				return i;
		}
		return -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX) && ipd_port == 24) {
		return 4;
	}

	if (ipd_port < 16)
		return 0;
	else if (ipd_port < 32)
		return 1;
	else if (ipd_port < 36)
		return 2;
	else if (ipd_port < 40)
		return 3;
	else if (ipd_port < 42)
		return 4;
	else if (ipd_port < 44)
		return 5;
	else if (ipd_port < 46)
		return 6;
	else if (ipd_port < 48)
		return 7;

	debug("%s: Illegal IPD port number %d\n", __func__, ipd_port);
	return -1;
}

/**
 * Returns the interface index number for an IPD/PKO port
 * number.
 *
 * @param ipd_port IPD/PKO port number
 *
 * Return: Interface index number
 */
int cvmx_helper_get_interface_index_num(int ipd_port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		const struct ipd_port_map *port_map;
		int port;
		enum port_map_if_type type = INVALID_IF_TYPE;
		int i;
		int num_interfaces;

		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			port_map = ipd_port_map_68xx;
		} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

			port_map = ipd_port_map_78xx;
			ipd_port = xp.port;
		} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
			struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

			port_map = ipd_port_map_73xx;
			ipd_port = xp.port;
		} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
			struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

			port_map = ipd_port_map_75xx;
			ipd_port = xp.port;
		} else {
			return -1;
		}

		num_interfaces = cvmx_helper_get_number_of_interfaces();

		/* Get the interface type of the ipd port */
		for (i = 0; i < num_interfaces; i++) {
			if (ipd_port >= port_map[i].first_ipd_port &&
			    ipd_port <= port_map[i].last_ipd_port) {
				type = port_map[i].type;
				break;
			}
		}

		/* Convert the ipd port to the interface port */
		switch (type) {
		/* Ethernet interfaces have a channel in lower 4 bits
		 * that is does not discriminate traffic, and is ignored.
		 */
		case GMII:
			port = ipd_port - port_map[i].first_ipd_port;

			/* CN68XX adds 0x40 to IPD_PORT when in XAUI/RXAUI
			 * mode of operation, adjust for that case
			 */
			if (port >= port_map[i].ipd_port_adj)
				port -= port_map[i].ipd_port_adj;

			port >>= 4;
			return port;

		/*
		 * These interfaces do not have physical ports,
		 * but have logical channels instead that separate
		 * traffic into logical streams
		 */
		case ILK:
		case SRIO:
		case NPI:
		case LB:
			port = ipd_port - port_map[i].first_ipd_port;
			return port;

		default:
			printf("ERROR: %s: Illegal IPD port number %#x\n",
			       __func__, ipd_port);
			return -1;
		}
	}
	if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return ipd_port & 3;
	if (ipd_port < 32)
		return ipd_port & 15;
	else if (ipd_port < 40)
		return ipd_port & 3;
	else if (ipd_port < 48)
		return ipd_port & 1;

	debug("%s: Illegal IPD port number\n", __func__);

	return -1;
}
