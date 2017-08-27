/*
 * mem.c  Memory allocation, deallocation stuff.
 *
 * Version:     $Id$
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
 * Copyright 2000,2001,2006  The FreeRADIUS server project
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 */

RCSID("$Id$")

#include <stdio.h>
#include "rlm_eap.h"

#ifdef HAVE_PTHREAD_H
#define PTHREAD_MUTEX_LOCK pthread_mutex_lock
#define PTHREAD_MUTEX_UNLOCK pthread_mutex_unlock
#else
#define PTHREAD_MUTEX_LOCK(_x)
#define PTHREAD_MUTEX_UNLOCK(_x)
#endif

/*
 * Allocate a new eap_packet_t
 */
EAP_DS *eap_ds_alloc(eap_handler_t *handler)
{
	EAP_DS	*eap_ds;

	eap_ds = talloc_zero(handler, EAP_DS);
	eap_ds->response = talloc_zero(eap_ds, eap_packet_t);
	if (!eap_ds->response) {
		eap_ds_free(&eap_ds);
		return NULL;
	}
	eap_ds->request = talloc_zero(eap_ds, eap_packet_t);
	if (!eap_ds->response) {
		eap_ds_free(&eap_ds);
		return NULL;
	}

	return eap_ds;
}

void eap_ds_free(EAP_DS **eap_ds_p)
{
	EAP_DS *eap_ds;

	if (!eap_ds_p) return;

	eap_ds = *eap_ds_p;
	if (!eap_ds) return;

	if (eap_ds->response) talloc_free(eap_ds->response);
	if (eap_ds->request) talloc_free(eap_ds->request);

	talloc_free(eap_ds);
	*eap_ds_p = NULL;
}

static int _eap_handler_free(eap_handler_t *handler)
{
	rlm_eap_t *inst = handler->inst_holder;

	if (handler->identity) {
		talloc_free(handler->identity);
		handler->identity = NULL;
	}

	if (handler->prev_eapds) eap_ds_free(&(handler->prev_eapds));
	if (handler->eap_ds) eap_ds_free(&(handler->eap_ds));

	if ((handler->opaque) && (handler->free_opaque)) {
		handler->free_opaque(handler->opaque);
		handler->opaque = NULL;
	}

	handler->opaque = NULL;
	handler->free_opaque = NULL;

	if (handler->certs) pairfree(&handler->certs);

	PTHREAD_MUTEX_LOCK(&(inst->handler_mutex));
	if (inst->handler_tree) {
		rbtree_deletebydata(inst->handler_tree, handler);
	}
	PTHREAD_MUTEX_UNLOCK(&(inst->handler_mutex));

	return 0;
}

/*
 * Allocate a new eap_handler_t
 */
eap_handler_t *eap_handler_alloc(rlm_eap_t *inst)
{
	eap_handler_t	*handler;

	PTHREAD_MUTEX_LOCK(&(inst->handler_mutex));
	handler = talloc_zero(inst, eap_handler_t);

	if (inst->handler_tree) {
		rbtree_insert(inst->handler_tree, handler);
	}

	handler->inst_holder = inst;
	talloc_set_destructor(handler, _eap_handler_free);
	PTHREAD_MUTEX_UNLOCK(&(inst->handler_mutex));

	return handler;
}

typedef struct check_handler_t {
	rlm_eap_t	*inst;
	eap_handler_t	*handler;
	int		trips;
} check_handler_t;

static int check_opaque_free(check_handler_t *check)
{
	int do_warning = false;
	uint8_t state[8];

	if (!check->inst || !check->handler) {
		return 0;
	}

	if (!check->inst->handler_tree) goto done;

	PTHREAD_MUTEX_LOCK(&(check->inst->handler_mutex));
	if (!rbtree_finddata(check->inst->handler_tree, check->handler)) {
		goto done;
	}

	/*
	 *	The session has continued *after* this packet.
	 *	Don't do a warning.
	 */
	if (check->handler->trips > check->trips) {
		goto done;
	}

	/*
	 *	No TLS means no warnings.
	 */
	if (!check->handler->tls) goto done;

	/*
	 *	If we're being deleted early, it's likely because we
	 *	received a transmit from the client that re-uses the
	 *	same RADIUS Id, which forces the current packet to be
	 *	deleted.  In that case, ignore the error.
	 */
	if (time(NULL) < (check->handler->timestamp + 3)) goto done;

	if (!check->handler->finished) {
		do_warning = true;
		memcpy(state, check->handler->state, sizeof(state));
	}

done:
	PTHREAD_MUTEX_UNLOCK(&(check->inst->handler_mutex));

	if (do_warning) {
		WDEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		WDEBUG("!! EAP session with state 0x%02x%02x%02x%02x%02x%02x%02x%02x did not finish!  !!",
		      state[0], state[1],
		      state[2], state[3],
		      state[4], state[5],
		      state[6], state[7]);

		WDEBUG("!! Please read http://wiki.freeradius.org/guide/Certificate_Compatibility     !!");
		WDEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}

	return 0;
}

void eaplist_free(rlm_eap_t *inst)
{
	eap_handler_t *node, *next;

       	for (node = inst->session_head; node != NULL; node = next) {
		next = node->next;
		talloc_free(node);
	}

	inst->session_head = inst->session_tail = NULL;
}

/*
 *	Return a 32-bit random number.
 */
static uint32_t eap_rand(fr_randctx *ctx)
{
	uint32_t num;

	num = ctx->randrsl[ctx->randcnt++];
	if (ctx->randcnt >= 256) {
		ctx->randcnt = 0;
		fr_isaac(ctx);
	}

	return num;
}


static eap_handler_t *eaplist_delete(rlm_eap_t *inst, REQUEST *request,
				   eap_handler_t *handler)
{
	rbnode_t *node;

	node = rbtree_find(inst->session_tree, handler);
	if (!node) return NULL;

	handler = rbtree_node2data(inst->session_tree, node);

	RDEBUG("Finished EAP session with state "
	       "0x%02x%02x%02x%02x%02x%02x%02x%02x",
	       handler->state[0], handler->state[1],
	       handler->state[2], handler->state[3],
	       handler->state[4], handler->state[5],
	       handler->state[6], handler->state[7]);
	/*
	 *	Delete old handler from the tree.
	 */
	rbtree_delete(inst->session_tree, node);

	/*
	 *	And unsplice it from the linked list.
	 */
	if (handler->prev) {
		handler->prev->next = handler->next;
	} else {
		inst->session_head = handler->next;
	}
	if (handler->next) {
		handler->next->prev = handler->prev;
	} else {
		inst->session_tail = handler->prev;
	}
	handler->prev = handler->next = NULL;

	return handler;
}


static void eaplist_expire(rlm_eap_t *inst, REQUEST *request, time_t timestamp)
{
	int i;
	eap_handler_t *handler;

	/*
	 *	Check the first few handlers in the list, and delete
	 *	them if they're too old.  We don't need to check them
	 *	all, as incoming requests will quickly cause older
	 *	handlers to be deleted.
	 *
	 */
	for (i = 0; i < 3; i++) {
		handler = inst->session_head;
		if (!handler) break;

		RDEBUG("Expiring EAP session with state "
		       "0x%02x%02x%02x%02x%02x%02x%02x%02x",
		       handler->state[0], handler->state[1],
		       handler->state[2], handler->state[3],
		       handler->state[4], handler->state[5],
		       handler->state[6], handler->state[7]);

		/*
		 *	Expire entries from the start of the list.
		 *	They should be the oldest ones.
		 */
		if ((timestamp - handler->timestamp) > inst->timer_limit) {
			rbnode_t *node;
			node = rbtree_find(inst->session_tree, handler);
			rad_assert(node != NULL);
			rbtree_delete(inst->session_tree, node);

			/*
			 *	handler == inst->session_head
			 */
			inst->session_head = handler->next;
			if (handler->next) {
				handler->next->prev = NULL;
			} else {
				inst->session_head = NULL;
				inst->session_tail = NULL;
			}
			talloc_free(handler);
		} else {
			break;
		}
	}
}

/*
 *	Add a handler to the set of active sessions.
 *
 *	Since we're adding it to the list, we guess that this means
 *	the packet needs a State attribute.  So add one.
 */
int eaplist_add(rlm_eap_t *inst, eap_handler_t *handler)
{
	int		status = 0;
	VALUE_PAIR	*state;
	REQUEST		*request = handler->request;

	rad_assert(handler != NULL);
	rad_assert(request != NULL);

	/*
	 *	Generate State, since we've been asked to add it to
	 *	the list.
	 */
	state = pairmake_reply("State", NULL, T_OP_EQ);
	if (!state) return 0;

	/*
	 *	The time at which this request was made was the time
	 *	at which it was received by the RADIUS server.
	 */
	handler->timestamp = request->timestamp;
	handler->status = 1;

	handler->src_ipaddr = request->packet->src_ipaddr;
	handler->eap_id = handler->eap_ds->request->id;

	/*
	 *	Playing with a data structure shared among threads
	 *	means that we need a lock, to avoid conflict.
	 */
	PTHREAD_MUTEX_LOCK(&(inst->session_mutex));

	/*
	 *	If we have a DoS attack, discard new sessions.
	 */
	if (rbtree_num_elements(inst->session_tree) >= inst->max_sessions) {
		status = -1;
		eaplist_expire(inst, request, handler->timestamp);
		goto done;
	}

	/*
	 *	Create a unique content for the State variable.
	 *	It will be modified slightly per round trip, but less so
	 *	than in 1.x.
	 */
	if (handler->trips == 0) {
		int i;

		for (i = 0; i < 4; i++) {
			uint32_t lvalue;

			lvalue = eap_rand(&inst->rand_pool);

			memcpy(handler->state + i * 4, &lvalue,
			       sizeof(lvalue));
		}
	}

	/*
	 *	Add some more data to distinguish the sessions.
	 */
	handler->state[4] = handler->trips ^ handler->state[0];
	handler->state[5] = handler->eap_id ^ handler->state[1];
	handler->state[6] = handler->type ^ handler->state[2];

	pairmemcpy(state, handler->state, sizeof(handler->state));

	/*
	 *	Big-time failure.
	 */
	status = rbtree_insert(inst->session_tree, handler);

	/*
	 *	Catch Access-Challenge without response.
	 */
	if (inst->handler_tree) {
		check_handler_t *check = talloc(handler, check_handler_t);

		check->inst = inst;
		check->handler = handler;
		check->trips = handler->trips;

		talloc_set_destructor(check, check_opaque_free);
		request_data_add(request, inst, 0, check, true);
	}

	if (status) {
		eap_handler_t *prev;

		prev = inst->session_tail;
		if (prev) {
			prev->next = handler;
			handler->prev = prev;
			handler->next = NULL;
			inst->session_tail = handler;
		} else {
			inst->session_head = inst->session_tail = handler;
			handler->next = handler->prev = NULL;
		}
	}

	/*
	 *	Now that we've finished mucking with the list,
	 *	unlock it.
	 */
 done:

	/*
	 *	We don't need this any more.
	 */
	if (status > 0) handler->request = NULL;

	PTHREAD_MUTEX_UNLOCK(&(inst->session_mutex));

	if (status <= 0) {
		pairfree(&state);

		if (status < 0) {
			static time_t last_logged = 0;

			if (last_logged < handler->timestamp) {
				last_logged = handler->timestamp;
				ERROR("rlm_eap (%s): Too many open sessions. Try increasing \"max_sessions\" "
				      "in the EAP module configuration", inst->xlat_name);
			}
		} else {
			ERROR("rlm_eap (%s): Failed to store handler", inst->xlat_name);
		}
		return 0;
	}

	RDEBUG("New EAP session, adding 'State' attribute to reply 0x%02x%02x%02x%02x%02x%02x%02x%02x",
	       state->vp_octets[0], state->vp_octets[1], state->vp_octets[2], state->vp_octets[3],
	       state->vp_octets[4], state->vp_octets[5], state->vp_octets[6], state->vp_octets[7]);

	return 1;
}

/*
 *	Find a a previous EAP-Request sent by us, which matches
 *	the current EAP-Response.
 *
 *	Then, release the handle from the list, and return it to
 *	the caller.
 *
 *	Also since we fill the eap_ds with the present EAP-Response we
 *	got to free the prev_eapds & move the eap_ds to prev_eapds
 */
eap_handler_t *eaplist_find(rlm_eap_t *inst, REQUEST *request,
			  eap_packet_raw_t *eap_packet)
{
	VALUE_PAIR	*state;
	eap_handler_t	*handler, myHandler;

	/*
	 *	We key the sessions off of the 'state' attribute, so it
	 *	must exist.
	 */
	state = pairfind(request->packet->vps, PW_STATE, 0, TAG_ANY);
	if (!state ||
	    (state->length != EAP_STATE_LEN)) {
		return NULL;
	}

	myHandler.src_ipaddr = request->packet->src_ipaddr;
	myHandler.eap_id = eap_packet->id;
	memcpy(myHandler.state, state->vp_strvalue, sizeof(myHandler.state));

	/*
	 *	Playing with a data structure shared among threads
	 *	means that we need a lock, to avoid conflict.
	 */
	PTHREAD_MUTEX_LOCK(&(inst->session_mutex));

	eaplist_expire(inst, request, request->timestamp);

	handler = eaplist_delete(inst, request, &myHandler);
	PTHREAD_MUTEX_UNLOCK(&(inst->session_mutex));

	/*
	 *	Might not have been there.
	 */
	if (!handler) {
		ERROR("rlm_eap (%s): No EAP session matching state "
		       "0x%02x%02x%02x%02x%02x%02x%02x%02x",
		       inst->xlat_name,
		       state->vp_octets[0], state->vp_octets[1],
		       state->vp_octets[2], state->vp_octets[3],
		       state->vp_octets[4], state->vp_octets[5],
		       state->vp_octets[6], state->vp_octets[7]);
		return NULL;
	}

	if (handler->trips >= 50) {
		ERROR("rlm_eap (%s): Aborting! More than 50 roundtrips "
		       "made in session with state "
		       "0x%02x%02x%02x%02x%02x%02x%02x%02x",
		       inst->xlat_name,
		       state->vp_octets[0], state->vp_octets[1],
		       state->vp_octets[2], state->vp_octets[3],
		       state->vp_octets[4], state->vp_octets[5],
		       state->vp_octets[6], state->vp_octets[7]);


		talloc_free(handler);
		return NULL;
	}
	handler->trips++;

	RDEBUG("Previous EAP request found for state "
	       "0x%02x%02x%02x%02x%02x%02x%02x%02x, released from the list",
		state->vp_octets[0], state->vp_octets[1],
		state->vp_octets[2], state->vp_octets[3],
		state->vp_octets[4], state->vp_octets[5],
		state->vp_octets[6], state->vp_octets[7]);

	/*
	 *	Remember what the previous request was.
	 */
	eap_ds_free(&(handler->prev_eapds));
	handler->prev_eapds = handler->eap_ds;
	handler->eap_ds = NULL;

	return handler;
}
