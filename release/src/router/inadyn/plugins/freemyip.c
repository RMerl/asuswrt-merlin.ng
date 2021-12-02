/* Plugin for FreeMyIP
 *
 * Copyright (C) 2017  Tomasz 'Cadence' Grabowski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "plugin.h"

/*
 * For API information, see https://freemyip.com/help
 * Maybe add support for "&myip=1.2.3.4"?
 */
#define FREEMYIP_UPDATE_IP_HTTP_REQUEST				\
	"GET %s?"						\
	"token=%s&"						\
	"domain=%s "						\
	"HTTP/1.0\r\n"						\
	"Host: %s\r\n"						\
	"User-Agent: %s\r\n\r\n"

static int request (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@freemyip.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.nousername   = 1,	/* Provider does not require username */

	.checkip_name = "wtfismyip.com",
	.checkip_url  = "/text",

	.server_name  = "freemyip.com",
	.server_url   = "/update"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			FREEMYIP_UPDATE_IP_HTTP_REQUEST,
			info->server_url,
			info->creds.password,
			alias->name,
			info->server_name.name, info->user_agent);
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	if (strstr(trans->rsp_body, "OK"))
		return RC_OK;

	return RC_DDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&plugin);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&plugin);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
