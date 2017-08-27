#ifndef _RLM_SECURID_H
#define _RLM_SECURID_H

#include <freeradius-devel/radiusd.h>
#include <freeradius-devel/modules.h>
#include <freeradius-devel/rad_assert.h>

#include "acexport.h"

#define SAFE_STR(s) s==NULL?"EMPTY":s

typedef enum {
	INITIAL_STATE = 0,
	NEXT_CODE_REQUIRED_STATE = 100,
	NEW_PIN_REQUIRED_STATE = 200,
	NEW_PIN_USER_CONFIRM_STATE = 201,
	NEW_PIN_AUTH_VALIDATE_STATE = 202,
	NEW_PIN_SYSTEM_ACCEPT_STATE = 203,
	NEW_PIN_SYSTEM_CONFIRM_STATE = 204,
	NEW_PIN_USER_SELECT_STATE = 205,
} SECURID_SESSION_STATE;

/*
 * SECURID_SESSION is used to identify existing securID sessions
 * to continue Next-Token code and New-Pin conversations with a client
 *
 * next = pointer to next
 * state = state attribute from the reply we sent
 * state_len = length of data in the state attribute.
 * src_ipaddr = client which sent us the RADIUS request containing
 *	      this SecurID conversation.
 * timestamp  = timestamp when this handler was last used.
 * trips = number of trips
 * identity = Identity of the user
 * request = RADIUS request data structure
 */

#define SECURID_STATE_LEN 32
typedef struct _securid_session_t {
	struct _securid_session_t *prev, *next;
	SDI_HANDLE		  sdiHandle;
	SECURID_SESSION_STATE	  securidSessionState;

	char			  state[SECURID_STATE_LEN];

	fr_ipaddr_t		  src_ipaddr;
	time_t			  timestamp;
	unsigned int		  session_id;
	int			  trips;

	char			  *pin;	     /* previous pin if user entered it during NEW-PIN mode process */
	char			  *identity; /* save user's identity name for future use */

} SECURID_SESSION;


/*
 *      Define a structure for our module configuration.
 *
 *      These variables do not need to be in a structure, but it's
 *      a lot cleaner to do so, and a pointer to the structure can
 *      be used as the instance handle.
 *      sessions = remembered sessions, in a tree for speed.
 *      mutex = ensure only one thread is updating the sessions list
 */
typedef struct rlm_securid_t {
	pthread_mutex_t	session_mutex;
	rbtree_t*	session_tree;
	SECURID_SESSION	*session_head, *session_tail;

	unsigned int	 last_session_id;

	/*
	 *	Configuration items.
	 */
	int		timer_limit;
	int		max_sessions;
	int		max_trips_per_session;
} rlm_securid_t;

/* Memory Management */
SECURID_SESSION*     securid_session_alloc(void);
void		     securid_session_free(rlm_securid_t *inst, REQUEST *request,SECURID_SESSION *session);

void		     securid_sessionlist_free(rlm_securid_t *inst,REQUEST *request);

int		     securid_sessionlist_add(rlm_securid_t *inst, REQUEST *request, SECURID_SESSION *session);
SECURID_SESSION*     securid_sessionlist_find(rlm_securid_t *inst, REQUEST *request);


#endif
