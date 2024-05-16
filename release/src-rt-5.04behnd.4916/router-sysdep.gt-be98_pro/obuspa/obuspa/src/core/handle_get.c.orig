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
 * \file handle_get.c
 *
 * Handles the GetRequest message, creating a GetResponse
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
#include "group_get_vector.h"

//------------------------------------------------------------------------------
// Structure used to marshall entries in get group vector for a path expression
typedef struct
{
    int index;              // start index of parameters in get group vector for this path expression
    int num_entries;        // number of entries in the get group vector for this path expression
    int err_code;           // error code if path resolution failed for this path expression
    char *err_msg;          // error message if path resolution failed for this path expression
} get_expr_info_t;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ExpandGetPathExpression(int get_expr_index, char *path_expr, int depth, get_expr_info_t *gi, group_get_vector_t *ggv);
void FormPathExprResponse(int get_expr_index, char *path_expr, get_expr_info_t *gi, group_get_vector_t *ggv, Usp__Msg *resp);
void AddResolvedPathResult(Usp__GetResp__RequestedPathResult *req_path_result, char *path, char *value);
Usp__GetResp__ResolvedPathResult *FindResolvedPath(Usp__GetResp__RequestedPathResult *req_path_result, char *obj_path);
Usp__Msg *CreateGetResp(char *msg_id);
Usp__GetResp__RequestedPathResult *AddGetResp_ReqPathRes(Usp__Msg *resp, char *requested_path, int err_code, char *err_msg);
Usp__GetResp__ResolvedPathResult *AddReqPathRes_ResolvedPathResult(Usp__GetResp__RequestedPathResult *req_path_result, char *obj_path);

Usp__GetResp__ResolvedPathResult__ResultParamsEntry *
AddResolvedPathRes_ParamsEntry(Usp__GetResp__ResolvedPathResult *resolved_path_res, char *param_name, char *value);

/*********************************************************************//**
**
** MSG_HANDLER_HandleGet
**
** Handles a USP Get message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mrt - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleGet(Usp__Msg *usp, char *controller_endpoint, mtp_reply_to_t *mrt)
{
    int i;              // Used to iterate over path expressions in the USP get request message
    char **path_exprs;
    int num_path_expr;
    Usp__Msg *resp = NULL;
    int size;
    int depth;
    group_get_vector_t ggv;
    get_expr_info_t *get_expr_info;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    USP_ASSERT(usp->header != NULL);
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_GET) ||
        (usp->body->request->get == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp, NULL);
        goto exit;
    }

    // Create a Get Response message
    resp = CreateGetResp(usp->header->msg_id);

    // Exit if there are no parameters to get
    path_exprs = usp->body->request->get->param_paths;
    num_path_expr = usp->body->request->get->n_param_paths;
    if ((path_exprs == NULL) || (num_path_expr == 0))
    {
        goto exit;
    }

    // Calculate the number of hierarchical levels to traverse in the data model when performing partial path resolution
    depth = usp->body->request->get->max_depth;
    if (depth == 0)
    {
        depth = FULL_DEPTH;
    }

    // Allocate vector to store marshalling info for each path expression
    size = num_path_expr*sizeof(get_expr_info_t);
    get_expr_info = USP_MALLOC(size);
    memset(get_expr_info, 0, size);

    // Iterate over all input get expressions, adding them to the get_expr_info and group get vectors
    GROUP_GET_VECTOR_Init(&ggv);
    for (i=0; i < num_path_expr; i++)
    {
        ExpandGetPathExpression(i, path_exprs[i], depth, &get_expr_info[i], &ggv);
    }

    // Get all parameters
    GROUP_GET_VECTOR_GetValues(&ggv);

    // Iterate over all input get expressions, consulting the group get vector to form the USP Get response message
    for (i=0; i < num_path_expr; i++)
    {
        FormPathExprResponse(i, path_exprs[i], &get_expr_info[i], &ggv, resp);
    }

    GROUP_GET_VECTOR_Destroy(&ggv);

    // Clean up get_expr_info vector
    for (i=0; i < num_path_expr; i++)
    {
        USP_SAFE_FREE(get_expr_info[i].err_msg);
    }
    USP_FREE(get_expr_info);

exit:
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mrt);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** ExpandGetPathExpression
**
** Expands the specified path expression into the group get vector and get_expr_info vectors
**
** \param   get_expr_index - index of the path expression in the USP Get request message
** \param   path_expr - USP path expression specifying which parameters to get
** \param   depth - Number of hierarchical levels to traverse in the data model when performing partial path resolution
** \param   gi - pointer to info about specified path expression
** \param   ggv - pointer to group get vector to add params found in this path expression to
**
** \return  None
**
**************************************************************************/
void ExpandGetPathExpression(int get_expr_index, char *path_expr, int depth, get_expr_info_t *gi, group_get_vector_t *ggv)
{
    int err;
    str_vector_t params;
    int_vector_t group_ids;
    combined_role_t combined_role;

    // Exit if the search path is not in the schema or the search path was invalid or an error occurred in evaluating the search path (eg a parameter get failed)
    // The get response will contain an error message in this case
    STR_VECTOR_Init(&params);
    INT_VECTOR_Init(&group_ids);
    MSG_HANDLER_GetMsgRole(&combined_role);
    err = PATH_RESOLVER_ResolveDevicePath(path_expr, &params, &group_ids, kResolveOp_Get, depth, &combined_role, 0);
    if (err != USP_ERR_OK)
    {
        gi->err_code = err;
        gi->err_msg = USP_STRDUP(USP_ERR_GetMessage());
        STR_VECTOR_Destroy(&params);
        INT_VECTOR_Destroy(&group_ids);
        return;
    }

    // Save the range of indexes for this path expression
    gi->index = ggv->num_entries;
    gi->num_entries = params.num_entries;

    // Exit if no params were found
    if (params.num_entries == 0)
    {
        STR_VECTOR_Destroy(&params);
        INT_VECTOR_Destroy(&group_ids);
        return;
    }

    // Move these params and group_ids to the group get vector
    // NOTE: Ownership of the strings in the params vector transfers to the group get vector
    GROUP_GET_VECTOR_AddParams(ggv, &params, &group_ids);

    // Clean up
    INT_VECTOR_Destroy(&group_ids);

    // Since we have moved the contents of the params vector to the get group vector, we can just free the params vector (not its content)
    USP_SAFE_FREE(params.vector);
}

/*********************************************************************//**
**
** FormPathExprResponse
**
** Forms the USP response for the specified path expression
**
** \param   get_expr_index - index order of the path expression in the USP get request
** \param   path_expr - USP path expression specifying which parameters to get
** \param   gi - pointer to info about specified path expression
** \param   ggv - pointer to group get vector containing paths and values of params which were retrieved
** \param   resp - pointer to GetResponse object
**
** \return  None
**
**************************************************************************/
void FormPathExprResponse(int get_expr_index, char *path_expr, get_expr_info_t *gi, group_get_vector_t *ggv, Usp__Msg *resp)
{
    int i;
    Usp__GetResp__RequestedPathResult *req_path_result;
    group_get_entry_t *gge;

    // Exit if the path resolution failed for this path expression, putting the error message in the get response
    if (gi->err_code != USP_ERR_OK)
    {
        (void)AddGetResp_ReqPathRes(resp, path_expr, gi->err_code, gi->err_msg);
        return;
    }

    // Exit if this path expression failed to resolve to any parameters. In this case just add a requested path response
    if (gi->num_entries == 0)
    {
        (void)AddGetResp_ReqPathRes(resp, path_expr, USP_ERR_OK, "");
        return;
    }

    // If there was an error in getting any of the parameters associated with the path expression,
    // then just add the first error, without any of the parameter values, for this path expression result
    for (i=0; i < gi->num_entries; i++)
    {
        gge = &ggv->vector[gi->index + i];
        if (gge->err_code != USP_ERR_OK)
        {
            (void)AddGetResp_ReqPathRes(resp, path_expr, gge->err_code, gge->err_msg);
            return;
        }
    }

    // If the code gets here, then the value of all parameters were retrieved successfully, so add their values to the result_params
    req_path_result = AddGetResp_ReqPathRes(resp, path_expr, USP_ERR_OK, "");
    for (i=0; i < gi->num_entries; i++)
    {
        gge = &ggv->vector[gi->index + i];

        // Simple format contains a resolved_path_result for every object (and sub object)
        AddResolvedPathResult(req_path_result, gge->path, gge->value);
    }
}

/*********************************************************************//**
**
** AddResolvedPathResult
**
** Adds the specified path to the resolved_path_result list
** This function creates a resolved_path_result entry for the parent object
** of the parameter, before adding the parameter to the result_params
**
** \param   req_path_result - pointer to requested_path_result to add this entry to
** \param   path - full data model path of the parameter
** \param   value - value of the parameter
**
** \return  None
**
**************************************************************************/
void AddResolvedPathResult(Usp__GetResp__RequestedPathResult *req_path_result, char *path, char *value)
{
    char obj_path[MAX_DM_PATH];
    char *param_name;
    Usp__GetResp__ResolvedPathResult *resolved_path_res;

    // Split the parameter into the parent object path and the name of the parameter within the object
    param_name = TEXT_UTILS_SplitPath(path, obj_path, sizeof(obj_path));

    // Add a resolved path result, if we don't already have one for the specified parent object
    resolved_path_res = FindResolvedPath(req_path_result, obj_path);
    if (resolved_path_res == NULL)
    {
        resolved_path_res = AddReqPathRes_ResolvedPathResult(req_path_result, obj_path);
    }

    // Add the parameter to the params
    AddResolvedPathRes_ParamsEntry(resolved_path_res, param_name, value);
}

/*********************************************************************//**
**
** FindResolvedPath
**
** Searches for the resolved path object which represents the specified object_path
**
** \param   req_path_result - pointer to requested_path_result to look for the specified object path in
** \param   obj_path - path to object in data model
**
** \return  Pointer to a ResolvedPath object, or NULL if no match was found
**
**************************************************************************/
Usp__GetResp__ResolvedPathResult *FindResolvedPath(Usp__GetResp__RequestedPathResult *req_path_result, char *obj_path)
{
    int i;
    int num_entries;
    int index;
    Usp__GetResp__ResolvedPathResult *resolved_path_result;

    // Determine limits of backwards search for matching object path
    // NOTE: The limit is fairly arbitrary. It is a balance between minimizing the USP response message size
    //       (trying to prevent any ResolvedPathResults with duplicate ResolvedPaths) and spending too much time
    //       searching the USP response to avoid duplicates. Should the limit not be sufficient, it is acceptable
    //       for USP Responses to contain ResolvedPathResults with duplicate ResolvedPaths.
    #define RESOLVED_PATH_SEARCH_LIMIT 10
    num_entries = req_path_result->n_resolved_path_results;
    index = num_entries - RESOLVED_PATH_SEARCH_LIMIT;
    if (index < 0)
    {
        index = 0;
    }

    // Search backwards, trying to find the one which matches the specified object path
    for (i=num_entries-1; i>=index; i--)
    {
        resolved_path_result = req_path_result->resolved_path_results[i];
        if (strcmp(resolved_path_result->resolved_path, obj_path)==0)
        {
            return resolved_path_result;
        }
    }

    // If the code gets here, then no matching object path was found
    return NULL;
}

/*********************************************************************//**
**
** CreateGetResp
**
** Dynamically creates an GetResponse object
** NOTE: The object is created without any requested_path_results
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the get request, which initiated this response
**
** \return  Pointer to a GetResponse object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateGetResp(char *msg_id)
{
    Usp__Msg *resp;
    Usp__Header *header;
    Usp__Body *body;
    Usp__Response *response;
    Usp__GetResp *get_resp;

    // Allocate memory to store the USP message
    resp = USP_MALLOC(sizeof(Usp__Msg));
    usp__msg__init(resp);

    header = USP_MALLOC(sizeof(Usp__Header));
    usp__header__init(header);

    body = USP_MALLOC(sizeof(Usp__Body));
    usp__body__init(body);

    response = USP_MALLOC(sizeof(Usp__Response));
    usp__response__init(response);

    get_resp = USP_MALLOC(sizeof(Usp__GetResp));
    usp__get_resp__init(get_resp);

    // Connect the structures together
    resp->header = header;
    header->msg_id = USP_STRDUP(msg_id);
    header->msg_type = USP__HEADER__MSG_TYPE__GET_RESP;

    resp->body = body;
    body->msg_body_case = USP__BODY__MSG_BODY_RESPONSE;
    body->response = response;
    response->resp_type_case = USP__RESPONSE__RESP_TYPE_GET_RESP;
    response->get_resp = get_resp;
    get_resp->n_req_path_results = 0;    // Start from an empty response list
    get_resp->req_path_results = NULL;

    return resp;
}

/*********************************************************************//**
**
** AddGetResp_ReqPathRes
**
** Dynamically adds a requested path result to the GetResponse object
** NOTE: The object is created without any entries in the result_params
**
** \param   resp - pointer to GetResponse object
** \param   requested_path - string containing one of the path expresssions from the Get request
** \param   err_code - numeric code indicating reason the get failed
** \param   err_msg - error message indicating reason the get failed
**
** \return  Pointer to dynamically allocated requested path result
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__GetResp__RequestedPathResult *
AddGetResp_ReqPathRes(Usp__Msg *resp, char *requested_path, int err_code, char *err_msg)
{
    Usp__GetResp *get_resp;
    Usp__GetResp__RequestedPathResult *req_path_result;
    int new_num;    // new number of requested_path_results

    // Allocate memory to store the requested_path_result
    req_path_result = USP_MALLOC(sizeof(Usp__GetResp__RequestedPathResult));
    usp__get_resp__requested_path_result__init(req_path_result);

    // Increase the size of the vector containing pointers to the requested_path_results
    get_resp = resp->body->response->get_resp;
    new_num = get_resp->n_req_path_results + 1;
    get_resp->req_path_results = USP_REALLOC(get_resp->req_path_results, new_num*sizeof(void *));
    get_resp->n_req_path_results = new_num;
    get_resp->req_path_results[new_num-1] = req_path_result;

    // Initialise the requested_path_result
    req_path_result->requested_path = USP_STRDUP(requested_path);
    req_path_result->err_code = err_code;
    req_path_result->err_msg = USP_STRDUP(err_msg);
    req_path_result->n_resolved_path_results = 0;     // Start from an empty list
    req_path_result->resolved_path_results = NULL;

    return req_path_result;
}

/*********************************************************************//**
**
** AddReqPathRes_ResolvedPathResult
**
** Dynamically adds a resolved_path_result object to a requested_path_result object
**
** \param   req_path_result - pointer to requested_path_result to add this entry to
** \param   obj_path - data model path of the object to add to the map
**
** \return  Pointer to dynamically allocated resolved_path_result
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__GetResp__ResolvedPathResult *AddReqPathRes_ResolvedPathResult(Usp__GetResp__RequestedPathResult *req_path_result, char *obj_path)
{
    Usp__GetResp__ResolvedPathResult *resolved_path_res_entry;

    int new_num;    // new number of entries in the result_params

    // Allocate memory to store the resolved_path_result entry
    resolved_path_res_entry = USP_MALLOC(sizeof(Usp__GetResp__ResolvedPathResult));
    usp__get_resp__resolved_path_result__init(resolved_path_res_entry);

    // Increase the size of the vector containing pointers to the map entries
    new_num = req_path_result->n_resolved_path_results + 1;
    req_path_result->resolved_path_results = USP_REALLOC(req_path_result->resolved_path_results, new_num*sizeof(void *));
    req_path_result->n_resolved_path_results = new_num;
    req_path_result->resolved_path_results[new_num-1] = resolved_path_res_entry;

    // Initialise the resolved_path_result
    resolved_path_res_entry->resolved_path = USP_STRDUP(obj_path);
    resolved_path_res_entry->n_result_params = 0;
    resolved_path_res_entry->result_params = NULL;

    return resolved_path_res_entry;
}

/*********************************************************************//**
**
** AddResolvedPathRes_ParamsEntry
**
** Dynamically adds a result_params entry to a resolved_path_result object
**
** \param   resolved_path_res - pointer to resolved_oath_result to add this entry to
** \param   param_name - name of the parameter (not including object path) of the parameter to add to the map
** \param   value - value of the parameter
**
** \return  Pointer to dynamically allocated result_params
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__GetResp__ResolvedPathResult__ResultParamsEntry *
AddResolvedPathRes_ParamsEntry(Usp__GetResp__ResolvedPathResult *resolved_path_res, char *param_name, char *value)
{
    Usp__GetResp__ResolvedPathResult__ResultParamsEntry *res_params_entry;

    int new_num;    // new number of entries in the result_params

    // Allocate memory to store the result_params entry
    res_params_entry = USP_MALLOC(sizeof(Usp__GetResp__ResolvedPathResult__ResultParamsEntry));
    usp__get_resp__resolved_path_result__result_params_entry__init(res_params_entry);

    // Increase the size of the vector containing pointers to the map entries
    new_num = resolved_path_res->n_result_params + 1;
    resolved_path_res->result_params = USP_REALLOC(resolved_path_res->result_params, new_num*sizeof(void *));
    resolved_path_res->n_result_params = new_num;
    resolved_path_res->result_params[new_num-1] = res_params_entry;

    // Initialise the result_params_entry
    res_params_entry->key = USP_STRDUP(param_name);
    res_params_entry->value = USP_STRDUP(value);

    return res_params_entry;
}




























