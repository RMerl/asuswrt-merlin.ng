/* source: xio-exec.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses of exec type */

#include "xiosysincludes.h"
#include "xioopen.h"
#include "nestlex.h"

#include "xio-progcall.h"
#include "xio-exec.h"

#if WITH_EXEC

static int xioopen_exec(int argc, const char *argv[], struct opt *opts,
		int xioflags,	/* XIO_RDONLY etc. */
		xiofile_t *fd,
		unsigned groups,
		int dummy1, int dummy2, int dummy3
		);

const struct addrdesc addr_exec = { "exec",   3, xioopen_exec, GROUP_FD|GROUP_FORK|GROUP_EXEC|GROUP_SOCKET|GROUP_SOCK_UNIX|GROUP_TERMIOS|GROUP_FIFO|GROUP_PTY|GROUP_PARENT, 0, 0, 0 HELP(":<command-line>") };

const struct optdesc opt_dash = { "dash", "login", OPT_DASH, GROUP_EXEC, PH_PREEXEC, TYPE_BOOL, OFUNC_SPEC };

static int xioopen_exec(int argc, const char *argv[], struct opt *opts,
		int xioflags,	/* XIO_RDONLY, XIO_MAYCHILD etc. */
		xiofile_t *fd,
		unsigned groups,
		int dummy1, int dummy2, int dummy3
		) {
   int status;
   bool dash = false;
   int duptostderr;

   if (argc != 2) {
      Error3("\"%s:%s\": wrong number of parameters (%d instead of 1)", argv[0], argv[1], argc-1);
   }
      
   retropt_bool(opts, OPT_DASH, &dash);

   status = _xioopen_foxec(xioflags, &fd->stream, groups, &opts, &duptostderr);
   if (status < 0)  return status;
   if (status == 0) {	/* child */
      const char *ends[] = { " ", NULL };
      const char *hquotes[] = { "'", NULL };
      const char *squotes[] = { "\"", NULL };
      const char *nests[] = {
	 "'", "'",
	 "(", ")",
	 "[", "]",
	 "{", "}",
	 NULL
      } ;
      char **pargv = NULL;
      int pargc;
      size_t len;
      const char *strp;
      char *token; /*! */
      char *tokp;
      char *path = NULL;
      char *tmp;
      int numleft;

      /*! Close(something) */
      /* parse command line */
      Debug1("child: args = \"%s\"", argv[1]);
      pargv = Malloc(8*sizeof(char *));
      if (pargv == NULL)  return STAT_RETRYLATER;
      len = strlen(argv[1])+1;
      strp = argv[1];
      token = Malloc(len); /*! */
      tokp = token;
      if (nestlex(&strp, &tokp, &len, ends, hquotes, squotes, nests,
		  true, true, false) < 0) {
	 Error("internal: miscalculated string lengths");
      }
      *tokp++ = '\0';
      pargv[0] = strrchr(tokp-1, '/');
      if (pargv[0] == NULL)  pargv[0] = token;  else  ++pargv[0];
      pargc = 1;
      while (*strp == ' ') {
	 while (*++strp == ' ')  ;
	 if ((pargc & 0x07) == 0) {
	    pargv = Realloc(pargv, (pargc+8)*sizeof(char *));
	    if (pargv == NULL)  return STAT_RETRYLATER;
	 }
	 pargv[pargc++] = tokp;
	 if (nestlex(&strp, &tokp, &len, ends, hquotes, squotes, nests,
		     true, true, false) < 0) {
	    Error("internal: miscalculated string lengths");
	 }
	 *tokp++ = '\0';
      }
      pargv[pargc] = NULL;

      if ((tmp = Malloc(strlen(pargv[0])+2)) == NULL) {
	 return STAT_RETRYLATER;
      }
      if (dash) {
	 tmp[0] = '-';
	 strcpy(tmp+1, pargv[0]);
      } else {
	 strcpy(tmp, pargv[0]);
      }
      pargv[0] = tmp;

      if (setopt_path(opts, &path) < 0) {
	 /* this could be dangerous, so let us abort this child... */
	 Exit(1);
      }

      if ((numleft = leftopts(opts)) > 0) {
	 Error1("%d option(s) could not be used", numleft);
	 showleft(opts);
	 return STAT_NORETRY;
      }

      /* only now redirect stderr */
      if (duptostderr >= 0) {
	 diag_dup();
	 Dup2(duptostderr, 2);
      }
      Notice1("execvp'ing \"%s\"", token);
      Execvp(token, pargv);
      /* here we come only if execvp() failed */
      switch (pargc) {
      case 1: Error3("execvp(\"%s\", \"%s\"): %s", token, pargv[0], strerror(errno)); break; 
      case 2: Error4("execvp(\"%s\", \"%s\", \"%s\"): %s", token, pargv[0], pargv[1], strerror(errno)); break; 
      case 3:
      default:
	 Error5("execvp(\"%s\", \"%s\", \"%s\", \"%s\", ...): %s", token, pargv[0], pargv[1], pargv[2], strerror(errno)); break; 
      }
      Exit(1);	/* this child process */
   }

   /* parent */
   return 0;
}
#endif /* WITH_EXEC */
