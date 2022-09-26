/*	This is pstree written by Fred Hucht (c) 1993-2015	*
 *	EMail: fred AT thp.uni-due.de				*
 *	Feel free to copy and redistribute in terms of the	*
 * 	GNU public license. 					*
 *
 * $Id: pstree.c,v 2.39 2015/05/13 12:24:47 fred Exp $
 */
static char *WhatString[]= {
  "@(#)pstree $Revision: 2.39 $ by Fred Hucht (C) 1993-2015",
  "@(#)EMail: fred AT thp.uni-due.de",
  "$Id: pstree.c,v 2.39 2015/05/13 12:24:47 fred Exp $"
};

#define MAXLINE 8192

#if defined(_AIX) || defined(___AIX)	/* AIX >= 3.1 */
/* Under AIX, we directly read the process table from the kernel */
# ifndef _AIX50
/* problems with getprocs() under AIX 5L
 * workaround contributed by Chris Benesch <chris AT fdbs.com> */
#  define USE_GetProcessesDirect
# endif /*_AIX50*/
#  define HAS_TERMDEF
extern char *termdef(int, char);
#  define _ALL_SOURCE
#  include <procinfo.h>
#  define USE_GETPROCS

#  ifdef USE_GETPROCS
#    define IFNEW(a,b) a
#    define ProcInfo procsinfo
#    ifndef _AIX61
/* workaround contributed by Michael Staats <michael.staats AT gmx.de> */
extern getprocs(struct procsinfo *, int, struct fdsinfo *, int, pid_t *, int);
#    endif /*_AIX61*/
#  else /*USE_GETPROCS*/
#    define IFNEW(a,b) b
#    define ProcInfo procinfo
extern getproc(struct procinfo *, int, int);
extern getuser(struct procinfo *, int, void *, int);
#  endif /*USE_GETPROCS*/

#  ifndef _AIX61
/* workaround contributed by Michael Staats <michael.staats AT gmx.de> */
extern getargs(struct ProcInfo *, int, char *, int);
#  endif /*_AIX61*/

/*#  define PSCMD 	"ps -ekf"
  #  define PSFORMAT 	"%s %ld %ld %*20c %*s %[^\n]"*/
#  define HAS_PGID
#  define UID2USER
#  define PSCMD 	"ps -eko uid,pid,ppid,pgid,thcount,args"
#  define PSFORMAT 	"%ld %ld %ld %ld %ld %[^\n]"
#  define PSVARS	&P[i].uid, &P[i].pid, &P[i].ppid, &P[i].pgid, &P[i].thcount, P[i].cmd
#  define PSVARSN	6
/************************************************************************/
#elif defined(__linux) || (defined __alpha && defined(_SYSTYPE_BSD) || defined (Tru64))
/* TRU64 contributed by Frank Parkin <fparki AT acxiom.co.uk>
 */
#  ifdef __linux
#    define USE_GetProcessesDirect
#    include <glob.h>
#    include <sys/stat.h>
#  endif
#  define UID2USER
#  define HAS_PGID
#  define PSCMD 	"ps -eo uid,pid,ppid,pgid,args"
#  define PSFORMAT 	"%ld %ld %ld %ld %[^\n]"
#  define PSVARS	&P[i].uid, &P[i].pid, &P[i].ppid, &P[i].pgid, P[i].cmd
#  define PSVARSN	5
/************************************************************************/
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
/* NetBSD contributed by Gary D. Duzan <gary AT wheel.tiac.net>
 * FreeBSD contributed by Randall Hopper <rhh AT ct.picker.com> 
 * Darwin / Mac OS X patch by Yuji Yamano <yyamano AT kt.rim.or.jp>
 * wide output format fix for NetBSD by Jeff Brown <jabrown AT caida.org>
 * (Net|Open|Free)BSD & Darwin merged by Ralf Meyer <ralf AT thp.Uni-Duisburg.DE>
 */
#  define HAS_PGID
#  define PSCMD 	"ps -axwwo user,pid,ppid,pgid,command"
#  define PSFORMAT 	"%s %ld %ld %ld %[^\n]"
#  define PSVARS	P[i].name, &P[i].pid, &P[i].ppid, &P[i].pgid, P[i].cmd
#  define PSVARSN	5
#  define ZOMBIES_HAVE_PID_0
/************************************************************************/
#elif defined(sun) && (!defined(__SVR4)) /* Solaris 1.x */
/* contributed by L. Mark Larsen <mlarsen AT ptdcs2.intel.com> */
/* new cpp criteria by Pierre Belanger <belanger AT risq.qc.ca> */
#  define solaris1x
#  define UID2USER
#  ifdef mc68000
/* contributed by Paul Kern <pkern AT utcc.utoronto.ca> */
#    define PSCMD 	"ps laxw"
#    define PSFORMAT 	"%*7c%ld %ld %ld %*d %*d %*d %*x %*d %*d %*x %*14c %[^\n]"
#    define uid_t	int
#    define NEED_STRSTR
#  else
#    define PSCMD 	"ps jaxw"
#    define PSFORMAT 	"%ld %ld %*d %*d %*s %*d %*s %ld %*s %[^\n]"
#    define PSVARS 	&P[i].ppid, &P[i].pid, &P[i].uid, P[i].cmd
#    define PSVARSN	4
#  endif
/************************************************************************/
#elif defined(sun) && (defined(__SVR4)) /* Solaris 2.x */
/* contributed by Pierre Belanger <belanger AT risq.qc.ca> */
#  define solaris2x
#  define PSCMD         "ps -ef"
#  define PSFORMAT      "%s %ld %ld %*d %*s %*s %*s %[^\n]"
/************************************************************************/
#elif defined(bsdi)
/* contributed by Dean Gaudet <dgaudet AT hotwired.com> */
#  define UID2USER
#  define PSCMD 	"ps laxw"
#  define PSFORMAT 	"%ld %ld %ld %*d %*d %*d %*d %*d %*s %*s %*s %*s %[^\n]"
/************************************************************************/
#elif defined(_BSD)	/* Untested */
#  define UID2USER
#  define PSCMD 	"ps laxw"
#  define PSFORMAT 	"%*d %*c %ld %ld %ld %*d %*d %*d %*x %*d %d %*15c %*s %[^\n]"
/************************************************************************/
#elif defined(__convex)	/* ConvexOS */
#  define UID2USER
#  define PSCMD 	"ps laxw"
#  define PSFORMAT 	"%*s %ld %ld %ld %*d %*g %*d %*d %*21c %*s %[^\n]"
/************************************************************************/
#else			/* HP-UX, A/UX etc. */
#  define PSCMD 	"ps -ef"
#  define PSFORMAT 	"%s %ld %ld %*20c %*s %[^\n]"
#endif
/*********************** end of configurable part ***********************/

#ifndef PSVARS 		/* Set default */
# ifdef UID2USER
#  define PSVARS	&P[i].uid, &P[i].pid, &P[i].ppid, P[i].cmd
# else
#  define PSVARS	P[i].name, &P[i].pid, &P[i].ppid, P[i].cmd
# endif
# define PSVARSN	4
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* For str...() */
#ifdef NEED_SNPRINTF
#include <stdarg.h>
int snprintf(char *, int, char *, ...);
#endif
#include <unistd.h>		/* For getopt() */
#include <pwd.h>		/* For getpwnam() */

#include <sys/ioctl.h>		/* For TIOCGSIZE/TIOCGWINSZ */
/* #include <termios.h> */

#ifdef DEBUG
# include <errno.h>
#endif

#ifdef NEED_STRSTR
static char *strstr(char *, char *);
#endif

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

struct TreeChars {
  char *s2, 		/* SS String between header and pid */
    *p, 		/* PP dito, when parent of printed childs */
    *pgl,		/* G  Process group leader */
    *npgl,		/* N  No process group leader */
    *barc, 		/* C  bar for line with child */
    *bar, 		/* B  bar for line without child */
    *barl,		/* L  bar for last child */
    *sg,		/*    Start graphics (alt char set) */
    *eg,		/*    End graphics (alt char set) */
    *init;		/*    Init string sent at the beginning */
};

/* Example:
 * |-+- 01111 ...        CPPN 01111 ...
 * | \-+=   01112 ...    B LPPG 01112 ...
 * |   |--= 01113 ...    B   CSSG 01113 ...
 * |   \--= 01114 ...    B   LSSG 01114 ...
 * \--- 01115 ...        LSSN 01115 ...
 */

enum { G_ASCII = 0, G_PC850 = 1, G_VT100 = 2, G_UTF8 = 3, G_LAST };

/* VT sequences contributed by Randall Hopper <rhh AT ct.picker.com> */
/* UTF8 sequences contributed by Mark-Andre Hopf <mhopf AT mark13.org> */
static struct TreeChars TreeChars[] = {
  /* SS          PP          G       N       C       B       L      sg      eg      init */
  { "--",       "-+",       "=",    "-",    "|",    "|",    "\\",   "",     "",     ""             }, /*Ascii*/
  { "\304\304", "\304\302", "\372", "\304", "\303", "\263", "\300", "",     "",     ""             }, /*Pc850*/
  { "qq",       "qw",       "`",    "q",    "t",    "x",    "m",    "\016", "\017", "\033(B\033)0" }, /*Vt100*/
  { "\342\224\200\342\224\200",
    /**/        "\342\224\200\342\224\254",
    /**/                    "=",
    /**/                            "\342\224\200",
    /**/                                    "\342\224\234",
    /**/                                            "\342\224\202",
    /**/                                                    "\342\224\224",
    /**/                                                            "",     "",     ""             }  /*UTF8*/
}, *C;

static int MyPid, NProc, Columns, RootPid;
static short showall = TRUE, soption = FALSE, Uoption = FALSE;
static char *name = "", *str = NULL, *Progname;
static long ipid = -1;
static char *input = NULL;

static int atLdepth=0;    /* LOPTION - track how deep in the print chain we are */
static int maxLdepth=100; /* LOPTION - will be changed by -l n option */
static int compress = FALSE;

#ifdef DEBUG
static int debug = FALSE;
#endif

struct Proc {
  long uid, pid, ppid, pgid;
  char name[32], cmd[MAXLINE];
  int  print;
  long parent, child, sister;
  unsigned long thcount;
} *P;

#ifdef UID2USER
static void uid2user(uid_t uid, char *name, int len) {
#define NUMUN 128
  static struct un_ {
    uid_t uid;
    char name[32];
  } un[NUMUN];
  static short n = 0;
  short i;
  char uid_name[32];
  char *found;
#ifdef DEBUG
  if (name == NULL) {
    for (i = 0; i < n; i++)
      fprintf(stderr, "uid = %3d, name = %s\n", un[i].uid, un[i].name);
    return;
  }
#endif
  for (i = n - 1; i >= 0 && un[i].uid != uid; i--);
  if (i >= 0) { /* found locally */
    found = un[i].name;
  } else {
    struct passwd *pw = getpwuid(uid);
    if (pw) {
      found = pw->pw_name;
    } else {
      /* fix by Stan Sieler & Philippe Torche */
      snprintf(uid_name, sizeof(uid_name), "#%d", uid);
      found = uid_name;
    }
    if (n < NUMUN) {
      un[n].uid = uid;
      strncpy(un[n].name, found, 9);
      un[n].name[8] = '\0';
      n++;
    }
  }
  strncpy(name, found, len);
  name[len-1] = '\0';
}
#endif

#if defined(_AIX) || defined(___AIX)	/* AIX 3.x / 4.x */
static int GetProcessesDirect(void) {
  int i, nproc, maxnproc = 1024;
  
  struct ProcInfo *proc;
  int idx;
#ifndef USE_GETPROCS
  struct userinfo user;
#endif
  
  do {
    proc = malloc(maxnproc * sizeof(struct ProcInfo));
    if (proc == NULL) {
      fprintf(stderr, "Problems with malloc.\n");
      exit(1);
    }
    
    /* Get process table */
    idx = 0;
    nproc = IFNEW(getprocs(proc, sizeof(struct procsinfo), NULL, 0,
			   &idx, maxnproc),
		  getproc(proc, maxnproc, sizeof(struct procinfo))
		  );
#ifdef DEBUG
    idx = errno; /* Don't ask... */
    if (debug)
      fprintf(stderr,
	      "nproc = %d maxnproc = %d" IFNEW(" idx = %d ","") "\n",
	      nproc, maxnproc, idx);
    errno = idx;
#endif
#ifdef USE_GETPROCS
    if (nproc == -1) {
      perror("getprocs");
      exit(1);
    } else if (nproc == maxnproc) {
      nproc = -1;
    }
#endif
    if (nproc == -1) {
      free(proc);
      maxnproc *= 2;
    } 
  } while (nproc == -1);
  
  P = malloc((nproc+1) * sizeof(struct Proc));
  if (P == NULL) {
    fprintf(stderr, "Problems with malloc.\n");
    exit(1);
  }
  
  for (i = 0; i < nproc; i++) {
#ifndef USE_GETPROCS
    getuser(&proc[i],sizeof(struct procinfo),
	    &user,   sizeof(struct userinfo));
#endif
    P[i].uid     = proc[i].pi_uid;
    P[i].pid     = proc[i].pi_pid;
    P[i].ppid    = proc[i].pi_ppid;
    P[i].pgid    = proc[i].pi_pgrp;
    P[i].thcount = IFNEW(proc[i].pi_thcount, 1);
    
    uid2user(P[i].uid, P[i].name, sizeof(P[i].name));
    
    if (IFNEW(proc[i].pi_state,proc[i].pi_stat) == SZOMB) {
      strcpy(P[i].cmd, "<defunct>");
    } else {
      char *c = P[i].cmd;
      int ci = 0;
      getargs(&proc[i], sizeof(struct procinfo), c, MAXLINE - 2);
      c[MAXLINE-2] = c[MAXLINE-1] = '\0';

      /* Collect args. Stop when we encounter two '\0' */
      while (c[ci] != '\0' && (ci += strlen(&c[ci])) < MAXLINE - 2)
	c[ci++] = ' ';
      
      /* Drop trailing blanks */
      ci = strlen(c);
      while (ci > 0 && c[ci-1] == ' ') ci--;
      c[ci] = '\0';
      
      /* Replace some unprintables with '?' */
      for (ci = 0; c[ci] != '\0'; ci++)
	if (c[ci] == '\n' || c[ci] == '\t') c[ci] = '?';
      
      /* Insert [ui_comm] when getargs returns nothing */
      if (c[0] == '\0') {
	int l = strlen(IFNEW(proc[i].pi_comm,user.ui_comm));
	c[0] = '[';
	strcpy(c+1, IFNEW(proc[i].pi_comm,user.ui_comm));
	c[l+1] = ']';
	c[l+2] = '\0';
      }
    }
#ifdef DEBUG
    if (debug)
      fprintf(stderr,
	      "%d: uid=%5ld, name=%8s, pid=%5ld, ppid=%5ld, pgid=%5ld, tsize=%7u, dvm=%4u, "
	      "thcount=%2d, cmd[%d]='%s'\n",
	      i, P[i].uid, P[i].name, P[i].pid, P[i].ppid, P[i].pgid,
	      IFNEW(proc[i].pi_tsize,user.ui_tsize),
	      IFNEW(proc[i].pi_dvm,user.ui_dvm),
	      proc[i].pi_thcount,
	      strlen(P[i].cmd),P[i].cmd);
#endif
    P[i].parent = P[i].child = P[i].sister = -1;
    P[i].print = FALSE;
  }
  free(proc);
  return nproc;
}

#endif /* _AIX */

#ifdef __linux
static int GetProcessesDirect(void) {
  glob_t globbuf;
  unsigned int i, j;
  
  glob("/proc/[0-9]*", GLOB_NOSORT, NULL, &globbuf);
  
  P = calloc(globbuf.gl_pathc, sizeof(struct Proc));
  if (P == NULL) {
    fprintf(stderr, "Problems with malloc.\n");
    exit(1);
  }
  
  for (i = j = 0; i < globbuf.gl_pathc; i++) {
    char *pdir, name[32];
    int c;
    FILE *tn;
    int k = 0;
    
    pdir = globbuf.gl_pathv[globbuf.gl_pathc - i - 1];
    
    /* if processes change their UID this change is only reflected in the owner of pdir.
     * fixed since version 2.36 */
    {
      struct stat st;
      if (stat(pdir, &st) != 0) { /* get uid */
	continue; /* process vanished since glob() */
      }
      P[j].uid = st.st_uid;
      uid2user(P[j].uid, P[j].name, sizeof(P[j].name));
    }
    
    snprintf(name, sizeof(name), "%s%s",
	     globbuf.gl_pathv[globbuf.gl_pathc - i - 1], "/stat");
    tn = fopen(name, "r");
    if (tn == NULL) continue; /* process vanished since glob() */
    fscanf(tn, "%ld %s %*c %ld %ld",
	   &P[j].pid, P[j].cmd, &P[j].ppid, &P[j].pgid);
    fclose(tn);
    P[j].thcount = 1;
    
    snprintf(name, sizeof(name), "%s%s",
	     globbuf.gl_pathv[globbuf.gl_pathc - i - 1], "/cmdline");
    tn = fopen(name, "r");
    if (tn == NULL) continue; /* process vanished since glob() */
    while (k < MAXLINE - 1 && EOF != (c = fgetc(tn))) {
      P[j].cmd[k++] = c == '\0' ? ' ' : c;
    }
    if (k > 0) P[j].cmd[k] = '\0';
    fclose(tn);
    
#ifdef DEBUG
    if (debug) fprintf(stderr,
		       "uid=%5ld, name=%8s, pid=%5ld, ppid=%5ld, pgid=%5ld, thcount=%ld, cmd='%s'\n",
		       P[j].uid, P[j].name, P[j].pid, P[j].ppid, P[j].pgid, P[j].thcount, P[j].cmd);
#endif
    P[j].parent = P[j].child = P[j].sister = -1;
    P[j].print  = FALSE;
    j++;
  }
  globfree(&globbuf);
  return j;
}
#endif /* __linux */

static int GetProcesses(void) {
  FILE *tn;
  int i = 0;
  char line[MAXLINE], command[] = PSCMD;
  
  /* file read code contributed by Paul Kern <pkern AT utcc.utoronto.ca> */
  if (input != NULL) {
    if (strcmp(input, "-") == 0)
      tn = stdin;
    else if (NULL == (tn = fopen(input,"r"))) {
      perror(input);
      exit(1);
    }
  } else {
#ifdef DEBUG
    if (debug) fprintf(stderr, "calling '%s'\n", command);
#endif
    if (NULL == (tn = (FILE*)popen(command,"r"))) {
      perror("Problems with pipe");
      exit(1);
    }
  }
#ifdef DEBUG
  if (debug) fprintf(stderr, "popen:errno = %d\n", errno);
#endif
  
  if (NULL == fgets(line, MAXLINE, tn)) { /* Throw away header line */
    fprintf(stderr, "No input.\n");
    exit(1);
  }
  
#ifdef DEBUG
  if (debug) fputs(line, stderr);
#endif
  
  P = malloc(sizeof(struct Proc));
  if (P == NULL) {
    fprintf(stderr, "Problems with malloc.\n");
    exit(1);
  }
  
  while (NULL != fgets(line, MAXLINE, tn)) {
    int len, num;
    len = strlen(line);
#ifdef DEBUG
    if (debug) {
      fprintf(stderr, "len=%3d ", len);
      fputs(line, stderr);
    }
#endif
    
    if (len == MAXLINE - 1) { /* line too long, drop remaining stuff */
      char tmp[MAXLINE];
      while (MAXLINE - 1 == strlen(fgets(tmp, MAXLINE, tn)));
    }      
    
    P = realloc(P, (i+1) * sizeof(struct Proc));
    if (P == NULL) {
      fprintf(stderr, "Problems with realloc.\n");
      exit(1);
    }
    
    memset(&P[i], 0, sizeof(*P));
    
#ifdef solaris1x
    { /* SunOS allows columns to run together.  With the -j option, the CPU
       * time used can run into the numeric user id, so make sure there is
       * space between these two columns.  Also, the order of the desired
       * items is different. (L. Mark Larsen <mlarsen AT ptdcs2.intel.com>)
       */
      char buf1[45], buf2[MAXLINE];
      buf1[44] = '\0';
      sscanf(line, "%44c%[^\n]", buf1, buf2);
      snprintf(line, sizeof(line), "%s %s", buf1, buf2);
    }
#endif
    
    num = sscanf(line, PSFORMAT, PSVARS);
    
    if (num != PSVARSN) {
#ifdef DEBUG
      if (debug) fprintf(stderr, "dropped line, num=%d != %d\n", num, PSVARSN);
#endif
      continue;
    }
    
#ifdef UID2USER	/* get username */
    uid2user(P[i].uid, P[i].name, sizeof(P[i].name));
#endif

#ifdef DEBUG
    if (debug) fprintf(stderr,
		      "uid=%5ld, name=%8s, pid=%5ld, ppid=%5ld, pgid=%5ld, thcount=%ld, cmd='%s'\n",
		      P[i].uid, P[i].name, P[i].pid, P[i].ppid, P[i].pgid, P[i].thcount, P[i].cmd);
#endif
    P[i].parent = P[i].child = P[i].sister = -1;
    P[i].print  = FALSE;
    i++;
  }
  if (input != NULL)
    fclose(tn);
  else
    pclose(tn);
  return i;
}

static int GetRootPid(void) {
  int me;
  for (me = 0; me < NProc; me++) {
    if (P[me].pid == 1) return P[me].pid;
  }
  /* PID == 1 not found, so we'll take process with PPID == 0
   * Fix for TRU64 TruCluster with uniq PIDs
   * reported by Frank Parkin <fparki AT acxiom.co.uk>
   * re-reported by Eric van Doorn <Eric.van.Doorn AT isc.politie.nl>,
   * because fix was not published by me :-/ */
  for (me = 0; me < NProc; me++) {
    if (P[me].ppid == 0) return P[me].pid;
  }
  /* OK, still nothing found. Maybe it is FreeBSD and won't show foreign
   * processes. So we also accept PPID == 1 */
  for (me = 0; me < NProc; me++) {
    if (P[me].ppid == 1) return P[me].pid;
  }
  /* Still nothing. Maybe it is something like Solaris Zone. We'll take
   * the process with PID == PPID */
  for (me = 0; me < NProc; me++) {
    if (P[me].pid == P[me].ppid) return P[me].pid;
  }
  /* Should not happen */
  fprintf(stderr,
	  "%s: No process found with PID == 1 || PPID == 0 || PPID == 1\n"
	  "          || PID == PPID, contact author.\n",
	  Progname);
  exit(1);
}

#ifdef ZOMBIES_HAVE_PID_0
void FixZombies(void) {
  int me, num = 0;
  for (me = 0; me < NProc; me++) {
    if (P[me].pid == 0) num++;
  }
  if (num > 1) for (me = 0; me < NProc; me++) {
    if (P[me].pid == 0 && P[me].ppid != 0 && P[me].ppid != -1) {
      P[me].pid = -1;
#ifdef DEBUG
      if (debug) fprintf(stderr,
			 "fixed zombie %s with ppid %ld\n",
			 P[me].cmd, (long)P[me].ppid);
#endif
    }
  }
}
#endif

int get_pid_index(long pid) {
  int me;
  for (me = NProc - 1;me >= 0 && P[me].pid != pid; me--); /* Search process */
  return me;
}

#define EXIST(idx) ((idx) != -1)

static void MakeTree(void) {
  /* Build the process hierarchy. Every process marks itself as first child
   * of it's parent or as sister of first child of it's parent */
  int me;  
  for (me = 0; me < NProc; me++) {
    int parent;
    parent = get_pid_index(P[me].ppid);
    if (parent != me && parent != -1) { /* valid process, not me */
      P[me].parent = parent;
      if (P[parent].child == -1) /* first child */
	P[parent].child = me;
      else {
	int sister;
	for (sister = P[parent].child; EXIST(P[sister].sister); sister = P[sister].sister);
	P[sister].sister = me;
      }
    }
  }
}

static void MarkChildren(int me) {
  int child;
  P[me].print = TRUE;
  for (child = P[me].child; EXIST(child); child = P[child].sister)
    MarkChildren(child);
}

static void MarkProcs(void) {
  int me;
  for (me = 0; me < NProc; me++) {
    if (showall) {
      P[me].print = TRUE;
    } else {
      int parent;
      if (0 == strcmp(P[me].name, name)		/* for -u */
	 || (Uoption &&
	     0 != strcmp(P[me].name, "root"))	/* for -U */
	 || P[me].pid == ipid			/* for -p */
	 || (soption
	     && NULL != strstr(P[me].cmd, str)
	     && P[me].pid != MyPid)		/* for -s */
	 ) {
	/* Mark parents */
	for (parent = P[me].parent; EXIST(parent); parent = P[parent].parent) {
	  P[parent].print = TRUE;
	}
	/* Mark children */
	MarkChildren(me);
      }
    }
#if 0 /* experimental thread compression */
    {
      int parent = P[me].parent;
      int ancestor; /* oldest parent with same cmd */
      if (0 == strcmp(P[me].cmd, P[parent].cmd)) {
	P[me].print = FALSE;
	for (parent = P[me].parent;
	     EXIST(parent) && (0 == strcmp(P[me].cmd, P[parent].cmd));
	     parent = P[parent].parent) {
	  ancestor = parent;
	}
	fprintf(stderr, "%d: %d\n",
		P[me].pid,
		P[ancestor].pid);
	P[ancestor].thcount++;
      }
    }
#endif
  }
}

static void DropProcs(void) {
  int me;
  for (me = 0; me < NProc; me++) if (P[me].print) {
    int child, sister;
    /* Drop children that won't print */
    for (child = P[me].child;
	 EXIST(child) && !P[child].print; child = P[child].sister);
    P[me].child = child;
    /* Drop sisters that won't print */
    for (sister = P[me].sister;
	 EXIST(sister) && !P[sister].print; sister = P[sister].sister);
    P[me].sister = sister;
  }
}

static void PrintTree(int idx, const char *head) {
  char nhead[MAXLINE], out[4 * MAXLINE], thread[16] = {'\0'};
  int child;
  
  if (head[0] == '\0' && !P[idx].print) return;
  /*if (!P[idx].print) return;*/
  
  if (P[idx].thcount > 1) snprintf(thread, sizeof(thread), "[%ld]", P[idx].thcount);
 
  if(atLdepth == maxLdepth) return;    /* LOPTION */
  ++atLdepth;                          /* LOPTION */
 
  
  snprintf(out, sizeof(out),
	   "%s%s%s%s%s%s %05ld %s %s%s" /*" (ch=%d, si=%d, pr=%d)"*/,
	   C->sg,
	   head,
	   head[0] == '\0' ? "" : EXIST(P[idx].sister) ? C->barc : C->barl,
	   EXIST(P[idx].child)       ? C->p   : C->s2,
	   P[idx].pid == P[idx].pgid ? C->pgl : C->npgl,
	   C->eg,
	   P[idx].pid, P[idx].name,
	   thread,
	   P[idx].cmd
	   /*,P[idx].child,P[idx].sister,P[idx].print*/);
  
  out[Columns-1] = '\0';
  puts(out);
  
  /* Process children */
  snprintf(nhead, sizeof(nhead), "%s%s ", head,
	   head[0] == '\0' ? "" : EXIST(P[idx].sister) ? C->bar : " ");

  /*
    if ( compress ) {
    int c1, c2, flag = 0;
    for ( c1 = P[idx].child; EXIST(c1); c1 = P[c1].sister ) {
      for ( c2 = P[c1].sister; EXIST(c2); c2 = P[c2].sister ) {
	if ( 0 == strcmp(P[c1].cmd, P[c2].cmd) ) {
	  flag = 1;
	  printf("%d:%d ", c1, c2);
	  P[c1].pid = -1;
	  P[c2].print = FALSE;
	}
      }
    }
    if ( flag ) printf("\n");
  }
  */

  for (child = P[idx].child; EXIST(child); child = P[child].sister) {
    PrintTree(child, nhead);
  }

  --atLdepth;                          /* LOPTION */

}

static void Usage(void) {
  fprintf(stderr,
	  "%s\n"
	  "%s\n\n"
	  "Usage: %s "
#ifdef DEBUG
	  "[-d] "
#endif
	  "[-f file] [-g n] [-l n] [-u user] [-U] [-s string] [-p pid] [-w] [pid ...]\n"
	  /*"   -a        align output\n"*/
#ifdef DEBUG
	  "   -d        print debugging info to stderr\n"
#endif
	  "   -f file   read input from <file> (- is stdin) instead of running\n"
	  "             \"%s\"\n"
	  "   -g n      use graphics chars for tree. n=1: IBM-850, n=2: VT100, n=3: UTF-8\n"
	  "   -l n      print tree to n level deep\n"
	  "   -u user   show only branches containing processes of <user>\n"
	  "   -U        don't show branches containing only root processes\n"
          "   -s string show only branches containing process with <string> in commandline\n"
          "   -p pid    show only branches containing process <pid>\n"
	  "   -w        wide output, not truncated to window width\n"
	  "   pid ...   process ids to start from, default is 1 (probably init)\n"
	  , WhatString[0] + 4, WhatString[1] + 4, Progname, PSCMD);
#ifdef HAS_PGID
  fprintf(stderr, "\n%sProcess group leaders are marked with '%s%s%s'.\n",
	  C->init, C->sg, C->pgl, C->eg);
#endif
  exit(1);
}

int main(int argc, char **argv) {
  extern int optind;
  extern char *optarg;
  int ch;
  long pid;
  int graph = G_ASCII, wide = FALSE;
  
  C = &TreeChars[graph];
  
  Progname = strrchr(argv[0],'/');
  Progname = (NULL == Progname) ? argv[0] : Progname + 1;
  
  while ((ch = getopt(argc, argv, "cdf:g:hl:p:s:u:Uw?")) != EOF)
    switch(ch) {
      /*case 'a':
	align   = TRUE;
	break;*/
    case 'c':
      compress = TRUE;
      break;
#ifdef DEBUG
    case 'd':
      debug   = TRUE;
      break;
#endif
    case 'f':
      input   = optarg;
      break;
    case 'g':
      graph   = atoi(optarg);
      if (graph < 0 || graph >= G_LAST) {
	fprintf(stderr, "%s: Invalid graph parameter.\n",
		Progname);
	exit(1);
      }
      C = &TreeChars[graph];
      break;
    case 'l':                                 /* LOPTION */
      maxLdepth = atoi(optarg);               /* LOPTION */
      if(maxLdepth < 1) maxLdepth = 1;        /* LOPTION */
      break;                                  /* LOPTION */
    case 'p':
      showall = FALSE;
      ipid    = atoi(optarg);
      break;
    case 's':
      showall = FALSE;
      soption = TRUE;
      str     = optarg;
      break;
    case 'u':
      showall = FALSE;
      name    = optarg;
      if (
#ifdef solaris2x
	 (int)
#endif
	 NULL == getpwnam(name)) {
	fprintf(stderr, "%s: User '%s' does not exist.\n",
		Progname, name);
	exit(1);
      }
      break;
    case 'U':
      showall = FALSE;
      Uoption = TRUE;
      break;
    case 'w':
      wide    = TRUE;
      break;
    case 'h':
    case '?':
    default :
      Usage();
      break;
    }
  
#ifdef USE_GetProcessesDirect
  NProc = input == NULL ? GetProcessesDirect() : GetProcesses();
#else
  NProc = GetProcesses();
#endif
  
#ifdef ZOMBIES_HAVE_PID_0
  FixZombies();
#endif
  
  if (NProc == 0) {
    fprintf(stderr, "%s: No processes read.\n", Progname);
    exit(1);
  }

#ifdef DEBUG
  if (debug) fprintf(stderr, "NProc = %d processes found.\n", NProc);
#endif
  
  RootPid = GetRootPid();

#ifdef DEBUG
  if (debug) fprintf(stderr, "RootPid = %d.\n", RootPid);
#endif

#if defined(UID2USER) && defined(DEBUG)
  if (debug) uid2user(0,NULL,0);
#endif
  MyPid = getpid();
  
  if (wide)
    Columns = MAXLINE - 1;
  else {
#if defined (HAS_TERMDEF)
    Columns = atoi((char*)termdef(fileno(stdout),'c'));
#elif defined(TIOCGWINSZ)
    struct winsize winsize;
    if ( ioctl(fileno(stdout), TIOCGWINSZ, &winsize) != -1 )
      Columns = winsize.ws_col;
#elif defined(TIOCGSIZE)
    struct ttysize ttysize;
    if ( ioctl(fileno(stdout), TIOCGSIZE, &ttysize) != -1 )
      Columns = ttysize.ts_cols;
#else
    char *env = getenv("COLUMNS");
    Columns = env ? atoi(env) : 80;
#endif
  }
  if (Columns == 0) Columns = MAXLINE - 1;
  
  printf("%s", C->init);
  
  Columns += strlen(C->sg) + strlen(C->eg); /* Don't count hidden chars */

  if (Columns >= MAXLINE) Columns = MAXLINE - 1;
  
#ifdef DEBUG
  if (debug) fprintf(stderr, "Columns = %d\n", Columns);
#endif
  
  MakeTree();
  MarkProcs();
  DropProcs();
  
  if (argc == optind) { /* No pids */
    PrintTree(get_pid_index(RootPid), "");
  } else while (optind < argc) {
    int idx;
    pid = (long)atoi(argv[optind]);
    idx = get_pid_index(pid);
    if (idx > -1) PrintTree(idx, "");
    optind++;
  }
  free(P);
  return 0;
}

#ifdef NEED_STRSTR
/* Contributed by Paul Kern <pkern AT utcc.utoronto.ca> */
static char * strstr(s1, s2)
     register char *s1, *s2;
{
  register int n1, n2;
  
  if (n2 = strlen(s2))
    for (n1 = strlen(s1); n1 >= n2; s1++, n1--)
      if (strncmp(s1, s2, n2) == 0)
	return s1;
  return NULL;
}
#endif /* NEED_STRSTR */

#ifdef NEED_SNPRINTF
int snprintf (char *s, int namesiz, char *format, ...) {      
  /* Original portable version by Michael E. White.
     This version of Stan Sieler (sieler AT allegro.com) */

  int  chars_needed;              /* not including trailing null */
  
  char bigbuf [1024] = {'\0'};    /* note: 1024 is a guess, and may not be large enough! */
  
  va_list ap;         /* some systems allow "va_list ap = NULL;", others *do not* (like MACH) */
  
  va_start (ap, format);
  chars_needed = vsprintf (bigbuf, format, ap); /* note: chars_needed does not include trailing null */
  va_end (ap);

  /* 0 is documented as "don't write anything" ... while not specifically spelled out
     (e.g., does it also mean "don't internally call vsprintf"?), one can imply that it simply means
     "don't write to the output buffer 's'.  (Otherwise, if we didn't call vsprintf, we wouldn't
     know what value of chars_needed to return!) */

   if (namesiz <= 0)
     ;     /* Don't touch 's' buffer at all! Note: on some systems, a negative namesiz
	      will cause the process to abort. By checking for <= 0, not just 0, we differ
	      in that area, but it's a reasonable difference. */
   
   else if (chars_needed >= namesiz)  
     {     /* oh oh, output too large for 'name' buffer... */
       memcpy (s, bigbuf, namesiz - 1);
       s [namesiz - 1] = '\0';
     }
   
   else    /* size is ok */
     {
       memcpy (s, bigbuf, chars_needed); /* chars_needed < namesiz */
       s [chars_needed] = '\0';
       /* note: above two could be replaced by strcpy (s, bigbuf)
	  since we know strlen (bigbuf) is acceptable.  
	  But, why copy byte at a time, comparing to null, when
	  we *know* the length? */
     }
   
   return chars_needed;    /* May be larger than namesiz, but that's ok
			      In fact, not just 'ok', it's *useful*! */
}
#endif  /* NEED_SNPRINTF */

/*
 * $Log: pstree.c,v $
 * Revision 2.39  2015/05/13 12:24:47  fred
 * Summary: Don't use uninitialized structs when ioctl() fails, e.g. if run with stdout
 * redirected. Problem reported by Jan Stary.
 *
 * Revision 2.38  2015/04/20 14:50:42  fred
 * Summary: Added patch for AIX61 contributed by Michael Staats
 *
 * Revision 2.37  2015/04/20 10:15:29  fred
 * Summary: V2.36
 *
 * Revision 2.36  2013-04-12 11:47:03+02  fred
 * Some processes like apache under a recent Linux were listed with UID
 * root instead of the correct UID, as they use setuid(). We now read the
 * UID from the owner of /proc/PID instead of /proc/PID/stat, as this
 * seems to be updated correctly. Thanks to Tom Schmidt
 * <tschmidt AT micron.com> for pointing out this bug.
 *
 * Revision 2.35  2013-02-28 08:33:02+01  fred
 * Added Stan Sieler's fix to my adaption of snprintf fix by Stan Sieler :-)
 *
 * Revision 2.34  2013-02-27 16:57:25+01  fred
 * Added snprintf fix by Stan Sieler
 *
 * Revision 2.33  2009-11-10 22:12:39+01  fred
 * Added UTF8, enlarged MAXLINE
 *
 * Revision 2.32  2007-10-26 21:39:50+02  fred
 * Added option -l provided by Michael E. White <mewhite AT us.ibm.com>
 *
 * Revision 2.31  2007-06-08 17:45:23+02  fred
 * Fixed problem with users with long login name (Reported by Oleg A. Mamontov)
 *
 * Revision 2.30  2007-05-10 23:13:04+02  fred
 * *** empty log message ***
 *
 * Revision 2.29  2007-05-10 22:37:13+02  fred
 * Added fix for Solaris Zone and bug fix from Philippe Torche
 *
 * Revision 2.28  2007-05-10 22:01:07+02  fred
 * Added new determination of window width
 *
 * Revision 2.27  2005-04-08 22:08:45+02  fred
 * Also accept PPID==1 if nothing else is found. Should fix problem with
 * FreeBSD and security.bsd.see_other_uids=0.
 *
 * Revision 2.26  2004-10-15 13:59:03+02  fred
 * Fixed small bug with char/int variable c
 * reported by Tomas Dvorak <tomas_dvorak AT mailcan.com>
 *
 * Revision 2.25  2004-05-14 16:41:39+02  fred
 * Added workaround for spurious blank lines in ps output under AIX 5.2
 * reported by Dean Rowswell <rowswell AT ca.ibm.com>
 *
 * Revision 2.24  2004-04-14 09:10:29+02  fred
 * *** empty log message ***
 *
 * Revision 2.23  2004-02-16 10:55:20+01  fred
 * Fix for zombies (pid == 0) under FreeBSD
 *
 * Revision 2.22  2003-12-12 10:58:46+01  fred
 * Added support for TRU64 v5.1b TruCluster
 *
 * Revision 2.21  2003-10-06 13:55:47+02  fred
 * Fixed SEGV under Linux when process table changes during run
 *
 * Revision 2.20  2003-07-09 20:07:29+02  fred
 * cosmetic
 *
 * Revision 2.19  2003/05/26 15:33:35  fred
 * Merged FreeBSD, (Open|Net)BSD; added Darwin (APPLE), fixed wide output
 * in FreeBSD
 *
 * Revision 2.18  2003/03/13 18:53:22  fred
 * Added getenv("COLUMNS"), cosmetic changes
 *
 * Revision 2.17  2001/12/17 12:18:02  fred
 * Changed ps call to something like ps -eo uid,pid,ppid,pgid,args under
 * AIX and Linux, workaround for AIX 5L.
 *
 * Revision 2.17  2001-12-13 08:27:00+08  chris
 * Added workaround for AIX Version >= 5
 *
 * Revision 2.16  2000-03-01 10:42:22+01  fred
 * Added support for thread count (thcount) in other OSs than AIX
 *
 * Revision 2.15  2000-03-01 10:18:56+01  fred
 * Added process group support for {Net|Open}BSD following a suggestion
 * by Ralf Meyer <ralf AT thp.Uni-Duisburg.de>
 *
 * Revision 2.14  1999-03-22 20:45:02+01  fred
 * Fixed bug when line longer than MAXLINE, set MAXLINE=512
 *
 * Revision 2.13  1998-12-17 19:31:53+01  fred
 * Fixed problem with option -f when input file is empty
 *
 * Revision 2.12  1998-12-07 17:08:59+01  fred
 * Added -f option and sun 68000 support by Paul Kern
 * <pkern AT utcc.utoronto.ca>
 *
 * Revision 2.11  1998-05-23 13:30:28+02  fred
 * Added vt100 sequences, NetBSD support
 *
 * Revision 2.10  1998-02-02 15:04:57+01  fred
 * Fixed bug in MakeTree()/get_pid_index() when parent doesn't
 * exist. Thanks to Igor Schein <igor AT andrew.air-boston.com> for the bug
 * report.
 *
 * Revision 2.9  1998-01-07 16:55:26+01  fred
 * Added support for getprocs()
 *
 * Revision 2.9  1998-01-06 17:13:19+01  fred
 * Added support for getprocs() under AIX
 *
 * Revision 2.8  1997-10-22 15:09:39+02  fred
 * Cosmetic
 *
 * Revision 2.7  1997-10-22 15:01:40+02  fred
 * Minor changes in getprocs for AIX
 *
 * Revision 2.6  1997/10/16 16:35:19  fred
 * Added uid2name() caching username lookup, added patch for Solaris 2.x
 *
 * Revision 2.5  1997-02-05 14:24:53+01  fred
 * return PrintTree when nothing to do.
 *
 * Revision 2.4  1997/02/05 09:54:08  fred
 * Fixed bug when P[i].cmd is empty
 *
 * Revision 2.3  1997-02-04 18:40:54+01  fred
 * Cosmetic
 *
 * Revision 2.2  1997-02-04 14:11:17+01  fred
 * *** empty log message ***
 *
 * Revision 2.1  1997-02-04 13:55:14+01  fred
 * Rewritten
 *
 * Revision 1.13  1997-02-04 09:01:59+01  fred
 * Start of rewrite
 *
 * Revision 1.12  1996-09-17 21:54:05+02  fred
 * *** empty log message ***
 *
 * Revision 1.11  1996-09-17 21:52:52+02  fred
 * revision added
 *
 * Revision 1.10  1996-09-17 21:45:35+02  fred
 * replace \n and \t with ? in output
 *
 * Revision 1.4  1996-09-17 21:43:14+02  fred
 * Moved under RCS, replace \n and \t with ?
 */
