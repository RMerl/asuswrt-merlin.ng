/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2017-2024  CommScope, Inc
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
 * \file device_bulkdata.c
 *
 * Implements the Device.BulkData data model object
 *
 */

#ifndef REMOVE_DEVICE_BULKDATA
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <zlib.h>
#include <curl/curl.h>

#include "common_defs.h"
#include "data_model.h"
#include "usp_api.h"
#include "msg_handler.h"
#include "path_resolver.h"
#include "dm_access.h"
#include "json.h"
#include "sync_timer.h"
#include "iso8601.h"
#include "text_utils.h"
#include "retry_wait.h"
#include "bdc_exec.h"
#include "dm_exec.h"
#include "group_get_vector.h"
#include "usp_broker.h"

//------------------------------------------------------------------------------
// String versions of defines in vendor_defs.h
#define BULKDATA_MAX_PROFILES_STR  TO_STR(BULKDATA_MAX_PROFILES)
#define BULKDATA_MINIMUM_REPORTING_INTERVAL_STR      TO_STR(BULKDATA_MINIMUM_REPORTING_INTERVAL)

//------------------------------------------------------------------------------
// Definitions for formats that we support
#define BULKDATA_ENCODING_TYPE "JSON"
#define BULKDATA_JSON_REPORT_FORMAT "NameValuePair"


// Definitions for Device.BulkData.Profile.{i}.JSONEncoding.ReportTimestamp
#define BULKDATA_JSON_TIMESTAMP_FORMAT_EPOCH   "Unix-Epoch"
#define BULKDATA_JSON_TIMESTAMP_FORMAT_ISO8601 "ISO-8601"
#define BULKDATA_JSON_TIMESTAMP_FORMAT_NONE    "None"

// Definitions for Device.BulkData.Profile.{i}.HTTP.Method
#define BULKDATA_HTTP_METHOD_POST       "POST"
#define BULKDATA_HTTP_METHOD_PUT        "PUT"
#define BULKDATA_DEFAULT_HTTP_METHOD    BULKDATA_HTTP_METHOD_POST
#define BULKDATA_HTTP_METHODS_SUPPORTED BULKDATA_HTTP_METHOD_PUT ", " BULKDATA_HTTP_METHOD_POST

//---------------------------------------------------------------------------------------------
// Structure representing a report
typedef struct
{
    time_t collection_time;     // time at which the report was collected
    kv_vector_t  report_map;    // Map containing parameter path vs JSON type+parameter value
} report_t;

//---------------------------------------------------------------------------------------------
// Structure representing enabled profiles
typedef struct
{
    int profile_id;                 // A value of -1 here denotes that the slot is not used. Instance number of profile in Device.Bulkdata.Profile.{i}
    bool is_enabled;
    bool is_working;                // Set to false if last post failed

    // Cached versions of parameters from the data model for this profile
    int reporting_interval;
    time_t time_reference;
    bool retry_enable;
    unsigned retry_minimum_wait_interval;
    unsigned retry_interval_multiplier;

    // The following variables are only used when the profile is started (ie enabled)
    report_t reports[BULKDATA_MAX_RETAINED_FAILED_REPORTS+1]; // Plus 1 because this array includes failed reports + current report
    int num_retained_reports;
    unsigned retry_count;           // Number of failed attempts. Count of what the next retry attempt will be. After a failed send, this starts counting from 1.

} bulkdata_profile_t;

//---------------------------------------------------------------------------------------------
// Bulkdata library global context
static bulkdata_profile_t bulkdata_profiles[BULKDATA_MAX_PROFILES];

//---------------------------------------------------------------------------------------------
// Bulk Data Collection Protocol
#define BULKDATA_PROTOCOL_HTTP "HTTP"
#define BULKDATA_PROTOCOL_USP_EVENT "USPEventNotif"

typedef enum
{
    kBdcProtocol_HTTP,
    kBdcProtocol_UspEvent,

    kBdcProtocol_Max               // This should always be the last value in this enumeration. It is used to statically size arrays based on one entry for each active enumeration
} bdc_protocol_t;

// Array to convert from string to enumeration, and vice-versa
const enum_entry_t bdc_protocols[kBdcProtocol_Max] =
{
    { kBdcProtocol_HTTP,               BULKDATA_PROTOCOL_HTTP },       // This is the default value for notification type
    { kBdcProtocol_UspEvent,           BULKDATA_PROTOCOL_USP_EVENT },
};

//---------------------------------------------------------------------------------------------
// Profile Push Event
#define BULKDATA_PROFILE_PUSH_EVENT "Device.BulkData.Profile.{i}.Push!"

// Array of arguments sent in Push! event
static char *profile_push_event_args[] =
{
    "Data",
};

//---------------------------------------------------------------------------------------------
// Structure containing retrieved controlling parameters for a specific profile
// String sizes are taken from TR-181
typedef struct
{
    int num_retained_failed_reports;
    char report_timestamp[33];
    char url[1025];
    char username[257];
    char password[257];
    char compression[9];
    char method[9];
    bool use_date_header;
} profile_ctrl_params_t;

//------------------------------------------------------------------------------
// Global enable for all collection profiles (Device.BulkData.Enable)
static bool global_enable = false;

//--------------------------------------------------------------------------------------------
// Array of default URI query parameters, and their shortened name to put in the query
typedef struct
{
    char *ref;
    char *name;
} default_uri_query_params_t;

default_uri_query_params_t default_uri_query_params[] =
{
    { "Device.DeviceInfo.ManufacturerOUI", "oui"},
    { "Device.DeviceInfo.ProductClass", "pc"},
    { "Device.DeviceInfo.SerialNumber", "sn"},
};

//-------------------------------------------------------------------------
// Structure used to store the info read from Device.BulkData.Profile.{i}.Parameter,
// and marshall the data from the get group vector
typedef struct
{
    char *path_expr;        // Path expression stored in Device.BulkData.Profile.{i}.Parameter.{i}.Reference
    char *alt_name;         // Alt name stored in Device.BulkData.Profile.{i}.Parameter.{i}.Name
    int index;              // start index of parameters in get group vector for this path expression
    int num_entries;        // number of entries in the get group vector for this path expression
} param_ref_entry_t;

typedef struct
{
    param_ref_entry_t *vector;
    int num_entries;
} param_ref_vector_t;

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int Validate_AddBulkDataProfile(dm_req_t *req);
int Validate_NumberOfRetainedFailedReports(dm_req_t *req, char *value);
int Validate_BulkDataProtocol(dm_req_t *req, char *value);
int Validate_BulkDataEncodingType(dm_req_t *req, char *value);
int Validate_BulkDataReportingInterval(dm_req_t *req, char *value);
int Validate_BulkDataReference(dm_req_t *req, char *value);
int Validate_BulkDataReportFormat(dm_req_t *req, char *value);
int Validate_BulkDataReportTimestamp(dm_req_t *req, char *value);
int Validate_BulkDataCompression(dm_req_t *req, char *value);
int Validate_BulkDataHTTPMethod(dm_req_t *req, char *value);
int Validate_BulkDataRetryMinimumWaitInterval(dm_req_t *req, char *value);
int Validate_BulkDataRetryIntervalMultiplier(dm_req_t *req, char *value);
int NotifyChange_BulkDataGlobalEnable(dm_req_t *req, char *value);
int NotifyChange_BulkDataProfileEnable(dm_req_t *req, char *value);
int NotifyChange_BulkDataReportingInterval(dm_req_t *req, char *value);
int NotifyChange_BulkDataTimeReference(dm_req_t *req, char *value);
int NotifyChange_BulkDataRetryEnable(dm_req_t *req, char *value);
int NotifyChange_BulkDataRetryMinimumWaitInterval(dm_req_t *req, char *value);
int NotifyChange_BulkDataRetryIntervalMultiplier(dm_req_t *req, char *value);
int NotifyChange_BulkDataURL(dm_req_t *req, char *value);
int Notify_BulkDataProfileAdded(dm_req_t *req);
int Notify_BulkDataProfileDeleted(dm_req_t *req);
int Get_BulkDataGlobalStatus(dm_req_t *req, char *buf, int len);
int Get_BulkDataProfileStatus(dm_req_t *req, char *buf, int len);
int ProcessBulkDataProfileAdded(int instance);
void ProcessBulkDataProfileDeleted(bulkdata_profile_t *bp);
int bulkdata_stop_profile(bulkdata_profile_t *bp);
void bulkdata_process_profile(int id);
void bulkdata_process_profile_http(bulkdata_profile_t *bp);
void bulkdata_process_profile_usp_event(bulkdata_profile_t *bp);
bulkdata_profile_t *bulkdata_find_free_profile(void);
bulkdata_profile_t *bulkdata_find_profile(int profile_id);
int bulkdata_calc_report_map(bulkdata_profile_t *bp, kv_vector_t *report_map);
int bulkdata_reduce_to_alt_name(char *spec, char *path, char *alt_name, char *out_buf, int buf_len);
char *bulkdata_generate_json_report(bulkdata_profile_t *bp, char *report_timestamp);
unsigned char *bulkdata_compress_report(profile_ctrl_params_t *ctrl, char *input_buf, int input_len, int *p_output_len);
int bulkdata_schedule_sending_http_report(profile_ctrl_params_t *ctrl, bulkdata_profile_t *bp, unsigned char *json_report, int report_len);
int bulkdata_start_profile(bulkdata_profile_t *bp);
int bulkdata_resync_profile(bulkdata_profile_t *bp, int *delta_time);
unsigned bulkdata_calc_waittime_to_next_send(bulkdata_profile_t *bp);
unsigned bulkdata_calc_waittime_to_next_reporting_interval(time_t interval, time_t time_reference);
void bulkdata_clear_retained_reports(bulkdata_profile_t *bp);
void bulkdata_drop_oldest_retained_reports(bulkdata_profile_t *bp, int num_reports_to_keep);
int bulkdata_platform_get_uri_query_name_map(int profile_id, kv_vector_t *name_map);
int bulkdata_platform_calc_uri_query_escaped_map(kv_vector_t *name_map, kv_vector_t *escaped_map);
char *bulkdata_platform_calc_uri_query_string(kv_vector_t *escaped_map);
int bulkdata_platform_get_param_refs(int profile_id, param_ref_vector_t *param_refs);
void bulkdata_expand_param_ref(param_ref_entry_t *pr, group_get_vector_t *ggv);
void bulkdata_append_to_result_map(param_ref_entry_t *pr, group_get_vector_t *ggv, kv_vector_t *report_map);

/*********************************************************************//**
**
** DEVICE_BULKDATA_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_BULKDATA_Init(void)
{
    int i;
    int err = USP_ERR_OK;
    bulkdata_profile_t *bp;

    // Initialise all profiles
    memset(bulkdata_profiles, 0, sizeof(bulkdata_profiles));
    for (i=0; i<BULKDATA_MAX_PROFILES; i++)
    {
        bp = &bulkdata_profiles[i];
        bp->profile_id = INVALID;
    }

    // Register data model elements implemented by this component
    // Device.BulkData.
    err = USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Enable", "false", NULL, NotifyChange_BulkDataGlobalEnable, DM_BOOL);
    err |= USP_REGISTER_VendorParam_ReadOnly("Device.BulkData.Status", Get_BulkDataGlobalStatus, DM_STRING);
    err |= USP_REGISTER_Param_Constant("Device.BulkData.MinReportingInterval", BULKDATA_MINIMUM_REPORTING_INTERVAL_STR, DM_UINT);
    err |= USP_REGISTER_Param_SupportedList("Device.BulkData.Protocols", bdc_protocols, NUM_ELEM(bdc_protocols));
    err |= USP_REGISTER_Param_Constant("Device.BulkData.EncodingTypes", BULKDATA_ENCODING_TYPE, DM_STRING);
    err |= USP_REGISTER_Param_Constant("Device.BulkData.ParameterWildCardSupported", "true", DM_BOOL);
    err |= USP_REGISTER_Param_Constant("Device.BulkData.MaxNumberOfProfiles", BULKDATA_MAX_PROFILES_STR, DM_INT);
    err |= USP_REGISTER_Param_Constant("Device.BulkData.MaxNumberOfParameterReferences", "-1", DM_INT);

    // Device.BulkData.Profile.{i}
    err |= USP_REGISTER_Object("Device.BulkData.Profile.{i}", Validate_AddBulkDataProfile, NULL, Notify_BulkDataProfileAdded,
                                                              NULL, NULL, Notify_BulkDataProfileDeleted);
    err |= USP_REGISTER_Param_NumEntries("Device.BulkData.ProfileNumberOfEntries", "Device.BulkData.Profile.{i}");
    err |= USP_REGISTER_DBParam_Alias("Device.BulkData.Profile.{i}.Alias", NULL);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.Enable", "false", NULL, NotifyChange_BulkDataProfileEnable, DM_BOOL);
    err |= USP_REGISTER_VendorParam_ReadOnly("Device.BulkData.Profile.{i}.X_ARRIS-COM_Status", Get_BulkDataProfileStatus, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.Name", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.NumberOfRetainedFailedReports", "0", Validate_NumberOfRetainedFailedReports, NULL, DM_INT);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.Protocol", BULKDATA_PROTOCOL_HTTP, Validate_BulkDataProtocol, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.EncodingType", BULKDATA_ENCODING_TYPE, Validate_BulkDataEncodingType, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.ReportingInterval", "86400", Validate_BulkDataReportingInterval, NotifyChange_BulkDataReportingInterval, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.TimeReference", UNKNOWN_TIME_STR, NULL, NotifyChange_BulkDataTimeReference, DM_DATETIME);

    // Device.BulkData.Profile.{i}.Parameter.{i}
    err |= USP_REGISTER_Object("Device.BulkData.Profile.{i}.Parameter.{i}", NULL, NULL, NULL,
                                                                            NULL, NULL, NULL);
    err |= USP_REGISTER_Param_NumEntries("Device.BulkData.Profile.{i}.ParameterNumberOfEntries", "Device.BulkData.Profile.{i}.Parameter.{i}");
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.Parameter.{i}.Name", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.Parameter.{i}.Reference", "", Validate_BulkDataReference, NULL, DM_STRING);

    // Device.BulkData.Profile.{i}.JSONEncoding
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.JSONEncoding.ReportFormat", BULKDATA_JSON_REPORT_FORMAT, Validate_BulkDataReportFormat, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.JSONEncoding.ReportTimestamp", BULKDATA_JSON_TIMESTAMP_FORMAT_EPOCH, Validate_BulkDataReportTimestamp, NULL, DM_STRING);

    // Device.BulkData.Profile.{i}.HTTP
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.URL", "", NULL, NotifyChange_BulkDataURL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.Username", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_Secure("Device.BulkData.Profile.{i}.HTTP.Password", "", NULL, NULL);
    err |= USP_REGISTER_Param_Constant("Device.BulkData.Profile.{i}.HTTP.CompressionsSupported", "GZIP", DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.Compression", "None", Validate_BulkDataCompression, NULL, DM_STRING);
    err |= USP_REGISTER_Param_Constant("Device.BulkData.Profile.{i}.HTTP.MethodsSupported", BULKDATA_HTTP_METHODS_SUPPORTED, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.Method", BULKDATA_HTTP_METHOD_POST, Validate_BulkDataHTTPMethod, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.UseDateHeader", "true", NULL, NULL, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.RetryEnable", "false", NULL, NotifyChange_BulkDataRetryEnable, DM_BOOL);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.RetryMinimumWaitInterval", "5", Validate_BulkDataRetryMinimumWaitInterval, NotifyChange_BulkDataRetryMinimumWaitInterval, DM_UINT);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.RetryIntervalMultiplier", "2000", Validate_BulkDataRetryIntervalMultiplier, NotifyChange_BulkDataRetryIntervalMultiplier, DM_UINT);

    // Device.BulkData.Profile.{i}.HTTP.RequestURIParameter.{i}
    err |= USP_REGISTER_Object("Device.BulkData.Profile.{i}.HTTP.RequestURIParameter.{i}", NULL, NULL, NULL,
                                                                                           NULL, NULL, NULL);
    err |= USP_REGISTER_Param_NumEntries("Device.BulkData.Profile.{i}.HTTP.RequestURIParameterNumberOfEntries", "Device.BulkData.Profile.{i}.HTTP.RequestURIParameter.{i}");
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.RequestURIParameter.{i}.Name", "", NULL, NULL, DM_STRING);
    err |= USP_REGISTER_DBParam_ReadWrite("Device.BulkData.Profile.{i}.HTTP.RequestURIParameter.{i}.Reference", "", Validate_BulkDataReference, NULL, DM_STRING);

    // Register Push! Event
    err |= USP_REGISTER_Event(BULKDATA_PROFILE_PUSH_EVENT);
    err |= USP_REGISTER_EventArguments(BULKDATA_PROFILE_PUSH_EVENT, profile_push_event_args, NUM_ELEM(profile_push_event_args));


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
** DEVICE_BULKDATA_Start
**
** Starts this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_BULKDATA_Start(void)
{
    int i;
    int err;
    int_vector_t iv;
    int instance;
    char *device_bulkdata_profile_root = "Device.BulkData.Profile";
    char path[MAX_DM_PATH];

    // Exit if unable to read the value of the global enable
    err = DM_ACCESS_GetBool("Device.BulkData.Enable", &global_enable);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the object instance numbers present in the profile table
    INT_VECTOR_Init(&iv);
    err = DATA_MODEL_GetInstances(device_bulkdata_profile_root, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add all profiles to the bulkdata_profiles array
    for (i=0; i < iv.num_entries; i++)
    {
        instance = iv.vector[i];
        err = ProcessBulkDataProfileAdded(instance);
        if (err != USP_ERR_OK)
        {
            // If failed to add, then delete it
            USP_SNPRINTF(path, sizeof(path), "%s.%d", device_bulkdata_profile_root, instance);
            USP_LOG_Warning("%s: Deleting %s as it contained invalid parameters.", __FUNCTION__, path);
            err = DATA_MODEL_DeleteInstance(path, 0); // NOTE: This will cascade to delete from bulkdata_profiles array
            if (err != USP_ERR_OK)
            {
                goto exit;
            }
        }
    }

    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_BULKDATA_Stop
**
** Frees up all memory associated with this module
**
** \param   None
**
** \return  None
**
**************************************************************************/
void DEVICE_BULKDATA_Stop(void)
{
    int i;
    bulkdata_profile_t *bp;

    // Iterate over all profiles, deleting them (this stops them, and frees dynamic memory associated with them)
    for (i=0; i < BULKDATA_MAX_PROFILES; i++)
    {
        bp = &bulkdata_profiles[i];
        if (bp->profile_id != INVALID)
        {
            ProcessBulkDataProfileDeleted(bp);
        }
    }
}

/*********************************************************************//**
**
** DEVICE_BULKDATA_NotifyTransferResult
**
** Signalled from the BDC thread to notify that a report has been sent (successfully or unsuccessfully)
** It is also called if we failed to send a message to the BDC thread to kick off sending a report
** This function restarts the profile's sync timer, so that the profile fires again in the future
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
** \param   transfer_result - Result code of the transfer
**
** \return  None
**
**************************************************************************/
void DEVICE_BULKDATA_NotifyTransferResult(int profile_id, bdc_transfer_result_t transfer_result)
{
    bulkdata_profile_t *bp;
    int err;
    int delta_time;

    // Exit if unable to find the profile
    // NOTE: This could occur, if the profile was deleted by the controller whilst the report was still being sent by BDC thread
    bp = bulkdata_find_profile(profile_id);
    if (bp == NULL)
    {
        return;
    }

    if (transfer_result == kBDCTransferResult_Success)
    {
        // Report(s) have been successfully sent, so don't retain them
        bulkdata_clear_retained_reports(bp);
        bp->is_working = true;
    }
    else
    {
        USP_LOG_Warning("BULK DATA: Failed to send report (profile_id=%d) on retry_count=%d", profile_id, bp->retry_count);
        if (bp->retry_enable)
        {
            // Report(s) have not been sent successfully, so start the retry mechanism (or increment the retry_count, if it is already in progress)
            bp->retry_count++;
        }
        bp->is_working = false;
    }


    // Exit if unable to restart the sync timer with the time until the next reporting interval (or retry)
    err = bulkdata_resync_profile(bp, &delta_time);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // If next processing event will be a retry, log how long it is until the retry
    // NOTE: This debug is not strictly true. If it is time for the next report before the retry,
    // then the next report will be sent at the time logged
    if ((transfer_result != kBDCTransferResult_Success) && (bp->retry_enable))
    {
        USP_LOG_Info("BULK DATA: Retrying profile_id=%d in %d seconds", profile_id, delta_time);
    }

    // Uncomment the next line to see how much memory is in use by USP Agent after each post
    //USP_MEM_PrintSummary();
}

/*********************************************************************//**
**
** Validate_AddBulkDataProfile
**
** Checks that an extra instance can be added to Device.BulkData.Profile.{i}
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_AddBulkDataProfile(dm_req_t *req)
{
    int err;
    int num_instances;

    // Exit if unable to determine the number of instances in the bulk data profile table
    err = DATA_MODEL_GetNumInstances("Device.BulkData.Profile.", &num_instances);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to add another bulk data profile
    if (num_instances >= BULKDATA_MAX_PROFILES)
    {
        USP_ERR_SetMessage("%s: Only a maximum of %d bulk data profiles supported", __FUNCTION__, BULKDATA_MAX_PROFILES);
        return USP_ERR_RESOURCES_EXCEEDED;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_NumberOfRetainedFailedReports
**
** Validates Device.BulkData.Profile.{i}.NumberOfRetainedFailedReports
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_NumberOfRetainedFailedReports(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Signed(req, -1, BULKDATA_MAX_RETAINED_FAILED_REPORTS);
}

/*********************************************************************//**
**
** Validate_BulkDataProtocol
**
** Validates Device.BulkData.Profile.{i}.Protocol
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataProtocol(dm_req_t *req, char *value)
{
    bdc_protocol_t protocol;

    protocol = TEXT_UTILS_StringToEnum(value, bdc_protocols, NUM_ELEM(bdc_protocols));
    if (protocol == INVALID)
    {
        USP_ERR_SetMessage("%s: Invalid or unsupported protocol (%s)", __FUNCTION__, value);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BulkDataEncodingType
**
** Validates Device.BulkData.Profile.{i}.EncodingType
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataEncodingType(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if (strcmp(value, BULKDATA_ENCODING_TYPE) != 0)
    {
        USP_ERR_SetMessage("%s: Only EncodingType supported is '%s'", __FUNCTION__, BULKDATA_ENCODING_TYPE);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BulkDataReportingInterval
**
** Validates Device.BulkData.Profile.{i}.ReportingInterval
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataReportingInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, BULKDATA_MINIMUM_REPORTING_INTERVAL, UINT_MAX);
}

/*********************************************************************//**
**
** Validate_BulkDataReference
**
** Validates Device.BulkData.Profile.{i}.Parameter.{i}.Reference
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataReference(dm_req_t *req, char *value)
{
    // Exit if path is not a valid reference
    // NOTE: TR157 Bulk Data Collection only allows partial paths and wildcards
    // However for USP we also allow search expressions. We do not allow reference following
    // (because the alt-name reduction rules in TR-157 do not support it)

#ifdef USE_LEGACY_PATH_VALIDATION
    int err;
    combined_role_t combined_role;

    // NOTE: The resolved paths are validated against the current controller's role.
    // So even if a partial path is specified here, it will fail to validate if permissions do not allow it
    MSG_HANDLER_GetMsgRole(&combined_role);
    err = PATH_RESOLVER_ResolveDevicePath(value, NULL, NULL, kResolveOp_GetBulkData, FULL_DEPTH, &combined_role, 0);
    return err;

#else
    // When running as a USP Broker, the DM elements referenced by the path might not have been registered yet
    // So since we cannot resolve the path, instead try to verify it textually
    int err;
    char *p;
    bool inside_brackets;

    // Exit if the path contained a reference follow (outside of a unique key search expression)
    // Reference following is not allowed for BulkData because the alt-name reduction rules in TR-157 do not support it
    inside_brackets = false;
    p = value;
    while (*p != '\0')
    {
        if (*p == '[')
        {
            inside_brackets = true;
        }
        else if (*p == ']')
        {
            inside_brackets = false;
        }
        else if ((*p == '+') && (inside_brackets == false))
        {
            USP_ERR_SetMessage("%s: Bulk Data collection does not allow reference following in search expressions", __FUNCTION__);
            return USP_ERR_INVALID_PATH_SYNTAX;
        }
        p++;
    }

    err = PATH_RESOLVER_ValidatePath(value, kSubNotifyType_ValueChange);

    return err;

#endif
}

/*********************************************************************//**
**
** Validate_BulkDataReportFormat
**
** Validates Device.BulkData.Profile.{i}.JSONEncoding.ReportFormat
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataReportFormat(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if (strcmp(value, BULKDATA_JSON_REPORT_FORMAT) != 0)
    {
        USP_ERR_SetMessage("%s: Only JSON Report Format supported is '%s'", __FUNCTION__, BULKDATA_JSON_REPORT_FORMAT);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BulkDataReportTimestamp
**
** Validates Device.BulkData.Profile.{i}.JSONEncoding.ReportTimestamp
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataReportTimestamp(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if ( (strcmp(value, BULKDATA_JSON_TIMESTAMP_FORMAT_EPOCH) != 0) &&
         (strcmp(value, BULKDATA_JSON_TIMESTAMP_FORMAT_ISO8601) != 0) &&
         (strcmp(value, BULKDATA_JSON_TIMESTAMP_FORMAT_NONE) != 0))
    {
        USP_ERR_SetMessage("%s: ReportTimestamp must be one of '%s, '%s' or '%s'", __FUNCTION__, BULKDATA_JSON_TIMESTAMP_FORMAT_EPOCH, BULKDATA_JSON_TIMESTAMP_FORMAT_ISO8601, BULKDATA_JSON_TIMESTAMP_FORMAT_NONE);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BulkDataCompression
**
** Validates Device.BulkData.Profile.{i}.HTTP.Compression
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataCompression(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if ((strcmp(value, "None") != 0) && (strcmp(value, "GZIP") != 0))
    {
        USP_ERR_SetMessage("%s: Only 'GZIP' and 'None' HTTP Compressions supported.", __FUNCTION__);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BulkDataHTTPMethod
**
** Validates Device.BulkData.Profile.{i}.HTTP.Method
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataHTTPMethod(dm_req_t *req, char *value)
{
    // Exit if trying to set a value outside of the range we accept
    if ( (strcmp(value, BULKDATA_HTTP_METHOD_POST) != 0) &&
         (strcmp(value, BULKDATA_HTTP_METHOD_PUT) != 0))
    {
        USP_ERR_SetMessage("%s: ReportTimestamp must be either '%s' or '%s'", __FUNCTION__, BULKDATA_HTTP_METHOD_POST, BULKDATA_HTTP_METHOD_PUT);
        return USP_ERR_INVALID_VALUE;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Validate_BulkDataRetryMinimumWaitInterval
**
** Validates Device.BulkData.Profile.{i}.HTTP.RetryMinimumWaitInterval
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataRetryMinimumWaitInterval(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1, 65535);
}

/*********************************************************************//**
**
** Validate_BulkDataRetryIntervalMultiplier
**
** Validates Device.BulkData.Profile.{i}.HTTP.RetryIntervalMultiplier
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Validate_BulkDataRetryIntervalMultiplier(dm_req_t *req, char *value)
{
    return DM_ACCESS_ValidateRange_Unsigned(req, 1000, 65535);
}

/*********************************************************************//**
**
** NotifyChange_BulkDataGlobalEnable
**
** Called whenever Device.BulkData.GlobalEnable is changed
** This function turns on/off bulk data collection profiles based on the value of the enables
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataGlobalEnable(dm_req_t *req, char *value)
{
    int i;
    int err;
    bulkdata_profile_t *bp;

    // Exit if value has not changed
    if (val_bool == global_enable)
    {
        return USP_ERR_OK;
    }
    global_enable = val_bool;

    // Iterate over all profiles, starting or stopping them
    for (i=0; i<BULKDATA_MAX_PROFILES; i++)
    {
        // Skip to next profile, if this one is not in use
        bp = &bulkdata_profiles[i];
        if (bp->profile_id == INVALID)
        {
            continue;
        }

        // Skip to next profile, if this one is not enabled.
        // If it is not enabled, then changing the global enable makes no difference to it - it'll still stay disabled
        if (bp->is_enabled == false)
        {
            continue;
        }

        if (global_enable)
        {
            // Exit if unable to turn on profile enabled by the global enable
            err = bulkdata_start_profile(bp);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
        else
        {
            // Exit if unable to turn off profile that was enabled by the previous global enable
            err = bulkdata_stop_profile(bp);
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
** NotifyChange_BulkDataProfileEnable
**
** Called whenever Device.BulkData.Profile.{i}.Enable is changed
** This function turns on/off bulk data collection profiles based on the value of the enables
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataProfileEnable(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;
    int err;

    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);

    // Exit if profile enable has not changed
    if (val_bool == bp->is_enabled)
    {
        return USP_ERR_OK;
    }
    bp->is_enabled = val_bool;

    // Exit if global enable is not enabled
    // If it is not enabled, then changing the profile enable makes no difference to it - it'll still stay disabled
    if (global_enable == false)
    {
        return USP_ERR_OK;
    }

    if (bp->is_enabled)
    {
        // Exit if unable to turn on profile
        err = bulkdata_start_profile(bp);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }
    else
    {
        // Exit if unable to turn off profile
        err = bulkdata_stop_profile(bp);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_BulkDataReportingInterval
**
** Called whenever Device.BulkData.Profile.{i}.TimeReference changes
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataReportingInterval(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;
    int err;

    // Copy changed value into bulkdata_profile[]
    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);
    bp->reporting_interval = val_int;

    // Modify the time at which the profile next fires (either to send a new report or retry sending an old one)
    err = bulkdata_resync_profile(bp, NULL);
    return err;
}

/*********************************************************************//**
**
** NotifyChange_BulkDataTimeReference
**
** Called whenever Device.BulkData.Profile.{i}.TimeReference changes
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataTimeReference(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;
    int err;

    // Determine profile to be updated
    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);

    // Set the new value
    bp->time_reference = RETRY_WAIT_UseRandomBaseIfUnknownTime(val_datetime);

    // Modify the time at which the profile next fires (either to send a new report or retry sending an old one)
    err = bulkdata_resync_profile(bp, NULL);
    return err;
}

/*********************************************************************//**
**
** NotifyChange_BulkDataRetryEnable
**
** Called whenever Device.BulkData.Profile.{i}.HTTP.RetryEnable changes
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataRetryEnable(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;

    // Copy changed value into bulkdata_profile[]
    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);
    bp->retry_enable = val_bool;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** NotifyChange_BulkDataRetryMinimumWaitInterval
**
** Called whenever Device.BulkData.Profile.{i}.HTTP.RetryMinimumWaitInterval changes
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataRetryMinimumWaitInterval(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;
    int err;

    // Copy changed value into bulkdata_profile[]
    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);
    bp->retry_minimum_wait_interval = val_uint;

    // Modify the time at which the profile next fires (either to send a new report or retry sending an old one)
    err = bulkdata_resync_profile(bp, NULL);
    return err;
}

/*********************************************************************//**
**
** NotifyChange_BulkDataRetryIntervalMultiplier
**
** Called whenever Device.BulkData.Profile.{i}.HTTP.RetryIntervalMultiplier changes
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataRetryIntervalMultiplier(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;
    int err;

    // Copy changed value into bulkdata_profile[]
    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);
    bp->retry_interval_multiplier = val_uint;

    // Modify the time at which the profile next fires (either to send a new report or retry sending an old one)
    err = bulkdata_resync_profile(bp, NULL);
    return err;
}

/*********************************************************************//**
**
** NotifyChange_BulkDataURL
**
** Called whenever Device.BulkData.Profile.{i}.HTTP.URL changes
**
** \param   req - pointer to structure identifying the parameter
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int NotifyChange_BulkDataURL(dm_req_t *req, char *value)
{
    bulkdata_profile_t *bp;

    // Reset the connectivity stats
    bp = bulkdata_find_profile(inst1);
    USP_ASSERT(bp != NULL);


    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Notify_BulkDataProfileAdded
**
** Called whenever a new profile (Device.BulkData.Profile.{i}) is added
**
** \param   req - pointer to structure identifying the instance
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_BulkDataProfileAdded(dm_req_t *req)
{
    int err;

    err = ProcessBulkDataProfileAdded(inst1);

    // NOTE: If an error occurred adding the profile to the bulkdata_profiles[], then
    // this is OK. The code will report the profile's status as 'Error'

    return err;
}

/*********************************************************************//**
**
** Notify_BulkDataProfileDeleted
**
** Called whenever a profile (Device.BulkData.Profile.{i}) is deleted
**
** \param   req - pointer to structure identifying the instance
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Notify_BulkDataProfileDeleted(dm_req_t *req)
{
    bulkdata_profile_t *bp;

    bp = bulkdata_find_profile(inst1);
    if (bp != NULL)
    {
        ProcessBulkDataProfileDeleted(bp);
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_BulkDataGlobalStatus
**
** Called to get the value of Device.BulkData.Status
**
** \param   req - pointer to structure identifying the instance
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_BulkDataGlobalStatus(dm_req_t *req, char *buf, int len)
{
    int i;
    bulkdata_profile_t *bp;
    char *status;
    int num_slots;
    int num_instances;
    int err;

    // Exit if global enable is disabled
    if (global_enable == false)
    {
        USP_STRNCPY(buf, "Disabled", len);
        return USP_ERR_OK;
    }

    // Exit if unable to determine the number of instances in the bulk data profile table
    err = DATA_MODEL_GetNumInstances("Device.BulkData.Profile.", &num_instances);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // By default global status follows whether global enable is on or off
    // However if any profiles are not working, then change the status to error
    status =  "Enabled";
    num_slots = 0;
    for (i=0; i<BULKDATA_MAX_PROFILES; i++)
    {
        // Skip to next profile, if this profile is not in use
        bp = &bulkdata_profiles[i];
        if (bp->profile_id == INVALID)
        {
            continue;
        }
        num_slots++;

        // Skip to next profile, if this profile is not enabled
        if (bp->is_enabled == false)
        {
            continue;
        }

        // Found a profile which is not working, so set status to error, and exit loop
        if (bp->is_working == false)
        {
            status = "Error";
            break;
        }
    }

    // If there are ever more instances in the USP DB than slots used in bulkdata_profiles[], then overall status is error
    // (as one of the instances which is not represented by a slot is in error)
    if (num_slots != num_instances)
    {
        status = "Error";
    }

    USP_STRNCPY(buf, status, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_BulkDataProfileStatus
**
** Called to get the value of Device.BulkData.Profile.{i}.X_ARRIS-COM_Status
**
** \param   req - pointer to structure identifying the instance
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_BulkDataProfileStatus(dm_req_t *req, char *buf, int len)
{
    bulkdata_profile_t *bp;
    char *status;

    // Find the profile
    bp = bulkdata_find_profile(inst1);
    if (bp == NULL)
    {
        // Cope with case of profile existing, but not having an entry in the bulkdata_profile array
        // This could occur if ProcessBulkDataProfileAdded() failed
        status = "Error";
        goto exit;
    }

    // Determine the status of the profile
    if ((bp->is_enabled==false) || (global_enable==false))
    {
        // Profile is disabled
        status = "Disabled";
    }
    else
    {
        // Profile is enabled, and may be working or not
        status = (bp->is_working) ? "Enabled" : "Error";
    }

exit:
    USP_STRNCPY(buf, status, len);
    return USP_ERR_OK;
}


/*********************************************************************//**
**
** ProcessBulkDataProfileAdded
**
** Called when a new profile has been added by a controller
** Also called at bootup to seed the bulkdata_profile[] array
**
** \param   instance - instance number of the profile that has been added
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ProcessBulkDataProfileAdded(int instance)
{
    int err;
    char path[MAX_DM_PATH];
    time_t base;
    bulkdata_profile_t *bp;

    // Exit if unable to find a free bulk data profile
    // NOTE: This should never happen, as Validate_AddBulkDataProfile() should have prevented this code from being called
    bp = bulkdata_find_free_profile();
    if (bp == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to add a new bulk data profile. Only %d profiles supported", __FUNCTION__, BULKDATA_MAX_PROFILES);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Clear the profile
    memset(bp, 0, sizeof(bulkdata_profile_t));

    // Exit if unable to get the Enable for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.Enable", instance);
    err = DM_ACCESS_GetBool(path, &bp->is_enabled);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the ReportingInterval for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.ReportingInterval", instance);
    err = DM_ACCESS_GetInteger(path, &bp->reporting_interval);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get the TimeReference for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.TimeReference", instance);
    err = DM_ACCESS_GetDateTime(path, &base);
    if (err != USP_ERR_OK)
    {
        return err;
    }
    bp->time_reference = RETRY_WAIT_UseRandomBaseIfUnknownTime(base);

    // Exit if unable to get RetryMinimumWaitInterval for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.RetryMinimumWaitInterval", instance);
    err = DM_ACCESS_GetUnsigned(path, &bp->retry_minimum_wait_interval);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get RetryIntervalMultiplier for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.RetryIntervalMultiplier", instance);
    err = DM_ACCESS_GetUnsigned(path, &bp->retry_interval_multiplier);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get RetryEnable for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.RetryEnable", instance);
    err = DM_ACCESS_GetBool(path, &bp->retry_enable);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Since successful, mark the profile as in use
    bp->profile_id = instance;
    bp->is_working = true;          // Assumed to be working correctly until first post proves otherwise


    // Start the profile, if enabled
    if ((bp->is_enabled) && (global_enable))
    {
        // Exit if unable to start the profile
        err = bulkdata_start_profile(bp);
        if (err != USP_ERR_OK)
        {
            bp->profile_id = INVALID;   // Ensure that profile slot is not leaked
            return err;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ProcessBulkDataProfileDeleted
**
** Called to stop a profile and delete all memory associated with it
**
** \param   bp - pointer to profile
**
** \return  None
**
**************************************************************************/
void ProcessBulkDataProfileDeleted(bulkdata_profile_t *bp)
{
    // Stop the profile, if started
    if ((global_enable) && (bp->is_enabled))
    {
        bulkdata_stop_profile(bp);
    }

    // Mark slot as not in use
    bp->profile_id = INVALID;
}

/*********************************************************************//**
**
**  bulkdata_platform_get_uri_query_params
**
**  Returns a (dynamically allocated) string containing the URI query parameters to append to the URL
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
**
** \return  dynamically allocated string containing the URI query parameters or NULL if an error occurred
**
**************************************************************************/
char *bulkdata_platform_get_uri_query_params(int profile_id)
{
    int err;
    kv_vector_t name_map;
    kv_vector_t escaped_map;
    char *query_string = NULL;

    // Initialize maps
    KV_VECTOR_Init(&name_map);
    KV_VECTOR_Init(&escaped_map);

    // Get the map of URI query parameters to get (path vs short name)
    err = bulkdata_platform_get_uri_query_name_map(profile_id, &name_map);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Forming a map containing the {name, value} of all URI query parameters
    // NOTE: These names and values are the URL escaped versions
    err = bulkdata_platform_calc_uri_query_escaped_map(&name_map, &escaped_map);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Finally create the output URI query string
    query_string = bulkdata_platform_calc_uri_query_string(&escaped_map);

exit:
    KV_VECTOR_Destroy(&name_map);
    KV_VECTOR_Destroy(&escaped_map);

    return query_string;
}

/*********************************************************************//**
**
**  bulkdata_platform_get_uri_query_name_map
**
**  Returns a map containing the short names of all URI query parameters (path vs short name)
**  NOTE: This function also adds the default query parameters to the map
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
** \param   name_map - initialised map structure in which to return the map of all URI query parameters for the specified profile
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_platform_get_uri_query_name_map(int profile_id, kv_vector_t *name_map)
{
    int err;
    int i;
    default_uri_query_params_t *p;
    int_vector_t iv;
    char path[MAX_DM_PATH];
    char ref[257];   // The sizes of these are set by TR-181
    char name[65];
    char *query_name; // Pointer to name to use in the URI query

    // Exit if unable to obtain the instance numbers of the request URI parameter table
    INT_VECTOR_Init(&iv);
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.RequestURIParameter.", profile_id);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add the default URI query parameters to the map (key=reference, value=alt_name)
    for (i=0; i<NUM_ELEM(default_uri_query_params); i++)
    {
        p = &default_uri_query_params[i];
        KV_VECTOR_Add(name_map, p->ref, p->name);
    }

    // Add all URI query parameters to the map
    for (i=0; i < iv.num_entries; i++)
    {
        // Exit if unable to get the parameter reference path
        USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.RequestURIParameter.%d.Reference", profile_id, i+1);
        err = DATA_MODEL_GetParameterValue(path, ref, sizeof(ref), 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Skip this parameter, if it is still blank (when the ACS creates a row, the default is for this parameter to be blank)
        if (ref[0] == '\0') {
            continue;
        }

        // Exit if unable to get the parameter alt-name
        USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.RequestURIParameter.%d.Name", profile_id, i+1);
        err = DATA_MODEL_GetParameterValue(path, name, sizeof(name), 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // If it's shortened name is blank, then make it the same as it's path
        query_name = &name[0];
        if (name[0] == '\0')
        {
            query_name = &ref[0];
        }

        // Add the path and alt-name to a map
        KV_VECTOR_Add(name_map, ref, query_name);
    }

    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);
    return err;
}

/*********************************************************************//**
**
**  bulkdata_platform_calc_uri_query_escaped_map
**
**  Returns a map containing the {short_name, value} of all URI query parameters
**  NOTE: The key and value are the URL escaped versions
**
** \param   name_map - input map containing the path and short name to use for all parameters
** \param   escaped_map - map in which to return escaped short name and escaped value of all URI query parameters
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_platform_calc_uri_query_escaped_map(kv_vector_t *name_map, kv_vector_t *escaped_map)
{
    CURL *handle = NULL;
    int err;
    int i;
    char *path;
    char *short_name;
    char *escaped_value = NULL;
    char *escaped_name = NULL;
    kv_pair_t *kv;
    char value[MAX_DM_VALUE_LEN];

    // Create a CURL easy handle to use when escaping the query parameters
    handle = curl_easy_init();
    if (handle == NULL)
    {
        USP_LOG_Error("%s: curl_easy_init failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all parameters to get
    // NOTE: If any failure occurs, then just skip that parameter from the URI query parameters
    for (i=0; i < name_map->num_entries; i++)
    {
        kv = &name_map->vector[i];
        path = kv->key;
        short_name = kv->value;

        // Skip if unable to get the value of the parameter
        err = DATA_MODEL_GetParameterValue(path, value, sizeof(value), 0);
        if (err != USP_ERR_OK)
        {
            continue;
        }

        // Escape the value string
        escaped_value = curl_easy_escape(handle, value, 0);
        if (escaped_value == NULL)
        {
            USP_LOG_Warning("%s: curl_easy_escape failed for '%s'. Not including %s in BDC URI query params", __FUNCTION__, value, path);
            continue;
        }

        // Escape the short name string
        escaped_name = curl_easy_escape(handle, short_name, 0);
        if (escaped_name == NULL)
        {
            curl_free(escaped_value);
            USP_LOG_Warning("%s: curl_easy_escape failed for '%s'. Not including %s in BDC URI query params", __FUNCTION__, short_name, path);
            continue;
        }

        // Add the escaped strings to the output map
        KV_VECTOR_Add(escaped_map, escaped_name, escaped_value);

        // Tidy up
        curl_free(escaped_value);
        curl_free(escaped_name);
    }

    curl_easy_cleanup(handle);
    return USP_ERR_OK;;
}

/*********************************************************************//**
**
**  bulkdata_platform_calc_uri_query_string
**
**  Returns a dynamically allocated string of the form "Name1=Value1&Name2=Value2"
**
** \param   escaped_map - pointer to structure containing the input map of {Name, Value}
**
** \return  Pointer to dynamically allocated string containing the URI query parameters
**
**************************************************************************/
char *bulkdata_platform_calc_uri_query_string(kv_vector_t *escaped_map)
{
    char *query_string = NULL;   // Pointer to dynamically allocated output string
    int cur_len, new_len;  // length of query string, not including NULL terminator
    bool first_query_param;
    char *name;
    char *value;
    int name_len, value_len;
    char *q;
    int i;
    kv_pair_t *kv;

    // Create an empty query string to start from
    query_string = USP_MALLOC(1);
    *query_string = '\0';
    cur_len = 0;

    // Iterate over all items in the map
    first_query_param = true;
    for (i=0; i < escaped_map->num_entries; i++)
    {
        kv = &escaped_map->vector[i];
        name = kv->key;
        name_len = strlen(name);
        value = kv->value;
        value_len = strlen(value);

        // Increase the size of the query string buffer
        new_len = cur_len + 1 + name_len + 1 + value_len;  // Plus 2 to include '? or '&' and '=' character
        query_string = USP_REALLOC(query_string, new_len+1);  // Plus 1 to include NULL terminator

        // Append '&' or '?', depending on whether this is the first query parameter which we are serializing
        q = &query_string[cur_len];
        if (first_query_param) {
            *q++ = '?';                 // First query param, starts with '?'
            first_query_param = false;
        }
        else {
            *q++ = '&';                 // Subsequent query params, start with '&'
        }

        // Append "name="
        strcpy(q, name);
        q += name_len;
        *q++ = '=';

        // Append "value\0"
        strcpy(q, value);
        q += value_len;
        *q++ = '\0';

        // Move to next value
        cur_len = new_len;
    }

    return query_string;
}

/*********************************************************************//**
**
**  bulkdata_calc_report_map
**
**  Calculates the map containing {parameter name vs type/value} for the specified profile
**
** \param   context - pointer to global bulkdata context
** \param   bp - pointer to bulk data profile to get the report map for
** \param   report_map - initialised map in which to return the report map
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_calc_report_map(bulkdata_profile_t *bp, kv_vector_t *report_map)
{
    int err;
    int i;
    param_ref_vector_t  param_refs;    // Vector containing info about each Device.BulkData.Profile.{i}.Parameter entry
    param_ref_entry_t *pr;
    group_get_vector_t ggv;

    // Get the paths and alternative names of all parameter references that are to be posted from this profile
    memset(&param_refs, 0, sizeof(param_refs));
    err = bulkdata_platform_get_param_refs(bp->profile_id, &param_refs);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: bulkdata_platform_get_param_refs() failed", __FUNCTION__);
        return err;
    }

    // Exit if no parameter references were found
    if (param_refs.num_entries == 0)
    {
        return USP_ERR_OK;
    }

    // Expand the parameter references into the set of parameters which they reference
    GROUP_GET_VECTOR_Init(&ggv);
    for (i=0; i < param_refs.num_entries; i++)
    {
        bulkdata_expand_param_ref(&param_refs.vector[i], &ggv);
    }

    // Get all parameters
    GROUP_GET_VECTOR_GetValues(&ggv);

    // Iterate over all param references, adding them to the report map
    for (i=0; i < param_refs.num_entries; i++)
    {
        bulkdata_append_to_result_map(&param_refs.vector[i], &ggv, report_map);
    }

    GROUP_GET_VECTOR_Destroy(&ggv);

    // Clean up param_refs
    for (i=0; i < param_refs.num_entries; i++)
    {
        pr = &param_refs.vector[i];
        USP_FREE(pr->path_expr);
        USP_SAFE_FREE(pr->alt_name);
    }
    USP_FREE(param_refs.vector);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** bulkdata_platform_get_param_refs
**
** Obtains all parameter references vs alternative names for a given profile instance.
** This is a map of the parameters to report on in a profile (key=path_expr, value=alt_name)
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
** \param   param_refs = vector in which to return the param refs that have been read from the USP database
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_platform_get_param_refs(int profile_id, param_ref_vector_t *param_refs)
{
    int i;
    char path[MAX_DM_PATH];
    int err;
    int_vector_t iv;
    int instance;
    char reference[MAX_DM_PATH];
    char alt_name[MAX_DM_SHORT_VALUE_LEN];
    param_ref_entry_t *pr;
    int size;

    // Initialize defaults for vectors (so that exit can destroy them safely)
    INT_VECTOR_Init(&iv);

    // Exit if unable to obtain the instance numbers of the profile's parameter table
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.Parameter.", profile_id);
    err = DATA_MODEL_GetInstances(path, &iv);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Exit if no instances were found
    if (iv.num_entries == 0)
    {
        goto exit;      // NOTE: err will be set to USP_ERR_OK if the code got here
    }

    // Allocate vector to store marshalling info for all param refs
    param_refs->num_entries = iv.num_entries;
    size = iv.num_entries*sizeof(param_ref_entry_t);
    param_refs->vector = USP_MALLOC(size);
    memset(param_refs->vector, 0, size);

    // Iterate over all entries in the Profile.{i}.Parameter table, getting their path expression vs alt_name
    for (i=0; i < iv.num_entries; i++)
    {
        // Exit if unable to obtain the parameter reference
        instance = iv.vector[i];
        USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.Parameter.%d.Reference", profile_id, instance);
        err = DATA_MODEL_GetParameterValue(path, reference, sizeof(reference), 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Exit if unable to obtain the parameter alt_name
        USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.Parameter.%d.Name", profile_id, instance);
        DATA_MODEL_GetParameterValue(path, alt_name, sizeof(alt_name), 0);
        if (err != USP_ERR_OK)
        {
            goto exit;
        }

        // Add the parameter reference and alt-name to param ref info
        pr = &param_refs->vector[i];
        pr->path_expr = USP_STRDUP(reference);
        pr->alt_name = USP_STRDUP(alt_name);
    }

    err = USP_ERR_OK;

exit:
    INT_VECTOR_Destroy(&iv);

    if (err != USP_ERR_OK)
    {
        USP_SAFE_FREE(param_refs->vector);
        param_refs->vector = NULL;
        param_refs->num_entries = 0;
    }
    return err;
}

/*********************************************************************//**
**
** bulkdata_expand_param_ref
**
** Expands the specified parameter reference, storing all the parameters it references into the get group vector
**
** \param   pr - pointer to parameter reference to expand
** \param   ggv - pointer to vector to add params found in this path expression to
**
** \return  None
**
**************************************************************************/
void bulkdata_expand_param_ref(param_ref_entry_t *pr, group_get_vector_t *ggv)
{
    int err;
    str_vector_t params;
    int_vector_t group_ids;

    STR_VECTOR_Init(&params);
    INT_VECTOR_Init(&group_ids);

    // Exit if unable to get the resolved paths
    // NOTE: This uses the full access role because the Bulk Data Collector is not a Controller and hence not subject to ControllerTrust permissions
    err = PATH_RESOLVER_ResolveDevicePath(pr->path_expr, &params, &group_ids, kResolveOp_GetBulkData, FULL_DEPTH, INTERNAL_ROLE, DONT_LOG_RESOLVER_ERRORS);
    if (err != USP_ERR_OK)
    {
        STR_VECTOR_Destroy(&params);
        INT_VECTOR_Destroy(&group_ids);
        return;
    }

    // Save the range of indexes for this path expression
    pr->index = ggv->num_entries;
    pr->num_entries = params.num_entries;

    // Exit if no params were found
    if (params.num_entries == 0)
    {
        STR_VECTOR_Destroy(&params);
        INT_VECTOR_Destroy(&group_ids);
        return;
    }

    // Expand the get group vector to contain the extra parameter requests
    // NOTE: Ownership of the strings in the params vector transfers to the group get vector
    GROUP_GET_VECTOR_AddParams(ggv, &params, &group_ids);

    // Clean up
    INT_VECTOR_Destroy(&group_ids);

    // Since we have moved the contents of the params vector to the get group vector, we can just free the params vector (not its content)
    USP_SAFE_FREE(params.vector);
}

/*********************************************************************//**
**
**  bulkdata_append_to_result_map
**
**  Appends the parameters for the specified parameter reference to the results map
**  When doing this, take account of alternative name, and type of each parameter
**
** \param   pr - pointer to parameter reference to expand
** \param   ggv - pointer to group get vector containing paths and values of params which were retrieved
** \param   report_map - map which this function appends the 'param_values' map to
**
** \return  None
**
**************************************************************************/
void bulkdata_append_to_result_map(param_ref_entry_t *pr, group_get_vector_t *ggv, kv_vector_t *report_map)
{
    int err;
    int i;
    char reduced_path[MAX_DM_PATH];
    char param_type_value[MAX_DM_VALUE_LEN+1];       // plus 1 to include leading JSON type character
    char type;
    group_get_entry_t *gge;

    // Iterate over each parameter, adding it to the result map
    for (i=0; i < pr->num_entries; i++)
    {
        // Skip to next parameter if an error occurred getting the parameter, or no value was obtained for it
        gge = &ggv->vector[pr->index + i];
        if ((gge->err_code != USP_ERR_OK) || (gge->value == NULL))
        {
            continue;
        }

        // Calculate the path name to put into the result map
        err = bulkdata_reduce_to_alt_name(pr->path_expr, gge->path, pr->alt_name, reduced_path, sizeof(reduced_path));
        if (err != USP_ERR_OK)
        {
            USP_ERR_SetMessage("%s: bulkdata_reduce_to_alt_name(%s) failed", __FUNCTION__, gge->path);
            continue; // Skip this parameter, if an error occurred
        }

        // Calculate the JSON type of the parameter
        type = DATA_MODEL_GetJSONParameterType(gge->path);

        // Form the value string containing JSON type character, followed by actual value
        param_type_value[0] = type;
        USP_STRNCPY(&param_type_value[1], gge->value, sizeof(param_type_value)-1);

        // Finally insert the string into the report map
        KV_VECTOR_Add(report_map, reduced_path, param_type_value);
    }
}

/*********************************************************************//**
**
** bulkdata_platform_get_profile_control_params
**
** Obtains all of the controlling parameters (eg reporting interval,
** time reference, URL, username, password etc) for a given profile into a structure
**
** \param   bp - pointer to profile
** \param   ctrl_params - pointer to structure in which to return the control parameters of the specified profile
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_platform_get_profile_control_params(bulkdata_profile_t *bp, profile_ctrl_params_t *ctrl_params)
{
    int err;
    char path[MAX_DM_PATH];

    // Exit if unable to get NumberOfRetainedFailedReports
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.NumberOfRetainedFailedReports", bp->profile_id);
    err = DM_ACCESS_GetInteger(path, &ctrl_params->num_retained_failed_reports);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Limit the maximum number of reports retained
    if (ctrl_params->num_retained_failed_reports == -1)
    {
        ctrl_params->num_retained_failed_reports = BULKDATA_MAX_RETAINED_FAILED_REPORTS;
    }

    // Exit if unable to get UseDateHeader
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.UseDateHeader", bp->profile_id);
    err = DM_ACCESS_GetBool(path, &ctrl_params->use_date_header);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get URL
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.URL", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, ctrl_params->url, sizeof(ctrl_params->url), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get Username
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.Username", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, ctrl_params->username, sizeof(ctrl_params->username), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get Password
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.Password", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, ctrl_params->password, sizeof(ctrl_params->password), SHOW_PASSWORD);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get Compression
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.Compression", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, ctrl_params->compression, sizeof(ctrl_params->compression), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get Method
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.HTTP.Method", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, ctrl_params->method, sizeof(ctrl_params->method), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to get ReportTimestamp
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.JSONEncoding.ReportTimestamp", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, ctrl_params->report_timestamp, sizeof(ctrl_params->report_timestamp), 0);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  bulkdata_start_profile
**
**  Starts the specified bulk data profile
**
** \param   bp - pointer to profile
**
** \return  USP_ERR_OK if operation completed successfully
**
**************************************************************************/
int bulkdata_start_profile(bulkdata_profile_t *bp)
{
    int err;
    int wait_time;


    // Determine the time until the timer should next fire
    wait_time = bulkdata_calc_waittime_to_next_reporting_interval(bp->reporting_interval, bp->time_reference);

    // Exit if unable to start this profile's sync timer
    err = SYNC_TIMER_Add(bulkdata_process_profile, bp->profile_id, time(NULL) + wait_time);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  bulkdata_resync_profile
**
**  Called to update the time until the profile is next serviced
**  (either to send a new report, or retry sending an old report)
**
** \param   bp - pointer to profile
** \param   delta_time - pointer to variable in which to return the amount of time until this timer next fires, or NULL if unused by called
**
** \return  USP_ERR_OK if operation completed successfully
**
**************************************************************************/
int bulkdata_resync_profile(bulkdata_profile_t *bp, int *delta_time)
{
    int err;
    int wait_time;

    // Calculate the amount of time until this profile is next serviced (either to send a new report or resend an old report)
    wait_time = bulkdata_calc_waittime_to_next_send(bp);

    // Save the wait time for the caller, if the caller wants it
    if (delta_time != NULL)
    {
        *delta_time = wait_time;
    }

    // Exit if the profile is not enabled (as it will not have a registered sync timer to reload).
    if ((global_enable == false) || (bp->is_enabled==false))
    {
        return USP_ERR_OK;
    }

    // Exit if unable to restart the sync timer with the time until the next reporting interval (or retry)
    err = SYNC_TIMER_Reload(bulkdata_process_profile, bp->profile_id, time(NULL) + wait_time);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  bulkdata_calc_waittime_to_next_send
**
**  Determines the number of seconds until the report should be sent
**  This may be because of a retry or because we have reached the scheduled report time
**
** \param   bp - pointer to profile
**
** \return
**
**************************************************************************/
unsigned bulkdata_calc_waittime_to_next_send(bulkdata_profile_t *bp)
{
    unsigned wait_time;
    unsigned retry_time;

    // By default the time until we next send a report is the next scheduled time
    wait_time = bulkdata_calc_waittime_to_next_reporting_interval(bp->reporting_interval, bp->time_reference);

    // However if we are retrying to send a report, then the retry might come first
    if (bp->retry_count != 0)
    {
        retry_time = RETRY_WAIT_Calculate(bp->retry_count, bp->retry_minimum_wait_interval, bp->retry_interval_multiplier);
        if (retry_time < wait_time)
        {
            // The retry comes first
            wait_time = retry_time;
        }
        else
        {
            // If the retry doesn't come first, then cancel the retry, because the next send will be a scheduled send
            bp->retry_count = 0;
        }
    }

    return wait_time;
}

/*********************************************************************//**
**
**  bulkdata_calc_waittime_to_next_reporting_interval
**
**  Determines the number of seconds until the report should be regenerated and sent
**
** \param   interval - Device.BulkData.Profile.{i}.ReportingInterval
** \param   time_reference - Device.BulkData.Profile.{i}.TimeReference
**
** \return  Number of seconds until the next scheduled report for this profile should be generated and sent
**
**************************************************************************/
unsigned bulkdata_calc_waittime_to_next_reporting_interval(time_t interval, time_t time_reference)
{
    time_t cur_time;
    time_t wait_time;

    cur_time = time(NULL);
    wait_time = interval - ((cur_time - time_reference) % interval);
    if (wait_time > interval)          // Needed if time_ref > cur_time
    {
        wait_time -= interval;         // Do not use % operator, as we never want to get a value of 0 (want 'interval' instead)
    }

    return wait_time;
}

/*********************************************************************//**
**
**  bulkdata_stop_profile
**
**  Stops the specified bulk data profile
**
** \param   bp - pointer to profile
**
** \return  USP_ERR_OK if operation completed successfully
**
**************************************************************************/
int bulkdata_stop_profile(bulkdata_profile_t *bp)
{
    int err;


    // Free all dynamic memory associated with this profile
    bulkdata_clear_retained_reports(bp);

    // Exit if unable to stop the sync timer
    err = SYNC_TIMER_Remove(bulkdata_process_profile, bp->profile_id);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  bulkdata_process_profile
**
**  Called when it is time to generate a report on a profile
**
** \param   id - identifier of the profile to generate a report on
**
** \return  None
**
**************************************************************************/
void bulkdata_process_profile(int id)
{
    bulkdata_profile_t *bp;
    int err;
    bdc_protocol_t protocol;
    char path[MAX_DM_PATH];

    // Exit if unable to find the profile
    // NOTE: This should never occur if the software is working correctly
    bp = bulkdata_find_profile(id);
    if (bp == NULL)
    {
        return;
    }

    // Exit if unable to get Protocol for this profile
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.Protocol", bp->profile_id);
    err = DM_ACCESS_GetEnum(path, &protocol, bdc_protocols, NUM_ELEM(bdc_protocols));
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Process the report using the required protocol
    switch(protocol)
    {
        case kBdcProtocol_HTTP:
            bulkdata_process_profile_http(bp);
            break;

        case kBdcProtocol_UspEvent:
            bulkdata_process_profile_usp_event(bp);
            break;

        default:
            break;
    }
}

/*********************************************************************//**
**
**  bulkdata_process_profile_http
**
**  Perform the work of processing a profile
**
** \param   bp - pointer to bulk data profile to process
**
** \return  None
**
**************************************************************************/
void bulkdata_process_profile_http(bulkdata_profile_t *bp)
{
    int err;
    report_t *cur_report;
    char *json_report;
    profile_ctrl_params_t ctrl;
    unsigned char *compressed_report;
    int compressed_len;
    char buf[48];

    // Exit if unable to obtain the control parameters for this profile
    err = bulkdata_platform_get_profile_control_params(bp, &ctrl);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // If we are not retrying to send a failed report(s) then append the report map for this reporting interval
    if (bp->retry_count == 0)
    {
        // Drop the oldest retained reports, if we would store more than we're meant to
        if (bp->num_retained_reports > ctrl.num_retained_failed_reports)
        {
            bulkdata_drop_oldest_retained_reports(bp, ctrl.num_retained_failed_reports);
        }

        // Append the report map for this reporting interval
        cur_report = &bp->reports[bp->num_retained_reports];
        cur_report->collection_time = time(NULL);

        KV_VECTOR_Init(&cur_report->report_map);

        // Exit if unable to get the map containing the report contents
        err = bulkdata_calc_report_map(bp, &cur_report->report_map);
        if (err != USP_ERR_OK)
        {
            USP_ERR_SetMessage("%s: bulkdata_calc_report_map failed", __FUNCTION__);
            return;
        }
        bp->num_retained_reports++;
    }

    // Exit if unable to generate the report
    json_report = bulkdata_generate_json_report(bp, ctrl.report_timestamp);
    if (json_report == NULL)
    {
        USP_ERR_SetMessage("%s: bulkdata_generate_json_report failed", __FUNCTION__);
        return;
    }

    // Print out the JSON report, if debugging is enabled
    USP_LOG_Info("\nBULK DATA: %sing at time %s, to url=%s", ctrl.method, iso8601_cur_time(buf, sizeof(buf)), ctrl.url);
    USP_LOG_Info("BULK DATA: using compression method=%s", ctrl.compression);
    if (enable_protocol_trace)
    {
        USP_LOG_String(kLogLevel_Info, kLogType_Protocol, json_report);
    }

    // Compress the report, if enabled
    compressed_report = bulkdata_compress_report(&ctrl, json_report, strlen(json_report), &compressed_len);
    if (compressed_report != (unsigned char *)json_report)
    {
        free(json_report);
    }
    // NOTE: From this point on, only the compressed_report exists

    // Exit if failed to tell BDC thread to send the report
    err = bulkdata_schedule_sending_http_report(&ctrl, bp, compressed_report, compressed_len);
    if (err != USP_ERR_OK)
    {
        DEVICE_BULKDATA_NotifyTransferResult(bp->profile_id, kBDCTransferResult_Failure_Other);
    }
}

/*********************************************************************//**
**
**  bulkdata_process_profile_usp_event
**
**  Send the json report using USP Push! event
**
** \param   bp - pointer to bulk data profile to process
**
** \return  None
**
**************************************************************************/
void bulkdata_process_profile_usp_event(bulkdata_profile_t *bp)
{
    int err;
    char path[MAX_DM_PATH];
    kv_vector_t event_args;
    kv_pair_t kv;
    report_t *cur_report;
    char *json_report;
    char report_timestamp[33];

    // Exit if the MTP has not been connected to successfully after bootup
    // This is to prevent BDC events being enqueued before the Boot! event is sent (the Boot! event is only sent after successfully connecting to the MTP).
    if (DM_EXEC_IsNotificationsEnabled() == false)
    {
        goto exit;
    }

    // Exit if unable to get ReportTimestamp
    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.JSONEncoding.ReportTimestamp", bp->profile_id);
    err = DATA_MODEL_GetParameterValue(path, report_timestamp, sizeof(report_timestamp), 0);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // When sending via USP events, only one report is ever sent in each USP event
    // So ensure all retained reports are removed. NOTE: Clearing the reports here is only necessary when switching protocol from HTTP to USP event, and where HTTP had some unsent reports
    bulkdata_clear_retained_reports(bp);
    cur_report = &bp->reports[0];
    cur_report->collection_time = time(NULL);
    KV_VECTOR_Init(&cur_report->report_map);

    // Exit if unable to get the map containing the report contents
    err = bulkdata_calc_report_map(bp, &cur_report->report_map);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: bulkdata_calc_report_map failed", __FUNCTION__);
        return;
    }
    bp->num_retained_reports = 1;

    // Exit if unable to generate the report
    json_report = bulkdata_generate_json_report(bp, report_timestamp);
    if (json_report == NULL)
    {
        USP_ERR_SetMessage("%s: bulkdata_generate_json_report failed", __FUNCTION__);
        return;
    }

    // NOTE: No need to print out the JSON report here, the USP message trace will contain it

    // Construct event_args manually to avoid the overhead of a malloc and copy of the report in KV_VECTOR_Add()
    kv.key = "Data";
    kv.value = json_report;
    event_args.vector = &kv;
    event_args.num_entries = 1;

    USP_SNPRINTF(path, sizeof(path), "Device.BulkData.Profile.%d.Push!", bp->profile_id);
    DEVICE_SUBSCRIPTION_ProcessAllEventCompleteSubscriptions(path, &event_args);

    // Free the report. No need to free the event_args as json_report is the only thing dynamically allocated in it
    free(json_report);      // The report is not allocated via USP_MALLOC

    // From the point of view of this code, the report(s) have been successfully sent, so don't retain them
    // NOTE: Sending of the reports successfully is delegated to the USP notification retry mechanism
    bulkdata_clear_retained_reports(bp);
    bp->retry_count = 0;

exit:
    // Exit if unable to restart the sync timer with the time until the next reporting interval
    err = bulkdata_resync_profile(bp, NULL);
    if (err != USP_ERR_OK)
    {
        return;
    }

    // Uncomment the next line to see how much memory is in use by USP Agent after each post
//    USP_MEM_PrintSummary();
}

/*********************************************************************//**
**
**  bulkdata_drop_oldest_retained_reports
**
**  Drops the oldest retained reports to get the number of reports retained down to the specified number
**
** \param   bp - pointer to bulk data profile containing reports
** \param   num_reports_to_keep - the total number of failed reports to retain
**
** \return  None
**
**************************************************************************/
void bulkdata_drop_oldest_retained_reports(bulkdata_profile_t *bp, int num_reports_to_keep)
{
    int i;
    int reports_to_destroy;

    // Destroy the oldest report maps
    reports_to_destroy = bp->num_retained_reports - num_reports_to_keep;
    for (i=0; i<reports_to_destroy; i++)
    {
        KV_VECTOR_Destroy(&bp->reports[i].report_map);
    }

    // Move down the reports to keep
    if (num_reports_to_keep > 0)
    {
        memmove(&bp->reports[0], &bp->reports[reports_to_destroy], num_reports_to_keep*sizeof(report_t));
    }

    bp->num_retained_reports = num_reports_to_keep;
}

/*********************************************************************//**
**
**  bulkdata_clear_retained_reports
**
**  Clears out all of the retained reports
**
** \param   bp - pointer to bulk data profile to clear all report maps
**
** \return  None
**
**************************************************************************/
void bulkdata_clear_retained_reports(bulkdata_profile_t *bp)
{
    int i;
    report_t *r;

    for (i=0; i < bp->num_retained_reports; i++)
    {
        r = &bp->reports[i];
        KV_VECTOR_Destroy(&r->report_map);
        r->collection_time = 0;
    }

    bp->num_retained_reports = 0;
    bp->retry_count = 0;
}

/*********************************************************************//**
**
**  bulkdata_reduce_to_alt_name
**
**  Reduce the full path name to one that (potentially) contains an alternative name
**  See TR-157 section A.3.2 for the rules involving reduction
**
** \param   spec - path specification (This may be a partial path, or contain wildcards, and one of it's expansions was 'path')
** \param   path - fully expanded data model path
** \param   alt_name - alternative name for the above path
** \param   out_buf - pointer to buffer in which to return the reduced name
** \param   buf_len - size of buffer in which to return the reduced name
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_reduce_to_alt_name(char *spec, char *path, char *alt_name, char *out_buf, int buf_len)
{
    char *s;    // pointer stepping thru spec
    char *p;    // pointer stepping thru path
    char *o;    // pointer stepping thru out_buf
    char *t;    // temp pointer

    memset(out_buf, 0, buf_len);

    // Exit if no alt_name - no reduction necessary
    if (*alt_name == '\0')
    {
        USP_STRNCPY(out_buf, path, buf_len);
        return USP_ERR_OK;
    }


    // Exit if out_buf is not large enough to hold initial alt_name root
    if (buf_len < strlen(alt_name) + 1)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    p = path;
    s = spec;
    USP_STRNCPY(out_buf, alt_name, buf_len);  // If there is an alt_name specified, this always comes first
    o = &out_buf[strlen(out_buf)];
    *o++ = '.';     // add trailing delimiter, assuming there will be other nodes to add

    while (*p != '\0')
    {
        // Exit if out_buf is not large enough to hold any more
        if (o >= &out_buf[buf_len]-1)
        {
            return USP_ERR_INTERNAL_ERROR;
        }

        if (*p == *s)
        {
            // Skip character, if path received equals spec
            p++;
            s++;
        }
        else
        {
            if (*s == '*')
            {
                // Copy index characters at wildcard
                *o++ = *p++;
                if (*p == '.')  // Only move off the wildcard character when we have copied all of the index characters
                {
                    s++;
                    *o++ = '.';
                }
            }
            else if (*s == '[')
            {
                // Copy index characters at search expression
                *o++ = *p++;
                if (*p == '.')  // Only move after the search expression when we have copied all of the index characters
                {
                    // Find the end of the unique key
                    // NOTE: We should always find the end of the unique key, as the code shouldn't be performing alt name reduction unless the path expression is valid
                    t = strchr(s, ']');
                    if (t != NULL)
                    {
                        s = t;
                    }
                    s++;
                    *o++ = '.';
                }
            }
            else
            {
                // Copy characters at end of path, avoiding copying double '.' into output (This is necessary if spec is a partial path that does not end in a '.)
                if ((o[-1] == '.') && (*p == '.'))
                {
                    p++;    // case of double '.', skip second '.'
                }
                else
                {
                    *o++ = *p++;
                }
            }
        }
    }

    // Remove any trailing delimiter
    if (o[-1] == '.')
    {
        o[-1] = '\0';
    }

    // Ensure buffer is zero terminated
    *o = '\0';
    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  bulkdata_generate_json_report
**
**  Generates a JSON name-value pair format report
**  NOTE: The report contains all retained failed reports, as well as the current report
**  See TR-157 section A.4.2 (end) for an example, and section A.3.5.2 for layout of content containing failed report transmissions
**
** \param   bp - pointer to bulk data profile containing all reports (current and retained)
** \param   report_timestamp - value of Device.BulkData.Profile.{i}.JSONEncoding.ReportTimestamp
**
** \return  pointer to NULL terminated dynamically allocated buffer containing the serialized report to send
**
**************************************************************************/
char *bulkdata_generate_json_report(bulkdata_profile_t *bp, char *report_timestamp)
{
    JsonNode *top;          // top of report
    JsonNode *array;        // array of reports (retained + current)
    JsonNode *element;      // element of json array, containing an individual report
    char *param_path;
    char *param_type_value;
    char param_type;
    char *param_value;
    kv_vector_t *report_map;
    report_t *report;
    double value_as_number;
    long long value_as_ll;
    unsigned long long value_as_ull;
    bool value_as_bool;
    char *result;
    int i, j;
    char buf[32];
    kv_pair_t *kv;
    int err;

    top = json_mkobject();
    array = json_mkarray();

    // Iterate over all reports adding them to the JSON array
    for (i=0; i < bp->num_retained_reports; i++)
    {
        report = &bp->reports[i];
        report_map = &report->report_map;

        // Add Collection time to each json report element (only if specified and not 'None')
        element = json_mkobject();
        if (strcmp(report_timestamp, "Unix-Epoch")==0)
        {
            json_append_member(element, "CollectionTime", json_mknumber(report->collection_time));
        }
        else if (strcmp(report_timestamp, "ISO-8601")==0)
        {
            result = iso8601_from_unix_time(report->collection_time, buf, sizeof(buf));
            if (result != NULL)
            {
                json_append_member(element, "CollectionTime", json_mkstring(buf));
            }
        }

        // Iterate over each parameter, adding it to the json element. Take account of the parameter's type
        for (j=0; j < report_map->num_entries; j++)
        {
            kv = &report_map->vector[j];
            param_path = kv->key;
            param_type_value = kv->value;
            param_type = param_type_value[0];           // First character denotes the type of the parameter
            param_value = &param_type_value[1];         // Subsequent characters contain the parameter's value

            switch (param_type)
            {
                case 'S':
                    json_append_member(element, param_path, json_mkstring(param_value) );
                    break;

                case 'U':
                    value_as_ull = strtoull(param_value, NULL, 10);
                    json_append_member(element, kv->key, json_mkulonglong(value_as_ull) );
                    break;

                case 'L':
                    value_as_ll = strtoll(param_value, NULL, 10);
                    json_append_member(element, kv->key, json_mklonglong(value_as_ll) );
                    break;

                case 'N':
                    value_as_number = atof(param_value);
                    json_append_member(element, param_path, json_mknumber(value_as_number) );
                    break;

                case 'B':
                    err = TEXT_UTILS_StringToBool(param_value, &value_as_bool);
                    if (err == USP_ERR_OK)
                    {
                        json_append_member(element, param_path, json_mkbool(value_as_bool) );
                    }
                    break;

                default:
                    USP_ERR_SetMessage("%s: Invalid JSON parameter type ('%c') in report map for %s", __FUNCTION__, param_type_value[0], param_path);
                    break;
            }
        }

        // Add the json element to the json array
        json_append_element(array, element);
    }

    // Finally add the array to the report top level
    json_append_member(top, "Report", array);

    // Serialize the JSON tree
    result = json_stringify(top, " ");

    // Clean up the JSON tree
    json_delete(top);        // Other JsonNodes which are children of this top level tree will be deleted

    return result;
}

/*********************************************************************//**
**
**  bulkdata_compress_report
**
**  Compresses the report to send
**
** \param   ctrl - parameters controlling the profile e.g. type of compression to use
** \param   input_buf - pointer to buffer containing the uncompressed report
** \param   input_len - length of the data in the buffer containing the uncompressed report
** \param   p_output_len - pointer to variable in which to return the length of the compressed report
**
** \return  pointer to compressed report to send
**          NOTE: if compression is not required or fails, the uncompressed report is returned
**
**************************************************************************/
unsigned char *bulkdata_compress_report(profile_ctrl_params_t *ctrl, char *input_buf, int input_len, int *p_output_len)
{
    z_stream zlib_ctx;
    int err;
    int output_len;
    unsigned char *output_buf;

    // Exit if not GZIP compression - in this case we just don't compress the report
    if (strcmp(ctrl->compression, "GZIP") !=0)
    {
        *p_output_len = input_len;
        return (unsigned char *)input_buf;
    }

    // Initialise the zlib context
    memset(&zlib_ctx, 0, sizeof(zlib_ctx));
    zlib_ctx.zalloc = Z_NULL;
    zlib_ctx.zfree = Z_NULL;
    zlib_ctx.opaque = NULL;

    // Exit if unable to start deflate
    #define WINDOW_BITS  (15+16)  // Plus 16 to get a gzip wrapper, as suggested by the zlib documentation
    #define MEM_LEVEL 8           // This is the default value, as suggested by the zlib documentation
    err = deflateInit2(&zlib_ctx, Z_DEFAULT_COMPRESSION, Z_DEFLATED, WINDOW_BITS, MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (err != Z_OK)
    {
        USP_LOG_Warning("%s: WARNING: deflateInit2 returned %d. Falling back to sending uncompressed data", __FUNCTION__, err);
        *p_output_len = input_len;
        return (unsigned char *)input_buf;
    }

    // Allocate a worst case buffer to hold the compressed data
    output_len = (int)deflateBound(&zlib_ctx, input_len);
    output_buf = malloc(output_len);  // Use malloc because the uncompressed report was generated with malloc() and this needs to be consistent
    if (output_buf == NULL)
    {
        USP_LOG_Warning("%s: WARNING: malloc failed. Falling back to sending uncompressed data", __FUNCTION__);
        deflateEnd(&zlib_ctx);
        *p_output_len = input_len;
        return (unsigned char *)input_buf;
    }

    // Initialise the zlib context for this compression
    zlib_ctx.next_in  = (unsigned char *)input_buf;
    zlib_ctx.avail_in = input_len;
    zlib_ctx.next_out = output_buf;
    zlib_ctx.avail_out= output_len;

    // Exit if compression failed
    err = deflate(&zlib_ctx, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        USP_LOG_Warning("%s: WARNING: deflate failed (err=%d). Falling back to sending uncompressed data", __FUNCTION__, err);
        deflateEnd(&zlib_ctx);
        free(output_buf);
        *p_output_len = input_len;
        return (unsigned char *)input_buf;
    }

    // Deallocate all compression state stored in the zlib context
    // NOTE: We ignore errors from this and just log them
    err = deflateEnd(&zlib_ctx);
    if (err != Z_OK)
    {
        USP_LOG_Warning("%s: WARNING: deflateEnd failed (err=%d, %s). Ignoring error.", __FUNCTION__, err, zlib_ctx.msg);
    }

    USP_LOG_Info("%s: BulkDataReport(uncompressed size=%d, compressed size=%lu)", __FUNCTION__, input_len, zlib_ctx.total_out);
    *p_output_len = zlib_ctx.total_out;
    return output_buf;
}

/*********************************************************************//**
**
**  bulkdata_schedule_sending_http_report
**
**  Tells the BDC thread to send the report
**  NOTE: Ownership of report passes to BDC_EXEC
**
** \param   ctrl - parameters controlling the profile e.g. URL to upload report to
** \param   bp - pointer to bulk data profile to get the report map for
** \param   report - pointer to buffer containing the report to send (which may be compressed)
** \param   report_len - length of json_report
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int bulkdata_schedule_sending_http_report(profile_ctrl_params_t *ctrl, bulkdata_profile_t *bp, unsigned char *report, int report_len)
{
    char *query_string = NULL;
    char *full_url = NULL;
    unsigned flags;
    int err;
    char *username;
    char *password;

    // Exit if URL is empty
    if (ctrl->url[0] == '\0')
    {
        USP_LOG_Error("%s: Profile %d started but it's URL has not been setup", __FUNCTION__, bp->profile_id);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Exit if unable to generate the URI query string
    query_string = bulkdata_platform_get_uri_query_params(bp->profile_id);
    if (query_string == NULL)
    {
        USP_ERR_SetMessage("%s: bulkdata_platform_get_uri_query_params failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }
    USP_LOG_Info("BULK DATA: uri_query_string='%s'", query_string);

    // Create the full URL containing the base url and appended query string
    full_url = USP_MALLOC(strlen(ctrl->url) + strlen(query_string) + 1); // Plus 1 to include NULL terminator
    strcpy(full_url, ctrl->url);
    strcat(full_url, query_string);

    // Create a copy of the auth credentials, to pass ownership to the BDC thread
    username = USP_STRDUP(ctrl->username);
    password = USP_STRDUP(ctrl->password);

    // Form the flags controlling various BDC options
    flags = 0;
    if (strcmp(ctrl->method, "PUT")==0)
    {
        flags |= BDC_FLAG_PUT;
    }

    if (strcmp(ctrl->compression, "GZIP")==0)
    {
        flags |= BDC_FLAG_GZIP;
    }

    if (ctrl->use_date_header)
    {
        flags |= BDC_FLAG_DATE_HEADER;
    }

    // Exit if failed to post a message to BDC thread
    // NOTE: Ownership of full_url, query_string, report, username and password passes to BDC_EXEC
    err = BDC_EXEC_PostReportToSend(bp->profile_id, full_url, query_string, username, password, report, report_len, flags);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  bulkdata_find_free_profile
**
**  Find a free profile slot
**
** \param   None
**
** \return  pointer to a free profile slot, or NULL if unable to fins a free profile
**
**************************************************************************/
bulkdata_profile_t *bulkdata_find_free_profile(void)
{
    int i;
    bulkdata_profile_t *bp;

    // Iterate over all profiles, exiting if a free one has been found
    for (i=0; i<BULKDATA_MAX_PROFILES; i++)
    {
        bp = &bulkdata_profiles[i];
        if (bp->profile_id == INVALID)
        {
            return bp;
        }
    }

    // If the code gets here, no matching profile was found
    return NULL;
}

/*********************************************************************//**
**
**  bulkdata_find_profile
**
**  Find the specified profile
**
** \param   profile_id - Instance number of profile in Device.Bulkdata.Profile.{i}
**
** \return  pointer to a free profile slot, or NULL if unable to find a free profile
**
**************************************************************************/
bulkdata_profile_t *bulkdata_find_profile(int profile_id)
{
    int i;
    bulkdata_profile_t *bp;

    // Iterate over all profiles, exiting if a matching one has been found
    for (i=0; i<BULKDATA_MAX_PROFILES; i++)
    {
        bp = &bulkdata_profiles[i];
        if (bp->profile_id == profile_id)
        {
            return bp;
        }
    }

    // If the code gets here, no matching profile was found
    return NULL;
}

//------------------------------------------------------------------------------------------
// Code to test the bulkdata_reduce_to_alt_name() function
#if 0
char *reduce_to_alt_test_cases[] =
{
    // Path Expression                      // Resolved path                        // Expected Result

    // fully qualified
    "Device.Stuff.Hello",                   "Device.Stuff.Hello",                   "alt",

    // partial path
    "Device.Stuff.Hello.",                  "Device.Stuff.Hello.Obj.1.Param",       "alt.Obj.1.Param",

    // partial path, no trailing '.' in expression
    "Device.Stuff.Hello",                   "Device.Stuff.Hello.Obj.2.Param",       "alt.Obj.2.Param",

    //------------------------
    // basic wildcard
    "Device.1.Stuff.*.Hello",               "Device.1.Stuff.72.Hello",              "alt.72",

    // wildcard with partial path
    "Device.*.ObjA.",                       "Device.56.ObjA.ObjB.7.Param",          "alt.56.ObjB.7.Param",

    // 2 wildcards
    "Device.*.Stuff.*.Hello",               "Device.56.Stuff.72.Hello",             "alt.56.72",

    // 2 wildcards with partial path
    "Device.*.Stuff.*.Hello.",              "Device.56.Stuff.72.Hello.Obj.3.Param", "alt.56.72.Obj.3.Param",

    // wildcard with partial path, but path expression does not contain a trailing '.'
    "Device.*.Stuff.*.Hello",               "Device.56.Stuff.72.Hello.Obj.4.Param", "alt.56.72.Obj.4.Param",

    //------------------------
    // basic search expression
    "Device.[Param == \"string\"].Param",   "Device.56.Param",                      "alt.56",

    // search expression with partial path
    "Device.[Param == \"string\"].ObjA.",   "Device.56.ObjA.ObjB.7.Param",          "alt.56.ObjB.7.Param",

    // 2 search expressions
    "Device.[A==1].Stuff.[B==2].Hello",     "Device.56.Stuff.72.Hello",             "alt.56.72",

    // 2 search expressions with partial path
    "Device.[A==1].Stuff.[B==2].Hello.",    "Device.56.Stuff.72.Hello.Obj.3.Param", "alt.56.72.Obj.3.Param",

    // 2 search expressions with partial path, but path expression does not contain a trailing '.'
    "Device.[A==1].Stuff.[B==2].Hello",     "Device.56.Stuff.72.Hello.Obj.4.Param", "alt.56.72.Obj.4.Param",
};

void Test_ReduceToAltName(void)
{
    int i;
    int err;
    char buf[MAX_DM_PATH];

    for (i=0; i < NUM_ELEM(reduce_to_alt_test_cases); i+=3)
    {
        err = bulkdata_reduce_to_alt_name(reduce_to_alt_test_cases[i], reduce_to_alt_test_cases[i+1], "alt", buf, sizeof(buf));
        if ((err != USP_ERR_OK) || (strcmp(buf, reduce_to_alt_test_cases[i+2]) != 0))
        {
            printf("ERROR: [%d] Test case result for '%s => %s' is '%s' (expected '%s')\n", i/3, reduce_to_alt_test_cases[i], reduce_to_alt_test_cases[i+1], buf, reduce_to_alt_test_cases[i+2]);
        }
    }
}
#endif

#endif // REMOVE_DEVICE_BULKDATA
