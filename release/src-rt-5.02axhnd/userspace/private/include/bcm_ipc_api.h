/***********************************************************************
 *
 *  Copyright (c) 2017  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2017:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef _BCM_IPC_API_H_
#define _BCM_IPC_API_H_

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <limits.h>
#include <stdarg.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>

#include "os_defs.h"
#include "bcm_queue.h"

#define BCM_IPC_WAIT_FOREVER        0xFFFFFFFF  /**< Wait timeout. Wait forever */
#define BCM_IPC_MAX_NAME_LENGTH     16

/** Simple Message queue control block.
 * Simple message queue doesn't support waiting on.
 *
 */

/** Message header */
typedef struct bcmIpcMsg_struct bcmIpcMsg_t;

/** Message queue control block */
typedef struct bcmIpcMsgQueue_struct bcmIpcMsgQueue_t;

typedef STAILQ_HEAD(, bcmIpcMsg_t) bcmIpcMsgList_t;


/** Message queue endpoint type */
typedef enum
{
    BCM_IPC_MSG_QUEUE_EP_LOCAL,
    BCM_IPC_MSG_QUEUE_EP_DOMAIN_SOCKET,   /**< Domain-socket based endpoint */
    BCM_IPC_MSG_QUEUE_EP_UDP_SOCKET,      /**< UDP socket-based endpoint */
    BCM_IPC_MSG_QUEUE_EP_USER_DEFINED     /**< User-defined endpoint */
} bcmIpcMsgQueueEpType_t;

/** Message queue parameters */
typedef struct
{
    const char *name;           /**< Queue name (for logging and debugging) */
    UINT32 size;              /**< Max queue size. 0=unlimited */
    UINT32 high_wm;           /**< Optional high water mark. Log is generated when queue occupancy exceeds hwm */
    UINT32 low_wm;            /**< Optional low water mark. Log is generated when queue occupancy drops below lwm */
    UINT32 flags;             /**< TBD flags. For example, single-core, m-core */
    void (*notify)(bcmIpcMsgQueue_t *q, const char *txt); /***< Called when queue congestion state changes */
    bcmIpcMsgQueueEpType_t     ep_type; /**< Queue endpoint type */

    const char *local_ep_address;       /**< Queue local endpoint address */
    const char *remote_ep_address;      /**< Queue local endpoint address */

    UINT32 max_mtu;                   /**< Max MTU size */
#define BCM_IPC_MSG_QUEUE_DEFAULT_MAX_MTU         (64*1024)

    /** Optional "pack" callback. Not needed if 2 processes are on the same core. */
    int (*pack)(bcmIpcMsgQueue_t *queue, bcmIpcMsg_t *msg, uint8_t **buf, UINT32 *buf_length);

    /** Optional "unpack" callback. Not needed if 2 processes are on the same core. */
    int (*unpack)(bcmIpcMsgQueue_t *queue, uint8_t *buf, UINT32 buf_length, bcmIpcMsg_t **msg);

    /** Optional callback that releases packed buffer */
    void (*free_packed)(bcmIpcMsgQueue_t *queue, uint8_t *buf);

    /** Optional "open" callback. Must be set for user-defined queue, NULL otherwise */
    int (*open)(bcmIpcMsgQueue_t *queue);

    /** Optional "close" callback. Must be set for user-defined queue, NULL otherwise */
    int (*close)(bcmIpcMsgQueue_t *queue);

    /** Optional "send" callback. Must be set for user-defined queue, NULL otherwise */
    int (*send)(bcmIpcMsgQueue_t *queue, uint8_t *buf, UINT32 buf_length);

    /** Optional "recv" callback. Must be set for user-defined queue, NULL otherwise */
    int (*recv)(bcmIpcMsgQueue_t *queue, UINT32 timeout, uint8_t **buf, UINT32 *buf_length);

} bcmIpcMsgQueueParam_t;

/** Message queue statistics */
typedef struct
{
    UINT32 msg_in;            /**< Number of messages currently in the queue */
    UINT32 msg_sent;          /**< Number of messages successfully submitted into the queue */
    UINT32 msg_received;      /**< Number of messages received from the queue */
    UINT32 msg_discarded;     /**< Number of messages discarded because of queue overflow */
    UINT32 msg_almost_full;   /**< Number of messages submitted to queue when it was above high water mark */
    UBOOL8 is_congested;    /**< True=the queue is currently congested */
} bcmIpcMsgQueueStat_t;

/** Message queue info */
typedef struct
{
    bcmIpcMsgQueueParam_t parm;    /**< Queue parameters */
    bcmIpcMsgQueueStat_t  stat;    /**< Queue statistics */
} bcmIpcMsgQueueInfo_t;

typedef struct bcmIpcMsgQueueNw_struct
{
    bcmIpcMsgQueueParam_t parm;       /**< Queue parameters */
    bcmIpcMsgQueueStat_t stat;        /**< Queue statistics */
    pthread_mutex_t      lock;        /**< Queue protection lock */
    bcmIpcMsgList_t      msgl;        /**< Message list */
    bcmIpcMsgList_t      msgl_urg;    /**< Urgent message list */
    UINT32               flags;       /**< Queue flags */
} bcmIpcMsgQueueNw_t;

#ifdef DESKTOP_LINUX

struct bcmIpcMsgQueue_struct
{
    bcmIpcMsgQueueNw_t q;               /**< Queue control block */
    UBOOL8             is_waiting;      /**< TRUE if task is waiting on queue */
    char name[BCM_IPC_MAX_NAME_LENGTH];   /**< Queue name */

    long ep;                            /**< Endpoint handle (e.g., socket) */
    void *ep_extra_data;                /**< Extra data - depending on ep type */
    UINT32 ep_extra_data_size;          /**< Extra data size */
    UINT8 *send_buf;                    /**< Send buffer */
    UINT8 *recv_buf;                    /**< Receive buffer */
    pthread_mutex_t send_lock;          /**< Mutex that protects send_buf */

    UINT32   magic;                     /**< magic number */
#define BCM_IPC_MSG_QUEUE_VALID           (('m'<<24) | ('q' << 16) | ('u' << 8) | 'e')
#define BCM_IPC_MSG_QUEUE_DELETED         (('m'<<24) | ('q' << 16) | ('u' << 8) | '~')
    STAILQ_ENTRY(bcmIpcMsgQueue_t) list; /* Queue list */
};

#else

/** Message queue control block */
typedef struct bcmIpcMsgQueue_struct
{
    bcmIpcMsgQueueNw_t q;               /**< Queue control block */
    UBOOL8             is_waiting;      /**< TRUE if task is waiting on queue */
    char name[BCM_IPC_MAX_NAME_LENGTH];   /**< Queue name */

    long ep;                            /**< Endpoint handle (e.g., socket) */
    void *ep_extra_data;                /**< Extra data - depending on ep type */
    UINT32 ep_extra_data_size;          /**< Extra data size */
    UINT8 *send_buf;                    /**< Send buffer */
    UINT8 *recv_buf;                    /**< Receive buffer */
    pthread_mutex_t send_lock;          /**< Mutex that protects send_buf */

    UINT32   magic;                     /**< magic number */
#define BCM_IPC_MSG_QUEUE_VALID           (('m'<<24) | ('q' << 16) | ('u' << 8) | 'e')
#define BCM_IPC_MSG_QUEUE_DELETED         (('m'<<24) | ('q' << 16) | ('u' << 8) | '~')
    STAILQ_ENTRY(bcmIpcMsgQueue_t) list; /* Queue list */
} bcmIpcMsgQueue_t;
#endif

/** Message send flags */
typedef enum
{
    BCM_IPC_MSG_SEND_AUTO_FREE = 0x00000000,      /**< Automatically release message in case of error. This is the default behaviour */
    BCM_IPC_MSG_SEND_NO_FREE_ON_ERROR   =0x00000001,/**< Do NOT free message in case of transmit error */
    BCM_IPC_MSG_SEND_NO_FREE   = 0x00000002,      /**< Urgent message */
    BCM_IPC_MSG_SEND_URGENT    = 0x00000004,      /**< Ignore destination queue size limit */
} bcmIpcMsgSendFlags_t;

typedef UINT32 bcmIpcMsgId_t;
typedef UINT32 bcmIpcMsgInstance_t;
typedef UINT32 bcmIpcModuleId_t;

/** Registered message handler */
typedef void (*bcmIpcMsgHandlerPtr_t)(bcmIpcModuleId_t module_id, bcmIpcMsg_t *msg);
typedef int (*bcmIpcModuleInitPtr_t)(long data);
typedef void (*bcmIpcModuleExitPtr_t)(long data);

#ifdef DESKTOP_LINUX

struct bcmIpcMsg_struct
{
    bcmIpcMsgId_t   type;        /**< Message type */
    bcmIpcMsgInstance_t instance;/**< Message recipient instance (e.g., optical link id) */
    bcmIpcMsgHandlerPtr_t handler;/**< Message handler. Can be set by the sender or message dispatcher */
    bcmIpcModuleId_t sender;     /**< Sender module */
    STAILQ_ENTRY(bcmIpcMsg_t) next; /**< Next message pointer */
    void *data;                 /**< Message data pointer */
    void *start;                /**< Message data block start (for release) */
    UINT32 size;              /**< Message data size */
    bcmIpcMsgSendFlags_t      send_flags;    /**< Flags the message was sent with */
#define BCM_IPC_MSG_QUEUE_SIZE_UNLIMITED    0xFFFFFFFF      /**< Unlimited queue */
    void (*release)(bcmIpcMsg_t *);       /**< Release callback */
    void (*data_release)(bcmIpcMsg_t *);  /**< Data release callback. Used when message is released to message pool */
};

#else

/** Message header */
typedef struct bcmIpcMsg_struct
{
    bcmIpcMsgId_t   type;        /**< Message type */
    bcmIpcMsgInstance_t instance;/**< Message recipient instance (e.g., optical link id) */
    bcmIpcMsgHandlerPtr_t handler;/**< Message handler. Can be set by the sender or message dispatcher */
    bcmIpcModuleId_t sender;     /**< Sender module */
    STAILQ_ENTRY(bcmIpcMsg_t) next; /**< Next message pointer */
    void *data;                 /**< Message data pointer */
    void *start;                /**< Message data block start (for release) */
    UINT32 size;              /**< Message data size */
    bcmIpcMsgSendFlags_t      send_flags;    /**< Flags the message was sent with */
#define BCM_IPC_MSG_QUEUE_SIZE_UNLIMITED    0xFFFFFFFF      /**< Unlimited queue */
    void (*release)(bcmIpcMsg_t *);       /**< Release callback */
    void (*data_release)(bcmIpcMsg_t *);  /**< Data release callback. Used when message is released to message pool */
} bcmIpcMsg_t;
#endif

/* Helper functions that pack / unpack message header for IPC */

/** Size of message header on the line for IPC */
#define BCM_IPC_MSG_HDR_SIZE      10

/** Create message queue.
 *
 * \param[in,out]  queue        Queue control block
 * \param[in]      parm         Queue parameters
 * \returns 0=OK or error code <0
 */
int bcm_ipc_msg_queue_create(bcmIpcMsgQueue_t *queue, const bcmIpcMsgQueueParam_t *parm);

/** Destroy queue
 *
 * \param[in]   queue           Queue handle
 * \returns 0=OK or error code <0
 */
int bcm_ipc_msg_queue_destroy(bcmIpcMsgQueue_t *queue);

/** Get queue info
 *
 * \param[in]   queue           Queue handle
 * \param[out]  info            Queue information
 * \returns 0=OK or error code <0
 */
int bcm_ipc_msg_queue_query(const bcmIpcMsgQueue_t *queue, bcmIpcMsgQueueInfo_t *info);

/** Message queue iterator
 * \param[in] prev      Previous queue. *prev==NULL - get first
 * \return: 0=OK or error code (including "no more") <0
 */
int bcm_ipc_msg_queue_get_next(const bcmIpcMsgQueue_t **prev);

/** Send message to queue
 *
 * \param[in]   queue           Queue handle
 * \param[in]   msg             Message
 * \param[in]   flags           flags. Combination of \ref bcmIpcMsgSendFlags_t
 *
 * msg is released automatically regardless of bcm_ipc_msg_send() outcome, unless
 * BCM_IPC_MSG_SEND_NO_FREE_ON_ERROR flag is set.
 *
 * \returns 0=OK or error code <0
 */
int bcm_ipc_msg_send(bcmIpcMsgQueue_t *queue, bcmIpcMsg_t *msg, bcmIpcMsgSendFlags_t flags);

#define bcm_ipc_msg_send_std(queue, msg) \
    bcm_ipc_msg_send(queue, msg, BCM_IPC_MSG_SEND_AUTO_FREE)

/** Send message to module
 *
 * For module to be able to receive the message, it must be registered
 * using bcmIpc_msg_register()
 *
 * \param[in]   module_id       Module id
 * \param[in]   msg             Message. msg->handler must be set
 * \param[in]   flags           flags. Combination of \ref bcmIpcMsgSendFlags_t
 *
 * msg is released automatically regardless of bcm_ipc_msg_send_to_module() outcome, unless
 * BCM_IPC_MSG_SEND_NO_FREE_ON_ERROR flag is set.
 *
 * \returns 0=OK or error code <0
 */
int bcm_ipc_msg_recv(bcmIpcMsgQueue_t *queue, UINT32 timeout, bcmIpcMsg_t **msg);


#endif /* _BCM_IPC_API_H_ */
