/*
 * rlm_eap.h    Local Header file.
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
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 * Copyright 2003  Alan DeKok <aland@freeradius.org>
 * Copyright 2006  The FreeRADIUS server project
 */
#ifndef _RLM_EAP_H
#define _RLM_EAP_H

RCSIDH(rlm_eap_h, "$Id$")

#include <freeradius-devel/modpriv.h>
#include "eap.h"
#include "eap_types.h"

/*
 * Keep track of which sub modules we've loaded.
 */
typedef struct eap_module {
	char const		*name;
	rlm_eap_module_t	*type;
	lt_dlhandle		handle;
	CONF_SECTION		*cs;
	void			*instance;
} eap_module_t;

/*
 * This structure contains eap's persistent data.
 * sessions = remembered sessions, in a tree for speed.
 * types = All supported EAP-Types
 * mutex = ensure only one thread is updating the sessions[] struct
 */
typedef struct rlm_eap {
	rbtree_t	*session_tree;
	eap_handler_t	*session_head, *session_tail;
	rbtree_t	*handler_tree; /* for debugging only */
	eap_module_t 	*methods[PW_EAP_MAX_TYPES];

	/*
	 *	Configuration items.
	 */
	int		timer_limit;

	char const	*default_method_name;
	eap_type_t	default_method;

	int		ignore_unknown_types;
	int		mod_accounting_username_bug;

	int		max_sessions;

#ifdef HAVE_PTHREAD_H
	pthread_mutex_t	session_mutex;
	pthread_mutex_t	handler_mutex;
#endif

	char const	*xlat_name; /* no xlat's yet */
	fr_randctx	rand_pool;
} rlm_eap_t;

/*
 *	For simplicity in the rest of the code.
 */
#ifndef HAVE_PTHREAD_H
/*
 *	This is easier than ifdef's throughout the code.
 */
#define pthread_mutex_init(_x, _y)
#define pthread_mutex_destroy(_x)
#define pthread_mutex_lock(_x)
#define pthread_mutex_unlock(_x)
#endif

/* function definitions */
/* EAP-Type */
int      	eap_module_load(rlm_eap_t *inst, eap_module_t **method, eap_type_t num,
		    		CONF_SECTION *cs);
eap_rcode_t	eap_method_select(rlm_eap_t *inst, eap_handler_t *handler);

/* EAP */
int  		eap_start(rlm_eap_t *inst, REQUEST *request);
void 		eap_fail(eap_handler_t *handler);
void 		eap_success(eap_handler_t *handler);
rlm_rcode_t 	eap_compose(eap_handler_t *handler);
eap_handler_t 	*eap_handler(rlm_eap_t *inst, eap_packet_raw_t **eap_msg, REQUEST *request);

/* Memory Management */
EAP_DS      	*eap_ds_alloc(eap_handler_t *handler);
eap_handler_t 	*eap_handler_alloc(rlm_eap_t *inst);
void	    	eap_ds_free(EAP_DS **eap_ds);
int 	    	eaplist_add(rlm_eap_t *inst, eap_handler_t *handler);
eap_handler_t 	*eaplist_find(rlm_eap_t *inst, REQUEST *request,
			      eap_packet_raw_t *eap_packet);
void		eaplist_free(rlm_eap_t *inst);

/* State */
void	    	generate_key(void);
VALUE_PAIR  	*generate_state(time_t timestamp);
int	    	verify_state(VALUE_PAIR *state, time_t timestamp);

#endif /*_RLM_EAP_H*/
