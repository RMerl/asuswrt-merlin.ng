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
 * \file handle_delete.c
 *
 * Handles the DeleteRequest message, creating a DeleteResponse
 *
 */

#include <stdio.h>
#include <string.h>
#include <protobuf-c/protobuf-c.h>

#include "usp-msg.pb-c.h"
#include "common_defs.h"
#include "msg_handler.h"
#include "dm_trans.h"
#include "path_resolver.h"
#include "device.h"
#include "text_utils.h"
#include "group_del_vector.h"


//------------------------------------------------------------------------------
// Structure used to marshall entries in group del vector for a path expression
// This structure is equivalent to the deleted_obj_results object in the USP Set Response message
typedef struct
{
    char *req_path;     // Path expression string (owned by USP Delete Request message).
    int index;          // Start index of objects in group del vector for this requested path
    int num_objects;    // Number of objects in group del vector index that the path expression resolved to

    int err_code;       // error code if the requested path failed to resolve
    char *err_msg;      // textual cause of error if the requested path failed to resolve
} del_expr_info_t;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
Usp__Msg *ProcessDel_AllowPartialTrue(char *msg_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv, combined_role_t *combined_role);
Usp__Msg *ProcessDel_AllowPartialFalse(char *msg_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv, combined_role_t *combined_role);
Usp__Msg *CreateFullDeleteResp(char *msg_id, del_expr_info_t *del_expr_info, int num_del_expr_info, group_del_vector_t *gdv);
Usp__Msg *CreateBasicDeleteResp(char *msg_id);
Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *AddDeleteResp_OperSuccess(Usp__DeleteResp *del_resp, char *path);
void AddOperSuccess_AffectedPath(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path);
void AddOperSuccess_UnaffectedPathError(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path, uint32_t err_code, char *err_msg);
Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *AddDeleteResp_OperFailure(Usp__DeleteResp *del_resp, char *path, uint32_t err_code, char *err_msg);
int DeleteAllInstancesProvidedByGroupId(int group_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv, char **failed_path);

/*********************************************************************//**
**
** MSG_HANDLER_HandleDelete
**
** Handles a USP Delete message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleDelete(Usp__Msg *usp, char *controller_endpoint, mtp_conn_t *mtpc)
{
    int i;
    int err;
    Usp__Delete *del;
    Usp__Msg *resp = NULL;
    str_vector_t obj_paths;
    int_vector_t group_ids;
    combined_role_t combined_role;
    char *exp_path;
    group_del_vector_t gdv;
    del_expr_info_t *del_expr_info = NULL;
    del_expr_info_t *di;
    int num_del_expr = 0;

    GROUP_DEL_VECTOR_Init(&gdv);

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_DELETE) ||
        (usp->body->request->delete_ == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }

    // Exit if there are no objects to delete, sending back an empty delete response
    del = usp->body->request->delete_;
    if ((del->obj_paths == NULL) || (del->n_obj_paths == 0))
    {
        resp = CreateBasicDeleteResp(usp->header->msg_id);
        goto exit;
    }

    DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths();

    // Iterate over all paths in the message, resolving them into a set of objects to delete
    num_del_expr = del->n_obj_paths;
    del_expr_info = USP_MALLOC(num_del_expr*sizeof(del_expr_info_t));
    MSG_HANDLER_GetMsgRole(&combined_role);
    for (i=0; i < num_del_expr; i++)
    {
        exp_path = del->obj_paths[i];
        di = &del_expr_info[i];
        di->req_path = exp_path;
        di->index = gdv.num_entries;

        STR_VECTOR_Init(&obj_paths);
        INT_VECTOR_Init(&group_ids);
        err = PATH_RESOLVER_ResolveDevicePath(exp_path, &obj_paths, &group_ids, kResolveOp_Del, FULL_DEPTH, &combined_role, 0);
        if (err == USP_ERR_OK)
        {
            // NOTE: Path resolution may result in no objects to delete, if the requested object has already been deleted
            GROUP_DEL_VECTOR_AddObjectsToDelete(&gdv, &obj_paths, &group_ids);
            di->num_objects = obj_paths.num_entries;
            di->err_code = USP_ERR_OK;
            di->err_msg = NULL;
        }
        else
        {
            di->num_objects = 0;
            di->err_code = err;
            di->err_msg = USP_STRDUP(USP_ERR_GetMessage());
        }

        STR_VECTOR_Destroy(&obj_paths);
        INT_VECTOR_Destroy(&group_ids);
    }

    // Process differently, depending on whether allow_partial is set or not
    if (del->allow_partial == true)
    {
        resp = ProcessDel_AllowPartialTrue(usp->header->msg_id, del_expr_info, num_del_expr, &gdv, &combined_role);
    }
    else
    {
        resp = ProcessDel_AllowPartialFalse(usp->header->msg_id, del_expr_info, num_del_expr, &gdv, &combined_role);
    }

exit:
    // Free del_exp_info vector
    for (i=0; i<num_del_expr; i++)
    {
        di = &del_expr_info[i];
        USP_SAFE_FREE(di->err_msg);
        // NOTE: No need to free req_path, as ownership of it stays with the Delete request message
    }
    USP_SAFE_FREE(del_expr_info);

    GROUP_DEL_VECTOR_Destroy(&gdv);
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mtpc);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** ProcessDel_AllowPartialTrue
**
** Processes a Delete request where AllowPartial is true. This means that failure to delete any object does not affect deletion of any other object
**
** \param   msg_id - string containing the message id of the USP message, which initiated this response
** \param   del_expr_info - pointer to array tying the resolved objects to delete in group del vector, back to the requested paths to delete in the delete request message
** \param   num_del_expr - number of entries in del_expr_info
** \param   gdv - pointer to group del vector containing the objects to delete
** \param   combined_role - roles to use when performing the delete
**
** \return  Pointer to an AddResponse message
**
**************************************************************************/
Usp__Msg *ProcessDel_AllowPartialTrue(char *msg_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv, combined_role_t *combined_role)
{
    int i;
    int err;
    dm_trans_vector_t trans;
    group_del_entry_t *gde;
    Usp__Msg *resp;

    // Iterate over all objects to delete
    for (i=0; i < gdv->num_entries; i++)
    {
        gde = &gdv->vector[i];
        err = DM_TRANS_Start(&trans);
        if (err == USP_ERR_OK)
        {
            // Delete the specified object
            err = DATA_MODEL_DeleteInstance(gde->path, CHECK_DELETABLE);

            // Commit or abort the transaction based on whether the object deleted successfully
            if (err == USP_ERR_OK)
            {
                err = DM_TRANS_Commit();
            }
            else
            {
                DM_TRANS_Abort();
            }
        }

        // Update group del vector if an error occurred deleting this object (and no error has been saved yet for this object)
        if (err != USP_ERR_OK)
        {
            gde->err_code = err;
            gde->err_msg = USP_STRDUP(USP_ERR_GetMessage());
        }
    }

    // Create the delete response from the results stored in the group del vector
    resp = CreateFullDeleteResp(msg_id, del_expr_info, num_del_expr, gdv);

    return resp;
}

/*********************************************************************//**
**
** ProcessDel_AllowPartialFalse
**
** Processes a Delete request where AllowPartial is false. This means that failure to delete any object results in USP error
**
** \param   msg_id - string containing the message id of the USP message, which initiated this request
** \param   del_expr_info - pointer to array tying the resolved objects to delete in group del vector, back to the requested paths to delete in the delete request message
** \param   num_del_expr - number of entries in del_expr_info
** \param   gdv - pointer to group del vector containing the objects to delete
** \param   combined_role - roles to use when performing the add
**
** \return  Pointer to an AddResponse message
**
**************************************************************************/
Usp__Msg *ProcessDel_AllowPartialFalse(char *msg_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv, combined_role_t *combined_role)
{
    int i, j;
    int err = USP_ERR_OK;
    Usp__Msg *resp = NULL;
    group_del_entry_t *gde;
    dm_trans_vector_t trans;
    del_expr_info_t *di;
    int outer_err;
    int group_id;
    char *failed_path = NULL;

    // Exit if any of the paths failed to resolve
    for (i=0; i < num_del_expr; i++)
    {
        di = &del_expr_info[i];
        if (di->err_code != USP_ERR_OK)
        {
            outer_err = ERROR_RESP_CalcOuterErrCode(di->err_code);
            resp = ERROR_RESP_Create(msg_id, outer_err, USP_ERR_GetMessage());
            ERROR_RESP_AddParamError(resp, di->req_path, di->err_code, USP_ERR_GetMessage());
            goto exit;
        }
    }

    // Ensure all resolved paths are provided by the same data model provider as all the others
    // This is necessary because it is not possible to wind back the deletion of objects across multiple providers
    if (GROUP_DEL_VECTOR_AreAllPathsTheSameGroupId(gdv, &group_id) == false)
    {
        USP_ERR_SetMessage("%s: Cannot process a delete with allow_partial=false across multiple USP Services", __FUNCTION__);
        resp = ERROR_RESP_Create(msg_id, USP_ERR_RESOURCES_EXCEEDED, USP_ERR_GetMessage());
        goto exit;
    }

    // Exit if unable to start a transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_Create(msg_id, err, USP_ERR_GetMessage());
        goto exit;
    }

    // Delete all instances provided by the internal data model of this executable
    for (i=0; i < num_del_expr; i++)
    {
        di = &del_expr_info[i];
        for (j=di->index; j < di->index + di->num_objects; j++)
        {
            gde = &gdv->vector[j];
            if (gde->group_id == NON_GROUPED)
            {
                // Exit if unable to delete any of the specified objects
                err = DATA_MODEL_DeleteInstance(gde->path, CHECK_DELETABLE);
                if (err != USP_ERR_OK)
                {
                    failed_path = di->req_path;
                    goto exit;
                }
            }
        }
    }

    // Delete all instances owned by the data model provider component (if a data model provider component was involved in this delete)
    if (group_id != NON_GROUPED)
    {
        err = DeleteAllInstancesProvidedByGroupId(group_id, del_expr_info, num_del_expr, gdv, &failed_path);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

    // Commit transaction for all deleted instances provided by the internal data model
    err = DM_TRANS_Commit();
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_Create(msg_id, err, USP_ERR_GetMessage());
        goto exit;
    }

    // Create the delete response from the results stored in the group del vector
    resp = CreateFullDeleteResp(msg_id, del_expr_info, num_del_expr, gdv);

exit:
    // If no response has been created yet, then this must be because an error occurred
    // So create an error response and rollback the database transaction
    if (resp == NULL)
    {
        USP_ASSERT(err != USP_ERR_OK);
        USP_ASSERT(failed_path != NULL);
        outer_err = ERROR_RESP_CalcOuterErrCode(err);
        resp = ERROR_RESP_Create(msg_id, outer_err, USP_ERR_GetMessage());
        ERROR_RESP_AddParamError(resp, failed_path, err, USP_ERR_GetMessage());
        DM_TRANS_Abort();
    }

    return resp;
}

/*********************************************************************//**
**
** DeleteAllInstancesProvidedByGroupId
**
** Deletes all objects in the group delete vector provided by the specified data model provider component
** NOTE: This function is only called when allow_partial=false, and it aims to delete all objects using a single atomic multi-delete
**
** \param   group_id - identifies the data model provider component, which we want to delete objects from
** \param   del_expr_info - pointer to array tying the resolved objects to delete in group del vector, back to the requested paths to delete in the delete request message
** \param   num_del_expr - number of entries in del_expr_info
** \param   gdv - pointer to group del vector containing the objects to delete
** \param   failed_path - pointer to variable in which to return a pointer to the first path that failed to delete
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DeleteAllInstancesProvidedByGroupId(int group_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv, char **failed_path)
{
    int i, j;
    int err;
    group_del_entry_t *gde;
    del_expr_info_t *di;
    dm_multi_del_cb_t   multi_del_cb;
    int index = INVALID;
    char **paths;
    int num_paths;
    int count;

    // Get the multi-delete vendor hook
    USP_ASSERT(group_id < MAX_VENDOR_PARAM_GROUPS);
    multi_del_cb = group_vendor_hooks[group_id].multi_del_cb;

    // If no multi delete vendor hook was registered, then fallback to deleting the instances individually
    // NOTE: This has the disadvantage that the delete is not atomic - if any instance fails to delete, we cannot add back any instances which were deleted successfully
    // Also it can lead to instance cache mismatch, as the failure will rollback the instance cache, but instances have been deleted on the data model provider component.
    // (this is avoided by setting the instance cache expiry time to -1 for USP Services)
    if (multi_del_cb == NULL)
    {
        for (i=0; i < num_del_expr; i++)
        {
            di = &del_expr_info[i];
            for (j=di->index; j < di->index + di->num_objects; j++)
            {
                gde = &gdv->vector[j];
                if (gde->group_id == group_id)
                {
                    // Exit if unable to delete any of the specified objects
                    err = DATA_MODEL_DeleteInstance(gde->path, CHECK_DELETABLE);
                    if (err != USP_ERR_OK)
                    {
                        *failed_path = di->req_path;
                        goto exit;
                    }
                }
            }
        }
        err = USP_ERR_OK;
        goto exit;
    }

    // If the code gets here, then a multi-delete vendor hook was registered
    // Form the array of paths to delete for the specified data model provider component
    paths = USP_MALLOC(gdv->num_entries*sizeof(char *));  // NOTE: This array is larger than strictly required, if some of the objects in the group delete vector were in the local data model
    num_paths = 0;
    for (i=0; i < gdv->num_entries; i++)
    {
        gde = &gdv->vector[i];
        if (gde->group_id == group_id)
        {
            paths[num_paths] = gde->path;
            num_paths++;
        }
    }

    // Attempt to delete all paths using a single multi-delete IPC operation
    err = multi_del_cb(group_id, false, paths, num_paths, &index);
    USP_FREE(paths);        // NOTE: No need to delete the entries in the paths array, as they are still owned by group delete vector

    // Exit if an error occurred, determining the first path which failed to delete
    if (err != USP_ERR_OK)
    {
        if (index != INVALID)
        {
            count = 0;
            for (i=0; i < num_del_expr; i++)
            {
                di = &del_expr_info[i];
                for (j=di->index; j < di->index + di->num_objects; j++)
                {
                    gde = &gdv->vector[j];
                    if (gde->group_id == group_id)
                    {
                        if (count == index)
                        {
                            *failed_path = di->req_path;
                            goto exit;
                        }
                        count++;
                    }
                }
            }
        }
    }

    // If the code gets here, then no error occurred, so all objects were deleted successfully
    // Ensure that the objects are deleted from the instance cache, and that they would result in USP notifications being sent
    for (i=0; i < gdv->num_entries; i++)
    {
        gde = &gdv->vector[i];
        if (gde->group_id == group_id)
        {
            DATA_MODEL_NotifyInstanceDeleted(gde->path);
        }
    }
    err = USP_ERR_OK;

exit:
    return err;
}

/*********************************************************************//**
**
** CreateFullDeleteResp
**
** Forms a DeleteResponse object using the results stored in the specified group del vector
**
** \param   msg_id - string containing the message id of the del request, which initiated this response
** \param   del_expr_info - pointer to array tying the resolved objects to delete in group del vector, back to the requested paths to delete in the delete request message
** \param   num_del_expr - number of entries in del_expr_info
** \param   gdv - pointer to group del vector containing the results of attempting to add the objects
**
** \return  Pointer to an AddResponse message
**
**************************************************************************/
Usp__Msg *CreateFullDeleteResp(char *msg_id, del_expr_info_t *del_expr_info, int num_del_expr, group_del_vector_t *gdv)
{
    int i, j;
    Usp__Msg *resp;
    Usp__DeleteResp *del_resp;
    del_expr_info_t *di;
    group_del_entry_t *gde;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success;
    group_del_entry_t *first_failure;

    // Create a Delete Response
    // NOTE: All allow_partial=false error cases have already been dealt with, so this code doesn't have to handle them
    resp = CreateBasicDeleteResp(msg_id);
    del_resp = resp->body->response->delete_resp;

    // Iterate over all requested paths to delete
    for (i=0; i < num_del_expr; i++)
    {
        di = &del_expr_info[i];
        if (di->err_code != USP_ERR_OK)
        {
            // Add an oper_failure for this requested path
            AddDeleteResp_OperFailure(del_resp, di->req_path, di->err_code, di->err_msg);
        }
        else
        {
            first_failure = GROUP_DEL_VECTOR_FindFirstFailureIfAllFailed(gdv, di->index, di->num_objects);
            if (first_failure != NULL)
            {
                // Add an oper_failure for this requested path (since all objects requested to be deleted failed to delete)
                AddDeleteResp_OperFailure(del_resp, di->req_path, first_failure->err_code, first_failure->err_msg);
            }
            else
            {
                // Add an oper_success for this requested path, containing the results of deleting each resolved object
                // NOTE: It is possible for no objects to have been deleted, if the requested path referenced a object which had already been deleted
                oper_success = AddDeleteResp_OperSuccess(del_resp, di->req_path);
                for (j = di->index; j < di->index + di->num_objects; j++)
                {
                    gde = &gdv->vector[j];
                    if (gde->err_code == USP_ERR_OK)
                    {
                        AddOperSuccess_AffectedPath(oper_success, gde->path);
                    }
                    else
                    {
                        AddOperSuccess_UnaffectedPathError(oper_success, gde->path, gde->err_code, gde->err_msg);
                    }
                }
            }
        }
    }

    return resp;
}

/*********************************************************************//**
**
** CreateBasicDeleteResp
**
** Dynamically creates a DeleteResponse object
** NOTE: The object is created without any deleted_obj_results
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the add request, which initiated this response
**
** \return  Pointer to a DeleteResponse object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateBasicDeleteResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__DeleteResp *del_resp;

    // Create Delete Response
    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__DELETE_RESP, USP__RESPONSE__RESP_TYPE_DELETE_RESP);
    del_resp = USP_MALLOC(sizeof(Usp__DeleteResp));
    usp__delete_resp__init(del_resp);
    msg->body->response->delete_resp = del_resp;

    // Start from an empty list
    del_resp->n_deleted_obj_results = 0;
    del_resp->deleted_obj_results = NULL;

    return msg;
}

/*********************************************************************//**
**
** AddDeleteResp_OperSuccess
**
** Dynamically adds a deleted object result success object to the DeleteResponse object
**
** \param   del_resp - pointer to DeleteResponse object
** \param   path - requested path of object to delete
**
** \return  Pointer to dynamically allocated deleted object result success object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *
AddDeleteResp_OperSuccess(Usp__DeleteResp *del_resp, char *path)
{
    Usp__DeleteResp__DeletedObjectResult *deleted_obj_res;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus *oper_status;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success;
    int new_num;    // new number of entries in the created object result array

    // Allocate memory to store the created object result
    deleted_obj_res = USP_MALLOC(sizeof(Usp__DeleteResp__DeletedObjectResult));
    usp__delete_resp__deleted_object_result__init(deleted_obj_res);

    // Allocate memory to store the created oper status object
    oper_status = USP_MALLOC(sizeof(Usp__DeleteResp__DeletedObjectResult__OperationStatus));
    usp__delete_resp__deleted_object_result__operation_status__init(oper_status);

    // Allocate memory to store the created oper success object
    oper_success = USP_MALLOC(sizeof(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess));
    usp__delete_resp__deleted_object_result__operation_status__operation_success__init(oper_success);

    // Increase the size of the vector
    new_num = del_resp->n_deleted_obj_results + 1;
    del_resp->deleted_obj_results = USP_REALLOC(del_resp->deleted_obj_results, new_num*sizeof(void *));
    del_resp->n_deleted_obj_results = new_num;
    del_resp->deleted_obj_results[new_num-1] = deleted_obj_res;

    // Fill in its members
    deleted_obj_res->requested_path = USP_STRDUP(path);
    deleted_obj_res->oper_status = oper_status;

    oper_status->oper_status_case = USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS;
    oper_status->oper_success = oper_success;

    oper_success->n_affected_paths = 0;
    oper_success->affected_paths = NULL;
    oper_success->n_unaffected_path_errs = 0;
    oper_success->unaffected_path_errs = NULL;

    return oper_success;
}

/*********************************************************************//**
**
** AddOperSuccess_AffectedPath
**
** Dynamically adds an affected path to the DeleteResponse OperSuccess object
**
** \param   oper_success - pointer to DeleteResponse OperSuccess object
** \param   path - path of the object resolved from the search path
**
** \return  None
**
**************************************************************************/
void AddOperSuccess_AffectedPath(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path)
{
    int new_num;    // new number of entries in the affected path list

    // Increase the size of the vector
    new_num = oper_success->n_affected_paths + 1;
    oper_success->affected_paths = USP_REALLOC(oper_success->affected_paths, new_num*sizeof(void *));
    oper_success->n_affected_paths = new_num;

    // Add the path to the vector
    oper_success->affected_paths[new_num-1] = TEXT_UTILS_StrDupWithTrailingDot(path);
}

/*********************************************************************//**
**
** AddOperSuccess_UnaffectedPathError
**
** Dynamically adds an unaffected path error to the DeleteResponse OperSuccess object
**
** \param   oper_success - pointer to DeleteResponse OperSuccess object
** \param   path - path of the object resolved from the search path
** \param   err_code - numeric code indicating reason object failed to be deleted
** \param   err_msg - error message indicating reason object failed to be deleted
**
** \return  None
**
**************************************************************************/
void AddOperSuccess_UnaffectedPathError(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success,
                                        char *path, uint32_t err_code, char *err_msg)
{
    int new_num;    // new number of entries in the unaffected path error list
    Usp__DeleteResp__UnaffectedPathError *unaffected_path_err;

    // Allocate memory to store the unaffected path error
    unaffected_path_err = USP_MALLOC(sizeof(Usp__DeleteResp__UnaffectedPathError));
    usp__delete_resp__unaffected_path_error__init(unaffected_path_err);

    // Increase the size of the vector
    new_num = oper_success->n_unaffected_path_errs + 1;
    oper_success->unaffected_path_errs = USP_REALLOC(oper_success->unaffected_path_errs, new_num*sizeof(void *));
    oper_success->n_unaffected_path_errs = new_num;
    oper_success->unaffected_path_errs[new_num-1] = unaffected_path_err;

    // Fill in its members
    unaffected_path_err = oper_success->unaffected_path_errs[new_num-1];
    unaffected_path_err->unaffected_path = TEXT_UTILS_StrDupWithTrailingDot(path);
    unaffected_path_err->err_code = err_code;
    unaffected_path_err->err_msg = USP_STRDUP(err_msg);
}

/*********************************************************************//**
**
** AddDeleteResp_OperFailure
**
** Dynamically adds a deleted object result failure object to the DeleteResponse object
**
** \param   del_resp - pointer to DeleteResponse object
** \param   path - requested search path of object(s) that failed to delete
** \param   err_code - numeric code indicating reason object failed to be deleted
** \param   err_msg - error message indicating reason object failed to be deleted
**
** \return  Pointer to dynamically allocated deleted object result failure object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *
AddDeleteResp_OperFailure(Usp__DeleteResp *del_resp, char *path, uint32_t err_code, char *err_msg)
{
    Usp__DeleteResp__DeletedObjectResult *deleted_obj_res;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus *oper_status;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *oper_failure;
    int new_num;    // new number of entries in the created object result array

    // Allocate memory to store the created object result
    deleted_obj_res = USP_MALLOC(sizeof(Usp__DeleteResp__DeletedObjectResult));
    usp__delete_resp__deleted_object_result__init(deleted_obj_res);

    // Allocate memory to store the created oper status object
    oper_status = USP_MALLOC(sizeof(Usp__DeleteResp__DeletedObjectResult__OperationStatus));
    usp__delete_resp__deleted_object_result__operation_status__init(oper_status);

    // Allocate memory to store the created oper failure object
    oper_failure = USP_MALLOC(sizeof(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure));
    usp__delete_resp__deleted_object_result__operation_status__operation_failure__init(oper_failure);

    // Increase the size of the vector
    new_num = del_resp->n_deleted_obj_results + 1;
    del_resp->deleted_obj_results = USP_REALLOC(del_resp->deleted_obj_results, new_num*sizeof(void *));
    del_resp->n_deleted_obj_results = new_num;
    del_resp->deleted_obj_results[new_num-1] = deleted_obj_res;

    // Fill in its members
    deleted_obj_res->requested_path = USP_STRDUP(path);
    deleted_obj_res->oper_status = oper_status;

    oper_status->oper_status_case = USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE;
    oper_status->oper_failure = oper_failure;

    oper_failure->err_code = err_code;
    oper_failure->err_msg = USP_STRDUP(err_msg);

    return oper_failure;
}

