/* Plugin for Hurricate Electric's IPv6 service
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

#include "md5.h"
#include "plugin.h"

/* HE tunnelbroker.com specific update request format */
#define HE_IPV6TB_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"ip=%s&"							\
	"apikey=%s&"							\
	"pass=%s&"							\
	"tid=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"
#define MD5_DIGEST_BYTES  16

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "ipv6tb@he.net",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "checkip.dns.he.net",
	.checkip_url  = "/",

	.server_name  = "ipv4.tunnelbroker.net",
	.server_url   = "/ipv4_end.php"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	int           i;
	char          digeststr[MD5_DIGEST_BYTES * 2 + 1];
	unsigned char digestbuf[MD5_DIGEST_BYTES];

	md5((unsigned char *)info->creds.password, strlen(info->creds.password), digestbuf);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		sprintf(&digeststr[i * 2], "%02x", digestbuf[i]);

	return snprintf(ctx->request_buf, ctx->request_buflen,
			HE_IPV6TB_UPDATE_IP_REQUEST,
			info->server_url,
			alias->address,
			info->creds.username,
			digeststr,
			alias->name,
			info->server_name.name,
			info->user_agent);
}

/*
 * Hurricate Electric IPv6 tunnelbroker specific response validator
 * Own IP address and 'already in use' are the good answers.
 */
static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *resp = trans->rsp_body;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	if (strstr(resp, alias->address) ||
	    strstr(resp, "-ERROR: This tunnel is already associated with this IP address."))
		return 0;

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
