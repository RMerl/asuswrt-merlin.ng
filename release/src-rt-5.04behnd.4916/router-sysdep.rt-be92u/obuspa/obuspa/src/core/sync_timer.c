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
 * \file sync_timer.c
 *
 * Implements a basic repeating timer mechanism
 * Each timer has a period and a callback. The callback is called
 *
 */
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>

#include "common_defs.h"
#include "sync_timer.h"
#include "usp_api.h"

//--------------------------------------------------------------------------------------
// Structure describing a timer
typedef struct
{
    bool       enabled;         // Cleared after a timeout has fired, to prevent it firing again until an updated time has been registered
    time_t     next_timeout;    // time at which this timer should next fire
    timer_cb_t timer_cb;        // function to call when timer period has expired.
    int        id;              // unique identifier for this callback (allocated by caller of this library) within the namespace of the callback
} sync_timer_t;

//--------------------------------------------------------------------------------------
// Structure containing a dynamic array of timers
typedef struct
{
    int num_entries;
    sync_timer_t *vector;
} timer_vector_t;

static timer_vector_t sync_timers;

//--------------------------------------------------------------------------------------
// Variable that is always updated to reflect the time at which the next timer should fire
static time_t first_sync_timer_time;



//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int FindSyncTimer(timer_cb_t timer_cb, int id);
void UpdateFirstSyncTimerTime(void);

/*********************************************************************//**
**
** SYNC_TIMER_Init
**
** Initialises the synchronous timers
**
** \param   iv - pointer to structure to initialise
**
** \return  None
**
**************************************************************************/
void SYNC_TIMER_Init(void)
{
    sync_timers.vector = NULL;
    sync_timers.num_entries = 0;
    first_sync_timer_time = END_OF_TIME;
}

/*********************************************************************//**
**
** SYNC_TIMER_Destroy
**
** Frees all memory associated with sync timers
**
** \param   None
**
** \return  None
**
**************************************************************************/
void SYNC_TIMER_Destroy(void)
{
    USP_SAFE_FREE(sync_timers.vector);

}

/*********************************************************************//**
**
** SYNC_TIMER_Add
**
** Adds a new timer which will callback the specified function at the specified time
**
** \param   timer_cb - callback function to call when timer expires - This also identifies a namespace for the id
** \param   id - unique identifier for this sync timer, within the namespace of the callback
** \param   callback_time - absolute time at which the callback should fire
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SYNC_TIMER_Add(timer_cb_t timer_cb, int id, time_t callback_time)
{
    int new_num_entries;
    sync_timer_t *st;
    int index;

    // Exit if the callback is not defined
    if (timer_cb == NULL)
    {
        USP_ERR_SetMessage("%s: Timer callback should not be NULL", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if a timer for this callback has already been registered
    index = FindSyncTimer(timer_cb, id);
    if (index != INVALID)
    {
        USP_ERR_SetMessage("%s: Timer for callback=%p, id=%d has already been registered", __FUNCTION__, timer_cb, id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add this timer to the vector
    new_num_entries = sync_timers.num_entries + 1;
    sync_timers.vector = USP_REALLOC(sync_timers.vector, new_num_entries*sizeof(sync_timer_t));
    st = &sync_timers.vector[ sync_timers.num_entries ];
    st->timer_cb = timer_cb;
    st->id = id;
    st->next_timeout = callback_time;
    st->enabled = true;

    sync_timers.num_entries = new_num_entries;

    // Update the time at which the next timer should fire
    UpdateFirstSyncTimerTime();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SYNC_TIMER_Reload
**
** Restarts the specified timer with a new time to fire
**
** \param   timer_cb - callback function to call when timer expires - This also identifies a namespace for the id
** \param   id - unique identifier for this sync timer, within the namespace of the callback
** \param   callback_time - absolute time at which the callback should fire
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SYNC_TIMER_Reload(timer_cb_t timer_cb, int id, time_t callback_time)
{
    sync_timer_t *st;
    int index;

    // Exit if timer could not be found
    index = FindSyncTimer(timer_cb, id);
    if (index == INVALID)
    {
        USP_ERR_SetMessage("%s: Unable to find timer registered with callback=%p, id=%d", __FUNCTION__, timer_cb, id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Reload the timer
    st = &sync_timers.vector[index];
    st->enabled = true;
    st->next_timeout = callback_time;

    // Update the time at which the next timer should fire
    UpdateFirstSyncTimerTime();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SYNC_TIMER_Remove
**
** Removes the specified timer
**
** \param   timer_cb - callback function to call when timer expires - This also identifies a namespace for the id
** \param   id - unique identifier for this sync timer, within the namespace of the callback
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int SYNC_TIMER_Remove(timer_cb_t timer_cb, int id)
{
    int index;
    int num_to_move;

    // Exit if timer could not be found
    index = FindSyncTimer(timer_cb, id);
    if (index == INVALID)
    {
        USP_ERR_SetMessage("%s: Unable to find timer registered with callback=%p, id=%d", __FUNCTION__, timer_cb, id);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Remove this timer from the vector, by moving down the rest of the timers in the array
    num_to_move = sync_timers.num_entries - 1 - index;
    if (num_to_move > 0)
    {
        memmove(&sync_timers.vector[index], &sync_timers.vector[index+1], num_to_move*sizeof(sync_timer_t));
    }
    sync_timers.num_entries--;

    // Update the time at which the next timer should fire
    UpdateFirstSyncTimerTime();

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SYNC_TIMER_TimeToNext
**
** Returns the time (in ms) until the next timer fires
**
** \param   None
**
** \return  time in seconds until next timer should fire
**
**************************************************************************/
int SYNC_TIMER_TimeToNext(void)
{
    time_t cur_time;
    int delta;

    // Calculate the time delta from now to the time at which the first timer should fire
    cur_time = time(NULL);
    delta = first_sync_timer_time - cur_time;

    // If the first sync time should already have fired, then just return a zero second delay
    if (delta < 0)
    {
        delta = 0;
    }

    // Exit with largest delay possible, if actual delay wanted is larger than that
    if (delta > (INT_MAX/1000))
    {
        return INT_MAX;
    }

    // Return time in ms
    return delta * 1000;
}

/*********************************************************************//**
**
** SYNC_TIMER_Execute
**
** Processes all timers which have reached the time for them to fire
**
** \param   None
**
** \return  None
**
**************************************************************************/
void SYNC_TIMER_Execute(void)
{
    int i;
    time_t cur_time;
    sync_timer_t *st;
    timer_cb_t timer_cb;

    // Exit if it is not yet time for any of the timers to fire
    cur_time = time(NULL);
    if (cur_time < first_sync_timer_time)
    {
        return;
    }

    // Iterate over all timers
    for (i=0; i < sync_timers.num_entries; i++)
    {
        // Determine if this timer should fire
        st = &sync_timers.vector[i];
        if ((st->enabled) && (cur_time >= st->next_timeout))
        {
            // Mark the timer as fired, if the callback wants the timer to continue, then it can call SYNC_TIMER_Reload()
            st->enabled = false;

            // Call the registered callback
            timer_cb = st->timer_cb;
            USP_ASSERT(timer_cb != NULL)
            timer_cb(st->id);
        }
    }

    // Update the time at which the next timer should fire
    UpdateFirstSyncTimerTime();
}

/*********************************************************************//**
**
** SYNC_TIMER_PRIV_GetVector
**
** Gets information about the dynamically allocated sync timers vector
** This information is necessary to make meminfo collection work correctly
** as this vector is reallocated both before and after meminfo collection
** so we have to 'seed' meminfo with it, in order that meminfo does not generate
** errors such as 'realloc without previous alloc'.
**
** \param   allocated_size - pointer to variable in which to return the size of the sync timers vector
**
** \return  pointer to memory allocated in the sync_timers vector
**
**************************************************************************/
void *SYNC_TIMER_PRIV_GetVector(int *allocated_size)
{
    *allocated_size = sync_timers.num_entries * sizeof(timer_vector_t);
    return sync_timers.vector;
}

/*********************************************************************//**
**
** FindSyncTimer
**
** Finds the timer identified by matching callback
**
** \param   timer_cb - callback function identifying the timer to reload
**
** \return  index of matching timer, or INVALID if no match was found
**
**************************************************************************/
int FindSyncTimer(timer_cb_t timer_cb, int id)
{
    int i;
    sync_timer_t *st;

    // Iterate over all timers
    for (i=0; i < sync_timers.num_entries; i++)
    {
        st = &sync_timers.vector[i];
        if ((st->timer_cb == timer_cb) && (st->id == id))
        {
            return i;
        }
    }

    // If the code gets here, then no match was found
    return INVALID;
}

/*********************************************************************//**
**
** UpdateFirstSyncTimerTime
**
** Updates the time at which the first timer should fire
**
** \param   None
**
** \return  None
**
**************************************************************************/
void UpdateFirstSyncTimerTime(void)
{
    int i;
    sync_timer_t *st;
    time_t first;

    // Iterate over all timers
    first = END_OF_TIME;
    for (i=0; i < sync_timers.num_entries; i++)
    {
        // Skip this timer if it is not enabled
        st = &sync_timers.vector[i];
        if (st->enabled == false)
        {
            continue;
        }

        // Update the time at which the first timer fires
        if (st->next_timeout < first)
        {
            first = st->next_timeout;
        }
    }

    first_sync_timer_time = first;
}
