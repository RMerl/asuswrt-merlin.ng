/*
 *
 * Copyright (C) 2023-2024, Broadband Forum
 * Copyright (C) 2023-2024  CommScope, Inc
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
 * \file plugin.c
 *
 * Module enabling vendor layer plugins
 *
 */
#include <dlfcn.h>

#include "common_defs.h"
#include "dllist.h"
#include "plugin.h"

//--------------------------------------------------------------------
// Structure representing a dynamically loaded plugin
typedef int (* API_FUNC_PTR0) (void);
typedef int (* API_FUNC_PTR1) (kv_vector_t *);

typedef struct
{
    double_link_t link;     // Doubly linked list pointers. These must always be first in this structure
    void *dl_handle;

    API_FUNC_PTR0 vendor_init;
    API_FUNC_PTR0 vendor_start;
    API_FUNC_PTR0 vendor_stop;
#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
    API_FUNC_PTR1 vendor_get_factory_reset_params;
#endif
} plugin_t;

//--------------------------------------------------------------------
// Linked list of loaded plugins
static double_linked_list_t plugins_list = {NULL, NULL};

/*********************************************************************//**
**
** PLUGIN_Load
**
** Load a plugin located at the given filepath
** Check for undefined symbols in the plugin, also check that the expected
** vendor API functions are present and store function pointers.
**
** \param   path - path to the plugin file
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PLUGIN_Load(const char *path)
{
    int err = USP_ERR_INTERNAL_ERROR;
    plugin_t *plugin;

    // Allocate new plug-in
    plugin = USP_MALLOC(sizeof(plugin_t));
    memset(plugin, 0, sizeof(plugin_t));

    // Open with RTLD_NOW to check for undefined symbols in the plugin
    // (with RTLD_LAZY the plugin would load, then later abort if we hit
    // the undefined symbol)
    plugin->dl_handle = dlopen(path, RTLD_NOW);
    if (plugin->dl_handle == NULL)
    {
        USP_LOG_Error("%s: Failed to load plugin %s\n", __FUNCTION__, dlerror());
        goto exit;
    }

    plugin->vendor_init = (API_FUNC_PTR0) dlsym(plugin->dl_handle, "VENDOR_Init");
    if (plugin->vendor_init == NULL)
    {
        USP_LOG_Error("%s: Can't find VENDOR_Init in %s: %s\n", __FUNCTION__, path, dlerror());
        goto exit;
    }

    plugin->vendor_start = (API_FUNC_PTR0) dlsym(plugin->dl_handle, "VENDOR_Start");
    if (plugin->vendor_start == NULL)
    {
        USP_LOG_Error("%s: Can't find VENDOR_Start in %s: %s\n", __FUNCTION__, path, dlerror());
        goto exit;
    }

    plugin->vendor_stop = (API_FUNC_PTR0) dlsym(plugin->dl_handle, "VENDOR_Stop");
    if (plugin->vendor_stop == NULL)
    {
        USP_LOG_Error("%s: Can't find VENDOR_Stop in %s: %s\n", __FUNCTION__, path, dlerror());
        goto exit;
    }

#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
    // This function is optional, so no error if it is not present
    plugin->vendor_get_factory_reset_params = (API_FUNC_PTR1) dlsym(plugin->dl_handle, "VENDOR_GetFactoryResetParams");
#endif

    err = USP_ERR_OK;

exit:
    if (err == USP_ERR_OK)
    {
        DLLIST_LinkToTail(&plugins_list, plugin);
    }
    else
    {
        USP_FREE(plugin);
    }

    return err;
}

/*********************************************************************//**
**
** PLUGIN_Init
**
** Calls VENDOR_Init in all registered plugins
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PLUGIN_Init(void)
{
    int err;
    plugin_t *plugin;

    plugin = (plugin_t *) plugins_list.head;
    while (plugin != NULL)
    {
        err = plugin->vendor_init();
        if (err != USP_ERR_OK)
        {
            return err;
        }

        plugin = (plugin_t *) plugin->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** PLUGIN_Start
**
** Calls VENDOR_Init in all registered plugins
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int PLUGIN_Start(void)
{
    int err;
    plugin_t *plugin;

    plugin = (plugin_t *) plugins_list.head;
    while (plugin != NULL)
    {
        err = plugin->vendor_start();
        if (err != USP_ERR_OK)
        {
            return err;
        }

        plugin = (plugin_t *) plugin->link.next;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** PLUGIN_Stop
**
** Calls VENDOR_Stop in all registered plugins
**
** \param   None
**
** \return  None
**
**************************************************************************/
void PLUGIN_Stop(void)
{
    plugin_t * plug_iter;

    plug_iter = (plugin_t *) plugins_list.head;
    while (plug_iter != NULL)
    {
        plug_iter->vendor_stop();
        plug_iter = (plugin_t *) plug_iter->link.next;
    }
}

/*********************************************************************//**
**
** PLUGIN_GetFactoryResetParams
**
** Calls VENDOR_GetFactoryResetParams in all registered plugins adding to the list of factory reset parameters
**
** \param   params - list of factory reset parameters to add to
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
#ifdef INCLUDE_PROGRAMMATIC_FACTORY_RESET
int PLUGIN_GetFactoryResetParams(kv_vector_t *params)
{
    plugin_t * plugin;
    int err;

    plugin = (plugin_t *) plugins_list.head;
    while (plugin != NULL)
    {
        if (plugin->vendor_get_factory_reset_params != NULL)
        {
            err = plugin->vendor_get_factory_reset_params(&params);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }

        plugin = (plugin_t *) plugin->link.next;
    }

    return USP_ERR_OK;
}
#endif


