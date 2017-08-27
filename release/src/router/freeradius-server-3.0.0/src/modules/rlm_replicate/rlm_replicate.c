/*
 *   This program is is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2 if the
 *   License as published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file rlm_replicate.c
 * @brief Duplicate RADIUS requests.
 *
 * @copyright 2011-2013  The FreeRADIUS server project
 */
RCSID("$Id$")

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>

#ifdef WITH_PROXY
static void cleanup(RADIUS_PACKET *packet)
{
	if (!packet) return;
	if (packet->sockfd >= 0) {
		close(packet->sockfd);
	}

	rad_free(&packet);
}

/** Copy packet to multiple servers
 *
 * Create a duplicate of the packet and send it to a list of realms
 * defined by the presence of the Replicate-To-Realm VP in the control
 * list of the current request.
 *
 * This is pretty hacky and is 100% fire and forget. If you're looking
 * to forward authentication requests to multiple realms and process
 * the responses, this function will not allow you to do that.
 *
 * @param[in] instance 	of this module.
 * @param[in] request 	The current request.
 * @param[in] list	of attributes to copy to the duplicate packet.
 * @param[in] code	to write into the code field of the duplicate packet.
 * @return RCODE fail on error, invalid if list does not exist, noop if no replications succeeded, else ok.
 */
static int replicate_packet(UNUSED void *instance, REQUEST *request, pair_lists_t list, unsigned int code)
{
	int rcode = RLM_MODULE_NOOP;
	VALUE_PAIR *vp, **vps;
	vp_cursor_t cursor;
	home_server *home;
	REALM *realm;
	home_pool_t *pool;
	RADIUS_PACKET *packet = NULL;

	/*
	 *	Send as many packets as necessary to different
	 *	destinations.
	 */
	paircursor(&cursor, &request->config_items);
	while ((vp = pairfindnext(&cursor, PW_REPLICATE_TO_REALM, 0, TAG_ANY))) {
		realm = realm_find2(vp->vp_strvalue);
		if (!realm) {
			REDEBUG2("Cannot Replicate to unknown realm \"%s\"", vp->vp_strvalue);
			continue;
		}

		/*
		 *	We shouldn't really do this on every loop.
		 */
		switch (request->packet->code) {
		default:
			REDEBUG2("Cannot replicate unknown packet code %d", request->packet->code);
			cleanup(packet);
			return RLM_MODULE_FAIL;

		case PW_AUTHENTICATION_REQUEST:
			pool = realm->auth_pool;
			break;

#ifdef WITH_ACCOUNTING

		case PW_ACCOUNTING_REQUEST:
			pool = realm->acct_pool;
			break;
#endif

#ifdef WITH_COA
		case PW_COA_REQUEST:
		case PW_DISCONNECT_REQUEST:
			pool = realm->acct_pool;
			break;
#endif
		}

		if (!pool) {
			RWDEBUG2("Cancelling replication to Realm %s, as the realm is local", realm->name);
			continue;
		}

		home = home_server_ldb(realm->name, pool, request);
		if (!home) {
			REDEBUG2("Failed to find live home server for realm %s", realm->name);
			continue;
		}

		/*
		 *	For replication to multiple servers we re-use the packet
		 *	we built here.
		 */
		if (!packet) {
			packet = rad_alloc(request, 1);
			if (!packet) {
				return RLM_MODULE_FAIL;
			}

			packet->code = code;
			packet->id = fr_rand() & 0xff;
			packet->sockfd = fr_socket(&home->src_ipaddr, 0);
			if (packet->sockfd < 0) {
				REDEBUG("Failed opening socket: %s", fr_strerror());
				rcode = RLM_MODULE_FAIL;
				goto done;
			}

			vps = radius_list(request, list);
			if (!vps) {
				RWDEBUG("List '%s' doesn't exist for this packet",
					fr_int2str(pair_lists, list, "<INVALID>"));
				rcode = RLM_MODULE_INVALID;
				goto done;
			}

			/*
			 *	Don't assume the list actually contains any
			 *	attributes.
			 */
			if (*vps) {
				packet->vps = paircopy(packet, *vps);
				if (!packet->vps) {
					rcode = RLM_MODULE_FAIL;
					goto done;
				}
			}

			/*
			 *	For CHAP, create the CHAP-Challenge if
			 *	it doesn't exist.
			 */
			if ((code == PW_AUTHENTICATION_REQUEST) &&
			    (pairfind(request->packet->vps, PW_CHAP_PASSWORD, 0, TAG_ANY) != NULL) &&
			    (pairfind(request->packet->vps, PW_CHAP_CHALLENGE, 0, TAG_ANY) == NULL)) {
				uint8_t *p;
				vp = radius_paircreate(request, &packet->vps, PW_CHAP_CHALLENGE, 0);
				vp->length = AUTH_VECTOR_LEN;
				vp->vp_octets = p = talloc_array(vp, uint8_t, vp->length);
				memcpy(p, request->packet->vector, AUTH_VECTOR_LEN);
			}
		} else {
			size_t i;

			for (i = 0; i < sizeof(packet->vector); i++) {
				packet->vector[i] = fr_rand() & 0xff;
			}

			packet->id++;
			talloc_free(packet->data);
			packet->data = NULL;
			packet->data_len = 0;
		}

		/*
		 *	(Re)-Write these.
		 */
		packet->dst_ipaddr = home->ipaddr;
		packet->dst_port = home->port;
		memset(&packet->src_ipaddr, 0, sizeof(packet->src_ipaddr));
		packet->src_port = 0;

		/*
		 *	Encode, sign and then send the packet.
		 */
		RDEBUG("Replicating list '%s' to Realm '%s'", fr_int2str(pair_lists, list, "<INVALID>"),
		       realm->name);
		if (rad_send(packet, NULL, home->secret) < 0) {
			REDEBUG("Failed replicating packet: %s", fr_strerror());
			rcode = RLM_MODULE_FAIL;
			goto done;
		}

		/*
		 *	We've sent it to at least one destination.
		 */
		rcode = RLM_MODULE_OK;
	}

	done:

	cleanup(packet);
	return rcode;
}
#else
static rlm_rcode_t replicate_packet(void *instance, REQUEST *request, pair_lists_t list, unsigned int code)
{
	RDEBUG("Replication is unsupported in this build");
	return RLM_MODULE_FAIL;
}
#endif

static rlm_rcode_t mod_authorize(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_REQUEST, request->packet->code);
}

static rlm_rcode_t mod_preaccounting(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_REQUEST, request->packet->code);
}

static rlm_rcode_t mod_accounting(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_REPLY, request->reply->code);
}

static rlm_rcode_t mod_pre_proxy(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_PROXY_REQUEST, request->proxy->code);
}

static rlm_rcode_t mod_post_proxy(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_PROXY_REPLY, request->proxy_reply->code);
}

static rlm_rcode_t mod_post_auth(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_REPLY, request->reply->code);
}

static rlm_rcode_t mod_recv_coa(void *instance, REQUEST *request)
{
	return replicate_packet(instance, request, PAIR_LIST_REQUEST, request->packet->code);
}

/*
 *	The module name should be the only globally exported symbol.
 *	That is, everything else should be 'static'.
 *
 *	If the module needs to temporarily modify it's instantiation
 *	data, the type should be changed to RLM_TYPE_THREAD_UNSAFE.
 *	The server will then take care of ensuring that the module
 *	is single-threaded.
 */
module_t rlm_replicate = {
	RLM_MODULE_INIT,
	"replicate",
	RLM_TYPE_THREAD_SAFE,		/* type */
	0,
	NULL,				/* CONF_PARSER */
	NULL,				/* instantiation */
	NULL,				/* detach */
	{
		NULL,			/* authentication */
		mod_authorize,		/* authorization */
		mod_preaccounting,	/* preaccounting */
		mod_accounting,		/* accounting */
		NULL,			/* checksimul */
		mod_pre_proxy,		/* pre-proxy */
		mod_post_proxy,		/* post-proxy */
		mod_post_auth		/* post-auth */
#ifdef WITH_COA
		, mod_recv_coa,		/* coa-request */
		NULL
#endif
	},
};
