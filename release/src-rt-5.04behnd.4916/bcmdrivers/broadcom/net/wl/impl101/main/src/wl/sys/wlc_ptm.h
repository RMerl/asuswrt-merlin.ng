/*
 * +--------------------------------------------------------------------------+
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>>
 * PTM (Precision Time Management) Protocol Support
 * $Id:$
 * +--------------------------------------------------------------------------+
 */

#ifndef _WLC_PTM_H_
#define _WLC_PTM_H_

#if !defined(WL_MLO)
#error "PTM protocol is used only when MLO is defined."
#endif
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_hw.h>
#include <pcie_core.h>
#include <wlc_hw_priv.h>
#if (defined BCMPCIEDEV && !defined(BCMPCIEDEV_DISABLED))
#include <pcieregsoffs.h>
#endif

#define WL_PTM_FLAG_PTM_SUPPORTED	(0x1 << 0)
#define WL_PTM_FLAG_PTM_ENABLED		(0x1 << 1)
#define WL_PTM_FLAG_PTM_EANBLE_FAILED	(0x1 << 2)

/* config space tm capability/control register */
#define	PCIE_CFG_PTM_PTM_CAP_REG		0x204
#define PCIE_CFG_PTM_PTM_CTRL_REG		0x208

#define	PCIE_CFG_PTM_REQUESTER_CAPABLE			0x1
#define PCIE_CFG_PTM_ENABLE				0x1

/* PCIe space PTM registers: addresses are offset from PCIE_ENUM_BASE */
typedef volatile struct pcie_ptm_regs {
	uint32	PAD1[0x1E0];
	uint32	ptm_control;
	uint32	ptm_status;
	uint32	PAD2[0x6];
	uint32	ptm_master_time_hi;
	uint32	ptm_master_time_lo;
	uint32	PAD3[0x2];
	uint32	ptm_master_time_cmd;
	uint32	PAD4[0x2];
	uint32	ptm_invalidate_thresh;
} pcie_ptm_regs_t;

/* PCIE_ENUM :: PTM_CONTROL :: PTM_INTERVAL [15:12] */
#define PTM_INTERVAL_MASK                    0x0000f000
#define PTM_INTERVAL_BITS                    4
#define PTM_INTERVAL_SHIFT                   12
#define PTM_INTERVAL_DEFAULT                 0x00000000

/* PCIE_ENUM :: PTM_CONTROL :: PTM_REQ_RETRY_CNT [11:10] */
#define PTM_REQ_RETRY_CNT_MASK               0x00000c00
#define PTM_REQ_RETRY_CNT_BITS               2
#define PTM_REQ_RETRY_CNT_SHIFT              10
#define PTM_REQ_RETRY_CNT_DEFAULT            0x00000000

/* PCIE_ENUM :: PTM_CONTROL :: PTM_MASTER_UPDATE_EN [02:02] */
#define PTM_MASTER_UPDATE_EN_MASK            0x00000004
#define PTM_MASTER_UPDATE_EN_BITS            1
#define PTM_MASTER_UPDATE_EN_SHIFT           2
#define PTM_MASTER_UPDATE_EN_DEFAULT         0x00000000

/* PCIE_ENUM :: PTM_CONTROL :: PTM_ENABLE [01:01] */
#define PTM_ENABLE_MASK                      0x00000002
#define PTM_ENABLE_BITS                      1
#define PTM_ENABLE_SHIFT                     1
#define PTM_ENABLE_DEFAULT                   0x00000000

/* PTM_STATUS - PTM Status Register */
/* PCIE_ENUM :: PTM_STATUS :: PCIE_MASTER_TIME_VALID [08:08] */
#define PCIE_MASTER_TIME_VALID_MASK           0x00000100
#define PCIE_MASTER_TIME_VALID_BITS           1
#define PCIE_MASTER_TIME_VALID_SHIFT          8
#define PCIE_MASTER_TIME_VALID_DEFAULT        0x00000000

/* PCIE_ENUM :: PTM_CONTROL :: PTM_DIALOG_REQ [00:00] */
#define PTM_DIALOG_REQ_MASK                  0x00000001
#define PTM_DIALOG_REQ_BITS                  1
#define PTM_DIALOG_REQ_SHIFT                 0
#define PTM_DIALOG_REQ_DEFAULT               0x00000000

/* D11 register */
#define		D11_TIME1_SNAP_OFFSET		(0x14C0)
#define		D11_TIME1_SNAP(wlc_hw)		((volatile uint8*)wlc_hw->regs \
							+ D11_TIME1_SNAP_OFFSET)
#define		D11_TIME1_SNAP_PTM_VALID	(0x8000)
#define		D11_TIME1_SNAP_SNAPSHOT		(0x0001)

#if defined(CONFIG_BCM96764) || defined(CONFIG_BCM96765) || defined(CONFIG_BCM96766)
/* integrared radio */
#define		CONFIG_WIFI_MLO_CONTROL_ADDR	0x800E0000
#define		CONFIG_WIFI_MLO_BLOCK_SIZE	0x00010000
#define		PTM_VALID			0x1
#endif /* CONFIG_BCM96764 || CONFIG_BCM96765 || CONFIG_BCM96766 */

int wlc_ptm_enable(wlc_hw_info_t *wlc_hw);
void wlc_ptm_init(wlc_hw_info_t *wlc_hw);
#if defined(BCMDBG)
int wlc_ptm_dump(void *ctx, struct bcmstrbuf *b);
#endif /* BCMDBG */
#endif /* _WLC_PTM_H */
