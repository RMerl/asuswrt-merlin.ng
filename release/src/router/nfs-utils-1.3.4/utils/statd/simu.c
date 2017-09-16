/*
 * Copyright (C) 1995, 1997-1999 Jeffrey A. Uphoff
 *
 * NSM for Linux.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <netdb.h>
#include <arpa/inet.h>

#include "sockaddr.h"
#include "rpcmisc.h"
#include "statd.h"
#include "notlist.h"

extern void my_svc_exit (void);


/*
 * Services SM_SIMU_CRASH requests.
 *
 * Although the kernel contacts the statd service via only IPv4
 * transports, the statd service can receive other requests, such
 * as SM_NOTIFY, from remote peers via IPv6.
 */
void *
sm_simu_crash_1_svc (__attribute__ ((unused)) void *argp, struct svc_req *rqstp)
{
  struct sockaddr *sap = nfs_getrpccaller(rqstp->rq_xprt);
  char buf[INET6_ADDRSTRLEN];
  static char *result = NULL;

  xlog(D_CALL, "Received SM_SIMU_CRASH");

  if (!nfs_is_v4_loopback(sap))
    goto out_nonlocal;

  if ((int)nfs_get_port(sap) >= IPPORT_RESERVED) {
    xlog_warn("SM_SIMU_CRASH call from unprivileged port");
    goto failure;
  }

  my_svc_exit ();

  if (rtnl)
    nlist_kill (&rtnl);

 failure:
  return ((void *)&result);

 out_nonlocal:
  if (!statd_present_address(sap, buf, sizeof(buf)))
    buf[0] = '\0';
  xlog_warn("SM_SIMU_CRASH call from non-local host %s", buf);
  goto failure;
}
