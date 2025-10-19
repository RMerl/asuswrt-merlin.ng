/*
 *
 * Copyright (C) 2019-2020, Broadband Forum
 * Copyright (C) 2016-2020  CommScope, Inc
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
 * \file vendor_factory_reset_example.c
 *
 * Example of parameters for a programmatic factory reset
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "usp_err_codes.h"
#include "vendor_defs.h"
#include "vendor_api.h"
#include "usp_api.h"

#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
//--------------------------------------------------------------------------
static const kv_pair_t factory_reset_parameters[] =
{
    //Note: If an object is migrated to BDK mdm, the object has to be removed here
    //After all objects are migrated, this file won't be compiled.
    { "Internal.Reboot.Cause", "LocalFactoryReset" },
};

/*********************************************************************//**
**
** VENDOR_GetFactoryResetParams
**
** Called to get the list of parameters to put into a factory reset database
** NOTE: This function is called before VENDOR_Init()
** NOTE: This function is only called, if a USP database does not exist and cannot be created via any of the other methods
** NOTE: The factory reset database can alternatively be specified using the FACTORY_RESET_FILE define
**
** \param   kvv - pointer to key value vector structure in which to return the factory reset parameter settings
**                NOTE: ownership of the key-value vector passes to the caller. The caller will free this vector when finished with it.
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int VENDOR_GetFactoryResetParams(kv_vector_t *kvv)
{
    int i;
    kv_pair_t *kv;

    // Add all fixed parameter values to the returned vector
    #define NUM_ELEM(x) (sizeof((x)) / sizeof((x)[0]))
    USP_ARG_Init(kvv);
    for (i=0; i<NUM_ELEM(factory_reset_parameters); i++)
    {
        kv = (kv_pair_t *) &factory_reset_parameters[i];
        USP_ARG_Add(kvv, kv->key, kv->value);
    }

    return USP_ERR_OK;
}

#endif // INCLUDE_PROGRAMMATIC_FACTORY_RESET

