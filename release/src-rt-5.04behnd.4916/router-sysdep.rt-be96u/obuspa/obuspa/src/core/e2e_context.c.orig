/*
 *
 * Copyright (C) 2022, Broadband Forum
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
 * \file e2e_context.c
 *
 * End-to-End exchange utilities.
 *
 */

#include "vendor_defs.h"  // For E2ESESSION_EXPERIMENTAL_USP_V_1_2

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include <inttypes.h>  // For PRIu64
#include <math.h>

#include "mtp_exec.h"
#include "msg_handler.h"
#include "e2e_context.h"
#include "iso8601.h"
#include "proto_trace.h"
#include "text_utils.h"

const enum_entry_t e2e_session_events[] = {
    { kE2EEvent_None,           "None"},
    { kE2EEvent_Establishment,  "Establishment"},
    { kE2EEvent_Termination,    "Termination"},
    { kE2EEvent_Restart,        "Restart"},
};

const enum_entry_t e2e_session_status[] = {
    { kE2EStatus_Up,          "Up"},
    { kE2EStatus_Negotiating, "Negotiating"},
    { kE2EStatus_Down,        "Down"},
};

const enum_entry_t e2e_session_modes[] = {
    { kE2EMode_Require, "Require"},
    { kE2EMode_Allow,   "Allow"},
    { kE2EMode_Forbid,  "Forbid"},
};

const enum_entry_t sar_states[] = {
    { USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__NONE,      "NONE"},
    { USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__BEGIN,     "BEGIN"},
    { USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__INPROCESS, "INPROCESS"},
    { USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__COMPLETE,  "COMPLETE"},
};

int HandleSessionContextRecord(UspRecord__Record *rec, ctrust_role_t role, mtp_reply_to_t *mrt);
void ClearE2eSessionState(e2e_session_t * e2e);
int ValidateNoSessionContextHandling(UspRecord__Record *rec);
int ValidateSessionContextHandling(UspRecord__Record *rec, mtp_reply_to_t *mrt);
bool IsValidE2eSarState(sar_vector_t *sar_vector, int sar_state);
bool IsValidSessionId(uint64_t session_id);
unsigned ComputePayloadCapacity(UspRecord__Record *src_rec, unsigned max_record_size);

/*********************************************************************//**
**
** E2E_CONTEXT_QueueUspSessionRecord
**
** Serializes a protobuf USP SessionContext Record structure for each
** segmented USP Message, then queues it, to be sent to a controller.
** The End-to-End Session Context of the associated controller is used by
** the encapsulation mechanism of the segmented payloads.
**
** \param   usi - Information about the USP Message to send
** \param   endpoint_id - controller to send the message to
** \param   usp_msg_id - pointer to string containing the msg_id of the serialized USP Message
** \param   mrt - details of where this USP response message should be sent
** \param   expiry_time - time at which the USP message should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int E2E_CONTEXT_QueueUspSessionRecord(usp_send_item_t *usi, char *endpoint_id, char *usp_msg_id,
                                      mtp_reply_to_t *mrt, time_t expiry_time)
{
    // TODO: Incomplete implementation.
    // This implementation does not cope with:
    // - retransmission of USP Records,
    // - sequence_id exhaustion,
    // - USP Record integrity (e.g. MAC, signature)
    int err = USP_ERR_OK;
    mtp_send_item_t mtp_send_item;
    UspRecord__SessionContextRecord ctxSession;
    UspRecord__Record rec;
    e2e_session_t *e2esession = usi->curr_e2e_session;
    unsigned max_payload_size = UINT_MAX;
    int bytes_queued = 0;
    const bool segmentation_enabled = (e2esession->max_record_size > 0);
    mtp_content_type_t content_type;

    USP_LOG_Debug("%s: Length of the USP Message to send: %d bytes", __FUNCTION__, usi->msg_packed_size);

    // Log the USP Message (if available)
    // For E2E records, logging is performed here, as it cannot be done later by the MTP, as the message is segmented by then
    // NOTE: The MTP does not log USP records of content_type=kMtpContentType_E2E_XXX, as they could be segmented
    if (usi->usp_msg != NULL)
    {
        char time_buf[MAX_ISO8601_LEN];
        USP_PROTOCOL("\n");
        USP_LOG_Info("%s built at time %s",
                     MSG_HANDLER_UspMsgTypeToString(usi->usp_msg->header->msg_type),
                     iso8601_cur_time(time_buf, sizeof(time_buf)));
        PROTO_TRACE_ProtobufMessage(&usi->usp_msg->base);
    }

    // Establish an E2E Session Context if not already set
    USP_ASSERT(e2esession != NULL);
    if (e2esession->status == kE2EStatus_Down)
    {
        e2esession->current_session_id++;  // Use the next session_id
        e2esession->last_sent_sequence_id = 0;
        e2esession->last_recv_sequence_id = 0;
        e2esession->status = kE2EStatus_Negotiating;
    }

    usp_record__session_context_record__init(&ctxSession);
    ctxSession.session_id = e2esession->current_session_id;
    ctxSession.sequence_id = (e2esession->last_sent_sequence_id + 1);  // The seq_id is set again later during the while loop
    ctxSession.expected_id = (e2esession->last_recv_sequence_id + 1);
    ctxSession.retransmit_id = 0;

    // Fill in the USP Record structure with common values for all USP Records during the E2ESession
    usp_record__record__init(&rec);
    rec.version = AGENT_CURRENT_PROTOCOL_VERSION;
    rec.to_id = endpoint_id;
    rec.from_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    rec.record_type_case = USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT;
    rec.payload_security = USP_RECORD__RECORD__PAYLOAD_SECURITY__PLAINTEXT;
    rec.session_context = &ctxSession;

    // Calculate the maximal allowed payload size according to USP Record fields
    if (segmentation_enabled)
    {
        max_payload_size = ComputePayloadCapacity(&rec, e2esession->max_record_size);
    }

    // Segmenting USP Message into USP Record payloads
    USP_LOG_Debug("%s: USP Message of %d bytes will be segmented in at least %d segments (%u bytes max by segment).",
                  __FUNCTION__,
                  usi->msg_packed_size,
                  (int)ceil((double)usi->msg_packed_size / (double)max_payload_size),
                  max_payload_size);
    USP_ASSERT(max_payload_size > 50);  // If it comes so small, it means there is an algo problem in obuspa.

    do
    {
        ProtobufCBinaryData segment;
        const int remaining_size = usi->msg_packed_size - bytes_queued;

        segment.len = MIN(max_payload_size, remaining_size);
        segment.data = usi->msg_packed + bytes_queued;

        // USP Record contains only one plaintext USP Message segment.
        ctxSession.n_payload = 1;
        ctxSession.payload = &segment;

        // This is the first USP Record in SAR
        if (bytes_queued == 0)
        {
            // ... and this is also the last segment, so there's no segmentation.
            if (remaining_size <= segment.len)
            {
                ctxSession.payload_sar_state = USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__NONE;
                content_type = kMtpContentType_E2E_FullMessage;
            }
            else
            {
                ctxSession.payload_sar_state = USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__BEGIN;
                content_type = kMtpContentType_E2E_Begin;
            }
        }
        // No more segmentation required; this is the last segment
        else if (remaining_size <= segment.len)
        {
            ctxSession.payload_sar_state = USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__COMPLETE;
            content_type = kMtpContentType_E2E_Complete;
        }
        else
        {
            ctxSession.payload_sar_state = USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__INPROCESS;
            content_type = kMtpContentType_E2E_InProcess;
        }

        // Assign the sequence_id for this USP Record.
        // and update the last_sent_sequence_id.
        ctxSession.sequence_id = ++(e2esession->last_sent_sequence_id);

        // In case of PLAINTEXT, the payloadrec_sar_state is always
        // equal to payload_sar_state.
        ctxSession.payloadrec_sar_state = ctxSession.payload_sar_state;

        // Serialize the protobuf record structure into a buffer
        {
            const int len = usp_record__record__get_packed_size(&rec);
            uint8_t *buf = USP_MALLOC(len);
            const int size = usp_record__record__pack(&rec, buf);

            // If these are not equal, then we may have had a buffer overrun, so terminate
            USP_ASSERT(size == len);

            // Prepare the MTP item information now it is serialized.
            MTP_EXEC_MtpSendItem_Init(&mtp_send_item);
            mtp_send_item.usp_msg_type = usi->usp_msg_type;
            mtp_send_item.content_type = content_type;
            mtp_send_item.pbuf = buf;
            mtp_send_item.pbuf_len = len;
        }

        if (segmentation_enabled &&
            (mtp_send_item.pbuf_len > e2esession->max_record_size))
        {
            USP_ERR_SetMessage("%s: Packed USP Record size (%d) exceeds the maximum (%u).",
                               __FUNCTION__, mtp_send_item.pbuf_len, e2esession->max_record_size);
            USP_FREE(mtp_send_item.pbuf);
            return USP_ERR_GENERAL_FAILURE;
        }

        // Exit if unable to queue the message, to send to a controller
        // NOTE: If successful, ownership of the buffer passes to the MTP layer. If not successful, buffer is freed here
        err = DEVICE_CONTROLLER_QueueBinaryMessage(&mtp_send_item, endpoint_id, usp_msg_id, mrt, expiry_time);
        if (err != USP_ERR_OK)
        {
            USP_FREE(mtp_send_item.pbuf);
            return err;
        }

        // Move the buffer iterator to the next segment
        bytes_queued += segment.len;
    } while (bytes_queued < usi->msg_packed_size);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** E2E_CONTEXT_HandleUspRecord
**
** Handle a USP Record struct and switch between a SessionContext or
** NoSessionContext USP Record handler.
**
** \param   rec - pointer to unpacked USP Record struct containing chuncked payload
** \param   role - Role allowed for this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int E2E_CONTEXT_HandleUspRecord(UspRecord__Record *rec, ctrust_role_t role, mtp_reply_to_t *mrt)
{
    int err = USP_ERR_OK;

    switch (rec->record_type_case)
    {
        case USP_RECORD__RECORD__RECORD_TYPE_NO_SESSION_CONTEXT:
        {
            err = ValidateNoSessionContextHandling(rec);
            if (err != USP_ERR_OK)
            {
                // If an error did occur, ValidateNoSessionContextHandling will send an E2E session termination disconnect record
                // Do not return an error, as this would cause a USP disconnect record to be sent in ProcessBinaryUspRecord(),
                // causing the MTP to be disconnected
                err = USP_ERR_OK;
                goto exit;
            }

            // Process the encapsulated USP Message in a NoSessionContext record
            err = MSG_HANDLER_HandleBinaryMessage(rec->no_session_context->payload.data,
                                                  rec->no_session_context->payload.len,
                                                  role, rec->from_id, mrt);
            break;
        }

        case USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT:
        {
            err = ValidateSessionContextHandling(rec, mrt);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Process the encapsulated USP Message segment in a SessionContext record
            err = HandleSessionContextRecord(rec, role, mrt);
            break;
        }

        default:
            TERMINATE_BAD_CASE(rec->record_type_case);
            break;
    }

exit:
    return err;
}

/*********************************************************************//**
**
** E2E_CONTEXT_SarStateToString
**
** Convenience function to convert a SAR state enumeration of a Session
** Context record to a string.
**
** \param   state - protobuf enumeration of the SAR state of USP Session Context Record
**
** \return  pointer to string or 'UNKNOWN'
**
**************************************************************************/
char *E2E_CONTEXT_SarStateToString(int state)
{
    return TEXT_UTILS_EnumToString(state, sar_states, NUM_ELEM(sar_states));
}

/*********************************************************************//**
**
** E2E_CONTEXT_E2eSessionEventToString
**
** Convenience function to convert a E2E Session event enumeration
** to a string.
**
** \param   event - e2e_event_t enum
**
** \return  pointer to string or 'UNKNOWN'
**
**************************************************************************/
char *E2E_CONTEXT_E2eSessionEventToString(int event)
{
    return TEXT_UTILS_EnumToString(event, e2e_session_events, NUM_ELEM(e2e_session_events));
}

/*********************************************************************//**
**
** E2E_CONTEXT_E2eSessionStatusToString
**
** Convenience function to convert a E2E Session status enumeration
** to a string.
**
** \param   status - e2e_session_status_t enum
**
** \return  pointer to string or 'UNKNOWN'
**
**************************************************************************/
char *E2E_CONTEXT_E2eSessionStatusToString(int status)
{
    return TEXT_UTILS_EnumToString(status, e2e_session_status, NUM_ELEM(e2e_session_status));
}

/*********************************************************************//**
**
** E2E_CONTEXT_E2eSessionModeToString
**
** Convenience function to convert a E2E Session mode enumeration
** to a string.
**
** \param   mode - e2e_session_mode_t enum
**
** \return  pointer to string or 'UNKNOWN'
**
**************************************************************************/
char *E2E_CONTEXT_E2eSessionModeToString(int mode)
{
    return TEXT_UTILS_EnumToString(mode, e2e_session_modes, NUM_ELEM(e2e_session_modes));
}

/*********************************************************************//**
**
** E2E_CONTEXT_E2eSessionModeToEnum
**
** Convenience function to convert a E2E Session mode string
** to an enumerated integer representation.
**
** \param   str - pointer to string to convert
**
** \return  Enumerated value or INVALID if unable to convert
**
**************************************************************************/
int E2E_CONTEXT_E2eSessionModeToEnum(char *str)
{
    return TEXT_UTILS_StringToEnum(str, e2e_session_modes, NUM_ELEM(e2e_session_modes));
}

/*********************************************************************//**
**
** E2E_CONTEXT_E2eSessionEvent
**
** Process the E2E event and update the E2E Session according to.
**
** This function can be call when the E2ESession.SessionMode parameter
** changed or the E2ESession.Reset() operation is executed. In these situation,
** the DM_EXEC_PostE2eEvent() function posts an event, which will
** be handled by the data model's message queue..
**
** This function can also be called directly if the caller is also part
** of the data model thread, e.g., msg_handler.
**
** Note: The USP_ERR_GetMessage() is used as the reason string to send in the
**       USP Disconnect Record. If empty, the string representation of the
**       USP error code 7105 (Session Context terminated) is used.
**
** +---------------+-------------+-------------------+----------------------+
** |  e2e_event_t  |  E2E state  | Disconnect Record | Kick-off E2E Session |
** +---------------+-------------+-------------------+----------------------+
** | None          |     Kept    |      Not sent     |        Nothing       |
** | Establishment |   Cleared   |        Sent       |  Empty payload sent  |
** | Termination   |   Cleared   |        Sent       |        Nothing       |
** | Restart       |   Cleared   |        Sent       |  Empty payload sent  |
** +---------------+-------------+-------------------+----------------------+
**
** \param   event - E2E event type
** \param   request - Instance from the Device.LocalAgent.Request table,
**                    or INVALID when not called from the async operation
** \param   controller - Instance from the Device.LocalAgent.Controller table
**
** \return  None
**
**************************************************************************/
void E2E_CONTEXT_E2eSessionEvent(e2e_event_t event, int request, int controller)
{
    char err_msg[256];
    int err = USP_ERR_OK;
    char *dest_endpoint = NULL;
    mtp_reply_to_t mtp_reply_to = {0};  // Ensures mtp_reply_to.is_reply_to_specified=false
    usp_send_item_t usp_send_item;
    e2e_session_t* curr_e2e_session = NULL;
    char *event_str = NULL;

    event_str = E2E_CONTEXT_E2eSessionEventToString(event);
    USP_LOG_Debug("%s: Event %s received on Controller instance %d", __FUNCTION__, event_str, controller);

    curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(controller);
    if (curr_e2e_session == NULL)
    {
        USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: E2E Session Context not supported", __FUNCTION__);
        err = USP_ERR_SESS_CONTEXT_NOT_ALLOWED;
        goto exit;
    }

    // Apply the buffered SessionMode as the current mode.
    if (curr_e2e_session->mode != curr_e2e_session->mode_buffered)
    {
        USP_LOG_Debug("%s: SessionMode on Controller instance %d changed: %s => %s",
                      __FUNCTION__, controller,
                      E2E_CONTEXT_E2eSessionModeToString(curr_e2e_session->mode),
                      E2E_CONTEXT_E2eSessionModeToString(curr_e2e_session->mode_buffered));
        curr_e2e_session->mode = curr_e2e_session->mode_buffered;
    }

    if (event == kE2EEvent_None) { goto exit; }

    // Exit if unable to determine the endpoint of the controller
    // This could occur if the controller had been deleted
    dest_endpoint = DEVICE_CONTROLLER_FindEndpointIdByInstance(controller);
    if (dest_endpoint == NULL)
    {
        USP_LOG_Error("%s: dest_endpoint is NULL", __FUNCTION__);
        USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: Cannot find the destination endpoint", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Clear the E2ESession state of the controller
    ClearE2eSessionState(curr_e2e_session);

    if (event == kE2EEvent_Termination || event == kE2EEvent_Restart)
    {
        USP_ERR_ReplaceEmptyMessage("%s", USP_ERR_UspErrToString(USP_ERR_SESS_CONTEXT_TERMINATED));
        // Send the Disconnect Record to the related controller
        MSG_HANDLER_QueueUspDisconnectRecord(kMtpContentType_E2E_SessTermination,
                                             dest_endpoint,
                                             USP_ERR_SESS_CONTEXT_TERMINATED,
                                             USP_ERR_GetMessage(),
                                             &mtp_reply_to,
                                             END_OF_TIME);
    }

    if (event == kE2EEvent_Establishment || event == kE2EEvent_Restart)
    {
        // Send a USP Record with empty payload to kick off the E2E.
        MSG_HANDLER_UspSendItem_Init(&usp_send_item);
        usp_send_item.usp_msg_type = INVALID_USP_MSG_TYPE;
        usp_send_item.msg_packed_size = 0;
        usp_send_item.curr_e2e_session = curr_e2e_session;
        usp_send_item.usp_msg = NULL;
        MSG_HANDLER_QueueUspRecord(&usp_send_item, dest_endpoint, NULL, &mtp_reply_to, END_OF_TIME);
    }

exit:
    // Inform the protocol handler, that the operation has completed (through a Notify Message)
    if (request != INVALID)
    {
        USP_LOG_Info("=== E2ESession.Reset async operation completed on Controller instance %d ===", controller);
        USP_SIGNAL_OperationComplete(request, err, (err != USP_ERR_OK) ? err_msg : NULL, NULL);
    }

    USP_LOG_Debug("%s: Event %s completed on Controller instance %d", __FUNCTION__, event_str, controller);
}

/*********************************************************************//**
**
** E2E_CONTEXT_ValidateSessionContextRecord
**
** Validates whether a received USP SessionContext Record can be accepted
** by USP Agent for processing.
**
** \param   ctx - pointer to protobuf structure describing the received
**                SessionContext fields
**
** \return  USP_ERR_OK if record is valid
**
**************************************************************************/
int E2E_CONTEXT_ValidateSessionContextRecord(UspRecord__SessionContextRecord *ctx)
{
    // Exit if invalid protobuf structure, restarting the session content (R-E2E.23)
    if (ctx == NULL)
    {
        USP_ERR_SetMessage("%s: Ignoring E2E USP record without Session Context", __FUNCTION__);
        E2E_CONTEXT_E2eSessionEvent(kE2EEvent_Restart, INVALID, MSG_HANDLER_GetMsgControllerInstance());
        return USP_ERR_RECORD_NOT_PARSED;
    }

    // Exit if the SAR state fields are not sync'ed, restarting the session content (R-E2E.23)
    if (ctx->payloadrec_sar_state != ctx->payload_sar_state)
    {
        USP_ERR_SetMessage("%s: Ignoring E2E USP record with unsupported SAR states combination", __FUNCTION__);
        E2E_CONTEXT_E2eSessionEvent(kE2EEvent_Restart, INVALID, MSG_HANDLER_GetMsgControllerInstance());
        return USP_ERR_SECURE_SESS_NOT_SUPPORTED;
    }

    // Exit if this record does not contain exactly one payload, restarting the session content (R-E2E.23)
    if (ctx->n_payload != 1 || ctx->payload == NULL)
    {
        USP_ERR_SetMessage("%s: Ignoring E2E USP record without exactly one payload (n: %zu, p: %p)",
                           __FUNCTION__, ctx->n_payload, ctx->payload);
        E2E_CONTEXT_E2eSessionEvent(kE2EEvent_Restart, INVALID, MSG_HANDLER_GetMsgControllerInstance());
        return USP_ERR_RECORD_FIELD_INVALID;
    }

    // If the code gets here, then the SessionContext fields passed validation,
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** E2E_CONTEXT_IsToSendThroughSessionContext
**
** Convenience function to determine if the USP Message payload must be send
** within a USP Record of SessionContext type or not.
**
** \param   e2e - Associated E2E session for the USP Message to send.
**
** \return  true if the Session Context record type is to be used
**
**************************************************************************/
bool E2E_CONTEXT_IsToSendThroughSessionContext(e2e_session_t *e2e)
{
    if (e2e == NULL)
    {
        return false;
    }

    // If the E2E is active, use it
    if (e2e->status != kE2EStatus_Down)
    {
        return true;
    }

    // If SessionMode is set to Require, allow it.
    if (e2e->mode == kE2EMode_Require)
    {
        return true;
    }

    return false;
}

/*********************************************************************//**
**
** HandleSessionContextRecord
**
** Handle a SessionContext record which contains a segmented USP Message
** in its serialized form. The E2ESession context of the related controller
** is used to associated that payload segment to other segments previously
** or to be received.
**
** \param   rec - pointer to unpacked USP Record struct containing chuncked payload
** \param   role - Role allowed for this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int HandleSessionContextRecord(UspRecord__Record *rec, ctrust_role_t role, mtp_reply_to_t *mrt)
{
    // TODO: Incomplete implementation.
    // This implementation does not cope with:
    // - missing and out-of-order USP Records,
    // - duplicated USP Records,
    // - sequence_id exhaustion,
    // - USP Record integrity (e.g. MAC, signature)
    // It assumes that the received records:
    // - are valid (e.g. data integrity),
    // - are not duplicated (e.g. retransmissions).
    // - are all received (e.g. missing).

    // The protobuf struct must have exactly one payload (empty or not).
    USP_ASSERT(rec->session_context->payload != NULL);
    USP_ASSERT(rec->session_context->n_payload == 1);

    int err = USP_ERR_OK;
    const uint64_t recv_sess_id = rec->session_context->session_id;
    const uint64_t recv_seq_id = rec->session_context->sequence_id;
    const ProtobufCBinaryData recv_payload = rec->session_context->payload[0];
    e2e_session_t *curr_e2e_session = NULL;
    sar_vector_t *sar_vector = NULL;

    curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(MSG_HANDLER_GetMsgControllerInstance());
    sar_vector = &(curr_e2e_session->received_payloads);

    // If the USP Endpoint receives a USP Record with a different session_id field, the USP Endpoint
    // resets the Session Context and reuses the new session_id (and process the USP Record thereafter).
    if (curr_e2e_session->current_session_id != recv_sess_id)
    {
        USP_LOG_Info("%s: E2E Session Context has new session_id.", __FUNCTION__);
        ClearE2eSessionState(curr_e2e_session);
        curr_e2e_session->current_session_id = recv_sess_id;
    }

    // Update the E2E Session Context
    curr_e2e_session->last_recv_sequence_id = recv_seq_id;
    curr_e2e_session->status = kE2EStatus_Up;

    // If an empty payload, skip the USP Message handling for that USP Record
    if (recv_payload.len == 0)
    {
        return USP_ERR_OK;
    }

    // If in 'None' SAR state, skips the buffering and send it directly to the USP Message handler
    if (rec->session_context->payload_sar_state == USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__NONE)
    {
        err = MSG_HANDLER_HandleBinaryMessage(recv_payload.data,
                                              recv_payload.len,
                                              role, rec->from_id, mrt);
        return err;
    }

    // Append the payload in the SAR vector.
    if (!SAR_VECTOR_Append(sar_vector,
                           recv_sess_id,
                           recv_seq_id,
                           recv_payload.data,
                           recv_payload.len))
    {
        // If false is returned, the segment is not valid according to the SAR vector.
        // So terminate/restart the E2E session in that case.
        const e2e_event_t event = (curr_e2e_session->mode == kE2EMode_Require) ? kE2EEvent_Restart : kE2EEvent_Termination;
        USP_ERR_SetMessage("%s: USP Record segment caused an error (sess: %"PRIu64" seq: %"PRIu64").",
                           __FUNCTION__,
                           recv_sess_id,
                           recv_seq_id);
        E2E_CONTEXT_E2eSessionEvent(event, INVALID, MSG_HANDLER_GetMsgControllerInstance());
        return USP_ERR_OK;
    }

    // If this is the last segment, reassemble and process the USP Message
    if (rec->session_context->payload_sar_state == USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__COMPLETE)
    {
        int reassembled_size = 0;
        unsigned char *buf = SAR_VECTOR_Serialize(sar_vector, &reassembled_size);  // The ownership of the memory buffer belongs to the caller.

        // SAR vector is no more needed, free it now.
        SAR_VECTOR_Destroy(sar_vector);

        // Process the serialized USP Message
        err = USP_ERR_INTERNAL_ERROR;
        if (buf)
        {
            err = MSG_HANDLER_HandleBinaryMessage(buf, reassembled_size, role, rec->from_id, mrt);
        }

        USP_SAFE_FREE(buf);
    }

    return err;
}

/*********************************************************************//**
**
** ClearE2eSessionState
**
** Clean the state variables of the E2E Session structure.
**
** This call clears the volatile information about the Session Context, i.e.:
** - The E2ESession.Status
** - The SAR vector structure
** - The last exchanged sequence_ids
** - The originator flag
**
** This excludes:
** - The configuration related to the E2ESession datamodel is not changed
** - The session_id is neither changed (must be remembered for the
**   next E2E establishment)
**
** \param   e2e - pointer to E2E Session structure to clean up
**
** \return  None
**
**************************************************************************/
void ClearE2eSessionState(e2e_session_t *e2e)
{
    e2e->status = kE2EStatus_Down;
    e2e->last_sent_sequence_id = 0;
    e2e->last_recv_sequence_id = 0;
    if (e2e->received_payloads.num_entries > 0)
    {
        USP_LOG_Warning("%s: The %d USP Message segments are removed of the SAR buffer.",
                        __FUNCTION__, e2e->received_payloads.num_entries);
    }
    SAR_VECTOR_Destroy(&e2e->received_payloads);
    USP_LOG_Info("%s: The E2E Session is reset.", __FUNCTION__);
}

/*********************************************************************//**
**
** ValidateNoSessionContextHandling
**
** Validates whether a received USP NoSessionContext Record can be handled
** by USP Agent for processing according to the E2ESession state.
**
** \param   rec - pointer to protobuf structure describing the received USP record
**
** \return  USP_ERR_OK if record is valid
**
**************************************************************************/
int ValidateNoSessionContextHandling(UspRecord__Record *rec)
{
    const int ctrl_inst = MSG_HANDLER_GetMsgControllerInstance();
    e2e_session_t *curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(ctrl_inst);

    USP_ASSERT(rec->record_type_case == USP_RECORD__RECORD__RECORD_TYPE_NO_SESSION_CONTEXT);

    if (curr_e2e_session == NULL)
    {
        return USP_ERR_OK;
    }

    // SessionContext record is mandatory: restart the E2E Session
    if (curr_e2e_session->mode == kE2EMode_Require)
    {
        USP_ERR_SetMessage("%s: SessionContext record is mandatory.", __FUNCTION__);
        E2E_CONTEXT_E2eSessionEvent(kE2EEvent_Restart, INVALID, ctrl_inst);
        return USP_ERR_INVALID_CONFIGURATION;
    }

    // The E2ESession is still up: terminate the E2E Session
    if (curr_e2e_session->status != kE2EStatus_Down)
    {
        USP_ERR_SetMessage("%s: The E2E Session is still up.", __FUNCTION__);
        E2E_CONTEXT_E2eSessionEvent(kE2EEvent_Termination, INVALID, ctrl_inst);
        return USP_ERR_INVALID_CONFIGURATION;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ValidateSessionContextHandling
**
** Validates whether a received USP SessionContext Record can be handled
** by USP Agent for processing according to the E2ESession state.
**
** \param   rec - pointer to protobuf structure describing the received USP record
** \param   mrt - details of where response to this USP message should be sent
**
** \return  USP_ERR_OK if record is valid
**
**************************************************************************/
int ValidateSessionContextHandling(UspRecord__Record *rec, mtp_reply_to_t *mrt)
{
    int err = USP_ERR_OK;
    const int ctrl_inst = MSG_HANDLER_GetMsgControllerInstance();
    e2e_session_t *curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(ctrl_inst);
    UspRecord__SessionContextRecord *ctx = NULL;

    USP_ASSERT(rec->record_type_case == USP_RECORD__RECORD__RECORD_TYPE_SESSION_CONTEXT);
    ctx = rec->session_context;

    // Exit if E2ESession is forbidden
    if ((curr_e2e_session == NULL) ||
        (curr_e2e_session->mode == kE2EMode_Forbid))
    {
        USP_ERR_SetMessage("%s: E2E Session Context forbidden. (%s)",
                           __FUNCTION__, USP_ERR_UspErrToString(USP_ERR_SESS_CONTEXT_NOT_ALLOWED));
        err = USP_ERR_SESS_CONTEXT_NOT_ALLOWED;
        goto exit;
    }

    // Exit if one of the IDs is wrong.
    if ((!IsValidSessionId(ctx->session_id)) ||
        (ctx->sequence_id < E2E_FIRST_SEQ_ID) ||
        (ctx->expected_id < E2E_FIRST_SEQ_ID))
    {
        err = USP_ERR_RECORD_FIELD_INVALID;
        USP_ERR_SetMessage("%s: E2E USP Record containing wrong identifiers (sess: %"PRIu64", seq: %"PRIu64", exp: %"PRIu64"). Ignoring USP Record content.",
                           __FUNCTION__,
                           ctx->session_id,
                           ctx->sequence_id,
                           ctx->expected_id);
        goto exit;
    }

    // Validate the E2E Session Context IDs.
    // Exit if the received sequence_id is different than 1
    // when the received session_id is different than current session_id.
    if ((ctx->session_id != curr_e2e_session->current_session_id) &&
        (ctx->sequence_id != E2E_FIRST_SEQ_ID))
    {
        err = USP_ERR_RECORD_FIELD_INVALID;
        USP_ERR_SetMessage("%s: Received USP Record with unexpected IDs (sess: %"PRIu64" seq: %"PRIu64"). Ignoring USP Record content.",
                            __FUNCTION__,
                            ctx->session_id,
                            ctx->sequence_id);
        goto exit;
    }

    // Validate the SAR state of the segment.
    // When not matching, something unexpected happened and the segmented USP Message
    // cannot be recovered. The SAR vector is reset through the E2E Session Event.
    if (IsValidE2eSarState(&(curr_e2e_session->received_payloads), rec->session_context->payload_sar_state) == false)
    {
        err = USP_ERR_RECORD_FIELD_INVALID;
        USP_ERR_SetMessage("%s: Received USP Record with unexpected SAR state (%s). Ignoring USP Record content.",
                           __FUNCTION__,
                           E2E_CONTEXT_SarStateToString(rec->session_context->payload_sar_state));
        goto exit;
    }

exit:
    if (err != USP_ERR_OK)
    {
        // A USP Disconnect Record is sent through the E2E Event
        const e2e_event_t event = (curr_e2e_session->mode == kE2EMode_Require) ? kE2EEvent_Restart : kE2EEvent_Termination;
        E2E_CONTEXT_E2eSessionEvent(event, INVALID, ctrl_inst);
    }
    return err;
}

/*********************************************************************//**
**
** IsValidE2eSarState
**
** Convenience function to validate if the SAR state matches with the current
** state of the E2E Session. The 'None' and 'Begin' SAR states are only valid
** when the SAR vector is empty and the other SAR states are valid when the SAR
** vector already have content.
**
** \param   sar_vector - pointer to the relevant SAR vector.
** \param   sar_state - protobuf enumeration of the SAR state of USP Session Context Record
**
** \return  true if the SAR state matches the E2E session state.
**
**************************************************************************/
bool IsValidE2eSarState(sar_vector_t *sar_vector, int sar_state)
{
    const bool has_content = (sar_vector->num_entries > 0);
    switch (sar_state)
    {
        case USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__NONE:
        case USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__BEGIN:
        {
            // The vector must be empty
            if (has_content)
            {
                return false;
            }
            break;
        }
        case USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__INPROCESS:
        case USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__COMPLETE:
        default:
        {
            // The vector must have content
            if (!has_content)
            {
                return false;
            }
            break;
        }
    }

    return true;
}

/*********************************************************************//**
**
** IsValidSessionId
**
** Convenience function to validate if the given value is a valid as
** session_id for a USP Session Context Record.
**
** According to USP specification, the session context identifier
** must be greater than 1, i.e. a value smaller than 2 is invalid.
**
** \param   session_id - value to validate.
**
** \return  true if the session_id is valid.
**
**************************************************************************/
bool IsValidSessionId(uint64_t session_id)
{
    return (session_id >= E2E_FIRST_VALID_SESS_ID);
}

/*********************************************************************//**
**
** ComputePayloadCapacity
**
** Computes the maximal capacity a SessionContextRecord can have.
** That SessionContextRecord capacity is dependent of:
** - the maximal size the serialized USP record can be,
** - the size of the USP Record header fields, and
** - the size of the SessionContextRecord header fields.
**
** The computed capacity is obtained by filling the USP Record with an
** overweighted SessionContext record. The resulting packed size
** represents the maximal size the header fields could be.
**
** \param   src_rec - pointer to the USP Record structure, which also
**                    contains the SessionContextRecord struct.
** \param   max_record_size - the max size a packed USP record must be
**
** \return  The capacity of the SessionContextRecord for its payload.
**
**************************************************************************/
unsigned ComputePayloadCapacity(UspRecord__Record *src_rec, unsigned max_record_size)
{
    unsigned estimated_header_size = 0;
    ProtobufCBinaryData payload;

    // Copy of the struct fields and target local struct instances.
    UspRecord__SessionContextRecord ctx = *(src_rec->session_context);
    UspRecord__Record rec = *src_rec;
    ctx.payload = &payload;
    rec.session_context = &ctx;

    // Flip the first bit to get anything > 0, but at similar weight than original value
    ctx.session_id |= 1;
    ctx.expected_id |= 1;

    // Set sequence_id field to its maximum for the first computation.
    // - This will reserve a 10 bytes.
    ctx.sequence_id = UINT64_MAX;

    // Set to non-default enums
    ctx.payload_sar_state = USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__BEGIN;
    ctx.payloadrec_sar_state = USP_RECORD__SESSION_CONTEXT_RECORD__PAYLOAD_SARSTATE__BEGIN;

    // Set a payload with an overweighted size (i.e. the max allowed USP Record size).
    // That way, the obtained packed size includes the maximal size the header fields required.
    payload.len = max_record_size;
    payload.data = NULL;
    ctx.n_payload = 1;

    // Get the packed size of the metadata
    estimated_header_size = usp_record__record__get_packed_size(&rec) - max_record_size;

    // The Payload capacity is the max size of the packed USP Record minus its header fields size.
    return (max_record_size - estimated_header_size);
}

#endif  // E2ESESSION_EXPERIMENTAL_USP_V_1_2
