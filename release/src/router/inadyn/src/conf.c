/* libConfuse interface to parse inadyn.conf v2 format
 *
 * Copyright (C) 2014-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <string.h>
#include <confuse.h>

#include "cache.h"
#include "ddns.h"
#include "ssl.h"

/*
 * period        = 600
 * forced-update = 604800
 *
 * provider default@freedns.afraid.org
 * {
 *   wildcard = false
 *   username = example
 *   password = secret
 *   hostname = { "example.homenet.org", "example.afraid.org" }
 * }
 *
 * provider default@dyndns.org
 * {
 *   ssl      = true
 *   username = admin
 *   password = supersecret
 *   hostname = example.dyndns.org
 * }
 */
static LIST_HEAD(head, di) info_list = LIST_HEAD_INITIALIZER(info_list);

static void conf_errfunc(cfg_t *cfg, const char *format, va_list args)
{
	char fmt[80];

	if (cfg && cfg->filename && cfg->line)
		snprintf(fmt, sizeof(fmt), "%s:%d: %s", cfg->filename, cfg->line, format);
	else if (cfg && cfg->filename)
		snprintf(fmt, sizeof(fmt), "%s: %s", cfg->filename, format);
	else
		snprintf(fmt, sizeof(fmt), "%s", format);

	vlogit(LOG_ERR, fmt, args);
}

#ifdef DROP_CHECK_CONFIG
/*
 * Convert deprecated 'alias' setting to new 'hostname',
 * same functionality with new name.
 */
static int deprecate_alias(cfg_t *cfg)
{
	size_t i;
	cfg_opt_t *alias, *hostname;

	alias = cfg_getopt(cfg, "alias");
	if (!alias || cfg_opt_size(alias) <= 0)
		return 0;

	hostname = cfg_getopt(cfg, "hostname");
	if (cfg_opt_size(hostname) > 0) {
		cfg_error(cfg, "Both 'hostname' and 'alias' set, cannot convert deprecated 'alias' to 'hostname'");
		return -1;
	}

	cfg_error(cfg, "converting 'alias' to 'hostname'.");
	for (i = 0; i < cfg_opt_size(alias); i++)
		cfg_opt_setnstr(hostname, cfg_opt_getnstr(alias, i), i);

	cfg_free_value(alias);

	return 0;
}

static int validate_period(cfg_t *cfg, cfg_opt_t *opt)
{
	int val = cfg_getint(cfg, opt->name);

	if (val < DDNS_MIN_PERIOD)
		cfg_setint(cfg, opt->name, DDNS_MIN_PERIOD);
	if (val > DDNS_MAX_PERIOD)
		cfg_setint(cfg, opt->name, DDNS_MAX_PERIOD);

	return 0;
}

static int validate_hostname(cfg_t *cfg, const char *provider, cfg_opt_t *hostname)
{
	size_t i;

	if (!hostname) {
		cfg_error(cfg, "DDNS hostname setting is missing in provider %s", provider);
		return -1;
	}

	if (!cfg_opt_size(hostname)) {
		cfg_error(cfg, "No hostnames listed in DDNS provider %s", provider);
		return -1;
	}

	for (i = 0; i < cfg_opt_size(hostname); i++) {
		char *name = cfg_opt_getnstr(hostname, i);
		ddns_info_t info;

		if (sizeof(info.alias[0].name) < strlen(name)) {
			cfg_error(cfg, "Too long DDNS hostname (%s) in provider %s", name, provider);
			return -1;
		}
	}

	if (i >= DDNS_MAX_ALIAS_NUMBER) {
		cfg_error(cfg, "Too many hostname aliases, MAX %d supported!", DDNS_MAX_ALIAS_NUMBER);
		return -1;
	}

	return 0;
}

/* No need to validate username/password for custom providers */
static int validate_common(cfg_t *cfg, const char *provider, int custom)
{
	ddns_system_t *ds;

	ds = plugin_find(provider, 0);
	if (!ds) {
		ds = plugin_find(provider, 1);
		if (!ds) {
			cfg_error(cfg, "Invalid DDNS provider %s", provider);
			return -1;
		}
	}

	if (!custom) {
		if (!ds->nousername && !cfg_getstr(cfg, "username")) {
			cfg_error(cfg, "Missing username setting for DDNS provider %s", provider);
			return -1;
		}

		if (!cfg_getstr(cfg, "password")) {
			cfg_error(cfg, "Missing password setting for DDNS provider %s", provider);
			return -1;
		}
	}

	return deprecate_alias(cfg) ||
		validate_hostname(cfg, provider, cfg_getopt(cfg, "hostname"));
}

static int validate_provider(cfg_t *cfg, cfg_opt_t *opt)
{
	const char *provider;

	cfg = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);
	provider = cfg_title(cfg);

	if (!provider) {
		cfg_error(cfg, "Missing DDNS provider name");
		return -1;
	}

	return validate_common(cfg, provider, 0);
}

static int validate_custom(cfg_t *cfg, cfg_opt_t *opt)
{
	cfg = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);
	if (!cfg)
		return -1;

	if (!cfg_getstr(cfg, "ddns-server")) {
		cfg_error(cfg, "Missing 'ddns-server' for custom DDNS provider");
		return -1;
	}

	return validate_common(cfg, "custom", 1);
}
#endif

/* server:port => server:80 if port is not given */
static int getserver(const char *server, ddns_name_t *name)
{
	char *str, *ptr;

	if (strlen(server) > sizeof(name->name))
		return 1;

	str = strdup(server);
	if (!str)
		return 1;

	ptr = strchr(str, ':');
	if (ptr) {
		*ptr++ = 0;
		name->port = atonum(ptr);
		if (-1 == name->port)
			name->port = HTTP_DEFAULT_PORT;
	} else {
		/* Let *ssl.c and tcp.c figure it out later */
		name->port = 0;
	}

	strlcpy(name->name, str, sizeof(name->name));
	free(str);

	return 0;
}

static int cfg_getserver(cfg_t *cfg, char *server, ddns_name_t *name)
{
	const char *str;

	str = cfg_getstr(cfg, server);
	if (!str)
		return 1;

	return getserver(str, name);
}

#if 0
/* TODO: Move to a separate file */
#define string_startswith(string, prefix) strncasecmp(string, prefix, strlen(prefix)) == 0

static int parseproxy(const char *proxy, tcp_proxy_type_t *type, ddns_name_t *name)
{
	int ret = 0;
	char *tmp, *str, *host, *protocol;
	int len;

	tmp = str = strdup(proxy);

	do {
		tmp = strstr(str, "://");
		if (tmp) {
			host = tmp + 3;
			if (string_startswith(str, "socks5h"))
				*type = PROXY_SOCKS5_HOSTNAME;
			else if (string_startswith(str, "socks5"))
				*type = PROXY_SOCKS5;
			else if (string_startswith(str, "socks4a"))
				*type = PROXY_SOCKS4A;
			else if (string_startswith(str, "socks4") || string_startswith(str, "socks"))
				*type = PROXY_SOCKS4;
			else {
				len = tmp - str;
				protocol = malloc(len + 2);
				strncpy(protocol, str, len);
				protocol[len + 1] = 0;
				logit(LOG_ERR, "Unsupported proxy protocol '%s'.", protocol);
				free(protocol);
				ret = 1;
				break;
			}

			tmp = strchr(host, ':');
			if (tmp) {
				*tmp++ = 0;
				name->port = atonum(tmp);
				if (-1 == name->port) {
					logit(LOG_ERR, "Invalid proxy port.");
					ret = 1;
					break;
				}

				strlcpy(name->name, host, sizeof(name->name));
			} else {
				logit(LOG_ERR, "No proxy port is specified.");
				ret = 1;
				break;
			}
		} else {
			/* Currently we do not support http proxy. */
//			*type = PROXY_HTTP;
//			logit(LOG_WARNING, "No proxy protocol is specified, use http proxy.");

			logit(LOG_WARNING, "No proxy protocol is specified.");
			ret = 1;
			break;
		}
	} while (0);

	free(str);
	return ret;
}

static int cfg_parseproxy(cfg_t *cfg, char *server, tcp_proxy_type_t *type, ddns_name_t *name)
{
	const char *str;

	str = cfg_getstr(cfg, server);
	if (!str)
		return 1;

	return parseproxy(str, type, name);
}
#endif

static int set_provider_opts(cfg_t *cfg, ddns_info_t *info, int custom)
{
	ddns_system_t *system;
	const char *str;
	size_t j;

	if (custom)
		str = "custom";
	else
		str = cfg_title(cfg);

	system = plugin_find(str, 0);
	if (!system) {
		system = plugin_find(str, 1);
		if (!system) {
			logit(LOG_ERR, "Cannot find an DDNS plugin for provider '%s'", str);
			return 1;
		}
		logit(LOG_WARNING, "Guessing DDNS plugin '%s' from '%s'", system->name, str);
	}

	info->system = system;

	if (getserver(system->checkip_name, &info->checkip_name))
		goto error;
	if (strlen(system->checkip_url) > sizeof(info->checkip_url))
		goto error;
	strlcpy(info->checkip_url, system->checkip_url, sizeof(info->checkip_url));

	if (getserver(system->server_name, &info->server_name))
		goto error;
	if (strlen(system->server_url) > sizeof(info->server_url))
		goto error;
	strlcpy(info->server_url, system->server_url, sizeof(info->server_url));

	info->wildcard = cfg_getbool(cfg, "wildcard");
	info->ttl = cfg_getint(cfg, "ttl");
	info->proxied = cfg_getbool(cfg, "proxied");
	info->ssl_enabled = cfg_getbool(cfg, "ssl");
	str = cfg_getstr(cfg, "username");
	if (str && strlen(str) <= sizeof(info->creds.username))
		strlcpy(info->creds.username, str, sizeof(info->creds.username));
	str = cfg_getstr(cfg, "password");
	if (str && strlen(str) <= sizeof(info->creds.password))
		strlcpy(info->creds.password, str, sizeof(info->creds.password));
	info->ifname = cfg_getstr(cfg, "iface");

	for (j = 0; j < cfg_size(cfg, "hostname"); j++) {
		size_t pos = info->alias_count;

		str = cfg_getnstr(cfg, "hostname", j);
		if (!str)
			continue;

		if (info->alias_count == DDNS_MAX_ALIAS_NUMBER) {
			logit(LOG_WARNING, "Too many hostname aliases, skipping %s ...", str);
			continue;
		}

		strlcpy(info->alias[pos].name, str, sizeof(info->alias[pos].name));
		info->alias_count++;
	}

	if (custom) {
		info->append_myip = cfg_getbool(cfg, "append-myip");

		cfg_getserver(cfg, "ddns-server", &info->server_name);
		str = cfg_getstr(cfg, "ddns-path");
		if (str && strlen(str) <= sizeof(info->server_url))
			strlcpy(info->server_url, str, sizeof(info->server_url));

		for (j = 0; j < cfg_size(cfg, "ddns-response"); j++) {
			size_t pos = info->server_response_num;

			str = cfg_getnstr(cfg, "ddns-response", j);
			if (!str)
				continue;

			if (info->server_response_num >= NELEMS(info->server_response)) {
				logit(LOG_WARNING, "Skipping response '%s', only %zu custom responses supported",
				      str, NELEMS(info->server_response));
				continue;
			}

			strlcpy(info->server_response[pos], str, sizeof(info->server_response[pos]));
			info->server_response_num++;
		}

		/* Default check, if no configured custom response string(s) */
		if (!cfg_size(cfg, "ddns-response")) {
			for (j = 0; j < NELEMS(info->server_response); j++) {
				if (!generic_responses[j])
					break;

				strlcpy(info->server_response[j], generic_responses[j], sizeof(info->server_response[j]));
				info->server_response_num++;
			}
		}
	}

	/*
	 * Follows the ssl setting by default, except for providers
	 * known to NOT support HTTPS for their checkip servers.
	 *
	 * This setting may only be disabled by the user, with the
	 * custom provider section being the exeception to the rule.
	 */
	info->checkip_ssl = info->ssl_enabled;

	/* Check known status of checkip server for provider */
	switch (system->checkip_ssl) {
	case DDNS_CHECKIP_SSL_UNSUPPORTED:
		info->checkip_ssl = 0;
		break;

	case DDNS_CHECKIP_SSL_REQUIRED:
		info->checkip_ssl = 1;
		break;

	default:
	case DDNS_CHECKIP_SSL_SUPPORTED:
		if (!cfg_getbool(cfg, "checkip-ssl"))
			info->checkip_ssl = 0;
	}

	/* The checkip server can be set for all provider types */
	if (!cfg_getserver(cfg, "checkip-server", &info->checkip_name)) {
		str = cfg_getstr(cfg, "checkip-path");
		if (str && strlen(str) <= sizeof(info->checkip_url))
			strlcpy(info->checkip_url, str, sizeof(info->checkip_url));
		else
			strlcpy(info->checkip_url, "/", sizeof(info->checkip_url));

		if (!strcasecmp(info->checkip_name.name, "default")) {
			strlcpy(info->checkip_name.name, DDNS_MY_IP_SERVER, sizeof(info->checkip_name.name));
			strlcpy(info->checkip_url, DDNS_MY_CHECKIP_URL, sizeof(info->checkip_url));
			info->checkip_ssl = DDNS_MY_IP_SSL;
		}

		/*
		 * If a custom checkip server is defined, the
		 * checkip-ssl setting is fully honored.
		 */
		info->checkip_ssl = cfg_getbool(cfg, "checkip-ssl");
	}

	/* The checkip-command overrides any default or custom checkip-server */
	str = cfg_getstr(cfg, "checkip-command");
	if (str && strlen(str) > 0)
		info->checkip_cmd = strdup(str);
	else if (script_cmd)
		info->checkip_cmd = strdup(script_cmd);

	/* The per-provider user-agent setting, defaults to the global setting */
	info->user_agent = cfg_getstr(cfg, "user-agent");
	if (!info->user_agent)
		info->user_agent = user_agent;

	/* A per-proivder optional proxy server:port */
#if 0
	cfg_parseproxy(cfg, "proxy", &info->proxy_type, &info->proxy_name);
#endif
	return 0;

error:
	logit(LOG_ERR, "Failed setting up %s DDNS provider, skipping.", str);
	return 1;
}

static int create_provider(cfg_t *cfg, int custom)
{
	ddns_info_t *info;

	info = calloc(1, sizeof(*info));
	if (!info) {
		logit(LOG_ERR, "Failed allocating memory for provider %s", cfg_title(cfg));
		return 1;
	}

	http_construct(&info->checkip);
	http_construct(&info->server);
	if (set_provider_opts(cfg, info, custom)) {
		free(info);
		return 1;
	}

	LIST_INSERT_HEAD(&info_list, info, link);
	return 0;
}

ddns_info_t *conf_info_iterator(int first)
{
	static ddns_info_t *ptr = NULL;

	if (first) {
		ptr = LIST_FIRST(&info_list);
		return ptr;
	}

	if (!ptr || ptr == LIST_END(&info_list))
		return NULL;

	ptr = LIST_NEXT(ptr, link);
	return ptr;
}

void conf_info_cleanup(void)
{
	ddns_info_t *ptr, *tmp;

	LIST_FOREACH_SAFE(ptr, &info_list, link, tmp) {
		if (ptr->creds.encoded_password)
			free(ptr->creds.encoded_password);
		if (ptr->checkip_cmd)
			free(ptr->checkip_cmd);
		if (ptr->data)
			free(ptr->data);
		LIST_REMOVE(ptr, link);
		free(ptr);
	}
}

cfg_t *conf_parse_file(char *file, ddns_t *ctx)
{
	int ret = 0;
	size_t i;
	cfg_opt_t provider_opts[] = {
		CFG_FUNC    ("include",      &cfg_include),
		CFG_STR     ("username",     NULL, CFGF_NONE),
		CFG_STR     ("password",     NULL, CFGF_NONE),
		CFG_STR_LIST("hostname",     NULL, CFGF_NONE),
		CFG_STR_LIST("alias",        NULL, CFGF_DEPRECATED),
		CFG_BOOL    ("ssl",          cfg_true, CFGF_NONE),
		CFG_BOOL    ("wildcard",     cfg_false, CFGF_NONE),
		CFG_INT     ("ttl",          -1, CFGF_NODEFAULT),
		CFG_BOOL    ("proxied",      cfg_false, CFGF_NONE),
		CFG_STR     ("iface",          NULL, CFGF_NONE), /* interface name */
		CFG_STR     ("checkip-server", NULL, CFGF_NONE), /* Syntax:  name:port */
		CFG_STR     ("checkip-path",   NULL, CFGF_NONE), /* Default: "/" */
		CFG_BOOL    ("checkip-ssl",    cfg_true, CFGF_NONE),
		CFG_STR     ("checkip-command",NULL, CFGF_NONE), /* Syntax: /path/to/cmd [args] */
		CFG_STR     ("user-agent",     NULL, CFGF_NONE),
//		CFG_STR     ("proxy",          NULL, CFGF_NONE), /* Syntax:  name:port */
		CFG_END()
	};
	cfg_opt_t custom_opts[] = {
		/* Same as a general provider */
		CFG_FUNC    ("include",      &cfg_include),
		CFG_STR     ("username",     NULL, CFGF_NONE),
		CFG_STR     ("password",     NULL, CFGF_NONE),
		CFG_STR_LIST("hostname",     NULL, CFGF_NONE),
		CFG_STR_LIST("alias",        NULL, CFGF_DEPRECATED),
		CFG_BOOL    ("ssl",          cfg_true, CFGF_NONE),
		CFG_BOOL    ("wildcard",     cfg_false, CFGF_NONE),
		CFG_INT     ("ttl",          -1, CFGF_NODEFAULT),
		CFG_BOOL    ("proxied",      cfg_false, CFGF_NONE),
		CFG_STR     ("iface",          NULL, CFGF_NONE), /* interface name */
		CFG_STR     ("checkip-server", NULL, CFGF_NONE), /* Syntax:  name:port */
		CFG_STR     ("checkip-path",   NULL, CFGF_NONE), /* Default: "/" */
		CFG_BOOL    ("checkip-ssl",    cfg_true, CFGF_NONE),
		CFG_STR     ("checkip-command",NULL, CFGF_NONE), /* Syntax: /path/to/cmd [args] */
		CFG_STR     ("user-agent",     NULL, CFGF_NONE),
//		CFG_STR     ("proxy",          NULL, CFGF_NONE), /* Syntax:  name:port */
		/* Custom settings */
		CFG_BOOL    ("append-myip",    cfg_false, CFGF_NONE),
		CFG_STR     ("ddns-server",    NULL, CFGF_NONE),
		CFG_STR     ("ddns-path",      NULL, CFGF_NONE),
		CFG_STR_LIST("ddns-response",  NULL, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_BOOL("verify-address", cfg_true, CFGF_NONE),
		CFG_BOOL("fake-address",  cfg_false, CFGF_NONE),
		CFG_BOOL("allow-ipv6",    cfg_false, CFGF_NONE),
		CFG_BOOL("secure-ssl",    cfg_true, CFGF_NONE),
		CFG_BOOL("broken-rtc",    cfg_false, CFGF_NONE),
		CFG_STR ("ca-trust-file", NULL, CFGF_NONE),
		CFG_STR ("cache-dir",	  NULL, CFGF_DEPRECATED | CFGF_DROP),
		CFG_INT ("period",	  DDNS_DEFAULT_PERIOD, CFGF_NONE),
		CFG_INT ("iterations",    DDNS_DEFAULT_ITERATIONS, CFGF_NONE),
		CFG_INT ("forced-update", DDNS_FORCED_UPDATE_PERIOD, CFGF_NONE),
		CFG_STR ("iface",         NULL, CFGF_NONE),
		CFG_STR ("user-agent",    NULL, CFGF_NONE),
		CFG_SEC ("provider",      provider_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_SEC ("custom",        custom_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	if (!cfg) {
		logit(LOG_ERR, "Failed initializing configuration file parser: %s", strerror(errno));
		return NULL;
	}

	/* Custom logging, rather than default Confuse stderr logging */
	cfg_set_error_function(cfg, conf_errfunc);

#ifdef DROP_CHECK_CONFIG
	/* Validators */
	cfg_set_validate_func(cfg, "period", validate_period);
	cfg_set_validate_func(cfg, "provider", validate_provider);
	cfg_set_validate_func(cfg, "custom", validate_custom);
#endif

	switch (cfg_parse(cfg, file)) {
	case CFG_FILE_ERROR:
		logit(LOG_ERR, "Cannot read configuration file %s", file);
		return NULL;

	case CFG_PARSE_ERROR:
		logit(LOG_ERR, "Parse error in %s", file);
		return NULL;

	case CFG_SUCCESS:
		break;
	}

	/* Set global options */
	ctx->normal_update_period_sec = cfg_getint(cfg, "period");
	ctx->error_update_period_sec  = DDNS_ERROR_UPDATE_PERIOD;
	ctx->forced_update_period_sec = cfg_getint(cfg, "forced-update");
	if (once)
		ctx->total_iterations = 1;
	else
		ctx->total_iterations = cfg_getint(cfg, "iterations");

	verify_addr                   = cfg_getbool(cfg, "verify-address");
	ctx->forced_update_fake_addr  = cfg_getbool(cfg, "fake-address");

	/* Command line --iface=IFNAME takes precedence */
	if (!use_iface)
		iface                 = cfg_getstr(cfg, "iface");
	user_agent                    = cfg_getstr(cfg, "user-agent");
	if (!user_agent)
		user_agent            = DDNS_USER_AGENT;
	allow_ipv6                    = cfg_getbool(cfg, "allow-ipv6");
	secure_ssl                    = cfg_getbool(cfg, "secure-ssl");
	broken_rtc                    = cfg_getbool(cfg, "broken-rtc");
	ca_trust_file                 = cfg_getstr(cfg, "ca-trust-file");
	if (ca_trust_file && !fexist(ca_trust_file)) {
		logit(LOG_ERR, "Cannot find CA trust file %s", ca_trust_file);
		return NULL;
	}

	for (i = 0; i < cfg_size(cfg, "provider"); i++)
		ret |= create_provider(cfg_getnsec(cfg, "provider", i), 0);

	for (i = 0; i < cfg_size(cfg, "custom"); i++)
		ret |= create_provider(cfg_getnsec(cfg, "custom", i), 1);

	if (ret)
		return NULL;

	return cfg;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
