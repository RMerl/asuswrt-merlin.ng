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
 * $Id: wlc_mux.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief Transmit Path MUX layer for Broadcom 802.11 Networking Driver
 */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <siutils.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wl_dbg.h>

#include <wlc_mux.h>

typedef struct mux_source mux_source_t;

/**
 * @brief State structure for the MUX module created by wlc_mux_module_attach().
 *
 * The MUX module maintains state for all MUXs created. Common functionality for the
 * collection of all MUXs, such as IOVar control or debug dump functions, will be handled
 * by the MUX module.
 */
struct wlc_mux_info {
	osl_t *osh;                     /**< @brief OSL handle */
	wlc_info_t *wlc;                /**< @brief wlc_info_t handle */
	wlc_pub_t *pub;                 /**< @brief wlc_pub_t handle */
	uint unit;                      /**< @brief wl driver unit number for debug messages */
	struct wlc_mux *mux_list;       /**< @brief list of allocated @ref wlc_mux structures */
};

/**
 * @brief State structure for an individual MUX created by wlc_mux_alloc().
 *
 * The wlc_mux structure contains state pertaining to just one MUX. It will contain state
 * that can be configured per-MUX, information about attached sources, state of the
 * current output ordering for sources, etc.
 */
struct wlc_mux {
	osl_t *osh;                     /**< @brief OSL handle */
	wlc_mux_info_t* muxi;           /**< @brief MUX Module handle */
	struct wlc_mux *next;           /**< @brief next in list of MUX structures */
	uint sources;                   /**< @brief count of source members */
	uint active_sources;            /**< @brief count of source members on active list */

	/**
	 * @brief List of sources that will be polled for output data
	 *
	 * This is the list of input sources that will be polled for data when output data
	 * is requested from the MUX. To avoid unnecessary polling, sources that do not
	 * provide data when polled will be moved off the active_loop and onto the
	 * inactive loop.
	 */
	mux_source_t *active_loop;

	/**
	 * @brief List of sources that will not be polled for output data
	 *
	 * This is the list of input sources that have become inactive. These sources will
	 * not be polled for data when output data is requested from the MUX. A source
	 * needs to be "woken" by calling wlc_mux_source_wake() to move it to the active
	 * list.
	 */
	mux_source_t *inactive_loop;

	/**
	 * @brief Pointer to delimit the end of the early order list and beginning of
	 * normal order service list.
	 *
	 * As sources are moved from the inactive_loop to the active_loop, they are
	 * inserted at the head of the active_loop in a first-come first-served
	 * order. This pointer points to the first source past the end of the newly
	 * activated source. If any new activations happen before the head of the
	 * active_loop catches up with this pointer, the new activations will be inserted
	 * before this pointer.
	 */
	mux_source_t *next_normal_order_service;

	uint8 ac;                       /**< @brief Access Category of this MUX */
	uint8 flags;                    /**< @brief state flags for MUX */
	uint quanta;                    /**< @brief output service quanta for each source */
#ifdef CREDIT_BALANCE
	uint accumulation_limit;
#endif // endif
};

#ifdef PARTIAL_QUANTA
enum mux_flags {
	MUX_F_HEAD_IN_PROGRESS = 0x1
};
#endif // endif

/**
 * State structure for a MUX source created by wlc_mux_add_source(). The mux_source
 * structure contains state for a registered input source to a MUX. It has state for the
 * output function callback a particular source, current output credit/deficit,
 * etc. The mux_sources are maintained in double-linked lists.
 */
struct mux_source {

/* XXX DoubleLList pointers: This burns 2 pointers per mux source. Could reduce to to single byte
 * pointers if total # of unique sources is <= 256.
 * With 32b pointers, saves 6 bytes per mux source, costs 4 bytes for table storage, net 2
 * bytes saving over 2 pointers per mux.
 *
 * Could also compile for single link list if targeting few sources (few SCBs)
 */
	/* double linked list */
	mux_source_t *next;		/**< @brief Double-linked-list next ptr */
	mux_source_t *prev;		/**< @brief Double-linked-list prev ptr */

	uint8 flags;			/**< @brief state flags for mux source */
	int time_balance;		/**< @brief current usec time credit/deficit */

/* XXX output_fn: should be few of these. Possibly keep a table of fn pointers and
 * in each mux_source, keep only the index to the fn
 */
	mux_output_fn_t output_fn;	/**< @brief mux source's registered output packet fn */
	void *ctx;			/**< @brief context for output_fn */
};

enum source_flags {
	SOURCE_F_ACTIVE = 0x1,
	SOURCE_F_STOPPED = 0x2
};

static int mux_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
                       const char *name,
                       void *p, uint plen,
                       void *a, int alen,
                       int vsize,
                       struct wlc_if *wlcif);

static void mux_list_insert(wlc_mux_info_t *muxi, wlc_mux_t* mux);
static void mux_list_remove(wlc_mux_info_t *muxi, wlc_mux_t* mux);

static void mux_deactivate(wlc_mux_t *mux, mux_source_t *m);
static void mux_activate(wlc_mux_t *mux, mux_source_t *m);

/*
 * Double Linked List utils
 */
#ifdef NEED_INSERT_AFTER
static void mux_dlist_insert_after(mux_source_t *cur, mux_source_t *m);
#endif // endif
static void mux_dlist_insert_before(mux_source_t *cur, mux_source_t *m);
static void mux_dlist_insert(mux_source_t **list_head, mux_source_t *m);
static void mux_dlist_delete(mux_source_t **list_head, mux_source_t *m);

/*
 * Dump support
 */
#if defined(BCMDBG) || defined(WLDUMP)
static int wlc_mux_module_dump(wlc_mux_info_t *muxi, struct bcmstrbuf *b);
static void mux_dump(wlc_mux_t *mux, struct bcmstrbuf *b);
static void mux_loop_dump(mux_source_t *head, int count, struct bcmstrbuf *b);
static void mux_source_dump(mux_source_t *m, struct bcmstrbuf *b);
#endif /* defined(BCMDBG) || defined(WLDUMP) */

static const bcm_iovar_t mux_iovars[] = {
	{NULL, 0, 0, 0, 0 }
};

/* Module Attach/Detach */

/**
 * Create the MUX module infrastructure for the wl driver. wlc_module_register() is called
 * to register the module's handlers. The dump function for MUX state is also registered.
 *
 * @param wlc    driver wlc_info_t pointer
 * @return A wlc_mux_info_t structure, or NULL in case of failure.
 */
wlc_mux_info_t*
wlc_mux_module_attach(wlc_info_t *wlc)
{
	wlc_mux_info_t *muxi;
	osl_t *osh = wlc->osh;
	uint unit = wlc->pub->unit;
	int err;

	/* allocate the main state structure */
	muxi = MALLOCZ(osh, sizeof(wlc_mux_info_t));
	if (muxi == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			unit, __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}

	/* initialize cache of handles/info */
	muxi->wlc = wlc;
	muxi->pub = wlc->pub;
	muxi->osh = osh;
	muxi->unit = unit;

	err = wlc_module_register(muxi->pub,
	                    mux_iovars,
	                    "mux", muxi,
	                    mux_doiovar,
	                    NULL /* watchdog_fn_t watchdog_fn */,
	                    NULL /* up_fn_t up_fn */,
	                    NULL /* down_fn_t down_fn */);

	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(WLDUMP)
	wlc_dump_register(muxi->pub, "mux", (dump_fn_t)wlc_mux_module_dump, muxi);
#endif // endif

	return muxi;

fail:
	/* use detach as a common failure deallocation */
	wlc_mux_module_detach(muxi);

	return NULL;
}

/**
 * Free all resources assoicated with the MUX module infrastructure.
 * This is done at the cleanup stage when freeing the driver.
 *
 * @param muxi    MUX module infrastructe state structure
 */
void
wlc_mux_module_detach(wlc_mux_info_t *muxi)
{

	if (muxi == NULL) {
		return;
	}

	/* Free all mux structs on mux_list.
	 * Each time the head element of muxi->mux_list is freed with wlc_mux_free(),
	 * the mux_list pointer is updated to point to the remainder of the list.
	 */

	while (muxi->mux_list) {
		wlc_mux_free(muxi->mux_list);
	}

	wlc_module_unregister(muxi->pub, "mux", muxi);

	MFREE(muxi->osh, muxi, sizeof(wlc_mux_info_t));
}

#if defined(BCMDBG) || defined(WLDUMP)
/**
 * Dump a text detail of the MUX infrastructure and all MUX layer state into a text buffer.
 *
 * @param muxi    MUX module infrastructe state structure
 * @param b       a bcmstrbuf struct for formatted text output
 */
static int
wlc_mux_module_dump(wlc_mux_info_t *muxi, struct bcmstrbuf *b)
{
	wlc_mux_t *mux;

	for (mux = muxi->mux_list; mux != NULL; mux = mux->next) {
		bcm_bprintf(b, "MUX (%p):\n", mux);
		mux_dump(mux, b);

		bcm_bprintf(b, "\n");

	}

	return 0;
}
#endif /* BCMDBG || WLDUMP */

/**
 * IOVar handler for the MUX infrastructure module
 *
 * This handler is registered to the driver in wlc_module_register() to handle all
 * IOVar requests to any component of the MUX layer.
 *
 * @param hdl     pointer to the wlc_mux_info_t MUX module infrastructe state structure
 *
 * @return        A BCME error code.
 */
static int
mux_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
            const char *name,
            void *p, uint plen,
            void *a, int alen,
            int vsize,
            struct wlc_if *wlcif)
{

	/* when needed ...
	 * wlc_mux_info_t *muxi = hdl;
	 */
	int err = BCME_OK;

	switch (actionid) {
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/*
 * Round Robin alloc/free
 */

/**
 * Create a new MUX packet output scheduler. MUX packet sources for this MUX will be allocated
 * using wlc_mux_add_source().
 *
 * @param muxi    MUX module infrastructe state structure
 * @param ac      Access Category for this MUX
 * @param quanta  microseconds, quanta for output allocated to each MUX source
 *
 * @return A pointer to a new wlc_mux_t, or NULL on failure.
 */
wlc_mux_t*
wlc_mux_alloc(wlc_mux_info_t *muxi, uint ac, uint quanta)
{
	wlc_mux_t *mux;
	osl_t *osh = muxi->osh;

	/* allocate the main state structure */
	mux = MALLOCZ(osh, sizeof(wlc_mux_t));
	if (mux == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			muxi->unit, __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}
	mux->osh = osh;
	mux->muxi = muxi;
	mux->ac = (uint8)ac;
	mux->quanta = quanta;
#ifdef CREDIT_BALANCE
	mux->accumulation_limit = 4 * quanta; /* default the limit to 4x quanta */
#endif // endif
	mux_list_insert(muxi, mux);

	return mux;

fail:
	return NULL;
}

/**
 * Free a MUX created earlier with wlc_mux_alloc(). The wlc_mux_t structure and all resources, other
 * than MUX sources, are freed. Each source that was added to the MUX (wlc_mux_add_source()) must
 * be deleted (wlc_mux_del_source()) before calling wlc_mux_free().
 *
 * @param mux     MUX state structure
 */
void
wlc_mux_free(wlc_mux_t *mux)
{
	if (mux == NULL) {
		return;
	}

	/* All the sources must be deleted before calling wlc_mux_free().
	 * This needs to be done because the pointers to the MUX sources and MUX itself
	 * are held by code modules outside of the MUX. The outside modules need to delete
	 * the sources before deleting the MUX so that they do not make subsequent
	 * wlc_mux_del_source() calls with a deallocated wlc_mux_t pointer.
	 */
	ASSERT(mux->sources == 0);

	/* remove this struct pointer from the MUX infrastructre */
	mux_list_remove(mux->muxi, mux);

	MFREE(mux->osh, mux, sizeof(wlc_mux_t));
}

/**
 * Insert a wlc_mux_t into the list of MUXs in the MUX infrastructure.
 *
 * @param muxi    MUX module infrastructe state structure
 * @param mux     MUX state structure to add to the infrastructure
 */
static void
mux_list_insert(wlc_mux_info_t *muxi, wlc_mux_t* mux)
{
	ASSERT(mux->next == NULL);

	/* insert mux at head of mux_list */
	mux->next = muxi->mux_list;
	muxi->mux_list = mux;
}

/**
 * Remove a wlc_mux_t from the list of MUXs in the MUX infrastructure.
 *
 * @param muxi    MUX module infrastructe state structure
 * @param mux     MUX state structure to delete from the infrastructure
 */
static void
mux_list_remove(wlc_mux_info_t *muxi, wlc_mux_t* mux)
{
	ASSERT(muxi->mux_list != NULL);

	/* remove the mux from the module mux_info list */

	if (mux == muxi->mux_list) {
		/* mux is the head of the list, set head to next mux */
		muxi->mux_list = mux->next;
	} else {
		wlc_mux_t* prev;
		wlc_mux_t* p;

		/* walk the module mux_info list of muxes to find and remove this mux */
		for (prev = muxi->mux_list, p = prev->next;
		     p != NULL;
		     prev = p, p = p->next) {
			if (p != mux)
				continue;
			/* we found the match */
			/* remove from the list by setting prev pointer to p->next */
			prev->next = p->next;
			break;
		}

		/* we should have found the element in the list */
		ASSERT(p != NULL);
	}

	/* clean up the next pointer on the removed wlc_mux_t since mux is no longer on the list */
	mux->next = NULL;
}

/*
 * Configuration
 */

int
wlc_mux_set_quanta(wlc_mux_t *mux, uint quanta)
{
	mux->quanta = quanta;

	return BCME_OK;
}

uint
wlc_mux_get_quanta(wlc_mux_t *mux)
{
	return mux->quanta;
}

/*
 * Output
 */

/**
 * Request output packets from a MUX.
 *
 * This function will request output from the MUX sources to satisfy the 'request_time'
 * request, possibly gathering output from several sources. The packets gathered are
 * collected in the provided 'output_q'.
 *
 * The supplied TX time estimate of the packets in the output_q is returned by this funciton.
 * The supplied time may exceed the requested time.
 *
 * @param mux		pointer to the MUX structure
 * @param request_time	the total estimated TX time the caller is requesting from the MUX
 * @param output_q	pointer to a packet queue that this function will store the output packets
 *
 * @return		The estimated TX time of the packets returned in the output_q. The time
 *			returned may be greater than the 'request_time'.
 */
uint
wlc_mux_output(wlc_mux_t *mux, uint request_time, struct spktq *output_q)
{
	uint supplied_time = 0;
	uint quanta = mux->quanta;
	mux_source_t *src, *next;
	uint inc;
	int bal;
	int stalled = FALSE;

	/* WES: Note: could use double or higher factor on quanta if we knew that
	 * the epoc time, time through a round (sources * quanta), was a fraction of
	 * the requested time for output (multiple epoc times to add to requested_time).
	 * This could save on individual requests to the source members but still allow each
	 * to have a slice of time in the current request.
	 *  epoc_time = sources * quanta;
	 *  if (request_time >= 2 * epoc_time) {
	 *      quanta = quanta * (request_time / epoc_time);
	 *  }
	 * But if quanta is already a good amount of time, say 2ms, then the overhead of
	 * a single ask to dequeue from the source should be amortized over 2ms of data
	 * when loaded, and when unloaded, it does not matter how much time is requested,
	 * you only get the few pkts that have accumulated.
	 *
	 * So skipping this optimization work assuming that quanta will be a significant
	 * amount of time.
	 */

#ifdef PARTIAL_QUANTA
	/* handle a partial drain from the last output event */
	if (mux->flags & MUX_F_HEAD_IN_PROGRESS) {

		/* clear the in_prog flag since we are completing this source drain */
		mux->flags &= ~MUX_F_HEAD_IN_PROGRESS;

		src = mux->active_loop;

		/* find the source's remaining time balance from last round */
		bal = src->time_balance;

		/* request data from src */
		inc = (src->output_fn)(src->ctx, mux->ac, bal, output_q);

		supplied_time += inc;
		bal -= inc;

		/* adjust the credit/deficit for the source */
		src->time_balance = bal;

		/* move to the next src */
		mux->active_loop = src->next;
	}
#endif /* PARTIAL_QUANTA */

	/* init the starting point */
	src = mux->active_loop;

	/* exit on simple empty conditions */
	if (src == NULL) {
		WL_NONE(("%s: MUX %p req %d, exit with no active sources\n",
		         __FUNCTION__, mux, request_time));
		return 0;
	}

	/* walk the active src loop while there is still time to fill */
	while (supplied_time < request_time) {

		WL_NONE(("%s: MUX %p req %d sup %d src %p\n", __FUNCTION__,
		         mux, request_time, supplied_time, src));
		/* calculate how much time to ask for this source */
		if (src->next == src) {
			/* Since there is only one source, just ask the one source for the
			 * requested time. This will wipe out any debt.
			 */
			bal = request_time;
			WL_NONE(("%s: singleton src, bal %d\n", __FUNCTION__, bal));
		} else {
			/* find the source's new time balance after adding a quanta */
			bal = src->time_balance + quanta;
			WL_NONE(("%s: src->bal %d qant %d bal %d\n",
			         __FUNCTION__, src->time_balance, quanta, bal));
		}

		/* XXX bal needs to be adjusted by (request_time - supplied_time) if we
		 * want to slice quanta exactly to the request_time.
		 * If request to src is cut short by (request_time - supplied_time)
		 * limiting, mark current src as 'in_prog' if the actual 'inc' time supplied
		 * by src is less than the full quanta.  Alternative is to just always ask
		 * for a quanta even if it takes us over the request_time (resulting in an
		 * overfill of TxQ), or always stop if quanta would take us over
		 * request_time (resulting in less than high-watermark filling of TxQ.
		 */

		/* if the balance is positive, ask for some data */
		if (bal > 0) {

			/* if the src is stopped, do not ask for any output, and let the
			 * src deactivate below as in the stalled case
			 */
			if (src->flags & SOURCE_F_STOPPED) {
				inc = 0;
			} else {
				/* request data from src */
				inc = (src->output_fn)(src->ctx, mux->ac, bal, output_q);
			}

			/* if 'src' was already drained or stopped, remove from the active loop */
			if (inc == 0) {
				if (src->flags & SOURCE_F_STOPPED) {
					WL_NONE(("%s: src stopped, deactivate\n", __FUNCTION__));
				} else {
					WL_NONE(("%s: src output 0, deactivate\n", __FUNCTION__));
				}

				stalled = TRUE;

				bal = 0;
			} else {
				uint tmp = supplied_time + inc;
				WL_NONE(("%s: output %d\n", __FUNCTION__, inc));
				/* detect wrap on uint and clamp to max int */
				if (tmp >= supplied_time) {
					/* sum did not wrap, so save */
					supplied_time = tmp;
				} else {
					/* sum wrapped, so clamp to UINT_MAX */
					supplied_time = (uint)-1; /* UINT_MAX */
				}
				bal -= inc;
#if !defined(CREDIT_BALANCE)
				/* sources only carry a debit, not a credit
				 * adjust the balance to zero if positive
				 */
				if (bal > 0) {
					bal = 0;
				}
#endif // endif
			}
		}

#ifdef CREDIT_BALANCE
		/* clamp the credit at the configured limit to avoid latency glitches */
		if (bal > mux->accumulation_limit) {
			bal = mux->accumulation_limit;
		}
#endif // endif
		/* adjust the credit/deficit for the source */
		src->time_balance = bal;

		/* save 'next' in case src is removed from the loop (clearing src->next) */
		next = src->next;

		/* The next pointer is assumed to be the next in the active_loop.
		 * We have a logic problem if something above has moved 'src' to the inactive_loop
		 */
		ASSERT(src->flags & SOURCE_F_ACTIVE);

		/* remove the src from the active list if it stalled */
		if (stalled) {
			stalled = FALSE; /* reset for next iteration */

			/* move the stalled src to inactive list */
			mux_deactivate(mux, src);

			/* Exit if we drained all source traffic.
			 * If (src->next == src) before the call to mux_deactivate() above,
			 * then 'src' was the last source in the active list
			 */
			if (next == src) {
				/* the active list must be empty */
				break;
			}
		}

		/* move to the next src */
		src = next;

		/* if there is an early insert queue, check if we just finished servicing
		 * all its sources.
		 */
		if (mux->next_normal_order_service == src) {
			mux->next_normal_order_service = NULL;
		}
	}

	/* update the starting point of the active loop for next service */
	if (mux->active_loop != NULL) {
		/* we are about update the beginning of the active_loop,
		 * ASSERT that the src is a member of the active_loop.
		 */
		ASSERT(src->flags & SOURCE_F_ACTIVE);
		mux->active_loop = src;
	}

	WL_NONE(("%s: exit, supplied %d\n", __FUNCTION__, supplied_time));

	return supplied_time;
}

/**
 * A source will be marked as "stalled" if it's ouput fuction ever returns no data. This
 * function will mark the source as no longer stalled so that it will be polled for output
 * data.
 *
 * @see wlc_mux_source_start()
 * @see wlc_mux_source_stop()
 */
void
wlc_mux_source_wake(wlc_mux_t *mux, mux_source_handle_t hdl)
{
	mux_source_t *m = (mux_source_t*)hdl;

	if (m != NULL) {
		/* Move the the source to the active loop */
		mux_activate(mux, m);
	} else {
		ASSERT(m != NULL);
	}
}

/**
 * Change the output state of the source to active. Sources must be active to be polled for
 * output data. Calling wlc_mux_source_start() will also clear any stalled indication just
 * as wlc_mux_source_wake() does.
 *
 * @see wlc_mux_source_stop()
 * @see wlc_mux_source_wake()
 */
void
wlc_mux_source_start(wlc_mux_t *mux, mux_source_handle_t hdl)
{
	mux_source_t *m = (mux_source_t*)hdl;

	if (m != NULL) {
		/* The source is no longer stopped */
		m->flags &= ~SOURCE_F_STOPPED;

		/* Make sure the the source is on the active loop */
		mux_activate(mux, m);
	} else {
		ASSERT(m != NULL);
	}
}

/**
 * Change the output state of the source to inactive. Sources that are inactive will not
 * be polled for output data. wlc_mux_source_start() must be called on a source to make
 * them active again.
 *
 * @see wlc_mux_source_start()
 * @see wlc_mux_source_wake()
 */
void
wlc_mux_source_stop(wlc_mux_t *mux, mux_source_handle_t hdl)
{
	mux_source_t *m = (mux_source_t*)hdl;

	if (m != NULL) {
		/* Mark the source as stopped */
		m->flags |= SOURCE_F_STOPPED;
	} else {
		ASSERT(m != NULL);
	}
}

static void
mux_deactivate(wlc_mux_t *mux, mux_source_t *m)
{
	/* deactivate only if marked active */
	if (m->flags & SOURCE_F_ACTIVE) {
		ASSERT(mux->inactive_loop != m);

		m->flags &= ~SOURCE_F_ACTIVE;

		/* clean up flags/pointers to active loop */

#ifdef PARTIAL_QUANTA
		/* if this source is at the head and marked as in_progress,
		 * clear the in_progress flag since we are removing
		 */
		if (mux->active_loop == m && (mux->flags & MUX_F_HEAD_IN_PROGRESS)) {
			mux->flags &= ~MUX_F_HEAD_IN_PROGRESS;
		}
#endif // endif
		/* if this source is end of the early insertion queue,
		 * update the end pointer
		 */
		if (m == mux->next_normal_order_service) {
			/* if there are more sources in the active list,
			 * point to the next in line.
			 * Otherwise, clear the end pointer.
			 */
			if (m->next != m) {
				mux->next_normal_order_service = m->next;
			} else {
				mux->next_normal_order_service = NULL;
			}
		}

		mux_dlist_delete(&mux->active_loop, m);
		mux_dlist_insert(&mux->inactive_loop, m);
		mux->active_sources--;

		ASSERT(mux->active_loop != m);
	} else {
		ASSERT(mux->active_loop != m);
	}
}

static void
mux_activate(wlc_mux_t *mux, mux_source_t *m)
{
	/* activate only if not already active */
	if (m->flags & SOURCE_F_ACTIVE) {
		ASSERT(mux->inactive_loop != m);
		return;
	}
	ASSERT(mux->active_loop != m);

	mux_dlist_delete(&mux->inactive_loop, m);
	ASSERT(mux->inactive_loop != m);

	/* if there is an early insertion queue, add to the end, otherwise just add to the
	 * head of the active list
	 */
	if (mux->next_normal_order_service != NULL) {
		mux_dlist_insert_before(mux->next_normal_order_service, m);
	} else {
		mux_dlist_insert(&mux->active_loop, m);
		/* this source is end of the new early insertion list, so init
		 * next_normal_order_service to the source after us. This could
		 * be a pointer to us if the active list was empty before our
		 * insertion.
		 */
		mux->next_normal_order_service = m->next;
	}

	m->flags |= SOURCE_F_ACTIVE;
	mux->active_sources++;
}

#ifdef NEED_INSERT_AFTER
static void
mux_dlist_insert_after(mux_source_t *cur, mux_source_t *m)
{
	mux_source_t *next;

	m->next = next = cur->next;
	m->prev = cur;

	cur->next = m;
	next->prev = m;
}
#endif // endif

static void
mux_dlist_insert_before(mux_source_t *cur, mux_source_t *m)
{
	mux_source_t *prev;

	m->next = cur;
	m->prev = prev = cur->prev;

	cur->prev = m;
	prev->next = m;
}

static void
mux_dlist_insert(mux_source_t **list_head, mux_source_t *m)
{
	if (*list_head == NULL) {
		m->next = m->prev = m;
	} else {
		ASSERT(*list_head != m);

		mux_dlist_insert_before(*list_head, m);
	}

	*list_head = m;
}

static void
mux_dlist_delete(mux_source_t **list_head, mux_source_t *m)
{
	mux_source_t *next;
	mux_source_t *prev;

	next = m->next;
	prev = m->prev;
	m->next = m->prev = NULL;

	if (next == m) {
		*list_head = NULL;
		return;
	}

	next->prev = prev;
	prev->next = next;

	if (*list_head == m) {
		*list_head = next;
	}
}

/* Registration */

/**
 * Add a new source to a MUX.
 * The source's initial state will be enabled and stalled.
 *
 * @param mux           Pointer to the MUX structure
 * @param ctx           Context pointer to be provided as a parameter to the output_fn
 * @param output_fn     The mux_output_fn_t the MUX will call to request packets from
 *                      this mux source
 * @param phdl          A return parameter in which this call will provide the mux_source_handle_t
 *                      to be used in other MUX source function calls.
 * @return		An error code. BCME_OK on success.
 *
 * @see wlc_mux_source_start()
 * @see wlc_mux_source_stop()
 * @see wlc_mux_source_wake()
 */
int
wlc_mux_add_source(wlc_mux_t *mux, void *ctx, mux_output_fn_t output_fn, mux_source_handle_t *phdl)
{
	mux_source_t *m;

	/* allocate the source state structure */
	m = MALLOCZ(mux->osh, sizeof(mux_source_t));
	if (m == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			mux->muxi->unit, __FUNCTION__, MALLOCED(mux->osh)));
		return BCME_NOMEM;
	}
	m->output_fn = output_fn;
	m->ctx = ctx;

	/* add the source into the mux state */
	mux_dlist_insert(&mux->inactive_loop, m);
	mux->sources++;

	*phdl = m;

	return BCME_OK;
}

/**
 * Delete a source from a MUX.
 *
 * @param mux           Pointer to the MUX structure.
 * @param hdl           The mux_source_handle_t that was returned by wlc_mux_add_source().
 */
void
wlc_mux_del_source(wlc_mux_t *mux, mux_source_handle_t hdl)
{
	mux_source_t *m = (mux_source_t*)hdl;

	if (m == NULL) {
		return;
	}

	/* remove the source from the mux state */

	/* deactivte first, which will move the source to the inactive_loop */
	mux_deactivate(mux, m);

	/* now remove from the inactive list */
	mux_dlist_delete(&mux->inactive_loop, m);

	mux->sources--;

	/* free up the source state */
	MFREE(mux->osh, m, sizeof(mux_source_t));
}

/**
 * Update the output fn for a MUX source
 *
 * @param mux           Pointer to the MUX structure.
 * @param hdl           The mux_source_handle_t that was returned by wlc_mux_add_source().
 * @param ctx           Context pointer to be provided as a parameter to the output_fn
 * @param output_fn     The mux_output_fn_t the MUX will call to request packets from
 *                      this mux source
 */
void
wlc_mux_source_set_output_fn(wlc_mux_t *mux, mux_source_handle_t hdl,
                             void *ctx, mux_output_fn_t output_fn)
{
	mux_source_t *m = (mux_source_t*)hdl;

	m->output_fn = output_fn;
	m->ctx = ctx;
}

#if defined(BCMDBG) || defined(WLDUMP)
/**
 * Dump a text detail of state in a single MUX created by wlc_mux_alloc() into a text buffer.
 *
 * @param mux     MUX state structure
 * @param b       a bcmstrbuf struct for formatted text output
 */
static void
mux_dump(wlc_mux_t *mux, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "next -> %p:\n", mux->next);
	bcm_bprintf(b, "sources %u (active %u)\n", mux->sources, mux->active_sources);
	bcm_bprintf(b, "ac %u quanta %uus flags 0x%x\n", mux->ac, mux->quanta, mux->flags);

	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "Active Loop: head %p\n", mux->active_loop, b);
	mux_loop_dump(mux->active_loop, mux->active_sources, b);

	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "Normal Order Pointer %p\n", mux->next_normal_order_service);

	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "Inactive Loop: head %p\n", mux->inactive_loop);
	mux_loop_dump(mux->inactive_loop, mux->sources - mux->active_sources, b);
}

/**
 * Dump a text detail of the MUX sources on a MUX source list into a text buffer.
 *
 * @param head    pointer to a MUX source at the head of a list
 * @param count   number of MUX sources on the list
 * @param b       a bcmstrbuf struct for formatted text output
 */
static void
mux_loop_dump(mux_source_t *head, int count, struct bcmstrbuf *b)
{
	mux_source_t *m = head;
	int i = 0;

	while (m != NULL) {
		mux_source_dump(m, b);
		i++;

		if (m->next == NULL || m->prev == NULL) {
			bcm_bprintf(b, "ERR: NULL link pointer\n");
		} else {
			if (m->prev->next != m) {
				bcm_bprintf(b, "Prev (%p) next ptr (%p) does not match cur (%p)\n",
				            m->prev, m->prev->next, m);
			}
			if (m->next->prev != m) {
				bcm_bprintf(b, "Next (%p) prev ptr (%p) does not match cur (%p)\n",
				            m->next, m->next->prev, m);
			}
		}

		m = m->next;

		if (i == count) {
			if (m != head) {
				bcm_bprintf(b, "ERR: count %d but not back at head pointer %p\n",
				            count, head);
			}

			break;
		}

		if (m == head) {
			if (i != count) {
				bcm_bprintf(b, "ERR: back at head pointer %p, but count %d did not "
				            "reach expected\n",
				            head, i, count);
			}
			break;
		}

		bcm_bprintf(b, "\n");
	}
}

/**
 * Dump a text detail of an individual MUX source into a text buffer.
 *
 * @param m    pointer to a MUX source
 * @param b    a bcmstrbuf struct for formatted text output
 */
static void
mux_source_dump(mux_source_t *m, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "ptr %p flags 0x%x time_bal %d\n",
	            m, m->flags, m->time_balance);
	bcm_bprintf(b, "output_fn %p ctx %p\n", m->output_fn, m->ctx);
	bcm_bprintf(b, "next -> 0x%p:\n", m->next);
	bcm_bprintf(b, "prev -> 0x%p:\n", m->prev);
}
#endif /* BCMDBG || WLDUMP */

#ifdef UNIT_TEST
/******************************************************************************
 */
#include <stdlib.h>
extern int osl_debug_memdump(osl_t *osh, struct bcmstrbuf *buf);
#define MALLOC_DUMP(o, b) osl_debug_memdump(o, b)
uint wl_msg_level = 1;

int
wlc_module_register(wlc_pub_t *pub, const bcm_iovar_t *iovars,
                               const char *name, void *hdl, iovar_fn_t iovar_fn,
                               watchdog_fn_t watchdog_fn, up_fn_t up_fn, down_fn_t down_fn)
{
	return BCME_OK;
}

int
wlc_module_unregister(wlc_pub_t *pub, const char *name, void *hdl)
{
	return BCME_OK;
}

uint
test_out(void *ctx, uint ac, uint request_time, struct spktq *output_q)
{
	return 0;
}

int
main(int argc, char** argv)
{
	wlc_mux_info_t* mi;
	wlc_mux_t *mux0, *mux1, *mux2, *mux3;
	mux_source_handle_t m0, m1, m2, m3;
	int err;
	wlc_info_t wlc;
	wlc_pub_t pub;
	int char_buf_size = 128*1024;
	char *char_buf = malloc(char_buf_size);
	struct bcmstrbuf b;
	char* ctx0 = "context";

	wlc.osh = osl_attach();
	wlc.pub = &pub;
	wlc.pub->unit = 1;

	/* create the main module */
	mi = wlc_mux_module_attach(&wlc);

	/* create 4 muxes */
	mux0 = wlc_mux_alloc(mi, 0, 2000);
	mux1 = wlc_mux_alloc(mi, 1, 2000);
	mux2 = wlc_mux_alloc(mi, 2, 1000);
	mux3 = wlc_mux_alloc(mi, 3, 1000);

	printf("*** 4 muxes allocated\n\n");

	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	wlc_mux_free(mux0);
	wlc_mux_free(mux1);

	printf("*** 2 muxes freed\n\n");
	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	wlc_mux_free(mux3);

	/* Add some sources */
	err = wlc_mux_add_source(mux2, ctx0, &test_out, &m0);
	if (err)
		printf("line %d: ERR from wlc_mux_add_source() %d!\n", __LINE__, err);

	err = wlc_mux_add_source(mux2, ctx0, &test_out, &m1);
	if (err)
		printf("line %d: ERR from wlc_mux_add_source() %d!\n", __LINE__, err);

	err = wlc_mux_add_source(mux2, ctx0, &test_out, &m2);
	if (err)
		printf("line %d: ERR from wlc_mux_add_source() %d!\n", __LINE__, err);

	err = wlc_mux_add_source(mux2, ctx0, &test_out, &m3);
	if (err)
		printf("line %d: ERR from wlc_mux_add_source() %d!\n", __LINE__, err);

	printf("*** Added Some Sources\n\n");
	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	wlc_mux_source_start(mux2, m0);
	wlc_mux_source_start(mux2, m1);

	printf("*** Activate Some Sources\n\n");
	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	wlc_mux_source_stop(mux2, m1);
	wlc_mux_source_start(mux2, m2);
	wlc_mux_del_source(mux2, m0);

	printf("*** m1 stop, m2 start, m0 del\n\n");
	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	wlc_mux_del_source(mux2, m1);
	wlc_mux_del_source(mux2, m2);
	wlc_mux_del_source(mux2, m3);

	printf("*** all sources freed\n\n");
	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	wlc_mux_free(mux2);

	printf("*** all muxes freed\n\n");
	bcm_binit(&b, char_buf, char_buf_size);
	wlc_mux_module_dump(mi, &b);
	puts(char_buf);

	/* free the main module */
	wlc_mux_module_detach(mi);

	/* get the memory stats */
	bcm_binit(&b, char_buf, char_buf_size);
	MALLOC_DUMP(wlc.osh, &b);
	puts(char_buf);

	free(char_buf);

	return 0;
}

#endif /* UNIT_TEST */
