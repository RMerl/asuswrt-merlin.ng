/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2018-2024  CommScope, Inc
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
 * \file retry_wait.c
 *
 * Calculates a time to wait between retries according to the standard TR069 algorithm, detailed in:-
 *     TR-069 Section 3.2.1.1 Session Retry
 *     TR-157 Section A.2.2.1
 *
 * This algorithm is used for :-
 *     bulk data collection post retries
 *     subscription notification retries
 *
 * This file also contains various functions related to random number generation and usage
 *
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>

#include "common_defs.h"
#include "usp_log.h"
#include "nu_macaddr.h"
#include "retry_wait.h"
#include "iso8601.h"

//-----------------------------------------------------------------------
// Random number generator seeds used by each thread
unsigned dm_thread_random_seed = 0;
unsigned mtp_thread_random_seed = 0;


//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
unsigned GenerateSeed(void);

/*********************************************************************//**
**
**  RETRY_WAIT_Init
**
**  Initialise the seeds for each thread's random number generator
**
** \param   None
**
** \return  None
**
**************************************************************************/
void RETRY_WAIT_Init(void)
{
    dm_thread_random_seed = GenerateSeed();
    mtp_thread_random_seed = GenerateSeed() + 1;    // Plus 1 because there is a possibility that it could end up with the same seed as the data model thread
}

/*********************************************************************//**
**
**  RETRY_WAIT_Calculate
**
**  Determines the number of seconds until the specified retry should occur
**  (See TR-157 Section A.2.2.1 for details of how this is calculated)
**
** \param   retry_count - Number specifying the retry attempt that we want to calculate the delta time to. Counts from 1.
** \param   m - The minimum wait interval
** \param   k - The interval multiplier
**
** \return  Number of seconds until the next retry
**
**************************************************************************/
unsigned RETRY_WAIT_Calculate(unsigned retry_count, double m, double k)
{
    unsigned min_seconds;
    unsigned max_seconds;
    unsigned range;
    unsigned wait_time;
    int random_value;

    // Limit maximum retry count
    if (retry_count > 10)
    {
        retry_count = 10;
    }

    // This function should not be called with a retry_count of 0
    // However, if it is, just treat it the same as a retry count of 1
    if (retry_count <= 0)
    {
        retry_count = 1;
    }

    min_seconds = (unsigned) (m * pow(k/1000, retry_count-1));
    max_seconds = (unsigned) (m * pow(k/1000, retry_count));

    // NOTE: Following statement is not strictly necessary, since k must be >= 1000. It is there to prevent cases where K<1000 from calculating large delays
    if (max_seconds < min_seconds)
    {
        max_seconds = min_seconds;
    }

    range = max_seconds - min_seconds;
    random_value = rand_r(&dm_thread_random_seed);
    if (range > 0)
    {
        wait_time = min_seconds + random_value % range;
    }
    else
    {
        // This case is needed to prevent a divide by zero error, which could occur in above modulus operation
        // if k==1000 (the lowest value TR-181 allows it to be)
        wait_time = min_seconds;
    }

    return wait_time;
}

/*********************************************************************//**
**
**  RETRY_WAIT_UseRandomBaseIfUnknownTime
**
**  Returns a periodic base to use
**  Normally this is the same as the input base, however if the input base is UNKNOWN_TIME, then we choose a random base
**
** \param   base - periodic base time to use or replace with a random base
**
** \return  UNIX time for periodic base
**
**************************************************************************/
time_t RETRY_WAIT_UseRandomBaseIfUnknownTime(time_t base)
{
    // Exit if base has been specified (ie not the unknown time)
    if (base != UNKNOWN_TIME)
    {
        return base;
    }

    // If the time is UNKNOWN_TIME, then the client may select any time
    // We choose a random time to ensure that the server is not hit at the same time by all devices in a population
    return rand_r(&dm_thread_random_seed) % 86400;
}

/*********************************************************************//**
**
**  GenerateSeed
**
**  Generates a seed for the random number generator with either a number from /dev/urandom (if available)
**  or the MAC address + time otherwise
**
** \param   None
**
** \return  a seed for the random number generator
**
**************************************************************************/
unsigned GenerateSeed(void)
{
    unsigned char buf[MAC_ADDR_LEN];
    unsigned int seed;
    FILE *fp;
    int num_elem;
    int err;
    struct timeval cur_time;

    // Fallback if unable to open /dev/urandom
    fp = fopen("/dev/urandom", "r");
    if (fp == NULL)
    {
        goto fallback;
    }

    // Fallback if unable to read /dev/urandom
    num_elem = fread(&buf[2], sizeof(unsigned int), 1, fp);
    fclose(fp);
    if (num_elem != 1)
    {
        USP_LOG_Warning("%s: WARNING: Unable to read /dev/urandom", __FUNCTION__);
        goto fallback;
    }

    // Since we have successfully read a seed from /dev/urandom, exit
    goto exit;

fallback:
    // The code gets here if it failed to read from /dev/urandom
    // Attempt to get MAC address (as something which is unique per STB)
    err = nu_macaddr_wan_macaddr(buf);
    if (err != USP_ERR_OK)
    {
        // If unable to get the MAC address of the WAN interface, then just use a fixed string.
        // NOTE: In this case, the seed will not be random in a population after a power outage
        memset(buf, '0', sizeof(buf));
    }

    // Get the current time and combine it's low order byte with the (non-OUI portion of) MAC address to form a seed
    gettimeofday(&cur_time, NULL);
    buf[2] = cur_time.tv_usec & 0xFF;

exit:
    memcpy(&seed, &buf[2], sizeof(seed));
    return seed;
}

