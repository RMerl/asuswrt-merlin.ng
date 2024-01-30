/* Common plugin methods, built around DynDNS standard HTTP API
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

/*
 * DynDNS request composer -- common to many other DDNS providers as well
 */
int common_request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	char wildcard[20] = "";

	if (info->wildcard)
		strlcpy(wildcard, "&wildcard=ON", sizeof(wildcard));

	return snprintf(ctx->request_buf, ctx->request_buflen,
			info->system->server_req,
			info->server_url,
			alias->name,
			alias->address,
			wildcard,
			info->server_name.name,
			info->creds.encoded_password,
			info->user_agent);
}

/*
 * DynDNS response validator -- common to many other DDNS providers as well
 *  'good' or 'nochg' are the good answers,
 */
int common_response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *body = trans->rsp_body;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	if (strstr(body, "good") || strstr(body, "nochg")  || strstr(body, "OK"))
		return 0;

	if (strstr(body, "dnserr") || strstr(body, "911") || strstr(body, "abuse"))
		return RC_DDNS_RSP_RETRY_LATER;

	if (strstr(body, "badauth") || strstr(body, "!donator"))
		return RC_DDNS_RSP_AUTH_FAIL;

	/* Loopia responds "[200 OK] nohost" when no DNS record exists */
	if (strstr(body, "nohost"))
		return RC_DDNS_RSP_NOHOST;

	if (strstr(body, "nofqdn"))
		return RC_DDNS_RSP_NOHOST;

	return RC_DDNS_RSP_NOTOK;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
