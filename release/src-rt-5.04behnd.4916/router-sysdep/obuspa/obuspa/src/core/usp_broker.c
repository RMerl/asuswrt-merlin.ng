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
 * \file device_usp_service.c
 *
 * Implements Device.USPServices
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common_defs.h"
#include "msg_handler.h"
#include "msg_utils.h"
#include "device.h"
#include "data_model.h"
#include "dm_exec.h"
#include "dm_inst_vector.h"
#include "iso8601.h"
#include "text_utils.h"
#include "usp_broker.h"
#include "proto_trace.h"
#include "path_resolver.h"  // For FULL_DEPTH
#include "cli.h"

#ifndef REMOVE_USP_BROKER

//------------------------------------------------------------------------------
// Time to wait for a response from a USP Service
#define RESPONSE_TIMEOUT  30

//------------------------------------------------------------------------------
// Location of the Device.USPService.USPService table within the data model
#define DEVICE_SERVICE_ROOT "Device.USPServices.USPService"

//------------------------------------------------------------------------------
// Path to use when querying the USP Service's subscription table
static char *subs_partial_path = "Device.LocalAgent.Subscription.";

//------------------------------------------------------------------------------
// String to use in all messages and subscription ID's allocated by the Broker
static char *broker_unique_str = "BROKER";

//------------------------------------------------------------------------------
// Structure mapping the instance in the Broker's subscription table with the subscription table in the USP Service
// This table is consulted to route a USP notification received from a USP Service back to the Controller that subscribed to it on the Broker
typedef struct
{
    double_link_t link;         // Doubly linked list pointers. These must always be first in this structure
    int broker_instance;        // Instance number in the Broker's Device.LocalAgent.Subscription.{i}
                                // NOTE: Since the broker's subscription may have a ReferenceList containing many paths,
                                //       it is possible for there to be more than one entry in this map with the same broker_instance
    char *path;                 // Data model path which is subscribed to on the USP Service
    int service_instance;       // Instance number in the Service's Device.LocalAgent.Subscription.{i}
    char *subscription_id;      // Subscription Id in the USP Service's subscription table.
                                // NOTE: This is allocated by the Broker to avoid non-uniqueness in the USP Service, if USP Controllers choose the same ID in the Broker's subscription table
} subs_map_t;

//------------------------------------------------------------------------------
// Structure mapping the instance in the Broker's Request table to the command key and path of an active USP operation
// This table is consulted to delete entries in the Broker's request table, when the operation complete notification is received from the USP Service
typedef struct
{
    double_link_t link;         // Doubly linked list pointers. These must always be first in this structure
    int request_instance;       // Instance number in the Broker's Device.LocalAgent.Request.{i}
    char *path;                 // Data model path of USP Command which has been invoked
    char *command_key;          // Command key of the Operate Request
} req_map_t;

//------------------------------------------------------------------------------
// Structure mapping a USP request message which has been passed through to a USP Service, back to the originator of the request
// This table is consulted when the corresponding USP response message is received from the USP service, to route the response
// back to the originator of the request
typedef struct
{
    double_link_t link;         // Doubly linked list pointers. These must always be first in this structure
    char *broker_msg_id;        // The USP message ID assigned by the Broker to avoid non-uniqueness of message IDs across different originators
    char *original_msg_id;      // The USP message ID assigned by the originator
    char *originator;           // EndpointID for the originator of the message
    mtp_conn_t mtp_conn;        // Structure containing the MTP details of the originator of the request
} msg_map_t;

//------------------------------------------------------------------------------
// Array containing the list of connected USP Services
typedef struct
{
    int instance;                   // instance number in Device.USP.USPService.{i}. Set to INVALID, if this entry is not in use
    char *endpoint_id;              // Endpoint Id of the USP service
    mtp_conn_t controller_mtp;      // Identifies the MTP to use when acting as a controller sending to the Endpoint's agent
    mtp_conn_t agent_mtp;           // Identifies the MTP to use when acting as an agent sending to the Endpoint's controller
    int group_id;                   // Group Id assigned for this endpoint
    bool has_controller;            // Set if the USP Service's Controller is connected via the Broker's agent socket
    str_vector_t gsdm_msg_ids;      // Message Ids of all outstanding GSDM requests sent to the USP Service
    str_vector_t registered_paths;  // vector of top level data model objects that the USP Service provides
    double_linked_list_t subs_map;  // linked list implementing a table mapping the subscription in the Broker's subscription table to the subscription in the Service's subscription table
    double_linked_list_t req_map;   // linked list implementing a table mapping the instance in the Broker's request table to the command_key of the request
    double_linked_list_t msg_map;   // vector mapping the message ID of a request passed thru to this USP service, back to the originating controller which sent the request
} usp_service_t;

static usp_service_t usp_services[MAX_USP_SERVICES] = {{0}};


//------------------------------------------------------------------------------
// Defines for flags argument of HandleUspServiceAgentDisconnect()
#define DONT_FAIL_USP_COMMANDS_IN_PROGRESS 0x00000000
#define FAIL_USP_COMMANDS_IN_PROGRESS      0x00000001

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int GetUspService_EndpointID(dm_req_t *req, char *buf, int len);
int GetUspService_Protocol(dm_req_t *req, char *buf, int len);
int GetUspService_DMPaths(dm_req_t *req, char *buf, int len);
int GetUspService_HasController(dm_req_t *req, char *buf, int len);
usp_service_t *FindUspServiceByEndpoint(char *endpoint_id);
usp_service_t *FindUspServiceByInstance(int instance);
usp_service_t *FindUspServiceByGroupId(int group_id);
usp_service_t *FindUnusedUspService(void);
int CalcNextUspServiceInstanceNumber(void);
void CalcBrokerMessageId(char *msg_id, int len);
bool IsValidUspServicePath(char *path);
int ProcessGetResponse(Usp__Msg *resp, kv_vector_t *kvv);
int ProcessGetInstancesResponse(Usp__Msg *resp, usp_service_t *us, bool within_vendor_hook);
int CompareGetInstances_CurInst(const void *entry1, const void *entry2);
Usp__Msg *CreateRegisterResp(char *msg_id);
void AddRegisterResp_RegisteredPathResult(Usp__RegisterResp *reg_resp, char *requested_path, int err_code);
int CompareGsdm_SupportedObj(const void *entry1, const void *entry2);
bool ProcessGsdm_RequestedPath(Usp__GetSupportedDMResp__RequestedObjectResult *ror, usp_service_t *us);
void ProcessGsdm_SupportedObject(Usp__GetSupportedDMResp__SupportedObjectResult *sor, int group_id);
unsigned CalcParamType(Usp__GetSupportedDMResp__ParamValueType value_type);
usp_service_t *AddUspService(char *endpoint_id, mtp_conn_t *mtpc);
bool IsPathAlreadyRegistered(char *requested_path);
bool IsPathAlreadyRegisteredAsAParent(usp_service_t *us, char *registered_path);
void FreeUspService(usp_service_t *us);
void QueueGetSupportedDMToUspService(usp_service_t *us, str_vector_t *paths);
void ApplyPermissionsToUspService(usp_service_t *us);
int Broker_GroupGet(int group_id, kv_vector_t *kvv);
int Broker_GroupSet(int group_id, kv_vector_t *params, unsigned *param_types, int *failure_index);
int Broker_GroupAdd(int group_id, char *path, int *instance);
int Broker_GroupDelete(int group_id, char *path);
int Broker_GroupSubscribe(int instance, int group_id, subs_notify_t type, char *path, bool persistent);
int Broker_GroupUnsubscribe(int instance, int group_id, subs_notify_t type, char *path);
int Broker_MultiDelete(int group_id, bool allow_partial, char **paths, int num_paths, int *failure_index);
int Broker_CreateObj(int group_id, char *path, group_add_param_t *params, int num_params, int *instance, kv_vector_t *unique_keys);
int Broker_SyncOperate(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);
int Broker_AsyncOperate(dm_req_t *req, kv_vector_t *input_args, int instance);
int Broker_RefreshInstances(int group_id, char *path, int *expiry_period);
int CalcFailureIndex(Usp__Msg *resp, kv_vector_t *params, int *modified_err);
int ProcessAddResponse(Usp__Msg *resp, char *path, int *instance, kv_vector_t *unique_keys, group_add_param_t *params, int num_params);
int ProcessSetResponse(Usp__Msg *resp, kv_vector_t *params, int *failure_index);
void LogSetResponse_OperFailure(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure);
bool CheckSetResponse_OperSuccess(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *oper_success, kv_vector_t *params);
void PropagateParamErr(char *path, int err_code, char *err_msg, group_add_param_t *params, int num_params);
int ValidateAddResponsePath(char *schema_path, char *instantiated_path, int *instance);
int ProcessDeleteResponse(Usp__Msg *resp, str_vector_t *paths, int *failure_index);
void SubsMap_Init(double_linked_list_t *sm);
void SubsMap_Destroy(double_linked_list_t *sm);
void SubsMap_Add(double_linked_list_t *sm, int service_instance, char *path, char *subscription_id, int broker_instance);
void SubsMap_Remove(double_linked_list_t *sm, subs_map_t *smap);
subs_map_t *SubsMap_FindByUspServiceSubsId(double_linked_list_t *sm, char *subscription_id, int broker_instance);
subs_map_t *SubsMap_FindByBrokerInstanceAndPath(double_linked_list_t *sm, int broker_instance, char *path);
subs_map_t *SubsMap_FindByPath(double_linked_list_t *sm, char *path);
void ReqMap_Init(double_linked_list_t *rm);
void ReqMap_Destroy(double_linked_list_t *rm);
req_map_t *ReqMap_Add(double_linked_list_t *rm, int request_instance, char *path, char *command_key);
void ReqMap_Remove(double_linked_list_t *rm, req_map_t *rmap);
req_map_t *ReqMap_Find(double_linked_list_t *rm, char *path, char *command_key);
int SyncSubscriptions(usp_service_t *us);
int UspService_DeleteInstances(usp_service_t *us, bool allow_partial, str_vector_t *paths, int *failure_index);
int UspService_RefreshInstances(usp_service_t *us, str_vector_t *paths, bool within_vendor_hook);
int ProcessGetSubsResponse(usp_service_t *us, Usp__Msg *resp);
void ProcessGetSubsResponse_ResolvedPathResult(usp_service_t *us, Usp__GetResp__ResolvedPathResult *res, str_vector_t *subs_to_delete);
char *GetParamValueFromResolvedPathResult(Usp__GetResp__ResolvedPathResult *res, char *name);
int SendOperateAndProcessResponse(int group_id, char *path, bool is_sync, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args, bool *is_complete);
int ProcessOperateResponse(Usp__Msg *resp, char *path, bool is_sync, kv_vector_t *output_args, bool *is_complete);
void DeleteMatchingOperateRequest(usp_service_t *us, char *obj_path, char *command_name, char *command_key);
void UpdateUspServiceMRT(usp_service_t *us, mtp_conn_t *mtpc);
void ProcessUniqueKeys(char *path, Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry **unique_keys, int num_unique_keys);
bool AttemptPassThruForGetRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec);
bool AttemptPassThruForSetRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec);
bool AttemptPassThruForAddRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec);
bool AttemptPassThruForDeleteRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec);
bool AttemptPassThruForNotification(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, UspRecord__Record *rec);
bool CheckPassThruPermissions(dm_node_t *node, int depth, unsigned short required_permissions, combined_role_t *combined_role);
int PassThruToUspService(usp_service_t *us, Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, UspRecord__Record *rec);
void MsgMap_Init(double_linked_list_t *mm);
void MsgMap_Destroy(double_linked_list_t *mm);
msg_map_t *MsgMap_Add(double_linked_list_t *mm, char *original_msg_id, char *broker_msg_id, char *endpoint_id, mtp_conn_t *mtpc);
void MsgMap_Remove(double_linked_list_t *mm, msg_map_t *map);
msg_map_t *MsgMap_Find(double_linked_list_t *mm, char *msg_id);
bool AttemptPassThruForResponse(Usp__Msg *usp, char *endpoint_id);
void HandleUspServiceAgentDisconnect(usp_service_t *us, unsigned flags);
Usp__Msg *CreateDeRegisterResp(char *msg_id);
Usp__DeregisterResp__DeregisteredPathResult *AddDeRegisterResp_DeRegisteredPathResult(Usp__DeregisterResp *dreg_resp, char *requested_path, char *path, int err_code, char *err_msg);
void DeRegisterAllPaths(usp_service_t *us, Usp__DeregisterResp *dreg_resp);
void RemoveDeRegisterResp_DeRegisteredPathResult(Usp__DeregisterResp *dreg_resp);
void AddDeRegisterRespSuccess_Path(Usp__DeregisterResp__DeregisteredPathResult *dreg_path_result, char *path);
int DeRegisterUspServicePath(usp_service_t *us, char *path);
bool IsPathASpecialException(char *path, int group_id);
int ShouldPathBeAddedToDataModel(usp_service_t *us, char *path);
void RegisterBrokerVendorHooks(usp_service_t *us);
void DeregisterBrokerVendorHooks(usp_service_t *us);
void AddTopLevelTableNodes(dm_node_t *parent, str_vector_t *sv);
bool IsUspServiceNotificationMatch(Usp__Notify *notify, usp_service_t *us, int broker_instance);

/*********************************************************************//**
**
** USP_BROKER_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_BROKER_Init(void)
{
    int i;
    usp_service_t *us;
    int err = USP_ERR_OK;

    // Register Device.UspServices object
    err |= USP_REGISTER_Object(DEVICE_SERVICE_ROOT ".{i}", USP_HOOK_DenyAddInstance, NULL, NULL,
                                                           USP_HOOK_DenyDeleteInstance, NULL, NULL);

    err |= USP_REGISTER_Param_NumEntries("Device.USPServices.USPServiceNumberOfEntries", DEVICE_SERVICE_ROOT ".{i}");

    // Register Device.USPServices.USPService parameters
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_SERVICE_ROOT ".{i}.EndpointID", GetUspService_EndpointID, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_SERVICE_ROOT ".{i}.Protocol", GetUspService_Protocol, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_SERVICE_ROOT ".{i}.DataModelPaths", GetUspService_DMPaths, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_SERVICE_ROOT ".{i}.HasController", GetUspService_HasController, DM_BOOL);

    // Register unique key for table
    char *unique_keys[] = { "EndpointID" };
    err |= USP_REGISTER_Object_UniqueKey("Device.USPServices.USPService.{i}", unique_keys, NUM_ELEM(unique_keys));

    // Exit if any errors occurred
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Mark all entries in the USP services array as unused
    memset(usp_services, 0, sizeof(usp_services));
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        us->instance = INVALID;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_BROKER_Start
**
** Starts this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_BROKER_Start(void)
{
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_BROKER_Stop
**
** Stops this component
**
** \param   None
**
** \return  None
**
**************************************************************************/
void USP_BROKER_Stop(void)
{
    int i;
    usp_service_t *us;

    // Iterate over all USP services freeing all memory allocated by the USP Service (including data model)
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if (us->instance != INVALID)
        {
            // NOTE: USP Commands which are currently still in progress on a USP Service should send their OperationComplete
            // indicating failure after reboot. Hence we shouldn't remove them from the USP DB here
            HandleUspServiceAgentDisconnect(us, DONT_FAIL_USP_COMMANDS_IN_PROGRESS);
            FreeUspService(us);
        }
    }
}

/*********************************************************************//**
**
** USP_BROKER_AddUspService
**
** Called when a USP Service has connected successfully over UDS, to add the service into the USP services table
**
** \param   endpoint_id - endpoint of USP service to add
** \param   mtpc - pointer to structure specifying which protocol (and MTP instance) the endpoint is using
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int USP_BROKER_AddUspService(char *endpoint_id, mtp_conn_t *mtpc)
{
    int err;
    usp_service_t *us;
    char path[MAX_DM_PATH];

    // Exit if this endpoint has already registered (this could happen as there may be 2 UDS connections to the endpoint)
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us != NULL)
    {
        // Ensure that the connection details to both USP Broker's controller and agent sockets are saved
        UpdateUspServiceMRT(us, mtpc);
        goto exit;
    }

    // Exit if unable to add the USP service into the internal data structure
    us = AddUspService(endpoint_id, mtpc);
    if (us == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to register any more USP services", __FUNCTION__);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // Exit if unable to inform this USP Service instance into the data model
    USP_SNPRINTF(path, sizeof(path), DEVICE_SERVICE_ROOT ".%d", us->instance);
    err = USP_DM_InformInstance(path);
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

exit:
#ifdef ENABLE_UDS
    // Mark the USP Service as having a controller, if it connected on the Broker's agent socket
    if ((mtpc->protocol == kMtpProtocol_UDS) && (mtpc->uds.path_type == kUdsPathType_BrokersAgent))
    {
        us->has_controller = true;
    }
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** USP_BROKER_HandleUspServiceDisconnect
**
** Called when a USP Service disconnects from UDS
**
** \param   endpoint_id - endpoint that disconnected
** \param   path_type - whether the endpoint was connected to the Broker's Controller or the Broker's Agent socket
**
** \return  None
**
**************************************************************************/
void USP_BROKER_HandleUspServiceDisconnect(char *endpoint_id, uds_path_t path_type)
{
    usp_service_t *us;
    char path[MAX_DM_PATH];

    // Exit if we don't know anything about this endpoint
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        return;
    }

    switch(path_type)
    {
        case kUdsPathType_BrokersAgent:
            // USP Service's controller disconnected
            DM_EXEC_FreeMTPConnection(&us->agent_mtp);
            us->has_controller = false;
            break;

        case kUdsPathType_BrokersController:
            // USP Service's agent disconnected
            DM_EXEC_FreeMTPConnection(&us->controller_mtp);
            HandleUspServiceAgentDisconnect(us, FAIL_USP_COMMANDS_IN_PROGRESS);
            break;

        default:
        case kUdsPathType_Invalid:
            TERMINATE_BAD_CASE(path_type);
            break;
    }

    // If the Service is not now connected via either the Broker's controller or the Broker's Agent socket,
    // then remove the USP Service entirely from the USP Service table
    if ((us->controller_mtp.protocol == kMtpProtocol_None) && (us->agent_mtp.protocol == kMtpProtocol_None))
    {
        // Mark the group_id allocated to this USP Service as not-in-use
        DeregisterBrokerVendorHooks(us);

        // Inform the data model, that this entry in the USP Service table has been deleted
        USP_SNPRINTF(path, sizeof(path), "%s.%d", DEVICE_SERVICE_ROOT, us->instance);
        DATA_MODEL_NotifyInstanceDeleted(path);

        // Finally free the USP Service, as all state related to it in the rest of the system has been undone
        FreeUspService(us);
    }
}

/*********************************************************************//**
**
** USP_BROKER_HandleRegister
**
** Handles a USP Register message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP service which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void USP_BROKER_HandleRegister(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    Usp__Msg *resp = NULL;
    int i;
    int err;
    usp_service_t *us = NULL;
    Usp__Register *reg;
    Usp__Register__RegistrationPath *rp;
    Usp__RegisterResp *reg_resp;
    bool allow_partial;
    str_vector_t accepted_paths;    // List of paths accepted from this register message, which have not been previously registered
    char path[MAX_DM_PATH];

    STR_VECTOR_Init(&accepted_paths);

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_REGISTER) ||
        (usp->body->request->register_ == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }

    // Extract flags controlling what the response contains
    reg = usp->body->request->register_;
    allow_partial = (bool) reg->allow_partial;

    // Exit if there are no paths to register
    if ((reg->n_reg_paths == 0) || (reg->reg_paths == NULL))
    {
        USP_ERR_SetMessage("%s: No paths in register message", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_REGISTER_FAILURE, resp);
        goto exit;
    }

    // Add USP Service if not already added
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        us = AddUspService(endpoint_id, mtpc);
        if (us == NULL)
        {
            USP_ERR_SetMessage("%s: Unable to register any more USP services", __FUNCTION__);
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_REGISTER_FAILURE, resp);
            goto exit;
        }

        // Exit if unable to inform this USP Service instance into the data model
        USP_SNPRINTF(path, sizeof(path), DEVICE_SERVICE_ROOT ".%d", us->instance);
        err = USP_DM_InformInstance(path);
        if (err != USP_ERR_OK)
        {
            resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_REGISTER_FAILURE, resp);
            goto exit;
        }
    }

    // Create a Register Response message
    resp = CreateRegisterResp(usp->header->msg_id);
    reg_resp = resp->body->response->register_resp;

    // Iterate over all paths in the request message, checking that they do not conflict
    // with any other paths which have already been registered (i.e. owned by the USP Broker or any other USP Service)
    for (i=0; i < reg->n_reg_paths; i++)
    {
        rp = reg->reg_paths[i];
        USP_ASSERT((rp != NULL) && (rp->path != NULL));

        err = ShouldPathBeAddedToDataModel(us, rp->path);
        if (err == USP_ERR_OK)
        {
            // Path should be added to data model
            STR_VECTOR_Add(&accepted_paths, rp->path);
        }
        else
        {
            // Path should not be added to data model
            // Exit if we are not allowing partial registration (in which case, no paths are registered)
            if (allow_partial == false)
            {
                resp = ERROR_RESP_CreateSingle(usp->header->msg_id, err, resp);
                STR_VECTOR_Destroy(&accepted_paths);
                goto exit;
            }
        }

        // Add the registered path result (which may be successful or unsuccessful)
        AddRegisterResp_RegisteredPathResult(reg_resp, rp->path, err);
    }

    // Add all accepted paths to the list of registered paths
    for (i=0; i<accepted_paths.num_entries; i++)
    {
        STR_VECTOR_Add(&us->registered_paths, accepted_paths.vector[i]);
    }

exit:
    // Queue the response, if one was created
    if (resp != NULL)
    {
        MSG_HANDLER_QueueMessage(endpoint_id, resp, mtpc);
        usp__msg__free_unpacked(resp, pbuf_allocator);
    }

    // If any paths were accepted, then register the paths into the data model and
    // kick off a query to get the supported data model for the registered paths
    if (accepted_paths.num_entries > 0)
    {
        QueueGetSupportedDMToUspService(us, &accepted_paths);
    }

    STR_VECTOR_Destroy(&accepted_paths);
}

/*********************************************************************//**
**
** USP_BROKER_HandleDeRegister
**
** Handles a USP DeRegister message
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP service which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void USP_BROKER_HandleDeRegister(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    Usp__Msg *resp = NULL;
    int i;
    int err;
    usp_service_t *us;
    Usp__Deregister *dreg;
    Usp__DeregisterResp *dreg_resp;
    char *path;
    bool is_valid;
    bool is_exception;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_DEREGISTER) ||
        (usp->body->request->deregister == NULL) )
    {
        USP_ERR_SetMessage("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        resp = ERROR_RESP_CreateSingle(usp->header->msg_id, USP_ERR_MESSAGE_NOT_UNDERSTOOD, resp);
        goto exit;
    }
    dreg = usp->body->request->deregister;

    // Create a Deregister Response message
    resp = CreateDeRegisterResp(usp->header->msg_id);
    dreg_resp = resp->body->response->deregister_resp;

    // Exit if endpoint is not a USP Service
    us = FindUspServiceByEndpoint(endpoint_id);
    if ((us == NULL) || (us->registered_paths.num_entries == 0))
    {
        USP_ERR_SetMessage("%s: Endpoint '%s' has not registered any paths", __FUNCTION__, endpoint_id);
        for (i=0; i < dreg->n_paths; i++)
        {
            path = dreg->paths[i];
            AddDeRegisterResp_DeRegisteredPathResult(dreg_resp, path, path, USP_ERR_DEREGISTER_FAILURE, USP_ERR_GetMessage());
        }
        goto exit;
    }

    // Exit if there are any outstanding GSDM requests
    // In this case, we disallow the deregister until after the GSDM response sequence has completed, to prevent the registered data model being inconsistent with the GSDM response
    if (us->gsdm_msg_ids.num_entries > 0)
    {
        USP_ERR_SetMessage("%s: Cannot deregister whilst registration follow-on sequence in progress", __FUNCTION__);
        for (i=0; i < dreg->n_paths; i++)
        {
            path = dreg->paths[i];
            AddDeRegisterResp_DeRegisteredPathResult(dreg_resp, path, path, USP_ERR_DEREGISTER_FAILURE, USP_ERR_GetMessage());
        }
        goto exit;
    }

    // Iterate over all paths in the deregister message, deregistering each one
    for (i=0; i < dreg->n_paths; i++)
    {
        path = dreg->paths[i];
        if (*path == '\0')
        {
            // Special case of deregistering all paths owned by the USP Service
            DeRegisterAllPaths(us, dreg_resp);
        }
        else
        {
            is_exception = IsPathASpecialException(path, INVALID);
            if (is_exception)
            {
                // Deregister one of the special exception paths
                err = DeRegisterUspServicePath(us, path);
            }
            else
            {
                is_valid = IsValidUspServicePath(path);
                if (is_valid)
                {
                    // Deregister a DM object (normal case)
                    err = DeRegisterUspServicePath(us, path);
                }
                else
                {
                    // Path to deregister was invalid from a textual perspective and wasn't a special exception
                    err = USP_ERR_DEREGISTER_FAILURE;
                }
            }

            // Add the result to the Deregister response
            AddDeRegisterResp_DeRegisteredPathResult(dreg_resp, path, path, err, USP_ERR_GetMessage());
        }
    }

exit:
    // Queue the response, if one was created
    if (resp != NULL)
    {
        MSG_HANDLER_QueueMessage(endpoint_id, resp, mtpc);
        usp__msg__free_unpacked(resp, pbuf_allocator);
    }
}

/*********************************************************************//**
**
** USP_BROKER_HandleGetSupportedDMResp
**
** Handles a USP GetSupportedDM response message
** This response will have been initiated by the USP Broker, in order to discover the data model of a USP Service
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP service which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void USP_BROKER_HandleGetSupportedDMResp(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    int i;
    Usp__GetSupportedDMResp *gsdm;
    Usp__GetSupportedDMResp__RequestedObjectResult *ror;
    usp_service_t *us;
    int index;
    bool is_accepted;
    str_vector_t ipaths;
    dm_node_t *node;

    // NOTE: Errors in response messages should be ignored according to R-MTP.5 (they should not send a USP ERROR response)

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_RESPONSE) ||
        (usp->body->response == NULL) || (usp->body->response->resp_type_case != USP__RESPONSE__RESP_TYPE_GET_SUPPORTED_DM_RESP) ||
        (usp->body->response->get_supported_dm_resp == NULL) )
    {
        USP_LOG_Error("%s: Incoming message is invalid or inconsistent", __FUNCTION__);
        return;
    }

    // Exit if endpoint is not a USP Service
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        USP_LOG_Error("%s: Incoming GSDM Response is from an unexpected endpoint (%s)", __FUNCTION__, endpoint_id);
        return;
    }

    // Exit if we are not expecting this GSDM response
    index = STR_VECTOR_Find(&us->gsdm_msg_ids, usp->header->msg_id);
    if (index == INVALID)
    {
        USP_LOG_Error("%s: Ignoring GSDM response from endpoint '%s' because msg_id='%s' was unexpected", __FUNCTION__, endpoint_id, usp->header->msg_id);
        return;
    }

    // Exit if the reponse did not contain the GSDM of any paths
    gsdm = usp->body->response->get_supported_dm_resp;
    if (gsdm->n_req_obj_results == 0)
    {
        USP_LOG_Error("%s: Incoming GSDM Response from endpoint_is=%s  contains no results", __FUNCTION__, endpoint_id);
        return;
    }

    // Since we've received the response now, free the expected msg_id
    STR_VECTOR_RemoveByIndex(&us->gsdm_msg_ids, index);

    // Iterate over all RequestedObjectResults, registering the data model elements provided
    // by this USP service into this USP Broker's data model
    STR_VECTOR_Init(&ipaths);
    for (i=0; i < gsdm->n_req_obj_results; i++)
    {
        ror = gsdm->req_obj_results[i];
        is_accepted = ProcessGsdm_RequestedPath(ror, us);
        if (is_accepted)
        {
            // Add all top level multi-instance objects under this path to ipaths
            // NOTE: We cannot just GetInstances to this path directly because GetInstances requires paths to multi-instance objects
            node = DM_PRIV_GetNodeFromPath(ror->req_obj_path, NULL, NULL, 0);
            USP_ASSERT(node != NULL);
            AddTopLevelTableNodes(node, &ipaths);
        }
    }

    // Apply permissions to the nodes that have just been added
    ApplyPermissionsToUspService(us);

    // Ensure that the USP Service contains only the subscriptions which it is supposed to
    SyncSubscriptions(us);

    // Get a baseline set of instances for this USP Service into the instance cache
    // This is necessary, otherwise an Object creation subscription that uses the legacy polling mechanism (via refresh instances vendor hook)
    // may erroneously fire, immediately after this service has registered
    if (ipaths.num_entries > 0)
    {
        UspService_RefreshInstances(us, &ipaths, false);
    }

    STR_VECTOR_Destroy(&ipaths);
}

/*********************************************************************//**
**
** USP_BROKER_HandleNotification
**
** Handles a USP Notification message received from a USP Service
** This function determines which USP Controller (connected to the USP Broker) set the subscription on the Broker
** and forwards the notification to it
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP service which sent this message
** \param   mtpc - details of where response to this USP message should be sent
**
** \return  None - This code must handle any errors by sending back error messages
**
**************************************************************************/
void USP_BROKER_HandleNotification(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc)
{
    int err;
    Usp__Notify *notify;
    usp_service_t *us;
    subs_map_t *smap;
    Usp__Notify__OperationComplete *op;
    int items_converted;
    int broker_instance;
    bool is_match;

    // Exit if message is invalid or failed to parse
    // This code checks the parsed message enums and pointers for expectations and validity
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_NOTIFY) ||
        (usp->body->request->notify == NULL) )
    {
        USP_ERR_SetMessage("%s: Notification is invalid or inconsistent", __FUNCTION__);
        err = USP_ERR_REQUEST_DENIED;
        goto exit;
    }

    // Exit if the notification is expecting a response (because we didn't ask for that)
    notify = usp->body->request->notify;
    if (notify->send_resp == true)
    {
        USP_ERR_SetMessage("%s: Notification has send_resp=true, but subscription was setup with NotifRetry=false", __FUNCTION__);
        err = USP_ERR_REQUEST_DENIED;
        goto exit;
    }

    // Exit if endpoint is not a USP Service
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        USP_ERR_SetMessage("%s: Notification is from an unexpected endpoint (%s)", __FUNCTION__, endpoint_id);
        err = USP_ERR_REQUEST_DENIED;
        goto exit;
    }

    // Exit if the Subscription ID was not created by the Broker
    if (strstr(notify->subscription_id, broker_unique_str) == NULL)
    {
        USP_ERR_SetMessage("%s: Notification is not for the Broker (subs_id=%s)", __FUNCTION__, notify->subscription_id);
        err = USP_ERR_REQUEST_DENIED;
        goto exit;
    }

    // Exit if unable to extract the broker's subscription instance number from the subscription ID
    items_converted = sscanf(notify->subscription_id, "%d", &broker_instance);
    if (items_converted != 1)
    {
        USP_ERR_SetMessage("%s: Notification contains unexpected subscription Id (%s)", __FUNCTION__, notify->subscription_id);
        err = USP_ERR_REQUEST_DENIED;
        goto exit;
    }

    // Exit if the subscription_id of the received notification doesn't match any that we are expecting
    smap = SubsMap_FindByUspServiceSubsId(&us->subs_map, notify->subscription_id, broker_instance);
    if (smap == NULL)
    {
        // Subscription doesn't match any that have been paired up
        // However this may be because the notification was received before the USP Service registration sequence has completed (eg Device.Boot!)

        // Exit if the notification does not match the expected entry in the Broker's subscription table
        is_match = IsUspServiceNotificationMatch(notify, us, broker_instance);
        if (is_match == false)
        {
            err = USP_ERR_REQUEST_DENIED;
            goto exit;
        }
    }

    // Forward the notification back to the controller that set up the subscription on the Broker
    err = DEVICE_SUBSCRIPTION_RouteNotification(usp, broker_instance);

    // If this is an OperationComplete notification, then delete the associated request
    // in the Broker's Request table and from this USP Service's request mapping table
    if (notify->notification_case == USP__NOTIFY__NOTIFICATION_OPER_COMPLETE)
    {
        op = notify->oper_complete;
        DeleteMatchingOperateRequest(us, op->obj_path, op->command_name, op->command_key);
    }

exit:
    // Send a USP ERROR response if an error was detected (as per R-MTP.5)
    if (err != USP_ERR_OK)
    {
        MSG_HANDLER_QueueErrorMessage(err, endpoint_id, mtpc, usp->header->msg_id);
    }
}

/*********************************************************************//**
**
** USP_BROKER_IsPathVendorSubscribable
**
** Determines whether the specified path can be handled by a vendor layer subscription
**
** \param   notify_type - Type of subscription
** \param   path - data model path under consideration
** \param   is_present - pointer to variable in which to return whether the path is present in the data model or NULL if the caller does not care about this
**                       (This is used to decide whether to delete a subscription on a USP service when syncing the subscriptions)
**
** \return  group_id of the data model provider component that can handle this subscription,
**          or NON_GROUPED, if the path cannot be subscribed to in the vendor layer
**
**************************************************************************/
int USP_BROKER_IsPathVendorSubscribable(subs_notify_t notify_type, char *path, bool *is_present)
{
    dm_node_t *node;

    // Keep the compiler happy, as notify_type is not currently used
    // notify_type is included in the API, because it could potentially be used in the future as part of the decision
    (void)notify_type;

    // Determine whether the path is an absolute path, wildcarded path or partial path
    // We believe that all USP Services support subscribing to paths of these types
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);

    // Fill in whether the path was present in the data model
    if (is_present != NULL)
    {
        *is_present = (node == NULL) ? false : true;
    }

    // Exit if this path is not subscribable in the vendor layer
    // i.e. it is either not present in the data model or contains search expressions or reference follows
    if (node == NULL)
    {
        return NON_GROUPED;
    }

    return node->group_id;
}

/*********************************************************************//**
**
** USP_BROKER_GetUspServiceInstance
**
** Determines the instance number in Device.USPServices.USPService.{i} with the specified EndpointID
**
** \param   endpoint_id - endpoint of USP service to find
** \param   flags - Bitmask of flags controling operations e.g. ONLY_CONTROLLER_CONNECTIONS
**
** \return  instance number in Device.USPServices.USPService.{i} or INVALID if no USP Service is currenty connected with the specified EndpointID
**
**************************************************************************/
int USP_BROKER_GetUspServiceInstance(char *endpoint_id, unsigned flags)
{
    usp_service_t *us;

    // Exit if endpoint is not connected as a USP Service
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        return INVALID;
    }

    // Exit if the caller wanted only USP Services acting as Controllers (i.e. connected on the Broker's agent connection)
    if ((flags & ONLY_CONTROLLER_CONNECTIONS) & (us->has_controller==false))
    {
        return INVALID;
    }

    return us->instance;
}

/*********************************************************************//**
**
** USP_BROKER_GetAllRegisteredGroupIds
**
** Returns a list of all group_ids of USP Services which have registered a data model path
**
** \param   iv - pointer to int vector in which to return all group_ids currently registered
**
** \return  None
**
**************************************************************************/
void USP_BROKER_GetAllRegisteredGroupIds(int_vector_t *iv)
{
    int i;
    usp_service_t *us;

    // Iterate over all USP services adding those that have registered data model elements to the list
    INT_VECTOR_Init(iv);
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if ((us->instance != INVALID) && (us->registered_paths.num_entries > 0))
        {
            INT_VECTOR_Add(iv, us->group_id);
        }
    }
}

/*********************************************************************//**
**
** USP_BROKER_GetNotifyDestForEndpoint
**
** Determines a destination MTP to send a USP Record to based on the endpoint to send it to
** This function is usually used to determine the destination MTP for USP notifications
**
** \param   endpoint_id - endpoint to send the message to
** \param   usp_msg_type - type of the USP message to be sent
**
** \return  pointer to mtp_conn destination or NULL if none found
**
**************************************************************************/
mtp_conn_t *USP_BROKER_GetNotifyDestForEndpoint(char *endpoint_id, Usp__Header__MsgType usp_msg_type)
{
    usp_service_t *us;
    mtp_conn_t *mtpc;

    // Exit if destination endpoint is not connected as a USP Service
    // NOTE: If this agent is running as a USP Service, then the destination endpoint is a Broker, not a USP Service, and will not appear in the USPServices table
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        return NULL;
    }

    // Determine whether to send the USP message from either the Broker's controller or the Broker's agent connection
    // (Most types of messages can ony be sent from one or other connection, as they are either controller or agent initiated messages)
    switch(usp_msg_type)
    {
        case USP__HEADER__MSG_TYPE__ERROR:
            // The code shouldn't get here for USP Error messages, as they are response messages
            // (so this function should not have been called) and can be sent from either the Broker's Controller
            // or the Broker's Agent, so this function cannot determine which to use
            USP_ASSERT(usp_msg_type != USP__HEADER__MSG_TYPE__ERROR);
            return NULL; // Needed otherwise the compiler thinks that mtpc may be uninitialised
            break;

        case USP__HEADER__MSG_TYPE__GET:
        case USP__HEADER__MSG_TYPE__SET:
        case USP__HEADER__MSG_TYPE__ADD:
        case USP__HEADER__MSG_TYPE__DELETE:
        case USP__HEADER__MSG_TYPE__OPERATE:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES:
        case USP__HEADER__MSG_TYPE__NOTIFY_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
        case USP__HEADER__MSG_TYPE__REGISTER_RESP:
        case USP__HEADER__MSG_TYPE__DEREGISTER_RESP:
            mtpc = &us->controller_mtp;
            break;

        case USP__HEADER__MSG_TYPE__GET_RESP:
        case USP__HEADER__MSG_TYPE__SET_RESP:
        case USP__HEADER__MSG_TYPE__ADD_RESP:
        case USP__HEADER__MSG_TYPE__DELETE_RESP:
        case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        case USP__HEADER__MSG_TYPE__NOTIFY:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP:
        case USP__HEADER__MSG_TYPE__REGISTER:
        case USP__HEADER__MSG_TYPE__DEREGISTER:
            mtpc = &us->agent_mtp;
            break;

        default:
            TERMINATE_BAD_CASE(usp_msg_type);
            return NULL; // Needed otherwise the compiler thinks that mtpc may be uninitialised
            break;
    }

    // Exit if the USP service has connected to the Broker, but not via the correct UDS socket for the message type
    if (mtpc->is_reply_to_specified == false)
    {
        return NULL;
    }

    return mtpc;
}

/*********************************************************************//**
**
** USP_BROKER_AttemptPassthru
**
** If the USP Message is a request, then route it to the relevant USP Service, if it can be satisfied by a single USP Service
** and there are no permissions preventing the request being fulfilled
** If the USP Message is a response to a previous passthru message, then route it back to the original requestor
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   combined_role - roles that the originator has (inherited & assigned)
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool USP_BROKER_AttemptPassthru(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec)
{
    USP_ASSERT(combined_role != INTERNAL_ROLE);

    switch(usp->header->msg_type)
    {
        case USP__HEADER__MSG_TYPE__GET:
            return AttemptPassThruForGetRequest(usp, endpoint_id, mtpc, combined_role, rec);
            break;

        case USP__HEADER__MSG_TYPE__SET:
            return AttemptPassThruForSetRequest(usp, endpoint_id, mtpc, combined_role, rec);
            break;

        case USP__HEADER__MSG_TYPE__ADD:
            return AttemptPassThruForAddRequest(usp, endpoint_id, mtpc, combined_role, rec);
            break;

        case USP__HEADER__MSG_TYPE__DELETE:
            return AttemptPassThruForDeleteRequest(usp, endpoint_id, mtpc, combined_role, rec);
            break;

        case USP__HEADER__MSG_TYPE__ERROR:
            return AttemptPassThruForResponse(usp, endpoint_id);
            break;

        case USP__HEADER__MSG_TYPE__GET_RESP:
        case USP__HEADER__MSG_TYPE__SET_RESP:
        case USP__HEADER__MSG_TYPE__ADD_RESP:
        case USP__HEADER__MSG_TYPE__DELETE_RESP:
            return AttemptPassThruForResponse(usp, endpoint_id);
            break;

        case USP__HEADER__MSG_TYPE__NOTIFY:
            return AttemptPassThruForNotification(usp, endpoint_id, mtpc, rec);
            break;

        case USP__HEADER__MSG_TYPE__OPERATE:
        case USP__HEADER__MSG_TYPE__OPERATE_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_DM_RESP:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES:
        case USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP:
        case USP__HEADER__MSG_TYPE__NOTIFY_RESP:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO:
        case USP__HEADER__MSG_TYPE__GET_SUPPORTED_PROTO_RESP:
        case USP__HEADER__MSG_TYPE__REGISTER:
        case USP__HEADER__MSG_TYPE__REGISTER_RESP:
        case USP__HEADER__MSG_TYPE__DEREGISTER:
        case USP__HEADER__MSG_TYPE__DEREGISTER_RESP:
        default:
            // These messages are not supported for passthru, so exit
            return false;
            break;
    }

    return false;
}

/*********************************************************************//**
**
** USP_BROKER_AttemptDirectGetForCli
**
** This function sees if it's possible to perform a CLI initiated Get, without resolving the path on the Broker first
** This is similar to performing a passthru optimization for CLI initiated Gets
** NOTE: Only absolute paths, partial paths and wildcarded paths are supported by this function (Reference follow and search expressions are not)
**
** \param   path - path expression to get
**
** \return  true if the get has been handled here, false if the caller should perform path resolution and the get
**
**************************************************************************/
bool USP_BROKER_AttemptDirectGetForCli(char *path)
{
    int i;
    int err;
    dm_node_t *node;
    kv_vector_t kvv;
    kv_pair_t kv;
    Usp__Msg *req;
    Usp__Msg *resp = NULL;
    usp_service_t *us;
    char msg_id[MAX_MSG_ID_LEN];

    // Exit if this path does not exist in the data model, or is not an absolute, partial or wildcarded path
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
    if (node == NULL)
    {
        return false;
    }

    // Exit, if no USP Service associated with the group_id
    us = FindUspServiceByGroupId(node->group_id);
    if (us == NULL)
    {
        return false;
    }

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return true;
    }

    // Form the USP Get Request message
    kv.key = path;
    kv.value = NULL;
    kvv.vector = &kv;
    kvv.num_entries = 1;
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_GetReq(msg_id, &kvv);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    #define CLI_GET_RESPONSE_TIMEOUT (3*60)        // 3 minutes
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__GET_RESP,
                                                 CLI_GET_RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return true;
    }

    // Exit if unable to process the get response, retrieving the parameter values and putting them into the key-value-vector output argument
    KV_VECTOR_Init(&kvv);
    err = MSG_UTILS_ProcessUspService_GetResponse(resp, &kvv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Print out the values of all parameters retrieved
    for (i=0; i < kvv.num_entries; i++)
    {
        CLI_SERVER_SendResponse(kvv.vector[i].key);
        CLI_SERVER_SendResponse(" => ");
        CLI_SERVER_SendResponse(kvv.vector[i].value);
        CLI_SERVER_SendResponse("\n");
    }

exit:
    KV_VECTOR_Destroy(&kvv);
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return true;
}

/*********************************************************************//**
**
** AddUspService
**
** Called when a USP Service has connected and sent a register message
**
** \param   endpoint_id - endpoint of USP service to register
** \param   mtpc - pointer to structure specifying which protocol (and MTP instance) the endpoint is using
**
** \return  pointer to entry in usp_services[] or NULL if an error occurred
**
**************************************************************************/
usp_service_t *AddUspService(char *endpoint_id, mtp_conn_t *mtpc)
{
    usp_service_t *us;
    int group_id;

    // Exit if no free entries in the usp_services array
    us = FindUnusedUspService();
    if (us == NULL)
    {
        USP_ERR_SetMessage("%s: Too many USP services (%d) already registered. Increase MAX_USP_SERVICES", __FUNCTION__, MAX_USP_SERVICES);
        return NULL;
    }

    // Exit if no free group_id to assign to this USP service
    group_id = DATA_MODEL_FindUnusedGroupId();
    if (group_id == INVALID)
    {
        USP_ERR_SetMessage("%s: No free group id. Increase MAX_VENDOR_PARAM_GROUPS from %d", __FUNCTION__, MAX_VENDOR_PARAM_GROUPS);
        return NULL;
    }

    // Initialise the USP Service
    memset(us, 0, sizeof(usp_service_t));
    us->instance = CalcNextUspServiceInstanceNumber();
    us->endpoint_id = USP_STRDUP(endpoint_id);
    us->group_id = group_id;
    us->has_controller = false;
    STR_VECTOR_Init(&us->gsdm_msg_ids);
    STR_VECTOR_Init(&us->registered_paths);
    SubsMap_Init(&us->subs_map);
    ReqMap_Init(&us->req_map);
    MsgMap_Init(&us->msg_map);
    us->controller_mtp.protocol = kMtpProtocol_None;
    us->agent_mtp.protocol = kMtpProtocol_None;

    // Mark the group_id as 'in-use' in the data model by registering group vendor hooks for it
    RegisterBrokerVendorHooks(us);

    // Store the connection details for this USP Service
    UpdateUspServiceMRT(us, mtpc);

    return us;
}

/*********************************************************************//**
**
** UpdateUspServiceMRT
**
** Called to add or update the info for the connection to the specified USP Service
**
** \param   us - USP Service whose connection info needs updating
** \param   mtpc - pointer to structure specifying which protocol (and MTP instance) the endpoint is using
**
** \return  None
**
**************************************************************************/
void UpdateUspServiceMRT(usp_service_t *us, mtp_conn_t *mtpc)
{

#ifdef ENABLE_UDS
    if (mtpc->protocol == kMtpProtocol_UDS)
    {
        // The UDS MTP uses different connections for sending the Broker's controller and agent messages
        // So decide which one to copy these connection details into
        mtp_conn_t *dest;
        switch(mtpc->uds.path_type)
        {
            case kUdsPathType_BrokersAgent:
                dest = &us->agent_mtp;
                break;

            case kUdsPathType_BrokersController:
                dest = &us->controller_mtp;
                break;

            default:
                TERMINATE_BAD_CASE(mtpc->uds.path_type);
                return; // Needed otherwise the compiler thinks that dest may be uninitialised
                break;
        }

        if (dest->protocol != kMtpProtocol_None)
        {
            DM_EXEC_FreeMTPConnection(dest);
        }
        DM_EXEC_CopyMTPConnection(dest, mtpc);
    }
    else
#endif
    {
        // All other MTP protocols use the same connection for sending the Broker's controller and agent messages
        if (us->controller_mtp.protocol != kMtpProtocol_None)
        {
            DM_EXEC_FreeMTPConnection(&us->controller_mtp);
        }
        DM_EXEC_CopyMTPConnection(&us->controller_mtp, mtpc);

        if (us->agent_mtp.protocol != kMtpProtocol_None)
        {
            DM_EXEC_FreeMTPConnection(&us->agent_mtp);
        }
        DM_EXEC_CopyMTPConnection(&us->agent_mtp, mtpc);
    }

}

/*********************************************************************//**
**
** DeRegisterUspServicePath
**
** Deregisters a data model path which the specified USP Service is providing
**
** \param   us - pointer to USP service in usp_services[]
** \param   path - path of the data model object to deregister
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DeRegisterUspServicePath(usp_service_t *us, char *path)
{
    int i;
    int index;
    subs_map_t *smap;
    subs_map_t *next_smap;
    req_map_t *rmap;
    req_map_t *next_rmap;
    dm_node_t *parent;
    bool is_child;
    char err_msg[128];
    int err;
    int len;
    char *rpath;
    int rpath_len;

    // Exit if this endpoint did not register the specified path
    index = STR_VECTOR_Find(&us->registered_paths, path);
    if (index == INVALID)
    {
        USP_ERR_SetMessage("%s: Path never registered by endpoint_id=%s", __FUNCTION__, us->endpoint_id);
        return USP_ERR_DEREGISTER_FAILURE;
    }

    // Determine the data model node representing the top level path to deregister
    parent = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    USP_ASSERT(parent != NULL);

    // Iterate over all subscriptions on the USP Service, unsubscribing from those which are not owned by the USP Service anymore
    // and marking them as being provided by the core mechanism
    smap = (subs_map_t *) us->subs_map.head;
    while (smap != NULL)
    {
        next_smap = (subs_map_t *) smap->link.next;     // Save off the next pointer, as ths entry may get deleted by DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs()

        is_child = DM_PRIV_IsChildOf(smap->path, parent);
        if (is_child)
        {
            err = DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs(us->group_id, smap->broker_instance, smap->service_instance, smap->path);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
        smap = next_smap;
    }

    // If this is the last path to be deregistered from the USP Service, then unsubscribe all 'Device.' subscriptions
    if (us->registered_paths.num_entries == 1)
    {
        USP_ASSERT(index == 0);

        smap = (subs_map_t *) us->subs_map.head;
        while (smap != NULL)
        {
            next_smap = (subs_map_t *) smap->link.next;     // Save off the next pointer, as ths entry may get deleted by DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs()

            if (strcmp(smap->path, dm_root)==0)
            {
                err = DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs(us->group_id, smap->broker_instance, smap->service_instance, smap->path);
                if (err != USP_ERR_OK)
                {
                    return err;
                }
            }
            smap = next_smap;
        }
    }

    // Send an OperationComplete indicating failure for all currently active USP Commands which are children of the path being deregistered
    // This also results in the entry in the Broker's Request table for the USP Command being deleted
    rmap = (req_map_t *) us->req_map.head;
    while (rmap != NULL)
    {
        next_rmap = (req_map_t *) rmap->link.next;     // Save off the next pointer, as ths entry may get deleted by DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs()

        is_child = DM_PRIV_IsChildOf(rmap->path, parent);
        if (is_child)
        {
            USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: USP Service %s deregistered %s whilst command was in progress", __FUNCTION__, us->endpoint_id, path);
            DEVICE_REQUEST_OperationComplete(rmap->request_instance, USP_ERR_COMMAND_FAILURE, err_msg, NULL);
            ReqMap_Remove(&us->req_map, rmap);
        }

        rmap = next_rmap;
    }

    // NOTE: There is no need to remove any entries from the passthru map because the USP Service will still respond
    // to those messages, just possibly with an error stating that the requested object is not owned by it anymore

    // Remove the specified path from the supported data model (the instance cache for this object will also be removed)
    DATA_MODEL_DeRegisterPath(path);

    // Remove the path from the list of paths that were registered as owned by the USP Service
    STR_VECTOR_RemoveByIndex(&us->registered_paths, index);

    // Remove all children of this path from the registered path list
    len = strlen(path);
    for (i=0; i < us->registered_paths.num_entries; i++)
    {
        rpath = us->registered_paths.vector[i];
        rpath_len = strlen(rpath);
        if ((rpath_len > len) && (memcmp(rpath, path, len)==0))
        {
            USP_FREE(rpath);
            us->registered_paths.vector[i] = NULL;
        }
    }
    STR_VECTOR_RemoveUnusedEntries(&us->registered_paths);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FreeUspService
**
** Frees all memory associated with the specified USP servce and marks the USP Service as not in use
**
** \param   us - pointer to USP service in usp_services[]
**
** \return  None
**
**************************************************************************/
void FreeUspService(usp_service_t *us)
{
    // Free all dynamically allocated memory associated with this entry
    USP_SAFE_FREE(us->endpoint_id);
    DM_EXEC_FreeMTPConnection(&us->controller_mtp);
    DM_EXEC_FreeMTPConnection(&us->agent_mtp);
    STR_VECTOR_Destroy(&us->gsdm_msg_ids);

    STR_VECTOR_Destroy(&us->registered_paths);
    SubsMap_Destroy(&us->subs_map);
    ReqMap_Destroy(&us->req_map);
    MsgMap_Destroy(&us->msg_map);

    // Mark the entry as not-in-use
    us->instance = INVALID;
}

/*********************************************************************//**
**
** QueueGetSupportedDMToUspService
**
** Sends a GetSupportedDM request to the specified USP Service
** And registers all paths owned by the USP Service into the data model
**
** \param   us - pointer to USP service in usp_services[]
** \param   paths - pointer to string vector containing paths to include in the GSDM request
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void QueueGetSupportedDMToUspService(usp_service_t *us, str_vector_t *paths)
{
    int i;
    Usp__Msg *req = NULL;
    char msg_id[MAX_MSG_ID_LEN];
    dm_node_t *node;
    dm_object_info_t *info;
    char *path;
    bool is_exception;

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return;
    }

    // Register all special exception paths into the Broker's data model
    // NOTE: We cannot put these paths into the GSDM request as they are not objects
    // So instead we register them into the Broker's data model directly here, based on them being standard TR181 data model elements
    // and we remove them from the paths vector so that they are not put into the GSDM request later in this function
    for (i=0; i < paths->num_entries; i++)
    {
        path = paths->vector[i];
        is_exception = IsPathASpecialException(path, us->group_id);
        if (is_exception == true)
        {
            // Apply permissions to the special exception paths that have just been added
            DEVICE_CTRUST_ApplyPermissionsToSubTree(path);
            USP_FREE(path);
            paths->vector[i] = NULL;   // Mark the entry as unused
        }
    }

    // Exit if after removing all special exception paths, there are no paths left to get the supported data model of
    STR_VECTOR_RemoveUnusedEntries(paths);
    if (paths->num_entries == 0)
    {
        // Sync the Broker's subscriptions with the USP Service now, because we're skipping the GSDM part of the registration sequence
        // NOTE: No need to call UspService_RefreshInstances(), as the paths being registered are all special exception paths, which we know do not contain instance numbers
        SyncSubscriptions(us);
        return;
    }

    // Create the GSDM request
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_GetSupportedDMReq(msg_id, paths);

    // Queue the GSDM request
    MSG_HANDLER_QueueMessage(us->endpoint_id, req, &us->controller_mtp);
    usp__msg__free_unpacked(req, pbuf_allocator);

    // Add the msg_id of the GSDM request to the list of GSDM responses we're expecting
    STR_VECTOR_Add(&us->gsdm_msg_ids, msg_id);

    // Register these paths owned by the USP Service into the data model as single instance objects
    // This is necessary to ensure that no other USP Services can register the same path
    // Whether the path is actually a single or multi-instance object will be discovered (and correctly set) when the GSDM response is processed
    // NOTE: The path could already exist in the data model, if it had been registered as a parent of a child object previously
    //       eg now registering Device.Parent and previously registered Device.Parent.Child
    for (i=0; i < paths->num_entries; i++)
    {
        path = paths->vector[i];
        node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_SingleInstance, SUPPRESS_PRE_EXISTANCE_ERR);
        if (node != NULL)
        {
            info = &node->registered.object_info;
            memset(info, 0, sizeof(dm_object_info_t));
            node->group_id = us->group_id;
            info->group_writable = false;
            DM_INST_VECTOR_Init(&info->inst_vector);
        }
        else
        {
            USP_LOG_Error("%s: Requested path '%s' could not be registered into the data model", __FUNCTION__, path);
        }
    }
}

/*********************************************************************//**
**
** ApplyPermissionsToUspService
**
** Calculates the permissions for all nodes owned by the specified USP Service
**
** \param   us - pointer to USP service in usp_services[]
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void ApplyPermissionsToUspService(usp_service_t *us)
{
    int i;
    char *path;

    // Iterate over all paths registered for the USP Service
    for (i=0; i < us->registered_paths.num_entries; i++)
    {
        path = us->registered_paths.vector[i];
        DEVICE_CTRUST_ApplyPermissionsToSubTree(path);
    }
}

/*********************************************************************//**
**
** RegisterBrokerVendorHooks
**
** Registers the USP Broker's set of vendor hooks for the specified USP Service
** This has the side effect of marking thr group_id of the USP service as 'in use'
**
** \param   us - pointer to USP service in usp_services[]
**
** \return  None
**
**************************************************************************/
void RegisterBrokerVendorHooks(usp_service_t *us)
{
    USP_REGISTER_GroupVendorHooks(us->group_id, Broker_GroupGet, Broker_GroupSet, Broker_GroupAdd, Broker_GroupDelete);
    USP_REGISTER_SubscriptionVendorHooks(us->group_id, Broker_GroupSubscribe, Broker_GroupUnsubscribe);
    USP_REGISTER_MultiDeleteVendorHook(us->group_id, Broker_MultiDelete);
    USP_REGISTER_CreateObjectVendorHook(us->group_id, Broker_CreateObj);
}

/*********************************************************************//**
**
** DeregisterBrokerVendorHooks
**
** Deregisters all of the USP Broker's vendor hooks for the specified USP Service
** This has the side effect of marking thr group_id of the USP service as 'not in use'
**
** \param   us - pointer to USP service in usp_services[]
**
** \return  None
**
**************************************************************************/
void DeregisterBrokerVendorHooks(usp_service_t *us)
{
    USP_REGISTER_GroupVendorHooks(us->group_id, NULL, NULL, NULL, NULL);
    USP_REGISTER_SubscriptionVendorHooks(us->group_id, NULL, NULL);
    USP_REGISTER_MultiDeleteVendorHook(us->group_id, NULL);
    USP_REGISTER_CreateObjectVendorHook(us->group_id, NULL);
}

/*********************************************************************//**
**
** Broker_GroupGet
**
** GroupGet vendor hook for parameters owned by the USP service
** This function sends a USP Get request in order to obtain the parameter values from the USP service
** Then it waits for a USP Get Response and parses it, to return the parameter values
**
** \param   group_id - group ID of the USP service
** \param   kvv - key-value vector containing the parameter names as keys
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_GroupGet(int group_id, kv_vector_t *kvv)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    usp_service_t *us;
    char msg_id[MAX_MSG_ID_LEN];

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form the USP Get Request message
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_GetReq(msg_id, kvv);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__GET_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the get response, retrieving the parameter values and putting them into the key-value-vector output argument
    err = ProcessGetResponse(resp, kvv);

    // Free the get response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** Broker_GroupSet
**
** GroupSet vendor hook for parameters owned by the USP service
** This function sends a USP Set request in order to set the parameter values in the USP service
** Then it waits for a USP Set Response and parses it, to return whether the set was successful
**
** \param   group_id - group ID of the USP service
** \param   params - key-value vector containing the parameter names as keys and the parameter values as values
** \param   param_types - UNUSED: array containing the type of each parameter in the params vector
** \param   failure_index - pointer to value in which to return the index of the first parameter in the params vector
**                          that failed to be set. This value is only consulted if an error is returned.
**                          Setting it to INVALID indicates that all parameters failed (e.g. communications failure)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_GroupSet(int group_id, kv_vector_t *params, unsigned *param_types, int *failure_index)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    usp_service_t *us;
    char msg_id[MAX_MSG_ID_LEN];

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Create Set Request
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_SetReq(msg_id, params);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__SET_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the set response, determining if it was successful or not
    err = ProcessSetResponse(resp, params, failure_index);

    // Free the set response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** Broker_GroupAdd
**
** GroupAdd vendor hook for objects owned by the USP service
** This function sends a USP Add request in order to add a new instance
** Then it waits for a USP Add Response and parses it, to return whether the add was successful
**
** \param   group_id - group ID of the USP service
** \param   path - path of the object in the data model (no trailing dot)
** \param   instance - pointer to variable in which to return instance number
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_GroupAdd(int group_id, char *path, int *instance)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    usp_service_t *us;
    char obj_path[MAX_DM_PATH];
    char msg_id[MAX_MSG_ID_LEN];

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form the USP Add Request message, ensuring that the path contains a trailing dot
    USP_SNPRINTF(obj_path, sizeof(obj_path), "%s.", path);
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_AddReq(msg_id, obj_path, NULL, 0);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__ADD_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the add response, determining if it was successful or not
    err = ProcessAddResponse(resp, obj_path, instance, NULL, NULL, 0);

    // Free the add response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** Broker_GroupDelete
**
** GroupDelete vendor hook for objects owned by the USP service
** This function sends a USP Delete request in order to delete an existing instance
** Then it waits for a USP Delete Response and parses it, to return whether the delete was successful
**
** \param   group_id - group ID of the USP service
** \param   path - path of the object in the data model (no trailing dot)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_GroupDelete(int group_id, char *path)
{
    int err;
    usp_service_t *us;
    char obj_path[MAX_DM_PATH];
    str_vector_t paths;
    char *single_path;

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form a statically allocated string vector containing a single instance (containing a trailing dot)
    USP_SNPRINTF(obj_path, sizeof(obj_path), "%s.", path);
    paths.num_entries = 1;
    paths.vector = &single_path;
    single_path = obj_path;

    // Send the Delete request and process the Delete response
    err = UspService_DeleteInstances(us, false, &paths, NULL);

    return err;
}

/*********************************************************************//**
**
** Broker_MultiDelete
**
** Multi Delete vendor hook for objects owned by the USP service
** This function sends a USP Delete request in order to delete a set of instances atomically (ie it uses allow_partial=false)
** Then it waits for a USP Delete Response and parses it, to return whether the delete was successful
**
** \param   group_id - group ID of the USP service
** \param   allow_partial - if set to false, if any of the objects fails to delete, then none should be deleted
** \param   paths - pointer to array of strings containing the objects to delete
** \param   num_paths - number of objects to delete
** \param   failure_index - pointer to variable in which to return the index of the first object which failed to delete in the paths array
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_MultiDelete(int group_id, bool allow_partial, char **paths, int num_paths, int *failure_index)
{
    int i;
    int err;
    usp_service_t *us;
    str_vector_t obj_paths;

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form a string vector from the array passed in, containing all of the paths with a trailing dot added
    obj_paths.num_entries = num_paths;
    obj_paths.vector = USP_MALLOC(num_paths*sizeof(char *));
    for (i=0; i<num_paths; i++)
    {
        obj_paths.vector[i] = TEXT_UTILS_StrDupWithTrailingDot(paths[i]);
    }

    // Send the Delete request and process the Delete response
    err = UspService_DeleteInstances(us, allow_partial, &obj_paths, failure_index);
    STR_VECTOR_Destroy(&obj_paths);

    return err;
}

/*********************************************************************//**
**
** Broker_CreateObj
**
** Create Object vendor hook for objects owned by the USP service
** This function sends a USP Add request with child params in order to add a new instance
** Then it waits for a USP Add Response and parses it, to return whether the add was successful, and the unique keys if it was
**
** \param   group_id - group ID of the USP service
** \param   path - path of the object in the data model (no trailing dot)
** \param   params - pointer to array containing the child parameters and their input and output arguments
** \param   num_params - number of child parameters to set
** \param   instance - pointer to variable in which to return instance number of the successfully created object
** \param   unique_keys - pointer to key-value vector in which to return the name and values of the unique keys for the object
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_CreateObj(int group_id, char *path, group_add_param_t *params, int num_params, int *instance, kv_vector_t *unique_keys)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    usp_service_t *us;
    char obj_path[MAX_DM_PATH];
    char msg_id[MAX_MSG_ID_LEN];

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form the USP Add Request message, ensuring that the path contains a trailing dot
    USP_SNPRINTF(obj_path, sizeof(obj_path), "%s.", path);
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_AddReq(msg_id, obj_path, params, num_params);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__ADD_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the add response, determining if it was successful or not
    err = ProcessAddResponse(resp, obj_path, instance, unique_keys, params, num_params);

    // Free the add response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** Broker_SyncOperate
**
** Sync Operation vendor hook for USP commands owned by USP Services
**
** \param   req - pointer to structure identifying the operation in the data model
** \param   command_key - pointer to string containing the command key for this operation
** \param   input_args - vector containing input arguments and their values
** \param   output_args - vector to return output arguments in
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_SyncOperate(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args)
{
    int err;
    bool is_complete = false;   // Unused by this function as err comtains the same information for sync commands

    #define IS_SYNC true
    #define IS_ASYNC false
    err = SendOperateAndProcessResponse(req->group_id, req->path, IS_SYNC, command_key, input_args, output_args, &is_complete);

    return err;
}

/*********************************************************************//**
**
** Broker_AsyncOperate
**
** Async Operation vendor hook for USP commands owned by USP Services
**
** \param   req - pointer to structure identifying the operation in the data model
** \param   input_args - vector containing input arguments and their values
** \param   instance - instance number of this operation in the Device.LocalAgent.Request table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_AsyncOperate(dm_req_t *req, kv_vector_t *input_args, int instance)
{
    int err;
    char path[MAX_DM_PATH];
    char command_key[MAX_DM_VALUE_LEN];
    kv_vector_t *output_args;
    subs_map_t *smap;
    req_map_t *rmap;
    usp_service_t *us;
    bool is_complete = false;

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(req->group_id);
    USP_ASSERT(us != NULL);

    // Exit if no subscription was setup on the USP service for an OperateComplete. We disallow async commands from being started
    // unless there is a subscription setup because otherwise the Broker will not know when the USP Command has completed and hence
    // will never delete the request from the Broker's Request table
    smap = SubsMap_FindByPath(&us->subs_map, req->path);
    if (smap == NULL)
    {
        USP_ERR_SetMessage("%s: OperationComplete subscription must be set before invoking '%s'", __FUNCTION__, req->path);
        return USP_ERR_REQUEST_DENIED;
    }

    // Exit if unable to get the value of the command key
    USP_SNPRINTF(path, sizeof(path), "Device.LocalAgent.Request.%d.CommandKey", instance);
    err = DATA_MODEL_GetParameterValue(path, command_key, sizeof(command_key), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if the combination of path and command_key are not unique.
    // If this is not the case, then a controller will be unable to distinguish OperationComplete notifications for each request
    rmap = ReqMap_Find(&us->req_map, req->path, command_key);
    if (rmap != NULL)
    {
        USP_ERR_SetMessage("%s: Command_key='%s' is not unique for path '%s'", __FUNCTION__, command_key, req->path);
        return USP_ERR_REQUEST_DENIED;
    }

    // Add the request to the request mapping table
    // This is done before sending the OperateRequest because an (incorrect) USP Service might send the OperationComplete notification before the OperateResponse message
    rmap = ReqMap_Add(&us->req_map, instance, req->path, command_key);

    // Exit if an error occurred whilst trying to send the Operate Request and receive the Operate Response
    output_args = USP_ARG_Create();
    err = SendOperateAndProcessResponse(req->group_id, req->path, IS_ASYNC, command_key, input_args, output_args, &is_complete);
    if (err != USP_ERR_OK)
    {
        USP_ARG_Delete(output_args);
        ReqMap_Remove(&us->req_map, rmap);
        return err;
    }

    // Since Operate Response has been successful, change the Status in the Request table to active
    USP_SIGNAL_OperationStatus(instance, "Active");

    // Deal with the case of the operate response unexpectedly (for an async operation) indicating that it has completed
    if (is_complete)
    {
        USP_SIGNAL_OperationComplete(instance, USP_ERR_OK, NULL, output_args);  // ownership of output_args passes to USP_SIGNAL_OperationComplete()
        ReqMap_Remove(&us->req_map, rmap);
    }
    else
    {
        USP_ARG_Delete(output_args);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Broker_RefreshInstances
**
** RefreshInstances vendor hook called for top level objects owned by the USP service
** This function sends a USP GetInstances request in order to obtain the instance numbers from the USP service
** Then it waits for a USP GetInstances Response and parses it, caching the instance numbers in the data model
**
** \param   group_id - group ID of the USP service
** \param   path - schema path to the top-level multi-instance node to refresh the instances of (partial path - does not include trailing '{i}')
** \param   expiry_period - Pointer to variable in which to return the number of seconds to cache the refreshed instances result
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_RefreshInstances(int group_id, char *path, int *expiry_period)
{
    usp_service_t *us;
    str_vector_t sv;
    int err;

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Create a string vector on the stack with the single path that we want to query
    sv.num_entries = 1;
    sv.vector = &path;

    // Send the request and parse the response, adding the retrieved instance numbers into the instances cache
    err = UspService_RefreshInstances(us, &sv, true);

    // Update the expiry time, if successful
    if (err == USP_ERR_OK)
    {
        // Setting an expiry time of -1 seconds, means that the instances for a USP Service in the instance cache
        // will only be valid for the current USP Message being processed. This is necessary because passthru USP messages
        // do not update the instance cache, so if the expiry time isn't -1, we would see a lot of confusing behaviour
        // due to instance cache mismatch
        #define BROKER_INSTANCE_CACHE_EXPIRY_PERIOD -1       // in seconds
        *expiry_period = BROKER_INSTANCE_CACHE_EXPIRY_PERIOD;
    }

    return err;
}

/*********************************************************************//**
**
** Broker_GroupSubscribe
**
** Subscribe vendor hook for parameters owned by the USP service
** This function performs a USP Add request on the USP Service's subscription table
** Then it waits for a USP Add Response and parses it, to return whether the subscription was successfully registered
**
** \param   broker_instance - Instance number of the subscription in the Broker's Device.LocalAgent.Subscription.{i}
** \param   group_id - group ID of the USP service
** \param   notify_type - type of subscription to register
** \param   path - path of the data model element to subscribe to
** \param   persistent - specifies whether the subscription should be persisted on the USP service
**                       NOTE: In general, it does not matter if the subscription is not persisted on the USP service, as the Broker
**                             will add at during the registration sequence. However for some subscriptions eg Device.Boot!, it may
**                             be necessary for them to be persisted on the USP Service, in order that the subscription exists at the time the event is generated (ie at startup)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_GroupSubscribe(int broker_instance, int group_id, subs_notify_t notify_type, char *path, bool persistent)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    usp_service_t *us;
    int service_instance;  // Instance of the subscription in the USP Service's Device.LocalAgent.Subscription.{i}
    char subscription_id[MAX_DM_SHORT_VALUE_LEN];
    char *persistent_str = (persistent) ? "true" : "false";
    static unsigned id_count = 1;
    char *obj_path = "Device.LocalAgent.Subscription.";
    group_add_param_t params[] = {
                           // Name,  value,  is_required, err_code, err_msg
                           {"NotifType", TEXT_UTILS_EnumToString(notify_type, notify_types, NUM_ELEM(notify_types)), true, USP_ERR_OK, NULL },
                           {"ReferenceList", path, true, USP_ERR_OK, NULL },
                           {"ID", subscription_id, true, USP_ERR_OK, NULL },
                           {"Persistent", persistent_str, true, USP_ERR_OK, NULL },
                           {"TimeToLive", "0", true, USP_ERR_OK, NULL },
                           {"NotifRetry", "false", true, USP_ERR_OK, NULL },
                           {"NotifExpiration", "0", true, USP_ERR_OK, NULL },
                           {"Enable", "true", true, USP_ERR_OK, NULL }
                         };
    char msg_id[MAX_MSG_ID_LEN];

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form the value of the subscription ID
    USP_SNPRINTF(subscription_id, sizeof(subscription_id), "%d-%d-%x-%s", broker_instance, id_count, (unsigned) time(NULL), broker_unique_str);
    id_count++;

    // Form the USP Add Request message, ensuring that the path contains a trailing dot
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_AddReq(msg_id, obj_path, params, NUM_ELEM(params));

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__ADD_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the add response, saving it's details in the subscription mapping table, if successful
    err = ProcessAddResponse(resp, obj_path, &service_instance, NULL, NULL, 0);
    if (err == USP_ERR_OK)
    {
        SubsMap_Add(&us->subs_map, service_instance, path, subscription_id, broker_instance);
    }

    // Free the add response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** Broker_GroupUnsubscribe
**
** Unsubscribe vendor hook for parameters owned by the USP service
** This function performs a USP Delete request on the USP Service's subscription table
** Then it waits for a USP Delete Response and parses it, to return whether the subscription was successfully deregistered
**
** \param   broker_instance - Instance number of the subscription in the Broker's Device.LocalAgent.Subscription.{i}
** \param   group_id - group ID of the USP service
** \param   notify_type - type of subscription to deregister (UNUSED)
** \param   path - path of the data model element to unsubscribe from
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Broker_GroupUnsubscribe(int broker_instance, int group_id, subs_notify_t notify_type, char *path)
{
    int err;
    usp_service_t *us;
    subs_map_t *smap;
    char obj_path[MAX_DM_PATH];
    str_vector_t paths;
    char *single_path;

    // Kepp compiler happy with unused argument
    (void)notify_type;

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if this path was never subscribed to
    smap = SubsMap_FindByBrokerInstanceAndPath(&us->subs_map, broker_instance, path);
    if (smap == NULL)
    {
        USP_ERR_SetMessage("%s: Not subscribed to path %s", __FUNCTION__, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form a statically allocated string vector containing a single instance
    USP_SNPRINTF(obj_path, sizeof(obj_path), "Device.LocalAgent.Subscription.%d.", smap->service_instance);
    paths.num_entries = 1;
    paths.vector = &single_path;
    single_path = obj_path;

    // Send the Delete request and process the Delete response
    err = UspService_DeleteInstances(us, false, &paths, NULL);

    // Remove from the subscription mapping table
    SubsMap_Remove(&us->subs_map, smap);

    return err;
}

/*********************************************************************//**
**
** SyncSubscriptions
**
** Ensures that the USP Service contains only the subscriptions which it is supposed to
** and that the state in the Broker is aware of the mapping between the subscriptions in the USP Service and the Broker
**
** \param   us - pointer to USP service in usp_services[]
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SyncSubscriptions(usp_service_t *us)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    kv_vector_t kvv;        // NOTE: None of the data in this structure will be dynamically allocated, so it does not have to be freed
    kv_pair_t kv;
    char msg_id[MAX_MSG_ID_LEN];

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form a USP Get Request message to get all of the USP Service's subscription table
    kv.key = subs_partial_path;
    kv.value = NULL;
    kvv.vector = &kv;
    kvv.num_entries = 1;

    // Form the USP Get Request message
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_GetReq(msg_id, &kvv);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__GET_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the get response, pairing up subscription instances from USP Service to Broker, and deleting stale subscriptions on the USP Service
    err = ProcessGetSubsResponse(us, resp);

    // Free the get response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    DEVICE_SUBSCRIPTION_StartAllVendorLayerSubsForGroup(us->group_id);

    return err;
}

/*********************************************************************//**
**
** ProcessGetSubsResponse
**
** Processes a Get Response containing the subscriptions which the USP Service has when it registers with the Broker
** The subscriptions from the USP service are paired with any existing subscriptions in the Broker
** and stale subscriptions in the USP Service are deleted
**
** \param   us - pointer to USP service in usp_services[]
** \param   resp - USP response message in protobuf-c structure
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessGetSubsResponse(usp_service_t *us, Usp__Msg *resp)
{
    int i;
    int err;
    Usp__GetResp *get;
    Usp__GetResp__RequestedPathResult *rpr;
    str_vector_t subs_to_delete;

    // Exit if failed to validate that the Message body contains a Get Response (eg if the Message Body is an Error response)
    // NOTE: It is possible for the USP Service to send back an Error response instead of a GetResponse, but only if the GetRequest was not understood
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_GET_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if get response is missing
    get = resp->body->response->get_resp;
    if (get == NULL)
    {
        USP_LOG_Error("%s: Missing get response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if there is more than one requested path result (since we requested only one partial path)
    if (get->n_req_path_results != 1)
    {
        USP_LOG_Error("%s: Expected only 1 requested path result, but got %d", __FUNCTION__, (int)get->n_req_path_results);
        return USP_ERR_INTERNAL_ERROR;
    }
    rpr = get->req_path_results[0];

    // Exit if requested path does not match the one we requested
    if (strcmp(rpr->requested_path, subs_partial_path) != 0)
    {
        USP_LOG_Error("%s: Requested path was '%s' but expected %s", __FUNCTION__, rpr->requested_path, subs_partial_path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if we received an error for this requested path
    if (rpr->err_code != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Received err=%d (%s) when getting the subscription table", __FUNCTION__, rpr->err_code, rpr->err_msg);
        return rpr->err_code;
    }

    // Iterate over all resolved_path_results (each one represents an instance in the USP Service's subscription table)
    // Pair up these instances with the matching instance in the Broker and determine if any need deleting
    STR_VECTOR_Init(&subs_to_delete);
    for (i=0; i < rpr->n_resolved_path_results; i++)
    {
        ProcessGetSubsResponse_ResolvedPathResult(us, rpr->resolved_path_results[i], &subs_to_delete);
    }

    // Delete all USP Service subscription table instances which are stale
    if (subs_to_delete.num_entries > 0)
    {
        UspService_DeleteInstances(us, false, &subs_to_delete, NULL);  // NOTE: Intentionally ignoring any error, since we can't sensibly do anything other than ignore it
    }

    STR_VECTOR_Destroy(&subs_to_delete);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ProcessGetSubsResponse_ResolvedPathResult
**
** Processes a subscription instance read from the USP Service's subscription table
** If it matches a subscription instance in the Broker's subscription table, then pair them up in the subs mapping table
** otherwise mark for deletion stale subscriptions that were created by the Broker in the USP Service's subscription table
** NOTE: This function needs to cope with the fact that the USP service may issue multiple register messages, so consequently:
**       (a) Some of the subscription instances may already be paired up in the subs mapping table (due to a previous register)
**       (b) It may not be possible to pair up some of the instances (because they would be covered by a later register)
**           In this case, we shoudn't delete these
**
** \param   us - pointer to USP service in usp_services[]
** \param   res - pointer to resolved_path_result structure containing a number of parameters and their associated values
** \param   subs_to_delete - pointer to vector which is updated by this function with any USP Service subscription instances to delete
**
** \return  None
**
**************************************************************************/
void ProcessGetSubsResponse_ResolvedPathResult(usp_service_t *us, Usp__GetResp__ResolvedPathResult *res, str_vector_t *subs_to_delete)
{
    dm_node_t *node;
    dm_instances_t inst;
    int service_instance;
    char *path;
    char *notify_type_str;
    char *subscription_id;
    char *enable_str;
    subs_notify_t notify_type;
    subs_map_t *smap;
    int broker_instance;
    bool is_present;
    int subs_group_id;
    bool enable;
    int err;
    int items_converted;
    bool was_marked;

    // Exit if unable to extract the instance number of this subscription in the USP Service's subscription table
    node = DM_PRIV_GetNodeFromPath(res->resolved_path, &inst, NULL, 0);
    if (node == NULL)
    {
        USP_LOG_Error("%s: Resolved path was '%s' but expected %s.XXX.", __FUNCTION__, res->resolved_path, subs_partial_path);
        return;
    }
    service_instance = inst.instances[0];

    // Exit if unable to extract the parameters for this instance of the subscription table
    // NOTE: Ownership of strings stay with the USP message data structure
    path = GetParamValueFromResolvedPathResult(res, "ReferenceList");
    notify_type_str = GetParamValueFromResolvedPathResult(res, "NotifType");
    subscription_id = GetParamValueFromResolvedPathResult(res, "ID");
    enable_str = GetParamValueFromResolvedPathResult(res, "Enable");
    if ((path == NULL) || (notify_type_str==NULL) || (subscription_id == NULL) || (enable_str == NULL))
    {
        USP_LOG_Error("%s: Unable to extract parameters for USP Service's subs table instance %d", __FUNCTION__, service_instance);
        return;
    }

    // Exit if the USP Service reported back an unknown subscription type
    notify_type = TEXT_UTILS_StringToEnum(notify_type_str, notify_types, NUM_ELEM(notify_types));
    if (notify_type == INVALID)
    {
        USP_LOG_Error("%s: USP Service returned unknown notify type (%s)", __FUNCTION__, notify_type_str);
        return;
    }

    // Exit if the USP Service's Subscription ID was not created by the Broker
    if (strstr(subscription_id, broker_unique_str) == NULL)
    {
        return;
    }

    // Exit if the subscription was not enabled. Since all subscriptions that the Broker creates on the USP Service are enabled,
    // this is an error condition. Cope with it by deleting the subscription. The subscription will be recreated (with Enable set)
    // if it is present on the Broker when DEVICE_SUBSCRIPTION_StartAllVendorLayerSubsForGroup() is called
    err = TEXT_UTILS_StringToBool(enable_str, &enable);
    if ((err != USP_ERR_OK) || (enable != true))
    {
        STR_VECTOR_Add(subs_to_delete, res->resolved_path);
        return;
    }

    // Determine whether this path can be satisfied by the vendor layer
    subs_group_id = USP_BROKER_IsPathVendorSubscribable(notify_type, path, &is_present);

    // Exit if the path does not exist currently in the Broker's data model
    // This could happen if the USP Service issues multiple Register requests, and this subscription will only be paired up after a later Register request
    // But we delete the subscription anyway, because we don't know whether the USP Service will eventually register the path
    // (It will be re-created if the USP Service does eventually register the path)
    if (is_present==false)
    {
        STR_VECTOR_Add(subs_to_delete, res->resolved_path);
        return;
    }

    // Exit if the path is not owned by this USP Service, and so should not be set on this USP Service
    // (except subscriptions to 'Device.', which are set on all USP Services)
    // We delete the subscription in this case, because the path exists in the data model, but is not owned by this USP Service
    if ((subs_group_id != us->group_id) && (strcmp(path, dm_root) != 0))
    {
        STR_VECTOR_Add(subs_to_delete, res->resolved_path);
        return;
    }

    // Exit if unable to extract the broker's subscription instance number from the subscription ID
    // We delete the subscription in this case as the subscription ID is malformed
    items_converted = sscanf(subscription_id, "%d", &broker_instance);
    if (items_converted != 1)
    {
        STR_VECTOR_Add(subs_to_delete, res->resolved_path);
        return;
    }

    // Exit if this subscription is already in the subs mapping table
    // This could happen if the USP Service issues multiple Register requests, and this subscription was paired up by a previous register sequence
    smap = SubsMap_FindByUspServiceSubsId(&us->subs_map, subscription_id, broker_instance);
    if (smap != NULL)
    {
        return;
    }

    // Mark the Broker's subscription matching this as owned by the USP Service
    was_marked = DEVICE_SUBSCRIPTION_MarkVendorLayerSubs(broker_instance, notify_type, path, us->group_id);
    if (was_marked == false)
    {
        // The USP Service's subscription does not match any enabled subscriptions owned by the Broker
        // In which case, this is a stale subscription i.e. the subscription has already been deleted (or disabled) on the Broker,
        // so needs to be deleted on the USP Service (to synchronize them)
        STR_VECTOR_Add(subs_to_delete, res->resolved_path);
        return;
    }

    // If the code gets here, then the subscription should be added to the subscription mapping table
    // (It will already have been marked as owned by the Vendor layer in DEVICE_SUBSCRIPTION_MarkVendorLayerSubs)
    SubsMap_Add(&us->subs_map, service_instance, path, subscription_id, broker_instance);
}

/*********************************************************************//**
**
** DeleteMatchingOperateRequest
**
** Deletes the instance in the Broker's request table that matches the specified path and command_key
** of the USP Command that has completed
**
** \param   us - pointer to USP service on which the notification was received
** \param   obj_path - path to the parent object of the USP command that has completed
** \param   command_name - name of the USP command that has completed
** \param   command_key - command_key of the request for the USP command that has completed
**
** \return  None
**
**************************************************************************/
void DeleteMatchingOperateRequest(usp_service_t *us, char *obj_path, char *command_name, char *command_key)
{
    char command_path[MAX_DM_PATH];
    req_map_t *rmap;

    // Form the full path to the USP Command
    USP_SNPRINTF(command_path, sizeof(command_path), "%s%s", obj_path, command_name);

    // Exit if unable to find a match for this USP command
    // This could occur if the USP Service (incorrectly) emitted multiple OperateComplete notifications per single Operate request
    rmap = ReqMap_Find(&us->req_map, command_path, command_key);
    if (rmap == NULL)
    {
        USP_LOG_Error("%s: Received an Operation Complete for %s (command_key=%s), but no entry in request map", __FUNCTION__, command_path, command_key);
        return;
    }

    // Delete the request from the Broker's request table
    DEVICE_REQUEST_DeleteInstance(rmap->request_instance);

    // Remove the request from the request mapping table
    ReqMap_Remove(&us->req_map, rmap);
}

/*********************************************************************//**
**
** UspService_DeleteInstances
**
** Sends a Delete Request and Processes the Delete response from a USP Service
** NOTE: This function always uses allow_partial=false and ProcessDelteResponse() assumes that this is the case
**
** \param   us - pointer to USP Service to delete the instances on
** \param   allow_partial - if set to false, if any of the objects fails to delete, then none should be deleted
** \param   paths - pointer to vector containing the list of data model objects to delete
**                  NOTE: All object paths must be absolute (no wildcards etc)
** \param   failure_index - pointer to variable in which to return the first index of the entry in paths that failed to delete,
**                          or NULL if the caller doesn't care about this
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UspService_DeleteInstances(usp_service_t *us, bool allow_partial, str_vector_t *paths, int *failure_index)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    char msg_id[MAX_MSG_ID_LEN];

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form the USP Delete Request message
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_DeleteReq(msg_id, paths, allow_partial);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__DELETE_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the delete response, determining if it was successful or not
    err = ProcessDeleteResponse(resp, paths, failure_index);

    // Free the delete response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** UspService_RefreshInstances
**
** Called to refresh the instances of a set of top level objects
** This function sends a USP GetInstances request in order to obtain the instance numbers from the USP service
** Then it waits for a USP GetInstances Response and parses it, caching the instance numbers in the data model
**
** \param   us - pointer to USP service to query
** \param   paths - paths to the top-level multi-instance nodes to refresh the instances of
** \param   within_vendor_hook - Determines whether this function is being called within the context of the
**                               refresh instances vendor hook (This has some restrictions on which object instances may be refreshed)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int UspService_RefreshInstances(usp_service_t *us, str_vector_t *paths, bool within_vendor_hook)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    char msg_id[MAX_MSG_ID_LEN];

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Create the GetInstances request
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_GetInstancesReq(msg_id, paths);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    #define RESPONSE_TIMEOUT  30
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__GET_INSTANCES_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the GetInstances response, retrieving the instance numbers and caching them in the data model
    err = ProcessGetInstancesResponse(resp, us, within_vendor_hook);

    // Free the GetInstances response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** AddTopLevelTableNodes
**
** Function called recursively to add the object paths of all top level multi-instance object nodes to a string vector
**
** \param   parent - pointer to data model node to see whether to add
** \param   sv - pointer to string vector in which to add the schema paths
**
** \return  None
**
**************************************************************************/
void AddTopLevelTableNodes(dm_node_t *parent, str_vector_t *sv)
{
    dm_node_t *child;
    char *path;
    char *p;

    // Exit if we've found a top level multi-instance object. We do not have to recurse any further for nested multi-instance objects
    if (parent->type == kDMNodeType_Object_MultiInstance)
    {
        STR_VECTOR_Add(sv, parent->path);

        // Convert the schema path back to an object path by in-place truncating the string at the terminating '{i}'
        path = sv->vector[ sv->num_entries-1 ];
        p = strchr(path, '{');
        USP_ASSERT(p != NULL);
        *p = '\0';
        return;
    }

    // Iterate over list of children
    child = (dm_node_t *) parent->child_nodes.head;
    while (child != NULL)
    {
        AddTopLevelTableNodes(child, sv);

        // Move to next sibling in the data model tree
        child = (dm_node_t *) child->link.next;
    }
}

/*********************************************************************//**
**
** GetParamValueFromResolvedPathResult
**
** Finds the specified parameter in the resolved_path_result of a GetResponse and returns it's value
**
** \param   res - pointer to resolved_path_result structure containing a number of parameters and their associated values
** \param   name - name of the parameter to find
**
** \return  pointer to value of the parameter (in the resolved_path_result structure) or NULL if the parameter was not found
**
**************************************************************************/
char *GetParamValueFromResolvedPathResult(Usp__GetResp__ResolvedPathResult *res, char *name)
{
    int i;
    Usp__GetResp__ResolvedPathResult__ResultParamsEntry *rpe;

    // Iterate over all parameters in the resolved_path_result structure
    for (i=0; i < res->n_result_params; i++)
    {
        rpe = res->result_params[i];
        if (strcmp(rpe->key, name)==0)
        {
            return rpe->value;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** ProcessGetResponse
**
** Processes a Get Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   params - key-value vector in which to return the paameter values
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessGetResponse(Usp__Msg *resp, kv_vector_t *kvv)
{
    int i;
    int err;
    Usp__GetResp *get;
    Usp__GetResp__RequestedPathResult *rpr;
    Usp__GetResp__ResolvedPathResult *res;
    Usp__GetResp__ResolvedPathResult__ResultParamsEntry *rpe;

    // Exit if failed to validate that the Message body contains a Get Response (eg if the Message Body is an Error response)
    // NOTE: It is possible for the USP Service to send back an Error response instead of a GetResponse, but only if the GetRequest was not understood
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
    // NOTE: Each path that we requested was a single parameter (no wildcards or partial paths), so we expect to get a single value of a single object for each result
    USP_ASSERT((get->n_req_path_results==0) || (get->req_path_results != NULL));
    for (i=0; i < get->n_req_path_results; i++)
    {
        rpr = get->req_path_results[i];
        USP_ASSERT(rpr != NULL)

        // Skip if we received an error for this parameter
        if (rpr->err_code != USP_ERR_OK)
        {
            if (rpr->err_msg != NULL)
            {
                USP_ERR_ReplaceEmptyMessage("%s", rpr->err_msg);
            }
            else
            {
                USP_ERR_ReplaceEmptyMessage("Failed to get %s", rpr->requested_path);
            }
            continue;
        }

        // Skip if we did not receive a resolved path result
        if ((rpr->n_resolved_path_results < 1) || (rpr->resolved_path_results == NULL) || (rpr->resolved_path_results[0] == NULL))
        {
            USP_ERR_ReplaceEmptyMessage("%s: Did not receive resolved path result for '%s'", __FUNCTION__, rpr->requested_path);
            continue;
        }

        // Skip if we did not receive a result params entry
        res  = rpr->resolved_path_results[0];
        if ((res->n_result_params < 1) || (res->result_params == NULL) || (res->result_params[0] == NULL))
        {
            USP_ERR_ReplaceEmptyMessage("%s: Did not receive result params entry for '%s'", __FUNCTION__, rpr->requested_path);
            continue;
        }

        // Skip if we did not receive a value for the parameter
        rpe = res->result_params[0];
        if (rpe->value == NULL)
        {
            USP_ERR_ReplaceEmptyMessage("%s: Did not receive value for '%s'", __FUNCTION__, rpr->requested_path);
            continue;
        }

        // Fill in the parameter value in the returned key-value vector
        // NOTE: If we received a value for a parameter which we didn't request, then just ignore it. The group get caller will detect any missing parameter values
        KV_VECTOR_ReplaceWithHint(kvv, rpr->requested_path, rpe->value, i);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ProcessSetResponse
**
** Processes a Set Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   params - key-value vector containing the parameter names as keys and the parameter values as values
** \param   failure_index - pointer to value in which to return the index of the first parameter in the params vector
**                          that failed to be set. This value is only consulted if an error is returned.
**                          Setting it to INVALID indicates that all parameters failed (e.g. communications failure)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessSetResponse(Usp__Msg *resp, kv_vector_t *params, int *failure_index)
{
    int i;
    int err;
    Usp__SetResp *set;
    Usp__SetResp__UpdatedObjectResult *obj_result;
    Usp__SetResp__UpdatedObjectResult__OperationStatus *oper_status;
    bool is_success = false;

    // Default to indicating that all parameters failed
    *failure_index = INVALID;

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_SET_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        *failure_index = CalcFailureIndex(resp, params, &err);
        return err;
    }

    // Exit if set response object is missing
    set = resp->body->response->set_resp;
    if (set == NULL)
    {
        USP_ERR_SetMessage("%s: Missing set response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all UpdatedObjResults, checking that all were successful
    // NOTE: We expect all of them to be successful if the code gets here since the Set Request has allow_partial=false and
    // all parameters are required to set, so we should have received an ERROR response if any failed to set
    for (i=0; i < set->n_updated_obj_results; i++)
    {
        obj_result = set->updated_obj_results[i];
        oper_status = obj_result->oper_status;
        switch(oper_status->oper_status_case)
        {
            case USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
                // Check that the parameters and values reported as being set, match the ones we requested
                // NOTE: This code does not verify that we got a success response for EVERY param that we requested, only that the ones indicated in the response were ones we requested
                is_success = CheckSetResponse_OperSuccess(oper_status->oper_success, params);
                break;

            case USP__SET_RESP__UPDATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
                // Log all failures. NOTE: We should have received an Error response instead, if the USP Service was implemented correctly
                is_success = false;
                LogSetResponse_OperFailure(oper_status->oper_failure);
                break;

            default:
                TERMINATE_BAD_CASE(oper_status->oper_status_case);
                break;
        }
    }

    err = (is_success) ? USP_ERR_OK : USP_ERR_INTERNAL_ERROR;

    return err;
}

/*********************************************************************//**
**
** CheckSetResponse_OperSuccess
**
** Checks the OperSucces object in the SetResponse, ensuring that the parameters were set to the expected values
**
** \param   oper_success - OperSuccess object to check
** \param   params - key-value vector containing the parameters (and values) that we expected to be set
**
** \return  true if no errors were detected in the set response
**
**************************************************************************/
bool CheckSetResponse_OperSuccess(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationSuccess *oper_success, kv_vector_t *params)
{
    int i, j;
    Usp__SetResp__UpdatedInstanceResult *updated_inst_result;
    Usp__SetResp__UpdatedInstanceResult__UpdatedParamsEntry *updated_param;
    Usp__SetResp__ParameterError *param_err;
    char path[MAX_DM_PATH];
    bool is_success = true;
    int index;
    char *expected_value;

    for (i=0; i < oper_success->n_updated_inst_results; i++)
    {
        updated_inst_result = oper_success->updated_inst_results[i];

        // Log all errors (for non-required parameters)
        // NOTE: We should not get any of these, as we marked all params as required in the request
        for (j=0; j < updated_inst_result->n_param_errs; j++)
        {
            param_err = updated_inst_result->param_errs[j];
            USP_ERR_SetMessage("%s: SetResponse returned err=%d for param=%s%s but should have returned ERROR Response", __FUNCTION__, param_err->err_code, updated_inst_result->affected_path, param_err->param);
            is_success = false;
        }

        // Check that the USP Service hasn't set the wrong value for any params
        for (j=0; j < updated_inst_result->n_updated_params; j++)
        {
            updated_param = updated_inst_result->updated_params[j];
            USP_SNPRINTF(path, sizeof(path), "%s%s", updated_inst_result->affected_path, updated_param->key);

            // Skip if this param was not one we requested to be set
            index = KV_VECTOR_FindKey(params, path, 0);
            if (index == INVALID)
            {
                USP_ERR_SetMessage("%s: SetResponse contained a success entry for param=%s but we never requested it to be set", __FUNCTION__, path);
                is_success = false;
                continue;
            }

            // Check that the parameter was set to the expected value
            expected_value = params->vector[index].value;
            if (strcmp(expected_value, updated_param->value) != 0)
            {
                USP_ERR_SetMessage("%s: SetResponse contained the wrong value for param=%s (expected='%s', got='%s')", __FUNCTION__, path, expected_value, updated_param->key);
                is_success = false;
            }
        }
    }

    return is_success;
}


/*********************************************************************//**
**
** LogSetResponse_OperFailure
**
** Logs all errors indicated by an OperFailure object in the SetResponse
** NOTE: We do not expect any OperFailures, because allow_partial=false and all parameters are required to set
**       If any parameter failed to set, an ERROR response would have been received instead
**
** \param   oper_failure - OperFailure object to og all errors from
**
** \return  None
**
**************************************************************************/
void LogSetResponse_OperFailure(Usp__SetResp__UpdatedObjectResult__OperationStatus__OperationFailure *oper_failure)
{
    int i, j;
    Usp__SetResp__UpdatedInstanceFailure *updated_inst_failure;
    Usp__SetResp__ParameterError *param_err;

    for (i=0; i < oper_failure->n_updated_inst_failures; i++)
    {
        updated_inst_failure = oper_failure->updated_inst_failures[i];
        for (j=0; j < updated_inst_failure->n_param_errs; j++)
        {
            param_err = updated_inst_failure->param_errs[j];
            USP_LOG_Error("%s: SetResponse returned err=%d for param=%s%s but should have returned ERROR Response", __FUNCTION__, param_err->err_code, updated_inst_failure->affected_path, param_err->param);
        }
    }
}

/*********************************************************************//**
**
** CalcFailureIndex
**
** Calculates the index of the first parameter found that failed to set
**
** \param   resp - USP response message in protobuf-c structure
** \param   params - key-value vector containing the parameter names as keys and the parameter values as values
** \param   modified_err - pointer to error code to modify with the more specific error given for the first parameter which failed (if available)
**
** \return  index of the first parameter in the params vector that failed to be set
**          INVALID indicates that all parameters failed or that we do not know which parameter failed first
**
**************************************************************************/
int CalcFailureIndex(Usp__Msg *resp, kv_vector_t *params, int *modified_err)
{
    int i;
    Usp__Error *err_obj;
    int index;
    int lowest_index;
    char *path;
    int first_err;       // This is the error code given for the first parameter that failed, which may be different than the holistic error for all of the params that failed

    // Exit if cause of error was something other than an error response
    if ((resp->body == NULL) || (resp->body->msg_body_case != USP__BODY__MSG_BODY_ERROR) || (resp->body->error == NULL))
    {
        return INVALID;
    }

    // Exit if the Error response does not contain details of which parameter(s) were in error
    err_obj = resp->body->error;
    if ((err_obj->n_param_errs == 0) || (err_obj->param_errs == NULL))
    {
        return INVALID;
    }

    // Iterate over all parameters in error, finding the first one in the list of parameters to set, that failed
    lowest_index = INT_MAX;
    first_err = *modified_err;
    for (i=0; i< err_obj->n_param_errs; i++)
    {
        path = err_obj->param_errs[i]->param_path;
        index = KV_VECTOR_FindKey(params, path, 0);
        if (index < lowest_index)
        {
            lowest_index = index;
            first_err = err_obj->param_errs[i]->err_code;
        }
    }

    // Exit if none of the parameters in the error matched those that we were trying to set
    // NOTE: This should never happen, because we expect that at least one of the params in error was one that we were trying to set
    if (lowest_index == INT_MAX)
    {
        return INVALID;
    }

    // Return the index of the first parameter which failed, and it's associated, more specific, error code
    *modified_err = first_err;
    return lowest_index;
}

/*********************************************************************//**
**
** ProcessAddResponse
**
** Processes an Add Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   path - path of the object in the data model that we requested an instance to be added to
** \param   instance - pointer to variable in which to return instance number of object that was added
** \param   unique_keys - pointer to key-value vector in which to return the name and values of the unique keys for the object, or NULL if this info is not required
** \param   params - pointer to array containing the child parameters and their input and output arguments or NULL if not used
**                   This function fills in the err_code and err_msg output arguments if a parameter failed to set
** \param   num_params - number of child parameters that were attempted to be set
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessAddResponse(Usp__Msg *resp, char *path, int *instance, kv_vector_t *unique_keys, group_add_param_t *params, int num_params)
{
    int i;
    int err;
    Usp__AddResp *add;
    Usp__AddResp__CreatedObjectResult *created_obj_result;
    Usp__AddResp__CreatedObjectResult__OperationStatus *oper_status;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationFailure *oper_failure;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess *oper_success;
    Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry *uk;
    Usp__AddResp__ParameterError *pe;
    char *param_errs_path;

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_ADD_RESP, &param_errs_path);
    if (err != USP_ERR_OK)
    {
        PropagateParamErr(param_errs_path, err, USP_ERR_GetMessage(), params, num_params);
        return err;
    }

    // Exit if add response is missing
    add = resp->body->response->add_resp;
    if (add == NULL)
    {
        USP_ERR_SetMessage("%s: Missing add response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if there isn't exactly 1 created_obj_result (since we only requested one object to be created)
    if (add->n_created_obj_results != 1)
    {
        USP_ERR_SetMessage("%s: Unexpected number of objects created (%d)", __FUNCTION__, (int)add->n_created_obj_results);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if this response seems to be for a different requested path
    created_obj_result = add->created_obj_results[0];
    if (strcmp(created_obj_result->requested_path, path) != 0)
    {
        USP_ERR_SetMessage("%s: Unexpected requested path in AddResponse (got=%s, expected=%s)", __FUNCTION__, created_obj_result->requested_path, path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Determine whether the object was created successfully or failed
    oper_status = created_obj_result->oper_status;
    switch(oper_status->oper_status_case)
    {
        case USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
            oper_failure = oper_status->oper_failure;
            USP_ERR_SetMessage("%s", oper_failure->err_msg);
            err = oper_failure->err_code;
            if (err == USP_ERR_OK)      // Since this result is indicated as a failure, return a failure code to the caller
            {
                err = USP_ERR_INTERNAL_ERROR;
            }
            break;

        case USP__ADD_RESP__CREATED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
            oper_success = oper_status->oper_success;
            // Determine the instance number of the object that was added (validating that it is for the requested path)
            err = ValidateAddResponsePath(path, oper_success->instantiated_path, instance);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            if (oper_success->n_unique_keys > 0)
            {
                // Register the unique keys for this object, if they haven't been already
                USP_ASSERT(&((Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry *)0)->key == &((Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry *)0)->key);  // Checks that Usp__AddResp__CreatedObjectResult__OperationStatus__OperationSuccess__UniqueKeysEntry is same structure as Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry
                ProcessUniqueKeys(oper_success->instantiated_path, (Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry **)oper_success->unique_keys, oper_success->n_unique_keys);

                // Copy the unique keys into the key-value vector to be returned
                if (unique_keys != NULL)
                {
                    for (i=0; i < oper_success->n_unique_keys; i++)
                    {
                        uk = oper_success->unique_keys[i];
                        KV_VECTOR_Add(unique_keys, uk->key, uk->value);
                    }
                }
            }

            if (params != NULL)
            {
                // Copy across all param errs from the USP response back into the caller's params array
                for (i=0; i < oper_success->n_param_errs; i++)
                {
                    pe = oper_success->param_errs[i];
                    PropagateParamErr(pe->param, pe->err_code, pe->err_msg, params, num_params);
                }
            }

            break;

        default:
            TERMINATE_BAD_CASE(oper_status->oper_status_case);
    }

exit:
    return err;
}

/*********************************************************************//**
**
** PropagateParamErr
**
** Copies the specified parameter error into the matching parameter in the params array
** This function is called when creating an object when one or more of its child parameters fail to set
**
** \param   path - Path of the parameter which failed to set (usually this is a full/schema path, rather than just a parameter name - it just depends on the source of the USP message)
** \param   err_code - reason for the parameter not being set
** \param   err_msg - textual reason for the parameter not being set
** \param   params - array of parameters that were attempted to be set
** \param   num_params - number of parameters that were attempted to be set
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void PropagateParamErr(char *path, int err_code, char *err_msg, group_add_param_t *params, int num_params)
{
    int i;
    group_add_param_t *gap;

    // Iterate over all parameter names in the array, finding the first one which matches the tail end of the specified path
    for (i=0; i<num_params; i++)
    {
        gap = &params[i];
        if (TEXT_UTILS_StringTailCmp(path, gap->param_name)==0)
        {
            // Copy the error into the params array, in order that it can be returned to the original caller
            gap->err_code = err_code;
            gap->err_msg = USP_STRDUP(err_msg);
            return;
        }
    }
}

/*********************************************************************//**
**
** ValidateAddResponsePath
**
** Validates that the instantiated path in the Add Response is for the object we requested to be added
**
** \param   requested_path - Path of object that we requested to add an instance to
** \param   instantiated_path - Path of the object which was created by the set request
** \param   instance - pointer to variable in which to return instance number of object that was added
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAddResponsePath(char *requested_path, char *instantiated_path, int *instance)
{
    int err;
    char *expected_schema_path;
    char *received_schema_path;
    dm_req_instances_t expected_inst;
    dm_req_instances_t received_inst;

    // Determine the schema path of the object that we requested
    err = DATA_MODEL_SplitPath(requested_path, &expected_schema_path, &expected_inst, NULL);
    USP_ASSERT(err == USP_ERR_OK);

    // Exit if instantiated object was not in our data model
    err = DATA_MODEL_SplitPath(instantiated_path, &received_schema_path, &received_inst, NULL);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Unknown AddResponse instantiated path %s", __FUNCTION__, instantiated_path);
        return err;
    }

    // Exit if the instantiated object was not the object requested
    if (strcmp(received_schema_path, expected_schema_path) != 0)
    {
        USP_ERR_SetMessage("%s: AddResponse contains unexpected object (requested=%s, got=%s)", __FUNCTION__, requested_path, instantiated_path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the instantiated object does not have a trailing instance number
    if (received_inst.order == 0)
    {
        USP_ERR_SetMessage("%s: AddResponse contains object without instance number (%s)", __FUNCTION__, instantiated_path);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Return the instance number of the object that got created
    *instance = received_inst.instances[received_inst.order-1];
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ProcessDeleteResponse
**
** Processes a Delete Response that we have received from a USP Service
** NOTE: This function assumes that the Delete Request used allow_partial=false, when processing the Delete response
**
** \param   resp - USP response message in protobuf-c structure
** \param   paths - pointer to vector containing the list of data model objects that we requested to delete
** \param   failure_index - pointer to variable in which to return the first index of the entry in paths that failed to delete,
**                          or NULL if the caller doesn't care about this
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessDeleteResponse(Usp__Msg *resp, str_vector_t *paths, int *failure_index)
{
    int i;
    int err;
    int index;
    Usp__DeleteResp *del;
    Usp__DeleteResp__DeletedObjectResult *deleted_obj_result;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus *oper_status;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationFailure *oper_failure;
    Usp__DeleteResp__DeletedObjectResult__OperationStatus__OperationSuccess *oper_success;
    char *param_errs_path = NULL;

    // Set default return value
    if (failure_index != NULL)
    {
        *failure_index = INVALID;
    }

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_DELETE_RESP, &param_errs_path);
    if (err != USP_ERR_OK)
    {
        // Determine which path failed to delete (which might have been indicated in the ERROR Response)
        if (failure_index != NULL)
        {
            *failure_index = STR_VECTOR_Find(paths, param_errs_path);
        }

        return err;
    }

    // Exit if delete response is missing
    del = resp->body->response->delete_resp;
    if (del == NULL)
    {
        USP_ERR_SetMessage("%s: Missing delete response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the number of deleted_obj_results does not match the expected number
    if (del->n_deleted_obj_results != paths->num_entries)
    {
        USP_ERR_SetMessage("%s: Unexpected number of objects deleted (got=%d, expected=%d)", __FUNCTION__, (int)del->n_deleted_obj_results, paths->num_entries);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all instances that have been deleted, checking that they are the ones that were requested
    for (i=0; i < del->n_deleted_obj_results; i++)
    {
        // Exit if this response is for a different requested path
        deleted_obj_result = del->deleted_obj_results[0];
        index = STR_VECTOR_Find(paths, deleted_obj_result->requested_path);
        if (index == INVALID)
        {
            USP_ERR_SetMessage("%s: Unexpected requested path in DeleteResponse (%s)", __FUNCTION__, deleted_obj_result->requested_path);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Determine whether the object was deleted successfully or failed
        oper_status = deleted_obj_result->oper_status;
        switch(oper_status->oper_status_case)
        {
            case USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
                // NOTE: The USP Service should have sent an Error response instead of an OperFailure, because we sent the Delete request with allow_partial=false
                oper_failure = oper_status->oper_failure;
                USP_ERR_SetMessage("%s", oper_failure->err_msg);

                if (failure_index != NULL)
                {
                    *failure_index = i;
                }
                return oper_failure->err_code;
                break;

            case USP__DELETE_RESP__DELETED_OBJECT_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
                // We do not check that the instance exists in the affected_paths array, because if the instance was already deleted, then it won't be in this array
                // Log if we got any unaffected paths (since we tried to delete only one object per requested path, we are not expecting any)
                oper_success = oper_status->oper_success;
                if (oper_success->n_unaffected_path_errs >0)
                {
                    USP_LOG_Error("%s: DeleteResponse contained %d unaffected path errors, but shouldn't have", __FUNCTION__, (int)oper_success->n_unaffected_path_errs);
                }
                err = USP_ERR_OK;
                break;

            default:
                TERMINATE_BAD_CASE(oper_status->oper_status_case);
                break;
        }
    }

    return err;
}

/*********************************************************************//**
**
** SendOperateAndProcessResponse
**
** Common function to send an Operate Request to a USP Service and wait for the Operate Response, then parse it
**
** \param   group_id - Identifies which USP Service to send the Operate Request to (and receive the Operate Response from)
** \param   path - Data model path of the USP command to invoke
** \param   command_key - pointer to string containing the command key for this operation
** \param   input_args - vector containing input arguments and their values
** \param   output_args - vector to return output arguments in
** \param   is_complete - pointer to variable in which to return whether the operate response was indicating that the operate had completed
**                        or NULL if this information is not required
**                        This argument is only needed for async commands to differentiate an operate response containing an operate result from one not containing an operate result
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SendOperateAndProcessResponse(int group_id, char *path, bool is_sync, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args, bool *is_complete)
{
    int err;
    Usp__Msg *req;
    Usp__Msg *resp;
    usp_service_t *us;
    char msg_id[MAX_MSG_ID_LEN];

    // Find USP Service associated with the group_id
    us = FindUspServiceByGroupId(group_id);
    USP_ASSERT(us != NULL);

    // Exit if there is no connection to the USP Service anymore (this could occur if the socket disconnected in the meantime)
    if (us->controller_mtp.protocol == kMtpProtocol_None)
    {
        USP_LOG_Warning("%s: WARNING: Unable to send to UspService=%s. Connection dropped", __FUNCTION__, us->endpoint_id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Form the USP Operate Request message
    CalcBrokerMessageId(msg_id, sizeof(msg_id));
    req = MSG_UTILS_Create_OperateReq(msg_id, path, command_key, input_args);

    // Send the request and wait for a response
    // NOTE: request message is consumed by DM_EXEC_SendRequestAndWaitForResponse()
    resp = DM_EXEC_SendRequestAndWaitForResponse(us->endpoint_id, req, &us->controller_mtp,
                                                 USP__HEADER__MSG_TYPE__OPERATE_RESP,
                                                 RESPONSE_TIMEOUT);

    // Exit if timed out waiting for a response
    if (resp == NULL)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Process the operate response, determining if it was successful or not
    err = ProcessOperateResponse(resp, path, is_sync, output_args, is_complete);

    // Free the operate response, since we've finished with it
    usp__msg__free_unpacked(resp, pbuf_allocator);

    return err;
}

/*********************************************************************//**
**
** ProcessOperateResponse
**
** Processes a Operate Response that we have received from a USP Service
**
** \param   resp - USP response message in protobuf-c structure
** \param   path - USP command that was attempted
** \param   is_sync - set to true if the USP command is synchronous
** \param   output_args - pointer to key-value vector to fill in with the output arguments parsed from the USP esponse message
** \param   is_complete - pointer to variable in which to return whether the operate response was indicating that the operate had completed
**                        or NULL if this information is not required
**                        This argument is only needed for async commands to differentiate an operate response containing an operate result from one not containing an operate result
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessOperateResponse(Usp__Msg *resp, char *path, bool is_sync, kv_vector_t *output_args, bool *is_complete)
{
    int i;
    int err;
    Usp__OperateResp *oper;
    Usp__OperateResp__OperationResult *res;
    Usp__OperateResp__OperationResult__OutputArgs *args;
    Usp__OperateResp__OperationResult__CommandFailure *fail;
    Usp__OperateResp__OperationResult__OutputArgs__OutputArgsEntry *entry;
    bool is_finished = false;

    // Initialise default output arguments
    KV_VECTOR_Init(output_args);

    // Exit if the Message body contained an Error response, or the response failed to validate
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_OPERATE_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if operate response is missing
    oper = resp->body->response->operate_resp;
    if (oper == NULL)
    {
        USP_ERR_SetMessage("%s: Missing operate response", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if the number of operation_results does not match the expected number
    if (oper->n_operation_results != 1)
    {
        USP_ERR_SetMessage("%s: Unexpected number of operation results (got=%d, expected=1)", __FUNCTION__, (int)oper->n_operation_results);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if the operation wasn't the one we requested
    res = oper->operation_results[0];
    if (strcmp(res->executed_command, path) != 0)
    {
        USP_ERR_SetMessage("%s: Unexpected operation in response (got='%s', expected=%s')", __FUNCTION__, res->executed_command, path);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Determine if the operation was successful (for sync command) or successfully started (for async commands)
    switch(res->operation_resp_case)
    {
        case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OBJ_PATH:
            if (is_sync)
            {
                // This case should only occur for Async commands
                USP_ERR_SetMessage("%s: Synchronous operation unexpectedly returning request table path (%s)", __FUNCTION__, res->req_obj_path);
                err = USP_ERR_INTERNAL_ERROR;
            }
            else
            {
                // Async Operation started
                err = USP_ERR_OK;
            }
            break;

        case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_REQ_OUTPUT_ARGS:
            // Operation succeeded: Copy across output arguments
            args = res->req_output_args;
            for (i=0; i < args->n_output_args; i++)
            {
                entry = args->output_args[i];
                KV_VECTOR_Add(output_args, entry->key, entry->value);
            }

            is_finished = true;
            err = USP_ERR_OK;
            break;

        case USP__OPERATE_RESP__OPERATION_RESULT__OPERATION_RESP_CMD_FAILURE:
            // Operation failed
            fail = res->cmd_failure;
            USP_ERR_SetMessage("%s", fail->err_msg);
            err = fail->err_code;
            break;

        default:
            break;
    }

exit:
    if (is_complete != NULL)
    {
        *is_complete = is_finished;
    }

    return err;
}

/*********************************************************************//**
**
** ProcessGetInstancesResponse
**
** Processes a GetInstances Response that we have received from a USP Service,
** adding all object instances in it into the data model cache
**
** \param   resp - USP response message in protobuf-c structure
** \param   us - pointer to USP Service which we received the GetInstancesResponse from
** \param   within_vendor_hook - Determines whether this function is being called within the context of the
**                               refresh instances vendor hook (This has some restrictions on which object instances may be refreshed)
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessGetInstancesResponse(Usp__Msg *resp, usp_service_t *us, bool within_vendor_hook)
{
    int i, j;
    int err;
    Usp__GetInstancesResp *geti;
    Usp__GetInstancesResp__RequestedPathResult *rpr;
    Usp__GetInstancesResp__CurrInstance *ci;
    char *path;
    time_t expiry_time;

    // Exit if failed to validate that the Message body contains a GetInstances Response
    err = MSG_UTILS_ValidateUspResponse(resp, USP__RESPONSE__RESP_TYPE_GET_INSTANCES_RESP, NULL);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if get instances response is missing
    geti = resp->body->response->get_instances_resp;
    if (geti == NULL)
    {
        USP_ERR_SetMessage("%s: Missing get instances response", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all requested path results
    expiry_time = time(NULL) + BROKER_INSTANCE_CACHE_EXPIRY_PERIOD;
    USP_ASSERT((geti->n_req_path_results==0) || (geti->req_path_results != NULL));
    for (i=0; i < geti->n_req_path_results; i++)
    {
        // Skip this result if it is not filled in. NOTE: This should never happen
        rpr = geti->req_path_results[i];
        if (rpr == NULL)
        {
            continue;
        }

        // Exit if we received an error for this object
        if (rpr->err_code != USP_ERR_OK)
        {
            if (rpr->err_msg != NULL)
            {
                USP_ERR_SetMessage("%s: Received error '%s' for object '%s'", __FUNCTION__, rpr->err_msg, rpr->requested_path);
            }
            return rpr->err_code;
        }

        // Ensure the instances are in hierarchical order. This is necessary because DM_INST_VECTOR_RefreshInstance() requires parent instances to be registered before child instances
        qsort(rpr->curr_insts, rpr->n_curr_insts, sizeof(Usp__GetInstancesResp__CurrInstance *), CompareGetInstances_CurInst);

        // Iterate over all current instance objects
        for (j=0; j < rpr->n_curr_insts; j++)
        {
            ci = rpr->curr_insts[j];
            if (ci != NULL)
            {
                path = ci->instantiated_obj_path;
                if ((path != NULL) && (*path != '\0'))
                {
                    // Cache the object instance in the data model
                    // Intentionally ignoring any errors as we want to continue adding the other instances found
                    if (within_vendor_hook)
                    {
                        DM_INST_VECTOR_RefreshInstance(path);
                    }
                    else
                    {
                        DM_INST_VECTOR_SeedInstance(path, expiry_time, us->group_id);
                    }

                    // Register the unique keys for this object, if they haven't been already
                    if (ci->n_unique_keys > 0)
                    {
                        ProcessUniqueKeys(path, ci->unique_keys, ci->n_unique_keys);
                    }
                }
            }
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CompareGetInstances_CurInst
**
** Used by qsort to compare two entries in the rpr->curr_insts[] array
** The entries need to be sorted so that parent object instances appear before child object instances in the array
**
** \param   entry1 - pointer to first entry in the rpr->curr_insts[] array
** \param   entry2 - pointer to second entry in the rpr->curr_insts[] array
**
** \return  None
**
**************************************************************************/
int CompareGetInstances_CurInst(const void *entry1, const void *entry2)
{
    Usp__GetInstancesResp__CurrInstance *p1;
    Usp__GetInstancesResp__CurrInstance *p2;

    p1 = *((Usp__GetInstancesResp__CurrInstance **) entry1);
    p2 = *((Usp__GetInstancesResp__CurrInstance **) entry2);

    return strcmp(p1->instantiated_obj_path, p2->instantiated_obj_path);
}

/*********************************************************************//**
**
** ProcessUniqueKeys
**
** Registers the specified unique keys with the specified object, if relevant, and not already registered
**
** \param   path - Instantiated data model model path of the object
** \param   unique_keys - pointer to unique keys to process
** \param   num_unique_keys - number of unique keys
**
** \return  None
**
**************************************************************************/
void ProcessUniqueKeys(char *path, Usp__GetInstancesResp__CurrInstance__UniqueKeysEntry **unique_keys, int num_unique_keys)
{
    int i;
    dm_node_t *node;
    char *key_names[MAX_COMPOUND_KEY_PARAMS];  // NOTE: Ownership if the key names stays with the caller, rather than being transferred to tis array

    // Exit if path does not exist in the data model
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
    if (node == NULL)
    {
        USP_LOG_Warning("%s: USP Service erroneously provided a data model path (%s) which was not registered", __FUNCTION__, path);
        return;
    }

    // Exit if the node is not a multi-instance object
    if (node->type != kDMNodeType_Object_MultiInstance)
    {
        USP_LOG_Warning("%s: USP Service erroneously provided unique keys for a non multi-instance object", __FUNCTION__);
        return;
    }

    // Exit if the unique keys are already registered for the node. This is likely to be the case if this function has been called before for the table
    if (node->registered.object_info.unique_keys.num_entries != 0)
    {
        return;
    }

    // Truncate the list of unique keys to register if it's more than we can cope with
    if (num_unique_keys > MAX_COMPOUND_KEY_PARAMS)
    {
        USP_LOG_Error("%s: Truncating the number of unique keys registered for object %s. Increase MAX_COMPOUND_KEY_PARAMS to %d", __FUNCTION__, path, num_unique_keys);
        num_unique_keys = MAX_COMPOUND_KEY_PARAMS;
    }

    // Form array of unique key parameter names to register
    for (i=0; i < num_unique_keys; i++)
    {
        key_names[i] = unique_keys[i]->key;
    }

    USP_REGISTER_Object_UniqueKey(path, key_names, num_unique_keys); // Intentionally ignoring the error
}

/*********************************************************************//**
**
** ProcessGsdm_RequestedPath
**
** Parses the specified RequestedObjectResult of a GSDM Response, registering the data model elements found into the USP Broker's data model
**
** \param   ror - pointer to result object to parse
** \param   us - USP Service that sent the GDSM response
**
** \return  true if the RequestedObjectResult contained a successful response, and the data model elemts were added into the Broker's data model
**
**************************************************************************/
bool ProcessGsdm_RequestedPath(Usp__GetSupportedDMResp__RequestedObjectResult *ror, usp_service_t *us)
{
    int i;
    int index;

    // Exit if this result was not one of the paths that the USP Service originally requested
    index = STR_VECTOR_Find(&us->registered_paths, ror->req_obj_path);
    if (index == INVALID)
    {
        USP_LOG_Error("%s: Ignoring requested_object_result for '%s', as it wasn't requested", __FUNCTION__, ror->req_obj_path);
        return false;
    }

    // Exit if the USP Service encountered an error providing the supported data model for this path
    if (ror->err_code != USP_ERR_OK)
    {
        USP_LOG_Error("%s: USP Service did not provide data model for '%s' (err_code=%d, err_msg='%s')", __FUNCTION__, ror->req_obj_path, ror->err_code, ror->err_msg);
        return false;
    }

    // Ensure the supported objects are in hierarchical order. This is necessary because parent DM elements must be registered before child DM elements
    qsort(ror->supported_objs, ror->n_supported_objs, sizeof(Usp__GetSupportedDMResp__SupportedObjectResult *), CompareGsdm_SupportedObj);

    // Iterate over all supported objects, registering them into the data model
    for (i=0; i < ror->n_supported_objs; i++)
    {
        ProcessGsdm_SupportedObject(ror->supported_objs[i], us->group_id);
    }

    return true;
}

/*********************************************************************//**
**
** CompareGsdm_SupportedObj
**
** Used by qsort to compare two entries in the ror->supported_objs[] array
** The entries need to be sorted so that parent objects appear before child objects in the array
**
** \param   entry1 - pointer to first entry in the ror->supported_objs[] array
** \param   entry2 - pointer to second entry in the ror->supported_objs[] array
**
** \return  None
**
**************************************************************************/
int CompareGsdm_SupportedObj(const void *entry1, const void *entry2)
{
    Usp__GetSupportedDMResp__SupportedObjectResult *p1;
    Usp__GetSupportedDMResp__SupportedObjectResult *p2;

    p1 = *((Usp__GetSupportedDMResp__SupportedObjectResult **) entry1);
    p2 = *((Usp__GetSupportedDMResp__SupportedObjectResult **) entry2);

    return strcmp(p1->supported_obj_path, p2->supported_obj_path);
}

/*********************************************************************//**
**
** ProcessGsdm_SupportedObject
**
** Parses the specified SupportedObjectResult, registering the data model elements found into the USP Broker's data model
**
** \param   sor - pointer to result object to parse
** \param   group_id - group_id of the USP Service
**
** \return  None
**
**************************************************************************/
void ProcessGsdm_SupportedObject(Usp__GetSupportedDMResp__SupportedObjectResult *sor, int group_id)
{
    int i;
    int len;
    int err;
    char *p;
    bool is_writable;
    char path[MAX_DM_PATH];
    Usp__GetSupportedDMResp__SupportedParamResult *sp;
    Usp__GetSupportedDMResp__SupportedEventResult *se;
    Usp__GetSupportedDMResp__SupportedCommandResult *sc;
    unsigned type_flags;
    dm_node_t *node;

    USP_STRNCPY(path, sor->supported_obj_path, sizeof(path));
    len = strlen(path);

    // Exit if the path does not begin with "Device."
    if (strncmp(path, dm_root, dm_root_len) != 0)
    {
        USP_LOG_Error("%s: Object path to register is invalid (%s)", __FUNCTION__, path);
        return;
    }

    // Register the object
    if (sor->is_multi_instance)
    {
        // MULTI-INSTANCE OBJECT
        // Exit if unable to register the object
        is_writable = (sor->access != USP__GET_SUPPORTED_DMRESP__OBJ_ACCESS_TYPE__OBJ_READ_ONLY);

        // Add this path to the data model
        // NOTE: If the path is already present, then this just modifies its group_id
        node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_MultiInstance, OVERRIDE_LAST_TYPE);
        if (node == NULL)
        {
            USP_LOG_Error("%s: Failed to register multi-instance object '%s'", __FUNCTION__, path);
            return;
        }
        node->group_id = group_id;
        node->registered.object_info.group_writable = is_writable;

        // Register a refresh instances vendor hook if this is a top level object
        // (i.e one that contains only one instance separator, at the end of the string
        #define INSTANCE_SEPARATOR "{i}"
        p = strstr(path, INSTANCE_SEPARATOR);
        if ((p != NULL) && (strcmp(p, "{i}.") == 0))
        {
            // Exit if unable to register a refresh instances vendor hook
            err = USP_REGISTER_Object_RefreshInstances(path, Broker_RefreshInstances);
            if (err != USP_ERR_OK)
            {
                USP_LOG_Error("%s: Failed to register refresh instances vendor hook for object '%s'", __FUNCTION__, path);
                return;
            }
        }
    }
    else
    {
        // SINGLE-INSTANCE OBJECT
        // Add this path to the data model
        node = DM_PRIV_AddSchemaPath(path, kDMNodeType_Object_SingleInstance, SUPPRESS_PRE_EXISTANCE_ERR);
        if (node == NULL)
        {
            USP_LOG_Error("%s: Failed to register single-instance object '%s'", __FUNCTION__, path);
            return;
        }
        node->group_id = group_id;
    }

    //-----------------------------------------------------
    // Iterate over all child parameters, registering them
    for (i=0; i < sor->n_supported_params; i++)
    {
        sp = sor->supported_params[i];

        // Concatenate the parameter name to the end of the path
        USP_STRNCPY(&path[len], sp->param_name, sizeof(path)-len);

        // Skip if this parameter has already been registered. This may be the case, if a child object is registered before a parent object
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
        if (node != NULL)
        {
            continue;
        }

        // Register the parameter into the data model
        type_flags = CalcParamType(sp->value_type);

        if (sp->value_change == USP__GET_SUPPORTED_DMRESP__VALUE_CHANGE_TYPE__VALUE_CHANGE_WILL_IGNORE)
        {
            type_flags |= DM_VALUE_CHANGE_WILL_IGNORE;
        }

        if (sp->access == USP__GET_SUPPORTED_DMRESP__PARAM_ACCESS_TYPE__PARAM_READ_ONLY)
        {
            err = USP_REGISTER_GroupedVendorParam_ReadOnly(group_id, path, type_flags);
        }
        else
        {
            err = USP_REGISTER_GroupedVendorParam_ReadWrite(group_id, path, type_flags);
        }

        // Log an error, if failed to register the parameter
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to register parameter '%s'", __FUNCTION__, path);
        }
    }

    //-----------------------------------------------------
    // Iterate over all child USP events, registering them
    for (i=0; i < sor->n_supported_events; i++)
    {
        se = sor->supported_events[i];

        // Concatenate the event name to the end of the path
        USP_STRNCPY(&path[len], se->event_name, sizeof(path)-len);

        // Skip if this event has already been registered. This may be the case, if a child object is registered before a parent object
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
        if (node != NULL)
        {
            continue;
        }

        // Skip this event, if failed to register the event
        err = USP_REGISTER_Event(path);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to register event '%s'", __FUNCTION__, path);
            continue;
        }

        // Register the group_id for this event
        err = USP_REGISTER_GroupId(path, group_id);
        USP_ASSERT(err == USP_ERR_OK);

        // Skip, if failed to register the event's arguments
        err = USP_REGISTER_EventArguments(path, se->arg_names, se->n_arg_names);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to register arguments for event '%s'", __FUNCTION__, path);
            continue;
        }
    }

    //-----------------------------------------------------
    // Iterate over all child USP commands, registering them
    for (i=0; i < sor->n_supported_commands; i++)
    {
        sc = sor->supported_commands[i];

        // Concatenate the command name to the end of the path
        USP_STRNCPY(&path[len], sc->command_name, sizeof(path)-len);

        // Skip if this command has already been registered. This may be the case, if a child object is registered before a parent object
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
        if (node != NULL)
        {
            continue;
        }

        // Register this command
        switch(sc->command_type)
        {
            case USP__GET_SUPPORTED_DMRESP__CMD_TYPE__CMD_SYNC:
                err = USP_REGISTER_SyncOperation(path, Broker_SyncOperate);
                break;

            case USP__GET_SUPPORTED_DMRESP__CMD_TYPE__CMD_ASYNC:
            default:
                err = USP_REGISTER_AsyncOperation(path, Broker_AsyncOperate, NULL);
                break;
        }

        // Skip this command, if failed to register
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to register command '%s'", __FUNCTION__, path);
            continue;
        }

        // Register the group_id for this USP command
        err = USP_REGISTER_GroupId(path, group_id);
        USP_ASSERT(err == USP_ERR_OK);

        // Skip, if failed to register the command's arguments
        err = USP_REGISTER_OperationArguments(path, sc->input_arg_names, sc->n_input_arg_names, sc->output_arg_names, sc->n_output_arg_names);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: Failed to register arguments for command '%s'", __FUNCTION__, path);
            continue;
        }
    }
}

/*********************************************************************//**
**
** ShouldPathBeAddedToDataModel
**
** Determines whether a path, specified in the register message, should be added into the Broker's data model
**
** \param   us - USP Service that is attempting to register the path
** \param   path - path to data model element which USP Service wants to add to Broker's data model
**
** \return  USP_ERR_OK if the path should be added to the Broker's data model,
**          otherwise return an error code indicating why the paths shouldn't be added
**
**************************************************************************/
int ShouldPathBeAddedToDataModel(usp_service_t *us, char *path)
{
    bool is_registered;
    bool is_exception;
    bool is_valid;
    bool is_same_group_id;
    dm_node_t *node;

    // Exit if this path is the parent of one which has already been registered to this USP Service
    // In which case, we allow the parent to be registered after the child, but only if the parent
    // does not have any children owned by another USP Service or the internal data model
    is_registered = IsPathAlreadyRegisteredAsAParent(us, path);
    if (is_registered)
    {
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
        USP_ASSERT(node != NULL);

        is_same_group_id = DM_PRIV_AreAllChildrenGroupId(node, us->group_id);
        if (is_same_group_id == false)
        {
            USP_ERR_SetMessage("%s: Requested path '%s' has some children owned by a different USP Service", __FUNCTION__, path);
            return USP_ERR_REGISTER_FAILURE;
        }

        return USP_ERR_OK;
    }

    // Exit if path is already registered, so should not be added.
    is_registered = IsPathAlreadyRegistered(path);
    if (is_registered)
    {
        return USP_ERR_PATH_ALREADY_REGISTERED;
    }

    // Exit if path is one of the list of special exception paths, so should be added
    is_exception = IsPathASpecialException(path, INVALID);
    if (is_exception == true)
    {
        return USP_ERR_OK;
    }

    // Exit if path looks textually invalid, so should not be added
    is_valid = IsValidUspServicePath(path);
    if (is_valid == false)
    {
        return USP_ERR_REGISTER_FAILURE;
    }

    // If the code gets here, then the path looks valid, so should be added
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** IsPathAlreadyRegisteredAsAParent
**
** Determines whether the specified path has already been registered as a parent of a path owned by the specified USP Service
** and that the path doesn't have any children which are registered to any other USP Services
** This deals with the case of Device.Parent being registered after Device.Parent.Child
**
** \param   us - USP Service that is attempting to register the path
** \param   path - path of the data model object to register
**
** \return  true if the path has already been registered into the data model (as a parent of an object owned by this USP Service)
**
**************************************************************************/
bool IsPathAlreadyRegisteredAsAParent(usp_service_t *us, char *path)
{
    int i;
    int len;
    char *rpath;
    int rpath_len;

    // Exit if the requested path does not end in a dot. This error will be handled later by IsValidUspServicePath()
    // (We check this to prevent the code below from matching on a partial object name)
    len = strlen(path);
    if (path[len-1] != '.')
    {
        return false;
    }

    // Iterate over all paths registered by this USP Service, seeing if any already contain this path, as their parent
    for (i=0; i < us->registered_paths.num_entries; i++)
    {
        rpath = us->registered_paths.vector[i];
        rpath_len = strlen(rpath);
        if ((rpath_len > len) && (memcmp(rpath, path, len)==0))
        {
            return true;
        }
    }

    // If the code gets here, then no match was found
    return false;
}

/*********************************************************************//**
**
** IsPathAlreadyRegistered
**
** Determines whether the specified path has been registered into the data model before
**
** \param   requested_path - path of the data model object to register
**
** \return  true if the path has already been registered into the data model
**
**************************************************************************/
bool IsPathAlreadyRegistered(char *requested_path)
{
    int i;
    usp_service_t *us;
    int index;
    unsigned flags;

    // Exit if this path has already been registered by any of the USP Services (including this USP Service)
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        index = STR_VECTOR_Find(&us->registered_paths, requested_path);
        if (index != INVALID)
        {
            USP_ERR_SetMessage("%s: Requested path '%s' has already been registered by endpoint '%s'", __FUNCTION__, requested_path, us->endpoint_id);
            return USP_ERR_PATH_ALREADY_REGISTERED;
        }
    }

    // Exit if this path already exists in the schema e.g it may be one of the paths that are internal to this USP Broker
    flags = DATA_MODEL_GetPathProperties(requested_path, INTERNAL_ROLE, NULL, NULL, NULL, DONT_LOG_ERRORS);
    if (flags & PP_EXISTS_IN_SCHEMA)
    {
        USP_ERR_SetMessage("%s: Requested path '%s' already exists in the data model", __FUNCTION__, requested_path);
        return USP_ERR_PATH_ALREADY_REGISTERED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** IsPathASpecialException
**
** Determines whether the path is one of the specific data model elements that we allow to be registered individually
** These are parameters, commands and events directly under Device.
** These DM elements are not allowed to be registsred by requirement R-REG.2 of the USP Spec, as they are not an object
** However, we need to allow them, as they may be owned by a USP Service
** NOTE: If the path is an exception, and the group_id argument is not INVALID, then this function also adds the path
**       into the Broker's data model.
**
** \param   path - path of the data model object to register
** \param   group_id - If set to INVALID, the path is not registered. If set to a value other then INVALID, the path is registered into the Broker's data model
**
** \return  true if the path is one one of the specific data model elements that we allow to be registered individually, false otherwise
**
**************************************************************************/
bool IsPathASpecialException(char *path, int group_id)
{
#ifdef REMOVE_DEVICE_REBOOT
    // Handle Device.Reboot()
    if (strcmp(path, "Device.Reboot()")==0)
    {
        if (group_id != INVALID)
        {
            USP_REGISTER_SyncOperation(path, Broker_SyncOperate);
            USP_REGISTER_GroupId(path, group_id);
        }
        return true;
    }
#endif

#ifdef REMOVE_DEVICE_FACTORY_RESET
    // Handle Device.FactoryReset()
    if (strcmp(path, "Device.FactoryReset()")==0)
    {
        if (group_id != INVALID)
        {
            USP_REGISTER_SyncOperation(path, Broker_SyncOperate);
            USP_REGISTER_GroupId(path, group_id);
        }
        return true;
    }
#endif

#ifdef REMOVE_DEVICE_SCHEDULE_TIMER
    // Handle Device.ScheduleTimer()
    if (strcmp(path, "Device.ScheduleTimer()")==0)
    {
        if (group_id != INVALID)
        {
            static char *sched_timer_input_args[] = { "DelaySeconds" };
            USP_REGISTER_AsyncOperation(path, Broker_AsyncOperate, NULL);
            USP_REGISTER_GroupId(path, group_id);
            USP_REGISTER_OperationArguments(path, sched_timer_input_args, NUM_ELEM(sched_timer_input_args), NULL, 0);
        }
        return true;
    }
#endif

#ifdef REMOVE_DEVICE_BOOT_EVENT
    // Handle Device.Boot!
    if (strcmp(path, "Device.Boot!")==0)
    {
        if (group_id != INVALID)
        {
            static char *boot_event_args[] = { "CommandKey", "Cause", "FirmwareUpdated", "ParameterMap" };
            USP_REGISTER_Event(path);
            USP_REGISTER_GroupId(path, group_id);
            USP_REGISTER_EventArguments(path, boot_event_args, NUM_ELEM(boot_event_args));
        }
        return true;
    }
#endif

    // Handle Device.LocalAgent.TransferComplete!
    if (strcmp(path, "Device.LocalAgent.TransferComplete!")==0)
    {
        if (group_id != INVALID)
        {
            static char *transfer_complete_args[] = { "Command", "CommandKey", "Requestor",
                                                      "TransferType", "Affected", "TransferURL",
                                                      "StartTime", "CompleteTime", "FaultCode", "FaultString",
                                                      "CheckSumAlgorithm", "CheckSum" };
            USP_REGISTER_Event(path);
            USP_REGISTER_GroupId(path, group_id);
            USP_REGISTER_EventArguments(path, transfer_complete_args, NUM_ELEM(transfer_complete_args));
        }
        return true;
    }

#ifdef REMOVE_SELF_TEST_DIAG_EXAMPLE
    // Handle Device.SelfTestDiagnostics()
    if (strcmp(path, "Device.SelfTestDiagnostics()")==0)
    {
        if (group_id != INVALID)
        {
            static char *self_test_output_args[] ={ "Status", "Results" };

            USP_REGISTER_AsyncOperation(path, Broker_AsyncOperate, NULL);
            USP_REGISTER_GroupId(path, group_id);
            USP_REGISTER_OperationArguments(path, NULL, 0, self_test_output_args, NUM_ELEM(self_test_output_args));
        }
        return true;
    }
#endif

    // Handle Device.PacketCaptureDiagnostics()
    if (strcmp(path, "Device.PacketCaptureDiagnostics()")==0)
    {
        if (group_id != INVALID)
        {
            static char *packet_capture_input_args[] = { "Interface", "Format", "Duration", "PacketCount",
                                                         "ByteCount", "FileTarget", "FilterExpression",
                                                         "Username", "Password" };
            static char *packet_capture_output_args[] ={ "Status",
                                                         "PacketCaptureResult.{i}.FileLocation",
                                                         "PacketCaptureResult.{i}.StartTime",
                                                         "PacketCaptureResult.{i}.EndTime",
                                                         "PacketCaptureResult.{i}.Count" };

            USP_REGISTER_AsyncOperation(path, Broker_AsyncOperate, NULL);
            USP_REGISTER_GroupId(path, group_id);
            USP_REGISTER_OperationArguments(path, packet_capture_input_args, NUM_ELEM(packet_capture_input_args),
                                                  packet_capture_output_args, NUM_ELEM(packet_capture_output_args));
        }
        return true;
    }

    // Handle Device.InterfaceStackNumberOfEntries
    if (strcmp(path, "Device.InterfaceStackNumberOfEntries")==0)
    {
        if (group_id != INVALID)
        {
            USP_REGISTER_GroupedVendorParam_ReadOnly(group_id, path, DM_UINT);
        }
        return true;
    }

    // Handle Device.RootDataModelVersion
    if (strcmp(path, "Device.RootDataModelVersion")==0)
    {
        if (group_id != INVALID)
        {
            USP_REGISTER_GroupedVendorParam_ReadOnly(group_id, path, DM_STRING);
        }
        return true;
    }

    // If the code gets here, then the path did not match any of the exceptions
    return false;
}

/*********************************************************************//**
**
** IsValidUspServicePath
**
** Determines whether the specified path is textually a valid data model path for a register message
**
** \param   path - Data model path received in the USP Register message
**
** \return  true if the path appears to be valid
**
**************************************************************************/
bool IsValidUspServicePath(char *path)
{
    int len;
    char *p;

    // Exit if the path does not start with 'Device.'
    if (strncmp(path, dm_root, dm_root_len) != 0)
    {
        USP_ERR_SetMessage("%s: Requested path '%s' does not start 'Device.'", __FUNCTION__, path);
        return false;
    }

    // Exit if the path is for a data model element directly under Device., which has not been handled earlier as a special exception
    p = strchr(&path[dm_root_len], '.');
    if (p == NULL)
    {
        USP_ERR_SetMessage("%s: %s is not one of the data model elements directly under Device. that are supported for registration", __FUNCTION__, path);
        return false;
    }

    // Exit if the path does not end in a dot
    len = strlen(path);
    if (path[len-1] != '.')
    {
        USP_ERR_SetMessage("%s: Requested path '%s' must be a data model object ending in '.'", __FUNCTION__, path);
        return false;
    }

    // Exit if the path contains any characters it shouldn't (Only alphanumerics and period character are allowed)
    p = path;
    while (*p != '\0')
    {
        if ((IS_ALPHA_NUMERIC(*p) == false) && (*p != '-') && (*p != '_') && (*p != '.'))
        {
            USP_ERR_SetMessage("%s: Requested path '%s' is invalid. (e.g. It must not contain '{i}')", __FUNCTION__, path);
            return false;
        }
        p++;
    }

    // Exit if path contains any instance numbers (ie a period immediately followed by an instance number)
    p = strchr(path, '.');
    while (p != NULL)
    {
        p++;        // Move to character after path delimiter
        if (IS_NUMERIC(*p))
        {
            USP_ERR_SetMessage("%s: Requested path '%s' is invalid. It is not allowed to contain instance numbers.", __FUNCTION__, path);
            return false;
        }

        // Move to next path delimiter
        p = strchr(p, '.');
    }

    return true;
}

/*********************************************************************//**
**
** CalcParamType
**
** Convert from the protobuf parameter type enumeration to our enumeration
**
** \param   value_type - protobuf parameter type enumeration to convert
**
** \return  internal parameter type enumeration
**
**************************************************************************/
unsigned CalcParamType(Usp__GetSupportedDMResp__ParamValueType value_type)
{
    unsigned type_flags;

    switch(value_type)
    {
        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_BASE_64:
            type_flags = DM_BASE64;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_BOOLEAN:
            type_flags = DM_BOOL;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_DATE_TIME:
            type_flags = DM_DATETIME;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_DECIMAL:
            type_flags = DM_DECIMAL;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_HEX_BINARY:
            type_flags = DM_HEXBIN;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_INT:
            type_flags = DM_INT;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_LONG:
            type_flags = DM_LONG;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNSIGNED_INT:
            type_flags = DM_UINT;
            break;

        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_UNSIGNED_LONG:
            type_flags = DM_ULONG;
            break;

        default:
        case USP__GET_SUPPORTED_DMRESP__PARAM_VALUE_TYPE__PARAM_STRING:
            type_flags = DM_STRING;
            break;
    }

    return type_flags;
}

/*********************************************************************//**
**
** HandleUspServiceAgentDisconnect
**
** Called when a USP Service's agent disconnects
** This causes all of the data model registered by the USP Service to be removed from the Broker's supported data model
**
** \param   us - USP Service whose agent has disconnected from UDS
** \param   flags - bitmask of flags controlling execution e.g. FAIL_USP_COMMANDS_IN_PROGRESS
**
** \return  None
**
**************************************************************************/
void HandleUspServiceAgentDisconnect(usp_service_t *us, unsigned flags)
{
    int i;
    char *path;
    char err_msg[256];
    req_map_t *rmap;

    // Mark all subscriptions that are currently being satisfied by this USP Service as being satisfied by the core mechanism
    DEVICE_SUBSCRIPTION_FreeAllVendorLayerSubsForGroup(us->group_id);
    SubsMap_Destroy(&us->subs_map);

    // Send an OperationComplete indicating failure for all currently active USP Commands being processed by the USP Service
    // This also results in the entry in the Broker's Request table for the USP Command being deleted
    if (flags & FAIL_USP_COMMANDS_IN_PROGRESS)
    {
        while (us->req_map.head != NULL)
        {
            rmap = (req_map_t *) us->req_map.head;
            USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: USP Service implementing command (%s) disconnected", __FUNCTION__, us->endpoint_id);
            DEVICE_REQUEST_OperationComplete(rmap->request_instance, USP_ERR_COMMAND_FAILURE, err_msg, NULL);

            ReqMap_Remove(&us->req_map, rmap);
        }
    }

    // NOTE: The passback message_ids in us->msg_map are all responses from the Agent of the USP Service
    // Since this agent has disconnected, these message_ids are not expected anymore and so should be removed from the mapping table
    // If the USP Service hadn't crashed, but had just restarted the UDS connection, then sent the expected response,
    // the response would be discarded as it wouldn't match any that would be in the us->msg_map after the USP Service had reconnected
    MsgMap_Destroy(&us->msg_map);

    // Remove all paths owned by the USP Service from the supported data model (the instance cache for these objects is also removed)
    for (i=0; i < us->registered_paths.num_entries; i++)
    {
        path = us->registered_paths.vector[i];
        DATA_MODEL_DeRegisterPath(path);  // Intentionally ignoring error
    }
    STR_VECTOR_Destroy(&us->registered_paths);
}

/*********************************************************************//**
**
** GetUspService_EndpointID
**
** Gets the value of Device.USPServices.USPService.{i}.EndpointID
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetUspService_EndpointID(dm_req_t *req, char *buf, int len)
{
    usp_service_t *us;

    us = FindUspServiceByInstance(inst1);
    USP_ASSERT(us != NULL);

    USP_STRNCPY(buf, us->endpoint_id, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetUspService_Protocol
**
** Gets the value of Device.USPServices.USPService.{i}.Protocol
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetUspService_Protocol(dm_req_t *req, char *buf, int len)
{
    usp_service_t *us;
    mtp_protocol_t protocol;
    char *protocol_str;

    us = FindUspServiceByInstance(inst1);
    USP_ASSERT(us != NULL);

    // We use the protocol used by the Broker's controller socket, or if this is not connected, the protocol used by the Broker's agent socket
    protocol = (us->controller_mtp.protocol != kMtpProtocol_None) ? us->controller_mtp.protocol : us->agent_mtp.protocol;
    protocol_str = DEVICE_MTP_EnumToString(protocol);

    USP_STRNCPY(buf, protocol_str, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetUspService_DMPaths
**
** Gets the value of Device.USPServices.USPService.{i}.DataModelPaths
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetUspService_DMPaths(dm_req_t *req, char *buf, int len)
{
    usp_service_t *us;

    us = FindUspServiceByInstance(inst1);
    USP_ASSERT(us != NULL);

    TEXT_UTILS_ListToString(us->registered_paths.vector, us->registered_paths.num_entries, buf, len);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetUspService_HasController
**
** Gets the value of Device.USPServices.USPService.{i}.HasController
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetUspService_HasController(dm_req_t *req, char *buf, int len)
{
    usp_service_t *us;

    us = FindUspServiceByInstance(inst1);
    USP_ASSERT(us != NULL);

    val_bool = us->has_controller;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CreateRegisterResp
**
** Dynamically creates a Register Response object
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the request, which initiated this response
**
** \return  Pointer to a Register Response object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateRegisterResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__RegisterResp *reg_resp;

    // Create Register Response
    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__REGISTER_RESP, USP__RESPONSE__RESP_TYPE_REGISTER_RESP);
    reg_resp = USP_MALLOC(sizeof(Usp__RegisterResp));
    usp__register_resp__init(reg_resp);
    msg->body->response->register_resp = reg_resp;

    return msg;
}

/*********************************************************************//**
**
** AddRegisterResp_RegisteredPathResult
**
** Dynamically adds a registered path result to the RegisterResponse object
**
** \param   reg_resp - pointer to RegisterResponse object
** \param   requested_path - path that was requested to be registered
** \param   err_code - numeric code indicating whether the path was registered successfully or not
**
** \return  None
**
**************************************************************************/
void AddRegisterResp_RegisteredPathResult(Usp__RegisterResp *reg_resp, char *requested_path, int err_code)
{
    Usp__RegisterResp__RegisteredPathResult *reg_path_result;
    Usp__RegisterResp__RegisteredPathResult__OperationStatus *oper_status;
    Usp__RegisterResp__RegisteredPathResult__OperationStatus__OperationFailure *oper_failure;
    Usp__RegisterResp__RegisteredPathResult__OperationStatus__OperationSuccess *oper_success;
    char *err_str;
    int new_num;    // new number of requested_path_results

    // Create the RegistereddPathResult object
    reg_path_result = USP_MALLOC(sizeof(Usp__RegisterResp__RegisteredPathResult));
    usp__register_resp__registered_path_result__init(reg_path_result);

    // Increase the size of the vector containing pointers to the registered_path_results
    // adding the RegisteredPathReult object to the end
    new_num = reg_resp->n_registered_path_results + 1;
    reg_resp->registered_path_results = USP_REALLOC(reg_resp->registered_path_results, new_num*sizeof(void *));
    reg_resp->n_registered_path_results = new_num;
    reg_resp->registered_path_results[new_num-1] = reg_path_result;

    // Create an OperationStatus object
    oper_status = USP_MALLOC(sizeof(Usp__RegisterResp__RegisteredPathResult__OperationStatus));
    usp__register_resp__registered_path_result__operation_status__init(oper_status);

    if (err_code == USP_ERR_OK)
    {
        // Create an OperSuccess object, and add it into the OperationStatus object
        oper_success = USP_MALLOC(sizeof(Usp__RegisterResp__RegisteredPathResult__OperationStatus__OperationSuccess));
        usp__register_resp__registered_path_result__operation_status__operation_success__init(oper_success);
        oper_success->registered_path = USP_STRDUP(requested_path);

        oper_status->oper_status_case = USP__REGISTER_RESP__REGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS;
        oper_status->oper_success = oper_success;
    }
    else
    {
        // Create an OperFailure object, and add it into the OperationStatus object
        oper_failure = USP_MALLOC(sizeof(Usp__RegisterResp__RegisteredPathResult__OperationStatus__OperationFailure));
        usp__register_resp__registered_path_result__operation_status__operation_failure__init(oper_failure);
        err_str = USP_ERR_GetMessage();
        oper_failure->err_code = err_code;
        oper_failure->err_msg = USP_STRDUP(err_str);

        oper_status->oper_status_case = USP__REGISTER_RESP__REGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE;
        oper_status->oper_failure = oper_failure;
    }

    // Add the OperStatus object into the RegisterPathResult object
    reg_path_result->requested_path = USP_STRDUP(requested_path);
    reg_path_result->oper_status = oper_status;
}

/*********************************************************************//**
**
** DeRegisterAllPaths
**
** This function is called to handle the special case of a path in the Deregister request
** containing empty string, which denotes that all paths currently owned by the USP service should be deregistered
** This function deregisters all paths and deals with the complex case of some paths deregistering
** successfully and some paths deregistering unsuccessfully
**
** \param   us - USP Service to deregister all paths of
** \param   dreg_resp - Deregister response message to add to
**
** \return  None
**
**************************************************************************/
void DeRegisterAllPaths(usp_service_t *us, Usp__DeregisterResp *dreg_resp)
{
    int err;
    char path[MAX_DM_PATH];
    char err_msg[256];
    Usp__DeregisterResp__DeregisteredPathResult *dreg_path_result = NULL;

    // NOTE: We drain the vector, rather than iterating over it because DeRegisterUspServicePath removes entries from the array
    while (us->registered_paths.num_entries > 0)
    {
        USP_STRNCPY(path, us->registered_paths.vector[0], sizeof(path));    // Take a copy of registered path, because DeRegisterUspServicePath is going to free it from us->registered_paths
        err = DeRegisterUspServicePath(us, path);

        if (err == USP_ERR_OK)
        {
            // Path deregistered successfully
            if (dreg_path_result == NULL)
            {
                // No success object added yet, so add one now with this path
                dreg_path_result = AddDeRegisterResp_DeRegisteredPathResult(dreg_resp, "", path, err, NULL);
            }
            else
            {
                // Success object already exists, so just add another path
                AddDeRegisterRespSuccess_Path(dreg_path_result, path);
            }
        }
        else
        {
            // Path failed to deregister
            // Remove the current result from the DeRegister response for this registered path
            RemoveDeRegisterResp_DeRegisteredPathResult(dreg_resp);

            // Exit, noting the first path which failed in the response
            USP_SNPRINTF(err_msg, sizeof(err_msg), "%s: Failed to deregister %s (%s)", __FUNCTION__, path, USP_ERR_GetMessage());
            AddDeRegisterResp_DeRegisteredPathResult(dreg_resp, "", path, err, err_msg);
            return;
        }
    }
}

/*********************************************************************//**
**
** CreateDeRegisterResp
**
** Dynamically creates a DeRegister Response object
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the request, which initiated this response
**
** \return  Pointer to a DeRegister Response object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *CreateDeRegisterResp(char *msg_id)
{
    Usp__Msg *msg;
    Usp__DeregisterResp *dreg_resp;

    // Create Register Response
    msg = MSG_HANDLER_CreateResponseMsg(msg_id, USP__HEADER__MSG_TYPE__DEREGISTER_RESP, USP__RESPONSE__RESP_TYPE_DEREGISTER_RESP);
    dreg_resp = USP_MALLOC(sizeof(Usp__DeregisterResp));
    usp__deregister_resp__init(dreg_resp);
    msg->body->response->deregister_resp = dreg_resp;

    return msg;
}

/*********************************************************************//**
**
** AddDeRegisterResp_DeRegisteredPathResult
**
** Dynamically adds a deregistered path result to the DeRegisterResponse object
**
** \param   dereg_resp - pointer to DeRegisterResponse object
** \param   requested_path - path that was requested to be deregistered
** \param   path - path that was actually deregistered (this may differ from the requested path in the special case of deregistering all paths for a USP service)
** \param   err_code - numeric code indicating whether the path was deregistered successfully or not
** \param   err_msg - textual error message to include if err_code indicated an error
**
** \return  Pointer to deregistered path result object
**
**************************************************************************/
Usp__DeregisterResp__DeregisteredPathResult *AddDeRegisterResp_DeRegisteredPathResult(Usp__DeregisterResp *dreg_resp, char *requested_path, char *path, int err_code, char *err_msg)
{
    Usp__DeregisterResp__DeregisteredPathResult *dreg_path_result;
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus *oper_status;
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationFailure *oper_failure;
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationSuccess *oper_success;
    char **dreg_paths;
    int new_num;    // new number of requested_path_results

    // Create the DeRegisteredPathResult object
    dreg_path_result = USP_MALLOC(sizeof(Usp__DeregisterResp__DeregisteredPathResult));
    usp__deregister_resp__deregistered_path_result__init(dreg_path_result);

    // Increase the size of the vector containing pointers to the deregistered_path_results
    // adding the RegisteredPathResult object to the end
    new_num = dreg_resp->n_deregistered_path_results + 1;
    dreg_resp->deregistered_path_results = USP_REALLOC(dreg_resp->deregistered_path_results, new_num*sizeof(void *));
    dreg_resp->n_deregistered_path_results = new_num;
    dreg_resp->deregistered_path_results[new_num-1] = dreg_path_result;

    // Create an OperationStatus object
    oper_status = USP_MALLOC(sizeof(Usp__DeregisterResp__DeregisteredPathResult__OperationStatus));
    usp__deregister_resp__deregistered_path_result__operation_status__init(oper_status);

    if (err_code == USP_ERR_OK)
    {
        // Create an OperSuccess object, and add it into the OperationStatus object
        oper_success = USP_MALLOC(sizeof(Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationSuccess));
        usp__deregister_resp__deregistered_path_result__operation_status__operation_success__init(oper_success);
        oper_success->n_deregistered_path = 1;

        dreg_paths = USP_MALLOC(sizeof(char *));
        oper_success->deregistered_path = dreg_paths;
        dreg_paths[0] = USP_STRDUP(path);

        oper_status->oper_status_case = USP__DEREGISTER_RESP__DEREGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS;
        oper_status->oper_success = oper_success;
    }
    else
    {
        // Create an OperFailure object, and add it into the OperationStatus object
        oper_failure = USP_MALLOC(sizeof(Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationFailure));
        usp__deregister_resp__deregistered_path_result__operation_status__operation_failure__init(oper_failure);
        oper_failure->err_code = err_code;
        oper_failure->err_msg = USP_STRDUP(err_msg);

        oper_status->oper_status_case = USP__DEREGISTER_RESP__DEREGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE;
        oper_status->oper_failure = oper_failure;
    }

    // Add the OperStatus object into the DeRegisterPathResult object
    dreg_path_result->requested_path = USP_STRDUP(requested_path);
    dreg_path_result->oper_status = oper_status;

    return dreg_path_result;
}

/*********************************************************************//**
**
** RemoveDeRegisterResp_DeRegisteredPathResult
**
** Dynamically removes the last deregistered path result from the DeRegisterResponse object
**
** \param   dereg_resp - pointer to DeRegisterResponse object
**
** \return  None
**
**************************************************************************/
void RemoveDeRegisterResp_DeRegisteredPathResult(Usp__DeregisterResp *dreg_resp)
{
    Usp__DeregisterResp__DeregisteredPathResult *dreg_path_result;
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus *oper_status;
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationFailure *oper_failure;
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationSuccess *oper_success;
    int i;

    // Exit if there is no deregistered path result to remove
    if (dreg_resp->n_deregistered_path_results == 0)
    {
        return;
    }

    dreg_path_result = dreg_resp->deregistered_path_results[dreg_resp->n_deregistered_path_results - 1];
    oper_status = dreg_path_result->oper_status;
    switch(oper_status->oper_status_case)
    {
        case USP__DEREGISTER_RESP__DEREGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_SUCCESS:
            oper_success = oper_status->oper_success;
            for (i=0; i < oper_success->n_deregistered_path; i++)
            {
                USP_FREE(oper_success->deregistered_path[i]);
            }
            USP_FREE(oper_success->deregistered_path);
            USP_FREE(oper_success);
            break;

        case USP__DEREGISTER_RESP__DEREGISTERED_PATH_RESULT__OPERATION_STATUS__OPER_STATUS_OPER_FAILURE:
            oper_failure = oper_status->oper_failure;
            USP_FREE(oper_failure->err_msg);
            USP_FREE(oper_failure);
            break;

        default:
            TERMINATE_BAD_CASE(oper_status->oper_status_case);
            break;
    }

    USP_FREE(oper_status);
    USP_SAFE_FREE(dreg_path_result->requested_path);
    USP_FREE(dreg_path_result);
    dreg_resp->n_deregistered_path_results--;
}


/*********************************************************************//**
**
** AddDeRegisterRespSuccess_Path
**
** Dynamically adds a path to the success object of a deregistered path result object
**
** \param   dreg_path_result - pointer to deregistered path result object
** \param   path - path that was deregistered to add to success object
**
** \return  None
**
**************************************************************************/
void AddDeRegisterRespSuccess_Path(Usp__DeregisterResp__DeregisteredPathResult *dreg_path_result, char *path)
{
    Usp__DeregisterResp__DeregisteredPathResult__OperationStatus__OperationSuccess *oper_success;
    int new_num;

    oper_success = dreg_path_result->oper_status->oper_success;
    new_num = oper_success->n_deregistered_path + 1;
    oper_success->deregistered_path = USP_REALLOC(oper_success->deregistered_path, new_num*sizeof(char *));
    oper_success->n_deregistered_path = new_num;
    oper_success->deregistered_path[new_num-1] = USP_STRDUP(path);
}

/*********************************************************************//**
**
** AttemptPassThruForResponse
**
** Route the USP response message back to the USP Service that originated the request
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool AttemptPassThruForResponse(Usp__Msg *usp, char *endpoint_id)
{
    usp_service_t *us;
    msg_map_t *map;

    // Exit if message was badly formed - the error will be handled by the normal handlers
    if ((usp->body == NULL) ||
        ((usp->body->msg_body_case != USP__BODY__MSG_BODY_RESPONSE) && (usp->body->msg_body_case != USP__BODY__MSG_BODY_ERROR)) ||
        (usp->header == NULL) || (usp->header->msg_id == NULL))
    {
        return false;
    }

    // Exit if this response did not come from a USP Service
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        return false;
    }

    // Exit if this is not a response to any of the request messages which have been passed through to the USP Service
    map = MsgMap_Find(&us->msg_map, usp->header->msg_id);
    if (map == NULL)
    {
        return false;
    }

    // Remap the message_id in the response back to the original message_id that the originator is expecting
    USP_FREE(usp->header->msg_id);
    usp->header->msg_id = USP_STRDUP(map->original_msg_id);
    USP_LOG_Info("Passback %s to '%s'", MSG_HANDLER_UspMsgTypeToString(usp->header->msg_type), map->originator);

    // Send the message back to the originator
    // NOTE: Ignoring any errors, since if we cannot send the response, there's nothing we can do other than drop it
    MSG_HANDLER_QueueMessage(map->originator, usp, &map->mtp_conn);

    // Remove the message map, since we are not expecting another response from the USP service for the same message_id
    MsgMap_Remove(&us->msg_map, map);

    return true;
}

/*********************************************************************//**
**
** AttemptPassThruForGetRequest
**
** Route the Get request to the relevant USP Service, if it can be satisfied by a single USP Service
** and there are no permissions preventing the request being fulfilled
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   combined_role - roles that the originator has (inherited & assigned)
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool AttemptPassThruForGetRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec)
{
    int i;
    Usp__Get *get;
    char *path;
    dm_node_t *node;
    int group_id = INVALID;
    int depth;
    bool is_permitted;
    usp_service_t *us = NULL;
    int err;

    // Exit if message was badly formed - the error will be handled by the normal handlers
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_GET) ||
        (usp->body->request->get == NULL) || (usp->body->request->get->n_param_paths==0))
    {
        return false;
    }

    // Calculate the number of hierarchical levels to traverse in the data model when checking permissions
    depth = usp->body->request->get->max_depth;
    if (depth == 0)
    {
        depth = FULL_DEPTH;
    }

    get = usp->body->request->get;
    for (i=0; i < get->n_param_paths; i++)
    {
        // Exit if the path is not a simple path (ie absolute, wildcarded or partial) or is not currently registered into the data model
        path = get->param_paths[i];
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
        if (node == NULL)
        {
            return false;
        }

        // Exit if the path is not an object or a vendor param (only these types can be registered by a USP Service and used in a GET Request)
        if ((IsObject(node)==false) && (IsVendorParam(node)==false))
        {
            return false;
        }

        // Exit if path is owned by the Broker's internal data model, rather than a USP Service
        if (node->group_id == NON_GROUPED)
        {
            return false;
        }

        if (i==0)
        {
            // Exit if the first path is not owned by a USP Service (it could be grouped, but not owned by a USP service)
            us = FindUspServiceByGroupId(node->group_id);
            if (us == NULL)
            {
                return false;
            }
            USP_ASSERT(us->controller_mtp.is_reply_to_specified == true);   // Because the USP Service couldn't have registered a data model unless it was connected to the Broker's controller path

            // Save the group_id of the first path
            group_id = node->group_id;
        }
        else
        {
            // Exit if subsequent paths are not for the same USP Service as previous paths
            if (node->group_id != group_id)
            {
                return false;
            }
        }

        // Exit if the originator does not have permission to get all the referenced parameters
        is_permitted = CheckPassThruPermissions(node, depth, PERMIT_GET | PERMIT_GET_INST, combined_role);
        if (is_permitted == false)
        {
            return false;
        }
    }

    // Exit if unable to pass the USP message through to the USP Service
    USP_ASSERT(us != NULL);
    err = PassThruToUspService(us, usp, endpoint_id, mtpc, rec);
    if (err != USP_ERR_OK)
    {
        return false;
    }

    return true;
}

/*********************************************************************//**
**
** AttemptPassThruForSetRequest
**
** Route the Set request to the relevant USP Service, if it can be satisfied by a single USP Service
** and there are no permissions preventing the request being fulfilled
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   combined_role - roles that the originator has (inherited & assigned)
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool AttemptPassThruForSetRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec)
{
    int i, j;
    Usp__Set *set;
    dm_node_t *obj_node;
    dm_node_t *param_node;
    int group_id = INVALID;
    usp_service_t *us = NULL;
    int err;
    Usp__Set__UpdateObject *obj;
    Usp__Set__UpdateParamSetting *param;
    char path[MAX_DM_PATH];
    unsigned short permission_bitmask;

    // Exit if message was badly formed - the error will be handled by the normal handlers
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_SET) ||
        (usp->body->request->set == NULL) || (usp->body->request->set->n_update_objs==0))
    {
        return false;
    }

    // Iterate over all objects to update
    set = usp->body->request->set;
    for (i=0; i < set->n_update_objs; i++)
    {
        // Exit if the object path to update is not a simple path (ie absolute, wildcarded or partial)
        obj = set->update_objs[i];
        obj_node = DM_PRIV_GetNodeFromPath(obj->obj_path, NULL, NULL, DONT_LOG_ERRORS);
        if (obj_node == NULL)
        {
            return false;
        }

        // Exit if the object to update isn't actually an object (in which case the error should be handled by the normal handler)
        if (IsObject(obj_node)==false)
        {
            return false;
        }

        if (i==0)
        {
            // Exit if the first object to update is not owned by a USP Service (it could be grouped, but not owned by a USP service)
            us = FindUspServiceByGroupId(obj_node->group_id);
            if (us == NULL)
            {
                return false;
            }
            USP_ASSERT(us->controller_mtp.is_reply_to_specified == true);   // Because the USP Service couldn't have registered a data model unless it was connected to the Broker's controller path

            // Save the group_id of the first path
            group_id = obj_node->group_id;
        }
        else
        {
            // Exit if subsequent objects to update are not for the same USP Service as previous paths
            if (obj_node->group_id != group_id)
            {
                return false;
            }
        }

        // Iterate over all child parameters to set
        for (j=0; j < obj->n_param_settings; j++)
        {
            param = obj->param_settings[j];
            USP_SNPRINTF(path, sizeof(path), "%s.%s", obj_node->path, param->param);

            // Exit if the parameter path to update does not exist
            param_node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
            if (param_node == NULL)
            {
                return false;
            }

            // Exit if the parameter to update isn't a vendor param (USP Services only register vendor params)
            if (IsVendorParam(param_node)==false)
            {
                return false;
            }

            USP_ASSERT(param_node->group_id == group_id);  // Since this is a child parameter of the object, it must have the same group_id

            // Exit if the originator does not have permission to set this child parameter
            permission_bitmask = DM_PRIV_GetPermissions(param_node, combined_role);
            if ((permission_bitmask & PERMIT_SET) == 0)
            {
                return false;
            }
        }
    }

    // Exit if unable to pass the USP message through to the USP Service
    USP_ASSERT(us != NULL);
    err = PassThruToUspService(us, usp, endpoint_id, mtpc, rec);
    if (err != USP_ERR_OK)
    {
        return false;
    }

    return true;
}

/*********************************************************************//**
**
** AttemptPassThruForAddRequest
**
** Route the Add request to the relevant USP Service, if it can be satisfied by a single USP Service
** and there are no permissions preventing the request being fulfilled
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   combined_role - roles that the originator has (inherited & assigned)
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool AttemptPassThruForAddRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec)
{
    int i, j;
    Usp__Add *add;
    dm_node_t *obj_node;
    dm_node_t *param_node;
    int group_id = INVALID;
    usp_service_t *us = NULL;
    int err;
    Usp__Add__CreateObject *obj;
    Usp__Add__CreateParamSetting *param;
    char path[MAX_DM_PATH];
    unsigned short permission_bitmask;

    // Exit if message was badly formed - the error will be handled by the normal handlers
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_ADD) ||
        (usp->body->request->add == NULL) || (usp->body->request->add->n_create_objs==0))
    {
        return false;
    }

    // Iterate over all objects to update
    add = usp->body->request->add;
    for (i=0; i < add->n_create_objs; i++)
    {
        // Exit if the object path to add is not a simple path (ie absolute, wildcarded or partial)
        obj = add->create_objs[i];
        obj_node = DM_PRIV_GetNodeFromPath(obj->obj_path, NULL, NULL, DONT_LOG_ERRORS);
        if (obj_node == NULL)
        {
            return false;
        }

        // Exit if the object to add isn't a muli-instance object (in which case the error should be handled by the normal handler)
        if (obj_node->type != kDMNodeType_Object_MultiInstance)
        {
            return false;
        }

        // Exit if the originator does not have permission to add an instance of this object
        permission_bitmask = DM_PRIV_GetPermissions(obj_node, combined_role);
        if ((permission_bitmask & PERMIT_ADD) == 0)
        {
            return false;
        }

        // Exit if the object is owned by the internal data model (ie not owned by a USP service)
        if (obj_node->group_id == NON_GROUPED)
        {
            return false;
        }

        if (i==0)
        {
            // Exit if the first object is grouped, but not owned by a USP service
            // Subsequent objects must be for the same group as this one ie same USP Service
            us = FindUspServiceByGroupId(obj_node->group_id);
            if (us == NULL)
            {
                return false;
            }

            USP_ASSERT(us->controller_mtp.is_reply_to_specified == true);   // Because the USP Service couldn't have registered a data model unless it was connected to the Broker's controller path

            // Save the group_id of the first path
            group_id = obj_node->group_id;
        }
        else
        {
            // Exit if subsequent objects to update are not for the same USP Service as previous paths
            if (obj_node->group_id != group_id)
            {
                return false;
            }
        }

        // Iterate over all child parameters to set in this object
        for (j=0; j < obj->n_param_settings; j++)
        {
            param = obj->param_settings[j];
            USP_SNPRINTF(path, sizeof(path), "%s.%s", obj_node->path, param->param);

            // Exit if the parameter path to update does not exist
            param_node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
            if (param_node == NULL)
            {
                return false;
            }

            // Exit if the parameter to set isn't a vendor param (USP Services only register vendor params)
            if (IsVendorParam(param_node)==false)
            {
                return false;
            }

            USP_ASSERT(param_node->group_id == group_id);  // Since this is a child parameter of the object, it must have the same group_id

            // Exit if the originator does not have permission to set this child parameter
            permission_bitmask = DM_PRIV_GetPermissions(param_node, combined_role);
            if ((permission_bitmask & PERMIT_SET) == 0)
            {
                return false;
            }
        }
    }

    // Exit if unable to pass the USP message through to the USP Service
    USP_ASSERT(us != NULL);
    err = PassThruToUspService(us, usp, endpoint_id, mtpc, rec);
    if (err != USP_ERR_OK)
    {
        return false;
    }

    return true;
}

/*********************************************************************//**
**
** AttemptPassThruForDeleteRequest
**
** Route the Delete request to the relevant USP Service, if it can be satisfied by a single USP Service
** and there are no permissions preventing the request being fulfilled
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   combined_role - roles that the originator has (inherited & assigned)
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool AttemptPassThruForDeleteRequest(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, combined_role_t *combined_role, UspRecord__Record *rec)
{
    int i;
    Usp__Delete *del;
    dm_node_t *node;
    int group_id = INVALID;
    usp_service_t *us = NULL;
    char *path;
    int err;
    unsigned short permission_bitmask;

    // Exit if message was badly formed - the error will be handled by the normal handlers
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_DELETE) ||
        (usp->body->request->delete_ == NULL) || (usp->body->request->delete_->n_obj_paths==0))
    {
        return false;
    }

    // Iterate over all objects to update
    del = usp->body->request->delete_;
    for (i=0; i < del->n_obj_paths; i++)
    {
        // Exit if the object path to delete is not a simple path (ie absolute, wildcarded or partial)
        path = del->obj_paths[i];
        node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, DONT_LOG_ERRORS);
        if (node == NULL)
        {
            return false;
        }

        // Exit if the object to delete isn't a muli-instance object (in which case the error should be handled by the normal handler)
        if (node->type != kDMNodeType_Object_MultiInstance)
        {
            return false;
        }

        if (i==0)
        {
            // Exit if the first object to update is not owned by a USP Service (it could be grouped, but not owned by a USP service)
            us = FindUspServiceByGroupId(node->group_id);
            if (us == NULL)
            {
                return false;
            }
            USP_ASSERT(us->controller_mtp.is_reply_to_specified == true);   // Because the USP Service couldn't have registered a data model unless it was connected to the Broker's controller path

            // Save the group_id of the first path
            group_id = node->group_id;
        }
        else
        {
            // Exit if subsequent objects to update are not for the same USP Service as previous paths
            if (node->group_id != group_id)
            {
                return false;
            }
        }

        // Exit if the originator does not have permission to delete an instance of this object
        permission_bitmask = DM_PRIV_GetPermissions(node, combined_role);
        if ((permission_bitmask & PERMIT_DEL) == 0)
        {
            return false;
        }
    }

    // Exit if unable to pass the USP message through to the USP Service
    USP_ASSERT(us != NULL);
    err = PassThruToUspService(us, usp, endpoint_id, mtpc, rec);
    if (err != USP_ERR_OK)
    {
        return false;
    }

    return true;
}


/*********************************************************************//**
**
** AttemptPassThruForNotification
**
** Passback the received notification to the relevant USP Service/Controller
** This function determines which USP Controller (connected to the USP Broker) set the subscrption on the Broker
** and forwards the notification to it
**
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - endpoint of USP service which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  true if the message has been handled here, false if it should be handled by the normal handlers
**
**************************************************************************/
bool AttemptPassThruForNotification(Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, UspRecord__Record *rec)
{
    int err;
    Usp__Notify *notify;
    usp_service_t *us;
    subs_map_t *smap;
    int broker_instance;
    int items_converted;

    // Exit if message was badly formed - the error will be handled by the normal handlers
    if ((usp->body == NULL) || (usp->body->msg_body_case != USP__BODY__MSG_BODY_REQUEST) ||
        (usp->body->request == NULL) || (usp->body->request->req_type_case != USP__REQUEST__REQ_TYPE_NOTIFY) ||
        (usp->body->request->notify == NULL) )
    {
        return false;
    }

    // Exit if the notification is expecting a response (because we didn't ask for that) - the error will be handled by the normal handlers
    notify = usp->body->request->notify;
    if (notify->send_resp == true)
    {
        return false;
    }

    // Exit if the notification is for Operation Complete. These need to write to the Request table in the Broker, which requires a
    // USP database transaction, which cannot be performed in passthru (because a database transaction is probably already in progress
    // before calling the vendor hook that is allowing the passthru to occur whilst blocked waiting for a response from a USP Service)
    // Also we do not handle OnBoardRequests from USP Services currently - so let the normal handler flag this
    if ((notify->notification_case == USP__NOTIFY__NOTIFICATION_OPER_COMPLETE) ||
        (notify->notification_case == USP__NOTIFY__NOTIFICATION_ON_BOARD_REQ))
    {
        return false;
    }

    // Exit if the notification was for object creation/deletion and we are in the midst of processing an Add request
    // In this case, we want to hold back object creation notifications until after the Add Response has been sent
    // The reason why we also hold back object deletion notifications during processing an Add is because they could occur
    // when rolling back a failed Add with allow_partial=false
    if ((notify->notification_case == USP__NOTIFY__NOTIFICATION_OBJ_CREATION) ||
        (notify->notification_case == USP__NOTIFY__NOTIFICATION_OBJ_DELETION))
    {
        if (MSG_HANDLER_GetMsgType() == USP__HEADER__MSG_TYPE__ADD)
        {
            return false;
        }
    }

    // Exit if originator endpoint is not a USP Service (we shouldn't receive notifications from anything else) - the error will be handled by the normal handlers
    us = FindUspServiceByEndpoint(endpoint_id);
    if (us == NULL)
    {
        return false;
    }

    // Exit if the Subscription ID was not created by the Broker
    if (strstr(notify->subscription_id, broker_unique_str) == NULL)
    {
        return false;
    }

    // Exit if unable to extract the broker's subscription instance number from the subscription ID
    items_converted = sscanf(notify->subscription_id, "%d", &broker_instance);
    if (items_converted != 1)
    {
        return false;
    }

    // Exit if the subscription_id of the received notification does not match any that we are expecting
    smap = SubsMap_FindByUspServiceSubsId(&us->subs_map, notify->subscription_id, broker_instance);
    if (smap == NULL)
    {
        return false;
    }

    // Log this message, if not already done so by caller
    if (rec != NULL)
    {
        PROTO_TRACE_ProtobufMessage(&rec->base);
        PROTO_TRACE_ProtobufMessage(&usp->base);
    }
    USP_LOG_Info("Passthru NOTIFY");

    // Forward the notification back to the controller that set up the subscription on the Broker
    err = DEVICE_SUBSCRIPTION_RouteNotification(usp, broker_instance);
    if (err != USP_ERR_OK)
    {
        return false;
    }

    // NOTE: There is no need to send a NotifyResponse to the USP Service which sent this notification, because
    // this Broker code always sets NotifRetry=false on the USP Service

    // The notification was passed back successfully
    return true;
}

/*********************************************************************//**
**
** IsUspServiceNotificationMatch
**
** Determines whether the specified notification, which was received before the registration
** sequence had completed (ie subscriptions on broker and USP Service have not been synched)
** matches the specified broker subscription
**
** \param   notify - pointer to parsed protobuf notify structure of the notification
** \param   us - USP Service that sent the notification
** \param   broker_instance - expected instance number of the subscription on the USP Broker, extracted from the subscription ID of the received notification
**
** \return  true if the received notification matches the broker's subscription, false if it doesn't
**
**************************************************************************/
bool IsUspServiceNotificationMatch(Usp__Notify *notify, usp_service_t *us, int broker_instance)
{
    char buf[MAX_DM_PATH];
    char *path;
    bool is_match;
    subs_notify_t notify_type;
    dm_node_t *node;

    // Extract the full path to the data model element
    switch(notify->notification_case)
    {
        case USP__NOTIFY__NOTIFICATION_EVENT:
            USP_SNPRINTF(buf, sizeof(buf), "%s%s", notify->event->obj_path, notify->event->event_name);
            path = buf;
            notify_type = kSubNotifyType_Event;
            break;

        case USP__NOTIFY__NOTIFICATION_VALUE_CHANGE:
            path = notify->value_change->param_path;
            notify_type = kSubNotifyType_ValueChange;
            break;

        case USP__NOTIFY__NOTIFICATION_OBJ_CREATION:
            path = notify->obj_creation->obj_path;
            notify_type = kSubNotifyType_ObjectCreation;
            break;

        case USP__NOTIFY__NOTIFICATION_OBJ_DELETION:
            path = notify->obj_deletion->obj_path;
            notify_type = kSubNotifyType_ObjectDeletion;
            break;

        default:
            return false;
            break;
    }

    // Exit if the path is not yet registered into the data model. This could occur if the GSDM response has not yet been received in the registration sequence
    // NOTE: The path has to be in the data model, otherwise we will be unable to retrieve permissions for forwarding the notification
    //       This limitation means that most notifications cannot be forwarded until after the GSDM response has been retrieved
    node = DM_PRIV_GetNodeFromPath(path, NULL, NULL, 0);
    if (node == NULL)
    {
        USP_ERR_SetMessage("%s: Notification path (%s) does not exist in the data model", __FUNCTION__, path);
        return false;
    }

    // Exit if the notification path is not owned by the USP Service
    if (node->group_id != us->group_id)
    {
        USP_ERR_SetMessage("%s: Notification path (%s) is not owned by USP Service", __FUNCTION__, path);
        return false;
    }

    // Exit if we do not have an enabled matching subscription for this notification
    is_match = DEVICE_SUBSCRIPTION_IsMatch(broker_instance, notify_type, path);
    if (is_match == false)
    {
        USP_ERR_SetMessage("%s: Notification contains unexpected subscription Id (%s)", __FUNCTION__, notify->subscription_id);
        return false;
    }

    return true;
}

/*********************************************************************//**
**
** CheckPassThruPermissions
**
** Determines whether the originator has permission to access the specified node and child nodes
** NOTE: This function is called recursively
**
** \param   node - pointer to node in the data model to check the permissions of
** \param   depth - the number of hierarchical levels to traverse in the data model when checking permissions
** \param   required_permissions - bitmask of permissions that must be allowed
** \param   combined_role - roles that the originator has (inherited & assigned)
**
** \return  true if the originator has permission, false otherwise
**
**************************************************************************/
bool CheckPassThruPermissions(dm_node_t *node, int depth, unsigned short required_permissions, combined_role_t *combined_role)
{
    bool is_permitted;
    unsigned short permission_bitmask;
    dm_node_t *child;

    // Exit if the originator does not have permission
    permission_bitmask = DM_PRIV_GetPermissions(node, combined_role);
    if ((permission_bitmask & required_permissions) != required_permissions)
    {
        return false;
    }

    // Exit if there are no more hierarchical levels to traverse in the data model when checking permissions
    if (depth <= 1)
    {
        return true;
    }

    // Recursively check the permissions of all child nodes
    child = (dm_node_t *) node->child_nodes.head;
    while (child != NULL)
    {
        is_permitted = CheckPassThruPermissions(child, depth-1, required_permissions, combined_role);
        if (is_permitted == false)
        {
            return false;
        }

        child = (dm_node_t *) child->link.next;
    }

    // If the code gets here, then all child nodes passed the permission check
    return true;
}

/*********************************************************************//**
**
** PassThruToUspService
**
** Sends the USP request message to the specified USP Service, and saves the msg_id so
** that it can route the response message back to the originator
**
** \param   us - pointer to USP Service to send the message to
** \param   usp - pointer to parsed USP message structure. This is always freed by the caller (not this function)
** \param   endpoint_id - originator endpoint which sent this message
** \param   mtpc - details of where response to this USP message should be sent
** \param   rec - pointer to parsed USP record structure to log, or NULL if this message has already been logged by the caller
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PassThruToUspService(usp_service_t *us, Usp__Msg *usp, char *endpoint_id, mtp_conn_t *mtpc, UspRecord__Record *rec)
{
    int err;
    char broker_msg_id[MAX_MSG_ID_LEN];
    char *original_msg_id;

    // Log this message, if not already done so by caller
    if (rec != NULL)
    {
        PROTO_TRACE_ProtobufMessage(&rec->base);
        PROTO_TRACE_ProtobufMessage(&usp->base);
    }

    // Remap the messageID from that in the original message to avoid duplicate message IDs from different originators
    CalcBrokerMessageId(broker_msg_id, sizeof(broker_msg_id));
    original_msg_id = usp->header->msg_id;
    USP_LOG_Info("Passthru %s to '%s'", MSG_HANDLER_UspMsgTypeToString(usp->header->msg_type), us->endpoint_id);
    usp->header->msg_id = USP_STRDUP(broker_msg_id);

    // Exit if unable to send the message to the USP service
    err = MSG_HANDLER_QueueMessage(us->endpoint_id, usp, &us->controller_mtp);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Save the details of where to route the response back to
    MsgMap_Add(&us->msg_map, original_msg_id, broker_msg_id, endpoint_id, mtpc);
    err = USP_ERR_OK;

exit:
    USP_FREE(original_msg_id);
    return err;
}

/*********************************************************************//**
**
** CalcBrokerMessageId
**
** Creates a unique message id for messages sent from this USP Broker to a USP Service
**
** \param   msg_id - pointer to buffer in which to write the message id
** \param   len - length of buffer
**
** \return  None
**
**************************************************************************/
void CalcBrokerMessageId(char *msg_id, int len)
{
    static unsigned count = 0;

    count++;               // Pre-increment before forming message, because we want to count from 1

    // Form a message id string which is unique.
    {
        // In production, the string must be unique because we don't want the Broker receiving stale responses
        // and treating them as fresh (in the case of the Broker crashing and restarting)
        USP_SNPRINTF(msg_id, len, "%s-%d-%u", broker_unique_str, count, (unsigned) time(NULL) );
    }
}

/*********************************************************************//**
**
** FindUspServiceByEndpoint
**
** Finds the specified endpoint in the usp_services[] array
**
** \param   endpoint_id - endpoint of USP service to match
**
** \return  pointer to matching USP service, or NULL if no match found
**
**************************************************************************/
usp_service_t *FindUspServiceByEndpoint(char *endpoint_id)
{
    int i;
    usp_service_t *us;

    // Iterate over all USP services finding the matching endpoint
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if ((us->instance != INVALID) && (strcmp(us->endpoint_id, endpoint_id)==0))
        {
            return us;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindUspServiceByInstance
**
** Finds the specified instance in the usp_services[] array
**
** \param   instance - instance number to match
**
** \return  pointer to matching USP service, or NULL if no match found
**
**************************************************************************/
usp_service_t *FindUspServiceByInstance(int instance)
{
    int i;
    usp_service_t *us;

    // Iterate over all USP services finding the matching endpoint
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if (us->instance == instance)
        {
            return us;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindUspServiceByGroupId
**
** Finds the specified instance in the usp_services[] array
**
** \param   group_id - group_id to match
**
** \return  pointer to matching USP service, or NULL if no match found
**
**************************************************************************/
usp_service_t *FindUspServiceByGroupId(int group_id)
{
    int i;
    usp_service_t *us;

    // Iterate over all USP services finding the matching endpoint
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if ((us->instance != INVALID) && (us->group_id == group_id))
        {
            return us;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** FindUnusedUspService
**
** Finds an unused entry in the usp_services[] array
**
** \param   None
**
** \return  pointer to unused entry, or NULL if all entries have been allocated
**
**************************************************************************/
usp_service_t *FindUnusedUspService(void)
{
    int i;
    usp_service_t *us;

    // Iterate over all USP services finding a free entry
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if (us->instance == INVALID)
        {
            return us;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** CalcNextUspServiceInstanceNumber
**
** Finds the next instance number to allocate to a newly connected USP service
**
** \param   None
**
** \return  instance number in Device.USPServices.USPService.{i}
**
**************************************************************************/
int CalcNextUspServiceInstanceNumber(void)
{
    int i;
    int max_instance = 0;
    usp_service_t *us;

    // Iterate over all USP services finding the highest instance number
    for (i=0; i<MAX_USP_SERVICES; i++)
    {
        us = &usp_services[i];
        if ((us->instance != INVALID) && (us->instance > max_instance))
        {
            max_instance = us->instance;
        }
    }

    return max_instance+1;
}

/*********************************************************************//**
**
** SubsMap_Init
**
** Initialises a subscription mapping table
**
** \param   sm - pointer to subscription mapping table
**
** \return  None
**
**************************************************************************/
void SubsMap_Init(double_linked_list_t *sm)
{
    DLLIST_Init(sm);
}

/*********************************************************************//**
**
** SubsMap_Destroy
**
** Frees all dynamically allocated memory associated with a subscription mapping table
**
** \param   sm - pointer to subscription mapping table
**
** \return  None
**
**************************************************************************/
void SubsMap_Destroy(double_linked_list_t *sm)
{
    while (sm->head != NULL)
    {
        SubsMap_Remove(sm, (subs_map_t *)sm->head);
    }
}

/*********************************************************************//**
**
** SubsMap_Add
**
** Adds an entry into the specified subscription mapping table
**
** \param   sm - pointer to subscription mapping table
** \param   service_instance - Instance of the subscription in the USP Service's Device.LocalAgent.Subscription.{i}
** \param   path - data model path which was subscribed to in the vendor layer
** \param   subscription_id - Id of the subscription in the USP Service's subscription table
** \paam    broker_instance - Instance of the subscription in the USP Broker's Device.LocalAgent.Subscription.{i}
**
** \return  None
**
**************************************************************************/
void SubsMap_Add(double_linked_list_t *sm, int service_instance, char *path, char *subscription_id, int broker_instance)
{
    subs_map_t *smap;

    smap = USP_MALLOC(sizeof(subs_map_t));
    smap->service_instance = service_instance;
    smap->path = USP_STRDUP(path);
    smap->subscription_id = USP_STRDUP(subscription_id);
    smap->broker_instance = broker_instance;

    DLLIST_LinkToTail(sm, smap);
}

/*********************************************************************//**
**
** SubsMap_Remove
**
** Removes the specified entry from the vector
**
** \param   sm - pointer to subscription mapping table
** \param   smap - pointer to entry in subscription mapping table to remove
**
** \return  None
**
**************************************************************************/
void SubsMap_Remove(double_linked_list_t *sm, subs_map_t *smap)
{
    // Remove the entry from the list
    DLLIST_Unlink(sm, smap);

    // Free all memory associated with this entry
    USP_FREE(smap->path);
    USP_FREE(smap->subscription_id);
    USP_FREE(smap);
}

/*********************************************************************//**
**
** SubsMap_FindByUspServiceSubsId
**
** Finds the entry in the specified subscription mapping table that matches the specified subscription_id of the USP Service
**
** \param   sm - pointer to subscription mapping table
** \param   subscription_id - Id of the subscription in the USP Service's subscription table
** \param   broker_instance - instance number of the expected subscription on the Broker (extracted from the USP Service's subscription_id)
**
** \return  Pointer to entry in subscription map table, or NULL if no match was found
**
**************************************************************************/
subs_map_t *SubsMap_FindByUspServiceSubsId(double_linked_list_t *sm, char *subscription_id, int broker_instance)
{
    subs_map_t *smap;

    smap = (subs_map_t *) sm->head;
    while (smap != NULL)
    {
        if ((smap->broker_instance == broker_instance) && (strcmp(smap->subscription_id, subscription_id)==0))
        {
            return smap;
        }

        smap = (subs_map_t *) smap->link.next;
    }

    return NULL;
}

/*********************************************************************//**
**
** SubsMap_FindByBrokerInstanceAndPath
**
** Finds the entry in the specified subscription mapping table that matches the specified subscription path
** for the specified Broker subscription table instance number
**
** \param   sm - pointer to subscription mapping table
** \param   broker_instance - instance number in the Broker's subscription table to match
** \param   path - subscription path to match
**
** \return  Pointer to entry in subscription map table, or NULL if no match was found
**
**************************************************************************/
subs_map_t *SubsMap_FindByBrokerInstanceAndPath(double_linked_list_t *sm, int broker_instance, char *path)
{
    subs_map_t *smap;

    smap = (subs_map_t *) sm->head;
    while (smap != NULL)
    {
        if ((smap->broker_instance == broker_instance) && (strcmp(smap->path, path)==0))
        {
            return smap;
        }

        smap = (subs_map_t *) smap->link.next;
    }

    return NULL;
}

/*********************************************************************//**
**
** SubsMap_FindByPath
**
** Finds the entry in the specified subscription mapping table with a
** path specification that matches the specified absolute path
** NOTE: The path specification in the subscription may be an absolute path, partial path, or wildcarded path
**
** \param   sm - pointer to subscription mapping table
** \param   path - absolute path to match
**
** \return  Pointer to entry in subscription map table, or NULL if no match was found
**
**************************************************************************/
subs_map_t *SubsMap_FindByPath(double_linked_list_t *sm, char *path)
{
    subs_map_t *smap;

    smap = (subs_map_t *) sm->head;
    while (smap != NULL)
    {
        if (TEXT_UTILS_IsPathMatch(path, smap->path))
        {
            return smap;
        }

        smap = (subs_map_t *) smap->link.next;
    }

    return NULL;
}

/*********************************************************************//**
**
** ReqMap_Init
**
** Initialises a request mapping table
**
** \param   rm - pointer to request mapping table
**
** \return  None
**
**************************************************************************/
void ReqMap_Init(double_linked_list_t *rm)
{
    DLLIST_Init(rm);
}

/*********************************************************************//**
**
** ReqMap_Destroy
**
** Frees all dynamically allocated memory associated with a request mapping table
**
** \param   rm - pointer to request mapping table
**
** \return  None
**
**************************************************************************/
void ReqMap_Destroy(double_linked_list_t *rm)
{
    while (rm->head != NULL)
    {
        ReqMap_Remove(rm, (req_map_t *)rm->head);
    }
}

/*********************************************************************//**
**
** ReqMap_Add
**
** Adds an entry into the specified request mapping table
**
** \param   rm - pointer to request mapping table
** \param   request_instance - Instance of the request in the USP Broker's request table
** \param   path - data model path of the USP command being invoked
** \param   command_key - command_key for the USP command being invoked
**
** \return  pointer to entry created
**
**************************************************************************/
req_map_t *ReqMap_Add(double_linked_list_t *rm, int request_instance, char *path, char *command_key)
{
    req_map_t *rmap;

    rmap = USP_MALLOC(sizeof(req_map_t));
    rmap->request_instance = request_instance;
    rmap->path = USP_STRDUP(path);
    rmap->command_key = USP_STRDUP(command_key);

    DLLIST_LinkToTail(rm, rmap);

    return rmap;
}

/*********************************************************************//**
**
** ReqMap_Remove
**
** Removes the specified entry from the vector
**
** \param   rm - pointer to request mapping table
** \param   rmap - pointer to entry in request mapping table to remove
**
** \return  None
**
**************************************************************************/
void ReqMap_Remove(double_linked_list_t *rm, req_map_t *rmap)
{
    // Remove the entry from the list
    DLLIST_Unlink(rm, rmap);

    // Free all memory associated with this entry
    USP_FREE(rmap->path);
    USP_FREE(rmap->command_key);
    USP_FREE(rmap);
}

/*********************************************************************//**
**
** ReqMap_Find
**
** Returns the entry in the request mapping table which matches the specified path and command_key
**
** \param   rm - pointer to request mapping table
** \param   path - data model path of the USP Command under consideration
** \param   command_key - command key for the operate request
**
** \return  Pointer to entry in request map table, or NULL if no match was found
**
**************************************************************************/
req_map_t *ReqMap_Find(double_linked_list_t *rm, char *path, char *command_key)
{
    req_map_t *rmap;

    rmap = (req_map_t *) rm->head;
    while (rmap != NULL)
    {
        if ((strcmp(rmap->path, path)==0) && (strcmp(rmap->command_key, command_key)==0))
        {
            return rmap;
        }

        rmap = (req_map_t *) rmap->link.next;
    }

    return NULL;
}

/*********************************************************************//**
**
** MsgMap_Init
**
** Initialises a message mapping table
**
** \param   mm - pointer to message mapping table
**
** \return  None
**
**************************************************************************/
void MsgMap_Init(double_linked_list_t *mm)
{
    DLLIST_Init(mm);
}

/*********************************************************************//**
**
** MsgMap_Destroy
**
** Frees all dynamically allocated memory associated with a message mapping table
**
** \param   mm - pointer to message mapping table
**
** \return  None
**
**************************************************************************/
void MsgMap_Destroy(double_linked_list_t *mm)
{
    while (mm->head != NULL)
    {
        MsgMap_Remove(mm, (msg_map_t *)mm->head);
    }
}

/*********************************************************************//**
**
** MsgMap_Add
**
** Adds an entry into the specified message mapping table
**
** \param   mm - pointer to message mapping table
** \param   original_msg_id - MessageID of the original request message
** \param   broker_msg_id - Remapped MessageID used by the Broker, when routing the request to the USP Service
** \param   endpoint_id - EndpointID for originator of the message
** \param   mtpc - pointer to structure containing the MTP to send the response (from the USP Service) back on
**
** \return  pointer to entry created
**
**************************************************************************/
msg_map_t *MsgMap_Add(double_linked_list_t *mm, char *original_msg_id, char *broker_msg_id, char *endpoint_id, mtp_conn_t *mtpc)
{
    msg_map_t *map;

    map = USP_MALLOC(sizeof(msg_map_t));

    map->original_msg_id = USP_STRDUP(original_msg_id);
    map->broker_msg_id = USP_STRDUP(broker_msg_id);
    map->originator = USP_STRDUP(endpoint_id);
    DM_EXEC_CopyMTPConnection(&map->mtp_conn, mtpc);

    DLLIST_LinkToTail(mm, map);

    return map;
}

/*********************************************************************//**
**
** MsgMap_Remove
**
** Removes the specified entry from the vector
**
** \param   mm - pointer to message mapping table
** \param   map - pointer to entry in message mapping table to remove
**
** \return  None
**
**************************************************************************/
void MsgMap_Remove(double_linked_list_t *mm, msg_map_t *map)
{
    // Remove the entry from the list
    DLLIST_Unlink(mm, map);

    // Free all memory associated with this entry
    USP_FREE(map->original_msg_id);
    USP_FREE(map->broker_msg_id);
    USP_FREE(map->originator);
    DM_EXEC_FreeMTPConnection(&map->mtp_conn);
    USP_FREE(map);
}

/*********************************************************************//**
**
** MsgMap_Find
**
** Returns the entry in the message mapping table which matches the received messageID
**
** \param   mm - pointer to message mapping table
** \param   msg_id - MessageID of USP response message received back from the USP Service
**
** \return  Pointer to entry in message map table, or NULL if no match was found
**
**************************************************************************/
msg_map_t *MsgMap_Find(double_linked_list_t *mm, char *msg_id)
{
    msg_map_t *map;

    map = (msg_map_t *) mm->head;
    while (map != NULL)
    {
        if (strcmp(map->broker_msg_id, msg_id)==0)
        {
            return map;
        }

        map = (msg_map_t *) map->link.next;
    }

    return NULL;
}

#endif // REMOVE_USP_BROKER
