/* Plugin for CloudXNS
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

#include <time.h>

#include "md5.h"
#include "plugin.h"

/* cloudxns.net specific update request format */
#define CLOUDXNS_UPDATE_IP_REQUEST		\
	"PUT %s/%u "				\
	"HTTP/1.0\r\n"				\
	"Host: %s\r\n"				\
	"User-Agent: %s\r\n"			\
	"API-KEY: %s\r\n"			\
	"API-REQUEST-DATE: %s\r\n"		\
	"API-HMAC: %s\r\n"			\
	"Content-Length: %zd\r\n\r\n"		\
	"%s"
#define CLOUDXNS_GET_REQUEST			\
	"GET %s HTTP/1.0\r\n"			\
	"Host: %s\r\n"				\
	"User-Agent: %s\r\n"			\
	"API-KEY: %s\r\n"			\
	"API-REQUEST-DATE: %s\r\n"		\
	"API-HMAC: %s\r\n\r\n"
#define CLOUDXNS_UPDATE_PARAM_BODY		\
	"{\"domain_id\":\"%u\","		\
	"\"host\":\"%s\","			\
	"\"value\":\"%s\"}"
#define MD5_DIGEST_BYTES  16

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@cloudxns.net",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DYNDNS_MY_IP_SSL,

	.server_name  = "www.cloudxns.net",
	.server_url   = "/api2/record"
};

/* http://stackoverflow.com/a/744822 TODO: Move to a separate file */
static int string_endswith(const char *str, const char *suffix)
{
	size_t lenstr;
	size_t lensuffix;

	if (!str || !suffix)
		return 0;

	lenstr = strlen(str);
	lensuffix = strlen(suffix);
	if (lensuffix > lenstr)
		return 0;

	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static char *get_time(char *str, size_t len)
{
	time_t rawtime;

	time(&rawtime);
	strftime(str, len, "%a %b %d %T %Y", localtime(&rawtime));

	return str;
}

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	int           i, rc = 0;
	http_t        client;
	http_trans_t  trans;
	char          digeststr[MD5_DIGEST_BYTES * 2 + 1];
	unsigned char digestbuf[MD5_DIGEST_BYTES];
	char          buffer[256], domain[256], prefix[64], param[256], date[30];
	char          *tmp, *item;
	int           domain_id = 0, record_id = 0;
	size_t        hostlen, domainlen, len, paramlen;

	get_time(date, sizeof(date));

	/*
	 * API_KEY = info->creds.username
	 * SECRET_KEY = info->creds.password
	 */
	do {
		TRY(http_construct(&client));

		http_set_port(&client, info->server_name.port);
		http_set_remote_name(&client, info->server_name.name);

		client.ssl_enabled = info->ssl_enabled;
		TRY(http_init(&client, "Sending domain list query"));

		/* HMAC=md5(API_KEY+URL+DATE+SECRET_KEY) */
		len = snprintf(buffer, sizeof(buffer), "%shttp%s://www.cloudxns.net/api2/domain%s%s",
			       info->creds.username, info->ssl_enabled ? "s" : "", date, info->creds.password);
		md5((unsigned char *)buffer, len, digestbuf);
		for (i = 0; i < MD5_DIGEST_BYTES; i++)
			sprintf(&digeststr[i * 2], "%02x", digestbuf[i]);

		trans.req_len     = snprintf(ctx->request_buf, ctx->request_buflen, CLOUDXNS_GET_REQUEST, "/api2/domain",
					     info->server_name.name, info->user_agent, info->creds.username, date, digeststr);
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
		 * { "code":1, "message":"Operate successfully", "total":"1",
		 *   "data": [{
		 *              "id":"12345", "domain":"example.com.", "status":"ok",
		 *              "level":"3", "take_over_status":"Taken over",
		 *              "create_time":"2017-04-04 07:58:56",
		 *              "update_time":"2017-04-08 18:43:57",
		 *              "ttl":"600"
		 *    }]
		 * }
		 */
		tmp = strchr(trans.rsp_body, '[');
		if (!tmp) {
			rc = RC_DDNS_INVALID_OPTION;
			break;
		}

		for (item = tmp; item; item = strstr(item, ",{")) {
			int num, id;

			item++;
			num = sscanf(item, "{\"id\":\"%u\",\"domain\":\"%255[^\"]", &id, domain);
			if (num == 2 && *domain) {
				domain[strlen(domain) - 1] = 0;  /* Remove trailing dot */
				if (string_endswith(alias->name, domain)) {
					domain_id = id;
					break;
				}
			}
		}

		if (domain_id == 0) {
			logit(LOG_ERR, "Hostname '%s' not found in domains list!", alias->name);
			rc = RC_DDNS_INVALID_OPTION;
			break;
		}
		logit(LOG_DEBUG, "CloudXNS Domain: '%s' ID: %u", domain, domain_id);


		TRY(http_construct(&client));

		http_set_port(&client, info->server_name.port);
		http_set_remote_name(&client, info->server_name.name);

		client.ssl_enabled = info->ssl_enabled;
		TRY(http_init(&client, "Sending records list query"));

		/* HMAC=md5(API_KEY+URL+DATE+SECRET_KEY) */
		len = snprintf(buffer, sizeof(buffer), "%shttp%s://www.cloudxns.net/api2/record/%u%s%s",
			       info->creds.username, info->ssl_enabled ? "s" : "", domain_id, date, info->creds.password);
		md5((unsigned char *)buffer, len, digestbuf);
		for (i = 0; i < MD5_DIGEST_BYTES; i++)
			sprintf(&digeststr[i * 2], "%02x", digestbuf[i]);

		snprintf(buffer, sizeof(buffer), "/api2/record/%u", domain_id);
		trans.req_len     = snprintf(ctx->request_buf, ctx->request_buflen, CLOUDXNS_GET_REQUEST,
					     buffer, info->server_name.name, info->user_agent, info->creds.username, date, digeststr);
		trans.req         = ctx->request_buf;
		trans.rsp         = ctx->work_buf;
		trans.max_rsp_len = ctx->work_buflen - 1;	/* Save place for a \0 at the end */

		rc  = http_transaction(&client, &trans);
		rc |= http_exit(&client);

		http_destruct(&client, 1);

		if (rc)
			break;

		/* TODO: Check & log errors. */
		TRY(http_status_valid(trans.status));

		tmp = strchr(trans.rsp_body, '[');
		if (!tmp) {
			rc = RC_DDNS_INVALID_OPTION;
			break;
		}

		hostlen = strlen(alias->name);
		domainlen = strlen(domain);
		if (hostlen == domainlen) {
			prefix[0] = '@';
			prefix[1] = 0;
		} else {
			strncpy(prefix, alias->name, hostlen - domainlen - 1);
			prefix[hostlen - domainlen - 1] = 0;
		}

		for (item = tmp; item; item = strstr(item, ",{")) {
			int num, id;
			char _prefix[64];

			item++;
			num = sscanf(item, "{\"record_id\":\"%u\",\"host_id\":\"%*d\",\"host\":\"%63[^\"]", &id, _prefix);
			if (num == 2 && *_prefix) {
				if (string_compare(prefix, _prefix)) {
					record_id = id;
					break;
				}
			}
		}

		if (record_id == 0) {
			logit(LOG_ERR, "Record '%s' not found in records list!", prefix);
			rc = RC_DDNS_INVALID_OPTION;
			break;
		}
		logit(LOG_DEBUG, "CloudXNS Record: '%s' ID: %u", prefix, record_id);

		paramlen = snprintf(param, sizeof(param), CLOUDXNS_UPDATE_PARAM_BODY, domain_id, prefix, alias->address);

		/* HMAC=md5(API_KEY+URL+PARAM_BODY+DATE+SECRET_KEY) */
		len = snprintf(buffer, sizeof(buffer), "%shttp%s://www.cloudxns.net/api2/record/%u%s%s%s",
			       info->creds.username, info->ssl_enabled ? "s" : "", record_id, param, date, info->creds.password);
		md5((unsigned char *)buffer, len, digestbuf);
		for (i = 0; i < MD5_DIGEST_BYTES; i++)
			sprintf(&digeststr[i * 2], "%02x", digestbuf[i]);

		return snprintf(ctx->request_buf, ctx->request_buflen,
			       CLOUDXNS_UPDATE_IP_REQUEST,
			       info->server_url,
			       record_id,
			       info->server_name.name,
			       info->user_agent,
			       info->creds.username, date, digeststr,
			       paramlen, param);
	} while (0);

	return -1;
}

/*
 * CloudXNS specific response validator.  With added whitespace for
 * clarity:
 * { "code":1, "message":"success",
 *   "data": {
 *             "id":12345, "domain_name":"example.com.",
 *             "value":"1.2.3.4","bak_data":""
 *   }
 * }
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
