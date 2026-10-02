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
 * \file wsserver.h
 *
 * Header file for WebSockets connection (agent acting as websocket server)
 *
 */
#ifndef WSSERVER_H
#define WSSERVER_H

#include "common_defs.h"
#include "usp-msg.pb-c.h"
#include "device.h"             // for mtp_reply_to_t
#include "mtp_exec.h"           // for mtp_status_t


//------------------------------------------------------------------------------
// Structure containing configuration parameters for the agent's websocket server
typedef struct
{
    unsigned port;              // port to listen on
    char *path;                 // resource path in the URL to the websocket server
    bool enable_encryption;     // set if the server should use TLS connections
    unsigned keep_alive;        // Keep alive interval (in seconds)
} wsserv_config_t;

//------------------------------------------------------------------------------
// API
int WSSERVER_Init(void);
int WSSERVER_Start(void);
void WSSERVER_QueueBinaryMessage(mtp_send_item_t *msi, int conn_id, time_t expiry_time);
void *WSSERVER_Main(void *args);
int WSSERVER_EnableServer(wsserv_config_t *config);
int WSSERVER_DisableServer(void);
void WSSERVER_ActivateScheduledActions(void);
mtp_status_t WSSERVER_GetMtpStatus();
void WSSERVER_DisconnectEndpoint(char *endpoint_id);
int WSSERVER_GetMTPForEndpointId(char *endpoint_id, mtp_reply_to_t *mrt);

#endif

