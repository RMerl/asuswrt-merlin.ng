/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2017 Vincent Bernat <bernat@luffy.cx>
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

#include "writer.h"
#include "../compat/compat.h"
#include "../log.h"

enum tag {
	STRING,
	BOOL,
	ARRAY,
	OBJECT
};

struct element {
	struct element *parent;	   /* Parent (if any) */
	TAILQ_ENTRY(element) next; /* Sibling (if any) */
	char *key;		   /* Key if parent is an object */
	enum tag tag;		   /* Kind of element */
	union {
		char *string;	/* STRING */
		int boolean;	/* BOOL */
		TAILQ_HEAD(, element) children; /* ARRAY or OBJECT */
	};
};

struct json_writer_private {
	FILE *fh;
	int variant;
	struct element *root;
	struct element *current; /* should always be an object */
};

/* Create a new element. If a parent is provided, it will also be attached to
 * the parent. */
static struct element*
json_element_new(struct element *parent, const char *key, enum tag tag)
{
	struct element *child = malloc(sizeof(*child));
	if (child == NULL) fatal(NULL, NULL);
	child->parent = parent;
	child->key = key?strdup(key):NULL;
	child->tag = tag;
	TAILQ_INIT(&child->children);
	if (parent) TAILQ_INSERT_TAIL(&parent->children, child, next);
	return child;
}

/* Free the element content (but not the element itself) */
static void
json_element_free(struct element *current)
{
	struct element *el, *el_next;
	switch (current->tag) {
	case STRING:
		free(current->string);
		break;
	case BOOL:
		break;
	case ARRAY:
	case OBJECT:
		for (el = TAILQ_FIRST(&current->children);
		     el != NULL;
		     el = el_next) {
			el_next = TAILQ_NEXT(el, next);
			json_element_free(el);
			TAILQ_REMOVE(&current->children, el, next);
			if (current->tag == OBJECT) free(el->key);
			free(el);
		}
		break;
	}
}

static void
json_free(struct json_writer_private *p)
{
	json_element_free(p->root);
	free(p->root);
}

static void
json_string_dump(FILE *fh, const char *s)
{
	fprintf(fh, "\"");
	while (*s != '\0') {
		unsigned int c = *s;
		size_t len;
		switch (c) {
		case '"': fprintf(fh, "\\\""); s++; break;
		case '\\': fprintf(fh, "\\\\"); s++; break;
		case '\b': fprintf(fh, "\\b"); s++; break;
		case '\f': fprintf(fh, "\\f"); s++; break;
		case '\n': fprintf(fh, "\\n"); s++; break;
		case '\r': fprintf(fh, "\\r"); s++; break;
		case '\t': fprintf(fh, "\\t"); s++; break;
		default:
			len = utf8_validate_cz(s);
			if (len == 0) {
				/* Not a valid UTF-8 char, use a
				 * replacement character */
				fprintf(fh, "\\uFFFD");
				s++;
			} else if (c < 0x1f) {
				/* 7-bit ASCII character */
				fprintf(fh, "\\u%04X", c);
				s++;
			} else {
				/* UTF-8, write as is */
				while (len--) fprintf(fh, "%c", *s++);
			}
			break;
		}
	}
	fprintf(fh, "\"");
}

/* Dump an element to the specified file handle. */
static void
json_element_dump(FILE *fh, struct element *current, int indent)
{
	static const char pairs[2][2] = { "{}", "[]" };
	struct element *el;
	switch (current->tag) {
	case STRING:
		json_string_dump(fh, current->string);
		break;
	case BOOL:
		fprintf(fh, current->boolean?"true":"false");
		break;
	case ARRAY:
	case OBJECT:
		fprintf(fh, "%c\n%*s", pairs[(current->tag == ARRAY)][0],
		    indent + 2, "");
		TAILQ_FOREACH(el, &current->children, next) {
			if (current->tag == OBJECT)
				fprintf(fh, "\"%s\": ", el->key);
			json_element_dump(fh, el, indent + 2);
			if (TAILQ_NEXT(el, next))
				fprintf(fh, ",\n%*s", indent + 2, "");
		}
		fprintf(fh, "\n%*c", indent + 1,
		    pairs[(current->tag == ARRAY)][1]);
		break;
	}
}

static void
json_dump(struct json_writer_private *p)
{
	json_element_dump(p->fh, p->root, 0);
	fprintf(p->fh, "\n");
}

static void
json_start(struct writer *w, const char *tag,
    const char *descr)
{
	struct json_writer_private *p = w->priv;
	struct element *child;
	struct element *new;

	/* Look for the tag in the current object. */
	TAILQ_FOREACH(child, &p->current->children, next) {
		if (!strcmp(child->key, tag)) break;
	}
	if (!child)
		child = json_element_new(p->current, tag, ARRAY);

	/* Queue the new element. */
	new = json_element_new(child, NULL, OBJECT);
	p->current = new;
}

static void
json_attr(struct writer *w, const char *tag,
    const char *descr, const char *value)
{
	struct json_writer_private *p = w->priv;
	struct element *new = json_element_new(p->current, tag, STRING);
	if (value && (!strcmp(value, "yes") || !strcmp(value, "on"))) {
		new->tag = BOOL;
		new->boolean = 1;
	} else if (value && (!strcmp(value, "no") || !strcmp(value, "off"))) {
		new->tag = BOOL;
		new->boolean = 0;
	} else {
		new->string = strdup(value?value:"");
	}
}

static void
json_data(struct writer *w, const char *data)
{
	struct json_writer_private *p = w->priv;
	struct element *new = json_element_new(p->current, "value", STRING);
	new->string = strdup(data?data:"");
}

/* When an array has only one member, just remove the array. When an object has
 * `value` as the only key, remove the object. Moreover, for an object, move the
 * `name` key outside (inside a new object). This is a recursive function. We
 * think the depth will be limited. Also, the provided element can be
 * destroyed. Don't use it after this function!
 *
 * During the cleaning process, we will generate array of 1-size objects that
 * could be turned into an object. We don't do that since people may rely on
 * this format. Another problem is the format is changing depending on the
 * number of interfaces or the number of neighbors.
 */
static void
json_element_cleanup(struct element *el)
{
#ifndef ENABLE_JSON0
	struct element *child, *child_next;

	/* If array with one element, steal the content. Object with only one
	 * value whose key is "value", steal the content. */
	if ((el->tag == ARRAY || el->tag == OBJECT) &&
	    (child = TAILQ_FIRST(&el->children)) &&
	    !TAILQ_NEXT(child, next) &&
	    (el->tag == ARRAY || !strcmp(child->key, "value"))) {
		free(child->key);
		child->key = el->key;
		child->parent = el->parent;
		TAILQ_INSERT_BEFORE(el, child, next);
		TAILQ_REMOVE(&el->parent->children, el, next);
		free(el);
		json_element_cleanup(child);
		return;
	}

	/* Other kind of arrays, recursively clean */
	if (el->tag == ARRAY) {
		for (child = TAILQ_FIRST(&el->children);
		     child;
		     child = child_next) {
			child_next = TAILQ_NEXT(child, next);
			json_element_cleanup(child);
		}
		return;
	}

	/* Other kind of objects, recursively clean, but if one key is "name",
	 * use it's value as a key for a new object stealing the existing
	 * one. */
	if (el->tag == OBJECT) {
		struct element *name_child = NULL;
		for (child = TAILQ_FIRST(&el->children);
		     child;
		     child = child_next) {
			child_next = TAILQ_NEXT(child, next);
			json_element_cleanup(child);
		}
		/* Redo a check to find if we have a "name" key now */
		for (child = TAILQ_FIRST(&el->children);
		     child;
		     child = child_next) {
			child_next = TAILQ_NEXT(child, next);
			if (!strcmp(child->key, "name") &&
			    child->tag == STRING) {
				name_child = child;
			}
		}
		if (name_child) {
			struct element *new_el = json_element_new(NULL, NULL, OBJECT);
			/* Replace el by new_el in parent object/array */
			new_el->parent = el->parent;
			TAILQ_INSERT_BEFORE(el, new_el, next);
			TAILQ_REMOVE(&el->parent->children, el, next);
			new_el->key = el->key;

			/* new_el is parent of el */
			el->parent = new_el;
			el->key = name_child->string; /* stolen */
			TAILQ_INSERT_TAIL(&new_el->children, el, next);

			/* Remove "name" child */
			TAILQ_REMOVE(&el->children, name_child, next);
			free(name_child->key);
			free(name_child);
		}
		return;
	}
#endif
}

static void
json_cleanup(struct json_writer_private *p)
{
	if (p->variant != 0)
		json_element_cleanup(p->root);
}

static void
json_end(struct writer *w)
{
	struct json_writer_private *p = w->priv;
	while ((p->current = p->current->parent) != NULL && p->current->tag != OBJECT);
	if (p->current == NULL) {
		fatalx("lldpctl", "unbalanced tags");
		return;
	}

	/* Display current object if last one */
	if (p->current == p->root) {
		json_cleanup(p);
		json_dump(p);
		json_free(p);
		fprintf(p->fh,"\n");
		fflush(p->fh);
		p->root = p->current = json_element_new(NULL, NULL, OBJECT);
	}
}

static void
json_finish(struct writer *w)
{
	struct json_writer_private *p = w->priv;
	if (p->current != p->root)
		log_warnx("lldpctl", "unbalanced tags");
	json_free(p);
	free(p);
	free(w);
}

struct writer*
json_init(FILE *fh, int variant)
{
	struct writer *result;
	struct json_writer_private *priv;

	priv = malloc(sizeof(*priv));
	if (priv == NULL) fatal(NULL, NULL);

	priv->fh = fh;
	priv->root = priv->current = json_element_new(NULL, NULL, OBJECT);
	priv->variant = variant;

	result = malloc(sizeof(*result));
	if (result == NULL) fatal(NULL, NULL);

	result->priv   = priv;
	result->start  = json_start;
	result->attr   = json_attr;
	result->data   = json_data;
	result->end    = json_end;
	result->finish = json_finish;

	return result;
}
