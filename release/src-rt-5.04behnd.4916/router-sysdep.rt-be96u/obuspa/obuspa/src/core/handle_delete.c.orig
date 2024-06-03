/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
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

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int DeleteExpressionObjects(Usp__DeleteResp *del_resp, char *exp_path, bool allow_partial);
Usp__Msg *CreateDeleteResp(char *msg_id);
Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *AddDeleteResp_OperFailure(Usp__DeleteResp *del_resp, char *path, uint32_t err_code, char *err_msg);
Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *AddDeleteResp_OperSuccess(Usp__DeleteResp *del_resp, char *path);
void AddOperSuccess_AffectedPath(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path);
void AddOperSuccess_UnaffectedPathError(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path, uint32_t err_code, char *err_msg);
void RemoveDeleteResp_LastDeletedObjResult(Usp__DeleteResp *del_resp);
int ParamError_FromDeleteRespToErrResp(Usp__Msg *del_msg, Usp__Msg *err_msg);

/*********************************************************************//**
**
** MSG_HANDLER_HandleDelete
**
** Handles a USP Delete message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleDelete(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt)
{
    int i;
    int err;
    Usp__Delete *del;
    Usp__Msg *resp = NULL;
    char *exp_path;
    dm_trans_vector_t trans;
    int count;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_DELETE) ||
        (usp->body->request->delete_ == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp, NULL);
        goto exit;
    }

    // Create a Delete Response
    resp = CreateDeleteResp(usp->header->msg_id);

    // Exit if there are no objects to delete
    del = usp->body->request->delete_;
    if ((del->obj_paths == NULL) || (del->n_obj_paths == 0))
    {
        goto exit;
    }

    // Start a transaction here, if allow_partial is at the global level
    if (del->allow_partial == false)
    {
        err = DM_TRANS_Start(&trans);
        if (err != USP_ERR_OK)
        {
            // If failed to start a transaction, send an error message
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp, NULL);
            goto exit;
        }
    }

    // Iterate over all paths in the message
    for (i=0; i < del->n_obj_paths; i++)
    {
        // Delete the specified path
        exp_path = del->obj_paths[i];
        err = DeleteExpressionObjects(resp->body->response->delete_resp, exp_path, del->allow_partial);

        // If allow_partial is at the global level, and an error occurred, then fail this
        if ((del->allow_partial == false) && (err != USP_ERR_OK))
        {
            // A required object failed to delete
            // So delete the DeleteResponse message, and send an error message instead
            count = ParamError_FromDeleteRespToErrResp(resp, NULL);
            err = ERROR_RESP_CalcOuterErrCode(count, err);
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp, ParamError_FromDeleteRespToErrResp);

            // Abort the global transaction, only logging errors (the message we want to send back over USP is above)
            DM_TRANS_Abort();
            goto exit;
        }
    }

    // Commit transaction here, if allow_partial is at the global level
    if (del->allow_partial == false)
    {
        err = DM_TRANS_Commit();
        if (err != USP_ERR_OK)
        {
            // If failed to commit, delete the DeleteResponse message, and send an error message instead
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp, NULL);
            goto exit;
        }
    }


exit:
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mrt);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** DeleteExpressionObjects
**
** Deletes all the objects of the specified path expression
** Always fills in a DeletedObjectResult for this data model object
**
** \param   del_resp - pointer to USP delete response object, which is updated with the results of this operation
** \param   exp_path - pointer to expression path containing object(s) to delete
** \param   allow_partial - set to true if failures in this object do not affect all others.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DeleteExpressionObjects(Usp__DeleteResp *del_resp, char *exp_path, bool allow_partial)
{
    int i;
    int err;
    str_vector_t obj_paths;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success;
    combined_role_t combined_role;
    dm_trans_vector_t trans;

    // Exit if unable to resolve to objects that are instantiated
    STR_VECTOR_Init(&obj_paths);
    MSG_HANDLER_GetMsgRole(&combined_role);
    err = PATH_RESOLVER_ResolveDevicePath(exp_path, &obj_paths, NULL, kResolveOp_Del, FULL_DEPTH, &combined_role, 0);
    if (err != USP_ERR_OK)
    {
        AddDeleteResp_OperFailure(del_resp, exp_path, err, USP_ERR_GetMessage());
        goto exit;
    }

    // Add the OperationSuccess object (ie assume successful operation)
    oper_success = AddDeleteResp_OperSuccess(del_resp, exp_path);

    // Iterate over all object paths resolved for this Object expression
    for (i=0; i < obj_paths.num_entries; i++)
    {
        // Start a transaction here, if allow_partial is at the path expression level
        if (allow_partial == true)
        {
            // Exit if unable to start a transaction. Note if this occurs, an error response is returned
            err = DM_TRANS_Start(&trans);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }

        // Delete the object
        err = DATA_MODEL_DeleteInstance(obj_paths.vector[i], CHECK_DELETABLE);
        if (err == USP_ERR_OK)
        {
            // No error occurred, so add to the list of objects successfully deleted
            AddOperSuccess_AffectedPath(oper_success, obj_paths.vector[i]);

            // Commit the transaction here, if allow_partial is at the individual object level
            if (allow_partial == true)
            {
                err = DM_TRANS_Commit();
                if (err != USP_ERR_OK)
                {
                    goto exit;
                }
            }
        }
        else
        {
            // If the code gets here, the delete failed
            // Abort transaction here, if allow_partial is at the path expression level
            if (allow_partial == true)
            {
                // Signal back that we failed to delete this object, but continue with the next object in the loop
                AddOperSuccess_UnaffectedPathError(oper_success, obj_paths.vector[i], err, USP_ERR_GetMessage());
                DM_TRANS_Abort();          // Explicitly ignoring errors, as the error code we want to return is that of the
                                           // original failure
            }
            else
            {
                // Exit if the delete failed, and we don't allow partial deletion of all objects
                // Remove the success object and replace with a failure object.
                // The failure object will get converted into the param_errs element of the Error Message
                RemoveDeleteResp_LastDeletedObjResult(del_resp);
                AddDeleteResp_OperFailure(del_resp, exp_path, err, USP_ERR_GetMessage());
                goto exit;
            }
        }
    }

    // NOTE: If the code gets here, the last DeletedObjectResult will be an OperSuccess
    // If all deletions failed, then change the OperSuccess into an OperFailure
    if ((oper_success->n_affected_paths == 0) && (oper_success->n_unaffected_path_errs > 0))
    {
        // Keep a copy of the first unaffected path errs error message in the OperSuccess, as we want to put it in the OperFailure
        char err_msg[USP_ERR_MAXLEN];
        err = oper_success->unaffected_path_errs[0]->err_code;
        USP_STRNCPY(err_msg, oper_success->unaffected_path_errs[0]->err_msg, sizeof(err_msg));

        // Remove the success object and replace with a failure object
        RemoveDeleteResp_LastDeletedObjResult(del_resp);
        AddDeleteResp_OperFailure(del_resp, exp_path, err, err_msg);
    }

    // If the code gets here, then all objects have been deleted successfully
    err = USP_ERR_OK;

exit:
    STR_VECTOR_Destroy(&obj_paths);
    return err;
}

/*********************************************************************//**
**
** CreateDeleteResp
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
Usp__Msg *CreateDeleteResp(char *msg_id)
{
    Usp__Msg *resp;
    Usp__Header *header;
    Usp__Body *body;
    Usp__Response *response;
    Usp__DeleteResp *del_resp;

    // Allocate and initialise memory to store the parts of the USP message
    resp = USP_MALLOC(sizeof(Usp__Msg));
    usp__msg__init(resp);

    header = USP_MALLOC(sizeof(Usp__Header));
    usp__header__init(header);

    body = USP_MALLOC(sizeof(Usp__Body));
    usp__body__init(body);

    response = USP_MALLOC(sizeof(Usp__Response));
    usp__response__init(response);

    del_resp = USP_MALLOC(sizeof(Usp__DeleteResp));
    usp__delete_resp__init(del_resp);

    // Connect the structures together
    resp->header = header;
    header->msg_id = USP_STRDUP(msg_id);
    header->msg_type = USP__HEADER__MSG_TYPE__DELETE_RESP;

    resp->body = body;
    body->msg_body_case = USP__BODY__MSG_BODY_RESPONSE;
    body->response = response;
    response->resp_type_case = USP__RESPONSE__RESP_TYPE_DELETE_RESP;
    response->delete_resp = del_resp;

    del_resp->n_deleted_obj_results = 0;    // Start from an empty list
    del_resp->deleted_obj_results = NULL;

    return resp;
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

/*********************************************************************//**
**
** ParamError_FromDeleteRespToErrResp
**
** Extracts the parameters in error from the OperFailure object of the DeleteResponse
** and adds them as ParamError objects to an ErrResponse object if supplied.
** If not supplied, it just counts the number of ParamError objects that would be added.
**
** \param   del_msg - pointer to DeleteResponse object
** \param   err_msg - pointer to ErrResponse object. If NULL, this indicates that the purpose of this function is just
**                    to return the count of ParamErr objects that would be added
**
** \return  Number of ParamErr objects that were (or would be) added to an ErrResponse
**
**************************************************************************/
int ParamError_FromDeleteRespToErrResp(Usp__Msg *del_msg, Usp__Msg *err_msg)
{
    Usp__Body *body;
    Usp__Response *response;
    Usp__DeleteResp *del_resp;
    Usp__DeleteResp__DeletedObjectResult *deleted_obj_res;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus *oper_status;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *oper_failure;
    char *path;
    int err_code;
    char *err_str;
    int i;
    int num_params;
    int count = 0;

    // Navigate to the DeleteResponse object within the DeleteResponse message
    body = del_msg->body;
    USP_ASSERT(body != NULL);

    response = body->response;
    USP_ASSERT(response != NULL);

    del_resp = response->delete_resp;
    USP_ASSERT(del_resp != NULL);

    // Iterate over all object failures
    num_params = del_resp->n_deleted_obj_results;
    for (i=0; i < num_params; i++)
    {
        deleted_obj_res = del_resp->deleted_obj_results[i];
        USP_ASSERT(deleted_obj_res != NULL);

        oper_status = deleted_obj_res->oper_status;
        USP_ASSERT(oper_status != NULL);

        // Convert an OperFailure object into a ParamError object
        if (oper_status->oper_status_case == USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE)
        {
            if (err_msg != NULL)
            {
                oper_failure = oper_status->oper_failure;
                USP_ASSERT(oper_failure != NULL);

                // Extract the ParamError fields
                path = deleted_obj_res->requested_path;
                err_code = oper_failure->err_code;
                err_str = oper_failure->err_msg;
                ERROR_RESP_AddParamError(err_msg, path, err_code, err_str);
            }

            // Increment the number of param err fields
            count++;
        }
    }

    return count;
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
void AddOperSuccess_AffectedPath(Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success,
                                        char *path)
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
** RemoveDeleteResp_LastDeletedObjResult
**
** Removes the last DeletedObjResult object from the DeleteResp object
** The LastDeletedObjResult object will contain either an OperSuccess or an OperFailure
**
** \param   del_resp - pointer to delete response object to modify
**
** \return  None
**
**************************************************************************/
void RemoveDeleteResp_LastDeletedObjResult(Usp__DeleteResp *del_resp)
{
    int index;
    Usp__DeleteResp__DeletedObjectResult *deleted_obj_res;

    // Free the memory associated with the last updated obj_result
    index = del_resp->n_deleted_obj_results - 1;
    deleted_obj_res = del_resp->deleted_obj_results[index];
    protobuf_c_message_free_unpacked ((ProtobufCMessage*)deleted_obj_res, pbuf_allocator);

    // Fix the DeleteResp object, so that it does not reference the obj_result we have just removed
    del_resp->deleted_obj_results[index] = NULL;
    del_resp->n_deleted_obj_results--;
}
