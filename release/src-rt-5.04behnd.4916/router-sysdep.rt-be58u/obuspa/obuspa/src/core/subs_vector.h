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
 * \file subs_vector.h
 *
 * Implements a vector of subscriptions
 *
 */

#ifndef SUBS_VECTOR_H
#define SUBS_VECTOR_H

#include <time.h>

#include "common_defs.h"
#include "str_vector.h"
#include "kv_vector.h"

//------------------------------------------------------------------------------
// Enumeration of types of subscriptions
typedef enum
{
    kSubNotifyType_Invalid = INVALID,   // Used to mark a subscription for garbage collection and also as a return value if we failed to convert the textual representation of this enum
    kSubNotifyType_None = 0,            // This is the default value for the notification type parameter - indicates none setup yet
    kSubNotifyType_ValueChange = 1,
    kSubNotifyType_ObjectCreation,
    kSubNotifyType_ObjectDeletion,
    kSubNotifyType_OperationComplete,
    kSubNotifyType_Event,

    kSubNotifyType_Max                  // This should always be the last value in this enumeration. It is used to statically size arrays based on one entry for each active enumeration
} subs_notify_t;

//------------------------------------------------------------------------------
// Element of subscription vector
typedef struct
{
    bool enable;                        // Whether this subscription is enabled or not
    int instance;                       // Instance number of the subscription in the data model (Device.LocalAgent.Subscription.{i})
    int cont_instance;                  // Instance number of the recipient controller in Device.LocalAgent.Controller.{i}
    char *subscription_id;              // Device.LocalAgent.Subscription.{i}.ID
    bool notification_retry;            // Device.LocalAgent.Subscription.{i}.NotifRetry
    str_vector_t path_expressions;      // Device.LocalAgent.Subscription.{i}.ReferenceList
    subs_notify_t notify_type;          // Device.LocalAgent.Subscription.{i}.NotifType
    time_t expiry_time;                 // Time at which this subscription should be stopped and removed from the DB
    unsigned retry_expiry_period;       // Device.LocalAgent.Subscription.{i}.NotifExpiration
    kv_vector_t last_values;            // List of parameters+values from last time that the subscription was polled (if the subscription is a value change subscription)
    str_vector_t resolved_paths;       // Used to cache the resolved paths of an object deletion subscription before the object has been deleted from the data model
} subs_t;

//------------------------------------------------------------------------------
// Vector of enabled subscriptions
typedef struct
{
    int num_entries;
    subs_t *vector;
} subs_vector_t;

//-----------------------------------------------------------------------------------------
// Subscription Vector API
void SUBS_VECTOR_Init(subs_vector_t *sv);
void SUBS_VECTOR_Add(subs_vector_t *sv, subs_t *sub);
void SUBS_VECTOR_Remove(subs_vector_t *sv, subs_t *sub);
void SUBS_VECTOR_Destroy(subs_vector_t *sv);
void SUBS_VECTOR_DestroySubscriber(subs_t *sub);
subs_t *SUBS_VECTOR_GetSubsByInstance(subs_vector_t *suv, int instance);
void SUBS_VECTOR_MarkSubscriptionForDeletion(subs_t *sub);
void SUBS_VECTOR_GarbageCollectSubscriptions(subs_vector_t *suv);
void SUBS_VECTOR_Dump(subs_vector_t *suv);

#endif
