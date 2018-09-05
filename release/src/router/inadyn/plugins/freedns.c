/* Plugin for FreeDNS
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2006       Steve Horbachuk
 * Copyright (C) 2010-2017  Joachim Nilsson <troglobit@gmail.com>
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

#include "sha1.h"
#include "plugin.h"

/* freedns.afraid.org specific update request format */
#define FREEDNS_UPDATE_IP_REQUEST					\
	"GET %s?"							\
	"%s&"								\
	"address=%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n\r\n"
#define SHA1_DIGEST_BYTES 20

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@freedns.afraid.org",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = "freedns.afraid.org",
	.checkip_url  = "/dynamic/check.php",

	.server_name  = "freedns.afraid.org",
	.server_url   = "/dynamic/update.php"
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
#ifdef ENABLE_SIMULATION
	char         *hash = "<NIL>";
#else
	int           i, rc = 0;
	http_t        client;
	http_trans_t  trans;
	char         *buf, *tmp, *line, *hash = NULL;
	char          host[256], updateurl[256];
	char          buffer[256];
	char          digeststr[SHA1_DIGEST_BYTES * 2 + 1];
	unsigned char digestbuf[SHA1_DIGEST_BYTES];

	do {
		/* FreeDNS requires an API key, the following code fetches yours */
		TRY(http_construct(&client));

		http_set_port(&client, info->server_name.port);
		http_set_remote_name(&client, info->server_name.name);

		client.ssl_enabled = info->ssl_enabled;
		TRY(http_init(&client, "Fetching account API key"));

		snprintf(buffer, sizeof(buffer), "%s|%s",
			 info->creds.username, info->creds.password);
		sha1((unsigned char *)buffer, strlen(buffer), digestbuf);
		for (i = 0; i < SHA1_DIGEST_BYTES; i++)
			sprintf(&digeststr[i * 2], "%02x", digestbuf[i]);

		snprintf(buffer, sizeof(buffer), "/api/?action=getdyndns&v=2&sha=%s", digeststr);
		trans.req_len     = snprintf(ctx->request_buf, ctx->request_buflen, GENERIC_HTTP_REQUEST,
					     buffer, info->server_name.name, info->user_agent);
		trans.req         = ctx->request_buf;
		trans.rsp         = ctx->work_buf;
		trans.max_rsp_len = ctx->work_buflen - 1;	/* Save place for a \0 at the end */

		rc  = http_transaction(&client, &trans);
		http_exit(&client);
		http_destruct(&client, 1);

		logit(LOG_DEBUG, "Received API key(s), rc=%d:\n%s", rc, trans.rsp_body);
		if (rc)
			break;

		TRY(http_status_valid(trans.status));

		tmp = buf = strdup(trans.rsp_body);
		for (line = strsep(&tmp, "\n"); line; line = strsep(&tmp, "\n")) {
			int num;

			num = sscanf(line, "%255[^|\r\n]|%*[^|\r\n]|%255[^|\r\n]", host, updateurl);
			if (*line && num == 2 && !strcmp(host, alias->name)) {
				hash = strstr(updateurl, "?");
				break;
			}
		}
		free(buf);

		if (!hash)
			rc = RC_DDNS_RSP_NOTOK;
		else
			hash++;
	}
	while (0);

	if (rc) {
		if (rc == RC_DDNS_RSP_NOTOK)
			logit(LOG_INFO, "Cannot find your DNS name in the list of API keys");
		else
			logit(LOG_INFO, "Cannot find you FreeDNS account API keys");

		return 0;
	}
#endif /* ENABLE_SIMULATION */

	return snprintf(ctx->request_buf, ctx->request_buflen,
		       FREEDNS_UPDATE_IP_REQUEST,
		       info->server_url,
		       hash, alias->address,
		       info->server_name.name,
			info->user_agent);
}

/* Freedns afraid.org.specific response validator.
   ok blabla and n.n.n.n
    fail blabla and n.n.n.n
    are the good answers. We search our own IP address in response and that's enough.
*/
static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *resp = trans->rsp_body;

	DO(http_status_valid(trans->status));

	if (strstr(resp, alias->address))
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
