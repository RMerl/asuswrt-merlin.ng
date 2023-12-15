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

#include "user_api.h"
#include "tcpedit.h"
#include <stdlib.h>
#include <string.h>

/**
 * \brief Define the libpcap DLT Type value
 */
int
tcpedit_user_set_dlt_type(tcpedit_t *tcpedit, uint16_t type)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    user_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (user_config_t *)plugin->config;

    config->dlt = type;
    return TCPEDIT_OK;
}

/**
 * \brief Define the actual L2 header content.
 *
 * You need to set the data, it's length and which direction(s) to apply to.
 * BOTH - both directions (or in the case of no tcpprep cache file)
 * S2C - server to client (primary interface)
 * C2S - client to server (secondary interface)
 *
 * NOTE: the datalen value must be the same between each call.
 */
int
tcpedit_user_set_dlink(tcpedit_t *tcpedit, u_char *data, int datalen, tcpedit_user_dlt_direction direction)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    user_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (user_config_t *)plugin->config;

    /* sanity checks */
    if (datalen <= 0) {
        tcpedit_seterr(tcpedit, "%s", "user datalink length must be > 0");
        return TCPEDIT_ERROR;
    } else if (datalen > USER_L2MAXLEN) {
        tcpedit_seterr(tcpedit, "user datalink length is > %d.  Please increase USER_L2MAXLEN", USER_L2MAXLEN);
        return TCPEDIT_ERROR;
    }

    if ((config->length > 0) && (config->length != datalen)) {
        tcpedit_seterr(tcpedit, "%s", "Subsequent calls to tcpedit_user_set_dlink() must use the same datalen");
        return TCPEDIT_ERROR;
    } else {
        config->length = datalen;
        switch (direction) {
        case TCPEDIT_USER_DLT_BOTH:
            memcpy(config->l2server, data, datalen);
            memcpy(config->l2client, data, datalen);
            break;

        case TCPEDIT_USER_DLT_S2C:
            memcpy(config->l2server, data, datalen);
            break;

        case TCPEDIT_USER_DLT_C2S:
            memcpy(config->l2client, data, datalen);
            break;
        }
    }
    return TCPEDIT_OK;
}
