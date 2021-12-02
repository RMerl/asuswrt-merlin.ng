/* Plugin for dynv6.com
 *
 * Copyright (C) 2016  Sven Hoefer <sven@svenhoefer.com>
 * Copyright (C) 2021  Joachim Wiberg <troglobit@gmail.com>
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

#define DYNV6_UPDATE_IP_REQUEST						\
	"GET %s?"							\
	"%s=auto&"							\
	"hostname=%s&"							\
	"token=%s"							\
	" "								\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin4 = {
	.name         = "default@ipv4.dynv6.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "ipv4.dynv6.com",
	.server_url   =  "/api/update"
};

static ddns_system_t plugin6 = {
	.name         = "default@dynv6.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "dynv6.com",
	.server_url   =  "/api/update"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	const char *ver = "ipv6";

	if (strstr(info->system->name, "ipv4"))
		ver = "ipv4";

	return snprintf(ctx->request_buf, ctx->request_buflen,
			DYNV6_UPDATE_IP_REQUEST,
			info->server_url,
			ver,			/* ipv6=auto, or ipv4=auto */
			alias->name,
			info->creds.username,
			info->server_name.name,
			info->user_agent);
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *resp = trans->rsp_body;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	if (strstr(resp, "updated") || strstr(resp, "unchanged"))
		return 0;

	return RC_DDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&plugin4);
	plugin_register(&plugin6);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&plugin4);
	plugin_unregister(&plugin6);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
