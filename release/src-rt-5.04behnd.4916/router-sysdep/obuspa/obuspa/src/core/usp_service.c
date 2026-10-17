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

/**
 * \file usp_service.c
 *
 * Contains functions for a USP Endpoint cating as a USP Service
 *
 */

#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "msg_handler.h"
#include "msg_utils.h"
#include "text_utils.h"
#include "iso8601.h"
#include "usp_broker.h"
#include "os_utils.h"
#include "usp_api.h"
#include "usp_service.h"

#include <semaphore.h>

#ifndef REMOVE_USP_SERVICE
//------------------------------------------------------------------------
// Comma separated list containing the endpoint_id of the USP Broker, followed by the top-level data model objects to register
// This string is set by the value of the '-R' option and is not changed by subsequent '-c register' or '-c deregister' CLI invocations
char *usp_service_objects = NULL;

//------------------------------------------------------------------------------
// Message ID of the register message upon which we are waiting for a response (and Broker's endpoint_id)
static char *register_msg_id = NULL;

// The endpoint of the broker as an agent when service running as a controller
static char *usp_broker_agent_endpoint_id = NULL;

// The endpoint of the broker as a controller when service acting as a data model provider
static char *usp_broker_controller_endpoint_id = NULL;

// Identifies the MTP to use when acting as a controller sending to the Endpoint's agent
static mtp_conn_t usp_broker_agent_mtpc;

// Vendor callback function to be invoked on subscription notifications
static usp_service_notify_cb_t usp_service_notify_cb = NULL;

//------------------------------------------------------------------------------
// String constant used in lots of log messages. Defined once, to minimize memory footprint.
const char *null_args_log_msg = "%s: Input arguments are NULL";

//------------------------------------------------------------------------------

// Outgoing messages and their responses are sent and received asynchronously using the datamodel thread
// In order to accomplish this we post the following structure into USP_PROCESS_DoWork() which calls us
// back from the datamodel thread.  We handle all interaction with MSG_HANDLER_QueueMessage from within
// that thread.  When an outgoing message is sent to the broker, we store the message ID in the list below
// and filter for a matching response in USP_SERVICE_AsController_IsExpectedResponse.  If a response is
// not received within the specified time then a timeout will fire and return an error.

typedef struct
{
    kv_vector_t *params; // key-value vector containing the full path to all params returned and their associated values
} usp_service_get_request_t;

typedef struct
{
    int *instance;            // Pointer to variable in which to return the instance number of the object that was created
    kv_vector_t *unique_keys; // Pointer to KV vector in which to return the names of the unique keys (and associated values) of the object that was created
} usp_service_add_request_t;

typedef struct
{
    char *obj_path;
} usp_service_delete_request_t;

typedef struct
{
    kv_vector_t *obj_paths; // Pointer to key/value vector in which to return the full path of all supported data model parameters/events/operations
} usp_service_get_supported_dm_request_t;

typedef struct
{
    str_vector_t *instances; // Pointer to string vector in which to return the full path of all object instances discovered (all paths end in '.')
} usp_service_get_instances_request_t;

typedef struct
{
    char *operate_path;
    kv_vector_t *args;          // Pointer to key-value vector in which to return the output arguments and associated values (only used if invoking a synchronous USP command)
} usp_service_operate_request_t;

//------------------------------------------------------------------------------
// Outgoing USP control message and incoming USP response parameters
typedef struct
{
    double_link_t    link;               // linked list node structure
    Usp__Msg        *req;                // the USP message to send
    sem_t            semaphore;          // A semphore to signal to the sender when the response has arrived or a timeout occurred
    int              timeout;            // The time to wait before failing and returning an error
    int              err_code;           // USP_ERR_xxx indicting success of failure of the request
    char            *err_msg;            // pointer to buffer in which to return an error message (only used if error code is failed)
    int              err_msg_len;        // length of buffer in which to return an error message (only used if error code is failed)
    int              timeout_id;         // Each request has a corresponding timeout

    // NOTE: The discriminator for use of this union is req->header->msg_type
    union
    {
        usp_service_get_request_t get;
        usp_service_get_supported_dm_request_t gsdm;
        usp_service_get_instances_request_t get_inst;
        usp_service_add_request_t add;
        usp_service_delete_request_t del;
        usp_service_operate_request_t operate;
    };
} usp_service_req_t;

// The list of requests which we have not received responses to yet (when a USP Service acts as a Controller)
static double_linked_list_t usp_service_req_list = { NULL, NULL };

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
Usp__Msg *CreateUspServiceRegisterReq(char *msg_id, char **paths, int num_paths);
Usp__Msg *CreateUspServiceDeregisterReq(char *msg_id, char **paths, int num_paths);
void CalcUspServiceMessageId(char *msg_id, int len);
int ProcessUspService_SetResponse(Usp__Msg *resp);
int ProcessUspService_AddResponse(Usp__Msg *resp, kv_vector_t *unique_keys, int *instance);
int ProcessUspService_GetSupportedDMResponse(Usp__Msg *usp, kv_vector_t *sv);
int ProcessUspService_GetInstancesResponse(Usp__Msg *resp, str_vector_t *sv);
int ProcessUspService_DeleteResponse(Usp__Msg *resp, char *path);
int ProcessUspService_OperateResponse(Usp__Msg *resp, char *path, kv_vector_t *output_args);
void ProcessUspService_Notification(Usp__Notify *notify);
void HandleUspServiceControllerMessageResponseTimeout(int id);
void QueueUspServiceRequest(void *arg1, void *arg2);
int PerformUspServiceRequest(usp_service_req_t *req, const char *caller);
char * ParamTypeToUspServiceString(Usp__GetSupportedDMResp__ParamValueType value_type);

/*********************************************************************//**
**
** USP_SERVICE_SetBrokerAgent
**
** Configures the USP broker connection to use when acting as a controller
** Must be called from the datamodel thread
**
** \param  endpoint_id - endpoint of the broker agent path
** \param  mtp_conn - connection parameters to use for the broker
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_SetBrokerAgent(char *endpoint_id, mtp_conn_t *mtp_conn)
{
    USP_LOG_Debug("%s: Setting broker agent endpoint to : %s", __FUNCTION__, endpoint_id);

    USP_ASSERT(mtp_conn->protocol == kMtpProtocol_UDS);
    USP_ASSERT(mtp_conn->uds.conn_id != INVALID);

    // Free the string if function called twice (typically this function is expected to be called only once).
    USP_SAFE_FREE(usp_broker_agent_endpoint_id);

    usp_broker_agent_endpoint_id = USP_STRDUP(endpoint_id);
    usp_broker_agent_mtpc = *mtp_conn;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_SERVICE_QueueRegisterRequest
**
** Queues a USP Register Request to register parts of the data model provided by this endpoint with the USP Broker
**
** \param   endpoint_id - Endpoint ID of the Broker to send the message to
** \param   objects - comma separated list of objects to register
**
** \return  None
**
**************************************************************************/
void USP_SERVICE_QueueRegisterRequest(char *endpoint_id, char *objects)
{
    int i;
    char *path;
    str_vector_t sv;
    Usp__Msg *msg;
    dm_node_t *node;
    dm_instances_t inst;
    mtp_conn_t mtpc = {0};
    char msg_id[MAX_MSG_ID_LEN];

    USP_ASSERT(objects != NULL);
    mtpc.is_reply_to_specified = false;   // Force DEVICE_CONTROLLER_QueueBinaryMessage() to calculate an MRT destination for the request

    // Iterate over all paths, validating them
    TEXT_UTILS_SplitString(objects, &sv, ",");
    for (i=0; i < sv.num_entries; i++)
    {
        // Exit if path does not exist in the data model (of this endpoint)
        path = sv.vector[i];
        node = DM_PRIV_GetNodeFromPath(path, &inst, NULL, 0);
        if (node == NULL)
        {
            USP_LOG_Error("%s: Cannot register path '%s' with USP Broker as it is not present in the data model", __FUNCTION__, path);
            goto exit;
        }

        // Exit if path contains instance numbers
        if (inst.order > 0)
        {
            USP_LOG_Error("%s: Cannot register path '%s' with USP Broker as it contains instance numbers", __FUNCTION__, path);
            goto exit;
        }
    }

    // Create a register request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    msg = CreateUspServiceRegisterReq(msg_id, sv.vector, sv.num_entries);
    MSG_HANDLER_QueueMessage(endpoint_id, msg, &mtpc);

    // Save off the message_id and endpoint_id, in order that we can check the register response against them
    USP_SAFE_FREE(register_msg_id);
    USP_SAFE_FREE(usp_broker_controller_endpoint_id);
    register_msg_id = USP_STRDUP(msg_id);
    usp_broker_controller_endpoint_id = USP_STRDUP(endpoint_id);

    usp__msg__free_unpacked(msg, pbuf_allocator);

exit:
    STR_VECTOR_Destroy(&sv);
}

/*********************************************************************//**
**
** USP_SERVICE_HandleRegisterResp
**
** Handles a USP Register response message
** NOTE: This function just validates and logs the response.
**       No action is taken upon registration failures. It is expected that the USP Broker won't query the USP Service if it failed to register
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP broker which sent this message
** \param   mtpc - details of where this USP message came from
**
** \return  None
**
**************************************************************************/
void USP_SERVICE_HandleRegisterResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    int i;
    Usp__RegisterResp *reg;
    Usp__RegisterResp__RegisteredPathResult *rpr;
    Usp__RegisterResp__RegisteredPathResult__OperationStatus__OperationFailure *oper_failure;

    // NOTE: All errors in parsing response messages should be ignored according to R-MTP.5 (they should not send a USP ERROR response)

    // Exit, if not currently waiting for a register response
    if ((usp_broker_controller_endpoint_id == NULL) || (register_msg_id == NULL))
    {
        USP_LOG_Error("%s: Ignoring response that we were not expecting (msg_id=%s from endpoint=%s)", __FUNCTION__, usp->header->msg_id, endpoint_id);
        return;
    }

    // Exit if the response is not from the Broker we sent it to
    if (strcmp(endpoint_id, usp_broker_controller_endpoint_id) != 0)
    {
        USP_LOG_Error("%s: Incoming Register Response is from an unexpected endpoint (%s, expected %s)", __FUNCTION__, endpoint_id, usp_broker_controller_endpoint_id);
        return;
    }

    // Exit if the msg_id of this register response does not match the one we sent in our request
    if (strcmp(usp->header->msg_id, register_msg_id) != 0)
    {
        USP_LOG_Error("%s: Ignoring register response from endpoint '%s' because msg_id='%s' (expected '%s')", __FUNCTION__, endpoint_id, usp->header->msg_id, register_msg_id);
        return;
    }

    // Since we've determined that the response message was expected, reset the saved message_id and endpoint_id, as we're not expecting any other responses
    USP_FREE(register_msg_id);
    USP_FREE(usp_broker_controller_endpoint_id);
    register_msg_id = NULL;
    usp_broker_controller_endpoint_id = NULL;

    // Exit, if we received an ERROR response to the Register request
    if (usp->header->msg_type == USP__HEADER__MSG_TYPE__ERROR)
    {
        if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_ERROR) ||
            (usp->body->error == NULL) || (usp->body->error->err_msg == NULL))
        {
            USP_LOG_Error("%s: Incoming message (msg_id=%s) is invalid or inconsistent", __FUNCTION__, usp->header->msg_id);
            return;
        }

        USP_LOG_Error("%s: Received an ERROR Response to Register message (err_code=%d, err_msg='%s')", __FUNCTION__, usp->body->error->err_code, usp->body->error->err_msg);
        return;
    }

    // Exit if we did not receive a response message, or the message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_RESPONSE) ||
        (usp->body->response == NULL) || (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_REGISTER_RESP) ||
        (usp->body->response->register_resp == NULL) )
    {
        USP_LOG_Error("%s: Incoming message (msg_id=%s) is invalid or inconsistent", __FUNCTION__, usp->header->msg_id);
        return;
    }

    // Log all registration errors
    reg = usp->body->response->register_resp;
    for (i=0; i < reg->n_registered_path_results; i++)
    {
        rpr = reg->registered_path_results[i];
        switch (rpr->oper_status->oper_status_case)
        {
            case USP__REGISTER_RESP__REGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
                oper_failure = rpr->oper_status->oper_failure;
                USP_LOG_Error("%s: Failed to register path %s (err_code=%d, err_msg='%s')", __FUNCTION__, rpr->requested_path, oper_failure->err_code, oper_failure->err_msg);
                break;

            case USP__REGISTER_RESP__REGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
                USP_LOG_Info("%s: Successfully registered path '%s'", __FUNCTION__, rpr->requested_path);
                break;

            default:
            case USP__REGISTER_RESP__REGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS__NOT_SET:
                USP_LOG_Error("%s: Error when registering path '%s'", __FUNCTION__, rpr->requested_path);
                break;
        }
    }
}

/*********************************************************************//**
**
** USP_SERVICE_QueueDeregisterRequest
**
** Queues a USP Deregister Request to deregister parts of the data model provided by this endpoint with the USP Broker
**
** \param   endpoint_id - Endpoint ID of the Broker to send the message to
** \param   mtpc - details of where the deregister message should be sent (i.e. Broker's controller socket)
** \param   objects - comma separated list of objects to deregister
**
** \return  None
**
**************************************************************************/
void USP_SERVICE_QueueDeregisterRequest(char *endpoint_id, char *objects)
{
    int i;
    char *path;
    str_vector_t sv;
    Usp__Msg *msg = NULL;
    dm_node_t *node;
    dm_instances_t inst;
    mtp_conn_t mtpc = {0};
    char msg_id[MAX_MSG_ID_LEN];

    // Exit if no top-level objects are to be deregistered
    if (objects == NULL)
    {
        return;
    }

    mtpc.is_reply_to_specified = false;   // Force DEVICE_CONTROLLER_QueueBinaryMessage() to calculate an MRT destination for the request

    // Special case of deregistering all paths
    STR_VECTOR_Init(&sv);
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    if (*objects == '\0')
    {
        msg = CreateUspServiceDeregisterReq(msg_id, &objects, 1);
        MSG_HANDLER_QueueMessage(endpoint_id, msg, &mtpc);
        goto exit;
    }

    // Iterate over all paths, validating them
    TEXT_UTILS_SplitString(objects, &sv, ",");
    for (i=0; i < sv.num_entries; i++)
    {
        // Exit if path does not exist in the data model (of this endpoint)
        path = sv.vector[i];
        node = DM_PRIV_GetNodeFromPath(path, &inst, NULL, 0);
        if (node == NULL)
        {
            USP_LOG_Error("%s: Cannot deregister path '%s' as it is not present in the data model", __FUNCTION__, path);
            goto exit;
        }

        // Exit if path contains instance numbers
        if (inst.order > 0)
        {
            USP_LOG_Error("%s: Cannot deregister path '%s' as it contains instance numbers", __FUNCTION__, path);
            goto exit;
        }
    }

    // Create a deregister request
    msg = CreateUspServiceDeregisterReq(msg_id, sv.vector, sv.num_entries);
    MSG_HANDLER_QueueMessage(endpoint_id, msg, &mtpc);

exit:
    if (msg != NULL)
    {
        usp__msg__free_unpacked(msg, pbuf_allocator);
    }

    STR_VECTOR_Destroy(&sv);
}

/*********************************************************************//**
**
** USP_SERVICE_HandleDeRegisterResp
**
** Handles a USP Deregister response message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP broker which sent this message
** \param   mtpc - details of where this USP message came from
**
** \return  None
**
**************************************************************************/
void USP_SERVICE_HandleDeRegisterResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    // NOTE: This function is only called when using the CLI 'deregister' command, which is only intended for testing purposes
    // and hence, this function currently does nothing
    USP_LOG_Info("%s: Intentionally ignoring Deregister Response", __FUNCTION__);
}

/*********************************************************************//**
**
** USP_Service_Stop
**
** Frees up all memory associated with this module
** Must be called from datamodel thread
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_SERVICE_Stop(void)
{
    usp_service_req_t *req;
    usp_service_req_t *next_req;

    // Iterate through all control messages and flush and pending message responses
    req = (usp_service_req_t *) usp_service_req_list.head;
    while (req != NULL)
    {
        next_req = (usp_service_req_t *) req->link.next;

        USP_LOG_Warning("%s: Cancelling pending response : %s", __FUNCTION__, req->req->header->msg_id);

        // Remove the request from the list
        DLLIST_Unlink(&usp_service_req_list, req);
        req->err_code = USP_ERR_INTERNAL_ERROR;
        USP_SNPRINTF(req->err_msg, req->err_msg_len, "Cancelled pending response from agent");

        // Post the semaphore to unblock any waiting vendor thread
        sem_post(&req->semaphore);

        req = next_req;
    }

    // Free the dynamically allocated agent endpoint string if set
    USP_SAFE_FREE(usp_broker_agent_endpoint_id);
}

/*********************************************************************//**
**
** USP_SERVICE_AsController_IsExpectedResponse
**
** Called from Datamodel thread hook in message handler HandleUspMessage()
** Determines whether the specified USP message is a response to a request sent by this USP Service acting as a Controller
** If it is, then the USP response message is processed here
**
** \param  usp - a received USP message buffer
**
** \return bool - true if the message has been handled here and no further processing is necessary
**
**************************************************************************/
bool USP_SERVICE_AsController_IsExpectedResponse(Usp__Msg *usp)
{
    int err = USP_ERR_OK;
    char *usp_err_msg;
    usp_service_req_t *req;
    usp_service_req_t *next_req;

    USP_ASSERT(usp != NULL);

    // Handle any messages that contain a notification request and belong to an active USP service subscription on the Broker
    if (usp->body->msg_body_case == USP__BODY__MSG_BODY_REQUEST)
    {
        USP_ASSERT(usp->body->request != NULL);
        if (usp->body->request->req_type_case == USP__REQUEST__REQ_TYPE_NOTIFY)
        {
           ProcessUspService_Notification(usp->body->request->notify);
        }
        // always return false for notifications as they may also need to be handled elsewhere
        return false;
    }

    // Exit if we're not waiting for any responses
    req = (usp_service_req_t *)usp_service_req_list.head;
    if (req == NULL)
    {
        return false;
    }

    // Exit if message was a response and type of response was not one of the ones we handle here
    if (usp->body->msg_body_case == USP__BODY__MSG_BODY_RESPONSE)
    {
        USP_ASSERT(usp->body->response != NULL);
        if ((usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_GET_RESP) &&
            (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_SET_RESP) &&
            (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_ADD_RESP) &&
            (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_DELETE_RESP) &&
            (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_GET_SUPPORTED_DM_RESP) &&
            (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_GET_INSTANCES_RESP) &&
            (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_OPERATE_RESP))
        {
            return false;
        }
    }

    // note USP__BODY__MSG_BODY_ERROR will drop through here and be handled by the appropriate response handler below

    // iterate through all control messages that are waiting for a response
    while (req != NULL)
    {
        next_req = (usp_service_req_t *) req->link.next;

        // check the incoming message ID against the list of pending messages waiting for a response
        if ((usp->header->msg_id != NULL) && (strcmp(usp->header->msg_id, req->req->header->msg_id) == 0))
        {
            // Any errors detected in the USP response will be captured by USP_ERR.  Note that though errors
            // this does not imply total failure, and it may be that the response contains some values and not others
            USP_ERR_ClearMessage();

            // Process response based on message type of original request, as we could have received an ERROR response
            // we must process USP response here as the calling thread has ownership and will free it
            switch (req->req->header->msg_type)
            {
                case USP__HEADER__MSG_TYPE__GET:
                    err = MSG_UTILS_ProcessUspService_GetResponse(usp, req->get.params);
                    break;

                case USP__HEADER__MSG_TYPE__SET:
                    err = ProcessUspService_SetResponse(usp);
                    break;

                case USP__HEADER__MSG_TYPE__ADD:
                    err = ProcessUspService_AddResponse(usp, req->add.unique_keys, req->add.instance);
                    break;

                case USP__HEADER__MSG_TYPE__DELETE:
                    err = ProcessUspService_DeleteResponse(usp, req->del.obj_path );
                    break;

                case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
                    err = ProcessUspService_GetSupportedDMResponse(usp, req->gsdm.obj_paths);
                    break;

                case USP__HEADER__MSG_TYPE__GET_INSTANCES:
                    err = ProcessUspService_GetInstancesResponse(usp, req->get_inst.instances);
                    break;

                case USP__HEADER__MSG_TYPE__OPERATE:
                    err = ProcessUspService_OperateResponse(usp, req->operate.operate_path, req->operate.args);
                    break;

                default:
                    break;
            }

            // update the request error code with the value provided by the USP response
            req->err_code = err;
            // if we encounter any errors during processing, add them to the request/response structure
            if (req->err_code != USP_ERR_OK)
            {
                if (req->err_msg != NULL)
                {
                    usp_err_msg = USP_ERR_GetMessage();
                    USP_SNPRINTF(req->err_msg, req->err_msg_len , "%s", usp_err_msg);
                }
            }

            // cancel any active timeout associated with this control message
            SYNC_TIMER_Remove(HandleUspServiceControllerMessageResponseTimeout, req->timeout_id);

            // remove the message from the pending message response list
            DLLIST_Unlink(&usp_service_req_list, req);

            // post the semaphore to unblock the vendor thread
            sem_post(&req->semaphore);

            return true;
        }

        req = next_req;
    }

    // If the code gets here, then the USP message was not a response to a request from this USP Service acting as a Controller
    return false;
}

/*********************************************************************//**
**
** USP_SERVICE_Get
**
** Gets the specified data model parameters
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   params - A list of parameter paths as keys to GET and outputs the resulting keys/values
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_Get(kv_vector_t *params, int timeout, char *err_msg, int err_msg_len)
{
    char msg_id[MAX_MSG_ID_LEN];
    usp_service_req_t request;

    // Exit if input args are not specified
    if ((params == NULL) || (err_msg == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.req = MSG_UTILS_Create_GetReq(msg_id, params);
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;
    request.get.params = params;

    // Default output args
    // NOTE: Get returns the results in the same KV params structure passed in
    KV_VECTOR_Destroy(params);

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_Set
**
** Sets the specified data model parameters
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   params - A list of parameter paths and corresponding values to SET
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_Set(kv_vector_t *params, int timeout, char *err_msg, int err_msg_len)
{
    char msg_id[MAX_MSG_ID_LEN];
    usp_service_req_t request;

    // Exit if input args are not specified
    if ((params == NULL) || (err_msg == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.req = MSG_UTILS_Create_SetReq(msg_id, params);
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_Add
**
** Adds the specified data model object
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   path - The path to a multi instance object in the data model of which create a new instance
** \param   params - A list of initialisation parameters for the new object.  Used to pass back unique keys of new object.
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   instance - pointer to variable in which to return the new instance number
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_Add(char *path, kv_vector_t *params, int timeout, int *instance, char *err_msg, int err_msg_len)
{
    char msg_id[MAX_MSG_ID_LEN];
    int i;
    usp_service_req_t request;
    group_add_param_t *add_params = NULL;
    group_add_param_t *gap;

    // Exit if input args are not specified
    if ((path == NULL) || (params == NULL) || (err_msg == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form temporary array containing the parameters to set in the object
    // NOTE: Ownership of all strings in this array stays with the 'params' input argument
    // provide an empty params array if no parameters are passed in
    add_params = USP_MALLOC(params->num_entries * sizeof(group_add_param_t));
    for  (i=0; i<params->num_entries; i++)
    {
        gap = &add_params[i];
        gap->param_name = params->vector[i].key;
        gap->value = params->vector[i].value;
        gap->is_required = true;
        gap->err_code = USP_ERR_OK;
        gap->err_msg = NULL;
    }

    // Form request and free temporary array
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.req = MSG_UTILS_Create_AddReq(msg_id, path, add_params, params->num_entries);
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;
    request.add.unique_keys = params;
    request.add.instance = instance;
    USP_SAFE_FREE(add_params);

    // Default output args
    // NOTE: Add returns unique keys in the same KV params structure that the input parameters were passed in
    KV_VECTOR_Destroy(params);

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_Delete
**
** Deletes the specified data model object
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   obj_path - The full path to an object in the data model to delete
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_Delete(char *obj_path, int timeout, char *err_msg, int err_msg_len)
{
    str_vector_t obj_to_del;
    char msg_id[MAX_MSG_ID_LEN];
    usp_service_req_t request;

    // Exit if input args are not specified
    if ((obj_path == NULL) || (err_msg == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form temporary string vector containing the object to delete
    // NOTE: Ownership of the string in this vector stays with the 'obj_path' input argument
    obj_to_del.vector = &obj_path;
    obj_to_del.num_entries = 1;

    // Form request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.req = MSG_UTILS_Create_DeleteReq(msg_id, &obj_to_del, false);
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;
    request.del.obj_path = obj_path;

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_GetSupportedDM
**
** Gets the supported data model under the specified path
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   path - The path under which to query the supported datamodel (NOTE: All data model elements below this path will be returned)
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   supported_paths - key/value vector in which to return the full path of the data model elements discovered and their type
**                            NOTE: object paths end in '.', USP commands end in '()', events end in '!', and parameters end in an alphabetic character
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_GetSupportedDM(char *path, int timeout, kv_vector_t *supported_paths, char *err_msg, int err_msg_len)
{
    str_vector_t gsdm_path;
    char msg_id[MAX_MSG_ID_LEN];
    usp_service_req_t request;

    // Exit if input args are not specified
    if ((path == NULL) || (err_msg == NULL) || (supported_paths == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form temporary string vector containing the path to recursively get the supported data model of
    // NOTE: Ownership of the string in this vector stays with the 'path' input argument
    gsdm_path.vector = &path;
    gsdm_path.num_entries = 1;

    // Form request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.req = MSG_UTILS_Create_GetSupportedDMReq(msg_id, &gsdm_path);
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;
    request.gsdm.obj_paths = supported_paths;

    // Default output args
    KV_VECTOR_Init(supported_paths);

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_GetInstances
**
** Gets the instances of objects underneath the specified data model object
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   path - A path to a multi instance datamodel object to get all instance numbers of all objects underneath it in the data model
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   instances - pointer to string vector in which to return the full paths of each instantiated object (all paths end in '.')
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_GetInstances(char *path, int timeout, str_vector_t *instances, char *err_msg, int err_msg_len)
{
    str_vector_t get_instances_path;
    char msg_id[MAX_MSG_ID_LEN];
    usp_service_req_t request;

    // Exit if input args are not specified
    if ((path == NULL) || (err_msg == NULL) || (instances == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form temporary string vector containing the DM object to recursively get the paths of all instances underneath it
    // NOTE: Ownership of the string in this vector stays with the 'path' input argument
    get_instances_path.vector = &path;
    get_instances_path.num_entries = 1;

    // Form request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;
    request.get_inst.instances = instances;
    request.req = MSG_UTILS_Create_GetInstancesReq(msg_id, &get_instances_path);

    // Default output args
    STR_VECTOR_Init(instances);

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_Operate
**
** Invokes a USP Command
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
** Asynchronous operations will return values via a subsequent operation complete notification
** NOTE: This function is intended to be used only for data model elements not owned by this USP Service
**
** \param   path - The path to a an operation in the datamodel
** \param   args - passed in required input arguments.  Replaced with any returned output arguments on return from function
** \param   timeout - maximum length of time (seconds) to wait for a response before returning
** \param   cmd_key - pointer to a string to return the unique command key for this operation, or NULL.  Caller takes ownership and must free the memory.
** \param   err_msg - pointer to buffer in which to return an error message (only used if error code is failed)
** \param   err_msg_len - length of buffer in which to return an error message (only used if error code is failed)
**
** \return  USP_ERR_OK if okay, or an error
**
**************************************************************************/
int USP_SERVICE_Operate(char *path, kv_vector_t *args, int timeout, char **cmd_key, char *err_msg, int err_msg_len)
{
    char msg_id[MAX_MSG_ID_LEN];
    usp_service_req_t request;

    // Exit if input args are not specified
    if ((path == NULL) || (err_msg == NULL) || (args == NULL))
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form request
    CalcUspServiceMessageId(msg_id, sizeof(msg_id));
    request.req = MSG_UTILS_Create_OperateReq(msg_id, path, msg_id, args);   // Set command_key to be the same as msg_id
    request.err_msg = err_msg;
    request.err_msg_len = err_msg_len;
    request.timeout = timeout;
    request.operate.args = args;
    request.operate.operate_path = path;

    if (cmd_key != NULL)
    {
        *cmd_key = USP_STRDUP(msg_id);
    }

    // Default output args
    // NOTE: Operate returns any output args in the same KV params structure that the input_args were passed in
    KV_VECTOR_Destroy(args);

    return PerformUspServiceRequest(&request, __FUNCTION__);
}

/*********************************************************************//**
**
** USP_SERVICE_RegisterNotificationCallback
**
** Function called by vendor layer to register a function which is called back for each USP notification received
** USP Notifications must first have been subscribed-to on the USP Broker
** This function may be called by the vendor layer when a USP Service is acting as a USP Controller
**
** \param   cb - the callback function
**
** \return  USP_ERR_OK
**
**************************************************************************/
int USP_SERVICE_RegisterNotificationCallback(usp_service_notify_cb_t cb)
{
    // Exit if input args are not specified
    if (cb == NULL)
    {
        USP_LOG_Error(null_args_log_msg, __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if not running as a USP Service
    if (RUNNING_AS_USP_SERVICE() == false)
    {
        USP_ERR_SetMessage("%s: Not running as a USP Service", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    usp_service_notify_cb = cb;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** PerformUspServiceRequest
**
** Sends the specified request to the USP Broker, waits for a response, then parses the response
** This function is called when a USP Service is acting as a Controller
**
** \param   req - the request to perform
** \param   caller - name of function calling this. Used for error debug
**
** \return  USP_ERR_OK if request was sent, response received and response parsed successfully
**
**************************************************************************/
int PerformUspServiceRequest(usp_service_req_t *req, const char *caller)
{
    // Exit if called from the data model thread
    // The USP-Service-as-controller API functions cannot be called from the data model thread, as that will cause deadlock
    if (OS_UTILS_IsDataModelThread(caller, DONT_PRINT_WARNING))
    {
        USP_SNPRINTF(req->err_msg, req->err_msg_len, "%s() cannot be called from data model thread (would cause deadlock)", caller);
        usp__msg__free_unpacked(req->req, pbuf_allocator);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if not running as a USP Service
    if (RUNNING_AS_USP_SERVICE() == false)
    {
        USP_SNPRINTF(req->err_msg, req->err_msg_len, "%s: Not running as a USP Service", caller);
        usp__msg__free_unpacked(req->req, pbuf_allocator);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if not yet connected to the USP Broker (and hence haven't determined its endpoint_id yet)
    if (usp_broker_agent_endpoint_id == NULL)
    {
        USP_SNPRINTF(req->err_msg, req->err_msg_len, "%s: Not connected to USP Broker yet", caller);
        usp__msg__free_unpacked(req->req, pbuf_allocator);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Default output args
    req->err_msg[0] = '\0';
    req->err_code = USP_ERR_INTERNAL_ERROR;

    // Send request and wait for response
    sem_init(&req->semaphore,0,0);
    USP_PROCESS_DoWork( QueueUspServiceRequest, req, NULL);
    sem_wait(&req->semaphore);

    // Free resources created by this function and not being returned
    sem_destroy(&req->semaphore);
    usp__msg__free_unpacked(req->req, pbuf_allocator);

    return req->err_code;
}

/*********************************************************************//**
**
** HandleUspServiceControllerMessageResponseTimeout
**
** This function is called if a response message is not received within the
** specified timeout.  It is called in the context of datamodel thread
**
** \param   id - A unique ID identifying the pending message response
**
** \return  void
**
**************************************************************************/
void HandleUspServiceControllerMessageResponseTimeout(int id)
{
    usp_service_req_t *req;
    usp_service_req_t *next_req;

    // Iterate through all control messages to find the one matching this timeout id
    req = (usp_service_req_t *) usp_service_req_list.head;
    while (req != NULL)
    {
        next_req = (usp_service_req_t *) req->link.next;
        if (req->timeout_id == id)
        {
            USP_LOG_Warning("%s: Timed out waiting for msg id : %s", __FUNCTION__, req->req->header->msg_id);
            USP_ERR_ReplaceEmptyMessage("%s: Timed out waiting for response from agent", __FUNCTION__);

            // Remove the request from the list
            DLLIST_Unlink(&usp_service_req_list, req);
            req->err_code = USP_ERR_INTERNAL_ERROR;
            USP_SNPRINTF(req->err_msg, req->err_msg_len, "Timed out waiting for response from agent");

            // Post the semaphore to unblock the vendor thread
            sem_post(&req->semaphore);
            return;
        }
        req = next_req;
   }
}

/*********************************************************************//**
**
** QueueUspServiceRequest
**
** Called from Datamodel thread in response to calling USP_PROCESS_DoWork()
** This function queues a USP Request message to send to the Broker
** when this USP Service is acting as a Controller
**
** \param  arg1 - The request structure containing input and output parameters
** \param  arg2 - Not used
**
** \return  void
**
**************************************************************************/
void QueueUspServiceRequest(void *arg1, void *arg2)
{
    // use a rolling label to identify a response timeout uniquely
    static int label = 1;
    time_t timer_timeout;
    usp_service_req_t *request;

    USP_ASSERT(arg1 != NULL);

    request = (usp_service_req_t *) arg1;

    MSG_HANDLER_QueueMessage(usp_broker_agent_endpoint_id, request->req, &usp_broker_agent_mtpc);

    // The timeout_id is a unique integer used to correlate timeouts with messages.
    request->timeout_id = label++;

    DLLIST_LinkToTail(&usp_service_req_list, request);

    timer_timeout = time(NULL) + request->timeout;
    SYNC_TIMER_Add(HandleUspServiceControllerMessageResponseTimeout, request->timeout_id, timer_timeout);
}

/*********************************************************************//**
**
** CreateUspServiceRegisterReq
**
** Create a USP Register request message
**
** \param   msg_id - MessageId to put in the message
** \param   paths- array of paths to register
** \param   num_paths - num
**
** \return  Pointer to a Register Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateUspServiceRegisterReq(char *msg_id, char **paths, int num_paths)
{
    int i;
    Usp__Msg *msg;
    Usp__Register *reg;
    Usp__Register__RegistrationPath *rp;

    // Create Register Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__REGISTER, USP__REQUEST__REQ_TYPE_REGISTER);
    reg = USP_MALLOC(sizeof(Usp__Register));
    usp__register__init(reg);
    msg->body->request->register_ = reg;

    // Copy the paths into the Register
    reg->n_reg_paths = num_paths;
    reg->reg_paths = USP_MALLOC(num_paths*sizeof(void *));
    for (i=0; i<num_paths; i++)
    {
        rp = USP_MALLOC(sizeof(Usp__Register__RegistrationPath));
        usp__register__registration_path__init(rp);
        rp->path = USP_STRDUP(paths[i]);
        reg->reg_paths[i] = rp;
    }

    // Fill in the flags in the Register
    reg->allow_partial = false;

    return msg;
}

/*********************************************************************//**
**
** CreateUspServiceDeregisterReq
**
** Create a USP Deregister request message
**
** \param   msg_id - MessageId to put in the message
** \param   paths- array of paths to register
** \param   num_paths - num
**
** \return  Pointer to a Register Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateUspServiceDeregisterReq(char *msg_id, char **paths, int num_paths)
{
    int i;
    Usp__Msg *msg;
    Usp__Deregister *dreg;

    // Create Deegister Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__DEREGISTER, USP__REQUEST__REQ_TYPE_DEREGISTER);
    dreg = USP_MALLOC(sizeof(Usp__Deregister));
    usp__deregister__init(dreg);
    msg->body->request->deregister = dreg;

    // Copy the paths into the Deregister
    dreg->n_paths = num_paths;
    dreg->paths = USP_MALLOC(num_paths*sizeof(void *));
    for (i=0; i<num_paths; i++)
    {
        dreg->paths[i] = USP_STRDUP(paths[i]);
    }

    return msg;
}

/*********************************************************************//**
**
** CalcUspServiceMessageId
**
** Creates a unique message id for messages sent from this USP Service to a USP Broker
**
** \param   msg_id - pointer to buffer in which to write the message id
** \param   len - length of buffer
**
** \return  None
**
**************************************************************************/
void CalcUspServiceMessageId(char *msg_id, int len)
{
    static unsigned count = 0;
    char buf[MAX_ISO8601_LEN];
    char *endpoint_id;

    count++;               // Pre-increment before forming message, because we want to count from 1

    endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();

    // Form a message id string which is unique.
    {
        // In production, the string must be unique because we don't want the USP Service receiving stale responses
        // and treating them as fresh (in the case of the USP Service crashing and restarting)
        USP_SNPRINTF(msg_id, len, "%s-%d-%s", endpoint_id, count, iso8601_cur_time(buf, sizeof(buf)) );
    }
}

/*********************************************************************//**
**
** ParamTypeToUspServiceString
**
** Convert from the protobuf parameter type enumeration to a string
**
** \param   value_type - protobuf parameter type enumeration to convert
**
** \return  string representing parameter type
**
**************************************************************************/
char * ParamTypeToUspServiceString(Usp__GetSupportedDMResp__ParamValueType value_type)
{
    char *ret = "\0";

    switch(value_type)
    {
        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_BASE_64:
            ret = "base64";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_BOOLEAN:
            ret = "boolean";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_DATE_TIME:
            ret = "date_time";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_DECIMAL:
            ret = "decimal";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_HEX_BINARY:
            ret = "hex_binary";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_INT:
            ret = "int";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_LONG:
            ret = "long";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNSIGNED_INT:
            ret = "int";
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNSIGNED_LONG:
            ret = "ulong";
            break;

        default:
        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_STRING:
            ret = "string";
            break;
    }

    return ret;
}

/*********************************************************************//**
**
** ProcessUspService_AddResponse
**
** Processes a Add Response that we have received from a USP Service.  This
** function populates the returned kvv structure with all the unique keys
** found in the USP response message without performing any filtering.
**
** \param   resp - USP response message in protobuf-c structure
** \param   unique_keys - key-value vector in which to return the unique keys of the institated object
** \param   instance - pointer to integer in which to return the instance number of the institated object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessUspService_AddResponse(Usp__Msg *resp, kv_vector_t *unique_keys, int *instance)
{
    int i;
    int err = USP_ERR_OK;
    Usp__AddResp *add;
    Usp__AddResp__CreatedObjectResult *created_obj_result;
    Usp__AddResp__CreatedObjectResult__OperationStatus *oper_status;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *oper_failure;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry *uk;
    str_vector_t sv_params;

    STR_VECTOR_Init(&sv_params);

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_ADD_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if get response is missing
    add = resp->body->response->add_resp;
    if (add == NULL)
    {
        USP_ERR_SetMessage("%s: Missing add response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if there isn't exactly 1 created_obj_result (since the USP_SERVICE_Add API supports returning the instance number of only one object)
    if (add->n_created_obj_results != 1)
    {
        USP_ERR_SetMessage("%s: Too many objects created (%d). USP_SERVICE_Add() can only return a single instance number.", __FUNCTION__, (int)add->n_created_obj_results);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Determine whether the object was created successfully or failed
    created_obj_result = add->created_obj_results[0];
    oper_status = created_obj_result->oper_status;
    switch(oper_status->oper_status_case)
    {
        case USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
            oper_failure = oper_status->oper_failure;
            USP_ERR_SetMessage("%s", oper_failure->err_msg);
            err = oper_failure->err_code;
            break;

        case USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
            oper_success = oper_status->oper_success;

            TEXT_UTILS_SplitString(oper_success->instantiated_path, &sv_params, ".");

            // typically we would expect at least 3 entries - i.e. "Device.<path>.<instance>."
            if (sv_params.num_entries < 1)
            {
                USP_ERR_SetMessage("%s: Unable to determine instance of instantiated object : %s", __FUNCTION__, oper_success->instantiated_path);
                err = USP_ERR_INTERNAL_ERROR;
                goto exit;
            }

            err = TEXT_UTILS_StringToInteger(sv_params.vector[sv_params.num_entries-1], instance);
            if (err != USP_ERR_OK)
            {
               goto exit;
            }

            // output key vector should contain name and value of each unique key
            for (i=0; i < oper_success->n_unique_keys; i++)
            {
                uk = oper_success->unique_keys[i];
                KV_VECTOR_Add(unique_keys, uk->key, uk->value);
            }
            break;

        default:
            TERMINATE_BAD_CASE(oper_status->oper_status_case);
    }

exit:
    STR_VECTOR_Destroy(&sv_params);

    return err;
}

/*********************************************************************//**
**
** ProcessUspService_SetResponse
**
** Processes a Set Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessUspService_SetResponse(Usp__Msg *resp)
{
    int i;
    int err = USP_ERR_OK;
    Usp__SetResp *set;
    Usp__SetResp__UpdatedObjectResult *obj_result;
    Usp__SetResp__UpdatedObjectResult__OperationStatus *oper_status;
    Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure;

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_SET_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if set response object is missing
    set = resp->body->response->set_resp;
    if (set == NULL)
    {
        USP_ERR_SetMessage("%s: Missing set response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all UpdatedObjResults, checking that all were successful
    // NOTE: We expect all of them to be successful if the code gets here since the Set Request has allow_partial=false and
    // all parameters are required to set, so we should have received an ERROR response if any failed to set
    for (i=0; i < set->n_updated_obj_results; i++)
    {
        obj_result = set->updated_obj_results[i];
        oper_status = obj_result->oper_status;
        switch(oper_status->oper_status_case)
        {
            case USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
                break;

            case USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
                oper_failure = oper_status->oper_failure;
                USP_ERR_SetMessage("%s", oper_failure->err_msg);
                err = oper_failure->err_code;
                break;

            default:
                TERMINATE_BAD_CASE(oper_status->oper_status_case);
                break;
        }
    }

    return err;
}

/*********************************************************************//**
**
** ProcessUspService_GetSupportedDMResponse
**
** Processes a GSDM Response that we have received from a USP Service.  This
** function populates the returned sv structure with all paths found in the USP
** response message without performing any filtering.
**
** \param   resp - USP response message in protobuf-c structure
** \param   kv - key/value vector containing paths of all supported parameters and their type
**
** \return  USP_ERR_OK if successful
**************************************************************************/
int ProcessUspService_GetSupportedDMResponse(Usp__Msg *usp, kv_vector_t *kv)
{
    int i,j,k;
    int err = USP_ERR_OK;

    Usp__GetSupportedDMResp *gsdm;
    Usp__GetSupportedDMResp__SupportedObjectResult *sor;
    Usp__GetSupportedDMResp__RequestedObjectResult *ror;
    Usp__GetSupportedDMResp__SupportedParamResult *sp;
    Usp__GetSupportedDMResp__SupportedEventResult *se;
    Usp__GetSupportedDMResp__SupportedCommandResult *sc;

    char path[MAX_DM_PATH];
    char *type;

    err = MSG_UTILS_ValidateUspResponse(usp, USP__RESPONSE__RESP_TYPE_GET_SUPPORTED_DM_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if get response is missing
    gsdm = usp->body->response->get_supported_dm_resp;
    if (gsdm == NULL)
    {
        USP_ERR_SetMessage("%s: Missing get response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all requested objects
    for (i=0; i < gsdm->n_req_obj_results; i++)
    {
        ror = gsdm->req_obj_results[i];

        // Exit if the USP Service encountered an error providing the supported data model for this path
        if (ror->err_code != USP_ERR_OK)
        {
            USP_ERR_SetMessage("%s: USP broker did not provide data model for '%s' (err_code=%d, err_msg='%s')", __FUNCTION__, ror->req_obj_path, ror->err_code, ror->err_msg);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Iterate over all supported objects and add them to return vector
        for (j=0; j < ror->n_supported_objs; j++)
        {
            sor = ror->supported_objs[j];
            if ((sor->supported_obj_path != NULL) && (*sor->supported_obj_path != '\0'))
            {
                // Exit if the path does not begin with "Device."
                if (strncmp(sor->supported_obj_path, dm_root, dm_root_len) != 0)
                {
                    USP_ERR_SetMessage("%s: Response contains invalid data model path '%s'", __FUNCTION__, path);
                    return USP_ERR_INTERNAL_ERROR;
                }

                // Iterate over paramaters, adding them to the returned vector
                for (k=0; k < sor->n_supported_params; k++)
                {
                    sp = sor->supported_params[k];
                    USP_SNPRINTF(path, MAX_DM_PATH, "%s%s", sor->supported_obj_path,  sp->param_name);
                    type = ParamTypeToUspServiceString(sp->value_type);
                    KV_VECTOR_Add(kv, path, type);
                }

                // Iterate over events, adding them to the returned vector
                for (i=0; i < sor->n_supported_events; i++)
                {
                    se = sor->supported_events[i];
                    USP_SNPRINTF(path, MAX_DM_PATH, "%s%s", sor->supported_obj_path,  se->event_name);
                    KV_VECTOR_Add(kv, path, "event");
                }

                // Iterate over operations, adding them to the returned vector
                for (i=0; i < sor->n_supported_commands; i++)
                {
                    sc = sor->supported_commands[i];
                    USP_SNPRINTF(path, MAX_DM_PATH, "%s%s", sor->supported_obj_path,  sc->command_name);
                    switch(sc->command_type)
                    {
                        case USP__GET_SUPPORTED_DMRESP__CMD_TYPE__CMD_SYNC:
                            KV_VECTOR_Add(kv, path, "sync_cmd");
                            break;

                        case USP__GET_SUPPORTED_DMRESP__CMD_TYPE__CMD_ASYNC:
                        default:
                            KV_VECTOR_Add(kv, path, "async_cmd");
                            break;
                    }
                }
            }
        }
    }

    return err;
}

/*********************************************************************//**
**
** ProcessUspService_GetInstancesResponse
**
** Processes a GetInstances Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   sv - string vector containing paths of all instances of an object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessUspService_GetInstancesResponse(Usp__Msg *resp, str_vector_t *sv)
{
    int i, j;
    int err = USP_ERR_OK;
    Usp__GetInstancesResp *geti;
    Usp__GetInstancesResp__RequestedPathResult *rpr;
    Usp__GetInstancesResp__CurrInstance *ci;
    char *path;

    // Exit if failed to validate that the Message body contains a GetInstances Response
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_GET_INSTANCES_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if get instances response is missing
    geti = resp->body->response->get_instances_resp;
    if (geti == NULL)
    {
        USP_ERR_SetMessage("%s: Missing get instances response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all requested path results
    USP_ASSERT((geti->n_req_path_results==0) || (geti->req_path_results != NULL));
    for (i=0; i < geti->n_req_path_results; i++)
    {
        // Skip this result if it is not filled in. NOTE: This should never happen
        rpr = geti->req_path_results[i];
        if (rpr == NULL)
        {
            continue;
        }

        // Exit if we received an error for this object
        if (rpr->err_code != USP_ERR_OK)
        {
            USP_ERR_SetMessage("%s: Failed to get instances for object '%s' (err_msg=%s)", __FUNCTION__, rpr->requested_path, rpr->err_msg);
            STR_VECTOR_Destroy(sv);
            return rpr->err_code;
        }

        // Iterate over all current instance objects
        for (j=0; j < rpr->n_curr_insts; j++)
        {
            ci = rpr->curr_insts[j];
            if (ci != NULL)
            {
                path = ci->instantiated_obj_path;
                if ((path != NULL) && (*path != '\0'))
                {
                    if (strncmp(path, dm_root, dm_root_len) != 0)
                    {
                        USP_ERR_SetMessage("%s: Response contains invalid data model path '%s'", __FUNCTION__, path);
                        STR_VECTOR_Destroy(sv);
                        return USP_ERR_INTERNAL_ERROR;
                    }
                    STR_VECTOR_Add(sv, path);
                }
            }
        }
    }

    return USP_ERR_OK;
}


/*********************************************************************//**
**
** ProcessUspService_DeleteResponse
**
** Processes a Delete Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   path - pointer to string containing the data model object that we requested to delete
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessUspService_DeleteResponse(Usp__Msg *resp, char *path)
{
    int err = USP_ERR_OK;
    Usp__DeleteResp *del;
    Usp__DeleteResp__DeletedObjectResult *deleted_obj_result;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus *oper_status;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *oper_failure;
    char *param_errs_path = NULL;
    int i;

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_DELETE_RESP, &param_errs_path);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if delete response is missing
    del = resp->body->response->delete_resp;
    if (del == NULL)
    {
        USP_ERR_SetMessage("%s: Missing delete response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // iterate over all deleted objects
    for (i = 0 ; i < del->n_deleted_obj_results ; i++)
    {
       deleted_obj_result = del->deleted_obj_results[i];

       // Exit if this response is for a different requested path
       if (strcmp( path, deleted_obj_result->requested_path ))
       {
           USP_ERR_SetMessage("%s: Unexpected requested path in DeleteResponse (%s)", __FUNCTION__,  deleted_obj_result->requested_path);
           return USP_ERR_INTERNAL_ERROR;
       }

       // Determine whether the object was deleted successfully or failed
       oper_status = deleted_obj_result->oper_status;
       switch(oper_status->oper_status_case)
       {
           case USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
               // NOTE: The USP Service should have sent an Error response instead of an OperFailure, because we sent the Delete request with allow_partial=false
               oper_failure = oper_status->oper_failure;
               USP_ERR_SetMessage("Failed to delete %s (err_msg=%s)", deleted_obj_result->requested_path, oper_failure->err_msg);
               err = oper_failure->err_code;
               goto exit;
               break;

           case USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
               break;

           default:
               TERMINATE_BAD_CASE(oper_status->oper_status_case);
               break;
       }
    }

exit:
    return err;
}

/*********************************************************************//**
**
** ProcessUspService_OperateResponse
**
** Processes a Operate Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   path - USP command that was attempted
** \param   output_args - pointer to key-value vector to fill in with the output arguments parsed from the USP esponse message
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessUspService_OperateResponse(Usp__Msg *resp, char *path, kv_vector_t *output_args)
{
    int i;
    int err = USP_ERR_OK;
    Usp__OperateResp *oper;
    Usp__OperateResp__OperationResult *res;
    Usp__OperateResp__OperationResult__OutputArgs *args;
    Usp__OperateResp__OperationResult__CommandFailure *fail;
    Usp__OperateResp__OperationResult__OutputArgs__OutputArgsEntry *entry;

    // Initialise default output arguments
    KV_VECTOR_Init(output_args);

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_OPERATE_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if operate response is missing
    oper = resp->body->response->operate_resp;
    if (oper == NULL)
    {
        USP_ERR_SetMessage("%s: Missing operate response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the number of operation_results does not match the expected number
    // The implementation currently expects a single operation path (without wildcards)
    if (oper->n_operation_results != 1)
    {
        USP_ERR_SetMessage("%s: Too many operation results (%d). USP_SERVICE_Operate() can only return the output args of a single USP command", __FUNCTION__, (int)oper->n_operation_results);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the operation wasn't the one we requested
    res = oper->operation_results[0];
    if (strcmp(res->executed_command, path) != 0)
    {
        USP_ERR_SetMessage("%s: Unexpected operation in response (got='%s', expected=%s')", __FUNCTION__, res->executed_command, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Determine if the operation was successful (for sync command) or successfully started (for async commands)
    switch(res->operation_resp_case)
    {
        case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OBJ_PATH:
            // Async Operation started
            err = USP_ERR_OK;
            break;

        case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS:
            // Operation succeeded: Copy across output arguments
            args = res->req_output_args;
            for (i=0; i < args->n_output_args; i++)
            {
                entry = args->output_args[i];
                KV_VECTOR_Add(output_args, entry->key, entry->value);
            }

            err = USP_ERR_OK;
            break;

        case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE:
            // Operation failed
            fail = res->cmd_failure;
            USP_ERR_SetMessage("%s", fail->err_msg);
            err = fail->err_code;
            break;

        default:
            break;
    }

    return err;
}

/*********************************************************************//**
**
** ProcessUspService_Notification
**
** Processes a Notification request from the broker and calls the vendor
** layer callback if the notification pertains to an active subscription.
**
** \param   notify - A USP notification request in protobuf-c structure
**
** \return  True if the notification was handled, False if not for us.
**
**************************************************************************/
void ProcessUspService_Notification(Usp__Notify *notify)
{
    kv_vector_t args;
    char *cmd_key = NULL;
    char *err_msg = "";
    int err_code = USP_ERR_OK;
    char *path = NULL;
    char buf[MAX_DM_PATH];
    int i;

    if (notify == NULL)
    {
       return;
    }

    if (usp_service_notify_cb == NULL)
    {
       // ignore the notification as no callback is registered
       return;
    }

    KV_VECTOR_Init(&args);

    switch(notify->notification_case)
    {
        case USP__NOTIFY__NOTIFICATION_VALUE_CHANGE:
            path = notify->value_change->param_path;
            KV_VECTOR_Add(&args, notify->value_change->param_path, notify->value_change->param_value);
            break;

        case USP__NOTIFY__NOTIFICATION_EVENT:
            USP_SNPRINTF(buf, sizeof(buf), "%s%s", notify->event->obj_path, notify->event->event_name);
            path = buf;
            for (i = 0 ; i < notify->event->n_params ; i++)
            {
               KV_VECTOR_Add(&args, notify->event->params[i]->key, notify->event->params[i]->value);
            }
            break;

        case USP__NOTIFY__NOTIFICATION_OBJ_CREATION:
            path = notify->obj_creation->obj_path;
            for (i = 0 ; i < notify->obj_creation->n_unique_keys ; i++)
            {
               KV_VECTOR_Add(&args, notify->obj_creation->unique_keys[i]->key, notify->obj_creation->unique_keys[i]->value);
            }
            break;

        case USP__NOTIFY__NOTIFICATION_OBJ_DELETION:
            path = notify->obj_deletion->obj_path;
            break;

        case USP__NOTIFY__NOTIFICATION_OPER_COMPLETE:
            USP_SNPRINTF(buf, sizeof(buf), "%s%s", notify->oper_complete->obj_path, notify->oper_complete->command_name);
            path = buf;
            cmd_key = notify->oper_complete->command_key;
            switch (notify->oper_complete->operation_resp_case)
            {
                case USP__NOTIFY__OPERATION_COMPLETE__OPERATION_RESP_REQ_OUTPUT_ARGS:
                    for (i = 0 ; i < notify->oper_complete->req_output_args->n_output_args ; i++)
                    {
                       KV_VECTOR_Add(&args, notify->oper_complete->req_output_args->output_args[i]->key, notify->oper_complete->req_output_args->output_args[i]->value);
                    }
                    break;

                case USP__NOTIFY__OPERATION_COMPLETE__OPERATION_RESP_CMD_FAILURE:
                    err_code =  notify->oper_complete->cmd_failure->err_code;
                    err_msg =  notify->oper_complete->cmd_failure->err_msg;
                    break;

                default:
                    TERMINATE_BAD_CASE(notify->oper_complete->operation_resp_case);
                    break;
            }
            break;

        default:
        case USP__NOTIFY__NOTIFICATION__NOT_SET:
        case USP__NOTIFY__NOTIFICATION_ON_BOARD_REQ:
            USP_ERR_SetMessage("%s: Incorrect type (%d) in received notification", __FUNCTION__, notify->notification_case);
            return;
            break;
    }

    usp_service_notify_cb(notify->subscription_id, path, &args, cmd_key, err_code, err_msg);

    KV_VECTOR_Destroy(&args);

    return;
}


#endif // REMOVE_USP_SERVICE
