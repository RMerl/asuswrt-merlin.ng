/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (http://www.opensource.org/licenses/mit-license.php)
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDARG_H
# include <stdarg.h>
#else /* !HAVE_STDARG_H */
# error Not enough var arg support!
#endif /* HAVE_STDARG_H */

#include "json.h"
#include "debug.h"
#include "printbuf.h"

static int printbuf_initial_size = 32;
static int printbuf_extend(struct printbuf *p, int min_size);

void fjson_global_set_printbuf_initial_size(int size)
{
	printbuf_initial_size = size;
}

struct printbuf* printbuf_new(void)
{
	struct printbuf *p;

	p = (struct printbuf*)malloc(sizeof(struct printbuf));
	if(!p) return NULL;
	/* note: *ALL* data items must be initialized! */
	p->size = printbuf_initial_size;
	p->bpos = 0;
	if(!(p->buf = (char*)malloc(p->size))) {
		free(p);
		return NULL;
	}
	return p;
}


/**
 * Extend the buffer p so it has a size of at least min_size.
 *
 * If the current size is large enough, nothing is changed.
 *
 * Note: this does not check the available space!  The caller
 *  is responsible for performing those calculations.
 */
static int printbuf_extend(struct printbuf *p, int min_size)
{
	char *t;
	int new_size;

	if (p->size >= min_size)
		return 0;

	new_size = p->size * 2;
	if (new_size < min_size + 8)
		new_size =  min_size + 8;
#ifdef PRINTBUF_DEBUG
	MC_DEBUG("printbuf_memappend: realloc "
	  "bpos=%d min_size=%d old_size=%d new_size=%d\n",
	  p->bpos, min_size, p->size, new_size);
#endif /* PRINTBUF_DEBUG */
	if(!(t = (char*)realloc(p->buf, new_size)))
		return -1;
	p->size = new_size;
	p->buf = t;
	return 0;
}

int printbuf_memappend(struct printbuf *p, const char *buf, int size)
{
	if (p->size <= p->bpos + size + 1) {
		if (printbuf_extend(p, p->bpos + size + 1) < 0)
			return -1;
	}
	if(size > 1)
		memcpy(p->buf + p->bpos, buf, size);
	else
		p->buf[p->bpos]= *buf;
	p->bpos += size;
	p->buf[p->bpos]= '\0';
	return size;
}

/* same as printbuf_memappend(), but contains some performance enhancements */
void printbuf_memappend_no_nul(struct printbuf *p, const char *buf, const int size)
{
	if (p->size <= p->bpos + size) {
		if (printbuf_extend(p, p->bpos + size) < 0)
			/* ignore new data, best we can do */
			return;
	}
	memcpy(p->buf + p->bpos, buf, size);
	p->bpos += size;
}

/* add a single character to printbuf */
void printbuf_memappend_char(struct printbuf *p, const char c)
{
	if (p->size <= p->bpos + 1) {
		if (printbuf_extend(p, p->bpos + 1) < 0)
			/* ignore new data, best we can do */
			return;
	}
	p->buf[p->bpos++]= c;
}

void printbuf_terminate_string(struct printbuf *const p)
{
	if (p->size <= p->bpos + 1) {
		if (printbuf_extend(p, p->bpos + 1) < 0)
			--p->bpos; /* overwrite last byte, best we can do */
	}
	p->buf[p->bpos]= '\0';
}

int printbuf_memset(struct printbuf *pb, int offset, int charvalue, int len)
{
	int size_needed;

	if (offset == -1)
		offset = pb->bpos;
	size_needed = offset + len;
	if (pb->size < size_needed)
	{
		if (printbuf_extend(pb, size_needed) < 0)
			return -1;
	}

	memset(pb->buf + offset, charvalue, len);
	if (pb->bpos < size_needed)
		pb->bpos = size_needed;

	return 0;
}

#if !defined(HAVE_VSNPRINTF) /* !HAVE_VSNPRINTF */
# error Need vsnprintf!
#endif /* !HAVE_VSNPRINTF */

#if !defined(HAVE_VASPRINTF)
/* CAW: compliant version of vasprintf */
/* Note: on OpenCSW, we have vasprintf() inside the headers, but not inside the lib.
 * So we need to use a different name, else we get issues with redefinitions. We
 * we solve this by using the macro below, which just renames the function BUT
 * does not affect the (variadic) arguments.
 * rgerhards, 2017-04-11
 */
#define  vasprintf rs_vasprintf
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
static int rs_vasprintf(char **buf, const char *fmt, va_list ap)
{
	int chars;
	char *b;
	static char _T_emptybuffer = '\0';

	if(!buf) { return -1; }

	/* CAW: RAWR! We have to hope to god here that vsnprintf doesn't overwrite
	   our buffer like on some 64bit sun systems.... but hey, its time to move on */
	chars = vsnprintf(&_T_emptybuffer, 0, fmt, ap)+1;
	if(chars < 0) { chars *= -1; } /* CAW: old glibc versions have this problem */

	b = (char*)malloc(sizeof(char)*chars);
	if(!b) { return -1; }

	if((chars = vsprintf(b, fmt, ap)) < 0) {
		free(b);
	} else {
		*buf = b;
	}

	return chars;
}
#pragma GCC diagnostic pop
#endif /* !HAVE_VASPRINTF */

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
	would have been written - this code handles both cases. */
	if(size == -1 || size > 127) {
		va_start(ap, msg);
		if((size = vasprintf(&t, msg, ap)) < 0) { va_end(ap); return -1; }
		va_end(ap);
		printbuf_memappend(p, t, size);
		free(t);
		return size;
	} else {
		printbuf_memappend(p, buf, size);
		return size;
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
