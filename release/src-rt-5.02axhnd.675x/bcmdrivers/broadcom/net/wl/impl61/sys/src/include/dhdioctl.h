/*
 * Definitions for ioctls to access DHD iovars.
 * Based on wlioctl.h (for Broadcom 802.11abg driver).
 * (Moves towards generic ioctls for BCM drivers/iovars.)
 *
 * Definitions subject to change without notice.
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
 * $Id: dhdioctl.h 774649 2019-05-01 13:45:54Z $
 */

#ifndef _dhdioctl_h_
#define	_dhdioctl_h_

#include <typedefs.h>

/* Linux network driver ioctl encoding */
typedef struct dhd_ioctl {
	uint32 cmd;	/* common ioctl definition */
	void *buf;	/* pointer to user buffer */
	uint32 len;	/* length of user buffer */
	uint32 set;	/* get or set request boolean (optional) */
	uint32 used;	/* bytes read or written (optional) */
	uint32 needed;	/* bytes needed (optional) */
	uint32 driver;	/* to identify target driver */
} dhd_ioctl_t;

/* Underlying BUS definition */
enum {
	BUS_TYPE_USB = 0, /* for USB dongles */
	BUS_TYPE_SDIO, /* for SDIO dongles */
	BUS_TYPE_PCIE /* for PCIE dongles */
};

typedef enum {
	DMA_XFER_SUCCESS = 0,
	DMA_XFER_IN_PROGRESS,
	DMA_XFER_FAILED
} dma_xfer_status_t;

#ifdef EFI
struct control_signal_ops {
	uint32 signal;
	uint32 val;
};
enum {
	WL_REG_ON = 0,
	DEVICE_WAKE = 1,
	TIME_SYNC = 2
};

typedef struct wifi_properties {
	uint8 version;
	uint32 vendor;
	uint32 model;
	uint8 mac_addr[6];
	uint32 chip_revision;
	uint8 silicon_revision;
	uint8 is_powered;
	uint8 is_sleeping;
	char module_revision[16];	/* null terminated string */
	uint8 is_fw_loaded;
	char  fw_filename[32];		/* null terminated string */
	char nvram_filename[32];	/* null terminated string */
	uint8 channel;
	uint8 module_sn[6];
} wifi_properties_t;

#define DHD_WIFI_PROPERTIES_VERSION 0x1

#define DHD_OTP_SIZE_WORDS 912

#endif /* EFI */

/* per-driver magic numbers */
#define DHD_IOCTL_MAGIC		0x00444944

/* bump this number if you change the ioctl interface */
#define DHD_IOCTL_VERSION	1

/* DHD_IOCTL_MAXLEN_HIGH set to 32k for supporting the dump iovar's to get more info. */
#define	DHD_IOCTL_MAXLEN_HIGH	(32768)

/*
 * Increase the DHD_IOCTL_MAXLEN to 16K for supporting download of NVRAM files of size
 * > 8K. In the existing implementation when NVRAM is to be downloaded via the "vars"
 * DHD IOVAR, the NVRAM is copied to the DHD Driver memory. Later on when "dwnldstate" is
 * invoked with FALSE option, the NVRAM gets copied from the DHD driver to the Dongle
 * memory. The simple way to support this feature without modifying the DHD application,
 * driver logic is to increase the DHD_IOCTL_MAXLEN size. This macro defines the "size"
 * of the buffer in which data is exchanged between the DHD App and DHD driver.
 */
#define	DHD_IOCTL_MAXLEN	(16384)	/* max length ioctl buffer required */
#define	DHD_IOCTL_SMLEN		256		/* "small" length ioctl buffer required */

/* common ioctl definitions */
#define DHD_GET_MAGIC				0
#define DHD_GET_VERSION				1
#define DHD_GET_VAR				2
#define DHD_SET_VAR				3

/* message levels */
#define DHD_ERROR_VAL	0x0001
#define DHD_TRACE_VAL	0x0002
#define DHD_INFO_VAL	0x0004
#define DHD_DATA_VAL	0x0008
#define DHD_CTL_VAL	0x0010
#define DHD_TIMER_VAL	0x0020
#define DHD_HDRS_VAL	0x0040
#define DHD_BYTES_VAL	0x0080
#define DHD_INTR_VAL	0x0100
#define DHD_LOG_VAL	0x0200
#define DHD_GLOM_VAL	0x0400
#define DHD_EVENT_VAL	0x0800
#define DHD_BTA_VAL	0x1000
#define DHD_ISCAN_VAL	0x2000
#define DHD_ARPOE_VAL	0x4000
#define DHD_REORDER_VAL	0x8000
#define DHD_WL_VAL		0x10000
#define DHD_NOCHECKDIED_VAL		0x20000 /* UTF WAR */
#define DHD_WL_VAL2		0x40000
#define DHD_PNO_VAL		0x80000
#define DHD_RTT_VAL		0x100000
#define DHD_MSGTRACE_VAL	0x200000
#define DHD_FWLOG_VAL		0x400000
#define DHD_DBGIF_VAL		0x800000
#ifdef DHD_PCIE_NATIVE_RUNTIMEPM
#define DHD_RPM_VAL		0x1000000
#endif /* DHD_PCIE_NATIVE_RUNTIMEPM */
#define DHD_PKT_MON_VAL		0x2000000
#define DHD_PKT_MON_DUMP_VAL	0x4000000
#define DHD_ERROR_MEM_VAL	0x8000000
#define DHD_DNGL_IOVAR_SET_VAL	0x10000000 /**< logs the setting of dongle iovars */
#define DHD_LPBKDTDUMP_VAL	0x20000000
#define DHD_DNGL_IOVAR_GET_VAL	0x40000000 /**< logs 'get' of dongle iovars */

#ifdef SDTEST
/* For pktgen iovar */
typedef struct dhd_pktgen {
	uint32 version;		/* To allow structure change tracking */
	uint32 freq;		/* Max ticks between tx/rx attempts */
	uint32 count;		/* Test packets to send/rcv each attempt */
	uint32 print;		/* Print counts every <print> attempts */
	uint32 total;		/* Total packets (or bursts) */
	uint32 minlen;		/* Minimum length of packets to send */
	uint32 maxlen;		/* Maximum length of packets to send */
	uint32 numsent;		/* Count of test packets sent */
	uint32 numrcvd;		/* Count of test packets received */
	uint32 numfail;		/* Count of test send failures */
	uint32 mode;		/* Test mode (type of test packets) */
	uint32 stop;		/* Stop after this many tx failures */
} dhd_pktgen_t;

/* Version in case structure changes */
#define DHD_PKTGEN_VERSION 2

/* Type of test packets to use */
#define DHD_PKTGEN_ECHO		1 /* Send echo requests */
#define DHD_PKTGEN_SEND		2 /* Send discard packets */
#define DHD_PKTGEN_RXBURST	3 /* Request dongle send N packets */
#define DHD_PKTGEN_RECV		4 /* Continuous rx from continuous tx dongle */
#endif /* SDTEST */

/* Enter idle immediately (no timeout) */
#define DHD_IDLE_IMMEDIATE	(-1)

/* Values for idleclock iovar: other values are the sd_divisor to use when idle */
#define DHD_IDLE_ACTIVE	0	/* Do not request any SD clock change when idle */
#define DHD_IDLE_STOP   (-1)	/* Request SD clock be stopped (and use SD1 mode) */

typedef struct _dhd_pd11regs_param_t {
	uint16 start_idx;
	uint8 verbose;
	uint8 pad;
	uint8 plist[1];
} dhd_pd11regs_param_t;

typedef struct _dhd_pd11regs_buf_t {
	uint16 idx;
	uint8 pad[2];
	uint8 pbuf[1];
} dhd_pd11regs_buf_t;

/* BT logging */

#define BT_LOG_BUF_MAX_SIZE		(2 * 1024)
#define BT_LOG_BUF_NOT_AVAILABLE	0
#define BT_LOG_NEXT_BUF_NOT_AVAIL	1
#define BT_LOG_NEXT_BUF_AVAIL		2

typedef struct bt_log_buf_info {
	int availability;
	int size;
	char buf[BT_LOG_BUF_MAX_SIZE];
} bt_log_buf_info_t;
#endif /* _dhdioctl_h_ */
