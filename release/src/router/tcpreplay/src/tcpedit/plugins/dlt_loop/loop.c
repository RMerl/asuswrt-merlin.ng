/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "loop.h"
#include "../dlt_null/null.h"
#include "dlt_utils.h"
#include "tcpedit.h"
#include "tcpedit_stub.h"
#include <string.h>

/*
 * Basically, DLT_LOOP and DLT_NULL are the same thing except that the PF_ value
 * in the header is always network byte order in DLT_LOOP and host byte order
 * in DLT_NULL.  So since DLT_NULL has to handle both big & little endian values
 * we just send all DLT_LOOP processing over there
 */

static char dlt_name[] = "loop";
static char _U_ dlt_prefix[] = "loop";
static uint16_t dlt_value = DLT_LOOP;

/*
 * Function to register ourselves.  This function is always called, regardless
 * of what DLT types are being used, so it shouldn't be allocating extra buffers
 * or anything like that (use the dlt_loop_init() function below for that).
 * Tasks:
 * - Create a new plugin struct
 * - Fill out the provides/requires bit masks.  Note:  Only specify which fields are
 *   actually in the header.
 * - Add the plugin to the context's plugin chain
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_loop_register(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    /* create  a new plugin structure */
    plugin = tcpedit_dlt_newplugin();

    /* set what we provide & require */
    plugin->provides += PLUGIN_MASK_PROTO;
    plugin->
        requires
    += 0;

    /* what is our DLT value? */
    plugin->dlt = dlt_value;

    /* set the prefix name of our plugin.  This is also used as the prefix for our options */
    plugin->name = safe_strdup(dlt_prefix);

    /* we actually call all the DLT_NULL functions since NULL and LOOP are basically the same thing */
    plugin->plugin_init = dlt_loop_init;
    plugin->plugin_cleanup = dlt_loop_cleanup;
    plugin->plugin_parse_opts = dlt_null_parse_opts;
    plugin->plugin_decode = dlt_null_decode;
    plugin->plugin_encode = dlt_null_encode;
    plugin->plugin_proto = dlt_null_proto;
    plugin->plugin_l2addr_type = dlt_null_l2addr_type;
    plugin->plugin_l2len = dlt_null_l2len;
    plugin->plugin_get_layer3 = dlt_null_get_layer3;
    plugin->plugin_merge_layer3 = dlt_null_merge_layer3;
    plugin->plugin_get_mac = dlt_null_get_mac;

    /* add it to the available plugin list */
    return tcpedit_dlt_addplugin(ctx, plugin);
}

/*
 * Initializer function.  This function is called only once, if and only if
 * this plugin will be utilized.  Remember, if you need to keep track of any state,
 * store it in your plugin->config, not a global!
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_loop_init(tcpeditdlt_t *ctx)
{
    assert(ctx);

    if (tcpedit_dlt_getplugin(ctx, dlt_value) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to initialize unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    return TCPEDIT_OK; /* success */
}

/*
 * Since this is used in a library, we should manually clean up after ourselves
 * Unless you allocated some memory in dlt_loop_init(), this is just an stub.
 * Returns: TCPEDIT_ERROR | TCPEDIT_OK | TCPEDIT_WARN
 */
int
dlt_loop_cleanup(tcpeditdlt_t *ctx)
{
    tcpeditdlt_plugin_t *plugin;
    assert(ctx);

    if ((plugin = tcpedit_dlt_getplugin(ctx, dlt_value)) == NULL) {
        tcpedit_seterr(ctx->tcpedit, "Unable to cleanup unregistered plugin %s", dlt_name);
        return TCPEDIT_ERROR;
    }

    safe_free(plugin->name);
    plugin->name = NULL;
    safe_free(plugin->config);
    plugin->config = NULL;
    plugin->config_size = 0;

    return TCPEDIT_OK; /* success */
}

/* that's all folks! */
