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

#include <stdarg.h>
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
	"Content-Length: %zu\r\n\r\n"		\
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

struct cx {
	unsigned int record_id;
	unsigned int domain_id;

	char date[30];
	char hmac[MD5_DIGEST_BYTES * 2 + 1];
	char body[256];
	size_t len;
};

struct http {
	ddns_t      *ctx;
	ddns_info_t *info;
	char        *response;	/* pointer to ctx->work_buf */
};

static int setup    (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t plugin = {
	.name         = "default@cloudxns.net",

	.setup        = (setup_fn_t)setup,
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

static void hmac(char *dst, size_t sz, char *fmt, ...)
{
	unsigned char out[MD5_DIGEST_BYTES];
	va_list ap;
	size_t i, len;
	char buf[256];

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	md5((unsigned char *)buf, len, out);
	memset(dst, 0, sz);

	for (i = 0; i < MD5_DIGEST_BYTES; i++) {
		char hex[3];

                snprintf(hex, sizeof(hex), "%02x", out[i]);
                strlcat(dst, hex, sz);
	}
}

static int http_send(struct http *http, char *msg, char *fmt, ...)
{
	http_trans_t trans;
	ddns_info_t *info;
	va_list ap;
	http_t client;
	ddns_t *ctx;
	int rc;

	info = http->info;
	ctx = http->ctx;

	va_start(ap, fmt);
	trans.req_len     = vsnprintf(ctx->request_buf, ctx->request_buflen, fmt, ap);
	trans.req         = ctx->request_buf;
	trans.rsp         = ctx->work_buf;
	trans.max_rsp_len = ctx->work_buflen - 1;	/* Save place for a \0 at the end */
	va_end(ap);

	rc = http_construct(&client);
	if (rc)
		return rc;

	http_set_port(&client, info->server_name.port);
	http_set_remote_name(&client, info->server_name.name);
	client.ssl_enabled = info->ssl_enabled;

	rc = http_init(&client, msg);
	if (rc)
		goto err;

	rc = http_transaction(&client, &trans);
	http_exit(&client);
err:
	http_destruct(&client, 1);
	if (rc)
		return rc;

	http->response = trans.rsp_body;

	return http_status_valid(trans.status);
}

/*
 * API_KEY = info->creds.username
 * SECRET_KEY = info->creds.password
 */
static int setup(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	struct http   http = { ctx, info, NULL };
	struct cx    *cx;
	char          str[MD5_DIGEST_BYTES * 2 + 1];
	char          buffer[256], domain[256], prefix[SERVER_NAME_LEN];
	char          *tmp, *item;
	size_t        hostlen, domainlen;
	int           rc = 0;

	if (!info->data) {
		info->data = malloc(sizeof(struct cx));
		if (!info->data)
			return RC_OUT_OF_MEMORY;
	}
	cx = (struct cx *)info->data;
	memset(cx, 0, sizeof(struct cx));

	get_time(cx->date, sizeof(cx->date));

	/* HMAC=md5(API_KEY+URL+DATE+SECRET_KEY) */
	hmac(str, sizeof(str), "%shttp%s://www.cloudxns.net/api2/domain%s%s",
	     info->creds.username, info->ssl_enabled ? "s" : "",
	     cx->date, info->creds.password);

	rc = http_send(&http, "Sending domain list query",
		       CLOUDXNS_GET_REQUEST, "/api2/domain",
		       info->server_name.name, info->user_agent,
		       info->creds.username, cx->date, str);
	if (rc) {
		logit(LOG_WARNING, "Failed fetching domain list, rc: %d", rc);
		goto err;
	}

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
	tmp = strchr(http.response, '[');
	if (!tmp) {
		rc = RC_DDNS_INVALID_OPTION;
		goto err;
	}

	for (item = tmp; item; item = strstr(item, ",{")) {
		unsigned int id;
		int num;

		item++;
		num = sscanf(item, "{\"id\":\"%u\",\"domain\":\"%255[^\"]", &id, domain);
		if (num == 2 && *domain) {
			domain[strlen(domain) - 1] = 0;  /* Remove trailing dot */
			if (string_endswith(alias->name, domain)) {
				cx->domain_id = id;
				break;
			}
		}
	}

	if (cx->domain_id == 0) {
		logit(LOG_ERR, "Hostname '%s' not found in domains list!", alias->name);
		rc = RC_DDNS_INVALID_OPTION;
		goto err;
	}

	logit(LOG_DEBUG, "CloudXNS Domain: '%s' ID: %u", domain, cx->domain_id);

	/* HMAC=md5(API_KEY+URL+DATE+SECRET_KEY) */
	hmac(str, sizeof(str), "%shttp%s://www.cloudxns.net/api2/record/%u%s%s",
	     info->creds.username, info->ssl_enabled ? "s" : "",
	     cx->domain_id, cx->date, info->creds.password);

	snprintf(buffer, sizeof(buffer), "/api2/record/%u", cx->domain_id);
	rc = http_send(&http, "Sending records list query",
		       CLOUDXNS_GET_REQUEST, buffer,
		       info->server_name.name, info->user_agent,
		       info->creds.username, cx->date, str);
	if (rc) {
		logit(LOG_WARNING, "Failed fetching record ID, rc: %d", rc);
		goto err;
	}

	tmp = strchr(http.response, '[');
	if (!tmp) {
		rc = RC_DDNS_INVALID_OPTION;
		goto err;
	}

	hostlen = strlen(alias->name);
	domainlen = strlen(domain);
	if (hostlen == domainlen) {
		prefix[0] = '@';
		prefix[1] = 0;
	} else {
		size_t num = hostlen - domainlen - 1;

		if (num <= sizeof(prefix))
			strlcpy(prefix, alias->name, num);
	}
	
	for (item = tmp; item; item = strstr(item, ",{")) {
		unsigned int id;
		char _prefix[64];
		int num;

		item++;
		num = sscanf(item, "{\"record_id\":\"%u\",\"host_id\":\"%*d\",\"host\":\"%63[^\"]", &id, _prefix);
		if (num == 2 && *_prefix) {
			if (string_compare(prefix, _prefix)) {
				cx->record_id = id;
				break;
			}
		}
	}

	if (cx->record_id == 0) {
		logit(LOG_ERR, "Record '%s' not found in records list!", prefix);
		rc = RC_DDNS_INVALID_OPTION;
		goto err;
	}
	logit(LOG_DEBUG, "CloudXNS Record: '%s' ID: %u", prefix, cx->record_id);

	cx->len = snprintf(cx->body, sizeof(cx->body),
			   CLOUDXNS_UPDATE_PARAM_BODY,
			   cx->domain_id, prefix, alias->address);

	/* HMAC=md5(API_KEY+URL+PARAM_BODY+DATE+SECRET_KEY) */
	hmac(cx->hmac, sizeof(cx->hmac),
	     "%shttp%s://www.cloudxns.net/api2/record/%u%s%s%s",
	     info->creds.username, info->ssl_enabled ? "s" : "",
	     cx->record_id, cx->body, cx->date, info->creds.password);

	return 0;
err:
	free(info->data);
	info->data = NULL;
	return rc;
}

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	struct cx *cx = (struct cx *)info->data;

	(void)alias;

	/* cx is filled in by setup() */
	if (!cx)
		return -1;

	return snprintf(ctx->request_buf, ctx->request_buflen,
			CLOUDXNS_UPDATE_IP_REQUEST,
			info->server_url,
			cx->record_id,
			info->server_name.name,
			info->user_agent,
			info->creds.username, cx->date, cx->hmac,
			cx->len, cx->body);
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

	(void)info;
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
