/* source: xiohelp.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for the help function */

#include "xiosysincludes.h"
#include "xioopen.h"

#include "xiohelp.h"

#if WITH_HELP

/* keep consistent with xioopts.h:enum e_types ! */
static const char *optiontypenames[] = {
	"CONST",	"BIN",		"BOOL",		"BYTE",
	"INT",		"LONG",		"STRING",	"PTRDIFF",
	"SHORT",	"SIZE_T",	"SOCKADDR",	"UNSIGNED-INT",
	"UNSIGNED-LONG","UNSIGNED-SHORT","MODE_T",	"GID_T",
	"UID_T",	"INT[3]",	"STRUCT-TIMEVAL", "STRUCT-TIMESPEC",
	"DOUBLE",	"STRING-NULL",	"LONG-LONG",	"OFF_T",
	"OFF64_T",	"INT:INT",	"INT:INTP",	"INT:BIN",
	"INT:STRING",	"INT:INT:INT",	"INT:INT:BIN",	"INT:INT:STRING",
	"IP4NAME",

#if HAVE_STRUCT_LINGER
			"STRUCT-LINGER",
#endif
#if HAVE_STRUCT_IP_MREQN
					"STRUCT-IP_MREQN",
#elif HAVE_STRUCT_IP_MREQ
					"STRUCT-IP_MREQ",
#endif
} ;


/* keep consistent with xioopts.h:#define GROUP_* ! */
static const char *addressgroupnames[] = {
	"FD",		"FIFO",		"CHR",		"BLK",
	"REG",		"SOCKET",	"READLINE",	"undef",
	"NAMED",	"OPEN",		"EXEC",		"FORK",
	"LISTEN",	"DEVICE",	"CHILD",	"RETRY",
	"TERMIOS",	"RANGE",	"PTY",		"PARENT",
	"UNIX",		"IP4",		"IP6",		"INTERFACE",
	"UDP",		"TCP",		"SOCKS4",	"OPENSSL",
	"PROCESS",	"APPL",		"HTTP",		"SCTP"
} ;


/* keep consistent with xioopts.h:enum ephase ! */
static char *optionphasenames[] = {
	"ALL",		"INIT",		"EARLY",
	"PREOPEN",	"OPEN",		"PASTOPEN",
	"PRESOCKET",	"SOCKET",	"PASTSOCKET",
	"PREBIGEN",	"BIGEN",	"PASTBIGEN",
	"FD",
	"PREBIND",	"BIND",		"PASTBIND",	
	"PRELISTEN",	"LISTEN",	"PASTLISTEN",
	"PRECONNECT",	"CONNECT",	"PASTCONNECT",
	"PREACCEPT",	"ACCEPT",	"PASTACCEPT",
	"CONNECTED",
	"PREFORK",	"FORK",		"PASTFORK",
	"LATE",		"LATE2",
	"PREEXEC",	"EXEC",		"SPECIFIC",
	NULL
} ;


/* print a line about a single option */
static int xiohelp_option(FILE *of, const struct optname *on, const char *name) {
   int j;
   unsigned int groups;
   bool occurred;

   fprintf(of, "      %s\tgroups=", name);
   groups = on->desc->group;  occurred = false;
   for (j = 0; j < 32; ++j) {
      if (groups & 1) {
	 if (occurred)  { fputc(',', of); }
	 fprintf(of, "%s", addressgroupnames[j]);
	 occurred = true;
      }
      groups >>= 1;
   }
   fprintf(of, "\tphase=%s", optionphasenames[on->desc->phase]);
   fprintf(of, "\ttype=%s", optiontypenames[on->desc->type]);
   fputc('\n', of);
   return 0;
}

int xioopenhelp(FILE *of,
	       int level	/* 0..only addresses, 1..and options */
	       ) {
   const struct addrname *an;
   const struct optname *on;
   int i, j;
   unsigned int groups;
   bool occurred;

   fputs("   bi-address:\n", of);
   fputs("      pipe[,<opts>]\tgroups=FD,FIFO\n", of);
   if (level == 2) {
      fputs("      echo is an alias for pipe\n", of);
      fputs("      fifo is an alias for pipe\n", of);
   }
   fputs("      <single-address>!!<single-address>\n", of);
   fputs("      <single-address>\n", of);
   fputs("   single-address:\n", of);
   fputs("      <address-head>[,<opts>]\n", of);
   fputs("   address-head:\n", of);
   an = &addressnames[0];
   i = 0;
   while (addressnames[i].name) {
      if (!strcmp(an->name, an->desc->defname)) {
         /* it is a canonical address name */
	 fprintf(of, "      %s", an->name);
	 if (an->desc->syntax) {
	    fputs(an->desc->syntax, of); }
	 fputs("\tgroups=", of);
	 groups = an->desc->groups;  occurred = false;
	 for (j = 0; j < 32; ++j) {
	    if (groups & 1) {
	       if (occurred) { fputc(',', of); }
	       fprintf(of, "%s", addressgroupnames[j]);
	       occurred = true;
	    }
	    groups >>= 1;
	 }
	 fputc('\n', of);
      } else if (level == 2) {
         fprintf(of, "         %s is an alias name for %s\n", an->name, an->desc->defname);
      }
      ++an; ++i;
   }
   if (level == 2) {
      fputs("         <num> is a short form for fd:<num>\n", of);
      fputs("         <filename> is a short form for gopen:<filename>\n", of);
   }

   if (level <= 0)  return 0;

   fputs("   opts:\n", of);
   fputs("      <opt>{,<opts>}:\n", of);
   fputs("   opt:\n", of);
   on = optionnames;
   while (on->name != NULL) {
      if (on->desc->nickname!= NULL
	  && !strcmp(on->name, on->desc->nickname)) {
	 if (level == 2) {
	    fprintf(of, "      %s is an alias for %s\n", on->name, on->desc->defname);
	 } else {
	    xiohelp_option(of, on, on->name);
	 }
      } else if (on->desc->nickname == NULL &&
		 !strcmp(on->name, on->desc->defname)) {
	 xiohelp_option(of, on, on->name);
      } else if (level == 2) {
	 if (!strcmp(on->name, on->desc->defname)) {
	    xiohelp_option(of, on, on->name);
	 } else {
	    fprintf(of, "      %s is an alias for %s\n", on->name, on->desc->defname);
	 }
      }
      ++on;
   }
   fflush(of);
   return 0;
}

#endif /* WITH_HELP */
