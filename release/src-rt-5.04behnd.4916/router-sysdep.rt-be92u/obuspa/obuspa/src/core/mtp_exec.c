/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020, BT PLC
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
 * \file mtp_exec.c
 *
 * Main loop for MTP thread dealing with STOMP and CoAP Communications
 *
 */

#include <string.h>
#include <sys/socket.h>
#include <errno.h>

#include "common_defs.h"
#include "mtp_exec.h"
#include "dm_exec.h"
#include "os_utils.h"
#include "msg_handler.h"

#ifndef DISABLE_STOMP
#include "stomp.h"
#endif

#ifdef ENABLE_COAP
#include "usp_coap.h"
#endif

#ifdef ENABLE_MQTT
#include "mqtt.h"
#endif

#ifdef ENABLE_WEBSOCKETS
#include "wsclient.h"
#include "wsserver.h"
#endif

#ifdef ENABLE_UDS
#include "uds.h"
#endif

//------------------------------------------------------------------------------
// Enumeration that is set when a USP Agent stop has been scheduled (for when connections have finished sending and receiving messages)
scheduled_action_t mtp_exit_scheduled = kScheduledAction_Off;

//------------------------------------------------------------------------------
// Flag set if any MTP has a reconnect/disconnect/resubscribe action signalled.
// Whilst this flag is not strictly necessary (as each MTP contains reconnect/disconnect/resubscribe flags), it's use prevents the
// data model thread from having to wait on the MTP thread's mutex under normal conditions (ie when a reconnect/disconnect/resubscribe
// is not scheduled). This is advantageous because the MTP thread may be blocked for a while performing a connect, and we don't want
// that to hold up the data model thread when none of the MTPs actually need to reconnect/disconnect/resubscribe.
// NOTE: This flag is only accessed from the data model thread.
bool mtp_reconnect_scheduled = false;

#ifndef DISABLE_STOMP
//------------------------------------------------------------------------------
// Unix domain socket pair used to implement a wakeup message queue
// One socket is always used for sending, and the other always used for receiving
static int mtp_stomp_mq_sockets[2] = {-1, -1};

#define mq_stomp_rx_socket  mtp_stomp_mq_sockets[0]
#define mq_stomp_tx_socket  mtp_stomp_mq_sockets[1]

//------------------------------------------------------------------------------
// Flag set to true if the MTP thread has exited
// This gets set after a scheduled exit due to a stop command, Reboot or FactoryReset operation
bool is_stomp_mtp_thread_exited = false;
#endif

#ifdef ENABLE_COAP
//------------------------------------------------------------------------------
// Unix domain socket pair used to implement a wakeup message queue
// One socket is always used for sending, and the other always used for receiving
static int mtp_coap_mq_sockets[2] = {-1, -1};

#define mq_coap_rx_socket  mtp_coap_mq_sockets[0]
#define mq_coap_tx_socket  mtp_coap_mq_sockets[1]

//------------------------------------------------------------------------------
// Flag set to true if the MTP thread has exited
// This gets set after a scheduled exit due to a stop command, Reboot or FactoryReset operation
bool is_coap_mtp_thread_exited = false;
#endif

#ifdef ENABLE_MQTT
//------------------------------------------------------------------------------
// Unix domain socket pair used to implement a wakeup message queue
// One socket is always used for sending, and the other always used for receiving
static int mtp_mqtt_mq_sockets[2] = {-1, -1};

#define mq_mqtt_rx_socket  mtp_mqtt_mq_sockets[0]
#define mq_mqtt_tx_socket  mtp_mqtt_mq_sockets[1]

//------------------------------------------------------------------------------
// Flag set to true if the MTP thread has exited
// This gets set after a scheduled exit due to a stop command, Reboot or FactoryReset operation
bool is_mqtt_mtp_thread_exited = false;
#endif

#ifdef ENABLE_UDS
//------------------------------------------------------------------------------
// Unix domain socket pair used to implement a wakeup message queue
// One socket is always used for sending, and the other always used for receiving
static int mtp_uds_mq_sockets[2] = {-1, -1};
#define mq_uds_rx_socket  mtp_uds_mq_sockets[0]
#define mq_uds_tx_socket  mtp_uds_mq_sockets[1]

//------------------------------------------------------------------------------
// Flag set to true if the MTP thread has exited
// This gets set after a scheduled exit due to a stop command, Reboot or FactoryReset operation
bool is_uds_mtp_thread_exited = false;
#endif

//------------------------------------------------------------------------------
// Message to post on wakeup message queue
#define WAKEUP_MESSAGE 'W'

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ProcessMtpWakeupQueueSocketActivity(socket_set_t *set, int sock);

/*********************************************************************//**
**
** MTP_EXEC_MtpSendItem_Init
**
** Initialises the mtp_send_item_t struct with default values
**
** \param   msi - struct to initialize
**
** \return  None
**
**************************************************************************/
void MTP_EXEC_MtpSendItem_Init(mtp_send_item_t *msi)
{
    msi->content_type = kMtpContentType_DisconnectRecord;
    msi->usp_msg_type = INVALID_USP_MSG_TYPE;
    msi->pbuf = NULL;
    msi->pbuf_len = 0;
}

/*********************************************************************//**
**
** MTP_EXEC_Init
**
** Initialises the functionality in this module
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MTP_EXEC_Init(void)
{
    int err;

    // Avoid compilation error, if no MTPs are enabled
    (void)err;

#ifndef DISABLE_STOMP
    // Exit if unable to initialize the unix domain socket pair used to implement a wakeup message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, mtp_stomp_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair", errno);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

#ifdef ENABLE_COAP
    // Exit if unable to initialize the unix domain socket pair used to implement a wakeup message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, mtp_coap_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair", errno);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

#ifdef ENABLE_MQTT
    // Exit if unable to initialize the unix domain socket pair used to implement a wakeup message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, mtp_mqtt_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair", errno);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

#ifdef ENABLE_UDS
    // Exit if unable to initialize the unix domain socket pair used to implement a wakeup message queue
    err = socketpair(AF_UNIX, SOCK_DGRAM, 0, mtp_uds_mq_sockets);
    if (err != 0)
    {
        USP_ERR_ERRNO("socketpair", errno);
        return USP_ERR_INTERNAL_ERROR;
    }
#endif

    return USP_ERR_OK;
}

#ifndef DISABLE_STOMP
/*********************************************************************//**
**
** MTP_EXEC_StompWakeup
**
** Posts a message on the STOMP MTP thread's queue, to cause it to wakeup from the select()
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void MTP_EXEC_StompWakeup(void)
{
    char msg = WAKEUP_MESSAGE;
    int bytes_sent;

    // Send the message
    bytes_sent = send(mq_stomp_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        return;
    }
}
#endif

#ifdef ENABLE_UDS
/*********************************************************************//**
**
** MTP_EXEC_UdsWakeup
**
** Posts a message on the UDS MTP thread's queue, to cause it to wakeup from the select()
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void MTP_EXEC_UdsWakeup(void)
{
    char msg = WAKEUP_MESSAGE;
    int bytes_sent;

    // Send the message
    bytes_sent = send(mq_uds_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        return;
    }
}
#endif

#ifdef ENABLE_COAP
/*********************************************************************//**
**
** MTP_EXEC_CoapWakeup
**
** Posts a message on the CoAP MTP thread's queue, to cause it to wakeup from the select()
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void MTP_EXEC_CoapWakeup(void)
{
    char msg = WAKEUP_MESSAGE;
    int bytes_sent;

    // Send the message
    bytes_sent = send(mq_coap_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        return;
    }
}
#endif

#ifdef ENABLE_MQTT
/*********************************************************************//**
**
** MTP_EXEC_MqttWakeup
**
** Posts a message on the MQTT MTP thread's queue, to cause it to wakeup from the select()
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void MTP_EXEC_MqttWakeup(void)
{
    char msg = WAKEUP_MESSAGE;
    int bytes_sent;

    // Send the message
    bytes_sent = send(mq_mqtt_tx_socket, &msg, sizeof(msg), 0);
    if (bytes_sent != sizeof(msg))
    {
        char buf[USP_ERR_MAXLEN];
        USP_LOG_Error("%s(%d): send failed : (err=%d) %s", __FUNCTION__, __LINE__, errno, USP_ERR_ToString(errno, buf, sizeof(buf)) );
        return;
    }
}
#endif
/*********************************************************************//**
**
** MTP_EXEC_ScheduleExit
**
** Signals that the CPE should exit USP Agent when all queued messages have been sent
** This is also used as part of scheduling a reboot
** See comment header above definition of scheduled_action_t for an explanation of how scheduled actions work, and why
**
** \param   None
**
** \return  None
**
**************************************************************************/
void MTP_EXEC_ScheduleExit(void)
{
    mtp_exit_scheduled = kScheduledAction_Signalled;
}

/*********************************************************************//**
**
** MTP_EXEC_ActivateScheduledActions
**
** Called when all USP response messages have been queued.
** This function activates all scheduled actions which have been signalled
** See comment header above definition of scheduled_action_t for an explanation of how scheduled actions work, and why
**
** \param   None
**
** \return  None
**
**************************************************************************/
void MTP_EXEC_ActivateScheduledActions(void)
{
    bool any_mtp_exited = 0;

#ifndef DISABLE_STOMP
    any_mtp_exited = any_mtp_exited || is_stomp_mtp_thread_exited;
#endif
#ifdef ENABLE_COAP
    any_mtp_exited = any_mtp_exited || is_coap_mtp_thread_exited;
#endif
#ifdef ENABLE_MQTT
    any_mtp_exited = any_mtp_exited || is_mqtt_mtp_thread_exited;
#endif
#ifdef ENABLE_WEBSOCKETS
    any_mtp_exited = any_mtp_exited || is_wsclient_mtp_thread_exited;
#endif
#ifdef ENABLE_UDS
    any_mtp_exited = any_mtp_exited || is_uds_mtp_thread_exited;
#endif


    // Exit if any MTP thread has already exited (because if they have, there is no need to schedule any further actions)
    if (any_mtp_exited)
    {
        return;
    }

    // Activate the exit action, if signalled
    if (mtp_exit_scheduled == kScheduledAction_Signalled)
    {
        mtp_exit_scheduled = kScheduledAction_Activated;
#ifndef DISABLE_STOMP
        MTP_EXEC_StompWakeup();
#endif
#ifdef ENABLE_COAP
        MTP_EXEC_CoapWakeup();
#endif
#ifdef ENABLE_MQTT
        MTP_EXEC_MqttWakeup();
#endif
#ifdef ENABLE_WEBSOCKETS
        // Unlike other MTPs, no need to wakeup Websocket client or websocket server thread,
        // because the call to XXX_ActivateScheduledActions() later in this function, causes the thread to wake up
#endif

#ifdef ENABLE_UDS
        MTP_EXEC_UdsWakeup();
#endif

        // Ensure that exit still occurs, if no MTPs are compiled into the code
#ifdef DISABLE_STOMP
#ifndef ENABLE_COAP
#ifndef ENABLE_MQTT
#ifndef ENABLE_UDS
        DM_EXEC_HandleScheduledExit();
#endif
#endif
#endif
#endif
    }

    // Activate all scheduled reconnects, if any were signalled
    if (mtp_reconnect_scheduled)
    {
#ifndef DISABLE_STOMP
        STOMP_ActivateScheduledActions();
#endif
#ifdef ENABLE_MQTT
        MQTT_ActivateScheduledActions();
#endif
#ifdef ENABLE_WEBSOCKETS
        WSSERVER_ActivateScheduledActions();
#endif
#ifdef ENABLE_UDS
        UDS_ActivateScheduledActions();
#endif
        mtp_reconnect_scheduled = false;
    }

#ifdef ENABLE_WEBSOCKETS
    // NOTE: It's not possible to use the mtp_reconnect_scheduled flag with WSCLIENT, because kScheduledAction_Signalled is set
    // in the wsclient thread and mtp_reconnect_scheduled is modified only by the data model thread
    WSCLIENT_ActivateScheduledActions();
#endif
}

#ifndef DISABLE_STOMP
/*********************************************************************//**
**
** MTP_EXEC_StompMain
**
** Main loop of MTP thread for STOMP
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *MTP_EXEC_StompMain(void *args)
{
    int num_sockets;
    socket_set_t set;

    while(FOREVER)
    {
        // Create the set of all sockets to receive/transmit on (with timeout)
        SOCKET_SET_Clear(&set);
        STOMP_UpdateAllSockSet(&set);
        SOCKET_SET_AddSocketToReceiveFrom(mq_stomp_rx_socket, MAX_SOCKET_TIMEOUT, &set);

        // Wait for read/write activity on sockets or timeout
        num_sockets = SOCKET_SET_Select(&set);

        // Process socket activity
        switch(num_sockets)
        {
            case -1:
                // An unrecoverable error has occurred
                USP_LOG_Error("%s: Unrecoverable socket select() error. Aborting MTP thread", __FUNCTION__);
                return NULL;
                break;

            case 0:
                // No controllers with any activity, but we still may need to process a timeout, so fall-through
            default:
                // Process the wakeup queue
                ProcessMtpWakeupQueueSocketActivity(&set, mq_stomp_rx_socket);

                // Process activity on all STOMP message queues
                STOMP_ProcessAllSocketActivity(&set);
                break;
        }

        // Exit this thread, if an exit is scheduled and all responses have been sent
        if (mtp_exit_scheduled == kScheduledAction_Activated)
        {
            if (STOMP_AreAllResponsesSent())
            {
                // Free all memory associated with MTP layer
                STOMP_Destroy();

                // Signal the data model thread that this thread has exited
                DM_EXEC_PostMtpThreadExited(STOMP_EXITED);
                return NULL;
            }
        }
    }
}
#endif

#ifdef ENABLE_MQTT
/*********************************************************************//**
**
** MTP_EXEC_MqttMain
**
** Main loop of MTP thread for MQTT
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *MTP_EXEC_MqttMain(void *args)
{
    int num_sockets;
    socket_set_t set;

    while(FOREVER)
    {
        // Create the set of all sockets to receive/transmit on (with timeout)
        SOCKET_SET_Clear(&set);
        MQTT_UpdateAllSockSet(&set);
        SOCKET_SET_AddSocketToReceiveFrom(mq_mqtt_rx_socket, MAX_SOCKET_TIMEOUT, &set);

        // Wait for read/write activity on sockets or timeout
        num_sockets = SOCKET_SET_Select(&set);

        // Process socket activity
        switch(num_sockets)
        {
            case -1:
                // An unrecoverable error has occurred
                USP_LOG_Error("%s: Unrecoverable socket select() error. Aborting MTP thread", __FUNCTION__);
                return NULL;
                break;
            case 0:
                // No controllers with any activity, but we still may need to process a timeout, so fall-through
            default:
                // Process the wakeup queue
                ProcessMtpWakeupQueueSocketActivity(&set, mq_mqtt_rx_socket);

                // Process activity on all MQTT message queues
                MQTT_ProcessAllSocketActivity(&set);
                break;
        }

        // Exit this thread, if an exit is scheduled and all responses have been sent
        if (mtp_exit_scheduled == kScheduledAction_Activated)
        {
            if (MQTT_AreAllResponsesSent())
            {
                // Free all memory associated with MTP layer
                MQTT_Destroy();

                // Signal the data model thread that this thread has exited
                DM_EXEC_PostMtpThreadExited(MQTT_EXITED);
                return NULL;
            }
        }
    }
}
#endif // ENABLE_MQTT

#ifdef ENABLE_COAP
/*********************************************************************//**
**
** MTP_EXEC_CoapMain
**
** Main loop of MTP thread for CoAP
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *MTP_EXEC_CoapMain(void *args)
{
    int num_sockets;
    socket_set_t set;

    while(FOREVER)
    {
        // Create the set of all sockets to receive/transmit on (with timeout)
        SOCKET_SET_Clear(&set);
        COAP_UpdateAllSockSet(&set);
        SOCKET_SET_AddSocketToReceiveFrom(mq_coap_rx_socket, MAX_SOCKET_TIMEOUT, &set);

        // Wait for read/write activity on sockets or timeout
        num_sockets = SOCKET_SET_Select(&set);

        // Process socket activity
        switch(num_sockets)
        {
            case -1:
                // An unrecoverable error has occurred
                USP_LOG_Error("%s: Unrecoverable socket select() error. Aborting MTP thread", __FUNCTION__);
                return NULL;
                break;

                break;

            case 0:
                // No controllers with any activity, but we still may need to process a timeout, so fall-through
            default:
                // Process the wakeup queue
                ProcessMtpWakeupQueueSocketActivity(&set, mq_coap_rx_socket);

                // Process activity on all COAP message queues
                COAP_ProcessAllSocketActivity(&set);
                break;
        }

        // Exit this thread, if an exit is scheduled and all responses have been sent
        if (mtp_exit_scheduled == kScheduledAction_Activated)
        {
            if (COAP_AreAllResponsesSent())
            {
                // Free all memory associated with MTP layer
                COAP_Destroy();

                // Signal the data model thread that this thread has exited
                DM_EXEC_PostMtpThreadExited(COAP_EXITED);
                return NULL;
            }
        }
    }
}
#endif // ENABLE_COAP

#ifdef ENABLE_UDS
/*********************************************************************//**
**
** MTP_EXEC_UdsMain
**
** Main loop of MTP thread for UDS
**
** \param   args - arguments (currently unused)
**
** \return  None
**
**************************************************************************/
void *MTP_EXEC_UdsMain(void *args)
{
    int num_sockets;
    socket_set_t set;

    while(FOREVER)
    {
        // Create the set of all sockets to receive/transmit on (with timeout)
        SOCKET_SET_Clear(&set);
        UDS_UpdateAllSockSet(&set);

        // Wait for read/write activity on sockets or timeout
        SOCKET_SET_AddSocketToReceiveFrom(mq_uds_rx_socket, MAX_SOCKET_TIMEOUT, &set);
        num_sockets = SOCKET_SET_Select(&set);

        // Process socket activity
        switch(num_sockets)
        {
            case -1:
                // An unrecoverable error has occurred
                USP_LOG_Error("%s: Unrecoverable socket select() error. Aborting MTP thread", __FUNCTION__);
                return NULL;
                break;

            case 0:
                // No controllers with any activity, but we still may need to process a timeout, so fall-through
            default:
                // Process the wakeup queue
                ProcessMtpWakeupQueueSocketActivity(&set, mq_uds_rx_socket);

                // Process activity on all UDS message queues
                UDS_ProcessAllSocketActivity(&set);
                break;
        }

        // Exit this thread, if an exit is scheduled and all responses have been sent
        if (mtp_exit_scheduled == kScheduledAction_Activated)
        {
            if (UDS_AreAllResponsesSent())
            {
                // Free all memory associated with MTP layer
                UDS_Destroy();

                // Signal the data model thread that this thread has exited
                DM_EXEC_PostMtpThreadExited(UDS_EXITED);
                return NULL;
            }
        }
    }
}

#endif

/*********************************************************************//**
**
** ProcessMtpWakeupQueueSocketActivity
**
** Processes any activity on the message queue receiving socket
** NOTE: There are separate sockets for STOMP and CoAP MTP tasks, but all use this function for processing
**
** \param   set - pointer to socket set structure containing sockets with activity on them
** \param   sock - socket on which the wakeup message is received
**
** \return  None (any errors that occur are handled internally)
**
**************************************************************************/
void ProcessMtpWakeupQueueSocketActivity(socket_set_t *set, int sock)
{
    int bytes_read;
    char msg;

    // Exit if there is no activity on the wakeup message queue socket
    if (SOCKET_SET_IsReadyToRead(sock, set) == 0)
    {
        return;
    }

    // Exit if unable to purge this wakeup message from the queue
    bytes_read = recv(sock, &msg, sizeof(msg), 0);
    if (bytes_read != sizeof(msg))
    {
        USP_LOG_Error("%s: recv() did not return a full message", __FUNCTION__);
        return;
    }

    // Throw the message away, it's only purpose is to break the select()
    USP_ASSERT(msg == WAKEUP_MESSAGE);
}
