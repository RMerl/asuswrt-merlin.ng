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
 * \file handle_operate.c
 *
 * Handles the OperateRequest message, creating an OperateResponse
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <protobuf-c/protobuf-c.h>

#include "usp-msg.pb-c.h"
#include "common_defs.h"
#include "msg_handler.h"
#include "dm_trans.h"
#include "dm_access.h"
#include "path_resolver.h"
#include "proto_trace.h"
#include "device.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
Usp__Msg *CreateOperResp(char *msg_id);
void AddOperRes_CmdFailure(Usp__OperateResp *oper_resp, char *path, int err_code, char *err_msg);
void AddOperRes_ReqObjPath(Usp__OperateResp *oper_resp, char *path, int instance);
void AddOperRes_ReqOutputArgs(Usp__OperateResp *oper_resp, char *path, kv_vector_t *output_args);


/*********************************************************************//**
**
** MSG_HANDLER_HandleOperate
**
** Handles a USP Operate message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleOperate(Usp__Msg *usp, char *controller_endpoint, mtp_conn_t *mtpc)
{
    int i;
    int err;
    Usp__Operate *oper = NULL;
    Usp__Msg *resp = NULL;
    Usp__OperateResp *oper_resp;
    dm_trans_vector_t trans;
    str_vector_t paths;
    kv_vector_t input_args;
    kv_vector_t output_args;
    Usp__Operate__InputArgsEntry *ame;
    char *oper_path;
    int instance;
    combined_role_t combined_role;

    // Initialise all structures that may be freed on exit
    KV_VECTOR_Init(&input_args);
    STR_VECTOR_Init(&paths);

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_OPERATE) ||
        (usp->body->request->operate == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }

    // Exit if message says that it has args, but protobuf parser did not find them
    oper = usp->body->request->operate;
    if ((oper->n_input_args > 0) && (oper->input_args == NULL))
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }

    // Copy the operate arguments into a key value vector.
    // This is necessary to handle the freeing of this vector in a consistent way - whether it is constructed here, or after a power cycle retry
    for (i=0; i < oper->n_input_args; i++)
    {
        // Exit if key-value pair is not present in USP input message
        ame = oper->input_args[i];
        if ((ame == NULL) || (ame->key == NULL) || (ame->value == NULL))
        {
            USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
            goto exit;
        }

        // Add the key value pair from the USP input message to our key value vector
        KV_VECTOR_Add(&input_args, ame->key, ame->value);
    }

    // Create an Operate Response
    resp = CreateOperResp(usp->header->msg_id);
    oper_resp = resp->body->response->operate_resp;

    // Exit if unable to resolve the paths to the operation
    MSG_HANDLER_GetMsgRole(&combined_role);
    err = PATH_RESOLVER_ResolveDevicePath(oper->command, &paths, NULL, kResolveOp_Oper, FULL_DEPTH, &combined_role, 0);
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp);
        goto exit;
    }

    // Iterate over all operation paths
    for (i=0; i < paths.num_entries; i++)
    {
        // Exit if unable to start a transaction
        err = DM_TRANS_Start(&trans);
        if (err != USP_ERR_OK)
        {
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp);
            goto exit;
        }

        // Perform the operation
        oper_path = paths.vector[i];
        KV_VECTOR_Init(&output_args);
        err = DATA_MODEL_Operate(oper_path, &input_args, &output_args, oper->command_key, &instance);
        if (err != USP_ERR_OK)
        {
            // Operation failed
            AddOperRes_CmdFailure(oper_resp, oper_path, err, USP_ERR_GetMessage());

            // Abort the transaction
            DM_TRANS_Abort();
        }
        else
        {
            if (instance != INVALID)
            {
                // Asynchronous operation started successfully
                AddOperRes_ReqObjPath(oper_resp, oper_path, instance);
            }
            else
            {
                // Synchronous operation completed successfully
                AddOperRes_ReqOutputArgs(oper_resp, oper_path, &output_args);
            }

            // Commit the transaction
            DM_TRANS_Commit();
        }

        KV_VECTOR_Destroy(&output_args);
    }


exit:
    STR_VECTOR_Destroy(&paths);
    KV_VECTOR_Destroy(&input_args);

    // Send the response (if required)
    if ((oper != NULL) && (oper->send_resp) && (resp != NULL))
    {
        MSG_HANDLER_QueueMessage(controller_endpoint, resp, mtpc);
    }

    // Free the response structure
    if (resp != NULL)
    {
        usp__msg__free_unpacked(resp, pbuf_allocator);
    }
}

/*********************************************************************//**
**
** CreateOperResp
**
** Dynamically creates an OperResponse object
** NOTE: The object is created without any created_obj_results
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the add request, which initiated this response
**
** \return  Pointer to an OperResponse object
**
**************************************************************************/
Usp__Msg *CreateOperResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__OperateResp *oper_resp;


    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__OPERATE_RESP, USP__RESPONSE__RESP_TYPE_OPERATE_RESP);
    oper_resp = USP_MALLOC(sizeof(Usp__OperateResp));
    usp__operate_resp__init(oper_resp);
    msg->body->response->operate_resp = oper_resp;

    oper_resp->n_operation_results = 0;    // Start from an empty list
    oper_resp->operation_results = NULL;

    return msg;
}

/*********************************************************************//**
**
** AddOperRes_CmdFailure
**
** Adds a command failure to an OperationResponse object
**
** \param   oper_resp - pointer to operation response object to add this entry to
** \param   path - path to operation which failed
** \param   err_code - error code representing the cause of the failure to create
** \param   err_msg - string representing the cause of the error
**
** \return  None
**
**************************************************************************/
void AddOperRes_CmdFailure(Usp__OperateResp *oper_resp, char *path, int err_code, char *err_msg)
{
    Usp__OperateResp__OperationResult *oper_res;
    Usp__OperateResp__OperationResult__CommandFailure *cmd_failure;
    int new_num;    // new number of entries in the operation_result array

    // Allocate memory to store the structures
    oper_res = USP_MALLOC(sizeof(Usp__OperateResp__OperationResult));
    usp__operate_resp__operation_result__init(oper_res);

    cmd_failure = USP_MALLOC(sizeof(Usp__OperateResp__OperationResult__CommandFailure));
    usp__operate_resp__operation_result__command_failure__init(cmd_failure);

    // Increase the size of the vector
    new_num = oper_resp->n_operation_results + 1;
    oper_resp->operation_results = USP_REALLOC(oper_resp->operation_results, new_num*sizeof(void *));
    oper_resp->n_operation_results = new_num;
    oper_resp->operation_results[new_num-1] = oper_res;

    // Initialise the operation result
    oper_res->executed_command = USP_STRDUP(path);
    oper_res->operation_resp_case = USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE;
    oper_res->cmd_failure = cmd_failure;

    // Initialise the command failure
    cmd_failure->err_code = err_code;
    cmd_failure->err_msg = USP_STRDUP(err_msg);
}

/*********************************************************************//**
**
** AddOperRes_ReqObjPath
**
** Adds the request object path of an asynchronous command to an OperationResponse object
**
** \param   oper_resp - pointer to operation response object to add this entry to
** \param   path - path to asynchronous operation which was started
** \param   instance - request table instance number of async command that has just been started
** \param   err_msg - string representing the cause of the error
**
** \return  None
**
**************************************************************************/
void AddOperRes_ReqObjPath(Usp__OperateResp *oper_resp, char *path, int instance)
{
    Usp__OperateResp__OperationResult *oper_res;
    int new_num;    // new number of entries in the operation_result array
    char buf[MAX_DM_PATH];

    // Allocate memory to store the structures
    oper_res = USP_MALLOC(sizeof(Usp__OperateResp__OperationResult));
    usp__operate_resp__operation_result__init(oper_res);

    // Increase the size of the vector
    new_num = oper_resp->n_operation_results + 1;
    oper_resp->operation_results = USP_REALLOC(oper_resp->operation_results, new_num*sizeof(void *));
    oper_resp->n_operation_results = new_num;
    oper_resp->operation_results[new_num-1] = oper_res;

    // Initialise the operation result
    oper_res->executed_command = USP_STRDUP(path);
    oper_res->operation_resp_case = USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OBJ_PATH;
    USP_SNPRINTF(buf, sizeof(buf), "Device.LocalAgent.Request.%d", instance);
    oper_res->req_obj_path = USP_STRDUP(buf);
}

/*********************************************************************//**
**
** AddOperRes_ReqOutputArgs
**
** Adds the output argument response of a synchronous command to an OperationResponse object
**
** \param   oper_resp - pointer to operation response object to add this entry to
** \param   path - path to synchronous operation which succeeded
** \param   args - pointer to key-value vector containing output arguments (results) of the operation
**
** \return  None
**
**************************************************************************/
void AddOperRes_ReqOutputArgs(Usp__OperateResp *oper_resp, char *path, kv_vector_t *args)
{
    Usp__OperateResp__OperationResult *oper_res;
    Usp__OperateResp__OperationResult__OutputArgs *output_args;
    Usp__OperateResp__OperationResult__OutputArgs__OutputArgsEntry *entry;
    int new_num;    // new number of entries in the operation_result array
    int num_args;
    int i;
    kv_pair_t *arg;

    // Allocate memory to store the structures
    oper_res = USP_MALLOC(sizeof(Usp__OperateResp__OperationResult));
    usp__operate_resp__operation_result__init(oper_res);

    output_args = USP_MALLOC(sizeof(Usp__OperateResp__OperationResult__OutputArgs));
    usp__operate_resp__operation_result__output_args__init(output_args);

    // Increase the size of the vector
    new_num = oper_resp->n_operation_results + 1;
    oper_resp->operation_results = USP_REALLOC(oper_resp->operation_results, new_num*sizeof(void *));
    oper_resp->n_operation_results = new_num;
    oper_resp->operation_results[new_num-1] = oper_res;

    // Initialise the operation result
    oper_res->executed_command = USP_STRDUP(path);
    oper_res->operation_resp_case = USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS;
    oper_res->req_output_args = output_args;

    // Initialise the output args object
    num_args = args->num_entries;
    output_args->n_output_args = num_args;
    output_args->output_args = USP_MALLOC(num_args*sizeof(void *));

    // Iterate over all output arguments, adding them to the operate result object
    for (i=0; i<num_args; i++)
    {
        // Allocate an output arg map entry
        entry = USP_MALLOC(sizeof(Usp__OperateResp__OperationResult__OutputArgs__OutputArgsEntry));
        usp__operate_resp__operation_result__output_args__output_args_entry__init(entry);
        output_args->output_args[i] = entry;

        // Initialise the output arg map entry
        arg = &args->vector[i];
        entry->key = USP_STRDUP(arg->key);
        entry->value = USP_STRDUP(arg->value);
    }
}

