/*
 *
 * Copyright (C) 2021-2022, Broadband Forum
 * Copyright (C) 2021  CommScope, Inc
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
 * \file wsclient.h
 *
 * Header file for WebSockets connection (agent acting as websocket client)
 *
 */
#ifndef WSCLIENT_H
#define WSCLIENT_H

#include "common_defs.h"
#include "usp-msg.pb-c.h"
#include "device.h"             // for mtp_reply_to_t


//------------------------------------------------------------------------------
// Structure containing WebSocket client configuration parameters
typedef struct
{
    char *host;                     // Host to send USP messages to the controller on
    unsigned port;                  // Port to send USP messages to the controller on
    char *path;                     // Path to send USP messages to the controller on
    bool enable_encryption;         // Set if the connection should use TLS
    unsigned keep_alive_interval;   // Number of seconds between transmitting Websock PING frames
    unsigned retry_interval;        // Interval constant to use when calculating the exponential backoff period, when retrying a connection
    unsigned retry_multiplier;      // Multiplier constant to use when calculating the exponential backoff period, when retrying a connection
} wsclient_config_t;

//------------------------------------------------------------------------------
// API
int WSCLIENT_Init(void);
void WSCLIENT_Destroy(void);
int WSCLIENT_Start(void);
void WSCLIENT_StartClient(int cont_instance, int mtp_instance, char *cont_endpoint_id, wsclient_config_t *config);
void WSCLIENT_StopClient(int cont_instance, int mtp_instance);
void WSCLIENT_ActivateScheduledActions(void);
void WSCLIENT_QueueBinaryMessage(mtp_send_item_t *msi, int cont_instance, int mtp_instance, time_t expiry_time);
unsigned WSCLIENT_GetRetryCount(int cont_instance, int mtp_instance);
void *WSCLIENT_Main(void *args);
bool WSCLIENT_IsEndpointConnected(char *endpoint_id);

//------------------------------------------------------------------------------
// Exported global variables
extern bool is_wsclient_mtp_thread_exited;

#endif

