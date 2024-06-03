/*
 *
 * Copyright (C) 2019, Broadband Forum
 * Copyright (C) 2016-2019  CommScope, Inc
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
 * \file subs_vector.c
 *
 * Implements a data structure containing a vector of integers
 *
 */
#include <stdlib.h>
#include <string.h>

#include "common_defs.h"
#include "subs_vector.h"
#include "iso8601.h"
#include "text_utils.h"
#include "device.h"

/*********************************************************************//**
**
** SUBS_VECTOR_Init
**
** Initialises a subscription vector structure
**
** \param   suv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_Init(subs_vector_t *suv)
{
    suv->vector = NULL;
    suv->num_entries = 0;
}

/*********************************************************************//**
**
** SUBS_VECTOR_Add
**
** Adds the specified subscription into the vector of subscriptions
** NOTE: This function does not perform a deep copy of the subscription structure,
**       ownership of the dynamically allocated strings in the subscription pass to the vector
**
** \param   suv - pointer to vector structure to add the subscription to
** \param   sub - pointer to subscription structure to add to the vector
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_Add(subs_vector_t *suv, subs_t *sub)
{
    int new_num_entries;

    new_num_entries = suv->num_entries + 1;
    suv->vector = USP_REALLOC(suv->vector, new_num_entries*sizeof(subs_t));
    memcpy(&suv->vector[ suv->num_entries ], sub, sizeof(subs_t));
    suv->num_entries = new_num_entries;
}

/*********************************************************************//**
**
** SUBS_VECTOR_Remove
**
** Removes the specified subscription from the vector of subscriptions
** All dynamically allocated memory associated with this subscription will be freed
**
** \param   suv - pointer to vector structure to remove the subscription from
** \param   sub - pointer to subscription structure to remove from the vector
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_Remove(subs_vector_t *suv, subs_t *sub)
{
    SUBS_VECTOR_MarkSubscriptionForDeletion(sub);
    SUBS_VECTOR_GarbageCollectSubscriptions(suv);
}

/*********************************************************************//**
**
** SUBS_VECTOR_Destroy
**
** Deallocates all memory associated with the subscriptions vector
**
** \param   suv - pointer to structure to destroy all dynamically allocated memory it contains
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_Destroy(subs_vector_t *suv)
{
    int i;
    subs_t *sub;

    // Exit if vector is already empty
    if (suv->vector == NULL)
    {
        goto exit;
    }

    // Free all dynamically allocated memory associated with each subscription
    for (i=0; i < suv->num_entries; i++)
    {
        sub = &suv->vector[i];
        SUBS_VECTOR_MarkSubscriptionForDeletion(sub);  // NOTE: No need to garbage collect this, we will free all en-masse
    }

    // Free the vector itself
    USP_FREE(suv->vector);

exit:
    // Ensure structure is re-initialised
    suv->vector = NULL;
    suv->num_entries = 0;
}

/*********************************************************************//**
**
** SUBS_VECTOR_DestroySubscriber
**
** Frees all memory allocated to the specified subscriber structure
** NOTE: This function acts on an element of the subscriber vector
**
** \param   str - string version of enumeration
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void SUBS_VECTOR_DestroySubscriber(subs_t *sub)
{
    // Free all dynamically allocated memory associated with this subscriptipon
    USP_SAFE_FREE(sub->subscription_id);

    STR_VECTOR_Destroy(&sub->path_expressions);
    KV_VECTOR_Destroy(&sub->last_values);
    STR_VECTOR_Destroy(&sub->resolved_paths);
}

/*********************************************************************//**
**
** SUBS_VECTOR_GetSubsByInstance
**
** Deallocates all memory associated with the subscriptions vector
**
** \param   instance - instance number of the subscription in the data model (Device.LocalAgent.Subscription.{i})
**
** \return  None
**
**************************************************************************/
subs_t *SUBS_VECTOR_GetSubsByInstance(subs_vector_t *suv, int instance)
{
    int i;
    subs_t *sub;

    // Iterate over all subscriptions, finding the matching instance
    for (i=0; i < suv->num_entries; i++)
    {
        sub = &suv->vector[i];
        if (sub->instance == instance)
        {
            return sub;
        }
    }

    // If the code gets here, then no matching instance was found
    // NOTE: As the vector contains only enabled subscriptions, it might not be in the vector because it was not enabled
    return NULL;
}

/*********************************************************************//**
**
** SUBS_VECTOR_MarkSubscriptionForDeletion
**
** Deletes all dynamically allocated memory associated with the specified subscription,
** then marks the subscription for deletion.
**
** \param   sub - pointer to subscription to delete
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_MarkSubscriptionForDeletion(subs_t *sub)
{
    SUBS_VECTOR_DestroySubscriber(sub);

    // Finally mark the entry as ready for deletion, by marking it as invalid
    sub->instance = INVALID;
    sub->notify_type = kSubNotifyType_Invalid;
}

/*********************************************************************//**
**
** SUBS_VECTOR_GarbageCollectSubscriptions
**
** Removes all subscriptions from the vector that have been marked for deletion
**
** \param   suv - pointer to subscription vector
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_GarbageCollectSubscriptions(subs_vector_t *suv)
{
    int i;
    int j;      // index to copy into
    subs_t *sub;

    // Iterate over all subscriptions, compacting the vector
    j = 0;
    for (i=0; i < suv->num_entries; i++)
    {
        sub = &suv->vector[i];
        if (sub->notify_type != kSubNotifyType_Invalid)
        {
            // Copy down later entries in the array, over ones which have been removed
            if (j < i)
            {
                memcpy(&suv->vector[j], sub, sizeof(subs_t));
            }
            j++;
        }
    }

    // Store the number of valid entries found in the vector
    suv->num_entries = j;
}

/*********************************************************************//**
**
** SUBS_VECTOR_Dump
**
** Logs the contents of the subscription vector
**
** \param   suv - pointer to subscription vector
**
** \return  None
**
**************************************************************************/
void SUBS_VECTOR_Dump(subs_vector_t *suv)
{
    int i, j;
    subs_t *sub;
    char buf[MAX_ISO8601_LEN];

    // Exit if no subscriptions to print
    if (suv->num_entries == 0)
    {
        USP_DUMP("No enabled active subscriptions");
        return;
    }

    // Iterate over all subscriptions in the vector
    for (i=0; i < suv->num_entries; i++)
    {
        sub = &suv->vector[i];
        USP_DUMP("enable=%d", sub->enable);
        USP_DUMP("instance=%d", sub->instance);
        USP_DUMP("cont_instance=%d", sub->cont_instance);
        USP_DUMP("subscription_id=%s", sub->subscription_id);
        USP_DUMP("notification_retry=%d", sub->notification_retry);
        USP_DUMP("notify_type=%s", TEXT_UTILS_EnumToString(sub->notify_type, notify_types, NUM_ELEM(notify_types)) );
        USP_DUMP("expiry_time=%s", iso8601_from_unix_time(sub->expiry_time, buf, sizeof(buf)) );

        // Log all path expressions
        for (j=0; j < sub->path_expressions.num_entries; j++)
        {
            USP_DUMP("path[%d]=%s", j, sub->path_expressions.vector[j]);
        }

        // Log all last values
        for (j=0; j < sub->last_values.num_entries; j++)
        {
            USP_DUMP("last_values[%d] %s => %s", j, sub->last_values.vector[j].key, sub->last_values.vector[j].value);
        }
        USP_DUMP("-");

    }
}

