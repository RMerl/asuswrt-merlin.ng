/*
 * $Id: debug.c,v 1.3 2004/08/07 03:11:38 mclark Exp $
 *
 * Copyright Metaparadigm Pte. Ltd. 2004.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public (LGPL)
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details: http://www.gnu.org/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/param.h>

#include "debug.h"


static int _syslog = 0;
static int _debug = 0;

void mc_set_debug(int debug) { _debug = debug; }
int mc_get_debug() { return _debug; }

extern void mc_set_syslog(int syslog)
{
  _syslog = syslog;
}

void mc_abort(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  if(_syslog) vsyslog(LOG_ERR, msg, ap);
  else vprintf(msg, ap);
  exit(1);
}


void mc_debug(const char *msg, ...)
{
  va_list ap;
  if(_debug) {
    va_start(ap, msg);
    if(_syslog) vsyslog(LOG_DEBUG, msg, ap);
    else vprintf(msg, ap);
  }
}

void mc_error(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  if(_syslog) vsyslog(LOG_ERR, msg, ap);
  else vfprintf(stderr, msg, ap);
}

void mc_info(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  if(_syslog) vsyslog(LOG_INFO, msg, ap);
  else vfprintf(stderr, msg, ap);
}
