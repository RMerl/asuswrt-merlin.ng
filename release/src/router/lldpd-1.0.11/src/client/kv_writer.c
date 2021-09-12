/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2010 Andreas Hofmeister <andi@collax.com>
 *               2010 Vincent Bernat <bernat@luffy.cx>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "writer.h"
#include "../log.h"

#define SEP '.'

struct kv_writer_private {
	FILE *	fh;
	char *  prefix;
};

void
kv_start(struct writer *w , const char *tag, const char *descr)
{
	struct kv_writer_private *p = w->priv;
	char *newprefix;
	int s;

	s = strlen(p->prefix) + 1 + strlen(tag);
	if ((newprefix = malloc(s+1)) == NULL)
		fatal(NULL, NULL);
	if (strlen(p->prefix) > 0)
		snprintf(newprefix, s+1, "%s\1%s", p->prefix, tag);
	else
		snprintf(newprefix, s+1, "%s", tag);
	free(p->prefix);
	p->prefix = newprefix;
}

void
kv_data(struct writer *w, const char *data)
{
	struct kv_writer_private *p = w->priv;
	char *key = strdup(p->prefix);
	char *value = data?strdup(data):NULL;
	char *dot, *nl;
	if (!key) fatal(NULL, NULL);
	while ((dot = strchr(key, '\1')) != NULL)
		*dot = SEP;
	if (value) {
		nl = value;
		while ((nl = strchr(nl, '\n'))) {
			*nl = ' ';
			nl++;
		}
	}
	fprintf(p->fh, "%s=%s\n", key, value?value:"");
	free(key);
	free(value);
}

void
kv_end(struct writer *w)
{
	struct kv_writer_private *p = w->priv;
	char *dot;

	if ((dot = strrchr(p->prefix, '\1')) == NULL) {
		p->prefix[0] = '\0';
		fflush(p->fh);
	} else
		*dot = '\0';
}

void
kv_attr(struct writer *w, const char *tag, const char *descr, const char *value)
{
	if (!strcmp(tag, "name") || !strcmp(tag, "type")) {
		/* Special case for name, replace the last prefix */
		kv_end(w);
		kv_start(w, value, NULL);
	} else {
		kv_start(w, tag, NULL);
		kv_data(w, value);
		kv_end(w);
	}
}

void
kv_finish(struct writer *w)
{
	struct kv_writer_private *p = w->priv;

	free(p->prefix);
	free(w->priv);
	w->priv = NULL;

	free(w);
}

struct writer *
kv_init(FILE *fh)
{

	struct writer *result;
	struct kv_writer_private *priv;

	if ((priv = malloc(sizeof(*priv))) == NULL)
		fatal(NULL, NULL);

	priv->fh = fh;
	priv->prefix = strdup("");

	if ((result = malloc(sizeof(struct writer))) == NULL)
		fatal(NULL, NULL);

	result->priv  = priv;
	result->start = kv_start;
	result->attr  = kv_attr;
	result->data  = kv_data;
	result->end   = kv_end;
	result->finish= kv_finish;

	return result;
}
