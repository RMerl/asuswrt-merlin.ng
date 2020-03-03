/*
 * Dongle BUS interface Abstraction layer
 *   target serial buses like USB, SDIO, SPI, etc.
 *
 * Copyright (C) 2019, Broadcom. All Rights Reserved.
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
 * $Id: dbus.h 774649 2019-05-01 13:45:54Z $
 */

#ifndef __DBUS_H__
#define __DBUS_H__

#include "typedefs.h"

#ifdef BCMDBG
#define DBUSERR(args)         printf args
#define DBUSINFO(args)
#define DBUSTRACE(args)
#define DBUSDBGLOCK(args)

#else
#define DBUSTRACE(args)
#define DBUSERR(args)
#define DBUSINFO(args)
#define DBUSDBGLOCK(args)
#endif // endif

enum {
	DBUS_OK = 0,
	DBUS_ERR = -200,
	DBUS_ERR_TIMEOUT,
	DBUS_ERR_DISCONNECT,
	DBUS_ERR_NODEVICE,
	DBUS_ERR_UNSUPPORTED,
	DBUS_ERR_PENDING,
	DBUS_ERR_NOMEM,
	DBUS_ERR_TXFAIL,
	DBUS_ERR_TXTIMEOUT,
	DBUS_ERR_TXDROP,
	DBUS_ERR_RXFAIL,
	DBUS_ERR_RXDROP,
	DBUS_ERR_TXCTLFAIL,
	DBUS_ERR_RXCTLFAIL,
	DBUS_ERR_REG_PARAM,
	DBUS_STATUS_CANCELLED,
	DBUS_ERR_NVRAM,
	DBUS_JUMBO_NOMATCH,
	DBUS_JUMBO_BAD_FORMAT,
	DBUS_NVRAM_NONTXT,
	DBUS_ERR_RXZLP
};

#define BCM_OTP_SIZE_43236  84	/* number of 16 bit values */
#define BCM_OTP_SW_RGN_43236	24  /* start offset of SW config region */
#define BCM_OTP_ADDR_43236 0x18000800 /* address of otp base */

#define ERR_CBMASK_TXFAIL		0x00000001
#define ERR_CBMASK_RXFAIL		0x00000002
#define ERR_CBMASK_ALL			0xFFFFFFFF

#define DBUS_CBCTL_WRITE			0
#define DBUS_CBCTL_READ				1
#if defined(INTR_EP_ENABLE)
#define DBUS_CBINTR_POLL			2
#endif /* defined(INTR_EP_ENABLE) */

#define DBUS_TX_RETRY_LIMIT		3		/* retries for failed txirb */
#define DBUS_TX_TIMEOUT_INTERVAL	250		/* timeout for txirb complete, in ms */

/*
 * The max TCB/RCB data buffer size
 * With USB RPC aggregation on,
 *   rx buffer has to be a single big chunk memory due to dongle->host aggregation
 *     Upper layer has to do byte copy to deaggregate the buffer to satisfy WL driver
 *       one buffer per pkt requirement
 *     Windows Vista may be able to use MDL to workaround this requirement
 *   tx buffer has to copy over RPC buffer since they are managed in different domain
 *     Without copy, DBUS and RPC has to break the encapsulation, which is not implemented
 *     RPC aggregated buffer arrives as a chained buffers. bypte copy needs to traverse the chain
 *     to form one continuous USB irb.
 *   These buffer size must accomodate the MAX rpc agg size in both direction
 *   #define BCM_RPC_TP_DNGL_AGG_MAX_BYTE
 *   #define BCM_RPC_TP_HOST_AGG_MAX_BYTE
 * Without USB RPC aggregation, these buffer size can be smaller like normal 2K
 *  to fit max tcp pkt(ETH_MAX_DATA_SIZE) + d11/phy/rpc/overhead
 *
 * The number of buffer needed is upper layer dependent. e.g. rpc defines BCM_RPC_TP_DBUS_NTXQ
 */
#define DBUS_BUFFER_SIZE_TX	32000
#define DBUS_BUFFER_SIZE_RX	24000

#define DBUS_BUFFER_SIZE_TX_NOAGG	2048
#define DBUS_BUFFER_SIZE_RX_NOAGG	2048

/** DBUS types */
enum {
	DBUS_USB,
	DBUS_SDIO,
	DBUS_SPI,
	DBUS_UNKNOWN
};

enum dbus_state {
	DBUS_STATE_DL_PENDING,
	DBUS_STATE_DL_DONE,
	DBUS_STATE_UP,
	DBUS_STATE_DOWN,
	DBUS_STATE_PNP_FWDL,
	DBUS_STATE_DISCONNECT,
	DBUS_STATE_SLEEP,
	DBUS_STATE_DL_NEEDED
};

enum dbus_pnp_state {
	DBUS_PNP_DISCONNECT,
	DBUS_PNP_SLEEP,
	DBUS_PNP_RESUME
};

enum dbus_file {
    DBUS_FIRMWARE,
    DBUS_NVFILE
};

typedef enum _DEVICE_SPEED {
	INVALID_SPEED = -1,
	LOW_SPEED     =  1,	/**< USB 1.1: 1.5 Mbps */
	FULL_SPEED,     	/**< USB 1.1: 12  Mbps */
	HIGH_SPEED,		/**< USB 2.0: 480 Mbps */
	SUPER_SPEED,		/**< USB 3.0: 4.8 Gbps */
} DEVICE_SPEED;

typedef struct {
	int bustype;
	int vid;
	int pid;
	int devid;
	int chiprev; /**< chip revsion number */
	int mtu;
	int nchan; /**< Data Channels */
	int has_2nd_bulk_in_ep;
} dbus_attrib_t;

/* FIX: Account for errors related to DBUS;
 * Let upper layer account for packets/bytes
 */
typedef struct {
	uint32 rx_errors;
	uint32 tx_errors;
	uint32 rx_dropped;
	uint32 tx_dropped;
} dbus_stats_t;

/**
 * Configurable BUS parameters
 */
enum {
	DBUS_CONFIG_ID_RXCTL_DEFERRES = 1,
	DBUS_CONFIG_ID_AGGR_LIMIT,
	DBUS_CONFIG_ID_KEEPIF_ON_DEVRESET
};

typedef struct {
	uint32 config_id;
	union {
		uint32 general_param;
		bool rxctl_deferrespok;
		struct {
			int maxrxsf;
			int maxrxsize;
			int maxtxsf;
			int maxtxsize;
		} aggr_param;
	};
} dbus_config_t;

/**
 * External Download Info
 */
typedef struct dbus_extdl {
	uint8 *fw;
	int fwlen;
	uint8 *vars;
	int varslen;
} dbus_extdl_t;

struct dbus_callbacks;
struct exec_parms;

typedef void *(*probe_cb_t)(void *arg, const char *desc, uint32 bustype, uint32 hdrlen);
typedef void (*disconnect_cb_t)(void *arg);
typedef void *(*exec_cb_t)(struct exec_parms *args);

/** Client callbacks registered during dbus_attach() */
typedef struct dbus_callbacks {
	void (*send_complete)(void *cbarg, void *info, int status);
	void (*recv_buf)(void *cbarg, uint8 *buf, int len);
	void (*recv_pkt)(void *cbarg, void *pkt);
	void (*txflowcontrol)(void *cbarg, bool onoff);
	void (*errhandler)(void *cbarg, int err);
	void (*ctl_complete)(void *cbarg, int type, int status);
	void (*state_change)(void *cbarg, int state);
	void *(*pktget)(void *cbarg, uint len, bool send);
	void (*pktfree)(void *cbarg, void *p, bool send);
} dbus_callbacks_t;

struct dbus_pub;
struct bcmstrbuf;
struct dbus_irb;
struct dbus_irb_rx;
struct dbus_irb_tx;
struct dbus_intf_callbacks;

typedef struct {
	void* (*attach)(struct dbus_pub *pub, void *cbarg, struct dbus_intf_callbacks *cbs);
	void (*detach)(struct dbus_pub *pub, void *bus);

	int (*up)(void *bus);
	int (*down)(void *bus);
	int (*send_irb)(void *bus, struct dbus_irb_tx *txirb);
	int (*recv_irb)(void *bus, struct dbus_irb_rx *rxirb);
	int (*cancel_irb)(void *bus, struct dbus_irb_tx *txirb);
	int (*send_ctl)(void *bus, uint8 *buf, int len);
	int (*recv_ctl)(void *bus, uint8 *buf, int len);
	int (*get_stats)(void *bus, dbus_stats_t *stats);
	int (*get_attrib)(void *bus, dbus_attrib_t *attrib);

	int (*pnp)(void *bus, int evnt);
	int (*remove)(void *bus);
	int (*resume)(void *bus);
	int (*suspend)(void *bus);
	int (*stop)(void *bus);
	int (*reset)(void *bus);

	/* Access to bus buffers directly */
	void *(*pktget)(void *bus, int len);
	void (*pktfree)(void *bus, void *pkt);

	int  (*iovar_op)(void *bus, const char *name, void *params, int plen, void *arg, int len,
		bool set);
	void (*dump)(void *bus, struct bcmstrbuf *strbuf);
	int  (*set_config)(void *bus, dbus_config_t *config);
	int  (*get_config)(void *bus, dbus_config_t *config);

	bool (*device_exists)(void *bus);
	bool (*dlneeded)(void *bus);
	int  (*dlstart)(void *bus, uint8 *fw, int len);
	int  (*dlrun)(void *bus);
	bool (*recv_needed)(void *bus);

	void *(*exec_rxlock)(void *bus, exec_cb_t func, struct exec_parms *args);
	void *(*exec_txlock)(void *bus, exec_cb_t func, struct exec_parms *args);

	int (*tx_timer_init)(void *bus);
	int (*tx_timer_start)(void *bus, uint timeout);
	int (*tx_timer_stop)(void *bus);

	int (*sched_dpc)(void *bus);
	int (*lock)(void *bus);
	int (*unlock)(void *bus);
	int (*sched_probe_cb)(void *bus);

	int (*shutdown)(void *bus);

	int (*recv_stop)(void *bus);
	int (*recv_resume)(void *bus);

	int (*recv_irb_from_ep)(void *bus, struct dbus_irb_rx *rxirb, uint ep_idx);

	int (*readreg)(void *bus, uint32 regaddr, int datalen, uint32 *value);

	/* Add from the bottom */
} dbus_intf_t;

typedef struct dbus_pub {
	struct osl_info *osh;
	dbus_stats_t stats;
	dbus_attrib_t attrib;
	enum dbus_state busstate;
	DEVICE_SPEED device_speed;
	int ntxq, nrxq, rxsize;
	void *bus;
	struct shared_info *sh;
    void *dev_info;
} dbus_pub_t;

#define BUS_INFO(bus, type) (((type *) bus)->pub->bus)

#define	ALIGNED_LOCAL_VARIABLE(var, align)					\
	uint8	buffer[SDALIGN+64];						\
	uint8	*var = (uint8 *)(((uintptr)&buffer[0]) & ~(align-1)) + align;

/*
 * Public Bus Function Interface
 */

/*
 * FIX: Is there better way to pass OS/Host handles to DBUS but still
 *      maintain common interface for all OS??
 * Under Linux, param1 and param2 are NULL;
 */
extern int dbus_register(int vid, int pid, probe_cb_t prcb, disconnect_cb_t discb, void *prarg,
	void *param1, void *param2);
extern int dbus_deregister(void);

extern dbus_pub_t *dbus_attach(struct osl_info *osh, int rxsize, int nrxq, int ntxq,
	void *cbarg, dbus_callbacks_t *cbs, dbus_extdl_t *extdl, struct shared_info *sh);
extern void dbus_detach(dbus_pub_t *pub);

extern int dbus_download_firmware(dbus_pub_t *pub);
extern int dbus_up(dbus_pub_t *pub);
extern int dbus_down(dbus_pub_t *pub);
extern int dbus_stop(dbus_pub_t *pub);
extern int dbus_shutdown(dbus_pub_t *pub);
extern void dbus_flowctrl_rx(dbus_pub_t *pub, bool on);

extern int dbus_send_txdata(dbus_pub_t *dbus, void *pktbuf);
extern int dbus_send_buf(dbus_pub_t *pub, uint8 *buf, int len, void *info);
extern int dbus_send_pkt(dbus_pub_t *pub, void *pkt, void *info);
extern int dbus_send_ctl(dbus_pub_t *pub, uint8 *buf, int len);
extern int dbus_recv_ctl(dbus_pub_t *pub, uint8 *buf, int len);
extern int dbus_recv_bulk(dbus_pub_t *pub, uint32 ep_idx);
extern int dbus_poll_intr(dbus_pub_t *pub);
extern int dbus_get_stats(dbus_pub_t *pub, dbus_stats_t *stats);
extern int dbus_get_attrib(dbus_pub_t *pub, dbus_attrib_t *attrib);
extern int dbus_get_device_speed(dbus_pub_t *pub);
extern int dbus_set_config(dbus_pub_t *pub, dbus_config_t *config);
extern int dbus_get_config(dbus_pub_t *pub, dbus_config_t *config);
extern void * dbus_get_devinfo(dbus_pub_t *pub);

extern void *dbus_pktget(dbus_pub_t *pub, int len);
extern void dbus_pktfree(dbus_pub_t *pub, void* pkt);

extern int dbus_set_errmask(dbus_pub_t *pub, uint32 mask);
extern int dbus_pnp_sleep(dbus_pub_t *pub);
extern int dbus_pnp_resume(dbus_pub_t *pub, int *fw_reload);
extern int dbus_pnp_disconnect(dbus_pub_t *pub);

extern int dbus_iovar_op(dbus_pub_t *pub, const char *name,
	void *params, int plen, void *arg, int len, bool set);
#ifdef BCMDBG
extern void dbus_hist_dump(dbus_pub_t *pub, struct bcmstrbuf *b);
#endif /* BCMDBG */

extern void *dhd_dbus_txq(const dbus_pub_t *pub);
extern uint dhd_dbus_hdrlen(const dbus_pub_t *pub);

/*
 * Private Common Bus Interface
 */

/** IO Request Block (IRB) */
typedef struct dbus_irb {
	struct dbus_irb *next;	/**< it's casted from dbus_irb_tx or dbus_irb_rx struct */
} dbus_irb_t;

typedef struct dbus_irb_rx {
	struct dbus_irb irb; /* Must be first */
	uint8 *buf;
	int buf_len;
	int actual_len;
	void *pkt;
	void *info;
	void *arg;
} dbus_irb_rx_t;

typedef struct dbus_irb_tx {
	struct dbus_irb irb; 	/** Must be first */
	uint8 *buf;		/** mutually exclusive with struct member 'pkt' */
	int len;		/** length of field 'buf' */
	void *pkt;		/** mutually exclusive with struct member 'buf' */
	int retry_count;
	void *info;
	void *arg;
	void *send_buf;		/**< linear  bufffer for LINUX when aggreagtion is enabled */
} dbus_irb_tx_t;

/**
 * DBUS interface callbacks are different from user callbacks
 * so, internally, different info can be passed to upper layer
 */
typedef struct dbus_intf_callbacks {
	void (*send_irb_timeout)(void *cbarg, dbus_irb_tx_t *txirb);
	void (*send_irb_complete)(void *cbarg, dbus_irb_tx_t *txirb, int status);
	void (*recv_irb_complete)(void *cbarg, dbus_irb_rx_t *rxirb, int status);
	void (*errhandler)(void *cbarg, int err);
	void (*ctl_complete)(void *cbarg, int type, int status);
	void (*state_change)(void *cbarg, int state);
	bool (*isr)(void *cbarg, bool *wantdpc);
	bool (*dpc)(void *cbarg, bool bounded);
	void (*watchdog)(void *cbarg);
	void *(*pktget)(void *cbarg, uint len, bool send);
	void (*pktfree)(void *cbarg, void *p, bool send);
	struct dbus_irb* (*getirb)(void *cbarg, bool send);
	void (*rxerr_indicate)(void *cbarg, bool on);
} dbus_intf_callbacks_t;

/*
 * Porting: To support new bus, port these functions below
 */

/*
 * Bus specific Interface
 * Implemented by dbus_usb.c/dbus_sdio.c
 */
extern int dbus_bus_register(int vid, int pid, probe_cb_t prcb, disconnect_cb_t discb, void *prarg,
	dbus_intf_t **intf, void *param1, void *param2);
extern int dbus_bus_deregister(void);
extern void dbus_bus_fw_get(void *bus, uint8 **fw, int *fwlen, int *decomp);

/*
 * Bus-specific and OS-specific Interface
 * Implemented by dbus_usb_[linux/ndis].c/dbus_sdio_[linux/ndis].c
 */
extern int dbus_bus_osl_register(int vid, int pid, probe_cb_t prcb, disconnect_cb_t discb,
	void *prarg, dbus_intf_t **intf, void *param1, void *param2);
extern int dbus_bus_osl_deregister(void);

/*
 * Bus-specific, OS-specific, HW-specific Interface
 * Mainly for SDIO Host HW controller
 */
extern int dbus_bus_osl_hw_register(int vid, int pid, probe_cb_t prcb, disconnect_cb_t discb,
	void *prarg, dbus_intf_t **intf);
extern int dbus_bus_osl_hw_deregister(void);

extern uint usbdev_bulkin_eps(void);
#if defined(BCM_REQUEST_FW)
extern void *dbus_get_fw_nvfile(int devid, int chiprev, uint8 **fw, int *fwlen, int type,
  uint16 boardtype, uint16 boardrev);
extern void dbus_release_fw_nvfile(void *firmware);
#endif  /* #if defined(BCM_REQUEST_FW) */

#if defined(EHCI_FASTPATH_TX) || defined(EHCI_FASTPATH_RX)
/* XXX
 * Include file for the ECHI fastpath optimized USB
 * Practically all the lines below have equivalent in some structures in other include (or even
 * source) files This violates all kind of structure and layering, but cutting through layers is
 * what the optimization is about. The definitions are NOT literally borrowed from any GPLd code;
 * the file is intended to be GPL-clean
 *
 * Note that while some resemblance between this code and GPLd code in Linux might exist, it is
 * due to the common sibling. See FreeBSD: head/sys/dev/usb/controller/ehci.h for the source of
 * inspiration :-)
 *
 * The code assumes little endian throughout
 */

#if !defined(linux)
#error "EHCI fastpath is for Linux only."
#endif // endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
	/* Backward compatibility */
	typedef unsigned int gfp_t;

	#define dma_pool pci_pool
	#define dma_pool_create(name, dev, size, align, alloc) \
		pci_pool_create(name, dev, size, align, alloc, GFP_DMA | GFP_ATOMIC)
	#define dma_pool_destroy(pool) pci_pool_destroy(pool)
	#define dma_pool_alloc(pool, flags, handle) pci_pool_alloc(pool, flags, handle)
	#define dma_pool_free(pool, vaddr, addr) pci_pool_free(pool, vaddr, addr)

	#define dma_map_single(dev, addr, size, dir)	pci_map_single(dev, addr, size, dir)
	#define dma_unmap_single(dev, hnd, size, dir)	pci_unmap_single(dev, hnd, size, dir)
	#define DMA_FROM_DEVICE PCI_DMA_FROMDEVICE
	#define DMA_TO_DEVICE PCI_DMA_TODEVICE
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)) */

/* Availability of these functions varies (when present, they have two arguments) */
#ifndef hc32_to_cpu
	#define hc32_to_cpu(x)	le32_to_cpu(x)
	#define cpu_to_hc32(x)	cpu_to_le32(x)
	typedef unsigned int __hc32;
#else
	#error Two-argument functions needed
#endif // endif

/* Private USB opcode base */
#define EHCI_FASTPATH		0x31
#define	EHCI_SET_EP_BYPASS	EHCI_FASTPATH
#define	EHCI_SET_BYPASS_CB	(EHCI_FASTPATH + 1)
#define	EHCI_SET_BYPASS_DEV	(EHCI_FASTPATH + 2)
#define	EHCI_DUMP_STATE		(EHCI_FASTPATH + 3)
#define	EHCI_SET_BYPASS_POOL	(EHCI_FASTPATH + 4)
#define	EHCI_CLR_EP_BYPASS	(EHCI_FASTPATH + 5)

/*
 * EHCI QTD structure (hardware and extension)
 * NOTE that is does not need to (and does not) match its kernel counterpart
 */
#define EHCI_QTD_NBUFFERS       5
#define EHCI_QTD_ALIGN  	32
#define EHCI_BULK_PACKET_SIZE	512
#define EHCI_QTD_XACTERR_MAX	32

struct ehci_qtd {
	/* Hardware map */
	volatile uint32_t	qtd_next;
	volatile uint32_t	qtd_altnext;
	volatile uint32_t	qtd_status;
#define	EHCI_QTD_GET_BYTES(x)	(((x)>>16) & 0x7fff)
#define	EHCI_QTD_IOC            0x00008000
#define	EHCI_QTD_GET_CERR(x)	(((x)>>10) & 0x3)
#define EHCI_QTD_SET_CERR(x)    ((x) << 10)
#define	EHCI_QTD_GET_PID(x)	(((x)>>8) & 0x3)
#define EHCI_QTD_SET_PID(x)     ((x) <<  8)
#define EHCI_QTD_ACTIVE         0x80
#define EHCI_QTD_HALTED         0x40
#define EHCI_QTD_BUFERR         0x20
#define EHCI_QTD_BABBLE         0x10
#define EHCI_QTD_XACTERR        0x08
#define EHCI_QTD_MISSEDMICRO    0x04
	volatile uint32_t 	qtd_buffer[EHCI_QTD_NBUFFERS];
	volatile uint32_t 	qtd_buffer_hi[EHCI_QTD_NBUFFERS];

	/* Implementation extension */
	dma_addr_t		qtd_self;		/**< own hardware address */
	struct ehci_qtd		*obj_next;		/**< software link to the next QTD */
	void			*rpc;			/**< pointer to the rpc buffer */
	size_t			length;			/**< length of the data in the buffer */
	void			*buff;			/**< pointer to the reassembly buffer */
	int			xacterrs;		/**< retry counter for qtd xact error */
} __attribute__ ((aligned(EHCI_QTD_ALIGN)));

#define	EHCI_NULL	__constant_cpu_to_le32(1) /* HW null pointer shall be odd */

#define SHORT_READ_Q(token) (EHCI_QTD_GET_BYTES(token) != 0 && EHCI_QTD_GET_PID(token) == 1)

/**
 * Queue Head
 * NOTE This structure is slightly different from the one in the kernel; but needs to stay
 * compatible.
 */
struct ehci_qh {
	/* Hardware map */
	volatile uint32_t 	qh_link;
	volatile uint32_t 	qh_endp;
	volatile uint32_t 	qh_endphub;
	volatile uint32_t 	qh_curqtd;

	/* QTD overlay */
	volatile uint32_t	ow_next;
	volatile uint32_t	ow_altnext;
	volatile uint32_t	ow_status;
	volatile uint32_t	ow_buffer [EHCI_QTD_NBUFFERS];
	volatile uint32_t	ow_buffer_hi [EHCI_QTD_NBUFFERS];

	/* Extension (should match the kernel layout) */
	dma_addr_t		unused0;
	void 			*unused1;
	struct list_head	unused2;
	struct ehci_qtd		*dummy;
	struct ehci_qh		*unused3;

	struct ehci_hcd		*unused4;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct kref		unused5;
	unsigned		unused6;

	uint8_t			unused7;

	/* periodic schedule info */
	uint8_t			unused8;
	uint8_t			unused9;
	uint8_t			unused10;
	uint16_t		unused11;
	uint16_t		unused12;
	uint16_t		unused13;
	struct usb_device	*unused14;
#else
	unsigned		unused5;

	u8			unused6;

	/* periodic schedule info */
	u8			unused7;
	u8			unused8;
	u8			unused9;
	unsigned short		unused10;
	unsigned short		unused11;
#define NO_FRAME ((unsigned short)~0)
#ifdef EHCI_QUIRK_FIX
	struct usb_device	*unused12;
#endif /* EHCI_QUIRK_FIX */
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)) */
	struct ehci_qtd		*first_qtd;
		/* Link to the first QTD; this is an optimized equivalent of the qtd_list field */
		/* NOTE that ehci_qh in ehci.h shall reserve this word */
} __attribute__ ((aligned(EHCI_QTD_ALIGN)));

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0))
/** The corresponding structure in the kernel is used to get the QH */
struct hcd_dev {	/* usb_device.hcpriv points to this */
	struct list_head	unused0;
	struct list_head	unused1;

	/* array of QH pointers */
	void			*ep[32];
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)) */

int optimize_qtd_fill_with_rpc(const dbus_pub_t *pub,  int epn, struct ehci_qtd *qtd, void *rpc,
	int token, int len);
int optimize_qtd_fill_with_data(const dbus_pub_t *pub, int epn, struct ehci_qtd *qtd, void *data,
	int token, int len);
int optimize_submit_async(struct ehci_qtd *qtd, int epn);
void inline optimize_ehci_qtd_init(struct ehci_qtd *qtd, dma_addr_t dma);
struct ehci_qtd *optimize_ehci_qtd_alloc(gfp_t flags);
void optimize_ehci_qtd_free(struct ehci_qtd *qtd);
void optimize_submit_rx_request(const dbus_pub_t *pub, int epn, struct ehci_qtd *qtd_in, void *buf);
#endif /* EHCI_FASTPATH_TX || EHCI_FASTPATH_RX */

void  dbus_flowctrl_tx(void *dbi, bool on);
#endif /* __DBUS_H__ */
