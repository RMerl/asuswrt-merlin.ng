/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2016-2024  CommScope, Inc
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
 * \file handle_notify.c
 *
 * Handles the generation and queueing of Notify messages
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <protobuf-c/protobuf-c.h>

#include "usp-msg.pb-c.h"
#include "common_defs.h"
#include "msg_handler.h"
#include "proto_trace.h"
#include "data_model.h"
#include "subs_vector.h"
#include "subs_retry.h"
#include "iso8601.h"
#include "text_utils.h"
#include "device.h"

//------------------------------------------------------------------------------
// Array containing count of number of messages of each type
unsigned sub_notify_count[kSubNotifyType_Max] = {0};

//------------------------------------------------------------------------------
// Variable containing the count of onboard request messages
unsigned onboard_request_count = 0;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
Usp__Msg *CreateOperComplete(char *command, char *command_key, char *subscription_id, bool send_resp);
Usp__Msg *CreateNotify(char *msg_id, char *subscription_id, bool send_resp, Usp__Notify__NotificationCase notification_case);
char *OnBoardRequestMessageId(char *msg_id, int len);
void AddObjCreation_UniqueKeys(Usp__Notify__ObjectCreation *obj_creation, kv_vector_t *kvv);


/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_ValueChange
**
** Creates a Value Change Notify message
**
** \param   path - pointer to string representing the data model path of the parameter whose value has changed
** \param   value - pointer to string representing the new value of the parameter
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_ValueChange(char *path, char *value, char *subscription_id, bool send_resp)
{
    Usp__Msg *req;
    Usp__Notify__ValueChange *value_change;
    char msg_id[MAX_MSG_ID_LEN];

    // Get data stored in the NotifyRequest
    MSG_HANDLER_CalcNotifyMsgId(kSubNotifyType_ValueChange, msg_id, sizeof(msg_id));

    // Create a NotifyRequest
    req = CreateNotify(msg_id, subscription_id, send_resp, USP__NOTIFY__NOTIFICATION_VALUE_CHANGE);

    // Allocate and initialise memory to store the value change
    value_change = USP_MALLOC(sizeof(Usp__Notify__ValueChange));
    usp__notify__value_change__init(value_change);

    value_change->param_path = USP_STRDUP(path);
    value_change->param_value = USP_STRDUP(value);

    // Connect the value change into the NotifyRequest object
    req->body->request->notify->value_change = value_change;

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_HandleNotifyResp
**
** Handles a USP NotifyResponse message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**                         NOTE: Controller might not populate the 'reply-to' field for notify response messages
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleNotifyResp(Usp__Msg *usp, char *controller_endpoint, mtp_conn_t *mtpc)
{
    Usp__Msg *resp = NULL;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_RESPONSE) ||
        (usp->body->response == NULL) || (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_NOTIFY_RESP) ||
        (usp->body->response->notify_resp == NULL) ||
        (usp->header->msg_id == NULL) || (usp->body->response->notify_resp->subscription_id == NULL))
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        MSG_HANDLER_QueueMessage(controller_endpoint, resp, mtpc);
        usp__msg__free_unpacked(resp, pbuf_allocator);
        return;
    }

    // Stop attempting to retry sending the NotifyReq message as we have the response from the controller now
    SUBS_RETRY_Remove(usp->header->msg_id, usp->body->response->notify_resp->subscription_id);
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_ObjectCreation
**
** Creates an Object Creation Notify message for the specified controller endpoint
**
** \param   obj_path - path of object that has been successfully created in the data model
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_ObjectCreation(char *obj_path, char *subscription_id, bool send_resp)
{
    Usp__Msg *req;
    char msg_id[MAX_MSG_ID_LEN];
    Usp__Notify__ObjectCreation *obj_creation;
    kv_vector_t unique_key_params;
    int err;

    // Get data to store in the NotifyRequest
    MSG_HANDLER_CalcNotifyMsgId(kSubNotifyType_ObjectCreation, msg_id, sizeof(msg_id));

    // Create a NotifyRequest
    req = CreateNotify(msg_id, subscription_id, send_resp, USP__NOTIFY__NOTIFICATION_OBJ_CREATION);

    // Allocate and initialise memory to store the object creation structure. Connect the structure into the NotifyRequest
    obj_creation = USP_MALLOC(sizeof(Usp__Notify__ObjectCreation));
    usp__notify__object_creation__init(obj_creation);
    req->body->request->notify->obj_creation = obj_creation;

    // Fill in the name of the created object
    obj_creation->obj_path = TEXT_UTILS_StrDupWithTrailingDot(obj_path);

    // Exit if unable to get the unique keys
    err = DATA_MODEL_GetUniqueKeyParams(obj_path, &unique_key_params, INTERNAL_ROLE);
    if (err != USP_ERR_OK)
    {
        // If this occurs, we still want to notify that the object was created, so just don't put any unique keys in the message
        return req;
    }

    // Move the unique key parameters to the USP obj creation message
    // NOTE: The unique_key_params vector will be destroyed in the process, so we do not have to destroy it here
    if (unique_key_params.num_entries > 0)
    {
        AddObjCreation_UniqueKeys(obj_creation, &unique_key_params);
    }

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_ObjectDeletion
**
** Creates an Object Deletion Notify message for the specified controller endpoint
**
** \param   obj_path - path of object that has been successfully deleted from the data model
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_ObjectDeletion(char *obj_path, char *subscription_id, bool send_resp)
{
    Usp__Msg *req = NULL;
    char msg_id[MAX_MSG_ID_LEN];
    Usp__Notify__ObjectDeletion *obj_deletion;

    // Get data to store in the NotifyRequest
    MSG_HANDLER_CalcNotifyMsgId(kSubNotifyType_ObjectDeletion, msg_id, sizeof(msg_id));

    // Create a NotifyRequest
    req = CreateNotify(msg_id, subscription_id, send_resp, USP__NOTIFY__NOTIFICATION_OBJ_DELETION);

    // Allocate and initialise memory to store the object deletion structure. Connect the structure into the NotifyRequest
    obj_deletion = USP_MALLOC(sizeof(Usp__Notify__ObjectDeletion));
    usp__notify__object_deletion__init(obj_deletion);
    req->body->request->notify->obj_deletion = obj_deletion;

    // NOTE: The ObjectDeletion notify message differs from the ObjectCreation notify message,
    // in that it does not contain the unique key values identifying the object

    // Fill in the name of the deleted object
    obj_deletion->obj_path = TEXT_UTILS_StrDupWithTrailingDot(obj_path);

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_OperCompleteSuccess
**
** Creates an Operation Complete (Success) Notify message for the specified controller endpoint
**
** \param   output_args - key-value vector containing the output arguments of the completed operation. NULL indicates no output arguments.
** \param   command - path to operation in the data model
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_OperCompleteSuccess(kv_vector_t *output_args, char *command, char *command_key,
                                                          char *subscription_id, bool send_resp)
{
    Usp__Msg *req;
    Usp__Notify__OperationComplete *oper_complete;
    Usp__Notify__OperationComplete__OutputArgs *req_output_args;
    Usp__Notify__OperationComplete__OutputArgs__OutputArgsEntry *entry;
    int i;
    kv_pair_t *kv;
    int num_entries;

    // Create the operation complete request
    req = CreateOperComplete(command, command_key, subscription_id, send_resp);
    oper_complete = req->body->request->notify->oper_complete;
    oper_complete->operation_resp_case = USP__NOTIFY__OPERATION_COMPLETE__OPERATION_RESP_REQ_OUTPUT_ARGS;

    // Allocate and initialise memory for the oper complete output args structure
    req_output_args = USP_MALLOC(sizeof(Usp__Notify__OperationComplete__OutputArgs));
    usp__notify__operation_complete__output_args__init(req_output_args);
    oper_complete->req_output_args = req_output_args;

    // Exit if there are no output arguments to send
    if (output_args == NULL)
    {
        req_output_args->n_output_args = 0;
        req_output_args->output_args = NULL;
        return req;
    }

    // Allocate output_args array
    num_entries = output_args->num_entries;
    req_output_args->n_output_args = num_entries;
    req_output_args->output_args = USP_MALLOC(num_entries*sizeof(void *));

    // Iterate over all output args, adding them to the output arg map array
    for (i=0; i < num_entries; i++)
    {
        // Allocate and initialise memory to store the output arg entry
        entry = USP_MALLOC(sizeof(Usp__Notify__OperationComplete__OutputArgs__OutputArgsEntry));
        usp__notify__operation_complete__output_args__output_args_entry__init(entry);

        // Copy the param values into the output arg entry
        kv = &output_args->vector[i];
        entry->key = USP_STRDUP(kv->key);
        entry->value = USP_STRDUP(kv->value);

        // Attach the output arg entry into the output arg map array
        req_output_args->output_args[i] = entry;
    }

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_OperCompleteFailure
**
** Creates an Operation Complete (Failure) Notify message for the specified controller endpoint
**
** \param   err_code - error code representing the cause of the failure to create
** \param   err_msg - string representing the cause of the error
** \param   command - path to operation in the data model
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_OperCompleteFailure(int err_code, char *err_msg, char *command, char *command_key,
                                                          char *subscription_id, bool send_resp)
{
    Usp__Msg *req;
    Usp__Notify__OperationComplete *oper_complete;
    Usp__Notify__OperationComplete__CommandFailure *cmd_failure;

    // Create the operation complete request
    req = CreateOperComplete(command, command_key, subscription_id, send_resp);
    oper_complete = req->body->request->notify->oper_complete;
    oper_complete->operation_resp_case = USP__NOTIFY__OPERATION_COMPLETE__OPERATION_RESP_CMD_FAILURE;

    // Allocate and initialise memory for the cmd failure structure
    cmd_failure = USP_MALLOC(sizeof(Usp__Notify__OperationComplete__CommandFailure));
    usp__notify__operation_complete__command_failure__init(cmd_failure);
    oper_complete->cmd_failure = cmd_failure;

    // Fill in the cmd failure structure
    cmd_failure->err_code = err_code;
    cmd_failure->err_msg = USP_STRDUP(err_msg);

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_Event
**
** Creates an Event Notify message for the specified controller endpoint
**
** \param   event_name - full path of the event in the data model
** \param   output_args - list of output arguments and their associated values.  NULL indicates no output arguments.
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_Event(char *event_name, kv_vector_t *output_args, char *subscription_id, bool send_resp)
{
    char msg_id[MAX_MSG_ID_LEN];
    Usp__Msg *req;
    Usp__Notify__Event *event;
    Usp__Notify__Event__ParamsEntry *entry;
    int i;
    kv_pair_t *kv;
    int num_entries;
    char *name;
    char buf[MAX_DM_PATH];

    // Get data to store in the NotifyRequest
    MSG_HANDLER_CalcNotifyMsgId(kSubNotifyType_Event, msg_id, sizeof(msg_id));

    // Create a NotifyRequest
    req = CreateNotify(msg_id, subscription_id, send_resp, USP__NOTIFY__NOTIFICATION_EVENT);

    // Allocate and initialise memory to store the event structure. Connect the event structure into the NotifyRequest
    event = USP_MALLOC(sizeof(Usp__Notify__Event));
    usp__notify__event__init(event);
    req->body->request->notify->event = event;

    // Split the event name into object path and event
    name = TEXT_UTILS_SplitPath(event_name, buf, sizeof(buf));
    event->obj_path = USP_STRDUP(buf);
    event->event_name = USP_STRDUP(name);

    // Exit if there are no output arguments to add
    if (output_args == NULL)
    {
        event->n_params = 0;
        event->params = NULL;
        return req;
    }

    // Allocate param map array
    num_entries = output_args->num_entries;
    event->n_params = num_entries;
    event->params = USP_MALLOC(num_entries*sizeof(void *));

    // Iterate over all param values, adding them to the params
    for (i=0; i < num_entries; i++)
    {
        // Allocate and initialise memory to store the param map entry
        entry = USP_MALLOC(sizeof(Usp__Notify__Event__ParamsEntry));
        usp__notify__event__params_entry__init(entry);

        // Copy the param values into the param map entry
        kv = &output_args->vector[i];
        entry->key = USP_STRDUP(kv->key);
        entry->value = USP_STRDUP(kv->value);

        // Attach the param map entry into the param map array
        event->params[i] = entry;
    }

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_CreateNotifyReq_OnBoard
**
** Creates OnBoardRequest notification
**
** \param   oui - agent oui
** \param   product_class - agent product class
** \param   serial_number - agent serial number
** \param   send_resp - Set to true if we require the controller to send a response
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *MSG_HANDLER_CreateNotifyReq_OnBoard(char* oui, char* product_class, char* serial_number, bool send_resp)
{
    char msg_id[MAX_MSG_ID_LEN];
    Usp__Msg *req;
    Usp__Notify__OnBoardRequest *event;

    // Create message id for OnBoardRequest
    OnBoardRequestMessageId(msg_id, sizeof(msg_id));

    // Create the notification
    // Subscription id is an empty string according to R-NOT.7
    req = CreateNotify(msg_id, "", send_resp, USP__NOTIFY__NOTIFICATION_ON_BOARD_REQ);

    // Allocate and initialise memory to store the structure
    event = USP_MALLOC(sizeof(Usp__Notify__OnBoardRequest));
    usp__notify__on_board_request__init(event);

    // set the values
    event->oui = USP_STRDUP(oui);
    event->product_class = USP_STRDUP(product_class);
    event->serial_number = USP_STRDUP(serial_number);
    event->agent_supported_protocol_versions = USP_STRDUP(AGENT_SUPPORTED_PROTOCOL_VERSIONS);

    req->body->request->notify->on_board_req = event;

    return req;
}

/*********************************************************************//**
**
** MSG_HANDLER_CalcNotifyMsgId
**
** Creates a unique message id for a notify message
**
** \param   notify_type - type of notify message
** \param   msg_id - pointer to buffer in which to write the message id
** \param   len - length of buffer
**
** \return  pointer to start of buffer
**
**************************************************************************/
char *MSG_HANDLER_CalcNotifyMsgId(subs_notify_t notify_type, char *msg_id, int len)
{
    char *notify_str;
    unsigned count;
    char buf[MAX_ISO8601_LEN];

    count = sub_notify_count[notify_type];
    count++;               // Pre-increment before forming message, because we want to count from 1, and at bootup sub_notify_count[] is zeroed
    sub_notify_count[notify_type] = count;

    notify_str = TEXT_UTILS_EnumToString(notify_type, notify_types, NUM_ELEM(notify_types));

    // Form a message id string which is unique.
    {
        // In production, the string must be unique even across reboots because RabbitMQ queues responses from the controller
        // and may deliver them at Reboot (and we don't want these responses to inadvertently be for fresh NotifyRequests)
        USP_SNPRINTF(msg_id, len, "%s-%s-%d", notify_str, iso8601_cur_time(buf, sizeof(buf)), count);
    }

    return msg_id;
}


/*********************************************************************//**
**
** CreateOperComplete
**
** Creates an Operation Complete Notify message for the specified controller endpoint
** NOTE: This function does not fill in the success or failure structure - the caller must do this
**
** \param   command - path to operation in the data model
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
**
** \return  Pointer to message created
**
**************************************************************************/
Usp__Msg *CreateOperComplete(char *command, char *command_key, char *subscription_id, bool send_resp)
{
    char msg_id[MAX_MSG_ID_LEN];
    Usp__Msg *req;
    Usp__Notify__OperationComplete *oper_complete;
    char *name;
    char buf[MAX_DM_PATH];

    // Get data to store in the NotifyRequest
    MSG_HANDLER_CalcNotifyMsgId(kSubNotifyType_OperationComplete, msg_id, sizeof(msg_id));

    // Create a NotifyRequest
    req = CreateNotify(msg_id, subscription_id, send_resp, USP__NOTIFY__NOTIFICATION_OPER_COMPLETE);

    // Allocate and initialise memory to store the operation complete structure. Connect the structure into the NotifyRequest
    oper_complete = USP_MALLOC(sizeof(Usp__Notify__OperationComplete));
    usp__notify__operation_complete__init(oper_complete);
    req->body->request->notify->oper_complete = oper_complete;

    // Split the command into object path and name
    name = TEXT_UTILS_SplitPath(command, buf, sizeof(buf));
    oper_complete->obj_path = USP_STRDUP(buf);
    oper_complete->command_name = USP_STRDUP(name);

    oper_complete->command_key = USP_STRDUP(command_key);

    return req;
}

/*********************************************************************//**
**
** CreateNotify
**
** Dynamically creates a generic Notify object
**
** \param   msg_id - string containing the message id of the request, which initiated this response
** \param   subscription_id - identifier string which was set by the controller to identify this notification (Device.LocalAgent.Subscription.{i}.ID)
** \param   send_resp - Set to true if we require the controller to send a response (otherwise we keep retrying)
**                      The value of this parameter was set by the controller (in Device.LocalAgent.Subscription.{i}.NotifRetry)
** \param   notification_case - type of notification to send
**
** \return  Pointer to a Notify object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateNotify(char *msg_id, char *subscription_id, bool send_resp, Usp__Notify__NotificationCase notification_case)
{
    Usp__Msg *msg;
    Usp__Notify *notify;

    // Create Notify Request
    msg = MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__NOTIFY, USP__REQUEST__REQ_TYPE_NOTIFY);
    notify = USP_MALLOC(sizeof(Usp__Notify));
    usp__notify__init(notify);
    msg->body->request->notify = notify;

    notify->subscription_id = USP_STRDUP(subscription_id);
    notify->send_resp = send_resp;
    notify->notification_case = notification_case;

    return msg;
}

/*********************************************************************//**
**
** AddObjCreation_UniqueKeys
**
** Moves the specified unique keys to an ObjCreation object, destroying
** the key-value vector in the process (this is done to prevent unnecessary mallocs)
**
** \param   obj_creation - pointer to obj_creation object to add this unique key map to
** \param   kvv - pointer to key-value vector containing the unique key map
**
** \return  None
**
**************************************************************************/
void AddObjCreation_UniqueKeys(Usp__Notify__ObjectCreation *obj_creation, kv_vector_t *kvv)
{
    Usp__Notify__ObjectCreation__UniqueKeysEntry *entry;
    kv_pair_t *kv;
    int i;

    USP_ASSERT((kvv->num_entries > 0) && (kvv->vector != NULL));

    // Allocate the unique key map vector
    obj_creation->n_unique_keys = kvv->num_entries;
    obj_creation->unique_keys = USP_MALLOC(kvv->num_entries*sizeof(void *));

    // Add all unique keys to the unique key map
    for (i=0; i < kvv->num_entries; i++)
    {
        // Allocate memory to store the map entry
        entry = USP_MALLOC(sizeof(Usp__Notify__ObjectCreation__UniqueKeysEntry));
        usp__notify__object_creation__unique_keys_entry__init(entry);
        obj_creation->unique_keys[i] = entry;

        // Move the key and value from the key-value vector to the map entry
        kv = &kvv->vector[i];
        entry->key = kv->key;
        entry->value = kv->value;
    }

    // Finally destroy the key-value vector, since we have moved it's contents
    USP_FREE(kvv->vector);
    kvv->vector = NULL;         // Not strictly necessary
    kvv->num_entries = 0;
}

/*********************************************************************//**
**
** OnBoardRequestMessageId
**
** Creates a unique message id for onboard request message
**
** \param   msg_id - pointer to buffer in which to write the message id
** \param   len - length of buffer
**
** \return  pointer to start of buffer
**
**************************************************************************/
char *OnBoardRequestMessageId(char *msg_id, int len)
{
    char *notify_str = "OnBoardRequest";
    unsigned count;
    char buf[MAX_ISO8601_LEN];

    count = onboard_request_count;
    count++;
    onboard_request_count = count;

    // Form a message id string which is unique.
    {
        // In production, the string must be unique even across reboots because RabbitMQ queues responses from the controller
        // and may deliver them at Reboot (and we don't want these responses to inadvertently be for fresh NotifyRequests)
        USP_SNPRINTF(msg_id, len, "%s-%s-%d", notify_str, iso8601_cur_time(buf, sizeof(buf)), count);
    }

    return msg_id;
}

