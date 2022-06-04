/*
 * Broadcom Event  protocol definitions
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Dependencies: bcmeth.h
 *
 * $Id: dnglevent.h $
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * -----------------------------------------------------------------------------
 *
 */

/*
 * Broadcom dngl Ethernet Events protocol defines
 *
 */

#ifndef _DNGLEVENT_H_
#define _DNGLEVENT_H_

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif // endif
#include <bcmeth.h>
#include <ethernet.h>
#ifdef HEALTH_CHECK
#include <dngl_defs.h>
#endif /* HEALTH_CHECK */

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>
#define BCM_DNGL_EVENT_MSG_VERSION		1
#define DNGL_E_RSRVD_1				0x0
#define DNGL_E_RSRVD_2				0x1
#define DNGL_E_SOCRAM_IND			0x2
typedef BWL_PRE_PACKED_STRUCT struct
{
	uint16  version; /* Current version is 1 */
	uint16  reserved; /* reserved for any future extension */
	uint16  event_type; /* DNGL_E_SOCRAM_IND */
	uint16  datalen; /* Length of the event payload */
} BWL_POST_PACKED_STRUCT bcm_dngl_event_msg_t;

typedef BWL_PRE_PACKED_STRUCT struct bcm_dngl_event {
	struct ether_header eth;
	bcmeth_hdr_t        bcm_hdr;
	bcm_dngl_event_msg_t      dngl_event;
	/* data portion follows */
} BWL_POST_PACKED_STRUCT bcm_dngl_event_t;

typedef BWL_PRE_PACKED_STRUCT struct bcm_dngl_socramind {
	uint16			tag;	/* data tag */
	uint16			length; /* data length */
	uint8			value[1]; /* data value with variable length specified by length */
} BWL_POST_PACKED_STRUCT bcm_dngl_socramind_t;

/* SOCRAM_IND type tags */
#define SOCRAM_IND_ASSERT_TAG		0x1
#define SOCRAM_IND_TAG_HEALTH_CHECK	0x2
/* Health check top level module tags */
typedef BWL_PRE_PACKED_STRUCT struct bcm_dngl_healthcheck {
	uint16			top_module_tag;	/* top level module tag */
	uint16			top_module_len; /* Type of PCIE issue indication */
	uint8			value[1]; /* data value with variable length specified by length */
} BWL_POST_PACKED_STRUCT bcm_dngl_healthcheck_t;

/* Health check top level module tags */
#define HEALTH_CHECK_TOP_LEVEL_MODULE_PCIEDEV_RTE 1
#define HEALTH_CHECK_PCIEDEV_VERSION_1	1
#define HEALTH_CHECK_PCIEDEV_FLAG_IN_D3_SHIFT	0
#define HEALTH_CHECK_PCIEDEV_FLAG_AER_SHIFT		1
#define HEALTH_CHECK_PCIEDEV_FLAG_LINKDOWN_SHIFT		2
#define HEALTH_CHECK_PCIEDEV_FLAG_MSI_INT_SHIFT			3
#define HEALTH_CHECK_PCIEDEV_FLAG_NODS_SHIFT			4
#define HEALTH_CHECK_PCIEDEV_FLAG_IN_D3	1 << HEALTH_CHECK_PCIEDEV_FLAG_IN_D3_SHIFT
#define HEALTH_CHECK_PCIEDEV_FLAG_AER	1 << HEALTH_CHECK_PCIEDEV_FLAG_AER_SHIFT
#define HEALTH_CHECK_PCIEDEV_FLAG_LINKDOWN	1 << HEALTH_CHECK_PCIEDEV_FLAG_LINKDOWN_SHIFT
#define HEALTH_CHECK_PCIEDEV_FLAG_MSI_INT	1 << HEALTH_CHECK_PCIEDEV_FLAG_MSI_INT_SHIFT
#define HEALTH_CHECK_PCIEDEV_FLAG_NODS	1 << HEALTH_CHECK_PCIEDEV_FLAG_NODS_SHIFT
/* PCIE Module TAGs */
#define HEALTH_CHECK_PCIEDEV_INDUCED_IND	0x1
#define HEALTH_CHECK_PCIEDEV_H2D_DMA_IND	0x2
#define HEALTH_CHECK_PCIEDEV_D2H_DMA_IND	0x3
#define HEALTH_CHECK_PCIEDEV_IOCTL_STALL_IND	0x4
#define HEALTH_CHECK_PCIEDEV_D3ACK_STALL_IND	0x5
#define HEALTH_CHECK_PCIEDEV_NODS_IND	0x6
#define HEALTH_CHECK_PCIEDEV_LINKSPEED_FALLBACK_IND	0x7

#define HC_PCIEDEV_CONFIG_REGLIST_MAX	20
typedef BWL_PRE_PACKED_STRUCT struct bcm_dngl_pcie_hc {
	uint16			version; /* HEALTH_CHECK_PCIEDEV_VERSION_1 */
	uint16			reserved;
	uint16			pcie_err_ind_type; /* PCIE Module TAGs */
	uint16			pcie_flag;
	uint32			pcie_control_reg;
	uint32			pcie_config_regs[HC_PCIEDEV_CONFIG_REGLIST_MAX];
} BWL_POST_PACKED_STRUCT bcm_dngl_pcie_hc_t;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif /* _DNGLEVENT_H_ */
