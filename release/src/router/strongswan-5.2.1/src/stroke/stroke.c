/*
 * Copyright (C) 2007-2014 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <library.h>

#include "stroke_msg.h"
#include "stroke_keywords.h"

struct stroke_token {
    char *name;
    stroke_keyword_t kw;
};

static char *daemon_name = "charon";
static int output_verbosity = 1; /* CONTROL */

static char* push_string(stroke_msg_t *msg, char *string)
{
	unsigned long string_start = msg->length;

	if (string == NULL ||  msg->length + strlen(string) >= sizeof(stroke_msg_t))
	{
		return NULL;
	}
	else
	{
		msg->length += strlen(string) + 1;
		strcpy((char*)msg + string_start, string);
		return (char*)string_start;
	}
}

static int send_stroke_msg (stroke_msg_t *msg)
{
	stream_t *stream;
	char *uri, buffer[512], *pass;
	int count;

	msg->output_verbosity = output_verbosity;

	uri = lib->settings->get_str(lib->settings, "%s.plugins.stroke.socket",
								 "unix://" STROKE_SOCKET, daemon_name);
	stream = lib->streams->connect(lib->streams, uri);
	if (!stream)
	{
		fprintf(stderr, "failed to connect to stroke socket '%s'\n", uri);
		return -1;
	}

	if (!stream->write_all(stream, msg, msg->length))
	{
		fprintf(stderr, "sending stroke message failed\n");
		stream->destroy(stream);
		return -1;
	}

	while ((count = stream->read(stream, buffer, sizeof(buffer)-1, TRUE)) > 0)
	{
		buffer[count] = '\0';

		/* we prompt if we receive a magic keyword */
		if ((count >= 12 && streq(buffer + count - 12, "Passphrase:\n")) ||
			(count >= 10 && streq(buffer + count - 10, "Password:\n")) ||
			(count >=  5 && streq(buffer + count -  5, "PIN:\n")))
		{
			/* remove trailing newline */
			pass = strrchr(buffer, '\n');
			if (pass)
			{
				*pass = ' ';
			}
#ifdef HAVE_GETPASS
			pass = getpass(buffer);
#else
			pass = "";
#endif
			if (pass)
			{
				stream->write_all(stream, pass, strlen(pass));
				stream->write_all(stream, "\n", 1);
			}
		}
		else
		{
			printf("%s", buffer);
		}
	}
	if (count < 0)
	{
		fprintf(stderr, "reading stroke response failed\n");
	}
	stream->destroy(stream);
	return 0;
}

static int add_connection(char *name,
						  char *my_id, char *other_id,
						  char *my_addr, char *other_addr,
						  char *my_nets, char *other_nets)
{
	stroke_msg_t msg;

	memset(&msg, 0, sizeof(msg));
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.type = STR_ADD_CONN;

	msg.add_conn.name = push_string(&msg, name);
	msg.add_conn.version = 2;
	msg.add_conn.mode = 1;
	msg.add_conn.mobike = 1;
	msg.add_conn.dpd.action = 1;
	msg.add_conn.install_policy = 1;

	msg.add_conn.me.id = push_string(&msg, my_id);
	msg.add_conn.me.address = push_string(&msg, my_addr);
	msg.add_conn.me.ikeport = 500;
	msg.add_conn.me.subnets = push_string(&msg, my_nets);
	msg.add_conn.me.sendcert = 1;
	msg.add_conn.me.to_port = 65535;

	msg.add_conn.other.id = push_string(&msg, other_id);
	msg.add_conn.other.address = push_string(&msg, other_addr);
	msg.add_conn.other.ikeport = 500;
	msg.add_conn.other.subnets = push_string(&msg, other_nets);
	msg.add_conn.other.sendcert = 1;
	msg.add_conn.other.to_port = 65535;

	return send_stroke_msg(&msg);
}

static int del_connection(char *name)
{
	stroke_msg_t msg;

	msg.length = offsetof(stroke_msg_t, buffer);
	msg.type = STR_DEL_CONN;
	msg.initiate.name = push_string(&msg, name);
	return send_stroke_msg(&msg);
}

static int initiate_connection(char *name)
{
	stroke_msg_t msg;

	msg.length = offsetof(stroke_msg_t, buffer);
	msg.type = STR_INITIATE;
	msg.initiate.name = push_string(&msg, name);
	return send_stroke_msg(&msg);
}

static int terminate_connection(char *name)
{
	stroke_msg_t msg;

	msg.type = STR_TERMINATE;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.initiate.name = push_string(&msg, name);
	return send_stroke_msg(&msg);
}

static int terminate_connection_srcip(char *start, char *end)
{
	stroke_msg_t msg;

	msg.type = STR_TERMINATE_SRCIP;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.terminate_srcip.start = push_string(&msg, start);
	msg.terminate_srcip.end = push_string(&msg, end);
	return send_stroke_msg(&msg);
}

static int rekey_connection(char *name)
{
	stroke_msg_t msg;

	msg.type = STR_REKEY;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.rekey.name = push_string(&msg, name);
	return send_stroke_msg(&msg);
}

static int route_connection(char *name)
{
	stroke_msg_t msg;

	msg.type = STR_ROUTE;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.route.name = push_string(&msg, name);
	return send_stroke_msg(&msg);
}

static int unroute_connection(char *name)
{
	stroke_msg_t msg;

	msg.type = STR_UNROUTE;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.unroute.name = push_string(&msg, name);
	return send_stroke_msg(&msg);
}

static int show_status(stroke_keyword_t kw, char *connection)
{
	stroke_msg_t msg;

	switch (kw)
	{
		case STROKE_STATUSALL:
			msg.type = STR_STATUS_ALL;
			break;
		case STROKE_STATUSALL_NOBLK:
			msg.type = STR_STATUS_ALL_NOBLK;
			break;
		default:
			msg.type = STR_STATUS;
			break;
	}
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.status.name = push_string(&msg, connection);
	return send_stroke_msg(&msg);
}

static int list_flags[] = {
	LIST_PUBKEYS,
	LIST_CERTS,
	LIST_CACERTS,
	LIST_OCSPCERTS,
	LIST_AACERTS,
	LIST_ACERTS,
	LIST_GROUPS,
	LIST_CAINFOS,
	LIST_CRLS,
	LIST_OCSP,
	LIST_ALGS,
	LIST_PLUGINS,
	LIST_ALL
};

static int list(stroke_keyword_t kw, int utc)
{
	stroke_msg_t msg;

	msg.type = STR_LIST;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.list.utc = utc;
	msg.list.flags = list_flags[kw - STROKE_LIST_FIRST];
	return send_stroke_msg(&msg);
}

static int reread_flags[] = {
	REREAD_SECRETS,
	REREAD_CACERTS,
	REREAD_OCSPCERTS,
	REREAD_AACERTS,
	REREAD_ACERTS,
	REREAD_CRLS,
	REREAD_ALL
};

static int reread(stroke_keyword_t kw)
{
	stroke_msg_t msg;

	msg.type = STR_REREAD;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.reread.flags = reread_flags[kw - STROKE_REREAD_FIRST];
	return send_stroke_msg(&msg);
}

static int purge_flags[] = {
	PURGE_OCSP,
	PURGE_CRLS,
	PURGE_CERTS,
	PURGE_IKE,
};

static int purge(stroke_keyword_t kw)
{
	stroke_msg_t msg;

	msg.type = STR_PURGE;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.purge.flags = purge_flags[kw - STROKE_PURGE_FIRST];
	return send_stroke_msg(&msg);
}

static int export_flags[] = {
	EXPORT_X509,
	EXPORT_CONN_CERT,
	EXPORT_CONN_CHAIN,
};

static int export(stroke_keyword_t kw, char *selector)
{
	stroke_msg_t msg;

	msg.type = STR_EXPORT;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.export.selector = push_string(&msg, selector);
	msg.export.flags = export_flags[kw - STROKE_EXPORT_FIRST];
	return send_stroke_msg(&msg);
}

static int leases(stroke_keyword_t kw, char *pool, char *address)
{
	stroke_msg_t msg;

	msg.type = STR_LEASES;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.leases.pool = push_string(&msg, pool);
	msg.leases.address = push_string(&msg, address);
	return send_stroke_msg(&msg);
}

static int memusage()
{
	stroke_msg_t msg;

	msg.type = STR_MEMUSAGE;
	msg.length = offsetof(stroke_msg_t, buffer);
	return send_stroke_msg(&msg);
}

static int user_credentials(char *name, char *user, char *pass)
{
	stroke_msg_t msg;

	msg.type = STR_USER_CREDS;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.user_creds.name = push_string(&msg, name);
	msg.user_creds.username = push_string(&msg, user);
	msg.user_creds.password = push_string(&msg, pass);
	return send_stroke_msg(&msg);
}

static int counters(int reset, char *name)
{
	stroke_msg_t msg;

	msg.type = STR_COUNTERS;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.counters.name = push_string(&msg, name);
	msg.counters.reset = reset;

	return send_stroke_msg(&msg);
}

static int set_loglevel(char *type, u_int level)
{
	stroke_msg_t msg;

	msg.type = STR_LOGLEVEL;
	msg.length = offsetof(stroke_msg_t, buffer);
	msg.loglevel.type = push_string(&msg, type);
	msg.loglevel.level = level;
	return send_stroke_msg(&msg);
}

static int usage(char *error)
{
	FILE *out = error ? stderr : stdout;

	fprintf(out, "stroke [OPTIONS] command [ARGUMENTS]\n\n");
	fprintf(out, "Options:\n");
	fprintf(out, "  -h, --help             print this information.\n");
	fprintf(out, "  -d, --daemon=NAME      name of the daemon.\n");
	fprintf(out, "Commands:\n");
	fprintf(out, "  Add a connection:\n");
	fprintf(out, "    stroke add NAME MY_ID OTHER_ID MY_ADDR OTHER_ADDR\\\n");
	fprintf(out, "           MY_NET OTHER_NET\n");
	fprintf(out, "    where: ID is any IKEv2 ID \n");
	fprintf(out, "           ADDR is a IPv4 address\n");
	fprintf(out, "           NET is a IPv4 subnet in CIDR notation\n");
	fprintf(out, "  Delete a connection:\n");
	fprintf(out, "    stroke delete NAME\n");
	fprintf(out, "    where: NAME is a connection name added with \"stroke add\"\n");
	fprintf(out, "  Initiate a connection:\n");
	fprintf(out, "    stroke up NAME\n");
	fprintf(out, "    where: NAME is a connection name added with \"stroke add\"\n");
	fprintf(out, "  Initiate a connection without blocking:\n");
	fprintf(out, "    stroke up-nb NAME\n");
	fprintf(out, "    where: NAME is a connection name added with \"stroke add\"\n");
	fprintf(out, "  Terminate a connection:\n");
	fprintf(out, "    stroke down NAME\n");
	fprintf(out, "    where: NAME is a connection name added with \"stroke add\"\n");
	fprintf(out, "  Terminate a connection without blocking:\n");
	fprintf(out, "    stroke down-nb NAME\n");
	fprintf(out, "    where: NAME is a connection name added with \"stroke add\"\n");
	fprintf(out, "  Terminate a connection by remote srcip:\n");
	fprintf(out, "    stroke down-srcip START [END]\n");
	fprintf(out, "    where: START and optional END define the clients source IP\n");
	fprintf(out, "  Set loglevel for a logging type:\n");
	fprintf(out, "    stroke loglevel TYPE LEVEL\n");
	fprintf(out, "    where: TYPE is any|dmn|mgr|ike|chd|job|cfg|knl|net|asn|enc|tnc|imc|imv|pts|tls|esp|lib\n");
	fprintf(out, "           LEVEL is -1|0|1|2|3|4\n");
	fprintf(out, "  Show connection status:\n");
	fprintf(out, "    stroke status\n");
	fprintf(out, "  Show extended status information:\n");
	fprintf(out, "    stroke statusall\n");
	fprintf(out, "  Show extended status information without blocking:\n");
	fprintf(out, "    stroke statusall-nb\n");
	fprintf(out, "  Show list of authority and attribute certificates:\n");
	fprintf(out, "    stroke listcacerts|listocspcerts|listaacerts|listacerts\n");
	fprintf(out, "  Show list of end entity certificates, ca info records  and crls:\n");
	fprintf(out, "    stroke listcerts|listcainfos|listcrls|listall\n");
	fprintf(out, "  Show list of supported algorithms:\n");
	fprintf(out, "    stroke listalgs\n");
	fprintf(out, "  Reload authority and attribute certificates:\n");
	fprintf(out, "    stroke rereadcacerts|rereadocspcerts|rereadaacerts|rereadacerts\n");
	fprintf(out, "  Reload secrets and crls:\n");
	fprintf(out, "    stroke rereadsecrets|rereadcrls|rereadall\n");
	fprintf(out, "  Purge ocsp cache entries:\n");
	fprintf(out, "    stroke purgeocsp\n");
	fprintf(out, "  Purge CRL cache entries:\n");
	fprintf(out, "    stroke purgecrls\n");
	fprintf(out, "  Purge X509 cache entries:\n");
	fprintf(out, "    stroke purgecerts\n");
	fprintf(out, "  Purge IKE_SAs without a CHILD_SA:\n");
	fprintf(out, "    stroke purgeike\n");
	fprintf(out, "  Export credentials to the console:\n");
	fprintf(out, "    stroke exportx509 DN\n");
	fprintf(out, "    stroke exportconncert connname\n");
	fprintf(out, "    stroke exportconnchain connname\n");
	fprintf(out, "  Show current memory usage:\n");
	fprintf(out, "    stroke memusage\n");
	fprintf(out, "  Show leases of a pool:\n");
	fprintf(out, "    stroke leases [POOL [ADDRESS]]\n");
	fprintf(out, "  Set username and password for a connection:\n");
	fprintf(out, "    stroke user-creds NAME USERNAME [PASSWORD]\n");
	fprintf(out, "    where: NAME is a connection name added with \"stroke add\"\n");
	fprintf(out, "           USERNAME is the username\n");
	fprintf(out, "           PASSWORD is the optional password, you'll be asked to enter it if not given\n");
	fprintf(out, "  Show IKE counters:\n");
	fprintf(out, "    stroke listcounters [connection-name]\n");

	if (error)
	{
		fprintf(out, "\nError: %s\n", error);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	const stroke_token_t *token;
	char *cmd;
	int res = 0;

	library_init(NULL, "stroke");
	atexit(library_deinit);

	while (true)
	{
		struct option long_opts[] = {
			{"help",		no_argument,		NULL,	'h' },
			{"daemon",		required_argument,	NULL,	'd' },
			{0,0,0,0},
		};
		switch (getopt_long(argc, argv, "hd:", long_opts, NULL))
		{
			case EOF:
				break;
			case 'h':
				return usage(NULL);
			case 'd':
				daemon_name = optarg;
				continue;
			default:
				return usage("invalid option");
		}
		break;
	}

	if (optind == argc)
	{
		return usage("command missing");
	}

	cmd = argv[optind++];
	token = in_word_set(cmd, strlen(cmd));
	if (token == NULL)
	{
		return usage("unknown command");
	}

	/* make argv/argc only cover positional arguments */
	argv = &argv[optind];
	argc = argc - optind;

	switch (token->kw)
	{
		case STROKE_ADD:
			if (argc < 7)
			{
				return usage("\"add\" needs more arguments...");
			}
			res = add_connection(argv[0], argv[1], argv[2], argv[3], argv[4],
								 argv[5], argv[6]);
			break;
		case STROKE_DELETE:
		case STROKE_DEL:
			if (argc < 1)
			{
				return usage("\"delete\" needs a connection name");
			}
			res = del_connection(argv[0]);
			break;
		case STROKE_UP_NOBLK:
			output_verbosity = -1;
			/* fall-through */
		case STROKE_UP:
			if (argc < 1)
			{
				return usage("\"up\" needs a connection name");
			}
			res = initiate_connection(argv[0]);
			break;
		case STROKE_DOWN_NOBLK:
			output_verbosity = -1;
			/* fall-through */
		case STROKE_DOWN:
			if (argc < 1)
			{
				return usage("\"down\" needs a connection name");
			}
			res = terminate_connection(argv[0]);
			break;
		case STROKE_DOWN_SRCIP:
			if (argc < 1)
			{
				return usage("\"down-srcip\" needs start and optional end address");
			}
			res = terminate_connection_srcip(argv[0], argc > 1 ? argv[1] : NULL);
			break;
		case STROKE_REKEY:
			if (argc < 1)
			{
				return usage("\"rekey\" needs a connection name");
			}
			res = rekey_connection(argv[0]);
			break;
		case STROKE_ROUTE:
			if (argc < 1)
			{
				return usage("\"route\" needs a connection name");
			}
			res = route_connection(argv[0]);
			break;
		case STROKE_UNROUTE:
			if (argc < 1)
			{
				return usage("\"unroute\" needs a connection name");
			}
			res = unroute_connection(argv[0]);
			break;
		case STROKE_LOGLEVEL:
			if (argc < 2)
			{
				return usage("\"logtype\" needs more parameters...");
			}
			res = set_loglevel(argv[0], atoi(argv[1]));
			break;
		case STROKE_STATUS:
		case STROKE_STATUSALL:
		case STROKE_STATUSALL_NOBLK:
			res = show_status(token->kw, argc ? argv[0] : NULL);
			break;
		case STROKE_LIST_PUBKEYS:
		case STROKE_LIST_CERTS:
		case STROKE_LIST_CACERTS:
		case STROKE_LIST_OCSPCERTS:
		case STROKE_LIST_AACERTS:
		case STROKE_LIST_ACERTS:
		case STROKE_LIST_CAINFOS:
		case STROKE_LIST_CRLS:
		case STROKE_LIST_OCSP:
		case STROKE_LIST_ALGS:
		case STROKE_LIST_PLUGINS:
		case STROKE_LIST_ALL:
			res = list(token->kw, argc && streq(argv[0], "--utc"));
			break;
		case STROKE_REREAD_SECRETS:
		case STROKE_REREAD_CACERTS:
		case STROKE_REREAD_OCSPCERTS:
		case STROKE_REREAD_AACERTS:
		case STROKE_REREAD_ACERTS:
		case STROKE_REREAD_CRLS:
		case STROKE_REREAD_ALL:
			res = reread(token->kw);
			break;
		case STROKE_PURGE_OCSP:
		case STROKE_PURGE_CRLS:
		case STROKE_PURGE_CERTS:
		case STROKE_PURGE_IKE:
			res = purge(token->kw);
			break;
		case STROKE_EXPORT_X509:
		case STROKE_EXPORT_CONN_CERT:
		case STROKE_EXPORT_CONN_CHAIN:
			if (argc < 1)
			{
				return usage("\"export\" needs a name");
			}
			res = export(token->kw, argv[0]);
			break;
		case STROKE_LEASES:
			res = leases(token->kw, argc ? argv[0] : NULL,
						 argc > 1 ? argv[1] : NULL);
			break;
		case STROKE_MEMUSAGE:
			res = memusage();
			break;
		case STROKE_USER_CREDS:
			if (argc < 2)
			{
				return usage("\"user-creds\" needs a connection name, "
						   "username and optionally a password");
			}
			res = user_credentials(argv[0], argv[1], argc > 2 ? argv[2] : NULL);
			break;
		case STROKE_COUNTERS:
		case STROKE_COUNTERS_RESET:
			res = counters(token->kw == STROKE_COUNTERS_RESET,
						   argc ? argv[0] : NULL);
			break;
		default:
			return usage(NULL);
	}
	return res;
}
