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
 * \file device_subscription.c
 *
 * Implements the Device.LocalAgent.Subscription data model object
 *
 */
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_defs.h"
#include "device.h"
#include "dm_trans.h"
#include "usp_api.h"
#include "dm_access.h"
#include "iso8601.h"
#include "subs_vector.h"
#include "path_resolver.h"
#include "msg_handler.h"
#include "database.h"
#include "sync_timer.h"
#include "subs_retry.h"
#include "text_utils.h"
#include "expr_vector.h"
#include "json.h"
#include "group_get_vector.h"
#include "dm_inst_vector.h"

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include "e2e_context.h"
#endif

#ifndef REMOVE_USP_BROKER
#include "usp_broker.h"
#endif

//------------------------------------------------------------------------------
// Uncomment the following define to turn on extra debug which helps with debugging issues related to object
// creation/deletion notifications not being sent
//#define DEBUG_OBJECT_NOTIFICATIONS

#ifdef DEBUG_OBJECT_NOTIFICATIONS
#define USP_LOG_DebugNotifications(...)  USP_LOG_Info(__VA_ARGS__)
#else
#define USP_LOG_DebugNotifications(...)
#endif

//-------------------------------------------------------------------------
// The prefix to use when forming the auto-assigned value of an ID parameter
#define DEFAULT_ID_PREFIX "cpe-"

//------------------------------------------------------------------------------
// List of notification types that USP Agent currently supports
const enum_entry_t notify_types[kSubNotifyType_Max] =
{
    { kSubNotifyType_None,                  "" },       // This is the default value for notification type
    { kSubNotifyType_ValueChange,           "ValueChange" },
    { kSubNotifyType_ObjectCreation,        "ObjectCreation"},
    { kSubNotifyType_ObjectDeletion,        "ObjectDeletion"},
    { kSubNotifyType_OperationComplete,     "OperationComplete"},
    { kSubNotifyType_Event,                 "Event"},
};

// Convenience macro to convert from a type enumeration to the string form
#define NOTIFY_TYPE_STR(x)    TEXT_UTILS_EnumToString(x, notify_types, NUM_ELEM(notify_types))
//------------------------------------------------------------------------------
// Vector containing all subscriptions
subs_vector_t subscriptions;

//------------------------------------------------------------------------------
// Ordered vector of objects which have been recently added/deleted from the data model
// (ie since the last time DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions was called)
// Contains objects which need to be processed against the subscriptions
// NOTE: We use a single ordered vector to store both addition and deletion so that the notification messages
// are generated in the right order, should an object be in this list more than once (say added then deleted)
typedef struct
{
    char *obj_path;
    subs_notify_t notify_type;    // Type of life event ie object creation or object deletion
} obj_life_event_t;

typedef struct
{
    obj_life_event_t *vector;
    int num_entries;              // Total number of entries in the vector (creation + deletion entries)
    int num_creation;             // Number of creation events in the vector
    int num_deletion;             // Number of deletion events in the vector
} obj_life_event_vector_t;

obj_life_event_vector_t object_life_events;

//------------------------------------------------------------------------------
// Booleans to prevent excessive resolving of subscrioption path expressions. The flags ensure that path resolution happens at most once per DM_EXEC processing cycle
static bool object_creation_paths_resolved = false;
static bool object_deletion_paths_resolved = false;

//------------------------------------------------------------------------------
// Location of the subscriptions object within the data model
#define DEVICE_SUBS_ROOT "Device.LocalAgent.Subscription"
static const char device_subs_root[] = DEVICE_SUBS_ROOT;

static const char *periodic_event_str = "Device.LocalAgent.Periodic!";

#ifndef REMOVE_DEVICE_BOOT_EVENT
//------------------------------------------------------------------------------------
// Location of the boot event within the data model
#define DEVICE_BOOT_EVENT "Device.Boot!"
static const char device_boot_event[] = DEVICE_BOOT_EVENT;

// Array of arguments sent in Boot! event
static char *boot_event_args[] =
{
    "CommandKey",
    "Cause",
    "FirmwareUpdated",
    "ParameterMap",
};
#endif

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int ProcessSubscriptionAdded(int instance);
int CalcExpiryTime(int instance, time_t *expiry_time);
int Validate_SubsRefList(dm_req_t *req, char *value);
int NotifyChange_SubsEnable(dm_req_t *req, char *value);
int NotifyChange_NotifyType(dm_req_t *req, char *value);
int NotifyChange_SubsID(dm_req_t *req, char *value);
int NotifyChange_SubsRefList(dm_req_t *req, char *value);
int NotifyChange_Persistent(dm_req_t *req, char *value);
int NotifyChange_SubsTimeToLive(dm_req_t *req, char *value);
int NotifyChange_NotifRetry(dm_req_t *req, char *value);
int NotifyChange_NotifExpiration(dm_req_t *req, char *value);
int AutoPopulate_SubsID(dm_req_t *req, char *buf, int len);
int Validate_SubsID(dm_req_t *req, char *value);
int Validate_SubsNotifType(dm_req_t *req, char *value);
int Validate_SubsRefList_Inner(subs_notify_t notify_type, char *ref_list);
int Validate_BootParamName(dm_req_t *req, char *value);
int GetAuto_Recipient(dm_req_t *req, char *buf, int len);
int GetAuto_CreationDate(dm_req_t *req, char *buf, int len);
int NotifySubsAdded(dm_req_t *req);
int NotifySubsDeleted(dm_req_t *req);
void DeleteExpiredSubscriptions(void);
int DeleteNonPersistentSubscriptions(void);
void ProcessObjectLifeEventSubscription(subs_t *sub);
void ProcessAllValueChangeSubscriptions(void);
void ProcessValueChangeSubscription(subs_t *sub);
void SendValueChangeNotify(subs_t *sub, char *path, char *value);
void ResolveAllPathExpressions(int subs_instance, str_vector_t *path_expressions, int_vector_t *handler_group_ids, str_vector_t *resolved_paths, int_vector_t *group_ids, resolve_op_t op, int cont_instance);
void GetAllPathExpressionParameterValues(subs_t *sub, str_vector_t *path_expressions, int_vector_t *handler_group_ids, kv_vector_t *param_values);
char *SerializeToJSONObject(kv_vector_t *param_values);
void SendOperationCompleteNotify(subs_t *sub, char *command, char *command_key, int err_code, char *err_msg, kv_vector_t *output_args);
void SendNotify(Usp__Msg *req, subs_t *sub, char *path);
bool DoesSubscriptionSendNotification(subs_t *sub, char *event_name);
bool DoesSubscriptionMatchEvent(subs_t *subs, char *event_name);
bool HasControllerGotNotificationPermission(int cont_instance, char *path, unsigned short mask);
void RefreshInstancesForObjLifetimeSubscriptions(void);
void StartSubscription(subs_t *sub);
subs_t *FindSubsByInstance(int instance);
char *ExtractNotificationEventArg(Usp__Notify__Event *event, char *arg_name);

#ifndef REMOVE_USP_BROKER
void StartSubscriptionInVendorLayer(subs_t *sub, int group_id);
void StartVendorLayerDeviceDotSubs(subs_t *sub, int group_id);
void StartVendorLayerDeviceDotSubsForGroup(subs_t *sub, int group_id);
void StopSubscriptionInVendorLayer(subs_t *sub);
#endif
#ifndef REMOVE_DEVICE_BOOT_EVENT
void SeedLastValueChangeValues(void);
void ProcessAllBootSubscriptions(void);
#endif
#if !defined(REMOVE_DEVICE_BOOT_EVENT) || !defined(REMOVE_USP_BROKER)
void SendBootNotify(subs_t *sub, char *command_key, char *reboot_cause, char *firmware_updated);
#endif

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SUBSCRIPTION_Init(void)
{
    int err = USP_ERR_OK;

    // Register parameters implemented by Subscription table
    // NOTE: Recipient is registered before ID, as we want it to be auto assigned before ID,
    // in order that an auto-assigned ID can be validated as unique per recipient
    err |= USP_REGISTER_Object(DEVICE_SUBS_ROOT ".{i}", NULL, NULL, NotifySubsAdded,
                                                        NULL, NULL, NotifySubsDeleted);
    err |= USP_REGISTER_Param_NumEntries(DEVICE_SUBS_ROOT "NumberOfEntries", "Device.LocalAgent.Subscription.{i}");
    err |= USP_REGISTER_DBParam_Alias(DEVICE_SUBS_ROOT ".{i}.Alias", NULL);

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.Enable", "false", NULL, NotifyChange_SubsEnable, DM_BOOL);

    err |= USP_REGISTER_DBParam_ReadOnlyAuto(DEVICE_SUBS_ROOT ".{i}.Recipient", GetAuto_Recipient, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWriteAuto(DEVICE_SUBS_ROOT ".{i}.ID",  AutoPopulate_SubsID, Validate_SubsID, NotifyChange_SubsID, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadOnlyAuto(DEVICE_SUBS_ROOT ".{i}.CreationDate", GetAuto_CreationDate, DM_DATETIME);

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.NotifType", "", Validate_SubsNotifType, NotifyChange_NotifyType, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.ReferenceList", "", Validate_SubsRefList, NotifyChange_SubsRefList, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.Persistent", "false", NULL, NotifyChange_Persistent, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.TimeToLive", "0", NULL, NotifyChange_SubsTimeToLive, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.NotifRetry", "false", NULL, NotifyChange_NotifRetry, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_SUBS_ROOT ".{i}.NotifExpiration", "0", NULL, NotifyChange_NotifExpiration, DM_UINT);

    // Register unique keys for Subscription table
    char *unique_keys[] = { "ID", "Recipient" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_SUBS_ROOT ".{i}", unique_keys, NUM_ELEM(unique_keys));

    // Register parameters implemented by Boot Parameters table
    err |= USP_REGISTER_Object("Device.LocalAgent.Controller.{i}.BootParameter.{i}", NULL, NULL, NULL,
                                                                                     NULL, NULL, NULL);
    err |= USP_REGISTER_DBParam_Alias("Device.LocalAgent.Controller.{i}.BootParameter.{i}.Alias", NULL);

    err |= USP_REGISTER_DBParam_ReadWrite("Device.LocalAgent.Controller.{i}.BootParameter.{i}.Enable", "false", NULL, NULL, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.LocalAgent.Controller.{i}.BootParameter.{i}.ParameterName", "", Validate_BootParamName, NULL, DM_STRING);
    err |= USP_REGISTER_Param_NumEntries("Device.LocalAgent.Controller.{i}.BootParameterNumberOfEntries", "Device.LocalAgent.Controller.{i}.BootParameter.{i}");

    char *unique_keys1[] = { "ParameterName" };
    err |= USP_REGISTER_Object_UniqueKey("Device.LocalAgent.Controller.{i}.BootParameter.{i}", unique_keys1, NUM_ELEM(unique_keys1));

#ifndef REMOVE_DEVICE_BOOT_EVENT
    // Register Boot! Event
    err |= USP_REGISTER_Event((char *)device_boot_event);
    err |= USP_REGISTER_EventArguments((char *)device_boot_event, boot_event_args, NUM_ELEM(boot_event_args));
#endif

    if (err != USP_ERR_OK)
    {
        return err;
    }


    SUBS_VECTOR_Init(&subscriptions);
    SUBS_RETRY_Init();

    // Initialise ordered vector of object additions/deletions which need to be processed against the subscriptions
    object_life_events.vector = NULL;
    object_life_events.num_entries = 0;
    object_life_events.num_creation = 0;
    object_life_events.num_deletion = 0;

    // Create a timer which will be used to periodically poll for value change
    // NOTE: We create it here so that it is included in the base memory (before USP_MEM_StartCollection is called)
    SYNC_TIMER_Add(DEVICE_SUBSCRIPTION_Update, 0, END_OF_TIME);

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_Start
**
** Initialises the subscriptions vector with the values of all subscriptions from the DB
** and activates processing associated with those subscriptions
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SUBSCRIPTION_Start(void)
{
    int i;
    int err;
    int instance;
    int_vector_t iv;
    char path[MAX_DM_PATH];

    // Exit if unable to get the object instance numbers present in the subscription table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(DEVICE_SUBS_ROOT, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all subscriptions in the subscription table to the subscriptions vector
    // NOTE: This also seeds the initial values for all value change subscriptions
    for (i=0; i < iv.num_entries; i++)
    {
        instance = iv.vector[i];
        err = ProcessSubscriptionAdded(instance);
        if (err != USP_ERR_OK)
        {
            // If failed to add, then delete it
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_subs_root, instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

#ifndef REMOVE_DEVICE_BOOT_EVENT
    // Override the initial value for SoftwareVersion with the value before the current boot cycle
    SeedLastValueChangeValues();
#endif

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_Stop
**
** Frees up all memory associated with this module
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_Stop(void)
{
    SUBS_RETRY_Stop();
    SUBS_VECTOR_Destroy(&subscriptions);
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_Update
**
** Periodically called to update all subscriptions
** The first time this function is called, it registers a sync timer to periodically poll for value change
**
** \param   id - (unused) identifier of the sync timer which caused this callback
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_Update(int id)
{
    static bool boot_subs_processed = false;
    time_t cur_time;
    int poll_period;

    // Delete all subscriptions which have expired
    DeleteExpiredSubscriptions();

    // Process Boot subscriptions only once after power up
    cur_time = time(NULL);
    if (boot_subs_processed == false)
    {
#ifndef REMOVE_DEVICE_BOOT_EVENT
        ProcessAllBootSubscriptions();
#endif
        boot_subs_processed = true;

        // Also at bootup, delete all non-persistent subscriptions
        // NOTE: We do this after sending the Boot! event, because deletion of any subscriptions might trigger an
        // ObjectDeletion notification, and we'd like the notification to be sent after the Boot! event
        DeleteNonPersistentSubscriptions();
    }

    // Poll all value change subscriptions for change
    ProcessAllValueChangeSubscriptions();

    // Ensure that objects that require their instances to be refreshed by polling are refreshed
    // (This may result in object creation/deletion events when DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions is called)
    USP_LOG_DebugNotifications("DEVICE_SUBSCRIPTION_Update: enter");
    RefreshInstancesForObjLifetimeSubscriptions();
    USP_LOG_DebugNotifications("DEVICE_SUBSCRIPTION_Update: exit");

    // Determine the period for value change polling
    poll_period = VALUE_CHANGE_POLL_PERIOD;


    // Restart the timer to cause this function to be called periodically
    SYNC_TIMER_Reload(DEVICE_SUBSCRIPTION_Update, 0, cur_time + poll_period);
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_ProcessAllOperationCompleteSubscriptions
**
** Called to send back notification events for an async operation that has completed
**
** \param   command - path to operation in the data model
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   err_code - error code of the operation (USP_ERR_OK indicates success)
** \param   err_msg - error message if the operation failed
** \param   output_args - results of the completed operation (if successful). NULL indicates no output arguments.
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_ProcessAllOperationCompleteSubscriptions(char *command, char *command_key, int err_code, char *err_msg, kv_vector_t *output_args)
{
    int i;
    subs_t *sub;

    // Iterate over all enabled subscriptions, processing each operation complete subscription that matches
    // (there may be more than one subscriber)
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_OperationComplete))
        {
            // Send the event, if it matches this subscription
            if (DoesSubscriptionSendNotification(sub, command))
            {
                SendOperationCompleteNotify(sub, command, command_key, err_code, err_msg, output_args);
            }
        }
    }
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_ProcessAllEventCompleteSubscriptions
**
** Handles an event completing, sending it out to all subscribers
**
** \param   event_name - name of the event
** \param   output_args - arguments associated with of the event. NULL indicates no output arguments.
**
** \return  None - This code must handle any errors
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_ProcessAllEventCompleteSubscriptions(char *event_name, kv_vector_t *output_args)
{
    int i;
    subs_t *sub;
    Usp__Msg *req;

#ifdef VALIDATE_OUTPUT_ARG_NAMES
    if (output_args != NULL)
    {
        // Validate the names of the event arguments
        dm_node_t *node;
        dm_event_info_t *info;
        int err;

        node = DM_PRIV_GetNodeFromPath(event_name, NULL, NULL, 0);
        USP_ASSERT(node != NULL);

        info = &node->registered.event_info;
        err = KV_VECTOR_ValidateArguments(output_args, &info->event_args, NO_FLAGS);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Warning("%s: Output argument names do not match those registered (%s). Please check code.", __FUNCTION__, event_name);
        }
    }
#endif

    // Iterate over all enabled subscriptions, processing each event complete subscription that matches
    // (there may be more than one subscriber)
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_Event))
        {
            // Send the event, if it matches this subscription
            if (DoesSubscriptionSendNotification(sub, event_name))
            {
                // Create the notify message
                req = MSG_HANDLER_CreateNotifyReq_Event(event_name, output_args, sub->subscription_id, sub->notification_retry);

                // Send the Notify Request
                SendNotify(req, sub, event_name);
                usp__msg__free_unpacked(req, pbuf_allocator);
            }
        }
    }
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_ResolveObjectCreationPaths
**
** Resolves (and caches) the paths of all subscriptions for ObjectAdded
** This needs to be done AFTER the objects have been added to  the data model.
** If it was done before the object had been added, then the object would not exist
** in the resolved path, and hence would not match any of the resolved paths,
** and so the notify would not be sent
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_ResolveObjectCreationPaths(void)
{
    int i;
    subs_t *sub;

    // Exit if the object creation paths have already been resolved for the current USP message
    if (object_creation_paths_resolved == true)
    {
        return;
    }

    // Iterate over all enabled subscriptions
    for (i=0; i < subscriptions.num_entries; i++)
    {
        // See if this subscription is active, and is an object creation notification
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_ObjectCreation))
        {
            // Create a list of all objects which are referenced by this subscription
            // NOTE: We use kResolveOp_SubsAdd because this is the op that is used when validating the ReferenceList parameter of the Subscription table
            ResolveAllPathExpressions(sub->instance, &sub->path_expressions, &sub->handler_group_ids, &sub->cur_watch_objs, NULL, kResolveOp_SubsAdd, sub->cont_instance);
        }
    }

    object_creation_paths_resolved = true;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths
**
** Resolves (and caches) the paths of all subscriptions for ObjectDeletion
** This needs to be done BEFORE the objects are deleted from the data model.
** If it was done after the object had been deleted, then the object would not exist
** in the resolved path, and hence would not match any of the resolved paths,
** and so the notify would not be sent
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths(void)
{
    int i;
    subs_t *sub;

    // Exit if the object deletion paths have already been resolved for the current USP message
    if (object_deletion_paths_resolved == true)
    {
        return;
    }

    // Iterate over all enabled subscriptions
    for (i=0; i < subscriptions.num_entries; i++)
    {
        // See if this subscription is active, and is an object deletion notification
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_ObjectDeletion))
        {
            // Create a list of all objects which are referenced by this subscription
            // NOTE: We use kResolveOp_SubsDel because we want to determine all current instances of objects with the path expression
            ResolveAllPathExpressions(sub->instance, &sub->path_expressions, &sub->handler_group_ids, &sub->cur_watch_objs, NULL, kResolveOp_SubsDel, sub->cont_instance);
        }
    }

    object_deletion_paths_resolved = true;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent
**
** Called to notify this module that an object instance has been added or deleted from the data model
** This function simply adds the object life event to a queue, which will be serviced later
** by DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions()
** We use a queue because we want any USP subscription notification messages to be sent after
** the response to the USP add or delete request
**
** \param   obj_path - path to object successfully added/deleted in the data model
** \param   notify_type - type of object life event (creation/deletion) that occurred for this object
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_NotifyObjectLifeEvent(char *obj_path, subs_notify_t notify_type)
{
    int new_num_entries;
    obj_life_event_t *ole;

    // Queue the object life event for later processing (by DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions)
    new_num_entries = object_life_events.num_entries + 1;
    object_life_events.vector = USP_REALLOC(object_life_events.vector, new_num_entries*sizeof(obj_life_event_t));

    ole = &object_life_events.vector[ object_life_events.num_entries ];
    ole->obj_path = USP_STRDUP(obj_path);
    ole->notify_type = notify_type;

    object_life_events.num_entries = new_num_entries;

    // Increment the count of creation/deletion events in the vector
    switch(notify_type)
    {
        case kSubNotifyType_ObjectCreation:
            USP_LOG_DebugNotifications("%s(Created=%s)", __FUNCTION__, obj_path);
            object_life_events.num_creation++;
            break;

        case kSubNotifyType_ObjectDeletion:
            USP_LOG_DebugNotifications("%s(Deleted=%s)", __FUNCTION__, obj_path);
            object_life_events.num_deletion++;
            break;

        default:
            TERMINATE_BAD_CASE(notify_type);
            break;
    }
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions
**
** Called to send back notification events for all recently added or deleted objects
** (ie those objects added or deleted since last time this function was called)
** that have an active object subscription on them
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_ProcessAllObjectLifeEventSubscriptions(void)
{
    int i;
    subs_t *sub;
    obj_life_event_t *ole;

    USP_LOG_DebugNotifications("%s(num_creation=%d, num_deletion=%d)", __FUNCTION__, object_life_events.num_creation, object_life_events.num_deletion);

    // Exit if no object life events occurred
    if ((object_life_events.num_creation == 0) && (object_life_events.num_deletion == 0))
    {
        goto exit;
    }

    // Resolve the object creation subscription paths (this function is called after the instances have been added)
    if (object_life_events.num_creation > 0)
    {
        DEVICE_SUBSCRIPTION_ResolveObjectCreationPaths();
        DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths();   // NOTE: This does nothing, if the paths have already been resolved this processing cycle
    }

    // Iterate over all enabled subscriptions
    for (i=0; i < subscriptions.num_entries; i++)
    {
        // See if this subscription is active, and is an object life event notification
        sub = &subscriptions.vector[i];
        if ( (sub->enable) &&
             ((sub->notify_type == kSubNotifyType_ObjectCreation) || (sub->notify_type == kSubNotifyType_ObjectDeletion)) )
        {
            ProcessObjectLifeEventSubscription(sub);
        }
    }

    // Clear the list of object life events, since we have queued any notification messages which they matched
    for (i=0; i < object_life_events.num_entries; i++)
    {
        ole = &object_life_events.vector[i];
        USP_FREE(ole->obj_path);
    }
    USP_FREE(object_life_events.vector);
    object_life_events.vector = NULL;
    object_life_events.num_entries = 0;
    object_life_events.num_creation = 0;
    object_life_events.num_deletion = 0;

exit:
    // Move the current set of resolved paths for object deletion into the last_watch_objs
    // This is so that we have a baseline set of objects to match against, before deletion occurs
    // Also clear the current set of resolved paths for object deletion
    if (object_deletion_paths_resolved)
    {
        for (i=0; i < subscriptions.num_entries; i++)
        {
            sub = &subscriptions.vector[i];
            if ((sub->enable) && (sub->notify_type == kSubNotifyType_ObjectDeletion))
            {
                STR_VECTOR_Destroy(&sub->last_watch_objs);
                memcpy(&sub->last_watch_objs, &sub->cur_watch_objs, sizeof(str_vector_t));
                STR_VECTOR_Init(&sub->cur_watch_objs);
                STR_VECTOR_Destroy(&sub->cur_watch_objs);
            }
        }
    }

    // Clear the current set of resolved paths for object creation
    if (object_creation_paths_resolved)
    {
        for (i=0; i < subscriptions.num_entries; i++)
        {
            sub = &subscriptions.vector[i];
            if ((sub->enable) && (sub->notify_type == kSubNotifyType_ObjectCreation))
            {
                STR_VECTOR_Destroy(&sub->cur_watch_objs);
            }
        }
    }

    // Reset the flag that prevents us resolving the subscription paths more than one per DM_EXEC processing cycle
    object_creation_paths_resolved = false;
    object_deletion_paths_resolved = false;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_SendPeriodicEvent
**
** Sends out a periodic event to the specified controller (if it has any periodic event subscriptions enabled)
**
** \param   cont_instance - instance number of the controller in Device.LocalAgent.Controller.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_SendPeriodicEvent(int cont_instance)
{
    int i;
    subs_t *sub;
    Usp__Msg *req;
    kv_vector_t output_args;

    // Output arguments for the Periodic event are empty
    KV_VECTOR_Init(&output_args);

    // Iterate over all enabled subscriptions, finding all enabled periodic subscriptions for the specified controller
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_Event) &&
            (sub->cont_instance == cont_instance))
        {
            // Send the event, if it matches this subscription
            if (DoesSubscriptionSendNotification(sub, (char *)periodic_event_str))
            {
                // Create the notify message
                req = MSG_HANDLER_CreateNotifyReq_Event((char *)periodic_event_str, &output_args, sub->subscription_id, sub->notification_retry);

                // Send the Notify Request
                SendNotify(req, sub, (char *)periodic_event_str);
                usp__msg__free_unpacked(req, pbuf_allocator);
            }
        }
    }
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_NotifyControllerDeleted
**
** Deletes all subscriptions owned by a controller, after the controller has been deleted
**
** \param   cont_instance - instance number of the controller in Device.LocalAgent.Controller.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_NotifyControllerDeleted(int cont_instance)
{
    int i;
    subs_t *sub;
    char path[MAX_DM_PATH];

    // Iterate over all subscriptions, deleting all that match the controller
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if (sub->cont_instance == cont_instance)
        {
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_subs_root, sub->instance);
            DATA_MODEL_DeleteInstance(path, 0);        // NOTE: This will cascade to delete from subscriptions vector via the delete hook callback
        }
    }
}

#ifndef REMOVE_USP_BROKER
/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_RouteNotification
**
** Sends the specified USP notification to the controller that subscribed to it on the Broker
** The USP notification was received from a USP Service and it will be modified so that it appears to have come from the USP Broker
**
** \param   usp - pointer to parsed USP message structure. This will be freed by the caller (not this function)
** \param   instance - instance number of the subscription in the Broker's Device.LocalAgent.Subscription.{i}
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SUBSCRIPTION_RouteNotification(Usp__Msg *usp, int instance)
{
    subs_t *sub;
    Usp__Notify *notify;
    subs_notify_t notify_type;
    unsigned short perm_mask;
    char *path;
    char buf[MAX_DM_PATH];
    char msg_id[MAX_MSG_ID_LEN];
    char *command_key;
    char *reboot_cause;
    char *firmware_updated;

    // Calculate various values which depend on the type of the received message
    notify = usp->body->request->notify;
    switch(notify->notification_case)
    {
        case USP__NOTIFY__NOTIFICATION_VALUE_CHANGE:
            notify_type = kSubNotifyType_ValueChange;
            perm_mask = PERMIT_SUBS_VAL_CHANGE;
            path = notify->value_change->param_path;
            break;

        case USP__NOTIFY__NOTIFICATION_EVENT:
            notify_type = kSubNotifyType_Event;
            perm_mask = PERMIT_SUBS_EVT_OPER_COMP;
            USP_SNPRINTF(buf, sizeof(buf), "%s%s", notify->event->obj_path, notify->event->event_name);
            path = buf;
            break;

        case USP__NOTIFY__NOTIFICATION_OBJ_CREATION:
            notify_type = kSubNotifyType_ObjectCreation;
            perm_mask = PERMIT_SUBS_OBJ_ADD;
            path = notify->obj_creation->obj_path;
            break;

        case USP__NOTIFY__NOTIFICATION_OBJ_DELETION:
            notify_type = kSubNotifyType_ObjectDeletion;
            perm_mask = PERMIT_SUBS_OBJ_DEL;
            path = notify->obj_deletion->obj_path;
            break;

        case USP__NOTIFY__NOTIFICATION_OPER_COMPLETE:
            notify_type = kSubNotifyType_OperationComplete;
            perm_mask = PERMIT_SUBS_EVT_OPER_COMP;
            USP_SNPRINTF(buf, sizeof(buf), "%s%s", notify->oper_complete->obj_path, notify->oper_complete->command_name);
            path = buf;
            break;

        default:
        case USP__NOTIFY__NOTIFICATION__NOT_SET:
        case USP__NOTIFY__NOTIFICATION_ON_BOARD_REQ:
            USP_ERR_SetMessage("%s: Incorrect type (%d) in received notification", __FUNCTION__, notify->notification_case);
            return USP_ERR_REQUEST_DENIED;
            break;
    }

    // Find the corresponding subscription
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, instance);
    USP_ASSERT(sub != NULL)
    USP_ASSERT(sub->enable == true);  // The code should not have got here if the subscription is disabled, because there shouldn't have been an entry in the subscription mapping table

    // Exit if the notification received is not of the expected type
    if (sub->notify_type != notify_type)
    {
        USP_ERR_SetMessage("%s: Unexpected type (got=%s, expected=%s) in notification", __FUNCTION__, NOTIFY_TYPE_STR(notify_type), NOTIFY_TYPE_STR(sub->notify_type));
        return USP_ERR_REQUEST_DENIED;
    }

    // Exit if the Controller does not have permission. In this case we just silently drop the notification
    if (HasControllerGotNotificationPermission(sub->cont_instance, path, perm_mask) == false)
    {
        return USP_ERR_OK;
    }

    // Exit, sending a new Boot! event containing the ParameterMap for the originating controller
    // This is necessary because the Boot! event we received won't have been generated by the USP Service with the ParameterMap for the originating controller
    if ((notify->notification_case == USP__NOTIFY__NOTIFICATION_EVENT) && (strcmp(path, "Device.Boot!")==0))
    {
        // Extract arguments which we want to carry over to the new Boot! event
        command_key = ExtractNotificationEventArg(notify->event, "CommandKey");
        reboot_cause = ExtractNotificationEventArg(notify->event, "Cause");
        firmware_updated = ExtractNotificationEventArg(notify->event, "FirmwareUpdated");

        SendBootNotify(sub, command_key, reboot_cause, firmware_updated);
        return USP_ERR_OK;
    }

    // Modify the Subscription ID in the notification
    USP_FREE(notify->subscription_id);
    notify->subscription_id = USP_STRDUP(sub->subscription_id);

    // Modify the msg_id in the notification
    MSG_HANDLER_CalcNotifyMsgId(sub->notify_type, msg_id, sizeof(msg_id));
    USP_FREE(usp->header->msg_id);
    usp->header->msg_id = USP_STRDUP(msg_id);

    // Modify the send_resp
    notify->send_resp = sub->notification_retry;

    // Send the Notify Request to the controller which set up the subscription on the Broker
    // NOTE: This call also ensures that we now handle retries for this notification message to the originating controller
    SendNotify(usp, sub, path);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_MarkVendorLayerSubs
**
** Checks that the expected broker subscription instance has the specified notify_type and path
** and if so marks it as being satisfied by the vendor layer
** This function is called as part of synching the USP Service's subscriptions with the Broker,
** to ensure the Broker's internal state reflects the mapping
**
** \param   broker_instance - expected instance number of the subscription in the broker's subscription table
** \param   notify_type - type of notification to match
** \param   path - data model path for subscription to match
** \param   group_id - ID representing the USP Service that owns the path. This is used to mark the path as being satisfied by the USP Service
**
** \return  true if the subscription matched and is now marked as being satisfied by the vendor layer
**          false if the subscription did not match and should be deleted on the USP Service
**
**
**************************************************************************/
bool DEVICE_SUBSCRIPTION_MarkVendorLayerSubs(int broker_instance, subs_notify_t notify_type, char *path, int group_id)
{
    int i;
    int index;
    subs_t *sub;

    // Exit if expected instance did not exist in the Broker's subscription table
    sub = FindSubsByInstance(broker_instance);
    if (sub == NULL)
    {
        return false;
    }

    // Exit if subscription was not enabled, or was for wrong notify type
    if ((sub->enable==false) || (sub->notify_type != notify_type))
    {
        return false;
    }

    // Handle subscriptions to 'Device.'
    if (strcmp(path, dm_root)==0)
    {
        // Exit if this subscription does not contain 'Device.'
        index = STR_VECTOR_Find(&sub->path_expressions, dm_root);
        if (index == INVALID)
        {
            return false;
        }

        // Exit if we've already set a subscription to 'Device.' on this USP Service (for this subscription's notify type)
        // This could be the case if we're performing the registration sequence for an additional register from the USP Service
        index = INT_VECTOR_Find(&sub->device_group_ids, group_id);
        if (index != INVALID)
        {
            return true;
        }

        // Mark this path as being satisfied by the vendor layer
        INT_VECTOR_Add(&sub->device_group_ids, group_id);
        return true;
    }


    // Iterate over all paths for this subscription, finding the first matching path that is not already satisfied by a vendor layer subscription
    for (i=0; i < sub->path_expressions.num_entries; i++)
    {
        if ((sub->handler_group_ids.vector[i] == NON_GROUPED) && (strcmp(sub->path_expressions.vector[i], path) == 0))
        {
            // Mark this path as being satisfied by the vendor layer
            sub->handler_group_ids.vector[i] = group_id;
            return true;
        }
    }

    // If the code gets here, then no match was found
    return false;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_StartAllVendorLayerSubsForGroup
**
** Starts all vendor layer subscriptions for paths owned by the specified group
** This function is called after a USP Service registers to add all subscriptions owned by it, to it
**
** \param   group_id - ID representing the USP Service that we want to start vendor layer subscriptions for
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_StartAllVendorLayerSubsForGroup(int group_id)
{
    int i;
    subs_t *sub;

    // Iterate over all enabled subscriptions, starting vendor layer subscriptions for all paths that
    // are owned by the USP Service, that haven't already been started
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if (sub->enable)
        {
            StartSubscriptionInVendorLayer(sub, group_id);
        }
    }
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_FreeAllVendorLayerSubsForGroup
**
** Marks all subscriptions currently owned by the specified group as not-in-use by the group
** and not being handled by the vendor layer.
** This function is called when a USP Service disconnects
**
** \param   group_id - ID representing the USP Service that has disconnected
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_FreeAllVendorLayerSubsForGroup(int group_id)
{
    int i, j;
    subs_t *sub;

    // Iterate over all enabled subscriptions, marking all vendor layer subscriptions owned by the group, as not being owned by the group_id anymore
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if (sub->enable)
        {
            // Iterate over all paths for this subscription
            for (j=0; j < sub->path_expressions.num_entries; j++)
            {
                if (sub->handler_group_ids.vector[j] == group_id)
                {
                    // Mark this path as not being satisfied by the vendor layer
                    sub->handler_group_ids.vector[j] = NON_GROUPED;
                }
            }

            // Mark the subscription as not being provided by the vendor layer (if the subscription was for 'Device.')
            INT_VECTOR_Remove(&sub->device_group_ids, group_id);
        }
    }

}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs
**
** Removes the specified subscription on the USP Service and marks it as not being handled by the vendor layer
** This function is called when a USP Service deregisters a path
**
** \param   group_id - ID representing the USP Service that we wish to remove the subscription from
** \param   broker_instance - instance number of the subscription in our subscription table
** \param   service_instance - instance number of the subscription in the USP Service's subscription table
** \param   path - data model path that will be unsubscribed from on the USP Service
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SUBSCRIPTION_RemoveVendorLayerSubs(int group_id, int broker_instance, int service_instance, char *path)
{
    subs_t *sub;
    int index;
    dm_unsubscribe_cb_t unsubscribe_hook;
    int err;

    // Find the subscription here in the USP Broker matching the specified instance
    sub = FindSubsByInstance(broker_instance);
    USP_ASSERT(sub != NULL);                    // Since the data structures in the caller (USP Broker) should always be aligned with the data structures here

    if (strcmp(path, dm_root) == 0)
    {
        // Case of unsubscribing from 'Device.'
        // Find the group_id in the list of USP Services subscribed to 'Device.' for this subscription, and remove it
        index = INT_VECTOR_Find(&sub->device_group_ids, group_id);
        USP_ASSERT(index != INVALID);  // Because this function is only called if a subscription had already been successfully set on the USP Service for 'Device.'
        INT_VECTOR_Remove(&sub->device_group_ids, group_id);
    }
    else
    {
        // Case of unsubscribing from any path other than 'Device.'
        // Find the path in the ReferenceList
        index = STR_VECTOR_Find(&sub->path_expressions, path);
        USP_ASSERT(index != INVALID);               // Since the data structures in the caller (USP Broker) should always be aligned with the data structures here

        // Mark the path as not in use by the vendor layer
        USP_ASSERT(sub->handler_group_ids.num_entries > index);
        sub->handler_group_ids.vector[index] = NON_GROUPED;
    }

    // Exit if the unsubscribe vendor hook failed
    unsubscribe_hook = group_vendor_hooks[group_id].unsubscribe_cb;
    USP_ASSERT(unsubscribe_hook != NULL);
    err = unsubscribe_hook(broker_instance, group_id, sub->notify_type, path);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_IsMatch
**
** Determines whether the specified subscription table entry contains a subscription for the specified path (and notification type)
** This function is called to determine whether to pass through a notification from the USP Service
** in the case of the notification being received before the registration sequence has been completed with the USP Service
** Typically this function is called to match Device.Boot! event notifications
**
** \param   broker_instance - instance number of the subscription in our subscription table
** \param   notify_type - type of the notification to check against
** \param   path - data model path to check against
**
** \return  true if the specified instance contains an enabled subscription that matches, false otherwise
**
**************************************************************************/
bool DEVICE_SUBSCRIPTION_IsMatch(int broker_instance, subs_notify_t notify_type, char *path)
{
    int i;
    subs_t *sub;
    char *path_spec;

    // Exit if we don't have a subscription matching the specified instance number
    sub = FindSubsByInstance(broker_instance);
    if (sub == NULL)
    {
        return false;
    }

    // Exit if the subscription is not enabled
    if (sub->enable == false)
    {
        return false;
    }

    // Exit if the subscription is for a different type of notification
    if (sub->notify_type != notify_type)
    {
        return false;
    }

    // Iterate over all paths that the Broker's subscription is for, seeing if the path matches any of those
    for (i=0; i < sub->path_expressions.num_entries; i++)
    {
        // Exit if the notification path matches that defined by the path specification in the subscription (ie wildcards and partial paths - including Device.)
        // NOTE: This code handles subscriptions to 'Device.'
        path_spec = sub->path_expressions.vector[i];
        if (TEXT_UTILS_IsPathMatch(path, path_spec))
        {
            return true;
        }
    }

    // If the code gets here, then no match was found
    return false;
}
#endif

/*********************************************************************//**
**
** DEVICE_SUBSCRIPTION_Dump
**
** Convenience function to dump out the internal subscriptions vector
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_SUBSCRIPTION_Dump(void)
{
    SUBS_VECTOR_Dump(&subscriptions);
}

/*********************************************************************//**
**
** ProcessSubscriptionAdded
**
** Reads the specified subscription from the database and
** adds it to the subscription vector and activates processing associated with it
**
** \param   instance - instance number of the subscription in the data model
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessSubscriptionAdded(int instance)
{
    char path[MAX_DM_PATH];
    char controller_path[MAX_DM_PATH];
    subs_t sub;
    int err;

    // Initialise the structure representing this subscription
    memset(&sub, 0, sizeof(sub));
    sub.instance = instance;
    STR_VECTOR_Init(&sub.path_expressions);
    INT_VECTOR_Init(&sub.handler_group_ids);
    INT_VECTOR_Init(&sub.device_group_ids);
    KV_VECTOR_Init(&sub.last_values);
    STR_VECTOR_Init(&sub.cur_watch_objs);
    STR_VECTOR_Init(&sub.last_watch_objs);
    sub.skip_obj_notifications = false;

    // Exit if unable to calculate the expiry time for this subscription
    // NOTE: The subscription is not deleted by this function, but by the polling mechanism
    err = CalcExpiryTime(instance, &sub.expiry_time);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get whether this subscription was enabled or not
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Enable", device_subs_root, instance);
    err = DM_ACCESS_GetBool(path, &sub.enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get Recipient (value = reference to row in controller table)
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Recipient", device_subs_root, instance);
    err = DATA_MODEL_GetParameterValue(path, controller_path, sizeof(controller_path), 0);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if no recipient is setup
    if (controller_path[0] == '\0')
    {
        USP_ERR_SetMessage("%s: No recipient controller found in %s", __FUNCTION__, path);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Exit if unable to extract the instance number of the controller that created this subscription
    // NOTE: If the controller was deleted, then this subscription will get deleted by the caller of this function
    err = DM_ACCESS_ValidateReference(controller_path, "Device.LocalAgent.Controller.{i}", &sub.cont_instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get the Subscription ID
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ID", device_subs_root, instance);
    err = DM_ACCESS_GetString(path, &sub.subscription_id);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get NotifRetry
    USP_SNPRINTF(path, sizeof(path), "%s.%d.NotifRetry", device_subs_root, instance);
    err = DM_ACCESS_GetBool(path, &sub.notification_retry);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get Persistent
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Persistent", device_subs_root, instance);
    err = DM_ACCESS_GetBool(path, &sub.persistent);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get NotifExpiration
    USP_SNPRINTF(path, sizeof(path), "%s.%d.NotifExpiration", device_subs_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sub.retry_expiry_period);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Get ReferenceList
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ReferenceList", device_subs_root, instance);
    err = DM_ACCESS_GetStringVector(path, &sub.path_expressions);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Ensure that the subscription handler_group_ids for all of the ReferenceList path components are marked as not used
    INT_VECTOR_Create(&sub.handler_group_ids, sub.path_expressions.num_entries, NON_GROUPED);

    // Get NotifType
    USP_SNPRINTF(path, sizeof(path), "%s.%d.NotifType", device_subs_root, instance);
    err = DM_ACCESS_GetEnum(path, &sub.notify_type, notify_types, NUM_ELEM(notify_types));
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // If the code gets here, then we successfully retrieved all data about the subscription
    err = USP_ERR_OK;

exit:
    if (err == USP_ERR_OK)
    {
        if (sub.enable)
        {
            StartSubscription(&sub);
        }

        // We have successfully retrieved a subscription, so add it to the vector
        // NOTE: Ownership of the dynamically allocated memory referenced by the temp subscriber structure(sub) passes to the vector
        // So we do not have to call SUBS_VECTOR_DestroySubscriber(&sub)
        SUBS_VECTOR_Add(&subscriptions, &sub);
    }
    else
    {
        // Free all memory in the temporary subscriber structure used by this function
        SUBS_VECTOR_DestroySubscriber(&sub);
    }

    return err;
}

/*********************************************************************//**
**
** StartSubscription
**
** Called to start the specified subscription
**
** \param   sub - pointer to subscription to start
**
** \return  None
**
**************************************************************************/
void StartSubscription(subs_t *sub)
{
    // Exit if the subscription is not enabled, or the paths or notify type have not been set yet
    if ((sub->enable == false) || (sub->notify_type == kSubNotifyType_None) || (sub->path_expressions.num_entries == 0))
    {
        return;
    }

#ifndef REMOVE_USP_BROKER
    // Call the subscribe vendor hook for all paths in the subscription which can be satisfied by the vendor layer
    StartSubscriptionInVendorLayer(sub, INVALID);
#endif

    // Get the initial value of all parameters, if this is a value change subscription
    // NOTE: The parameters aren't obtained for paths which are handled by the vendor-layer
    if (sub->notify_type == kSubNotifyType_ValueChange)
    {
        KV_VECTOR_Destroy(&sub->last_values);  // last_values could exist if this subscription is being re-enabled
        GetAllPathExpressionParameterValues(sub, &sub->path_expressions, &sub->handler_group_ids, &sub->last_values);
    }

    // Get a baseline set of instances, if this is an object lifecycle subscription
    // Simply resolving the path expressions for ObjectCreation/Deletion subscriptions will result in the refresh_instances vendor hook being called if necessary
    // NOTE: The refresh instances vendor hook is only called if the cached instance numbers have expired
    // NOTE: The refresh instances vendor hook isn't called if all paths are handled by the vendor-layer
    if ((sub->notify_type == kSubNotifyType_ObjectCreation) || (sub->notify_type == kSubNotifyType_ObjectDeletion))
    {
        resolve_op_t op;
        op = (sub->notify_type == kSubNotifyType_ObjectCreation) ? kResolveOp_SubsAdd : kResolveOp_SubsDel;
        ResolveAllPathExpressions(sub->instance, &sub->path_expressions, &sub->handler_group_ids, NULL, NULL, op, sub->cont_instance);

        // Ensure that enabling this subscription does not result in it firing notifications,
        // (which would occur without this flag, as ResolveAllPathExpressions() internally queues object life cycle events, which are processed after this subscription has been enabled)
        sub->skip_obj_notifications = true;
    }
}

#ifndef REMOVE_USP_BROKER
/*********************************************************************//**
**
** StartSubscriptionInVendorLayer
**
** Starts subscriptions on all paths which the vendor layer can satisfy on the specified subscription
**
** \param   sub - pointer to structure representing an instance in Device.LocalAgent.Subscription table
** \param   group_id - ID representing the USP Service that we want to start vendor layer subscriptions for
**                     or INVALID, if we want to start vendor layer subscriptions on all USP Services referenced by this subscription
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void StartSubscriptionInVendorLayer(subs_t *sub, int group_id)
{
    int i;
    int err;
    char *path;
    int subs_group_id;
    dm_subscribe_cb_t subscribe_hook;

    // Exit if this subscription's notify type has not been setup yet
    // NOTE: This could happen if the path was set before the notify type. Nothing can be done in this case.
    // When notify_type is eventually set, this function will be called again
    if (sub->notify_type == kSubNotifyType_None)
    {
        return;
    }

    // Iterate over all paths split from the ReferenceList (there may be multiple comma separated paths)
    USP_ASSERT(sub->path_expressions.num_entries == sub->handler_group_ids.num_entries);
    for (i=0; i < sub->path_expressions.num_entries; i++)
    {
        // Handle 'Device.' subscriptions here
        path = sub->path_expressions.vector[i];
        if (strcmp(path, dm_root)==0)
        {
            StartVendorLayerDeviceDotSubs(sub, group_id);
            continue;
        }

        // Skip this path if it is already delegated to a USP Service
        if (sub->handler_group_ids.vector[i] != NON_GROUPED)
        {
            continue;
        }

        // Skip this path if it cannot be subscribed to in the vendor layer
        subs_group_id = USP_BROKER_IsPathVendorSubscribable(sub->notify_type, path, NULL);
        if (subs_group_id == NON_GROUPED)
        {
            continue;
        }

        // Skip this path if it is not owned by the specified USP Service (if applicable)
        if ((group_id != INVALID) && (group_id != subs_group_id))
        {
            continue;
        }

        // Skip if the data model provider component does not yet have a subscribe vendor hook registered
        // In this case the subscription will be started when the GSDM response has been received from the USP Service as part of syncing the subscriptions
        subscribe_hook = group_vendor_hooks[subs_group_id].subscribe_cb;
        if (subscribe_hook == NULL)
        {
            continue;
        }

        // Attempt to subscribe to path for the specified notification
        // NOTE: If this is not successful, then for value change and object creation/deletion, we fallback to the polled mechanism
        // to provide this subscription. For USP events and operation complete, there is no fallback, and we will not receive any
        // notifications from this subscription for the specified path if the subscribe vendor hook fails
        err = subscribe_hook(sub->instance, subs_group_id, sub->notify_type, path, sub->persistent);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Warning("%s: Subscribe vendor hook failed for %s (%s)", __FUNCTION__, path, NOTIFY_TYPE_STR(sub->notify_type));
            continue;
        }

        // If the code gets here, then the vendor layer subscription was successful, so mark this path as being provided by the vendor layer subscription
        sub->handler_group_ids.vector[i] = subs_group_id;
    }
}

/*********************************************************************//**
**
** StartVendorLayerDeviceDotSubs
**
** Starts a vendor layer subscription on 'Device.' for either USP Events or OperationComplete notifications (if not already started)
**
** \param   sub - subscription that has a ReferenceList of 'Device.'
** \param   group_id - ID representing the USP Service that we want to start vendor layer subscriptions for
**                     or INVALID, if we want to start vendor layer subscriptions on all USP Services referenced by this subscription
**
** \return  None
**
**************************************************************************/
void StartVendorLayerDeviceDotSubs(subs_t *sub, int group_id)
{
    int i;
    int_vector_t group_ids_in_use;

    USP_ASSERT((sub->notify_type == kSubNotifyType_Event) || (sub->notify_type == kSubNotifyType_OperationComplete));

    // Exit if group_id represents just a single USP Service
    if (group_id != INVALID)
    {
        StartVendorLayerDeviceDotSubsForGroup(sub, group_id);
        return;
    }

    // If the code gets here, then we need to set the subscription on all USP Services that have registered data model paths
    USP_BROKER_GetAllRegisteredGroupIds(&group_ids_in_use);
    for (i=0; i < group_ids_in_use.num_entries; i++)
    {
        group_id = group_ids_in_use.vector[i];
        StartVendorLayerDeviceDotSubsForGroup(sub, group_id);
    }

    INT_VECTOR_Destroy(&group_ids_in_use);
}

/*********************************************************************//**
**
** StartVendorLayerDeviceDotSubsForGroup
**
** Starts a vendor layer subscription on 'Device.' for a single USP Service
**
** \param   sub - subscription that has a ReferenceList of 'Device.'
** \param   group_id - ID representing the USP Service that we want to start vendor layer subscriptions for
**
** \return  None
**
**************************************************************************/
void StartVendorLayerDeviceDotSubsForGroup(subs_t *sub, int group_id)
{
    int index;
    int err;
    dm_subscribe_cb_t subscribe_hook;

    USP_ASSERT(group_id != INVALID);

    // Exit if the subscription has already been set for this USP Service
    index = INT_VECTOR_Find(&sub->device_group_ids, group_id);
    if (index != INVALID)
    {
        return;
    }

    // Exit if the data model provider component does not yet have a subscribe vendor hook registered
    // In this case the subscription will be started when the GSDM response has been received from the USP Service as part of syncing the subscriptions
    subscribe_hook = group_vendor_hooks[group_id].subscribe_cb;
    if (subscribe_hook == NULL)
    {
        return;
    }

    // Exit if unable to subscribe to 'Device.' on the specified USP Service
    // NOTE: If this is not successful, then there is no fallback. We will not receive any notifications for the specified path
    err = subscribe_hook(sub->instance, group_id, sub->notify_type, dm_root, sub->persistent);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Vendor layer failed to subscribe to 'Device.' for %s", __FUNCTION__, NOTIFY_TYPE_STR(sub->notify_type));
        return;
    }

    // Mark this USP Service as having a subscription to 'Device.' for the specified notification type
    INT_VECTOR_Add(&sub->device_group_ids, group_id);
}

/*********************************************************************//**
**
** StopSubscriptionInVendorLayer
**
** Stops vendor layer subscriptions on all paths on the specified subscription
**
** \param   sub - pointer to structure representing an instance in Device.LocalAgent.Subscription table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void StopSubscriptionInVendorLayer(subs_t *sub)
{
    int i;
    int err;
    char *path;
    int group_id;
    dm_unsubscribe_cb_t unsubscribe_hook;

    USP_ASSERT(sub->path_expressions.num_entries == sub->handler_group_ids.num_entries);

    // Iterate over all paths split from the ReferenceList (there may be multiple comma separated paths)
    for (i=0; i < sub->path_expressions.num_entries; i++)
    {
        // Skip 'Device.' subscriptions, they will be handled later in the function
        // NOTE: This test is not strictly necessary, as a 'Device.' subscription should never be marked as being provided by the vendor layer in handler_group_ids
        path = sub->path_expressions.vector[i];
        if (strcmp(path, dm_root)==0)
        {
            continue;
        }

        // Skip if this path was not subscribed in the vendor layer
        group_id = sub->handler_group_ids.vector[i];
        if (group_id == NON_GROUPED)
        {
            continue;
        }

        // Skip if the USP Service does not have an unsubscribe vendor hook registered
        unsubscribe_hook = group_vendor_hooks[group_id].unsubscribe_cb;
        if (unsubscribe_hook == NULL)
        {
            continue;
        }

        // Unsubscribe from the path on the USP Service
        err = unsubscribe_hook(sub->instance, group_id, sub->notify_type, path);
        if (err != USP_ERR_OK)
        {
            // NOTE: If this is not successful, we assume this is because the vendor layer subscription has already been deleted due to some other reason outside of our control
            USP_LOG_Error("%s: Unsubscribe vendor hook failed for '%s' (%s)", __FUNCTION__, path, NOTIFY_TYPE_STR(sub->notify_type));
        }

        // Mark this path as not being provided by the vendor layer subscription
        sub->handler_group_ids.vector[i] = NON_GROUPED;
    }

    // Iterate over all USP Services which have a subscription to 'Device.', unsubscribing them
    for (i=0; i < sub->device_group_ids.num_entries; i++)
    {
        group_id = sub->device_group_ids.vector[i];
        USP_ASSERT((group_id >= 0) && (group_id < MAX_VENDOR_PARAM_GROUPS));

        // Determine the unsubscribe vendor hook to call
        unsubscribe_hook = group_vendor_hooks[group_id].unsubscribe_cb;
        USP_ASSERT(unsubscribe_hook != NULL);       // Since the code cannot get here unless a subscribe vendor hook was setup. And if you setup a subscribe vendor hook using USP_REGISTER_SubscriptionVendorHooks(), then you must provide a matching unsubscribe vendor hook

        // Unsubscribe from the path on the USP Service
        err = unsubscribe_hook(sub->instance, group_id, sub->notify_type, dm_root);
        if (err != USP_ERR_OK)
        {
            // NOTE: If this is not successful, we assume this is because the vendor layer subscription has already been deleted due to some other reason outside of our control
            USP_LOG_Error("%s: Unsubscribe vendor hook failed for '%s' (%s)", __FUNCTION__, dm_root, NOTIFY_TYPE_STR(sub->notify_type));
        }
    }

    // Mark that 'Device.' is not being provided by vendor layer subscriptions anymore
    INT_VECTOR_Destroy(&sub->device_group_ids);
}
#endif

/*********************************************************************//**
**
** CalcExpiryTime
**
** Calculates the expiry time of the specified subscription instance
**
** \param   instance - data model instance number of subscription
** \param   expiry_time - pointer to variable in which to return the calculated expiry time
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CalcExpiryTime(int instance, time_t *expiry_time)
{
    int err;
    unsigned time_to_live;
    time_t creation_time;
    char path[MAX_DM_PATH];

    // Get TimeToLive
    USP_SNPRINTF(path, sizeof(path), "%s.%d.TimeToLive", device_subs_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &time_to_live);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if subscription does not ever expire
    if (time_to_live == 0)
    {
        *expiry_time = END_OF_TIME;
        return USP_ERR_OK;
    }

    // Get CreationDate
    USP_SNPRINTF(path, sizeof(path), "%s.%d.CreationDate", device_subs_root, instance);
    err = DM_ACCESS_GetDateTime(path, &creation_time);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Calculate expiry time
    *expiry_time = creation_time + time_to_live;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifySubsAdded
**
** Function called after a subscription has been added
**
** \param   req - pointer to structure identifying the subscription
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifySubsAdded(dm_req_t *req)
{
    int err;

    USP_LOG_Info("Subscription added (%s)", req->path);

    err = ProcessSubscriptionAdded(inst1);

    return err;
}

/*********************************************************************//**
**
** NotifySubsDeleted
**
** Function called after a subscription has been deleted
**
** \param   req - pointer to structure identifying the subscription
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifySubsDeleted(dm_req_t *req)
{
    subs_t *sub;

    USP_LOG_Info("Subscription deleted [%d]", inst1);

    // Delete the subscription from the vector (and stop all notification retries), if it has not already been deleted
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    if (sub != NULL)
    {
#ifndef REMOVE_USP_BROKER
        StopSubscriptionInVendorLayer(sub);
#endif
        SUBS_RETRY_Delete(sub->instance);
        SUBS_VECTOR_Remove(&subscriptions, sub);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_SubsEnable
**
** Function called when the Enable for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_SubsEnable(dm_req_t *req, char *value)
{
    subs_t *sub;
    bool cur_enable;
    bool new_enable;

    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

    // Exit if the Enable parameter has not changed
    new_enable = val_bool;
    cur_enable = sub->enable;
    if (cur_enable == new_enable)
    {
        return USP_ERR_OK;
    }

    // Save the new state
    sub->enable = new_enable;

    // Subscription is being enabled
    if (sub->enable)
    {
        StartSubscription(sub);
    }
#ifndef REMOVE_USP_BROKER
    else
    {
        StopSubscriptionInVendorLayer(sub);
    }
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_NotifyType
**
** Function called when the NotifType for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_NotifyType(dm_req_t *req, char *value)
{
    subs_notify_t new_notify_type;
    subs_t *sub;

    // Convert this parameter's value to the notify type enumeration
    new_notify_type = TEXT_UTILS_StringToEnum(value, notify_types, NUM_ELEM(notify_types));

    // Exit if the notify type has not changed
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);
    if (new_notify_type == sub->notify_type)
    {
        return USP_ERR_OK;
    }

    // Exit if the subscription is disabled. In this case, we just need to update to the new value
    if (sub->enable == false)
    {
        sub->notify_type = new_notify_type;
        return USP_ERR_OK;
    }

#ifndef REMOVE_USP_BROKER
    // Stop any vendor layer subscriptions that are using the old value of notify type
    StopSubscriptionInVendorLayer(sub);
#endif

    sub->notify_type = new_notify_type;

    StartSubscription(sub);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_SubsID
**
** Function called when the Subscription ID for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_SubsID(dm_req_t *req, char *value)
{
    subs_t *sub;

    // Determine which subscription this change affects
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

    // Update the Subscription ID for this subscription.
    // This will take effect at the next poll interval
    USP_FREE(sub->subscription_id);
    sub->subscription_id = USP_STRDUP(value);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_SubsRefList
**
** Function called when the ReferenceList for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_SubsRefList(dm_req_t *req, char *value)
{
    subs_t *sub;

    // Determine which subscription this change affects
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

#ifndef REMOVE_USP_BROKER
    // Delete all subscriptions on USP Services that relate to the old ReferenceList
    if (sub->enable)
    {
        StopSubscriptionInVendorLayer(sub);
    }
#endif

    // Delete out current set of path expressions and handler_group_ids
    STR_VECTOR_Destroy(&sub->path_expressions);
    INT_VECTOR_Destroy(&sub->handler_group_ids);

    // Then add this new set of path expressions
    // These will take effect at the next poll interval
    TEXT_UTILS_SplitString(value, &sub->path_expressions, ",");
    INT_VECTOR_Create(&sub->handler_group_ids, sub->path_expressions.num_entries, NON_GROUPED);

#ifndef REMOVE_USP_BROKER
    if (sub->enable)
    {
        StartSubscription(sub);
    }
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_Persistent
**
** Function called when Device.LocalAgent.Subscription.{i}.Persistent is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_Persistent(dm_req_t *req, char *value)
{
    subs_t *sub;

    // Determine which subscription this change affects
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

    // Update the persistent flag for this subscription.
    sub->persistent = val_bool;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_SubsTimeToLive
**
** Function called when the TimeToLive for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_SubsTimeToLive(dm_req_t *req, char *value)
{
    subs_t *sub;
    time_t cur_time;
    int err;

    // Determine which subscription this change affects
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

    // Update the expiry time for this subscription
    err = CalcExpiryTime(inst1, &sub->expiry_time);

    // Delete this subscription, if it has now expired
    // NOTE: The notify function for the delete will call NotifySubsDeleted(), which will remove the subscription from the vector
    cur_time = time(NULL);
    if (cur_time > sub->expiry_time)
    {
        char path[MAX_DM_PATH];
        USP_SNPRINTF(path, sizeof(path), "%s.%d", device_subs_root, inst1);

        DATA_MODEL_DeleteInstance(path, 0);        // NOTE: This will cascade to delete from subscriptions vector via the delete hook callback
    }

    return err;
}

/*********************************************************************//**
**
** NotifyChange_NotifRetry
**
** Function called when the NotifRetry for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_NotifRetry(dm_req_t *req, char *value)
{
    subs_t *sub;
    int err;

    // Determine which subscription this change affects
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

    // Update the notification_retry for this subscription.
    // This will take effect the next time a NotifyRequest is fired off from this subscription
    err = TEXT_UTILS_StringToBool(value, &sub->notification_retry);

    return err;
}

/*********************************************************************//**
**
** NotifyChange_NotifExpiration
**
** Function called when the NotifExpiration for a subscription is changed
**
** \param   req - pointer to structure identifying the subscription
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_NotifExpiration(dm_req_t *req, char *value)
{
    subs_t *sub;
    int err;

    // Determine which subscription this change affects
    sub = SUBS_VECTOR_GetSubsByInstance(&subscriptions, inst1);
    USP_ASSERT(sub != NULL);

    // Update the retry_expiry_period for this subscription.
    // This will take effect the next time a NotifyRequest is fired off from this subscription
    err = TEXT_UTILS_StringToUnsigned(value, &sub->retry_expiry_period);

    return err;
}

/*********************************************************************//**
**
** AutoPopulate_SubsID
**
** Called to get an auto-populated parameter value for the Subscription ID parameter
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer in which to store the value to use to auto-populate the parameter's value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if auto assigned ID was unique
**
**************************************************************************/
int AutoPopulate_SubsID(dm_req_t *req, char *buf, int len)
{
    int err;

    USP_SNPRINTF(buf, len, DEFAULT_ID_PREFIX "%d", inst1);
    err = Validate_SubsID(req, buf);

    return err;
}

/*********************************************************************//**
**
** Validate_SubsID
**
** Validates that the Subscription ID being added is unique for the recipient
**
** \param   req - pointer to structure identifying the subscription
** \param   value - value that the controller would like to set the Subscription ID to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_SubsID(dm_req_t *req, char *value)
{
    char controller_path[MAX_DM_PATH];
    char path[MAX_DM_PATH];
    int i;
    int err;
    subs_t *sub;
    int controller_instance;

    // Exit if no value set for subscription ID (i.e. set to an empty string)
    if (*value == '\0')
    {
        USP_ERR_SetMessage("%s: Subscription ID must be set to a non-empty string", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if unable to get the recipient for this entry in the subscription table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Recipient", device_subs_root, inst1);
    err = DATA_MODEL_GetParameterValue(path, controller_path, sizeof(controller_path), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the controller was deleted, then recipient will be set to an empty string
    // We will allow Subscription ID to be changed in this case, but log a warning
    // 2DO RH: Currently, deleting a controller does not propagate to updating the recipient of a subscription to empty string
    //         Hence this code block will never be run
    if (controller_path[0] == '\0')
    {
        USP_LOG_Warning("WARNING: Setting Subscription ID for %s, where recipient has been deleted. Hence cannot check uniqueness", req->path);
        return USP_ERR_OK;
    }

    // Exit if unable to extract the instance number of the controller that created this subscription
    err = DM_ACCESS_ValidateReference(controller_path, "Device.LocalAgent.Controller.{i}", &controller_instance);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Iterate over all subscriptions in the subscriptions vector
    for (i=0; i<subscriptions.num_entries; i++)
    {
        // Skip this entry if it is the instance we're currently changing the Subscription ID of
        sub = &subscriptions.vector[i];
        if (sub->instance == inst1)
        {
            continue;
        }

        // Skip this entry if it is for a different controller than the one we're changing the Subscription ID of
        // (SubscriptionID only needs to be a unique value within the namespace of each controller)
        if (sub->cont_instance != controller_instance)
        {
            continue;
        }

        // Exit if the Subscription ID of this entry matches the Subscription ID that the controller is trying to set
        if (strcmp(sub->subscription_id, value)==0)
        {
            USP_ERR_SetMessage("%s: Subscription ID (%s) is already in use by %s.%d (subs instance=%d)", __FUNCTION__, value, device_subs_root, sub->instance, inst1);
            return USP_ERR_UNIQUE_KEY_CONFLICT;
        }
    }


    // If the code gets here, then the new Subscription ID value is unique for this controller
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_SubsRefList
**
** Determines whether the path expressions given in the reference list are valid
** (both syntactically and schema-wise)
**
** \param   req - pointer to structure identifying the subscription
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_SubsRefList(dm_req_t *req, char *value)
{
    int err;
    char path[MAX_DM_PATH];
    subs_notify_t notify_type;
    char cur_value[MAX_DM_VALUE_LEN];

    // Exit if unable to get the current value of ReferenceList
    err = DATA_MODEL_GetParameterValue(req->path, cur_value, sizeof(cur_value), 0);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: Failed to get the current value of %s", __FUNCTION__, req->path);
        return err;
    }

    // Exit if the current value of ReferenceList is not empty (ReferenceList is immutable once set)
    if (cur_value[0] != '\0')
    {
        USP_ERR_SetMessage("%s: ReferenceList parameter values must not be changed once assigned (current_value='%s')", __FUNCTION__, cur_value);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Get the value of NotifType from the data model
    // We have to do this because this function may be called before the subs instance has been notified as added
    USP_SNPRINTF(path, sizeof(path), "%s.%d.NotifType", device_subs_root, inst1);
    err = DM_ACCESS_GetEnum(path, &notify_type, notify_types, NUM_ELEM(notify_types));
    if (err != USP_ERR_OK)
    {
        // NOTE: This should never happen, as the instance does exist in the data model,
        // even if NotifType has not been set yet (in which case we get the default value)
        return err;
    }

    if (notify_type == kSubNotifyType_Invalid)
    {
        // NOTE: Again, this should never happen, we should get a default value
        USP_ERR_SetMessage("%s: NotifType associated with ReferenceList is invalid", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the ReferenceList was not permitted, given the notify type
    err = Validate_SubsRefList_Inner(notify_type, value);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the code gets here, then the reference list was valid for the notification type
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_SubsNotifType
**
** Function called to validate the proposed NotifType
**
** \param   req - pointer to structure identifying the subscription
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_SubsNotifType(dm_req_t *req, char *value)
{
    int err;
    char path[MAX_DM_PATH];
    char ref_list[MAX_DM_VALUE_LEN];
    subs_notify_t notify_type;

    // Exit if notification type was invalid
    notify_type = TEXT_UTILS_StringToEnum(value, notify_types, NUM_ELEM(notify_types));
    if (notify_type == kSubNotifyType_Invalid)
    {
        USP_ERR_SetMessage("%s: Invalid notification type (%s) for %s.", __FUNCTION__, value, req->path);
        return USP_ERR_INVALID_VALUE;
    }

    // Get the value of ReferenceList from the data model
    // We have to do this because this function may be called before the subs instance has been notified as added
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ReferenceList", device_subs_root, inst1);
    err = DATA_MODEL_GetParameterValue(path, ref_list, sizeof(ref_list), 0);
    if (err != USP_ERR_OK)
    {
        // NOTE: This should never happen, as the instance does exist in the data model,
        // even if ReferenceList has not been set yet (in which case we get the default value)
        USP_ERR_SetMessage("%s: Unable to get the ReferenceList associated with NotifyType", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the ReferenceList was not permitted, given the notify type
    err = Validate_SubsRefList_Inner(notify_type, ref_list);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the code gets here, then the reference list was valid for the notification type
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BootParamName
**
** Determines whether the path expressions given in the reference list are valid
** (both syntactically and schema-wise)
**
** \param   req - pointer to structure identifying the boot parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BootParamName(dm_req_t *req, char *value)
{
    int err;

    // Exit if the path expression was not valid
    err = Validate_SubsRefList_Inner(kSubNotifyType_ValueChange, value);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // If the code gets here, then the path expression was valid
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_SubsRefList_Inner
**
** This function validates the reference list, given the expected notify type
** Validation includes checking the permission on all resolved paths
** This function is called whenever the subscription notify type or reference list are changed.
** It is also called whenever BootParameter.{i}.ParameterName is changed.
**
** \param   notify_type - Type of notification that ref_list should resolve to (ie which type of data model nodes)
** \param   ref_list - path expression identifying the data model watched by the subscrption
**
** \return  USP_ERR_OK if the referece_list is valid and permitted, given the notify type
**          NOTE: If this function is called before either of the parameters are available, then it returns success
**
**************************************************************************/
int Validate_SubsRefList_Inner(subs_notify_t notify_type, char *ref_list)
{
    int err = USP_ERR_OK;       // Default return code
    str_vector_t path_expressions;
    char *path;
    int i, j;

#ifdef USE_LEGACY_PATH_VALIDATION
    combined_role_t combined_role;
    resolve_op_t op;

    // When running as a pure USP Agent (not a USP Broker)...
    // Convert the notify type to a path resolver operation, so that we can check that
    // ReferenceList matches a node of the specified type, and that the controller has permission
    // to set the notification
    switch(notify_type)
    {
        case kSubNotifyType_ValueChange:
            op = kResolveOp_SubsValChange;
            break;

        case kSubNotifyType_ObjectCreation:
            op = kResolveOp_SubsAdd;
            break;

        case kSubNotifyType_ObjectDeletion:
            op = kResolveOp_SubsDel;
            break;

        case kSubNotifyType_OperationComplete:
            op = kResolveOp_SubsOper;
            break;

        case kSubNotifyType_Event:
            op = kResolveOp_SubsEvent;
            break;

        default:
        case kSubNotifyType_None:
            // If a notify type has not been set yet, then we cannot validate whether the controller is permitted
            // to set the ReferenceList, because we need the type to know which permission to check
            // So we OK this parameter being set (whether it was notify_type or ref_list)
            // When the other parameter in the pair is set, then we will perform a proper check
            return USP_ERR_OK;
            break;
    }

    MSG_HANDLER_GetMsgRole(&combined_role);
#endif

    // Split the reference list into a vector of path expressions
    STR_VECTOR_Init(&path_expressions);
    TEXT_UTILS_SplitString(ref_list, &path_expressions, ",");

    // Iterate over all path expressions
    for (i=0; i<path_expressions.num_entries; i++)
    {
        path = path_expressions.vector[i];

        // Exit if the ReferenceList contains duplicate paths
        for (j=0; j<path_expressions.num_entries; j++)
        {
            if ((j != i) && (strcmp(path, path_expressions.vector[j])==0))
            {
                USP_ERR_SetMessage("%s: Path '%s' is duplicated in ReferenceList '%s'", __FUNCTION__, path, ref_list);
                err = USP_ERR_INVALID_PATH;
                goto exit;
            }
        }

#ifdef USE_LEGACY_PATH_VALIDATION
        // The legacy path validation code actually resolves the path to determine whether it is valid

        // When running as a pure USP Agent, 'Device.' is supported. This code block just avoids resolving the path on Device.
        if ((op == kResolveOp_SubsOper) || (op == kResolveOp_SubsEvent))
        {
            if (strcmp(path, dm_root)==0)
            {
                err = USP_ERR_OK;
                goto exit;
            }
        }

        // Exit if path expression is invalid
        err = PATH_RESOLVER_ResolveDevicePath(path, NULL, NULL, op, FULL_DEPTH, &combined_role, 0);
#else
        // When running as a USP Broker, we cannot validate the path and controller permissions by resolving it, as the
        // USP Service owning the DM elements in the path may not have registered yet, so just attempt to validate it textually
        err = PATH_RESOLVER_ValidatePath(path, notify_type);
#endif

        if (err != USP_ERR_OK)
        {
            goto exit;
        }

    }

exit:
    STR_VECTOR_Destroy(&path_expressions);
    return err;
}

/*********************************************************************//**
**
** GetAuto_Recipient
**
** Function called when adding a new subscription row, to auto populate the name of the recipient of this subscription
** (This will be a reference to the controller who posted the AddRequest that caused this function to be called)
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer in which to return the auto populated value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetAuto_Recipient(dm_req_t *req, char *buf, int len)
{
    int instance;

    instance = MSG_HANDLER_GetMsgControllerInstance();
    USP_SNPRINTF(buf, len, "Device.LocalAgent.Controller.%d", instance);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetAuto_CreationDate
**
** Function called when adding a new subscription row, to auto populate the creation date of this subscription
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer in which to return the auto populated value
** \param   len - length of return buffer
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetAuto_CreationDate(dm_req_t *req, char *buf, int len)
{
    USP_ASSERT(len >= MAX_ISO8601_LEN);
    val_datetime = time(NULL);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DeleteExpiredSubscriptions
**
** Deletes all expired subscriptions from the data model and from the subscriptions vector
**
** \param   timer - pointer to timer which initiated this call
**
** \return  None
**
**************************************************************************/
void DeleteExpiredSubscriptions(void)
{
    int i;
    int err;
    subs_t *sub;
    dm_trans_vector_t trans;
    time_t cur_time;
    char path[MAX_DM_PATH];
    bool subscription_expired;

    // Iterate over all enabled subscriptions, seeing if any have expired
    cur_time = time(NULL);
    subscription_expired = false;
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if (cur_time >= sub->expiry_time)
        {
            subscription_expired = true;
            break;
        }
    }

    // Exit if no subscriptions have expired
    if (subscription_expired == false)
    {
        return;
    }

    // As we are deleting some subscriptions (from the data model and DB), wrap in a transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Iterate over all enabled subscriptions, deleting each one that has expired
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];

        // If this subscription has expired, then delete it from the Data model
        if (cur_time >= sub->expiry_time)
        {
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_subs_root, sub->instance);
            DATA_MODEL_DeleteInstance(path, 0);        // NOTE: This will cascade to delete from subscriptions vector via the delete hook callback
            // Intentionally ignoring errors
        }
    }

    // The commit will cascade to delete the subscription from the subscriptions vector
    DM_TRANS_Commit();
}

/*********************************************************************//**
**
** DeleteNonPersistentSubscriptions
**
** Called after bootup, to delete all non-persistent subscriptions from the data model
** and from the subscriptions vector
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DeleteNonPersistentSubscriptions(void)
{
    int i;
    subs_t *sub;
    int err;
    dm_trans_vector_t trans;
    time_t cur_time;
    char path[MAX_DM_PATH];

    // As we might delete some subscriptions (from the data model and DB), wrap in a transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Remove any expired or non-persistent subscriptions
    // NOTE: We do this after adding all subscriptions to the subscriptions vector, in order
    // that notify events are sent when the subscription is deleted (if an Object Deletion subscription
    // is present on the Subscriptions table)
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];

        // Remove this subscription if it is not marked as persistent, or if it has expired
        cur_time = time(NULL);
        if ((sub->persistent == false) || (cur_time >= sub->expiry_time))
        {
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_subs_root, sub->instance);
            err = DATA_MODEL_DeleteInstance(path, 0); // NOTE: This will cascade to delete from subscriptions vector via the delete hook callback
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

exit:
    // The commit will cascade to delete the subscription from the subscriptions vector
    DM_TRANS_Commit();

    return err;
}

#ifndef REMOVE_DEVICE_BOOT_EVENT
/*********************************************************************//**
**
** ProcessAllBootSubscriptions
**
** Called to send a Boot NotifyRequest for all controllers that have subscribed to it
**
** \param   timer - pointer to timer which initiated this call
**
** \return  None
**
**************************************************************************/
void ProcessAllBootSubscriptions(void)
{
    int i;
    subs_t *sub;
    reboot_info_t info;
    char *firmware_updated;

    // Determine common arguments for Boot! event
    DEVICE_LOCAL_AGENT_GetRebootInfo(&info);
    firmware_updated = (info.is_firmware_updated) ? "true" : "false";

    // Iterate over all enabled subscriptions, processing each boot event subscription that matches
    // (there may be more than one subscriber)
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_Event))
        {
            // Send the event, if it matches this subscription
            if (DoesSubscriptionSendNotification(sub, (char *)device_boot_event))
            {
                SendBootNotify(sub, info.command_key, info.cause, firmware_updated);
            }
        }
    }
}
#endif

/*********************************************************************//**
**
** ProcessObjectLifeEventSubscription
**
** Process a single life event subscription, seeing if it matches any of the recent object life events
** If it does, then send a USP notification message
**
** \param   sub - subscription to match the object life events against to see if it fires any notifications
**
** \return  None
**
**************************************************************************/
void ProcessObjectLifeEventSubscription(subs_t *sub)
{
    int i;
    int index;
    Usp__Msg *req = NULL;
    obj_life_event_t *ole;

#ifdef DEBUG_OBJECT_NOTIFICATIONS
    // Extra debug logs
    USP_LOG_DebugNotifications("----------------------------------");
    USP_LOG_DebugNotifications("%s(instance=%d, skip_obj_notifications=%d)", __FUNCTION__, sub->instance, sub->skip_obj_notifications);
    for (i=0; i < object_life_events.num_entries; i++)
    {
        ole = &object_life_events.vector[i];
        switch (ole->notify_type)
        {
            case kSubNotifyType_ObjectCreation:
                USP_LOG_DebugNotifications("ObjectLifeEvent: Creation(%s)", ole->obj_path);
                break;

            case kSubNotifyType_ObjectDeletion:
                USP_LOG_DebugNotifications("ObjectLifeEvent: Deletion(%s)", ole->obj_path);
                break;

            default:
                TERMINATE_BAD_CASE(ole->notify_type);   // Since ole->notify_type is only either object creation or deletion
                break;
        }
    }

    USP_LOG_DebugNotifications("cur_watch_objs");
    STR_VECTOR_Dump(&sub->cur_watch_objs);
    USP_LOG_DebugNotifications("last_watch_objs");
    STR_VECTOR_Dump(&sub->last_watch_objs);
    USP_LOG_DebugNotifications("End");
#endif


    // Exit if we are not sending notifications from this subscription this USP message processing period
    // NOTE: We do this if this subscription has just been enabled, and the object lifecycle events could have been generated when
    // obtaining a baseline set of objects (and hence should not fire any notifications from this subscription, but may fire notifications from others)
    if (sub->skip_obj_notifications)
    {
        sub->skip_obj_notifications = false; // Reset the flag, so that the subscription will fire notifications subsequently
        return;
    }

    // Exit if no object life events occurred of the type this subscription is acting upon
    if ( ((sub->notify_type == kSubNotifyType_ObjectCreation) && (object_life_events.num_creation==0)) ||
         ((sub->notify_type == kSubNotifyType_ObjectDeletion) && (object_life_events.num_deletion==0)) )
    {
        return;
    }

    // Iterate over all object life events which have occurred recently
    for (i=0; i < object_life_events.num_entries; i++)
    {
        // If this object life event matches an object referenced by this subscription,
        // then send a notification to the subscribing controller
        ole = &object_life_events.vector[i];
        if (ole->notify_type == sub->notify_type)
        {
            switch (ole->notify_type)
            {
                case kSubNotifyType_ObjectCreation:
                    index = STR_VECTOR_Find(&sub->cur_watch_objs, ole->obj_path);
                    if (index != INVALID)
                    {
                        req = MSG_HANDLER_CreateNotifyReq_ObjectCreation(ole->obj_path, sub->subscription_id, sub->notification_retry);
                    }
                    break;

                case kSubNotifyType_ObjectDeletion:
                    // NOTE: Deletion matches against the current set of objects resolved and the last set, because the current set may
                    // have been resolved after the deletion occured, and hence may not include the object which got deleted
                    index = STR_VECTOR_Find(&sub->cur_watch_objs, ole->obj_path);
                    if (index == INVALID)
                    {
                        index = STR_VECTOR_Find(&sub->last_watch_objs, ole->obj_path);
                    }

                    if (index != INVALID)
                    {
                        req = MSG_HANDLER_CreateNotifyReq_ObjectDeletion(ole->obj_path, sub->subscription_id, sub->notification_retry);
                    }
                    break;

                default:
                    TERMINATE_BAD_CASE(ole->notify_type);   // Since ole->notify_type is only either object creation or deletion
                    break;
            }

            // Send the Notify Request, if one was created
            if (req != NULL)
            {
                SendNotify(req, sub, ole->obj_path);
                usp__msg__free_unpacked(req, pbuf_allocator);
                req = NULL;
            }
        }
    }
}

/*********************************************************************//**
**
** ProcessAllValueChangeSubscriptions
**
** Called to Periodically poll all value change notifications
**
** \param   None
**
** \return  None
**
**************************************************************************/
void ProcessAllValueChangeSubscriptions(void)
{
    int i;
    subs_t *sub;

    // Iterate over all enabled subscriptions, processing each value change subscription
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_ValueChange))
        {
            ProcessValueChangeSubscription(sub);
        }
    }
}

/*********************************************************************//**
**
** ProcessValueChangeSubscription
**
** Processes one enabled subscription for value change
**
** \param   sub - pointer to subscription to poll
**
** \return  None
**
**************************************************************************/
void ProcessValueChangeSubscription(subs_t *sub)
{
    int i;
    kv_vector_t cur_values;
    kv_pair_t *pair;
    int index;
    int hint_index;
    char *value;

    // Get the current values of all parameters associated with this subscription
    GetAllPathExpressionParameterValues(sub, &sub->path_expressions, &sub->handler_group_ids, &cur_values);

    // Determine whether any of the values have changed from last time
    hint_index = 0;
    for (i=0; i < cur_values.num_entries; i++)
    {
        pair = &cur_values.vector[i];

        // Find the index in the last_values vector matching this parameter
        // NOTE: We pass in a hint based on where we expect to find the matching parameter
        // This hint will be a perfect match if the list of parameters generated by the path expressions have not changed since last time
        index = KV_VECTOR_FindKey(&sub->last_values, pair->key, hint_index);
        if (index != INVALID)
        {
            hint_index = index + 1;         // Calculate index for next hint
            value = sub->last_values.vector[index].value;

            if (strcmp(value, pair->value) != 0)
            {
                // The value has changed since last time, so send a Value Change NotifyRequest
                SendValueChangeNotify(sub, pair->key, pair->value);
            }
        }
        else
        {
            // If we do not have a value for the parameter from last time, then this does not trigger a value change
        }

    }

    // Finally, replace the last set of values with the current set
    KV_VECTOR_Destroy(&sub->last_values);
    memcpy(&sub->last_values, &cur_values, sizeof(kv_vector_t));
}

/*********************************************************************//**
**
** GetAllPathExpressionParameterValues
**
** Gets all of the parameters and their values associated with a set of path expressions
** NOTE: Parameters which are handled by a vendor-layer subscription do not appear in the output list
**
** \param   sub - pointer to subscription
** \param   path_expressions - vector of path expressions to get the values of
** \param   handler_group_ids - vector noting which of the path_expressions are handled by a vendor-layer subscription,
**                    or NULL if none of the path expressions are handled by a vendor-layer subscription
** \param   param_values - vector in which parameter values are returned (key=parameter name, value=parameter value)
**                         NOTE: This function overwrites any contents in this vector
**
** \return  None
**
**************************************************************************/
void GetAllPathExpressionParameterValues(subs_t *sub, str_vector_t *path_expressions, int_vector_t *handler_group_ids, kv_vector_t *param_values)
{
    str_vector_t params;
    int_vector_t group_ids;
    group_get_vector_t ggv;

    // Form a vector list containing all the parameters to get the value of (and their associated group_id)
    STR_VECTOR_Init(&params);
    ResolveAllPathExpressions(sub->instance, path_expressions, handler_group_ids, &params, &group_ids, kResolveOp_SubsValChange, sub->cont_instance);

    // Exit if there are no parameters to get
    // This could be the case if all parameters are being handled by a subscription in the vendor layer
    if (params.num_entries == 0)
    {
        KV_VECTOR_Init(param_values);
        return;
    }

    // Add the parameters to get to the group get vector
    GROUP_GET_VECTOR_Init(&ggv);
    GROUP_GET_VECTOR_AddParams(&ggv, &params, &group_ids);

    // Destroy the params and group_ids vectors (since their contents have been moved to the group get vector)
    USP_SAFE_FREE(params.vector);
    INT_VECTOR_Destroy(&group_ids);

    // Get the values of all the parameters
    GROUP_GET_VECTOR_GetValues(&ggv);

    // Convert to key-value pair vector, destroying the group get vector in the process
    GROUP_GET_VECTOR_ConvertToKeyValueVector(&ggv, param_values);
}

/*********************************************************************//**
**
** ResolveAllPathExpressions
**
** Creates a single list of resolved paths, given a list of path expressions
** NOTE: Parameters which are handled by a vendor-layer subscription do not appear in the list
**
** \param   sub_instance - Instance number of the subscription in Device.LocalAgent.Subscription.{i}. Used only for debug.
** \param   path_expressions - list of path expressions to resolve
** \param   handler_group_ids - vector noting which of the path_expressions are handled by a vendor-layer subscription,
**                    or NULL if none of the path expressions are handled by a vendor-layer subscription
** \param   resolved_paths - pointer to string vector in which to return all resolved path expressions.
**                           or NULL if we are only interested in whether the expression exists in the schema
**                           NOTE: The caller must ensure that the string vector is initialised (if non NULL)
**                                 This function will free any existing content in the string vector
** \param   group_ids - pointer to vector in which to return the group_id of the parameters
**                      or NULL if the caller is not interested in this
**                      NOTE: values in resolved_paths and group_ids relate by index
** \param   op - Operation being performed
** \param   cont_instance - Controller Instance number - used to determine the role to use for the recipient controller
**
** \return  None
**
**************************************************************************/
void ResolveAllPathExpressions(int subs_instance, str_vector_t *path_expressions, int_vector_t *handler_group_ids, str_vector_t *resolved_paths, int_vector_t *group_ids, resolve_op_t op, int cont_instance)
{
    char *expr;
    int i;
    int err;
    combined_role_t combined_role;

    USP_ASSERT((handler_group_ids==NULL) || (path_expressions->num_entries == handler_group_ids->num_entries));

    // Default to no resolved paths, ensuring that if resolved_paths contains any paths, they are freed
    if (resolved_paths != NULL)
    {
        STR_VECTOR_Destroy(resolved_paths);
    }

    // Default to no group_ids
    if (group_ids != NULL)
    {
        INT_VECTOR_Init(group_ids);
    }

    // Exit if we cannot retrieve the role to use for this endpoint
    err = DEVICE_CONTROLLER_GetCombinedRoleByInstance(cont_instance, &combined_role);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Form a vector list containing all the resolved parameters or object instances
    for (i=0; i < path_expressions->num_entries; i++)
    {
        // Skip this path expression, if it is handled by a vendor-layer subscription
        if ((handler_group_ids != NULL) && (handler_group_ids->vector[i] != NON_GROUPED))
        {
            continue;
        }

        expr = path_expressions->vector[i];
        PATH_RESOLVER_ResolveDevicePath(expr, resolved_paths, group_ids, op, FULL_DEPTH, &combined_role, DONT_LOG_RESOLVER_ERRORS);
        // NOTE: Ignoring any errors. Errors are possible if the vendor layer returns any error
        // eg due to an instances cache mismatch
    }
}

/*********************************************************************//**
**
** SendValueChangeNotify
**
** Creates, then sends a value change notify request message
**
** \param   sub - pointer to subscription to poll
** \param   path - data model path of parameter which has changed value
** \param   value - value of data model parameter (that has changed)
**
** \return  None
**
**************************************************************************/
void SendValueChangeNotify(subs_t *sub, char *path, char *value)
{
    Usp__Msg *req;

    // Form the ValueChange NotifyRequest message as a protobuf structure
    req = MSG_HANDLER_CreateNotifyReq_ValueChange(path, value, sub->subscription_id, sub->notification_retry);

    // Send the Notify Request
    SendNotify(req, sub, path);
    usp__msg__free_unpacked(req, pbuf_allocator);
}

#if !defined(REMOVE_DEVICE_BOOT_EVENT) || !defined(REMOVE_USP_BROKER)
/*********************************************************************//**
**
** SendBootNotify
**
** Sends a Boot notify request message
**
** \param   sub - pointer to boot subscription
** \param   command_key - pointer to string containing the CommandKey argument to put into the Boot! event
** \param   reboot_cause - pointer to string containing the Cause argument to put into the Boot! event
** \param   firmware_updated - pointer to string containing the FirmwareUpdated argument to put into the Boot! event
**          NOTE: The string arguments may be NULL if the USP Broker was unable to extract the argument from the USP Service's Boot! event
**
** \return  None
**
**************************************************************************/
void SendBootNotify(subs_t *sub, char *command_key, char *reboot_cause, char *firmware_updated)
{
    int err;
    int i;
    int_vector_t iv;
    Usp__Msg *req;
    char path[MAX_DM_PATH];
    char controller[MAX_DM_PATH];
    char expr[MAX_DM_PATH];
    int instance;                           // Instance number for BootParameter data model object
    bool enable;
    str_vector_t path_expr;
    kv_vector_t param_values;
    kv_vector_t event_params;
    char *json_object;

    // Add the cause (and associated command_key) of the last reboot
    KV_VECTOR_Init(&event_params);
    if (command_key != NULL)
    {
        USP_ARG_Add(&event_params, "CommandKey", command_key);
    }

    if (reboot_cause != NULL)
    {
        USP_ARG_Add(&event_params, "Cause", reboot_cause);
    }

    if (firmware_updated != NULL)
    {
        USP_ARG_Add(&event_params, "FirmwareUpdated", firmware_updated);
    }

    // Exit if unable to get the name of the controller table entry. This might be empty if the controller was deleted.
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Recipient", device_subs_root, sub->instance);
    err = DATA_MODEL_GetParameterValue(path, controller, sizeof(controller), 0);
    if ((err != USP_ERR_OK) || (controller[0] == '\0'))
    {
        return;
    }

    // Exit if unable to get the object instance numbers present in the boot parameters table
    USP_SNPRINTF(path, sizeof(path), "%s.BootParameter", controller);
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Form a list of enabled path expressions to get by reading them from the BootParameters table
    STR_VECTOR_Init(&path_expr);
    for (i=0; i < iv.num_entries; i++)
    {
        instance = iv.vector[i];

        // Skip this parameter if not enabled
        USP_SNPRINTF(path, sizeof(path), "%s.BootParameter.%d.Enable", controller, instance);
        err = DM_ACCESS_GetBool(path, &enable);
        if ((err != USP_ERR_OK) || (enable == false))
        {
            continue;
        }

        // Skip this path expression, if unable to get it
        USP_SNPRINTF(path, sizeof(path), "%s.BootParameter.%d.ParameterName", controller, instance);
        err = DATA_MODEL_GetParameterValue(path, expr, sizeof(expr), 0);
        if (err != USP_ERR_OK)
        {
            continue;
        }

        // Add the path expression to the list of path expressions to get
        STR_VECTOR_Add(&path_expr, expr);
    }

    // Get the values of all parameters specified by the list of path expressions into the param_values vector
    GetAllPathExpressionParameterValues(sub, &path_expr, NULL, &param_values);
    STR_VECTOR_Destroy(&path_expr);

    // Create a JSON object containing the boot params (and associated values)
    json_object = SerializeToJSONObject(&param_values);

    // Add the JSON Object as the value of the 'ParameterMap' argument
    USP_ARG_Add(&event_params, "ParameterMap", json_object);
    KV_VECTOR_Destroy(&param_values);
    free(json_object);

    // Form the Boot notify event message as a protobuf structure
    req = MSG_HANDLER_CreateNotifyReq_Event("Device.Boot!", &event_params, sub->subscription_id, sub->notification_retry);

    KV_VECTOR_Destroy(&event_params);

    // Send the Notify Request
    USP_SNPRINTF(path, sizeof(path), "%s.%d", device_subs_root, sub->instance);
    SendNotify(req, sub, path);
    usp__msg__free_unpacked(req, pbuf_allocator);

exit:
    INT_VECTOR_Destroy(&iv);
}
#endif

/*********************************************************************//**
**
** SerializeToJSONObject
**
** Serialises the specified parameter values to a JSON format object
** NOTE: The parameter values must already be in JSON format
**
** \param   param_values - key-value vector containing a list of parameters and their associated values in JSON format
**
** \return  pointer to dynamically allocated buffer containing the JSON format object
**
**************************************************************************/
char *SerializeToJSONObject(kv_vector_t *param_values)
{
    JsonNode *top;          // top of report
    double value_as_number;
    long long value_as_ll;
    unsigned long long value_as_ull;
    bool value_as_bool;
    int i;
    kv_pair_t *kv;
    int err;
    char *buf;
    char param_type;

    top = json_mkobject();

    // Iterate over each parameter, adding it to the json object. Take account of the parameter's type
    for (i=0; i < param_values->num_entries; i++)
    {
        kv = &param_values->vector[i];
        param_type = DATA_MODEL_GetJSONParameterType(kv->key);
        switch (param_type)
        {
            case 'S':
                json_append_member(top, kv->key, json_mkstring(kv->value) );
                break;

            case 'U':
                value_as_ull = strtoull(kv->value, NULL, 10);
                json_append_member(top, kv->key, json_mkulonglong(value_as_ull) );
                break;

            case 'L':
                value_as_ll = strtoll(kv->value, NULL, 10);
                json_append_member(top, kv->key, json_mklonglong(value_as_ll) );
                break;

            case 'N':
                value_as_number = atof(kv->value);
                json_append_member(top, kv->key, json_mknumber(value_as_number) );
                break;

            case 'B':
                err = TEXT_UTILS_StringToBool(kv->value, &value_as_bool);
                if (err == USP_ERR_OK)
                {
                    json_append_member(top, kv->key, json_mkbool(value_as_bool) );
                }
                break;

            default:
                USP_ASSERT(false);
                break;
        }
    }

    // Serialize the JSON tree
    buf = json_stringify(top, NULL);

    // Clean up the JSON tree
    json_delete(top);        // Other JsonNodes which are children of this top level tree will be deleted

    return buf;
}

/*********************************************************************//**
**
** SendOperationCompleteNotify
**
** Sends an operation complete notify request message
**
** \param   sub - pointer to subscription that caused this notify to be triggered
** \param   command - path to operation in the data model
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   err_code - error code of the operation (USP_ERR_OK indicates success)
** \param   err_msg - error message if the operation failed
** \param   output_args - results of the completed operation (if successful). NULL indicates no output arguments.
**
** \return  None
**
**************************************************************************/
void SendOperationCompleteNotify(subs_t *sub, char *command, char *command_key, int err_code, char *err_msg, kv_vector_t *output_args)
{
    Usp__Msg *req;

    // Form the Operation Complete NotifyRequest message as a protobuf structure
    if (err_code == USP_ERR_OK)
    {
        req = MSG_HANDLER_CreateNotifyReq_OperCompleteSuccess(output_args, command, command_key,
                                                              sub->subscription_id, sub->notification_retry);
    }
    else
    {
        req = MSG_HANDLER_CreateNotifyReq_OperCompleteFailure(err_code, err_msg, command, command_key,
                                                              sub->subscription_id, sub->notification_retry);
    }

    // Send the Notify Request
    SendNotify(req, sub, command);
    usp__msg__free_unpacked(req, pbuf_allocator);
}

/*********************************************************************//**
**
** SendNotify
**
** Sends the specified notify request message
**
** \param   req - pointer to USP notify request message to send. This is always freed by the caller (not this function)
** \param   sub - pointer to subscription that caused this notify to be triggered
** \param   path - data model path of parameter, operation or event which we are notifying
**
** \return  None
**
**************************************************************************/
void SendNotify(Usp__Msg *req, subs_t *sub, char *path)
{
    unsigned char *pbuf;
    int pbuf_len;
    int size;
    time_t cur_time;
    time_t retry_expiry_time;
    char *msg_id;
    char *dest_endpoint;
    mtp_conn_t mtp_conn = {0};  // Ensures mtp_conn.is_reply_to_specified=false
    usp_send_item_t usp_send_item;

    // Exit if unable to determine the endpoint of the controller
    // This could occur if the controller had been deleted
    dest_endpoint = DEVICE_CONTROLLER_FindEndpointIdByInstance(sub->cont_instance);
    if (dest_endpoint == NULL)
    {
        return;
    }

    // Serialize the protobuf structure into a binary format buffer
    pbuf_len = usp__msg__get_packed_size(req);
    pbuf = USP_MALLOC(pbuf_len);
    size = usp__msg__pack(req, pbuf);
    USP_ASSERT(size == pbuf_len);          // If these are not equal, then we may have had a buffer overrun, so terminate

    USP_LOG_Info("Sending NotifyRequest (%s for path=%s)", TEXT_UTILS_EnumToString(sub->notify_type, notify_types, NUM_ELEM(notify_types)), path);

    // Determine the time at which we should give up retrying, or expire the message in the MTP's send queue
    retry_expiry_time = END_OF_TIME;       // default to never expire
    if ((sub->notification_retry) && (sub->retry_expiry_period != 0))
    {
        cur_time = time(NULL);
        retry_expiry_time = cur_time + sub->retry_expiry_period;
    }

    // Marshal parameters to pass to MSG_HANDLER_QueueUspRecord()
    MSG_HANDLER_UspSendItem_Init(&usp_send_item);
    usp_send_item.usp_msg_type = USP__HEADER__MSG_TYPE__NOTIFY;
    usp_send_item.msg_packed = pbuf;
    usp_send_item.msg_packed_size = pbuf_len;
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
    usp_send_item.curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByInstance(sub->cont_instance);
    usp_send_item.usp_msg = req;
#endif

    // Send the message
    // NOTE: Intentionally ignoring error here. If the controller has been disabled or deleted, then
    // allow the subs retry code to remove any previous attempts from the retry array
    MSG_HANDLER_QueueUspRecord(&usp_send_item, dest_endpoint, req->header->msg_id, &mtp_conn, retry_expiry_time);

    // If the message should be retried until a NotifyResponse is received, then...
    if (sub->notification_retry)
    {
        // Add this message to the list of notification requests to retry
        // NOTE: Ownership of the serialized USP message passes to the subs retry module
        msg_id = req->header->msg_id;
        SUBS_RETRY_Add(sub->instance, msg_id, sub->subscription_id, dest_endpoint, path,
                       pbuf, pbuf_len, retry_expiry_time);
    }
    else
    {
        // Free the serialized USP Message because it is now encapsulated in USP Record messages.
        USP_FREE(pbuf);
    }
}

#ifndef REMOVE_DEVICE_BOOT_EVENT
/*********************************************************************//**
**
** SeedLastValueChangeValues
**
** Called at bootup to seed all ValueChange subscriptions with values from before the current boot cycle
** This function ensures that a ValueChange will fire on SoftwareVersion changing across a power cycle
** Currently no other parameter in the data model works this way (nor is intended to).
**
** \param   None
**
** \return  None
**
**************************************************************************/
void SeedLastValueChangeValues(void)
{
    int i;
    subs_t *sub;
    reboot_info_t info;

    // Get the last software version
    DEVICE_LOCAL_AGENT_GetRebootInfo(&info);

    // Iterate over all enabled subscriptions, replacing the initial value of SoftwareVersion with the value
    // before the current boot cycle
    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if ((sub->enable) && (sub->notify_type == kSubNotifyType_ValueChange))
        {
            KV_VECTOR_Replace(&sub->last_values, "Device.DeviceInfo.SoftwareVersion", info.last_software_version);
        }
    }
}
#endif

/*********************************************************************//**
**
** DoesSubscriptionSendNotification
**
** Determines whether the specified subscription matches the specified operation/event
**
** \param   sub - pointer to subscription to match
** \param   event_name - path of operation/event in the data model that has occurred
**
** \return  true if the specified subscription
**
**************************************************************************/
bool DoesSubscriptionSendNotification(subs_t *sub, char *event_name)
{
    bool send_notification;

    if (STR_VECTOR_Find(&sub->path_expressions, dm_root) == INVALID)
    {
        // Normal case (not a subscription on 'Device.') - Resolve the subscription path and see if any of the resolved paths match
        send_notification = DoesSubscriptionMatchEvent(sub, event_name);
    }
    else
    {
        // If the subscription is for 'Device.' then the subscription will fire if
        // the controller has permissions for the specified event
        send_notification = HasControllerGotNotificationPermission(sub->cont_instance, event_name, PERMIT_SUBS_EVT_OPER_COMP);
    }

    return send_notification;
}

/*********************************************************************//**
**
** DoesSubscriptionMatchEvent
**
** Determines whether the specified subscription is for the specified operation/event
** by forming a list of all data model paths that the subscription identifies
** and seeing if the specified path exists in this list
**
** \param   sub - pointer to subscription to match
** \param   event_name - path of operation/event in the data model that has occurred
**
** \return  true if the specified subscription
**
**************************************************************************/
bool DoesSubscriptionMatchEvent(subs_t *sub, char *event_name)
{
    resolve_op_t op;
    str_vector_t resolved_paths;
    int index;

#ifndef REMOVE_USP_BROKER
    // On a USP Broker, use a simpler way of matching the path expression
    // Why ? If the specified path is not present in the data model, then it would fail to match using the NORMAL method.
    // But we need to be able to send OperationComplete events for all USP async commands provided by a USP Service,
    // that were interrupted by a power cycle. These async commands will not be in the data model at startup because
    // the USP Service has not registered (and may never register).
    // Also during handling a deregistration, we do not want to use the NORMAL method, as that might need to refresh the instances
    // of the object being deregistered, in order to resolve the paths that the subscription references
    if ((sub->notify_type == kSubNotifyType_OperationComplete) || (sub->notify_type == kSubNotifyType_Event))
    {
        int i;
        char *path_spec;

        // Iterate over all path expressions, seeing if the USP Command matches the path expression
        // NOTE: This simpler method is only possible because we limit the path expressions for OperComplete & Events
        // to only absolute or wildcarded in a USP Broker. If the path expression is more complicated than this, then the NORMAL method is used
        for (i=0; i < sub->path_expressions.num_entries; i++)
        {
            path_spec = sub->path_expressions.vector[i];
            if (TEXT_UTILS_IsPathMatch(event_name, path_spec))
            {
                int err;
                combined_role_t combined_role;
                unsigned flags;
                unsigned short permission_bitmask;

                // Exit if unable to determine the role of the controller
                // This could occur if the controller had been deleted
                err = DEVICE_CONTROLLER_GetCombinedRoleByInstance(sub->cont_instance, &combined_role);
                if (err != USP_ERR_OK)
                {
                    return false;
                }

                // Exit if the path does not exist in the schema
                // This could occur at startup, if the previous power cycle had started an async command but not completed it
                // The Controller will have permission to send the notification in this case, because we prevent a controller
                // from setting an OperateComplete subscription if they do not have permission to receive the notification
                flags = DATA_MODEL_GetPathProperties(event_name, &combined_role, &permission_bitmask, NULL, NULL, 0);
                if ((flags & PP_EXISTS_IN_SCHEMA)==0)
                {
                    return true;
                }

                // Exit if the controller does not have permission to receive the notification
                if ((permission_bitmask & PERMIT_SUBS_EVT_OPER_COMP)==0)
                {
                    return false;
                }

                // If the code gets here, then the controller has permission to receive the notification
                return true;
            }
        }
    }
#endif

    // NORMAL method for determining whether the subscription matches the specified event
    // Determine the operation to be resolved by the path resolver
    USP_ASSERT((sub->notify_type == kSubNotifyType_OperationComplete) || (sub->notify_type == kSubNotifyType_Event));
    op = (sub->notify_type == kSubNotifyType_Event) ? kResolveOp_SubsEvent : kResolveOp_SubsOper;

    // Resolve a list of paths that the subscription references in the supported data model
    // NOTE: Resolution excludes paths for which the controller does not have permission to be notified of events
    STR_VECTOR_Init(&resolved_paths);
    ResolveAllPathExpressions(sub->instance, &sub->path_expressions, &sub->handler_group_ids, &resolved_paths, NULL, op, sub->cont_instance);

    // Exit if the specified path is present in the subscription
    index = STR_VECTOR_Find(&resolved_paths, event_name);
    STR_VECTOR_Destroy(&resolved_paths);
    if (index != INVALID)
    {
        return true;
    }

    // If the code gets here, then the subscription did not match
    return false;
}

/*********************************************************************//**
**
** HasControllerGotNotificationPermission
**
** Determines whether the specified controller has permission to send the specified notification
**
** \param   instance - instance number of the controller in Device.LocalAgent.Controller.{i}
** \param   path - data model path on which the notification has occurred
** \param   mask - bitmask identifying permission which we want to check (and type of notification which occurred)
**
** \return  true if the controller has permission to send the notification
**
**************************************************************************/
bool HasControllerGotNotificationPermission(int cont_instance, char *path, unsigned short mask)
{
    combined_role_t combined_role;
    unsigned short perm;
    int err;

    // Exit if unable to determine role used by the controller that set this subscription
    // NOTE: This could occur if the controller doesn't exist in the controller table anymore
    err = DEVICE_CONTROLLER_GetCombinedRoleByInstance(cont_instance, &combined_role);
    if (err != USP_ERR_OK)
    {
        return false;
    }

    // Determine permissions that this controller has for the notification that occurred
    // NOTE: Ignoring error message. If the path is not present in the data model, then the controller will not have permissions anyway
    DATA_MODEL_GetPermissions(path, &combined_role, &perm, 0);

    // Exit if controller has permission for this notification
    if (perm & mask)
    {
        return true;
    }

    // If the code gets here, then the controller did not have permission for the notification
    return false;
}

/*********************************************************************//**
**
** RefreshInstancesForObjLifetimeSubscriptions
**
** This function calls the refresh_instances callback for all top-level multi-instance objects that require it
** and which have an object creation/deletion subscription against them
**
** \param   None
**
** \return  None
**
**************************************************************************/
void RefreshInstancesForObjLifetimeSubscriptions(void)
{
    // Simply resolving the path expressions for ObjectCreation/Deletion subscriptions will result in the refresh_instances callback being called if necessary
    // (because the path resolver checks whether the instance being resolved exists, or gets the instances to resolve a wildcard etc)
    // And when instances are refreshed, the code automatically determines if any have been added or deleted

    // NOTE: We force the refresh instances vendor hook to always be called, because otherwise if there is an instances cache mismatch
    // then the set of objects to match against for deletion subscriptions will resolve to empty, and when the instances cache does
    // eventually expire, and generate object life events, the set to match against will already be empty, thus object
    // deletion events will be missed unless we force the refresh instances vendor hook to always be called
    DM_INST_VECTOR_SetRefreshOverride(true);
    DEVICE_SUBSCRIPTION_ResolveObjectCreationPaths();
    DEVICE_SUBSCRIPTION_ResolveObjectDeletionPaths();
    DM_INST_VECTOR_SetRefreshOverride(false);
}

/*********************************************************************//**
**
** FindSubsByInstance
**
** Finds the subscription entry matching the specified instance number
**
** \param   instance - instance number in Device.LocalAgent.Subscription table
**
** \return  None
**
**************************************************************************/
subs_t *FindSubsByInstance(int instance)
{
    int i;
    subs_t *sub;

    for (i=0; i < subscriptions.num_entries; i++)
    {
        sub = &subscriptions.vector[i];
        if (sub->instance == instance)
        {
            return sub;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** ExtractNotificationEventArg
**
** Returns the value of the specified argument in the specified event notification
**
** \param   event - pointer to parsed protobuf event structure
** \param   arg_name - name of argument
**
** \return  value of the specified argument or NULL if the argument was not present in the parsed protobuf
**
**************************************************************************/
char *ExtractNotificationEventArg(Usp__Notify__Event *event, char *arg_name)
{
    int i;
    Usp__Notify__Event__ParamsEntry *arg;

    // Iterate over all arguments for the event, finding the one which matches
    for (i=0; i < event->n_params; i++)
    {
        arg = event->params[i];
        if (strcmp(arg->key, arg_name)==0)
        {
            return arg->value;
        }
    }

    // If the code gets here, then no arguments matched
    return NULL;
}


