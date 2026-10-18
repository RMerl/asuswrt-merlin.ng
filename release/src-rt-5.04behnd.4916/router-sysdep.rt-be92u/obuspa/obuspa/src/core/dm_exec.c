/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020,  BT PLC
 * Copyright (C) 2022, Snom Technology GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file dm_exec.c
 *
 * Main loop for data model thread
 *
 */

#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "common_defs.h"
#include "mtp_exec.h"
#include "dm_exec.h"
#include "data_model.h"
#include "sync_timer.h"
#include "cli.h"
#include "data_model.h"
#include "dm_access.h"
#include "device.h"
#include "msg_handler.h"
#include "os_utils.h"
#include "database.h"
#include "dm_trans.h"
#include "nu_ipaddr.h"
#include "stomp.h"
#include "dm_inst_vector.h"
#include "usp-record.pb-c.h"
#include "proto_trace.h"
#include "uds.h"
#include "usp_broker.h"

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include "e2e_context.h"
#endif

#ifdef ENABLE_COAP
#include "usp_coap.h"
#endif

#ifdef ENABLE_WEBSOCKETS
#include "wsclient.h"
#include "wsserver.h"
#endif

#ifndef REMOVE_USP_SERVICE
#include "usp_service.h"
#endif

//------------------------------------------------------------------------------
// Socket pair used to implement the main data model message queue
// One socket is always used for sending, and the other always used for receiving
static int main_mq_sockets[2] = {-1, -1};

#define main_mq_rx_socket  main_mq_sockets[0]
#define main_mq_tx_socket  main_mq_sockets[1]

#ifndef REMOVE_USP_BROKER
//------------------------------------------------------------------------------
// Socket pair used to implement a filter message queue
// Messages sent to the filter message queue may be filtered (and processed) before being
// forwarded to the main data model message queue.
static int filter_mq_sockets[2] = {-1, -1};

#define filter_mq_rx_socket  filter_mq_sockets[0]
#define filter_mq_tx_socket  filter_mq_sockets[1]

// Flag to determine whether to post received USP Records to the main mesasage queue or the filter message queue
volatile bool divert_to_filter_queue = false;
#endif

//------------------------------------------------------------------------------
// The following macro is used to determine whether the call to send to the data model's message queue blocks
// Normally it would block, making the calling thread wait if the message queue is full
// However, if called from the data model thread, the send must not block (as that would cause a deadlock)
// NOTE: This macro should only be used in functions that are not called by MTP threads
//       MTP threads must not ever make send() invocations that block (as that causes deadlock if the queue is full)
#define BLOCK_UNLESS_DM_THREAD (OS_UTILS_IsDataModelThread(NULL, false) ? MSG_DONTWAIT : 0)

//-------------------------------------------------------------------------
// Type of message on data model's message queue
typedef enum
{
    kDmExecMsg_OperComplete,       // Sent from a thread performing an operation to signal that the operation has completed
    kDmExecMsg_OperStatus,         // Sent from a thread performing an operation to signal the new value for the status of the operation (the data model thread performs the actual update of this value)
    kDmExecMsg_EventComplete,      // Sent from a thread to signal that an event has occurred
    kDmExecMsg_ObjAdded,           // Sent from a thread to signal that an object has been added by the vendor
    kDmExecMsg_ObjDeleted,         // Sent from a thread to signal that an object has been deleted by the vendor
    kDmExecMsg_ProcessUspRecord,   // Sent from the MTP thread with a USP Record to process
    kDmExecMsg_MtpThreadExited,    // Sent to signal that the MTP thread has exited as requested by a scheduled exit
    kDmExecMsg_BdcTransferResult,  // Sent to signal that the BDC thread has sent (or failed to send) a report
    kDmExecMsg_StompHandshakeComplete, // Sent from the MTP thread to notify the controller trust role to use for all controllers connected to the specified stomp connection
    kDmExecMsg_MqttHandshakeComplete, // Sent from the MTP thread to notify the controller trust role to use for all controllers connected to the specified mqtt client
#ifdef ENABLE_WEBSOCKETS
    kDmExecMsg_WebsockHandshakeComplete, // Sent from the MTP thread to notify the controller trust role to use for all controllers connected to the specified websocket client
#endif
    kDmExecMsg_DoWork,             // Sent from a thread to cause the data model thread to call the provided callback
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    kDmExecMsg_E2eSessionEvent,    // Sent from a thread to signal an event related to the E2E Session Context has occurred.
#endif
#ifdef ENABLE_UDS
    kDmExecMsg_UdsHandshakeComplete, // Sent from UDS MTP on successful connection to another UDS endpoint
    kDmExecMsg_UdsDisconnected,    // Sent from UDS MTP after a UDS endpoint disconnects
#endif
} dm_exec_msg_type_t;


// Operation complete parameters in data model handler message
typedef struct
{
    int instance;  // Instance from the Device.LocalAgent.Request table
    int err_code;
    char *err_msg;
    kv_vector_t *output_args;
} oper_complete_msg_t;

// Event complete parameters in data model message
typedef struct
{
    char *event_name;
    kv_vector_t *output_args;
} event_complete_msg_t;

// Operation status parameters in data model message
typedef struct
{
    int instance;
    char *status;
} oper_status_msg_t;


// Process USP Record parameters in data model message
typedef struct
{
    unsigned char *pbuf;
    int pbuf_len;
    char *originator;               // endpoint_id that sent this USP record
    int role_instance;      // Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
    mtp_conn_t mtp_conn;    // destination to send the USP message response to
} process_usp_record_msg_t;

// Notify controller trust role for all controllers connected to the specified STOMP connection, and send connect records to them
typedef struct
{
    int stomp_instance;
    char *agent_queue;
    int role_instance;      // Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
} stomp_complete_msg_t;

// Notify controller trust role for all controllers connected to the specified MQTT client, and send connect records to them
typedef struct
{
    int mqtt_instance;
    int role_instance;      // Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}

    mqtt_protocolver_t version;
    char *client_id;
} mqtt_complete_msg_t;

// Notify controller trust role for the controller connected vaia websocket client
typedef struct
{
    int cont_instance;
    int role_instance;      // Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
} websock_complete_msg_t;

// Notify USP Service connections
typedef struct
{
    char *endpoint_id;
    uds_path_t path_type;
    unsigned conn_id;
} uds_complete_msg_t;

typedef struct
{
    char *endpoint_id;
    uds_path_t path_type;
} uds_disconnected_msg_t;

// Object added parameters in data model message
typedef struct
{
    char *path;
} obj_added_msg_t;

// Object deleted parameters in data model message
typedef struct
{
    char *path;
} obj_deleted_msg_t;

// Call the provided callback from the data model thread
typedef struct
{
    do_work_cb_t do_work_cb;
    void *arg1;
    void *arg2;
} do_work_msg_t;

// Management IP address changed parameters in data model message
typedef struct
{
    char ip_addr[NU_IPADDRSTRLEN];
} mgmt_ip_addr_msg_t;

// MTP Thread exited
typedef struct
{
    unsigned flags;         // Bitmask indicating which thread exited
} mtp_thread_exited_msg_t;

// Bulk Data Collection thread sent (or failed to send) a report
typedef struct
{
    int profile_id;         // Instance number of profile in Device.Bulkdata.Profile.{i}
    bdc_transfer_result_t transfer_result;   // Result code of sending the report
} bdc_transfer_result_msg_t;

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
// E2ESession Reset parameters in data model handler message
typedef struct
{
    e2e_event_t event;  // Type of E2E Event
    int request_instance;  // Instance from the Device.LocalAgent.Request table,
                           // or INVALID when not called from the async operation
    int controller_instance;  // Instance from the Device.LocalAgent.Controller table
} e2e_event_msg_t;
#endif

// Structure of data model message
typedef struct
{
    dm_exec_msg_type_t type;
    union
    {
        oper_complete_msg_t oper_complete;
        event_complete_msg_t event_complete;
        oper_status_msg_t oper_status;
        obj_added_msg_t obj_added;
        obj_deleted_msg_t obj_deleted;
        do_work_msg_t do_work;
        process_usp_record_msg_t usp_record;
        stomp_complete_msg_t stomp_complete;
        mqtt_complete_msg_t mqtt_complete;
        websock_complete_msg_t websock_complete;
        uds_complete_msg_t uds_complete;
        uds_disconnected_msg_t uds_disconnected;
        mgmt_ip_addr_msg_t mgmt_ip_addr;
        mtp_thread_exited_msg_t mtp_thread_exited;
        bdc_transfer_result_msg_t bdc_transfer_result;
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
        e2e_event_msg_t e2e_event;
#endif
    } params;

} dm_exec_msg_t;

//------------------------------------------------------------------------------------
// Mutex used to protect access to this component
// This mutex is only really necessary for an orderly shutdown, to ensure the thread isn't doing anything when we free it's memory
static pthread_mutex_t dm_access_mutex;

//------------------------------------------------------------------------------
// Bitmask of MTP threads that have exited. Used to only shutdown the datamodel when all MTP threads have exited
unsigned cumulative_mtp_threads_exited = 0;

//------------------------------------------------------------------------------
// Boolean which is set once the MTP has been connected to successfully
// The purpose of this flag is to avoid USP notifications getting enqueued before the MTP has been connected to successfully
// for the first time after bootup
static bool is_notifications_enabled = false;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void UpdateSockSet(socket_set_t *set);
void ProcessSocketActivity(socket_set_t *set);
void ProcessMessageQueueSocketActivity(socket_set_t *set);
void FreeDmExecMessageArguments(dm_exec_msg_t *msg);
#ifndef REMOVE_USP_BROKER
void ForwardPendingMessagesOnFilterQueue(socket_set_t *set);
Usp__Msg *IsMatchingMsgId(dm_exec_msg_t *msg, char *msg_id, char *responder, Usp__Header__MsgType header_type, bool *is_handled);
#endif
#ifdef ENABLE_UDS
void HandleUdsHandshakeComplete(char *endpoint_id, uds_path_t path_type, unsigned conn_id);
void HandleUdsDisconnected(char *endpoint_id, uds_path_t path_type);
#endif

#ifdef BDK_USP
void BDK_ProcessSocketActivity(socket_set_t *set);
void BDK_UpdateSocketSet(socket_set_t *set);
#endif

/*********************************************************************//**
**
** DM_EXEC_Init
**
** Initialises the functionality in this module
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_EXEC_Init(void)
{
    int err;

    // Exit if unable to initialize the socket pair used to implement the main message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, main_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair(main)", errno);
        return USP_ERR_INTERNAL_ERROR;
    }

#ifndef REMOVE_USP_BROKER
    // Exit if unable to initialize the socket pair used to implement the filter message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, filter_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair(filter)", errno);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

    // Exit if unable to create mutex protecting access to this subsystem
    err = OS_UTILS_InitMutex(&dm_access_mutex);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_EXEC_Destroy
**
** Frees all memory used by the data model thread
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DM_EXEC_Destroy(void)
{
    DATA_MODEL_Stop();
    DATABASE_Destroy();
    SYNC_TIMER_Destroy();
}

/*********************************************************************//**
**
** USP_SIGNAL_OperationComplete
**
** Posts an operation complete message on the data model's message queue
** NOTE: Ownership of the (dynamically allocated by caller) output_args passes to data model
**       But err_msg is copied by this function (ie ownership of err_msg does not pass to this function)
** NOTE: Error messages in this function are only logged rather than writing in the error message buffer (USP_ERR_SetMessage())
**       because this function is normally called from a non core thread and if they did write, this might cause corruption of
**       the core agent error message buffer
**
** \param   instance - instance number of operation in Device.LocalAgent.Request table
** \param   err_code - error code of the operation (USP_ERR_OK indicates success)
** \param   err_msg - error message if the operation failed, or NULL if operation was successful
** \param   output_args - results of the completed operation (if successful). NULL indicates no output arguments.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_SIGNAL_OperationComplete(int instance, int err_code, char *err_msg, kv_vector_t *output_args)
{
    dm_exec_msg_t  msg;
    oper_complete_msg_t *ocm;
    int bytes_sent;

    // Exit if this function has been called with a mismatch between err_code and err_msg
    if ( ((err_code == USP_ERR_OK) && (err_msg != NULL)) ||
         ((err_code != USP_ERR_OK) && (err_msg == NULL)) )
    {
        USP_LOG_Error("%s: Mismatch in calling arguments err_code=%d, but err_msg='%s'", __FUNCTION__, err_code, err_msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_OperComplete;
    ocm = &msg.params.oper_complete;
    ocm->instance = instance;
    ocm->err_code = err_code;
    ocm->err_msg = USP_STRDUP(err_msg);
    ocm->output_args = output_args;

    // Send the message - blocks if queue is full, unless calling from the data model thread (in which case discards the message)
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), BLOCK_UNLESS_DM_THREAD);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Unable to send kDmExecMsg_OperComplete (instance=%d, err_code=%d, err_msg=%s)", __FUNCTION__, instance, err_code, err_msg);
        FreeDmExecMessageArguments(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_SIGNAL_DataModelEvent
**
** Posts an event message on the data model's message queue
** NOTE: Ownership of the (dynamically allocated by caller) output_args passes to data model
**       But event_name is copied by this function (ie ownership of event_name does not pass to this function)
** NOTE: Error messages in this function are only logged rather than writing in the error message buffer (USP_ERR_SetMessage())
**       because this function is normally called from a non core thread and if they did write, this might cause corruption of
**       the core agent error message buffer
**
** \param   event_name - name of the event
** \param   output_args - arguments for the event. NULL indicates no output arguments.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_SIGNAL_DataModelEvent(char *event_name, kv_vector_t *output_args)
{
    dm_exec_msg_t  msg;
    event_complete_msg_t *ecm;
    int bytes_sent;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_EventComplete;
    ecm = &msg.params.event_complete;
    ecm->event_name = USP_STRDUP(event_name);
    ecm->output_args = output_args;

    // Send the message - blocks if queue is full, unless calling from the data model thread (in which case discards the message)
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), BLOCK_UNLESS_DM_THREAD);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Unable to send kDmExecMsg_EventComplete (event_name=%s)", __FUNCTION__, event_name);
        FreeDmExecMessageArguments(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_SIGNAL_OperationStatus
**
** Posts an operation status message on the data model's message queue
** This function can be used by an operation thread to set the value of Device.LocalAgent.Request.{i}.Status in a thread-safe way
** NOTE: Error messages in this function are only logged rather than writing in the error message buffer (USP_ERR_SetMessage())
**       because this function is normally called from a non core thread and if they did write, this might cause corruption of
**       the core agent error message buffer
**
** \param   instance - instance number of operation in Device.LocalAgent.Request table
** \param   status - status string to set
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_SIGNAL_OperationStatus(int instance, char *status)
{
    dm_exec_msg_t  msg;
    oper_status_msg_t *osm;
    int bytes_sent;

    // Exit if this function has been called with invalid parameters
    if (status == NULL)
    {
        USP_LOG_Error("%s: status input argument must point to a string", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_OperStatus;
    osm = &msg.params.oper_status;
    osm->instance = instance;
    osm->status = USP_STRDUP(status);

    // Send the message - blocks if queue is full, unless calling from the data model thread (in which case discards the message)
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), BLOCK_UNLESS_DM_THREAD);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Unable to send kDmExecMsg_OperStatus (instance=%d, status=%s)", __FUNCTION__, instance, status);
        FreeDmExecMessageArguments(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_SIGNAL_ObjectAdded
**
** Signals to USP core that the vendor has added an object instance to the data model
** This function may be called from any vendor thread
**
** \param   path - path of object that has been added
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_SIGNAL_ObjectAdded(char *path)
{
    dm_exec_msg_t  msg;
    obj_added_msg_t *oam;
    int bytes_sent;

    // Exit if this function has been called with invalid parameters
    if (path == NULL)
    {
        USP_LOG_Error("%s: path input argument must point to a string", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_ObjAdded;
    oam = &msg.params.obj_added;
    oam->path = USP_STRDUP(path);

    // Send the message - blocks if queue is full, unless calling from the data model thread (in which case discards the message)
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), BLOCK_UNLESS_DM_THREAD);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Unable to send kDmExecMsg_ObjAdded (path=%s)", __FUNCTION__, path);
        FreeDmExecMessageArguments(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_SIGNAL_ObjectDeleted
**
** Signals to USP core that the vendor has deleted an object instance from the data model
** This function may be called from any vendor thread
**
** \param   path - path of object that has been deleted
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_SIGNAL_ObjectDeleted(char *path)
{
    dm_exec_msg_t  msg;
    obj_deleted_msg_t *odm;
    int bytes_sent;

    // Exit if this function has been called with invalid parameters
    if (path == NULL)
    {
        USP_LOG_Error("%s: path input argument must point to a string", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_ObjDeleted;
    odm = &msg.params.obj_deleted;
    odm->path = USP_STRDUP(path);

    // Send the message - blocks if queue is full, unless calling from the data model thread (in which case discards the message)
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), BLOCK_UNLESS_DM_THREAD);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Unable to send kDmExecMsg_ObjDeleted (path=%s)", __FUNCTION__, path);
        FreeDmExecMessageArguments(&msg);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_PROCESS_DoWork
**
** This function allows the caller to perform arbitrary work in the data model thread
** It posts a message on the data model's queue, which when handled calls the specified callback
** The callback may call any of the USP_DM_XXX functions to perform whatever work it needs to do
** NOTE: Ownership of the memory pointed to by the arguments stays with the caller (vendor layer)
**
** \param   do_work_cb - function that will be called back from the data model thread
** \param   arg1 - first argument passed to the do work callback
** \param   arg2 - second argument passed to the do work callback
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_PROCESS_DoWork(do_work_cb_t do_work_cb, void *arg1, void *arg2)
{
    dm_exec_msg_t  msg;
    do_work_msg_t *dwm;
    int bytes_sent;

    // Exit if this function has been called with invalid parameters
    if (do_work_cb == NULL)
    {
        USP_LOG_Error("%s: callback must not be NULL", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_DoWork;
    dwm = &msg.params.do_work;
    dwm->do_work_cb = do_work_cb;
    dwm->arg1 = arg1;
    dwm->arg2 = arg2;

    // Send the message - blocks if queue is full, unless calling from the data model thread (in which case discards the message)
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), BLOCK_UNLESS_DM_THREAD);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Unable to send kDmExecMsg_DoWork", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DM_EXEC_PostUspRecord
**
** Posts a USP record to be processed by the data model thread
**
** \param   pbuf - pointer to buffer containing protobuf encoded USP record
**                 NOTE: This is part of a larger buffer (with STOMP), so must be copied before sending to the data model thread
** \param   pbuf_len - length of protobuf encoded message
** \param   originator - EndpointID that sent this record (if know, or UNKNOWN_ENDPOINT_ID if unknown)
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostUspRecord(unsigned char *pbuf, int pbuf_len, char *originator, int role_instance, mtp_conn_t *mtpc)
{
    dm_exec_msg_t  msg;
    process_usp_record_msg_t *pur;
    int bytes_sent;
    int mq_tx_socket = main_mq_tx_socket;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_ProcessUspRecord;
    pur = &msg.params.usp_record;
    pur->pbuf = USP_MALLOC(pbuf_len);
    memcpy(pur->pbuf, pbuf, pbuf_len);
    pur->pbuf_len = pbuf_len;
    pur->originator = USP_STRDUP(originator);
    pur->role_instance = role_instance;
    DM_EXEC_CopyMTPConnection(&pur->mtp_conn, mtpc);

#ifndef REMOVE_USP_BROKER
    // Send this message to the filter queue first, if the data model is waiting for a particular response msg_id
    if (divert_to_filter_queue)
    {
        mq_tx_socket = filter_mq_tx_socket;
    }
#endif

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Discarding received USP Record", __FUNCTION__);
        FreeDmExecMessageArguments(&msg);
        return;
    }
}

/*********************************************************************//**
**
** DM_EXEC_PostStompHandshakeComplete
**
** Posts the role associated with a Stomp connection, after the STOMP initial TLS handshake has completed
** This notifies the DataModel of the role to use for each controller connected to a STOMP broker
** This message will unblock processing of Boot! event and subscriptions, which are held up until the controller
** trust role associated with each controller is known (otherwise they would use the wrong role when getting data)
** Note: Restarting of async operations are also held up, because we want them to occur after the Boot! event
**
** \param   stomp_instance - instance number of STOMP connection in Device.STOMP.Connection.{i}
** \param   agent_queue - STOMP destination which the agent has actually subscribed to
**                        NOTE: This may have been set by Device.LocalAgent.MTP.{i}.STOMP.Destination, or it may have been set by the subscribe-dest STOMP header in the CONNECTED frame
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostStompHandshakeComplete(int stomp_instance, char *agent_queue, int role_instance)
{
    dm_exec_msg_t  msg;
    stomp_complete_msg_t *scm;
    int bytes_sent;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_StompHandshakeComplete;
    scm = &msg.params.stomp_complete;
    scm->stomp_instance = stomp_instance;
    scm->agent_queue = USP_STRDUP(agent_queue);
    scm->role_instance = role_instance;

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_StompHandshakeComplete (stomp_instance=%d, agent_queue=%s)", __FUNCTION__, stomp_instance, agent_queue);
        FreeDmExecMessageArguments(&msg);
        return;
    }
}

/*********************************************************************//**
**
** DM_EXEC_PostMqttHandshakeComplete
**
** Posts the role associated with an MQTT connection, after the TLS handshake has completed
** This notifies the DataModel of the role to use for each controller connected to an MQTT broker
** This message will unblock processing of Boot! event and subscriptions, which are held up until the controller
** trust role associated with each controller is known (otherwise they would use the wrong role when getting data)
** Note: Restarting of async operations are also held up, because we want them to occur after the Boot! event
**
** \param   mqtt_instance - instance number of connection in Device.MQTT.Client.{i}
** \param   version - MQTT version in use on the connection
** \param   client_id - MQTT client_id that the agent used or was assigned in the MQTTv5 CONNACK. This should be persistently stored in Device.MQTT.Client.{i}.ClientID
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostMqttHandshakeComplete(int mqtt_instance, mqtt_protocolver_t version, char *client_id, int role_instance)
{
    dm_exec_msg_t  msg;
    mqtt_complete_msg_t *mcm;
    int bytes_sent;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_MqttHandshakeComplete;
    mcm = &msg.params.mqtt_complete;
    mcm->mqtt_instance = mqtt_instance;
    mcm->version = version;
    mcm->client_id = USP_STRDUP(client_id);
    mcm->role_instance = role_instance;

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_MqttHandshakeComplete (mqtt_instance=%d)", __FUNCTION__, mqtt_instance);
        FreeDmExecMessageArguments(&msg);
        return;
    }
}

#ifdef ENABLE_WEBSOCKETS
/*********************************************************************//**
**
** DM_EXEC_PostWebsockHandshakeComplete
**
** Posts the inherited role associated with a Websocket client connection, after the TLS handshake has completed
** This message will unblock processing of Boot! event and subscriptions, which are held up until the controller
** trust role associated with each controller is known (otherwise they would use the wrong role when getting data)
** Note: Restarting of async operations are also held up, because we want them to occur after the Boot! event
**
** \param   cont_instance - instance number of controller to which the websocket is connected
** \param   role_instance - Inherited role instance in Device.LocalAgent.ControllerTrust.Role.{i}
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostWebsockHandshakeComplete(int cont_instance, int role_instance)
{
    dm_exec_msg_t  msg;
    websock_complete_msg_t *wcm;
    int bytes_sent;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_WebsockHandshakeComplete;
    wcm = &msg.params.websock_complete;
    wcm->cont_instance = cont_instance;
    wcm->role_instance = role_instance;

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_WebsockHandshakeComplete (cont_instance=%d)", __FUNCTION__, cont_instance);
        FreeDmExecMessageArguments(&msg);
        return;
    }
}
#endif

#ifdef ENABLE_UDS
/*********************************************************************//**
**
** DM_EXEC_PostUdsHandshakeComplete
**
** Called to indicate that a USP endpoint (either USP Service or USP Broker) has connected successfully
** The first time this is called, it will unblock processing of subscriptions on this device
**
** \param   endpoint_id - endpoint that has connected
** \param   path_type - whether the endpoint is connected to the Broker's Controller or the Broker's Agent socket
** \param   conn_id - Unique identifier for the connection to the endpoint
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostUdsHandshakeComplete(char *endpoint_id, uds_path_t path_type, unsigned conn_id)
{
    dm_exec_msg_t  msg;
    uds_complete_msg_t *ucm;
    int bytes_sent;

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_UdsHandshakeComplete;
    ucm = &msg.params.uds_complete;
    ucm->endpoint_id = USP_STRDUP(endpoint_id);
    ucm->path_type = path_type;
    ucm->conn_id = conn_id;

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_UdsHandshakeComplete (endpoint_id=%s, path_type=%s)", __FUNCTION__, endpoint_id, UDS_PathTypeToString(path_type));
        FreeDmExecMessageArguments(&msg);
        return;
    }
}

/*********************************************************************//**
**
** DM_EXEC_PostUdsDisconnected
**
** Called to indicate that a USP endpoint (either USP Service or USP Broker) has disconnected from this endpoint
**
** \param   endpoint_id - endpoint that has disconnected
** \param   path_type - whether the endpoint was connected to the Broker's Controller or the Broker's Agent socket
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostUdsDisconnected(char *endpoint_id, uds_path_t path_type)
{
    dm_exec_msg_t  msg;
    uds_disconnected_msg_t *udm;
    int bytes_sent;
    int mq_tx_socket = main_mq_tx_socket;

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_UdsDisconnected;
    udm = &msg.params.uds_disconnected;
    udm->endpoint_id = USP_STRDUP(endpoint_id);
    udm->path_type = path_type;

#ifndef REMOVE_USP_BROKER
    // Send this message to the filter queue first, if the data model is waiting for a particular response msg_id
    if (divert_to_filter_queue)
    {
        mq_tx_socket = filter_mq_tx_socket;
    }
#endif

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_UdsDisconnected (endpoint_id=%s, path_type=%s)", __FUNCTION__, endpoint_id, UDS_PathTypeToString(path_type));
        FreeDmExecMessageArguments(&msg);
        return;
    }
}
#endif

/*********************************************************************//**
**
** DM_EXEC_PostMtpThreadExited
**
** Signals that the MTP thread has exited, this will be because an exit was scheduled
** (either due to the controller requesting a reboot, or factory reset, or a stop CLI command being sent)
**
** \param   flags - flags determining which stomp thread exited
**
** \return  None
**
**************************************************************************/
void DM_EXEC_PostMtpThreadExited(unsigned flags)
{
    dm_exec_msg_t  msg;
    int bytes_sent;
    mtp_thread_exited_msg_t *tem;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_MtpThreadExited;
    tem = &msg.params.mtp_thread_exited;
    tem->flags = flags;

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_MtpThreadExited(flags=0x%04x)", __FUNCTION__, flags);
        FreeDmExecMessageArguments(&msg);  // unnecessary but included for consistency
        return;
    }
}


#ifndef REMOVE_DEVICE_BULKDATA
/*********************************************************************//**
**
** DM_EXEC_NotifyBdcTransferResult
**
** Posts a message that signals to data model thread that the specified Bulk Data Collection
** report has been sent, or failed to send
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
** \param   transfer_result - result code of the transfer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_EXEC_NotifyBdcTransferResult(int profile_id, bdc_transfer_result_t transfer_result)
{
    dm_exec_msg_t  msg;
    bdc_transfer_result_msg_t *btr;
    int bytes_sent;

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_BdcTransferResult;
    btr = &msg.params.bdc_transfer_result;
    btr->profile_id = profile_id;
    btr->transfer_result = transfer_result;

    // Send the message - blocks if the queue is full
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_BdcTransferResult (profile_id=%d, transfer_result=%d)", __FUNCTION__, profile_id, transfer_result);
        FreeDmExecMessageArguments(&msg);  // unnecessary but included for consistency
        return USP_ERR_INTERNAL_ERROR;
    }


    return USP_ERR_OK;
}
#endif

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
/*********************************************************************//**
**
** DM_EXEC_PostE2eEvent
**
** Signal an E2E Event message on the data model's message queue
** This function can be used by an operation thread to terminate/start/restart the
** associated Device.LocalAgent.Controller.1.E2ESession context in a thread-safe way.
**
** \param   event - Type of E2E event happened
** \param   request_instance - Instance from the Device.LocalAgent.Request table,
**                             or INVALID when not called from the async operation
** \param   controller_instance - Instance from the Device.LocalAgent.Controller table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DM_EXEC_PostE2eEvent(e2e_event_t event, int request_instance, int controller_instance)
{
    dm_exec_msg_t  msg;
    e2e_event_msg_t *erm;
    int bytes_sent;

    // Exit if this function has been called with invalid parameters
    if (controller_instance <= 0)
    {
        USP_LOG_Error("%s: Data Model instance must be valid", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if message queue is not setup yet
    if (main_mq_tx_socket == -1)
    {
        USP_LOG_Error("%s is being called before data model has been initialised", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form message
    memset(&msg, 0, sizeof(msg));
    msg.type = kDmExecMsg_E2eSessionEvent;
    erm = &msg.params.e2e_event;
    erm->event = event;
    erm->controller_instance = controller_instance;
    erm->request_instance = request_instance;

    // Send the message - does not block if the queue is full, discards the message instead
    bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        USP_LOG_Error("%s: Failed to send kDmExecMsg_E2eSessionEvent (event=%d, request_instance=%d, controller_instance=%d)", __FUNCTION__, event, request_instance, controller_instance);
        FreeDmExecMessageArguments(&msg);  // unnecessary but included for consistency
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}
#endif

/*********************************************************************//**
**
** DM_EXEC_EnableNotifications
**
** Unblocks processing of Boot! event and subscriptions, which are held up until the controller
** trust role associated with each controller is known (otherwise they would use the wrong role when getting data)
** Note: Restarting of async operations are also held up, because we want them to occur after the Boot! event
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DM_EXEC_EnableNotifications(void)
{
    // Exit if the notifications are already enabled
    if (is_notifications_enabled)
    {
        return;
    }

    // Then start the parts of the data model which were held up, waiting for the controller's role to be known


    // Queue object creation/deletion notifications which occurred between bootup and successfully MTP connecting
    // NOTE: This is done before the Boot! notification, rather than after, because the Boot! notification could refer
    // to parameters in some of the objects that were added, and it would be confusing to see the parameter value,
    // then object creation (i.e. out of order). Likewise it would be incorrect to discard creation/deletion events
    // which occurred between bootup and successfully MTP connecting.
    DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions();

    // Send out initial Boot NotifyReq, and determine the initial values of all value change parameters
    // This also starts the sync timer to poll for value change notifications
    DEVICE_SUBSCRIPTION_Update(0);

    // Restart all asynchronous operations that did not complete (and that are meant to be restarted after a reboot)
    // NOTE: This also sends out events for all operations that required a reboot to complete them
    DEVICE_REQUEST_RestartAsyncOperations();

    // Set flag, so that subsequent calls to this function (eg stomp reconnections) do not start the data model again
    is_notifications_enabled = true;
}

/*********************************************************************//**
**
** DM_EXEC_IsNotificationsEnabled
**
** Returns whether USP notifications may be generated.
** NOTE: USP notifications are not generated before the Boot! event has been sent
**       This function is called to avoid a large queue of USP notifications in the case of MTP connection failure
**
** \param   None
**
** \return  true if USP notifications can be sent
**
**************************************************************************/
bool DM_EXEC_IsNotificationsEnabled(void)
{
    return is_notifications_enabled;
}

/*********************************************************************//**
**
** DM_EXEC_CopyMTPConnection
**
** Performs a deep copy of the specified mtp_conn_t structure
**
** \param   dst - pointer to destination structure
** \param   src - pointer to source structure
**
** \return  None
**
**************************************************************************/
void DM_EXEC_CopyMTPConnection(mtp_conn_t *dst, mtp_conn_t *src)
{
    dst->is_reply_to_specified = src->is_reply_to_specified;
    dst->protocol = src->protocol;

    switch(src->protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            dst->stomp.instance = src->stomp.instance;
            dst->stomp.dest = USP_STRDUP(src->stomp.dest);
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            dst->coap.host = USP_STRDUP(src->coap.host);
            dst->coap.port = src->coap.port;
            dst->coap.resource = USP_STRDUP(src->coap.resource);
            dst->coap.encryption = src->coap.encryption;
            dst->coap.reset_session_hint = src->coap.reset_session_hint;
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            dst->mqtt.topic = USP_STRDUP(src->mqtt.topic);
            dst->mqtt.instance = src->mqtt.instance;
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            dst->ws.client_cont_instance = src->ws.client_cont_instance;
            dst->ws.client_mtp_instance = src->ws.client_mtp_instance;
            dst->ws.serv_conn_id = src->ws.serv_conn_id;
            break;
#endif

#ifdef ENABLE_UDS
        case kMtpProtocol_UDS:
            dst->uds.conn_id = src->uds.conn_id;
            dst->uds.path_type = src->uds.path_type;
            break;
#endif
        default:
            break;

    }
}

/*********************************************************************//**
**
** DM_EXEC_FreeMTPConnection
**
** Frees the specified mtp_conn_t structure, and marks it as not in use by setting protocol to kMtpProtocol_None
**
** \param   mtp - pointer to structure to free all dynamically allocated content from
**
** \return  None
**
**************************************************************************/
void DM_EXEC_FreeMTPConnection(mtp_conn_t *mtp)
{
    switch(mtp->protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            USP_SAFE_FREE(mtp->stomp.dest);
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            USP_SAFE_FREE(mtp->coap.host);
            USP_SAFE_FREE(mtp->coap.resource);
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            USP_SAFE_FREE(mtp->mqtt.topic);
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            break;
#endif

#ifdef ENABLE_UDS
        case kMtpProtocol_UDS:
            break;
#endif
        default:
            break;

    }
    mtp->protocol = kMtpProtocol_None;
}

/*********************************************************************//**
**
** DM_EXEC_HandleScheduledExit
**
** If this function is called, then USP Agent will stop running in this function
** after possibly signalling to the vendor that the CPE should reboot, or perform a factory reset
** NOTE: This function will call exit() in the vendor hooks or at the end of the function
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DM_EXEC_HandleScheduledExit(void)
{
    exit_action_t exit_action;
    dm_vendor_reboot_cb_t   reboot_cb;
    dm_vendor_factory_reset_cb_t   factory_reset_cb;

    // Determine whether to just exit, reboot, or perform a factory reset
    exit_action = DEVICE_LOCAL_AGENT_GetExitAction();

    switch(exit_action)
    {
        case kExitAction_Exit:
            USP_LOG_Info("Performing CLI initiated Stop.");
            MAIN_Stop();
            break;

        case kExitAction_Reboot:
            // NOTE: If reboot is scheduled, then the default for the vendor hook (in Test mode) is to exit here
            // The vendor hook may return or may exit the executable itself
            USP_LOG_Info("Performing Controller initiated Reboot.");
            reboot_cb = vendor_hook_callbacks.reboot_cb;
            MAIN_Stop();
            if (reboot_cb != NULL)
            {
                reboot_cb();
            }
            break;

        case kExitAction_FactoryReset:
            USP_LOG_Info("Performing Controller initiated FactoryReset");
            // If a file exists containing the factory reset database, then copy it, and modify the reboot cause
            // NOTE: This must be called before the data model is shutdown, as programmatic factory reset validates the parameter paths against the data model
            DATABASE_PerformFactoryReset_ControllerInitiated();

            MAIN_Stop();

            // NOTE: If factory reset is scheduled, then the default is to exit here
            // The vendor hook may return or may exit the executable itself
            factory_reset_cb = vendor_hook_callbacks.factory_reset_cb;
            if (factory_reset_cb != NULL)
            {
                factory_reset_cb();
            }
            break;

        default:
            MAIN_Stop();
            break;
    }

    // If nothing else has exited yet, then exit
    usleep(100000); // Sleep for 100us, just to allow all other threads to exit (that sent the kDmExecMsg_MtpThreadExited message)
    exit(0);
}

#ifndef REMOVE_USP_BROKER
/*********************************************************************//**
**
** DM_EXEC_SendRequestAndWaitForResponse
**
** Sends a USP Request message, then waits for the USP Response and returns it
** NOTE: Ownership of req input argument passes to this function, but ownership of mtpc argument stays with caller
** NOTE: The response may be a USP Response or a USP Error message
** NOTE: This function is called when we are acting as a Controller within a Broker
**
** \param   endpoint_id - endpoint to send the message to
** \param   req - pointer to protobuf-c structure describing the USP message to send
** \param   mtpc - details of where this USP message should be sent
** \param   header_type - Type of response message that we are expecting in the USP Message header
** \param   timeout - timeout (in seconds) for receiving a response message
**
** \return  USP response message or NULL if timed out or an error occurred
**
**************************************************************************/
Usp__Msg *DM_EXEC_SendRequestAndWaitForResponse(char *endpoint_id, Usp__Msg *req, mtp_conn_t *mtpc,
                                                Usp__Header__MsgType header_type, int timeout)
{
    char wanted_msg_id[MAX_MSG_ID_LEN];
    int timeout_ms;
    socket_set_t set;
    int bytes_read;
    int bytes_sent;
    dm_exec_msg_t  msg;
    Usp__Msg *resp;
    time_t cur_time;
    time_t end_time;
    int num_sockets;
    bool is_handled;

    // Divert all new posts of received USP records to the filter message queue
    // so that we can inspect them, before forwarding them to the main message queue
    // if they're not the response we're waiting for
    divert_to_filter_queue = true;

    // Send the request message
    MSG_HANDLER_QueueMessage(endpoint_id, req, mtpc);

    // Take a copy of the msg_id of the request, as we're waiting for a response messsage with the same msg_id
    // And then free the request protobuf-c structure, as we don't need it anymore
    USP_STRNCPY(wanted_msg_id, req->header->msg_id, sizeof(wanted_msg_id));
    usp__msg__free_unpacked(req, pbuf_allocator);

    // Wait for the response message, timeout, or USP Service disconnection
    cur_time = time(NULL);
    end_time = cur_time + timeout;
    while (cur_time < end_time)
    {
        timeout_ms = (end_time - cur_time) * 1000;
        SOCKET_SET_Clear(&set);
        SOCKET_SET_AddSocketToReceiveFrom(filter_mq_rx_socket, timeout_ms, &set);

        num_sockets = SOCKET_SET_Select(&set);
        if ((num_sockets == 1) && (SOCKET_SET_IsReadyToRead(filter_mq_rx_socket, &set)))
        {
            // Exit if unable to read the full message received
            bytes_read = recv(filter_mq_rx_socket, &msg, sizeof(msg), 0);
            if (bytes_read != sizeof(msg))
            {
                USP_ERR_SetMessage("%s: recv() did not return a full message", __FUNCTION__);
                return NULL;
            }

#ifdef ENABLE_UDS
            // Exit if the USP Service has disconnected from the Broker's controller socket (since this function uses only that socket)
            if (msg.type == kDmExecMsg_UdsDisconnected)
            {
                uds_disconnected_msg_t *udm;
                udm = &msg.params.uds_disconnected;
                if ((strcmp(udm->endpoint_id, endpoint_id)==0) && (udm->path_type==kUdsPathType_BrokersController))
                {
                    USP_ERR_SetMessage("%s: USP Service (%s) disconnected", __FUNCTION__, udm->endpoint_id);
                    return NULL;
                }
            }
#endif

            // Exit if we've received the response message we're waiting for, freeing the message from the queue
            resp = IsMatchingMsgId(&msg, wanted_msg_id, endpoint_id, header_type, &is_handled);
            if (resp != NULL)
            {
                divert_to_filter_queue = false;     // Switch back to posting to the main message queue
                FreeDmExecMessageArguments(&msg);
                return resp;
            }

            // Forward the message to the main queue if it was not passed through by IsMatchingMessageId
            // NOTE: main queue does not block if the queue is full, discards the message instead
            if (is_handled == false)
            {
                bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
                if (bytes_sent != sizeof(msg))
                {
                    char buf[USP_ERR_MAXLEN];
                    USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
                    USP_LOG_Error("%s: Discarding received USP Record", __FUNCTION__);
                    FreeDmExecMessageArguments(&msg);
                }
            }
            else
            {
                // If the message was handled by IsMatchingMsgId(), then we can free the message arguments
                FreeDmExecMessageArguments(&msg);
            }
        }

        cur_time = time(NULL);
    }

    // If the code gets here, the USP Service did not send a response within the requisite timeout
    USP_ERR_SetMessage("%s: Timed out waiting for response msg_id=%s from endpoint=%s", __FUNCTION__, wanted_msg_id, endpoint_id);

    // Switch back to posting to the main message queue
    divert_to_filter_queue = false;
    return NULL;
}
#endif // REMOVE_USP_BROKER

/*********************************************************************//**
**
** DM_EXEC_Main
**
** Main loop of the data model thread
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *DM_EXEC_Main(void *args)
{
    int err;
    int num_sockets;
    socket_set_t set;
    int enabled_connections = 0;

    // Exit if unable to connect to the unix domain socket used to implement the CLI server
    err = CLI_SERVER_Init();
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: CLI_SERVER_Init() failed. Aborting Data Model thread", __FUNCTION__);
        return NULL;
    }

    // Determine whether we have to wait for the connection, before enabling notifications
    // This is necessary because the contents of the Boot! event may be dependant on the permissions that the USP Controller has, and we'll only know this after connecting to it
#ifndef DISABLE_STOMP
    enabled_connections += DEVICE_STOMP_CountEnabledConnections();
#endif

#ifdef ENABLE_MQTT
    enabled_connections += DEVICE_MQTT_CountEnabledConnections();
#endif

#ifdef ENABLE_WEBSOCKETS
    enabled_connections += DEVICE_CONTROLLER_CountEnabledWebsockClientConnections();
#endif

    // Enable notifications now, if we don't have to wait for the connection before generating a Boot! notification
    if (enabled_connections == 0)
    {
        DM_EXEC_EnableNotifications();
    }

    OS_UTILS_LockMutex(&dm_access_mutex);

    while(FOREVER)
    {
        // Create the socket set to receive/transmit on (with timeout)
        UpdateSockSet(&set);

        // Unlock the mutex around the select()
        OS_UTILS_UnlockMutex(&dm_access_mutex);

        // Wait for read/write activity on sockets or timeout
        num_sockets = SOCKET_SET_Select(&set);

        OS_UTILS_LockMutex(&dm_access_mutex);

        // Process socket activity
        switch(num_sockets)
        {
            case -1:
                // An unrecoverable error has occurred
                USP_LOG_Error("%s: Unrecoverable socket select() error. Aborting Data Model thread", __FUNCTION__);
                return NULL;
                break;

            case 0:
                // No controllers with any activity, but we still may need to process a timeout, so fall-through
            default:
                // Controllers with activity
                ProcessSocketActivity(&set);
                break;
        }

        // Execute all timers which are ready to fire
        // NOTE: This is performed after processing socket activity, because one sync timer is DEVICE_SUBSCRIPTION_Update()
        // which resolves the set of objects that creation notification events should fire on, and we want the set of objects
        // to be resolved after any objects have been created by the socket activity event handler
        SYNC_TIMER_Execute();

        // Queue any object creation/deletion notifications which have been generated by the message or timer callbacks
        DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions();

        // Signal that we've finished with any instances in the cache that got locked whilst processing the message or timer callbacks
        DM_INST_VECTOR_NextLockPeriod();

        // Print out any memory allocations that got added for this time around the loop
        //USP_MEM_Print();
    }
}

/*********************************************************************//**
**
** UpdateSockSet
**
** Adds all sockets to wait for activity on, into the socket set
** Also updates the associated timeout for activity
** This function must be called every time before the call to select(), as select alters the socket set
**
** \param   set - pointer to socket set structure to update with sockets to wait for activity on
**
** \return  None
**
**************************************************************************/
void UpdateSockSet(socket_set_t *set)
{
    int delay_ms;

    SOCKET_SET_Clear(set);

    // Add the CLI server socket to the socket set
    CLI_SERVER_UpdateSocketSet(set);

    // Add the message queue receiving sockets to the socket set
    SOCKET_SET_AddSocketToReceiveFrom(main_mq_rx_socket, MAX_SOCKET_TIMEOUT, set);

#ifndef REMOVE_USP_BROKER
    SOCKET_SET_AddSocketToReceiveFrom(filter_mq_rx_socket, MAX_SOCKET_TIMEOUT, set);
#endif

#ifdef BDK_USP
    BDK_UpdateSocketSet(set);
#endif

    // Update socket timeout time with the time to the next timer
    delay_ms = SYNC_TIMER_TimeToNext();
    SOCKET_SET_UpdateTimeout(delay_ms, set);
}

/*********************************************************************//**
**
** ProcessSocketActivity
**
** Processes all activity on sockets (ie receiving messages and sending messages)
**
** \param   set - pointer to socket set structure containing sockets with activity on them
**
** \return  None
**
**************************************************************************/
void ProcessSocketActivity(socket_set_t *set)
{
#ifndef REMOVE_USP_BROKER
    // Forward any messages on the filter message queue to the main message queue
    if (SOCKET_SET_IsReadyToRead(filter_mq_rx_socket, set))
    {
        ForwardPendingMessagesOnFilterQueue(set);
    }
#endif

    // Process any pending message queue activity first - this allows internal state to be updated before controllers query it
    ProcessMessageQueueSocketActivity(set);

#ifdef BDK_USP
    //Process BDK messages
    BDK_ProcessSocketActivity(set);
#endif

    // Process the socket, if there is any activity from a CLI client
    CLI_SERVER_ProcessSocketActivity(set);
}

/*********************************************************************//**
**
** ProcessMessageQueueSocketActivity
**
** Processes activity on the main message queues
**
** \param   set - pointer to socket set structure containing sockets with activity on them
**
** \return  None (any errors that occur are handled internally)
**
**************************************************************************/
void ProcessMessageQueueSocketActivity(socket_set_t *set)
{
    int err;
    int bytes_read;
    dm_exec_msg_t  msg;
    oper_complete_msg_t *ocm;
    event_complete_msg_t *ecm;
    oper_status_msg_t *osm;
    obj_added_msg_t *oam;
    obj_deleted_msg_t *odm;
    do_work_msg_t *dwm;
    process_usp_record_msg_t *pur;
    mtp_thread_exited_msg_t *tem;
    unsigned all_mtp_exited = 0;
    mtp_conn_t *mtpc;

    // Exit if there is no activity on the main message queue socket
    if (SOCKET_SET_IsReadyToRead(main_mq_rx_socket, set) == 0)
    {
        return;
    }

    // Exit if unable to read the full message received
    bytes_read = recv(main_mq_rx_socket, &msg, sizeof(msg), 0);
    if (bytes_read != sizeof(msg))
    {
        USP_LOG_Error("%s: recv() did not return a full message", __FUNCTION__);
        return;
    }

    switch(msg.type)
    {
        case kDmExecMsg_ProcessUspRecord:
            pur = &msg.params.usp_record;
            mtpc = &pur->mtp_conn;
            MSG_HANDLER_HandleBinaryRecord(pur->pbuf, pur->pbuf_len, pur->originator, pur->role_instance, mtpc); // NOTE: Intentionally ignoring the error. Errors are handled in the function
            break;

#ifndef DISABLE_STOMP
        case kDmExecMsg_StompHandshakeComplete:
        {
            stomp_complete_msg_t *scm;
            scm = &msg.params.stomp_complete;

            DEVICE_CONTROLLER_QueueStompConnectRecord(scm->stomp_instance, scm->agent_queue);
            DEVICE_CONTROLLER_SetRolesFromStomp(scm->stomp_instance, scm->role_instance);
            DM_EXEC_EnableNotifications();
        }
            break;
#endif

#ifdef ENABLE_MQTT
        case kDmExecMsg_MqttHandshakeComplete:
        {
            mqtt_complete_msg_t *mcm;
            mcm = &msg.params.mqtt_complete;
            DEVICE_CONTROLLER_SetRolesFromMqtt(mcm->mqtt_instance, mcm->role_instance);
            if (mcm->version == kMqttProtocol_5_0)
            {
                DEVICE_MQTT_UpdateClientId(mcm->mqtt_instance, mcm->client_id);
            }
            DM_EXEC_EnableNotifications();
        }
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kDmExecMsg_WebsockHandshakeComplete:
        {
            websock_complete_msg_t *wcm;
            wcm = &msg.params.websock_complete;
            DEVICE_CONTROLLER_SetInheritedRole(wcm->cont_instance, wcm->role_instance);
            DM_EXEC_EnableNotifications();
        }
            break;
#endif

#ifdef ENABLE_UDS
        case kDmExecMsg_UdsHandshakeComplete:
        {
            uds_complete_msg_t *ucm;
            ucm = &msg.params.uds_complete;
            HandleUdsHandshakeComplete(ucm->endpoint_id, ucm->path_type, ucm->conn_id);
        }
            break;

        case kDmExecMsg_UdsDisconnected:
        {
            uds_disconnected_msg_t *udm;
            udm = &msg.params.uds_disconnected;
            HandleUdsDisconnected(udm->endpoint_id, udm->path_type);
        }
            break;
#endif

        case kDmExecMsg_OperComplete:
            ocm = &msg.params.oper_complete;
            DEVICE_REQUEST_OperationComplete(ocm->instance, ocm->err_code, ocm->err_msg, ocm->output_args);
            break;

        case kDmExecMsg_EventComplete:
            ecm = &msg.params.event_complete;
            DEVICE_SUBSCRIPTION_ProcessAllEventCompleteSubscriptions(ecm->event_name, ecm->output_args);
            break;

        case kDmExecMsg_OperStatus:
            osm = &msg.params.oper_status;
            USP_ASSERT(osm->status != NULL);
            DEVICE_REQUEST_UpdateOperationStatus(osm->instance, osm->status);
            break;


        case kDmExecMsg_ObjAdded:
            oam = &msg.params.obj_added;
            err = DATA_MODEL_NotifyInstanceAdded(oam->path);
            if (err == USP_ERR_OK)
            {
                // Send Object creation notifications if object existed in the schema
                DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(oam->path, kSubNotifyType_ObjectCreation);
            }
            break;

        case kDmExecMsg_ObjDeleted:
            odm = &msg.params.obj_deleted;
            DATA_MODEL_NotifyInstanceDeleted(odm->path);
            break;

        case kDmExecMsg_DoWork:
            dwm = &msg.params.do_work;
            dwm->do_work_cb(dwm->arg1, dwm->arg2);
            break;

        case kDmExecMsg_MtpThreadExited:
            tem = &msg.params.mtp_thread_exited;
            cumulative_mtp_threads_exited |= tem->flags;

            // Form bitmask of all MTP threads which must exit before a scheduled exit can be handled
            #ifndef DISABLE_STOMP
            all_mtp_exited |= STOMP_EXITED;
            #endif

            #ifdef ENABLE_COAP
            all_mtp_exited |= COAP_EXITED;
            #endif

            #ifdef ENABLE_MQTT
            all_mtp_exited |= MQTT_EXITED;
            #endif

            #ifdef ENABLE_WEBSOCKETS
            all_mtp_exited |= WSCLIENT_EXITED;
            all_mtp_exited |= WSSERVER_EXITED;
            #endif

            #ifdef ENABLE_UDS
            all_mtp_exited |= UDS_EXITED;
            #endif

            #ifndef REMOVE_DEVICE_BULKDATA
            all_mtp_exited |= BDC_EXITED;
            #endif

            if (cumulative_mtp_threads_exited == all_mtp_exited)
            {
                DM_EXEC_HandleScheduledExit();
            }
            break;

#ifndef REMOVE_DEVICE_BULKDATA
        case kDmExecMsg_BdcTransferResult:
            {
                bdc_transfer_result_msg_t *btr;
                btr = &msg.params.bdc_transfer_result;
                DEVICE_BULKDATA_NotifyTransferResult(btr->profile_id, btr->transfer_result);
            }
            break;
#endif

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
        case kDmExecMsg_E2eSessionEvent:
        {
            e2e_event_msg_t *eem = &msg.params.e2e_event;
            USP_ERR_ClearMessage();
            E2E_CONTEXT_E2eSessionEvent(eem->event, eem->request_instance, eem->controller_instance);
            break;
        }
#endif

        default:
            TERMINATE_BAD_CASE(msg.type);
            break;
    }

    // Free all dynamically allocated memory owned by the message's arguments
    FreeDmExecMessageArguments(&msg);
}

#ifdef ENABLE_UDS
/*********************************************************************//**
**
** HandleUdsHandshakeComplete
**
** Called after a USP endpoint has connected over UDS MTP
**
** \param   endpoint_id - endpoint that we've connected to
** \param   path_type - whether the endpoint is connected to the Broker's Controller or the Broker's Agent socket
** \param   conn_id - Unique identifier for the connection to the endpoint
**
** \return  None
**
**************************************************************************/
void HandleUdsHandshakeComplete(char *endpoint_id, uds_path_t path_type, unsigned conn_id)
{
#ifndef REMOVE_USP_BROKER
    // If running as USP Broker then...
    if (RUNNING_AS_USP_SERVICE() == false)
    {
        // Add the endpoint into the USP Service table
        mtp_conn_t mtpc;
        memset(&mtpc, 0, sizeof(mtpc));
        mtpc.protocol = kMtpProtocol_UDS;
        mtpc.is_reply_to_specified = true;
        mtpc.uds.conn_id = conn_id;
        mtpc.uds.path_type = path_type;
        USP_BROKER_AddUspService(endpoint_id, &mtpc);  // Deliberately ignoring the error - there's nothing more we can do than log it

        // Add the endpoint into the Controller table if it connected to the Broker's agent socket
        // NOTE: This also ensures that the controller's inherited role is full access
        if (path_type == kUdsPathType_BrokersAgent)
        {
            int uds_instance;
            uds_instance = UDS_GetInstanceForConnection(conn_id);
            if (uds_instance != INVALID)
            {
                DEVICE_CONTROLLER_AddController_UDS(endpoint_id, uds_instance);
            }
        }
    }
#endif

#ifndef REMOVE_USP_SERVICE
    // If running as a USP Service then...
    if (RUNNING_AS_USP_SERVICE()==true)
    {
        // Add the Broker's endpoint into the Controller table. This is necessary to accept messages from it, regardless of
        // whether the messages are from its agent or controller
        // NOTE: This also ensures that the controller's inherited role is full access
        int uds_instance;
        uds_instance = UDS_GetInstanceForConnection(conn_id);
        if (uds_instance != INVALID)
        {
            DEVICE_CONTROLLER_AddController_UDS(endpoint_id, uds_instance);
        }

        // Send a Register request if just connected to the Broker's Controller, and there are some objects to register
        if ((*usp_service_objects != '\0') && (path_type == kUdsPathType_BrokersController))
        {
            USP_SERVICE_QueueRegisterRequest(endpoint_id, usp_service_objects);
        }
    }

    if (path_type == kUdsPathType_BrokersAgent)
    {
        // Save the Broker's endpoint_id and MTP connection parameters for use by the USP Service when acting as a controller
        mtp_conn_t mtp_conn;
        memset(&mtp_conn, 0, sizeof(mtp_conn_t));
        mtp_conn.protocol = kMtpProtocol_UDS;
        mtp_conn.is_reply_to_specified = true;
        mtp_conn.uds.conn_id = conn_id;
        mtp_conn.uds.path_type = path_type;

        USP_SERVICE_SetBrokerAgent(endpoint_id, &mtp_conn);
    }

#endif

    DM_EXEC_EnableNotifications();
}

/*********************************************************************//**
**
** HandleUdsDisconnected
**
** Called after a USP endpoint has disconnected from UDS MTP
**
** \param   endpoint_id - endpoint that disconnected
** \param   path_type - whether the endpoint was connected to the Broker's Controller or the Broker's Agent socket
**
** \return  None
**
**************************************************************************/
void HandleUdsDisconnected(char *endpoint_id, uds_path_t path_type)
{
#ifndef REMOVE_USP_BROKER
    // If running as USP Broker then handle the disconnection
    if (RUNNING_AS_USP_SERVICE() == false)
    {
        // If it was connected to the Broker's controller socket, then remove all parts of the data model it registered
        // If it was connected to the Broker's agent socket, and is still connected to the Broker's controller socket, then just reset the has_controller flag in the USP Service table
        // If it is not connected to either the Broker's agent or Broker's controller socket, then remove it from the USP Service table
        USP_BROKER_HandleUspServiceDisconnect(endpoint_id, path_type);

        // NOTE: We do not remove the endpoint from the Controller table, as it's anticipated that it may reconnect
    }
#endif

#ifndef REMOVE_USP_SERVICE
    // If running as a USP Service then...
    if (RUNNING_AS_USP_SERVICE()==true)
    {
    }
#endif

}
#endif // ENABLE_UDS

/*********************************************************************//**
**
** Frees all dynamically allocated memory owned by the specified mq message
**
** \param   msg - pointer to message owning dynamically allocated arguments to free
**
** \return  None (any errors that occur are handled internally)
**
**************************************************************************/
void FreeDmExecMessageArguments(dm_exec_msg_t *msg)
{
    kv_vector_t *args;

    switch(msg->type)
    {
        case kDmExecMsg_ProcessUspRecord:
            USP_FREE(msg->params.usp_record.pbuf);
            USP_FREE(msg->params.usp_record.originator);
            DM_EXEC_FreeMTPConnection(&msg->params.usp_record.mtp_conn);
            break;

#ifndef DISABLE_STOMP
        case kDmExecMsg_StompHandshakeComplete:
            USP_SAFE_FREE(msg->params.stomp_complete.agent_queue);
            break;
#endif

#ifdef ENABLE_MQTT
        case kDmExecMsg_MqttHandshakeComplete:
            USP_SAFE_FREE(msg->params.mqtt_complete.client_id);
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kDmExecMsg_WebsockHandshakeComplete:
            // Nothing to free for this message
            break;
#endif

#ifdef ENABLE_UDS
        case kDmExecMsg_UdsHandshakeComplete:
            USP_SAFE_FREE(msg->params.uds_complete.endpoint_id);
            break;

        case kDmExecMsg_UdsDisconnected:
            USP_SAFE_FREE(msg->params.uds_disconnected.endpoint_id);
            break;
#endif

        case kDmExecMsg_OperComplete:
            args = msg->params.oper_complete.output_args;
            if (args != NULL)
            {
                KV_VECTOR_Destroy(args);
                USP_SAFE_FREE(args);
            }

            USP_SAFE_FREE(msg->params.oper_complete.err_msg);
            break;

        case kDmExecMsg_EventComplete:
            args = msg->params.event_complete.output_args;
            if (args != NULL)
            {
                KV_VECTOR_Destroy(args);
                USP_SAFE_FREE(args);
            }

            USP_SAFE_FREE(msg->params.event_complete.event_name);
            break;

        case kDmExecMsg_OperStatus:
            USP_FREE(msg->params.oper_status.status);
            break;


        case kDmExecMsg_ObjAdded:
            USP_FREE(msg->params.obj_added.path);
            break;

        case kDmExecMsg_ObjDeleted:
            USP_FREE(msg->params.obj_deleted.path);
            break;

        case kDmExecMsg_MtpThreadExited:
            break;

        case kDmExecMsg_BdcTransferResult:
            break;

        case kDmExecMsg_DoWork:
            break;

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
        case kDmExecMsg_E2eSessionEvent:
            break;
#endif

        default:
            TERMINATE_BAD_CASE(msg->type);
            break;
    }
}

#ifndef REMOVE_USP_BROKER
/*********************************************************************//**
**
** ForwardPendingMessagesOnFilterQueue
**
** Forward all messages on the filter message queue to the main message queue
** This is necessary to cope with any pending messages in the filter queue that
** have not been processed by DM_EXEC_SendAndProcessResponse()
**
** \param   set - pointer to socket set structure containing sockets with activity on them
**
** \return  None (any errors that occur are handled internally)
**
**************************************************************************/
void ForwardPendingMessagesOnFilterQueue(socket_set_t *set)
{
    int bytes_read;
    int bytes_sent;
    int num_sockets;
    dm_exec_msg_t  msg;

    // Form a socket set to only poll for activity on the filter message queue
    SOCKET_SET_Clear(set);
    SOCKET_SET_AddSocketToReceiveFrom(filter_mq_rx_socket, 0, set);
    SOCKET_SET_UpdateTimeout(0, set);

    num_sockets = SOCKET_SET_Select(set);
    while ((num_sockets == 1) && (SOCKET_SET_IsReadyToRead(filter_mq_rx_socket, set)))
    {
        // Exit if unable to read the full message received
        bytes_read = recv(filter_mq_rx_socket, &msg, sizeof(msg), 0);
        if (bytes_read != sizeof(msg))
        {
            USP_LOG_Error("%s: recv() did not return a full message", __FUNCTION__);
            return;
        }

        // Forward the message to the main queue - does not block if the queue is full, discards the message instead
        bytes_sent = send(main_mq_tx_socket, &msg, sizeof(msg), MSG_DONTWAIT);
        if (bytes_sent != sizeof(msg))
        {
            char buf[USP_ERR_MAXLEN];
            USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
            USP_LOG_Error("%s: Discarding received USP Record", __FUNCTION__);
            FreeDmExecMessageArguments(&msg);
            return;
        }

        // Determine if there are any futher messages pending
        num_sockets = SOCKET_SET_Select(set);
    }
}

/*********************************************************************//**
**
** IsMatchingMsgId
**
** Determines whether this is the response message that we're waiting for
**
** \param   msg - pointer to message from message queue
** \param   msg_id - message_id of the response message that we are waiting for
** \param   responder - endpoint_id which we expect to receive the response from
** \param   header_type - Type of response message in the header that we are expecting
** \param   is_handled - pointer to variable in which to return whether the message was handled here or not
**
** \return  pointer to unpacked USP message strcture, if this is the response message that we're waiting for
**          otherwise NULL if this is not the response message that we're waiting for, or an error occurred
**          NOTE that if NULL, the message may have been handled here or not, depending on the return value of is_handled
**
**************************************************************************/
Usp__Msg *IsMatchingMsgId(dm_exec_msg_t *msg, char *msg_id, char *responder, Usp__Header__MsgType header_type, bool *is_handled)
{
    process_usp_record_msg_t *pur;
    UspRecord__Record *rec = NULL;
    ProtobufCBinaryData *payload;
    Usp__Msg *usp = NULL;
    bool is_match = false;
    char *local_endpoint_id;
    combined_role_t combined_role;

    // Exit if this message is not a USP Record
    *is_handled = false;
    if (msg->type != kDmExecMsg_ProcessUspRecord)
    {
        goto exit;
    }

    // Exit if unable to unpack the USP record
    pur = &msg->params.usp_record;
    rec = usp_record__record__unpack(pbuf_allocator, pur->pbuf_len, pur->pbuf);
    if (rec == NULL)
    {
        goto exit;
    }

    // Exit if this record is not supposed to be processed by us
    local_endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    if ((rec->to_id == NULL) || (strcmp(rec->to_id, local_endpoint_id) != 0))
    {
        goto exit;
    }

    // Exit if this USP Record contains the invalid combination of encrypted payload carried in non-Session context
    // NOTE: This more specific check must come before the more general test for encrypted payload, otherwise we wouldn't detect it
    if ((rec->payload_security != USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT) &&
        (rec->record_type_case != USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT))
    {
        goto exit;
    }

    // Exit if this record contains an encrypted payload (which we don't yet support).
    if (rec->payload_security != USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT)
    {
        goto exit;
    }

    // Exit if this record was not a No-session-context
    if ((rec->record_type_case != USP_RECORD__RECORD__RECORD_TYPE_NO_SESSION_CONTEXT) || (rec->no_session_context == NULL))
    {
        goto exit;
    }

    // Exit if this record had no payload
    payload = &rec->no_session_context->payload;
    if ((payload->data == NULL) || (payload->len == 0))
    {
        goto exit;
    }

    // Exit if unable to unpack the USP message
    usp = usp__msg__unpack(pbuf_allocator, payload->len, payload->data);
    if (usp == NULL)
    {
        goto exit;
    }

    // Exit if this USP message can be handled by passthru to a USP Service
    DEVICE_CONTROLLER_GetCombinedRoleByEndpointId(rec->from_id, pur->role_instance, pur->mtp_conn.protocol, &combined_role);
    *is_handled = USP_BROKER_AttemptPassthru(usp, rec->from_id, &pur->mtp_conn, &combined_role, rec);
    if (*is_handled==true)
    {
        goto exit;
    }

    // Exit if this is not the message_id that we're waiting for, or we received it from the wrong endpoint
    if ((usp->header->msg_id == NULL) || (strcmp(usp->header->msg_id, msg_id) != 0) || (strcmp(rec->from_id, responder) !=0))
    {
        goto exit;
    }

    // Exit if this is not the response message that we're expecting or is not an error response
    if ((usp->header == NULL) ||
        ((usp->header->msg_type != header_type) && (usp->header->msg_type != USP__HEADER__MSG_TYPE__ERROR)) )
    {
        goto exit;
    }

    // If the code gets here, then the message_id matches, so log it
    is_match = true;
    PROTO_TRACE_ProtobufMessage(&rec->base);
    PROTO_TRACE_ProtobufMessage(&usp->base);



exit:
    // Always free the USP record structure
    if (rec != NULL)
    {
        usp_record__record__free_unpacked(rec, pbuf_allocator);
    }

    // Return the USP message, if it matched
    if (is_match)
    {
        return usp;
    }

    // Otherwise, since it didn't match, free the USP message structure, and return NULL
    usp__msg__free_unpacked(usp, pbuf_allocator);

    return NULL;
}
#endif  // REMOVE_USP_BROKER

