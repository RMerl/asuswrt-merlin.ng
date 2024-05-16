/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file stomp.h
 *
 * Header file for STOMP connection
 *
 */
#ifndef STOMP_H
#define STOMP_H

#include <time.h>
#include <openssl/bio.h>

#include "dllist.h"
#include "socket_set.h"
#include "mtp_exec.h"
#include "usp-msg.pb-c.h"

//------------------------------------------------------------------------------
// Structure containing the parameters controlling STOMP retries
typedef struct
{
    unsigned initial_interval;
    unsigned interval_multiplier;
    unsigned max_interval;
} stomp_retry_params_t;

//------------------------------------------------------------------------------
// Data model parameters for each stomp connection
typedef struct
{
    int instance;          // instance of this connection in the Device.STOMP.Connection.{i} table. Set to INVALID, if this entry is not used.
    bool enable;
    char *host;
    unsigned port;
    char *username;
    char *password;
    bool enable_encryption;
    char *virtual_host;
    bool enable_heartbeats;
    unsigned incoming_heartbeat_period;  // in ms. NOTE: the negotiated heartbeat_period is stored in seconds
    unsigned outgoing_heartbeat_period;  // in ms
    stomp_retry_params_t retry;      // parameters associated with retrying the connection after a failure
} stomp_conn_params_t;

//------------------------------------------------------------------------------
// API
int STOMP_Init(void);
void STOMP_Destroy(void);
int STOMP_Start(void);
void STOMP_UpdateAllSockSet(socket_set_t *set);
bool STOMP_AreAllResponsesSent(void);
void STOMP_ProcessAllSocketActivity(socket_set_t *set);
int STOMP_QueueBinaryMessage(mtp_send_item_t *msi, int instance, char *controller_queue, char *agent_queue, time_t expiry_time);
int STOMP_EnableConnection(stomp_conn_params_t *sp, char *stomp_queue);
int STOMP_DisableConnection(int instance, bool purge_queued_messages);
void STOMP_ScheduleReconnect(stomp_conn_params_t *sp, char *stomp_queue);
void STOMP_ActivateScheduledActions(void);
void STOMP_ScheduleResubscribe(int instance, char *stomp_queue);
mtp_status_t STOMP_GetMtpStatus(int instance);
char *STOMP_GetConnectionStatus(int instance, time_t *last_change_date);
void STOMP_UpdateRetryParams(int instance, stomp_retry_params_t *retry_params);
void STOMP_GetDestinationFromServer(int instance, char *buf, int len);


// Readability definitions for 'purge_queued_messages' argument of STOMP_StopConnection()
#define PURGE_QUEUED_MESSAGES true
#define DONT_PURGE_QUEUED_MESSAGES false

#endif

