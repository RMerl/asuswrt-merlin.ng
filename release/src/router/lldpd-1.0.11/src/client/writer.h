/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2010 Andreas Hofmeister <andi@collax.com>
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

#ifndef _WRITER_H
#define _WRITER_H

#include <stdio.h>

struct writer {
	void	* priv;
	void	(*start)(struct writer *, const char * tag, const char * descr);
	void	(*attr)(struct writer *, const char * tag, const char * descr, const char * value);
	void	(*data)(struct writer *, const char * data);
	void	(*end)(struct writer *);
	void	(*finish)(struct writer *);
};

#define tag_start(w,...)	w->start(w,## __VA_ARGS__)
#define tag_attr(w,...)		w->attr(w,## __VA_ARGS__)
#define tag_data(w,...)		w->data(w,## __VA_ARGS__)
#define tag_end(w,...)		w->end(w,## __VA_ARGS__)
#define tag_datatag(w,t,d,v)	do { if ((v) == NULL) break; w->start(w,t,d); w->data(w,v); w->end(w); } while(0);

extern struct writer *txt_init(FILE *);
extern struct writer *kv_init(FILE *);
extern struct writer *json_init(FILE *, int);

#ifdef USE_XML
extern struct writer *xml_init(FILE *);
#endif

/* utf8.c */
size_t utf8_validate_cz(const char *s);

#endif /* _WRITER_H */
