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
 * \file handle_get_supported_dm.c
 *
 * Handles the GetSupportedDM message, creating a GetSupportedDMResponse
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
// Defines for bits in gs_flags variable
#define FIRST_LEVEL_ONLY 0x00000001
#define RETURN_COMMANDS  0x00000002
#define RETURN_EVENTS    0x00000004
#define RETURN_PARAMS    0x00000008

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void ProcessSupportedPathInstances(char *schema_path, unsigned gs_flags, Usp__GetSupportedDMResp *gs_resp);
Usp__Msg *CreateGetSupportedDMResp(char *msg_id);
void WalkSchema(dm_node_t *parent, Usp__GetSupportedDMResp__RequestedObjectResult *ror, unsigned gs_flags, combined_role_t *combined_role);
Usp__GetSupportedDMResp__RequestedObjectResult *
AddGetSupportedDM_ReqObjResult(Usp__GetSupportedDMResp *gs_resp, char *requested_path, int err, char *err_msg, char *bbf_uri);
Usp__GetSupportedDMResp__SupportedObjectResult *
AddReqObjResult_SupportedObjResult(Usp__GetSupportedDMResp__RequestedObjectResult *ror, dm_node_t *node, unsigned short permissions);
void AddSupportedObjResult_SupportedCommandResult(Usp__GetSupportedDMResp__SupportedObjectResult *sor, dm_node_t *node);
void AddSupportedObjResult_SupportedEventResult(Usp__GetSupportedDMResp__SupportedObjectResult *sor, dm_node_t *node);
void AddSupportedObjResult_SupportedParamResult(Usp__GetSupportedDMResp__SupportedObjectResult *sor, dm_node_t *node, unsigned short permissions);
Usp__GetSupportedDMResp__ObjAccessType  CalcDMSchemaObjAccess(bool is_add_allowed, bool is_del_allowed);
Usp__GetSupportedDMResp__ParamAccessType  CalcDMSchemaParamAccess(bool is_read_allowed, bool is_write_allowed);
Usp__GetSupportedDMResp__ParamValueType CalcDMSchemaParamType(dm_node_t *node);

/*********************************************************************//**
**
** MSG_HANDLER_HandleGetSupportedDM
**
** Handles a USP GetSupportedDM message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   controller_endpoint - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void MSG_HANDLER_HandleGetSupportedDM(Usp__Msg *usp, char *controller_endpoint, mtp_conn_t *mtpc)
{
    Usp__Msg *resp = NULL;
    int i;
    Usp__GetSupportedDM *gs;
    Usp__GetSupportedDMResp *gs_resp;
    unsigned gs_flags;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_GET_SUPPORTED_DM) ||
        (usp->body->request->get_supported_dm == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }

    // Extract flags controlling what the response contains
    gs = usp->body->request->get_supported_dm;
    gs_flags = 0;
    gs_flags |= (gs->first_level_only) ? FIRST_LEVEL_ONLY : 0;
    gs_flags |= (gs->return_commands) ? RETURN_COMMANDS : 0;
    gs_flags |= (gs->return_events) ? RETURN_EVENTS : 0;
    gs_flags |= (gs->return_params) ? RETURN_PARAMS : 0;

    // Create a GetSupportedDM Response message
    resp = CreateGetSupportedDMResp(usp->header->msg_id);
    gs_resp = resp->body->response->get_supported_dm_resp;

    // Iterate over all object paths in the request message
    for (i=0; i < gs->n_obj_paths; i++)
    {
        ProcessSupportedPathInstances(gs->obj_paths[i], gs_flags, gs_resp);
    }

exit:
    MSG_HANDLER_QueueMessage(controller_endpoint, resp, mtpc);
    usp__msg__free_unpacked(resp, pbuf_allocator);
}

/*********************************************************************//**
**
** ProcessSupportedPathInstances
**
** Process each requested path
**
** \param   schema_path - Data model schema path to query
** \param   gs_flags - flags controlling which artifacts to put in the response
** \param   gs_resp - pointer to GetSupportedDMResponse object of USP response message
**
** \return  None - This code must handle any errors by reporting errors in the response message
**
**************************************************************************/
void ProcessSupportedPathInstances(char *schema_path, unsigned gs_flags, Usp__GetSupportedDMResp *gs_resp)
{
    dm_node_t *node;
    Usp__GetSupportedDMResp__RequestedObjectResult *ror;
    combined_role_t combined_role;

    // Exit if unable to find a node matching the specified schema path
    node = DM_PRIV_GetNodeFromPath(schema_path, NULL, NULL, 0);
    if (node == NULL)
    {
        ror = AddGetSupportedDM_ReqObjResult(gs_resp, schema_path, USP_ERR_INVALID_PATH, USP_ERR_GetMessage(), BBF_DATA_MODEL_URI);
        (void)ror; // Keep Clang static analyser happy
        return;
    }

    // Exit if node is not an object
    if (IsObject(node) == false)
    {
        USP_ERR_SetMessage("%s: Schema path (%s) does not represent an object", __FUNCTION__, schema_path);
        ror = AddGetSupportedDM_ReqObjResult(gs_resp, schema_path, USP_ERR_INVALID_PATH, USP_ERR_GetMessage(), BBF_DATA_MODEL_URI);
        (void)ror; // Keep Clang static analyser happy
        return;
    }

    // Add a requested object result, since we will have at least one object
    ror = AddGetSupportedDM_ReqObjResult(gs_resp, schema_path, USP_ERR_OK, "", BBF_DATA_MODEL_URI);

    // Get the role to use for permissions when walking the data model schema
    MSG_HANDLER_GetMsgRole(&combined_role);

    // Recurse through the schema, building up the response
    WalkSchema(node, ror, gs_flags, &combined_role);
}

/*********************************************************************//**
**
** WalkSchema
**
** Function called recursively to add the schema paths of all nodes to a string vector
**
** \param   parent - pointer to data model node representing schema object to add to the response message
** \param   ror - pointer to the RequestedObjResult object in the response message to add ostring vector in which to add the schema paths
** \param   gs_flags - flags controlling which artifacts to put in the response
** \param   combined_role - role to use for permissions when walking the schema
**
** \return  None
**
**************************************************************************/
void WalkSchema(dm_node_t *parent, Usp__GetSupportedDMResp__RequestedObjectResult *ror, unsigned gs_flags, combined_role_t *combined_role)
{
    dm_node_t *child;
    Usp__GetSupportedDMResp__SupportedObjectResult *sor = NULL;
    unsigned short parent_perm;
    unsigned short child_perm;

    // Add a SupportedObjectResult for this schema object, if the controller is permitted to query its schema
    // NOTE: code that adds to the SupportedObjResult is also guarded by the same test
    USP_ASSERT(IsObject(parent));
    parent_perm  = DM_PRIV_GetPermissions(parent, combined_role);
    if (parent_perm & PERMIT_OBJ_INFO)
    {
        sor = AddReqObjResult_SupportedObjResult(ror, parent, parent_perm);
    }

    // Iterate over list of children
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        // Get controller's permissions for this child
        child_perm = DM_PRIV_GetPermissions(child, combined_role);

        switch(child->type)
        {
            case kDMNodeType_Object_MultiInstance:
            case kDMNodeType_Object_SingleInstance:
                if ((gs_flags & FIRST_LEVEL_ONLY)==0)
                {
                    WalkSchema(child, ror, gs_flags, combined_role);
                }
                else
                {
                    // FirstLevelOnly==true shows immediate child objects only
                    child_perm  = DM_PRIV_GetPermissions(parent, combined_role);
                    if (child_perm & PERMIT_OBJ_INFO)
                    {
                        (void)AddReqObjResult_SupportedObjResult(ror, child, child_perm);
                    }
                }
                break;

            case kDMNodeType_Param_ConstantValue:
            case kDMNodeType_Param_NumEntries:
            case kDMNodeType_DBParam_ReadWrite:
            case kDMNodeType_DBParam_ReadOnly:
            case kDMNodeType_DBParam_ReadOnlyAuto:
            case kDMNodeType_DBParam_ReadWriteAuto:
            case kDMNodeType_DBParam_Secure:
            case kDMNodeType_VendorParam_ReadOnly:
            case kDMNodeType_VendorParam_ReadWrite:
                if ((gs_flags & RETURN_PARAMS) && (parent_perm & PERMIT_OBJ_INFO))
                {
                    AddSupportedObjResult_SupportedParamResult(sor, child, child_perm);
                }
                break;

            case kDMNodeType_SyncOperation:
            case kDMNodeType_AsyncOperation:
                if ((gs_flags & RETURN_COMMANDS) &&
                    (parent_perm & PERMIT_OBJ_INFO) && (parent_perm & PERMIT_CMD_INFO) &&
                    (child_perm & PERMIT_CMD_INFO))
                {
                    AddSupportedObjResult_SupportedCommandResult(sor, child);
                }
                break;

            case kDMNodeType_Event:
                if ((gs_flags & RETURN_EVENTS) &&
                    (parent_perm & PERMIT_OBJ_INFO) && (parent_perm & PERMIT_CMD_INFO) &&
                    (child_perm & PERMIT_CMD_INFO))
                {
                    AddSupportedObjResult_SupportedEventResult(sor, child);
                }
                break;

            case kDMNodeType_Max:
                TERMINATE_BAD_CASE(child->type);
                break;
        }

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** CreateGetSupportedDMResp
**
** Dynamically creates an GetSupportedDMResponse object
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the request, which initiated this response
**
** \return  Pointer to a GetInstances Response object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateGetSupportedDMResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__GetSupportedDMResp *get_sup_resp;

    // Create GSDM Response
    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP, USP__RESPONSE__RESP_TYPE_GET_SUPPORTED_DM_RESP);
    get_sup_resp = USP_MALLOC(sizeof(Usp__GetSupportedDMResp));
    usp__get_supported_dmresp__init(get_sup_resp);
    msg->body->response->get_supported_dm_resp = get_sup_resp;

    return msg;
}

/*********************************************************************//**
**
** AddGetSupportedDM_ReqObjResult
**
** Dynamically adds a ReqObjResult to a GetSupportedDMResp object
**
** \param   gs_resp - pointer to GetSupportedDMResp object of USP response message
** \param   requested_path - path identifying the set of schema objects that are to be addressed
** \param   err - USP error for this object. If no error, then USP_ERR_OK
** \param   err_msg - pointer to error message. If no err, then an empty string.
** \param   bbf_uri - pointer to string describing which data model profile the objects or parameters are present in
**
** \return  Pointer to a RequestedObjResult object
**
**************************************************************************/
Usp__GetSupportedDMResp__RequestedObjectResult *
AddGetSupportedDM_ReqObjResult(Usp__GetSupportedDMResp *gs_resp, char *requested_path, int err, char *err_msg, char *bbf_uri)
{
    Usp__GetSupportedDMResp__RequestedObjectResult *ror;
    int new_num;    // new number of entries in the requested obj result array

    // Allocate memory to store the RequestedObjResult object
    ror = USP_MALLOC(sizeof(Usp__GetSupportedDMResp__RequestedObjectResult));
    usp__get_supported_dmresp__requested_object_result__init(ror);

    // Increase the size of the vector
    new_num = gs_resp->n_req_obj_results + 1;
    gs_resp->req_obj_results = USP_REALLOC(gs_resp->req_obj_results, new_num*sizeof(void *));
    gs_resp->n_req_obj_results = new_num;
    gs_resp->req_obj_results[new_num-1] = ror;

    // Fill in the RequestedObjResult object
    ror->req_obj_path = USP_STRDUP(requested_path);
    ror->err_code = err;
    ror->err_msg = USP_STRDUP(err_msg);
    ror->data_model_inst_uri = USP_STRDUP(bbf_uri);

    return ror;
}

/*********************************************************************//**
**
** AddReqObjResult_SupportedObjResult
**
** Dynamically adds a SupportedObjResult to a RequestedObjResult object
**
** \param   ror - pointer to RequestedObjResult object
** \param   node - Data model node containing details to add
** \param   permissions - permissions bitmask for the node
**
** \return  Pointer to a SupportedObjResult object
**
**************************************************************************/
Usp__GetSupportedDMResp__SupportedObjectResult *
AddReqObjResult_SupportedObjResult(Usp__GetSupportedDMResp__RequestedObjectResult *ror, dm_node_t *node, unsigned short permissions)
{
    Usp__GetSupportedDMResp__SupportedObjectResult *sor;
    dm_object_info_t *info;
    int new_num;    // new number of entries in the requested obj result array
    int len;
    bool is_add_allowed = false;  // Assume that add and delete of the object are not allowed
    bool is_del_allowed = false;

    #define CAN_ADD 0x01
    #define CAN_DELETE 0x02

    // Allocate memory to store the SupportedObjResult object
    sor = USP_MALLOC(sizeof(Usp__GetSupportedDMResp__SupportedObjectResult));
    usp__get_supported_dmresp__supported_object_result__init(sor);

    // Increase the size of the vector
    new_num = ror->n_supported_objs + 1;
    ror->supported_objs = USP_REALLOC(ror->supported_objs, new_num*sizeof(void *));
    ror->n_supported_objs = new_num;
    ror->supported_objs[new_num-1] = sor;

    // Fill in the SupportedObjResult object. Path must include a trailing '.'
    len = strlen(node->path);
    sor->supported_obj_path = USP_MALLOC(len+2);  // Plus 2 to include trailing '.' and NULL terminator
    memcpy(sor->supported_obj_path, node->path, len);
    sor->supported_obj_path[len] = '.';
    sor->supported_obj_path[len+1] = '\0';

    // Determine properties, based on whether the object is multi-instance or not, and grouped or not
    if (node->type == kDMNodeType_Object_MultiInstance)
    {
        // Multi Instance object
        sor->is_multi_instance = true;
        info = &node->registered.object_info;
        if (node->group_id == NON_GROUPED)
        {
            // Non-grouped multi Instance object
            if ((info->validate_add_cb != USP_HOOK_DenyAddInstance) && (permissions & PERMIT_ADD))
            {
                is_add_allowed = true;
            }

            if ((info->validate_del_cb != USP_HOOK_DenyDeleteInstance) && (permissions & PERMIT_DEL))
            {
                is_del_allowed = true;
            }
        }
        else
        {
            // Grouped multi Instance object
            if (info->group_writable)
            {
                is_add_allowed = permissions & PERMIT_ADD;
                is_del_allowed = permissions & PERMIT_DEL;
            }
        }
    }
    else
    {
        // Single Instance object
        sor->is_multi_instance = false;
    }

    // Set the Access enumeration, based on peroperties calculated above
    sor->access = CalcDMSchemaObjAccess(is_add_allowed, is_del_allowed);

    // Divergent paths are not currently supported, so no divergent object instances to indicate
    sor->n_divergent_paths = 0;
    sor->divergent_paths = NULL;

    return sor;
}

/*********************************************************************//**
**
** AddSupportedObjResult_SupportedCommandResult
**
** Dynamically adds a SupportedCommandResult to a SupportedObjResult object
**
** \param   sor - pointer to the SupportedObjResult object
** \param   node - Data model node containing details to add
**
** \return  None
**
**************************************************************************/
void AddSupportedObjResult_SupportedCommandResult(Usp__GetSupportedDMResp__SupportedObjectResult *sor, dm_node_t *node)
{
    Usp__GetSupportedDMResp__SupportedCommandResult *cr;
    int new_num;    // new number of entries in the requested obj result array
    dm_oper_info_t *info;
    str_vector_t *sv;
    int i;

    // Allocate memory to store the SupportedCommandResult object
    cr = USP_MALLOC(sizeof(Usp__GetSupportedDMResp__SupportedCommandResult));
    usp__get_supported_dmresp__supported_command_result__init(cr);

    // Increase the size of the vector
    new_num = sor->n_supported_commands + 1;
    sor->supported_commands = USP_REALLOC(sor->supported_commands, new_num*sizeof(void *));
    sor->n_supported_commands = new_num;
    sor->supported_commands[new_num-1] = cr;

    // Fill in the SupportedCommandResult object
    cr->command_name = USP_STRDUP(node->name);
    cr->command_type = (node->type == kDMNodeType_SyncOperation) ? USP__GET_SUPPORTED_DMRESP__CMD_TYPE__CMD_SYNC : USP__GET_SUPPORTED_DMRESP__CMD_TYPE__CMD_ASYNC;

    // Copy the command's input arguments into the SupportedCommandResult
    info = &node->registered.oper_info;
    sv = &info->input_args;
    if (sv->num_entries > 0)
    {
        cr->n_input_arg_names = sv->num_entries;
        cr->input_arg_names = USP_MALLOC(sv->num_entries*sizeof(void *));
        for (i=0; i < sv->num_entries; i++)
        {
            cr->input_arg_names[i] = USP_STRDUP(sv->vector[i]);
        }
    }

    // Copy the command's output arguments into the SupportedCommandResult
    sv = &info->output_args;
    if (sv->num_entries > 0)
    {
        cr->n_output_arg_names = sv->num_entries;
        cr->output_arg_names = USP_MALLOC(sv->num_entries*sizeof(void *));
        for (i=0; i < sv->num_entries; i++)
        {
            cr->output_arg_names[i] = USP_STRDUP(sv->vector[i]);
        }
    }
}

/*********************************************************************//**
**
** AddSupportedObjResult_SupportedEvent
**
** Dynamically adds a SupportedEventResult to a SupportedObjResult object
**
** \param   sor - pointer to the SupportedObjResult object
** \param   node - Data model node containing details to add
**
** \return  None
**
**************************************************************************/
void AddSupportedObjResult_SupportedEventResult(Usp__GetSupportedDMResp__SupportedObjectResult *sor, dm_node_t *node)
{
    Usp__GetSupportedDMResp__SupportedEventResult *er;
    int new_num;    // new number of entries in the requested obj result array
    dm_event_info_t *info;
    str_vector_t *sv;
    int i;

    // Allocate memory to store the SupportedEventResult object
    er = USP_MALLOC(sizeof(Usp__GetSupportedDMResp__SupportedEventResult));
    usp__get_supported_dmresp__supported_event_result__init(er);

    // Increase the size of the vector
    new_num = sor->n_supported_events + 1;
    sor->supported_events = USP_REALLOC(sor->supported_events, new_num*sizeof(void *));
    sor->n_supported_events = new_num;
    sor->supported_events[new_num-1] = er;

    // Fill in the SupportedCommandResult object
    er->event_name = USP_STRDUP(node->name);

    // Copy the event's arguments into the SupportedEventResult
    info = &node->registered.event_info;
    sv = &info->event_args;
    if (sv->num_entries > 0)
    {
        er->n_arg_names = sv->num_entries;
        er->arg_names = USP_MALLOC(sv->num_entries*sizeof(void *));
        for (i=0; i < sv->num_entries; i++)
        {
            er->arg_names[i] = USP_STRDUP(sv->vector[i]);
        }
    }
}

/*********************************************************************//**
**
** AddSupportedObjResult_SupportedParam
**
** Dynamically adds a SupportedParamResult to a SupportedObjResult object
**
** \param   sor - pointer to the SupportedObjResult object
** \param   node - Data model node containing details to add
** \param   permissions - permissions bitmask for the node
**
** \return  None
**
**************************************************************************/
void AddSupportedObjResult_SupportedParamResult(Usp__GetSupportedDMResp__SupportedObjectResult *sor, dm_node_t *node, unsigned short permissions)
{
    Usp__GetSupportedDMResp__SupportedParamResult *pr;
    int new_num;    // new number of entries in the requested obj result array
    bool is_read_allowed = false;
    bool is_write_allowed = false;

    // Determine raw access type of the parameter
    switch(node->type)
    {
        case kDMNodeType_Param_ConstantValue:
        case kDMNodeType_Param_NumEntries:
        case kDMNodeType_DBParam_ReadOnly:
        case kDMNodeType_DBParam_ReadOnlyAuto:
        case kDMNodeType_VendorParam_ReadOnly:
            is_read_allowed = true;
            is_write_allowed = false;
            break;

        case kDMNodeType_DBParam_ReadWrite:
        case kDMNodeType_DBParam_ReadWriteAuto:
        case kDMNodeType_DBParam_Secure:
        case kDMNodeType_VendorParam_ReadWrite:
            is_read_allowed = true;
            is_write_allowed = true;
            break;

        case kDMNodeType_Max:
        case kDMNodeType_Event:
        case kDMNodeType_Object_MultiInstance:
        case kDMNodeType_Object_SingleInstance:
        case kDMNodeType_SyncOperation:
        case kDMNodeType_AsyncOperation:
            TERMINATE_BAD_CASE(node->type); // This function shouldn't have been called for these types
            break;
    }

    // Modify access type based on whether the controller has permissions
    if ((permissions & PERMIT_GET)==0)
    {
        is_read_allowed = false;
    }

    if ((permissions & PERMIT_SET)==0)
    {
        is_write_allowed = false;
    }

    // Exit if neither read or write of the parameter is allowed - in this case we do not put it in the schema that is returned
    if ((is_read_allowed == false) && (is_write_allowed == false))
    {
        return;
    }

    // Allocate memory to store the SupportedParamResult object
    pr = USP_MALLOC(sizeof(Usp__GetSupportedDMResp__SupportedParamResult));
    usp__get_supported_dmresp__supported_param_result__init(pr);

    // Increase the size of the vector
    new_num = sor->n_supported_params + 1;
    sor->supported_params = USP_REALLOC(sor->supported_params, new_num*sizeof(void *));
    sor->n_supported_params = new_num;
    sor->supported_params[new_num-1] = pr;

    // Fill in the SupportedParamst object
    pr->param_name = USP_STRDUP(node->name);
    pr->access = CalcDMSchemaParamAccess(is_read_allowed, is_write_allowed);
    pr->value_type = CalcDMSchemaParamType(node);
    pr->value_change = (node->registered.param_info.type_flags & DM_VALUE_CHANGE_WILL_IGNORE) ? USP__GET_SUPPORTED_DMRESP__VALUE_CHANGE_TYPE__VALUE_CHANGE_WILL_IGNORE : USP__GET_SUPPORTED_DMRESP__VALUE_CHANGE_TYPE__VALUE_CHANGE_ALLOWED;
}

/*********************************************************************//**
**
** CalcDMSchemaObjAccess
**
** Converts whether the object is allowed to be added and deleted to the
** access type enumeration required in the GetSupportedDM response
**
** \param   is_add_allowed - set if the object is multi-instance, capable of being added to, and the controller has permission to add
** \param   is_del_allowed - set if the object is multi-instance, capable of being deleted to, and the controller has permission to delete
**
** \return  access type enumeration required in the GetSupportedDM response
**
**************************************************************************/
Usp__GetSupportedDMResp__ObjAccessType
CalcDMSchemaObjAccess(bool is_add_allowed, bool is_del_allowed)
{
    if (is_add_allowed == false)
    {
        // In this stanza, Add is not allowed
        if (is_del_allowed == false)
        {
            // Add and Delete not allowed
            return USP__GET_SUPPORTED_DMRESP__OBJ_ACCESS_TYPE__OBJ_READ_ONLY;
        }
        else
        {
            // Add not allowed, Delete allowed
            return USP__GET_SUPPORTED_DMRESP__OBJ_ACCESS_TYPE__OBJ_DELETE_ONLY;
        }
    }
    else
    {
        // In this stanza, Add is allowed
        if (is_del_allowed == false)
        {
            // Add allowed, Delete not allowed
            return USP__GET_SUPPORTED_DMRESP__OBJ_ACCESS_TYPE__OBJ_ADD_ONLY;
        }
        else
        {
            // Add and Delete allowed
            return USP__GET_SUPPORTED_DMRESP__OBJ_ACCESS_TYPE__OBJ_ADD_DELETE;
        }
    }
}

/*********************************************************************//**
**
** CalcDMSchemaParamAccess
**
** Converts whether the parameter is allowed to be read or written to the
** access type enumeration required in the GetSupportedDM response
**
** \param   is_read_allowed - set if the parameter is capable of being read, and the controller has permission to read it
** \param   is_write_allowed - set if the parameter is capable of being written, and the controller has permission to write it
**
** \return  access type enumeration required in the GetSupportedDM response
**
**************************************************************************/
Usp__GetSupportedDMResp__ParamAccessType
CalcDMSchemaParamAccess(bool is_read_allowed, bool is_write_allowed)
{
    USP_ASSERT((is_read_allowed) || (is_write_allowed));

    if (is_read_allowed == true)
    {
        // In this stanza, Read is allowed
        if (is_write_allowed == true)
        {
            // Read and write allowed
            return USP__GET_SUPPORTED_DMRESP__PARAM_ACCESS_TYPE__PARAM_READ_WRITE;
        }
        else
        {
            // Read allowed, write not allowed
            return USP__GET_SUPPORTED_DMRESP__PARAM_ACCESS_TYPE__PARAM_READ_ONLY;
        }
    }
    else
    {
        // In this stanza, Read is not allowed
        if (is_write_allowed == true)
        {
            return USP__GET_SUPPORTED_DMRESP__PARAM_ACCESS_TYPE__PARAM_WRITE_ONLY;
        }
        // NOTE: Else case of neither read nor write allowed cannot occur. See assert above
    }

    // The code should never get here, as this function is never called with neither read or write allowed
    // however to keep the compiler happy, return a value here;
    return USP__GET_SUPPORTED_DMRESP__PARAM_ACCESS_TYPE__PARAM_WRITE_ONLY;
}

/*********************************************************************//**
**
** CalcDMSchemaParamType
**
** Calculates the type of the specified parameter to put in the GetSupportedDM response
**
** \param   node - Data model node for parameter
**
** \return  type of the parameter
**
**************************************************************************/
Usp__GetSupportedDMResp__ParamValueType CalcDMSchemaParamType(dm_node_t *node)
{
    unsigned type_flags;

    type_flags = node->registered.param_info.type_flags;
    if (type_flags & DM_STRING)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_STRING;
    }
    else if (type_flags & DM_DATETIME)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_DATE_TIME;
    }
    else if (type_flags & DM_BOOL)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_BOOLEAN;
    }
    else if (type_flags & DM_INT)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_INT;
    }
    else if (type_flags & DM_UINT)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNSIGNED_INT;
    }
    else if (type_flags & DM_ULONG)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNSIGNED_LONG;
    }
    else if (type_flags & DM_BASE64)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_BASE_64;
    }
    else if (type_flags & DM_HEXBIN)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_HEX_BINARY;
    }
    else if (type_flags & DM_DECIMAL)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_DECIMAL;
    }
    else if (type_flags & DM_LONG)
    {
        return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_LONG;
    }
    else
    {
        // This assert should only fire if this function is not updated when new types are added to the data model
        USP_ASSERT(false);
    }

    return USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNKNOWN;
}

