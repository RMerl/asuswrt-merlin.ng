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
 * \file device_uds.c
 *
 * Implements the Device.UDS data model object
 *
 */

#ifdef ENABLE_UDS

#include <time.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "dm_access.h"
#include "dm_trans.h"
#include "kv_vector.h"
#include "mtp_exec.h"
#include "device.h"
#include "text_utils.h"
#include "uds.h"
#include "iso8601.h"

//------------------------------------------------------------------------------
// Location of the UDS connection table within the data model
#define DEVICE_UDS_CONN_ROOT "Device.UnixDomainSockets.UnixDomainSocket"
static const char device_uds_conn_root[] = DEVICE_UDS_CONN_ROOT;

//------------------------------------------------------------------------------
// Cache of the parameters in the Device.UnixDomainSockets table
static uds_conn_params_t uds_conn_params[MAX_UDS_SERVERS];

//------------------------------------------------------------------------------
// Table used to convert from an enumeration of a UDS mode to a textual representation
const enum_entry_t uds_modes[] =
{
    { kUdsConnType_Server,  "Listen" },
    { kUdsConnType_Client,  "Connect" },
};

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
uds_conn_params_t *FindUnusedUdsParams(void);
int ValidateAdd_UdsConn(dm_req_t *req);
int Validate_UdsMode(dm_req_t *req, char *value);
int Validate_UdsPath(dm_req_t *req, char *value);
int ProcessUdsConnAdded(int instance);
void DestroyUdsConn(uds_conn_params_t *ucp);
int Notify_UdsConnAdded(dm_req_t *req);
int Notify_UdsConnDeleted(dm_req_t *req);
int NotifyChange_UdsMode(dm_req_t *req, char *value);
int NotifyChange_UdsPath(dm_req_t *req, char *value);
uds_conn_params_t *FindUdsParamsByInstance(int instance);
uds_path_t CalcUdsPathType(char *uds_path);

/*********************************************************************//**
**
** DEVICE_UDS_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_UDS_Init(void)
{
    int i;
    int err = USP_ERR_OK;
    uds_conn_params_t *ucp;

    // Exit if unable to initialise the lower level UDS component
    err = UDS_Init();
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Mark all UDS params slots as unused
    memset(uds_conn_params, 0, sizeof(uds_conn_params));
    for (i=0; i<NUM_ELEM(uds_conn_params); i++)
    {
        ucp = &uds_conn_params[i];
        ucp->instance = INVALID;
        ucp->path = NULL;
        ucp->mode = kUdsConnType_Invalid;
        ucp->path_type = kUdsPathType_Invalid;
    }

    // Register parameters implemented by this component
    err |= USP_REGISTER_Object(DEVICE_UDS_CONN_ROOT ".{i}", ValidateAdd_UdsConn, NULL, Notify_UdsConnAdded, NULL, NULL, Notify_UdsConnDeleted);
    err |= USP_REGISTER_Param_NumEntries("Device.UnixDomainSockets.UnixDomainSocketNumberOfEntries", DEVICE_UDS_CONN_ROOT ".{i}");
    err |= USP_REGISTER_DBParam_Alias(DEVICE_UDS_CONN_ROOT ".{i}.Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_UDS_CONN_ROOT ".{i}.Mode", "", Validate_UdsMode, NotifyChange_UdsMode, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite(DEVICE_UDS_CONN_ROOT ".{i}.Path", "", Validate_UdsPath, NotifyChange_UdsPath, DM_STRING);
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
** DEVICE_UDS_Start
**
** Initialises the UDS connection array from the DB and starts connections
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_UDS_Start(void)
{
    int i;
    int err;
    int_vector_t iv;
    int instance;
    char path[MAX_DM_PATH];
    uds_conn_params_t *ucp;

    // Exit if unable to get the object instance numbers present in the UDS table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(DEVICE_UDS_CONN_ROOT, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all DM UDS connections to the uds_conn_params array
    for (i=0; i < iv.num_entries; i++)
    {
        instance = iv.vector[i];
        err = ProcessUdsConnAdded(instance);
        if (err != USP_ERR_OK)
        {
            // Exit if unable to delete a UDS connection with bad parameters from the DB
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_uds_conn_root, instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0);
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }

        ucp = FindUdsParamsByInstance(instance);
        err = UDS_EnableConnection(ucp);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }
    }

    err = USP_ERR_OK;

exit:
    // Destroy the vector of instance numbers for the table
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_UDS_Stop
**
** Stops all UDS connections and frees all dynamically allocated memory
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DEVICE_UDS_Stop(void)
{
    int i;
    uds_conn_params_t *ucp;

    // Iterate over all UDS connections, freeing all memory used by it
    for (i=0; i<NUM_ELEM(uds_conn_params); i++)
    {
        ucp = &uds_conn_params[i];
        if (ucp->instance != INVALID)
        {
            DestroyUdsConn(ucp);
        }
    }
}

/*********************************************************************//**
**
** ValidateAdd_UdsConn
**
** Function called to determine whether a new UDS connection may be added
**
** \param   req - pointer to structure identifying the UDS connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ValidateAdd_UdsConn(dm_req_t *req)
{
    uds_conn_params_t *ucp;

    // Exit if unable to find a free UDS connection slot
    ucp = FindUnusedUdsParams();
    if (ucp == NULL)
    {
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_UdsMode
**
** Validates Device.UnixDomainSockets.UnixDomainSocket.{i}.Mode
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_UdsMode(dm_req_t *req, char *value)
{
    uds_connection_type_t mode;

    // Exit if mode was invalid
    mode = TEXT_UTILS_StringToEnum(value, uds_modes, NUM_ELEM(uds_modes));
    if (mode == kUdsConnType_Invalid)
    {
        USP_ERR_SetMessage("%s: Invalid mode (%s) for %s.", __FUNCTION__, value, req->path);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_UdsPath
**
** Validates Device.UnixDomainSockets.UnixDomainSocket.{i}.Path
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_UdsPath(dm_req_t *req, char *value)
{
    int max_len;

    max_len = sizeof(((struct sockaddr_un *)NULL)->sun_path) - 1;  // Minus 1 to include NULL terminator
    if (strlen(value) > max_len)
    {
        USP_ERR_SetMessage("%s: UDS socket path too long", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** FindUnusedUdsParams
**
** Finds the first free UDS params slot
**
** \param   None
**
** \return  Pointer to first free slot, or NULL if no slot was found
**
**************************************************************************/
uds_conn_params_t *FindUnusedUdsParams(void)
{
    int i;
    uds_conn_params_t *ucp;

    // Iterate over all UDS connections
    for (i=0; i<NUM_ELEM(uds_conn_params); i++)
    {
        // Exit if found an unused slot
        ucp = &uds_conn_params[i];
        if (ucp->instance == INVALID)
        {
            return ucp;
        }
    }

    // If the code gets here, then no free slot has been found
    USP_ERR_SetMessage("%s: Only %d USP connections are supported.", __FUNCTION__, (int) NUM_ELEM(uds_conn_params));
    return NULL;
}

/*********************************************************************//**
**
** FindUdsParamsByInstance
**
** Finds the UDS params slot by it's data model instance number
**
** \param   instance - instance number of the UDS connection in the data model
**
** \return  pointer to slot, or NULL if slot was not found
**
**************************************************************************/
uds_conn_params_t *FindUdsParamsByInstance(int instance)
{
    int i;
    uds_conn_params_t *ucp;

    // Iterate over all UDS connections
    for (i=0; i<NUM_ELEM(uds_conn_params); i++)
    {
        // Exit if found a uds connection that matches the instance number
        ucp = &uds_conn_params[i];
        if (ucp->instance == instance)
        {
            return ucp;
        }
    }

    // If the code gets here, then no matching slot was found
    return NULL;
}


/*********************************************************************//**
**
** DestroyUdsConn
**
** Frees all memory associated with the specified UDS connection slot
**
** \param   sp - pointer to UDS connection to free
**
** \return  None
**
**************************************************************************/
void DestroyUdsConn(uds_conn_params_t *ucp)
{
    // Disable the lower level connection
    UDS_DisableConnection(ucp->instance);

    // Free and DeInitialise the slot
    ucp->instance = INVALID;      // Mark slot as free
    USP_SAFE_FREE(ucp->path);
}

/*********************************************************************//**
**
** ProcessUdsConnAdded
**
** Reads the parameters for the specified UDS Connection from the database and processes them
**
** \param   instance - instance number of the UDS connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessUdsConnAdded(int instance)
{
    uds_conn_params_t *ucp;
    int err;
    char path[MAX_DM_PATH];

    // Exit if unable to add another UDS connection
    ucp = FindUnusedUdsParams();
    if (ucp == NULL)
    {
        USP_LOG_Error("%s: no free UDS params", __FUNCTION__);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    // Initialise to defaults
    memset(ucp, 0, sizeof(uds_conn_params_t));
    ucp->instance = instance;

    USP_SNPRINTF(path, sizeof(path), "%s.%d.Mode", device_uds_conn_root, instance);
    err = DM_ACCESS_GetEnum(path, &ucp->mode, uds_modes, NUM_ELEM(uds_modes));
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: bad mode param", __FUNCTION__);
        goto exit;
    }

    USP_SNPRINTF(path, sizeof(path), "%s.%d.Path", device_uds_conn_root, instance);
    err = DM_ACCESS_GetString(path, &ucp->path);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: bad path param", __FUNCTION__);
        goto exit;
    }

    ucp->path_type = CalcUdsPathType(ucp->path);

    // If the code gets here, then we successfully retrieved all data about the UDS connection
    err = USP_ERR_OK;

exit:
    if (err != USP_ERR_OK)
    {
        DestroyUdsConn(ucp);
    }

    return err;
}

/*********************************************************************//**
**
** Notify_UdsConnAdded
**
** Function called when a Uds Connection has been added to Device.UnixDomainSockets.UnixDomainSocket.{i}
**
** \param   req - pointer to structure identifying the UDS connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_UdsConnAdded(dm_req_t *req)
{
    int err;
    uds_conn_params_t *ucp;

    // Exit if failed to copy from DB into UDS_connection array
    err = ProcessUdsConnAdded(inst1);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Start the connection (if enabled)
    ucp = FindUdsParamsByInstance(inst1);
    USP_ASSERT(ucp != NULL);         // As we had just successfully added it

    // Exit if no free slots to enable the connection. (Enable is successful, even if the connection is trying to reconnect)
    err = UDS_EnableConnection(ucp);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_UdsConnDeleted
**
** Function called when a UDS Connection has been deleted from Device.UnixDomainSockets.UnixDomainSocket.{i}
**
** \param   req - pointer to structure identifying the connection
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_UdsConnDeleted(dm_req_t *req)
{
    uds_conn_params_t *ucp;

    // Exit if connection already deleted
    // NOTE: We might not find it if it was never added. This could occur if deleting from the DB at startup when we detected that the database params were invalid
    ucp = FindUdsParamsByInstance(inst1);
    if (ucp == NULL)
    {
        return USP_ERR_OK;
    }

    // Delete the connection from the array, and tell the lower level MTP to disconnect
    DestroyUdsConn(ucp);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_UdsMode
**
** Function called when Device.Device.UnixDomainSockets.UnixDomainSocket.{i}.Mode is modified
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_UdsMode(dm_req_t *req, char *value)
{
    uds_conn_params_t *ucp;
    uds_connection_type_t new_mode;

    bool schedule_reconnect = false;

    // Determine UDS connection to be updated
    ucp = FindUdsParamsByInstance(inst1);
    USP_ASSERT(ucp != NULL);

    // Determine whether to schedule a reconnect
    new_mode = TEXT_UTILS_StringToEnum(value, uds_modes, NUM_ELEM(uds_modes));
    if (new_mode != ucp->mode)
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    ucp->mode = new_mode;

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
       UDS_ScheduleReconnect(ucp);
    }

    return USP_ERR_OK;
}


/*********************************************************************//**
**
** NotifyChange_UdsPath
**
** Function called when Device.Device.UnixDomainSockets.UnixDomainSocket.{i}.Mode
**
** \param   req - pointer to structure identifying the path
** \param   value - new value of this parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_UdsPath(dm_req_t *req, char *value)
{
    uds_conn_params_t *ucp;

    bool schedule_reconnect = false;

    // Determine UDS connection to be updated
    ucp = FindUdsParamsByInstance(inst1);
    USP_ASSERT(ucp != NULL);

    // Determine whether to schedule a reconnect
    if (strcmp(ucp->path, value) != 0)
    {
        schedule_reconnect = true;
    }

    // Set the new value. This must be done before scheduling a reconnect, so that the reconnect uses the correct values
    USP_SAFE_FREE(ucp->path);
    ucp->path = USP_STRDUP(value);
    ucp->path_type = CalcUdsPathType(ucp->path);

    // Schedule a reconnect after the present response has been sent, if the value has changed
    if (schedule_reconnect)
    {
       UDS_ScheduleReconnect(ucp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** CalcUdsPathType
**
** Determines whether the specified Unix domain socket path is to the Broker's Controller or Agent
**
** \param   uds_path - File system path used by the Unix domain socket
**
** \return  type of uds_path - Broker's Controller or Agent
**
**************************************************************************/
uds_path_t CalcUdsPathType(char *uds_path)
{
    if ((strcmp(uds_path, "/var/run/usp/broker_controller_path")==0) || (strstr(uds_path, "controller") != NULL))
    {
        return kUdsPathType_BrokersController;
    }

    if ((strcmp(uds_path, "/var/run/usp/broker_agent_path")==0) || (strstr(uds_path, "agent") != NULL))
    {
        return kUdsPathType_BrokersAgent;
    }

    return kUdsPathType_Invalid;
}


#endif /* ENABLE_UDS */
