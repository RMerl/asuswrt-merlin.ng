/*
 *
 * Copyright (C) 2019-2022, Broadband Forum
 * Copyright (C) 2016-2022  CommScope, Inc
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
 * \file device_selftest_example.c
 *
 * Implements the Device.LocalAgent data model object
 *
 */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "common_defs.h"
#include "usp_api.h"
#include "dm_access.h"
#include "os_utils.h"

#ifndef REMOVE_SELF_TEST_DIAG_EXAMPLE
//------------------------------------------------------------------------------------
// Input conditions for SelfTestDiagnostics
typedef struct
{
    int request_instance;   // Instance number of this operation in the Device.LocalAgent.Request table
    unsigned test_number;
} selftest_input_cond_t;

//------------------------------------------------------------------------------------
// Output results of SelfTestDiagnostics
typedef struct
{
    char result_str[1025];
    char err_msg[256];
} selftest_output_res_t;

//------------------------------------------------------------------------------------
// Array of valid input arguments
static char *selftest_input_args[] =
{
    "X_ARRIS-COM_TestNumber",
};

//------------------------------------------------------------------------------------
// Array of valid output arguments
static char *selftest_output_args[] =
{
    "Results",
};

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
int DEVICE_SELF_TEST_Operate(dm_req_t *req, kv_vector_t *input_args, int instance);
void *SelfTestDiagThreadMain(void *param);
int ExecuteSelfTestDiagnostic(selftest_input_cond_t *cond, selftest_output_res_t *res);

/*********************************************************************//**
**
** DEVICE_SELF_TEST_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**          USP_ERR_INTERNAL_ERROR if any other error occurred
**
**************************************************************************/
int DEVICE_SELF_TEST_Init(void)
{
    int err = USP_ERR_OK;

    // Register self test diagnostics
    err |= USP_REGISTER_AsyncOperation("Device.SelfTestDiagnostics()", DEVICE_SELF_TEST_Operate, NULL);
    err |= USP_REGISTER_AsyncOperation_MaxConcurrency("Device.SelfTestDiagnostics()", 1);
    err |= USP_REGISTER_OperationArguments("Device.SelfTestDiagnostics()", selftest_input_args, NUM_ELEM(selftest_input_args),
                                                                           selftest_output_args, NUM_ELEM(selftest_output_args));

    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SELF_TEST_Operate
**
** Starts the asynchronous SelfTestDiagnostics operation
** Checks that all mandatory parameters are present and valid, defaults non-mandatory parameters,
** then starts a thread to perform the operation
**
** \param   req - pointer to structure identifying the operation in the data model
** \param   input_args - vector containing input arguments and their values
** \param   instance - instance number of this operation in the Device.LocalAgent.Request table
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SELF_TEST_Operate(dm_req_t *req, kv_vector_t *input_args, int instance)
{
    int err;
    selftest_input_cond_t *cond;

    // Allocate input conditions to pass to thread
    cond = USP_MALLOC(sizeof(selftest_input_cond_t));
    memset(cond, 0, sizeof(selftest_input_cond_t));
    cond->request_instance = instance;

    // Extract the input arguments using KV_VECTOR_ functions
    #define DEFAULT_SELF_TEST 1
    #define MAX_SELF_TEST     10
    err = USP_ARG_GetUnsignedWithinRange(input_args, "X_ARRIS-COM_TestNumber", DEFAULT_SELF_TEST, 1, MAX_SELF_TEST, &cond->test_number);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Log the input conditions for the operation
    USP_LOG_Info("=== SelfTestDiagnostics Conditions ===");
    USP_LOG_Info("test_number: %d", cond->test_number);

    // Exit if unable to start a thread to perform this operation
    // NOTE: ownership of input conditions passes to the thread
    err = OS_UTILS_CreateThread("SelfTestDiag", SelfTestDiagThreadMain, cond);
    if (err != USP_ERR_OK)
    {
        err = USP_ERR_COMMAND_FAILURE;
        goto exit;
    }

exit:
    // Exit if an error occurred (freeing the input conditions)
    if (err != USP_ERR_OK)
    {
        USP_FREE(cond);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Ownership of the input conditions has passed to the thread
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** SelfTestDiagThreadMain
**
** Main function for Self Test Diagnostics operation thread
**
** \param   param - pointer to input conditions
**
** \return  NULL
**
**************************************************************************/
void *SelfTestDiagThreadMain(void *param)
{
    selftest_input_cond_t *cond = (selftest_input_cond_t *) param;
    selftest_output_res_t results;
    selftest_output_res_t *res = &results;
    kv_vector_t *output_args;
    char *err_msg;
    int err = USP_ERR_OK;

    memset(&results, 0, sizeof(results));

    // Exit if unable to signal that this operation is active
    err = USP_SIGNAL_OperationStatus(cond->request_instance, "Active");
    if (err != USP_ERR_OK)
    {
        USP_SNPRINTF(res->err_msg, sizeof(res->err_msg), "%s: USP_SIGNAL_OperationStatus() failed", __FUNCTION__);
        goto exit;
    }

    // Perform the self test diagnostic
    err = ExecuteSelfTestDiagnostic(cond, res);

exit:
    // Log output results
    USP_LOG_Info("=== Self Test Diagnostics completed with result=%d ===", err);
    USP_LOG_Info("=== Self Test Diagnostics Results ===");
    USP_LOG_Info("Results: %s", res->result_str);

    // Save all results into the output arguments using KV_VECTOR_ functions
    output_args = USP_ARG_Create();
    USP_ARG_Add(output_args, "Results", res->result_str);

    // Inform the protocol handler, that the operation has completed
    // Ownership of the output args passes to protocol handler
    err_msg = (err != USP_ERR_OK) ? res->err_msg : NULL;
    USP_SIGNAL_OperationComplete(cond->request_instance, err, err_msg, output_args);

    // Free the input conditions
    USP_FREE(cond);

    return NULL;
}

/*********************************************************************//**
**
** ExecuteSelfTestDiagnostic
**
** Performs the self test diagnostic for the specified test_number
** Test 1 passes, Test 2 fails, and all other tests are not implemented
** NOTE: This function would need writing correctly for a real implementation
**
** \param   cond - pointer to input conditions
** \param   res - pointer to output results
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ExecuteSelfTestDiagnostic(selftest_input_cond_t *cond, selftest_output_res_t *res)
{
    int err;

    switch(cond->test_number)
    {
        case 1:
            USP_SNPRINTF(res->result_str, sizeof(res->result_str), "PASS");
            err = USP_ERR_OK;
            break;

        case 2:
            USP_SNPRINTF(res->result_str, sizeof(res->result_str), "FAIL");
            err = USP_ERR_OK;
            break;

        default:
            USP_SNPRINTF(res->err_msg, sizeof(res->err_msg), "%s: Self Test Diagnostics (test_number=%d) is not implemented", __FUNCTION__, cond->test_number);
            err = USP_ERR_COMMAND_FAILURE;
            break;
    }

    return err;
}

#endif // REMOVE_SELF_TEST_DIAG_EXAMPLE
