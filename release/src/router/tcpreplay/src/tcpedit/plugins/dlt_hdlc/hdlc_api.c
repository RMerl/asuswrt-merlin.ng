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

#include "hdlc_api.h"

/**
 * \brief Set the HDLC control value
 *
 * Reasonable values are 0-255
 */
int
tcpedit_hdlc_set_control(tcpedit_t *tcpedit, uint8_t control)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    hdlc_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (hdlc_config_t *)plugin->config;

    config->control = control;
    return TCPEDIT_OK;
}

/**
 * \brief Set the HDLC address value
 *
 * Reasonable values are 0x000F (unicast) and 0x00BF (broadcast), but we
 * accept any value
 */
int
tcpedit_hdlc_set_address(tcpedit_t *tcpedit, uint8_t address)
{
    tcpeditdlt_t *ctx;
    tcpeditdlt_plugin_t *plugin;
    hdlc_config_t *config;

    assert(tcpedit);

    ctx = tcpedit->dlt_ctx;
    assert(ctx);
    plugin = ctx->decoder;
    assert(plugin);
    config = (hdlc_config_t *)plugin->config;

    config->address = address;
    return TCPEDIT_OK;
}
