/*
 * stats.c	Internal statistics handling.
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
 * Copyright 2008  The FreeRADIUS server project
 * Copyright 2008  Alan DeKok <aland@deployingradius.com>
 */

RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/rad_assert.h>

#ifdef WITH_STATS

#define USEC (1000000)
#define EMA_SCALE (100)
#define PREC (USEC * EMA_SCALE)

#define F_EMA_SCALE (1000000)

static struct timeval	start_time;
static struct timeval	hup_time;

#define FR_STATS_INIT { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	\
				 { 0, 0, 0, 0, 0, 0, 0, 0 }}

fr_stats_t radius_auth_stats = FR_STATS_INIT;
#ifdef WITH_ACCOUNTING
fr_stats_t radius_acct_stats = FR_STATS_INIT;
#endif
#ifdef WITH_COA
fr_stats_t radius_coa_stats = FR_STATS_INIT;
fr_stats_t radius_dsc_stats = FR_STATS_INIT;
#endif

#ifdef WITH_PROXY
fr_stats_t proxy_auth_stats = FR_STATS_INIT;
#ifdef WITH_ACCOUNTING
fr_stats_t proxy_acct_stats = FR_STATS_INIT;
#endif
#ifdef WITH_COA
fr_stats_t proxy_coa_stats = FR_STATS_INIT;
fr_stats_t proxy_dsc_stats = FR_STATS_INIT;
#endif
#endif


static void tv_sub(struct timeval *end, struct timeval *start,
		   struct timeval *elapsed)
{
	elapsed->tv_sec = end->tv_sec - start->tv_sec;
	if (elapsed->tv_sec > 0) {
		elapsed->tv_sec--;
		elapsed->tv_usec = USEC;
	} else {
		elapsed->tv_usec = 0;
	}
	elapsed->tv_usec += end->tv_usec;
	elapsed->tv_usec -= start->tv_usec;

	if (elapsed->tv_usec >= USEC) {
		elapsed->tv_usec -= USEC;
		elapsed->tv_sec++;
	}
}

static void stats_time(fr_stats_t *stats, struct timeval *start,
		       struct timeval *end)
{
	struct timeval diff;
	uint32_t delay;

	if ((start->tv_sec == 0) || (end->tv_sec == 0) ||
	    (end->tv_sec < start->tv_sec)) return;

	tv_sub(end, start, &diff);

	if (diff.tv_sec >= 10) {
		stats->elapsed[7]++;
	} else {
		int i;
		uint32_t cmp;

		delay = (diff.tv_sec * USEC) + diff.tv_usec;

		cmp = 10;
		for (i = 0; i < 7; i++) {
			if (delay < cmp) {
				stats->elapsed[i]++;
				break;
			}
			cmp *= 10;
		}
	}
}

void request_stats_final(REQUEST *request)
{
	if (request->master_state == REQUEST_COUNTED) return;

	if (!request->listener) return;
	if (!request->client) return;

	if ((request->listener->type != RAD_LISTEN_NONE) &&
#ifdef WITH_ACCOUNTING
	    (request->listener->type != RAD_LISTEN_ACCT) &&
#endif
#ifdef WITH_COA
	    (request->listener->type != RAD_LISTEN_COA) &&
#endif
	    (request->listener->type != RAD_LISTEN_AUTH)) return;

	/* don't count statistic requests */
	if (request->packet->code == PW_STATUS_SERVER)
		return;

#undef INC_AUTH
#define INC_AUTH(_x) radius_auth_stats._x++;request->listener->stats._x++;request->client->auth._x++;


#undef INC_ACCT
#ifdef WITH_ACCOUNTING
#define INC_ACCT(_x) radius_acct_stats._x++;request->listener->stats._x++;request->client->acct._x++
#else
#define INC_ACCT(_x)
#endif

#undef INC_COA
#ifdef WITH_COA
#define INC_COA(_x) radius_coa_stats._x++;request->listener->stats._x++;request->client->coa._x++
#else
#define INC_COA(_x)
#endif

#undef INC_DSC
#ifdef WITH_DSC
#define INC_DSC(_x) radius_dsc_stats._x++;request->listener->stats._x++;request->client->dsc._x++
#else
#define INC_DSC(_x)
#endif

	/*
	 *	Update the statistics.
	 *
	 *	Note that we do NOT do this in a child thread.
	 *	Instead, we update the stats when a request is
	 *	deleted, because only the main server thread calls
	 *	this function, which makes it thread-safe.
	 */
	if (request->reply) switch (request->reply->code) {
	case PW_AUTHENTICATION_ACK:
		INC_AUTH(total_access_accepts);

		auth_stats:
		INC_AUTH(total_responses);

		/*
		 *	FIXME: Do the time calculations once...
		 */
		stats_time(&radius_auth_stats,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		stats_time(&request->client->auth,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		stats_time(&request->listener->stats,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		break;

	case PW_AUTHENTICATION_REJECT:
		INC_AUTH(total_access_rejects);
		goto auth_stats;

	case PW_ACCESS_CHALLENGE:
		INC_AUTH(total_access_challenges);
		goto auth_stats;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_RESPONSE:
		INC_ACCT(total_responses);
		stats_time(&radius_acct_stats,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		stats_time(&request->client->acct,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		break;
#endif

#ifdef WITH_COA
	case PW_COA_ACK:
		INC_COA(total_access_accepts);
	  coa_stats:
		INC_COA(total_responses);
		stats_time(&request->client->coa,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		break;

	case PW_COA_NAK:
		INC_COA(total_access_rejects);
		goto coa_stats;

	case PW_DISCONNECT_ACK:
		INC_DSC(total_access_accepts);
	  dsc_stats:
		INC_DSC(total_responses);
		stats_time(&request->client->dsc,
			   &request->packet->timestamp,
			   &request->reply->timestamp);
		break;

	case PW_DISCONNECT_NAK:
		INC_DSC(total_access_rejects);
		goto dsc_stats;
#endif

		/*
		 *	No response, it must have been a bad
		 *	authenticator.
		 */
	case 0:
		if (request->packet->code == PW_AUTHENTICATION_REQUEST) {
			if (request->reply->offset == -2) {
				INC_AUTH(total_bad_authenticators);
			} else {
				INC_AUTH(total_packets_dropped);
			}
		} else if (request->packet->code == PW_ACCOUNTING_REQUEST) {
			if (request->reply->offset == -2) {
				INC_ACCT(total_bad_authenticators);
			} else {
				INC_ACCT(total_packets_dropped);
			}
		}
		break;

	default:
		break;
	}

#ifdef WITH_PROXY
	if (!request->proxy || !request->proxy_listener) goto done;	/* simplifies formatting */

	switch (request->proxy->code) {
	case PW_AUTHENTICATION_REQUEST:
		proxy_auth_stats.total_requests += request->num_proxied_requests;
		request->proxy_listener->stats.total_requests += request->num_proxied_requests;
		request->home_server->stats.total_requests += request->num_proxied_requests;
		break;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_REQUEST:
		proxy_acct_stats.total_requests++;
		request->proxy_listener->stats.total_requests += request->num_proxied_requests;
		request->home_server->stats.total_requests += request->num_proxied_requests;
		break;
#endif

	default:
		break;
	}

	if (!request->proxy_reply) goto done;	/* simplifies formatting */

#undef INC
#define INC(_x) proxy_auth_stats._x += request->num_proxied_responses; request->proxy_listener->stats._x += request->num_proxied_responses; request->home_server->stats._x += request->num_proxied_responses;

	switch (request->proxy_reply->code) {
	case PW_AUTHENTICATION_ACK:
		INC(total_access_accepts);
	proxy_stats:
		INC(total_responses);
		stats_time(&proxy_auth_stats,
			   &request->proxy->timestamp,
			   &request->proxy_reply->timestamp);
		stats_time(&request->home_server->stats,
			   &request->proxy->timestamp,
			   &request->proxy_reply->timestamp);
		break;

	case PW_AUTHENTICATION_REJECT:
		INC(total_access_rejects);
		goto proxy_stats;

	case PW_ACCESS_CHALLENGE:
		INC(total_access_challenges);
		goto proxy_stats;

#ifdef WITH_ACCOUNTING
	case PW_ACCOUNTING_RESPONSE:
		proxy_acct_stats.total_responses++;
		request->proxy_listener->stats.total_responses++;
		request->home_server->stats.total_responses++;
		stats_time(&proxy_acct_stats,
			   &request->proxy->timestamp,
			   &request->proxy_reply->timestamp);
		stats_time(&request->home_server->stats,
			   &request->proxy->timestamp,
			   &request->proxy_reply->timestamp);
		break;
#endif

	default:
		proxy_auth_stats.total_unknown_types++;
		request->proxy_listener->stats.total_unknown_types++;
		request->home_server->stats.total_unknown_types++;
		break;
	}

 done:
#endif /* WITH_PROXY */

	request->master_state = REQUEST_COUNTED;
}

typedef struct fr_stats2vp {
	int	attribute;
	size_t	offset;
} fr_stats2vp;

/*
 *	Authentication
 */
static fr_stats2vp authvp[] = {
	{ 128, offsetof(fr_stats_t, total_requests) },
	{ 129, offsetof(fr_stats_t, total_access_accepts) },
	{ 130, offsetof(fr_stats_t, total_access_rejects) },
	{ 131, offsetof(fr_stats_t, total_access_challenges) },
	{ 132, offsetof(fr_stats_t, total_responses) },
	{ 133, offsetof(fr_stats_t, total_dup_requests) },
	{ 134, offsetof(fr_stats_t, total_malformed_requests) },
	{ 135, offsetof(fr_stats_t, total_bad_authenticators) },
	{ 136, offsetof(fr_stats_t, total_packets_dropped) },
	{ 137, offsetof(fr_stats_t, total_unknown_types) },
	{ 0, 0 }
};


#ifdef WITH_PROXY
/*
 *	Proxied authentication requests.
 */
static fr_stats2vp proxy_authvp[] = {
	{ 138, offsetof(fr_stats_t, total_requests) },
	{ 139, offsetof(fr_stats_t, total_access_accepts) },
	{ 140, offsetof(fr_stats_t, total_access_rejects) },
	{ 141, offsetof(fr_stats_t, total_access_challenges) },
	{ 142, offsetof(fr_stats_t, total_responses) },
	{ 143, offsetof(fr_stats_t, total_dup_requests) },
	{ 144, offsetof(fr_stats_t, total_malformed_requests) },
	{ 145, offsetof(fr_stats_t, total_bad_authenticators) },
	{ 146, offsetof(fr_stats_t, total_packets_dropped) },
	{ 147, offsetof(fr_stats_t, total_unknown_types) },
	{ 0, 0 }
};
#endif


#ifdef WITH_ACCOUNTING
/*
 *	Accounting
 */
static fr_stats2vp acctvp[] = {
	{ 148, offsetof(fr_stats_t, total_requests) },
	{ 149, offsetof(fr_stats_t, total_responses) },
	{ 150, offsetof(fr_stats_t, total_dup_requests) },
	{ 151, offsetof(fr_stats_t, total_malformed_requests) },
	{ 152, offsetof(fr_stats_t, total_bad_authenticators) },
	{ 153, offsetof(fr_stats_t, total_packets_dropped) },
	{ 154, offsetof(fr_stats_t, total_unknown_types) },
	{ 0, 0 }
};

#ifdef WITH_PROXY
static fr_stats2vp proxy_acctvp[] = {
	{ 155, offsetof(fr_stats_t, total_requests) },
	{ 156, offsetof(fr_stats_t, total_responses) },
	{ 157, offsetof(fr_stats_t, total_dup_requests) },
	{ 158, offsetof(fr_stats_t, total_malformed_requests) },
	{ 159, offsetof(fr_stats_t, total_bad_authenticators) },
	{ 160, offsetof(fr_stats_t, total_packets_dropped) },
	{ 161, offsetof(fr_stats_t, total_unknown_types) },
	{ 0, 0 }
};
#endif
#endif

static fr_stats2vp client_authvp[] = {
	{ 128, offsetof(fr_stats_t, total_requests) },
	{ 129, offsetof(fr_stats_t, total_access_accepts) },
	{ 130, offsetof(fr_stats_t, total_access_rejects) },
	{ 131, offsetof(fr_stats_t, total_access_challenges) },
	{ 132, offsetof(fr_stats_t, total_responses) },
	{ 133, offsetof(fr_stats_t, total_dup_requests) },
	{ 134, offsetof(fr_stats_t, total_malformed_requests) },
	{ 135, offsetof(fr_stats_t, total_bad_authenticators) },
	{ 136, offsetof(fr_stats_t, total_packets_dropped) },
	{ 137, offsetof(fr_stats_t, total_unknown_types) },
	{ 0, 0 }
};

#ifdef WITH_ACCOUNTING
static fr_stats2vp client_acctvp[] = {
	{ 148, offsetof(fr_stats_t, total_requests) },
	{ 149, offsetof(fr_stats_t, total_responses) },
	{ 150, offsetof(fr_stats_t, total_dup_requests) },
	{ 151, offsetof(fr_stats_t, total_malformed_requests) },
	{ 152, offsetof(fr_stats_t, total_bad_authenticators) },
	{ 153, offsetof(fr_stats_t, total_packets_dropped) },
	{ 154, offsetof(fr_stats_t, total_unknown_types) },
	{ 0, 0 }
};
#endif

static void request_stats_addvp(REQUEST *request,
				fr_stats2vp *table, fr_stats_t *stats)
{
	int i;
	fr_uint_t counter;
	VALUE_PAIR *vp;

	for (i = 0; table[i].attribute != 0; i++) {
		vp = radius_paircreate(request, &request->reply->vps,
				       table[i].attribute, VENDORPEC_FREERADIUS);
		if (!vp) continue;

		counter = *(fr_uint_t *) (((uint8_t *) stats) + table[i].offset);
		vp->vp_integer = counter;
	}
}


void request_stats_reply(REQUEST *request)
{
	VALUE_PAIR *flag, *vp;

	/*
	 *	Statistics are available ONLY on a "status" port.
	 */
	rad_assert(request->packet->code == PW_STATUS_SERVER);
	rad_assert(request->listener->type == RAD_LISTEN_NONE);

	flag = pairfind(request->packet->vps, 127, VENDORPEC_FREERADIUS, TAG_ANY);
	if (!flag || (flag->vp_integer == 0)) return;

	/*
	 *	Authentication.
	 */
	if (((flag->vp_integer & 0x01) != 0) &&
	    ((flag->vp_integer & 0xc0) == 0)) {
		request_stats_addvp(request, authvp, &radius_auth_stats);
	}

#ifdef WITH_ACCOUNTING
	/*
	 *	Accounting
	 */
	if (((flag->vp_integer & 0x02) != 0) &&
	    ((flag->vp_integer & 0xc0) == 0)) {
		request_stats_addvp(request, acctvp, &radius_acct_stats);
	}
#endif

#ifdef WITH_PROXY
	/*
	 *	Proxied authentication requests.
	 */
	if (((flag->vp_integer & 0x04) != 0) &&
	    ((flag->vp_integer & 0x20) == 0)) {
		request_stats_addvp(request, proxy_authvp, &proxy_auth_stats);
	}

#ifdef WITH_ACCOUNTING
	/*
	 *	Proxied accounting requests.
	 */
	if (((flag->vp_integer & 0x08) != 0) &&
	    ((flag->vp_integer & 0x20) == 0)) {
		request_stats_addvp(request, proxy_acctvp, &proxy_acct_stats);
	}
#endif
#endif

	/*
	 *	Internal server statistics
	 */
	if ((flag->vp_integer & 0x10) != 0) {
		vp = radius_paircreate(request, &request->reply->vps,
				       176, VENDORPEC_FREERADIUS);
		if (vp) vp->vp_date = start_time.tv_sec;
		vp = radius_paircreate(request, &request->reply->vps,
				       177, VENDORPEC_FREERADIUS);
		if (vp) vp->vp_date = hup_time.tv_sec;

#ifdef HAVE_PTHREAD_H
		int i, array[RAD_LISTEN_MAX], pps[2];

		thread_pool_queue_stats(array, pps);

		for (i = 0; i <= 4; i++) {
			vp = radius_paircreate(request, &request->reply->vps,
					       162 + i, VENDORPEC_FREERADIUS);

			if (!vp) continue;
			vp->vp_integer = array[i];
		}

		for (i = 0; i < 2; i++) {
			vp = radius_paircreate(request, &request->reply->vps,
					       181 + i, VENDORPEC_FREERADIUS);

			if (!vp) continue;
			vp->vp_integer = pps[i];
		}
#endif
	}

	/*
	 *	For a particular client.
	 */
	if ((flag->vp_integer & 0x20) != 0) {
		fr_ipaddr_t ipaddr;
		VALUE_PAIR *server_ip, *server_port = NULL;
		RADCLIENT *client = NULL;
		RADCLIENT_LIST *cl = NULL;

		/*
		 *	See if we need to look up the client by server
		 *	socket.
		 */
		server_ip = pairfind(request->packet->vps, 170, VENDORPEC_FREERADIUS, TAG_ANY);
		if (server_ip) {
			server_port = pairfind(request->packet->vps, 171, VENDORPEC_FREERADIUS, TAG_ANY);

			if (server_port) {
				ipaddr.af = AF_INET;
				ipaddr.ipaddr.ip4addr.s_addr = server_ip->vp_ipaddr;
				cl = listener_find_client_list(&ipaddr, server_port->vp_integer);

				/*
				 *	Not found: don't do anything
				 */
				if (!cl) return;
			}
		}


		vp = pairfind(request->packet->vps, 167, VENDORPEC_FREERADIUS, TAG_ANY);
		if (vp) {
			memset(&ipaddr, 0, sizeof(ipaddr));
			ipaddr.af = AF_INET;
			ipaddr.ipaddr.ip4addr.s_addr = vp->vp_ipaddr;
			client = client_find(cl, &ipaddr, IPPROTO_UDP);
#ifdef WITH_TCP
			if (!client) {
				client = client_find(cl, &ipaddr, IPPROTO_TCP);
			}
#endif

			/*
			 *	Else look it up by number.
			 */
		} else if ((vp = pairfind(request->packet->vps, 168, VENDORPEC_FREERADIUS, TAG_ANY)) != NULL) {
			client = client_findbynumber(cl, vp->vp_integer);
		}

		if (client) {
			/*
			 *	If found, echo it back, along with
			 *	the requested statistics.
			 */
			pairadd(&request->reply->vps, paircopyvp(request->reply, vp));

			/*
			 *	When retrieving client by number, also
			 *	echo back it's IP address.
			 */
			if ((vp->da->type == PW_TYPE_INTEGER) &&
			    (client->ipaddr.af == AF_INET)) {
				vp = radius_paircreate(request,
						       &request->reply->vps,
						       167, VENDORPEC_FREERADIUS);
				if (vp) {
					vp->vp_ipaddr = client->ipaddr.ipaddr.ip4addr.s_addr;
				}

				if (client->prefix != 32) {
					vp = radius_paircreate(request,
							       &request->reply->vps,
							       169, VENDORPEC_FREERADIUS);
					if (vp) {
						vp->vp_integer = client->prefix;
					}
				}
			}

			if (server_ip) {
				pairadd(&request->reply->vps,
					paircopyvp(request->reply, server_ip));
			}
			if (server_port) {
				pairadd(&request->reply->vps,
					paircopyvp(request->reply, server_port));
			}

			if ((flag->vp_integer & 0x01) != 0) {
				request_stats_addvp(request, client_authvp,
						    &client->auth);
			}
#ifdef WITH_ACCOUNTING
			if ((flag->vp_integer & 0x01) != 0) {
				request_stats_addvp(request, client_acctvp,
						    &client->acct);
			}
#endif
		} /* else client wasn't found, don't echo it back */
	}

	/*
	 *	For a particular "listen" socket.
	 */
	if (((flag->vp_integer & 0x40) != 0) &&
	    ((flag->vp_integer & 0x03) != 0)) {
		rad_listen_t *this;
		VALUE_PAIR *server_ip, *server_port;
		fr_ipaddr_t ipaddr;

		/*
		 *	See if we need to look up the server by socket
		 *	socket.
		 */
		server_ip = pairfind(request->packet->vps, 170, VENDORPEC_FREERADIUS, TAG_ANY);
		if (!server_ip) return;

		server_port = pairfind(request->packet->vps, 171, VENDORPEC_FREERADIUS, TAG_ANY);
		if (!server_port) return;

		ipaddr.af = AF_INET;
		ipaddr.ipaddr.ip4addr.s_addr = server_ip->vp_ipaddr;
		this = listener_find_byipaddr(&ipaddr,
					      server_port->vp_integer,
					      IPPROTO_UDP);

		/*
		 *	Not found: don't do anything
		 */
		if (!this) return;

		pairadd(&request->reply->vps,
			paircopyvp(request->reply, server_ip));
		pairadd(&request->reply->vps,
			paircopyvp(request->reply, server_port));

		if (((flag->vp_integer & 0x01) != 0) &&
		    ((request->listener->type == RAD_LISTEN_AUTH) ||
		     (request->listener->type == RAD_LISTEN_NONE))) {
			request_stats_addvp(request, authvp, &this->stats);
		}

#ifdef WITH_ACCOUNTING
		if (((flag->vp_integer & 0x02) != 0) &&
		    ((request->listener->type == RAD_LISTEN_ACCT) ||
		     (request->listener->type == RAD_LISTEN_NONE))) {
			request_stats_addvp(request, acctvp, &this->stats);
		}
#endif
	}

#ifdef WITH_PROXY
	/*
	 *	Home servers.
	 */
	if (((flag->vp_integer & 0x80) != 0) &&
	    ((flag->vp_integer & 0x03) != 0)) {
		home_server *home;
		VALUE_PAIR *server_ip, *server_port;
		fr_ipaddr_t ipaddr;

		/*
		 *	See if we need to look up the server by socket
		 *	socket.
		 */
		server_ip = pairfind(request->packet->vps, 170, VENDORPEC_FREERADIUS, TAG_ANY);
		if (!server_ip) return;

		server_port = pairfind(request->packet->vps, 171, VENDORPEC_FREERADIUS, TAG_ANY);
		if (!server_port) return;

#ifndef NDEBUG
		memset(&ipaddr, 0, sizeof(ipaddr));
#endif
		ipaddr.af = AF_INET;
		ipaddr.ipaddr.ip4addr.s_addr = server_ip->vp_ipaddr;
		home = home_server_find(&ipaddr, server_port->vp_integer,
					IPPROTO_UDP);

		/*
		 *	Not found: don't do anything
		 */
		if (!home) return;

		pairadd(&request->reply->vps,
			paircopyvp(request->reply, server_ip));
		pairadd(&request->reply->vps,
			paircopyvp(request->reply, server_port));

		vp = radius_paircreate(request, &request->reply->vps,
				       172, VENDORPEC_FREERADIUS);
		if (vp) vp->vp_integer = home->currently_outstanding;

		vp = radius_paircreate(request, &request->reply->vps,
				       173, VENDORPEC_FREERADIUS);
		if (vp) vp->vp_integer = home->state;

		if ((home->state == HOME_STATE_ALIVE) &&
		    (home->revive_time.tv_sec != 0)) {
			vp = radius_paircreate(request, &request->reply->vps,
					       175, VENDORPEC_FREERADIUS);
			if (vp) vp->vp_date = home->revive_time.tv_sec;
		}

		if ((home->state == HOME_STATE_ALIVE) &&
		    (home->ema.window > 0)) {
				vp = radius_paircreate(request,
						       &request->reply->vps,
						       178, VENDORPEC_FREERADIUS);
				if (vp) vp->vp_integer = home->ema.window;
				vp = radius_paircreate(request,
						       &request->reply->vps,
						       179, VENDORPEC_FREERADIUS);
				if (vp) vp->vp_integer = home->ema.ema1 / EMA_SCALE;
				vp = radius_paircreate(request,
						       &request->reply->vps,
						       180, VENDORPEC_FREERADIUS);
				if (vp) vp->vp_integer = home->ema.ema10 / EMA_SCALE;

		}

		if (home->state == HOME_STATE_IS_DEAD) {
			vp = radius_paircreate(request, &request->reply->vps,
					       174, VENDORPEC_FREERADIUS);
			if (vp) vp->vp_date = home->zombie_period_start.tv_sec + home->zombie_period;
		}

		/*
		 *	Show more information...
		 *
		 *	FIXME: do this for clients, too!
		 */
		vp = radius_paircreate(request, &request->reply->vps,
				       184, VENDORPEC_FREERADIUS);
		if (vp) vp->vp_date = home->last_packet_recv;

		vp = radius_paircreate(request, &request->reply->vps,
				       185, VENDORPEC_FREERADIUS);
		if (vp) vp->vp_date = home->last_packet_sent;

		if (((flag->vp_integer & 0x01) != 0) &&
		    (home->type == HOME_TYPE_AUTH)) {
			request_stats_addvp(request, proxy_authvp,
					    &home->stats);
		}

#ifdef WITH_ACCOUNTING
		if (((flag->vp_integer & 0x02) != 0) &&
		    (home->type == HOME_TYPE_ACCT)) {
			request_stats_addvp(request, proxy_acctvp,
					    &home->stats);
		}
#endif
	}
#endif	/* WITH_PROXY */
}

void radius_stats_init(int flag)
{
	if (!flag) {
		gettimeofday(&start_time, NULL);
		hup_time = start_time; /* it's just nicer this way */
	} else {
		gettimeofday(&hup_time, NULL);
	}
}

void radius_stats_ema(fr_stats_ema_t *ema,
		      struct timeval *start, struct timeval *end)
{
	int micro;
	time_t tdiff;
#ifdef WITH_STATS_DEBUG
	static int n = 0;
#endif
	if (ema->window == 0) return;

	rad_assert(start->tv_sec <= end->tv_sec);

	/*
	 *	Initialize it.
	 */
	if (ema->f1 == 0) {
		if (ema->window > 10000) ema->window = 10000;

		ema->f1 =  (2 * F_EMA_SCALE) / (ema->window + 1);
		ema->f10 = (2 * F_EMA_SCALE) / ((10 * ema->window) + 1);
	}


	tdiff = start->tv_sec;
	tdiff -= end->tv_sec;

	micro = (int) tdiff;
	if (micro > 40) micro = 40; /* don't overflow 32-bit ints */
	micro *= USEC;
	micro += start->tv_usec;
	micro -= end->tv_usec;

	micro *= EMA_SCALE;

	if (ema->ema1 == 0) {
		ema->ema1 = micro;
		ema->ema10 = micro;
	} else {
		int diff;

		diff = ema->f1 * (micro - ema->ema1);
		ema->ema1 += (diff / 1000000);

		diff = ema->f10 * (micro - ema->ema10);
		ema->ema10 += (diff / 1000000);
	}


#ifdef WITH_STATS_DEBUG
	DEBUG("time %d %d.%06d\t%d.%06d\t%d.%06d\n",
	      n, micro / PREC, (micro / EMA_SCALE) % USEC,
	      ema->ema1 / PREC, (ema->ema1 / EMA_SCALE) % USEC,
	      ema->ema10 / PREC, (ema->ema10 / EMA_SCALE) % USEC);
	n++;
#endif
}

#endif /* WITH_STATS */
