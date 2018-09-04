/* Plugin for Zerigo
 *
 * Copyright (C) 2011       Bryan Hoover <bhoover@wecs.com>
 * Copyright (C) 2014-2017  Joachim Nilsson <troglobit@gmail.com>
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

#define ZERIGO_UPDATE_IP_REQUEST					\
	"GET %s%s&"							\
	"ip=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: %s\r\n\r\n"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@zerigo.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "checkip.zerigo.com",
	.checkip_url  = "/",

	.server_name  = "update.zerigo.com",
	.server_url   = "/dynamic?host="
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			ZERIGO_UPDATE_IP_REQUEST,
			info->server_url,
			alias->name,
			alias->address,
			info->server_name.name,
			info->creds.encoded_password,
			info->user_agent);
}

/*
 * Zerigo return codes https://www.zerigo.com/docs/apis/dns/1.1
 *
 * OK
 *  Status: 2xx
 *
 * Invalid argument/password/alias/etc
 *  Status: 4xx
 *
 * Server error
 *  Status: 5xx
 */
static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *ptr, *rsp = trans->rsp_body;

	DO(http_status_valid(trans->status));

	ptr = strstr(rsp, "Status: ");
	if (ptr) {
		int code = 0;

		sscanf(++ptr, "%4d", &code);
		code /= 100;

		switch (code) {
		case 2:
			return 0;
			
		case 4:
			return RC_DDNS_INVALID_OPTION;

		case 5:
			return RC_DDNS_RSP_RETRY_LATER;
		}
	}

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
