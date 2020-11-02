/*
 * Public header file for event-component data structure and API
 *
 * This library provides a data structure and behavior for registering
 * interest in events and getting callbacks. The fundamental data structure
 * is an opaque type that servers instantiate. Servers then use function calls
 * to let clients express interest and register callback function pointers.
 * Servers can signal events and provide server-specific data on the event.
 *
 * The library guarantees that client callbacks occur in the same order that the
 * clients registered interest.
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

#ifndef BCM_NOTIF_PUB_H
#define BCM_NOTIF_PUB_H 1

#include <osl.h>
#include <bcm_mpool_pub.h>

/*
*****************************************************************************
* Opaque type definitions for Event components.
*****************************************************************************
*/

/*
 * The opaque type for global notifier state.
 */
typedef struct bcm_notif_module bcm_notif_module_t;

/*
 * The opaque type for a list of interested clients of an event.
 * This is incorporated and leveraged by a server.
 */
struct bcm_notif_list_struct;
typedef struct bcm_notif_list_struct *bcm_notif_h;

/*
 * Client data passed through during an event.
 * It's submitted by client to server at registration, and returned
 * from server to client upon notification. May be NULL if not needed.
 */
typedef void * bcm_notif_client_data;

/*
 * Server data passed to client upon an event.
 * Actual format is server-specific and must be cast at runtime.
 */
typedef void * bcm_notif_server_data;

/*
 * Type definition for a client callback routine.
 */
typedef void (*bcm_notif_client_callback)(bcm_notif_client_data, bcm_notif_server_data);

/*
 * Server context passed through to the server callback during an event.
 */
typedef void * bcm_notif_server_context;

/*
 * Type definition for a server callback routine.
 */
typedef void (*bcm_notif_server_callback)(bcm_notif_server_context, bcm_notif_client_data);

/*
**************************************************
* Function prototypes: operations on an event_list.
**************************************************
*/

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
bcm_notif_module_t* bcm_notif_attach(osl_t *osh,
                                     bcm_mpm_mgr_h mpm,
                                     int max_notif_servers,
                                     int max_notif_client);

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
void bcm_notif_detach(bcm_notif_module_t *notif_module);

/*
 * bcm_notif_create_list()
 *
 * Initialize new list. Allocates memory.
 *
 * Parameters:
 *    notif_module  Global notifier module object.
 *    hdlp       Pointer to opaque list handle.
 * Returns:
 *    BCME_OK     Object initialized successfully. May be used.
 *    BCME_NOMEM  Initialization failed due to no memory. Object must not be used
 */
int bcm_notif_create_list(bcm_notif_module_t *notif_module, bcm_notif_h *hdlp);

/*
 * bcm_notif_add_interest()
 *
 * Add an interested client
 *
 * Parameters
 *    list       Interest list to which the client is added
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
                           bcm_notif_client_data passthru);

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
 *    BCME_OK     Client interest added successfully
 *    BCME_ERROR  Failed for generic reason.
 *
 */
int bcm_notif_remove_interest(bcm_notif_h hdl,
                              bcm_notif_client_callback callback,
                              bcm_notif_client_data passthru);

/*
 * bcm_notif_signal()
 *
 * Notify all clients on an event list that the event has occured. Invoke their callbacks
 * and provide both the server data and the client passthru data.
 *
 * It is guaranteed that clients will be signaled in the same order in which they
 * expressed interest.
 *
 * Parameters
 *    hdl         Opaque list handle.
 *    server_data Server data for the notification
 * Returns:
 *    BCME_OK     Client interest added successfully
 *    BCME_BUSY   Recursion detected
 */
int bcm_notif_signal(bcm_notif_h listp, bcm_notif_server_data data);

/*
 * bcm_notif_signal_ex()
 *
 * Notify all clients on an event list that the event has occured. Invoke their callbacks
 * and provide both the server data and the client passthru data. Invoke server callback
 * upon every client callback's invocation to allow server to process each client's data.
 *
 * It is guaranteed that clients will be signaled in the same order in which they
 * expressed interest.
 *
 * Parameters
 *    hdl         Opaque list handle.
 *    server_data Server data for the notification
 *    cb          Server callback
 *    arg         Context to server callback
 * Returns:
 *    BCME_OK     Client interest signaled successfully
 *    BCME_BUSY   Recursion detected
 */
int bcm_notif_signal_ex(bcm_notif_h listp, bcm_notif_server_data data,
	bcm_notif_server_callback cb, bcm_notif_server_context arg);

/*
 * bcm_notif_delete_list()
 *
 * Destroy a list. Free all the elements as well as the list itself.
 * After this call, the list must be reinitialized before it can be used.
 *
 * Parameters
 *    hdlp       Pointer to opaque list handle.
 * Returns:
 *    BCME_OK     Event list successfully deleted.
 *    BCME_ERROR  General error
 */
int bcm_notif_delete_list(bcm_notif_h *hdlp);

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
int bcm_notif_dump_list(bcm_notif_h hdl, struct bcmstrbuf *b);

#endif /* BCM_NOTIF_PUB_H */
