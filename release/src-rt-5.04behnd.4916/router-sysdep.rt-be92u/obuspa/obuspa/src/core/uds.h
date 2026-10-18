/*
 *
 * Copyright (C) 2023-2024, Broadband Forum
 * Copyright (C) 2023-2024  CommScope, Inc
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


#ifndef UDS_H
#define UDS_H

#include "socket_set.h"
#include "mtp_exec.h"
#include "device.h"

//------------------------------------------------------------------------------
// Enumeration for Device.UnixDomainSockets.UnixDomainSocket.{i}.Mode
typedef enum
{
    kUdsConnType_Server = 0,
    kUdsConnType_Client = 1,
    kUdsConnType_Invalid = INVALID
} uds_connection_type_t;

// Structure containing configuration parameters for the agent's domain socket server
typedef struct
{
    int instance;               // domain socket instance from the datamodel
    char *path;                 // resource path in the URL to the domain socket server
    uds_path_t path_type;       // Specifies whether the Unix domain socket path is the USP Broker's agent or controller
    uds_connection_type_t mode; // server or client
} uds_conn_params_t;

//------------------------------------------------------------------------------
// Enumeration for type of UDS frame received. Do not modify the values of each entry - these are defined in the USP Specification
// NOTE: Frame type invalid has been added to set initial state of frame type and is not part of USP specification
// Likewise Echo and EchoResponse are not part of USP Specification
typedef enum
{
    kUdsFrameType_Invalid = INVALID,
    kUdsFrameType_Handshake = 1,
    kUdsFrameType_Error = 2,
    kUdsFrameType_UspRecord = 3,
} uds_frame_t;

//------------------------------------------------------------------------------
// API functions
int UDS_Init(void);
void *UDS(void *args);
int UDS_EnableConnection(uds_conn_params_t *config);
int UDS_DisableConnection(int instance);
void UDS_ScheduleReconnect(uds_conn_params_t *config);
void UDS_UpdateAllSockSet(socket_set_t *set);
void UDS_ProcessAllSocketActivity(socket_set_t *set);
int UDS_QueueBinaryMessage(mtp_send_item_t *msi, mtp_conn_t *mtpc, time_t expiry_time, uds_frame_t frame_type);
int UDS_GetMTPForEndpointId(char *endpoint_id, mtp_conn_t *mtpc);
int UDS_GetInstanceForConnection(unsigned conn_id);
void UDS_ActivateScheduledActions(void);
bool UDS_AreAllResponsesSent(void);
void UDS_Destroy(void);
char *UDS_PathTypeToString(uds_path_t path_type);
#endif // UDS_H
