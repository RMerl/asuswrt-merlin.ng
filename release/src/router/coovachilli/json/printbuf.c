/*
 * $Id: printbuf.c,v 1.3 2004/08/07 03:12:21 mclark Exp $
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
#include <stdarg.h>
#include <string.h>

#include "bits.h"
#include "debug.h"
#include "printbuf.h"


struct printbuf* printbuf_new()
{
  struct printbuf *p;

  if(!(p = calloc(1, sizeof(struct printbuf)))) return NULL;
  p->size = 32;
  p->bpos = 0;
  if(!(p->buf = malloc(p->size))) {
    free(p);
    return NULL;
  }
  return p;
}


int printbuf_memappend(struct printbuf *p, char *buf, int size)
{
  char *t;
  if(p->size - p->bpos <= size) {
    int new_size = max(p->size * 2, p->bpos + size + 8);
#if 0
    mc_debug("printbuf_memappend: realloc "
	     "bpos=%d wrsize=%d old_size=%d new_size=%d\n",
	     p->bpos, size, p->size, new_size);
#endif
    if(!(t = realloc(p->buf, new_size))) return -1;
    p->size = new_size;
    p->buf = t;
  }
  memcpy(p->buf + p->bpos, buf, size);
  p->bpos += size;
  p->buf[p->bpos]= '\0';
  return size;
}


int sprintbuf(struct printbuf *p, const char *msg, ...)
{
  va_list ap;
  char *t;
  int size;
  char buf[128];

  /* user stack buffer first */
  va_start(ap, msg);
  size = vsnprintf(buf, 128, msg, ap);
  va_end(ap);
  /* if string is greater than stack buffer, then use dynamic string
     with vasprintf.  Note: some implementation of vsnprintf return -1
     if output is truncated whereas some return the number of bytes that
     would have been writeen - this code handles both cases. */
  if(size == -1 || size > 127) {
    int ret;
    va_start(ap, msg);
    if((size = vasprintf(&t, msg, ap)) == -1) return -1;
    va_end(ap);
    ret = printbuf_memappend(p, t, size);
    free(t);
    return ret;
  } else {
    return printbuf_memappend(p, buf, size);
  }
}


void printbuf_reset(struct printbuf *p)
{
  p->buf[0] = '\0';
  p->bpos = 0;
}

void printbuf_free(struct printbuf *p)
{
  if(p) {
    free(p->buf);
    free(p);
  }
}
