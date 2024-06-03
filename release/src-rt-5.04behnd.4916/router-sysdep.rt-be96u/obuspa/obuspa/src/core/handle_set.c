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
 * \file handle_set.c
 *
 * Handles the SetRequest message, creating a SetResponse
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
#include "dm_trans.h"
#include "path_resolver.h"
#include "device.h"
#include "group_set_vector.h"

//------------------------------------------------------------------------------
// Structure used to marshall entries in set group vector for a path expression
// This structure is equivalent to the updated_obj_results object in the USP Set Response message
typedef struct
{
    char *requested_path;   // Pointer to path expression (owned by USP Set Request message).
    str_vector_t resolved_objs; // vector of object paths that have been resolved
    int index;              // start index of parameters in set group vector for this requested path
    int err_code;           // error code if path resolution failed for this requested path
    char *err_msg;          // error message if path resolution failed for this requested path

    int num_params;         // number of parameters to set in each resolved object
    Usp__Set__UpdateParamSetting **param_settings;  // Pointer to vector of names of parameters to set (owned by USP request message)
} set_expr_info_t;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void Grouped_HandleSet(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt);
Usp__Msg *CreateSetResp(char *msg_id);
Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *AddSetResp_OperFailure(Usp__SetResp *set_resp, char *path, int err_code, char *err_msg);
Usp__SetResp__UpdatedInstanceFailure *
AddOperFailure_UpdatedInstFailure(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure, char *path);
Usp__SetResp__ParameterError *
AddUpdatedInstFailure_ParamErr(Usp__SetResp__UpdatedInstanceFailure *updated_inst_failure, char *path, int err_code, char *err_msg);
Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *AddSetResp_OperSuccess(Usp__SetResp *set_resp, char *path);
Usp__SetResp__UpdatedInstanceResult *AddOperSuccess_UpdatedInstRes(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path);
Usp__SetResp__UpdatedInstanceResult__UpdatedParamsEntry *AddUpdatedInstRes_ParamsEntry(Usp__SetResp__UpdatedInstanceResult *updated_inst_result, char *key, char *value);
Usp__SetResp__ParameterError *AddUpdatedInstRes_ParamErr(Usp__SetResp__UpdatedInstanceResult *updated_inst_result, char *path, int err_code, char *err_msg);
void ExpandSetPathExpression(Usp__Set__UpdateObject *up, set_expr_info_t *si, group_set_vector_t *gsv);
Usp__Msg *ProcessAllowPartialFalse(char *msg_id, set_expr_info_t *set_expr_info, int num_set_expr, group_set_vector_t *gsv);
Usp__Msg *CreateErrRespFromFailedSetParams(char *msg_id, group_set_vector_t *gsv, int first_failure, int last_param_index);
Usp__Msg *ProcessAllowPartialTrue(char *msg_id, set_expr_info_t *set_expr_info, int num_set_expr, group_set_vector_t *gsv);
void ProcessAllowPartialTrue_Expression(char *msg_id, Usp__SetResp *set_resp, set_expr_info_t *si, group_set_vector_t *gsv);
void PopulateSetResp_OperSuccess(Usp__SetResp *set_resp, set_expr_info_t *si, group_set_vector_t *gsv);
void DestroySetExprInfo(set_expr_info_t *set_expr_info, int num_set_expr);
void PopulateSetResp_OperFailure(Usp__SetResp *set_resp, set_expr_info_t *si, group_set_vector_t *gsv);
void PopulateOperFailure_UpdatedInstFailure(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure, set_expr_info_t *si, int obj_index, group_set_vector_t *gsv);

/*********************************************************************//**
**
** MSG_HANDLER_HandleSet
**
** Handles a USP Set message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleSet(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt)
{
    Grouped_HandleSet(usp, controller_endpoint, mrt);
}

/*********************************************************************//**
**
** Grouped_HandleSet
**
** Handles a USP Set message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void Grouped_HandleSet(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt)
{
    int i;
    set_expr_info_t *set_expr_info = NULL;
    int num_set_expr = 0;
    group_set_vector_t gsv;
    Usp__Set *set;
    Usp__Msg *resp = NULL;
    int size;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    GROUP_SET_VECTOR_Init(&gsv);
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_SET) ||
        (usp->body->request->set == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp, NULL);
        goto exit;
    }

    // Exit if there are no parameters to set
    set = usp->body->request->set;
    if ((set->update_objs == NULL) || (set->n_update_objs == 0))
    {
        resp = CreateSetResp(usp->header->msg_id);
        goto exit;
    }

    // Allocate memory for all set expressions
    num_set_expr = set->n_update_objs;
    size = num_set_expr * sizeof(set_expr_info_t);
    set_expr_info = USP_MALLOC(size);
    memset(set_expr_info, 0, size);

    // Iterate over all expressions in the message, populating the set expression and group set vectors
    for (i=0; i < set->n_update_objs; i++)
    {
        ExpandSetPathExpression(set->update_objs[i], &set_expr_info[i], &gsv);
    }

    if (set->allow_partial == false)
    {
        resp = ProcessAllowPartialFalse(usp->header->msg_id, set_expr_info, num_set_expr, &gsv);
    }
    else
    {
        resp = ProcessAllowPartialTrue(usp->header->msg_id, set_expr_info, num_set_expr, &gsv);
    }

exit:
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mrt);
    usp__msg__free_unpacked(resp, pbuf_allocator);
    GROUP_SET_VECTOR_Destroy(&gsv);
    DestroySetExprInfo(set_expr_info, num_set_expr);

}

/*********************************************************************//**
**
** ExpandSetPathExpression
**
** Expands the specified path expression into the group set vector and set_expr_info vectors
**
** \param   up - pointer to parsed object to update in USP Get request messsage
** \param   si - pointer to info about specified path expression
** \param   gsv - pointer to group set vector to add the parameters to
**
** \return  None - any errors detected are stored for later processing in the set result or group set vectors
**
**************************************************************************/
void ExpandSetPathExpression(Usp__Set__UpdateObject *up, set_expr_info_t *si, group_set_vector_t *gsv)
{
    int i, j;
    int err;
    combined_role_t combined_role;
    char path[MAX_DM_PATH];
    char *affected_obj;
    Usp__Set__UpdateParamSetting *ps;

    // Set default return value
    si->requested_path = up->obj_path;
    si->index = gsv->num_entries;

    // Exit if there is no expression
    if ((up->obj_path == NULL) || (up->obj_path[0] == '\0'))
    {
        USP_ERR_SetMessage("%s: Expression missing in SetRequest", __FUNCTION__);
        si->err_code = USP_ERR_INVALID_ARGUMENTS;
        si->err_msg = USP_STRDUP( USP_ERR_GetMessage() );
        return;
    }

    // Exit if there are no parameters
    if ((up->n_param_settings == 0) || (up->param_settings == NULL))
    {
        USP_ERR_SetMessage("%s: Parameter names missing in SetRequest", __FUNCTION__);
        si->err_code = USP_ERR_INVALID_ARGUMENTS;
        si->err_msg = USP_STRDUP( USP_ERR_GetMessage() );
        return;
    }

    // Associate names of parameters to set with this path expression
    si->num_params = up->n_param_settings;
    si->param_settings = up->param_settings;

    // Exit if unable to resolve the path expression specifying the objects to update
    MSG_HANDLER_GetMsgRole(&combined_role);
    err = PATH_RESOLVER_ResolveDevicePath(up->obj_path, &si->resolved_objs, NULL, kResolveOp_Set, FULL_DEPTH, &combined_role, 0);
    if (err != USP_ERR_OK)
    {
        si->err_code = err;
        si->err_msg = USP_STRDUP( USP_ERR_GetMessage() );
        return;
    }

    // Exit if the path expression resolves to no objects
    if (si->resolved_objs.num_entries == 0)
    {
        return;
    }

    // Iterate over all objects which have been resolved from the path expression
    for (i=0; i < si->resolved_objs.num_entries; i++)
    {
        affected_obj = si->resolved_objs.vector[i];

        // Iterate over all parameters to be set for these objects
        for (j=0; j < up->n_param_settings; j++)
        {
            // Add the parameter (full path) to the group set vector
            ps = up->param_settings[j];
            USP_SNPRINTF(path, sizeof(path), "%s.%s", affected_obj, ps->param);
            GROUP_SET_VECTOR_Add(gsv, path, ps->value, ps->required, &combined_role);
        }
    }
}

/*********************************************************************//**
**
** ProcessAllowPartialFalse
**
** Processes a Set request where AllowPartial is false. This means that any error setting any required parameter aborts all
**
** \param   msg_id - string containing the message id of the USP message, which initiated this response
** \param   set_expr_info - pointer to array of set expr info structures
** \param   num_set_expr - number of elements in the array
** \param   gsv - group set vector containing all parameters to set
**
** \return  None - this function puts any errors into the response message
**
**************************************************************************/
Usp__Msg *ProcessAllowPartialFalse(char *msg_id, set_expr_info_t *set_expr_info, int num_set_expr, group_set_vector_t *gsv)
{
    int i;
    int err;
    dm_trans_vector_t trans;
    set_expr_info_t *si;
    int first_failure;
    Usp__Msg *resp;
    Usp__SetResp *set_resp;

    // Exit if any of the path expressions failed
    for (i=0; i < num_set_expr; i++)
    {
        si = &set_expr_info[i];
        if (si->err_code != USP_ERR_OK)
        {
            USP_ERR_SetMessage("%s", si->err_msg);
            resp = ERROR_RESP_CreateSingle(msg_id, si->err_code, NULL, NULL);
            return resp;
        }
    }

    // Exit if unable to start a transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_CreateSingle(msg_id, err, NULL, NULL);
        return resp;
    }

    // Attempt to set the values of all parameters
    GROUP_SET_VECTOR_SetValues(gsv, 0, gsv->num_entries);

    // Exit if any of the required parameters failed to set
    first_failure = GROUP_SET_VECTOR_GetFailureIndex(gsv, 0, gsv->num_entries);
    if (first_failure != INVALID)
    {
        DM_TRANS_Abort();
        resp = CreateErrRespFromFailedSetParams(msg_id, gsv, first_failure, gsv->num_entries - 1);
        return resp;
    }

    // Exit if unable to commit the transaction
    err = DM_TRANS_Commit();
    if (err != USP_ERR_OK)
    {
        resp = ERROR_RESP_CreateSingle(msg_id, err, NULL, NULL);
        return resp;
    }

    // If the code gets here, all required parameters have been set successfully
    // So form the response message
    resp = CreateSetResp(msg_id);
    set_resp = resp->body->response->set_resp;

    // Iterate over all path expressions
    for (i=0; i < num_set_expr; i++)
    {
        si = &set_expr_info[i];
        PopulateSetResp_OperSuccess(set_resp, si, gsv);
    }

    return resp;
}

/*********************************************************************//**
**
** CreateErrRespFromFailedSetParams
**
** Creates an error response containing all failed required parameters
**
** \param   msg_id - string containing the message id of the USP message, which initiated this response
** \param   gsv - group set vector containing all parameters
** \param   first_failure - index of the first failed parameter in the group set vector
** \param   last_param_index - index of the last parameter that could contain a failure that we want to report in param errs
**
** \return  pointer to USP error response message
**
**************************************************************************/
Usp__Msg *CreateErrRespFromFailedSetParams(char *msg_id, group_set_vector_t *gsv, int first_failure, int last_param_index)
{
    int i;
    int outer_err_code;
    group_set_entry_t *gse;
    Usp__Msg *resp;

    // Calculate the outer error code for the error response, based on the error message of the first failing parameter
    gse = &gsv->vector[first_failure];
    outer_err_code = ERROR_RESP_CalcOuterErrCode(1, gse->err_code);

    // Create an error response
    USP_ERR_SetMessage("%s", gse->err_msg);
    resp = ERROR_RESP_CreateSingle(msg_id, outer_err_code, NULL, NULL);

    // Populate the error response with param errors for all failing required parameters
    for (i=first_failure; i <= last_param_index; i++)
    {
        gse = &gsv->vector[i];
        if ((gse->err_code != USP_ERR_OK) && (gse->is_required))
        {
            ERROR_RESP_AddParamError(resp, gse->path, gse->err_code, gse->err_msg);
        }
    }

    return resp;
}

/*********************************************************************//**
**
** ProcessAllowPartialTrue
**
** Processes a Set request where AllowPartial is true, generating a response message
** AllowPartial==true means that failures abort the expression they affect
**
** \param   msg_id - string containing the message id of the USP message, which initiated this response
** \param   set_expr_info - vector of set results, one for each resolved object
** \param   num_set_expr - the number of entries in the set_expr_info vector
** \param   gsv - group set vector containing all parameters to set
**
** \return  None - this function puts any errors into the response message
**
**************************************************************************/
Usp__Msg *ProcessAllowPartialTrue(char *msg_id, set_expr_info_t *set_expr_info, int num_set_expr, group_set_vector_t *gsv)
{
    int i;
    Usp__Msg *resp;
    Usp__SetResp *set_resp;

    // Form the response message
    resp = CreateSetResp(msg_id);
    set_resp = resp->body->response->set_resp;

    // Iterate over all resolved expressions
    for (i=0; i < num_set_expr; i++)
    {
        ProcessAllowPartialTrue_Expression(msg_id, set_resp, &set_expr_info[i], gsv);
    }

    return resp;
}

/*********************************************************************//**
**
** ProcessAllowPartialTrue_Expression
**
** Processes a single expression in a Set request where AllowPartial is true, generating a response message
**
** \param   msg_id - string containing the message id of the USP message, which initiated this response
** \param   set_resp - pointer to set response message to populate with success or failure responses
** \param   si - pointer to set_expr_info describing the expression
** \param   gsv - group set vector containing all parameters to set
**
** \return  None - this function puts any errors into the response message
**
**************************************************************************/
void ProcessAllowPartialTrue_Expression(char *msg_id, Usp__SetResp *set_resp, set_expr_info_t *si, group_set_vector_t *gsv)
{
    int err;
    dm_trans_vector_t trans;
    int num_params_in_expr;
    int failure_index;

    // Exit if this path expression failed to resolve, adding a failure response
    if (si->err_code != USP_ERR_OK)
    {
        AddSetResp_OperFailure(set_resp, si->requested_path, si->err_code, si->err_msg);
        return;
    }

    // Exit if unable to start a transaction for this object, adding a failure response
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        (void)AddSetResp_OperFailure(set_resp, si->requested_path, err, USP_ERR_GetMessage());
        return;
    }

    // Attempt to set all parameters in this expression
    num_params_in_expr = si->resolved_objs.num_entries * si->num_params;
    GROUP_SET_VECTOR_SetValues(gsv, si->index, num_params_in_expr);

    // Exit if any of the required parameters failed to set
    failure_index = GROUP_SET_VECTOR_GetFailureIndex(gsv, si->index, num_params_in_expr);
    if (failure_index != INVALID)
    {
        DM_TRANS_Abort();
        PopulateSetResp_OperFailure(set_resp, si, gsv);
        return;
    }

    // Exit if failed to commit the changes, adding a failure response
    err = DM_TRANS_Commit();
    if (err != USP_ERR_OK)
    {
        (void)AddSetResp_OperFailure(set_resp, si->requested_path, err, USP_ERR_GetMessage());
        return;
    }

    // If the code gets here, then all of the required parameters in all objects were set successfully
    // So populate a success response for this data model object
    PopulateSetResp_OperSuccess(set_resp, si, gsv);
}

/*********************************************************************//**
**
** PopulateSetResp_OperFailure
**
** Adds an OperFailure to a SetResp object and populates it with UpdatedInstFailure objects
** for each object which had required parameters that failed to set
**
** \param   set_resp - pointer to set response message to populate
** \param   si - pointer to set_expr_info describing the expression that failed
** \param   gsv - group set vector containing all parameters to set that failed
**
** \return  None - this function puts any errors into the response message
**
**************************************************************************/
void PopulateSetResp_OperFailure(Usp__SetResp *set_resp, set_expr_info_t *si, group_set_vector_t *gsv)
{
    int i;
    Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure;

    // Add an OperFailure to the SetResp
    oper_failure = AddSetResp_OperFailure(set_resp, si->requested_path, USP_ERR_REQUIRED_PARAM_FAILED, "Failed to set required parameters");

    // Add UpdatedInstFailure objects for all objects with required parameters that failed
    for (i=0; i < si->resolved_objs.num_entries; i++)
    {
        PopulateOperFailure_UpdatedInstFailure(oper_failure, si, i, gsv);
    }
}

/*********************************************************************//**
**
** PopulateOperFailure_UpdatedInstFailure
**
** Populates the OperFailure object with UpdatedInstFailure objects for the specified
** data model object, if it had required parameters that failed to set
**
** \param   oper_failure - pointer to OperFailure object to add UpdatedInstFailure objects to
** \param   si - pointer to set_expr_info describing the expression that failed
** \param   obj_index - index of the object to process (in the resolved_objs vector)
** \param   gsv - group set vector containing all parameters to set that failed
**
** \return  None - this function puts any errors into the response message
**
**************************************************************************/
void PopulateOperFailure_UpdatedInstFailure(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure, set_expr_info_t *si, int obj_index, group_set_vector_t *gsv)
{
    int i;
    group_set_entry_t *gse;
    int param_index;  // index of the first parameter to be set for this object in the group set vector
    int failure_index;  // index of first required parameter which had an error in the group set vector
    char *obj_path;
    char *param_name;
    Usp__SetResp__UpdatedInstanceFailure *updated_inst_failure;

    // Exit if no required parameters failed for this object
    // In this case, no UpdatedInstFailure is added to the OperFailure
    param_index = si->index + obj_index*(si->num_params);
    failure_index = GROUP_SET_VECTOR_GetFailureIndex(gsv, param_index, si->num_params);
    if (failure_index == INVALID)
    {
        return;
    }

    // Add an UpdatedInstFailure object
    obj_path = si->resolved_objs.vector[obj_index];
    updated_inst_failure = AddOperFailure_UpdatedInstFailure(oper_failure, obj_path);

    // Iterate over all parameters, adding all required parameters (that failed to set) as ParamErr objects
    for (i=0; i < si->num_params; i++)
    {
        gse = &gsv->vector[param_index + i];
        if ((gse->err_code != USP_ERR_OK) && (gse->is_required))
        {
            param_name = si->param_settings[i]->param;
            AddUpdatedInstFailure_ParamErr(updated_inst_failure, param_name, gse->err_code, gse->err_msg);
        }
    }
}

/*********************************************************************//**
**
** PopulateSetResp_OperSuccess
**
** Adds an OperSuccess to a SetResp object
**
** \param   set_resp - pointer to set response message to populate
** \param   si - pointer to set_expr_info describing the expression that failed
** \param   gsv - group set vector containing all parameters to set that failed
**
** \return  None - this function puts any errors into the response message
**
**************************************************************************/
void PopulateSetResp_OperSuccess(Usp__SetResp *set_resp, set_expr_info_t *si, group_set_vector_t *gsv)
{
    int i, j;
    char *obj_path;
    int param_index;
    char *param_name;
    group_set_entry_t *gse;
    Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *oper_success;
    Usp__SetResp__UpdatedInstanceResult *updated_inst_res;

    // Add an OperSuccess to the SetResp
    oper_success = AddSetResp_OperSuccess(set_resp, si->requested_path);

    // Iterate over all resolved objects for this expression
    for (i=0; i < si->resolved_objs.num_entries; i++)
    {
        // Add an UpdatedInstResponse object for this resolved object
        obj_path = si->resolved_objs.vector[i];
        updated_inst_res = AddOperSuccess_UpdatedInstRes(oper_success, obj_path);

        // Iterate over all parameters set in the resolved object
        param_index = si->index + i*(si->num_params);
        for (j=0; j < si->num_params; j++)
        {
            gse = &gsv->vector[param_index + j];
            param_name = si->param_settings[j]->param;
            if (gse->err_code == USP_ERR_OK)
            {
                // The parameter was set successfully, so add it to the ParamMap
                AddUpdatedInstRes_ParamsEntry(updated_inst_res, param_name, gse->value);
            }
            else
            {
                // The parameter failed to be set, but was not required, so add it to the ParamErr list
                USP_ASSERT(gse->is_required == false);
                AddUpdatedInstRes_ParamErr(updated_inst_res, param_name, gse->err_code, gse->err_msg);
            }
        }
    }
}

/*********************************************************************//**
**
** DestroySetExprInfo
**
** Frees all memory associated with the specified set expr info array
**
** \param   set_expr_info - pointer to array of set expr info structures
** \param   num_set_expr - number of elements in the array
**
** \return  None
**
**************************************************************************/
void DestroySetExprInfo(set_expr_info_t *set_expr_info, int num_set_expr)
{
    int i;
    set_expr_info_t *si;

    // Exit if the set expr info array was never allocated - nothing to do
    if (set_expr_info == NULL)
    {
        return;
    }

    // Iterate over all elements of the array, freeing all dynamically allocated memory owned by the array
    for (i=0; i<num_set_expr; i++)
    {
        si = &set_expr_info[i];
        STR_VECTOR_Destroy(&si->resolved_objs);
        USP_SAFE_FREE(si->err_msg);
    }

    // Since all memory owned by the array has been freed, free the array itself
    USP_FREE(set_expr_info);
}

/*********************************************************************//**
**
** CreateSetResp
**
** Dynamically creates an GetResponse object
** NOTE: The object is created without any updated_obj_results
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the set request, which initiated this response
**
** \return  Pointer to a SetResponse object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateSetResp(char *msg_id)
{
    Usp__Msg *resp;
    Usp__Header *header;
    Usp__Body *body;
    Usp__Response *response;
    Usp__SetResp *set_resp;

    // Allocate and initialise memory to store the parts of the USP message
    resp = USP_MALLOC(sizeof(Usp__Msg));
    usp__msg__init(resp);

    header = USP_MALLOC(sizeof(Usp__Header));
    usp__header__init(header);

    body = USP_MALLOC(sizeof(Usp__Body));
    usp__body__init(body);

    response = USP_MALLOC(sizeof(Usp__Response));
    usp__response__init(response);

    set_resp = USP_MALLOC(sizeof(Usp__SetResp));
    usp__set_resp__init(set_resp);

    // Connect the structures together
    resp->header = header;
    header->msg_id = USP_STRDUP(msg_id);
    header->msg_type = USP__HEADER__MSG_TYPE__SET_RESP;

    resp->body = body;
    body->msg_body_case = USP__BODY__MSG_BODY_RESPONSE;
    body->response = response;
    response->resp_type_case = USP__RESPONSE__RESP_TYPE_SET_RESP;
    response->set_resp = set_resp;
    set_resp->n_updated_obj_results = 0;    // Start from an empty list
    set_resp->updated_obj_results = NULL;

    return resp;
}

/*********************************************************************//**
**
** AddSetResp_OperFailure
**
** Dynamically adds an operation failure object to the SetResponse object
**
** \param   resp - pointer to GetResponse object
** \param   path - requested path of object which failed to update
** \param   err_code - numeric code indicating reason object failed to be set
** \param   err_msg - error message indicating reason object failed to be set
**
** \return  Pointer to dynamically allocated operation failure object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *
AddSetResp_OperFailure(Usp__SetResp *set_resp, char *path, int err_code, char *err_msg)
{
    Usp__SetResp__UpdatedObjectResult *updated_obj_res;
    Usp__SetResp__UpdatedObjectResult__OperationStatus *oper_status;
    Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure;
    int new_num;    // new number of entries in the updated object result array

    // Allocate memory to store the updated object result
    updated_obj_res = USP_MALLOC(sizeof(Usp__SetResp__UpdatedObjectResult));
    usp__set_resp__updated_object_result__init(updated_obj_res);

    oper_status = USP_MALLOC(sizeof(Usp__SetResp__UpdatedObjectResult__OperationStatus));
    usp__set_resp__updated_object_result__operation_status__init(oper_status);

    oper_failure = USP_MALLOC(sizeof(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure));
    usp__set_resp__updated_object_result__operation_status__operation_failure__init(oper_failure);

    // Increase the size of the vector
    new_num = set_resp->n_updated_obj_results + 1;
    set_resp->updated_obj_results = USP_REALLOC(set_resp->updated_obj_results, new_num*sizeof(void *));
    set_resp->n_updated_obj_results = new_num;
    set_resp->updated_obj_results[new_num-1] = updated_obj_res;

    // Connect all objects together, and fill in their members
    updated_obj_res->requested_path = USP_STRDUP(path);
    updated_obj_res->oper_status = oper_status;

    oper_status->oper_status_case = USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE;
    oper_status->oper_failure = oper_failure;

    oper_failure->err_code = err_code;
    oper_failure->err_msg = USP_STRDUP(err_msg);
    oper_failure->n_updated_inst_failures = 0;
    oper_failure->updated_inst_failures = NULL;

    return oper_failure;
}

/*********************************************************************//**
**
** AddOperFailure_UpdatedInstFailure
**
** Dynamically adds an updated instance failure entry to an OperationFailure object
**
** \param   oper_failure - pointer to operation failure object to add this entry to
** \param   path - path to object which failed to be updated
**
** \return  Pointer to dynamically allocated updated instance result entry
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__UpdatedInstanceFailure *
AddOperFailure_UpdatedInstFailure(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure, char *path)
{
    Usp__SetResp__UpdatedInstanceFailure *updated_inst_failure;
    int new_num;    // new number of entries in the updated instance failure array
    int len;

    // Allocate memory to store the updated instance failure entry
    updated_inst_failure = USP_MALLOC(sizeof(Usp__SetResp__UpdatedInstanceFailure));
    usp__set_resp__updated_instance_failure__init(updated_inst_failure);

    // Increase the size of the vector
    new_num = oper_failure->n_updated_inst_failures + 1;
    oper_failure->updated_inst_failures = USP_REALLOC(oper_failure->updated_inst_failures, new_num*sizeof(void *));
    oper_failure->n_updated_inst_failures = new_num;
    oper_failure->updated_inst_failures[new_num-1] = updated_inst_failure;

    // Initialise the updated instance failure
    updated_inst_failure->n_param_errs = 0;
    updated_inst_failure->param_errs = NULL;

    // Add the object path with a trailing '.'
    len = strlen(path) + 2;   // Plus 2 to allow for adding a trailing '.' and NULL terminator
    updated_inst_failure->affected_path = USP_MALLOC(len);
    USP_SNPRINTF(updated_inst_failure->affected_path, len, "%s.", path);

    return updated_inst_failure;
}

/*********************************************************************//**
**
** AddUpdatedInstFailure_ParamErr
**
** Dynamically adds a param err entry to an updated instance failure object
**
** \param   updated_inst_failure - pointer to updated instance failure object to add this entry to
** \param   path - name of parameter which failed to update
** \param   err_code - error code representing the cause of the failure to update
** \param   err_msg - string representing the cause of the error
**
** \return  Pointer to dynamically allocated parameter_error entry
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__ParameterError *
AddUpdatedInstFailure_ParamErr(Usp__SetResp__UpdatedInstanceFailure *updated_inst_failure, char *path, int err_code, char *err_msg)
{
    Usp__SetResp__ParameterError *param_err_entry;
    int new_num;    // new number of entries in the param_err array

    // Allocate memory to store the param_err entry
    param_err_entry = USP_MALLOC(sizeof(Usp__SetResp__ParameterError));
    usp__set_resp__parameter_error__init(param_err_entry);

    // Increase the size of the vector
    new_num = updated_inst_failure->n_param_errs + 1;
    updated_inst_failure->param_errs = USP_REALLOC(updated_inst_failure->param_errs, new_num*sizeof(void *));
    updated_inst_failure->n_param_errs = new_num;
    updated_inst_failure->param_errs[new_num-1] = param_err_entry;

    // Initialise the param_err_entry
    param_err_entry->param = USP_STRDUP(path);
    param_err_entry->err_code = err_code;
    param_err_entry->err_msg = USP_STRDUP(err_msg);

    return param_err_entry;
}

/*********************************************************************//**
**
** AddSetResp_OperSuccess
**
** Dynamically adds an operation success object to the SetResponse object
**
** \param   resp - pointer to GetResponse object
** \param   path - requested path
**
** \return  Pointer to dynamically allocated operation success object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *
AddSetResp_OperSuccess(Usp__SetResp *set_resp, char *path)
{
    Usp__SetResp__UpdatedObjectResult *updated_obj_res;
    Usp__SetResp__UpdatedObjectResult__OperationStatus *oper_status;
    Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *oper_success;
    int new_num;    // new number of entries in the updated object result array

    // Allocate memory to store the updated object result
    updated_obj_res = USP_MALLOC(sizeof(Usp__SetResp__UpdatedObjectResult));
    usp__set_resp__updated_object_result__init(updated_obj_res);

    oper_status = USP_MALLOC(sizeof(Usp__SetResp__UpdatedObjectResult__OperationStatus));
    usp__set_resp__updated_object_result__operation_status__init(oper_status);

    oper_success = USP_MALLOC(sizeof(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess));
    usp__set_resp__updated_object_result__operation_status__operation_success__init(oper_success);

    // Increase the size of the vector
    new_num = set_resp->n_updated_obj_results + 1;
    set_resp->updated_obj_results = USP_REALLOC(set_resp->updated_obj_results, new_num*sizeof(void *));
    set_resp->n_updated_obj_results = new_num;
    set_resp->updated_obj_results[new_num-1] = updated_obj_res;

    // Connect all objects together, and fill in their members
    updated_obj_res->requested_path = USP_STRDUP(path);
    updated_obj_res->oper_status = oper_status;

    oper_status->oper_status_case = USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS;
    oper_status->oper_success = oper_success;

    oper_success->n_updated_inst_results = 0;
    oper_success->updated_inst_results = NULL;

    return oper_success;
}

/*********************************************************************//**
**
** AddOperSuccess_UpdatedInstRes
**
** Dynamically adds an updated instance result entry to an OperationSuccess object
**
** \param   oper_success - pointer to operation success object to add this entry to
** \param   path - path to object which was updated
**
** \return  Pointer to dynamically allocated updated instance result entry
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__UpdatedInstanceResult *
AddOperSuccess_UpdatedInstRes(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *oper_success, char *path)
{
    Usp__SetResp__UpdatedInstanceResult *updated_inst_result;
    int new_num;    // new number of entries in the updated instance result array
    int len;

    // Allocate memory to store the updated instance result entry
    updated_inst_result = USP_MALLOC(sizeof(Usp__SetResp__UpdatedInstanceResult));
    usp__set_resp__updated_instance_result__init(updated_inst_result);

    // Increase the size of the vector
    new_num = oper_success->n_updated_inst_results + 1;
    oper_success->updated_inst_results = USP_REALLOC(oper_success->updated_inst_results, new_num*sizeof(void *));
    oper_success->n_updated_inst_results = new_num;
    oper_success->updated_inst_results[new_num-1] = updated_inst_result;

    // Initialise the updated instance result
    updated_inst_result->n_updated_params = 0;
    updated_inst_result->updated_params = NULL;
    updated_inst_result->n_param_errs = 0;
    updated_inst_result->param_errs = NULL;

    // Add the object path with a trailing '.'
    len = strlen(path) + 2;   // Plus 2 to allow for adding a trailing '.' and NULL terminator
    updated_inst_result->affected_path = USP_MALLOC(len);
    USP_SNPRINTF(updated_inst_result->affected_path, len, "%s.", path);

    return updated_inst_result;
}

/*********************************************************************//**
**
** AddUpdatedInstRes_ParamsEntry
**
** Dynamically adds a param map entry to an updated instance result object
**
** \param   updated_inst_result - pointer to updated instance result object to add this entry to
** \param   key - name of the parameter which was updated successfully
** \param   value - value of the parameter which was updated
**
** \return  Pointer to dynamically allocated updated instance result entry
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__UpdatedInstanceResult__UpdatedParamsEntry *
AddUpdatedInstRes_ParamsEntry(Usp__SetResp__UpdatedInstanceResult *updated_inst_result, char *key, char *value)
{
    Usp__SetResp__UpdatedInstanceResult__UpdatedParamsEntry *entry;
    int new_num;    // new number of entries in the updated instance result array

    // Allocate memory to store the updated instance result entry
    entry = USP_MALLOC(sizeof(Usp__SetResp__UpdatedInstanceResult__UpdatedParamsEntry));
    usp__set_resp__updated_instance_result__updated_params_entry__init(entry);

    // Increase the size of the vector
    new_num = updated_inst_result->n_updated_params + 1;
    updated_inst_result->updated_params = USP_REALLOC(updated_inst_result->updated_params , new_num*sizeof(void *));
    updated_inst_result->n_updated_params = new_num;
    updated_inst_result->updated_params[new_num-1] = entry;

    // Initialise the result param map entry
    entry->key = USP_STRDUP(key);
    entry->value = USP_STRDUP(value);

    return entry;
}

/*********************************************************************//**
**
** AddUpdatedInstRes_ParamErr
**
** Dynamically adds a param err entry to an updated instance result object
**
** \param   updated_inst_result - pointer to updated instance result object to add this entry to
** \param   path - name of parameter which failed to update
** \param   err_code - error code representing the cause of the failure to update
** \param   err_msg - string representing the cause of the error
**
** \return  Pointer to dynamically allocated parameter_error entry
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__SetResp__ParameterError *
AddUpdatedInstRes_ParamErr(Usp__SetResp__UpdatedInstanceResult *updated_inst_result, char *path, int err_code, char *err_msg)
{
    Usp__SetResp__ParameterError *param_err_entry;
    int new_num;    // new number of entries in the param_err array

    // Allocate memory to store the param_err entry
    param_err_entry = USP_MALLOC(sizeof(Usp__SetResp__ParameterError));
    usp__set_resp__parameter_error__init(param_err_entry);

    // Increase the size of the vector
    new_num = updated_inst_result->n_param_errs + 1;
    updated_inst_result->param_errs = USP_REALLOC(updated_inst_result->param_errs, new_num*sizeof(void *));
    updated_inst_result->n_param_errs = new_num;
    updated_inst_result->param_errs[new_num-1] = param_err_entry;

    // Initialise the param_err_entry
    param_err_entry->param = USP_STRDUP(path);
    param_err_entry->err_code = err_code;
    param_err_entry->err_msg = USP_STRDUP(err_msg);

    return param_err_entry;
}

