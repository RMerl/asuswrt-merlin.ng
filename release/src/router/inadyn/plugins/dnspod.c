/* Plugin for DNSPod
 *
 * Copyright (C) 2017  Richard Yu <yurichard3839@gmail.com>
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

/* dnspod.cn specific update request format */
#define DNSPOD_API_REQUEST						\
	"POST /%s HTTP/1.0\r\n"						\
	"Host: %s\r\n"							\
	"User-Agent: %s\r\n"						\
	"Content-Length: %zd\r\n"					\
	"Content-Type: application/x-www-form-urlencoded\r\n\r\n"	\
	"%s"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@dnspod.cn",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "dnsapi.cn",
	.server_url   = ""
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	int           i, rc = 0;
	http_t        client;
	http_trans_t  trans;
	char          buffer[256], domain[256], prefix[64];
	char          *tmp;
	int           record_id = 0;
	size_t        paramlen;

	/*
	 * API_ID = info->creds.username
	 * API_TOKEN = info->creds.password
	 */
	do {
		TRY(http_construct(&client));

		http_set_port(&client, info->server_name.port);
		http_set_remote_name(&client, info->server_name.name);

		client.ssl_enabled = info->ssl_enabled;
		TRY(http_init(&client, "Sending record list query"));

		tmp = strchr(alias->name, '.');
		if (tmp) {
			if (tmp[1] != 0 && strchr(tmp + 1, '.') != NULL) {
				strcpy(domain, tmp + 1);
				strncpy(prefix, alias->name, tmp - alias->name);
				prefix[tmp - alias->name] = 0;
			} else {
				strcpy(domain, alias->name);
				prefix[0] = '@';
				prefix[1] = 0;
			}
		}

		/* login_token=API_ID,API_TOKEN */
		paramlen = snprintf(buffer, sizeof(buffer),
				    "login_token=%s%%2C%s&format=json&domain=%s&length=1&sub_domain=%s",
				    info->creds.username, info->creds.password, domain, prefix);

		trans.req_len     = snprintf(ctx->request_buf, ctx->request_buflen, DNSPOD_API_REQUEST, "Record.List",
					     info->server_name.name, info->user_agent, paramlen, buffer);
		trans.req         = ctx->request_buf;
		trans.rsp         = ctx->work_buf;
		trans.max_rsp_len = ctx->work_buflen - 1; /* Save place for a \0 at the end */

		rc  = http_transaction(&client, &trans);
		rc |= http_exit(&client);

		http_destruct(&client, 1);

		if (rc)
			break;

		/* TODO: Check & log errors */
		TRY(http_status_valid(trans.status));

		/*
		 * Example: with added whitespace and line breaks for clarity
		 *{
		 *    "status": {"code": "1", "message": "Action completed successful", "created_at": "2017-06-28 14:36:28"},
		 *    "domain": {
		 *        "id": "59753949",
		 *        "name": "example.org",
		 *        "punycode": "example.org",
		 *        "grade": "DP_Free",
		 *        "owner": "example@example.org",
		 *        "ext_status": "dnserror",
		 *        "ttl": 600,
		 *        "min_ttl": 600,
		 *        "dnspod_ns": ["f1g1ns1.dnspod.net", "f1g1ns2.dnspod.net"],
		 *        "status": "enable"
		 *    },
		 *    "info": {"sub_domains": "3", "record_total": "3"},
		 *    "records": [{
		 *        "id": "306419640",
		 *        "ttl": "600",
		 *        "value": "1.2.3.4",
		 *        "enabled": "1",
		 *        "status": "enabled",
		 *        "updated_on": "2017-06-28 12:28:01",
		 *        "name": "@",
		 *        "line": "\u9ed8\u8ba4",
		 *        "line_id": "0",
		 *        "type": "A",
		 *        "weight": null,
		 *        "monitor_status": "",
		 *        "remark": "",
		 *        "use_aqb": "no",
		 *        "mx": "0"
		 *    }]
		 *}
		 */
		tmp = strstr(trans.rsp_body, "[{");
		if (!tmp) {
			rc = RC_DDNS_INVALID_OPTION;
			break;
		}

		sscanf(tmp, "[{\"id\":\"%u\"", &record_id);

		if (record_id == 0) {
			logit(LOG_ERR, "Record '%s' not found in records list!", prefix);
			rc = RC_DDNS_INVALID_OPTION;
			break;
		}
		logit(LOG_DEBUG, "DNSPod Record: '%s' ID: %u", prefix, record_id);

		paramlen = snprintf(buffer, sizeof(buffer),
				    "login_token=%s%%2C%s&format=json&domain=%s&record_id=%u&record_line=%s&value=%s",
				    info->creds.username, info->creds.password,
				    domain, record_id, "%E9%BB%98%E8%AE%A4", alias->address);

		return snprintf(ctx->request_buf, ctx->request_buflen,
				DNSPOD_API_REQUEST, "Record.Ddns",
				info->server_name.name, info->user_agent, paramlen, buffer);
	} while (0);

	return -1;
}

/*
 * DNSPod specific response validator.  With added whitespace for
 * clarity:
 *{
 *    "status": {
 *        "code": "1",
 *        "message": "Action completed successful",
 *        "created_at": "2017-06-28 15:11:16"
 *    },
 *    "record": {
 *        "id": 306419640,
 *        "name": "@",
 *        "value": "1.2.3.4"
 *    }
 *}
 *
 * We search our own IP address in response and that's enough.
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
