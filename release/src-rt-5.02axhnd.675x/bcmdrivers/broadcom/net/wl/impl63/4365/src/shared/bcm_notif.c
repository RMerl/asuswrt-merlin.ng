/*
 * Implementation of event notification component.
 *
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
 * $Id$
 */

/* XXX: Define bcm_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <bcm_cfg.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include "bcm_notif_priv.h"
#include <bcm_notif_pub.h>
#include <bcm_mpool_pub.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif

#if defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG)

#if defined(ERR_USE_EVENT_LOG_RA)
#define	NOTIF_ERROR(args)	EVENT_LOG_RA(EVENT_LOG_TAG_NOTIF_ERROR, args)
#else
#define	NOTIF_ERROR(args)	EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_NOTIF_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */

#elif defined(BCMDBG_ERR) || defined(BCMDBG)
#define NOTIF_ERROR(args)	printf args
#else	/* BCMDBG */
#define NOTIF_ERROR(args)
#endif	/* BCMDBG */

static void dealloc_module(bcm_notif_module_t *notif_module);

/*
 * bcm_notif_attach()
 *
 * Notifier module attach-time initialization.
 *
 * Parameters:
 *    osh                Operating system handle.
 *    mpm                Memory pool manager handle.
 *    max_notif_servers  Maximum number of supported servers.
 *    max_notif_clients  Maximum number of supported clients.
 * Returns:
 *    Global notifier module object. NULL on error.
 */
static const char BCMATTACHDATA(rstr_notif_s)[] = "notif_s";
static const char BCMATTACHDATA(rstr_notif_c)[] = "notif_c";
bcm_notif_module_t* BCMATTACHFN(bcm_notif_attach)(osl_t *osh,
                                                  bcm_mpm_mgr_h mpm,
                                                  int max_notif_servers,
                                                  int max_notif_clients)
{
	bcm_notif_module_t *notif_module;
	int                ret;

#ifdef EVENT_LOG_COMPILE
	event_log_tag_start(EVENT_LOG_TAG_NOTIF_ERROR, EVENT_LOG_SET_ERROR,
		EVENT_LOG_TAG_FLAG_LOG|EVENT_LOG_TAG_FLAG_PRINT);
#endif // endif
	/* Allocate global notifier module state. */
	if ((notif_module = MALLOC(osh, sizeof(*notif_module))) == NULL) {
		NOTIF_ERROR(("%s: out of mem, malloced %d bytes\n",
		             __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}

	/* Init global notifier module state. */
	memset(notif_module, 0, sizeof(*notif_module));
	notif_module->osh = osh;
	notif_module->mpm = mpm;

	/* Create memory pool for server objects. */
	ret = bcm_mpm_create_prealloc_pool(mpm,
	                                   sizeof(struct bcm_notif_list_struct),
	                                   max_notif_servers, NULL, 0,
	                                   rstr_notif_s, &notif_module->server_mem_pool);
	if (ret != BCME_OK) {
		goto fail;
	}

	/* Create memory pool for client objects. */
	ret = bcm_mpm_create_prealloc_pool(mpm,
	                                   sizeof(struct bcm_notif_client_request),
	                                   max_notif_clients, NULL, 0,
	                                   rstr_notif_c, &notif_module->client_mem_pool);
	if (ret != BCME_OK) {
		goto fail;
	}

	/* Success. */
	return (notif_module);

fail:
	dealloc_module(notif_module);

	return (NULL);
}

/*
 * bcm_notif_detach()
 *
 * Notifier module detach-time deinitialization.
 *
 * Parameters:
 *    notif_module  Global notifier module object.
 * Returns:
 *    Nothing.
 */
void BCMATTACHFN(bcm_notif_detach)(bcm_notif_module_t *notif_module)
{
	dealloc_module(notif_module);
}

/*
 * dealloc_module()
 *
 * Helper function to perform common resource deallocation.
 *
 * Parameters:
 *    notif_module  Global notifier module object.
 * Returns:
 *    Nothing.
 */
static void BCMATTACHFN(dealloc_module)(bcm_notif_module_t *notif_module)
{
	if (notif_module != NULL) {

		if (notif_module->client_mem_pool != NULL) {
			bcm_mpm_delete_prealloc_pool(notif_module->mpm,
			                             &notif_module->client_mem_pool);
		}

		if (notif_module->server_mem_pool != NULL) {
			bcm_mpm_delete_prealloc_pool(notif_module->mpm,
			                             &notif_module->server_mem_pool);
		}

		MFREE(notif_module->osh, notif_module, sizeof(*notif_module));
	}
}

/*
 * bcm_notif_create_list()
 *
 * Initialize new list. Allocates memory.
 *
 * Parameters:
 *    notif_module  Global notifier module object.
 *    hdlp          Pointer to opaque list handle.
 * Returns:
 *    BCME_OK     Object initialized successfully. May be used.
 *    BCME_NOMEM  Initialization failed due to no memory. Object must not be used
 */
int bcm_notif_create_list(bcm_notif_module_t *notif_module, bcm_notif_h *hdlp)
{
	int result = BCME_OK;

	if ((*hdlp = bcm_mp_alloc(notif_module->server_mem_pool)) == NULL) {
		result = BCME_NOMEM;
	}
	else {
		memset(*hdlp, 0, sizeof(**hdlp));
		(*hdlp)->allow_list_operations = TRUE;
		(*hdlp)->notif_module = notif_module;
	}

	return (result);
}

/*
 * bcm_notif_add_interest()
 *
 * Add an interested client
 *
 * Parameters
 *    hdl        Opaque list handle.
 *    callback   Client callback routine
 *    passthru   Client pass-thru data
 * Returns:
 *    BCME_OK     Client interest added successfully
 *    BCME_NOMEM  Add failed due to no memory.
 *
 */
int bcm_notif_add_interest(bcm_notif_h hdl,
                           bcm_notif_client_callback callback,
                           bcm_notif_client_data passthru)
{
	int result = BCME_OK;
	struct bcm_notif_client_request *new_node = NULL;
	bcm_notif_module_t *notif_module = hdl->notif_module;

	/* Ensure that list operations are not performed within client callbacks. */
	if (!(hdl->allow_list_operations))
		return (BCME_BUSY);

	/* Allocate a new node for the new client interest request */
	if ((new_node = bcm_mp_alloc(notif_module->client_mem_pool)) == NULL) {
		result = BCME_NOMEM;
	}
	else {
		memset(new_node, 0, sizeof(*new_node));

		/* Initialize the new node with the client's information */
		new_node->callback = callback;
		new_node->passthru = passthru;
		/* Insert it at the tail of the linked list */
		if (hdl->tail == NULL) {
			/* This is now the first node in the list. */
			new_node->next = new_node;
			hdl->tail = new_node;
		}
		else {
			/* This is not the first node in the list. */
			/* Insert as new tail in circular list. */
			new_node->next = hdl->tail->next;
			hdl->tail->next = new_node;
			hdl->tail = new_node;
		}
	}

	return (result);
}

/*
 * bcm_notif_remove_interest()
 *
 * Remove an interested client. The callback and passthru must be identical to the data
 * supplied during registration.
 *
 * Parameters
 *    hdl        Opaque list handle.
 *    callback   Client callback routine
 *    passthru   Client pass-thru data
 * Returns:
 *    BCME_OK        Client interest added successfully
 *    BCME_NOTFOUND  Could not locate this specific client registration for removal.
 *
 */
int bcm_notif_remove_interest(bcm_notif_h hdl,
                              bcm_notif_client_callback callback,
                              bcm_notif_client_data passthru)
{
	int result = BCME_NOTFOUND;
	struct bcm_notif_client_request * nodep = hdl->tail;
	bcm_notif_module_t *notif_module = hdl->notif_module;

	/* Ensure that list operations are not performed within client callbacks. */
	if (!(hdl->allow_list_operations))
		return (BCME_BUSY);

	if (nodep && nodep == nodep->next) {
		/*
		 * Special case: list with just one node.
		 * If we get a hit, the list becomes empty because we delete the last node.
		 */
		if (nodep->callback == callback && nodep->passthru == passthru) {
			bcm_mp_free(notif_module->client_mem_pool, nodep);
			hdl->tail = NULL;
			result = BCME_OK;
		}
	} else {
		/*
		 * List with 0 or 2+ elements.
		 */
		struct bcm_notif_client_request * prevp;
		while (nodep) {
			prevp = nodep;
			nodep = nodep->next;
			if (nodep->callback == callback && nodep->passthru == passthru) {
				/*
				 * If we got a hit at the tail of the list, need to adjust
				 * the main list "tail" pointer.
				 */
				if (hdl->tail == nodep)
					hdl->tail = prevp;
				result = BCME_OK;
				prevp->next = nodep->next;
				bcm_mp_free(notif_module->client_mem_pool, nodep);
				/* Setting nodep to be 0 breaks us out of the loop. */
				nodep = 0;
			}
			/* We should not loop all the way around! */
			if (nodep == hdl->tail)
				break;
		}
	}

	return (result);
}

/* Notify all clients on an event list that the event has occured.
 * Invoke callback 'cb' if necessary upon every client callback's
 * invocation to allow the caller to process each client's data.
 */
static int
_bcm_notif_signal(bcm_notif_h hdl, bcm_notif_server_data data,
	bcm_notif_server_callback cb, bcm_notif_server_context arg)
{
	int result = BCME_OK;
	struct bcm_notif_client_request * nodep;

	if (!hdl)
		return result;

	/* Ensure that list operations are not performed within client callbacks. */
	if (!(hdl->allow_list_operations))
		return (BCME_BUSY);

	nodep = hdl->tail;

	if (nodep) {
		struct bcm_notif_client_request * firstp;
		nodep = nodep->next;
		firstp = nodep;

		/* Mark list to prevent against list operations within client callbacks. */
		hdl->allow_list_operations = FALSE;

		do {
			/* Signal the current client */
			nodep->callback(nodep->passthru, data);
			if (cb != NULL)
				cb(arg, data);
			/* Advance to next client registration */
			nodep = nodep->next;
		} while (nodep != firstp);

		/* Done client callbacks - allow list operations again. */
		hdl->allow_list_operations = TRUE;
	}

	return BCME_OK;
}

/*
 * bcm_notif_signal()
 *
 * Notify all clients on an event list that the event has occured. Invoke their
 * callbacks and provide both the server data and the client passthru data.
 *
 * Parameters
 *    hdl         Opaque list handle.
 *    server_data Server data for the notification
 * Returns:
 *    BCME_OK     Client interest added successfully
 *    BCME_BUSY   Recursion detected
 */
int bcm_notif_signal(bcm_notif_h hdl, bcm_notif_server_data data)
{
	return _bcm_notif_signal(hdl, data, NULL, NULL);
}

/*
 * bcm_notif_signal_ex()
 *
 * Notify all clients on an event list that the event has occured. Invoke their
 * callbacks and provide both the server data and the client passthru data.
 * Invoke server callback upon every client callback's invocation to allow
 * server to process each client's data.
 *
 * Parameters
 *    hdl         Opaque list handle.
 *    server_data Server data for the notification
 *    cb          Server callback
 *    arg         Context to server callback
 * Returns:
 *    BCME_OK     Client interest added successfully
 *    BCME_BUSY   Recursion detected
 */
int
bcm_notif_signal_ex(bcm_notif_h hdl, bcm_notif_server_data data,
	bcm_notif_server_callback cb, bcm_notif_server_context arg)
{
	return _bcm_notif_signal(hdl, data, cb, arg);
}

/*
 * bcm_notif_delete_list()
 *
 * Remove all the nodes and the list itself.
 *
 * Parameters
 *    hdlp       Pointer to opaque list handle.
 * Returns:
 *    BCME_OK     Event list successfully deleted.
 *    BCME_ERROR  General error
 */
int bcm_notif_delete_list(bcm_notif_h *hdl)
{
	/* First free all the nodes. */
	struct bcm_notif_client_request * nodep = (*hdl)->tail;
	struct bcm_notif_client_request * prevp;
	bcm_notif_module_t *notif_module = (*hdl)->notif_module;

	/* Ensure that list operations are not performed within client callbacks. */
	if (!((*hdl)->allow_list_operations))
		return (BCME_BUSY);

	while (nodep) {
		prevp = nodep;
		nodep = nodep->next;

		if (prevp == nodep) {
			/* Deleting the last node. Special case. */
			bcm_mp_free(notif_module->client_mem_pool, nodep);
			nodep = prevp = NULL;
		} else {
			prevp->next = nodep->next;
			bcm_mp_free(notif_module->client_mem_pool, nodep);
			nodep = prevp->next;
		}
	}

	/* Free the list itself in addition to the nodes. */
	memset(*hdl, 0, sizeof(**hdl));
	bcm_mp_free(notif_module->server_mem_pool, *hdl);
	*hdl = NULL;

	return (BCME_OK);
}

/*
 * bcm_notif_dump_list()
 *
 * For debugging, display interest list
 *
 * Parameters
 *    hdl        Opaque list handle.
 *    b          Output buffer.
 * Returns:
 *    BCME_OK     Event list successfully dumped.
 *    BCME_ERROR  General error.
 */
int bcm_notif_dump_list(bcm_notif_h hdl, struct bcmstrbuf *b)
{
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	struct bcm_notif_client_request * nodep;

	if (hdl == NULL)
		bcm_bprintf(b, "<uninit list>\n");
	else {
		nodep = hdl->tail;

		bcm_bprintf(b, "List id=0x%p: ", hdl);

		if (nodep == NULL) {
			bcm_bprintf(b, "(empty)");
		} else {
			/* List is not empty. Display all data in correct sequence. */
			struct bcm_notif_client_request * firstp = hdl->tail->next;
			nodep = nodep->next;
			do {
				bcm_bprintf(b, " [0x%p,0x%p]", nodep->callback,
				          nodep->passthru);
				nodep = nodep->next;
			} while (nodep != firstp);
		}
	}

	bcm_bprintf(b, "\n");
#endif   /* BCMDBG || BCMDBG_DUMP */

	return (BCME_OK);
}
