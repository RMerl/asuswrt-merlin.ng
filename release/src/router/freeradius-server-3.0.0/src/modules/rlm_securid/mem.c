/*
 *	mem.c  Session handling, mostly taken from src/modules/rlm_eap/mem.c
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
 * Copyright 2012  The FreeRADIUS server project
 * Copyright 2012  Alan DeKok <aland@networkradius.com>
 */

#include <stdio.h>
#include "rlm_securid.h"

static void		securid_sessionlist_clean_expired(rlm_securid_t *inst, REQUEST *request, time_t timestamp);

static SECURID_SESSION* securid_sessionlist_delete(rlm_securid_t *inst,
						   SECURID_SESSION *session);

SECURID_SESSION* securid_session_alloc(void)
{
	SECURID_SESSION	*session;

	session = rad_malloc(sizeof(SECURID_SESSION));
	memset(session, 0, sizeof(SECURID_SESSION));

	session->sdiHandle = SDI_HANDLE_NONE;

	return session;
}

void securid_session_free(UNUSED rlm_securid_t *inst,REQUEST *request,
			  SECURID_SESSION *session)
{
	if (!session)
		return;

	RDEBUG2("Freeing session id=%d identity='%s' state='%s'",
			 session->session_id,SAFE_STR(session->identity),session->state);

	if (session->identity) {
		free(session->identity);
		session->identity = NULL;
	}
	if (session->pin) {
		free(session->pin);
		session->pin = NULL;
	}

	if (session->sdiHandle != SDI_HANDLE_NONE) {
		SD_Close(session->sdiHandle);
		session->sdiHandle = SDI_HANDLE_NONE;
	}

	free(session);
}


void securid_sessionlist_free(rlm_securid_t *inst,REQUEST *request)
{
	SECURID_SESSION *node, *next;

	pthread_mutex_lock(&(inst->session_mutex));

       	for (node = inst->session_head; node != NULL; node = next) {
		next = node->next;
		securid_session_free(inst,request,node);
	}

	inst->session_head = inst->session_tail = NULL;

	pthread_mutex_unlock(&(inst->session_mutex));
}



/*
 *	Add a session to the set of active sessions.
 *
 *	Since we're adding it to the list, we guess that this means
 *	the packet needs a State attribute.  So add one.
 */
int securid_sessionlist_add(rlm_securid_t *inst,REQUEST *request,
			    SECURID_SESSION *session)
{
	int		status = 0;
	VALUE_PAIR	*state;

	rad_assert(session != NULL);
	rad_assert(request != NULL);

	/*
	 *	The time at which this request was made was the time
	 *	at which it was received by the RADIUS server.
	 */
	session->timestamp = request->timestamp;

	session->src_ipaddr = request->packet->src_ipaddr;

	/*
	 *	Playing with a data structure shared among threads
	 *	means that we need a lock, to avoid conflict.
	 */
	pthread_mutex_lock(&(inst->session_mutex));

	/*
	 *	If we have a DoS attack, discard new sessions.
	 */
	if (rbtree_num_elements(inst->session_tree) >= inst->max_sessions) {
		securid_sessionlist_clean_expired(inst, request, session->timestamp);
		goto done;
	}

	if (session->session_id == 0) {
		/* this is a NEW session (we are not inserting an updated session) */
		inst->last_session_id++;
		session->session_id = inst->last_session_id;
		RDEBUG2("Creating a new session with id=%d\n",session->session_id);
	}
	snprintf(session->state,sizeof(session->state)-1,"FRR-CH %d|%d",session->session_id,session->trips+1);
	RDEBUG2("Inserting session id=%d identity='%s' state='%s' to the session list",
			 session->session_id,SAFE_STR(session->identity),session->state);


	/*
	 *	Generate State, since we've been asked to add it to
	 *	the list.
	 */
	state = pairmake_reply("State", session->state, T_OP_EQ);
	if (!state) return -1;
	state->length = SECURID_STATE_LEN;

	status = rbtree_insert(inst->session_tree, session);
	if (status) {
		/* tree insert SUCCESS */
		/* insert the session to the linked list of sessions */
		SECURID_SESSION *prev;

		prev = inst->session_tail;
		if (prev) {
			/* insert to the tail of the list */
			prev->next = session;
			session->prev = prev;
			session->next = NULL;
			inst->session_tail = session;
		} else {
			/* 1st time */
			inst->session_head = inst->session_tail = session;
			session->next = session->prev = NULL;
		}
	}

	/*
	 *	Now that we've finished mucking with the list,
	 *	unlock it.
	 */
 done:
	pthread_mutex_unlock(&(inst->session_mutex));

	if (!status) {
		pairfree(&state);
		ERROR("rlm_securid: Failed to store session");
		return -1;
	}

	return 0;
}

/*
 *	Find existing session if any which matches the State variable in current AccessRequest
 *	Then, release the session from the list, and return it to
 *	the caller.
 *
 */
SECURID_SESSION *securid_sessionlist_find(rlm_securid_t *inst, REQUEST *request)
{
	VALUE_PAIR	*state;
	SECURID_SESSION* session;
	SECURID_SESSION mySession;

	/* clean expired sessions if any */
	pthread_mutex_lock(&(inst->session_mutex));
	securid_sessionlist_clean_expired(inst, request, request->timestamp);
	pthread_mutex_unlock(&(inst->session_mutex));

	/*
	 *	We key the sessions off of the 'state' attribute
	 */
	state = pairfind(request->packet->vps, PW_STATE, 0, TAG_ANY);
	if (!state) {
		return NULL;
	}

	if (state->length != SECURID_STATE_LEN) {
	  ERROR("rlm_securid: Invalid State variable. length=%d", (int) state->length);
		return NULL;
	}

	memset(&mySession,0,sizeof(mySession));
	mySession.src_ipaddr = request->packet->src_ipaddr;
	memcpy(mySession.state, state->vp_strvalue, sizeof(mySession.state));

	/*
	 *	Playing with a data structure shared among threads
	 *	means that we need a lock, to avoid conflict.
	 */
	pthread_mutex_lock(&(inst->session_mutex));
	session = securid_sessionlist_delete(inst, &mySession);
	pthread_mutex_unlock(&(inst->session_mutex));

	/*
	 *	Might not have been there.
	 */
	if (!session) {
		ERROR("rlm_securid: No SECURID session matching the State variable.");
		return NULL;
	}

	RDEBUG2("Session found identity='%s' state='%s', released from the list",
			 SAFE_STR(session->identity),session->state);
	if (session->trips >= inst->max_trips_per_session) {
		RDEBUG2("More than %d authentication packets for this SECURID session.  Aborted.",inst->max_trips_per_session);
		securid_session_free(inst,request,session);
		return NULL;
	}
	session->trips++;

	return session;
}


/************ private functions *************/
static SECURID_SESSION *securid_sessionlist_delete(rlm_securid_t *inst, SECURID_SESSION *session)
{
	rbnode_t *node;

	node = rbtree_find(inst->session_tree, session);
	if (!node) return NULL;

	session = rbtree_node2data(inst->session_tree, node);

	/*
	 *	Delete old session from the tree.
	 */
	rbtree_delete(inst->session_tree, node);

	/*
	 *	And unsplice it from the linked list.
	 */
	if (session->prev) {
		session->prev->next = session->next;
	} else {
		inst->session_head = session->next;
	}
	if (session->next) {
		session->next->prev = session->prev;
	} else {
		inst->session_tail = session->prev;
	}
	session->prev = session->next = NULL;

	return session;
}


static void securid_sessionlist_clean_expired(rlm_securid_t *inst, REQUEST *request, time_t timestamp)
{
	int num_sessions;
	SECURID_SESSION *session;

	num_sessions = rbtree_num_elements(inst->session_tree);
	RDEBUG2("There are %d sessions in the tree\n",num_sessions);

	/*
	 *	Delete old sessions from the list
	 *
	 */
       	while((session = inst->session_head)) {
		if ((timestamp - session->timestamp) > inst->timer_limit) {
			rbnode_t *node;
			node = rbtree_find(inst->session_tree, session);
			rad_assert(node != NULL);
			rbtree_delete(inst->session_tree, node);

			/*
			 *	session == inst->session_head
			 */
			inst->session_head = session->next;
			if (session->next) {
				session->next->prev = NULL;
			} else {
				inst->session_head = NULL;
				inst->session_tail = NULL;
			}

			RDEBUG2("Cleaning expired session: identity='%s' state='%s'\n",
					  SAFE_STR(session->identity),session->state);
			securid_session_free(inst,request,session);
		} else {
			/* no need to check all sessions since they are sorted by age */
			break;
		}
	}
}
