/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
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

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include <inttypes.h>  // For PRIu64
#include "e2e_context.h"
#endif

//------------------------------------------------------------------------
// Index of the controller that sent the current USP message being processed
// This needs to be saved off, in order that it can be used by data model transaction update notify callbacks
static int cur_msg_controller_instance = INVALID;

//------------------------------------------------------------------------
// Role to use with current USP message
// This is saved off before handling each message, as each message handler needs it fairly deeply in its processing
static combined_role_t cur_msg_combined_role = { ROLE_DEFAULT, ROLE_DEFAULT};

//------------------------------------------------------------------------
// Role to use with current USP message
static controller_info_t cur_msg_controller_info;

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
    { USP__HEADER__MSG_TYPE__DELETE,           "DEL"},
    { USP__HEADER__MSG_TYPE__DELETE_RESP,      "DELETE_RESP"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM, "GET_SUPPORTED_DM"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP, "GET_SUPPORTED_DM_RESP"},
    { USP__HEADER__MSG_TYPE__GET_INSTANCES,    "GET_INSTANCES"},
    { USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP, "GET_INSTANCES_RESP"},
    { USP__HEADER__MSG_TYPE__NOTIFY_RESP,      "NOTIFY_RESP"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO, "GET_SUPPORTED_PROTO"},
    { USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP, "GET_SUPPORTED_PROTO_RESP"}
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
int HandleUspMessage(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt);
int ValidateUspRecord(UspRecord__Record *rec, mtp_reply_to_t *mrt);
char *MtpSendItemToString(mtp_send_item_t *msi);
void CacheControllerRoleForCurMsg(char *endpoint_id, ctrust_role_t role, mtp_protocol_t protocol);
int QueueUspNoSessionRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id, mtp_reply_to_t *mrt, time_t expiry_time);

/*********************************************************************//**
**
** MSG_HANDLER_HandleBinaryRecord
**
** Main entry point to handling an incoming USP Record (which encapsulates a USP Message)
** NOTE: Parsing errors are handled locally by this function
**
** \param   pbuf - pointer to buffer containing protobuf encoded USP record
** \param   pbuf_len - length of protobuf encoded message
** \param   role - Role allowed for this message
** \param   mrt - MTP details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_HandleBinaryRecord(unsigned char *pbuf, int pbuf_len, ctrust_role_t role, mtp_reply_to_t *mrt)
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

    // Save off the controller sending this message into the mtp_reply_to_t structure, if not already
    // set by MTP (websockets can get this from Sec-WebSocket_extensions header))
    if (mrt->cont_endpoint_id == NULL)
    {
        mrt->cont_endpoint_id = USP_STRDUP(rec->from_id);
    }

#ifdef ENABLE_WEBSOCKETS
    // Exit if USP record received from the agent's websocket server is from a controller that is
    // already connected via the agent's websocket client (only one connection to a controller is allowed)
    if ((mrt->protocol == kMtpProtocol_WebSockets) && (mrt->wsserv_conn_id != INVALID))
    {
        if (WSCLIENT_IsEndpointConnected(rec->from_id))
        {
            USP_ERR_SetMessage("%s: Not permitting controller eid='%s' to connect. Already connected via agent's websocket client", __FUNCTION__, rec->from_id);
            err = USP_ERR_REQUEST_DENIED;
            goto exit;
        }
    }
#endif

    // Exit if USP record failed validation
    err = ValidateUspRecord(rec, mrt);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Print USP Record header in human readable form
    PROTO_TRACE_ProtobufMessage(&rec->base);

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    // Process the USP Record through the End-to-End exchange context.
    err = E2E_CONTEXT_HandleUspRecord(rec, role, mrt);
#else
    // Process directly the encapsulated USP Message contained in the USP Record struct.
    err = MSG_HANDLER_HandleBinaryMessage(rec->no_session_context->payload.data,
                                          rec->no_session_context->payload.len,
                                          role, rec->from_id, mrt);
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
** \param   role - Role allowed for this message
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful, USP_ERR_MESSAGE_NOT_UNDERSTOOD if unable to unpack the USP Record
**
**************************************************************************/
int MSG_HANDLER_HandleBinaryMessage(unsigned char *pbuf, int pbuf_len, ctrust_role_t role, char *controller_endpoint, mtp_reply_to_t *mrt)
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
    CacheControllerRoleForCurMsg(controller_endpoint, role, mrt->protocol);

    // Print USP message in human readable form
    PROTO_TRACE_ProtobufMessage(&usp->base);

    // Exit if unable to process the message
    err = HandleUspMessage(usp, controller_endpoint, mrt);
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
** \param   stomp_header - pointer to string containing the STOMP header (if sent over STOMP, NULL otherwise)
**                         This is only used for debug purposes
**
** \return  None
**
**************************************************************************/
void MSG_HANDLER_LogMessageToSend(mtp_send_item_t *msi,
                                  mtp_protocol_t protocol, char *host,
                                  unsigned char *stomp_header)
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
    if ((enable_protocol_trace) && (stomp_header != NULL))
    {
        USP_PROTOCOL("%s", stomp_header);
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
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
** \param   msg_id - String containing the message ID of the USP message which caused this error
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_QueueErrorMessage(int err, char *controller_endpoint, mtp_reply_to_t *mrt, char *msg_id)
{
    Usp__Msg *resp;
    resp = ERROR_RESP_CreateSingle(msg_id, err, NULL, NULL);
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mrt);
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
** \param   mrt - details of where this USP response message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_QueueMessage(char *endpoint_id, Usp__Msg *usp, mtp_reply_to_t *mrt)
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
    err = MSG_HANDLER_QueueUspRecord(&usp_send_item, endpoint_id, usp->header->msg_id, mrt, END_OF_TIME);

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
** \param   mrt - details of where this USP response message should be sent
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_QueueUspRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id, mtp_reply_to_t *mrt, time_t expiry_time)
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
        err = E2E_CONTEXT_QueueUspSessionRecord(usi, endpoint_id, usp_msg_id, mrt, expiry_time);
    }
    else
#endif
    {
        err = QueueUspNoSessionRecord(usi, endpoint_id, usp_msg_id, mrt, expiry_time);
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
** \param   mrt - details of where this USP record should be sent
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_HANDLER_QueueUspDisconnectRecord(mtp_content_type_t content_type, char *cont_endpoint_id, uint32_t reason_code, char* reason_str, mtp_reply_to_t *mrt, time_t expiry_time)
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
    err = DEVICE_CONTROLLER_QueueBinaryMessage(&mtp_send_item, cont_endpoint_id, NULL, mrt, expiry_time);
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
** HandleUspMessage
**
** Main entry point to handling a message
** NOTE: Parsing errors are handled locally by this function
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int HandleUspMessage(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt)
{
    int err = USP_ERR_OK;
    char buf[MAX_ISO8601_LEN];

    // Exit if the message was ill-formed
    if (usp->header == NULL)
    {
        USP_ERR_SetMessage("%s: Ignoring malformed USP message", __FUNCTION__);
        err = USP_ERR_MESSAGE_NOT_UNDERSTOOD;
        goto exit;
    }

    // Log the message
    USP_LOG_Info("%s : processing at time %s",
                MSG_HANDLER_UspMsgTypeToString(usp->header->msg_type),
                iso8601_cur_time(buf, sizeof(buf)) );

    // Process the message
    switch(usp->header->msg_type)
    {
        case USP__HEADER__MSG_TYPE__GET:
            MSG_HANDLER_HandleGet(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__SET:
            MSG_HANDLER_HandleSet(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__ADD:
            MSG_HANDLER_HandleAdd(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__DELETE:
            MSG_HANDLER_HandleDelete(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__OPERATE:
            MSG_HANDLER_HandleOperate(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__NOTIFY_RESP:
            MSG_HANDLER_HandleNotifyResp(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
            MSG_HANDLER_HandleGetSupportedProtocol(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__GET_INSTANCES:
            MSG_HANDLER_HandleGetInstances(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
            MSG_HANDLER_HandleGetSupportedDM(usp, controller_endpoint, mrt);
            break;

        case USP__HEADER__MSG_TYPE__NOTIFY:
            // Since Controllers shouldn't send Notify messages, according to R-MTP.5, a USP Error message should be sent in response
            USP_ERR_SetMessage("%s: Cannot handle USP message type %d. Sending back Error response", __FUNCTION__, usp->header->msg_type);
            MSG_HANDLER_QueueErrorMessage(USP_ERR_REQUEST_DENIED, controller_endpoint, mrt, usp->header->msg_id);
            break;

        default:
        case USP__HEADER__MSG_TYPE__ERROR:
        case USP__HEADER__MSG_TYPE__GET_RESP:
        case USP__HEADER__MSG_TYPE__SET_RESP:
        case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        case USP__HEADER__MSG_TYPE__ADD_RESP:
        case USP__HEADER__MSG_TYPE__DELETE_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP:
            // According to R-MTP.5, all received USP Error and USP Response messages should be ignored
            USP_ERR_SetMessage("%s: Cannot handle USP message type %d. Ignoring", __FUNCTION__, usp->header->msg_type);
            err = USP_ERR_REQUEST_DENIED;
            break;
    }

exit:
    cur_msg_controller_instance = INVALID;

    // Activate all STOMP reconnects or scheduled exits, now that we have queued all response messages
    MTP_EXEC_ActivateScheduledActions();

    return err;
}

/*********************************************************************//**
**
** ValidateUspRecord
**
** Validates whether a received USP record can be accepted by USP Agent for processing
** NOTE: Parsing errors are handled locally by this function
**
** \param   rec - pointer to protobuf structure describing the received USP record
** \param   mrt - MTP details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if record is valid
**
**************************************************************************/
int ValidateUspRecord(UspRecord__Record *rec, mtp_reply_to_t *mrt)
{
    int err;
    char *err_msg;
    char *endpoint_id;
    bool has_mtp;
    UspRecord__NoSessionContextRecord *ctx;

    // Exit if this record is not supposed to be processed by us
    endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    if ((rec->to_id == NULL) || (strcmp(rec->to_id, endpoint_id) != 0))
    {
        USP_ERR_SetMessage("%s: Ignoring USP record as it was addressed to endpoint_id=%s not %s", __FUNCTION__, rec->to_id, endpoint_id);
        return USP_ERR_REQUEST_DENIED;
    }

    // Exit if no controller endpoint_id to send the message back to
    if ((rec->from_id == NULL) || (rec->from_id[0] == '\0'))
    {
        USP_ERR_SetMessage("%s: Ignoring USP record as from_id is blank", __FUNCTION__);
        return USP_ERR_RECORD_FIELD_INVALID;
    }

    // Exit if the controller is unknown
    cur_msg_controller_instance = DEVICE_CONTROLLER_FindInstanceByEndpointId(rec->from_id);
    if (cur_msg_controller_instance == INVALID)
    {
        USP_ERR_SetMessage("%s: Ignoring message from endpoint_id=%s (unknown controller)", __FUNCTION__, rec->from_id);
        return USP_ERR_REQUEST_DENIED;
    }

    // Exit if we don't know where to send the response, ignoring the USP message
    // (because none was provided to the MTP when the message was received, and none is configured in the data model)
    if (mrt->is_reply_to_specified == false)
    {
        // Since no 'reply-to' was provided along with the received message, see if one is configured in the data model
        has_mtp = DEVICE_CONTROLLER_IsMTPConfigured(rec->from_id, mrt->protocol);
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
        MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mrt, END_OF_TIME);
        return err;
    }

    // Exit if this record contains an encrypted payload (which we don't yet support).
    if (rec->payload_security != USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT)
    {
        USP_ERR_SetMessage("%s: Ignoring E2E USP record containing an encrypted payload", __FUNCTION__);
        err = USP_ERR_SECURE_SESS_NOT_SUPPORTED;
        err_msg = USP_ERR_GetMessage();
        MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mrt, END_OF_TIME);
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
            MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mrt, END_OF_TIME);
            return err;
#endif
            break;

        case USP_RECORD__RECORD__RECORD_TYPE_DISCONNECT:
            // If we received a disconnect record, then initiate a disconnect
            USP_ERR_SetMessage("%s: USP Disconnect record received. Disconnecting.", __FUNCTION__);
            err = USP_ERR_SESS_CONTEXT_TERMINATED;
            err_msg = USP_ERR_GetMessage();
            MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_DisconnectRecord, rec->from_id, err, err_msg, mrt, END_OF_TIME);
            return err;
            break;

        default:
        case USP_RECORD__RECORD__RECORD_TYPE__NOT_SET:
        case USP_RECORD__RECORD__RECORD_TYPE_WEBSOCKET_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_MQTT_CONNECT:
        case USP_RECORD__RECORD__RECORD_TYPE_STOMP_CONNECT:
            // Exit if unsupported USP Record type OR unexpected record type for USP Agent
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
** CacheControllerRoleForCurMsg
**
** Retrieves the role to use for the specified controller, and caches it locally, so that
** it may be used subsequently when processing the current message by calling MSG_HANDLER_GetMsgRole()
**
** \param   endpoint_id - endpoint_id of the controller that has sent the current message being processed
** \param   role - Role allowed for this message from the MTP
** \param   protocol - protocol that the message was received on
**
** \return  None - if the controller is not recognised, then it will be granted an appropriately low set of permissions
**
**************************************************************************/
void CacheControllerRoleForCurMsg(char *endpoint_id, ctrust_role_t role, mtp_protocol_t protocol)
{
    int err;

    cur_msg_controller_info.endpoint_id = endpoint_id;

    // Get the combined role for this endpoint_id
    err = DEVICE_CONTROLLER_GetCombinedRoleByEndpointId(endpoint_id, &cur_msg_combined_role);
    if (err != USP_ERR_OK)
    {
        // If this is an unknown controller, then grant it a limited set of permissions
        cur_msg_combined_role.inherited = kCTrustRole_Untrusted;
        cur_msg_combined_role.assigned = INVALID_ROLE;
        return;
    }


    switch(protocol)
    {
#ifndef DISABLE_STOMP
        case kMtpProtocol_STOMP:
            // If the message was received over STOMP, then the inherited role will have been saved in DEVICE_CONTROLLER
            // when the STOMP handshake completed and will already equal the role passed with the USP message
            USP_ASSERT(cur_msg_combined_role.inherited == role);
            break;
#endif

#ifdef ENABLE_COAP
        case kMtpProtocol_CoAP:
            // If the message was received over CoAP, then the inherited role won't have been saved in DEVICE_CONTROLLER,
            // so override with the role that was passed with the USP message
            USP_ASSERT(cur_msg_combined_role.inherited == ROLE_DEFAULT);
            cur_msg_combined_role.inherited = role;
            break;
#endif

#ifdef ENABLE_MQTT
        case kMtpProtocol_MQTT:
            USP_ASSERT(cur_msg_combined_role.inherited == role);
            break;
#endif

#ifdef ENABLE_WEBSOCKETS
        case kMtpProtocol_WebSockets:
            // If the message was received over WebSockets, then the inherited role won't have been saved in DEVICE_CONTROLLER,
            // so override with the role that was passed with the USP message
            USP_ASSERT(cur_msg_combined_role.inherited == ROLE_DEFAULT);
            cur_msg_combined_role.inherited = role;
            break;
#endif
        default:
            TERMINATE_BAD_CASE(protocol);
            break;
    }
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
** \param   mrt - details of where this USP response message should be sent
** \param   expiry_time - time at which the USP record should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int QueueUspNoSessionRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id,
                            mtp_reply_to_t *mrt, time_t expiry_time)
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
    err = DEVICE_CONTROLLER_QueueBinaryMessage(&mtp_send_item, endpoint_id, usp_msg_id, mrt, expiry_time);
    if (err != USP_ERR_OK)
    {
        USP_FREE(mtp_send_item.pbuf);
    }

    return err;
}
