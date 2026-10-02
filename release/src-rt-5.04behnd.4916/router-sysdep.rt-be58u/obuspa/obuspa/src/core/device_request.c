/*
 *
 * Copyright (C) 2019-2023, Broadband Forum
 * Copyright (C) 2017-2023  CommScope, Inc
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
 * \file device_request.c
 *
 * Implements data model parameters for the Device.LocalAgent.Request.{i} table
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common_defs.h"
#include "data_model.h"
#include "device.h"
#include "usp_api.h"
#include "dm_access.h"
#include "dm_trans.h"
#include "dm_inst_vector.h"
#include "msg_handler.h"


//------------------------------------------------------------------------------
// Location of the request object within the data model
#define DEVICE_REQ_ROOT "Device.LocalAgent.Request"
char *device_req_root = DEVICE_REQ_ROOT;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
bool IsRequestInstanceValid(int instance);
int RestartAsyncOperation(char *path, int instance);
int ReadOperationArgs(int instance, kv_vector_t *args, char *prefix);
int DeleteRequestArgs(dm_req_t *req);

/*********************************************************************//**
**
** DEVICE_REQUEST_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_REQUEST_Init(void)
{
    int err = USP_ERR_OK;


    err |= USP_REGISTER_Object(DEVICE_REQ_ROOT ".{i}",
                              USP_HOOK_DenyAddInstance, NULL, NULL,
                              USP_HOOK_DenyDeleteInstance, NULL, DeleteRequestArgs);
    err |= USP_REGISTER_Param_NumEntries("Device.LocalAgent.RequestNumberOfEntries", DEVICE_REQ_ROOT ".{i}");
    err |= USP_REGISTER_DBParam_Alias(DEVICE_REQ_ROOT ".{i}.Alias", NULL);

    err |= USP_REGISTER_DBParam_ReadOnly(DEVICE_REQ_ROOT ".{i}.Originator", "", DM_STRING);
    err |= USP_REGISTER_DBParam_ReadOnly(DEVICE_REQ_ROOT ".{i}.Command", "", DM_STRING);
    err |= USP_REGISTER_DBParam_ReadOnly(DEVICE_REQ_ROOT ".{i}.CommandKey", "", DM_STRING);
    err |= USP_REGISTER_DBParam_ReadOnly(DEVICE_REQ_ROOT ".{i}.Status", "", DM_STRING);

    char *unique_keys[] = { "Originator", "Command", "CommandKey" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_REQ_ROOT ".{i}", unique_keys, NUM_ELEM(unique_keys));

    // Arguments associated with Async Operations that have the RESTART_ON_REBOOT flag set
    // These objects shadow the objects in the request table - having the same instance number for the operation
    // The lifetime of these shadow objects match those of the object in the request table.
    err |= USP_REGISTER_Object("Internal.Request.{i}", NULL, NULL, NULL, NULL, NULL, NULL);
    err |= USP_REGISTER_Object("Internal.Request.{i}.InputArgs.{i}", NULL, NULL, NULL, NULL, NULL, NULL);
    err |= USP_REGISTER_DBParam_ReadWrite("Internal.Request.{i}.InputArgs.{i}.Name", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Internal.Request.{i}.InputArgs.{i}.Value", "", NULL, NULL, DM_STRING);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_REQUEST_Add
**
** Adds a new instance to the Request table
**
** \param   path - pointer to string representing the command
** \param   command_key - pointer to string used by controller to identify the operation in a notification
** \param   instance - pointer to variable in which to return the instance number of the request added to the table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_REQUEST_Add(char *path, char *command_key, int *instance)
{
    int err;
    char param[MAX_DM_PATH];
    char *originator;

    // Exit if unable to create a new row in the request table
    err = DATA_MODEL_AddInstance(device_req_root, instance, 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to write the command into the table
    USP_SNPRINTF(param, sizeof(param), "%s.%d.Command", device_req_root, *instance);
    err = DATA_MODEL_SetParameterValue(param, path, 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to write the command_key into the table
    USP_SNPRINTF(param, sizeof(param), "%s.%d.CommandKey", device_req_root, *instance);
    err = DATA_MODEL_SetParameterValue(param, command_key, 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to write the status into the table
    USP_SNPRINTF(param, sizeof(param), "%s.%d.Status", device_req_root, *instance);
    err = DATA_MODEL_SetParameterValue(param, "Requested", 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Ensure that there is a value for originator
    originator = MSG_HANDLER_GetMsgControllerEndpointId();
    if (originator == NULL)
    {
        // The following should never happen, but if it does, just log it and use an empty value for originator
        USP_LOG_Warning("%s: WARNING: Originator of operate request not known", __FUNCTION__);
        originator = "";
    }

    // Exit if unable to write the originator into the table
    USP_SNPRINTF(param, sizeof(param), "%s.%d.Originator", device_req_root, *instance);
    err = DATA_MODEL_SetParameterValue(param, originator, 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}


/*********************************************************************//**
**
** DEVICE_REQUEST_OperationComplete
**
** Handles an async operation completing, including sending events to subscribers
** and removing the operation from the request table
**
** \param   instance - instance number of operation in Device.LocalAgent.Request table
** \param   err_code - error code of the operation (USP_ERR_OK indicates success)
** \param   err_msg - error message if the operation failed, or NULL if operation was successful
** \param   output_args - results of the completed operation (if successful)
**
** \return  None - This code must handle any errors
**
**************************************************************************/
void DEVICE_REQUEST_OperationComplete(int instance, int err_code, char *err_msg, kv_vector_t *output_args)
{
    int err;
    char path[MAX_DM_PATH];
    dm_trans_vector_t trans;
    char command[MAX_DM_PATH];
    char command_key[MAX_DM_SHORT_VALUE_LEN];

    // Exit if unable to find this instance in the request table
    if (IsRequestInstanceValid(instance) == false)
    {
        // This should only ever happen, if an operation has been cancelled but the vendor had previously queued the operation complete
        USP_LOG_Error("%s: Ignoring OperationComplete for %s.%d (invalid instance)", __FUNCTION__, device_req_root, instance);
        return;
    }

    // Get the command for this instance from the request table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Command", device_req_root, instance);
    err = DATA_MODEL_GetParameterValue(path, command, sizeof(command), 0);
    if (err != USP_ERR_OK)
    {
        return;
    }

#ifdef VALIDATE_OUTPUT_ARG_NAMES
    if (output_args != NULL)
    {
        // Validate the names of the output arguments
        dm_node_t *node;
        dm_oper_info_t *info;

        node = DM_PRIV_GetNodeFromPath(command, NULL, NULL);
        USP_ASSERT(node != NULL);

        info = &node->registered.oper_info;
        err = KV_VECTOR_ValidateArguments(output_args, &info->output_args, NO_FLAGS);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Warning("%s: Output argument names do not match those registered (%s). Please check code.", __FUNCTION__, command);
        }
    }
#endif

    // Get the command_key for this instance from the request table
    USP_SNPRINTF(path, sizeof(path), "%s.%d.CommandKey", device_req_root, instance);
    err = DATA_MODEL_GetParameterValue(path, command_key, sizeof(command_key), 0);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Exit if unable to start a database transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Send operation complete events to all subscribers
    DEVICE_SUBSCRIPTION_ProcessAllOperationCompleteSubscriptions(command, command_key, err_code, err_msg, output_args);

    // Exit if unable to remove this operation from the request table
    // NOTE: Deletion of this instance will cascade to delete any persisted input args
    USP_SNPRINTF(path, sizeof(path), "%s.%d", device_req_root, instance);
    err = DATA_MODEL_DeleteInstance(path, 0);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Unable to delete %s after operation completed", __FUNCTION__, path);
        DM_TRANS_Abort();
        return;
    }

    // Commit the transaction
    DM_TRANS_Commit();
}

/*********************************************************************//**
**
** DEVICE_REQUEST_UpdateOperationStatus
**
** Updates the status of an operation in the request table
**
** \param   instance - instance number of operation in Device.LocalAgent.Request table
** \param   status - string to store in the request table
**
** \return  None - This code must handle any errors
**
**************************************************************************/
void DEVICE_REQUEST_UpdateOperationStatus(int instance, char *status)
{
    int err;
    char path[MAX_DM_PATH];
    dm_trans_vector_t trans;

    // Exit if unable to find this instance in the request table
    if (IsRequestInstanceValid(instance) == false)
    {
        // This should only ever happen, if an operation has been cancelled but the vendor had previously queued the operation status
        USP_LOG_Error("%s: Ignoring OperationStatus update message for %s.%d (invalid instance)", __FUNCTION__, device_req_root, instance);
        return;
    }

    // Exit if unable to start a database transaction
    err = DM_TRANS_Start(&trans);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Set the status of this operation
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Status", device_req_root, instance);
    err = DATA_MODEL_SetParameterValue(path, status, 0);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Commit the transaction
    DM_TRANS_Commit();
}

/*********************************************************************//**
**
** DEVICE_REQUEST_RestartAsyncOperations
**
** Called after bootup to restart all async operations that should be restarted
** Also any operations which required a reboot to complete send their completion events from here
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_REQUEST_RestartAsyncOperations(void)
{
    int err;
    int instance;   // instance in Device.LocalAgent.Request.{i}
    int i;
    int_vector_t inst;
    char path[MAX_DM_PATH];
    char op_path[MAX_DM_PATH];
    bool is_restart;
    int err_code;
    char err_msg[USP_ERR_MAXLEN];
    kv_vector_t output_args;

    // Exit if unable to get all async operation requests that were started last boot time
    KV_VECTOR_Init(&output_args);
    INT_VECTOR_Init(&inst);
    err = DATA_MODEL_GetInstances(device_req_root, &inst);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over all request instances
    for (i=0; i < inst.num_entries; i++)
    {
        instance = inst.vector[i];

        // Exit if unable to retrieve the name of the operation
        USP_SNPRINTF(path, sizeof(path), "%s.%d.Command", device_req_root, instance);
        err = DATA_MODEL_GetParameterValue(path, op_path, sizeof(op_path), 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Exit if unable to determine whether the operation should be restarted
        KV_VECTOR_Init(&output_args);       // Not strictly necessary
        err = DATA_MODEL_ShouldOperationRestart(op_path, instance, &is_restart, &err_code, err_msg, sizeof(err_msg), &output_args);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        if (is_restart)
        {
            // Attempt to restart the operation, sending an operation complete and removing the entry
            // from the Request table if this fails
            RestartAsyncOperation(op_path, instance);
        }
        else
        {
            // The operation should not be restarted, this means that it completed (either successfully or unsuccessfully)
            // So send an operation complete message, and remove the entry in the Request table
            DEVICE_REQUEST_OperationComplete(instance, err_code, err_msg, &output_args);
        }

        KV_VECTOR_Destroy(&output_args);
    }

    // If code gets here all requests were successfully restarted or finished (sending an operation complete)
    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&inst);
    KV_VECTOR_Destroy(&output_args);
    return err;
}

/*********************************************************************//**
**
** DEVICE_REQUEST_PersistOperationArgs
**
** Persists the Input or Output arguments associated with an Async operation
**
** \param   instance - Instance number in the Request table for the operation
** \param   args - pointer to vector containing the arguments to persist
** \parm    prefix - string specifying whether these are input or output arguments
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_REQUEST_PersistOperationArgs(int instance, kv_vector_t *args, char *prefix)
{
    int err;
    int i;
    kv_pair_t *kvv;
    char path[MAX_DM_PATH];

    // Exit if unable to create the shadow request instance object
    USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d", instance);
    err = DATA_MODEL_AddInstance(path, NULL, 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Iterate over all arguments
    for (i=0; i < args->num_entries; i++)
    {
        kvv = &args->vector[i];

        // Exit if unable to add these instances
        USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d.%sArgs.%d", instance, prefix, i+1);
        err = DATA_MODEL_AddInstance(path, NULL, 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if unable to persist the name of the argument
        USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d.%sArgs.%d.Name", instance, prefix, i+1);
        err = DATA_MODEL_SetParameterValue(path, kvv->key, 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if unable to persist the value of the argument
        USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d.%sArgs.%d.Value", instance, prefix, i+1);
        err = DATA_MODEL_SetParameterValue(path, kvv->value, 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_REQUEST_CountMatchingRequests
**
** Determines the number of times that the specified command is present in the Request table
** and hence already in progress
**
** \param   command_path - Schema path of the command in the data model which we want to count the number of occurrences of
**
** \return  Number of occurrences of the specified command in the request table, or INVALID if an error occurred
**
**************************************************************************/
int DEVICE_REQUEST_CountMatchingRequests(char *command_path)
{
    int_vector_t iv;
    dm_instances_t inst;
    bool is_qualified_instance;
    dm_node_t *command_node;
    dm_node_t *node;
    int err;
    int i;
    int instance;
    char path[MAX_DM_PATH];
    char buf[MAX_DM_VALUE_LEN];
    int count = 0;

    INT_VECTOR_Init(&iv);

    // Find node representing the specified command to find
    command_node = DM_PRIV_GetNodeFromPath(command_path, NULL, NULL);
    USP_ASSERT(command_node != NULL);

    // Find node representing the request table
    node = DM_PRIV_GetNodeFromPath(DEVICE_REQ_ROOT ".{i}", &inst, &is_qualified_instance);
    USP_ASSERT(node != NULL);

    // Get an array of instances for in the request table
    err = DM_INST_VECTOR_GetInstances(node, &inst, &iv);
    if (err != USP_ERR_OK)
    {
        count = INVALID;
        goto exit;
    }

    // Iterate over all instances in the request table
    for (i=0; i < iv.num_entries; i++)
    {
        // Form the param path containing the command for this instance in the table
        instance = iv.vector[i];
        USP_SNPRINTF(path, sizeof(path), "%s.%d.Command", DEVICE_REQ_ROOT, instance);

        // Exit if unable to get the command parameter for this instance
        err = DATA_MODEL_GetParameterValue(path, buf, sizeof(buf), 0);
        if (err != USP_ERR_OK)
        {
            count = INVALID;
            goto exit;
        }

        // Exit if unable to get the data model node associated with the command in this entry of the request table
        // NOTE: This should never happen. It could only happen if commands have been removed from the data model schema
        node = DM_PRIV_GetNodeFromPath(buf, NULL, NULL);
        if (node == NULL)
        {
            goto exit;
        }

        // Increase the count if the command in this instance of the table, matched the specified command
        if (node == command_node)
        {
            count++;
        }
    }

exit:
    INT_VECTOR_Destroy(&iv);
    return count;
}

/*********************************************************************//**
**
** IsRequestInstanceValid
**
** Determines whether the specified operation in Device.LocalAgent.Request.{i} is still present in the table
** (if it is not present, it may have been cancelled)
**
** \param   instance - instance number of operation in Device.LocalAgent.Request table
**
** \return  true if the instance number is still present in the table
**
**************************************************************************/
bool IsRequestInstanceValid(int instance)
{
    int i;
    int err;
    int_vector_t iv;

    // Exit if unable to get a list of the instance numbers in the request table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(device_req_root, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Iterate over all instance numbers in the request table, seeing if this
    // instance is present in the table (ie has not been cancelled)
    for (i=0; i < iv.num_entries; i++)
    {
        if (iv.vector[i] == instance)
        {
            INT_VECTOR_Destroy(&iv);
            return true;
        }
    }

exit:
    // If the code gets here, no match was found
    INT_VECTOR_Destroy(&iv);
    return false;
}

/*********************************************************************//**
**
** RestartAsyncOperation
**
** Restart the specified asynchronous operation
**
** \param   path - name of the operation in the data model
** \param   instance - Instance number in the Request table for the operation
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int RestartAsyncOperation(char *path, int instance)
{
    int err;
    kv_vector_t input_args;
    char err_msg[USP_ERR_MAXLEN];

    // Exit if unable to read the input arguments of the operation (which were persisted when it was first started)
    // NOTE: The persisted input arguments contain SAVED_TIME_REF_ARG_NAME, so that any delta time arguments can
    // be calculated relative to time at which the operate was received
    err = ReadOperationArgs(instance, &input_args, "Input");
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to restart the operation
    err = DATA_MODEL_RestartAsyncOperation(path, &input_args, instance);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

exit:
    KV_VECTOR_Destroy(&input_args);

    // Send an operation complete, if an error occurred when trying to restart the operation
    if (err != USP_ERR_OK)
    {
        USP_STRNCPY(err_msg, USP_ERR_GetMessage(), sizeof(err_msg));
        DEVICE_REQUEST_OperationComplete(instance, err, err_msg, NULL);
        return err;
    }

    return err;
}

/*********************************************************************//**
**
** ReadOperationArgs
**
** Reads the arguments of the operation from the USP database back into a key-value vector
**
** \param   instance - Instance number in the Request table for the operation
** \param   args - pointer to vector in which to return the arguments read from the USP database
** \parm    prefix - string specifying whether these are input or output arguments
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ReadOperationArgs(int instance, kv_vector_t *args, char *prefix)
{
    int err;
    int i;
    char path[MAX_DM_PATH];
    char key[MAX_DM_PATH];
    char value[MAX_DM_VALUE_LEN];
    int num_args;

    // Initialise the return vector
    KV_VECTOR_Init(args);

    // Exit if unable to get the number of persisted arguments
    // NOTE: It is safe to use DATA_MODEL_GetNumInstances() rather than
    // DATA_MODEL_GetInstances() because we always ensure the instances are contiguous and start from 1
    USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d.%sArgs", instance, prefix);
    err = DATA_MODEL_GetNumInstances(path, &num_args);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Iterate over all arguments
    for (i=0; i < num_args; i++)
    {
        // Exit if unable to get the name of the argument
        USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d.%sArgs.%d.Name", instance, prefix, i+1);
        err = DATA_MODEL_GetParameterValue(path, key, sizeof(key), 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if unable to get the value of the argument
        USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d.%sArgs.%d.Value", instance, prefix, i+1);
        err = DATA_MODEL_GetParameterValue(path, value, sizeof(value), 0);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Add the argument to the return vector
        USP_ARG_Add(args, key, value);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DeleteRequestArgs
**
** Function called to cascade the delete of a request object to
** the shadow object containing the persisted arguments for the operation
**
** \param   req - pointer to structure containing path information of the request that was deleted
**
** \return  USP_ERR_OK if validated successfully
**
**************************************************************************/
int DeleteRequestArgs(dm_req_t *req)
{
    int err;
    char path[MAX_DM_PATH];

    // Exit if unable to delete the shadow object
    USP_SNPRINTF(path, sizeof(path), "Internal.Request.%d", inst1);
    err = DATA_MODEL_DeleteInstance(path, IGNORE_NO_INSTANCE);

    return err;
}

