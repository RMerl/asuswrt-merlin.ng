/*
 * Copyright (C) 1996, 1999 Olaf Kirch
 * Modified by Jeffrey A. Uphoff, 1997-1999.
 * Modified by H.J. Lu, 1998.
 * Modified by Lon Hohberger, Oct. 2000
 *   - Bugfix handling client responses.
 *   - Paranoia on NOTIFY_CALLBACK case
 *
 * NSM for Linux.
 */

/*
 * After reboot, notify all hosts on our notify list. In order not to
 * hang statd with delivery to dead hosts, we perform all RPC calls in
 * parallel.
 *
 * It would have been nice to use the portmapper's rmtcall feature,
 * but that's not possible for security reasons (the portmapper would
 * have to forward the call with root privs for most statd's, which
 * it won't if it's worth its money).
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_rmt.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "sm_inter.h"
#include "statd.h"
#include "notlist.h"
#include "ha-callout.h"

#include "nsm.h"
#include "nfsrpc.h"

#if SIZEOF_SOCKLEN_T - 0 == 0
#define socklen_t int
#endif

static int		sockfd = -1;	/* notify socket */

/* How many times to try looking for an unused privileged port */
#define MAX_BRP_RETRIES	100

/*
 * Initialize socket used to notify lockd of peer reboots.
 *
 * Returns the file descriptor of the new socket if successful;
 * otherwise returns -1 and logs an error.
 *
 * Lockd rejects such requests if the source port is not privileged.
 * statd_get_socket() must be invoked while statd still holds root
 * privileges in order for the socket to acquire a privileged source
 * port.
 */
int
statd_get_socket(void)
{
	struct sockaddr_in	sin;
	struct servent *se;
	static int prevsocks[MAX_BRP_RETRIES];
	unsigned int retries;

	if (sockfd >= 0)
		return sockfd;

	retries = 0;
	do {
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			xlog(L_ERROR, "%s: Can't create socket: %m", __func__);
			break;
		}

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		if (bindresvport(sockfd, &sin) < 0) {
			xlog(D_GENERAL, "%s: can't bind to reserved port",
					__func__);
			break;
		}
		se = getservbyport(sin.sin_port, "udp");
		if (se == NULL)
			break;

		if (retries == MAX_BRP_RETRIES) {
			xlog(D_GENERAL, "%s: No unused privileged ports",
					__func__);
			break;
		}

		/* rather not use that port, try again */
		prevsocks[retries++] = sockfd;
	} while (1);

	while (retries)
		close(prevsocks[--retries]);

	if (sockfd < 0)
		return -1;

	return sockfd;
}

static notify_list *
recv_rply(u_long *portp)
{
	char			msgbuf[NSM_MAXMSGSIZE];
	ssize_t			msglen;
	notify_list		*lp = NULL;
	XDR			xdr;
	struct sockaddr_in	sin;
	socklen_t		alen = (socklen_t)sizeof(sin);
	uint32_t		xid;

	memset(msgbuf, 0, sizeof(msgbuf));
	msglen = recvfrom(sockfd, msgbuf, sizeof(msgbuf), 0,
				(struct sockaddr *)(char *)&sin, &alen);
	if (msglen == (ssize_t)-1) {
		xlog_warn("%s: recvfrom failed: %m", __func__);
		return NULL;
	}

	memset(&xdr, 0, sizeof(xdr));
	xdrmem_create(&xdr, msgbuf, (unsigned int)msglen, XDR_DECODE);
	xid = nsm_parse_reply(&xdr);
	if (xid == 0)
		goto done;
	if (sin.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
		struct in_addr addr = sin.sin_addr;
		char buf[INET_ADDRSTRLEN];

		xlog_warn("%s: Unrecognized reply from %s", __func__,
				inet_ntop(AF_INET, &addr, buf,
						(socklen_t)sizeof(buf)));
		goto done;
	}

	for (lp = notify; lp != NULL; lp = lp->next) {
		/* LH - this was a bug... it should have been checking
		 * the xid from the response message from the client,
		 * not the static, internal xid */
		if (lp->xid != xid)
			continue;
		if (lp->port == 0)
			*portp = nsm_recv_getport(&xdr);
		break;
	}

done:
	xdr_destroy(&xdr);
	return lp;
}

/*
 * Notify operation for a single list entry
 */
static int
process_entry(notify_list *lp)
{
	struct sockaddr_in	sin;

	if (NL_TIMES(lp) == 0) {
		xlog(D_GENERAL, "%s: Cannot notify localhost, giving up",
				__func__);
		return 0;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port   = lp->port;
	/* LH - moved address into switch */

	/* __FORCE__ loopback for callbacks to lockd ... */
	/* Just in case we somehow ignored it thus far */
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (sin.sin_port == 0)
		lp->xid = nsm_xmit_getport(sockfd, &sin,
					(rpcprog_t)NL_MY_PROG(lp),
					(rpcvers_t)NL_MY_VERS(lp));
	else {
		struct mon m;

		memcpy(m.priv, NL_PRIV(lp), SM_PRIV_SIZE);

		m.mon_id.mon_name = NL_MON_NAME(lp);
		m.mon_id.my_id.my_name = NULL;
		m.mon_id.my_id.my_prog = NL_MY_PROG(lp);
		m.mon_id.my_id.my_vers = NL_MY_VERS(lp);
		m.mon_id.my_id.my_proc = NL_MY_PROC(lp);

		lp->xid = nsm_xmit_nlmcall(sockfd,
				(struct sockaddr *)(char *)&sin,
				(socklen_t)sizeof(sin), &m, NL_STATE(lp));
	}
	if (lp->xid == 0) {
		xlog_warn("%s: failed to notify port %d",
				__func__, ntohs(lp->port));
	}
	NL_TIMES(lp) -= 1;

	return 1;
}

/*
 * Process a datagram received on the notify socket
 */
int
process_reply(FD_SET_TYPE *rfds)
{
	notify_list		*lp;
	u_long			port;

	if (sockfd == -1 || !FD_ISSET(sockfd, rfds))
		return 0;

	/* Should not be processed again. */
	FD_CLR (sockfd, rfds);

	if (!(lp = recv_rply(&port)))
		return 1;

	if (lp->port == 0) {
		if (port != 0) {
			lp->port = htons((unsigned short) port);
			process_entry(lp);
			NL_WHEN(lp) = time(NULL) + NOTIFY_TIMEOUT;
			nlist_remove(&notify, lp);
			nlist_insert_timer(&notify, lp);
			return 1;
		}
		xlog_warn("%s: service %d not registered on localhost",
			__func__, NL_MY_PROG(lp));
	} else {
		xlog(D_GENERAL, "%s: Callback to %s (for %d) succeeded",
			__func__, NL_MY_NAME(lp), NL_MON_NAME(lp));
	}
	nlist_free(&notify, lp);
	return 1;
}

/*
 * Process a notify list, either for notifying remote hosts after reboot
 * or for calling back (local) statd clients when the remote has notified
 * us of a crash. 
 */
int
process_notify_list(void)
{
	notify_list	*entry;
	time_t		now;

	while ((entry = notify) != NULL && NL_WHEN(entry) < time(&now)) {
		if (process_entry(entry)) {
			NL_WHEN(entry) = time(NULL) + NOTIFY_TIMEOUT;
			nlist_remove(&notify, entry);
			nlist_insert_timer(&notify, entry);
		} else {
			xlog(L_ERROR,
				"%s: Can't callback %s (%d,%d), giving up",
					__func__,
					NL_MY_NAME(entry),
					NL_MY_PROG(entry),
					NL_MY_VERS(entry));
			nlist_free(&notify, entry);
		}
	}

	return 1;
}
