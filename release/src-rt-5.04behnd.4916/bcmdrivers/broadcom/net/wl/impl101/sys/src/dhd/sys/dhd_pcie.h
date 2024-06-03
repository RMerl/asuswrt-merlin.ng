/*
 * Linux DHD Bus Module for PCIE
 *
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_pcie.h 827313 2023-07-10 16:04:24Z $
 */

#ifndef dhd_pcie_h
#define dhd_pcie_h

#include <bcmpcie.h>
#include <hnd_cons.h>
#include <bcm_btrace_shared.h>
#ifdef SUPPORT_LINKDOWN_RECOVERY
#ifdef CONFIG_ARCH_MSM
#ifdef CONFIG_ARCH_MSM8994
#include <linux/msm_pcie.h>
#else
#include <mach/msm_pcie.h>
#endif /* CONFIG_ARCH_MSM8994 */
#endif /* CONFIG_ARCH_MSM */
#endif /* SUPPORT_LINKDOWN_RECOVERY */

/* defines */
#define DHD_PCIE_SHARED_VERSION 0x05

#define PCMSGBUF_HDRLEN 0
#define DONGLE_REG_MAP_SIZE (32 * 1024)
#define DONGLE_TCM_MAP_SIZE (4096 * 1024)
#define DONGLE_MIN_MEMSIZE (128 *1024)
#ifdef DHD_DEBUG
#define DHD_PCIE_SUCCESS 0
#define DHD_PCIE_FAILURE 1
#endif /* DHD_DEBUG */
#define	REMAP_ENAB(bus)			((bus)->remap)
#define	REMAP_ISADDR(bus, a)		(((a) >= ((bus)->orig_ramsize)) && ((a) < ((bus)->ramsize)))

/*
 * Router with 4366 can have 128 stations and 16 BSS,
 * hence (128 stations x 4 access categories for ucast) + 16 bc/mc flowrings
 */

#if defined(BCM_ROUTER)
#define MAX_DHD_TX_FLOWS	1020
#else
#define MAX_DHD_TX_FLOWS	192
#endif /* BCM_ROUTER */

#if defined BCM_PCAP
#define PCAP_DYNAMIC_RINGS	2
#else
#define PCAP_DYNAMIC_RINGS	0
#endif

/* Budget for dynamic rings */
#define MAX_DHD_DYNAMIC_RINGS	(PCAP_DYNAMIC_RINGS)

#define MAX_DHD_RINGS	(MAX_DHD_TX_FLOWS + BCMPCIE_COMMON_MSGRINGS + MAX_DHD_DYNAMIC_RINGS)

/* user defined data structures */
/* Device console log buffer state */
#if defined(HNDUCLS)
#define CONSOLE_LINE_MAX	512
#else
#define CONSOLE_LINE_MAX	192
#endif /* HNDUCLS */
#define CONSOLE_BUFFER_MAX	2024

#ifndef MAX_CNTL_D3ACK_TIMEOUT
#define MAX_CNTL_D3ACK_TIMEOUT 2
#endif /* MAX_CNTL_D3ACK_TIMEOUT */

/* Host backup queue depth for PCIe DMA index */
#define PCIE_HBQD(dhd)               ((dhd)->pcie_hbqd)
#define PCIE_HBQD_VAL(qd, rw)        (((qd) << 16) | (rw))
#define PCIE_HBQD_RW_INDEX(val)      ((val) & 0xFFFF)
#define PCIE_HBQD_QD(val)            (((val) >> 16) & 0xFFFF)

/* implicit DMA for h2d wr and d2h rd indice from Host memory to TCM */
#define IDMA_ENAB(dhd)		((dhd)->idma_enable)
#define IDMA_ACTIVE(dhd)	(((dhd)->idma_enable) && ((dhd)->idma_inited))

#define IDMA_CAPABLE(bus)	(((bus)->sih->buscorerev == 19) || ((bus)->sih->buscorerev >= 23))
#define IDMA64_CAPABLE(bus)	(((bus)->sih->buscorerev >= 132))
#define IDMA_SETS(dhd)      ((dhd)->idma_set)
#define IDMA_FLOWS_PER_SET(dhd)   \
	(BCMPCIE_IDMA_BYTES_PER_SET/(dhd)->bus->rw_index_sz)
/*
 * Ch2_HostToDevXDoorbell0 Register (Offset #160)
 * Name		Bits	Description
 * FRG_ID	3:0	Flow Ring Group for dma transfer [0-15].
 * 			Valid only when DMA TYPE = 0001
 * DMA_TYPE	7:4	Specify type of DMA transfer is requested.
 *			DMA_TYPE	MODE	Description
 *			0000	NO_IDMA	Do not initiate an IDMA transfer.
 *				Interrupt ARM.
 *			0001	IDMA	Transfer complete frame for Specified
 *				FRG_ID.
 *			0010	UNDEFINED	NO OP
 *			0011	UNDEFINED	NO OP
 *			0100	UNDEFINED	NO OP (reserved for  TXPOST)
 *			0101	HWA_RXPOST	Directly update RXPOST write index
 *						with "INDEX_VAL".
 *			0110	HWA_TXCPL	Directly update TXCPL read index
 *						with "INDEX_VAL".
 *			0111	HWA_RXCPL	Directly update RXCPL read index
 *						with "INDEX_VAL".
 *			1xxx	Reserved	NO OP
 * RINGINDEX_NUM
 *		15:8	Specifies the Index number to Update (used only  if there
 * 			are more than one Index in a ring)
 * INDEX_VAL	31:16	Index value to update in the HWA block.
 *
 * PCIe corerev >= 132
 * FRG_ID    9:8,3:0    Flow Ring group for DMA transfer [0-63]. Valid only when DMA_TYPE = 0001
 *
 */
/* Bits in Ch2_HostToDevXDoorbell */
#define CH2_H2DEV_DB_FRG_ID_SHIFT      0
#define CH2_H2DEV_DB_FRG_ID_MASK \
	(0xf << CH2_H2DEV_DB_FRG_ID_SHIFT)
#define CH2_H2DEV_DB_DMA_TYPE_SHIFT      4
#define CH2_H2DEV_DB_DMA_TYPE_MASK \
	(0xf << CH2_H2DEV_DB_DMA_TYPE_SHIFT)
#define CH2_H2DEV_DB_RINGINDEX_NUM_SHIFT      8
#define CH2_H2DEV_DB_RINGINDEX_NUM_MASK \
	(0xff << CH2_H2DEV_DB_RINGINDEX_NUM_SHIFT)
#define CH2_H2DEV_DB_INDEX_VAL_SHIFT      16
#define CH2_H2DEV_DB_INDEX_VAL_MASK \
	(0xffff << CH2_H2DEV_DB_INDEX_VAL_SHIFT)
/* PCIe corerev >= 132
 * FRG_ID    9:8,3:0    Flow Ring group for DMA transfer [0-63].
 * Valid only when DMA_TYPE = 0001
 */
#define CH2_H2DEV_DB_FRG_ID_EXT_SHIFT      8
#define CH2_H2DEV_DB_FRG_ID_EXT_MASK \
	(0x3 << CH2_H2DEV_DB_FRG_ID_EXT_SHIFT)

#define CH2_H2DEV_DB_FRG_ID_BITS      4

#define DMA_TYPE_IDMA		1
#define DMA_TYPE_HWA_RXPOST	5
#define DMA_TYPE_HWA_TXCPL	6
#define DMA_TYPE_HWA_RXCPL	7

#ifdef DHD_DEBUG

typedef struct dhd_console {
	 uint		count;	/* Poll interval msec counter */
	 uint		log_addr;		 /* Log struct address (fixed) */
	 hnd_log_t	 log;			 /* Log struct (host copy) */
	 uint		 bufsize;		 /* Size of log buffer */
	 uint8		 *buf;			 /* Log buffer (host copy) */
	 uint		 last;			 /* Last buffer read index */
} dhd_console_t;
#endif /* DHD_DEBUG */

typedef struct dhd_ring_mem {
	uint32 daddr32;		/* pcie_ipc_ring_mem location in dongle */
	uint32 wr_daddr32;  /* location of WR index in dongle array of wr indices */
	uint32 rd_daddr32;  /* location of RD index in dongle array of wr indices */
} dhd_ring_mem_t;

typedef struct dhd_btrace {
	uint32		process_last_ts;	/* Last time we polled for trace data */
	uint32		process_interval;	/* Trace data processing interval in ms */
	void		*local_buf;		/* Local buffer pointer */
	uint32		remote_buf;		/* Remote buffer address in dongle */
	uint		buf_size;		/* Local and remote buffer sizes */
	uint8		flags_h2d;		/* Local copy of H2D flags */
	uint8		trace_seq;		/* Trace file sequence, for truncating */
	char		filename[64];		/* Current output filename */
} dhd_btrace_t;

/** Instantiated once for each hardware (dongle) instance that this DHD manages */
typedef struct dhd_bus {
	dhd_pub_t	*dhd;	/**< pointer to per hardware (dongle) unique instance */
#ifndef NDIS
	struct pci_dev  *dev;		/* pci device handle */
#endif
#if defined(BCM_ROUTER) && defined(DHD_USE_PERIM)
	dll_t       txqueue_pend;   /* pending list of tx flowring queues */
	dll_t       txqueue_done;   /* done list of tx flowring queues */
#endif /* BCM_ROUTER && DHD_USE_PERIM */
	dll_t       const_flowring; /* constructed list of tx flowring queues */

	si_t		*sih;			/* Handle for SI calls */
	char		*vars;			/* Variables (from CIS and/or other) */
	uint		varsz;			/* Size of variables buffer */
	uint32		sbaddr;			/* Current SB window pointer (-1, invalid) */
	sbpcieregs_t	*reg;			/* Registers for PCIE core */

	uint		armrev;			/* CPU core revision */
	uint		ramrev;			/* SOCRAM core revision */
	uint32		ramsize;		/* Size of RAM in SOCRAM (bytes) */
	uint32		orig_ramsize;		/* Size of RAM in SOCRAM (bytes) */
	uint32		srmemsize;		/* Size of SRMEM */

	uint32		bus;			/* gSPI or SDIO bus */
	uint32		intstatus;		/* Intstatus bits (events) pending */
	bool		dpc_sched;		/* Indicates DPC schedule (intrpt rcvd) */
	bool		fcstate;		/* State of dongle flow-control */

	uint16		cl_devid;		/* cached devid for dhdsdio_probe_attach() */
	char		*fw_path;		/* module_param: path to firmware image */
	char		*nv_path;		/* module_param: path to nvram vars file */
#ifdef CACHE_FW_IMAGES
	int			processed_nvram_params_len;	/* Modified len of NVRAM info */
#endif

#ifdef BCM_ROUTER
	char		*nvram_params;		/* user specified nvram params. */
	int			nvram_params_len;
#endif /* BCM_ROUTER */

	struct pktq	txq;			/* Queue length used for flow-control */

	uint		intrcount;		/* Count of device interrupt callbacks */
	uint		lastintrs;		/* Count as of last watchdog timer */
	bool		intr;			/* Use interrupts */
	bool		ipend;			/* Device interrupt is pending */
	bool		intdis;			/* Interrupts disabled by isr */

	bool		remap;		/* Contiguous 1MB RAM: 512K socram + 512K devram
					 * Available with socram rev 16
					 * Remap region not DMA-able
					 */
	uint32		resetinstr;
	uint32		dongle_ram_base;

	/* PCIe IPC : Local copy of IPC, Rings and HME structures fetched from dongle */
	uint32              pcie_ipc_revision; /* dongle advertized PCIe IPC rev */
	uint32              pcie_ipc_hme_user_size; /* HME user size based on dongle IPC rev */
	pcie_ipc_t          pcie_ipc;
	pcie_ipc_rings_t    pcie_ipc_rings;
	pcie_ipc_hme_t    * pcie_ipc_hme;
	/* End of PCIe IPC */

	/* Well known dongle addresses specifying locations of: */
	daddr32_t   pcie_ipc_daddr32; /* pcie_ipc_t */
	daddr32_t   pcie_ipc_rings_daddr32; /* pcie_ipc_rings_t */
	daddr32_t   pcie_ipc_ring_mem_daddr32; /* pcie_ipc-ring_mem */
	daddr32_t   pcie_ipc_h2d_mb_daddr32; /* 4B h2d mailbox */
	daddr32_t   pcie_ipc_d2h_mb_daddr32; /* 4B d2h mailbox */
	daddr32_t   console_daddr32; /* hnd_cons_t */
	daddr32_t   btrace_daddr32;	/* btrace_ipc_t */

	uint32      max_h2d_rings;
	uint32      max_d2h_rings;
	uint32      max_flowrings;
	uint32      max_h2d_dynamic_rings;
	uint32      max_d2h_dynamic_rings;
	uint32      rw_index_sz;
	dhd_ring_mem_t ring_mem[MAX_DHD_RINGS];

	uint32      max_interfaces;

#ifdef DHD_DEBUG
	dhd_console_t	console;		/* Console output polling support */
	dhd_btrace_t	btrace;			/* Binary tracing support */
#endif /* DHD_DEBUG */

	volatile char	*regs;		/* pci device memory va */
	volatile char	*tcm;		/* pci device memory va */
	osl_t		*osh;
	uint32		nvram_csm;	/* Nvram checksum */
	bool		bus_flowctrl;
	/* Host Memory Extension buffer for DHD hybridfw feature. Dongle SWPAGING */
	dhd_dma_buf_t   hybridfw_dma_buf;
#if defined(DHD_BHM)
	dhd_dma_buf_t	bhm_mem_dma_buf;	/* Dongle BHM reserved memory */
	bool		hme_from_bhm;		/* Atleast one HME user mem is carved from BHM */
	bool		dngl_bhm_offset;	/* Dongle sends memory offset from BHM base address
						 * for the HME user allocated from BHM
						 */
#endif /* DHD_BHM */
	bool		db1_for_mb;
	uint16		pollrate;
	uint16		polltick;

	volatile uint32  *pcie_mb_intr_addr;
	volatile uint32  *pcie_mb_intr_2_addr;
	volatile uint32  *pcie_db1_intr_2_addr;
	void    *pcie_mb_intr_osh;

	uint32 def_intmask;
	uint	wait_for_d3_ack;

	bool	sleep_allowed;
	bool	ltrsleep_on_unload;
	bool	suspended;
	bool	device_wake_state;
	dhd_timeout_t doorbell_timer;

#ifdef SUPPORT_LINKDOWN_RECOVERY
#ifdef CONFIG_ARCH_MSM
	struct msm_pcie_register_event pcie_event;
	uint8 islinkdown;
#endif /* CONFIG_ARCH_MSM */
#endif /* SUPPORT_LINKDOWN_RECOVERY */
	uint32 d3_inform_cnt;
	uint32 d0_inform_cnt;
	uint32 d0_inform_in_use_cnt;
	uint32 ramtop_addr;	/* Dongle address of unused space at top of RAM */

	uint8	force_suspend;
	bool	idma_enabled;
	bool	intr_enabled;
#ifdef PCIE_OOB
	bool	oob_enabled;
#endif /* PCIE_OOB */
	void *gen3ep;
	haddr64_t bar2_haddr64;
	ulong	bar2_size;
} dhd_bus_t;

/* Given a HME user id and size, return PCIE HME USER attributes (pcie_ipc_hme_user_t).
 * PCIe HEM user size is dependent on dongle IPC revision.
 */
#define DHD_PCIE_IPC_HME_USER(_ipc_hme, _user_id, _user_size)		\
	((pcie_ipc_hme_user_t *)((uintptr)(_ipc_hme) +			\
		sizeof(pcie_ipc_hme_t) + ((_user_id) * (_user_size))))

/* function declarations */

extern uint32* dhdpcie_bus_reg_map(osl_t *osh, ulong addr, int size);
extern int dhdpcie_bus_register(void);
extern void dhdpcie_bus_unregister(void);
extern bool dhdpcie_chipmatch(uint16 vendor, uint16 device);

extern dhd_bus_t * dhdpcie_bus_attach(osl_t *osh,
	volatile char *regs, volatile char *tcm, haddr64_t bar2_haddr64, void *pci_dev);
extern uint32 dhdpcie_bus_cfg_read_dword(dhd_bus_t *bus, uint32 addr, uint32 size);
extern void dhdpcie_bus_cfg_write_dword(dhd_bus_t *bus, uint32 addr, uint32 size, uint32 data);
extern void dhdpcie_bus_intr_enable(dhd_bus_t *bus);
extern void dhdpcie_bus_intr_disable(dhd_bus_t *bus);
extern void dhdpcie_bus_release(dhd_bus_t *bus);
extern int32 dhdpcie_bus_isr(dhd_bus_t *bus);
extern void dhdpcie_free_irq(dhd_bus_t *bus);
extern void dhdpcie_bus_ringbell_fast(dhd_bus_t *bus, uint32 value);
extern int dhdpcie_bus_suspend(dhd_bus_t *bus, bool state);
extern int dhdpcie_pci_suspend_resume(dhd_bus_t *bus, bool state);
extern bool dhdpcie_tcm_valid(dhd_bus_t *bus); /* pcie ipc is valid */
#ifndef BCMPCIE_OOB_HOST_WAKE
extern void dhdpcie_pme_active(osl_t *osh, bool enable);
#endif /* !BCMPCIE_OOB_HOST_WAKE */
extern bool dhdpcie_pme_cap(osl_t *osh);
extern int dhdpcie_prepare_pcie_ep(osl_t *osh, struct pci_dev *pdev, bool force_bp_reset);
extern uint32 dhdpcie_lcreg(osl_t *osh, uint32 mask, uint32 val);
extern void dhdpcie_set_pmu_min_res_mask(dhd_bus_t *bus, uint min_res_mask);
extern uint8 dhdpcie_clkreq(osl_t *osh, uint32 mask, uint32 val);
#ifdef OEM_ANDROID
extern int dhdpcie_start_host_pcieclock(dhd_bus_t *bus);
extern int dhdpcie_stop_host_pcieclock(dhd_bus_t *bus);
extern int dhdpcie_disable_device(dhd_bus_t *bus);
extern int dhdpcie_enable_device(dhd_bus_t *bus);
extern int dhdpcie_alloc_resource(dhd_bus_t *bus);
extern void dhdpcie_free_resource(dhd_bus_t *bus);
extern int dhdpcie_bus_request_irq(dhd_bus_t *bus);
#endif /* OEM_ANDROID */
#ifdef BCMPCIE_OOB_HOST_WAKE
extern int dhdpcie_oob_intr_register(dhd_bus_t *bus);
extern void dhdpcie_oob_intr_unregister(dhd_bus_t *bus);
extern void dhdpcie_oob_intr_set(dhd_bus_t *bus, bool enable);
#endif /* BCMPCIE_OOB_HOST_WAKE */
#ifdef PCIE_OOB
extern void dhd_oob_set_bt_reg_on(dhd_bus_t *bus, bool val);
extern int dhd_oob_get_bt_reg_on(dhd_bus_t *bus);
#endif /* PCIE_OOB */

/* PCIe IPC : Read the structures from dongle's memory into local memory */
extern pcie_ipc_t * dhdpcie_get_ipc(dhd_bus_t *bus);
extern pcie_ipc_rings_t * dhdpcie_get_ipc_rings(dhd_bus_t *bus);
/* Use PCIe IPC HME User configuration to allocate a HME compatible DMA buf */
extern int dhd_hme_buf_alloc(dhd_bus_t *bus, dhd_dma_buf_t *user_dma_buf, uint32 hme_user_id);
extern int dhd_hme_buf_free(dhd_bus_t *bus, dhd_dma_buf_t *user_dma_buf, uint32 hme_user_id);
#if defined(DHD_BHM)
extern int dhdpcie_bhm_mem_alloc(dhd_bus_t *bus);
extern void dhdpcie_bhm_mem_free(dhd_bus_t *bus);
#endif /* DHD_BHM */

extern const char *dhdpcie_get_device_name(dhd_bus_t *bus);

extern int dhd_buzzz_dump_dngl(dhd_bus_t *bus);

extern void dhd_bus_hostready(dhd_bus_t *bus);

extern void dhdpcie_bus_read_sysmem(dhd_bus_t *bus, daddr32_t daddr32, uint8 *data, uint size);

#ifdef BCMDBG_PCIE
extern void dhdpcie_dumpregs(dhd_bus_t *bus, char *dbgstr);
#endif /* BCMDBG_PCIE */
#if defined(MLO_IPC)
extern int dhdpcie_mlo_h2d_notify(dhd_bus_t *bus, uint8 state);
extern void dhdpcie_bus_get_pcie_gen_lane(dhd_pub_t *dhd_pub, int *pcie_gen, int *pcie_lane);
extern haddr64_t dhdpcie_bus_cfg_set_mlo_win(dhd_pub_t *dhd_pub);
#if defined(BCM_ROUTER)
extern int dhdpcie_bus_bridge_init(dhd_pub_t *dhd_pub);
extern void dhdpcie_bus_bridge_deinit(dhd_pub_t *dhd_pub);
extern haddr64_t dhdpcie_get_bar2_win(dhd_bus_t *bus);
#endif /* BCM_ROUTER */
#endif /* MLO_IPC */
#endif /* dhd_pcie_h */
