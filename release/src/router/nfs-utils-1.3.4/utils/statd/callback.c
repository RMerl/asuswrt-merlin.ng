/*
 * Copyright (C) 1995, 1997-1999 Jeffrey A. Uphoff
 * Modified by Olaf Kirch, Oct. 1996.
 * Modified by Lon Hohberger, Oct. 2000.
 *
 * NSM for Linux.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <netdb.h>

#include "rpcmisc.h"
#include "statd.h"
#include "notlist.h"
#include "ha-callout.h"

/* Callback notify list. */
/* notify_list *cbnl = NULL; ... never used */


/*
 * Services SM_NOTIFY requests.
 *
 * When NLM uses an SM_MON request to tell statd to monitor a remote,
 * the request contains a "mon_name" argument.  This is usually the
 * "caller_name" argument of an NLMPROC_LOCK request.  On Linux, the
 * NLM can send statd the remote's IP address instead of its
 * caller_name.  The NSM protocol does not allow both the remote's
 * caller_name and it's IP address to be sent in the same SM_MON
 * request.
 *
 * The remote's caller_name is useful because it makes it simple
 * to identify rebooting remotes by matching the "mon_name" argument
 * they sent via an SM_NOTIFY request.
 *
 * The caller_name string may not be a fully qualified domain name,
 * or even registered in the DNS database, however.  Having the
 * remote's IP address is useful because then there is no ambiguity
 * about where to send an SM_NOTIFY after the local system reboots.
 *
 * Without the actual caller_name, however, statd must use an
 * heuristic to match an incoming SM_NOTIFY request to one of the
 * hosts it is currently monitoring.  The incoming mon_name in an
 * SM_NOTIFY address is converted to a list of IP addresses using
 * DNS.  Each mon_name on statd's monitor list is also converted to
 * an address list, and the two lists are checked to see if there is
 * a matching address.
 *
 * There are some risks to this strategy:
 *
 *   1.  The external DNS database is not reliable.  It can change
 *       over time, or the forward and reverse mappings could be
 *       inconsistent.
 *
 *   2.  If statd's monitor list becomes substantial, finding a match
 *       can generate a not inconsequential amount of DNS traffic.
 *
 *   3.  statd is a single-threaded service.  When DNS becomes slow or
 *       unresponsive, statd also becomes slow or unresponsive.
 *
 *   4.  If the remote does not have a DNS entry at all (or if the
 *       remote can resolve itself, but the local host can't resolve
 *       the remote's hostname), the remote cannot be monitored, and
 *       therefore NLM locking cannot be provided for that host.
 *
 *   5.  Local DNS resolution can produce different results for the
 *       mon_name than the results the remote might see for the same
 *       query, especially if the remote did not send a caller_name
 *       or mon_name that is a fully qualified domain name.
 *
 *       Note that a caller_name is passed from NFS client to server,
 *       but the client never knows what mon_name the server might use
 *       to notify it of a reboot.  On Linux, the client extracts the
 *       server's name from the devname it was passed by the mount
 *       command.  This is often not a fully-qualified domain name.
 */
void *
sm_notify_1_svc(struct stat_chge *argp, struct svc_req *rqstp)
{
	notify_list    *lp, *call;
	static char    *result = NULL;
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char		ip_addr[INET6_ADDRSTRLEN];

	xlog(D_CALL, "Received SM_NOTIFY from %s, state: %d",
				argp->mon_name, argp->state);

	if (!statd_present_address(sap, ip_addr, sizeof(ip_addr))) {
		xlog_warn("Unrecognized sender address");
		return ((void *) &result);
	}

	ha_callout("sm-notify", argp->mon_name, ip_addr, argp->state);

	/* quick check - don't bother if we're not monitoring anyone */
	if (rtnl == NULL) {
		xlog_warn("SM_NOTIFY from %s while not monitoring any hosts",
				argp->mon_name);
		return ((void *) &result);
	}

	/* okir change: statd doesn't remove the remote host from its
	 * internal monitor list when receiving an SM_NOTIFY call from
	 * it. Lockd will want to continue monitoring the remote host
	 * until it issues an SM_UNMON call.
	 */
	for (lp = rtnl ; lp ; lp = lp->next)
		if (NL_STATE(lp) != argp->state &&
		    (statd_matchhostname(argp->mon_name, lp->dns_name) ||
		     statd_matchhostname(ip_addr, lp->dns_name))) {
			NL_STATE(lp) = argp->state;
			call = nlist_clone(lp);
			nlist_insert(&notify, call);
		}


	return ((void *) &result);
}
