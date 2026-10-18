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
 * \file handle_add.c
 *
 * Handles the AddRequest message, creating an AddResponse
 *
 */

#include <stdio.h>
#include <protobuf-c/protobuf-c.h>

#include "usp-msg.pb-c.h"
#include "common_defs.h"
#include "msg_handler.h"
#include "dm_trans.h"
#include "dm_access.h"
#include "path_resolver.h"
#include "device.h"
#include "text_utils.h"
#include "group_add_vector.h"
#include "dm_inst_vector.h"


//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int ResolveObjectsToAdd(Usp__Add__CreateObject *cr, group_add_vector_t *gav, combined_role_t *combined_role);
Usp__Msg *CreateFullAddResp(char *msg_id, group_add_vector_t *gav);
Usp__Msg *CreateBasicAddResp(char *msg_id);
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *AddResp_OperSuccess(Usp__AddResp *add_resp, char *req_path, char *path);
Usp__AddResp__ParameterError *AddResp_OperSuccess_ParamErr(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path, int err_code, char *err_msg);
void AddOperSuccess_UniqueKeys(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success, kv_vector_t *kvv);
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *AddResp_OperFailure(Usp__AddResp *add_resp, char *path, int err_code, char *err_msg);
Usp__Msg *CreateAddResponseError(char *msg_id, group_add_entry_t *gae);
Usp__Msg *ProcessAdd_AllowPartialTrue(char *msg_id, Usp__Msg *usp, combined_role_t *combined_role);
Usp__Msg *ProcessAdd_AllowPartialFalse(char *msg_id, Usp__Msg *usp, combined_role_t *combined_role);
bool AreAllPathsTheSameGroupId(Usp__Add *add);
int CalcGroupIdForPath(char *path_expr);

/*********************************************************************//**
**
** MSG_HANDLER_HandleAdd
**
** Handles a USP Add message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleAdd(Usp__Msg *usp, char *controller_endpoint, mtp_conn_t *mtpc)
{
    Usp__Add *add;
    Usp__Msg *resp = NULL;
    combined_role_t combined_role;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_ADD) ||
        (usp->body->request->add == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_Create(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, USP_ERR_GetMessage());
        goto exit;
    }

    // Exit if there are no objects to create
    add = usp->body->request->add;
    if ((add->create_objs == NULL) || (add->n_create_objs == 0))
    {
        resp = CreateBasicAddResp(usp->header->msg_id);
        goto exit;
    }

    // Process differently, depending on whether allow_partial is set or not
    MSG_HANDLER_GetMsgRole(&combined_role);
    if (add->allow_partial == true)
    {
        resp = ProcessAdd_AllowPartialTrue(usp->header->msg_id, usp, &combined_role);
    }
    else
    {
        resp = ProcessAdd_AllowPartialFalse(usp->header->msg_id, usp, &combined_role);
    }

exit:
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mtpc);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** ProcessAdd_AllowPartialTrue
**
** Processes an Add request where AllowPartial is true. This means that a failure to create any object does not affect creation of any of the other objects
**
** \param   msg_id - string containing the message id of the USP message, which initiated this request
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   combined_role - roles to use when performing the add
**
** \return  Pointer to an AddResponse message
**
**************************************************************************/
Usp__Msg *ProcessAdd_AllowPartialTrue(char *msg_id, Usp__Msg *usp, combined_role_t *combined_role)
{
    int i, j;
    int err;
    int start_index;
    Usp__Add *add;
    Usp__Msg *resp = NULL;
    group_add_vector_t gav;
    group_add_entry_t *gae;
    dm_trans_vector_t trans;

    GROUP_ADD_VECTOR_Init(&gav);

    // Iterate over all path expressions, resolving them into the objects to add
    add = usp->body->request->add;
    for (i=0; i < add->n_create_objs; i++)
    {
        start_index = gav.num_entries;
        ResolveObjectsToAdd(add->create_objs[i], &gav, combined_role);

        // Add all objects that have been resolved for this path expression
        for (j=start_index; j < gav.num_entries; j++)
        {
            gae = &gav.vector[j];
            if (gae->err_code == USP_ERR_OK)        // Only attempt to add an object if the path resolution was successful
            {
                err = DM_TRANS_Start(&trans);
                if (err == USP_ERR_OK)
                {
                    // Create the specified object
                    err = GROUP_ADD_VECTOR_CreateObject(gae, combined_role);

                    // Commit or abort the transaction based on whether the object created successfully
                    if (err == USP_ERR_OK)
                    {
                        err = DM_TRANS_Commit();
                    }
                    else
                    {
                        DM_TRANS_Abort();
                    }
                }

                // Update group add vector if an error occurred creating this object (and no error has been saved yet for this object)
                if ((err != USP_ERR_OK) && (gae->err_msg == NULL))
                {
                    gae->err_code = err;
                    gae->err_msg = USP_STRDUP(USP_ERR_GetMessage());
                }
            }
        }

        // NOTE: Intentionally ignoring any errors, as allow_partial=true
    }

    // Create the add response from the results stored in the group add vector
    resp = CreateFullAddResp(usp->header->msg_id, &gav);

    GROUP_ADD_VECTOR_Destroy(&gav);
    return resp;
}

/*********************************************************************//**
**
** ProcessAdd_AllowPartialFalse
**
** Processes an Add request where AllowPartial is false. This means that failure to create any object results in USP error
**
** \param   msg_id - string containing the message id of the USP message, which initiated this request
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   combined_role - roles to use when performing the add
**
** \return  Pointer to an AddResponse message
**
**************************************************************************/
Usp__Msg *ProcessAdd_AllowPartialFalse(char *msg_id, Usp__Msg *usp, combined_role_t *combined_role)
{
    int i, j;
    int err;
    int start_index;
    int rollback_span;      // One more than the instance number of the objects in group add vector that have been created successfully
    Usp__Add *add;
    Usp__Msg *resp = NULL;
    group_add_vector_t gav;
    group_add_entry_t *gae;
    dm_trans_vector_t trans;

    GROUP_ADD_VECTOR_Init(&gav);
    add = usp->body->request->add;

    // Ensure all resolved paths are provided by the same data model provider as all the others
    // This is necessary because it is not possible to wind back the creation of objects across multiple providers
    if (AreAllPathsTheSameGroupId(add) == false)
    {
        USP_ERR_SetMessage("%s: Cannot process an add with allow_partial=false across multiple USP Services", __FUNCTION__);
        resp = ERROR_RESP_Create(usp->header->msg_id, USP_ERR_RESOURCES_EXCEEDED, USP_ERR_GetMessage());
        goto exit;
    }

    // Start a transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_Create(usp->header->msg_id, err, USP_ERR_GetMessage());
        goto exit;
    }

    // Iterate over all path expressions, resolving them into the objects to add
    rollback_span = 0;
    for (i=0; i < add->n_create_objs; i++)
    {
        // Exit if unable to resolve this path expression
        start_index = gav.num_entries;
        err = ResolveObjectsToAdd(add->create_objs[i], &gav, combined_role);
        if (err != USP_ERR_OK)
        {
            DM_TRANS_Abort();

            DM_TRANS_Start(&trans);
            GROUP_ADD_VECTOR_Rollback(&gav, rollback_span); // Must come after abort, as otherwise the changes it makes to instance cache would be rolled bacl
            DM_TRANS_Commit();

            resp = CreateAddResponseError(usp->header->msg_id, &gav.vector[gav.num_entries-1]);
            goto exit;
        }

        // Add all objects that have been resolved for this path expression
        for (j=start_index; j < gav.num_entries; j++)
        {
            // Exit if unable to create the specified object (since allow_partial=false)
            gae = &gav.vector[j];
            err = GROUP_ADD_VECTOR_CreateObject(gae, combined_role);
            if (err != USP_ERR_OK)
            {
                DM_TRANS_Abort();

                DM_TRANS_Start(&trans);
                GROUP_ADD_VECTOR_Rollback(&gav, rollback_span); // Must come after abort, as otherwise the changes it makes to instance cache would be rolled bacl
                DM_TRANS_Commit();

                resp = CreateAddResponseError(usp->header->msg_id, gae);
                goto exit;
            }
            rollback_span = j+1;
        }
    }

    // Commit transaction
    err = DM_TRANS_Commit();
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_Create(usp->header->msg_id, err, USP_ERR_GetMessage());
        goto exit;
    }

    resp = CreateFullAddResp(usp->header->msg_id, &gav);

exit:
    GROUP_ADD_VECTOR_Destroy(&gav);
    return resp;
}

/*********************************************************************//**
**
** ResolveObjectsToAdd
**
** Resolves a requested path into a set of object tables to add instances to
** and adds this list of objects to the group add vector, for the objects to be added later
**
** \param   gae - pointer to object to add
** \param   combined_role - roles to use when performing the add
** \param   allow_partial - set to true if failures in this object do not affect all others.
**                          If allow_partial is set then we perform a transaction at this level
**
** \return  USP_ERR_OK if successful (NOTE: This includes failing to create an object if allow_partial=true)
**
**************************************************************************/
int ResolveObjectsToAdd(Usp__Add__CreateObject *cr, group_add_vector_t *gav, combined_role_t *combined_role)
{
    int i, j;
    int err;
    str_vector_t res_paths;
    int_vector_t group_ids;
    Usp__Add__CreateParamSetting *cps;

    STR_VECTOR_Init(&res_paths);
    INT_VECTOR_Init(&group_ids);

    // Exit if no expression
    if ((cr->obj_path == NULL) || (cr->obj_path[0] == '\0'))
    {
        USP_ERR_SetMessage("%s: Expression missing in AddRequest", __FUNCTION__);
        err = USP_ERR_MESSAGE_NOT_UNDERSTOOD;
        goto exit;
    }

    // Exit if unable to resolve the path expression into a list of objects to add instances to
    err = PATH_RESOLVER_ResolveDevicePath(cr->obj_path, &res_paths, &group_ids, kResolveOp_Add, FULL_DEPTH, combined_role, 0);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if no objects were resolved
    if (res_paths.num_entries == 0)
    {
        USP_ERR_SetMessage("%s: Expression does not reference any objects", __FUNCTION__);
        err = USP_ERR_INVALID_ARGUMENTS;
        goto exit;
    }

    // Add all resolved objects to the group add vector
    USP_ASSERT(res_paths.num_entries == group_ids.num_entries);
    for (i=0; i<res_paths.num_entries; i++)
    {
        GROUP_ADD_VECTOR_AddObjectToCreate(gav, cr->obj_path, res_paths.vector[i], group_ids.vector[i]);
        for (j=0; j < cr->n_param_settings; j++)
        {
            cps = cr->param_settings[j];
            GROUP_ADD_VECTOR_AddParamSetting(gav, cps->param, cps->value, cps->required);
        }
    }

    err = USP_ERR_OK;

exit:
    // If an error occurred, add the requested path to the group add vector, but mark it as in error
    if (err != USP_ERR_OK)
    {
        GROUP_ADD_VECTOR_AddObjectNotCreated(gav, cr->obj_path, err, USP_ERR_GetMessage());
    }

    STR_VECTOR_Destroy(&res_paths);
    INT_VECTOR_Destroy(&group_ids);
    return err;
}

/*********************************************************************//**
**
** CreateFullAddResp
**
** Forms an AddResponse object using the results stored in the specified group add vector
**
** \param   msg_id - string containing the message id of the add request, which initiated this response
** \param   gav - pointer to group add vector containing the results of attempting to add the objects
**
** \return  Pointer to an AddResponse message
**
**************************************************************************/
Usp__Msg *CreateFullAddResp(char *msg_id, group_add_vector_t *gav)
{
    int i, j;
    Usp__Msg *resp;
    Usp__AddResp *add_resp;
    group_add_entry_t *gae;
    group_add_param_t *ps;
    char path[MAX_DM_PATH];
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success;

    // Create an Add Response
    // NOTE: All allow_partial=false error cases have already been dealt with
    resp = CreateBasicAddResp(msg_id);
    add_resp = resp->body->response->add_resp;

    for (i=0; i < gav->num_entries; i++)
    {
        gae = &gav->vector[i];
        if (gae->err_code == USP_ERR_OK)
        {
            // Add an oper_success for this entry
            USP_ASSERT(gae->instance > 0);
            USP_SNPRINTF(path, sizeof(path), "%s.%d", gae->res_path, gae->instance);
            oper_success = AddResp_OperSuccess(add_resp, gae->req_path, path);

            // Add param_errs for all non-required parameters which failed to be set
            for (j=0; j < gae->num_params; j++)
            {
                ps = &gae->params[j];
                if (ps->err_code != USP_ERR_OK)
                {
                    USP_ASSERT(ps->is_required == false);
                    AddResp_OperSuccess_ParamErr(oper_success, ps->param_name, ps->err_code, ps->err_msg);
                }
            }

            // Add unique keys
            if (gae->unique_keys.num_entries != 0)
            {
                AddOperSuccess_UniqueKeys(oper_success, &gae->unique_keys);
            }
        }
        else
        {
            // Add an oper_failure for this entry
            ps = GROUP_ADD_VECTOR_FindFirstFailedParam(gae);
            if (ps != NULL)
            {
                AddResp_OperFailure(add_resp, gae->req_path, ps->err_code, ps->err_msg);
            }
            else
            {
                AddResp_OperFailure(add_resp, gae->req_path, gae->err_code, gae->err_msg);
            }
        }
    }

    return resp;
}

/*********************************************************************//**
**
** CreateBasicAddResp
**
** Dynamically creates an AddResponse object
** NOTE: The object is created without any created_obj_results
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the add request, which initiated this response
**
** \return  Pointer to an AddResponse object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateBasicAddResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__AddResp *add_resp;

    // Create Add Response
    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__ADD_RESP, USP__RESPONSE__RESP_TYPE_ADD_RESP);
    add_resp = USP_MALLOC(sizeof(Usp__AddResp));
    usp__add_resp__init(add_resp);
    msg->body->response->add_resp = add_resp;

    // Start from an empty list
    add_resp->n_created_obj_results = 0;
    add_resp->created_obj_results = NULL;

    return msg;
}

/*********************************************************************//**
**
** AddResp_OperSuccess
**
** Dynamically adds an operation success object to the AddResponse object
** NOTE: The object is created without any param_err or unique_keys entries
**
** \param   add_resp - pointer to AddResponse object
** \param   req_path - requested path
** \param   path - instantiated path
**
** \return  Pointer to dynamically allocated operation success object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *
AddResp_OperSuccess(Usp__AddResp *add_resp, char *req_path, char *path)
{
    Usp__AddResp__CreatedObjectResult *created_obj_res;
    Usp__AddResp__CreatedObjectResult__OperationStatus *oper_status;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success;
    int new_num;    // new number of entries in the created object result array

    // Allocate memory to store the created object result
    created_obj_res = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult));
    usp__add_resp__created_object_result__init(created_obj_res);

    oper_status = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult__OperationStatus));
    usp__add_resp__created_object_result__operation_status__init(oper_status);

    oper_success = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess));
    usp__add_resp__created_object_result__operation_status__operation_success__init(oper_success);

    // Increase the size of the vector
    new_num = add_resp->n_created_obj_results + 1;
    add_resp->created_obj_results = USP_REALLOC(add_resp->created_obj_results, new_num*sizeof(void *));
    add_resp->n_created_obj_results = new_num;
    add_resp->created_obj_results[new_num-1] = created_obj_res;

    // Connect all objects together, and fill in their members
    created_obj_res->requested_path = USP_STRDUP(req_path);
    created_obj_res->oper_status = oper_status;

    oper_status->oper_status_case = USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS;
    oper_status->oper_success = oper_success;

    oper_success->n_param_errs = 0;


    oper_success->instantiated_path = TEXT_UTILS_StrDupWithTrailingDot(path);
    oper_success->param_errs = NULL;             // Start from an empty list
    oper_success->n_param_errs = 0;


    oper_success->unique_keys = NULL;   // Start from an empty list
    oper_success->n_unique_keys = 0;

    return oper_success;
}

/*********************************************************************//**
**
** AddResp_OperSuccess_ParamErr
**
** Dynamically adds a parameter_error entry to an OperationSuccess object
**
** \param   oper_success - pointer to operation success object to add this entry to
** \param   path - name of parameter which failed to create
** \param   err_code - error code representing the cause of the failure to create
** \param   err_msg - string representing the cause of the error
**
** \return  Pointer to dynamically allocated parameter_error entry
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__AddResp__ParameterError *
AddResp_OperSuccess_ParamErr(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success,
                             char *path, int err_code, char *err_msg)
{
    Usp__AddResp__ParameterError *param_err_entry;
    int new_num;    // new number of entries in the param_err array

    // Allocate memory to store the param_err entry
    param_err_entry = USP_MALLOC(sizeof(Usp__AddResp__ParameterError));
    usp__add_resp__parameter_error__init(param_err_entry);

    // Increase the size of the vector
    new_num = oper_success->n_param_errs + 1;
    oper_success->param_errs = USP_REALLOC(oper_success->param_errs, new_num*sizeof(void *));
    oper_success->n_param_errs = new_num;
    oper_success->param_errs[new_num-1] = param_err_entry;

    // Initialise the param_err_entry
    param_err_entry->param = USP_STRDUP(path);
    param_err_entry->err_code = err_code;
    param_err_entry->err_msg = USP_STRDUP(err_msg);

    return param_err_entry;
}

/*********************************************************************//**
**
** AddOperSuccess_UniqueKeys
**
** Moves the specified unique keys to an OperationSuccess object, destroying
** the key-value vector in the process (this is done to prevent unnecessary mallocs)
**
** \param   oper_success - pointer to oper success object to add this unique key map to
** \param   kvv - pointer to key-value vector containing the unique key map
**
** \return  None
**
**************************************************************************/
void AddOperSuccess_UniqueKeys(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success, kv_vector_t *kvv)
{
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry *entry;
    kv_pair_t *kv;
    int i;

    // Allocate the unique key map vector
    oper_success->n_unique_keys = kvv->num_entries;
    oper_success->unique_keys = USP_MALLOC(kvv->num_entries*sizeof(void *));

    // Add all unique keys to the unique key map
    for (i=0; i < kvv->num_entries; i++)
    {
        // Allocate memory to store the map entry
        entry = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry));
        usp__add_resp__created_object_result__operation_status__operation_success__unique_keys_entry__init(entry);
        oper_success->unique_keys[i] = entry;

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
** AddResp_OperFailure
**
** Dynamically adds an operation failure object to the AddResponse object
**
** \param   resp - pointer to AddResponse object
** \param   path - requested path of object which failed to create
** \param   err_code - numeric code indicating reason object failed to be created
** \param   err_msg - error message indicating reason object failed to be created
**
** \return  Pointer to dynamically allocated operation failure object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *
AddResp_OperFailure(Usp__AddResp *add_resp, char *path, int err_code, char *err_msg)
{
    Usp__AddResp__CreatedObjectResult *created_obj_res;
    Usp__AddResp__CreatedObjectResult__OperationStatus *oper_status;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *oper_failure;
    int new_num;    // new number of entries in the created object result array

    // Allocate memory to store the created object result
    created_obj_res = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult));
    usp__add_resp__created_object_result__init(created_obj_res);

    oper_status = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult__OperationStatus));
    usp__add_resp__created_object_result__operation_status__init(oper_status);

    oper_failure = USP_MALLOC(sizeof(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure));
    usp__add_resp__created_object_result__operation_status__operation_failure__init(oper_failure);

    // Increase the size of the vector
    new_num = add_resp->n_created_obj_results + 1;
    add_resp->created_obj_results = USP_REALLOC(add_resp->created_obj_results, new_num*sizeof(void *));
    add_resp->n_created_obj_results = new_num;
    add_resp->created_obj_results[new_num-1] = created_obj_res;

    // Connect all objects together, and fill in their members
    created_obj_res->requested_path = USP_STRDUP(path);
    created_obj_res->oper_status = oper_status;

    oper_status->oper_status_case = USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE;
    oper_status->oper_failure = oper_failure;

    oper_failure->err_code = err_code;
    oper_failure->err_msg = USP_STRDUP(err_msg);

    return oper_failure;
}

/*********************************************************************//**
**
** CreateAddResponseError
**
** Creates an ERROR response for the specified object which failed to create
**
** \param   msg_id - string containing the message id of the request, which initiated this response
** \param   gae - pointer to object which failed to create
**
** \return  Pointer to a USP Error response message object
**
**************************************************************************/
Usp__Msg *CreateAddResponseError(char *msg_id, group_add_entry_t *gae)
{
    Usp__Msg *resp;
    int outer_err;
    char path[MAX_DM_PATH];
    group_add_param_t *ps;

    // Determine whether the cause of failure was at the param or object level
    ps = GROUP_ADD_VECTOR_FindFirstFailedParam(gae);
    if (ps != NULL)
    {
        // Cause of failure was at param level
        outer_err = ERROR_RESP_CalcOuterErrCode(ps->err_code);
        resp = ERROR_RESP_Create(msg_id, outer_err, ps->err_msg);
        USP_SNPRINTF(path, sizeof(path), "%s.{i}.%s", gae->res_path, ps->param_name);
        ERROR_RESP_AddParamError(resp, path, ps->err_code, ps->err_msg);
    }
    else
    {
        // Cause of failure was at object level
        outer_err = ERROR_RESP_CalcOuterErrCode(gae->err_code);
        resp = ERROR_RESP_Create(msg_id, outer_err, gae->err_msg);
        ERROR_RESP_AddParamError(resp, gae->res_path, gae->err_code, gae->err_msg);
    }

    return resp;
}

/*********************************************************************//**
**
** AreAllPathsTheSameGroupId
**
** Determines whether all of the path expressions in an Add request are targetted at the same data model provider component
** This test is necessary for allow_partial=false, because if any object fails to create
** it is not possible to wind back previous objects that have been created successfully in other data model providers
** NOTE: Path expressions containing errors or reference following are ignored by this function as they cannot be handled here
**       (Unfortunately this could result in being unable to undo an object creation in these cases)
**
** \param   add - pointer to parsed Add Request object
**
** \return  true if all path expressions reference the same USP Service (or reference the Broker's internal data model)
**
**************************************************************************/
bool AreAllPathsTheSameGroupId(Usp__Add *add)
{
    int i;
    char *path;
    int group_id;
    int first_group_id = NON_GROUPED;

    for (i=0; i < add->n_create_objs; i++)
    {
        path = add->create_objs[i]->obj_path;
        if ((path != NULL) && (*path != '\0'))
        {
            group_id = CalcGroupIdForPath(path);
            if (group_id != NON_GROUPED)
            {
                if (first_group_id == NON_GROUPED)
                {
                    first_group_id = group_id;
                }
                else
                {
                    if (group_id != first_group_id)
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

/*********************************************************************//**
**
** CalcGroupIdForPath
**
** Determines the GroupId for the specified path expression
** NOTE: If the path expression contains reference following outside of square barckets (ie outside of a search expression)
**       then the group_id cannot be determined and NON_GROUPED will be returned
**
** \param   path_expr - path expression
**
** \return  group_id of the path, or NON_GROUPED if unable to determine it, or if the path is owned by our internal data model
**
**************************************************************************/
int CalcGroupIdForPath(char *path_expr)
{
    dm_node_t *node;
    char buf[MAX_DM_PATH];
    char *src;
    char *dst;
    int dst_remaining;

    // Copy the path expression into buf, replacing any search expressions within square brackets with a wildcard
    src = path_expr;
    dst = buf;
    dst_remaining = sizeof(buf) - 1;
    while ((*src != '\0') && (dst_remaining > 0))
    {
        if (*src == '[')
        {
            // Replace characters within square braces with an asterisk
            *dst++ = '*';
            dst_remaining--;

            // Skip past the closing square brace
            src = strchr(src, ']');
            if (src == NULL)
            {
                return NON_GROUPED;  // No closing brace found - path expression in error
            }
            src++;
        }
        else if (*src == '+')
        {
            // Unable to cope with reference following
            return NON_GROUPED;
        }
        else
        {
            *dst++ = *src++;
            dst_remaining--;
        }
    }
    *dst = '\0';        // Terminate the path in buf

    // Exit if unable to determine which node in the data model this references
    node = DM_PRIV_GetNodeFromPath(buf, NULL, NULL, DONT_LOG_ERRORS);
    if (node == NULL)
    {
        return NON_GROUPED;
    }

    return node->group_id;
}

