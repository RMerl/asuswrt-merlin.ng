/*
 * Copyright (C) 2007-2015 Tobias Brunner
 * Copyright (C) 2006 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <credentials/auth_cfg.h>

#include <library.h>
#include <utils/debug.h>

#include <stroke_msg.h>

#include "starterstroke.h"
#include "confread.h"
#include "files.h"

#define IPV4_LEN	 4
#define IPV6_LEN	16

static stroke_msg_t *create_stroke_msg(int type)
{
	stroke_msg_t *msg;

	INIT(msg,
		.type = type,
		.length = offsetof(stroke_msg_t, buffer),
	);
	return msg;
}

#define push_string(msg, field, str) \
	push_string_impl(msg, offsetof(stroke_msg_t, field), str)
#define push_string_end(msg, offset, field, str) \
	push_string_impl(msg, offset + offsetof(stroke_end_t, field), str)

static void push_string_impl(stroke_msg_t **msg, size_t offset, char *string)
{
	size_t cur_len = (*msg)->length, str_len;

	if (!string)
	{
		return;
	}
	str_len = strlen(string) + 1;
	if (cur_len + str_len >= UINT16_MAX)
	{
		(*msg)->length = UINT16_MAX;
		return;
	}
	while (cur_len + str_len > sizeof(stroke_msg_t) + (*msg)->buflen)
	{
		*msg = realloc(*msg, sizeof(stroke_msg_t) + (*msg)->buflen +
					   STROKE_BUF_LEN_INC);
		(*msg)->buflen += STROKE_BUF_LEN_INC;
	}
	(*msg)->length += str_len;
	strcpy((char*)*msg + cur_len, string);
	*(char**)((char*)*msg + offset) = (char*)cur_len;
}

static int send_stroke_msg(stroke_msg_t *msg)
{
	stream_t *stream;
	char *uri, buffer[64];
	int count;

	if (msg->length == UINT16_MAX)
	{
		DBG1(DBG_APP, "stroke message exceeds maximum buffer size");
		free(msg);
		return -1;
	}

	/* starter is not called from commandline, and therefore absolutely silent */
	msg->output_verbosity = -1;

	uri = lib->settings->get_str(lib->settings, "%s.plugins.stroke.socket",
								 "unix://" CHARON_CTL_FILE, daemon_name);
	stream = lib->streams->connect(lib->streams, uri);
	if (!stream)
	{
		DBG1(DBG_APP, "failed to connect to stroke socket '%s'", uri);
		free(msg);
		return -1;
	}

	if (!stream->write_all(stream, msg, msg->length))
	{
		DBG1(DBG_APP, "sending stroke message failed");
		stream->destroy(stream);
		free(msg);
		return -1;
	}
	while ((count = stream->read(stream, buffer, sizeof(buffer)-1, TRUE)) > 0)
	{
		buffer[count] = '\0';
		DBG1(DBG_APP, "%s", buffer);
	}
	if (count < 0)
	{
		DBG1(DBG_APP, "reading stroke response failed");
	}
	stream->destroy(stream);
	free(msg);
	return 0;
}

static char* connection_name(starter_conn_t *conn)
{
	 /* if connection name is '%auto', create a new name like conn_xxxxx */
	static char buf[32];

	if (streq(conn->name, "%auto"))
	{
		sprintf(buf, "conn_%lu", conn->id);
		return buf;
	}
	return conn->name;
}

static void add_end(stroke_msg_t **msg, size_t offset, starter_end_t *conn_end)
{
	stroke_end_t *msg_end;

	push_string_end(msg, offset, auth, conn_end->auth);
	push_string_end(msg, offset, auth2, conn_end->auth2);
	push_string_end(msg, offset, id, conn_end->id);
	push_string_end(msg, offset, id2, conn_end->id2);
	push_string_end(msg, offset, rsakey, conn_end->rsakey);
	push_string_end(msg, offset, cert, conn_end->cert);
	push_string_end(msg, offset, cert2, conn_end->cert2);
	push_string_end(msg, offset, cert_policy, conn_end->cert_policy);
	push_string_end(msg, offset, ca, conn_end->ca);
	push_string_end(msg, offset, ca2, conn_end->ca2);
	push_string_end(msg, offset, groups, conn_end->groups);
	push_string_end(msg, offset, groups2, conn_end->groups2);
	push_string_end(msg, offset, updown, conn_end->updown);
	if (conn_end->host)
	{
		push_string_end(msg, offset, address, conn_end->host);
	}
	else
	{
		push_string_end(msg, offset, address, "%any");
	}
	push_string_end(msg, offset, subnets, conn_end->subnet);
	push_string_end(msg, offset, sourceip, conn_end->sourceip);
	push_string_end(msg, offset, dns, conn_end->dns);

	/* we can't assign it earlier as msg might change */
	msg_end = (stroke_end_t*)((char*)(*msg) + offset);
	msg_end->ikeport = conn_end->ikeport;
	msg_end->sendcert = conn_end->sendcert;
	msg_end->hostaccess = conn_end->hostaccess;
	msg_end->tohost = !conn_end->subnet;
	msg_end->allow_any = conn_end->allow_any;
	msg_end->protocol = conn_end->protocol;
	msg_end->from_port = conn_end->from_port;
	msg_end->to_port = conn_end->to_port;
}

int starter_stroke_add_conn(starter_config_t *cfg, starter_conn_t *conn)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_ADD_CONN);
	msg->add_conn.version = conn->keyexchange;
	push_string(&msg, add_conn.name, connection_name(conn));
	push_string(&msg, add_conn.eap_identity, conn->eap_identity);
	push_string(&msg, add_conn.aaa_identity, conn->aaa_identity);
	push_string(&msg, add_conn.xauth_identity, conn->xauth_identity);

	msg->add_conn.mode = conn->mode;
	msg->add_conn.proxy_mode = conn->proxy_mode;

	if (!(conn->options & SA_OPTION_DONT_REKEY))
	{
		msg->add_conn.rekey.reauth = !(conn->options & SA_OPTION_DONT_REAUTH);
		msg->add_conn.rekey.ipsec_lifetime = conn->sa_ipsec_life_seconds;
		msg->add_conn.rekey.ike_lifetime = conn->sa_ike_life_seconds;
		msg->add_conn.rekey.margin = conn->sa_rekey_margin;
		msg->add_conn.rekey.life_bytes = conn->sa_ipsec_life_bytes;
		msg->add_conn.rekey.margin_bytes = conn->sa_ipsec_margin_bytes;
		msg->add_conn.rekey.life_packets = conn->sa_ipsec_life_packets;
		msg->add_conn.rekey.margin_packets = conn->sa_ipsec_margin_packets;
		msg->add_conn.rekey.fuzz = conn->sa_rekey_fuzz;
	}
	msg->add_conn.rekey.tries = conn->sa_keying_tries;

	msg->add_conn.mobike = conn->options & SA_OPTION_MOBIKE;
	msg->add_conn.force_encap = conn->options & SA_OPTION_FORCE_ENCAP;
	msg->add_conn.fragmentation = conn->fragmentation;
	msg->add_conn.ikedscp = conn->ikedscp;
	msg->add_conn.ipcomp = conn->options & SA_OPTION_COMPRESS;
	msg->add_conn.install_policy = conn->install_policy;
	msg->add_conn.aggressive = conn->aggressive;
	msg->add_conn.pushmode = conn->options & SA_OPTION_MODECFG_PUSH;
	msg->add_conn.crl_policy = (crl_policy_t)cfg->setup.strictcrlpolicy;
	msg->add_conn.unique = cfg->setup.uniqueids;
	push_string(&msg, add_conn.algorithms.ike, conn->ike);
	push_string(&msg, add_conn.algorithms.esp, conn->esp);
	push_string(&msg, add_conn.algorithms.ah, conn->ah);
	msg->add_conn.dpd.delay = conn->dpd_delay;
	msg->add_conn.dpd.timeout = conn->dpd_timeout;
	msg->add_conn.dpd.action = conn->dpd_action;
	msg->add_conn.close_action = conn->close_action;
	msg->add_conn.sha256_96 = conn->sha256_96;
	msg->add_conn.inactivity = conn->inactivity;
	msg->add_conn.ikeme.mediation = conn->me_mediation;
	push_string(&msg, add_conn.ikeme.mediated_by, conn->me_mediated_by);
	push_string(&msg, add_conn.ikeme.peerid, conn->me_peerid);
	msg->add_conn.reqid = conn->reqid;
	msg->add_conn.replay_window = conn->replay_window;
	msg->add_conn.mark_in.value = conn->mark_in.value;
	msg->add_conn.mark_in.mask = conn->mark_in.mask;
	msg->add_conn.mark_out.value = conn->mark_out.value;
	msg->add_conn.mark_out.mask = conn->mark_out.mask;
	msg->add_conn.tfc = conn->tfc;

	add_end(&msg, offsetof(stroke_msg_t, add_conn.me), &conn->left);
	add_end(&msg, offsetof(stroke_msg_t, add_conn.other), &conn->right);

	if (!msg->add_conn.me.auth && !msg->add_conn.other.auth &&
		 conn->authby)
	{	/* leftauth/rightauth not set, use legacy options */
		if (streq(conn->authby, "rsa")   || streq(conn->authby, "rsasig")   ||
			streq(conn->authby, "ecdsa") || streq(conn->authby, "ecdsasig") ||
			streq(conn->authby, "pubkey"))
		{
			push_string(&msg, add_conn.me.auth, "pubkey");
			push_string(&msg, add_conn.other.auth, "pubkey");
		}
		else if (streq(conn->authby, "secret") || streq(conn->authby, "psk"))
		{
			push_string(&msg, add_conn.me.auth, "psk");
			push_string(&msg, add_conn.other.auth, "psk");
		}
		else if (streq(conn->authby, "xauthrsasig"))
		{
			push_string(&msg, add_conn.me.auth, "pubkey");
			push_string(&msg, add_conn.other.auth, "pubkey");
			if (conn->options & SA_OPTION_XAUTH_SERVER)
			{
				push_string(&msg, add_conn.other.auth2, "xauth");
			}
			else
			{
				push_string(&msg, add_conn.me.auth2, "xauth");
			}
		}
		else if (streq(conn->authby, "xauthpsk"))
		{
			push_string(&msg, add_conn.me.auth, "psk");
			push_string(&msg, add_conn.other.auth, "psk");
			if (conn->options & SA_OPTION_XAUTH_SERVER)
			{
				push_string(&msg, add_conn.other.auth2, "xauth");
			}
			else
			{
				push_string(&msg, add_conn.me.auth2, "xauth");
			}
		}
	}
	return send_stroke_msg(msg);
}

int starter_stroke_del_conn(starter_conn_t *conn)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_DEL_CONN);
	push_string(&msg, del_conn.name, connection_name(conn));
	return send_stroke_msg(msg);
}

int starter_stroke_route_conn(starter_conn_t *conn)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_ROUTE);
	push_string(&msg, route.name, connection_name(conn));
	return send_stroke_msg(msg);
}

int starter_stroke_unroute_conn(starter_conn_t *conn)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_UNROUTE);
	push_string(&msg, route.name, connection_name(conn));
	return send_stroke_msg(msg);
}

int starter_stroke_initiate_conn(starter_conn_t *conn)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_INITIATE);
	push_string(&msg, initiate.name, connection_name(conn));
	return send_stroke_msg(msg);
}

int starter_stroke_add_ca(starter_ca_t *ca)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_ADD_CA);
	push_string(&msg, add_ca.name, ca->name);
	push_string(&msg, add_ca.cacert, ca->cacert);
	push_string(&msg, add_ca.crluri, ca->crluri);
	push_string(&msg, add_ca.crluri2, ca->crluri2);
	push_string(&msg, add_ca.ocspuri, ca->ocspuri);
	push_string(&msg, add_ca.ocspuri2, ca->ocspuri2);
	push_string(&msg, add_ca.certuribase, ca->certuribase);
	return send_stroke_msg(msg);
}

int starter_stroke_del_ca(starter_ca_t *ca)
{
	stroke_msg_t *msg;

	msg = create_stroke_msg(STR_DEL_CA);
	push_string(&msg, del_ca.name, ca->name);
	return send_stroke_msg(msg);
}

int starter_stroke_configure(starter_config_t *cfg)
{
	stroke_msg_t *msg;

	if (cfg->setup.cachecrls)
	{
		msg = create_stroke_msg(STR_CONFIG);
		msg->config.cachecrl = 1;
		return send_stroke_msg(msg);
	}
	return 0;
}
