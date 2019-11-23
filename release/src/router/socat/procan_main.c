/* source: procan_main.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

const char copyright[] = "procan by Gerhard Rieger and contributors - send bug reports to socat@dest-unreach.org";

#include <signal.h>	/* sig_atomic_t for error.h */
#include <time.h>	/* struct timespec for error.h */
#include <stdlib.h>	/* strtoul() */
#include <string.h>
#include <stdio.h>
#include "config.h"
#if HAVE_SYS_SELECT_H
#include <sys/select.h>	/* select(), fdset on FreeBSD */
#endif
#include "mytypes.h"
#include "error.h"
#include "procan.h"
#include "hostan.h"


#define WITH_HELP 1

static void procan_usage(FILE *fd);


int main(int argc, const char *argv[]) {
   const char **arg1;
#if 0
   unsigned int n = 1024;	/* this is default on my Linux */
#endif

   diag_set('p', strchr(argv[0], '/') ? strrchr(argv[0], '/')+1 : argv[0]);

   arg1 = argv+1;  --argc;
   while (arg1[0] && (arg1[0][0] == '-')) {
      switch (arg1[0][1]) {
#if WITH_HELP
      case '?': case 'h': procan_usage(stdout); exit(0);
#endif /* WITH_HELP */
      case 'c': procan_cdefs(stdout); exit(0);
#if LATER
      case 'V': procan_version(stdout); exit(0);
      case 'l': diag_set(arg1[0][2], &arg1[0][3]); break;
      case 'd': diag_set('d', NULL); break;
#endif
#if 0
      case 'n': n = strtoul(&arg1[0][2], NULL, 0); break;
#endif
      case '\0': break;
      default:
	 diag_set_int('e', E_FATAL);
	 Error1("unknown option \"%s\"", arg1[0]);
#if WITH_HELP
	 procan_usage(stderr);
#endif
	 exit(1);
      }
      if (arg1[0][1] == '\0')
	 break;
      ++arg1; --argc;
   }
   if (argc != 0) {
      Error1("%d superfluous arguments", argc);
#if WITH_HELP
      procan_usage(stderr);
#endif
      exit(1);
   }
   procan(stdout);
   hostan(stdout);
   return 0;
}


#if WITH_HELP
static void procan_usage(FILE *fd) {
   fputs(copyright, fd); fputc('\n', fd);
   fputs("Analyze system parameters of process\n", fd);
   fputs("Usage:\n", fd);
   fputs("procan [options]\n", fd);
   fputs("   options:\n", fd);
#if LATER
   fputs("      -V     print version information to stdout, and exit\n", fd);
#endif
#if WITH_HELP
   fputs("      -?|-h  print a help text describing command line options\n", fd);
#endif
   fputs("      -c     print values of compile time C defines\n", fd);
#if LATER
   fputs("      -d     increase verbosity (use up to 4 times; 2 are recommended)\n", fd);
#endif
#if 0
   fputs("      -ly[facility]  log to syslog, using facility (default is daemon)\n", fd);
   fputs("      -lf<logfile>   log to file\n", fd);
   fputs("      -ls            log to stderr (default if no other log)\n", fd);
#endif
#if 0
   fputs("      -n<fdnum>      first file descriptor number not analyzed\n", fd);
#endif
}
#endif /* WITH_HELP */
