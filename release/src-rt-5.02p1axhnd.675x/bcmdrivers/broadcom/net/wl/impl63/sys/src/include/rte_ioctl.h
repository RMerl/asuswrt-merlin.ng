/*
 * HND Run Time Environment ioctl.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: rte_ioctl.h 783707 2020-02-06 13:20:07Z $
 */

#ifndef _rte_ioctl_h_
#define _rte_ioctl_h_

/* RTE IOCTL definitions for generic ether devices */
#define RTEGHWADDR		0x8901
#define RTESHWADDR		0x8902
#define RTEGMTU			0x8903
#define RTEGSTATS		0x8904
#define RTEGALLMULTI		0x8905
#define RTESALLMULTI		0x8906
#define RTEGPROMISC		0x8907
#define RTESPROMISC		0x8908
#define RTESMULTILIST	0x8909
#define RTEGUP			0x890A
#define RTEGPERMADDR		0x890B
#define RTEDEVPWRSTCHG		0x890C	/* Device pwr state change for PCIedev */
#define RTEDEVPMETOGGLE		0x890D	/* Toggle PME# to wake up the host */
#define RTEDEVTIMESYNC		0x890E	/* Device TimeSync */
#define RTEDEVDSNOTIFY		0x890F	/* Bus DS state notification */
#define RTED11DMALPBK_INIT	0x8910	/* D11 DMA loopback init */
#define RTED11DMALPBK_UNINIT	0x8911	/* D11 DMA loopback uninit */
#define RTED11DMALPBK_RUN	0x8912	/* D11 DMA loopback run */

#define RTE_IOCTL_QUERY			0x00
#define RTE_IOCTL_SET			0x01
#define RTE_IOCTL_OVL_IDX_MASK	0x1e
#define RTE_IOCTL_OVL_RSV		0x20
#define RTE_IOCTL_OVL			0x40
#define RTE_IOCTL_OVL_IDX_SHIFT	1

enum hnd_ioctl_cmd {
	HND_RTE_DNGL_IS_SS = 1, /* true if device connected at super speed */

	/* PCIEDEV specific wl <--> bus ioctls */
	BUS_GET_VAR = 2,
	BUS_SET_VAR = 3,
	BUS_FLUSH_RXREORDER_Q = 4,
	BUS_SET_LTR_STATE = 5,
	BUS_FLUSH_CHAINED_PKTS = 6,
	BUS_SET_COPY_COUNT = 7,
	BUS_UPDATE_FLOW_PKTS_MAX = 8,
	BUS_UPDATE_EXTRA_TXLFRAGS = 9,
	BUS_UPDATE_FRWD_RESRV_BUFCNT = 10,
	BUS_CFP_FLOW_DELINK = 11,
	BUS_SET_MONITOR_MODE = 12,
	BUS_FLOW_FLUSH_PEND = 13,
	BUS_SBTOPCIE_ACCESS_START = 14,
	BUS_SBTOPCIE_ACCESS_STOP = 15,
	BUS_TAF_SCHEDULER_CONFIG = 16
};

#define SDPCMDEV_SET_MAXTXPKTGLOM	1
#define RTE_MEMUSEINFO_VER 0x00

typedef struct memuse_info {
	uint16 ver;			/* version of this struct */
	uint16 len;			/* length in bytes of this structure */
	uint32 tot;			/* Total memory */
	uint32 text_len;	/* Size of Text segment memory */
	uint32 data_len;	/* Size of Data segment memory */
	uint32 bss_len;		/* Size of BSS segment memory */

	uint32 arena_size;	/* Total Heap size */
	uint32 arena_free;	/* Heap memory available or free */
	uint32 inuse_size;	/* Heap memory currently in use */
	uint32 inuse_hwm;	/* High watermark of memory - reclaimed memory */
	uint32 inuse_overhead;	/* tally of allocated mem_t blocks */
	uint32 inuse_total;	/* Heap in-use + Heap overhead memory  */
	uint32 free_lwm;        /* Least free size since reclaim */
	uint32 mf_count;        /* Malloc failure count */

	uint32 max_flowring_alloc; /* max mem allocated for any flowring */
	uint32 max_bsscfg_alloc; /* max mem allocated for any bsscfg */
	uint32 max_scb_alloc; /* max mem allocated for any scb */
	uint32 total_flowring_alloc; /* total memory allocated for flowrings */
	uint32 total_bsscfg_alloc; /* total memory allocated for bsscfgs */
	uint32 total_scb_alloc; /* total memory allocated for scbs */
} memuse_info_t;

/* For D11 DMA loopback test */
typedef struct d11_dmalpbk_args {
	uint8 *buf;
	int32 len;
} d11_dmalpbk_args_t;

/* ioctl buffer passed b/w WL & PCIEDEV driver to setup SB2PCIE access to host */
typedef struct sbtopcie_info {
	uint32	len;		/* Max length bytes to be accessed */
	uint64	haddr64;	/* 64 bit host physical address stored in dongle */
	uint32*	remap_addr;	/* Ptr to remapped address */
	uint32*	base_lo;	/* ptr to base register backup */
	uint32*	base_hi;	/* ptr to base register backup */
} sbtopcie_info_t;

#endif /* _rte_ioctl_h_ */
