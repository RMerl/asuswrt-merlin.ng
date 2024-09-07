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
#include "group_set_vector.h"

//------------------------------------------------------------------------------
// String vector storing the param_name associated with each OperFailure object in the response message
// This may be used if the OperFailure objects are converted to param_err objects in an ERROR response, if the Add subsequently fails
// NOTE: In the case when this is read from to convert from an OperFailure to a param_err object, it will contain only one entry
//       because we do this when allow_partial=false and the first object fails to create
//       It may contain multiple entries in the case of allow_partial=true, but that case never leads to an ERROR response
//       containing param_err objects
str_vector_t add_oper_failure_param_names;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int CreateExpressionObjects(Usp__AddResp *add_resp, Usp__Add__CreateObject *cr, bool allow_partial);
int CreateObject_Trans(char *obj_path, Usp__AddResp *add_resp, Usp__Add__CreateObject *cr, bool allow_partial);
int CreateObject(char *obj_path, Usp__AddResp *add_resp, Usp__Add__CreateObject *cr, bool allow_partial);
Usp__Msg *CreateAddResp(char *msg_id);
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *AddResp_OperFailure(Usp__AddResp *add_resp, char *path, char *param_name, int err_code, char *err_msg);
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *AddResp_OperSuccess(Usp__AddResp *add_resp, char *req_path, char *path);
void RemoveAddResp_LastCreatedObjResult(Usp__AddResp *add_resp);
Usp__AddResp__ParameterError *AddResp_OperSuccess_ParamErr(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path, int err_code, char *err_msg);
void AddOperSuccess_UniqueKeys(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success, kv_vector_t *kvv);
int ParamError_FromAddRespToErrResp(Usp__Msg *add_msg, Usp__Msg *err_msg);


/*********************************************************************//**
**
** MSG_HANDLER_HandleAdd
**
** Handles a USP Add message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleAdd(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt)
{
    int i;
    int err;
    Usp__Add *add;
    Usp__Add__CreateObject *cr;
    Usp__Msg *resp = NULL;
    dm_trans_vector_t trans;
    int count;

    STR_VECTOR_Init(&add_oper_failure_param_names);

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_ADD) ||
        (usp->body->request->add == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp, NULL);
        goto exit;
    }

    // Create an Add Response
    resp = CreateAddResp(usp->header->msg_id);

    // Exit if there are no objects to create
    add = usp->body->request->add;
    if ((add->create_objs == NULL) || (add->n_create_objs == 0))
    {
        goto exit;
    }

    // Start a transaction here, if allow_partial is at the global level
    if (add->allow_partial == false)
    {
        err = DM_TRANS_Start(&trans);
        if (err != USP_ERR_OK)
        {
            // If failed to start a transaction, send an error message
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp, NULL);
            goto exit;
        }
    }

    // Iterate over all create objects in the message
    for (i=0; i < add->n_create_objs; i++)
    {
        // Create the specified object
        cr = add->create_objs[i];
        err = CreateExpressionObjects(resp->body->response->add_resp, cr, add->allow_partial);

        // If allow_partial is at the global level, and an error occurred, then fail this
        if ((add->allow_partial == false) && (err != USP_ERR_OK))
        {
            // A required object failed to create
            // So delete the AddResponse message, and send an error message instead
            count = ParamError_FromAddRespToErrResp(resp, NULL);
            err = ERROR_RESP_CalcOuterErrCode(count, err);
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp, ParamError_FromAddRespToErrResp);

            // Abort the global transaction, only logging errors (the message we want to send back over USP is above)
            DM_TRANS_Abort();
            goto exit;
        }
    }

    // Commit transaction here, if allow_partial is at the global level
    if (add->allow_partial == false)
    {
        err = DM_TRANS_Commit();
        if (err != USP_ERR_OK)
        {
            // If failed to commit, delete the AddResponse message, and send an error message instead
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp, NULL);
            goto exit;
        }
    }


exit:
    STR_VECTOR_Destroy(&add_oper_failure_param_names);

    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mrt);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** CreateExpressionObjects
**
** Creates all the objects of the specified path expressions
** Always fills in an OperFailure or OperSuccess for this data model object
**
** \param   add_resp - pointer to USP add response object, which is updated with the results of this operation
** \param   cr - pointer to parsed object to create
** \param   allow_partial - set to true if failures in one object do not affect all others.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CreateExpressionObjects(Usp__AddResp *add_resp, Usp__Add__CreateObject *cr, bool allow_partial)
{
    int i;
    int err;
    str_vector_t obj_paths;
    combined_role_t combined_role;
    char err_msg[128];

    // Return OperFailure if there is no expression
    STR_VECTOR_Init(&obj_paths);
    if ((cr->obj_path == NULL) || (cr->obj_path[0] == '\0'))
    {
        USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: Expression missing in AddRequest", __FUNCTION__);
        AddResp_OperFailure(add_resp, cr->obj_path, NULL, USP_ERR_INVALID_ARGUMENTS, err_msg);
        err = USP_ERR_OK;
        goto exit;
    }

    // Return OperFailure if an internal error occurred
    MSG_HANDLER_GetMsgRole(&combined_role);
    err = PATH_RESOLVER_ResolveDevicePath(cr->obj_path, &obj_paths, NULL, kResolveOp_Add, FULL_DEPTH, &combined_role, 0);
    if (err != USP_ERR_OK)
    {
        AddResp_OperFailure(add_resp, cr->obj_path, NULL, err, USP_ERR_GetMessage());
        goto exit;
    }

    // Return OperFailure if none of the specified objects exist in the schema
    if (obj_paths.num_entries == 0)
    {
        USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: Expression does not reference any objects", __FUNCTION__);
        AddResp_OperFailure(add_resp, cr->obj_path, NULL, USP_ERR_INVALID_ARGUMENTS, err_msg);
        err = USP_ERR_OK;
        goto exit;
    }

    // Iterate over all object paths resolved for this Object expression
    for (i=0; i < obj_paths.num_entries; i++)
    {
        err = CreateObject_Trans(obj_paths.vector[i], add_resp, cr, allow_partial);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

    // If the code gets here, then all parameters of all objects have been set successfully
    err = USP_ERR_OK;

exit:
    STR_VECTOR_Destroy(&obj_paths);
    return err;
}

/*********************************************************************//**
**
** CreateObject_Trans
**
** Wrapper around CreateObject() which performs a transaction at this level, if allow_partial is true
**
** \param   obj_path - path to the object to create
** \param   add_resp - pointer to USP add response object, which is updated with the results of this operation
** \param   cr - pointer to parsed USP CreateObject message
** \param   allow_partial - set to true if failures in this object do not affect all others.
**                          If allow_partial is set then we perform a transaction at this level
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CreateObject_Trans(char *obj_path, Usp__AddResp *add_resp, Usp__Add__CreateObject *cr, bool allow_partial)
{
    int err;
    dm_trans_vector_t trans;

    // Start a transaction here, if allow_partial is at the object level
    if (allow_partial == true)
    {
        // Return OperFailure, if failed to start a transaction
        err = DM_TRANS_Start(&trans);
        if (err != USP_ERR_OK)
        {
            AddResp_OperFailure(add_resp, cr->obj_path, NULL, err, USP_ERR_GetMessage());
            return err;
        }
    }

    // Create the specified object
    err = CreateObject(obj_path, add_resp, cr, allow_partial);

    // Commit/Abort transaction here, if allow_partial is at the object level
    if (allow_partial == true)
    {
        if (err == USP_ERR_OK)
        {
            err = DM_TRANS_Commit();
            if (err != USP_ERR_OK)
            {
                // If transaction failed, then replace the OperSuccess with OperFailure
                // To do this, we remove the last OperSuccessObject from the USP message
                RemoveAddResp_LastCreatedObjResult(add_resp);
                AddResp_OperFailure(add_resp, cr->obj_path, NULL, err, USP_ERR_GetMessage());
            }
        }
        else
        {
            // Because allow_partial=true, we rollback the creation of this object, but do not fail the entire message
            DM_TRANS_Abort();
            err = USP_ERR_OK;
        }
    }

    return err;
}

/*********************************************************************//**
**
** CreateObject
**
** Creates the specified object (if not already created), and overrides it's default parameters with those specified
**
** \param   obj_path - path to the object to create
** \param   add_resp - pointer to USP add response object, which is updated with the results of this operation
** \param   cr - pointer to parsed USP CreateObject message
** \param   allow_partial - set to true if failures in this object do not affect all others.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CreateObject(char *obj_path, Usp__AddResp *add_resp, Usp__Add__CreateObject *cr, bool allow_partial)
{
    int err;
    int instance;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success;
    Usp__Add__CreateParamSetting *ps;
    char full_path[MAX_DM_PATH];
    char *param_name;
    int i;
    int len;
    kv_vector_t unique_key_params;
    combined_role_t combined_role;
    group_set_vector_t gsv;
    group_set_entry_t *gse;

    // Exit if unable to add the specified object (and set the default values of all its child parameters)
    err = DATA_MODEL_AddInstance(obj_path, &instance, CHECK_CREATABLE);
    if (err != USP_ERR_OK)
    {
        // Add an OperFailure object if this object failed to add, but we are continuing because allow_partial=true
        // We don't add an OperFailure object in the case of allow_partial=false because then it would get
        // converted into a param_errs object in the USP Error message
        if (allow_partial==true)
        {
            AddResp_OperFailure(add_resp, cr->obj_path, NULL, err, USP_ERR_GetMessage());
        }
        return err;
    }

    // Create the path to the object
    len = USP_SNPRINTF(full_path, sizeof(full_path), "%s.%d", obj_path, instance);
    MSG_HANDLER_GetMsgRole(&combined_role);

    // Assume OperSuccess
    oper_success = AddResp_OperSuccess(add_resp, cr->obj_path, full_path);

    // Add all parameters to be set to the group set vector
    GROUP_SET_VECTOR_Init(&gsv);
    for (i=0; i < cr->n_param_settings; i++)
    {
        ps = cr->param_settings[i];
        USP_SNPRINTF(&full_path[len], sizeof(full_path)-len, ".%s", ps->param);
        GROUP_SET_VECTOR_Add(&gsv, full_path, ps->value, ps->required, &combined_role);
    }

    // Perform the set of the parameters for this object
    GROUP_SET_VECTOR_SetValues(&gsv, 0, gsv.num_entries);

    // Iterate over all parameters
    USP_ASSERT(gsv.num_entries == cr->n_param_settings);
    for (i=0; i < cr->n_param_settings; i++)
    {
        ps = cr->param_settings[i];
        gse = &gsv.vector[i];

        if (gse->err_code != USP_ERR_OK)
        {
            // The parameter was not set successfully
            if (ps->required)
            {
                // Exit if this parameter was required to be set, but failed
                param_name = ps->param;
                err = gse->err_code;
                USP_ERR_SetMessage("%s", gse->err_msg);
                goto exit;
            }
            else
            {
                // This parameter failed to be set, but was not required
                // So add it to the ParamErr list
                AddResp_OperSuccess_ParamErr(oper_success, ps->param, gse->err_code, gse->err_msg);
            }
        }
    }

    // If the code gets here, then all overriden parameters for this object have been successfully set
    // So now we need to get the values of all parameters used as unique keys

    // Exit if unable to retrieve the parameters used as unique keys for this object
    full_path[len] = '\0';
    param_name = NULL;
    err = DATA_MODEL_GetUniqueKeyParams(full_path, &unique_key_params, &combined_role);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if any unique keys have been left with a default value which is not unique
    err = DATA_MODEL_ValidateDefaultedUniqueKeys(full_path, &unique_key_params, &gsv);
    if (err != USP_ERR_OK)
    {
        KV_VECTOR_Destroy(&unique_key_params);
        goto exit;
    }

    // Move the unique key parameters to the USP oper success message
    // NOTE: The unique_key_params vector will be destroyed in the process, so we do not have to destroy it here
    if (unique_key_params.num_entries > 0)
    {
        AddOperSuccess_UniqueKeys(oper_success, &unique_key_params);
    }
    err = USP_ERR_OK;

exit:
    if (err != USP_ERR_OK)
    {
        // If object failed to create, then replace the OperSuccess with OperFailure
        // To do this, we remove the last OperSuccessObject from the USP message
        RemoveAddResp_LastCreatedObjResult(add_resp);
        AddResp_OperFailure(add_resp, cr->obj_path, param_name, err, USP_ERR_GetMessage());

        // NOTE: We do not delete the object here. Deletion is handled by the transaction rolling back
        // the data model instances vector and the database transaction (for child parameters)
    }

    // Clean up
    GROUP_SET_VECTOR_Destroy(&gsv);

    return err;
}

/*********************************************************************//**
**
** CreateAddResp
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
Usp__Msg *CreateAddResp(char *msg_id)
{
    Usp__Msg *resp;
    Usp__Header *header;
    Usp__Body *body;
    Usp__Response *response;
    Usp__AddResp *add_resp;

    // Allocate and initialise memory to store the parts of the USP message
    resp = USP_MALLOC(sizeof(Usp__Msg));
    usp__msg__init(resp);

    header = USP_MALLOC(sizeof(Usp__Header));
    usp__header__init(header);

    body = USP_MALLOC(sizeof(Usp__Body));
    usp__body__init(body);

    response = USP_MALLOC(sizeof(Usp__Response));
    usp__response__init(response);

    add_resp = USP_MALLOC(sizeof(Usp__AddResp));
    usp__add_resp__init(add_resp);

    // Connect the structures together
    resp->header = header;
    header->msg_id = USP_STRDUP(msg_id);
    header->msg_type = USP__HEADER__MSG_TYPE__ADD_RESP;

    resp->body = body;
    body->msg_body_case = USP__BODY__MSG_BODY_RESPONSE;
    body->response = response;
    response->resp_type_case = USP__RESPONSE__RESP_TYPE_ADD_RESP;
    response->add_resp = add_resp;

    add_resp->n_created_obj_results = 0;    // Start from an empty list
    add_resp->created_obj_results = NULL;

    return resp;
}

/*********************************************************************//**
**
** AddResp_OperFailure
**
** Dynamically adds an operation failure object to the AddResponse object
**
** \param   resp - pointer to AddResponse object
** \param   path - requested path of object which failed to create
** \param   param_name - name of first parameter which caused the create to fail, or NULL if cause of failure was not a parameter
** \param   err_code - numeric code indicating reason object failed to be created
** \param   err_msg - error message indicating reason object failed to be created
**
** \return  Pointer to dynamically allocated operation failure object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *
AddResp_OperFailure(Usp__AddResp *add_resp, char *path, char *param_name, int err_code, char *err_msg)
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

    // In the protobuf schema, the OperFailure object does not store the param_name of the parameter which caused the failure
    // But we might need this later, if the OperFailure object is converted into a param_errs object of an Error message
    // (because a later object caused a failure when allow_partial=false).
    // So for each OperFailure object, we store the associated param_name in a string vector
    // NOTE: This is not ideal, but we cannot add this extra variable into the OperFailure object without either changing
    // the protobuf schema for USP messages, or manually hacking the code generated to implement the probuf message
    if (param_name == NULL)
    {
        param_name = "";
    }
    STR_VECTOR_Add(&add_oper_failure_param_names, param_name);

    return oper_failure;
}

/*********************************************************************//**
**
** ParamError_FromAddRespToErrResp
**
** Extracts the parameters in error from the OperFailure object of the AddResponse
** and adds them as ParamError objects to an ErrResponse object if supplied.
** If not supplied, it just counts the number of ParamError objects that would be added.
**
** \param   add_msg - pointer to AddResponse object
** \param   err_msg - pointer to ErrResponse object. If NULL, this indicates that the purpose of this function is just
**                    to return the count of ParamErr objects that would be added
**
** \return  Number of ParamErr objects that were (or would be) added to an ErrResponse
**
**************************************************************************/
int ParamError_FromAddRespToErrResp(Usp__Msg *add_msg, Usp__Msg *err_msg)
{
    Usp__Body *body;
    Usp__Response *response;
    Usp__AddResp *add_resp;
    Usp__AddResp__CreatedObjectResult *created_obj_res;
    Usp__AddResp__CreatedObjectResult__OperationStatus *oper_status;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *oper_failure;
    char *obj_path;
    char *param_name;
    char path[MAX_DM_PATH];
    int err_code;
    char *err_str;
    int i;
    int num_params;
    int count = 0;

    // Navigate to the AddResponse object within the AddResponse message
    body = add_msg->body;
    USP_ASSERT(body != NULL);

    response = body->response;
    USP_ASSERT(response != NULL);

    add_resp = response->add_resp;
    USP_ASSERT(add_resp != NULL);

    // Iterate over all object failures
    num_params = add_resp->n_created_obj_results;
    for (i=0; i < num_params; i++)
    {
        created_obj_res = add_resp->created_obj_results[i];
        USP_ASSERT(created_obj_res != NULL);

        oper_status = created_obj_res->oper_status;
        USP_ASSERT(oper_status != NULL);

        // Convert an OperFailure object into a ParamError object
        if (oper_status->oper_status_case == USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE)
        {
            if (err_msg != NULL)
            {
                oper_failure = oper_status->oper_failure;
                USP_ASSERT(oper_failure != NULL);

                // Extract the ParamError fields
                obj_path = created_obj_res->requested_path;
                err_code = oper_failure->err_code;
                err_str = oper_failure->err_msg;

                // Get the name of the parameter associated with this OperFailure
                if (count < add_oper_failure_param_names.num_entries)
                {
                    param_name = add_oper_failure_param_names.vector[count];
                }
                else
                {
                    // NOTE: This should never occur, as when we add an OperFailure we also add to add_oper_failure_param_names
                    param_name = "";
                }

                // Form the full path for the param_err and add it to the response
                if (param_name[0] != '\0')
                {
                    // Path includes a param name
                    USP_SNPRINTF(path, sizeof(path), "%s{i}.%s", obj_path, param_name);
                }
                else
                {
                    // Path is just an object name
                    USP_SNPRINTF(path, sizeof(path), "%s", obj_path);
                }

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
** RemoveAddResp_LastCreatedObjResult
**
** Removes the last CreatedObjResult object from the AddResp object
** The CreatedObjResult object will contain either an OperSuccess or an OperFailure
**
** \param   add_resp - pointer to add response object to modify
**
** \return  None
**
**************************************************************************/
void RemoveAddResp_LastCreatedObjResult(Usp__AddResp *add_resp)
{
    int index;
    Usp__AddResp__CreatedObjectResult *created_obj_res;

    // Free the memory associated with the last created obj_result
    index = add_resp->n_created_obj_results - 1;
    created_obj_res = add_resp->created_obj_results[index];
    protobuf_c_message_free_unpacked ((ProtobufCMessage*)created_obj_res, pbuf_allocator);

    // Fix the SetResp object, so that it does not reference the obj_result we have just removed
    add_resp->created_obj_results[index] = NULL;
    add_resp->n_created_obj_results--;
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
void AddOperSuccess_UniqueKeys(Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success,
                               kv_vector_t *kvv)
{
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry *entry;
    kv_pair_t *kv;
    int i;

    USP_ASSERT((kvv->num_entries > 0) && (kvv->vector != NULL));

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

