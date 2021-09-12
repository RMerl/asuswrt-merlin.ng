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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "writer.h"
#include "../log.h"

static char sep[] = "-------------------------------------------------------------------------------";

struct txt_writer_private {
	FILE *	fh;
	int	level;
	int	attrs;
};

static void
txt_start(struct writer *w , const char *tag, const char *descr) {
	struct txt_writer_private *p = w->priv;
	int i = 0;
	char buf[128];

	if (p->level == 0) {
		fprintf(p->fh, "%s\n", sep);
	} else {
		fprintf(p->fh, "\n");
	}

	for (i = 1; i < p->level; i++) {
		fprintf(p->fh, "  ");
	}

	snprintf(buf, sizeof(buf), "%s:", descr);
	fprintf(p->fh, "%-13s", buf);

	if (p->level == 0)
		fprintf(p->fh, "\n%s", sep);

	p->level++;
	p->attrs = 0;
}

static void
txt_attr(struct writer *w, const char *tag, const char *descr, const char *value) {
	struct txt_writer_private *p = w->priv;

	if (descr == NULL || strlen(descr) == 0) {
		fprintf(p->fh, "%s%s", (p->attrs > 0 ? ", " : " "), value?value:"(none)");
	} else {
		fprintf(p->fh, "%s%s: %s", (p->attrs > 0 ? ", " : " "), descr, value?value:"(none)");
	}

	p->attrs++;
}

static void
txt_data(struct writer *w, const char *data) {
	struct txt_writer_private *p = w->priv;
	char *nl, *begin;
	char *v = begin = data?strdup(data):NULL;

	if (v == NULL) {
		fprintf(p->fh, " %s", data?data:"(none)");
		return;
	}

	fprintf(p->fh, " ");
	while ((nl = strchr(v, '\n')) != NULL) {
		*nl = '\0';
		fprintf(p->fh, "%s\n", v);
		v = nl + 1;

		/* Indent */
		int i;
		for (i = 1; i < p->level - 1; i++) {
			fprintf(p->fh, "  ");
		}
		fprintf(p->fh, "%-14s", " ");
	}
	fprintf(p->fh, "%s", v);
	free(begin);
}

static void
txt_end(struct writer *w) {
	struct txt_writer_private *p = w->priv;
	p->level--;

	if (p->level == 1) {
		fprintf(p->fh, "\n%s", sep);
		fflush(p->fh);
	}
}

static void
txt_finish(struct writer *w) {
	struct txt_writer_private *p = w->priv;

	fprintf(p->fh, "\n");

	free(w->priv);
	w->priv = NULL;

	free(w);
}

struct writer*
txt_init(FILE* fh) {

	struct writer *result;
	struct txt_writer_private *priv;

	priv = malloc(sizeof(*priv));
	if (!priv) {
		fatalx("lldpctl", "out of memory");
		return NULL;
	}

	priv->fh = fh;
	priv->level = 0;
	priv->attrs = 0;

	result = malloc(sizeof(struct writer));
	if (!result) {
		fatalx("lldpctl", "out of memory");
		free(priv);
		return NULL;
	}

	result->priv  = priv;
	result->start = txt_start;
	result->attr  = txt_attr;
	result->data  = txt_data;
	result->end   = txt_end;
	result->finish= txt_finish;

	return result;
}
