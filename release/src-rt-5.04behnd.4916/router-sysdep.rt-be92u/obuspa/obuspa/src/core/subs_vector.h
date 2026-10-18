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
// Element of subscription vector
typedef struct
{
    bool enable;                        // Whether this subscription is enabled or not
    int instance;                       // Instance number of the subscription in the data model (Device.LocalAgent.Subscription.{i})
    int cont_instance;                  // Instance number of the recipient controller in Device.LocalAgent.Controller.{i}
    char *subscription_id;              // Device.LocalAgent.Subscription.{i}.ID
    bool notification_retry;            // Device.LocalAgent.Subscription.{i}.NotifRetry
    str_vector_t path_expressions;      // Device.LocalAgent.Subscription.{i}.ReferenceList
                                        // This vector has the same indexes as the handler_group_ids vector
    int_vector_t handler_group_ids;     // If the path_expression is being handled by a vendor layer subscription, this is set to the group_id of the path.
                                        // Set to NON_GROUPED if the path_expression is not being handled by the vendor layer
                                        // This vector has the same indexes as the path_expressions vector
                                        // For internal services, the handle is set to the group_id of the subscription path, if the path is being handled by the vendor layer

    int_vector_t device_group_ids;      // Vector containing the group_ids of all data model provider components that have had a subscription set on them for 'Device.'
    subs_notify_t notify_type;          // Device.LocalAgent.Subscription.{i}.NotifType
    time_t expiry_time;                 // Time at which this subscription should be stopped and removed from the DB
    bool persistent;                    // Whether the subscription should be persisted across reboots
    unsigned retry_expiry_period;       // Device.LocalAgent.Subscription.{i}.NotifExpiration
    kv_vector_t last_values;            // List of parameters+values from last time that the subscription was polled (if the subscription is a value change subscription)
    str_vector_t cur_watch_objs;        // Current set of objects to match against object creation or deletions. The lifetime of this vector is just the current DM_EXEC processing period
    str_vector_t last_watch_objs;       // Stores the last set of objects to match for object deletion (not object creation). This set is matched against, if the deletion occurs before being able to resolve the current set of resolved paths
    bool skip_obj_notifications;        // Set if object creation/deletion notifications should not be sent whilst processing the current USP message
                                        // This flag is used to prevent an obj added/deleted subscription from immediately firing when it is enabled, because the instances
                                        // have changed since the last time the objects refersh instances vendor hook was called
#ifdef BDK_USP
    char *trigger_action;
    kv_vector_t trigger_config_settings;
#endif
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
