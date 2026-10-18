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
 * \file mtp_exec.h
 *
 * Header file for MTP main execution loop
 *
 */
#ifndef MTP_EXEC_H
#define MTP_EXEC_H

#include <time.h>
#include <stdbool.h>

#include "vendor_defs.h"    // for E2ESESSION_EXPERIMENTAL_USP_V_1_2
#include "usp-msg.pb-c.h"

//-----------------------------------------------------------------------------------------------
// Enumeration of Device.LocalAgent.MTP.{i}.Status
typedef enum
{
    kMtpStatus_Error,
    kMtpStatus_Down,
    kMtpStatus_Up,
} mtp_status_t;

//------------------------------------------------------------------------------
// Enumeration for MTP protocol type. Only the types below are supported by the code
typedef enum
{
    kMtpProtocol_None,      // None setup yet. The default.
#ifndef DISABLE_STOMP
    kMtpProtocol_STOMP,
#endif
#ifdef ENABLE_COAP
    kMtpProtocol_CoAP,
#endif
#ifdef ENABLE_MQTT
    kMtpProtocol_MQTT,
#endif
#ifdef ENABLE_WEBSOCKETS
    kMtpProtocol_WebSockets,
#endif
#ifdef ENABLE_UDS
    kMtpProtocol_UDS,
#endif

    // The following enumeration should always be the last - it is used to size arrays
    kMtpProtocol_Max
} mtp_protocol_t;

//------------------------------------------------------------------------------
// Enumeration describing what the contents of pbuf are to MSG_HANDLER_LogMessageToSend()
typedef enum
{
    kMtpContentType_UspMessage,       // Protobuf encoded USP Record containing an unsegmented USP message. No session context.
    kMtpContentType_ConnectRecord,    // A STOMP, MQTT, WebSockets or UDS USP Connect record
    kMtpContentType_DisconnectRecord, // A USP Disconnect record that causes the MTP to disconnect (and retry)
#ifdef E2ESESSION_EXPERIMENTAL_USP_V_1_2
    kMtpContentType_E2E_SessTermination, // A USP Disconnect record that doesn't cause the MTP to disconnect (used to terminate an E2E session)
    kMtpContentType_E2E_FullMessage,  // USP Record containing a full USP message in a session context
                                      // NOTE: The reason this is differentiated from kMtpContentType_UspMessage is that this USP message is logged before sending
    kMtpContentType_E2E_Begin,        // USP Record containing the first fragment of a segmented USP message
    kMtpContentType_E2E_InProcess,    // USP Record containing a middle fragment of a segmented USP message
    kMtpContentType_E2E_Complete,     // USP Record containing the last fragment of a segmented USP message
#endif
} mtp_content_type_t;

//------------------------------------------------------------------------------
// Macro to determine whether the content is a connect or disconnect record
// Used to ensure there are no stale USP connect or disconnect records in the MTP's USP record message queue
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    #define IsUspConnectOrDisconnectRecord(type)  ((type == kMtpContentType_ConnectRecord) || (type == kMtpContentType_DisconnectRecord) || (type == kMtpContentType_E2E_SessTermination))
#else
    #define IsUspConnectOrDisconnectRecord(type)  ((type == kMtpContentType_ConnectRecord) || (type == kMtpContentType_DisconnectRecord))
#endif

//------------------------------------------------------------------------------
// Structure containing common elements about payload/body content to send by the MTP
typedef struct
{
    mtp_content_type_t content_type;    // Type of content in the payload.
    Usp__Header__MsgType usp_msg_type;  // USP Message type
    uint8_t *pbuf;                      // Payload to be sent by the MTP (USP Record)
    int pbuf_len;                       // Length of the payload
} mtp_send_item_t;

//------------------------------------------------------------------------------
// Structure containing a count of causes of connectivity failures for a particular MTP (eg STOMP, HTTP)
typedef struct
{
    time_t reset_time;  // Time at which the counters were reset. In the case of STOMP, it is also the time at which the 'TotalUpTime' was reset.
    unsigned dns;       // Count of number of times a connection failed due to unable to get IP address of server via DNS
    unsigned authentication; // Count of number of times the password or the SSL handshake was rejected
    unsigned connect;   // Count of number of times unable to connct to the server
    unsigned readwrite; // Count of number of times a connection failed due to unable to send/receive from server
    unsigned timeout;   // Count of number of times a connection failed due to a timeout. For STOMP, this is STOMP handshake or server heartbeat timeout. For BDC this is timed out getting HTTP response code
    unsigned other;     // Count of number of times that other errors caused the connection to fail. These are mainly protocol errors/unexpected data.
} mtp_failure_count_t;

//------------------------------------------------------------------------------
// Enumeration used to determine when to action a STOMP reconnect or MTP thread exit
// A reconnect is signalled by calling STOMP_ScheduleReconnect()
// An exit is signalled by calling MTP_EXEC_ScheduleExit()
// But neither of these functions activate a reconnect or exit in themselves, because if they did, the MTP
// thread might perform the action immediately, and we want all response messages to be sent before performing the action
// So, instead, only after the response message has been put on the message queue do we activate (by calling MTP_EXEC_ActivateScheduledActions)
// the actions. Once an action has been activated it is then scheduled to occur once all responses have been sent.
typedef enum
{
    kScheduledAction_Off,             // The action is not scheduled
    kScheduledAction_Signalled,       // The action is signalled but not activated (because a response message might need to be put in the message queue).
    kScheduledAction_Activated       // The action occurs when all queued USP response messages have been sent
} scheduled_action_t;

//------------------------------------------------------------------------------
// Global Variables
extern scheduled_action_t mtp_exit_scheduled;
extern bool mtp_reconnect_scheduled;
extern bool is_coap_mtp_thread_exited;
extern bool is_stomp_mtp_thread_exited;
extern bool is_mqtt_mtp_thread_exited;
extern bool is_uds_mtp_thread_exited;

//------------------------------------------------------------------------------
// API functions
void MTP_EXEC_MtpSendItem_Init(mtp_send_item_t *msi);
int MTP_EXEC_Init(void);
void MTP_EXEC_ScheduleExit(void);
void MTP_EXEC_ActivateScheduledActions(void);
#ifndef DISABLE_STOMP
void *MTP_EXEC_StompMain(void *args);
void MTP_EXEC_StompWakeup(void);
#endif
#ifdef ENABLE_COAP
void *MTP_EXEC_CoapMain(void *args);
void MTP_EXEC_CoapWakeup(void);
#endif
#ifdef ENABLE_MQTT
void *MTP_EXEC_MqttMain(void *args);
void MTP_EXEC_MqttWakeup(void);
#endif
#ifdef ENABLE_UDS
void *MTP_EXEC_UdsMain(void *args);
void MTP_EXEC_UdsWakeup(void);
#endif

//------------------------------------------------------------------------------

#endif
