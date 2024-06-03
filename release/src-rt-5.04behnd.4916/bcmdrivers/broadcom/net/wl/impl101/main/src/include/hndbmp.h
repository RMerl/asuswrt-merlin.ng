/**
 * +--------------------------------------------------------------------------+
 *
 *  Dongle mode Buffer Memory Pager (BMP) service.
 *
 *  Major changes to BMP service must upgrade the BMP Service Revision (HNDBMP)
 *  - E.g. New BMP Users, design changes pertaining to HBM/DBM management.
 *         Implementation changes need not.
 *
 *  Revision 1: Draft of standalone BMP service with a UnitTest User
 *  Revision 2: Integration of User: AMPDU RxReorder Queue
 *  Revision  : Integration of User: IV Trace Window
 *
 *
 *  Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *  SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 *  OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 *  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *  <<Broadcom-WL-IPTag/Open:>>
 *
 *  $Id: hndbmp.h 831709 2023-10-25 02:34:58Z $
 *
 *  vim: set ts=4 noet sw=4 tw=80:
 *  -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * +--------------------------------------------------------------------------+
 */
#ifndef __hnd_bmp_h_included__
#define __hnd_bmp_h_included__

#include <typedefs.h>

struct si_pub;                          // Forward decl: siutils.h
struct osl_info;                        // Forward decl: osl_decl.h

// List of well known BMP user application indices
#define BMP_USR_ARQW0           0       // AMPDU Rx Reorder Queue W0 = BA64
#define BMP_USR_ARQW1           1       // AMPDU Rx Reorder Queue W1 = BA256
#define BMP_USR_IVTW0           2       // Replay/IV Trace Window W0 = BA64
#define BMP_USR_IVTW1           3       // Replay/IV Trace Window W1 = BA256
#define BMP_USR_TAFSC           4       // TAF TID scheduler
#define BMP_USR_TXSRB           5       // MLO TxS Reorder Buffer
#define BMP_USR_TXSRQ           6       // MLO TxS Reorder Queue
#define BMP_USR_TOTAL           7       // 7 well known users of BMP service

/**
 * +--------------------------------------------------------------------------+
 * Buffer Memory Pager Users HME memory
 * +--------------------------------------------------------------------------+
 */

/* AMPDU RxReorder Queue (ARQ) state:
 *
 * Per ARQ: 8 Byte BMP preamble + BA sized 4B pktid array
 * Per BA256 ARQ = [8 Bytes]    + [(256 x 4B) = 1024 Bytes]
 * Per BA64  ARQ = [8 Bytes]    + [( 64 x 4B) =  256 Bytes]
 *
 * Buffer size MUST be multiples of 8 Bytes, allowing for 64bit PIO access of
 * the 8 Byte BMP header preamble using a sbtopcie uint64 aligned access.
 *
 * BA64  Buffer size =  264 Bytes
 * BA256 Buffer size = 1032 Bytes
 *
 * BA256 enable: 512 ARQ64 and 384 ARQ256 = (512 x 264B) + (384 x 1032B)
 *                                        = 519 KBytes
 *               Rounded to 4KB HME Pages = 130 HME Pages = 532,480 Bytes
 *
 * BA256 disable: 1024 ARQ64 and 0 ARQ256 = (1024 x 264B) + (0 x 1032B)
 *                                        = 264 KBytes
 *                                        = 66 HME Pages = 270,336 Bytes
 */
#if defined(WLAMPDU_BMPARQ)
#define BMP_USER_ARQ_BYTES	((BMPARQ_BA64_HBM_MAX * BMPARQ_BA64_BUF_SZ) + \
				 (BMPARQ_BA256_HBM_MAX * BMPARQ_BA256_BUF_SZ))
#else /* ! WLAMPDU_BMPARQ */
#define BMP_USER_ARQ_BYTES	(0)
#endif /* WLAMPDU_BMPARQ */
/* TAF Scheduler (TSC), default 2048 entries
 * Per TID: 8 Byte BMP preamble + TAF_STATIC_CONTEXT_SIZE
 *   Non-debug = [188 Bytes]
 *   Debug     = [192 Bytes]
 */
#ifdef BCMDBG
#define BMP_USER_BMPTSC_SIZE	((192 + 8) * 2048)
#else
#define BMP_USER_BMPTSC_SIZE	((188 + 8) * 2048)
#endif /* BCMDBG */

#define BMP_USER_IVT_BYTES	(0)

#if defined(WL_TXS_MN_BMPTSR)
/* MLO Tx status ncons Reordering Queue(TRQ)
 * Per TRQ: 8 Byte BMP preamble + [(sizeof(void *) + sizeof(bmp_idx_t))] *
 * [Depth 256] 1536B = 1544 Bytes
 */
#define BMP_USER_TXSRQ_BYTES	(BMPTRQ_BUF_SZ * BMPTRQ_HBM_MAX)

/* MLO Tx status ncons Reordering Buffer(TRB)
 * Per TRB: 8 Byte BMP preamble + sizeof(d11txsts_mn_t) 64B = 72 Bytes
 */
#define BMP_USER_TXSRB_BYTES	(BMPTRB_BUF_SZ * BMPTRB_HBM_MAX)
#else
#define BMP_USER_TXSRQ_BYTES	(0)
#define BMP_USER_TXSRB_BYTES	(0)
#endif /* WL_TXS_MN_BMPTSR */

#define BMP_HME_SIZE		(BMP_USER_ARQ_BYTES + BMP_USER_IVT_BYTES + \
				 BMP_USER_BMPTSC_SIZE + BMP_USER_TXSRQ_BYTES + \
				 BMP_USER_TXSRB_BYTES)

/**
 * +--------------------------------------------------------------------------+
 *  BMP Service API
 * +--------------------------------------------------------------------------+
 */

typedef uint16 bmp_idx_t;               // BMP index refers to HBM buffer index

#define BMP_IDX_ERR     ((bmp_idx_t)0)  // Error return
#define BMP_IDX_INV     ((bmp_idx_t)0)  // Used to imply invalid buffer

#define BMP_DMA_MAX     (4096)          // Max buffer size constrained to 4 KB

typedef enum  bmp_dir
{
	bmp_dir_d2h         = 0,            // PageOUT from DBM to HBM
	bmp_dir_h2d         = 1             // PageIN  from HBM to DBM
} bmp_dir_t;

/**
 *  Every Buffer has a 8 Byte preamble header, which an user application may
 *  use to save a buffer to user context info.
 *
 *  Caution: HBM BMP header is accessed as a uint64 SBTOPCIE PIO, and hence
 *  a user's buffer size must be 8 Byte aligned (i.e. buffer size is a multiple
 *  of 8).
 */
#define BMP_HDR_BYTES   (8)             // Preamble Header size in Bytes
#define BMP_HDR_WORDS   (2)             // Preamble Header size in uint32 words

typedef union bmp_hdr
{
	uint8        u8[BMP_HDR_BYTES];
	uint32      u32[BMP_HDR_WORDS];
	uint64      u64;
} bmp_hdr_t;

//  BMP service callback reason codes delivered in requests to User
typedef enum bmp_cb_reason
{
	BMP_CB_HBM_UNLINK         = 0,      // Unlink a clean HBM buffer
	BMP_CB_HBM_THRESH         = 1,      // HBM Pool low threshold crossed
	BMP_CB_DBM_UNLINK         = 2       // Unlink a DBM buffer from LRU
} bmp_cb_reason_t;

typedef int (*bmp_cb_fn_t)(void * cb_data, bmp_cb_reason_t reason,
                           bmp_idx_t bmp_idx, bmp_hdr_t * bmp_hdr);

/**
 * +--------------------------------------------------------------------------+
 *  BMP object management APIs.
 *  NOTE: Parameter bmp_idx refers to a 16bit HBM index.
 * +--------------------------------------------------------------------------+
 */

//  BUF     Fetch a pointer to cached DBM buffer
void      * bmp_buf(uint32 usr_idx, bmp_idx_t bmp_idx);

//  GET     Allocate a new BMP buffer, with optional reverse link BMP header
bmp_idx_t   bmp_get(uint32 usr_idx, bmp_hdr_t * usr_hdr);

//  PUT     Deallocate a previously allocated HBM buffer.
void        bmp_put(uint32 usr_idx, bmp_idx_t bmp_idx);

//  SET     Setup the BMP header to link buffer to user context
void        bmp_set(uint32 usr_idx, bmp_idx_t bmp_idx, bmp_hdr_t * bmp_hdr);

//  CLR     User indicates that a BMP buffer is not used, but still allocated.
//          DBM cached buffer may be freed without PageOUT as buffer is clear.
void        bmp_clr(uint32 usr_idx, bmp_idx_t bmp_idx);

//  POP     Page Operation:  PageOut to HBM or PageIN from HBM
void        bmp_pop(uint32 usr_idx, bmp_idx_t bmp_idx, bmp_dir_t bmp_dir);

//  CNT     Access Run-time statistics counters for DBM and HBM
void        bmp_cnt(uint32 usr_idx, uint32 * dbm_free,
                                    uint32 * hbm_free, uint32 * hbm_clear);

/**
 * +--------------------------------------------------------------------------+
 *  BMP FSM transitions APIs : Commence or Terminate a BMP epoch
 * +--------------------------------------------------------------------------+
 */
//  BGN     Begin an epoch pre WLAN RxProcess - e.g. start of CFP binning
void        bmp_bgn(uint32 usr_idx);

//  END     End an epoch post WLAN RxProcess - e.g. RxCpl handoffs to Bus layer
void        bmp_end(uint32 usr_idx);

//  SYNC    Sync DMA completions of all ongoing PageOUTs or PageINs
void        bmp_sync(void);

/**
 * +--------------------------------------------------------------------------+
 *  BMP service debug and audit APIs
 * +--------------------------------------------------------------------------+
 */
void        bmp_audit_usr(uint32 usr_idx); // BCMDBG || BMP_DEBUG_BUILD

void        bmp_dump_usr(uint32 usr_idx, bool verbose, struct bcmstrbuf *b);
void        bmp_dump_sys(bool verbose);

/**
 * +--------------------------------------------------------------------------+
 *  BMP user configuration
 * +--------------------------------------------------------------------------+
 *
 *  User applications may register with BMP during the PCIe IPC Link phase
 *  (i.e. after hme_link_pcie_ipc), specifying:
 *    - Well known BMP user application's index
 *    - Size of each buffer. Buffer size must be a multiple of 8Bytes
 *    - Max number of buffers in Host Buffer Memory   max 64K  (HBM)
 *    - Max number of buffers in Dongle Buffer Memory max 255  (DBM)
 *    - User's callback handler, to be invoked on HBM or DBM depletions
 *
 *  Specify HBM and DBM items in power-of-2.
 *  One buffer (#0) is reserved by BMP service and not available to User.
 *
 *  Returns BCME_OK or BCME_ERROR
 */

//  REG     Register a User Application with BMP service
int         bmp_reg(uint32 usr_idx, uint32 buf_size, uint32 hbm_total,
                    uint32 dbm_total, bmp_cb_fn_t cb_fn, void * cb_data);

/**
 * +--------------------------------------------------------------------------+
 *  BMP service initialization and command line control
 * +--------------------------------------------------------------------------+
 */
//  INIT    BMP Service initialization, see rte.c, hnd_init
int         BCMATTACHFN(bmp_init)(struct si_pub * sih, struct osl_info * osh);
//  CMD     BMP Service console command handler, see rte.c: hnd_cons_add_cmd
void        bmp_cmd(void *arg, int argc, char *argv[]);

#endif /* __hnd_bmp_h_included__ */
