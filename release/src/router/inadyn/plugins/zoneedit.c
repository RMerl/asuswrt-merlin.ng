/* Plugin for ZoneEdit
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

#define ZONEEDIT_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"host=%s&"							\
	"dnsto=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: %s\r\n\r\n"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@zoneedit.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "dynamic.zoneedit.com",
	.checkip_url  = "/checkip.html",

	.server_name  = "dynamic.zoneedit.com",
	.server_url   = "/auth/dynamic.html"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			ZONEEDIT_UPDATE_IP_REQUEST,
			info->server_url,
			alias->name,
			alias->address,
			info->server_name.name,
			info->creds.encoded_password,
			info->user_agent);
}

/*
 * ZoneEdit OK codes are:
 *  CODE=200, 201
 *  CODE=707, for duplicated updates
 */
static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	int code = -1;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	sscanf(trans->rsp_body, "%*s CODE=\"%4d\" ", &code);
	switch (code) {
	case 200:
	case 201:
	case 707:
		/* XXX: is 707 really OK? */
		return 0;
	default:
		break;
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
