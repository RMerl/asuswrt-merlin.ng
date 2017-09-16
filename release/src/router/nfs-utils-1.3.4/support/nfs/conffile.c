/*	$OpenBSD: conf.c,v 1.55 2003/06/03 14:28:16 ho Exp $	*/
/*	$EOM: conf.c,v 1.48 2000/12/04 02:04:29 angelos Exp $	*/

/*
 * Copyright (c) 1998, 1999, 2000, 2001 Niklas Hallqvist.  All rights reserved.
 * Copyright (c) 2000, 2001, 2002 Håkan Olsson.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code was written under funding by Ericsson Radio Systems.
 */

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <syslog.h>

#include "conffile.h"
#include "xlog.h"

#pragma GCC visibility push(hidden)

static void conf_load_defaults(void);
static int conf_set(int , char *, char *, char *, 
	char *, int , int );

struct conf_trans {
	TAILQ_ENTRY (conf_trans) link;
	int trans;
	enum conf_op { CONF_SET, CONF_REMOVE, CONF_REMOVE_SECTION } op;
	char *section;
	char *arg;
	char *tag;
	char *value;
	int override;
	int is_default;
};

TAILQ_HEAD (conf_trans_head, conf_trans) conf_trans_queue;

/*
 * Radix-64 Encoding.
 */
static const uint8_t bin2asc[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const uint8_t asc2bin[] =
{
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255,  62, 255, 255, 255,  63,
   52,  53,  54,  55,  56,  57,  58,  59,
   60,  61, 255, 255, 255, 255, 255, 255,
  255,   0,   1,   2,   3,   4,   5,   6,
    7,   8,   9,  10,  11,  12,  13,  14,
   15,  16,  17,  18,  19,  20,  21,  22,
   23,  24,  25, 255, 255, 255, 255, 255,
  255,  26,  27,  28,  29,  30,  31,  32,
   33,  34,  35,  36,  37,  38,  39,  40,
   41,  42,  43,  44,  45,  46,  47,  48,
   49,  50,  51, 255, 255, 255, 255, 255
};

struct conf_binding {
  LIST_ENTRY (conf_binding) link;
  char *section;
  char *arg;
  char *tag;
  char *value;
  int is_default;
};

char *conf_path;
LIST_HEAD (conf_bindings, conf_binding) conf_bindings[256];

static char *conf_addr;

static __inline__ uint8_t
conf_hash(char *s)
{
	uint8_t hash = 0;

	while (*s) {
		hash = ((hash << 1) | (hash >> 7)) ^ tolower (*s);
		s++;
	}
	return hash;
}

/*
 * Insert a tag-value combination from LINE (the equal sign is at POS)
 */
static int
conf_remove_now(char *section, char *tag)
{
	struct conf_binding *cb, *next;

	cb = LIST_FIRST(&conf_bindings[conf_hash (section)]);
	for (; cb; cb = next) {
		next = LIST_NEXT(cb, link);
		if (strcasecmp(cb->section, section) == 0
				&& strcasecmp(cb->tag, tag) == 0) {
			LIST_REMOVE(cb, link);
			xlog(LOG_INFO,"[%s]:%s->%s removed", section, tag, cb->value);
			free(cb->section);
			free(cb->arg);
			free(cb->tag);
			free(cb->value);
			free(cb);
			return 0;
		}
	}
	return 1;
}

static int
conf_remove_section_now(char *section)
{
  struct conf_binding *cb, *next;
  int unseen = 1;

	cb = LIST_FIRST(&conf_bindings[conf_hash (section)]);
	for (; cb; cb = next) {
		next = LIST_NEXT(cb, link);
		if (strcasecmp(cb->section, section) == 0) {
			unseen = 0;
			LIST_REMOVE(cb, link);
			xlog(LOG_INFO, "[%s]:%s->%s removed", section, cb->tag, cb->value);
			free(cb->section);
			free(cb->arg);
			free(cb->tag);
			free(cb->value);
			free(cb);
			}
		}
	return unseen;
}

/*
 * Insert a tag-value combination from LINE (the equal sign is at POS)
 * into SECTION of our configuration database.
 */
static int
conf_set_now(char *section, char *arg, char *tag, 
	char *value, int override, int is_default)
{
	struct conf_binding *node = 0;

	if (override)
		conf_remove_now(section, tag);
	else if (conf_get_section(section, arg, tag)) {
		if (!is_default) {
			xlog(LOG_INFO, "conf_set: duplicate tag [%s]:%s, ignoring...\n", 
				section, tag);
		}
		return 1;
	}
	node = calloc(1, sizeof *node);
	if (!node) {
		xlog_warn("conf_set: calloc (1, %lu) failed", (unsigned long)sizeof *node);
		return 1;
	}
	node->section = strdup(section);
	if (arg)
		node->arg = strdup(arg);
	node->tag = strdup(tag);
	node->value = strdup(value);
	node->is_default = is_default;

	LIST_INSERT_HEAD(&conf_bindings[conf_hash (section)], node, link);
	return 0;
}

/*
 * Parse the line LINE of SZ bytes.  Skip Comments, recognize section
 * headers and feed tag-value pairs into our configuration database.
 */
static void
conf_parse_line(int trans, char *line, size_t sz)
{
	char *val, *ptr;
	size_t i, valsize;
	size_t j;
	static char *section = 0;
	static char *arg = 0;
	static int ln = 0;

	/* Lines starting with '#' or ';' are comments.  */
	ln++;
	/* Ignore blank lines */
	if (*line == '\0')
		return;

	/* Strip off any leading blanks */
	while (isblank(*line)) 
		line++;

	if (*line == '#' || *line == ';')
		return;

	/* '[section]' parsing...  */
	if (*line == '[') {
		line++;
		/* Strip off any blanks after '[' */
		while (isblank(*line)) 
			line++;
		for (i = 0; i < sz; i++) {
			if (line[i] == ']') {
				break;
			}
		}
		if (section)
			free(section);
		if (i == sz) {
			xlog_warn("config file error: line %d: "
 				"non-matched ']', ignoring until next section", ln);
			section = 0;
			return;
		}
		/* Strip off any blanks before ']' */
		val = line;
		j=0;
		while (*val && !isblank(*val)) 
			val++, j++;
		if (*val)
			i = j;
		section = malloc(i+1);
		if (!section) {
			xlog_warn("conf_parse_line: %d: malloc (%lu) failed", ln,
						(unsigned long)i);
			return;
		}
		strncpy(section, line, i);
		section[i] = '\0';

		if (arg) 
			free(arg);
		arg = 0;

		ptr = strchr(val, '"');
		if (ptr == NULL)
			return;
		line = ++ptr;
		while (*ptr && *ptr != '"' && *ptr != ']')
			ptr++;
		if (*ptr == '\0' || *ptr == ']') {
			xlog_warn("config file error: line %d: "
 				"non-matched '\"', ignoring until next section", ln);
		}  else {
			*ptr = '\0';
			arg = strdup(line);
			if (!arg) 
				xlog_warn("conf_parse_line: %d: malloc arg failed", ln);
		}
		return;
	}

	/* Deal with assignments.  */
	for (i = 0; i < sz; i++) {
		if (line[i] == '=') {
			/* If no section, we are ignoring the lines.  */
			if (!section) {
			xlog_warn("config file error: line %d: "
				"ignoring line due to no section", ln);
				return;
			}
			line[strcspn (line, " \t=")] = '\0';
			val = line + i + 1 + strspn (line + i + 1, " \t");
			valsize = 0;
			while (val[valsize++]);

			/* Skip trailing spaces and comments */
			for (j = 0; j < valsize; j++) {
				if (val[j] == '#' || val[j] == ';' || isspace(val[j])) {
					val[j] = '\0';
					break;
				}
			}
			/* XXX Perhaps should we not ignore errors?  */
			conf_set(trans, section, arg, line, val, 0, 0);
			return;
		}
	}
	/* Other non-empty lines are weird.  */
	i = strspn(line, " \t");
	if (line[i])
		xlog_warn("config file error: line %d:", ln);

	return;
}

/* Parse the mapped configuration file.  */
static void
conf_parse(int trans, char *buf, size_t sz)
{
	char *cp = buf;
	char *bufend = buf + sz;
	char *line;

	line = cp;
	while (cp < bufend) {
		if (*cp == '\n') {
			/* Check for escaped newlines.  */
			if (cp > buf && *(cp - 1) == '\\')
				*(cp - 1) = *cp = ' ';
			else {
				*cp = '\0';
				conf_parse_line(trans, line, cp - line);
				line = cp + 1;
			}
		}
		cp++;
	}
	if (cp != line)
		xlog_warn("conf_parse: last line non-terminated, ignored.");
}

static void
conf_load_defaults(void)
{
	/* No defaults */
	return;
}

void
conf_init (void)
{
	unsigned int i;

	for (i = 0; i < sizeof conf_bindings / sizeof conf_bindings[0]; i++)
		LIST_INIT (&conf_bindings[i]);

	TAILQ_INIT (&conf_trans_queue);
	conf_reinit();
}

/* Open the config file and map it into our address space, then parse it.  */
void
conf_reinit(void)
{
	struct conf_binding *cb = 0;
	int fd, trans;
	unsigned int i;
	size_t sz;
	char *new_conf_addr = 0;
	struct stat sb;

	if ((stat (conf_path, &sb) == 0) || (errno != ENOENT)) {
		sz = sb.st_size;
		fd = open (conf_path, O_RDONLY, 0);
		if (fd == -1) {
			xlog_warn("conf_reinit: open (\"%s\", O_RDONLY) failed", conf_path);
			return;
		}

		new_conf_addr = malloc(sz);
		if (!new_conf_addr) {
			xlog_warn("conf_reinit: malloc (%lu) failed", (unsigned long)sz);
			goto fail;
		}

		/* XXX I assume short reads won't happen here.  */
		if (read (fd, new_conf_addr, sz) != (int)sz) {
			xlog_warn("conf_reinit: read (%d, %p, %lu) failed",
   				fd, new_conf_addr, (unsigned long)sz);
			goto fail;
		}
		close(fd);

		trans = conf_begin();
		/* XXX Should we not care about errors and rollback?  */
		conf_parse(trans, new_conf_addr, sz);
	}
	else
		trans = conf_begin();

	/* Load default configuration values.  */
	conf_load_defaults();

	/* Free potential existing configuration.  */
	if (conf_addr) {
		for (i = 0; i < sizeof conf_bindings / sizeof conf_bindings[0]; i++) {
			cb = LIST_FIRST (&conf_bindings[i]);
			for (; cb; cb = LIST_FIRST (&conf_bindings[i]))
				conf_remove_now(cb->section, cb->tag);
		}
		free (conf_addr);
	}

	conf_end(trans, 1);
	conf_addr = new_conf_addr;
	return;

fail:
	if (new_conf_addr)
		free(new_conf_addr);
	close (fd);
}

/*
 * Return the numeric value denoted by TAG in section SECTION or DEF
 * if that tag does not exist.
 */
int
conf_get_num(char *section, char *tag, int def)
{
	char *value = conf_get_str(section, tag);

	if (value)
		return atoi(value);

	return def;
}

/* Validate X according to the range denoted by TAG in section SECTION.  */
int
conf_match_num(char *section, char *tag, int x)
{
	char *value = conf_get_str (section, tag);
	int val, min, max, n;

	if (!value)
		return 0;
	n = sscanf (value, "%d,%d:%d", &val, &min, &max);
	switch (n) {
	case 1:
		xlog(LOG_INFO, "conf_match_num: %s:%s %d==%d?", section, tag, val, x);
		return x == val;
	case 3:
		xlog(LOG_INFO, "conf_match_num: %s:%s %d<=%d<=%d?", section, 
			tag, min, x, max);
		return min <= x && max >= x;
	default:
		xlog(LOG_INFO, "conf_match_num: section %s tag %s: invalid number spec %s",
			section, tag, value);
	}
	return 0;
}

/* Return the string value denoted by TAG in section SECTION.  */
char *
conf_get_str(char *section, char *tag)
{
	struct conf_binding *cb;

	cb = LIST_FIRST (&conf_bindings[conf_hash (section)]);
	for (; cb; cb = LIST_NEXT (cb, link)) {
		if (strcasecmp (section, cb->section) == 0
				&& strcasecmp (tag, cb->tag) == 0)
			return cb->value;
	}
	return 0;
}
/*
 * Find a section that may or may not have an argument
 */
char *
conf_get_section(char *section, char *arg, char *tag)
{
	struct conf_binding *cb;

	cb = LIST_FIRST (&conf_bindings[conf_hash (section)]);
	for (; cb; cb = LIST_NEXT (cb, link)) {
		if (strcasecmp(section, cb->section) != 0)
			continue;
		if (arg && strcasecmp(arg, cb->arg) != 0)
			continue;
		if (strcasecmp(tag, cb->tag) != 0)
			continue;
		return cb->value;
	}
	return 0;
}

/*
 * Build a list of string values out of the comma separated value denoted by
 * TAG in SECTION.
 */
struct conf_list *
conf_get_list(char *section, char *tag)
{
	char *liststr = 0, *p, *field, *t;
	struct conf_list *list = 0;
	struct conf_list_node *node;

	list = malloc (sizeof *list);
	if (!list)
		goto cleanup;
	TAILQ_INIT (&list->fields);
	list->cnt = 0;
	liststr = conf_get_str(section, tag);
	if (!liststr)
		goto cleanup;
	liststr = strdup (liststr);
	if (!liststr)
		goto cleanup;
	p = liststr;
	while ((field = strsep (&p, ",")) != NULL) {
		/* Skip leading whitespace */
		while (isspace (*field))
			field++;
		/* Skip trailing whitespace */
		if (p) {
			for (t = p - 1; t > field && isspace (*t); t--)
				*t = '\0';
		}
		if (*field == '\0') {
			xlog(LOG_INFO, "conf_get_list: empty field, ignoring...");
			continue;
		}
		list->cnt++;
		node = calloc (1, sizeof *node);
		if (!node)
			goto cleanup;
		node->field = strdup (field);
		if (!node->field) {
			free(node);
			goto cleanup;
		}
		TAILQ_INSERT_TAIL (&list->fields, node, link);
	}
	free (liststr);
	return list;

cleanup:
	if (list)
		conf_free_list(list);
	if (liststr)
		free(liststr);
	return 0;
}

struct conf_list *
conf_get_tag_list(char *section, char *arg)
{
	struct conf_list *list = 0;
	struct conf_list_node *node;
	struct conf_binding *cb;

	list = malloc(sizeof *list);
	if (!list)
		goto cleanup;
	TAILQ_INIT(&list->fields);
	list->cnt = 0;
	cb = LIST_FIRST(&conf_bindings[conf_hash (section)]);
	for (; cb; cb = LIST_NEXT(cb, link)) {
		if (strcasecmp (section, cb->section) == 0) {
			if (arg != NULL && strcasecmp(arg, cb->arg) != 0)
				continue;
			list->cnt++;
			node = calloc(1, sizeof *node);
			if (!node)
				goto cleanup;
			node->field = strdup(cb->tag);
			if (!node->field) {
				free(node);
				goto cleanup;
			}
			TAILQ_INSERT_TAIL(&list->fields, node, link);
		}
	}
	return list;

cleanup:
	if (list)
		conf_free_list(list);
	return 0;
}

/* Decode a PEM encoded buffer.  */
int
conf_decode_base64 (uint8_t *out, uint32_t *len, unsigned char *buf)
{
	uint32_t c = 0;
	uint8_t c1, c2, c3, c4;

	while (*buf) {
		if (*buf > 127 || (c1 = asc2bin[*buf]) == 255)
			return 0;

		buf++;
		if (*buf > 127 || (c2 = asc2bin[*buf]) == 255)
			return 0;

		buf++;
		if (*buf == '=') {
			c3 = c4 = 0;
			c++;

			/* Check last four bit */
			if (c2 & 0xF)
				return 0;

			if (strcmp((char *)buf, "==") == 0)
				buf++;
			else
				return 0;
		} else if (*buf > 127 || (c3 = asc2bin[*buf]) == 255)
			return 0;
		else {
			if (*++buf == '=') {
				c4 = 0;
				c += 2;

				/* Check last two bit */
				if (c3 & 3)
					return 0;

			if (strcmp((char *)buf, "="))
				return 0;
			} else if (*buf > 127 || (c4 = asc2bin[*buf]) == 255)
				return 0;
			else
				c += 3;
		}

		buf++;
		*out++ = (c1 << 2) | (c2 >> 4);
		*out++ = (c2 << 4) | (c3 >> 2);
		*out++ = (c3 << 6) | c4;
	}

	*len = c;
	return 1;
}

void
conf_free_list(struct conf_list *list)
{
	struct conf_list_node *node = TAILQ_FIRST(&list->fields);

	while (node) {
		TAILQ_REMOVE(&list->fields, node, link);
		if (node->field)
			free(node->field);
		free (node);
		node = TAILQ_FIRST(&list->fields);
	}
	free (list);
}

int
conf_begin(void)
{
  static int seq = 0;

  return ++seq;
}

static struct conf_trans *
conf_trans_node(int transaction, enum conf_op op)
{
	struct conf_trans *node;

	node = calloc (1, sizeof *node);
	if (!node) {
		xlog_warn("conf_trans_node: calloc (1, %lu) failed",
		(unsigned long)sizeof *node);
		return 0;
	}
	node->trans = transaction;
	node->op = op;
	TAILQ_INSERT_TAIL (&conf_trans_queue, node, link);
	return node;
}

/* Queue a set operation.  */
static int
conf_set(int transaction, char *section, char *arg,
	char *tag, char *value, int override, int is_default)
{
	struct conf_trans *node;

	node = conf_trans_node(transaction, CONF_SET);
	if (!node)
		return 1;
	node->section = strdup(section);
	if (!node->section) {
		xlog_warn("conf_set: strdup(\"%s\") failed", section);
		goto fail;
	}
	/* Make Section names case-insensitive */
	upper2lower(node->section);

	if (arg) {
		node->arg = strdup(arg);
		if (!node->arg) {
			xlog_warn("conf_set: strdup(\"%s\") failed", arg);
			goto fail;
		}
	} else
		node->arg = NULL;

	node->tag = strdup(tag);
	if (!node->tag) {
		xlog_warn("conf_set: strdup(\"%s\") failed", tag);
		goto fail;
	}
	node->value = strdup(value);
	if (!node->value) {
		xlog_warn("conf_set: strdup(\"%s\") failed", value);
		goto fail;
	}
	node->override = override;
	node->is_default = is_default;
	return 0;

fail:
	if (node->tag)
		free(node->tag);
	if (node->section)
		free(node->section);
	if (node)
		free(node);
	return 1;
}

/* Queue a remove operation.  */
int
conf_remove(int transaction, char *section, char *tag)
{
	struct conf_trans *node;

	node = conf_trans_node(transaction, CONF_REMOVE);
	if (!node)
		goto fail;
	node->section = strdup(section);
	if (!node->section) {
		xlog_warn("conf_remove: strdup(\"%s\") failed", section);
		goto fail;
	}
	node->tag = strdup(tag);
	if (!node->tag) {
		xlog_warn("conf_remove: strdup(\"%s\") failed", tag);
		goto fail;
	}
	return 0;

fail:
	if (node && node->section)
		free (node->section);
	if (node)
		free (node);
	return 1;
}

/* Queue a remove section operation.  */
int
conf_remove_section(int transaction, char *section)
{
	struct conf_trans *node;

	node = conf_trans_node(transaction, CONF_REMOVE_SECTION);
	if (!node)
		goto fail;
	node->section = strdup(section);
	if (!node->section) {
		xlog_warn("conf_remove_section: strdup(\"%s\") failed", section);
		goto fail;
	}
	return 0;

fail:
	if (node)
		free(node);
	return 1;
}

/* Execute all queued operations for this transaction.  Cleanup.  */
int
conf_end(int transaction, int commit)
{
	struct conf_trans *node, *next;

	for (node = TAILQ_FIRST(&conf_trans_queue); node; node = next) {
		next = TAILQ_NEXT(node, link);
		if (node->trans == transaction) {
			if (commit) {
				switch (node->op) {
				case CONF_SET:
					conf_set_now(node->section, node->arg, 
						node->tag, node->value, node->override, 
						node->is_default);
					break;
				case CONF_REMOVE:
					conf_remove_now(node->section, node->tag);
					break;
				case CONF_REMOVE_SECTION:
					conf_remove_section_now(node->section);
					break;
				default:
					xlog(LOG_INFO, "conf_end: unknown operation: %d", node->op);
				}
			}
			TAILQ_REMOVE (&conf_trans_queue, node, link);
			if (node->section)
				free(node->section);
			if (node->tag)
				free(node->tag);
			if (node->value)
				free(node->value);
			free (node);
		}
	}
	return 0;
}

/*
 * Dump running configuration upon SIGUSR1.
 * Configuration is "stored in reverse order", so reverse it again.
 */
struct dumper {
	char *s, *v;
	struct dumper *next;
};

static void
conf_report_dump(struct dumper *node)
{
	/* Recursive, cleanup when we're done.  */
	if (node->next)
		conf_report_dump(node->next);

	if (node->v)
		xlog(LOG_INFO, "%s=\t%s", node->s, node->v);
	else if (node->s) {
		xlog(LOG_INFO, "%s", node->s);
		if (strlen(node->s) > 0)
			free(node->s);
	}

	free (node);
}

void
conf_report (void)
{
	struct conf_binding *cb, *last = 0;
	unsigned int i, len, diff_arg = 0;
	char *current_section = (char *)0;
	char *current_arg = (char *)0;
	struct dumper *dumper, *dnode;

	dumper = dnode = (struct dumper *)calloc(1, sizeof *dumper);
	if (!dumper)
		goto mem_fail;

	xlog(LOG_INFO, "conf_report: dumping running configuration");

	for (i = 0; i < sizeof conf_bindings / sizeof conf_bindings[0]; i++)
		for (cb = LIST_FIRST(&conf_bindings[i]); cb; cb = LIST_NEXT(cb, link)) {
			if (!cb->is_default) {
				/* Make sure the Section arugment is the same */
				if (current_arg && current_section && cb->arg) {
					if (strcmp(cb->section, current_section) == 0 &&
						strcmp(cb->arg, current_arg) != 0)
					diff_arg = 1;
				}
				/* Dump this entry.  */
				if (!current_section || strcmp(cb->section, current_section) 
							|| diff_arg) {
					if (current_section || diff_arg) {
						len = strlen (current_section) + 3;
						if (current_arg)
							len += strlen(current_arg) + 3;
						dnode->s = malloc(len);
						if (!dnode->s)
							goto mem_fail;

						if (current_arg)
							snprintf(dnode->s, len, "[%s \"%s\"]", 
								current_section, current_arg);
						else
							snprintf(dnode->s, len, "[%s]", current_section);

						dnode->next = 
							(struct dumper *)calloc(1, sizeof (struct dumper));
						dnode = dnode->next;
						if (!dnode)
							goto mem_fail;

						dnode->s = "";
						dnode->next = 
							(struct dumper *)calloc(1, sizeof (struct dumper));
						dnode = dnode->next;
						if (!dnode)
						goto mem_fail;
					}
					current_section = cb->section;
					current_arg = cb->arg;
					diff_arg = 0;
				}
				dnode->s = cb->tag;
				dnode->v = cb->value;
				dnode->next = (struct dumper *)calloc (1, sizeof (struct dumper));
				dnode = dnode->next;
				if (!dnode)
					goto mem_fail;
				last = cb;
		}
	}

	if (last) {
		len = strlen(last->section) + 3;
		if (last->arg)
			len += strlen(last->arg) + 3;
		dnode->s = malloc(len);
		if (!dnode->s)
			goto mem_fail;
		if (last->arg)
			snprintf(dnode->s, len, "[%s \"%s\"]", last->section, last->arg);
		else
			snprintf(dnode->s, len, "[%s]", last->section);
	}
	conf_report_dump(dumper);
	return;

mem_fail:
	xlog_warn("conf_report: malloc/calloc failed");
	while ((dnode = dumper) != 0) {
		dumper = dumper->next;
		if (dnode->s)
			free(dnode->s);
		free(dnode);
	}
	return;
}
