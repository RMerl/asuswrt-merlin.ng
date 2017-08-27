/*
 * listen.c	Handle socket stuff
 *
 * Version:	$Id$
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2005,2006  The FreeRADIUS server project
 * Copyright 2005  Alan DeKok <aland@ox.org>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>
#include <freeradius-devel/process.h>
#include <freeradius-devel/protocol.h>

#include <freeradius-devel/detail.h>

#ifdef WITH_UDPFROMTO
#include <freeradius-devel/udpfromto.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif


#ifdef DEBUG_PRINT_PACKET
static void print_packet(RADIUS_PACKET *packet)
{
	char src[256], dst[256];

	ip_ntoh(&packet->src_ipaddr, src, sizeof(src));
	ip_ntoh(&packet->dst_ipaddr, dst, sizeof(dst));

	fprintf(stderr, "ID %d: %s %d -> %s %d\n", packet->id,
		src, packet->src_port, dst, packet->dst_port);

}
#endif


static rad_listen_t *listen_alloc(TALLOC_CTX *ctx, RAD_LISTEN_TYPE type);

#ifdef WITH_COMMAND_SOCKET
static int command_tcp_recv(rad_listen_t *listener);
static int command_tcp_send(rad_listen_t *listener, REQUEST *request);
static int command_write_magic(int newfd, listen_socket_t *sock);
#endif

static fr_protocol_t master_listen[RAD_LISTEN_MAX];

/*
 *	Xlat for %{listen:foo}
 */
static ssize_t xlat_listen(UNUSED void *instance, REQUEST *request,
			   char const *fmt, char *out,
			   size_t outlen)
{
	char const *value = NULL;
	CONF_PAIR *cp;

	if (!fmt || !out || (outlen < 1)) return 0;

	if (!request || !request->listener) {
		RWDEBUG("No listener associated with this request");
		*out = '\0';
		return 0;
	}

	cp = cf_pair_find(request->listener->cs, fmt);
	if (!cp || !(value = cf_pair_value(cp))) {
		RDEBUG("Listener does not contain config item \"%s\"", fmt);
		*out = '\0';
		return 0;
	}

	strlcpy(out, value, outlen);

	return strlen(out);
}

/*
 *	Find a per-socket client.
 */
RADCLIENT *client_listener_find(rad_listen_t *listener,
				fr_ipaddr_t const *ipaddr, int src_port)
{
#ifdef WITH_DYNAMIC_CLIENTS
	int rcode;
	REQUEST *request;
	RADCLIENT *created;
#endif
	time_t now;
	RADCLIENT *client;
	RADCLIENT_LIST *clients;
	listen_socket_t *sock;

	rad_assert(listener != NULL);
	rad_assert(ipaddr != NULL);

	sock = listener->data;
	clients = sock->clients;

	/*
	 *	This HAS to have been initialized previously.
	 */
	rad_assert(clients != NULL);

	client = client_find(clients, ipaddr,sock->proto);
	if (!client) {
		char name[256], buffer[128];

#ifdef WITH_DYNAMIC_CLIENTS
	unknown:		/* used only for dynamic clients */
#endif

		/*
		 *	DoS attack quenching, but only in daemon mode.
		 *	If they're running in debug mode, show them
		 *	every packet.
		 */
		if (debug_flag == 0) {
			static time_t last_printed = 0;

			now = time(NULL);
			if (last_printed == now) return NULL;

			last_printed = now;
		}

		listener->print(listener, name, sizeof(name));

		ERROR("Ignoring request to %s from unknown client %s port %d"
#ifdef WITH_TCP
		       " proto %s"
#endif
		       , name, inet_ntop(ipaddr->af, &ipaddr->ipaddr,
					 buffer, sizeof(buffer)), src_port
#ifdef WITH_TCP
		       , (sock->proto == IPPROTO_UDP) ? "udp" : "tcp"
#endif
		       );
		return NULL;
	}

#ifndef WITH_DYNAMIC_CLIENTS
	return client;		/* return the found client. */
#else

	/*
	 *	No server defined, and it's not dynamic.  Return it.
	 */
	if (!client->client_server && !client->dynamic) return client;

	now = time(NULL);

	/*
	 *	It's a dynamically generated client, check it.
	 */
	if (client->dynamic && (src_port != 0)) {
		/*
		 *	Lives forever.  Return it.
		 */
		if (client->lifetime == 0) return client;

		/*
		 *	Rate-limit the deletion of known clients.
		 *	This makes them last a little longer, but
		 *	prevents the server from melting down if (say)
		 *	10k clients all expire at once.
		 */
		if (now == client->last_new_client) return client;

		/*
		 *	It's not dead yet.  Return it.
		 */
		if ((client->created + client->lifetime) > now) return client;

		/*
		 *	This really puts them onto a queue for later
		 *	deletion.
		 */
		client_delete(clients, client);

		/*
		 *	Go find the enclosing network again.
		 */
		client = client_find(clients, ipaddr, sock->proto);

		/*
		 *	WTF?
		 */
		if (!client) goto unknown;
		if (!client->client_server) goto unknown;

		/*
		 *	At this point, 'client' is the enclosing
		 *	network that configures where dynamic clients
		 *	can be defined.
		 */
		rad_assert(client->dynamic == 0);

	} else if (!client->dynamic && client->rate_limit) {
		/*
		 *	The IP is unknown, so we've found an enclosing
		 *	network.  Enable DoS protection.  We only
		 *	allow one new client per second.  Known
		 *	clients aren't subject to this restriction.
		 */
		if (now == client->last_new_client) goto unknown;
	}

	client->last_new_client = now;

	request = request_alloc(NULL);
	if (!request) goto unknown;

	request->listener = listener;
	request->client = client;
	request->packet = rad_recv(listener->fd, 0x02); /* MSG_PEEK */
	if (!request->packet) {				/* badly formed, etc */
		request_free(&request);
		goto unknown;
	}
	request->reply = rad_alloc_reply(request, request->packet);
	if (!request->reply) {
		request_free(&request);
		goto unknown;
	}
	gettimeofday(&request->packet->timestamp, NULL);
	request->number = 0;
	request->priority = listener->type;
	request->server = client->client_server;
	request->root = &mainconfig;

	/*
	 *	Run a fake request through the given virtual server.
	 *	Look for FreeRADIUS-Client-IP-Address
	 *		 FreeRADIUS-Client-Secret
	 *		...
	 *
	 *	and create the RADCLIENT structure from that.
	 */
	DEBUG("server %s {", request->server);

	rcode = process_authorize(0, request);

	DEBUG("} # server %s", request->server);

	if (rcode != RLM_MODULE_OK) {
		request_free(&request);
		goto unknown;
	}

	/*
	 *	If the client was updated by rlm_dynamic_clients,
	 *	don't create the client from attribute-value pairs.
	 */
	if (request->client == client) {
		created = client_from_request(clients, request);
	} else {
		created = request->client;

		/*
		 *	This frees the client if it isn't valid.
		 */
		if (!client_validate(clients, client, created)) goto unknown;
	}

	request->server = client->server;
	exec_trigger(request, NULL, "server.client.add", false);

	request_free(&request);

	if (!created) goto unknown;

	return created;
#endif
}

static int listen_bind(rad_listen_t *this);


/*
 *	Process and reply to a server-status request.
 *	Like rad_authenticate and rad_accounting this should
 *	live in it's own file but it's so small we don't bother.
 */
int rad_status_server(REQUEST *request)
{
	int rcode = RLM_MODULE_OK;
	DICT_VALUE *dval;

	switch (request->listener->type) {
#ifdef WITH_STATS
	case RAD_LISTEN_NONE:
#endif
	case RAD_LISTEN_AUTH:
		dval = dict_valbyname(PW_AUTZ_TYPE, 0, "Status-Server");
		if (dval) {
			rcode = process_authorize(dval->value, request);
		} else {
			rcode = RLM_MODULE_OK;
		}

		switch (rcode) {
		case RLM_MODULE_OK:
		case RLM_MODULE_UPDATED:
			request->reply->code = PW_AUTHENTICATION_ACK;
			break;

		case RLM_MODULE_FAIL:
		case RLM_MODULE_HANDLED:
			request->reply->code = 0; /* don't reply */
			break;

		default:
		case RLM_MODULE_REJECT:
			request->reply->code = PW_AUTHENTICATION_REJECT;
			break;
		}
		break;

#ifdef WITH_ACCOUNTING
	case RAD_LISTEN_ACCT:
		dval = dict_valbyname(PW_ACCT_TYPE, 0, "Status-Server");
		if (dval) {
			rcode = process_accounting(dval->value, request);
		} else {
			rcode = RLM_MODULE_OK;
		}

		switch (rcode) {
		case RLM_MODULE_OK:
		case RLM_MODULE_UPDATED:
			request->reply->code = PW_ACCOUNTING_RESPONSE;
			break;

		default:
			request->reply->code = 0; /* don't reply */
			break;
		}
		break;
#endif

#ifdef WITH_COA
		/*
		 *	This is a vendor extension.  Suggested by Glen
		 *	Zorn in IETF 72, and rejected by the rest of
		 *	the WG.  We like it, so it goes in here.
		 */
	case RAD_LISTEN_COA:
		dval = dict_valbyname(PW_RECV_COA_TYPE, 0, "Status-Server");
		if (dval) {
			rcode = process_recv_coa(dval->value, request);
		} else {
			rcode = RLM_MODULE_OK;
		}

		switch (rcode) {
		case RLM_MODULE_OK:
		case RLM_MODULE_UPDATED:
			request->reply->code = PW_COA_ACK;
			break;

		default:
			request->reply->code = 0; /* don't reply */
			break;
		}
		break;
#endif

	default:
		return 0;
	}

#ifdef WITH_STATS
	/*
	 *	Full statistics are available only on a statistics
	 *	socket.
	 */
	if (request->listener->type == RAD_LISTEN_NONE) {
		request_stats_reply(request);
	}
#endif

	return 0;
}

#ifdef WITH_TCP
static int dual_tcp_recv(rad_listen_t *listener)
{
	int rcode;
	RADIUS_PACKET	*packet;
	RAD_REQUEST_FUNP fun = NULL;
	listen_socket_t *sock = listener->data;
	RADCLIENT	*client = sock->client;

	/*
	 *	Allocate a packet for partial reads.
	 */
	if (!sock->packet) {
		sock->packet = rad_alloc(NULL, 0);
		if (!sock->packet) return 0;

		sock->packet->sockfd = listener->fd;
		sock->packet->src_ipaddr = sock->other_ipaddr;
		sock->packet->src_port = sock->other_port;
		sock->packet->dst_ipaddr = sock->my_ipaddr;
		sock->packet->dst_port = sock->my_port;
	}

	/*
	 *	Grab the packet currently being processed.
	 */
	packet = sock->packet;

	rcode = fr_tcp_read_packet(packet, 0);

	/*
	 *	Still only a partial packet.  Put it back, and return,
	 *	so that we'll read more data when it's ready.
	 */
	if (rcode == 0) {
		return 0;
	}

	if (rcode == -1) {	/* error reading packet */
		char buffer[256];

		ERROR("Invalid packet from %s port %d: closing socket",
		       ip_ntoh(&packet->src_ipaddr, buffer, sizeof(buffer)),
		       packet->src_port);
	}

	if (rcode < 0) {	/* error or connection reset */
		listener->status = RAD_LISTEN_STATUS_REMOVE_NOW;

		/*
		 *	Decrement the number of connections.
		 */
		if (sock->parent->limit.num_connections > 0) {
			sock->parent->limit.num_connections--;
		}
		if (sock->client->limit.num_connections > 0) {
			sock->client->limit.num_connections--;
		}

		/*
		 *	Tell the event handler that an FD has disappeared.
		 */
		DEBUG("Client has closed connection");
		event_new_fd(listener);

		/*
		 *	Do NOT free the listener here.  It's in use by
		 *	a request, and will need to hang around until
		 *	all of the requests are done.
		 *
		 *	It is instead free'd in remove_from_request_hash()
		 */
		return 0;
	}

	/*
	 *	Some sanity checks, based on the packet code.
	 */
	switch(packet->code) {
	case PW_AUTHENTICATION_REQUEST:
		if (listener->type != RAD_LISTEN_AUTH) goto bad_packet;
		FR_STATS_INC(auth, total_requests);
		fun = rad_authenticate;
		break;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_REQUEST:
		if (listener->type != RAD_LISTEN_ACCT) goto bad_packet;
		FR_STATS_INC(acct, total_requests);
		fun = rad_accounting;
		break;
#endif

	case PW_STATUS_SERVER:
		if (!mainconfig.status_server) {
			FR_STATS_INC(auth, total_unknown_types);
			WDEBUG("Ignoring Status-Server request due to security configuration");
			rad_free(&sock->packet);
			return 0;
		}
		fun = rad_status_server;
		break;

	default:
	bad_packet:
		FR_STATS_INC(auth, total_unknown_types);

		DEBUG("Invalid packet code %d sent from client %s port %d : IGNORED",
		      packet->code, client->shortname, packet->src_port);
		rad_free(&sock->packet);
		return 0;
	} /* switch over packet types */

	if (!request_receive(listener, packet, client, fun)) {
		FR_STATS_INC(auth, total_packets_dropped);
		rad_free(&sock->packet);
		return 0;
	}

	sock->packet = NULL;	/* we have no need for more partial reads */
	return 1;
}

static int dual_tcp_accept(rad_listen_t *listener)
{
	int newfd, src_port;
	rad_listen_t *this;
	socklen_t salen;
	struct sockaddr_storage src;
	listen_socket_t *sock;
	fr_ipaddr_t src_ipaddr;
	RADCLIENT *client = NULL;

	salen = sizeof(src);

	DEBUG2(" ... new connection request on TCP socket.");

	newfd = accept(listener->fd, (struct sockaddr *) &src, &salen);
	if (newfd < 0) {
		/*
		 *	Non-blocking sockets must handle this.
		 */
#ifdef EWOULDBLOCK
		if (errno == EWOULDBLOCK) {
			return 0;
		}
#endif

		DEBUG2(" ... failed to accept connection.");
		return -1;
	}

	if (!fr_sockaddr2ipaddr(&src, salen, &src_ipaddr, &src_port)) {
		close(newfd);
		DEBUG2(" ... unknown address family.");
		return 0;
	}

	/*
	 *	Enforce client IP address checks on accept, not on
	 *	every packet.
	 */
	if ((client = client_listener_find(listener,
					   &src_ipaddr, src_port)) == NULL) {
		close(newfd);
		FR_STATS_INC(auth, total_invalid_requests);
		return 0;
	}

	/*
	 *	Enforce max_connections on client && listen section.
	 */
	if ((client->limit.max_connections != 0) &&
	    (client->limit.max_connections == client->limit.num_connections)) {
		/*
		 *	FIXME: Print client IP/port, and server IP/port.
		 */
		INFO("Ignoring new connection due to client max_connections (%d)", client->limit.max_connections);
		close(newfd);
		return 0;
	}

	sock = listener->data;
	if ((sock->limit.max_connections != 0) &&
	    (sock->limit.max_connections == sock->limit.num_connections)) {
		/*
		 *	FIXME: Print client IP/port, and server IP/port.
		 */
		INFO("Ignoring new connection due to socket max_connections");
		close(newfd);
		return 0;
	}
	client->limit.num_connections++;
	sock->limit.num_connections++;

	/*
	 *	Add the new listener.
	 */
	this = listen_alloc(listener, listener->type);
	if (!this) return -1;

	/*
	 *	Copy everything, including the pointer to the socket
	 *	information.
	 */
	sock = this->data;
	memcpy(this->data, listener->data, sizeof(*sock));
	memcpy(this, listener, sizeof(*this));
	this->next = NULL;
	this->data = sock;	/* fix it back */

	sock->parent = listener->data;
	sock->other_ipaddr = src_ipaddr;
	sock->other_port = src_port;
	sock->client = client;
	sock->opened = sock->last_packet = time(NULL);

	/*
	 *	Set the limits.  The defaults are the parent limits.
	 *	Client limits on max_connections are enforced dynamically.
	 *	Set the MINIMUM of client/socket idle timeout or lifetime.
	 */
	memcpy(&sock->limit, &sock->parent->limit, sizeof(sock->limit));

	if (client->limit.idle_timeout &&
	    ((sock->limit.idle_timeout == 0) ||
	     (client->limit.idle_timeout < sock->limit.idle_timeout))) {
		sock->limit.idle_timeout = client->limit.idle_timeout;
	}

	if (client->limit.lifetime &&
	    ((sock->limit.lifetime == 0) ||
	     (client->limit.lifetime < sock->limit.lifetime))) {
		sock->limit.lifetime = client->limit.lifetime;
	}

	this->fd = newfd;
	this->status = RAD_LISTEN_STATUS_INIT;


#ifdef WITH_COMMAND_SOCKET
	if (this->type == RAD_LISTEN_COMMAND) {
		this->recv = command_tcp_recv;
		this->send = command_tcp_send;
		command_write_magic(this->fd, sock);
	} else
#endif
	{

		this->recv = dual_tcp_recv;

#ifdef WITH_TLS
		if (this->tls) {
			this->recv = dual_tls_recv;
		this->send = dual_tls_send;
		}
#endif
	}

	/*
	 *	FIXME: set O_NONBLOCK on the accept'd fd.
	 *	See djb's portability rants for details.
	 */

	/*
	 *	Tell the event loop that we have a new FD.
	 *	This can be called from a child thread...
	 */
	event_new_fd(this);

	return 0;
}
#endif


/*
 *	This function is stupid and complicated.
 */
int common_socket_print(rad_listen_t const *this, char *buffer, size_t bufsize)
{
	size_t len;
	listen_socket_t *sock = this->data;
	char const *name = master_listen[this->type].name;

#define FORWARD len = strlen(buffer); if (len >= (bufsize + 1)) return 0;buffer += len;bufsize -= len
#define ADDSTRING(_x) strlcpy(buffer, _x, bufsize);FORWARD

	ADDSTRING(name);

	if (sock->interface) {
		ADDSTRING(" interface ");
		ADDSTRING(sock->interface);
	}

#ifdef WITH_TCP
	if (this->recv == dual_tcp_accept) {
		ADDSTRING(" proto tcp");
	}
#endif

#ifdef WITH_TCP
	/*
	 *	TCP sockets get printed a little differently, to make
	 *	it clear what's going on.
	 */
	if (sock->client) {
		ADDSTRING(" from client (");
		ip_ntoh(&sock->other_ipaddr, buffer, bufsize);
		FORWARD;

		ADDSTRING(", ");
		snprintf(buffer, bufsize, "%d", sock->other_port);
		FORWARD;
		ADDSTRING(") -> (");

		if ((sock->my_ipaddr.af == AF_INET) &&
		    (sock->my_ipaddr.ipaddr.ip4addr.s_addr == htonl(INADDR_ANY))) {
			strlcpy(buffer, "*", bufsize);
		} else {
			ip_ntoh(&sock->my_ipaddr, buffer, bufsize);
		}
		FORWARD;

		ADDSTRING(", ");
		snprintf(buffer, bufsize, "%d", sock->my_port);
		FORWARD;

		if (this->server) {
			ADDSTRING(", virtual-server=");
			ADDSTRING(this->server);
		}

		ADDSTRING(")");

		return 1;
	}

#ifdef WITH_PROXY
	/*
	 *	Maybe it's a socket that we opened to a home server.
	 */
	if ((sock->proto == IPPROTO_TCP) &&
	    (this->type == RAD_LISTEN_PROXY)) {
		ADDSTRING(" (");
		ip_ntoh(&sock->my_ipaddr, buffer, bufsize);
		FORWARD;

		ADDSTRING(", ");
		snprintf(buffer, bufsize, "%d", sock->my_port);
		FORWARD;
		ADDSTRING(") -> home_server (");

		if ((sock->other_ipaddr.af == AF_INET) &&
		    (sock->other_ipaddr.ipaddr.ip4addr.s_addr == htonl(INADDR_ANY))) {
			strlcpy(buffer, "*", bufsize);
		} else {
			ip_ntoh(&sock->other_ipaddr, buffer, bufsize);
		}
		FORWARD;

		ADDSTRING(", ");
		snprintf(buffer, bufsize, "%d", sock->other_port);
		FORWARD;

		ADDSTRING(")");

		return 1;
	}
#endif	/* WITH_PROXY */
#endif	/* WITH_TCP */

	ADDSTRING(" address ");

	if ((sock->my_ipaddr.af == AF_INET) &&
	    (sock->my_ipaddr.ipaddr.ip4addr.s_addr == htonl(INADDR_ANY))) {
		strlcpy(buffer, "*", bufsize);
	} else {
		ip_ntoh(&sock->my_ipaddr, buffer, bufsize);
	}
	FORWARD;

	ADDSTRING(" port ");
	snprintf(buffer, bufsize, "%d", sock->my_port);
	FORWARD;

#ifdef WITH_TLS
	if (this->tls) {
		ADDSTRING(" (TLS)");
		FORWARD;
	}
#endif

	if (this->server) {
		ADDSTRING(" as server ");
		ADDSTRING(this->server);
	}

#undef ADDSTRING
#undef FORWARD

	return 1;
}

extern int check_config;	/* radiusd.c */

static CONF_PARSER performance_config[] = {
	{ "skip_duplicate_checks", PW_TYPE_BOOLEAN,
	  offsetof(rad_listen_t, nodup), NULL,   NULL },

	{ "synchronous", PW_TYPE_BOOLEAN,
	  offsetof(rad_listen_t, synchronous), NULL,   NULL },

	{ "workers", PW_TYPE_INTEGER,
	  offsetof(rad_listen_t, workers), NULL,   NULL },

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};


static CONF_PARSER limit_config[] = {
	{ "max_pps", PW_TYPE_INTEGER,
	  offsetof(listen_socket_t, max_rate), NULL,   NULL },

#ifdef WITH_TCP
	{ "max_connections", PW_TYPE_INTEGER,
	  offsetof(listen_socket_t, limit.max_connections), NULL,   "16" },

	{ "lifetime", PW_TYPE_INTEGER,
	  offsetof(listen_socket_t, limit.lifetime), NULL,   "0" },

	{ "idle_timeout", PW_TYPE_INTEGER,
	  offsetof(listen_socket_t, limit.idle_timeout), NULL,   "30" },
#endif

	{ NULL, -1, 0, NULL, NULL }		/* end the list */
};

/*
 *	Parse an authentication or accounting socket.
 */
int common_socket_parse(CONF_SECTION *cs, rad_listen_t *this)
{
	int		rcode;
	int		listen_port;
	fr_ipaddr_t	ipaddr;
	listen_socket_t *sock = this->data;
	char		*section_name = NULL;
	CONF_SECTION	*client_cs, *parentcs;
	CONF_SECTION	*subcs;

	this->cs = cs;

	/*
	 *	Try IPv4 first
	 */
	memset(&ipaddr, 0, sizeof(ipaddr));
	ipaddr.ipaddr.ip4addr.s_addr = htonl(INADDR_NONE);
	rcode = cf_item_parse(cs, "ipaddr", PW_TYPE_IPADDR,
			      &ipaddr.ipaddr.ip4addr, NULL);
	if (rcode < 0) return -1;

	if (rcode == 0) { /* successfully parsed IPv4 */
		ipaddr.af = AF_INET;

	} else {	/* maybe IPv6? */
		rcode = cf_item_parse(cs, "ipv6addr", PW_TYPE_IPV6ADDR,
				      &ipaddr.ipaddr.ip6addr, NULL);
		if (rcode < 0) return -1;

		if (rcode == 1) {
			cf_log_err_cs(cs,
				   "No address specified in listen section");
			return -1;
		}
		ipaddr.af = AF_INET6;
	}

	rcode = cf_item_parse(cs, "port", PW_TYPE_INTEGER,
			      &listen_port, "0");
	if (rcode < 0) return -1;

	if ((listen_port < 0) || (listen_port > 65535)) {
			cf_log_err_cs(cs,
				   "Invalid value for \"port\"");
			return -1;
	}

	sock->proto = IPPROTO_UDP;

	if (cf_pair_find(cs, "proto")) {
#ifndef WITH_TCP
		cf_log_err_cs(cs,
			   "System does not support the TCP protocol.  Delete this line from the configuration file.");
		return -1;
#else
		char *proto = NULL;
#ifdef WITH_TLS
		CONF_SECTION *tls;
#endif

		rcode = cf_item_parse(cs, "proto", PW_TYPE_STRING_PTR,
				      &proto, "udp");
		if (rcode < 0) return -1;

		if (strcmp(proto, "udp") == 0) {
			sock->proto = IPPROTO_UDP;

		} else if (strcmp(proto, "tcp") == 0) {
			sock->proto = IPPROTO_TCP;
		} else {
			cf_log_err_cs(cs,
				   "Unknown proto name \"%s\"", proto);
			return -1;
		}

		/*
		 *	TCP requires a destination IP for sockets.
		 *	UDP doesn't, so it's allowed.
		 */
#ifdef WITH_PROXY
		if ((this->type == RAD_LISTEN_PROXY) &&
		    (sock->proto != IPPROTO_UDP)) {
			cf_log_err_cs(cs,
				   "Proxy listeners can only listen on proto = udp");
			return -1;
		}
#endif	/* WITH_PROXY */

#ifdef WITH_TLS
		tls = cf_section_sub_find(cs, "tls");

		/*
		 *	Don't allow TLS configurations for UDP sockets.
		 */
		if (sock->proto != IPPROTO_TCP) {
			cf_log_err_cs(cs,
				   "TLS transport is not available for UDP sockets.");
			return -1;
		}

		if (tls) {
			/*
			 *	FIXME: Make this better.
			 */
			if (listen_port == 0) listen_port = 2083;

			this->tls = tls_server_conf_parse(tls);
			if (!this->tls) {
				return -1;
			}

#ifdef HAVE_PTRHEAD_H
			if (pthread_mutex_init(&sock->mutex, NULL) < 0) {
				rad_assert(0 == 1);
				listen_free(&this);
				return 0;
			}
#endif

		}
#else  /* WITH_TLS */
		/*
		 *	Built without TLS.  Disallow it.
		 */
		if (cf_section_sub_find(cs, "tls")) {
			cf_log_err_cs(cs,
				   "TLS transport is not available in this executable.");
			return -1;
		}
#endif	/* WITH_TLS */

#endif	/* WITH_TCP */

		/*
		 *	No "proto" field.  Disallow TLS.
		 */
	} else if (cf_section_sub_find(cs, "tls")) {
		cf_log_err_cs(cs,
			   "TLS transport is not available in this \"listen\" section.");
		return -1;
	}

	/*
	 *	Magical tuning methods!
	 */
	subcs = cf_section_sub_find(cs, "performance");
	if (subcs) {
		rcode = cf_section_parse(subcs, this,
					 performance_config);
		if (rcode < 0) return -1;

		if (this->synchronous && sock->max_rate) {
			WARN("Setting 'max_pps' is incompatible with 'synchronous'.  Disabling 'max_pps'");
			sock->max_rate = 0;
		}

		if (!this->synchronous && this->workers) {
			WARN("Setting 'workers' requires 'synchronous'.  Disabling 'workers'");
			this->workers = 0;
		}
	}

	subcs = cf_section_sub_find(cs, "limit");
	if (subcs) {
		rcode = cf_section_parse(subcs, sock,
					 limit_config);
		if (rcode < 0) return -1;

		if (sock->max_rate && ((sock->max_rate < 10) || (sock->max_rate > 1000000))) {
			cf_log_err_cs(cs,
				      "Invalid value for \"max_pps\"");
			return -1;
		}

#ifdef WITH_TCP
		if ((sock->limit.idle_timeout > 0) && (sock->limit.idle_timeout < 5)) {
			WARN("Setting idle_timeout to 5");
			sock->limit.idle_timeout = 5;
		}

		if ((sock->limit.lifetime > 0) && (sock->limit.lifetime < 5)) {
			WARN("Setting lifetime to 5");
			sock->limit.lifetime = 5;
		}

		if ((sock->limit.lifetime > 0) && (sock->limit.idle_timeout > sock->limit.lifetime)) {
			WARN("Setting idle_timeout to 0");
			sock->limit.idle_timeout = 0;
		}

		/*
		 *	Force no duplicate detection for TCP sockets.
		 */
		if (sock->proto == IPPROTO_TCP) {
			this->nodup = true;
		}

	} else {
		sock->limit.max_connections = 60;
		sock->limit.idle_timeout = 30;
		sock->limit.lifetime = 0;
#endif
	}

	sock->my_ipaddr = ipaddr;
	sock->my_port = listen_port;

#ifdef WITH_PROXY
	if (check_config) {
		if (home_server_find(&sock->my_ipaddr, sock->my_port, sock->proto)) {
				char buffer[128];

				EDEBUG("We have been asked to listen on %s port %d, which is also listed as a home server.  This can create a proxy loop.",
				      ip_ntoh(&sock->my_ipaddr, buffer, sizeof(buffer)),
				      sock->my_port);
				return -1;
		}

		return 0;	/* don't do anything */
	}
#endif

	/*
	 *	If we can bind to interfaces, do so,
	 *	else don't.
	 */
	if (cf_pair_find(cs, "interface")) {
		char const *value;
		CONF_PAIR *cp = cf_pair_find(cs, "interface");

		rad_assert(cp != NULL);
		value = cf_pair_value(cp);
		if (!value) {
			cf_log_err_cs(cs,
				   "No interface name given");
			return -1;
		}
		sock->interface = value;
	}

#ifdef WITH_DHCP
	/*
	 *	If we can do broadcasts..
	 */
	if (cf_pair_find(cs, "broadcast")) {
#ifndef SO_BROADCAST
		cf_log_err_cs(cs,
			   "System does not support broadcast sockets.  Delete this line from the configuration file.");
		return -1;
#else
		char const *value;
		CONF_PAIR *cp = cf_pair_find(cs, "broadcast");

		if (this->type != RAD_LISTEN_DHCP) {
			cf_log_err_cp(cp,
				   "Broadcast can only be set for DHCP listeners.  Delete this line from the configuration file.");
			return -1;
		}

		rad_assert(cp != NULL);
		value = cf_pair_value(cp);
		if (!value) {
			cf_log_err_cs(cs,
				   "No broadcast value given");
			return -1;
		}

		/*
		 *	Hack... whatever happened to cf_section_parse?
		 */
		sock->broadcast = (strcmp(value, "yes") == 0);
#endif
	}
#endif

	/*
	 *	And bind it to the port.
	 */
	if (listen_bind(this) < 0) {
		char buffer[128];
		cf_log_err_cs(cs,
			   "Error binding to port for %s port %d",
			   ip_ntoh(&sock->my_ipaddr, buffer, sizeof(buffer)),
			   sock->my_port);
		return -1;
	}

#ifdef WITH_PROXY
	/*
	 *	Proxy sockets don't have clients.
	 */
	if (this->type == RAD_LISTEN_PROXY) return 0;
#endif

	/*
	 *	The more specific configurations are preferred to more
	 *	generic ones.
	 */
	client_cs = NULL;
	parentcs = cf_top_section(cs);
	rcode = cf_item_parse(cs, "clients", PW_TYPE_STRING_PTR,
			      &section_name, NULL);
	if (rcode < 0) return -1; /* bad string */
	if (rcode == 0) {
		/*
		 *	Explicit list given: use it.
		 */
		client_cs = cf_section_sub_find_name2(parentcs,
						      "clients",
						      section_name);
		if (!client_cs) {
			client_cs = cf_section_find(section_name);
		}
		if (!client_cs) {
			cf_log_err_cs(cs,
				   "Failed to find clients %s {...}",
				   section_name);
			return -1;
		}
	} /* else there was no "clients = " entry. */

	if (!client_cs) {
		CONF_SECTION *server_cs;

		server_cs = cf_section_sub_find_name2(parentcs,
						      "server",
						      this->server);
		/*
		 *	Found a "server foo" section.  If there are clients
		 *	in it, use them.
		 */
		if (server_cs &&
		    (cf_section_sub_find(server_cs, "client") != NULL)) {
			client_cs = server_cs;
		}
	}

	/*
	 *	Still nothing.  Look for global clients.
	 */
	if (!client_cs) client_cs = parentcs;

	sock->clients = clients_parse_section(client_cs);
	if (!sock->clients) {
		cf_log_err_cs(cs,
			   "Failed to load clients for this listen section");
		return -1;
	}

#ifdef WITH_TCP
	if (sock->proto == IPPROTO_TCP) {
		/*
		 *	Re-write the listener receive function to
		 *	allow us to accept the socket.
		 */
		this->recv = dual_tcp_accept;
	}
#endif

	return 0;
}

/*
 *	Send an authentication response packet
 */
static int auth_socket_send(rad_listen_t *listener, REQUEST *request)
{
	rad_assert(request->listener == listener);
	rad_assert(listener->send == auth_socket_send);

#ifdef WITH_UDPFROMTO
	/*
	 *	Overwrite the src ip address on the outbound packet
	 *	with the one specified by the client.
	 *	This is useful to work around broken DSR implementations
	 *	and other routing issues.
	 */
	if (request->client->src_ipaddr.af != AF_UNSPEC) {
		request->reply->src_ipaddr = request->client->src_ipaddr;
	}
#endif

	if (rad_send(request->reply, request->packet,
		     request->client->secret) < 0) {
		RERROR("Failed sending reply: %s",
			       fr_strerror());
		return -1;
	}

	return 0;
}


#ifdef WITH_ACCOUNTING
/*
 *	Send an accounting response packet (or not)
 */
static int acct_socket_send(rad_listen_t *listener, REQUEST *request)
{
	rad_assert(request->listener == listener);
	rad_assert(listener->send == acct_socket_send);

	/*
	 *	Accounting reject's are silently dropped.
	 *
	 *	We do it here to avoid polluting the rest of the
	 *	code with this knowledge
	 */
	if (request->reply->code == 0) return 0;

#ifdef WITH_UDPFROMTO
	/*
	 *	Overwrite the src ip address on the outbound packet
	 *	with the one specified by the client.
	 *	This is useful to work around broken DSR implementations
	 *	and other routing issues.
	 */
	if (request->client->src_ipaddr.af != AF_UNSPEC) {
		request->reply->src_ipaddr = request->client->src_ipaddr;
	}
#endif

	if (rad_send(request->reply, request->packet,
		     request->client->secret) < 0) {
		RERROR("Failed sending reply: %s",
			       fr_strerror());
		return -1;
	}

	return 0;
}
#endif

#ifdef WITH_PROXY
/*
 *	Send a packet to a home server.
 *
 *	FIXME: have different code for proxy auth & acct!
 */
static int proxy_socket_send(rad_listen_t *listener, REQUEST *request)
{
	rad_assert(request->proxy_listener == listener);
	rad_assert(listener->send == proxy_socket_send);

	if (rad_send(request->proxy, NULL,
		     request->home_server->secret) < 0) {
		RERROR("Failed sending proxied request: %s",
			       fr_strerror());
		return -1;
	}

	return 0;
}
#endif

#ifdef WITH_STATS
/*
 *	Check if an incoming request is "ok"
 *
 *	It takes packets, not requests.  It sees if the packet looks
 *	OK.  If so, it does a number of sanity checks on it.
  */
static int stats_socket_recv(rad_listen_t *listener)
{
	ssize_t		rcode;
	int		code, src_port;
	RADIUS_PACKET	*packet;
	RADCLIENT	*client = NULL;
	fr_ipaddr_t	src_ipaddr;

	rcode = rad_recv_header(listener->fd, &src_ipaddr, &src_port, &code);
	if (rcode < 0) return 0;

	FR_STATS_INC(auth, total_requests);

	if (rcode < 20) {	/* AUTH_HDR_LEN */
		FR_STATS_INC(auth, total_malformed_requests);
		return 0;
	}

	if ((client = client_listener_find(listener,
					   &src_ipaddr, src_port)) == NULL) {
		rad_recv_discard(listener->fd);
		FR_STATS_INC(auth, total_invalid_requests);
		return 0;
	}

	FR_STATS_TYPE_INC(client->auth.total_requests);

	/*
	 *	We only understand Status-Server on this socket.
	 */
	if (code != PW_STATUS_SERVER) {
		DEBUG("Ignoring packet code %d sent to Status-Server port",
		      code);
		rad_recv_discard(listener->fd);
		FR_STATS_INC(auth, total_unknown_types);
		return 0;
	}

	/*
	 *	Now that we've sanity checked everything, receive the
	 *	packet.
	 */
	packet = rad_recv(listener->fd, 1); /* require message authenticator */
	if (!packet) {
		FR_STATS_INC(auth, total_malformed_requests);
		DEBUG("%s", fr_strerror());
		return 0;
	}

	if (!request_receive(listener, packet, client, rad_status_server)) {
		FR_STATS_INC(auth, total_packets_dropped);
		rad_free(&packet);
		return 0;
	}

	return 1;
}
#endif


/*
 *	Check if an incoming request is "ok"
 *
 *	It takes packets, not requests.  It sees if the packet looks
 *	OK.  If so, it does a number of sanity checks on it.
  */
static int auth_socket_recv(rad_listen_t *listener)
{
	ssize_t		rcode;
	int		code, src_port;
	RADIUS_PACKET	*packet;
	RAD_REQUEST_FUNP fun = NULL;
	RADCLIENT	*client = NULL;
	fr_ipaddr_t	src_ipaddr;

	rcode = rad_recv_header(listener->fd, &src_ipaddr, &src_port, &code);
	if (rcode < 0) return 0;

	FR_STATS_INC(auth, total_requests);

	if (rcode < 20) {	/* AUTH_HDR_LEN */
		FR_STATS_INC(auth, total_malformed_requests);
		return 0;
	}

	if ((client = client_listener_find(listener,
					   &src_ipaddr, src_port)) == NULL) {
		rad_recv_discard(listener->fd);
		FR_STATS_INC(auth, total_invalid_requests);
		return 0;
	}

	FR_STATS_TYPE_INC(client->auth.total_requests);

	/*
	 *	Some sanity checks, based on the packet code.
	 */
	switch(code) {
	case PW_AUTHENTICATION_REQUEST:
		fun = rad_authenticate;
		break;

	case PW_STATUS_SERVER:
		if (!mainconfig.status_server) {
			rad_recv_discard(listener->fd);
			FR_STATS_INC(auth, total_unknown_types);
			WDEBUG("Ignoring Status-Server request due to security configuration");
			return 0;
		}
		fun = rad_status_server;
		break;

	default:
		rad_recv_discard(listener->fd);
		FR_STATS_INC(auth,total_unknown_types);

		DEBUG("Invalid packet code %d sent to authentication port from client %s port %d : IGNORED",
		      code, client->shortname, src_port);
		return 0;
		break;
	} /* switch over packet types */

	/*
	 *	Now that we've sanity checked everything, receive the
	 *	packet.
	 */
	packet = rad_recv(listener->fd, client->message_authenticator);
	if (!packet) {
		FR_STATS_INC(auth, total_malformed_requests);
		DEBUG("%s", fr_strerror());
		return 0;
	}

#ifdef __APPLE__
#ifdef WITH_UDPFROMTO
	/*
	 *	This is a NICE Mac OSX bug.  Create an interface with
	 *	two IP address, and then configure one listener for
	 *	each IP address.  Send thousands of packets to one
	 *	address, and some will show up on the OTHER socket.
	 *
	 *	This hack works ONLY if the clients are global.  If
	 *	each listener has the same client IP, but with
	 *	different secrets, then it will fail the rad_recv()
	 *	check above, and there's nothing you can do.
	 */
	{
		listen_socket_t *sock = listener->data;
		rad_listen_t *other;
		
		other = listener_find_byipaddr(&packet->dst_ipaddr,
					       packet->dst_port, sock->proto);
		if (other) listener = other;
	}
#endif
#endif
	

	if (!request_receive(listener, packet, client, fun)) {
		FR_STATS_INC(auth, total_packets_dropped);
		rad_free(&packet);
		return 0;
	}

	return 1;
}


#ifdef WITH_ACCOUNTING
/*
 *	Receive packets from an accounting socket
 */
static int acct_socket_recv(rad_listen_t *listener)
{
	ssize_t		rcode;
	int		code, src_port;
	RADIUS_PACKET	*packet;
	RAD_REQUEST_FUNP fun = NULL;
	RADCLIENT	*client = NULL;
	fr_ipaddr_t	src_ipaddr;

	rcode = rad_recv_header(listener->fd, &src_ipaddr, &src_port, &code);
	if (rcode < 0) return 0;

	FR_STATS_INC(acct, total_requests);

	if (rcode < 20) {	/* AUTH_HDR_LEN */
		FR_STATS_INC(acct, total_malformed_requests);
		return 0;
	}

	if ((client = client_listener_find(listener,
					   &src_ipaddr, src_port)) == NULL) {
		rad_recv_discard(listener->fd);
		FR_STATS_INC(acct, total_invalid_requests);
		return 0;
	}

	FR_STATS_TYPE_INC(client->acct.total_requests);

	/*
	 *	Some sanity checks, based on the packet code.
	 */
	switch(code) {
	case PW_ACCOUNTING_REQUEST:
		fun = rad_accounting;
		break;

	case PW_STATUS_SERVER:
		if (!mainconfig.status_server) {
			rad_recv_discard(listener->fd);
			FR_STATS_INC(acct, total_unknown_types);

			WDEBUG("Ignoring Status-Server request due to security configuration");
			return 0;
		}
		fun = rad_status_server;
		break;

	default:
		rad_recv_discard(listener->fd);
		FR_STATS_INC(acct, total_unknown_types);

		DEBUG("Invalid packet code %d sent to a accounting port from client %s port %d : IGNORED",
		      code, client->shortname, src_port);
		return 0;
	} /* switch over packet types */

	/*
	 *	Now that we've sanity checked everything, receive the
	 *	packet.
	 */
	packet = rad_recv(listener->fd, 0);
	if (!packet) {
		FR_STATS_INC(acct, total_malformed_requests);
		ERROR("%s", fr_strerror());
		return 0;
	}

	/*
	 *	There can be no duplicate accounting packets.
	 */
	if (!request_receive(listener, packet, client, fun)) {
		FR_STATS_INC(acct, total_packets_dropped);
		rad_free(&packet);
		return 0;
	}

	return 1;
}
#endif


#ifdef WITH_COA
static int do_proxy(REQUEST *request)
{
	VALUE_PAIR *vp;

	if (request->in_proxy_hash ||
	    (request->proxy_reply && (request->proxy_reply->code != 0))) {
		return 0;
	}

	vp = pairfind(request->config_items, PW_HOME_SERVER_POOL, 0, TAG_ANY);
	if (!vp) return 0;

	if (!home_pool_byname(vp->vp_strvalue, HOME_TYPE_COA)) {
		REDEBUG2("Cannot proxy to unknown pool %s",
			vp->vp_strvalue);
		return 0;
	}

	return 1;
}

/*
 *	Receive a CoA packet.
 */
static int rad_coa_recv(REQUEST *request)
{
	int rcode = RLM_MODULE_OK;
	int ack, nak;
	VALUE_PAIR *vp;

	/*
	 *	Get the correct response
	 */
	switch (request->packet->code) {
	case PW_COA_REQUEST:
		ack = PW_COA_ACK;
		nak = PW_COA_NAK;
		break;

	case PW_DISCONNECT_REQUEST:
		ack = PW_DISCONNECT_ACK;
		nak = PW_DISCONNECT_NAK;
		break;

	default:		/* shouldn't happen */
		return RLM_MODULE_FAIL;
	}

#ifdef WITH_PROXY
#define WAS_PROXIED (request->proxy)
#else
#define WAS_PROXIED (0)
#endif

	if (!WAS_PROXIED) {
		/*
		 *	RFC 5176 Section 3.3.  If we have a CoA-Request
		 *	with Service-Type = Authorize-Only, it MUST
		 *	have a State attribute in it.
		 */
		vp = pairfind(request->packet->vps, PW_SERVICE_TYPE, 0, TAG_ANY);
		if (request->packet->code == PW_COA_REQUEST) {
			if (vp && (vp->vp_integer == 17)) {
				vp = pairfind(request->packet->vps, PW_STATE, 0, TAG_ANY);
				if (!vp || (vp->length == 0)) {
					REDEBUG("CoA-Request with Service-Type = Authorize-Only MUST contain a State attribute");
					request->reply->code = PW_COA_NAK;
					return RLM_MODULE_FAIL;
				}
			}
		} else if (vp) {
			/*
			 *	RFC 5176, Section 3.2.
			 */
			REDEBUG("Disconnect-Request MUST NOT contain a Service-Type attribute");
			request->reply->code = PW_DISCONNECT_NAK;
			return RLM_MODULE_FAIL;
		}

		rcode = process_recv_coa(0, request);
		switch (rcode) {
		case RLM_MODULE_FAIL:
		case RLM_MODULE_INVALID:
		case RLM_MODULE_REJECT:
		case RLM_MODULE_USERLOCK:
		default:
			request->reply->code = nak;
			break;

		case RLM_MODULE_HANDLED:
			return rcode;

		case RLM_MODULE_NOOP:
		case RLM_MODULE_NOTFOUND:
		case RLM_MODULE_OK:
		case RLM_MODULE_UPDATED:
			if (do_proxy(request)) return RLM_MODULE_OK;
			request->reply->code = ack;
			break;
		}
	} else if (request->proxy_reply) {
		/*
		 *	Start the reply code with the proxy reply
		 *	code.
		 */
		request->reply->code = request->proxy_reply->code;
	}

	/*
	 *	Copy State from the request to the reply.
	 *	See RFC 5176 Section 3.3.
	 */
	vp = paircopy2(request->reply, request->packet->vps, PW_STATE, 0, TAG_ANY);
	if (vp) pairadd(&request->reply->vps, vp);

	/*
	 *	We may want to over-ride the reply.
	 */
	if (request->reply->code) {
		rcode = process_send_coa(0, request);
		switch (rcode) {
			/*
			 *	We need to send CoA-NAK back if Service-Type
			 *	is Authorize-Only.  Rely on the user's policy
			 *	to do that.  We're not a real NAS, so this
			 *	restriction doesn't (ahem) apply to us.
			 */
		case RLM_MODULE_FAIL:
		case RLM_MODULE_INVALID:
		case RLM_MODULE_REJECT:
		case RLM_MODULE_USERLOCK:
		default:
			/*
			 *	Over-ride an ACK with a NAK
			 */
			request->reply->code = nak;
			break;

		case RLM_MODULE_HANDLED:
			return rcode;

		case RLM_MODULE_NOOP:
		case RLM_MODULE_NOTFOUND:
		case RLM_MODULE_OK:
		case RLM_MODULE_UPDATED:
			/*
			 *	Do NOT over-ride a previously set value.
			 *	Otherwise an "ok" here will re-write a
			 *	NAK to an ACK.
			 */
			if (request->reply->code == 0) {
				request->reply->code = ack;
			}
			break;
		}
	}

	return RLM_MODULE_OK;
}


/*
 *	Check if an incoming request is "ok"
 *
 *	It takes packets, not requests.  It sees if the packet looks
 *	OK.  If so, it does a number of sanity checks on it.
  */
static int coa_socket_recv(rad_listen_t *listener)
{
	ssize_t		rcode;
	int		code, src_port;
	RADIUS_PACKET	*packet;
	RAD_REQUEST_FUNP fun = NULL;
	RADCLIENT	*client = NULL;
	fr_ipaddr_t	src_ipaddr;

	rcode = rad_recv_header(listener->fd, &src_ipaddr, &src_port, &code);
	if (rcode < 0) return 0;

	if (rcode < 20) {	/* AUTH_HDR_LEN */
		FR_STATS_INC(coa, total_requests);
		FR_STATS_INC(coa, total_malformed_requests);
		return 0;
	}

	if ((client = client_listener_find(listener,
					   &src_ipaddr, src_port)) == NULL) {
		rad_recv_discard(listener->fd);
		FR_STATS_INC(coa, total_requests);
		FR_STATS_INC(coa, total_invalid_requests);
		return 0;
	}

	/*
	 *	Some sanity checks, based on the packet code.
	 */
	switch(code) {
	case PW_COA_REQUEST:
		FR_STATS_INC(coa, total_requests);
		fun = rad_coa_recv;
		break;

	case PW_DISCONNECT_REQUEST:
		FR_STATS_INC(dsc, total_requests);
		fun = rad_coa_recv;
		break;

	default:
		rad_recv_discard(listener->fd);
		FR_STATS_INC(coa, total_unknown_types);
		DEBUG("Invalid packet code %d sent to coa port from client %s port %d : IGNORED",
		      code, client->shortname, src_port);
		return 0;
	} /* switch over packet types */

	/*
	 *	Now that we've sanity checked everything, receive the
	 *	packet.
	 */
	packet = rad_recv(listener->fd, client->message_authenticator);
	if (!packet) {
		FR_STATS_INC(coa, total_malformed_requests);
		DEBUG("%s", fr_strerror());
		return 0;
	}

	if (!request_receive(listener, packet, client, fun)) {
		FR_STATS_INC(coa, total_packets_dropped);
		rad_free(&packet);
		return 0;
	}

	return 1;
}
#endif

#ifdef WITH_PROXY
/*
 *	Recieve packets from a proxy socket.
 */
static int proxy_socket_recv(rad_listen_t *listener)
{
	RADIUS_PACKET	*packet;
	char		buffer[128];

	packet = rad_recv(listener->fd, 0);
	if (!packet) {
		ERROR("%s", fr_strerror());
		return 0;
	}

	/*
	 *	FIXME: Client MIB updates?
	 */
	switch(packet->code) {
	case PW_AUTHENTICATION_ACK:
	case PW_ACCESS_CHALLENGE:
	case PW_AUTHENTICATION_REJECT:
		break;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_RESPONSE:
		break;
#endif

#ifdef WITH_COA
	case PW_DISCONNECT_ACK:
	case PW_DISCONNECT_NAK:
	case PW_COA_ACK:
	case PW_COA_NAK:
		break;
#endif

	default:
		/*
		 *	FIXME: Update MIB for packet types?
		 */
		ERROR("Invalid packet code %d sent to a proxy port "
		       "from home server %s port %d - ID %d : IGNORED",
		       packet->code,
		       ip_ntoh(&packet->src_ipaddr, buffer, sizeof(buffer)),
		       packet->src_port, packet->id);
		rad_free(&packet);
		return 0;
	}

	if (!request_proxy_reply(packet)) {
		rad_free(&packet);
		return 0;
	}

	return 1;
}

#ifdef WITH_TCP
/*
 *	Recieve packets from a proxy socket.
 */
static int proxy_socket_tcp_recv(rad_listen_t *listener)
{
	RADIUS_PACKET	*packet;
	listen_socket_t	*sock = listener->data;
	char		buffer[128];

	packet = fr_tcp_recv(listener->fd, 0);
	if (!packet) {
		listener->status = RAD_LISTEN_STATUS_REMOVE_NOW;
		event_new_fd(listener);
		return 0;
	}

	/*
	 *	FIXME: Client MIB updates?
	 */
	switch(packet->code) {
	case PW_AUTHENTICATION_ACK:
	case PW_ACCESS_CHALLENGE:
	case PW_AUTHENTICATION_REJECT:
		break;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_RESPONSE:
		break;
#endif

	default:
		/*
		 *	FIXME: Update MIB for packet types?
		 */
		ERROR("Invalid packet code %d sent to a proxy port "
		       "from home server %s port %d - ID %d : IGNORED",
		       packet->code,
		       ip_ntoh(&packet->src_ipaddr, buffer, sizeof(buffer)),
		       packet->src_port, packet->id);
		rad_free(&packet);
		return 0;
	}

	packet->src_ipaddr = sock->other_ipaddr;
	packet->src_port = sock->other_port;
	packet->dst_ipaddr = sock->my_ipaddr;
	packet->dst_port = sock->my_port;

	/*
	 *	FIXME: Have it return an indication of packets that
	 *	are OK to ignore (dups, too late), versus ones that
	 *	aren't OK to ignore (unknown response, spoofed, etc.)
	 *
	 *	Close the socket on bad packets...
	 */
	if (!request_proxy_reply(packet)) {
		rad_free(&packet);
		return 0;
	}

	sock->opened = sock->last_packet = time(NULL);

	return 1;
}
#endif
#endif


static int client_socket_encode(UNUSED rad_listen_t *listener, REQUEST *request)
{
	if (!request->reply->code) return 0;

	if (rad_encode(request->reply, request->packet,
		       request->client->secret) < 0) {
		RERROR("Failed encoding packet: %s",
			       fr_strerror());
		return -1;
	}

	if (rad_sign(request->reply, request->packet,
		     request->client->secret) < 0) {
		RERROR("Failed signing packet: %s",
			       fr_strerror());
		return -1;
	}

	return 0;
}


static int client_socket_decode(UNUSED rad_listen_t *listener, REQUEST *request)
{
	if (rad_verify(request->packet, NULL,
		       request->client->secret) < 0) {
		return -1;
	}

	return rad_decode(request->packet, NULL,
			  request->client->secret);
}

#ifdef WITH_PROXY
static int proxy_socket_encode(UNUSED rad_listen_t *listener, REQUEST *request)
{
	if (rad_encode(request->proxy, NULL, request->home_server->secret) < 0) {
		RERROR("Failed encoding proxied packet: %s",
			       fr_strerror());
		return -1;
	}

	if (rad_sign(request->proxy, NULL, request->home_server->secret) < 0) {
		RERROR("Failed signing proxied packet: %s",
			       fr_strerror());
		return -1;
	}

	return 0;
}


static int proxy_socket_decode(UNUSED rad_listen_t *listener, REQUEST *request)
{
	/*
	 *	rad_verify is run in event.c, received_proxy_response()
	 */

	return rad_decode(request->proxy_reply, request->proxy,
			   request->home_server->secret);
}
#endif

#include "command.c"

/*
 *	Temporarily NOT const!
 */
static fr_protocol_t master_listen[RAD_LISTEN_MAX] = {
#ifdef WITH_STATS
	{ RLM_MODULE_INIT, "status", sizeof(listen_socket_t), NULL,
	  common_socket_parse, NULL,
	  stats_socket_recv, auth_socket_send,
	  common_socket_print, client_socket_encode, client_socket_decode },
#else
	/*
	 *	This always gets defined.
	 */
	{ RLM_MODULE_INIT, "status", 0, NULL,
	  NULL, NULL, NULL, NULL, NULL, NULL, NULL},	/* RAD_LISTEN_NONE */
#endif

#ifdef WITH_PROXY
	/* proxying */
	{ RLM_MODULE_INIT, "proxy", sizeof(listen_socket_t), NULL,
	  common_socket_parse, NULL,
	  proxy_socket_recv, proxy_socket_send,
	  common_socket_print, proxy_socket_encode, proxy_socket_decode },
#endif

	/* authentication */
	{ RLM_MODULE_INIT, "auth", sizeof(listen_socket_t), NULL,
	  common_socket_parse, NULL,
	  auth_socket_recv, auth_socket_send,
	  common_socket_print, client_socket_encode, client_socket_decode },

#ifdef WITH_ACCOUNTING
	/* accounting */
	{ RLM_MODULE_INIT, "acct", sizeof(listen_socket_t), NULL,
	  common_socket_parse, NULL,
	  acct_socket_recv, acct_socket_send,
	  common_socket_print, client_socket_encode, client_socket_decode},
#endif

#ifdef WITH_DETAIL
	/* detail */
	{ RLM_MODULE_INIT, "detail", sizeof(listen_detail_t), NULL,
	  detail_parse, detail_free,
	  detail_recv, detail_send,
	  detail_print, detail_encode, detail_decode },
#endif

#ifdef WITH_VMPS
	/* vlan query protocol */
	{ 0, "vmps", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
#endif

#ifdef WITH_DHCP
	/* dhcp query protocol */
	{ 0, "dhcp", 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
#endif

#ifdef WITH_COMMAND_SOCKET
	/* TCP command socket */
	{ RLM_MODULE_INIT, "control", sizeof(fr_command_socket_t), NULL,
	  command_socket_parse, command_socket_free,
	  command_domain_accept, command_domain_send,
	  command_socket_print, command_socket_encode, command_socket_decode },
#endif

#ifdef WITH_COA
	/* Change of Authorization */
	{ RLM_MODULE_INIT, "coa", sizeof(listen_socket_t), NULL,
	  common_socket_parse, NULL,
	  coa_socket_recv, auth_socket_send, /* CoA packets are same as auth */
	  common_socket_print, client_socket_encode, client_socket_decode },
#endif

};



/*
 *	Binds a listener to a socket.
 */
static int listen_bind(rad_listen_t *this)
{
	int rcode;
	struct sockaddr_storage salocal;
	socklen_t	salen;
	listen_socket_t *sock = this->data;
#ifndef WITH_TCP
#define proto_for_port "udp"
#define sock_type SOCK_DGRAM
#else
	char const *proto_for_port = "udp";
	int sock_type = SOCK_DGRAM;

	if (sock->proto == IPPROTO_TCP) {
#ifdef WITH_VMPS
		if (this->type == RAD_LISTEN_VQP) {
			ERROR("VQP does not support TCP transport");
			return -1;
		}
#endif

		proto_for_port = "tcp";
		sock_type = SOCK_STREAM;
	}
#endif

	/*
	 *	If the port is zero, then it means the appropriate
	 *	thing from /etc/services.
	 */
	if (sock->my_port == 0) {
		struct servent	*svp;

		switch (this->type) {
		case RAD_LISTEN_AUTH:
			svp = getservbyname ("radius", proto_for_port);
			if (svp != NULL) {
				sock->my_port = ntohs(svp->s_port);
			} else {
				sock->my_port = PW_AUTH_UDP_PORT;
			}
			break;

#ifdef WITH_ACCOUNTING
		case RAD_LISTEN_ACCT:
			svp = getservbyname ("radacct", proto_for_port);
			if (svp != NULL) {
				sock->my_port = ntohs(svp->s_port);
			} else {
				sock->my_port = PW_ACCT_UDP_PORT;
			}
			break;
#endif

#ifdef WITH_PROXY
		case RAD_LISTEN_PROXY:
			/* leave it at zero */
			break;
#endif

#ifdef WITH_VMPS
		case RAD_LISTEN_VQP:
			sock->my_port = 1589;
			break;
#endif

#ifdef WITH_COMMAND_SOCKET
		case RAD_LISTEN_COMMAND:
			sock->my_port = PW_RADMIN_PORT;
			break;
#endif

#ifdef WITH_COA
		case RAD_LISTEN_COA:
			svp = getservbyname ("radius-dynauth", "udp");
			if (svp != NULL) {
				sock->my_port = ntohs(svp->s_port);
			} else {
				sock->my_port = PW_COA_UDP_PORT;
			}
			break;
#endif

		default:
			WDEBUG("Internal sanity check failed in binding to socket.  Ignoring problem.");
			return -1;
		}
	}

	/*
	 *	Don't open sockets if we're checking the config.
	 */
	if (check_config) {
		this->fd = -1;
		return 0;
	}

	/*
	 *	Copy fr_socket() here, as we may need to bind to a device.
	 */
	this->fd = socket(sock->my_ipaddr.af, sock_type, 0);
	if (this->fd < 0) {
		char buffer[256];

		this->print(this, buffer, sizeof(buffer));

		ERROR("Failed opening %s: %s", buffer, strerror(errno));
		return -1;
	}

#ifdef FD_CLOEXEC
	/*
	 *	We don't want child processes inheriting these
	 *	file descriptors.
	 */
	rcode = fcntl(this->fd, F_GETFD);
	if (rcode >= 0) {
		if (fcntl(this->fd, F_SETFD, rcode | FD_CLOEXEC) < 0) {
			close(this->fd);
			ERROR("Failed setting close on exec: %s", strerror(errno));
			return -1;
		}
	}
#endif

	/*
	 *	Bind to a device BEFORE touching IP addresses.
	 */
	if (sock->interface) {
#ifdef SO_BINDTODEVICE
		struct ifreq ifreq;

		memset(&ifreq, 0, sizeof(ifreq));
		strlcpy(ifreq.ifr_name, sock->interface, sizeof(ifreq.ifr_name));

		fr_suid_up();
		rcode = setsockopt(this->fd, SOL_SOCKET, SO_BINDTODEVICE,
				   (char *)&ifreq, sizeof(ifreq));
		fr_suid_down();
		if (rcode < 0) {
			close(this->fd);
			ERROR("Failed binding to interface %s: %s",
			       sock->interface, strerror(errno));
			return -1;
		} /* else it worked. */
#else
#ifdef HAVE_STRUCT_SOCKADDR_IN6
#ifdef HAVE_NET_IF_H
		/*
		 *	Odds are that any system supporting "bind to
		 *	device" also supports IPv6, so this next bit
		 *	isn't necessary.  But it's here for
		 *	completeness.
		 *
		 *	If we're doing IPv6, and the scope hasn't yet
		 *	been defined, set the scope to the scope of
		 *	the interface.
		 */
		if (sock->my_ipaddr.af == AF_INET6) {
			if (sock->my_ipaddr.scope == 0) {
				sock->my_ipaddr.scope = if_nametoindex(sock->interface);
				if (sock->my_ipaddr.scope == 0) {
					close(this->fd);
					ERROR("Failed finding interface %s: %s",
					       sock->interface, strerror(errno));
					return -1;
				}
			} /* else scope was defined: we're OK. */
		} else
#endif
#endif
				/*
				 *	IPv4: no link local addresses,
				 *	and no bind to device.
				 */
		{
			close(this->fd);
			ERROR("Failed binding to interface %s: \"bind to device\" is unsupported", sock->interface);
			return -1;
		}
#endif
	}

#ifdef WITH_TCP
	if (sock->proto == IPPROTO_TCP) {
		int on = 1;

		if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
			close(this->fd);
			ERROR("Failed to reuse address: %s", strerror(errno));
			return -1;
		}
	}
#endif

#if defined(WITH_TCP) && defined(WITH_UDPFROMTO)
	else			/* UDP sockets get UDPfromto */
#endif

#ifdef WITH_UDPFROMTO
	/*
	 *	Initialize udpfromto for all sockets.
	 */
	if (udpfromto_init(this->fd) != 0) {
		ERROR("Failed initializing udpfromto: %s",
		       strerror(errno));
		close(this->fd);
		return -1;
	}
#endif

	/*
	 *	Set up sockaddr stuff.
	 */
	if (!fr_ipaddr2sockaddr(&sock->my_ipaddr, sock->my_port, &salocal, &salen)) {
		close(this->fd);
		return -1;
	}

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	if (sock->my_ipaddr.af == AF_INET6) {
		/*
		 *	Listening on '::' does NOT get you IPv4 to
		 *	IPv6 mapping.  You've got to listen on an IPv4
		 *	address, too.  This makes the rest of the server
		 *	design a little simpler.
		 */
#ifdef IPV6_V6ONLY

		if (IN6_IS_ADDR_UNSPECIFIED(&sock->my_ipaddr.ipaddr.ip6addr)) {
			int on = 1;

			if (setsockopt(this->fd, IPPROTO_IPV6, IPV6_V6ONLY,
				       (char *)&on, sizeof(on)) < 0) {
				ERROR("Failed setting socket to IPv6 "
				       "only: %s", strerror(errno));

		       		close(this->fd);
				return -1;
			}
		}
#endif /* IPV6_V6ONLY */
	}
#endif /* HAVE_STRUCT_SOCKADDR_IN6 */

	if (sock->my_ipaddr.af == AF_INET) {
		UNUSED int flag;

#if defined(IP_MTU_DISCOVER) && defined(IP_PMTUDISC_DONT)
		/*
		 *	Disable PMTU discovery.  On Linux, this
		 *	also makes sure that the "don't fragment"
		 *	flag is zero.
		 */
		flag = IP_PMTUDISC_DONT;
		if (setsockopt(this->fd, IPPROTO_IP, IP_MTU_DISCOVER,
			       &flag, sizeof(flag)) < 0) {
			ERROR("Failed disabling PMTU discovery: %s",
			       strerror(errno));

			close(this->fd);
			return -1;
		}
#endif

#if defined(IP_DONTFRAG)
		/*
		 *	Ensure that the "don't fragment" flag is zero.
		 */
		flag = 0;
		if (setsockopt(this->fd, IPPROTO_IP, IP_DONTFRAG,
			       &flag, sizeof(flag)) < 0) {
			ERROR("Failed setting don't fragment flag: %s",
			       strerror(errno));

			close(this->fd);
			return -1;
		}
#endif
	}

#ifdef WITH_DHCP
#ifdef SO_BROADCAST
	if (sock->broadcast) {
		int on = 1;

		if (setsockopt(this->fd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
			ERROR("Can't set broadcast option: %s",
			       strerror(errno));
			return -1;
		}
	}
#endif
#endif

	/*
	 *	May be binding to priviledged ports.
	 */
	if (sock->my_port != 0) {
#ifdef SO_REUSEADDR
		int on = 1;

		if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
			ERROR("Can't set re-use address option: %s",
			       strerror(errno));
			return -1;
		}
#endif

		fr_suid_up();
		rcode = bind(this->fd, (struct sockaddr *) &salocal, salen);
		fr_suid_down();
		if (rcode < 0) {
			char buffer[256];
			close(this->fd);

			this->print(this, buffer, sizeof(buffer));
			ERROR("Failed binding to %s: %s\n",
			       buffer, strerror(errno));
			return -1;
		}

		/*
		 *	FreeBSD jail issues.  We bind to 0.0.0.0, but the
		 *	kernel instead binds us to a 1.2.3.4.  If this
		 *	happens, notice, and remember our real IP.
		 */
		{
			struct sockaddr_storage	src;
			socklen_t		sizeof_src = sizeof(src);

			memset(&src, 0, sizeof_src);
			if (getsockname(this->fd, (struct sockaddr *) &src,
					&sizeof_src) < 0) {
				ERROR("Failed getting socket name: %s",
				       strerror(errno));
				return -1;
			}

			if (!fr_sockaddr2ipaddr(&src, sizeof_src,
						&sock->my_ipaddr, &sock->my_port)) {
				ERROR("Socket has unsupported address family");
				return -1;
			}
		}
	}

#ifdef WITH_TCP
	if (sock->proto == IPPROTO_TCP) {
		if (listen(this->fd, 8) < 0) {
			close(this->fd);
			ERROR("Failed in listen(): %s", strerror(errno));
			return -1;
		}
	} else
#endif

	  if (!this->workers && fr_nonblock(this->fd) < 0) {
		  close(this->fd);
		  ERROR("Failed setting non-blocking on socket: %s",
			 strerror(errno));
		  return -1;
	  }

	/*
	 *	Mostly for proxy sockets.
	 */
	sock->other_ipaddr.af = sock->my_ipaddr.af;

/*
 *	Don't screw up other people.
 */
#undef proto_for_port
#undef sock_type

	return 0;
}


static int listener_free(void *ctx)
{
	rad_listen_t *this;

	this = talloc_get_type_abort(ctx, rad_listen_t);

	/*
	 *	Other code may have eaten the FD.
	 */
	if (this->fd >= 0) close(this->fd);

	if (master_listen[this->type].free) {
		master_listen[this->type].free(this);
	}

#ifdef WITH_TCP
	if ((this->type == RAD_LISTEN_AUTH)
#ifdef WITH_ACCT
	    || (this->type == RAD_LISTEN_ACCT)
#endif
#ifdef WITH_PROXY
	    || (this->type == RAD_LISTEN_PROXY)
#endif
#ifdef WITH_COMMAND_SOCKET
	    || ((this->type == RAD_LISTEN_COMMAND) &&
		(((fr_command_socket_t *) this->data)->magic != COMMAND_SOCKET_MAGIC))
#endif
		) {
		listen_socket_t *sock = this->data;

#ifdef WITH_TLS
		if (sock->request) {
			pthread_mutex_destroy(&(sock->mutex));
			request_free(&sock->request);
			sock->packet = NULL;

			if (sock->ssn) session_free(sock->ssn);
			request_free(&sock->request);
		} else
#endif
			rad_free(&sock->packet);
	}
#endif				/* WITH_TCP */

	return 0;
}

/*
 *	Allocate & initialize a new listener.
 */
static rad_listen_t *listen_alloc(TALLOC_CTX *ctx, RAD_LISTEN_TYPE type)
{
	rad_listen_t *this;

	this = talloc_zero(ctx, rad_listen_t);

	this->type = type;
	this->recv = master_listen[this->type].recv;
	this->send = master_listen[this->type].send;
	this->print = master_listen[this->type].print;
	this->encode = master_listen[this->type].encode;
	this->decode = master_listen[this->type].decode;

	talloc_set_destructor((void *) this, listener_free);

	this->data = talloc_zero_array(this, uint8_t, master_listen[this->type].inst_size);

	return this;
}

#ifdef WITH_PROXY
/*
 *	Externally visible function for creating a new proxy LISTENER.
 *
 *	Not thread-safe, but all calls to it are protected by the
 *	proxy mutex in event.c
 */
rad_listen_t *proxy_new_listener(home_server *home, int src_port)
{
	rad_listen_t *this;
	listen_socket_t *sock;
	char buffer[256];

	if (!home) return NULL;

	if ((home->limit.max_connections > 0) &&
	    (home->limit.num_connections >= home->limit.max_connections)) {
		WDEBUG("Home server has too many open connections (%d)",
		      home->limit.max_connections);
		return NULL;
	}

	this = listen_alloc(mainconfig.config, RAD_LISTEN_PROXY);

	sock = this->data;
	sock->other_ipaddr = home->ipaddr;
	sock->other_port = home->port;
	sock->home = home;

	sock->my_ipaddr = home->src_ipaddr;
	sock->my_port = src_port;
	sock->proto = home->proto;

	if (debug_flag >= 2) {
		this->print(this, buffer, sizeof(buffer));
		DEBUG("Opening new %s", buffer);
	}

#ifdef WITH_TCP
	sock->opened = sock->last_packet = time(NULL);

	if (home->proto == IPPROTO_TCP) {
		this->recv = proxy_socket_tcp_recv;

		/*
		 *	FIXME: connect() is blocking!
		 *	We do this with the proxy mutex locked, which may
		 *	cause large delays!
		 *
		 *	http://www.developerweb.net/forum/showthread.php?p=13486
		 */
		this->fd = fr_tcp_client_socket(&home->src_ipaddr,
						&home->ipaddr, home->port);
#ifdef WITH_TLS
		if (home->tls) {
			DEBUG("Trying SSL to port %d\n", home->port);
			sock->ssn = tls_new_client_session(home->tls, this->fd);
			if (!sock->ssn) {
				listen_free(&this);
				return NULL;
			}

			this->recv = proxy_tls_recv;
			this->send = proxy_tls_send;
		}
#endif
	} else
#endif
		this->fd = fr_socket(&home->src_ipaddr, src_port);

	if (this->fd < 0) {
		this->print(this, buffer,sizeof(buffer));
		DEBUG("Failed opening client socket ::%s:: : %s",
		      buffer, fr_strerror());
		listen_free(&this);
		return NULL;
	}

	/*
	 *	Figure out which port we were bound to.
	 */
	if (sock->my_port == 0) {
		struct sockaddr_storage	src;
		socklen_t		sizeof_src = sizeof(src);

		memset(&src, 0, sizeof_src);
		if (getsockname(this->fd, (struct sockaddr *) &src,
				&sizeof_src) < 0) {
			ERROR("Failed getting socket name: %s",
			       strerror(errno));
			listen_free(&this);
			return NULL;
		}

		if (!fr_sockaddr2ipaddr(&src, sizeof_src,
					&sock->my_ipaddr, &sock->my_port)) {
			ERROR("Socket has unsupported address family");
			listen_free(&this);
			return NULL;
		}
	}

	return this;
}
#endif


static const FR_NAME_NUMBER listen_compare[] = {
#ifdef WITH_STATS
	{ "status",	RAD_LISTEN_NONE },
#endif
	{ "auth",	RAD_LISTEN_AUTH },
#ifdef WITH_ACCOUNTING
	{ "acct",	RAD_LISTEN_ACCT },
#endif
#ifdef WITH_DETAIL
	{ "detail",	RAD_LISTEN_DETAIL },
#endif
#ifdef WITH_PROXY
	{ "proxy",	RAD_LISTEN_PROXY },
#endif
#ifdef WITH_VMPS
	{ "vmps",	RAD_LISTEN_VQP },
#endif
#ifdef WITH_DHCP
	{ "dhcp",	RAD_LISTEN_DHCP },
#endif
#ifdef WITH_COMMAND_SOCKET
	{ "control",	RAD_LISTEN_COMMAND },
#endif
#ifdef WITH_COA
	{ "coa",	RAD_LISTEN_COA },
#endif
	{ NULL, 0 },
};


static rad_listen_t *listen_parse(CONF_SECTION *cs, char const *server)
{
	int		type, rcode;
	char		*listen_type;
	rad_listen_t	*this;
	CONF_PAIR	*cp;
	char const	*value;
	lt_dlhandle	handle;
	char		buffer[32];

	cp = cf_pair_find(cs, "type");
	if (!cp) {
		cf_log_err_cs(cs,
			   "No type specified in listen section");
		return NULL;
	}

	value = cf_pair_value(cp);
	if (!value) {
		cf_log_err_cp(cp,
			      "Type cannot be empty");
		return NULL;
	}

	snprintf(buffer, sizeof(buffer), "proto_%s", value);
	handle = lt_dlopenext(buffer);
	if (handle) {
		fr_protocol_t	*proto;

		proto = dlsym(handle, buffer);
		if (!proto) {
			cf_log_err_cs(cs,
				      "Failed linking to protocol %s : %s\n",
				      value, dlerror());
			dlclose(handle);
			return NULL;
		}

		type = fr_str2int(listen_compare, value, -1);
		rad_assert(type >= 0); /* shouldn't be able to compile an invalid type */

		memcpy(&master_listen[type], proto, sizeof(*proto));

		/*
		 *	And throw away the handle.
		 *	@todo: fix it later
		 */

		if (master_listen[type].magic !=  RLM_MODULE_INIT) {
			ERROR("Failed to load protocol '%s' due to internal sanity check problem",
			       master_listen[type].name);
			return NULL;
		}
	}

	cf_log_info(cs, "listen {");

	listen_type = NULL;
	rcode = cf_item_parse(cs, "type", PW_TYPE_STRING_PTR,
			      &listen_type, "");
	if (rcode < 0) return NULL;
	if (rcode == 1) {
		cf_log_err_cs(cs,
			   "No type specified in listen section");
		return NULL;
	}

	type = fr_str2int(listen_compare, listen_type, -1);
	if (type < 0) {
		cf_log_err_cs(cs,
			   "Invalid type \"%s\" in listen section.",
			   listen_type);
		return NULL;
	}

	/*
	 *	Allow listen sections in the default config to
	 *	refer to a server.
	 */
	if (!server) {
		rcode = cf_item_parse(cs, "virtual_server", PW_TYPE_STRING_PTR,
				      &server, NULL);
		if (rcode == 1) { /* compatiblity with 2.0-pre */
			rcode = cf_item_parse(cs, "server", PW_TYPE_STRING_PTR,
					      &server, NULL);
		}
		if (rcode < 0) return NULL;
	}

#ifdef WITH_PROXY
	/*
	 *	We were passed a virtual server, so the caller is
	 *	defining a proxy listener inside of a virtual server.
	 *	This isn't allowed right now.
	 */
	else if (type == RAD_LISTEN_PROXY) {
		ERROR("Error: listen type \"proxy\" Cannot appear in a virtual server section");
		return NULL;
	}
#endif

	/*
	 *	Set up cross-type data.
	 */
	this = listen_alloc(cs, type);
	this->server = server;
	this->fd = -1;

	/*
	 *	Call per-type parser.
	 */
	if (master_listen[type].parse(cs, this) < 0) {
		listen_free(&this);
		return NULL;
	}

	cf_log_info(cs, "}");

	return this;
}

#ifdef WITH_PROXY
static int is_loopback(fr_ipaddr_t const *ipaddr)
{
	/*
	 *	We shouldn't proxy on loopback.
	 */
	if ((ipaddr->af == AF_INET) &&
	    (ipaddr->ipaddr.ip4addr.s_addr == htonl(INADDR_LOOPBACK))) {
		return 1;
	}

#ifdef HAVE_STRUCT_SOCKADDR_IN6
	if ((ipaddr->af == AF_INET6) &&
	    (IN6_IS_ADDR_LINKLOCAL(&ipaddr->ipaddr.ip6addr))) {
		return 1;
	}
#endif

	return 0;
}
#endif


#ifdef HAVE_PTHREAD_H
/*
 *	A child thread which does NOTHING other than read and process
 *	packets.
 */
static void *recv_thread(void *arg)
{
	rad_listen_t *this = arg;

	while (1) {
		this->recv(this);
		DEBUG("%p", &this);
	}

	return NULL;
}
#endif


/*
 *	Generate a list of listeners.  Takes an input list of
 *	listeners, too, so we don't close sockets with waiting packets.
 */
int listen_init(CONF_SECTION *config, rad_listen_t **head,
#ifdef WITH_TLS
	        int spawn_flag
#else
		UNUSED int spawn_flag
#endif
	        )

{
	int		override = false;
	CONF_SECTION	*cs = NULL;
	rad_listen_t	**last;
	rad_listen_t	*this;
	fr_ipaddr_t	server_ipaddr;
	int		auth_port = 0;
#ifdef WITH_PROXY
	int		defined_proxy = 0;
#endif

	/*
	 *	We shouldn't be called with a pre-existing list.
	 */
	rad_assert(head && (*head == NULL));

	memset(&server_ipaddr, 0, sizeof(server_ipaddr));

	last = head;
	server_ipaddr.af = AF_UNSPEC;

	/*
	 *	If the port is specified on the command-line,
	 *	it over-rides the configuration file.
	 *
	 *	FIXME: If argv[0] == "vmpsd", then don't listen on auth/acct!
	 */
	if (mainconfig.port >= 0) {
		auth_port = mainconfig.port;

		/*
		 *	-p X but no -i Y on the command-line.
		 */
		if ((mainconfig.port > 0) &&
		    (mainconfig.myip.af == AF_UNSPEC)) {
			ERROR("The command-line says \"-p %d\", but there is no associated IP address to use",
			       mainconfig.port);
			return -1;
		}
	}

	/*
	 *	If the IP address was configured on the command-line,
	 *	use that as the "bind_address"
	 */
	if (mainconfig.myip.af != AF_UNSPEC) {
		listen_socket_t *sock;

		memcpy(&server_ipaddr, &mainconfig.myip,
		       sizeof(server_ipaddr));
		override = true;

#ifdef WITH_VMPS
		if (strcmp(progname, "vmpsd") == 0) {
			this = listen_alloc(config, RAD_LISTEN_VQP);
			if (!auth_port) auth_port = 1589;
		} else
#endif
			this = listen_alloc(config, RAD_LISTEN_AUTH);

		sock = this->data;

		sock->my_ipaddr = server_ipaddr;
		sock->my_port = auth_port;

		sock->clients = clients_parse_section(config);
		if (!sock->clients) {
			cf_log_err_cs(config,
				   "Failed to find any clients for this listen section");
			listen_free(&this);
			return -1;
		}

		if (listen_bind(this) < 0) {
			listen_free(head);
			ERROR("There appears to be another RADIUS server running on the authentication port %d", sock->my_port);
			listen_free(&this);
			return -1;
		}
		auth_port = sock->my_port;	/* may have been updated in listen_bind */
		if (override) {
			cs = cf_section_sub_find_name2(config, "server",
						       mainconfig.name);
			if (cs) this->server = mainconfig.name;
		}

		*last = this;
		last = &(this->next);

#ifdef WITH_VMPS
		/*
		 *	No acct for vmpsd
		 */
		if (strcmp(progname, "vmpsd") == 0) goto add_sockets;
#endif

#ifdef WITH_ACCOUNTING
		/*
		 *	Open Accounting Socket.
		 *
		 *	If we haven't already gotten acct_port from
		 *	/etc/services, then make it auth_port + 1.
		 */
		this = listen_alloc(config, RAD_LISTEN_ACCT);
		sock = this->data;

		/*
		 *	Create the accounting socket.
		 *
		 *	The accounting port is always the
		 *	authentication port + 1
		 */
		sock->my_ipaddr = server_ipaddr;
		sock->my_port = auth_port + 1;

		sock->clients = clients_parse_section(config);
		if (!sock->clients) {
			cf_log_err_cs(config,
				   "Failed to find any clients for this listen section");
			return -1;
		}

		if (listen_bind(this) < 0) {
			listen_free(&this);
			listen_free(head);
			ERROR("There appears to be another RADIUS server running on the accounting port %d", sock->my_port);
			return -1;
		}

		if (override) {
			cs = cf_section_sub_find_name2(config, "server",
						       mainconfig.name);
			if (cs) this->server = mainconfig.name;
		}

		*last = this;
		last = &(this->next);
#endif
	}

	/*
	 *	They specified an IP on the command-line, ignore
	 *	all listen sections except the one in '-n'.
	 */
	if (mainconfig.myip.af != AF_UNSPEC) {
		CONF_SECTION *subcs;
		char const *name2 = cf_section_name2(cs);

		cs = cf_section_sub_find_name2(config, "server",
					       mainconfig.name);
		if (!cs) goto add_sockets;

		/*
		 *	Should really abstract this code...
		 */
		for (subcs = cf_subsection_find_next(cs, NULL, "listen");
		     subcs != NULL;
		     subcs = cf_subsection_find_next(cs, subcs, "listen")) {
			this = listen_parse(subcs, name2);
			if (!this) {
				listen_free(head);
				return -1;
			}

			*last = this;
			last = &(this->next);
		} /* loop over "listen" directives in server <foo> */

		goto add_sockets;
	}

	/*
	 *	Walk through the "listen" sections, if they exist.
	 */
	for (cs = cf_subsection_find_next(config, NULL, "listen");
	     cs != NULL;
	     cs = cf_subsection_find_next(config, cs, "listen")) {
		this = listen_parse(cs, NULL);
		if (!this) {
			listen_free(head);
			return -1;
		}

		*last = this;
		last = &(this->next);
	}

	/*
	 *	Check virtual servers for "listen" sections, too.
	 *
	 *	FIXME: Move to virtual server init?
	 */
	for (cs = cf_subsection_find_next(config, NULL, "server");
	     cs != NULL;
	     cs = cf_subsection_find_next(config, cs, "server")) {
		CONF_SECTION *subcs;
		char const *name2 = cf_section_name2(cs);

		for (subcs = cf_subsection_find_next(cs, NULL, "listen");
		     subcs != NULL;
		     subcs = cf_subsection_find_next(cs, subcs, "listen")) {
			this = listen_parse(subcs, name2);
			if (!this) {
				listen_free(head);
				return -1;
			}

			*last = this;
			last = &(this->next);
		} /* loop over "listen" directives in virtual servers */
	} /* loop over virtual servers */

add_sockets:
	/*
	 *	No sockets to receive packets, this is an error.
	 *	proxying is pointless.
	 */
	if (!*head) {
		ERROR("The server is not configured to listen on any ports.  Cannot start.");
		return -1;
	}

	/*
	 *	Print out which sockets we're listening on, and
	 *	add them to the event list.
	 */
	for (this = *head; this != NULL; this = this->next) {
#ifdef WITH_PROXY
		if (this->type == RAD_LISTEN_PROXY) {
			defined_proxy = 1;
		}

#endif

#ifdef WITH_TLS
		if (!spawn_flag && this->tls) {
			cf_log_err_cs(this->cs, "Threading must be enabled for TLS sockets to function properly.");
			cf_log_err_cs(this->cs, "You probably need to do 'radiusd -fxx -l stdout' for debugging");
			return -1;
		}
#endif
		if (!check_config) {
			if (this->workers && !spawn_flag) {
				WARN("Setting 'workers' requires 'synchronous'.  Disabling 'workers'");
				this->workers = 0;
			}

			if (this->workers) {
#ifndef HAVE_PTHREAD_H
				WARN("Setting 'workers' requires 'synchronous'.  Disabling 'workers'");
				this->workers = 0;
#else
				int i, rcode;
				char buffer[256];

				this->print(this, buffer, sizeof(buffer));

				for (i = 0; i < this->workers; i++) {
					pthread_t id;

					/*
					 *	FIXME: create detached?
					 */
					rcode = pthread_create(&id, 0, recv_thread, this);
					if (rcode != 0) {
						ERROR("Thread create failed: %s",
						      strerror(rcode));
						fr_exit(1);
					}

					DEBUG("Thread %d for %s\n", i, buffer);
				}
#endif
			} else
				event_new_fd(this);

		}
	}

	/*
	 *	If we're proxying requests, open the proxy FD.
	 *	Otherwise, don't do anything.
	 */
#ifdef WITH_PROXY
	if ((mainconfig.proxy_requests == true) &&
	    !check_config &&
	    (*head != NULL) && !defined_proxy) {
		listen_socket_t *sock = NULL;
		int		port = 0;
		home_server	home;

		memset(&home, 0, sizeof(home));

		/*
		 *
		 */
		home.proto = IPPROTO_UDP;
		home.src_ipaddr = server_ipaddr;

		/*
		 *	Find the first authentication port,
		 *	and use it
		 */
		for (this = *head; this != NULL; this = this->next) {
			switch (this->type) {
			case RAD_LISTEN_AUTH:
				sock = this->data;

				if (is_loopback(&sock->my_ipaddr)) continue;

				if (home.src_ipaddr.af == AF_UNSPEC) {
					home.src_ipaddr = sock->my_ipaddr;
				}
				port = sock->my_port + 2;
				break;
#ifdef WITH_ACCT
			case RAD_LISTEN_ACCT:
				sock = this->data;

				if (is_loopback(&sock->my_ipaddr)) continue;

				if (home.src_ipaddr.af == AF_UNSPEC) {
					home.src_ipaddr = sock->my_ipaddr;
				}
				port = sock->my_port + 1;
				break;
#endif
			default:
				break;
			}
		}

		/*
		 *	Address is still unspecified, use IPv4.
		 */
		if (home.src_ipaddr.af == AF_UNSPEC) {
			home.src_ipaddr.af = AF_INET;
			/* everything else is already set to zero */
		}

		home.ipaddr.af = home.src_ipaddr.af;
		/* everything else is already set to zero */

		this = proxy_new_listener(&home, port);
		if (!this) {
			listen_free(head);
			return -1;
		}

		if (!event_new_fd(this)) {
			listen_free(&this);
			listen_free(head);
			return -1;
		}
	}
#endif

	/*
	 *	Haven't defined any sockets.  Die.
	 */
	if (!*head) return -1;

	xlat_register("listen", xlat_listen, NULL, NULL);

	return 0;
}

/*
 *	Free a linked list of listeners;
 */
void listen_free(rad_listen_t **head)
{
	rad_listen_t *this;

	if (!head || !*head) return;

	this = *head;
	while (this) {
		rad_listen_t *next = this->next;
		talloc_free(this);
		this = next;
	}

	*head = NULL;
}

#ifdef WITH_STATS
RADCLIENT_LIST *listener_find_client_list(fr_ipaddr_t const *ipaddr,
					  int port)
{
	rad_listen_t *this;

	for (this = mainconfig.listen; this != NULL; this = this->next) {
		listen_socket_t *sock;

		if ((this->type != RAD_LISTEN_AUTH)
#ifdef WITH_ACCOUNTING
		    && (this->type != RAD_LISTEN_ACCT)
#endif
		    ) continue;

		sock = this->data;

		if ((sock->my_port == port) &&
		    (fr_ipaddr_cmp(ipaddr, &sock->my_ipaddr) == 0)) {
			return sock->clients;
		}
	}

	return NULL;
}
#endif

rad_listen_t *listener_find_byipaddr(fr_ipaddr_t const *ipaddr, int port, int proto)
{
	rad_listen_t *this;

	for (this = mainconfig.listen; this != NULL; this = this->next) {
		listen_socket_t *sock;

		sock = this->data;

		if (sock->my_port != port) continue;
		if (sock->proto != proto) continue;
		if (fr_ipaddr_cmp(ipaddr, &sock->my_ipaddr) != 0) continue;

		return this;
	}

	/*
	 *	Failed to find a specific one.  Find INADDR_ANY
	 */
	for (this = mainconfig.listen; this != NULL; this = this->next) {
		listen_socket_t *sock;

		sock = this->data;

		if (sock->my_port != port) continue;
		if (sock->proto != proto) continue;
		if (!fr_inaddr_any(&sock->my_ipaddr)) continue;

		return this;
	}

	return NULL;
}
