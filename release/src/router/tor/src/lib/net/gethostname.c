/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file gethostname.c
 * \brief Mockable wrapper for gethostname().
 */

#include "orconfig.h"
#include "lib/net/gethostname.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include <winsock2.h>
#endif

/** Get name of current host and write it to <b>name</b> array, whose
 * length is specified by <b>namelen</b> argument. Return 0 upon
 * successful completion; otherwise return return -1. (Currently,
 * this function is merely a mockable wrapper for POSIX gethostname().)
 */
MOCK_IMPL(int,
tor_gethostname,(char *name, size_t namelen))
{
   return gethostname(name,namelen);
}
