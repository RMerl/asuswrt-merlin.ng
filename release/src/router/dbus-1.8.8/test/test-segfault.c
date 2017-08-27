/* This is simply a process that segfaults */
#include <config.h>
#include <stdlib.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_SETRLIMIT
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

int
main (int argc, char **argv)
{
  char *p;  

#if HAVE_SETRLIMIT
  /* No core dumps please, we know we crashed. */
  struct rlimit r = { 0, };
  
  getrlimit (RLIMIT_CORE, &r);
  r.rlim_cur = 0;
  setrlimit (RLIMIT_CORE, &r);
#endif

#if defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE)
  /* Really, no core dumps please. On Linux, if core_pattern is
   * set to a pipe (for abrt/apport/corekeeper/etc.), RLIMIT_CORE of 0
   * is ignored (deliberately, so people can debug init(8) and other
   * early stuff); but Linux has PR_SET_DUMPABLE, so we can avoid core
   * dumps anyway. */
  prctl (PR_SET_DUMPABLE, 0, 0, 0, 0);
#endif

#ifdef HAVE_RAISE
  raise (SIGSEGV);
#endif
  p = NULL;
  *p = 'a';
  
  return 0;
}
