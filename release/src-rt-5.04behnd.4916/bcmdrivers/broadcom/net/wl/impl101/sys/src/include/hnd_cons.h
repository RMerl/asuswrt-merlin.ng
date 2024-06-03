/*
 * Console support for RTE - for host use only.
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
 * $Id: hnd_cons.h 821234 2023-02-06 14:16:52Z $
 */
#ifndef	_hnd_cons_h_
#define	_hnd_cons_h_

#include <typedefs.h>
#include <siutils.h>

#if defined(RWL_DONGLE) || defined(UART_REFLECTOR)
/* For Dongle uart tranport max cmd len is 256 bytes + header length (16 bytes)
 *  In case of ASD commands we are not sure about how much is the command size
 *  To be on the safe side, input buf len CBUF_LEN is increased to max (512) bytes.
 */
#define RWL_MAX_DATA_LEN 	(512 + 8)	/* allow some extra bytes for '/n' termination */
#define CBUF_LEN	(RWL_MAX_DATA_LEN + 64)  /* allow 64 bytes for header ("rwl...") */
#else
#define CBUF_LEN	(128)
#endif /* RWL_DONGLE || UART_REFLECTOR */

#ifndef LOG_BUF_LEN
#if defined(BCMDBG) || defined(BCM_BIG_LOG)
#define LOG_BUF_LEN	(16 * 1024)
#elif defined(ATE_BUILD)
#define LOG_BUF_LEN	(1024 * 1024)
#elif defined(BCMQT)
#define LOG_BUF_LEN	(16 * 1024)
#else
#define LOG_BUF_LEN	1024
#endif
#endif /* LOG_BUF_LEN */

#if defined(HNDUCLS)
#define UCLS_DEBUG

#define UCLS_RING_SIZE		512
#define UCLS_RING_ALIGN_BITS	9
#define UCLS_RING_ALIGN		(1 << UCLS_RING_ALIGN_BITS)
#ifndef UCLS_RING_DEPTH
#define UCLS_RING_DEPTH		256
#endif
/* Assume that the ring depth is a power of 2 */
#define UCLS_RING_MASK	(UCLS_RING_DEPTH - 1)
#endif /* HNDUCLS */

#ifdef BOOTLOADER_CONSOLE_OUTPUT
#undef RWL_MAX_DATA_LEN
#undef CBUF_LEN
#undef LOG_BUF_LEN
#define RWL_MAX_DATA_LEN (4 * 1024 + 8)
#define CBUF_LEN	(RWL_MAX_DATA_LEN + 64)
#define LOG_BUF_LEN (16 * 1024)
#endif

typedef struct {
#ifdef BCMDONGLEHOST
	uint32		buf;		/* Can't be pointer on (64-bit) hosts */
#else
	char		*buf;
#endif
	uint		buf_size;
	uint		idx;
	uint		out_idx;	/* output index */
} hnd_log_t;

typedef struct {
	/* Virtual UART
	 *   When there is no UART (e.g. Quickturn), the host should write a complete
	 *   input line directly into cbuf and then write the length into vcons_in.
	 *   This may also be used when there is a real UART (at risk of conflicting with
	 *   the real UART).  vcons_out is currently unused.
	 */
	volatile uint	vcons_in;
	volatile uint	vcons_out;

	/* Output (logging) buffer
	 *   Console output is written to a ring buffer log_buf at index log_idx.
	 *   The host may read the output when it sees log_idx advance.
	 *   Output will be lost if the output wraps around faster than the host polls.
	 */
	hnd_log_t	log;

	/* Console input line buffer
	 *   Characters are read one at a time into cbuf until <CR> is received, then
	 *   the buffer is processed as a command line.  Also used for virtual UART.
	 */
	uint		cbuf_idx;
	char		cbuf[CBUF_LEN];
} hnd_cons_t;

#if defined(HNDUCLS)
#include <bcmpcie.h>

#define UCLS_SLOT_MAX	2

typedef enum ucls_cap {
	UCLS_CAP_NONE = 0,		/* UCLS not capable */
	UCLS_CAP_SW = 1,		/* SW supported UCLS capable */
	UCLS_CAP_HW = 2			/* HW supported UCLS capable */
} ucls_cap_t;

typedef enum culs_mode {
	UCLS_MODE_NORMAL = 0,
	UCLS_MODE_AGGRESSIVE = 1,
	UCLS_MODE_MAX = 2
} ucls_mode_t;

typedef struct hnd_cons_ucls {
	ucls_cap_t cap;
	ucls_mode_t mode;
	uint16 ring_size;
	uint16 ring_depth;
	haddr64_t ring_buf;
	uint16 rdidx;
	uint16 wridx;
	/* dongle side attributes */
	uint32 slot_iter;
	void *slots[UCLS_SLOT_MAX];
	void *slot_mem;
	uint16 bgn_idx;			/* logbuf index for xfer */
	uint16 end_idx;			/* index to the latest newline character */
	/* m2m_ucls_eng_regs_t *regs_emu; */
	void *regs_emu;
	/* volatile m2m_ucls_eng_regs_t *regs; */
	volatile void *regs;
	void *pciedev;
	void *timer;
	uint32 timestamp;		/* timestamp with last successful transfer */
#if defined(UCLS_DEBUG)
	uint32 watermark;		/* high watermark of available bytes in the logbuf */
	uint32 overwrite;		/* Number of characters overwritten */
	uint32 accumulate;		/* Number of characters accumulated */
	uint32 xfer_success;		/* Number of successful d2h xfer */
	uint32 xfer_fail;		/* Number of d2h xfer failures */
#endif /* UCLS_DEBUG */
	uint16 xfer_len;		/* The number of bytes copied to the slot buffer */
	bool ring_bell;			/* flag to ring the doorbell */
} hnd_cons_ucls_t;

#if defined(DONGLEBUILD)
extern uint32 pciedev_ucls_doorbell_addr(void *pciedev);
int ucls_link_pcie_ipc(void *pciedev, pcie_ipc_t *pcie_ipc);
hnd_cons_ucls_t *cons_ucls_get(void);
#endif /* DONGLEBUILD */
#endif /* HNDUCLS */

#endif /* _hnd_cons_h_ */
