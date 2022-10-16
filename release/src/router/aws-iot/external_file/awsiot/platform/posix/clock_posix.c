/*
 * AWS IoT Device SDK for Embedded C 202103.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file clock_posix.c
 * @brief Implementation of the functions in clock.h for POSIX systems.
 */

/* POSIX include. Allow the default POSIX header to be overridden. */
#ifdef POSIX_TIME_HEADER
    #include POSIX_TIME_HEADER
#else
    #include <time.h>
#endif

/* Platform clock include. */
#include "clock.h"

/*
 * Time conversion constants.
 */
#define NANOSECONDS_PER_MILLISECOND    ( 1000000L )    /**< @brief Nanoseconds per millisecond. */
#define MILLISECONDS_PER_SECOND        ( 1000L )       /**< @brief Milliseconds per second. */

/*-----------------------------------------------------------*/

uint32_t Clock_GetTimeMs( void )
{
    int64_t timeMs;
    struct timespec timeSpec;

    /* Get the MONOTONIC time. */
    ( void ) clock_gettime( CLOCK_MONOTONIC, &timeSpec );

    /* Calculate the milliseconds from timespec. */
    timeMs = ( timeSpec.tv_sec * MILLISECONDS_PER_SECOND )
             + ( timeSpec.tv_nsec / NANOSECONDS_PER_MILLISECOND );

    /* Libraries need only the lower 32 bits of the time in milliseconds, since
     * this function is used only for calculating the time difference.
     * Also, the possible overflows of this time value are handled by the
     * libraries. */
    return ( uint32_t ) timeMs;
}

/*-----------------------------------------------------------*/

void Clock_SleepMs( uint32_t sleepTimeMs )
{
    /* Convert parameter to timespec. */
    struct timespec sleepTime = { 0 };

    sleepTime.tv_sec = ( ( time_t ) sleepTimeMs / ( time_t ) MILLISECONDS_PER_SECOND );
    sleepTime.tv_nsec = ( ( int64_t ) sleepTimeMs % MILLISECONDS_PER_SECOND ) * NANOSECONDS_PER_MILLISECOND;

    /* High resolution sleep. */
    ( void ) nanosleep( &sleepTime, NULL );
}
