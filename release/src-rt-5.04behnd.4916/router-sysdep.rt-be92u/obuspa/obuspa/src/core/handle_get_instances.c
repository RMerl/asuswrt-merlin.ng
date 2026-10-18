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
 * \file handle_get_instances.c
 *
 * Handles the GetInstances message, creating a GetInstancesResponse
 *
 */

#include <stdlib.h>
#include <string.h>
#include <protobuf-c/protobuf-c.h>

#include "common_defs.h"
#include "usp-msg.pb-c.h"
#include "msg_handler.h"
#include "data_model.h"
#include "dm_access.h"
#include "path_resolver.h"
#include "device.h"
#include "text_utils.h"

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ProcessRequestedPathInstances(char *requested_path, bool first_level_only, Usp__GetInstancesResp *gi_resp);
Usp__Msg *CreateGetInstancesResp(char *msg_id);
Usp__GetInstancesResp__RequestedPathResult *AddGetInstances_RequestedPathResult(Usp__GetInstancesResp *gi_resp, char *requested_path, int err, char *err_msg);
void AddRequestedPathResult_CurrInstance(Usp__GetInstancesResp__RequestedPathResult *req_path_res, char *path, kv_vector_t *unique_keys);
void RemoveAddResp_LastRequestedPathResult(Usp__GetInstancesResp *gi_resp);

/*********************************************************************//**
**
** MSG_HANDLER_HandleGetInstances
**
** Handles a USP GetInstances message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleGetInstances(Usp__Msg *usp, char *controller_endpoint, mtp_conn_t *mtpc)
{
    Usp__Msg *resp = NULL;
    int i;
    Usp__GetInstances *gi;
    Usp__GetInstancesResp *gi_resp;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_GET_INSTANCES) ||
        (usp->body->request->get_instances == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }

    // Create a GetInstances Response message
    resp = CreateGetInstancesResp(usp->header->msg_id);
    gi_resp = resp->body->response->get_instances_resp;

    // Iterate over all Object paths in the request message
    gi = usp->body->request->get_instances;
    for (i=0; i < gi->n_obj_paths; i++)
    {
        ProcessRequestedPathInstances(gi->obj_paths[i], gi->first_level_only, gi_resp);
    }

exit:
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mtpc);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}


/*********************************************************************//**
**
** Processes each requested path
**
** \param   requested_path - path to resolve representing the objects we want to get the instances of
** \param   first_level_only - if true, return only the instances of the objects identified in the requested path
**                             if false, return (recursively) all nested multi-instance objects
** \param   gi_resp - pointer to GetInstancesResponse object of USP response message
**
** \return  None - This code must handle any errors by reporting errors in the response message
**
**************************************************************************/
void ProcessRequestedPathInstances(char *requested_path, bool first_level_only, Usp__GetInstancesResp *gi_resp)
{
    int i;
    int err;
    combined_role_t combined_role;
    str_vector_t obj_paths;
    char *path;
    kv_vector_t unique_keys;
    Usp__GetInstancesResp__RequestedPathResult *req_path_res;
    unsigned flags;

    // Initialise variables used in this function
    MSG_HANDLER_GetMsgRole(&combined_role);
    STR_VECTOR_Init(&obj_paths);
    KV_VECTOR_Init(&unique_keys);

    // Exit if unable to resolve the requested path
    flags = (first_level_only==false) ? GET_ALL_INSTANCES : 0;
    err = PATH_RESOLVER_ResolveDevicePath(requested_path, &obj_paths, NULL, kResolveOp_Instances, FULL_DEPTH, &combined_role, flags);
    if (err != USP_ERR_OK)
    {
        AddGetInstances_RequestedPathResult(gi_resp, requested_path, err, USP_ERR_GetMessage());
        goto exit;
    }

    // Sort the instances
#ifndef DONT_SORT_GET_INSTANCES
    STR_VECTOR_Sort(&obj_paths);
#endif

    // Iterate over all resolved objects, obtaining their unique keys and adding to the LastRequestedPathResult
    req_path_res = AddGetInstances_RequestedPathResult(gi_resp, requested_path, USP_ERR_OK, "");
    for (i=0; i < obj_paths.num_entries; i++)
    {
        // Exit if unable to obtain the unique keys for this object
        KV_VECTOR_Init(&unique_keys);  // not strictly necessary
        path = obj_paths.vector[i];
        err = DATA_MODEL_GetUniqueKeyParams(path, &unique_keys, &combined_role);
        if (err != USP_ERR_OK)
        {
            // Remove all resolved instances and their unique keys, and replace with an error message
            RemoveAddResp_LastRequestedPathResult(gi_resp);
            AddGetInstances_RequestedPathResult(gi_resp, requested_path, err, USP_ERR_GetMessage());
            goto exit;
        }

        // Add this instance to the requested path result
        AddRequestedPathResult_CurrInstance(req_path_res, path, &unique_keys);
        KV_VECTOR_Destroy(&unique_keys);
    }

exit:
    STR_VECTOR_Destroy(&obj_paths);
    KV_VECTOR_Destroy(&unique_keys);
}

/*********************************************************************//**
**
** CreateGetInstancesResp
**
** Dynamically creates an GetInstancesResponse object
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the request, which initiated this response
**
** \return  Pointer to a GetInstances Response object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateGetInstancesResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__GetInstancesResp *get_inst_resp;

    // Create GetInstances Response
    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP, USP__RESPONSE__RESP_TYPE_GET_INSTANCES_RESP);
    get_inst_resp = USP_MALLOC(sizeof(Usp__GetInstancesResp));
    usp__get_instances_resp__init(get_inst_resp);
    msg->body->response->get_instances_resp = get_inst_resp;

    return msg;
}

/*********************************************************************//**
**
** AddGetInstances_RequestedPathResult
**
** Dynamically adds a RequestedPathResult object to a GetInstancesResponse object
**
** \param   gi_resp - pointer to GetInstancesResponse object of USP response message
** \param   requested_path - path identifying the set of object instances that are to be addressed
** \param   err - USP error for this object. If no error, then USP_ERR_OK
** \param   err_msg - pointer to error message. If no err, then an empty string.
**
** \return  Pointer to a RequestedPathResult object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__GetInstancesResp__RequestedPathResult *
AddGetInstances_RequestedPathResult(Usp__GetInstancesResp *gi_resp, char *requested_path, int err, char *err_msg)
{
    Usp__GetInstancesResp__RequestedPathResult *req_path_res;
    int new_num;    // new number of entries in the requested path result array

    // Allocate memory to store the RequestedPathResult object
    req_path_res = USP_MALLOC(sizeof(Usp__GetInstancesResp__RequestedPathResult));
    usp__get_instances_resp__requested_path_result__init(req_path_res);

    // Increase the size of the vector
    new_num = gi_resp->n_req_path_results + 1;
    gi_resp->req_path_results = USP_REALLOC(gi_resp->req_path_results, new_num*sizeof(void *));
    gi_resp->n_req_path_results = new_num;
    gi_resp->req_path_results[new_num-1] = req_path_res;

    // Fill in the RequestedPathResult object
    req_path_res->requested_path = USP_STRDUP(requested_path);
    req_path_res->err_code = err;
    req_path_res->err_msg = USP_STRDUP(err_msg);

    return req_path_res;
}

/*********************************************************************//**
**
** AddRequestedPathResult_CurrInstance
**
** Dynamically adds a CurrInstance object to a RequestedPathResult object
**
** \param   req_path_res - pointer to RequestedPathResult object
** \param   path - path identifying an instance of an object
** \param   unique_keys - key-value vecator containing the names and values of all unique keys for the specified object
**
** \return  None
**
**************************************************************************/
void AddRequestedPathResult_CurrInstance(Usp__GetInstancesResp__RequestedPathResult *req_path_res, char *path, kv_vector_t *unique_keys)
{
    int i;
    Usp__GetInstancesResp__CurrInstance *cur_inst;
    Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry *unique_key;
    int new_num;    // new number of entries in the curr_insts array
    kv_pair_t *kv;
    int len;

    // Allocate memory to store the CurrInstance object
    cur_inst = USP_MALLOC(sizeof(Usp__GetInstancesResp__CurrInstance));
    usp__get_instances_resp__curr_instance__init(cur_inst);

    // Increase the size of the vector
    new_num = req_path_res->n_curr_insts + 1;
    req_path_res->curr_insts = USP_REALLOC(req_path_res->curr_insts, new_num*sizeof(void *));
    req_path_res->n_curr_insts = new_num;
    req_path_res->curr_insts[new_num-1] = cur_inst;

    // Fill in the CurrInst object
    cur_inst->n_unique_keys = unique_keys->num_entries;
    cur_inst->unique_keys = USP_MALLOC(unique_keys->num_entries*sizeof(void *));

    // Copy the instantiated path, adding a trailing '.' to the end of it
    len = strlen(path);
    cur_inst->instantiated_obj_path = USP_MALLOC(len+2);   // Plus 2 to include trailing '.' and NULL terminator
    memcpy(cur_inst->instantiated_obj_path, path, len);
    cur_inst->instantiated_obj_path[len] = '.';
    cur_inst->instantiated_obj_path[len+1] = '\0';

    // Fill in the UniqueKeys array
    for (i=0; i < unique_keys->num_entries; i++)
    {
        // Allocate memory to store this unique key object
        unique_key = USP_MALLOC(sizeof(Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry));
        usp__get_instances_resp__curr_instance__unique_keys_entry__init(unique_key);

        // Fill in this unique key
        kv = &unique_keys->vector[i];
        unique_key->key = USP_STRDUP(kv->key);
        unique_key->value = USP_STRDUP(kv->value);

        // Attach this unique key in the unique_keys array (of the CurrInstance object)
        cur_inst->unique_keys[i] = unique_key;
    }
}

/*********************************************************************//**
**
** RemoveAddResp_LastRequestedPathResult
**
** Removes the last RequestedPathResult object from the GetInstancesResponse object
**
** \param   gi_resp - pointer to GetInstancesResponse object of USP response message
**
** \return  None
**
**************************************************************************/
void RemoveAddResp_LastRequestedPathResult(Usp__GetInstancesResp *gi_resp)
{
    int index;
    Usp__GetInstancesResp__RequestedPathResult *req_path_res;

    // Free the memory associated with the last created obj_result
    index = gi_resp->n_req_path_results - 1;
    req_path_res = gi_resp->req_path_results[index];
    protobuf_c_message_free_unpacked ((ProtobufCMessage*)req_path_res, pbuf_allocator);

    // Fix the GetInstancesResponse object, so that it does not reference the RequestedPathResult we have just removed
    gi_resp->req_path_results[index] = NULL;
    gi_resp->n_req_path_results--;
}


