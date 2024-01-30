/* Plugin for ChangeIP and OVH
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2021  Joachim Wiberg <troglobit@gmail.com>
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

#define CHANGEIP_UPDATE_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"system=dyndns&"						\
	"hostname=%s&"							\
	"myip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: %s\r\n\r\n"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@changeip.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "ip.changeip.com",
	.checkip_url  = "/",

	.server_name  = "nic.changeip.com",
	.server_url   = "/nic/update"
};

static ddns_system_t ovh = {
	.name         = "default@ovh.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "www.ovh.com",
	.server_url   = "/nic/update"
};


static ddns_system_t strato = {
	.name         = "default@strato.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "dyndns.strato.com",
	.server_url   = "/nic/update"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			info->system->server_req,
			info->server_url,
			alias->name,
			alias->address,
			info->server_name.name,
			info->creds.encoded_password,
			info->user_agent);
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	return common_response(trans, info, alias);
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&plugin, CHANGEIP_UPDATE_IP_HTTP_REQUEST);
	plugin_register(&ovh, CHANGEIP_UPDATE_IP_HTTP_REQUEST);
	plugin_register_v6(&ovh, CHANGEIP_UPDATE_IP_HTTP_REQUEST);
	plugin_register(&strato, CHANGEIP_UPDATE_IP_HTTP_REQUEST);
	plugin_register_v6(&strato, CHANGEIP_UPDATE_IP_HTTP_REQUEST);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&plugin);
	plugin_unregister(&ovh);
	plugin_unregister(&strato);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
