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
 * @file backoff_algorithm.h
 * @brief API for calculating backoff period for retry attempts using
 * exponential backoff with jitter algorithm.
 * This library represents the "Full Jitter" backoff strategy explained in the
 * following document.
 * https://aws.amazon.com/blogs/architecture/exponential-backoff-and-jitter/
 *
 */

#ifndef BACKOFF_ALGORITHM_H_
#define BACKOFF_ALGORITHM_H_

/* Standard include. */
#include <stdint.h>

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @ingroup backoff_algorithm_constants
 * @brief Constant to represent unlimited number of retry attempts.
 */
#define BACKOFF_ALGORITHM_RETRY_FOREVER    ( UINT32_MAX )

/**
 * @ingroup backoff_algorithm_enum_types
 * @brief Status for @ref BackoffAlgorithm_GetNextBackoff.
 */
typedef enum BackoffAlgorithmStatus
{
    BackoffAlgorithmSuccess = 0,     /**< @brief The function successfully calculated the next back-off value. */
    BackoffAlgorithmRetriesExhausted /**< @brief The function exhausted all retry attempts. */
} BackoffAlgorithmStatus_t;

/**
 * @ingroup backoff_algorithm_struct_types
 * @brief Represents parameters required for calculating the back-off delay for the
 * next retry attempt.
 */
typedef struct BackoffAlgorithmContext
{
    /**
     * @brief The maximum backoff delay (in milliseconds) between consecutive retry attempts.
     */
    uint16_t maxBackoffDelay;

    /**
     * @brief The total number of retry attempts completed.
     * This value is incremented on every call to #BackoffAlgorithm_GetNextBackoff API.
     */
    uint32_t attemptsDone;

    /**
     * @brief The maximum backoff value (in milliseconds) for the next retry attempt.
     */
    uint16_t nextJitterMax;

    /**
     * @brief The maximum number of retry attempts.
     */
    uint32_t maxRetryAttempts;
} BackoffAlgorithmContext_t;

/**
 * @brief Initializes the context for using backoff algorithm. The parameters
 * are required for calculating the next retry backoff delay.
 * This function must be called by the application before the first new retry attempt.
 *
 * @param[out] pContext The context to initialize with parameters required
 * for the next backoff delay calculation function.
 * @param[in] maxBackOff The maximum backoff delay (in milliseconds) between
 * consecutive retry attempts.
 * @param[in] backOffBase The base value (in milliseconds) of backoff delay to
 * use in the exponential backoff and jitter model.
 * @param[in] maxAttempts The maximum number of retry attempts. Set the value to
 * #BACKOFF_ALGORITHM_RETRY_FOREVER to retry for ever.
 */
/* @[define_backoffalgorithm_initializeparams] */
void BackoffAlgorithm_InitializeParams( BackoffAlgorithmContext_t * pContext,
                                        uint16_t backOffBase,
                                        uint16_t maxBackOff,
                                        uint32_t maxAttempts );
/* @[define_backoffalgorithm_initializeparams] */

/**
 * @brief Simple exponential backoff and jitter function that provides the
 * delay value for the next retry attempt.
 * After a failure of an operation that needs to be retried, the application
 * should use this function to obtain the backoff delay value for the next retry,
 * and then wait for the backoff time period before retrying the operation.
 *
 * @param[in, out] pRetryContext Structure containing parameters for the next backoff
 * value calculation.
 * @param[in] randomValue The random value to use for calculation of the backoff period.
 * The random value should be in the range of [0, UINT32_MAX].
 * @param[out] pNextBackOff This will be populated with the backoff value (in milliseconds)
 * for the next retry attempt. The value does not exceed the maximum backoff delay
 * configured in the context.
 *
 * @note For generating a random number, it is recommended to use a Random Number Generator
 * that is seeded with a device-specific entropy source so that possibility of collisions
 * between multiple devices retrying the network operations can be mitigated.
 *
 * @return #BackoffAlgorithmSuccess after a successful sleep;
 * #BackoffAlgorithmRetriesExhausted when all attempts are exhausted.
 */
/* @[define_backoffalgorithm_getnextbackoff] */
BackoffAlgorithmStatus_t BackoffAlgorithm_GetNextBackoff( BackoffAlgorithmContext_t * pRetryContext,
                                                          uint32_t randomValue,
                                                          uint16_t * pNextBackOff );
/* @[define_backoffalgorithm_getnextbackoff] */

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* ifndef BACKOFF_ALGORITHM_H_ */
