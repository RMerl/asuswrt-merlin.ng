/*
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_scbq.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief Per-SCB Tx Queuing modulue for Broadcom 802.11 Networking Driver
 */

#include <wlc_cfg.h>

#if defined(TXQ_MUX)

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlc_types.h>
#include <siutils.h>
#include <wlc_rate.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlc_bsscfg.h>
#include <wlc_scb.h>
#include <wlc_scb_ratesel.h> /* for wlc_scb_ratesel_probe_ready() */
#include <wlc_prot.h> /* for WLC_PROT_CFG_SHORTPREAMBLE() */
#include <wlc_airtime.h>
#include <wlc_txtime.h>
#include <wlc_tx.h>
#include <wlc_ampdu.h>
#include <wlc_scbq.h>

/**
 * @brief Definition of state structure for the SCBQ module created by wlc_scbq_module_attach().
 *
 */
struct wlc_scbq_info {
	osl_t *osh;             /**< @brief OSL handle */
	wlc_info_t *wlc;        /**< @brief wlc_info_t handle */
	wlc_pub_t *pub;         /**< @brief wlc_pub_t handle */
	uint unit;              /**< @brief wl driver unit number for debug messages */
	int scb_handle;         /**< @brief scb cubby handle to retrieve data from scb */
	uint16 global_fc;       /**< @brief global flow control flags */
};

/**
 * @brief SCB cubby structure defining layout of reserved cubby space
 *
 * The cubby space reserved in each SCB for the SCBQ is just this structure, which contains
 * a pointer to the allocated per-SCB state.
 */
struct scbq_cubby {
	struct scbq_cubby_info *pinfo;	/**< @brief pointer to per-scb info structure */
};

/**
 * @brief per-scb state for SCBQ
 */
typedef struct scbq_cubby_info {
	wlc_scbq_info_t     *scbq_info;        /**< @brief SCBQ module info */
	struct scb          *scb;
	struct pktq         scb_txq;           /**< @brief multi-prio queue for buffered packets */
	uint16              fc;                /**< @brief flow control flags */
	wlc_mux_t           *mux[AC_COUNT];    /**< @brief the MUX we feed */
	mux_source_handle_t mux_hdl[AC_COUNT]; /**< @brief our MUX source handle */
	uint8               flags[AC_COUNT];   /**< @brief per-MUX & per-AC flags */
	scbq_overflow_fn_t  packet_overflow_handler; /**< @brief packet drop handler */
} scbq_cubby_info_t;

/**
 * Flag bits for scbq_cubby_info_t flags[] member. Flags are per AC.
 */
enum scbq_flags {
	SCBQ_F_STALLED = 0x01,      /**< @brief per-MUX & per-AC flags */
};

#define SCB_SCBQ_CUBBY_PTR(scbq_info, scb) \
	((struct scbq_cubby *)SCB_CUBBY((scb), (scbq_info)->scb_handle))
#define SCB_SCBQ_CUBBY_INFO(scbq_info, scb) \
	(SCB_SCBQ_CUBBY_PTR(scbq_info, scb)->pinfo)

#if defined(BCMDBG) || defined(WLDUMP)
static int wlc_scbq_module_dump(wlc_scbq_info_t *scbq_info, struct bcmstrbuf *b);
#endif /* defined(BCMDBG) || defined(WLDUMP) */
static int wlc_scbq_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
                            const char *name,
                            void *p, uint plen,
                            void *a, int alen,
                            int vsize,
                            struct wlc_if *wlcif);
static int wlc_scbq_cubby_init(void *context, struct scb *scb);
static void wlc_scbq_cubby_deinit(void *context, struct scb *scb);
static void wlc_scbq_scb_tx_flush(wlc_scbq_info_t *scbq_info, struct scb *scb);
#ifdef BCMDBG
static void wlc_scbq_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#endif // endif

static void wlc_scbq_enq(void *ctx, struct scb *scb, void *p, uint prec);
static uint wlc_scbq_txpktcnt(void *hdl);
static void wlc_scbq_txmod_flush(void *context, struct scb *scb);

static void wlc_scbq_scb_tx_stop(wlc_scbq_info_t *scbq_info, struct scb *scb);
static void wlc_scbq_scb_tx_start(wlc_scbq_info_t *scbq_info, struct scb *scb);

static uint wlc_scbq_output(void *ctx, uint ac, uint request_time, struct spktq *output_q);
static void wlc_scbq_init_timecalc(scbq_cubby_info_t *scbq_cubby, ratespec_t rspec, uint ac,
                                   timecalc_t *tc);

/* iovar table */
enum {
	IOV_SCBQ_GLOBAL_FC
};

static const bcm_iovar_t scbq_iovars[] = {
#if defined(BCMDBG)
	{"scbq_global_fc", IOV_SCBQ_GLOBAL_FC, 0, IOVT_INT32, 0},
#endif /* BCMDBG */
	{NULL, 0, 0, 0, 0 }
};

static txmod_fns_t BCMATTACHDATA(scbq_txmod_fns) = {
	wlc_scbq_enq,
	wlc_scbq_txpktcnt,
	wlc_scbq_txmod_flush,
	NULL
};

/* Module Attach/Detach */

/**
 * Create the SCBQ module infrastructure for the wl driver. wlc_module_register() is called
 * to register the module's handlers.
 * The module registers an SCB cubby and handlers, a Tx Module, and a dump function
 * for SCBQ state is also registered.
 */
wlc_scbq_info_t*
wlc_scbq_module_attach(wlc_info_t *wlc)
{
	wlc_scbq_info_t *scbq_info;
	osl_t *osh = wlc->osh;
	uint unit = wlc->pub->unit;
	scb_cubby_dump_t cubby_dump;

	/* allocate the main state structure */
	scbq_info = MALLOCZ(osh, sizeof(wlc_scbq_info_t));
	if (scbq_info == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			unit, __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}

	/* initialize cache of handles/info */
	scbq_info->wlc = wlc;
	scbq_info->pub = wlc->pub;
	scbq_info->osh = osh;
	scbq_info->unit = unit;

	wlc_module_register(scbq_info->pub,
	                    scbq_iovars,
	                    "scbq", scbq_info,
	                    wlc_scbq_doiovar,
	                    NULL /* watchdog_fn_t watchdog_fn */,
	                    NULL /* up_fn_t up_fn */,
	                    NULL /* down_fn_t down_fn */);

#if defined(BCMDBG) || defined(WLDUMP)
	wlc_dump_register(scbq_info->pub, "scbq", (dump_fn_t)wlc_scbq_module_dump, scbq_info);
#endif // endif

	/* register an scb cubby dump fn if the build is appropriate */
#if defined(BCMDBG) || defined(WLDUMP)
	cubby_dump = wlc_scbq_scb_dump;
#else
	cubby_dump = NULL;
#endif // endif

	/* reserve cubby in the scb container */
	scbq_info->scb_handle =
	        wlc_scb_cubby_reserve(wlc,
	                              sizeof(struct scbq_cubby), /* size of cubby space */
	                              wlc_scbq_cubby_init,	/* cubby init fn */
	                              wlc_scbq_cubby_deinit,	/* cubby deinit fn */
	                              cubby_dump,		/* cubby debug dump */
	                              scbq_info);		/* context handle */

	if (scbq_info->scb_handle < 0) {
		WL_ERROR(("wl%d:%s wlc_scb_cubby_reserve() failed\n",
		          unit, __FUNCTION__));
		goto fail;
	}

	/* register txmod function */
	wlc_txmod_fn_register(wlc, TXMOD_SCBQ, scbq_info, scbq_txmod_fns);

	return scbq_info;

fail:
	/* use detach as a common failure deallocation */
	wlc_scbq_module_detach(scbq_info);

	return NULL;
}

/**
 * Free all resources assoicated with the SCBQ module infrastructure.
 */
void
wlc_scbq_module_detach(wlc_scbq_info_t *scbq_info)
{
	if (scbq_info == NULL) {
		return;
	}

	wlc_module_unregister(scbq_info->pub, "scbq", scbq_info);

	MFREE(scbq_info->osh, scbq_info, sizeof(wlc_scbq_info_t));
}
/**
 * Set the packet overflow handler for SCBQ
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 * @param overflow_fn   Overflow palicy handler function
 */
void
wlc_scbq_register_overflow_fn(wlc_scbq_info_t *scbq_info,
	struct scb *scb, scbq_overflow_fn_t overflow_fn)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);

	/* In the init case this pointer is NULL,
	 * this prevents constant printouts as each SCB is initialized.
	 */
	if (scbq_cubby->packet_overflow_handler != NULL) {
		WL_INFORM(("wl%d: %s: Registering packet overflow handler 0x%p.\n",
		scbq_info->wlc->pub->unit, __FUNCTION__, overflow_fn));
	}
	scbq_cubby->packet_overflow_handler = overflow_fn;
}

/**
 * Update the output fn an SCBQ
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 * @param ctx           Context pointer to be provided as a parameter to the output_fn
 * @param output_fn     The mux_output_fn_t this SCBQ will register to the MUX to provide
 *                      output packets for this source.
 */
void
wlc_scbq_set_output_fn(wlc_scbq_info_t *scbq_info, struct scb *scb,
                       void *ctx, mux_output_fn_t output_fn)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	int ac;

	/* update the output function for each ac's mux source */
	for (ac = 0; ac < AC_COUNT; ac++) {
		wlc_mux_source_set_output_fn(scbq_cubby->mux[ac], scbq_cubby->mux_hdl[ac],
		                             ctx, output_fn);
	}
}

/**
 * Reset the output fn for an SCBQ to the default output fn
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 */
void
wlc_scbq_reset_output_fn(wlc_scbq_info_t *scbq_info, struct scb *scb)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);

	wlc_scbq_set_output_fn(scbq_info, scb, scbq_cubby, wlc_scbq_output);
}

/**
 * Return the given scb's tx queue
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 * @return              The multi-precidence Tx Q associated with the SCB of interest,
 *                      or NULL if no tx q is associated to the SCB
 */
struct pktq*
wlc_scbq_txq(wlc_scbq_info_t *scbq_info, struct scb *scb)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);

	return (scbq_cubby != NULL) ? &scbq_cubby->scb_txq : NULL;
}

/**
 * Default packet queue overflow handler.
 * Packets are tail dropped in the default handler
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module.
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 * @param pktq          Multi precedence packet queue.
 * @param pkt		Data packet.
 * @param prec		Queue precedence.
 */
static void wlc_scbq_drop_packet(wlc_scbq_info_t *scbq_info,
	struct scb *scb, struct pktq *q, void *pkt, uint prec)
{
	/* Tail drop packets */
	PKTFREE(scbq_info->osh, pkt, TRUE);
	WLCIFCNTINCR(scb, txnobuf);
	WLCNTINCR(scbq_info->wlc->pub->_cnt->txnobuf);

#ifdef PKTQ_LOG
	/* Update pktq_log stats */
	if (q->pktqlog) {
		pktq_counters_t* prec_cnt = q->pktqlog->_prec_cnt[prec];
		WLCNTCONDINCR(prec_cnt, prec_cnt->dropped);
	}
#else
	BCM_REFERENCE(prec);
#endif // endif
	WL_INFORM(("wl%d: %s:  prec:%d. Tail dropping.\n",
		scbq_info->wlc->pub->unit, __FUNCTION__, prec));
	return;
}

#if defined(BCMDBG) || defined(WLDUMP)
/**
 * Dump current state and statistics of the SCBQ as a whole
 */
static int
wlc_scbq_module_dump(wlc_scbq_info_t *scbq_info, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "flow control %04x\n", scbq_info->global_fc);

	return BCME_OK;
}
#endif /* BCMDBG || WLDUMP */

static int
wlc_scbq_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
                 const char *name,
                 void *p, uint plen,
                 void *a, int alen,
                 int vsize,
                 struct wlc_if *wlcif)
{
#if defined(BCMDBG)
	wlc_scbq_info_t *scbq_info = hdl;
	int32 *ret_int_ptr = (int32 *)a;
#endif // endif
	int32 int_val = 0;
	bool bool_val;
	int err = BCME_OK;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);

	switch (actionid) {
#if defined(BCMDBG)
	case IOV_GVAL(IOV_SCBQ_GLOBAL_FC):
		*ret_int_ptr = scbq_info->global_fc;
		break;
#endif /* BCMDBG */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/*
 * SCB Cubby support
 */

/**
 * @brief SCBQ's SCB Cubby initialization function
 *
 * SCBQ's SCB Cubby initialization function. The funciton is registerd via a call
 * to wlc_scb_cubby_reserve() by wlc_scbq_module_attach().
 * This function is responsible for initialzing the cubby space for SCBQ when a new
 * SCB is initialized.
 *
 * In addition to allocating and initializing the per-SCB cubby info, this function
 * activates the TXMOD_SCBQ Tx Module and creates the MUX entries in the MUX of the
 * TxQ associated with the SCB's bsscfg.
 */
static int
wlc_scbq_cubby_init(void *context, struct scb *scb)
{
	wlc_scbq_info_t *scbq_info = (wlc_scbq_info_t *)context;
	struct scbq_cubby *cubby = SCB_SCBQ_CUBBY_PTR(scbq_info, scb);
	scbq_cubby_info_t *scbq_cubby;
	wlc_mux_t *mux;
	wlc_txq_info_t *qi;
	int ac;
	int err;
	int i;

	/* Internal scbs get special queue attachemnts */
	if (SCB_INTERNAL(scb)) {
		return 0;
	}

	scbq_cubby = MALLOCZ(scbq_info->osh, sizeof(scbq_cubby_info_t));
	if (!scbq_cubby)
		return BCME_NOMEM;
	cubby->pinfo = scbq_cubby;

	scbq_cubby->scbq_info = scbq_info;
	scbq_cubby->scb = scb;

	/* pktq_init() sets the length for the entire multi precendence queue */
	pktq_init(&scbq_cubby->scb_txq, WLC_PREC_COUNT, PKTQ_LEN_MAX);

	/* Set the per-precedence queue lengths */
	for (i = 0; i < WLC_PREC_COUNT; i++) {
		pktq_set_max_plen(&scbq_cubby->scb_txq, i, SCBQ_LEN_MAX);
	}

	/* Init tx path to our queue */
	wlc_txmod_config(scbq_info->wlc, scb, TXMOD_SCBQ);

	/* Inherit the global flow control flags */
	scbq_cubby->fc = scbq_info->global_fc;

	/* create MUX input members for each AC */
	qi = scb->bsscfg->wlcif->qi;
	for (ac = 0; ac < AC_COUNT; ac++) {
		mux = qi->ac_mux[ac];
		scbq_cubby->mux[ac] = mux;
		scbq_cubby->flags[ac] = SCBQ_F_STALLED;
		err = wlc_mux_add_source(mux,
		                         scbq_cubby,
		                         wlc_scbq_output,
		                         &scbq_cubby->mux_hdl[ac]);

		if (err) {
			WL_ERROR(("wl%d:%s wlc_mux_add_member() failed err = %d\n",
			          scbq_info->unit, __FUNCTION__, err));
		}

		/* If flow control is non-zero (tx is stopped),
		 * then set the mux sources as stopped
		 */
		if (scbq_cubby->fc != 0) {
			wlc_mux_source_stop(mux, scbq_cubby->mux_hdl[ac]);
		}
	}

	/* Set the default packet overflow handler */
	wlc_scbq_register_overflow_fn(scbq_info, scb, wlc_scbq_drop_packet);

	return 0;
}

/**
 * @brief SCBQ's SCB Cubby de-initialization function
 *
 * SCBQ's SCB Cubby de-initialization function. This funciton is registerd via a call
 * to wlc_scb_cubby_reserve() by wlc_scbq_module_attach(). This funciton cleans up
 * the allocation and other work done by wlc_scbq_cubby_init().
 */
static void
wlc_scbq_cubby_deinit(void *context, struct scb *scb)
{
	wlc_scbq_info_t *scbq_info = (wlc_scbq_info_t *)context;
	struct scbq_cubby *cubby = SCB_SCBQ_CUBBY_PTR(scbq_info, scb);
	scbq_cubby_info_t *scbq_cubby = NULL;
	wlc_mux_t *mux;
	int ac;

	ASSERT(cubby);

	if (cubby)
		scbq_cubby = cubby->pinfo;
	if (!scbq_cubby)
		return;

	/* remove our MUX members */
	for (ac = 0; ac < AC_COUNT; ac++) {
		mux = scbq_cubby->mux[ac];
		wlc_mux_del_source(mux, scbq_cubby->mux_hdl[ac]);
	}

	wlc_scbq_scb_tx_flush(scbq_info, scb);
#ifdef PKTQ_LOG
	wlc_pktq_stats_free(scbq_info->wlc, &scbq_cubby->scb_txq);
#endif // endif
	MFREE(scbq_info->osh, scbq_cubby, sizeof(scbq_cubby_info_t));
	cubby->pinfo = NULL;
}

/**
 * @brief SCBQ's SCB Cubby tx flush function
 *
 * SCBQ's SCB Cubby tx flush function. This funciton is registerd via a call
 * to wlc_scb_cubby_reserve() by wlc_scbq_module_attach().  This funciton is responsible
 * for freeing any packets the SCBQ may be holding on behalf of the SCB.
 */
static void
wlc_scbq_scb_tx_flush(wlc_scbq_info_t *scbq_info, struct scb *scb)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);

	/* free all buffered tx packets */
	pktq_flush(scbq_info->osh, &scbq_cubby->scb_txq, TRUE, NULL, 0);
}

#ifdef BCMDBG
/**
 * @brief SCBQ's SCB Cubby state dump function
 *
 * SCBQ's SCB Cubby state dump function. This funciton is registerd via a call to
 * wlc_scb_cubby_reserve() by wlc_scbq_module_attach().  This function will dump the
 * current state and statistics of the SCBQ for an individual SCB. The dump
 * output appears as part of the SCB dump.
 */
static void
wlc_scbq_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_scbq_info_t *scbq_info = (wlc_scbq_info_t *)ctx;
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	struct pktq *q;
	uint prec, ac;
	int tot_len;

	if (scbq_cubby == NULL) {
		return;
	}

	q = &scbq_cubby->scb_txq;
	tot_len = pktq_len(q);

	/* print the total pkt count for all precs */
	bcm_bprintf(b, "     scbq: cubby %p tot qlen %d fc %04x\n",
	            scbq_cubby, tot_len, scbq_cubby->fc);

	for (ac = 0; ac < AC_COUNT; ac++) {
		bcm_bprintf(b, "           %d: mux_hdl %p flags %x mux %p\n",
		            ac, scbq_cubby->mux_hdl[ac],
		            scbq_cubby->flags[ac], scbq_cubby->mux[ac]);
	}

	if (tot_len == 0) {
		return;
	}

	/* print the per-prec pkt count for non-zero values */
	bcm_bprintf(b, "           prec len ");
	for (prec = 0; prec < q->num_prec; prec++) {
		uint plen = pktq_plen(q, prec);
		if (plen > 0) {
			bcm_bprintf(b, "%d:%u ", prec, plen);
		}
	}
	bcm_bprintf(b, "\n");
}
#endif /* BCMDBG */

/*
 * SCB TXMOD support
 */

/**
 * @brief SCBQ's Tx Module enqueue function
 *
 * SCBQ's Tx Module enqueue function. This funciton is registerd through the
 * @ref scbq_txmod_fns table via a call to wlc_txmod_fn_register() by
 * wlc_scbq_module_attach(). Data packets on the tx path will flow through
 * this fucntion.
 */
static void BCMFASTPATH
wlc_scbq_enq(void *ctx, struct scb *scb, void *pkt, uint prec)
{
	wlc_scbq_info_t *scbq_info = (wlc_scbq_info_t *)ctx;
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	struct pktq *q = &scbq_cubby->scb_txq;
	uint ac_idx;
#ifdef PKTQ_LOG
	pktq_counters_t* prec_cnt = NULL;
#endif // endif
#ifdef PKTQ_LOG
	/* Update pktq_log stats */
	if (q->pktqlog) {
		prec_cnt = q->pktqlog->_prec_cnt[prec];
		WLCNTCONDINCR(prec_cnt, prec_cnt->requested);
	}
#endif // endif
	/* Invoke overflow handler when SCBQ fills up */
	if (pktq_pfull(q, prec)) {
		scbq_cubby->packet_overflow_handler(scbq_info, scb, q, pkt, prec);
		WL_NONE(("wl%d: %s: scbq overflow, prec:%d.\n",
			scbq_info->wlc->pub->unit, __FUNCTION__, prec));
		return;
	}
#ifdef BCMDBG_POOL
	/* WES:XXX BCMDBG_POOL debug from wlc_txq_enq()
	 * looks like the PHDR logic should be wrapped into the PKTPOOLSETSTATE()
	 * instead of copy&paste
	 */
	if (WLPKTTAG(pkt)->flags & WLF_PHDR) {
		void *pdata;

		pdata = PKTNEXT(scbq_info->osh, pkt);
		ASSERT(pdata);
		ASSERT(WLPKTTAG(pdata)->flags & WLF_DATA);
		PKTPOOLSETSTATE(pdata, POOL_TXENQ);
	}
	else {
		PKTPOOLSETSTATE(pkt, POOL_TXENQ);
	}
#endif /* BCMDBG_POOL */

#ifdef PKTQ_LOG
	if (q->pktqlog) {
		prec_cnt = q->pktqlog->_prec_cnt[prec];
	}

	WLCNTCONDINCR(prec_cnt, prec_cnt->stored);

	if (prec_cnt) {
		uint32 qlen;
		uint32* max_used = &prec_cnt->max_used;

		qlen = pktq_plen(q, prec);
		if (qlen > *max_used) {
			*max_used = qlen;
		}
	}
#endif /* PKTQ_LOG */

	PKTDBG_TRACE(scbq_info->osh, pkt, PKTLIST_PRECQ);

	/*
	 * Enqueue the pkt on the SCB's txq
	 */
	(void)pktq_penq(q, prec, pkt);

	/* convert prec to ac fifo number, 4 precs per ac fifo */
	ac_idx = prec / (WLC_PREC_COUNT / AC_COUNT);

	/* if our mux source was stalled (previous output call returned 0 pkts), wake it up */
	if (scbq_cubby->flags[ac_idx] & SCBQ_F_STALLED) {
		wlc_mux_source_wake(scbq_cubby->mux[ac_idx], scbq_cubby->mux_hdl[ac_idx]);
		scbq_cubby->flags[ac_idx] &= ~SCBQ_F_STALLED;
	}
}

/**
 * @brief SCBQ's Tx Module packet count function
 *
 * SCBQ's Tx Module packet count function. This funciton is registerd through the
 * @ref scbq_txmod_fns table via a call to wlc_txmod_fn_register() by
 * wlc_scbq_module_attach(). This function is responsible for returning the count
 * of all tx packets held by the SCBQ's Tx Module.
 */
static uint
wlc_scbq_txpktcnt(void *ctx)
{
	wlc_scbq_info_t *scbq_info = (wlc_scbq_info_t *)ctx;
	struct scb *scb;
	struct scb_iter scbiter;
	scbq_cubby_info_t *scbq_cubby;
	int pktcnt = 0;

	FOREACHSCB(scbq_info->wlc->scbstate, &scbiter, scb) {
		scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
		if (scbq_cubby != NULL) {
			pktcnt += pktq_len(&scbq_cubby->scb_txq);
		}
	}

	return pktcnt;
}

/**
 * @brief SCBQ's Tx Module packet flush function
 *
 * SCBQ's Tx Module packet flush function. This funciton is registerd through the
 * @ref scbq_txmod_fns table via a call to wlc_txmod_fn_register() by
 * wlc_scbq_module_attach(). This function is responsible for freeing all tx packets
 * held by the SCBQ's Tx Module for the specified SCB.
 */
static void
wlc_scbq_txmod_flush(void *ctx, struct scb *scb)
{
	wlc_scbq_info_t *scbq_info = (wlc_scbq_info_t *)ctx;

	wlc_scbq_scb_tx_flush(scbq_info, scb);
}

/*
 * Tx MUX support
 */

static const uint16 fifo2prec_map[4] = {
	0x000F,  /* BK */
	0x00F0,  /* BE */
	0x0F00,  /* VI */
	0xF000,  /* VO */
};

/**
 * Set a Stop Flag to prevent tx output from all SCBs
 */
void
wlc_scbq_global_stop_flag_set(wlc_scbq_info_t *scbq_info, scbq_stop_flag_t flag)
{
	struct scb *scb;
	struct scb_iter scbiter;
	uint16 global_fc;

	global_fc = scbq_info->global_fc;

	if (global_fc & (uint16)flag) {
		return;
	}

	scbq_info->global_fc = global_fc | (uint16)flag;

	if (global_fc == 0) {
		FOREACHSCB(scbq_info->wlc->scbstate, &scbiter, scb) {
			wlc_scbq_scb_stop_flag_set(scbq_info, scb, flag);
		}
	}
}

/**
 * Clear a Stop Flag preventing tx output from all SCBs
 */
void
wlc_scbq_global_stop_flag_clear(wlc_scbq_info_t *scbq_info, scbq_stop_flag_t flag)
{
	struct scb *scb;
	struct scb_iter scbiter;
	uint16 global_fc;

	global_fc = scbq_info->global_fc;

	scbq_info->global_fc = global_fc & ~((uint16)flag);

	if (global_fc == (uint16)flag) {
		FOREACHSCB(scbq_info->wlc->scbstate, &scbiter, scb) {
			wlc_scbq_scb_stop_flag_clear(scbq_info, scb, flag);
		}
	}
}

/**
 * Set a Stop Flag to prevent tx output from the given scb
 */
void
wlc_scbq_scb_stop_flag_set(wlc_scbq_info_t *scbq_info, struct scb *scb, scbq_stop_flag_t flag)
{
	scbq_cubby_info_t *scbq_cubby;
	uint16 fc;

	scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	fc = scbq_cubby->fc;

	scbq_cubby->fc = fc | (uint16)flag;

	if (fc == 0) {
		wlc_scbq_scb_tx_stop(scbq_info, scb);
	}
}

/**
 * Clear a Stop Flag preventing tx output from the given scb
 */
void
wlc_scbq_scb_stop_flag_clear(wlc_scbq_info_t *scbq_info, struct scb *scb, scbq_stop_flag_t flag)
{
	scbq_cubby_info_t *scbq_cubby;
	uint16 fc;

	scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	fc = scbq_cubby->fc;

	scbq_cubby->fc = fc & ~((uint16)flag);

	if (fc == (uint16)flag) {
		wlc_scbq_scb_tx_start(scbq_info, scb);
	}
}

/**
 * Update the scbq state to indicate that the MUX Source is stalled for the specified AC.
 * When MUX Source is stalled, it will not be polled for output packets.
 * A stall happens when the SCBQ's MUX Source is polled for output and returns no data.
 * This function should be called by an SCBQ's MUX Source output fn when it returns no data
 * so that SCBQ code will wake the MUX Source as new packets are enqueued.
 *
 * This function should not be called as a form of flow control. Instead, the SCBQ fns
 * wlc_scbq_scb_tx_stop()/wlc_scbq_scb_tx_start() are used to flow control an SCBQ
 * Tx path.
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 * @param ac            Access Category that we need to mark as stalled.
 */
void
wlc_scbq_scb_stall_set(wlc_scbq_info_t *scbq_info, struct scb *scb, uint ac)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);

	scbq_cubby->flags[ac] |= SCBQ_F_STALLED;
}

/**
 * Clear a MUX Source stall condition for the given scb and AC.
 * When cleared the scb's MUX Source is polled for output packets.
 * The SCBQ code will automatically clear a stall condition as packets are enqueued
 * on the SCBQ.
 *
 * This function should be used when conditions change that allow packets to flow from an SCBQ's MUX
 * Source output fn.
 *
 * This function should not be called as a form of flow control. Instead, the SCBQ fns
 * wlc_scbq_scb_tx_stop()/wlc_scbq_scb_tx_start() are used to flow control an SCBQ
 * Tx path.
 *
 * @param scbq_info     A pointer to the state structure for the SCBQ module
 *                      created by wlc_scbq_module_attach().
 * @param scb           A pointer to the SCB of interest.
 * @param ac            Access Category for which we want to clear a stall condition.
 */
void
wlc_scbq_scb_stall_clear(wlc_scbq_info_t *scbq_info, struct scb *scb, uint ac)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);

	/* clear the stall condition if it is set */
	if (scbq_cubby->flags[ac] & SCBQ_F_STALLED) {
		wlc_mux_source_wake(scbq_cubby->mux[ac], scbq_cubby->mux_hdl[ac]);
		scbq_cubby->flags[ac] &= ~SCBQ_F_STALLED;
	}
}

/**
 * Stop tx output from the given scb
 */
static void
wlc_scbq_scb_tx_stop(wlc_scbq_info_t *scbq_info, struct scb *scb)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	int ac;

	for (ac = 0; ac < AC_COUNT; ac++) {
		wlc_mux_source_stop(scbq_cubby->mux[ac], scbq_cubby->mux_hdl[ac]);
	}
}

/**
 * Start tx output from the given scb
 */
static void
wlc_scbq_scb_tx_start(wlc_scbq_info_t *scbq_info, struct scb *scb)
{
	scbq_cubby_info_t *scbq_cubby = SCB_SCBQ_CUBBY_INFO(scbq_info, scb);
	int ac;

	for (ac = 0; ac < AC_COUNT; ac++) {
		wlc_mux_source_start(scbq_cubby->mux[ac], scbq_cubby->mux_hdl[ac]);
	}
}

/**
 * @brief SCBQ's Tx MUX output function
 *
 * SCBQ's Tx MUX output function. This funciton is registerd via a call to wlc_mux_add_member() by
 * wlc_scbq_cubby_init(). This function will supply the packets to the caller from packets held
 * in the SCBQ's per-SCB queues. The packets to output are collected in the provided 'output_q'.
 *
 * This function returns an estimate of the total TX time of the packets added to the output_q.
 * The supplied time may exceed the requested time.
 *
 * @param ctx           a pointer to the scbq_cubby_info_t
 * @param ac            Access Category traffic being requested
 * @param request_time  the total estimated TX time the caller is requesting from the SCB
 * @param output_q      pointer to a packet queue to use to store the output packets
 * @return              The estimated TX time of the packets returned in the output_q. The time
 *                      returned may be greater than the 'request_time'
 */
static uint BCMFASTPATH
wlc_scbq_output(void *ctx, uint ac, uint request_time, struct spktq *output_q)
{
	scbq_cubby_info_t *scbq_cubby = (scbq_cubby_info_t *)ctx;
	struct pktq *q = &scbq_cubby->scb_txq;
	struct scb *scb = scbq_cubby->scb;
	wlc_scbq_info_t *scbq_info = scbq_cubby->scbq_info;
	wlc_info_t *wlc = scbq_info->wlc;
	osl_t *osh = scbq_info->osh;
	wlc_pkttag_t *pkttag;
	ratespec_t rspec;
	uint16 prec_map;
	int prec;
	void *pkt[DOT11_MAXNUMFRAGS] = {0};
	int i, count;
	uint supplied_time = 0;
	timecalc_t timecalc;
	uint current_pktlen = 0;
	uint current_pkttime = 0;
#ifdef WLAMPDU_MAC
	bool check_epoch = TRUE;
#endif /* WLAMPDU_MAC */

	ASSERT(ac < AC_COUNT);

	/* use a prec_map that matches the AC fifo parameter */
	prec_map = fifo2prec_map[ac];

	WL_TMP(("%s: entry, CUBBY %p req %u ac %u pmap %x qlen %d\n", __FUNCTION__,
	        scbq_cubby, request_time, ac, prec_map, pktq_len(q)));

	/*
	 * create a pkt time caclculation cache to cut down on work for per-pkt time esitmate
	 */
	rspec = wlc_tx_current_ucast_rate(wlc, scb, ac);
	wlc_scbq_init_timecalc(scbq_cubby, rspec, ac, &timecalc);

	while (supplied_time < request_time && (pkt[0] = pktq_mdeq(q, prec_map, &prec))) {
		wlc_txh_info_t txh_info;
		uint pktlen;
		uint pkttime;

		pkttag = WLPKTTAG(pkt[0]);

		/* separate packet preparation and time calculation for
		 * MPDU (802.11 formatted packet with txparams), and
		 * MSDU (Ethernet or 802.3 stack packet)
		 */

		if ((pkttag->flags & WLF_MPDU) == 0) {
			/*
			 * MSDU packet prep
			 */

			int err;
			uint fifo; /* is this param needed anymore ? */
			err = wlc_prep_sdu(wlc, scb, pkt, &count, &fifo);
			/* WES:XXX Should the error be checked for memory error in the frag case?
			 * wlc_prep_sdu() is currently returning BCME_BUSY when it cannot allocate
			 * frags.
			 * Also returns BCME_ERROR on toss.
			 */
			if (err == BCME_OK) {
				if (count == 1) {
					/* optimization: skip the txtime calculation if the total
					 * pkt len is the same as the last time through the loop
					 */
					pktlen = pkttotlen(osh, pkt[0]);
					if (current_pktlen == pktlen) {
						pkttime = current_pkttime;
					} else {
						/* XXX Performance:
						 * expensive per pkt to get frame size
						 */
						wlc_get_txh_info(wlc, pkt[0], &txh_info);

						/* calculate and store the estimated pkt tx time */
						pkttime = wlc_scbq_timecalc(&timecalc,
						                            txh_info.d11FrameSize);

						current_pktlen = pktlen;
						current_pkttime = pkttime;
					}

					WLPKTTIME(pkt[0]) = (uint16)pkttime;
				} else {
					uint fragtime = 0;
					pkttime = 0;
					for (i = 0; i < count; i++) {
						/* XXX Performance:
						 * expensive per pkt to get frame size
						 */
						wlc_get_txh_info(wlc, pkt[i], &txh_info);

						/* calculate and store the estimated pkt tx time */
						fragtime = wlc_scbq_timecalc(&timecalc,
						                             txh_info.d11FrameSize);
						WLPKTTIME(pkt[i]) = (uint16)fragtime;
						pkttime += fragtime;
					}
				}
			} else if (err == BCME_ERROR) {
				/* BCME_ERROR indicates a tossed packet */

				/* pkt[] should be invalid and count zero */
				ASSERT(count == 0);

				/* let the code finish the loop adding no time
				 * for this dequeued packet, and enqueue nothing to
				 * output_q since count == 0
				 */
				pkttime = 0;
			} else {
				/* should be no other errors */
				ASSERT(err == BCME_OK);
				/* XXX, Well BUSY can still happen for frags, but should not
				 * run into that. Need to implement.
				 * BUSY for block_datafifo, but we check before this loop
				 * BUSY for SCB_ISMULTI(), but until we run AP, no SCB is multi
				 */
				PKTFREE(osh, pkt[0], TRUE);
				pkttime = 0;
				count = 0;
			}
		} else {
			/*
			 * MPDU packet prep
			 */

			uint fifo; /* is this param needed anymore ? */

			/* fetch the rspec saved in tx_prams at the head of the pkt
			 * before tx_params are removed by wlc_prep_pdu()
			 */
			rspec = wlc_pdu_txparams_rspec(osh, pkt[0]);

			count = 1;
			(void)wlc_prep_pdu(wlc, scb, pkt[0], &fifo);

			wlc_get_txh_info(wlc, pkt[0], &txh_info);

			/* calculate and store the estimated pkt tx time */
			pkttime = wlc_tx_mpdu_time(wlc, scb, rspec, ac, txh_info.d11FrameSize);
			WLPKTTIME(pkt[0]) = (uint16)pkttime;
		}

		supplied_time += pkttime;

		for (i = 0; i < count; i++) {
#ifdef WLAMPDU_MAC
			/* For AQM AMPDU Aggregation:
			 * If there is a transition from A-MPDU aggregation frames to a
			 * non-aggregation frame, the epoch needs to change. Otherwise the
			 * non-agg frames may get included in an A-MPDU.
			 */
			if (check_epoch && AMPDU_AQM_ENAB(wlc->pub)) {
				/* Once we check the condition, we don't need to check again since
				 * we are enqueuing an non_ampdu frame so wlc_ampdu_was_ampdu() will
				 * be false.
				 */
				check_epoch = FALSE;
				/* if the previous frame in the fifo was an ampdu mpdu,
				 * change the epoch
				 */
				if (wlc_ampdu_was_ampdu(wlc->ampdu_tx, ac)) {
					bool epoch;

					wlc_get_txh_info(wlc, pkt[i], &txh_info);
					epoch = wlc_ampdu_chgnsav_epoch(wlc->ampdu_tx,
					                                ac,
					                                AMU_EPOCH_CHG_MPDU,
					                                scb,
					                                (uint8)PKTPRIO(pkt[i]),
					                                &txh_info);
					wlc_txh_set_epoch(wlc, txh_info.tsoHdrPtr, epoch);
				}
			}
#endif /* WLAMPDU_MAC */

#ifdef WL11N
			{
				uint16 frameid;
				frameid = ltoh16(txh_info.TxFrameID);
				if (frameid & TXFID_RATE_PROBE_MASK) {
					wlc_scb_ratesel_probe_ready(wlc->wrsi, scb,
					                            frameid, FALSE, 0);
				}
			}
#endif // endif
			/* add this pkt to the output queue */
			pktenq(output_q, pkt[i]);
		}
	}

	if (supplied_time == 0) {
		scbq_cubby->flags[ac] |= SCBQ_F_STALLED;
	}

#ifdef WLAMPDU_MAC
	/* For Ucode/HW AMPDU Aggregation:
	 * If there are non-aggreation packets added to a fifo, make sure the epoch will
	 * change the next time entries are made to the aggregation info side-channel.
	 * Otherwise, the agg logic may include the non-aggreation packets into an A-AMPDU.
	 */
	if (pktq_len(output_q) > 0 && AMPDU_MAC_ENAB(wlc->pub) && !AMPDU_AQM_ENAB(wlc->pub)) {
		wlc_ampdu_change_epoch(wlc->ampdu_tx, ac, AMU_EPOCH_CHG_MPDU);
	}
#endif /* WLAMPDU_MAC */

	/* Return with output packets on 'output_q', and tx time estimate as return value */

	WL_TMP(("%s: exit supplied %dus, %u pkts\n", __FUNCTION__,
	        supplied_time, pktq_len(output_q)));

	return supplied_time;
}

/**
 * @brief Initialize a timecalc_t struct for fast packet TX time calculations
 *
 * Initialize a timecalc_t struct for fast packet TX time calculations.
 * This function will initialize a timecalc_t struct with parameters derived from the current
 * band and BSS, and the length independent fixed time overhead. The resulting timecalc_t may
 * be used by wlc_scbq_timecalc() to do a TX time estimate for a packet.
 *
 * @param scbq_cubby    a pointer to the scbq_cubby_info_t
 * @param rspec         the rate for all calculations
 * @param ac_idx        ACI for all calculations, used to estimate MAC access delay
 * @param tc            pointer to a timecalc_t struct
 */
static void
wlc_scbq_init_timecalc(scbq_cubby_info_t *scbq_cubby, ratespec_t rspec, uint ac,
                       timecalc_t *tc)
{
	wlc_scbq_info_t *scbq_info = scbq_cubby->scbq_info;
	wlc_info_t *wlc = scbq_info->wlc;
	struct scb* scb = scbq_cubby->scb;
	wlcband_t *band = wlc_scbband(scb);
	wlc_bsscfg_t *bsscfg;
	uint fixed_overhead_us;

	bsscfg = scb->bsscfg;

	fixed_overhead_us = wlc_tx_mpdu_frame_seq_overhead(rspec, bsscfg, band, ac);

	tc->rspec = rspec;
	tc->fixed_overhead_us = fixed_overhead_us;
	tc->is2g = BAND_2G(band->bandtype);

	/* calculate the preample type */
	if (RSPEC_ISLEGACY(rspec)) {
		/* For legacy reates calc the short/long preamble.
		 * Only applicable for 2, 5.5, and 11.
		 * Check the bss config and other overrides.
		 */

		uint mac_rate = (rspec & RATE_MASK);

		if ((mac_rate == WLC_RATE_2M ||
		     mac_rate == WLC_RATE_5M5 ||
		     mac_rate == WLC_RATE_11M) &&
		    WLC_PROT_CFG_SHORTPREAMBLE(wlc->prot, bsscfg) &&
		    (scb->flags & SCB_SHORTPREAMBLE) &&
		    (bsscfg->PLCPHdr_override != WLC_PLCP_LONG)) {
			tc->short_preamble = 1;
		} else {
			tc->short_preamble = 0;
		}
	} else {
		/* For VHT, always MM, for HT, assume MM and don't bother with Greenfield */
		tc->short_preamble = 0;
	}
}

/**
 * @brief Calculate a TX airtime estimate using a previously initialized timecalc_t struct
 *
 * Calculate a TX airtime estimate using a previously initialized timecalc_t struct.
 * This function uses an initialzed timecalc_t struct to calculate the TX airtime for a
 * packet with the given MPDU length.
 *
 * @param tc            pointer to a timecalc_t struct
 * @param mpdu_len      length of packet starting with 802.11 header and including the FCS
 *
 * @return              The estimated TX time of the packet
 */
uint
wlc_scbq_timecalc(timecalc_t *tc, uint mpdu_len)
{
	return (tc->fixed_overhead_us +
	        wlc_txtime(tc->rspec, (tc->is2g != 0), tc->short_preamble, mpdu_len));
}
#endif /* TXQ_MUX */
