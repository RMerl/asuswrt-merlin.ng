/*
 * Copyright (C) 1995-1997, 1999 Jeffrey A. Uphoff
 *
 * NSM for Linux.
 */

#include "config.h"
#ifndef SIMULATIONS
# error How the hell did we get here?
#endif

/* If we're running the simulator, we're debugging.  Pretty simple. */
#ifndef DEBUG
# define DEBUG
#endif

#include <signal.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpcmisc.h>
#include "statd.h"
#include "sim_sm_inter.h"

static void daemon_simulator (void);
static void sim_killer (int sig);
static void simulate_crash (char *);
static void simulate_mon (char *, char *, char *, char *, char *);
static void simulate_stat (char *, char *);
static void simulate_unmon (char *, char *, char *, char *);
static void simulate_unmon_all (char *, char *, char *);

static int sim_port = 0;

extern void sim_sm_prog_1 (struct svc_req *, register SVCXPRT);
extern void svc_exit (void);

void
simulator (int argc, char **argv)
{
  xlog_stderr (1);
  xlog_syslog (0);
  xlog_open ("statd simulator");

  if (argc == 2)
    if (!strcasecmp (*argv, "crash"))
      simulate_crash (*(&argv[1]));

  if (argc == 3) {
    if (!strcasecmp (*argv, "stat"))
      simulate_stat (*(&argv[1]), *(&argv[2]));
  }
  if (argc == 4) {
    if (!strcasecmp (*argv, "unmon_all"))
      simulate_unmon_all (*(&argv[1]), *(&argv[2]), *(&argv[3]));
  }
  if (argc == 5) {
    if (!strcasecmp (*argv, "unmon"))
      simulate_unmon (*(&argv[1]), *(&argv[2]), *(&argv[3]), *(&argv[4]));
  }
  if (argc == 6) {
    if (!strcasecmp (*argv, "mon"))
      simulate_mon (*(&argv[1]), *(&argv[2]), *(&argv[3]), *(&argv[4]),
		    *(&argv[5]));
  }
  xlog_err ("WTF?  Give me something I can use!");
}

static void
simulate_mon (char *calling, char *monitoring, char *as, char *proggy,
	      char *fool)
{
  CLIENT *client;
  sm_stat_res *result;
  mon mon;

  xlog (D_GENERAL, "Calling %s (as %s) to monitor %s", calling, as,
	   monitoring);

  if ((client = clnt_create (calling, SM_PROG, SM_VERS, "udp")) == NULL)
    xlog_err ("%s", clnt_spcreateerror ("clnt_create"));

  memcpy (mon.priv, fool, SM_PRIV_SIZE);
  mon.mon_id.my_id.my_name = xstrdup (as);
  sim_port = atoi (proggy) * SIM_SM_PROG;
  mon.mon_id.my_id.my_prog = sim_port; /* Pseudo-dummy */
  mon.mon_id.my_id.my_vers = SIM_SM_VERS;
  mon.mon_id.my_id.my_proc = SIM_SM_MON;
  mon.mon_id.mon_name = monitoring;

  if (!(result = sm_mon_1 (&mon, client)))
    xlog_err ("%s", clnt_sperror (client, "sm_mon_1"));

  free (mon.mon_id.my_id.my_name);

  if (result->res_stat != STAT_SUCC) {
    xlog_err ("SM_MON request failed, state: %d", result->state);
  } else {
    xlog (D_GENERAL, "SM_MON result successful, state: %d\n", result->state);
    xlog (D_GENERAL, "Waiting for callback");
    daemon_simulator ();
    exit (0);
  }
}

static void
simulate_unmon (char *calling, char *unmonitoring, char *as, char *proggy)
{
  CLIENT *client;
  sm_stat *result;
  mon_id mon_id;

  xlog (D_GENERAL, "Calling %s (as %s) to unmonitor %s", calling, as,
	   unmonitoring);

  if ((client = clnt_create (calling, SM_PROG, SM_VERS, "udp")) == NULL)
    xlog_err ("%s", clnt_spcreateerror ("clnt_create"));

  mon_id.my_id.my_name = xstrdup (as);
  mon_id.my_id.my_prog = atoi (proggy) * SIM_SM_PROG;
  mon_id.my_id.my_vers = SIM_SM_VERS;
  mon_id.my_id.my_proc = SIM_SM_MON;
  mon_id.mon_name = unmonitoring;

  if (!(result = sm_unmon_1 (&mon_id, client)))
    xlog_err ("%s", clnt_sperror (client, "sm_unmon_1"));

  free (mon_id.my_id.my_name);
  xlog (D_GENERAL, "SM_UNMON request returned state: %d\n", result->state);
  exit (0);
}

static void
simulate_unmon_all (char *calling, char *as, char *proggy)
{
  CLIENT *client;
  sm_stat *result;
  my_id my_id;

  xlog (D_GENERAL, "Calling %s (as %s) to unmonitor all hosts", calling, as);

  if ((client = clnt_create (calling, SM_PROG, SM_VERS, "udp")) == NULL)
    xlog_err ("%s", clnt_spcreateerror ("clnt_create"));

  my_id.my_name = xstrdup (as);
  my_id.my_prog = atoi (proggy) * SIM_SM_PROG;
  my_id.my_vers = SIM_SM_VERS;
  my_id.my_proc = SIM_SM_MON;

  if (!(result = sm_unmon_all_1 (&my_id, client)))
    xlog_err ("%s", clnt_sperror (client, "sm_unmon_all_1"));

  free (my_id.my_name);
  xlog (D_GENERAL, "SM_UNMON_ALL request returned state: %d\n", result->state);
  exit (0);
}

static void
simulate_crash (char *host)
{
  CLIENT *client;

  if ((client = clnt_create (host, SM_PROG, SM_VERS, "udp")) == NULL)
    xlog_err ("%s", clnt_spcreateerror ("clnt_create"));

  if (!sm_simu_crash_1 (NULL, client))
    xlog_err ("%s", clnt_sperror (client, "sm_simu_crash_1"));

  exit (0);
}

static void
simulate_stat (char *calling, char *monitoring)
{
  CLIENT *client;
  sm_name checking;
  sm_stat_res *result;
  
  if ((client = clnt_create (calling, SM_PROG, SM_VERS, "udp")) == NULL)
    xlog_err ("%s", clnt_spcreateerror ("clnt_create"));

  checking.mon_name = monitoring;

  if (!(result = sm_stat_1 (&checking, client)))
    xlog_err ("%s", clnt_sperror (client, "sm_stat_1"));

  if (result->res_stat == STAT_SUCC)
    xlog (D_GENERAL, "STAT_SUCC from %s for %s, state: %d", calling,
	     monitoring, result->state);
  else
    xlog (D_GENERAL, "STAT_FAIL from %s for %s, state: %d", calling,
	     monitoring, result->state);

  exit (0);
}

static void
sim_killer (int sig)
{
  pmap_unset (sim_port, SIM_SM_VERS);
  xlog_err ("Simulator caught signal %d, un-registering and exiting", sig);
}

static void
daemon_simulator (void)
{
  signal (SIGHUP, sim_killer);
  signal (SIGINT, sim_killer);
  signal (SIGTERM, sim_killer);
  pmap_unset (sim_port, SIM_SM_VERS);
  /* this registers both UDP and TCP services */
  rpc_init("statd", sim_port, SIM_SM_VERS, sim_sm_prog_1, 0);
  svc_run ();
  pmap_unset (sim_port, SIM_SM_VERS);
}

void *
sim_sm_mon_1_svc (struct status *argp, struct svc_req *rqstp)
{
  static char *result;

  xlog (D_GENERAL, "Recieved state %d for mon_name %s (opaque \"%s\")",
	   argp->state, argp->mon_name, argp->priv);
  svc_exit ();
  return ((void *)&result);
}
