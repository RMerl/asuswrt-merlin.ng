/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
 * Copyright (C) 2020, BT PLC
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
 * \file msg_handler.c
 *
 * Handles a message, parsing it, then actioning it. Potentially this could result in a message to send back to the controller.
 *
 */

#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "device.h"
#include "iso8601.h"
#include "proto_trace.h"
#include "text_utils.h"
#include "usp-record.pb-c.h"
#include "stomp.h"
#include "wsclient.h"
#include "msg_handler.h"
#include "usp_record.h"
#include "dm_exec.h"

#ifndef REMOVE_USP_BROKER
#include "usp_broker.h"
#endif

#ifndef REMOVE_USP_SERVICE
#include "usp_service.h"
#endif

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include <inttypes.h>  // For PRIu64
#include "e2e_context.h"
#endif

//------------------------------------------------------------------------
// Index of the controller that sent the current USP message being processed
// This is used to set the Originator in the Request table and used by E2E session context (each controller potentially has an associated E2E session)
// NOTE: In the case of USP Services this value may be INVALID if the current USP message being processed is from a USP Service acting as an agent
static int cur_msg_controller_instance = INVALID;

//------------------------------------------------------------------------
// Role to use with current USP message
// This is saved off before handling each message, as each message handler needs it fairly deeply in its processing
static combined_role_t cur_msg_combined_role = { 0, 0 };

//------------------------------------------------------------------------
// Role to use with current USP message
static controller_info_t cur_msg_controller_info;

//------------------------------------------------------------------------
// When processing a message, this is set to the type of message being processed
// It is consulted when processing an Add request, to prevent object creation notifications from being passed through before sending the Add Respnse
static int cur_msg_type = INVALID;

//------------------------------------------------------------------------
// Array used to convert from the USP Message type enumeration to it's string representation
static enum_entry_t usp_msg_types[] = {
    { USP__HEADER__MSG_TYPE__ERROR,            "ERROR"},
    { USP__HEADER__MSG_TYPE__GET,              "GET"},
    { USP__HEADER__MSG_TYPE__GET_RESP,         "GET_RESP"},
    { USP__HEADER__MSG_TYPE__NOTIFY,           "NOTIFY"},
    { USP__HEADER__MSG_TYPE__SET,              "SET"},
    { USP__HEADER__MSG_TYPE__SET_RESP,         "SET_RESP"},
    { USP__HEADER__MSG_TYPE__OPERATE,          "OPERATE"},
    { USP__HEADER__MSG_TYPE__OPERATE_RESP,     "OPERATE_RESP"},
    { USP__HEADER__MSG_TYPE__ADD,              "ADD"},
    { USP__HEADER__MSG_TYPE__ADD_RESP,         "ADD_RESP"},
    { USP__HEADER__MSG_TYPE__DELETE,           "DELETE"},
    { USP__HEADER__MSG_TYPE__DELETE_RESP,      "DELETE_RESP"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM, "GET_SUPPORTED_DM"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP, "GET_SUPPORTED_DM_RESP"},
    { USP__HEADER__MSG_TYPE__GET_INSTANCES,    "GET_INSTANCES"},
    { USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP, "GET_INSTANCES_RESP"},
    { USP__HEADER__MSG_TYPE__NOTIFY_RESP,      "NOTIFY_RESP"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO, "GET_SUPPORTED_PROTO"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP, "GET_SUPPORTED_PROTO_RESP"},
    { USP__HEADER__MSG_TYPE__REGISTER,         "REGISTER"},
    { USP__HEADER__MSG_TYPE__REGISTER_RESP,    "REGISTER_RESP"},
    { USP__HEADER__MSG_TYPE__DEREGISTER,       "DEREGISTER"},
    { USP__HEADER__MSG_TYPE__DEREGISTER_RESP,  "DEREGISTER_RESP"}
};

//------------------------------------------------------------------------------
// Array used to convert from the MTP content type enumeration to it's string representation
static enum_entry_t mtp_content_types[] = {
    { kMtpContentType_UspMessage,           "USP_MESSAGE" }, // Not actually used by MtpSendItemToString - the usp_msg_type[] is used instead
    { kMtpContentType_ConnectRecord,        "USP_CONNECT_RECORD" },
    { kMtpContentType_DisconnectRecord,     "USP_DISCONNECT_RECORD" },
#ifdef E2ESESSION_EXPERIMENTAL_USP_V_1_2
    { kMtpContentType_E2E_SessTermination,  "E2E_DISCONNECT_RECORD" },
    { kMtpContentType_E2E_FullMessage,      "E2E_FULL_MESSAGE" },
    { kMtpContentType_E2E_Begin,            "E2E_BEGIN" },
    { kMtpContentType_E2E_InProcess,        "E2E_INPROCESS" },
    { kMtpContentType_E2E_Complete,         "E2E_COMPLETE" },
#endif
};

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int HandleUspMessage(Usp__Msg *usp, char *endpoint, mtp_conn_t *mtpc);
int ValidateUspRecord(UspRecord__Record *rec, mtp_conn_t *mtpc);
char *MtpSendItemToString(mtp_send_item_t *msi);
int QueueUspNoSessionRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id, mtp_conn_t *mtpc, time_t expiry_time);
int ValidateUspMsgType(Usp__Header__MsgType msg_type, char *endpoint, mtp_conn_t *mtpc, char *msg_id);
void HandleGetSupportedDMResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
void HandleNotification(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);
void HandleUspError(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc);

/*********************************************************************//**
**
** MSG_HANDLER_HandleBinaryRecord
**
** Main entry point to handling an incoming USP Record (which encapsulates a USP Message)
** NOTE: Parsing errors are handled locally by this function
**
** \param   pbuf - pointer to buffer containing protobuf encoded USP record. Ownership of this buffer stays with the caller.
** \param   pbuf_len - length of protobuf encoded message
** \param   originator - EndpointID which sent this USP Record (if known) or UNKNOWN_ENDPOINT_ID
** \param   role_instance - Inherited role in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   mtpc - MTP details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_HandleBinaryRecord(unsigned char *pbuf, int pbuf_len, char *originator, int role_instance, mtp_conn_t *mtpc)
{
    int err = USP_ERR_OK;
    UspRecord__Record *rec = NULL;

    // Exit if unable to unpack the USP record, ignoring it as required by R-MTP.5
    rec = usp_record__record__unpack(pbuf_allocator, pbuf_len, pbuf);
    if (rec == NULL)
    {
        USP_ERR_SetMessage("%s: usp_record__record__unpack failed. Ignoring USP Record", __FUNCTION__);
        return USP_ERR_RECORD_NOT_PARSED;
    }

    // Exit if the originator of the message was known from the MTP, but is inconsistent with the USP Record from_id
    if ((originator != UNKNOWN_ENDPOINT_ID) && (strcmp(originator, rec->from_id) != 0))
    {
        USP_ERR_SetMessage("%s: Ignoring USP record with inconsistent endpoint (MTP endpoint=%s, from_id=%s)", __FUNCTION__, originator, rec->from_id);
        return USP_ERR_REQUEST_DENIED;
    }

#ifdef ENABLE_WEBSOCKETS
    // Exit if USP record received from the agent's websocket server is from a controller that is
    // already connected via the agent's websocket client (only one connection to a controller is allowed)
    if ((mtpc->protocol == kMtpProtocol_WebSockets) && (mtpc->ws.serv_conn_id != INVALID))
    {
        if (WSCLIENT_IsEndpointConnected(rec->from_id))
        {
            USP_ERR_SetMessage("%s: Not permitting controller eid='%s' to connect. Already connected via agent's websocket client", __FUNCTION__, rec->from_id);
            err = USP_ERR_REQUEST_DENIED;
            goto exit;
        }
    }
#endif

#if !defined(REMOVE_USP_BROKER) && !defined(REMOVE_USP_SERVICE)
    // USP Brokers and USP Services might receive USP Connect records. If so, just log them, then ignore them
    switch (rec->record_type_case)
    {
        case USP_RECORD__RECORD__RECORD_TYPE_WEBSOCKET_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_MQTT_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_STOMP_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_UDS_CONNECT:
            {
                char buf[MAX_ISO8601_LEN];
                USP_LOG_Info("USP_CONNECT_RECORD received at time %s, from endpoint_id=%s over %s",
                    iso8601_cur_time(buf, sizeof(buf)),
                    originator,
                    DEVICE_MTP_EnumToString(mtpc->protocol) );

                PROTO_TRACE_ProtobufMessage(&rec->base);
                err = USP_ERR_OK;
                goto exit;
            }
            break;

        default:
            break;
    }
#endif

    // Exit if USP record failed validation
    err = ValidateUspRecord(rec, mtpc);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Print USP Record header in human readable form
    PROTO_TRACE_ProtobufMessage(&rec->base);

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    // Process the USP Record through the End-to-End exchange context.
    err = E2E_CONTEXT_HandleUspRecord(rec, role_instance, mtpc);
#else
    // Process directly the encapsulated USP Message contained in the USP Record struct.
    err = MSG_HANDLER_HandleBinaryMessage(rec->no_session_context->payload.data,
                                          rec->no_session_context->payload.len,
                                          role_instance, rec->from_id, mtpc);
#endif

exit:
    // Free the unpacked USP record
    usp_record__record__free_unpacked(rec, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** MSG_HANDLER_HandleBinaryMessage
**
** Main entry point to handling a USP message
** NOTE: Parsing errors are handled locally by this function
**
** \param   pbuf - pointer to buffer containing protobuf encoded USP message
** \param   pbuf_len - length of protobuf encoded message
** \param   role_instance - Inherited role in Device.LocalAgent.ControllerTrust.Role.{i}
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful, USP_ERR_MESSAGE_NOT_UNDERSTOOD if unable to unpack the USP Record
**
**************************************************************************/
int MSG_HANDLER_HandleBinaryMessage(unsigned char *pbuf, int pbuf_len, int role_instance, char *endpoint_id, mtp_conn_t *mtpc)
{
    int err;
    Usp__Msg *usp;

    // Exit if unable to unpack the USP message. The failure is ignored according to R-MTP.5, because we cannot determine if this is a USP Request
    usp = usp__msg__unpack(pbuf_allocator, pbuf_len, pbuf);
    if (usp == NULL)
    {
        USP_ERR_SetMessage("%s: usp__msg__unpack failed. Ignoring USP Message", __FUNCTION__);
        return USP_ERR_MESSAGE_NOT_UNDERSTOOD;
    }

    // Set the role that the controller should use when handling this message
    cur_msg_controller_info.endpoint_id = endpoint_id;
    DEVICE_CONTROLLER_GetCombinedRoleByEndpointId(endpoint_id, role_instance, mtpc->protocol, &cur_msg_combined_role);

    // Print USP message in human readable form
    PROTO_TRACE_ProtobufMessage(&usp->base);

    // Exit if unable to process the message
    err = HandleUspMessage(usp, endpoint_id, mtpc);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // If code gets here, then it was successful
    err = USP_ERR_OK;

exit:
    // Free the unpacked USP message
    usp__msg__free_unpacked(usp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** MSG_HANDLER_LogMessageToSend
**
** Logs protobuf level protocol trace for the USP Record currently being sent out.
** If the USP Record contains a USP Message in a NonSessionContext payload, the Message is printed.
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is not passed to this function and stays with the caller.
** \param   protocol - MTP on which the USP Record is to be sent (for use by debug)
** \param   host - hostname of controller to send the USP Record to (for use by debug)
** \param   header - pointer to string containing extra STOMP/MQTT header info (NULL if not required)
**                         This is only used for debug purposes
**
** \return  None
**
**************************************************************************/
void MSG_HANDLER_LogMessageToSend(mtp_send_item_t *msi,
                                  mtp_protocol_t protocol, char *host,
                                  char *header)
{
    char buf[MAX_ISO8601_LEN];
    UspRecord__Record *rec;
    ProtobufCBinaryData *payload;
    Usp__Msg *msg;
    USP_ASSERT(msi != NULL);

    // Log the message
    USP_PROTOCOL("\n");
    USP_LOG_Info("%s sending at time %s, to host %s over %s",
                MtpSendItemToString(msi),
                iso8601_cur_time(buf, sizeof(buf)),
                host,
                DEVICE_MTP_EnumToString(protocol) );

    // Print STOMP header (if message is being sent out on STOMP)
    if ((enable_protocol_trace) && (header != NULL))
    {
        USP_PROTOCOL("%s", header);
    }

    // Unpack the USP record and log it
    rec = usp_record__record__unpack(pbuf_allocator, msi->pbuf_len, msi->pbuf);
    USP_ASSERT(rec != NULL);
    PROTO_TRACE_ProtobufMessage(&rec->base);

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    if (rec->record_type_case == USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT)
    {
        UspRecord__SessionContextRecord *ctx = rec->session_context;
        USP_ASSERT(ctx != NULL)
        USP_LOG_Info("within E2ESession Record(session_id=%"PRIu64", sequence_id=%"PRIu64", state=%s, n_payload=%zu)",
                     rec->session_context->session_id,
                     rec->session_context->sequence_id,
                     E2E_CONTEXT_SarStateToString(ctx->payload_sar_state),
                     rec->session_context->n_payload);
        // NOTE: The USP Message sent through E2ESession is not printed here, because already done in E2E_CONTEXT_QueueUspSessionRecord
    }
#endif

    // Unpack the encapsulated USP Message and log it (if not empty)
    if (rec->record_type_case == USP_RECORD__RECORD__RECORD_TYPE_NO_SESSION_CONTEXT)
    {
        USP_ASSERT(rec->no_session_context != NULL);
        payload = &rec->no_session_context->payload;
        if ((payload != NULL) && (payload->data != NULL) && (payload->len != 0))
        {
            msg = usp__msg__unpack(pbuf_allocator, payload->len, payload->data);
            USP_ASSERT(msg != NULL);
            PROTO_TRACE_ProtobufMessage(&msg->base);
            usp__msg__free_unpacked(msg, pbuf_allocator);
        }
    }

    // Free the USP record protobuf structures
    usp_record__record__free_unpacked(rec, pbuf_allocator);
}

/*********************************************************************//**
**
** MSG_HANDLER_QueueErrorMessage
**
** Sends back an error message
** NOTE: The textual error message should have previously been set by the caller using USP_ERR_SetMessage()
**
** \param   err - USP error code to send back to controller to indicate cause of error
** \param   endpoint_id - endpoint to send the message to
** \param   mtpc - details of where response to this USP message should be sent
** \param   msg_id - String containing the message ID of the USP message which caused this error
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_QueueErrorMessage(int err, char *endpoint_id, mtp_conn_t *mtpc, char *msg_id)
{
    Usp__Msg *resp;
    resp = ERROR_RESP_CreateSingle(msg_id, err, NULL);
    MSG_HANDLER_QueueMessage(endpoint_id, resp, mtpc);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** MSG_HANDLER_QueueMessage
**
** Serializes a USP message to a buffer, then queues it, to be sent to a controller
**
** \param   endpoint_id - controller to send the message to
** \param   usp - pointer to protobuf-c structure describing the USP message to send
** \param   mtpc - details of where this USP response message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_QueueMessage(char *endpoint_id, Usp__Msg *usp, mtp_conn_t *mtpc)
{
    int err;
    usp_send_item_t usp_send_item;
    int pbuf_len;
    unsigned char *pbuf;
    int size;

    // Exit if parameters not specified
    if ((endpoint_id == NULL) || (usp == NULL))
    {
        USP_ERR_SetMessage("%s: invalid parameters", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Serialize the USP message into a buffer
    pbuf_len = usp__msg__get_packed_size(usp);
    pbuf = USP_MALLOC(pbuf_len);
    size = usp__msg__pack(usp, pbuf);
    USP_ASSERT(size == pbuf_len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    // Marshal parameters to pass to MSG_HANDLER_QueueUspRecord()
    MSG_HANDLER_UspSendItem_Init(&usp_send_item);
    usp_send_item.usp_msg_type = usp->header->msg_type;
    usp_send_item.msg_packed = pbuf;
    usp_send_item.msg_packed_size = pbuf_len;
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    usp_send_item.curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(MSG_HANDLER_GetMsgControllerInstance());;
    usp_send_item.usp_msg = usp;
#endif

    // Encapsulate this message in a USP record, then queue the record, to send to a controller
    err = MSG_HANDLER_QueueUspRecord(&usp_send_item, endpoint_id, usp->header->msg_id, mtpc, END_OF_TIME);

    // Free the serialized USP Message because it is now encapsulated in USP Record messages.
    USP_FREE(usp_send_item.msg_packed);

    return err;
}

/*********************************************************************//**
**
** MSG_HANDLER_QueueUspRecord
**
** Serializes a protobuf USP record structure to a buffer (with encapsulated USP message),
** then queues it, to be sent to a controller
**
** \param   usi - Information about the USP Message to send. The ownership of
**                the serialized payload is not passed to this function and stays with the caller.
** \param   endpoint_id - controller to send the message to
** \param   usp_msg_id - pointer to string containing the msg_id of the serialized USP Message
** \param   mtpc - details of where this USP response message should be sent
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_QueueUspRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id, mtp_conn_t *mtpc, time_t expiry_time)
{
    int err = USP_ERR_OK;

    // Exit if no controller setup to send the message to
    if (endpoint_id == NULL)
    {
        return USP_ERR_OK;
    }

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    if (E2E_CONTEXT_IsToSendThroughSessionContext(usi->curr_e2e_session))
    {
        err = E2E_CONTEXT_QueueUspSessionRecord(usi, endpoint_id, usp_msg_id, mtpc, expiry_time);
    }
    else
#endif
    {
        err = QueueUspNoSessionRecord(usi, endpoint_id, usp_msg_id, mtpc, expiry_time);
    }

    return err;
}

/*********************************************************************//**
**
** MSG_HANDLER_QueueUspDisconnectRecord
**
** Serializes a protobuf USP DisconnectRecord structure to a buffer,
** then queues it, to be sent to a controller.
**
** \param   content_type - indicates whether the disconnect record is to close an E2E session or not
** \param   cont_endpoint_id - controller to send the record to
** \param   reason_code - code of the message number to be printed in the Disconnect record
** \param   reason_str - pointer to the message to be printed in the Disconnect record.
** \param   mtpc - details of where this USP record should be sent
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_QueueUspDisconnectRecord(mtp_content_type_t content_type, char *cont_endpoint_id, uint32_t reason_code, char* reason_str, mtp_conn_t *mtpc, time_t expiry_time)
{
    mtp_send_item_t mtp_send_item;
    int err = USP_ERR_OK;

    // Exit if no controller setup to send the message to
    if (cont_endpoint_id == NULL)
    {
        return USP_ERR_OK;
    }

    USP_LOG_Debug("%s: USP Disconnect to send with reason %u", __FUNCTION__, reason_code);

    // Fill in the USP Record structure
    USPREC_Disconnect_Create(content_type, cont_endpoint_id, reason_code, reason_str, &mtp_send_item);

    // Exit if unable to queue the message, to send to a controller
    // NOTE: If successful, ownership of the USP record buffer passes to the MTP layer. If not successful, buffer is freed here
    err = DEVICE_CONTROLLER_QueueBinaryMessage(&mtp_send_item, cont_endpoint_id, NULL, mtpc, expiry_time);
    if (err != USP_ERR_OK)
    {
        USP_FREE(mtp_send_item.pbuf);
    }

    return err;
}

/*********************************************************************//**
**
** MSG_HANDLER_GetMsgControllerInstance
**
** Gets the instance number of the controller that sent the message which is currently being processed
**
** \param   None
**
** \return  instance number in Device.LocalAgent.Controller.{i} table
**
**************************************************************************/
int MSG_HANDLER_GetMsgControllerInstance(void)
{
    if (cur_msg_controller_instance != INVALID)
    {
        return cur_msg_controller_instance;
    }
    else
    {
        // This code is only triggered, if running a CLI command
        return 1;
    }
}

/*********************************************************************//**
**
** MSG_HANDLER_GetMsgRole
**
** Gets the role to use for the current message being processed
**
** \param   None
**
** \return  role to use for the controller that sent the current message
**
**************************************************************************/
void MSG_HANDLER_GetMsgRole(combined_role_t *combined_role)
{
    *combined_role = cur_msg_combined_role;
}

/*********************************************************************//**
**
** MSG_HANDLER_GetControllerInfo
**
** Gets the controller info structure for the current message being processed
**
** \param   controller_info - pointer to controller info structure
**
** \return  None
**
**************************************************************************/
void MSG_HANDLER_GetControllerInfo(controller_info_t *controller_info)
{
    *controller_info = cur_msg_controller_info;
}

/*********************************************************************//**
**
** MSG_HANDLER_GetMsgType
**
** Gets the current type of message being processed
**
** \param   None
**
** \return  type of current message being processed, or INVALID, if no message is currently being processed by the data model thread
**
**************************************************************************/
int MSG_HANDLER_GetMsgType(void)
{
    return cur_msg_type;
}

/*********************************************************************//**
**
** MSG_HANDLER_GetMsgControllerEndpointId
**
** Gets the endpoint_id of the controller that sent the message which is currently being processed
**
** \param   None
**
** \return  endpoint_id of controller
**
**************************************************************************/
char *MSG_HANDLER_GetMsgControllerEndpointId(void)
{
    char *endpoint_id;

    // Exit in case of running a CLI command, and hence no controller instance setup
    if (cur_msg_controller_instance == INVALID)
    {
        return "";
    }

    // Exit if unable to determine endpoint_id of the enabled controller
    endpoint_id = DEVICE_CONTROLLER_FindEndpointIdByInstance(cur_msg_controller_instance);
    if (endpoint_id == NULL)
    {
        return "";
    }

    return endpoint_id;
}

/*********************************************************************//**
**
** MSG_HANDLER_UspMsgTypeToString
**
** Convenience function to convert a USP Message type enumeration to a
** string for use by debug.
**
** \param   msg_type - protobuf enumeration of the type of USP Message or
**                     INT_MAX when there is no USP Message encapsulated.
**
** \return  pointer to string or 'UNKNOWN'
**
**************************************************************************/
char *MSG_HANDLER_UspMsgTypeToString(int msg_type)
{
    // Exit if this is an E2E session initiation USP Record with empty payload
    if (msg_type == INVALID_USP_MSG_TYPE)
    {
        return "USP Record";
    }

    return TEXT_UTILS_EnumToString(msg_type, usp_msg_types, NUM_ELEM(usp_msg_types));
}

/*********************************************************************//**
**
** MSG_HANDLER_UspSendItem_Init
**
** Initialises the usp_send_item_t struct with default values
**
** \param   usi - struct to initialize
**
** \return  None
**
**************************************************************************/
void MSG_HANDLER_UspSendItem_Init(usp_send_item_t *usi)
{
    usi->usp_msg_type = INVALID_USP_MSG_TYPE;
    usi->msg_packed = NULL;
    usi->msg_packed_size = 0;
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    usi->curr_e2e_session = NULL;
    usi->usp_msg = NULL;
#endif
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateRequestMsg
**
** Helper function to create a request message
** NOTE: This does not add the main body of the request, it is intended that the caller does this
**
** \param   msg_id - string containing the message id to use for the request
** \param   header_type - enumeration for the type of message to put in the header
** \param   req_type - enumeration for the type of message to put in the request object
**
** \return  Pointer to a USP message
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateRequestMsg(char *msg_id, Usp__Header__MsgType header_type, Usp__Request__ReqTypeCase req_type)
{
    Usp__Msg *msg;
    Usp__Request *request;

    msg = MSG_HANDLER_CreateUspMsg(msg_id, header_type, USP__BODY__MSG_BODY_REQUEST);
    request = USP_MALLOC(sizeof(Usp__Request));
    usp__request__init(request);
    msg->body->request = request;
    request->req_type_case = req_type;

    return msg;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateResponseMsg
**
** Helper function to create a response message
** NOTE: This does not add the main body of the response, it is intended that the caller does this
**
** \param   msg_id - string containing the message id to use for the response
** \param   header_type - enumeration for the type of message to put in the header
** \param   resp_type - enumeration for the type of message to put in the response object
**
** \return  Pointer to a USP message
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateResponseMsg(char *msg_id, Usp__Header__MsgType header_type, Usp__Response__RespTypeCase resp_type)
{
    Usp__Msg *msg;
    Usp__Response *response;

    msg = MSG_HANDLER_CreateUspMsg(msg_id, header_type, USP__BODY__MSG_BODY_RESPONSE);
    response = USP_MALLOC(sizeof(Usp__Response));
    usp__response__init(response);
    msg->body->response = response;
    response->resp_type_case = resp_type;

    return msg;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateUspMsg
**
** Helper function to create a USP message
** NOTE: This does not add the request/response part of the message, it is intended that the caller does this
**
** \param   msg_id - string containing the message id to use for the response
** \param   header_type - enumeration for the type of message to put in the header
** \param   body_type - enumeration for the type of message to put in the header
**
** \return  Pointer to a USP message
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateUspMsg(char *msg_id, Usp__Header__MsgType header_type, Usp__Body__MsgBodyCase body_type)
{
    Usp__Msg *msg;
    Usp__Header *header;
    Usp__Body *body;

    // Allocate and initialise memory to store the parts of the USP message
    msg = USP_MALLOC(sizeof(Usp__Msg));
    usp__msg__init(msg);

    header = USP_MALLOC(sizeof(Usp__Header));
    usp__header__init(header);

    body = USP_MALLOC(sizeof(Usp__Body));
    usp__body__init(body);

    // Connect the structures together
    msg->header = header;
    header->msg_id = USP_STRDUP(msg_id);
    header->msg_type = header_type;

    msg->body = body;
    body->msg_body_case = body_type;

    return msg;
}

/*********************************************************************//**
**
** HandleUspMessage
**
** Main entry point to handling a message
** NOTE: Parsing errors are handled locally by this function
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int HandleUspMessage(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    int err = USP_ERR_OK;
    char buf[MAX_ISO8601_LEN];
    Usp__Header__MsgType usp_msg_type;

    // Exit if the message was ill-formed
    if (usp->header == NULL)
    {
        USP_ERR_SetMessage("%s: Ignoring malformed USP message", __FUNCTION__);
        err = USP_ERR_MESSAGE_NOT_UNDERSTOOD;
        goto exit;
    }

#ifndef REMOVE_USP_SERVICE
{
    // Check with USP service to see if this is a response to an outgoing control message
    bool is_handled;
    is_handled = USP_SERVICE_AsController_IsExpectedResponse(usp);
    if (is_handled)
    {
        err = USP_ERR_OK;
        goto exit;
    }
}
#endif

    // Exit if the USP message type was not one that we can accept, or was received on the wrong UDS socket
    usp_msg_type = usp->header->msg_type;
    err = ValidateUspMsgType(usp_msg_type, endpoint_id, mtpc, usp->header->msg_id);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

#ifndef REMOVE_USP_BROKER
{
    // Attempt to pass this USP message directly through to a USP Service, if it only applies to a single USP Service
    // and the originating controller has permission to perform the request on all paths of the data model in the request
    bool is_handled;
    is_handled = USP_BROKER_AttemptPassthru(usp, endpoint_id, mtpc, &cur_msg_combined_role, NULL);
    if (is_handled)
    {
        err = USP_ERR_OK;
        goto exit;
    }
}
#endif

    // Log the message
    USP_LOG_Info("%s : processing at time %s",
                MSG_HANDLER_UspMsgTypeToString(usp->header->msg_type),
                iso8601_cur_time(buf, sizeof(buf)) );

    // Process the message
    cur_msg_type = usp_msg_type;
    switch(usp_msg_type)
    {
        case USP__HEADER__MSG_TYPE__GET:
            MSG_HANDLER_HandleGet(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__SET:
            MSG_HANDLER_HandleSet(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__ADD:
            MSG_HANDLER_HandleAdd(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__DELETE:
            MSG_HANDLER_HandleDelete(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__OPERATE:
            MSG_HANDLER_HandleOperate(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__NOTIFY_RESP:
            MSG_HANDLER_HandleNotifyResp(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
            MSG_HANDLER_HandleGetSupportedProtocol(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__GET_INSTANCES:
            MSG_HANDLER_HandleGetInstances(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
            MSG_HANDLER_HandleGetSupportedDM(usp, endpoint_id, mtpc);
            break;


        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP:
            HandleGetSupportedDMResp(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__NOTIFY:
            HandleNotification(usp, endpoint_id, mtpc);  // NOTE: Most notifications are handled by USP_BROKER_AttemptPassthru()
            break;

        case USP__HEADER__MSG_TYPE__ERROR:
            HandleUspError(usp, endpoint_id, mtpc);
            break;

#ifndef REMOVE_USP_BROKER
        case USP__HEADER__MSG_TYPE__REGISTER:
            USP_BROKER_HandleRegister(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__DEREGISTER:
            USP_BROKER_HandleDeRegister(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__GET_RESP:
        case USP__HEADER__MSG_TYPE__SET_RESP:
        case USP__HEADER__MSG_TYPE__ADD_RESP:
        case USP__HEADER__MSG_TYPE__DELETE_RESP:
        case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP:
            USP_LOG_Warning("%s: Received an unsolicited %s message. Maybe a non-passthru request timed out ? Ignoring", __FUNCTION__, TEXT_UTILS_EnumToString(usp_msg_type, usp_msg_types, NUM_ELEM(usp_msg_types)) );
            break;
#endif

#ifndef REMOVE_USP_SERVICE
        case USP__HEADER__MSG_TYPE__REGISTER_RESP:
            USP_SERVICE_HandleRegisterResp(usp, endpoint_id, mtpc);
            break;

        case USP__HEADER__MSG_TYPE__DEREGISTER_RESP:
            USP_SERVICE_HandleDeRegisterResp(usp, endpoint_id, mtpc);
            break;
#endif

        default:
            // The code in ValidateUspMsgType() should have ensured that the code does not get here
            TERMINATE_BAD_CASE(usp_msg_type);
            break;
    }

    err = USP_ERR_OK;

exit:
    cur_msg_controller_instance = INVALID;
    cur_msg_type = INVALID;

    // Activate all STOMP reconnects or scheduled exits, now that we have queued all response messages
    MTP_EXEC_ActivateScheduledActions();

    return err;
}

/*********************************************************************//**
**
** ValidateUspMsgType
**
** Validates the received USP message can be accepted for processing, based on its message type
**
** \param   usp_msg_type - type of the received USP message
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where this USP message was received from
** \param   msg_id - String containing the message ID of the received USP message
**
** \return  USP_ERR_OK if the message can be accepted for processing
**          USP_ERR_PERMISSION_DENIED if the message was ignored
**
**************************************************************************/
int ValidateUspMsgType(Usp__Header__MsgType msg_type, char *endpoint_id, mtp_conn_t *mtpc, char *msg_id)
{
    unsigned sender_role = 0;
    unsigned expected_sender_role;

    // Defines for bitmask representing the role that the sender sent the message as
    #define FROM_CONTROLLER  0x01
    #define FROM_AGENT  0x02

    // Set default expected sender - a Controller ACS. This applies if not running as a USP Service or not received from a USP Service
    expected_sender_role = FROM_CONTROLLER;

#ifndef REMOVE_USP_SERVICE
    if (RUNNING_AS_USP_SERVICE()==true)
    {
#ifdef ENABLE_UDS
        // Calculate expected sender role, if running as a USP Service
        if (mtpc->protocol == kMtpProtocol_UDS)
        {
            // If over UDS, ensure that controller-initiated messages are received by the USP Service on the Broker's controller socket,
            // and agent-initiated messages are received by the USP Service on the Broker's agent socket
            expected_sender_role = (mtpc->uds.path_type == kUdsPathType_BrokersController) ? FROM_CONTROLLER : FROM_AGENT;
        }
        else
#endif
        {
            // If non UDS, the USP message received by the USP Service could come from either a controller or an agent
            expected_sender_role = FROM_CONTROLLER | FROM_AGENT;
        }

        // Ensure that invalid and unsupported message types received by USP Services are not processed
        if ((msg_type == USP__HEADER__MSG_TYPE__REGISTER) ||
            (msg_type == USP__HEADER__MSG_TYPE__DEREGISTER) ||
            (msg_type == USP__HEADER__MSG_TYPE__GET_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__SET_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__ADD_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__DELETE_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__OPERATE_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP) ||
            (msg_type == USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP))
        {
            expected_sender_role = 0;
        }
    }
#endif

#ifndef REMOVE_USP_BROKER
    if (RUNNING_AS_USP_SERVICE()==false)
    {
#ifdef ENABLE_UDS
        // Determine if sender was acting as a controller or agent (or either)
        if (mtpc->protocol == kMtpProtocol_UDS)
        {
            // If received over UDS, ensure that controller-initiated messages are received by the Broker on the Broker's
            // agent socket, and agent-initiated messages are received by the Broker on the Broker's controller socket
            expected_sender_role = (mtpc->uds.path_type == kUdsPathType_BrokersController) ? FROM_AGENT : FROM_CONTROLLER;
        }
        else
#endif
        {
            // If received on non-UDS, ensure that the message was received from a USP Service (not ACS Controller),
            // before allowing either agent-initiated or controller-initiated messages to be accepted for processing
            int instance = USP_BROKER_GetUspServiceInstance(endpoint_id, 0);
            if (instance != INVALID)
            {
                expected_sender_role = FROM_CONTROLLER | FROM_AGENT;
            }
            else
            {
                // If the code gets here, then the endpoint sending the message was not a USP Service, so it must be a Controller ACS
                // Ensure that we ignore the Register response and Deregister response messages if received from a Controller ACS
                if ((msg_type == USP__HEADER__MSG_TYPE__REGISTER_RESP) || (msg_type == USP__HEADER__MSG_TYPE__DEREGISTER_RESP))
                {
                    expected_sender_role = 0;
                }
            }
        }

        // Ensure that invalid and unsupported message types received by USP Brokers are not processed
        if (msg_type == USP__HEADER__MSG_TYPE__DEREGISTER_RESP)
        {
            expected_sender_role = 0;
        }
    }
#endif

    // Calculate the role that the received message type implies
    switch(msg_type)
    {
        case USP__HEADER__MSG_TYPE__ERROR:
            sender_role = FROM_CONTROLLER | FROM_AGENT;  // USP Errors can be sent by either Controllers or Agents
            break;

        case USP__HEADER__MSG_TYPE__GET:
        case USP__HEADER__MSG_TYPE__SET:
        case USP__HEADER__MSG_TYPE__ADD:
        case USP__HEADER__MSG_TYPE__DELETE:
        case USP__HEADER__MSG_TYPE__OPERATE:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES:
        case USP__HEADER__MSG_TYPE__NOTIFY_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
        case USP__HEADER__MSG_TYPE__REGISTER_RESP:
        case USP__HEADER__MSG_TYPE__DEREGISTER_RESP:
            sender_role = FROM_CONTROLLER;  // These messages are only sent by controllers
            break;

        case USP__HEADER__MSG_TYPE__GET_RESP:
        case USP__HEADER__MSG_TYPE__SET_RESP:
        case USP__HEADER__MSG_TYPE__ADD_RESP:
        case USP__HEADER__MSG_TYPE__DELETE_RESP:
        case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        case USP__HEADER__MSG_TYPE__NOTIFY:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP:
        case USP__HEADER__MSG_TYPE__REGISTER:
        case USP__HEADER__MSG_TYPE__DEREGISTER:
            sender_role = FROM_AGENT;  // These messages are only sent by agents
            break;

        default:
            TERMINATE_BAD_CASE(msg_type);
            break;
    }

    // Exit if the received USP message can be accepted for processing
    if (expected_sender_role & sender_role)
    {
        return USP_ERR_OK;
    }

    // If the code gets here, then the USP message cannot be processed or was received on the wrong UDS socket
    // Determine whether to ignore the message, or send an error response
    switch(msg_type)
    {
        case USP__HEADER__MSG_TYPE__ERROR:
        case USP__HEADER__MSG_TYPE__NOTIFY_RESP:
        case USP__HEADER__MSG_TYPE__REGISTER_RESP:
        case USP__HEADER__MSG_TYPE__DEREGISTER_RESP:
        case USP__HEADER__MSG_TYPE__GET_RESP:
        case USP__HEADER__MSG_TYPE__SET_RESP:
        case USP__HEADER__MSG_TYPE__ADD_RESP:
        case USP__HEADER__MSG_TYPE__DELETE_RESP:
        case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP:
            // According to R-MTP.5, all received USP Error and USP Response messages should be ignored
#ifdef ENABLE_UDS
            if (mtpc->protocol == kMtpProtocol_UDS)
            {
                USP_ERR_SetMessage("%s: Cannot handle USP message type %s on %s. Ignoring", __FUNCTION__, TEXT_UTILS_EnumToString(msg_type, usp_msg_types, NUM_ELEM(usp_msg_types)), UDS_PathTypeToString(mtpc->uds.path_type));
            }
            else
#endif
            {
                USP_ERR_SetMessage("%s: Cannot handle USP message type %s. Ignoring", __FUNCTION__, TEXT_UTILS_EnumToString(msg_type, usp_msg_types, NUM_ELEM(usp_msg_types)) );
            }
            return USP_ERR_PERMISSION_DENIED;
            break;

        case USP__HEADER__MSG_TYPE__GET:
        case USP__HEADER__MSG_TYPE__SET:
        case USP__HEADER__MSG_TYPE__ADD:
        case USP__HEADER__MSG_TYPE__DELETE:
        case USP__HEADER__MSG_TYPE__OPERATE:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
        case USP__HEADER__MSG_TYPE__NOTIFY:
        case USP__HEADER__MSG_TYPE__REGISTER:
        case USP__HEADER__MSG_TYPE__DEREGISTER:
            // Since the sender shouldn't send these messages, or sent them on the wrong UDS MTP,
            // according to R-MTP.5, a USP Error message should be sent in response to an erroneous USP Request message
#ifdef ENABLE_UDS
            if (mtpc->protocol == kMtpProtocol_UDS)
            {
                USP_ERR_SetMessage("%s: Cannot handle USP message type %s on %s. Sending back Error response", __FUNCTION__, TEXT_UTILS_EnumToString(msg_type, usp_msg_types, NUM_ELEM(usp_msg_types)), UDS_PathTypeToString(mtpc->uds.path_type) );
            }
            else
#endif
            {
                USP_ERR_SetMessage("%s: Cannot handle USP message type %s. Sending back Error response", __FUNCTION__, TEXT_UTILS_EnumToString(msg_type, usp_msg_types, NUM_ELEM(usp_msg_types)) );
            }
            MSG_HANDLER_QueueErrorMessage(USP_ERR_REQUEST_DENIED, endpoint_id, mtpc, msg_id);
            return USP_ERR_REQUEST_DENIED;
            break;

        default:
            TERMINATE_BAD_CASE(msg_type);
            break;
    }

    return USP_ERR_REQUEST_DENIED;
}

/*********************************************************************//**
**
** ValidateUspRecord
**
** Validates whether a received USP record can be accepted by USP Agent for processing
** NOTE: Parsing errors are handled locally by this function
**
** \param   rec - pointer to protobuf structure describing the received USP record
** \param   mtpc - MTP details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if record is valid
**
**************************************************************************/
int ValidateUspRecord(UspRecord__Record *rec, mtp_conn_t *mtpc)
{
    int err;
    char *err_msg;
    char *endpoint_id;
    bool has_mtp;
    UspRecord__NoSessionContextRecord *ctx;
    int usp_service_instance = INVALID;

    // Exit if this record is not supposed to be processed by us
    endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    if ((rec->to_id == NULL) || (strcmp(rec->to_id, endpoint_id) != 0))
    {
        USP_ERR_SetMessage("%s: Ignoring USP record as it was addressed to endpoint_id=%s not %s", __FUNCTION__, rec->to_id, endpoint_id);
        return USP_ERR_REQUEST_DENIED;
    }

    // Exit if no controller endpoint_id to send the response back to
    if ((rec->from_id == NULL) || (rec->from_id[0] == '\0'))
    {
        USP_ERR_SetMessage("%s: Ignoring USP record as from_id is blank", __FUNCTION__);
        return USP_ERR_RECORD_FIELD_INVALID;
    }

#ifndef REMOVE_USP_BROKER
    usp_service_instance = USP_BROKER_GetUspServiceInstance(rec->from_id, 0);
#endif

    // Exit if the endpoint sending the message is unknown
    cur_msg_controller_instance = DEVICE_CONTROLLER_FindInstanceByEndpointId(rec->from_id);
    if ((cur_msg_controller_instance == INVALID) && (usp_service_instance == INVALID))
    {
        USP_ERR_SetMessage("%s: Ignoring message from endpoint_id=%s (unknown controller or USP service)", __FUNCTION__, rec->from_id);
        return USP_ERR_REQUEST_DENIED;
    }

    // Exit if we don't know where to send the response, ignoring the USP message
    // (because none was provided to the MTP when the message was received, and none is configured in the data model)
    if (mtpc->is_reply_to_specified == false)
    {
        // Since no 'reply-to' was provided along with the received message, see if one is configured in the data model
        has_mtp = DEVICE_CONTROLLER_IsMTPConfigured(rec->from_id, mtpc->protocol);
        if (has_mtp == false)
        {
            // Exit if there is none provided in the data model
            USP_ERR_SetMessage("%s: Ignoring message from endpoint_id=%s (No MTP and no reply-to)", __FUNCTION__, rec->from_id);
            return USP_ERR_REQUEST_DENIED;
        }
    }

    // Exit if this USP Record contains the invalid combination of encrypted payload carried in non-Session context
    // NOTE: This more specific check must come before the more general test for encrypted payload, otherwise we wouldn't detect it
    if ((rec->payload_security != USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT) &&
        (rec->record_type_case != USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT))
    {
        USP_ERR_SetMessage("%s: Received USP record contains an encrypted payload without Session Context", __FUNCTION__);
        err = USP_ERR_RECORD_FIELD_INVALID;
        err_msg = USP_ERR_GetMessage();
        MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mtpc, END_OF_TIME);
        return err;
    }

    // Exit if this record contains an encrypted payload (which we don't yet support).
    if (rec->payload_security != USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT)
    {
        USP_ERR_SetMessage("%s: Ignoring E2E USP record containing an encrypted payload", __FUNCTION__);
        err = USP_ERR_SECURE_SESS_NOT_SUPPORTED;
        err_msg = USP_ERR_GetMessage();
        MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mtpc, END_OF_TIME);
        return err;
    }

    // Print a warning if ignoring integrity check (which we don't yet support).
    if ((rec->mac_signature.len != 0) || (rec->mac_signature.data != NULL))
    {
        USP_LOG_Warning("%s: WARNING: Not performing integrity check on non-payload fields of received USP Record", __FUNCTION__);
    }

    // Ignore sender certificate (which we don't yet support).
    if ((rec->sender_cert.len != 0) || (rec->sender_cert.data != NULL))
    {
        USP_LOG_Warning("%s: Skipping sender certificate verification", __FUNCTION__);
    }

    // Validate fields based on record type
    switch (rec->record_type_case)
    {
        case USP_RECORD__RECORD__RECORD_TYPE_NO_SESSION_CONTEXT:
            // Exit if this record does not contain a payload
            ctx = rec->no_session_context;
            if ((ctx == NULL) || (ctx->payload.data == NULL) || (ctx->payload.len == 0))
            {
                USP_ERR_SetMessage("%s: Ignoring USP record as it does not contain a payload", __FUNCTION__);
                return USP_ERR_RECORD_FIELD_INVALID;
            }
            break;

        case USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT:
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
            err = E2E_CONTEXT_ValidateSessionContextRecord(rec->session_context);
            if (err != USP_ERR_OK)
            {
                return err;
            }
#else
            USP_ERR_SetMessage("%s: Session Context record type not supported", __FUNCTION__);
            err = USP_ERR_SESS_CONTEXT_NOT_ALLOWED;
            err_msg = USP_ERR_GetMessage();
            MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mtpc, END_OF_TIME);
            return err;
#endif
            break;

        case USP_RECORD__RECORD__RECORD_TYPE_DISCONNECT:
            // If we received a disconnect record, then initiate a disconnect
            USP_ERR_SetMessage("%s: USP Disconnect record received. Disconnecting.", __FUNCTION__);
            err = USP_ERR_SESS_CONTEXT_TERMINATED;
            err_msg = USP_ERR_GetMessage();
            MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mtpc, END_OF_TIME);
            return err;
            break;

        default:
        case USP_RECORD__RECORD__RECORD_TYPE__NOT_SET:
        case USP_RECORD__RECORD__RECORD_TYPE_WEBSOCKET_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_MQTT_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_STOMP_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_UDS_CONNECT:
            // Exit if unsupported USP Record type OR unexpected record type for USP Agent
            // NOTE: If running as a USP Broker, the USP Connect records should have already been ignored, and this function should not have been called
            USP_ERR_SetMessage("%s: Ignoring USP record with unsupported record type: %d", __FUNCTION__, rec->record_type_case);
            return USP_ERR_REQUEST_DENIED;
    }

    // If the code gets here, then the USP record passed validation, and the encapsulated USP message may be processed
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** MtpSendItemToString
**
** Returns a string summarizing the contents of the send item
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is not passed to this function and stays with the caller.
**
** \return  pointer to string summarizing the contents of the send item
**
**************************************************************************/
char *MtpSendItemToString(mtp_send_item_t *msi)
{
    // Exit if send item contains a full USP message, returning the USP message type
    if (msi->content_type == kMtpContentType_UspMessage)
    {
        return MSG_HANDLER_UspMsgTypeToString(msi->usp_msg_type);
    }

    // Otherwise return the type of content in the send item
    return TEXT_UTILS_EnumToString(msi->content_type, mtp_content_types, NUM_ELEM(mtp_content_types));
}

/*********************************************************************//**
**
** QueueUspNoSessionRecord
**
** Serializes a protobuf USP NoSessionContext Record structure for the
** given USP Message binary, then queues it, to be sent to a controller.
**
** \param   usp_send_item - Information about the USP Message to send
** \param   endpoint_id - controller to send the message to
** \param   usp_msg_id - pointer to string containing the msg_id of the serialized USP Message
** \param   mtpc - details of where this USP response message should be sent
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int QueueUspNoSessionRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id,
                            mtp_conn_t *mtpc, time_t expiry_time)
{
    int err = USP_ERR_OK;
    mtp_send_item_t mtp_send_item;
    UspRecord__NoSessionContextRecord ctxNoSession;
    UspRecord__Record rec;

    usp_record__no_session_context_record__init(&ctxNoSession);
    ctxNoSession.payload.data = usi->msg_packed;
    ctxNoSession.payload.len = usi->msg_packed_size;
    USP_ASSERT(ctxNoSession.payload.len > 0);  // A NoSessionContext MUST have content

    // Fill in the USP Record structure
    // NOTE: This is all statically allocated (or owned elsewhere), so no need to free
    usp_record__record__init(&rec);
    rec.version = AGENT_CURRENT_PROTOCOL_VERSION;
    rec.to_id = endpoint_id;
    rec.from_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    rec.payload_security = USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT;
    rec.record_type_case = USP_RECORD__RECORD__RECORD_TYPE_NO_SESSION_CONTEXT;
    rec.no_session_context = &ctxNoSession;

    // Serialize the protobuf record structure into a buffer
    {
        const int len = usp_record__record__get_packed_size(&rec);
        uint8_t *buf = USP_MALLOC(len);
        const int size = usp_record__record__pack(&rec, buf);
        USP_ASSERT(size == len);  // If these are not equal, then we may have had a buffer overrun, so terminate

        // Prepare the MTP item information now it is serialized.
        MTP_EXEC_MtpSendItem_Init(&mtp_send_item);
        mtp_send_item.usp_msg_type = usi->usp_msg_type;
        mtp_send_item.content_type = kMtpContentType_UspMessage;
        mtp_send_item.pbuf = buf;  // Ownership of the serialized USP Record passes to the queue, unless an error is returned.
        mtp_send_item.pbuf_len = len;
    }

    // Exit if unable to queue the message, to send to a controller
    // NOTE: If successful, ownership of the buffer passes to the MTP layer. If not successful, buffer is freed here
    err = DEVICE_CONTROLLER_QueueBinaryMessage(&mtp_send_item, endpoint_id, usp_msg_id, mtpc, expiry_time);
    if (err != USP_ERR_OK)
    {
        USP_FREE(mtp_send_item.pbuf);
    }

    return err;
}

/*********************************************************************//**
**
** HandleGetSupportedDMResp
**
** Handles a USP GetSupportedDMResp message
** NOTE: This function may be called by either a USP Broker or a USP Service
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void HandleGetSupportedDMResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
#ifndef REMOVE_USP_BROKER
    if (RUNNING_AS_USP_SERVICE()==false)
    {
        USP_BROKER_HandleGetSupportedDMResp(usp, endpoint_id, mtpc);
    }
#endif
}

/*********************************************************************//**
**
** HandleNotification
**
** Handles a USP Notification message
** NOTE: This function may be called by either a USP Broker or a USP Service
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void HandleNotification(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
#ifndef REMOVE_USP_BROKER
    if (RUNNING_AS_USP_SERVICE()==false)
    {
        USP_BROKER_HandleNotification(usp, endpoint_id, mtpc);
    }
#endif
}

/*********************************************************************//**
**
** HandleUspError
**
** Handles a received USP Error message
** NOTE: This function may be called by either a USP Broker or a USP Service
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void HandleUspError(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    if (RUNNING_AS_USP_SERVICE()==true)
    {
#ifndef REMOVE_USP_SERVICE
        USP_SERVICE_HandleRegisterResp(usp, endpoint_id, mtpc);
#endif
    }
    else
    {
        // According to R-MTP.5, all received USP Error messages should be ignored
        USP_LOG_Warning("%s: Ignoring received USP ERROR message", __FUNCTION__);
    }
}



//------------------------------------------------------------------------------------------
// Code to test the ValidateUspMsgType() function
#if 0
#define AS_BROKER NULL
#define AS_SERVICE "proto::broker,Device.Test."
#define FROM_BROKER "proto::broker"
#define FROM_SERVICE "proto::service1"
#define FROM_ACS "proto::controller"

#define ON_UDS_BROKERS_CONTROLLER "UDS_BC"
#define ON_UDS_BROKERS_AGENT      "UDS_BA"
#define ON_STOMP                  "STOMP"

#define ACCEPT "0"
#define ERROR  "7002"
#define IGNORE "7006"

char *validate_msg_test_cases[] =
{
#ifndef REMOVE_USP_SERVICE
// Service-as-Agent receiving from Broker via UDS: Broker's Controller socket
"001",     AS_SERVICE, "GET", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"002",     AS_SERVICE, "SET", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"003",     AS_SERVICE, "ADD", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"004",     AS_SERVICE, "DELETE", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"005",     AS_SERVICE, "OPERATE", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"006",     AS_SERVICE, "GET_SUPPORTED_DM", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"007",     AS_SERVICE, "GET_INSTANCES", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"008",     AS_SERVICE, "NOTIFY_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"009",     AS_SERVICE, "GET_SUPPORTED_PROTO", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"010",     AS_SERVICE, "REGISTER_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"011",     AS_SERVICE, "ERROR", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"012",     AS_SERVICE, "DEREGISTER_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"013",     AS_SERVICE, "GET_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"014",     AS_SERVICE, "SET_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"015",     AS_SERVICE, "ADD_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"016",     AS_SERVICE, "DELETE_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"017",     AS_SERVICE, "OPERATE_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"018",     AS_SERVICE, "GET_SUPPORTED_DM_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"019",     AS_SERVICE, "GET_INSTANCES_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"020",     AS_SERVICE, "GET_SUPPORTED_PROTO_RESP", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"021",     AS_SERVICE, "NOTIFY", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ERROR,
"022",     AS_SERVICE, "REGISTER", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ERROR,
"023",     AS_SERVICE, "DEREGISTER", FROM_BROKER, ON_UDS_BROKERS_CONTROLLER, ERROR,

// Service-as-Controller receiving from Broker via UDS: Broker's Agent socket
"101",     AS_SERVICE, "NOTIFY", FROM_BROKER, ON_UDS_BROKERS_AGENT, ACCEPT,
"102",     AS_SERVICE, "ERROR", FROM_BROKER, ON_UDS_BROKERS_AGENT, ACCEPT,
"103",     AS_SERVICE, "REGISTER_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"104",     AS_SERVICE, "DEREGISTER_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"105",     AS_SERVICE, "NOTIFY_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"106",     AS_SERVICE, "GET", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"107",     AS_SERVICE, "SET", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"108",     AS_SERVICE, "ADD", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"109",     AS_SERVICE, "DELETE", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"110",     AS_SERVICE, "OPERATE", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"111",     AS_SERVICE, "GET_SUPPORTED_DM", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"112",     AS_SERVICE, "GET_INSTANCES", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"113",     AS_SERVICE, "GET_SUPPORTED_PROTO", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"114",     AS_SERVICE, "REGISTER", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"115",     AS_SERVICE, "DEREGISTER", FROM_BROKER, ON_UDS_BROKERS_AGENT, ERROR,
"116",     AS_SERVICE, "GET_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"117",     AS_SERVICE, "SET_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"118",     AS_SERVICE, "ADD_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"119",     AS_SERVICE, "DELETE_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"120",     AS_SERVICE, "OPERATE_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"121",     AS_SERVICE, "GET_SUPPORTED_DM_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"122",     AS_SERVICE, "GET_INSTANCES_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
"123",     AS_SERVICE, "GET_SUPPORTED_PROTO_RESP", FROM_BROKER, ON_UDS_BROKERS_AGENT, IGNORE,
#endif

#ifndef REMOVE_USP_BROKER
// Broker-as-Agent receiving from ACS Controller via STOMP
"201",     AS_BROKER, "GET", FROM_ACS, ON_STOMP, ACCEPT,
"202",     AS_BROKER, "SET", FROM_ACS, ON_STOMP, ACCEPT,
"203",     AS_BROKER, "ADD", FROM_ACS, ON_STOMP, ACCEPT,
"204",     AS_BROKER, "DELETE", FROM_ACS, ON_STOMP, ACCEPT,
"205",     AS_BROKER, "OPERATE", FROM_ACS, ON_STOMP, ACCEPT,
"206",     AS_BROKER, "GET_SUPPORTED_DM", FROM_ACS, ON_STOMP, ACCEPT,
"207",     AS_BROKER, "GET_INSTANCES", FROM_ACS, ON_STOMP, ACCEPT,
"208",     AS_BROKER, "NOTIFY_RESP", FROM_ACS, ON_STOMP, ACCEPT,
"209",     AS_BROKER, "GET_SUPPORTED_PROTO", FROM_ACS, ON_STOMP, ACCEPT,
"210",     AS_BROKER, "REGISTER", FROM_ACS, ON_STOMP, ERROR,
"211",     AS_BROKER, "ERROR", FROM_ACS, ON_STOMP, ACCEPT,
"212",     AS_BROKER, "DEREGISTER_RESP", FROM_ACS, ON_STOMP, IGNORE,
"213",     AS_BROKER, "GET_RESP", FROM_ACS, ON_STOMP, IGNORE,
"214",     AS_BROKER, "SET_RESP", FROM_ACS, ON_STOMP, IGNORE,
"215",     AS_BROKER, "ADD_RESP", FROM_ACS, ON_STOMP, IGNORE,
"216",     AS_BROKER, "DELETE_RESP", FROM_ACS, ON_STOMP, IGNORE,
"217",     AS_BROKER, "OPERATE_RESP", FROM_ACS, ON_STOMP, IGNORE,
"218",     AS_BROKER, "GET_SUPPORTED_DM_RESP", FROM_ACS, ON_STOMP, IGNORE,
"219",     AS_BROKER, "GET_INSTANCES_RESP", FROM_ACS, ON_STOMP, IGNORE,
"220",     AS_BROKER, "GET_SUPPORTED_PROTO_RESP", FROM_ACS, ON_STOMP, IGNORE,
"221",     AS_BROKER, "REGISTER_RESP", FROM_ACS, ON_STOMP, IGNORE,
"222",     AS_BROKER, "NOTIFY", FROM_ACS, ON_STOMP, ERROR,
"223",     AS_BROKER, "DEREGISTER", FROM_ACS, ON_STOMP, ERROR,

// Broker-as-Agent receiving from Service via UDS: Broker's Agent socket
"301",     AS_BROKER, "GET", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"302",     AS_BROKER, "SET", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"303",     AS_BROKER, "ADD", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"304",     AS_BROKER, "DELETE", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"305",     AS_BROKER, "OPERATE", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"306",     AS_BROKER, "GET_SUPPORTED_DM", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"307",     AS_BROKER, "GET_INSTANCES", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"308",     AS_BROKER, "NOTIFY_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"309",     AS_BROKER, "GET_SUPPORTED_PROTO", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"310",     AS_BROKER, "REGISTER_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"311",     AS_BROKER, "ERROR", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ACCEPT,
"312",     AS_BROKER, "DEREGISTER_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"313",     AS_BROKER, "GET_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"314",     AS_BROKER, "SET_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"315",     AS_BROKER, "ADD_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"316",     AS_BROKER, "DELETE_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"317",     AS_BROKER, "OPERATE_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"318",     AS_BROKER, "GET_SUPPORTED_DM_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"319",     AS_BROKER, "GET_INSTANCES_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"320",     AS_BROKER, "GET_SUPPORTED_PROTO_RESP", FROM_SERVICE, ON_UDS_BROKERS_AGENT, IGNORE,
"321",     AS_BROKER, "NOTIFY", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ERROR,
"322",     AS_BROKER, "REGISTER", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ERROR,
"323",     AS_BROKER, "DEREGISTER", FROM_SERVICE, ON_UDS_BROKERS_AGENT, ERROR,

// Broker-as-Controller receiving from Service via UDS: Broker's Controller socket
"401",     AS_BROKER, "NOTIFY", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"402",     AS_BROKER, "ERROR", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"403",     AS_BROKER, "REGISTER", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"404",     AS_BROKER, "GET_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"405",     AS_BROKER, "SET_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"406",     AS_BROKER, "ADD_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"407",     AS_BROKER, "DELETE_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"408",     AS_BROKER, "OPERATE_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"409",     AS_BROKER, "GET_SUPPORTED_DM_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"410",     AS_BROKER, "GET_INSTANCES_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"411",     AS_BROKER, "GET_SUPPORTED_PROTO_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ACCEPT,
"412",     AS_BROKER, "REGISTER_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"413",     AS_BROKER, "DEREGISTER_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"414",     AS_BROKER, "NOTIFY_RESP", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, IGNORE,
"415",     AS_BROKER, "GET", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"416",     AS_BROKER, "SET", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"417",     AS_BROKER, "ADD", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"418",     AS_BROKER, "DELETE", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"419",     AS_BROKER, "OPERATE", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"420",     AS_BROKER, "GET_SUPPORTED_DM", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"421",     AS_BROKER, "GET_INSTANCES", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"422",     AS_BROKER, "GET_SUPPORTED_PROTO", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,
"423",     AS_BROKER, "DEREGISTER", FROM_SERVICE, ON_UDS_BROKERS_CONTROLLER, ERROR,

// Broker receiving from Service via STOMP
// NOTE: For these sets of tests to pass, a USP Service must have actually connected via STOMP and sent a Register request
//"501",     AS_BROKER, "GET", FROM_SERVICE, ON_STOMP, ACCEPT,
//"502",     AS_BROKER, "SET", FROM_SERVICE, ON_STOMP, ACCEPT,
//"503",     AS_BROKER, "ADD", FROM_SERVICE, ON_STOMP, ACCEPT,
//"504",     AS_BROKER, "DELETE", FROM_SERVICE, ON_STOMP, ACCEPT,
//"505",     AS_BROKER, "OPERATE", FROM_SERVICE, ON_STOMP, ACCEPT,
//"506",     AS_BROKER, "GET_SUPPORTED_DM", FROM_SERVICE, ON_STOMP, ACCEPT,
//"507",     AS_BROKER, "GET_INSTANCES", FROM_SERVICE, ON_STOMP, ACCEPT,
//"508",     AS_BROKER, "NOTIFY_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"509",     AS_BROKER, "GET_SUPPORTED_PROTO", FROM_SERVICE, ON_STOMP, ACCEPT,
//"510",     AS_BROKER, "REGISTER_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"511",     AS_BROKER, "NOTIFY", FROM_SERVICE, ON_STOMP, ACCEPT,
//"512",     AS_BROKER, "REGISTER", FROM_SERVICE, ON_STOMP, ACCEPT,
//"513",     AS_BROKER, "ERROR", FROM_SERVICE, ON_STOMP, ACCEPT,
//"514",     AS_BROKER, "GET_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"515",     AS_BROKER, "SET_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"516",     AS_BROKER, "ADD_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"517",     AS_BROKER, "DELETE_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"518",     AS_BROKER, "OPERATE_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"519",     AS_BROKER, "GET_SUPPORTED_DM_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"520",     AS_BROKER, "GET_INSTANCES_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"521",     AS_BROKER, "GET_SUPPORTED_PROTO_RESP", FROM_SERVICE, ON_STOMP, ACCEPT,
//"522",     AS_BROKER, "DEREGISTER_RESP", FROM_SERVICE, ON_STOMP, IGNORE,
//"523",     AS_BROKER, "DEREGISTER", FROM_SERVICE, ON_STOMP, ERROR,
#endif

};

void Test_ValidateUspMsgType(void)
{
    int i;
    Usp__Header__MsgType msg_type;
    mtp_conn_t mtpc;
    int expected;
    int err;
    int failure_count = 0;

    for (i=0; i<NUM_ELEM(validate_msg_test_cases); i+=6)
    {
#ifndef REMOVE_USP_SERVICE
        usp_service_objects = validate_msg_test_cases[i+1];
#endif
        msg_type = TEXT_UTILS_StringToEnum(validate_msg_test_cases[i+2], usp_msg_types, NUM_ELEM(usp_msg_types));
        if (msg_type==INVALID)
        {
            USP_LOG_Error("%s: Unknown msg type ('%s') in test case %s", __FUNCTION__, validate_msg_test_cases[i+2], validate_msg_test_cases[i]);
            exit(-1);
        }

        memset(&mtpc, 0, sizeof(mtpc));
#ifndef DISABLE_STOMP
        if (strcmp(validate_msg_test_cases[i+4], ON_STOMP)==0)
        {
            mtpc.protocol = kMtpProtocol_STOMP;
        }
#endif
#ifdef ENABLE_UDS
        else if (strcmp(validate_msg_test_cases[i+4], ON_UDS_BROKERS_CONTROLLER)==0)
        {
            mtpc.protocol = kMtpProtocol_UDS;
            mtpc.uds.path_type = kUdsPathType_BrokersController;
        }
        else if (strcmp(validate_msg_test_cases[i+4], ON_UDS_BROKERS_AGENT)==0)
        {
            mtpc.protocol = kMtpProtocol_UDS;
            mtpc.uds.path_type = kUdsPathType_BrokersAgent;
        }
#endif
        else
        {
            USP_LOG_Error("%s: Unknown protocol ('%s') in test case %s", __FUNCTION__, validate_msg_test_cases[i+4], validate_msg_test_cases[i]);
            exit(-1);
        }

        // Run the test case and log the results
        err = ValidateUspMsgType(msg_type, validate_msg_test_cases[i+3], &mtpc, "MSG-1");
        expected = atoi(validate_msg_test_cases[i+5]);
        if (expected != err)
        {
            USP_LOG_Error("****** FAIL: test case %s (got err=%d, expected=%d) *****", validate_msg_test_cases[i], err, expected);
            failure_count++;
        }
        else
        {
            USP_LOG_Info("PASS: test case %s", validate_msg_test_cases[i]);
        }
    }

    USP_LOG_Info("\nFailure Count=%d", failure_count);
    exit(0);
}

#endif
