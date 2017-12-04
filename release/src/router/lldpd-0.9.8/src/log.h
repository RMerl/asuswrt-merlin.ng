/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

/* log.c */
void             log_init(int, int, const char *);
void             log_warn(const char *, const char *, ...) __attribute__ ((format (printf, 2, 3)));
void             log_warnx(const char *, const char *, ...) __attribute__ ((format (printf, 2, 3)));
void             log_info(const char *, const char *, ...) __attribute__ ((format (printf, 2, 3)));
void             log_debug(const char *, const char *, ...) __attribute__ ((format (printf, 2, 3)));
void             fatal(const char*, const char *) __attribute__((__noreturn__));
void             fatalx(const char *, const char *) __attribute__((__noreturn__));

void		 log_register(void (*cb)(int, const char*));
void             log_accept(const char *);
void		 log_level(int);

/* version.c */
void		 version_display(FILE *, const char *, int);

#endif
