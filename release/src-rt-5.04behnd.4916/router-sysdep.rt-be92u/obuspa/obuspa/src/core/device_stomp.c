/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2017-2022  CommScope, Inc
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
 * \file device_stomp.c
 *
 * Implements the Device.STOMP data model object
 *
 */
#ifndef DISABLE_STOMP
#include <time.h>
#include <string.h>
#include <limits.h>

#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "dm_access.h"
#include "dm_trans.h"
#include "kv_vector.h"
#include "mtp_exec.h"
#include "device.h"
#include "text_utils.h"
#include "stomp.h"
#include "iso8601.h"

//------------------------------------------------------------------------------
// Location of the STOMP connection table within the data model
#define DEVICE_STOMP_CONN_ROOT "Device.STOMP.Connection"
static const char device_stomp_conn_root[] = DEVICE_STOMP_CONN_ROOT;

//------------------------------------------------------------------------------
// Cache of the parameters in the Device.STOMP.Connection table
static stomp_conn_params_t stomp_conn_params[MAX_STOMP_CONNECTIONS];

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int ValidateAdd_StompConn(dm_req_t *req);
int Notify_StompConnAdded(dm_req_t *req);
int Notify_StompConnDeleted(dm_req_t *req);
int Get_StompConnectionStatus(dm_req_t *req, char *buf, int len);
int Get_StompLastChangeDate(dm_req_t *req, char *buf, int len);
int Get_StompIsEncrypted(dm_req_t *req, char *buf, int len);
int Get_StompEnableEncryption(dm_req_t *req, char *buf, int len);
int Set_StompEnableEncryption(dm_req_t *req, char *buf);
int Validate_HeartbeatPeriod(dm_req_t *req, char *value);
int Validate_RetryInitialInterval(dm_req_t *req, char *value);
int Validate_RetryIntervalMultiplier(dm_req_t *req, char *value);
int Validate_RetryMaxInterval(dm_req_t *req, char *value);
int NotifyChange_StompEnable(dm_req_t *req, char *value);
int NotifyChange_StompHost(dm_req_t *req, char *value);
int NotifyChange_StompPort(dm_req_t *req, char *value);
int NotifyChange_StompUsername(dm_req_t *req, char *value);
int NotifyChange_StompPassword(dm_req_t *req, char *value);
int NotifyChange_StompEnableEncryption(dm_req_t *req, char *value);
int NotifyChange_VirtualHost(dm_req_t *req, char *value);
int ProcessStompConnAdded(int instance);
stomp_conn_params_t *FindUnusedStompParams(void);
void DestroyStompConn(stomp_conn_params_t *sp);
stomp_conn_params_t *FindStompParamsByInstance(int instance);
int NotifyChange_EnableHeartbeats(dm_req_t *req, char *value);
int NotifyChange_OutgoingHeartbeat(dm_req_t *req, char *value);
int NotifyChange_IncomingHeartbeat(dm_req_t *req, char *value);
int NotifyChange_RetryInitialInterval(dm_req_t *req, char *value);
int NotifyChange_RetryIntervalMultiplier(dm_req_t *req, char *value);
int NotifyChange_RetryMaxInterval(dm_req_t *req, char *value);
int EnableStompConnection(stomp_conn_params_t *sp);
void ScheduleStompReconnect(stomp_conn_params_t *sp);


/*********************************************************************//**
**
** DEVICE_STOMP_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_STOMP_Init(void)
{
    int err = USP_ERR_OK;
    int i;
    stomp_conn_params_t *sp;

    // Exit if unable to initialise the lower level STOMP component
    err = STOMP_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Mark all stomp params slots as unused
    memset(stomp_conn_params, 0, sizeof(stomp_conn_params));
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sp = &stomp_conn_params[i];
        sp->instance = INVALID;
    }

    // Register parameters implemented by this component
    err |= USP_REGISTER_Object(DEVICE_STOMP_CONN_ROOT ".{i}", ValidateAdd_StompConn, NULL, Notify_StompConnAdded,
                                                              NULL, NULL, Notify_StompConnDeleted);
    err |= USP_REGISTER_Param_NumEntries("Device.STOMP.ConnectionNumberOfEntries", DEVICE_STOMP_CONN_ROOT ".{i}");
    err |= USP_REGISTER_DBParam_Alias(DEVICE_STOMP_CONN_ROOT ".{i}.Alias", NULL);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_STOMP_CONN_ROOT ".{i}.Status", Get_StompConnectionStatus, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_STOMP_CONN_ROOT ".{i}.LastChangeDate", Get_StompLastChangeDate, DM_DATETIME);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.Enable", "false", NULL, NotifyChange_StompEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.Host", "", NULL, NotifyChange_StompHost, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.Port", "61613", DM_ACCESS_ValidatePort, NotifyChange_StompPort, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.Username", "", NULL, NotifyChange_StompUsername, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.EnableEncryption", "true", NULL, NotifyChange_StompEnableEncryption, DM_BOOL);
    err |= USP_REGISTER_VendorParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.X_ARRIS-COM_EnableEncryption", Get_StompEnableEncryption, Set_StompEnableEncryption, NotifyChange_StompEnableEncryption, DM_BOOL);

    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_STOMP_CONN_ROOT ".{i}.IsEncrypted", Get_StompIsEncrypted, DM_BOOL);
    err |=    USP_REGISTER_DBParam_Secure(DEVICE_STOMP_CONN_ROOT ".{i}.Password", "", NULL, NotifyChange_StompPassword);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.VirtualHost", "/", NULL, NotifyChange_VirtualHost, DM_STRING); // NOTE: RabbitMQ doesn't allow the virtual host be be an empty string

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.EnableHeartbeats", "false", NULL, NotifyChange_EnableHeartbeats, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.OutgoingHeartbeat", "0", Validate_HeartbeatPeriod, NotifyChange_OutgoingHeartbeat, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.IncomingHeartbeat", "0", Validate_HeartbeatPeriod, NotifyChange_IncomingHeartbeat, DM_UINT);

    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.ServerRetryInitialInterval", "60", Validate_RetryInitialInterval, NotifyChange_RetryInitialInterval, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.ServerRetryIntervalMultiplier", "2000", Validate_RetryIntervalMultiplier, NotifyChange_RetryIntervalMultiplier, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_STOMP_CONN_ROOT ".{i}.ServerRetryMaxInterval", "30720", Validate_RetryMaxInterval, NotifyChange_RetryMaxInterval, DM_UINT);


    // Register unique keys for tables
    char *unique_keys[] = { "Host", "Username", "VirtualHost" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_STOMP_CONN_ROOT ".{i}", unique_keys, NUM_ELEM(unique_keys));

    // Exit if any errors occurred
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_STOMP_Start
**
** Initialises the stomp connection array from the DB
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_STOMP_Start(void)
{
    int i;
    int err;
    int_vector_t iv;
    int instance;
    char path[MAX_DM_PATH];

    // Exit if unable to get the object instance numbers present in the stomp connection table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(DEVICE_STOMP_CONN_ROOT, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all stomp connections to the stomp connection array
    for (i=0; i < iv.num_entries; i++)
    {
        instance = iv.vector[i];
        err = ProcessStompConnAdded(instance);
        if (err != USP_ERR_OK)
        {
            // Exit if unable to delete a STOMP connection with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_stomp_conn_root, instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    // Exit if unable to create the SSL context to be used by STOMP
    err = STOMP_Start();
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    err = USP_ERR_OK;

exit:
    // Destroy the vector of instance numbers for the table
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_STOMP_Stop
**
** Frees up all memory associated with this module
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_STOMP_Stop(void)
{
    int i;
    stomp_conn_params_t *sp;

    // Iterate over all STOMP connections, freeing all memory used by it
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sp = &stomp_conn_params[i];
        if (sp->instance != INVALID)
        {
            DestroyStompConn(sp);
        }
    }
}

/*********************************************************************//**
**
** DEVICE_STOMP_StartAllConnections
**
** Starts all STOMP connections
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_STOMP_StartAllConnections(void)
{
    int i;
    stomp_conn_params_t *sp;
    int err;

    // Iterate over all STOMP connections, starting the ones that are enabled
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        sp = &stomp_conn_params[i];
        if ((sp->instance != INVALID) && (sp->enable == true))
        {
            // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
            err = EnableStompConnection(sp);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_STOMP_QueueBinaryMessage
**
** Function called to queue a message on the specified STOMP connection
**
** \param   msi - Information about the content to send. The ownership of
**                the payload buffer is passed to this function, unless an error is returned.
** \param   instance - instance number of the stomp connection in Device.STOMP.Connection.{i}
** \param   controller_queue - name of STOMP queue to send this message to
** \param   agent_queue - name of agent's STOMP queue configured for this connection in the data model.
**                        NOTE: This may be NULL, if agent's STOMP queue is set by subscribe_dest: STOMP header
** \param   expiry_time - time at which the USP message should be removed from the MTP send queue
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_STOMP_QueueBinaryMessage(mtp_send_item_t *msi, int instance, char *controller_queue, char *agent_queue, time_t expiry_time)
{
    int err = USP_ERR_GENERAL_FAILURE;
    stomp_conn_params_t *sp;
    USP_ASSERT(msi != NULL);

    // Exit if unable to find the specified STOMP connection
    sp = FindStompParamsByInstance(instance);
    if ((sp == NULL) || (sp->enable == false))
    {
        USP_ERR_SetMessage("%s: No internal STOMP connection matching Device.STOMP.Connection.%d", __FUNCTION__, instance);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to queue the message
    err = STOMP_QueueBinaryMessage(msi, instance, controller_queue, agent_queue, expiry_time);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_STOMP_ScheduleReconnect
**
** Schedules a reconnect to the specified STOMP connection, once that connection has finished sending any response
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
**
** \return  None
**
**************************************************************************/
void DEVICE_STOMP_ScheduleReconnect(int instance)
{
    stomp_conn_params_t *sp;

    // Exit if unable to find the specified STOMP connection
    sp = FindStompParamsByInstance(instance);
    if (sp == NULL)
    {
        return;
    }

    // Schedule a reconnect if this STOMP connection is enabled
    if (sp->enable)
    {
        ScheduleStompReconnect(sp);
    }
}

/*********************************************************************//**
**
** DEVICE_STOMP_GetMtpStatus
**
** Function called to get the value of Device.LocalAgent.MTP.{i}.Status for a STOMP connection
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
**
** \return  Status of the STOMP connection
**
**************************************************************************/
mtp_status_t DEVICE_STOMP_GetMtpStatus(int instance)
{
    stomp_conn_params_t *sp;

    // Exit if unable to find the specified STOMP connection
    // NOTE: This could occur if the connection was disabled, or the connection reference was incorrect
    sp = FindStompParamsByInstance(instance);
    if ((sp == NULL) || (sp->enable == false))
    {
        return kMtpStatus_Down;
    }

    return STOMP_GetMtpStatus(sp->instance);
}

/*********************************************************************//**
**
** DEVICE_STOMP_GetDestinationFromServer
**
** Function called to get the value of Device.LocalAgent.MTP.{i}.DestinationFromServer for a STOMP connection
**
** \param   instance - instance number of the connection in Device.STOMP.Connection.{i}
** \param   buf - pointer to buffer in which to return the value
** \param   len - length of buffer
**
** \return  None
**
**************************************************************************/
void DEVICE_STOMP_GetDestinationFromServer(int instance, char *buf, int len)
{
    stomp_conn_params_t *sp;

    // Set default return value
    *buf = '\0';

    // Exit if unable to find the specified STOMP connection
    // NOTE: This could occur if the connection was disabled, or the connection reference was incorrect
    sp = FindStompParamsByInstance(instance);
    if ((sp == NULL) || (sp->enable == false))
    {
        return;
    }

    STOMP_GetDestinationFromServer(instance, buf, len);
}

/*********************************************************************//**
**
** DEVICE_STOMP_CountEnabledConnections
**
** Returns a count of the number of enabled STOMP connections
**
** \param   None
**
** \return  returns a count of the number of enabled STOMP connections
**
**************************************************************************/
int DEVICE_STOMP_CountEnabledConnections(void)
{
    int i;
    int count = 0;
    stomp_conn_params_t *sp;

    // Iterate over all STOMP connections
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        // Increase the count if found an enabled connection
        sp = &stomp_conn_params[i];
        if ((sp->instance != INVALID) && (sp->enable))
        {
            count++;
        }
    }

    return count;
}

/*********************************************************************//**
**
** ValidateAdd_StompConn
**
** Function called to determine whether a new STOMP connection may be added
**
** \param   req - pointer to structure identifying the STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_StompConn(dm_req_t *req)
{
    stomp_conn_params_t *sp;

    // Exit if unable to find a free STOMP connection slot
    sp = FindUnusedStompParams();
    if (sp == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_StompConnAdded
**
** Function called when a Stomp Connection has been added to Device.STOMP.Connection.{i}
**
** \param   req - pointer to structure identifying the STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_StompConnAdded(dm_req_t *req)
{
    int err;
    stomp_conn_params_t *sp;

    // Exit if failed to copy from DB into stomp_connection array
    err = ProcessStompConnAdded(inst1);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Start the connection (if enabled)
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);         // As we had just successfully added it
    if (sp->enable == true)
    {
        // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
        err = EnableStompConnection(sp);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_StompConnDeleted
**
** Function called when a STOMP Connection has been deleted from Device.STOMP.Connection.{i}
**
** \param   req - pointer to structure identifying the connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_StompConnDeleted(dm_req_t *req)
{
    stomp_conn_params_t *sp;

    // Exit if connection already deleted
    // NOTE: We might not find it if it was never added. This could occur if deleting from the DB at startup when we detected that the database params were invalid
    sp = FindStompParamsByInstance(inst1);
    if (sp == NULL)
    {
        return USP_ERR_OK;
    }

    // Delete the connection from the array, if it has not already been deleted
    DestroyStompConn(sp);

    // Unpick references to this connection
    DEVICE_CONTROLLER_NotifyStompConnDeleted(inst1);
    DEVICE_MTP_NotifyStompConnDeleted(inst1);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_StompConnectionStatus
**
** Gets the value of Device.STOMP.Connection.{i}.Status
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_StompConnectionStatus(dm_req_t *req, char *buf, int len)
{
    stomp_conn_params_t *sp;
    char *status;

    // Determine stomp connection to be read
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    if (sp->enable == false)
    {
        status = "Disabled";
    }
    else
    {
        status = STOMP_GetConnectionStatus(sp->instance, NULL);
    }

    USP_STRNCPY(buf, status, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_StompLastChangeDate
**
** Gets the value of Device.STOMP.Connection.{i}.LastChangeDate
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_StompLastChangeDate(dm_req_t *req, char *buf, int len)
{
    time_t last_change_date;

    // Get the LastChangeDate of this connection
    // NOTE: Intentionally ignoring the status of the connection
    (void) STOMP_GetConnectionStatus(inst1, &last_change_date);

    // And write the time into the return buffer
    iso8601_from_unix_time(last_change_date, buf, len);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_StompIsEncrypted
**
** Gets the value of Device.STOMP.Connection.{i}.IsEncrypted
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_StompIsEncrypted(dm_req_t *req, char *buf, int len)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to query
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    val_bool = sp->enable_encryption;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_StompEnableEncryption
**
** Gets the value of Device.STOMP.Connection.{i}.X_ARRIS-COM_EnableEncryption
** This function aliases the legacy parameter 'X_ARRIS-COM_EnableEncryption' to the database parameter 'EnableEncryption'
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_StompEnableEncryption(dm_req_t *req, char *buf, int len)
{
    int err;
    char path[MAX_DM_PATH];

    // Form path to database parameter
    USP_SNPRINTF(path, sizeof(path), "Device.STOMP.Connection.%d.EnableEncryption", inst1);

    // Get the value
    err = DATA_MODEL_GetParameterValue(path, buf, len, 0);

    return err;
}

/*********************************************************************//**
**
** Set_StompEnableEncryption
**
** Sets the value of Device.STOMP.Connection.{i}.X_ARRIS-COM_EnableEncryption
** This function aliases the legacy parameter 'X_ARRIS-COM_EnableEncryption' to the database parameter 'EnableEncryption'
**
** \param   req - pointer to structure identifying the path
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Set_StompEnableEncryption(dm_req_t *req, char *buf)
{
    int err;
    char path[MAX_DM_PATH];

    // Form path to database parameter
    USP_SNPRINTF(path, sizeof(path), "Device.STOMP.Connection.%d.EnableEncryption", inst1);

    // Set the value
    err = DATA_MODEL_SetParameterValue(path, buf, 0);

    return err;
}


/*********************************************************************//**
**
** Validate_HeartbeatPeriod
**
** Function called to validate Device.STOMP.Connection.{i}.IncomingHeartbeat and OutgoingHeartbeat
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_HeartbeatPeriod(dm_req_t *req, char *value)
{
    // Exit if heartbeat period is too low
    #define MIN_HEARTBEAT_PERIOD 5000   // Minimum that we will allow is 5 seconds. 0 is also allowed (indicates 'off')
    if ((val_uint < MIN_HEARTBEAT_PERIOD) && (val_uint != 0))
    {
        USP_ERR_SetMessage("%s: Minimum allowed incoming or outgoing heartbeat period is %d ms (or 0 for off).", __FUNCTION__, MIN_HEARTBEAT_PERIOD);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_RetryInitialInterval
**
** Function called to validate Device.STOMP.Connection.{i}.ServerRetryInitialInterval
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_RetryInitialInterval(dm_req_t *req, char *value)
{

    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** Validate_RetryIntervalMultiplier
**
** Function called to validate Device.STOMP.Connection.{i}.ServerRetryIntervalMultiplier
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_RetryIntervalMultiplier(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1000, 65535);
}

/*********************************************************************//**
**
** Validate_RetryMaxInterval
**
** Function called to validate Device.STOMP.Connection.{i}.ServerRetryMaxInterval
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_RetryMaxInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, UINT_MAX);
}

/*********************************************************************//**
**
** NotifyChange_StompEnable
**
** Function called when Device.STOMP.Connection.{i}.Enable is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_StompEnable(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool old_value;
    int err;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);
    old_value = sp->enable;

    // Stop the connection if it has been disabled
    // NOTE: This code does not support sending a response back to a controller that disables it's own STOMP connection
    // However, as it is unlikely to be the case that a controller would ever do this, I have not added extra code to support this
    if ((old_value == true) && (val_bool == false))
    {
        STOMP_DisableConnection(sp->instance, PURGE_QUEUED_MESSAGES);
    }

    // Set the new value, we do this inbetween stopping and starting the connection because both must have the enable set to true
    sp->enable = val_bool;

    // Start the connection if it has been enabled
    if ((old_value == false) && (val_bool == true))
    {
        // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
        err = EnableStompConnection(sp);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_StompHost
**
** Function called when Device.STOMP.Connection.{i}.Host is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_StompHost(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool schedule_reconnect = false;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Determine whether to schedule a reconnect
    if ((strcmp(sp->host, value) != 0) && (sp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(sp->host);
    sp->host = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleStompReconnect(sp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_StompPort
**
** Function called when Device.STOMP.Connection.{i}.Port is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_StompPort(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool schedule_reconnect = false;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Determine whether to schedule a reconnect
    if ((sp->port != val_uint) && (sp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    sp->port = val_uint;

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleStompReconnect(sp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_StompUsername
**
** Function called when Device.STOMP.Connection.{i}.Username is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_StompUsername(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool schedule_reconnect = false;
    char buf[MAX_DM_SHORT_VALUE_LEN];
    dm_vendor_get_mtp_username_cb_t   get_mtp_username_cb;
    int err;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Determine whether to schedule a reconnect
    if ((strcmp(sp->username, value) != 0) && (sp->enable))
    {
        schedule_reconnect = true;
    }

    // Override a blank username in the database with that provided by a core vendor hook
    if (*value == '\0')
    {
        get_mtp_username_cb = vendor_hook_callbacks.get_mtp_username_cb;
        if (get_mtp_username_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_username_cb(inst1, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                return err;
            }

            value = buf;
        }
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(sp->username);
    sp->username = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleStompReconnect(sp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_StompPassword
**
** Function called when Device.STOMP.Connection.{i}.Password is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_StompPassword(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool schedule_reconnect = false;
    char buf[MAX_DM_SHORT_VALUE_LEN];
    dm_vendor_get_mtp_password_cb_t   get_mtp_password_cb;
    int err;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Override a blank password in the database with that provided by a core vendor hook
    if (*value == '\0')
    {
        get_mtp_password_cb = vendor_hook_callbacks.get_mtp_password_cb;
        if (get_mtp_password_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_password_cb(inst1, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                return err;
            }

            value = buf;
        }
    }

    // Determine whether to schedule a reconnect
    if ((strcmp(sp->password, value) != 0) && (sp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(sp->password);
    sp->password = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleStompReconnect(sp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_StompEnableEncryption
**
** Function called when Device.STOMP.Connection.{i}.EnableEncryption is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_StompEnableEncryption(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool schedule_reconnect = false;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Determine whether to schedule a reconnect
    if ((sp->enable_encryption != val_bool) && (sp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    sp->enable_encryption = val_bool;

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleStompReconnect(sp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_VirtualHost
**
** Function called when Device.STOMP.Connection.{i}.VirtualHost is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_VirtualHost(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;
    bool schedule_reconnect = false;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Determine whether to schedule a reconnect
    if ((strcmp(sp->virtual_host, value) != 0) && (sp->enable))
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(sp->virtual_host);
    sp->virtual_host = USP_STRDUP(value);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
        ScheduleStompReconnect(sp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_EnableHeartbeats
**
** Function called when Device.STOMP.Connection.{i}.EnableHeartbeats is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_EnableHeartbeats(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Set the new value
    // NOTE: We purposefully do not schedule a reconnect. This change takes effect, the next time USP Agent connects to the STOMP server
    sp->enable_heartbeats = val_uint;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_OutgoingHeartbeat
**
** Function called when Device.STOMP.Connection.{i}.OutgoingHeartbeat is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_OutgoingHeartbeat(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Set the new value
    // NOTE: We purposefully do not schedule a reconnect. This change takes effect, the next time USP Agent connects to the STOMP server
    sp->outgoing_heartbeat_period = val_uint;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_IncomingHeartbeat
**
** Function called when Device.STOMP.Connection.{i}.IncomingHeartbeat is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_IncomingHeartbeat(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Set the new value
    // NOTE: We purposefully do not schedule a reconnect. This change takes effect, the next time USP Agent connects to the STOMP server
    sp->incoming_heartbeat_period = val_uint;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_RetryInitialInterval
**
** Function called when Device.STOMP.Connection.{i}.ServerRetryInitialInterval
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_RetryInitialInterval(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Set the new value and notify it to the MTP
    sp->retry.initial_interval = val_uint;
    STOMP_UpdateRetryParams(inst1, &sp->retry);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_RetryIntervalMultiplier
**
** Function called when Device.STOMP.Connection.{i}.ServerRetryIntervalMultiplier
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_RetryIntervalMultiplier(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Set the new value and notify it to the MTP
    sp->retry.interval_multiplier = val_uint;
    STOMP_UpdateRetryParams(inst1, &sp->retry);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_RetryMaxInterval
**
** Function called when Device.STOMP.Connection.{i}.ServerRetryMaxInterval
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_RetryMaxInterval(dm_req_t *req, char *value)
{
    stomp_conn_params_t *sp;

    // Determine stomp connection to be updated
    sp = FindStompParamsByInstance(inst1);
    USP_ASSERT(sp != NULL);

    // Set the new value and notify it to the MTP
    sp->retry.max_interval = val_uint;
    STOMP_UpdateRetryParams(inst1, &sp->retry);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ProcessStompConnAdded
**
** Reads the parameters for the specified Stomp Connection from the database and processes them
**
** \param   instance - instance number of the STOMP connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessStompConnAdded(int instance)
{
    stomp_conn_params_t *sp;
    int err;
    char path[MAX_DM_PATH];
    char buf[MAX_DM_SHORT_VALUE_LEN];
    dm_vendor_get_mtp_username_cb_t   get_mtp_username_cb;
    dm_vendor_get_mtp_password_cb_t   get_mtp_password_cb;

    // Exit if unable to add another STOMP connection
    sp = FindUnusedStompParams();
    if (sp == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // Initialise to defaults
    memset(sp, 0, sizeof(stomp_conn_params_t));
    sp->instance = instance;

    // Exit if unable to get the enable for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Enable", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetBool(path, &sp->enable);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the host for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Host", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetString(path, &sp->host);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the port for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Port", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sp->port);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the username for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Username", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetString(path, &sp->username);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Override a blank username in the database with that provided by a core vendor hook
    if (sp->username[0] == '\0')
    {
        get_mtp_username_cb = vendor_hook_callbacks.get_mtp_username_cb;
        if (get_mtp_username_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_username_cb(instance, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Replace the blank password from the database with the password retrieved via core vendor hook
            USP_SAFE_FREE(sp->username);
            sp->username = USP_STRDUP(buf);
        }
    }

    // Exit if unable to get the password for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.Password", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetPassword(path, &sp->password);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Override a blank password in the database with that provided by a core vendor hook
    if (sp->password[0] == '\0')
    {
        get_mtp_password_cb = vendor_hook_callbacks.get_mtp_password_cb;
        if (get_mtp_password_cb != NULL)
        {
            // Exit if vendor hook failed
            err = get_mtp_password_cb(instance, buf, sizeof(buf));
            if (err != USP_ERR_OK)
            {
                goto exit;
            }

            // Replace the blank password from the database with the password retrieved via core vendor hook
            USP_SAFE_FREE(sp->password);
            sp->password = USP_STRDUP(buf);
        }
    }

    // Exit if unable to get the encryption enable for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.EnableEncryption", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetBool(path, &sp->enable_encryption);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the virtual host for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.VirtualHost", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetString(path, &sp->virtual_host);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the enable heartbeats for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.EnableHeartbeats", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetBool(path, &sp->enable_heartbeats);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the outgoing heartbeat for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.OutgoingHeartbeat", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sp->outgoing_heartbeat_period);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the incoming heartbeat for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.IncomingHeartbeat", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sp->incoming_heartbeat_period);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the server retry initial interval for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ServerRetryInitialInterval", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sp->retry.initial_interval);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the server retry interval multiplier for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ServerRetryIntervalMultiplier", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sp->retry.interval_multiplier);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if unable to get the server retry max interval for this STOMP connection
    USP_SNPRINTF(path, sizeof(path), "%s.%d.ServerRetryMaxInterval", device_stomp_conn_root, instance);
    err = DM_ACCESS_GetUnsigned(path, &sp->retry.max_interval);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // If the code gets here, then we successfully retrieved all data about the STOMP connection
    err = USP_ERR_OK;

exit:
    if (err != USP_ERR_OK)
    {
        DestroyStompConn(sp);
    }

    return err;
}

/*********************************************************************//**
**
** FindUnusedStompParams
**
** Finds the first free stomp params slot
**
** \param   None
**
** \return  Pointer to first free slot, or NULL if no slot was found
**
**************************************************************************/
stomp_conn_params_t *FindUnusedStompParams(void)
{
    int i;
    stomp_conn_params_t *sp;

    // Iterate over all STOMP connections
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        // Exit if found an unused slot
        sp = &stomp_conn_params[i];
        if (sp->instance == INVALID)
        {
            return sp;
        }
    }

    // If the code gets here, then no free slot has been found
    USP_ERR_SetMessage("%s: Only %d STOMP connections are supported.", __FUNCTION__, MAX_STOMP_CONNECTIONS);
    return NULL;
}

/*********************************************************************//**
**
** DestroyStompConn
**
** Frees all memory associated with the specified STOMP connection slot
**
** \param   sp - pointer to STOMP connection to free
**
** \return  None
**
**************************************************************************/
void DestroyStompConn(stomp_conn_params_t *sp)
{
    // Disable the lower level connection (if previously enabled)
    if (sp->enable)
    {
        STOMP_DisableConnection(sp->instance, PURGE_QUEUED_MESSAGES);
    }

    // Free and DeInitialise the slot
    sp->instance = INVALID;      // Mark slot as free
    sp->enable = false;
    sp->port = 0;
    USP_SAFE_FREE(sp->host);
    USP_SAFE_FREE(sp->username);
    USP_SAFE_FREE(sp->password);
    USP_SAFE_FREE(sp->virtual_host);
}

/*********************************************************************//**
**
** FindStompParamsByInstance
**
** Finds the stomp params slot by it's data model instance number
**
** \param   instance - instance number of the STOMP connection in the data model
**
** \return  pointer to slot, or NULL if slot was not found
**
**************************************************************************/
stomp_conn_params_t *FindStompParamsByInstance(int instance)
{
    int i;
    stomp_conn_params_t *sp;

    // Iterate over all STOMP connections
    for (i=0; i<MAX_STOMP_CONNECTIONS; i++)
    {
        // Exit if found a stomp connection that matches the instance number
        sp = &stomp_conn_params[i];
        if (sp->instance == instance)
        {
            return sp;
        }
    }

    // If the code gets here, then no matching slot was found
    return NULL;
}

/*********************************************************************//**
**
** EnableStompConnection
**
** Wrapper function to enable a STOMP connection with the current connection parameters
**
** \param   sp - STOMP connection parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int EnableStompConnection(stomp_conn_params_t *sp)
{
    int err;
    char *stomp_queue;

    stomp_queue = DEVICE_MTP_GetAgentStompQueue(sp->instance);
    err = STOMP_EnableConnection(sp, stomp_queue);

    return err;
}

/*********************************************************************//**
**
** ScheduleStompReconnect
**
** Wrapper function to schedule a STOMP reconnect with the current connection parameters
**
** \param   sp - STOMP connection parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void ScheduleStompReconnect(stomp_conn_params_t *sp)
{
    char *stomp_queue;

    stomp_queue = DEVICE_MTP_GetAgentStompQueue(sp->instance);
    STOMP_ScheduleReconnect(sp, stomp_queue);
}

#endif // DISABLE_STOMP
