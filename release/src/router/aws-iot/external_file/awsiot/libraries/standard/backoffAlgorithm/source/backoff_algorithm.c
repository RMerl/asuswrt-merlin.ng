/*
 * backoffAlgorithm v1.1.0
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
 * @file backoff_algorithm.c
 * @brief Implementation of the backoff algorithm API for a "Full Jitter" exponential backoff
 * with jitter strategy.
 */

/* Standard includes. */
#include <assert.h>
#include <stddef.h>

/* Include API header. */
#include "backoff_algorithm.h"

/*-----------------------------------------------------------*/

BackoffAlgorithmStatus_t BackoffAlgorithm_GetNextBackoff( BackoffAlgorithmContext_t * pRetryContext,
                                                          uint32_t randomValue,
                                                          uint16_t * pNextBackOff )
{
    BackoffAlgorithmStatus_t status = BackoffAlgorithmSuccess;

    assert( pRetryContext != NULL );
    assert( pNextBackOff != NULL );

    /* If maxRetryAttempts state of the context is set to the maximum, retry forever. */
    if( ( pRetryContext->maxRetryAttempts == BACKOFF_ALGORITHM_RETRY_FOREVER ) ||
        ( pRetryContext->attemptsDone < pRetryContext->maxRetryAttempts ) )
    {
        /* The next backoff value is a random value between 0 and the maximum jitter value
         * for the retry attempt. */

        /* Choose a random value for back-off time between 0 and the max jitter value. */
        *pNextBackOff = ( uint16_t ) ( randomValue % ( pRetryContext->nextJitterMax + ( uint32_t ) 1U ) );

        /* Increment the retry attempt. */
        pRetryContext->attemptsDone++;

        /* Double the max jitter value for the next retry attempt, only
         * if the new value will be less than the max backoff time value. */
        if( pRetryContext->nextJitterMax < ( pRetryContext->maxBackoffDelay / 2U ) )
        {
            pRetryContext->nextJitterMax += pRetryContext->nextJitterMax;
        }
        else
        {
            pRetryContext->nextJitterMax = pRetryContext->maxBackoffDelay;
        }
    }
    else
    {
        /* When max retry attempts are exhausted, let application know by
         * returning BackoffAlgorithmRetriesExhausted. Application may choose to
         * restart the retry process after calling BackoffAlgorithm_InitializeParams(). */
        status = BackoffAlgorithmRetriesExhausted;
    }

    return status;
}

/*-----------------------------------------------------------*/

void BackoffAlgorithm_InitializeParams( BackoffAlgorithmContext_t * pContext,
                                        uint16_t backOffBase,
                                        uint16_t maxBackOff,
                                        uint32_t maxAttempts )
{
    assert( pContext != NULL );

    /* Initialize the context with parameters used in calculating the backoff
     * value for the next retry attempt. */
    pContext->nextJitterMax = backOffBase;
    pContext->maxBackoffDelay = maxBackOff;
    pContext->maxRetryAttempts = maxAttempts;

    /* The total number of retry attempts is zero at initialization. */
    pContext->attemptsDone = 0;
}

/*-----------------------------------------------------------*/
