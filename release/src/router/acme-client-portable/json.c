/*	$Id$ */
/*
 * Copyright (c) 2016 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "jsmn.h"
#include "extern.h"

struct	jsmnp;

/*
 * A node in the JSMN parse tree.
 * Each of this corresponds to an object in the original JSMN token
 * list, although the contents have been extracted properly.
 */
struct	jsmnn {
	struct parse	*p; /* parser object */
	union {
		char *str; /* JSMN_PRIMITIVE, JSMN_STRING */
		struct jsmnp *obj; /* JSMN_OBJECT */
		struct jsmnn **array; /* JSMN_ARRAY */
	} d;
	size_t		 fields; /* entries in "d" */
	jsmntype_t	 type; /* type of node */
};

/*
 * Objects consist of node pairs: the left-hand side (before the colon)
 * and the right-hand side---the data.
 */
struct	jsmnp {
	struct jsmnn	*lhs; /* left of colon */
	struct jsmnn	*rhs; /* right of colon */
};

/*
 * Object for converting the JSMN token array into a tree.
 */
struct	parse {
	struct jsmnn	*nodes; /* all nodes */
	size_t		 cur; /* current number */
	size_t		 max; /* nodes in "nodes" */
};

/*
 * Recursive part for convertin a JSMN token array into a tree.
 * See "example/jsondump.c" for its construction (it's the same except
 * for how it handles allocation errors).
 */
static ssize_t
build(struct parse *parse, struct jsmnn **np,
	jsmntok_t *t, const char *js, size_t sz)
{
	size_t		 i, j;
	struct jsmnn	*n;
	ssize_t		 tmp;

	if (0 == sz)
		return (0);

	assert(parse->cur < parse->max);
	n = *np = &parse->nodes[parse->cur++];
	n->p = parse;
	n->type = t->type;

	switch (t->type) {
	case (JSMN_STRING):
		/* FALLTHROUGH */
	case (JSMN_PRIMITIVE):
		n->fields = 1;
		n->d.str = strndup
			(js + t->start,
			 t->end - t->start);
		if (NULL == n->d.str)
			break;
		return (1);
	case (JSMN_OBJECT):
		n->fields = t->size;
		n->d.obj = calloc(n->fields,
			sizeof(struct jsmnp));
		if (NULL == n->d.obj)
			break;
		for (i = j = 0; i < (size_t)t->size; i++) {
			tmp = build(parse,
				&n->d.obj[i].lhs,
				t + 1 + j, js, sz - j);
			if (tmp < 0)
				break;
			j += tmp;
			tmp = build(parse,
				&n->d.obj[i].rhs,
				t + 1 + j, js, sz - j);
			if (tmp < 0)
				break;
			j += tmp;
		}
		if (i < (size_t)t->size)
			break;
		return (j + 1);
	case (JSMN_ARRAY):
		n->fields = t->size;
		n->d.array = calloc(n->fields,
			sizeof(struct jsmnn *));
		if (NULL == n->d.array)
			break;
		for (i = j = 0; i < (size_t)t->size; i++) {
			tmp = build(parse,
				&n->d.array[i],
				t + 1 + j, js, sz - j);
			if (tmp < 0)
				break;
			j += tmp;
		}
		if (i < (size_t)t->size)
			break;
		return (j + 1);
	default:
		break;
	}

	return (-1);
}

/*
 * Fully free up a parse sequence.
 * This handles all nodes sequentially, not recursively.
 */
static void
jsmnparse_free(struct parse *p)
{
	size_t	 i;

	if (NULL == p)
		return;
	for (i = 0; i < p->max; i++)
		if (JSMN_ARRAY == p->nodes[i].type)
			free(p->nodes[i].d.array);
		else if (JSMN_OBJECT == p->nodes[i].type)
			free(p->nodes[i].d.obj);
		else if (JSMN_PRIMITIVE == p->nodes[i].type)
			free(p->nodes[i].d.str);
		else if (JSMN_STRING == p->nodes[i].type)
			free(p->nodes[i].d.str);
	free(p->nodes);
	free(p);
}

/*
 * Allocate a tree representation of "t".
 * This returns NULL on allocation failure or when sz is zero, in which
 * case all resources allocated along the way are freed already.
 */
static struct jsmnn *
jsmntree_alloc(jsmntok_t *t, const char *js, size_t sz)
{
	struct jsmnn	*first;
	struct parse	*p;

	if (0 == sz)
		return (NULL);

	p = calloc(1, sizeof(struct parse));
	if (NULL == p)
		return (NULL);

	p->max = sz;
	p->nodes = calloc(p->max, sizeof(struct jsmnn));
	if (NULL == p->nodes) {
		free(p);
		return (NULL);
	}

	if (build(p, &first, t, js, sz) < 0) {
		jsmnparse_free(p);
		first = NULL;
	}

	return (first);
}

/*
 * Call through to free parse contents.
 */
void
json_free(struct jsmnn *first)
{

	if (NULL != first)
		jsmnparse_free(first->p);
}

/*
 * Just check that the array object is in fact an object.
 */
static struct jsmnn *
json_getarrayobj(struct jsmnn *n)
{

	return (JSMN_OBJECT != n->type ? NULL : n);
}

/*
 * Extract an array from the returned JSON object, making sure that it's
 * the correct type.
 * Returns NULL on failure.
 */
static struct jsmnn *
json_getarray(struct jsmnn *n, const char *name)
{
	size_t		 i;

	if (JSMN_OBJECT != n->type)
		return (NULL);
	for (i = 0; i < n->fields; i++) {
		if (JSMN_STRING != n->d.obj[i].lhs->type &&
		    JSMN_PRIMITIVE != n->d.obj[i].lhs->type)
			continue;
		else if (strcmp(name, n->d.obj[i].lhs->d.str))
			continue;
		break;
	}
	if (i == n->fields)
		return (NULL);
	if (JSMN_ARRAY != n->d.obj[i].rhs->type)
		return (NULL);
	return (n->d.obj[i].rhs);
}

/*
 * Extract a single string from the returned JSON object, making sure
 * that it's the correct type.
 * Returns NULL on failure.
 */
static char *
json_getstr(struct jsmnn *n, const char *name)
{
	size_t		 i;
	char		*cp;

	if (JSMN_OBJECT != n->type)
		return (NULL);
	for (i = 0; i < n->fields; i++) {
		if (JSMN_STRING != n->d.obj[i].lhs->type &&
		    JSMN_PRIMITIVE != n->d.obj[i].lhs->type)
			continue;
		else if (strcmp(name, n->d.obj[i].lhs->d.str))
			continue;
		break;
	}
	if (i == n->fields)
		return (NULL);
	if (JSMN_STRING != n->d.obj[i].rhs->type &&
	    JSMN_PRIMITIVE != n->d.obj[i].rhs->type)
		return (NULL);

	cp = strdup(n->d.obj[i].rhs->d.str);
	if (NULL == cp)
		warn("strdup");
	return (cp);
}

/*
 * Completely free the challenge response body.
 */
void
json_free_challenge(struct chng *p)
{

	free(p->uri);
	free(p->token);
	p->uri = p->token = NULL;
}

/*
 * Parse the response from the ACME server when we're waiting to see
 * whether the challenge has been ok.
 */
int
json_parse_response(struct jsmnn *n)
{
	char		*resp;
	int		 rc;

	if (NULL == n)
		return (-1);
	if (NULL == (resp = json_getstr(n, "status")))
		return (-1);

	if (0 == strcmp(resp, "valid"))
		rc = 1;
	else if (0 == strcmp(resp, "pending"))
		rc = 0;
	else
		rc = -1;

	free(resp);
	return (rc);
}

/*
 * Parse the response for "challenge" from a new-authz, which consists
 * of challenge information, into a structure.
 * We can accept any type of challenge to check for, but we default to
 * http-01.
 * Returns zero on failure (parse error or couldn't find challenge) or
 * non-zero on success, in which case both challenge properties are
 * filled in.
 */
int
json_parse_challenge(struct jsmnn *n, 
	struct chng *p, const char *challenge)
{
	struct jsmnn	*array, *obj;
	size_t		 i;
	int		 rc;
	char		*type;

	if (NULL == challenge)
		challenge = "http-01";

	if (NULL == n)
		return (0);

	array = json_getarray(n, "challenges");
	if (NULL == array)
		return (0);

	for (i = 0; i < array->fields; i++) {
		obj = json_getarrayobj(array->d.array[i]);
		if (NULL == obj)
			continue;
		type = json_getstr(obj, "type");
		if (NULL == type)
			continue;
		rc = strcmp(type, challenge);
		free(type);
		if (rc)
			continue;

		p->uri = json_getstr(obj, "uri");
		p->token = json_getstr(obj, "token");
		return (NULL != p->uri &&
		       NULL != p->token);
	}

	return (0);
}

/*
 * Extract the CA paths from the JSON response object.
 * Return zero on failure, non-zero on success.
 */
int
json_parse_capaths(struct jsmnn *n, struct capaths *p)
{

	if (NULL == n)
		return (0);

	p->newauthz = json_getstr(n, "new-authz");
	p->newcert = json_getstr(n, "new-cert");
	p->newreg = json_getstr(n, "new-reg");
	p->revokecert = json_getstr(n, "revoke-cert");

	return (NULL != p->newauthz &&
	       NULL != p->newcert &&
	       NULL != p->newreg &&
	       NULL != p->revokecert);
}

/*
 * Free up all of our CA-noted paths (which may all be NULL).
 */
void
json_free_capaths(struct capaths *p)
{

	free(p->newauthz);
	free(p->newcert);
	free(p->newreg);
	free(p->revokecert);
	memset(p, 0, sizeof(struct capaths));
}

/*
 * Parse an HTTP response body from a buffer of size "sz".
 * Returns an opaque pointer on success, otherwise NULL on error.
 */
struct jsmnn *
json_parse(const char *buf, size_t sz)
{
	struct jsmnn	*n;
	jsmn_parser	 p;
	jsmntok_t	*tok;
	int		 r;
	size_t		 tokcount;

	jsmn_init(&p);
	tokcount = 128;

	/* Do this until we don't need any more tokens. */
again:
	tok = calloc(tokcount, sizeof(jsmntok_t));
	if (NULL == tok) {
		warn("calloc");
		return (NULL);
	}

	/* Actually try to parse the JSON into the tokens. */

	r = jsmn_parse(&p, buf, sz, tok, tokcount);
	if (r < 0 && JSMN_ERROR_NOMEM == r) {
		tokcount *= 2;
		free(tok);
		goto again;
	} else if (r < 0) {
		warnx("jsmn_parse: %d", r);
		free(tok);
		return (NULL);
	}

	/* Now parse the tokens into a tree. */

	n = jsmntree_alloc(tok, buf, r);
	free(tok);
	return (n);
}

/*
 * Format the "new-reg" resource request.
 */
char *
json_fmt_newreg(const char *license)
{
	char	*p;

	p = doasprintf("{"
		"\"resource\": \"new-reg\", "
		"\"agreement\": \"%s\""
		"}", license);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Format the "new-authz" resource request.
 */
char *
json_fmt_newauthz(const char *domain)
{
	char	*p;

	p = doasprintf("{"
		"\"resource\": \"new-authz\", "
		"\"identifier\": "
		"{\"type\": \"dns\", \"value\": \"%s\"}"
		"}", domain);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Format the "challenge" resource request.
 */
char *
json_fmt_challenge(const char *token, const char *thumb)
{
	char	*p;

	p = doasprintf("{"
		"\"resource\": \"challenge\", "
		"\"keyAuthorization\": \"%s.%s\""
		"}", token, thumb);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Format the "new-cert" resource request.
 */
char *
json_fmt_revokecert(const char *cert)
{
	char	*p;

	p = doasprintf("{"
		"\"resource\": \"revoke-cert\", "
		"\"certificate\": \"%s\""
		"}", cert);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Format the "new-cert" resource request.
 */
char *
json_fmt_newcert(const char *cert)
{
	char	*p;

	p = doasprintf("{"
		"\"resource\": \"new-cert\", "
		"\"csr\": \"%s\""
		"}", cert);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Header component of json_fmt_signed().
 */
char *
json_fmt_header_rsa(const char *exp, const char *mod)
{
	char	*p;

	p = doasprintf("{"
		"\"alg\": \"RS256\", "
		"\"jwk\": "
		"{\"e\": \"%s\", \"kty\": \"RSA\", \"n\": \"%s\"}"
		"}", exp, mod);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Protected component of json_fmt_signed().
 */
char *
json_fmt_protected_rsa(const char *exp, const char *mod, const char *nce)
{
	char	*p;

	p = doasprintf("{"
		"\"alg\": \"RS256\", "
		"\"jwk\": "
		"{\"e\": \"%s\", \"kty\": \"RSA\", \"n\": \"%s\"}, "
		"\"nonce\": \"%s\""
		"}", exp, mod, nce);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Signed message contents for the CA server.
 */
char *
json_fmt_signed(const char *header, const char *protected,
	const char *payload, const char *digest)
{
	char	*p;

	p = doasprintf("{"
		"\"header\": %s, "
		"\"protected\": \"%s\", "
		"\"payload\": \"%s\", "
		"\"signature\": \"%s\""
		"}", header, protected, payload, digest);
	if (NULL == p)
		warn("asprintf");
	return (p);
}

/*
 * Produce thumbprint input.
 * This isn't technically a JSON string--it's the input we'll use for
 * hashing and digesting.
 * However, it's in the form of a JSON string, so do it here.
 */
char *
json_fmt_thumb_rsa(const char *exp, const char *mod)
{
	char	*p;

	/*NOTE: WHITESPACE IS IMPORTANT. */

	p = doasprintf("{\"e\":\"%s\",\"kty\":\"RSA\",\"n\":\"%s\"}",
		exp, mod);
	if (NULL == p)
		warn("asprintf");
	return (p);
}
