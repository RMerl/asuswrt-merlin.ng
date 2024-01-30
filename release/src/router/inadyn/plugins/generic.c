/* Generic DDNS plugin
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

#include <ctype.h>
#include "plugin.h"

/*
 * Generic update format for sites that perform the update using:
 *
 *	http://some.address.domain/somesubdir?some_param_name=ALIAS
 *
 * With the standard http stuff and basic base64 encoded auth.
 * The parameter here is the entire request, except the the alias.
 */
#define GENERIC_BASIC_AUTH_UPDATE_IP_REQUEST				\
	"GET %s%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: %s\r\n\r\n"

const char * const generic_responses[] =
    { "OK", "good", "true", "updated", "success", "nochg", NULL };

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t generic = {
	.name         = "custom",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DDNS_MY_IP_SERVER,
	.checkip_url  = DDNS_MY_CHECKIP_URL,
	.checkip_ssl  = DDNS_MY_IP_SSL,

	.server_name  = "",
	.server_url   = ""
};

/* Replace %? with @str */
static void replace_fmt(char *fmt, char *str, size_t fmtlen)
{
	char *src = fmt + fmtlen;
	size_t len = strlen(str);

	memmove(fmt + len, src, strlen(src) + 1);
	memcpy(fmt, str, strlen(str));
}

/* Skip %? if it has not been specified */
static void skip_fmt(char *fmt, size_t len)
{
	char *src = fmt + len;

	memmove(fmt, src, strlen(src) + 1);
}


/*
 * Fully custom server URL, with % format specifiers
 *
 * %u - username
 * %p - password, if HTTP basic auth is not used
 * %h - hostname
 * %i - IP address
 */
static int custom_server_url(char *buf, ddns_info_t *info, ddns_alias_t *alias)
{
	char *ptr;

	while ((ptr = strchr(buf, '%'))) {
		if (!strncmp(ptr, "%u", 2)) {
			if (strnlen(info->creds.username, USERNAME_LEN) <= 0) {
				logit(LOG_ERR, "Format specifier in ddns-path used: '%%u',"
				      " but 'username' configuration option has not been specified!");
				skip_fmt(ptr, 2);
			} else {
				replace_fmt(ptr, info->creds.username, 2);
			}

			continue;
		}

		if (!strncmp(ptr, "%p", 2)) {
			if (strnlen(info->creds.password, PASSWORD_LEN) <= 0) {
				logit(LOG_ERR, "Format specifier in ddns-path used: '%%p',"
				      " but 'password' configuration option has not been specified!");
				skip_fmt(ptr, 2);
			} else {
				replace_fmt(ptr, info->creds.password, 2);
			}

			continue;
		}

		if (!strncmp(ptr, "%h", 2)) {
			replace_fmt(ptr, alias->name, 2);
			continue;
		}

		if (!strncmp(ptr, "%i", 2)) {
			replace_fmt(ptr, alias->address, 2);
			continue;
		}

		/* NOTE: This should be the last one */
		if (!strncmp(ptr, "%%", 2)) {
			replace_fmt(ptr, "%", 2);
			continue;
		}

		logit(LOG_ERR, "Unknown format specifier in ddns-path: '%c'", ptr[1]);
		return -1;
	}

	return strlen(buf);
}

static char tohex(char code)
{
	static const char hex[] = "0123456789abcdef";

	return hex[code & 15];
}

/* Used to check if user already URL encoded */
static int ishex(char *str)
{
	if (strlen(str) < 3)
		return 0;

	if (str[0] == '%' && isxdigit(str[1]) && isxdigit(str[2]))
		return 1;

	return 0;
}

/*
 * Simple URL encoder, with exceptions for /, ?, =, and &, which should
 * usually be encoded as well, but are here exposed raw to advanced
 * end-users.
 */
static char *url_encode(char *str)
{
	char *buf, *ptr;

	buf = calloc(strlen(str) * 3 + 1, sizeof(char));
	if (!buf)
		return NULL;
	ptr = buf;

	while (str[0]) {
		char ch = str[0];

		if (ishex(str)) {
			*ptr++ = *str++;
			*ptr++ = *str++;
			*ptr++ = *str++;
			continue;
		}

		if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~')
			*ptr++ = ch;
		else if (ch == '/' || ch == '?' || ch == '&' || ch == '=')
			*ptr++ = ch;
		else if (ch == ' ')
			*ptr++ = '+';
		else
			*ptr++ = '%', *ptr++ = tohex(ch >> 4), *ptr++ = tohex(ch & 15);
		str++;
	}
	*ptr = '\0';

	return buf;
}

/*
 * This function is called for every listed hostname alias in the
 * custom{} section.  There is currently no way to only call it
 * once, in case a DDNS provider supports many hostnames in the
 * HTTP GET URL.
 */
static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	char  buf[2 * sizeof(info->server_url)];
	char *arg = "";
	char *url;
	int ret;

	/* Operate of copy of URL, may contain %-modifiers */
	strlcpy(buf, info->server_url, sizeof(buf));

	/*
	 * if the user has specified modifiers, then they probably know
	 * how to append his hostname or IP, otherwise just append the
	 * hostname or ip (depending on the append_myip option)
	 */
	if (strchr(buf, '%')) {
		if (custom_server_url(buf, info, alias) <= 0)
			logit(LOG_ERR, "Invalid server URL: %s", info->server_url);
	} else {
		/* Backwards compat, default to append hostname */
		arg = alias->name;

		if (info->append_myip)
			arg = alias->address;
	}

	url = url_encode(buf);
	if (!url)
		return 0;

	ret = snprintf(ctx->request_buf, ctx->request_buflen,
		       info->system->server_req,
		       url, arg,
		       info->server_name.name,
		       info->creds.encoded_password,
		       info->user_agent);
	free(url);

	return ret;
}

static int response(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias)
{
	char *resp = trans->rsp_body;
	size_t i;

	(void)info;
	(void)alias;

	DO(http_status_valid(trans->status));

	for (i = 0; i < info->server_response_num; i++) {
		if (strcasestr(resp, info->server_response[i]))
			return 0;
	}

	return RC_DDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&generic, GENERIC_BASIC_AUTH_UPDATE_IP_REQUEST);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&generic);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
