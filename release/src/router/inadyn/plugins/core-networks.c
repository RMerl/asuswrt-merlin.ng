/* Plugin for core-networks.de
 *
 * Copyright (C) 2023 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#define CORE_NETWORKS_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"hostname=%s&"							\
	"myip=%s&"							\
	"%s "								\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: %s\r\n\r\n"


static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@core-networks.de",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "dyndns.core-networks.de",
	.server_url   =  "/"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	char *keepip;
	if (strstr(info->system->name, "ipv6"))
		keepip = "keepipv4=1";
	else
		keepip = "keepipv6=1";

	return snprintf(ctx->request_buf, ctx->request_buflen,
			info->system->server_req,
			info->server_url,
			alias->name,
			alias->address,
			keepip,
			info->server_name.name,
			info->creds.encoded_password,
			info->user_agent);
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *rsp = trans->rsp_body;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	if (strstr(rsp, "good"))
		return 0;

	return RC_DDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&plugin, CORE_NETWORKS_UPDATE_IP_REQUEST);
	plugin_register_v6(&plugin, CORE_NETWORKS_UPDATE_IP_REQUEST);
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
