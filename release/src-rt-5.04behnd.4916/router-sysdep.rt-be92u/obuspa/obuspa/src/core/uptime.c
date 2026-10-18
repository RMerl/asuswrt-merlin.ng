/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2008-2024  CommScope, Inc
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
 * @file uptime.c
 *
 * Container for non POSIX compliant time functions
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "uptime.h"

/*********************************************************************//**
**
** tu_uptime_msecs
**
** Returns the number of milli-seconds since the kernel was rebooted
**
** \param   None
**
** \return  Number of milli-seconds
**
**************************************************************************/
uint32_t
tu_uptime_msecs(void)
{
	time_t t;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	t = ((time_t)(ts.tv_sec * 1000) + (time_t)(ts.tv_nsec / 1000000));
	return (uint32_t)t;
}

/*********************************************************************//**
**
** tu_uptime_secs
**
** Returns the number of seconds since the kernel was rebooted
**
** \param   None
**
** \return  Number of seconds
**
**************************************************************************/
uint32_t
tu_uptime_secs(void)
{
	time_t t;
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	t = (time_t)ts.tv_sec;
	return (uint32_t)t;
}

