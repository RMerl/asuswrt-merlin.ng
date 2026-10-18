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
 * \file subs_retry.c
 *
 * Retries sending notification requests for subscriptions that have Device.LocalAgent.Subscription.{i}.NotifRetry set
 *
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "common_defs.h"
#include "msg_handler.h"
#include "iso8601.h"
#include "device.h"
#include "sync_timer.h"
#include "retry_wait.h"

#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
#include "e2e_context.h"
#endif

//------------------------------------------------------------------------
// Structure containing NotifyRequest message to retry sending and associated state machine
typedef struct
{
    int instance;               // Instance number of subscription that generated this message in Device.LocalAgent.Subscription.{i}
    char *msg_id;               // message_id allocated by this agent to uniquely identify this message
    char *subscription_id;      // id allocated by the controller, to uniquely identify the subscription
    char *dest_endpoint;        // controller to send this message to
    char *differentiator;       // string used to differentiate multiple messages being generated from the same subscription
                                // eg for a value change subscription, multiple messages are differentiated by data model path
                                // NOTE: This value might be NULL if the type of subscription cannot generate multiple messages
    unsigned char *pbuf;        // pointer to serialized USP message
    int  pbuf_len;              // length of serialized USP message

    time_t retry_expiry_time;   // Expiry time for this message
    int retry_count;            // Count of number of times this message will have been retried when next_retry_time has been reached (and the message retried)
    unsigned min_wait_interval; // Minimum wait interval parameter for RETRY_WAIT calculation
    unsigned interval_multiplier;// Interval multiplier parameter for RETRY_WAIT calculation

    time_t next_retry_time;     // Time at which the message should next be retried to be sent
} subs_retry_t;

//------------------------------------------------------------------------
// Array of subscription messages that should receive a response from the controller, or be retried
typedef struct
{
    int num_entries;
    subs_retry_t *vector;
} subs_retry_vector_t;

static subs_retry_vector_t subs_retry;

//------------------------------------------------------------------------
// Time at which first message to be retried, is to be retried
static time_t first_retry_time = END_OF_TIME;

//------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void SubsRetryExec(int id);
subs_retry_t *FindRetryEntry(int instance, char *differentiator);
time_t CalcNextSubsRetryTime(subs_retry_t *sr);
void DestroySubsRetryEntry(subs_retry_t *sr);
void UpdateFirstRetryTime(void);
void GarbageCollectSubsRetry(void);

/*********************************************************************//**
**
** SUBS_RETRY_Init
**
** Initialises the vector of subscriptions to retry
**
** \param   None
**
** \return  None
**
**************************************************************************/
void SUBS_RETRY_Init(void)
{
    subs_retry.num_entries = 0;
    subs_retry.vector = NULL;
    SYNC_TIMER_Add(SubsRetryExec, 0, END_OF_TIME);
}

/*********************************************************************//**
**
** SUBS_RETRY_Stop
**
** Frees all memory used by this component
**
** \param   None
**
** \return  None
**
**************************************************************************/
void SUBS_RETRY_Stop(void)
{
    int i;
    subs_retry_t *sr;

    for (i=0; i<subs_retry.num_entries; i++)
    {
        sr = &subs_retry.vector[i];
        USP_SAFE_FREE(sr->msg_id);
        USP_SAFE_FREE(sr->subscription_id);
        USP_SAFE_FREE(sr->dest_endpoint);
        USP_SAFE_FREE(sr->differentiator);
        USP_SAFE_FREE(sr->pbuf);
    }

    USP_SAFE_FREE(subs_retry.vector);
}

/*********************************************************************//**
**
** SUBS_RETRY_Add
**
** Adds the specified message to the list of messages to be retried if a response is not obtained
**
** \param   instance - Instance number of Subscription in Device.LocalAgent.Subscription.{i}
** \param   msg_id - message_id allocated by this agent to uniquely identify this message
** \param   subscription_id - id allocated by the controller, to uniquely identify the subscription
** \param   dest_endpoint - controller to send this message to
** \param   differentiator - string used to differentiate multiple messages being generated from the same subscription
**                           eg for a value change subscription, multiple messages are differentiated by data model path
**                           NOTE: This value might be NULL if the type of subscription cannot generate multiple messages
** \param   pbuf - pointer to serialized USP message
**                 NOTE: Ownership of the serialized USP message passes to this module
** \param   pbuf_len - length of protobuf binary message
** \param   retry_expiry_time - time at which retrying to send this message should stop
**
** \return  None
**
**************************************************************************/
void SUBS_RETRY_Add(int instance, char *msg_id, char *subscription_id, char *dest_endpoint, char *differentiator,
                    unsigned char *pbuf, int pbuf_len, time_t retry_expiry_time)
{
    int err;
    int new_num_entries;
    subs_retry_t *sr;
    unsigned min_wait_interval;
    unsigned interval_multiplier;

    // Exit if this controller has been disabled or deleted.
    // In this case, we do not bother retrying, and we make sure that we delete the existing entry (if one exists)
    err = DEVICE_CONTROLLER_GetSubsRetryParams(dest_endpoint, &min_wait_interval, &interval_multiplier);
    if (err != USP_ERR_OK)
    {
        sr = FindRetryEntry(instance, differentiator);
        if (sr != NULL)
        {
            USP_LOG_Warning("%s: Aborting sending subscription_id=%s because controller is disabled or deleted", __FUNCTION__, subscription_id);
            DestroySubsRetryEntry(sr);
            GarbageCollectSubsRetry();
            return;
        }
    }

    // See if this retry needs to replace an existing retry
    // This could be the case if a NotifyResponse has not been received, and the parameter's value has changed again
    sr = FindRetryEntry(instance, differentiator);
    if (sr == NULL)
    {
        // Add new retry entry
        new_num_entries = subs_retry.num_entries + 1;
        subs_retry.vector = USP_REALLOC(subs_retry.vector, new_num_entries*sizeof(subs_retry_t));
        sr = &subs_retry.vector[ subs_retry.num_entries ];
        subs_retry.num_entries = new_num_entries;
    }
    else
    {
        // Replace an existing entry. First free memory associated with current entry - but leave the entry allocated,
        // as we will replace it
        DestroySubsRetryEntry(sr);
    }

    // Fill in this entry
    sr->instance = instance;
    sr->msg_id = USP_STRDUP(msg_id);
    sr->subscription_id = USP_STRDUP(subscription_id);
    sr->dest_endpoint = USP_STRDUP(dest_endpoint);
    if (differentiator != NULL)
    {
        sr->differentiator = USP_STRDUP(differentiator);
    }
    else
    {
        sr->differentiator = NULL;
    }

    sr->pbuf = pbuf;
    sr->pbuf_len = pbuf_len;

    sr->retry_expiry_time = retry_expiry_time;
    sr->retry_count = 1;
    sr->min_wait_interval = min_wait_interval;
    sr->interval_multiplier = interval_multiplier;

    sr->next_retry_time = CalcNextSubsRetryTime(sr);
    USP_LOG_Info("Retrying sending notification (retry_count=%d) in %d seconds.", sr->retry_count, (int)(sr->next_retry_time-time(NULL)) );

    // Update time until next retry is sent
    UpdateFirstRetryTime();
}

/*********************************************************************//**
**
** SUBS_RETRY_Remove
**
** Removes the specified message from the retry list
** This function is called in response to a NotifyResponse being received from the controller
**
** \param   msg_id - string identifying the NotifyRequest message sent by this agent, that the controller is responding to
** \param   subscription_id - string set by the controller in Device.LocalAgent.Subscription.{i].ID
**
** \return  None
**
**************************************************************************/
void SUBS_RETRY_Remove(char *msg_id, char *subscription_id)
{
    int i;
    subs_retry_t *sr;

    // Iterate over all retry entries, finding the first entry that matches the one the controller is responding to
    // NOTE: USP Spec 1.2 clarified that subscription_id should be ignored, so only msg_id needs to match
    for (i=0; i < subs_retry.num_entries; i++)
    {
        sr = &subs_retry.vector[i];
        if (strcmp(sr->msg_id, msg_id) == 0)
        {
            // Remove this entry. We have had a response from the controller, so do not have to retry it anymore
            USP_LOG_Info("%s: Removing Notification retry for msg_id=%s (NotifyResponse received)", __FUNCTION__, msg_id);
            DestroySubsRetryEntry(sr);
            GarbageCollectSubsRetry();

            // Update time until next retry is sent
            UpdateFirstRetryTime();
            return;
        }
    }

    // If the code gets here, no matching NotifyRequest has been found to cancel, so just log this fact
    USP_LOG_Warning("%s: Ignoring NotifyResponse. Unknown msg_id=%s, subscription_id=%s", __FUNCTION__, msg_id, subscription_id);
}

/*********************************************************************//**
**
** SUBS_RETRY_Delete
**
** Removes all messages (specified by subscription instance) from the retry list
** This function is called when a subscription is deleted (either by a controller or automatically by the TimeToLive timeout)
**
** \param   instance - Instance number of Subscription in Device.LocalAgent.Subscription.{i}
**
** \return  None
**
**************************************************************************/
void SUBS_RETRY_Delete(int instance)
{
    int i;
    subs_retry_t *sr;

    // Iterate over all retries, removing all entries which were generated by the subscription
    for (i=0; i < subs_retry.num_entries; i++)
    {
        sr = &subs_retry.vector[i];
        if (sr->instance == instance)
        {
            // Mark this entry for deletion
            USP_LOG_Info("%s: Removing Notification retry for msg_id=%s (Subscription deleted)", __FUNCTION__, sr->msg_id);
            DestroySubsRetryEntry(sr);
        }
    }

    // Remove all entries marked for deletion
    GarbageCollectSubsRetry();

    // Update time until next retry is sent
    UpdateFirstRetryTime();
}

/*********************************************************************//**
**
** SubsRetryExec
**
** Retry sending all messages that have reached the time they need to be resent
** and calculate the time at which the next retry message should be sent
** This function is called back from a timer when it is time for a periodic notification to fire
**
** \param   id - (unused) identifier of the sync timer which caused this callback
**
** \return  None
**
**************************************************************************/
void SubsRetryExec(int id)
{
    int i;
    subs_retry_t *sr;
    time_t cur_time;
    char buf[MAX_ISO8601_LEN];
    mtp_conn_t mtp_conn = {0};  // Ensures mtp_conn.is_reply_to_specified=false

    cur_time = time(NULL);
    USP_ASSERT(cur_time >= first_retry_time);

    // Iterate over all retry entries, retrying all of those for which it is time to retry
    // Also calculate the time until the next retry should occur
    for (i=0; i < subs_retry.num_entries; i++)
    {
        // Mark this retry entry for deletion if it has reached the time where we give up retrying
        sr = &subs_retry.vector[i];
        if (cur_time >= sr->retry_expiry_time)
        {
            USP_LOG_Info("%s: Removing Notification retry for msg_id=%s (retry period expired at %s)", __FUNCTION__, sr->msg_id, iso8601_cur_time(buf, sizeof(buf)) );
            DestroySubsRetryEntry(sr);
        }
        else
        {
            // Determine if it is time to try resending the message
            if (cur_time >= sr->next_retry_time)
            {
                // Marshal parameters to pass to MSG_HANDLER_QueueUspRecord()
                usp_send_item_t usp_send_item;
                MSG_HANDLER_UspSendItem_Init(&usp_send_item);
                usp_send_item.usp_msg_type = USP__HEADER__MSG_TYPE__NOTIFY;
                usp_send_item.msg_packed = sr->pbuf;
                usp_send_item.msg_packed_size = sr->pbuf_len;
#if defined(E2ESESSION_EXPERIMENTAL_USP_V_1_2)
                usp_send_item.curr_e2e_session = DEVICE_CONTROLLER_FindE2ESessionByEndpointId(sr->dest_endpoint);
                usp_send_item.usp_msg = NULL;
#endif

                // Try resending the saved serialized USP message
                MSG_HANDLER_QueueUspRecord(&usp_send_item, sr->dest_endpoint, sr->msg_id, &mtp_conn, sr->retry_expiry_time);

                // Calculate next time until this message is retried
                sr->retry_count++;
                sr->next_retry_time = CalcNextSubsRetryTime(sr);

                // Mark this retry entry for deletion if the next retry is after the expiry time
                if (sr->next_retry_time >= sr->retry_expiry_time)
                {
                    USP_LOG_Info("%s: Removing Notification retry for msg_id=%s (next retry would be after expiry time)", __FUNCTION__, sr->msg_id);
                    DestroySubsRetryEntry(sr);
                }
                else
                {
                    USP_LOG_Info("%s: Retrying to send NotifyRequest with msg_id=%s. Next retry [%d] in %d seconds.", iso8601_cur_time(buf, sizeof(buf)), sr->msg_id, sr->retry_count, (int)(sr->next_retry_time-cur_time) );
                }
            }
        }
    }

    // Now remove all entries in the vector which have been marked for deletion
    GarbageCollectSubsRetry();

    // Restart the timer to cause this function to be called again when the next retry should occur
    UpdateFirstRetryTime();
}

/*********************************************************************//**
**
** FindRetryEntry
**
** Finds the entry in the retry vector which matches the specified incoming message
**
** \param   instance - Instance number of Subscription in Device.LocalAgent.Subscription.{i}
** \param   differentiator - string used to differentiate multiple messages being generated from the same subscription
**                           eg for a value change subscription, multiple messages are differentiated by data model path
**                           NOTE: This value might be NULL if the type of subscription cannot generate multiple messages
**
** \return  None
**
**************************************************************************/
subs_retry_t *FindRetryEntry(int instance, char *differentiator)
{
    int i;
    subs_retry_t *sr;

    // Iterate over all retries, finding the one which matches the incoming message
    for (i=0; i < subs_retry.num_entries; i++)
    {
        sr = &subs_retry.vector[i];
        if (sr->instance == instance)
        {
            if ((sr->differentiator == NULL) || (strcmp(sr->differentiator, differentiator)==0))
            {
                return sr;
            }
        }
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** CalcNextSubsRetryTime
**
** Calculates the time at which we should retry sending the request message
** (if no NotifyResponse is received in the meantime)
**
** \param   sr - pointer to entry
**
** \return  randomized time at which we should retry sending the request message
**
**************************************************************************/
time_t CalcNextSubsRetryTime(subs_retry_t *sr)
{
    time_t cur_time;
    time_t retry_time;
    unsigned wait_time;

    cur_time = time(NULL);


    wait_time = RETRY_WAIT_Calculate(sr->retry_count, sr->min_wait_interval, sr->interval_multiplier);
    retry_time = cur_time + wait_time;

    return retry_time;
}

/*********************************************************************//**
**
** UpdateFirstRetryTime
**
** Updates the time at which the first retry should fire
**
** \param   None
**
** \return  None
**
**************************************************************************/
void UpdateFirstRetryTime(void)
{
    int i;
    subs_retry_t *sr;
    time_t first;

    // Iterate over all retry entries, finding the first time that any of them fire
    first = END_OF_TIME;
    for (i=0; i < subs_retry.num_entries; i++)
    {
        sr = &subs_retry.vector[i];
        if (sr->next_retry_time < first)
        {
            first = sr->next_retry_time;
        }
    }

    // Restart the timer to send the first retry
    first_retry_time = first;
    SYNC_TIMER_Reload(SubsRetryExec, 0, first_retry_time);
}

/*********************************************************************//**
**
** DestroySubsRetryEntry
**
** Frees all memory associated with a retry entry, and marks it for deletion
**
** \param   sr - pointer to entry to free all memory of
**
** \return  None
**
**************************************************************************/
void DestroySubsRetryEntry(subs_retry_t *sr)
{
    // Free all dynamically allocated parts of this structure
    USP_FREE(sr->msg_id);
    USP_FREE(sr->subscription_id);
    USP_FREE(sr->dest_endpoint);
    USP_SAFE_FREE(sr->differentiator);
    USP_FREE(sr->pbuf);

    // Mark entry for deletion
    memset(sr, 0, sizeof(subs_retry_t));
    sr->instance = INVALID;
}

/*********************************************************************//**
**
** GarbageCollectSubsRetry
**
** Removes all entries in the subs_retry vector which have been marked for deletion
** NOTE: All dynamically allocated memory that they referenced will already have been destroyed (by DestroySubsRetryEntry),
**       so we do not have to do it here
**
** \param   msg_id - string identifying the NotifyRequest message sent by this agent, that the controller is responding to
** \param   subscription_id - string set by the controller in Device.LocalAgent.Subscription.{i].ID
**
** \return  None
**
**************************************************************************/
void GarbageCollectSubsRetry(void)
{
    int i, j;
    subs_retry_t *sr;

    // Remove all entries in the vector which have been marked for deletion by DestroySubsRetryEntry()
    // Iterate over all subscriptions, compacting the vector (Garbage collecting)
    j = 0;
    for (i=0; i < subs_retry.num_entries; i++)
    {
        sr = &subs_retry.vector[i];
        if (sr->instance != INVALID)
        {
            // Copy down later entries in the array, over ones which have been removed
            if (j < i)
            {
                memcpy(&subs_retry.vector[j], sr, sizeof(subs_retry_t));
            }
            j++;
        }
    }

    // Store the number of valid entries left in the vector
    subs_retry.num_entries = j;

    // Free the vector, if it now has no entries
    if (subs_retry.num_entries == 0)
    {
        USP_SAFE_FREE(subs_retry.vector);  // This will also set the vector to NULL
    }
}
