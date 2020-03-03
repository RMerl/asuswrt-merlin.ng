/*
 * $Id: debug.c,v 1.5 2006/01/26 02:16:28 Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if HAVE_SYSLOG_H
# include <syslog.h>
#endif /* HAVE_SYSLOG_H */

#if HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#include "debug.h"

static int _syslog = 0;
static int _debug = 0;

void mc_set_debug(int debug) { _debug = debug; }
int mc_get_debug(void) { return _debug; }

extern void mc_set_syslog(int syslog)
{
  _syslog = syslog;
}

void mc_abort(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
  if(_syslog) {
	  vsyslog(LOG_ERR, msg, ap);
  } else
#endif // endif
	  vprintf(msg, ap);
  va_end(ap);
  exit(1);
}

void mc_debug(const char *msg, ...)
{
  va_list ap;
  if(_debug) {
    va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_DEBUG, msg, ap);
	} else
#endif // endif
		vprintf(msg, ap);
    va_end(ap);
  }
}

void mc_error(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_ERR, msg, ap);
	} else
#endif // endif
		vfprintf(stderr, msg, ap);
  va_end(ap);
}

void mc_info(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
#if HAVE_VSYSLOG
    if(_syslog) {
		vsyslog(LOG_INFO, msg, ap);
	} else
#endif // endif
		vfprintf(stderr, msg, ap);
  va_end(ap);
}
