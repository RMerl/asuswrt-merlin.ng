/*
 * Copyright (C) 1995, 1997-1999 Jeffrey A. Uphoff
 * Modified by Olaf Kirch, 1996.
 * Modified by H.J. Lu, 1998.
 * Modified by Jeff Layton, 2010.
 *
 * NLM similator for Linux
 */

#ifdef RPC_CLNT
%#include <string.h>
#endif

/*
 * statd rejects monitor registrations for any non-lockd services, so pretend
 * to be lockd when testing. Furthermore, the only call we care about from
 * statd is #16, which is the downcall to notify the kernel of a host's status
 * change.
 */
program NLM_SM_PROG {
	/* version 3 of the NLM protocol */
	version NLM_SM_VERS3 {
		void	 NLM_SM_NOTIFY(struct nlm_sm_notify) = 16;
	} = 3;

	/* version 2 of NLM protocol */
	version NLM_SM_VERS4 {
		void	 NLM_SM_NOTIFY(struct nlm_sm_notify) = 16;
	} = 4;
} = 100021;

const  SM_MAXSTRLEN = 1024;
const  SM_PRIV_SIZE = 16;

/*
 * structure of the status message sent back by the status monitor
 * when monitor site status changes
 */
struct nlm_sm_notify {
	string mon_name<SM_MAXSTRLEN>;
	int state;
	opaque priv[SM_PRIV_SIZE]; /* stored private information */
};
