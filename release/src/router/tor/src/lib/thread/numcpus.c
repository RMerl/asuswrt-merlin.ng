/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file numcpus.c
 * \brief Compute the number of CPUs configured on this system.
 **/

#include "orconfig.h"
#include "lib/thread/numcpus.h"
#include "lib/log/log.h"
#include "lib/log/util_bug.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#include <stdlib.h>

/** Implementation logic for compute_num_cpus(). */
static int
compute_num_cpus_impl(void)
{
#ifdef _WIN32
  SYSTEM_INFO info;
  memset(&info, 0, sizeof(info));
  GetSystemInfo(&info);
  if (info.dwNumberOfProcessors >= 1 && info.dwNumberOfProcessors < INT_MAX)
    return (int)info.dwNumberOfProcessors;
  else
    return -1;
#elif defined(HAVE_SYSCONF)
#ifdef _SC_NPROCESSORS_CONF
  long cpus_conf = sysconf(_SC_NPROCESSORS_CONF);
#else
  long cpus_conf = -1;
#endif
#ifdef _SC_NPROCESSORS_ONLN
  long cpus_onln = sysconf(_SC_NPROCESSORS_ONLN);
#else
  long cpus_onln = -1;
#endif
  long cpus = -1;

  if (cpus_conf > 0 && cpus_onln < 0) {
    cpus = cpus_conf;
  } else if (cpus_onln > 0 && cpus_conf < 0) {
    cpus = cpus_onln;
  } else if (cpus_onln > 0 && cpus_conf > 0) {
    if (cpus_onln < cpus_conf) {
      log_info(LD_GENERAL, "I think we have %ld CPUS, but only %ld of them "
               "are available. Telling Tor to only use %ld. You can over"
               "ride this with the NumCPUs option",
               cpus_conf, cpus_onln, cpus_onln);
    }
    cpus = cpus_onln;
  }

  if (cpus >= 1 && cpus < INT_MAX)
    return (int)cpus;
  else
    return -1;
#else
  return -1;
#endif /* defined(_WIN32) || ... */
}

/** This is an arbitrary number but at this point in time, it is not that
 * uncommon to see servers up to that amount of CPUs. Most servers will likely
 * be around 16 to 32 cores now. Lets take advantage of large machines! The
 * "NumCPUs" torrc option overrides this maximum. */
#define MAX_DETECTABLE_CPUS 128

/** Return how many CPUs we are running with.  We assume that nobody is
 * using hot-swappable CPUs, so we don't recompute this after the first
 * time.  Return -1 if we don't know how to tell the number of CPUs on this
 * system.
 */
int
compute_num_cpus(void)
{
  static int num_cpus = -2;
  if (num_cpus == -2) {
    num_cpus = compute_num_cpus_impl();
    tor_assert(num_cpus != -2);
    if (num_cpus > MAX_DETECTABLE_CPUS) {
      /* LCOV_EXCL_START */
      log_notice(LD_GENERAL, "Wow!  I detected that you have %d CPUs. I "
                 "will not autodetect any more than %d, though.  If you "
                 "want to configure more, set NumCPUs in your torrc",
                 num_cpus, MAX_DETECTABLE_CPUS);
      num_cpus = MAX_DETECTABLE_CPUS;
      /* LCOV_EXCL_STOP */
    }
  }
  return num_cpus;
}
