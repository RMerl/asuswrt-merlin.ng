/*
 * Copyright (C) 2014 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netdb.h>

#include <library.h>
#include <utils/debug.h>

#include "keywords.h"
#include "confread.h"
#include "args.h"
#include "files.h"
#include "parser/conf_parser.h"

#define IKE_LIFETIME_DEFAULT         10800 /* 3 hours */
#define IPSEC_LIFETIME_DEFAULT        3600 /* 1 hour */
#define SA_REPLACEMENT_MARGIN_DEFAULT  540 /* 9 minutes */
#define SA_REPLACEMENT_FUZZ_DEFAULT    100 /* 100% of margin */
#define SA_REPLACEMENT_RETRIES_DEFAULT   3
#define SA_REPLAY_WINDOW_DEFAULT        -1 /* use charon.replay_window */

static const char ike_defaults[] = "aes128-sha1-modp2048,3des-sha1-modp1536";
static const char esp_defaults[] = "aes128-sha1,3des-sha1";

static const char firewall_defaults[] = IPSEC_SCRIPT " _updown iptables";

/**
 * Provided by GPERF
 */
extern kw_entry_t *in_word_set (char *str, unsigned int len);

static bool daemon_exists(char *daemon, char *path)
{
	struct stat st;
	if (stat(path, &st) != 0)
	{
		DBG1(DBG_APP, "Disabling %sstart option, '%s' not found", daemon, path);
		return FALSE;
	}
	return TRUE;
}

/**
 * Process deprecated keywords
 */
static bool is_deprecated(kw_token_t token, char *name, char *conn)
{
	switch (token)
	{
		case KW_SETUP_DEPRECATED:
		case KW_PKCS11_DEPRECATED:
			DBG1(DBG_APP, "# deprecated keyword '%s' in config setup", name);
			break;
		case KW_CONN_DEPRECATED:
		case KW_END_DEPRECATED:
		case KW_PFS_DEPRECATED:
			DBG1(DBG_APP, "# deprecated keyword '%s' in conn '%s'", name, conn);
			break;
		case KW_CA_DEPRECATED:
			DBG1(DBG_APP, "# deprecated keyword '%s' in ca '%s'", name, conn);
			break;
		default:
			return FALSE;
	}
	/* additional messages for some */
	switch (token)
	{
		case KW_PKCS11_DEPRECATED:
			DBG1(DBG_APP, "  use the 'pkcs11' plugin instead");
			break;
		case KW_PFS_DEPRECATED:
			DBG1(DBG_APP, "  PFS is enabled by specifying a DH group in the "
				 "'esp' cipher suite");
			break;
		default:
			break;
	}
	return TRUE;
}

/*
 * parse config setup section
 */
static void load_setup(starter_config_t *cfg, conf_parser_t *parser)
{
	enumerator_t *enumerator;
	dictionary_t *dict;
	kw_entry_t *entry;
	char *key, *value;

	DBG2(DBG_APP, "Loading config setup");
	dict = parser->get_section(parser, CONF_PARSER_CONFIG_SETUP, NULL);
	if (!dict)
	{
		return;
	}
	enumerator = dict->create_enumerator(dict);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		bool assigned = FALSE;

		entry = in_word_set(key, strlen(key));
		if (!entry)
		{
			DBG1(DBG_APP, "# unknown keyword '%s'", key);
			cfg->non_fatal_err++;
			continue;
		}
		if ((int)entry->token < KW_SETUP_FIRST || entry->token > KW_SETUP_LAST)
		{
			DBG1(DBG_APP, "# unsupported keyword '%s' in config setup", key);
			cfg->err++;
			continue;
		}
		if (is_deprecated(entry->token, key, ""))
		{
			cfg->non_fatal_err++;
			continue;
		}
		if (!assign_arg(entry->token, KW_SETUP_FIRST, key, value, cfg,
						&assigned))
		{
			DBG1(DBG_APP, "  bad argument value in config setup");
			cfg->err++;
		}
	}
	enumerator->destroy(enumerator);
	dict->destroy(dict);

	/* verify the executables are actually available */
#ifdef START_CHARON
	cfg->setup.charonstart = cfg->setup.charonstart &&
							 daemon_exists(daemon_name, cmd);
#else
	cfg->setup.charonstart = FALSE;
#endif
}

/*
 * parse a ca section
 */
static void load_ca(starter_ca_t *ca, starter_config_t *cfg,
					conf_parser_t *parser)
{
	enumerator_t *enumerator;
	dictionary_t *dict;
	kw_entry_t *entry;
	kw_token_t token;
	char *key, *value;

	DBG2(DBG_APP, "Loading ca '%s'", ca->name);
	dict = parser->get_section(parser, CONF_PARSER_CA, ca->name);
	if (!dict)
	{
		return;
	}
	enumerator = dict->create_enumerator(dict);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		bool assigned = FALSE;

		entry = in_word_set(key, strlen(key));
		if (!entry)
		{
			DBG1(DBG_APP, "# unknown keyword '%s'", key);
			cfg->non_fatal_err++;
			continue;
		}
		token = entry->token;
		if (token == KW_AUTO)
		{
			token = KW_CA_SETUP;
		}
		if (token < KW_CA_FIRST || token > KW_CA_LAST)
		{
			DBG1(DBG_APP, "# unsupported keyword '%s' in ca '%s'",
				 key, ca->name);
			cfg->err++;
			continue;
		}
		if (is_deprecated(token, key, ca->name))
		{
			cfg->non_fatal_err++;
			continue;
		}
		if (!assign_arg(token, KW_CA_FIRST, key, value, ca, &assigned))
		{
			DBG1(DBG_APP, "  bad argument value in ca '%s'", ca->name);
			cfg->err++;
		}
	}
	enumerator->destroy(enumerator);
	dict->destroy(dict);

	/* treat 'route' and 'start' as 'add' */
	if (ca->startup != STARTUP_NO)
	{
		ca->startup = STARTUP_ADD;
	}
}

/*
 * set some default values
 */
static void conn_defaults(starter_conn_t *conn)
{
	conn->startup = STARTUP_NO;
	conn->state   = STATE_IGNORE;
	conn->mode    = MODE_TUNNEL;
	conn->options = SA_OPTION_MOBIKE;

	conn->ike                   = strdupnull(ike_defaults);
	/* esp defaults are set after parsing the conn section */
	conn->sa_ike_life_seconds   = IKE_LIFETIME_DEFAULT;
	conn->sa_ipsec_life_seconds = IPSEC_LIFETIME_DEFAULT;
	conn->sa_rekey_margin       = SA_REPLACEMENT_MARGIN_DEFAULT;
	conn->sa_rekey_fuzz         = SA_REPLACEMENT_FUZZ_DEFAULT;
	conn->sa_keying_tries       = SA_REPLACEMENT_RETRIES_DEFAULT;
	conn->install_policy        = TRUE;
	conn->dpd_delay             =  30; /* seconds */
	conn->dpd_timeout           = 150; /* seconds */
	conn->replay_window         = SA_REPLAY_WINDOW_DEFAULT;

	conn->left.sendcert = CERT_SEND_IF_ASKED;
	conn->right.sendcert = CERT_SEND_IF_ASKED;

	conn->left.ikeport = 500;
	conn->right.ikeport = 500;

	conn->left.to_port = 0xffff;
	conn->right.to_port = 0xffff;
}

/*
 * parse left|right specific options
 */
static void kw_end(starter_conn_t *conn, starter_end_t *end, kw_token_t token,
				   char *key, char *value, starter_config_t *cfg)
{
	bool assigned = FALSE;

	if (is_deprecated(token, key, conn->name))
	{
		cfg->non_fatal_err++;
		return;
	}

	if (!assign_arg(token, KW_END_FIRST, key, value, end, &assigned))
	{
		goto err;
	}

	/* post processing of some keywords that were assigned automatically */
	switch (token)
	{
		case KW_HOST:
			if (value && strlen(value) > 0 && value[0] == '%')
			{
				if (streq(value, "%defaultroute"))
				{
					value = "%any";
				}
				if (!streq(value, "%any") && !streq(value, "%any4") &&
					!streq(value, "%any6"))
				{	/* allow_any prefix */
					end->allow_any = TRUE;
					value++;
				}
			}
			free(end->host);
			end->host = strdupnull(value);
			break;
		case KW_SOURCEIP:
			conn->mode = MODE_TUNNEL;
			conn->proxy_mode = FALSE;
			break;
		case KW_SENDCERT:
			if (end->sendcert == CERT_YES_SEND)
			{
				end->sendcert = CERT_ALWAYS_SEND;
			}
			else if (end->sendcert == CERT_NO_SEND)
			{
				end->sendcert = CERT_NEVER_SEND;
			}
			break;
		default:
			break;
	}

	if (assigned)
	{
		return;
	}

	/* individual processing of keywords that were not assigned automatically */
	switch (token)
	{
		case KW_PROTOPORT:
		{
			struct protoent *proto;
			struct servent *svc;
			char *sep, *port = "", *endptr;
			long int p;

			sep = strchr(value, '/');
			if (sep)
			{	/* protocol/port */
				*sep = '\0';
				port = sep + 1;
			}

			if (streq(value, "%any"))
			{
				end->protocol = 0;
			}
			else
			{
				proto = getprotobyname(value);
				if (proto)
				{
					end->protocol = proto->p_proto;
				}
				else
				{
					p = strtol(value, &endptr, 0);
					if ((*value && *endptr) || p < 0 || p > 0xff)
					{
						DBG1(DBG_APP, "# bad protocol: %s=%s", key, value);
						goto err;
					}
					end->protocol = (u_int8_t)p;
				}
			}
			if (streq(port, "%any"))
			{
				end->from_port = 0;
				end->to_port = 0xffff;
			}
			else if (streq(port, "%opaque"))
			{
				end->from_port = 0xffff;
				end->to_port = 0;
			}
			else if (*port)
			{
				svc = getservbyname(port, NULL);
				if (svc)
				{
					end->from_port = end->to_port = ntohs(svc->s_port);
				}
				else
				{
					p = strtol(port, &endptr, 0);
					if (p < 0 || p > 0xffff)
					{
						DBG1(DBG_APP, "# bad port: %s=%s", key, port);
						goto err;
					}
					end->from_port = p;
					if (*endptr == '-')
					{
						port = endptr + 1;
						p = strtol(port, &endptr, 0);
						if (p < 0 || p > 0xffff)
						{
							DBG1(DBG_APP, "# bad port: %s=%s", key, port);
							goto err;
						}
					}
					end->to_port = p;
					if (*endptr)
					{
						DBG1(DBG_APP, "# bad port: %s=%s", key, port);
						goto err;
					}
				}
			}
			if (sep)
			{	/* restore the original text in case also= is used */
				*sep = '/';
			}
			break;
		}
		default:
			break;
	}
	return;

err:
	DBG1(DBG_APP, "  bad argument value in conn '%s'", conn->name);
	cfg->err++;
}

/*
 * macro to handle simple flags
 */
#define KW_SA_OPTION_FLAG(sy, sn, fl) \
		if (streq(value, sy)) { conn->options |= fl; } \
		else if (streq(value, sn)) { conn->options &= ~fl; } \
		else { DBG1(DBG_APP, "# bad option value: %s=%s", key, value); cfg->err++; }

/*
 * parse settings not handled by the simple argument parser
 */
static void handle_keyword(kw_token_t token, starter_conn_t *conn, char *key,
						   char *value, starter_config_t *cfg)
{
	if ((token == KW_ESP && conn->ah) || (token == KW_AH && conn->esp))
	{
		DBG1(DBG_APP, "# can't have both 'ah' and 'esp' options");
		cfg->err++;
		return;
	}
	switch (token)
	{
		case KW_TYPE:
		{
			conn->mode = MODE_TRANSPORT;
			conn->proxy_mode = FALSE;
			if (streq(value, "tunnel"))
			{
				conn->mode = MODE_TUNNEL;
			}
			else if (streq(value, "beet"))
			{
				conn->mode = MODE_BEET;
			}
			else if (streq(value, "transport_proxy"))
			{
				conn->mode = MODE_TRANSPORT;
				conn->proxy_mode = TRUE;
			}
			else if (streq(value, "passthrough") || streq(value, "pass"))
			{
				conn->mode = MODE_PASS;
			}
			else if (streq(value, "drop") || streq(value, "reject"))
			{
				conn->mode = MODE_DROP;
			}
			else if (!streq(value, "transport"))
			{
				DBG1(DBG_APP, "# bad policy value: %s=%s", key, value);
				cfg->err++;
			}
			break;
		}
		case KW_COMPRESS:
			KW_SA_OPTION_FLAG("yes", "no", SA_OPTION_COMPRESS)
			break;
		case KW_MARK:
			if (!mark_from_string(value, &conn->mark_in))
			{
				cfg->err++;
				break;
			}
			conn->mark_out = conn->mark_in;
			break;
		case KW_MARK_IN:
			if (!mark_from_string(value, &conn->mark_in))
			{
				cfg->err++;
			}
			break;
		case KW_MARK_OUT:
			if (!mark_from_string(value, &conn->mark_out))
			{
				cfg->err++;
			}
			break;
		case KW_TFC:
			if (streq(value, "%mtu"))
			{
				conn->tfc = -1;
			}
			else
			{
				char *endptr;

				conn->tfc = strtoul(value, &endptr, 10);
				if (*endptr != '\0')
				{
					DBG1(DBG_APP, "# bad integer value: %s=%s", key, value);
					cfg->err++;
				}
			}
			break;
		case KW_KEYINGTRIES:
			if (streq(value, "%forever"))
			{
				conn->sa_keying_tries = 0;
			}
			else
			{
				char *endptr;

				conn->sa_keying_tries = strtoul(value, &endptr, 10);
				if (*endptr != '\0')
				{
					DBG1(DBG_APP, "# bad integer value: %s=%s", key, value);
					cfg->err++;
				}
			}
			break;
		case KW_REKEY:
			KW_SA_OPTION_FLAG("no", "yes", SA_OPTION_DONT_REKEY)
			break;
		case KW_REAUTH:
			KW_SA_OPTION_FLAG("no", "yes", SA_OPTION_DONT_REAUTH)
			break;
		case KW_MOBIKE:
			KW_SA_OPTION_FLAG("yes", "no", SA_OPTION_MOBIKE)
			break;
		case KW_FORCEENCAPS:
			KW_SA_OPTION_FLAG("yes", "no", SA_OPTION_FORCE_ENCAP)
			break;
		case KW_MODECONFIG:
			KW_SA_OPTION_FLAG("push", "pull", SA_OPTION_MODECFG_PUSH)
			break;
		case KW_XAUTH:
			KW_SA_OPTION_FLAG("server", "client", SA_OPTION_XAUTH_SERVER)
			break;
		default:
			break;
	}
}

/*
 * handles left|rightfirewall and left|rightupdown parameters
 */
static void handle_firewall(const char *label, starter_end_t *end,
							starter_config_t *cfg)
{
	if (end->firewall)
	{
		if (end->updown != NULL)
		{
			DBG1(DBG_APP, "# cannot have both %sfirewall and %supdown", label,
				 label);
			cfg->err++;
		}
		else
		{
			end->updown = strdupnull(firewall_defaults);
			end->firewall = FALSE;
		}
	}
}

/*
 * parse a conn section
 */
static void load_conn(starter_conn_t *conn, starter_config_t *cfg,
					  conf_parser_t *parser)
{
	enumerator_t *enumerator;
	dictionary_t *dict;
	kw_entry_t *entry;
	kw_token_t token;
	char *key, *value;

	DBG2(DBG_APP, "Loading conn '%s'", conn->name);
	dict = parser->get_section(parser, CONF_PARSER_CONN, conn->name);
	if (!dict)
	{
		return;
	}
	enumerator = dict->create_enumerator(dict);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		bool assigned = FALSE;

		entry = in_word_set(key, strlen(key));
		if (!entry)
		{
			DBG1(DBG_APP, "# unknown keyword '%s'", key);
			cfg->non_fatal_err++;
			continue;
		}
		token = entry->token;
		if (token >= KW_LEFT_FIRST && token <= KW_LEFT_LAST)
		{
			kw_end(conn, &conn->left, token - KW_LEFT_FIRST + KW_END_FIRST,
				   key, value, cfg);
			continue;
		}
		else if (token >= KW_RIGHT_FIRST && token <= KW_RIGHT_LAST)
		{
			kw_end(conn, &conn->right, token - KW_RIGHT_FIRST + KW_END_FIRST,
				   key, value, cfg);
			continue;
		}
		if (token == KW_AUTO)
		{
			token = KW_CONN_SETUP;
		}
		if (token < KW_CONN_FIRST || token > KW_CONN_LAST)
		{
			DBG1(DBG_APP, "# unsupported keyword '%s' in conn '%s'",
				 key, conn->name);
			cfg->err++;
			continue;
		}
		if (is_deprecated(token, key, conn->name))
		{
			cfg->non_fatal_err++;
			continue;
		}
		if (!assign_arg(token, KW_CONN_FIRST, key, value, conn,
						&assigned))
		{
			DBG1(DBG_APP, "  bad argument value in conn '%s'", conn->name);
			cfg->err++;
			continue;
		}
		if (!assigned)
		{
			handle_keyword(token, conn, key, value, cfg);
		}
	}
	enumerator->destroy(enumerator);
	dict->destroy(dict);

	handle_firewall("left", &conn->left, cfg);
	handle_firewall("right", &conn->right, cfg);

	if (!conn->esp && !conn->ah)
	{
		conn->esp = strdupnull(esp_defaults);
	}
}

/*
 * free the memory used by a starter_ca_t object
 */
static void confread_free_ca(starter_ca_t *ca)
{
	free_args(KW_CA_NAME, KW_CA_LAST, (char *)ca);
	free(ca);
}

/*
 * free the memory used by a starter_conn_t object
 */
static void confread_free_conn(starter_conn_t *conn)
{
	free_args(KW_END_FIRST, KW_END_LAST,  (char *)&conn->left);
	free_args(KW_END_FIRST, KW_END_LAST,  (char *)&conn->right);
	free_args(KW_CONN_NAME, KW_CONN_LAST, (char *)conn);
	free(conn);
}

/*
 * free the memory used by a starter_config_t object
 */
void confread_free(starter_config_t *cfg)
{
	starter_conn_t *conn = cfg->conn_first;
	starter_ca_t   *ca   = cfg->ca_first;

	free_args(KW_SETUP_FIRST, KW_SETUP_LAST, (char *)cfg);

	while (conn != NULL)
	{
		starter_conn_t *conn_aux = conn;

		conn = conn->next;
		confread_free_conn(conn_aux);
	}

	while (ca != NULL)
	{
		starter_ca_t *ca_aux = ca;

		ca = ca->next;
		confread_free_ca(ca_aux);
	}

	free(cfg);
}

/*
 * load and parse an IPsec configuration file
 */
starter_config_t* confread_load(const char *file)
{
	conf_parser_t *parser;
	starter_config_t *cfg = NULL;
	enumerator_t *enumerator;
	u_int total_err;
	char *name;

	parser = conf_parser_create(file);
	if (!parser->parse(parser))
	{
		parser->destroy(parser);
		return NULL;
	}

	INIT(cfg,
		.setup = {
			.uniqueids = TRUE,

		}
	);
#ifdef START_CHARON
	cfg->setup.charonstart = TRUE;
#endif

	/* load config setup section */
	load_setup(cfg, parser);

	/* load ca sections */
	enumerator = parser->get_sections(parser, CONF_PARSER_CA);
	while (enumerator->enumerate(enumerator, &name))
	{
		u_int previous_err = cfg->err;
		starter_ca_t *ca;

		INIT(ca,
			.name = strdup(name),
		);
		load_ca(ca, cfg, parser);

		if (cfg->err > previous_err)
		{
			total_err = cfg->err - previous_err;
			DBG1(DBG_APP, "# ignored ca '%s' due to %d parsing error%s", name,
				 total_err, (total_err > 1) ? "s" : "");
			confread_free_ca(ca);
			cfg->non_fatal_err += cfg->err - previous_err;
			cfg->err = previous_err;
		}
		else
		{
			if (cfg->ca_last)
			{
				cfg->ca_last->next = ca;
			}
			cfg->ca_last = ca;
			if (!cfg->ca_first)
			{
				cfg->ca_first = ca;
			}
			if (ca->startup != STARTUP_NO)
			{
				ca->state = STATE_TO_ADD;
			}
		}
	}
	enumerator->destroy(enumerator);

	/* load conn sections */
	enumerator = parser->get_sections(parser, CONF_PARSER_CONN);
	while (enumerator->enumerate(enumerator, &name))
	{
		u_int previous_err = cfg->err;
		starter_conn_t *conn;

		INIT(conn,
			.name = strdup(name),
		);
		conn_defaults(conn);
		load_conn(conn, cfg, parser);

		if (cfg->err > previous_err)
		{
			total_err = cfg->err - previous_err;
			DBG1(DBG_APP, "# ignored conn '%s' due to %d parsing error%s", name,
				 total_err, (total_err > 1) ? "s" : "");
			confread_free_conn(conn);
			cfg->non_fatal_err += cfg->err - previous_err;
			cfg->err = previous_err;
		}
		else
		{
			if (cfg->conn_last)
			{
				cfg->conn_last->next = conn;
			}
			cfg->conn_last = conn;
			if (!cfg->conn_first)
			{
				cfg->conn_first = conn;
			}
			if (conn->startup != STARTUP_NO)
			{
				conn->state = STATE_TO_ADD;
			}
		}
	}
	enumerator->destroy(enumerator);

	parser->destroy(parser);

	total_err = cfg->err + cfg->non_fatal_err;
	if (total_err > 0)
	{
		DBG1(DBG_APP, "### %d parsing error%s (%d fatal) ###",
			 total_err, (total_err > 1)?"s":"", cfg->err);
	}
	return cfg;
}
