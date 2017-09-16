/*
 * Copyright (C) 1995-1999 Jeffrey A. Uphoff
 * Major rewrite by Olaf Kirch, Dec. 1996.
 * Modified by H.J. Lu, 1998.
 * Tighter access control, Olaf Kirch June 1999.
 *
 * NSM for Linux.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>
#include <dirent.h>

#include "sockaddr.h"
#include "rpcmisc.h"
#include "nsm.h"
#include "statd.h"
#include "notlist.h"
#include "ha-callout.h"

notify_list *		rtnl = NULL;	/* Run-time notify list. */

/*
 * Reject requests from non-loopback addresses in order
 * to prevent attack described in CERT CA-99.05.
 *
 * Although the kernel contacts the statd service via only IPv4
 * transports, the statd service can receive other requests, such
 * as SM_NOTIFY, from remote peers via IPv6.
 */
static _Bool
caller_is_localhost(struct svc_req *rqstp)
{
	struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
	char buf[INET6_ADDRSTRLEN];

	if (!nfs_is_v4_loopback(sap))
		goto out_nonlocal;
	return true;

out_nonlocal:
	if (!statd_present_address(sap, buf, sizeof(buf)))
		buf[0] = '\0';
	xlog_warn("SM_MON/SM_UNMON call from non-local host %s", buf);
	return false;
}

/*
 * Services SM_MON requests.
 */
struct sm_stat_res *
sm_mon_1_svc(struct mon *argp, struct svc_req *rqstp)
{
	static sm_stat_res result;
	char		*mon_name = argp->mon_id.mon_name,
			*my_name  = argp->mon_id.my_id.my_name;
	struct my_id	*id = &argp->mon_id.my_id;
	char		*cp;
	notify_list	*clnt;
	struct sockaddr_in my_addr = {
		.sin_family		= AF_INET,
		.sin_addr.s_addr	= htonl(INADDR_LOOPBACK),
	};
	char *dnsname = NULL;
	int existing = 0;

	xlog(D_CALL, "Received SM_MON for %s from %s", mon_name, my_name);

	/* Assume that we'll fail. */
	result.res_stat = STAT_FAIL;
	result.state = -1;	/* State is undefined for STAT_FAIL. */

	/* 1.	Reject any remote callers.
	 *	Ignore the my_name specified by the caller, and
	 *	use "127.0.0.1" instead.
	 */
	if (!caller_is_localhost(rqstp))
		goto failure;

	/* 2.	Reject any registrations for non-lockd services.
	 *
	 *	This is specific to the linux kernel lockd, which
	 *	makes the callback procedure part of the lockd interface.
	 *	It is also prone to break when lockd changes its callback
	 *	procedure number -- which, in fact, has now happened once.
	 *	There must be a better way....   XXX FIXME
	 */
	if (id->my_prog != 100021 ||
	    (id->my_proc != 16 && id->my_proc != 24))
	{
		xlog_warn("Attempt to register callback to %d/%d",
			id->my_prog, id->my_proc);
		goto failure;
	}

	/*
	 * Check hostnames.  If I can't look them up, I won't monitor.  This
	 * might not be legal, but it adds a little bit of safety and sanity.
	 */

	/* must check for /'s in hostname!  See CERT's CA-96.09 for details. */
	if (strchr(mon_name, '/') || mon_name[0] == '.') {
		xlog(L_ERROR, "SM_MON request for hostname containing '/' "
		     "or starting '.': %s", mon_name);
		xlog(L_ERROR, "POSSIBLE SPOOF/ATTACK ATTEMPT!");
		goto failure;
	}

	/* my_name must not have white space */
	for (cp=my_name ; *cp ; cp++)
		if (*cp == ' ' || *cp == '\t' || *cp == '\r' || *cp == '\n')
			*cp = '_';

	/*
	 * Hostnames checked OK.
	 * Now choose a hostname to use for matching.  We cannot
	 * really trust much in the incoming NOTIFY, so to make
	 * sure that multi-homed hosts work nicely, we get an
	 * FQDN now, and use that for matching.
	 */
	dnsname = statd_canonical_name(mon_name);
	if (dnsname == NULL) {
		xlog(L_WARNING, "No canonical hostname found for %s", mon_name);
		goto failure;
	}

	/* Now check to see if this is a duplicate, and warn if so.
	 * I will also return STAT_FAIL. (I *think* this is how I should
	 * handle it.)
	 *
	 * Olaf requests that I allow duplicate SM_MON requests for
	 * hosts due to the way he is coding lockd. No problem,
	 * I'll just do a quickie success return and things should
	 * be happy.
	 */
	clnt = rtnl;

	while ((clnt = nlist_gethost(clnt, mon_name, 0))) {
		if (statd_matchhostname(NL_MY_NAME(clnt), my_name) &&
		    NL_MY_PROC(clnt) == id->my_proc &&
		    NL_MY_PROG(clnt) == id->my_prog &&
		    NL_MY_VERS(clnt) == id->my_vers) {
			if (memcmp(NL_PRIV(clnt), argp->priv, SM_PRIV_SIZE)) {
				xlog(D_GENERAL,
					"Received SM_MON request with new "
					"cookie for %s from procedure on %s",
					mon_name, my_name);

				existing = 1; 
				break;
			} else {
				/* Hey!  We already know you guys! */
				xlog(D_GENERAL,
					"Duplicate SM_MON request for %s "
					"from procedure on %s",
					mon_name, my_name);

				/* But we'll let you pass anyway. */
				free(dnsname);
				goto success;
			}
		}
		clnt = NL_NEXT(clnt);
	}

	/*
	 * We're committed...ignoring errors.  Let's hope that a malloc()
	 * doesn't fail.  (I should probably fix this assumption.)
	 */
	if (!existing && !(clnt = nlist_new(my_name, mon_name, 0))) {
		free(dnsname);
		xlog_warn("out of memory");
		goto failure;
	}

	NL_MY_PROG(clnt) = id->my_prog;
	NL_MY_VERS(clnt) = id->my_vers;
	NL_MY_PROC(clnt) = id->my_proc;
	memcpy(NL_PRIV(clnt), argp->priv, SM_PRIV_SIZE);
	clnt->dns_name = dnsname;

	/*
	 * Now, Create file on stable storage for host, first deleting any
	 * existing records on file.
	 */
	nsm_delete_monitored_host(dnsname, mon_name, my_name, 0);

	if (!nsm_insert_monitored_host(dnsname,
				(struct sockaddr *)(char *)&my_addr, argp)) {
		nlist_free(NULL, clnt);
		goto failure;
	}

	/* PRC: do the HA callout: */
	ha_callout("add-client", mon_name, my_name, -1);
	if (!existing)
		nlist_insert(&rtnl, clnt);
	xlog(D_GENERAL, "MONITORING %s for %s", mon_name, my_name);
 success:
	result.res_stat = STAT_SUCC;
	/* SUN's sm_inter.x says this should be "state number of local site".
	 * X/Open says '"state" will be contain the state of the remote NSM.'
	 * href=http://www.opengroup.org/onlinepubs/9629799/SM_MON.htm
	 * Linux lockd currently (2.6.21 and prior) ignores whatever is
	 * returned, and given the above contraction, it probably always will..
	 * So we just return what we always returned.  If possible, we
	 * have already told lockd about our state number via a sysctl.
	 * If lockd wants the remote state, it will need to
	 * use SM_STAT (and prayer).
	 */
	result.state = MY_STATE;
	return (&result);

failure:
	xlog_warn("STAT_FAIL to %s for SM_MON of %s", my_name, mon_name);
	return (&result);
}

static unsigned int
load_one_host(const char *hostname,
		__attribute__ ((unused)) const struct sockaddr *sap,
		const struct mon *m,
		__attribute__ ((unused)) const time_t timestamp)
{
	notify_list *clnt;

	clnt = nlist_new(m->mon_id.my_id.my_name,
				m->mon_id.mon_name, 0);
	if (clnt == NULL)
		return 0;

	clnt->dns_name = strdup(hostname);
	if (clnt->dns_name == NULL) {
		nlist_free(NULL, clnt);
		return 0;
	}

	xlog(D_GENERAL, "Adding record for %s to the monitor list...",
			hostname);

	NL_MY_PROG(clnt) = m->mon_id.my_id.my_prog;
	NL_MY_VERS(clnt) = m->mon_id.my_id.my_vers;
	NL_MY_PROC(clnt) = m->mon_id.my_id.my_proc;
	memcpy(NL_PRIV(clnt), m->priv, SM_PRIV_SIZE);

	nlist_insert(&rtnl, clnt);
	return 1;
}

void load_state(void)
{
	unsigned int count;

	count = nsm_load_monitor_list(load_one_host);
	if (count)
		xlog(D_GENERAL, "Loaded %u previously monitored hosts", count);
}

/*
 * Services SM_UNMON requests.
 *
 * There is no statement in the X/Open spec's about returning an error
 * for requests to unmonitor a host that we're *not* monitoring.  I just
 * return the state of the NSM when I get such foolish requests for lack
 * of any better ideas.  (I also log the "offense.")
 */
struct sm_stat *
sm_unmon_1_svc(struct mon_id *argp, struct svc_req *rqstp)
{
	static sm_stat  result;
	notify_list	*clnt;
	char		*mon_name = argp->mon_name,
			*my_name  = argp->my_id.my_name;
	struct my_id	*id = &argp->my_id;
	char		*cp;

	xlog(D_CALL, "Received SM_UNMON for %s from %s", mon_name, my_name);

	result.state = MY_STATE;

	if (!caller_is_localhost(rqstp))
		goto failure;

	/* my_name must not have white space */
	for (cp=my_name ; *cp ; cp++)
		if (*cp == ' ' || *cp == '\t' || *cp == '\r' || *cp == '\n')
			*cp = '_';


	/* Check if we're monitoring anyone. */
	if (rtnl == NULL) {
		xlog_warn("Received SM_UNMON request from %s for %s while not "
			"monitoring any hosts", my_name, argp->mon_name);
		return (&result);
	}
	clnt = rtnl;

	/*
	 * OK, we are.  Now look for appropriate entry in run-time list.
	 * There should only be *one* match on this, since I block "duplicate"
	 * SM_MON calls.  (Actually, duplicate calls are allowed, but only one
	 * entry winds up in the list the way I'm currently handling them.)
	 */
	while ((clnt = nlist_gethost(clnt, mon_name, 0))) {
		if (statd_matchhostname(NL_MY_NAME(clnt), my_name) &&
			NL_MY_PROC(clnt) == id->my_proc &&
			NL_MY_PROG(clnt) == id->my_prog &&
			NL_MY_VERS(clnt) == id->my_vers) {
			/* Match! */
			xlog(D_GENERAL, "UNMONITORING %s for %s",
					mon_name, my_name);

			/* PRC: do the HA callout: */
			ha_callout("del-client", mon_name, my_name, -1);

			nsm_delete_monitored_host(clnt->dns_name,
							mon_name, my_name, 1);
			nlist_free(&rtnl, clnt);

			return (&result);
		} else
			clnt = NL_NEXT(clnt);
	}

 failure:
	xlog_warn("Received erroneous SM_UNMON request from %s for %s",
		my_name, mon_name);
	return (&result);
}


struct sm_stat *
sm_unmon_all_1_svc(struct my_id *argp, struct svc_req *rqstp)
{
	short int       count = 0;
	static sm_stat  result;
	notify_list	*clnt;
	char		*my_name = argp->my_name;

	xlog(D_CALL, "Received SM_UNMON_ALL for %s", my_name);

	if (!caller_is_localhost(rqstp))
		goto failure;

	result.state = MY_STATE;

	if (rtnl == NULL) {
		xlog_warn("Received SM_UNMON_ALL request from %s "
			"while not monitoring any hosts", my_name);
		return (&result);
	}
	clnt = rtnl;

	while ((clnt = nlist_gethost(clnt, my_name, 1))) {
		if (NL_MY_PROC(clnt) == argp->my_proc &&
			NL_MY_PROG(clnt) == argp->my_prog &&
			NL_MY_VERS(clnt) == argp->my_vers) {
			/* Watch stack! */
			char            mon_name[SM_MAXSTRLEN + 1];
			notify_list	*temp;

			xlog(D_GENERAL,
				"UNMONITORING (SM_UNMON_ALL) %s for %s",
				NL_MON_NAME(clnt), NL_MY_NAME(clnt));
			strncpy(mon_name, NL_MON_NAME(clnt),
				sizeof (mon_name) - 1);
			mon_name[sizeof (mon_name) - 1] = '\0';
			temp = NL_NEXT(clnt);
			/* PRC: do the HA callout: */
			ha_callout("del-client", mon_name, my_name, -1);
			nsm_delete_monitored_host(clnt->dns_name,
							mon_name, my_name, 1);
			nlist_free(&rtnl, clnt);
			++count;
			clnt = temp;
		} else
			clnt = NL_NEXT(clnt);
	}

	if (!count) {
		xlog(D_GENERAL, "SM_UNMON_ALL request from %s with no "
			"SM_MON requests from it", my_name);
	}

 failure:
	return (&result);
}
