/* Plugin for dnsmax.com
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

#define DNSMAX_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"username=%s&"							\
	"password=%s&"							\
	"resellerid=1&"							\
	"clientname=" PACKAGE_NAME "&"					\
	"clientversion=" PACKAGE_VERSION "&"				\
	"protocolversion=2.0&"						\
	"updatehostname=%s&"						\
	"ip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"


static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t dnsmax_plugin = {
	.name         = "default@dnsmax.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "update.dnsmax.com",
	.server_url   =  "/update/"
};

static ddns_system_t thatip_plugin = {
	.name         = "default@thatip.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "update.dnsmax.com",
	.server_url   =  "/update/"
};


static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			info->system->server_req,
			info->server_url,
			info->creds.username,
			info->creds.password,
			alias->name,
			alias->address,
			info->server_name.name,
			info->user_agent);
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *rsp = trans->rsp_body;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	if (strstr(rsp, alias->address))
		return 0;

	return RC_DDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&dnsmax_plugin, DNSMAX_UPDATE_IP_REQUEST);
	plugin_register(&thatip_plugin, DNSMAX_UPDATE_IP_REQUEST);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&dnsmax_plugin);
	plugin_unregister(&thatip_plugin);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
