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
 * \file msg_utils.c
 *
 * Common message handling utility functions called by USP Broker and/or USP Service functionality
 *
 */

#include "common_defs.h"
#include "msg_utils.h"
#include "text_utils.h"
#include "msg_handler.h"

#if !defined(REMOVE_USP_BROKER) || !defined(REMOVE_USP_SERVICE)
//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
Usp__Set__UpdateObject *FindUpdateObject(Usp__Set *set, char *obj_path);
Usp__Set__UpdateObject *AddSetReq_UpdateObject(Usp__Set *set, char *obj_path);
Usp__Set__UpdateParamSetting *AddUpdateObject_ParamSettings(Usp__Set__UpdateObject *update_object, char *param_name, char *value);
void AddSetReq_Param(Usp__Set *set, char *path, char *value);

/*********************************************************************//**
**
** MSG_UTILS_AddSetReq_Param
**
** Adds the specified parameter to the Set Request
**
** \param   set - pointer to Set request to add the parameter to
** \param   path - data model path of the parameter
** \param   value - new value of the parameter to set
**
** \return  None
**
**************************************************************************/
void MSG_UTILS_AddSetReq_Param(Usp__Set *set, char *path, char *value)
{
    char obj_path[MAX_DM_PATH];
    char *param_name;
    Usp__Set__UpdateObject *update_object;

    // Split the parameter into the parent object path and the name of the parameter within the object
    param_name = TEXT_UTILS_SplitPath(path, obj_path, sizeof(obj_path));

    // Add an update object, if we don't already have one for the specified parent object
    update_object = FindUpdateObject(set, obj_path);
    if (update_object == NULL)
    {
        update_object = AddSetReq_UpdateObject(set, obj_path);
    }

    // Add the parameter to the param settings
    AddUpdateObject_ParamSettings(update_object, param_name, value);
}

/*********************************************************************//**
**
** MSG_UTILS_ValidateUspResponse
**
** Validates that the USP Message received from the USP Service was of the expected type
** This function is called as part of the group get/set/add/delete handlers for querying USP services
**
** \param   resp - USP response message in protobuf-c structure
** \param   response_type - type of USP response expected
** \param   param_errs_path - pointer to variable in which to return a pointer to the first path that failed (if the USP Message was an Error response)
**                            or NULL if the caller does not require this information
**                            NOTE: Ownership of the pointer returned in this variable stays with the parsed USP Message
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_UTILS_ValidateUspResponse(Usp__Msg *resp, Usp__Response__RespTypeCase response_type, char **param_errs_path)
{
    int err;
    Usp__Body *body;
    Usp__Response *response;
    Usp__Error__ParamError *pe;
    char *err_msg;

    // NOTE: Message header has already been validated by DM_EXEC_SendRequestAndWaitForResponse(), so not doing it again here
    if (param_errs_path != NULL)
    {
        *param_errs_path = NULL;        // Set default return value
    }

    // Exit if body is missing
    body = resp->body;
    if (body == NULL)
    {
        USP_ERR_SetMessage("%s: Missing message body", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the USP Service returned an ERROR to our request
    if (body->msg_body_case == USP__BODY__MSG_BODY_ERROR)
    {
        err = USP_ERR_INTERNAL_ERROR;  // default error code, if none present in error object

        // Use error code from error object, if present
        if ((body->error != NULL) && (body->error->err_code != USP_ERR_OK))
        {
            err = body->error->err_code;
            err_msg = body->error->err_msg;

            // Use more specific error code from param_errs object, if present
            if ((body->error->n_param_errs > 0) && (body->error->param_errs != NULL) && (body->error->param_errs[0] != NULL))
            {
                pe = body->error->param_errs[0];
                err = pe->err_code;
                if (pe->err_msg != NULL)
                {
                    err_msg = pe->err_msg;
                }

                if (param_errs_path != NULL)
                {
                    *param_errs_path = pe->param_path;
                }
            }

            USP_ERR_SetMessage("%s", err_msg);
        }

        return err;
    }

    // Exit if the USP Service didn't return a Response message to our request
    if (body->msg_body_case != USP__BODY__MSG_BODY_RESPONSE)
    {
        USP_ERR_SetMessage("%s: Received Unexpected message type %d (Expected a Response)", __FUNCTION__, body->msg_body_case);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if response is missing
    response = body->response;
    if (response == NULL)
    {
        USP_ERR_SetMessage("%s: Missing response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }


    // Exit if response is the expected type response
    if (response->resp_type_case != response_type)
    {
        USP_ERR_SetMessage("%s: Received Unexpected response type %d (Expected %d)", __FUNCTION__, response->resp_type_case, response_type);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** MSG_UTILS_Create_SetReq
**
** Create a USP Set request message
**
** \param   msg_id - string containing unique USP message ID to use for the request
** \param   kvv - pointer to key-value vector containing the parameters to get as the key, and the values to set as the value
**
** \return  Pointer to a Set Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_SetReq(char *msg_id, kv_vector_t *kvv)
{
    int i;
    Usp__Msg *msg;
    Usp__Set *set;
    kv_pair_t *kv;

    // Create Set Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__SET, USP__REQUEST__REQ_TYPE_SET);
    set = USP_MALLOC(sizeof(Usp__Set));
    usp__set__init(set);
    msg->body->request->set = set;

    // Initialise the set with initially no UpdateObjects
    set->allow_partial = false;
    set->n_update_objs = 0;
    set->update_objs = NULL;

    // Iterate over all parameters, adding them to the Set request
    for (i=0; i < kvv->num_entries; i++)
    {
        kv = &kvv->vector[i];
        AddSetReq_Param(set, kv->key, kv->value);
    }

    return msg;
}

/*********************************************************************//**
**
** MSG_UTILS_Create_GetReq
**
** Private function to construct a GET request USP message from a list of keys
**
** \param   msg_id - string containing unique USP message ID to use for the request
** \param   kvv - key value vector containing the keys and values to get
**
** \return  Pointer to a Usp__Msg structure- ownership passes to the caller
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_GetReq(char *msg_id, kv_vector_t *kvv)
{
    int i;
    int num_paths;
    Usp__Msg *msg;
    Usp__Get *get;

    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__GET, USP__REQUEST__REQ_TYPE_GET);
    get = USP_MALLOC(sizeof(Usp__Get));
    usp__get__init(get);
    msg->body->request->get = get;

    // Copy the paths into the Get
    num_paths = kvv->num_entries;
    get->n_param_paths = num_paths;
    get->param_paths = USP_MALLOC(num_paths*sizeof(char *));
    for (i=0; i<num_paths; i++)
    {
        get->param_paths[i] = USP_STRDUP(kvv->vector[i].key);
    }

    get->max_depth = 0;

    return msg;
}

/*********************************************************************//**
**
** MSG_UTILS_Create_AddReq
**
** Create a USP Add request message
**
** \param   msg_id - string containing unique USP message ID to use for the request
** \param   path - unqualified path of the object to add an instance to
** \param   params - Array containing initial values of the object's child parameters, or NULL if there are none to set
** \param   num_params - Number of child parameters to set
**
** \return  Pointer to an Add Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_AddReq(char *msg_id, char *path, group_add_param_t *params, int num_params)
{
    Usp__Msg *msg;
    Usp__Add *add;
    Usp__Add__CreateObject *create_obj;
    Usp__Add__CreateParamSetting *cps;
    group_add_param_t *p;
    int i;

    // Create Add Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__ADD, USP__REQUEST__REQ_TYPE_ADD);
    add = USP_MALLOC(sizeof(Usp__Add));
    usp__add__init(add);
    msg->body->request->add = add;

    // Fill in Add object
    add->allow_partial = false;
    add->n_create_objs = 1;
    add->create_objs = USP_MALLOC(sizeof(void *));

    create_obj = USP_MALLOC(sizeof(Usp__Add__CreateObject));
    usp__add__create_object__init(create_obj);
    add->create_objs[0] = create_obj;

    create_obj->obj_path = USP_STRDUP(path);

    // Exit if there are no parameters to set in this object
    if ((params==NULL) || (num_params == 0))
    {
        create_obj->n_param_settings = 0;
        create_obj->param_settings = NULL;
        return msg;
    }

    // Add all of the objects parameters initial values
    create_obj->n_param_settings = num_params;
    create_obj->param_settings = USP_MALLOC(num_params * sizeof(void *));

    for (i=0; i<num_params; i++)
    {
        cps = USP_MALLOC(sizeof(Usp__Add__CreateParamSetting));
        usp__add__create_param_setting__init(cps);
        create_obj->param_settings[i] = cps;

        p = &params[i];
        cps->param = USP_STRDUP(p->param_name);
        cps->value = USP_STRDUP(p->value);
        cps->required = p->is_required;
    }

    return msg;
}

/*********************************************************************//**
**
** MSG_UTILS_Create_DeleteReq
**
** Create a USP Delete request message containing multiple instances to delete
**
** \param   msg_id - string containing unique USP message ID to use for the request
** \param   paths - pointer to vector containing the list of data model objects to delete
**                  NOTE: All object paths must be absolute (no wildcards etc)
** \param   allow_partial - if set to true, then a failure to delete any object in the vector should result in no objects being deleted
**
** \return  Pointer to a Delete Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_DeleteReq(char *msg_id, str_vector_t *paths, bool allow_partial)
{
    int i;
    Usp__Msg *msg;
    Usp__Delete *del;
    int num_entries;

    // Create Delete Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__DELETE, USP__REQUEST__REQ_TYPE_DELETE);
    del = USP_MALLOC(sizeof(Usp__Delete));
    usp__delete__init(del);
    msg->body->request->delete_ = del;

    // Fill in Delete object
    num_entries = paths->num_entries;
    del->allow_partial = allow_partial;
    del->n_obj_paths = num_entries;
    del->obj_paths = USP_MALLOC(num_entries*sizeof(char *));

    // Copy across the object instances to delete
    for (i=0; i<num_entries; i++)
    {
        del->obj_paths[i] = USP_STRDUP(paths->vector[i]);
    }

    return msg;
}

/*********************************************************************//**
**
** MSG_UTILS_Create_GetSupportedDMReq
**
** Create a USP GetSupportedDM request message
**
** \param   msg_id - string containing the message id to use for the request
** \param   sv - pointer to string vector containing the paths to query
**
** \return  Pointer to a GetSupportedDM Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_GetSupportedDMReq(char *msg_id, str_vector_t *sv)
{
    int i;
    int num_paths;
    Usp__Msg *msg;
    Usp__GetSupportedDM *gsdm;

    // Create GSDM Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM, USP__REQUEST__REQ_TYPE_GET_SUPPORTED_DM);
    gsdm = USP_MALLOC(sizeof(Usp__GetSupportedDM));
    usp__get_supported_dm__init(gsdm);
    msg->body->request->get_supported_dm = gsdm;

    // Copy the paths into the GSDM
    num_paths = sv->num_entries;
    gsdm->n_obj_paths = num_paths;
    gsdm->obj_paths = USP_MALLOC(num_paths*sizeof(char *));
    for (i=0; i<num_paths; i++)
    {
        gsdm->obj_paths[i] = USP_STRDUP(sv->vector[i]);
    }

    // Fill in the flags in the GSDM
    gsdm->first_level_only = false;
    gsdm->return_commands = true;
    gsdm->return_events = true;
    gsdm->return_params = true;

    return msg;
}

/*********************************************************************//**
**
** CreateBroker_GetInstancesReq
**
** Create a USP GetInstances request message
**
** \param   msg_id - string containing the message id to use for the request
** \param   sv - pointer to string vector containing the top level data model object paths to recursively get all child instances of
**
** \return  Pointer to a GetInstances Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_GetInstancesReq(char *msg_id, str_vector_t *sv)
{
    int i;
    Usp__Msg *msg;
    Usp__GetInstances *geti;
    int num_entries;

    // Create GetInstances Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__GET_INSTANCES, USP__REQUEST__REQ_TYPE_GET_INSTANCES);
    geti = USP_MALLOC(sizeof(Usp__GetInstances));
    usp__get_instances__init(geti);
    msg->body->request->get_instances = geti;

    // Copy the paths into the GetInstances
    num_entries = sv->num_entries;
    geti->n_obj_paths = num_entries;
    geti->obj_paths = USP_MALLOC(num_entries*sizeof(char *));
    for (i=0; i<num_entries; i++)
    {
        geti->obj_paths[i] = USP_STRDUP(sv->vector[i]);
    }

    // Get all child instances
    geti->first_level_only = false;

    return msg;
}


/*********************************************************************//**
**
** MSG_UTILS_Create_OperateReq
**
** Create a USP Operate request message
**
** \param   msg_id - string containing the message id to use for the request
** \param   path - data model path of USP command
** \param   command_key - Key identifying the command in the Request table of the USP Service
** \param   input_args - pointer to key-value vector containing the input arguments and their values
**
** \return  Pointer to a Operate Request object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *MSG_UTILS_Create_OperateReq(char *msg_id, char *path, char *command_key, kv_vector_t *input_args)
{
    int i;
    Usp__Msg *msg;
    Usp__Operate *oper;
    int num_entries;
    Usp__Operate__InputArgsEntry *arg;
    kv_pair_t *kv;

    // Create Operate Request
    msg =  MSG_HANDLER_CreateRequestMsg(msg_id, USP__HEADER__MSG_TYPE__OPERATE, USP__REQUEST__REQ_TYPE_OPERATE);
    oper = USP_MALLOC(sizeof(Usp__Operate));
    usp__operate__init(oper);
    msg->body->request->operate = oper;

    // Fill in Operate object
    oper->command = USP_STRDUP(path);
    oper->command_key = USP_STRDUP(command_key);
    oper->send_resp = true;

    // Create input args
    num_entries = input_args->num_entries;
    oper->n_input_args = num_entries;
    oper->input_args = USP_MALLOC(num_entries*sizeof(Usp__Operate__InputArgsEntry *));

    // Copy across the input args
    for (i=0; i<num_entries; i++)
    {
        kv = &input_args->vector[i];
        arg = USP_MALLOC(sizeof(Usp__Operate__InputArgsEntry));
        usp__operate__input_args_entry__init(arg);

        arg->key = USP_STRDUP(kv->key);
        arg->value = USP_STRDUP(kv->value);
        oper->input_args[i] = arg;
    }

    return msg;
}

/*********************************************************************//**
**
** MSG_UTILS_ProcessUspService_GetResponse
**
** Processes a Get Response that we have received from a USP Service.  This
** function populates the returned kvv structure with all response parameters
** found in the USP response message without performing any filtering.
**
** \param   resp - USP response message in protobuf-c structure
** \param   kvv - key-value vector in which to return the parameter values
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int MSG_UTILS_ProcessUspService_GetResponse(Usp__Msg *resp, kv_vector_t *kvv)
{
    int i;
    int objIndex;
    int paramIndex;
    int err = USP_ERR_OK;
    Usp__GetResp *get;
    Usp__GetResp__RequestedPathResult *rpr;
    Usp__GetResp__ResolvedPathResult *res;
    Usp__GetResp__ResolvedPathResult__ResultParamsEntry *rpe;
    char path[MAX_DM_PATH];

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_GET_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if get response is missing
    get = resp->body->response->get_resp;
    if (get == NULL)
    {
        USP_ERR_SetMessage("%s: Missing get response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all requested path results
    USP_ASSERT((get->n_req_path_results==0) || (get->req_path_results != NULL));
    for (i=0; i < get->n_req_path_results; i++)
    {
        rpr = get->req_path_results[i];
        USP_ASSERT(rpr != NULL)

        // Exit if we received an error for this path
        if (rpr->err_code != USP_ERR_OK)
        {
            USP_ERR_SetMessage("Failed to get '%s' (err_msg=%s)", rpr->requested_path, rpr->err_msg);
            KV_VECTOR_Destroy(kvv);
            err = rpr->err_code;
            goto exit;
        }

        // Iterate over all data model objects resolved for this path
        for (objIndex = 0 ; objIndex < rpr->n_resolved_path_results ; objIndex++)
        {
             // Iterate over all data model parameters resolved for this object
            res  = rpr->resolved_path_results[objIndex];
            for (paramIndex = 0 ; paramIndex < res->n_result_params ; paramIndex++)
            {
                rpe = res->result_params[paramIndex];
                USP_ASSERT((rpe != NULL) && (rpe->value != NULL));

                // Add the full path and parameter value in the returned key-value vector
                USP_SNPRINTF(path, MAX_DM_PATH, "%s%s", res->resolved_path, rpe->key);
                KV_VECTOR_Add(kvv, path, rpe->value);
            }
        }
    }

exit:
    return err;
}

/*********************************************************************//**
**
** FindUpdateObject
**
** Searches for the update object in a Set Request, for the specified object_path
**
** \param   set - pointer to Set request to look for the specified object path in
** \param   obj_path - path to object in data model
**
** \return  Pointer to a ResolvedPath object, or NULL if no match was found
**
**************************************************************************/
Usp__Set__UpdateObject *FindUpdateObject(Usp__Set *set, char *obj_path)
{
    int i;
    int num_entries;
    int index;
    Usp__Set__UpdateObject *update_object;

    // Determine limits of backwards search for matching object path
    #define UPDATE_OBJECT_SEARCH_LIMIT 3
    num_entries = set->n_update_objs;
    index = num_entries - UPDATE_OBJECT_SEARCH_LIMIT;
    if (index < 0)
    {
        index = 0;
    }

    // Search backwards, trying to find the one which matches the specified object path
    for (i=num_entries-1; i>=index; i--)
    {
        update_object = set->update_objs[i];
        if (strcmp(update_object->obj_path, obj_path)==0)
        {
            return update_object;
        }
    }

    // If the code gets here, then no matching object path was found
    return NULL;
}

/*********************************************************************//**
**
** AddSetReq_UpdateObject
**
** Dynamically adds an update_object entry to a set request
**
** \param   set - pointer to Set request to add this entry to
** \param   obj_path - path to object in data model
**
** \return  Pointer to dynamically allocated update_object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Set__UpdateObject *AddSetReq_UpdateObject(Usp__Set *set, char *obj_path)
{
    Usp__Set__UpdateObject *update_object;

    int new_num;    // new number of entries in the set request

    // Allocate memory to store the update object
    update_object = USP_MALLOC(sizeof(Usp__Set__UpdateObject));
    usp__set__update_object__init(update_object);

    // Increase the size of the vector containing pointers to the update objects
    new_num = set->n_update_objs + 1;
    set->update_objs = USP_REALLOC(set->update_objs, new_num*sizeof(void *));
    set->n_update_objs = new_num;
    set->update_objs[new_num-1] = update_object;

    // Initialise the update object
    update_object->obj_path = USP_STRDUP(obj_path);
    update_object->n_param_settings = 0;
    update_object->param_settings = NULL;

    return update_object;
}

/*********************************************************************//**
**
** AddUpdateObject_ParamSettings
**
** Dynamically adds a param_settings entry to a update_object
**
** \param   update_object - pointer to update_ object to add this entry to
** \param   param_name - name of the parameter (not including object path) of the parameter to add
** \param   value - value of the parameter
**
** \return  Pointer to dynamically allocated param_settings object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Set__UpdateParamSetting *AddUpdateObject_ParamSettings(Usp__Set__UpdateObject *update_object, char *param_name, char *value)
{
    Usp__Set__UpdateParamSetting *param_settings;

    int new_num;    // new number of entries in the param_settings

    // Allocate memory to store the param_settings entry
    param_settings = USP_MALLOC(sizeof(Usp__Set__UpdateParamSetting));
    usp__set__update_param_setting__init(param_settings);

    // Increase the size of the vector containing pointers to the map entries
    new_num = update_object->n_param_settings + 1;
    update_object->param_settings = USP_REALLOC(update_object->param_settings, new_num*sizeof(void *));
    update_object->n_param_settings = new_num;
    update_object->param_settings[new_num-1] = param_settings;

    // Initialise the param_settings entry
    param_settings->param = USP_STRDUP(param_name);
    param_settings->value = USP_STRDUP(value);
    param_settings->required = true;

    return param_settings;
}


/*********************************************************************//**
**
** AddSetReq_Param
**
** Adds the specified parameter to the Set Request
**
** \param   set - pointer to Set request to add the parameter to
** \param   path - data model path of the parameter
** \param   value - new value of the parameter to set
**
** \return  None
**
**************************************************************************/
void AddSetReq_Param(Usp__Set *set, char *path, char *value)
{
    char obj_path[MAX_DM_PATH];
    char *param_name;
    Usp__Set__UpdateObject *update_object;

    // Split the parameter into the parent object path and the name of the parameter within the object
    param_name = TEXT_UTILS_SplitPath(path, obj_path, sizeof(obj_path));

    // Add an update object, if we don't already have one for the specified parent object
    update_object = FindUpdateObject(set, obj_path);
    if (update_object == NULL)
    {
        update_object = AddSetReq_UpdateObject(set, obj_path);
    }

    // Add the parameter to the param settings
    AddUpdateObject_ParamSettings(update_object, param_name, value);
}
#endif // !defined(REMOVE_USP_BROKER) && !defined(REMOVE_USP_SERVICE)
