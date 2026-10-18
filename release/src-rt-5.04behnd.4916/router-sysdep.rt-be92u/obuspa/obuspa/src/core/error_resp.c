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
 * \file error_resp.c
 *
 * Functions to dynamically create and populate a USP message containing an error
 *
 */

#include <stdlib.h>
#include <string.h>
#include <protobuf-c/protobuf-c.h>

#include "usp-msg.pb-c.h"
#include "common_defs.h"
#include "proto_trace.h"
#include "msg_handler.h"


/*********************************************************************//**
**
** ERROR_RESP_CreateSingle
**
** Creates a simple ErrorResponse containing a single error message
**
** \param   msg_id - string containing the message id of the USP message, which initiated this response
** \param   err_code - error code
** \param   src_msg - pointer to current USP response message. This will be deleted (if it exists).
**
** \return  Pointer to a USP message object
**
**************************************************************************/
Usp__Msg *ERROR_RESP_CreateSingle(char *msg_id, int err_code, Usp__Msg *src_msg)
{
    char *err_str;
    Usp__Msg *resp;

    // Create the new ErrorResponse
    err_str = USP_ERR_GetMessage();
    resp = ERROR_RESP_Create(msg_id, err_code, err_str);

    // Free the current USP response message (if one exists)
    if (src_msg != NULL)
    {
        usp__msg__free_unpacked(src_msg, pbuf_allocator);
    }

    return resp;
}

/*********************************************************************//**
**
** ERROR_RESP_Create
**
** Dynamically creates an Error object
** NOTE: The object is created without any param_errors
** NOTE: The object should be deleted using usp__msg__free_unpacked()
**
** \param   msg_id - string containing the message id of the request, which initiated this response
** \param   err_code - error code
** \param   err_msg - pointer to string containing reason for the error
**
** \return  Pointer to a USP Error response message object
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Msg *ERROR_RESP_Create(char *msg_id, int err_code, char *err_msg)
{
    Usp__Msg *msg;
    Usp__Error *error;

    msg = MSG_HANDLER_CreateUspMsg(msg_id, USP__HEADER__MSG_TYPE__ERROR, USP__BODY__MSG_BODY_ERROR);
    error = USP_MALLOC(sizeof(Usp__Error));
    usp__error__init(error);
    msg->body->error = error;

    error->err_code = err_code;
    error->err_msg = USP_STRDUP(err_msg);
    error->n_param_errs = 0;
    error->param_errs = NULL;

    return msg;
}

/*********************************************************************//**
**
** ERROR_RESP_AddParamError
**
** Adds a ParamError entry to the Error object
**
** \param   resp - pointer to Error object
** \param   path - path name of the object causing the error
** \param   err_code - numeric code indicating cause of error
** \param   err_msg - string containing additional information about the error
**
** \return  Pointer to dynamically allocated param_error (within the Error object)
**          NOTE: If out of memory, USP Agent is terminated
**
**************************************************************************/
Usp__Error__ParamError *
ERROR_RESP_AddParamError(Usp__Msg *resp, char *path, int err_code, char *err_msg)
{
    Usp__Error *error;
    Usp__Error__ParamError *param_err;
    int new_num;    // new number of param_error

    // Allocate memory to store the param_error
    param_err = USP_MALLOC(sizeof(Usp__Error__ParamError));
    usp__error__param_error__init(param_err);

    // Increase the size of the vector containing pointers to the param_errors
    error = resp->body->error;
    new_num = error->n_param_errs + 1;
    error->param_errs = USP_REALLOC(error->param_errs, new_num*sizeof(void *));
    error->n_param_errs = new_num;
    error->param_errs[new_num-1] = param_err;

    // Initialise the param_error
    param_err->param_path = USP_STRDUP(path);
    param_err->err_code = err_code;
    param_err->err_msg = USP_STRDUP(err_msg);

    return param_err;
}

/*********************************************************************//**
**
** ERROR_RESP_CalcOuterErrCode
**
** Used to calculate the outer err_code in ERROR messages responding to Set, Add and Delete messages
** These messages should return an outer error code of USP_ERR_INVALID_ARGUMENTS if the inner error code
** was caused by a bad input argument and there are param_err elements.
**
** \param   err - USP error code to use in the inner error code of an ERROR message
**
** \return  USP error code to use in the outer error code of an ERROR message
**
**************************************************************************/
int ERROR_RESP_CalcOuterErrCode(int err)
{
    switch(err)
    {
        case USP_ERR_INVALID_ARGUMENTS:
        case USP_ERR_INVALID_PATH_SYNTAX:
        case USP_ERR_UNSUPPORTED_PARAM:
        case USP_ERR_INVALID_TYPE:
        case USP_ERR_INVALID_VALUE:
        case USP_ERR_PARAM_READ_ONLY:
        case USP_ERR_VALUE_CONFLICT:
        case USP_ERR_OBJECT_DOES_NOT_EXIST:
        case USP_ERR_CREATION_FAILURE:
        case USP_ERR_NOT_A_TABLE:
        case USP_ERR_OBJECT_NOT_CREATABLE:
        case USP_ERR_SET_FAILURE:
        case USP_ERR_REQUIRED_PARAM_FAILED:
        case USP_ERR_OBJECT_NOT_DELETABLE:
        case USP_ERR_UNIQUE_KEY_CONFLICT:
        case USP_ERR_INVALID_PATH:
            // If error was caused by invalid arguments, then outer error code is invalid arguments
            err = USP_ERR_INVALID_ARGUMENTS;
            break;

        default:
            // don't change err
            break;
    }

    return err;
}

