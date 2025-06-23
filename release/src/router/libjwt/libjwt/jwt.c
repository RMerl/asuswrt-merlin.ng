/* Copyright (C) 2015-2024 Ben Collins <bcollins@maclara-llc.com>
   This file is part of the JWT C Library

   SPDX-License-Identifier:  MPL-2.0
   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <jwt.h>

#include "jwt-private.h"
#include "base64.h"
#include "config.h"

/* Number of elements in an array */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static jwt_malloc_t pfn_malloc = NULL;
static jwt_realloc_t pfn_realloc = NULL;
static jwt_free_t pfn_free = NULL;

void *jwt_malloc(size_t size)
{
	if (pfn_malloc)
		return pfn_malloc(size);

	return malloc(size);
}

static void *jwt_realloc(void *ptr, size_t size)
{
	if (pfn_realloc)
		return pfn_realloc(ptr, size);

	return realloc(ptr, size);
}

void jwt_freemem(void *ptr)
{
	if (pfn_free)
		pfn_free(ptr);
	else
		free(ptr);
}

static char *jwt_strdup(const char *str)
{
	size_t len;
	char *result;

	len = strlen(str);
	result = (char *)jwt_malloc(len + 1);
	if (!result)
		return NULL;

	memcpy(result, str, len);
	result[len] = '\0';
	return result;
}

/* A time-safe strcmp function */
int jwt_strcmp(const char *str1, const char *str2)
{
	/* Get the LONGEST length */
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	int len_max = len1 >= len2 ? len1 : len2;

	int i, ret = 0;

	/* Iterate the entire longest string no matter what. Only testing
	 * the shortest string would still allow attacks for
	 * "a" == "aKJSDHkjashaaHJASJ", adding a character each time one
	 * is found. */
	for (i = 0; i < len_max; i++) {
		char c1, c2;

		c1 = (i < len1) ? str1[i] : 0;
		c2 = (i < len2) ? str2[i] : 0;

		ret |= c1 ^ c2;
	}

	/* Don't forget to check length */
	ret |= len1 ^ len2;

	return ret;
}

static void *jwt_calloc(size_t nmemb, size_t size)
{
	size_t total_size;
	void *ptr;

	total_size = nmemb * size;
	if (!total_size)
		return NULL;

	ptr = jwt_malloc(total_size);
	if (ptr)
		memset(ptr, 0, total_size);

	return ptr;
}

const char *jwt_alg_str(jwt_alg_t alg)
{
	switch (alg) {
	case JWT_ALG_NONE:
		return "none";
	case JWT_ALG_HS256:
		return "HS256";
	case JWT_ALG_HS384:
		return "HS384";
	case JWT_ALG_HS512:
		return "HS512";
	case JWT_ALG_RS256:
		return "RS256";
	case JWT_ALG_RS384:
		return "RS384";
	case JWT_ALG_RS512:
		return "RS512";
	case JWT_ALG_ES256:
		return "ES256";
	case JWT_ALG_ES384:
		return "ES384";
	case JWT_ALG_ES512:
		return "ES512";
	case JWT_ALG_PS256:
		return "PS256";
	case JWT_ALG_PS384:
		return "PS384";
	case JWT_ALG_PS512:
		return "PS512";
	default:
		return NULL;
	}
}

jwt_alg_t jwt_str_alg(const char *alg)
{
	if (alg == NULL)
		return JWT_ALG_INVAL;

	if (!strcmp(alg, "none"))
		return JWT_ALG_NONE;
	else if (!strcmp(alg, "HS256"))
		return JWT_ALG_HS256;
	else if (!strcmp(alg, "HS384"))
		return JWT_ALG_HS384;
	else if (!strcmp(alg, "HS512"))
		return JWT_ALG_HS512;
	else if (!strcmp(alg, "RS256"))
		return JWT_ALG_RS256;
	else if (!strcmp(alg, "RS384"))
		return JWT_ALG_RS384;
	else if (!strcmp(alg, "RS512"))
		return JWT_ALG_RS512;
	else if (!strcmp(alg, "ES256"))
		return JWT_ALG_ES256;
	else if (!strcmp(alg, "ES384"))
		return JWT_ALG_ES384;
	else if (!strcmp(alg, "ES512"))
		return JWT_ALG_ES512;
	else if (!strcmp(alg, "PS256"))
		return JWT_ALG_PS256;
	else if (!strcmp(alg, "PS384"))
		return JWT_ALG_PS384;
	else if (!strcmp(alg, "PS512"))
		return JWT_ALG_PS512;

	return JWT_ALG_INVAL;
}

static void jwt_scrub_key(jwt_t *jwt)
{
	if (jwt->key) {
		/* Overwrite it so it's gone from memory. */
		memset(jwt->key, 0, jwt->key_len);

		jwt_freemem(jwt->key);
		jwt->key = NULL;
	}

	jwt->key_len = 0;
	jwt->alg = JWT_ALG_NONE;
}

int jwt_set_alg(jwt_t *jwt, jwt_alg_t alg, const unsigned char *key, int len)
{
	/* No matter what happens here, we do this. */
	jwt_scrub_key(jwt);

	if (alg < JWT_ALG_NONE || alg >= JWT_ALG_INVAL)
		return EINVAL;

	switch (alg) {
	case JWT_ALG_NONE:
		if (key || len)
			return EINVAL;
		break;

	default:
		if (!key || len <= 0)
			return EINVAL;

		jwt->key = jwt_malloc(len);
		if (!jwt->key)
			return ENOMEM;

		memcpy(jwt->key, key, len);
	}

	jwt->alg = alg;
	jwt->key_len = len;

	return 0;
}

jwt_alg_t jwt_get_alg(const jwt_t *jwt)
{
	return jwt->alg;
}

int jwt_new(jwt_t **jwt)
{
	if (!jwt)
		return EINVAL;

	*jwt = jwt_malloc(sizeof(jwt_t));
	if (!*jwt)
		return ENOMEM;

	memset(*jwt, 0, sizeof(jwt_t));

	(*jwt)->grants = json_object();
	if (!(*jwt)->grants) {
		jwt_freemem(*jwt);
		*jwt = NULL;
		return ENOMEM;
	}

	(*jwt)->headers = json_object();
	if (!(*jwt)->headers) {
		json_decref((*jwt)->grants);
		jwt_freemem(*jwt);
		*jwt = NULL;
		return ENOMEM;
	}

	return 0;
}

void jwt_free(jwt_t *jwt)
{
	if (!jwt)
		return;

	jwt_scrub_key(jwt);

	json_decref(jwt->grants);
	json_decref(jwt->headers);

	jwt_freemem(jwt);
}

jwt_t *jwt_dup(jwt_t *jwt)
{
	jwt_t *new = NULL;

	if (!jwt) {
		errno = EINVAL;
		goto dup_fail;
	}

	errno = 0;

	new = jwt_malloc(sizeof(jwt_t));
	if (!new) {
		errno = ENOMEM;
		return NULL;
	}

	memset(new, 0, sizeof(jwt_t));

	if (jwt->key_len) {
		new->alg = jwt->alg;
		new->key = jwt_malloc(jwt->key_len);
		if (!new->key) {
			errno = ENOMEM;
			goto dup_fail;
		}
		memcpy(new->key, jwt->key, jwt->key_len);
		new->key_len = jwt->key_len;
	}

	new->grants = json_deep_copy(jwt->grants);
	if (!new->grants)
		errno = ENOMEM;

	new->headers = json_deep_copy(jwt->headers);
	if (!new->headers)
		errno = ENOMEM;

dup_fail:
	if (errno) {
		jwt_free(new);
		new = NULL;
	}

	return new;
}

static const char *get_js_string(json_t *js, const char *key)
{
	const char *val = NULL;
	json_t *js_val;

	js_val = json_object_get(js, key);
	if (js_val) {
		if (json_typeof(js_val) == JSON_STRING)
			val = json_string_value(js_val);
		else
			errno = EINVAL;
	} else {
		errno = ENOENT;
	}

	return val;
}

static long get_js_int(json_t *js, const char *key)
{
	long val = -1;
	json_t *js_val;

	js_val = json_object_get(js, key);
	if (js_val) {
		if (json_typeof(js_val) == JSON_INTEGER)
			val = (long)json_integer_value(js_val);
		else
			errno = EINVAL;
	} else {
		errno = ENOENT;
	}

	return val;
}

static int get_js_bool(json_t *js, const char *key)
{
	int val = -1;
	json_t *js_val;

	js_val = json_object_get(js, key);
	if (js_val) {
		switch (json_typeof(js_val)) {
		case JSON_TRUE:
			val = 1;
			break;
		case JSON_FALSE:
			val = 0;
			break;
		default:
			errno = EINVAL;
		}
	} else {
		errno = ENOENT;
	}
	return val;
}

void *jwt_b64_decode(const char *src, int *ret_len)
{
	void *buf;
	char *new;
	int len, i, z;

	/* Decode based on RFC-4648 URI safe encoding. */
	len = (int)strlen(src);
	new = alloca(len + 4);
	if (!new)
		return NULL;

	for (i = 0; i < len; i++) {
		switch (src[i]) {
		case '-':
			new[i] = '+';
			break;
		case '_':
			new[i] = '/';
			break;
		default:
			new[i] = src[i];
		}
	}
	z = 4 - (i % 4);
	if (z < 4) {
		while (z--)
			new[i++] = '=';
	}
	new[i] = '\0';

	buf = jwt_malloc(i);
	if (buf == NULL)
		return NULL;

	*ret_len = jwt_Base64decode(buf, new);

	if (*ret_len == 0) {
		jwt_freemem(buf);
		buf = NULL;
	}

	return buf;
}


static json_t *jwt_b64_decode_json(char *src)
{
	json_t *js;
	char *buf;
	int len;

	buf = jwt_b64_decode(src, &len);

	if (buf == NULL)
		return NULL;

	buf[len] = '\0';

	js = json_loads(buf, 0, NULL);

	jwt_freemem(buf);

	return js;
}

void jwt_base64uri_encode(char *str)
{
	int len = (int)strlen(str);
	int i, t;

	for (i = t = 0; i < len; i++) {
		switch (str[i]) {
		case '+':
			str[t++] = '-';
			break;
		case '/':
			str[t++] = '_';
			break;
		case '=':
			break;
		default:
			str[t++] = str[i];
		}
	}

	str[t] = '\0';
}

static int jwt_sign(jwt_t *jwt, char **out, unsigned int *len, const char *str, unsigned int str_len)
{
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_sign_sha_hmac(jwt, out, len, str, str_len);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

#ifndef HAVE_OPENSSL
	/* XXX These do not work right now on OpenSSL */
	/* RSA-PSS */
	case JWT_ALG_PS256:
	case JWT_ALG_PS384:
	case JWT_ALG_PS512:
#endif

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_sign_sha_pem(jwt, out, len, str, str_len);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}

static int jwt_verify(jwt_t *jwt, const char *head, unsigned int head_len, const char *sig)
{
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_verify_sha_hmac(jwt, head, head_len, sig);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

#ifndef HAVE_OPENSSL
	/* XXX These do not work right now on OpenSSL */
	/* RSA-PSS */
	case JWT_ALG_PS256:
	case JWT_ALG_PS384:
	case JWT_ALG_PS512:
#endif

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_verify_sha_pem(jwt, head, head_len, sig);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}

static int jwt_parse_body(jwt_t *jwt, char *body)
{
	if (jwt->grants) {
		json_decref(jwt->grants);
		jwt->grants = NULL;
	}

	jwt->grants = jwt_b64_decode_json(body);
	if (!jwt->grants)
		return EINVAL;

	return 0;
}

static int jwt_parse_head(jwt_t *jwt, char *head)
{
	const char *alg;

	if (jwt->headers) {
		json_decref(jwt->headers);
		jwt->headers = NULL;
	}

	jwt->headers = jwt_b64_decode_json(head);
	if (!jwt->headers)
		return EINVAL;

	alg = get_js_string(jwt->headers, "alg");
	jwt->alg = jwt_str_alg(alg);
	if (jwt->alg == JWT_ALG_INVAL)
		return EINVAL;

	return 0;
}

static int jwt_verify_head(jwt_t *jwt)
{
	int ret = 0;

	if (jwt->alg != JWT_ALG_NONE) {
		if (jwt->key) {
			if (jwt->key_len <= 0)
				ret = EINVAL;
		} else {
			jwt_scrub_key(jwt);
		}
	} else {
		/* If alg is NONE, there should not be a key */
		if (jwt->key)
			ret = EINVAL;
	}

	return ret;
}

static int jwt_parse(jwt_t **jwt, const char *token, unsigned int *len)
{
	char *head = NULL;
	jwt_t *new = NULL;
	char *body, *sig;
	int ret = EINVAL;

	if (!jwt)
		return EINVAL;

	*jwt = NULL;
	head = jwt_strdup(token);

	if (!head)
		return ENOMEM;

	/* Find the components. */
	for (body = head; body[0] != '.'; body++) {
		if (body[0] == '\0')
			goto parse_done;
	}

	body[0] = '\0';
	body++;

	for (sig = body; sig[0] != '.'; sig++) {
		if (sig[0] == '\0')
			goto parse_done;
	}

	sig[0] = '\0';

	/* Now that we have everything split up, let's check out the
	 * header. */
	ret = jwt_new(&new);
	if (ret)
		goto parse_done;

	if ((ret = jwt_parse_head(new, head)))
		goto parse_done;

	ret = jwt_parse_body(new, body);
parse_done:
	if (ret) {
		jwt_free(new);
		*jwt = NULL;
	} else {
		*jwt = new;
		*len = sig - head;
	}

	jwt_freemem(head);

	return ret;
}

static int jwt_copy_key(jwt_t *jwt, const unsigned char *key, int key_len)
{
	int ret = 0;

	if (key_len) {
		jwt->key = jwt_malloc(key_len);
		if (jwt->key == NULL)
			return ENOMEM;
		memcpy(jwt->key, key, key_len);
		jwt->key_len = key_len;
	}

	return ret;
}

int jwt_decode(jwt_t **jwt, const char *token, const unsigned char *key,
	       int key_len)
{
	jwt_t *new = NULL;
	int ret = EINVAL;
	unsigned int payload_len;

	ret = jwt_parse(jwt, token, &payload_len);
	if (ret) {
		return ret;
	}
	new = *jwt;

	/* Copy the key over for verify_head. */
	ret = jwt_copy_key(new, key, key_len);
	if (ret)
		goto decode_done;

	ret = jwt_verify_head(new);
	if (ret)
		goto decode_done;

	/* Check the signature, if needed. */
	if (new->alg != JWT_ALG_NONE) {
		const char *sig = token + (payload_len + 1);
		ret = jwt_verify(new, token, payload_len, sig);
	} else {
		ret = 0;
	}

decode_done:
	if (ret) {
		jwt_free(new);
		*jwt = NULL;
	}

	return ret;
}

int jwt_decode_2(jwt_t **jwt, const char *token, jwt_key_p_t key_provider)
{
	jwt_t *new = NULL;
	int ret = EINVAL;
	unsigned int payload_len;
	jwt_key_t key;

	ret = jwt_parse(jwt, token, &payload_len);
	if (ret)
		return ret;
	new = *jwt;

	/* Obtain the key. */
	if (new->alg != JWT_ALG_NONE) {
		ret = key_provider(new, &key);
		if (ret)
			goto decode_done;
		ret = jwt_copy_key(new, key.jwt_key, key.jwt_key_len);
		if (ret)
			goto decode_done;
	}

	ret = jwt_verify_head(new);
	if (ret)
		goto decode_done;

	/* Check the signature, if needed. */
	if (new->alg != JWT_ALG_NONE) {
		ret = jwt_verify(new, token, payload_len,
				 token + (payload_len + 1));
	} else {
		ret = 0;
	}

decode_done:
	if (ret) {
		jwt_free(new);
		*jwt = NULL;
	}

	return ret;
}

const char *jwt_get_grant(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;

	return get_js_string(jwt->grants, grant);
}

long jwt_get_grant_int(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_int(jwt->grants, grant);
}

int jwt_get_grant_bool(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_bool(jwt->grants, grant);
}

char *jwt_get_grants_json(jwt_t *jwt, const char *grant)
{
	json_t *js_val = NULL;

	if (!jwt) {
		errno = EINVAL;
		return NULL;
	}

	if (grant && strlen(grant))
		js_val = json_object_get(jwt->grants, grant);
	else
		js_val = jwt->grants;

	if (js_val == NULL) {
		errno = ENOENT;
		return NULL;
	}

	errno = 0;

	return json_dumps(js_val, JSON_SORT_KEYS | JSON_COMPACT | JSON_ENCODE_ANY);
}

int jwt_add_grant(jwt_t *jwt, const char *grant, const char *val)
{
	if (!jwt || !grant || !strlen(grant) || !val)
		return EINVAL;

	if (get_js_string(jwt->grants, grant) != NULL)
		return EEXIST;

	if (json_object_set_new(jwt->grants, grant, json_string(val)))
		return EINVAL;

	return 0;
}

int jwt_add_grant_int(jwt_t *jwt, const char *grant, long val)
{
	if (!jwt || !grant || !strlen(grant))
		return EINVAL;

	if (get_js_int(jwt->grants, grant) != -1)
		return EEXIST;

	if (json_object_set_new(jwt->grants, grant, json_integer((json_int_t)val)))
		return EINVAL;

	return 0;
}

int jwt_add_grant_bool(jwt_t *jwt, const char *grant, int val)
{
	if (!jwt || !grant || !strlen(grant))
		return EINVAL;

	if (get_js_int(jwt->grants, grant) != -1)
		return EEXIST;

	if (json_object_set_new(jwt->grants, grant, json_boolean(val)))
		return EINVAL;

	return 0;
}

int jwt_add_grants_json(jwt_t *jwt, const char *json)
{
	json_t *js_val;
	int ret = -1;

	if (!jwt)
		return EINVAL;

	js_val = json_loads(json, JSON_REJECT_DUPLICATES, NULL);

	if (json_is_object(js_val))
		ret = json_object_update(jwt->grants, js_val);

	json_decref(js_val);

	return ret ? EINVAL : 0;
}

int jwt_del_grants(jwt_t *jwt, const char *grant)
{
	if (!jwt)
		return EINVAL;

	if (grant == NULL || !strlen(grant))
		json_object_clear(jwt->grants);
	else
		json_object_del(jwt->grants, grant);

	return 0;
}

const char *jwt_get_header(jwt_t *jwt, const char *header)
{
	if (!jwt || !header || !strlen(header)) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;

	return get_js_string(jwt->headers, header);
}

long jwt_get_header_int(jwt_t *jwt, const char *header)
{
	if (!jwt || !header || !strlen(header)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_int(jwt->headers, header);
}

int jwt_get_header_bool(jwt_t *jwt, const char *header)
{
	if (!jwt || !header || !strlen(header)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_bool(jwt->headers, header);
}

char *jwt_get_headers_json(jwt_t *jwt, const char *header)
{
	json_t *js_val = NULL;

	errno = EINVAL;

	if (!jwt)
		return NULL;

	if (header && strlen(header))
		js_val = json_object_get(jwt->headers, header);
	else
		js_val = jwt->headers;

	if (js_val == NULL)
		return NULL;

	errno = 0;

	return json_dumps(js_val, JSON_SORT_KEYS | JSON_COMPACT | JSON_ENCODE_ANY);
}

int jwt_add_header(jwt_t *jwt, const char *header, const char *val)
{
	if (!jwt || !header || !strlen(header) || !val)
		return EINVAL;

	if (get_js_string(jwt->headers, header) != NULL)
		return EEXIST;

	if (json_object_set_new(jwt->headers, header, json_string(val)))
		return EINVAL;

	return 0;
}

int jwt_add_header_int(jwt_t *jwt, const char *header, long val)
{
	if (!jwt || !header || !strlen(header))
		return EINVAL;

	if (get_js_int(jwt->headers, header) != -1)
		return EEXIST;

	if (json_object_set_new(jwt->headers, header, json_integer((json_int_t)val)))
		return EINVAL;

	return 0;
}

int jwt_add_header_bool(jwt_t *jwt, const char *header, int val)
{
	if (!jwt || !header || !strlen(header))
		return EINVAL;

	if (get_js_int(jwt->headers, header) != -1)
		return EEXIST;

	if (json_object_set_new(jwt->headers, header, json_boolean(val)))
		return EINVAL;

	return 0;
}

int jwt_add_headers_json(jwt_t *jwt, const char *json)
{
	json_t *js_val;
	int ret = -1;

	if (!jwt)
		return EINVAL;

	js_val = json_loads(json, JSON_REJECT_DUPLICATES, NULL);

	if (json_is_object(js_val))
		ret = json_object_update(jwt->headers, js_val);

	json_decref(js_val);

	return ret ? EINVAL : 0;
}

int jwt_del_headers(jwt_t *jwt, const char *header)
{
	if (!jwt)
		return EINVAL;

	if (header == NULL || !strlen(header))
		json_object_clear(jwt->headers);
	else
		json_object_del(jwt->headers, header);

	return 0;
}

static int __append_str(char **buf, const char *str)
{
	char *new;

	if (*buf == NULL)
		new = jwt_calloc(1, strlen(str) + 1);
	else
		new = jwt_realloc(*buf, strlen(*buf) + strlen(str) + 1);

	if (new == NULL)
		return ENOMEM;

	strcat(new, str);

	*buf = new;

	return 0;
}

#define APPEND_STR(__buf, __str) do {		\
	int ret = __append_str(__buf, __str);	\
	if (ret)				\
		return ret;			\
} while (0)

static int write_js(const json_t *js, char **buf, int pretty)
{
	/* Sort keys for repeatability */
	size_t flags = JSON_SORT_KEYS;
	char *serial;

	if (pretty) {
		APPEND_STR(buf, "\n");
		flags |= JSON_INDENT(4);
	} else {
		flags |= JSON_COMPACT;
	}

	serial = json_dumps(js, flags);

	APPEND_STR(buf, serial);

	jwt_freemem(serial);

	if (pretty)
		APPEND_STR(buf, "\n");

	return 0;
}

static int jwt_write_head(jwt_t *jwt, char **buf, int pretty)
{
	int ret = 0;

	if (jwt->alg != JWT_ALG_NONE) {

		/* Only add default 'typ' header if it has not been defined,
		 * allowing for any value of it. This allows for signaling
		 * of application specific extensions to JWT, such as PASSporT,
		 * RFC 8225. */
		if ((ret = jwt_add_header(jwt, "typ", "JWT"))) {
			if (ret != EEXIST)
				return ret;
		}
	}

	if ((ret = jwt_del_headers(jwt, "alg")))
		return ret;

	if ((ret = jwt_add_header(jwt, "alg", jwt_alg_str(jwt->alg))))
		return ret;

	return write_js(jwt->headers, buf, pretty);
}

static int jwt_write_body(jwt_t *jwt, char **buf, int pretty)
{
	return write_js(jwt->grants, buf, pretty);
}

static int jwt_dump(jwt_t *jwt, char **buf, int pretty)
{
	int ret;

	ret = jwt_write_head(jwt, buf, pretty);

	if (ret == 0)
		ret = __append_str(buf, ".");

	if (ret == 0)
		ret = jwt_write_body(jwt, buf, pretty);

	return ret;
}

char *jwt_dump_grants_str(jwt_t *jwt, int pretty)
{
	char *out = NULL;
	int err;

	errno = 0;

	err = jwt_write_body(jwt, &out, pretty);

	if (err) {
		errno = err;
		if (out)
			jwt_freemem(out);
		out = NULL;
	}

	return out;
}

int jwt_dump_fp(jwt_t *jwt, FILE *fp, int pretty)
{
	char *out = NULL;
	int ret = 0;

	ret = jwt_dump(jwt, &out, pretty);

	if (ret == 0)
		fputs(out, fp);

	if (out)
		jwt_freemem(out);

	return ret;
}

char *jwt_dump_str(jwt_t *jwt, int pretty)
{
	char *out = NULL;
	int err;

	err = jwt_dump(jwt, &out, pretty);

	if (err) {
		errno = err;
		if (out)
			jwt_freemem(out);
		out = NULL;
	} else {
		errno = 0;
	}

	return out;
}

static int jwt_encode(jwt_t *jwt, char **out)
{
	char *buf = NULL, *head, *body, *sig;
	int ret, head_len, body_len;
	unsigned int sig_len;

	/* First the header. */
	ret = jwt_write_head(jwt, &buf, 0);
	if (ret) {
		if (buf)
			jwt_freemem(buf);
		return ret;
	}

	head = alloca(strlen(buf) * 2);
	if (head == NULL) {
		jwt_freemem(buf);
		return ENOMEM;
	}
	jwt_Base64encode(head, buf, (int)strlen(buf));
	head_len = (int)strlen(head);

	jwt_freemem(buf);
	buf = NULL;

	/* Now the body. */
	ret = jwt_write_body(jwt, &buf, 0);
	if (ret) {
		if (buf)
			jwt_freemem(buf);
		return ret;
	}

	body = alloca(strlen(buf) * 2);
	if (body == NULL) {
		jwt_freemem(buf);
		return ENOMEM;
	}
	jwt_Base64encode(body, buf, (int)strlen(buf));
	body_len = (int)strlen(body);

	jwt_freemem(buf);
	buf = NULL;

	jwt_base64uri_encode(head);
	jwt_base64uri_encode(body);

	/* Allocate enough to reuse as b64 buffer. */
	buf = jwt_malloc(head_len + body_len + 2);
	if (buf == NULL)
		return ENOMEM;
	strcpy(buf, head);
	strcat(buf, ".");
	strcat(buf, body);

	ret = __append_str(out, buf);
	if (ret == 0)
		ret = __append_str(out, ".");
	if (ret) {
		if (buf)
			jwt_freemem(buf);
		return ret;
	}

	if (jwt->alg == JWT_ALG_NONE) {
		jwt_freemem(buf);
		return 0;
	}

	/* Now the signature. */
	ret = jwt_sign(jwt, &sig, &sig_len, buf, strlen(buf));
	jwt_freemem(buf);

	if (ret)
		return ret;

	buf = jwt_malloc(sig_len * 2);
	if (buf == NULL) {
		jwt_freemem(sig);
		return ENOMEM;
	}

	jwt_Base64encode(buf, sig, sig_len);

	jwt_freemem(sig);

	jwt_base64uri_encode(buf);
	ret = __append_str(out, buf);
	jwt_freemem(buf);

	return ret;
}

int jwt_encode_fp(jwt_t *jwt, FILE *fp)
{
	char *str = NULL;
	int ret;

	ret = jwt_encode(jwt, &str);
	if (ret) {
		if (str)
			jwt_freemem(str);
		return ret;
	}

	fputs(str, fp);
	jwt_freemem(str);

	return 0;
}

char *jwt_encode_str(jwt_t *jwt)
{
	char *str = NULL;

	errno = jwt_encode(jwt, &str);
	if (errno) {
		if (str)
			jwt_freemem(str);
		str = NULL;
	}

	return str;
}

void jwt_free_str(char *str)
{
	if (str)
		jwt_freemem(str);
}

int jwt_set_alloc(jwt_malloc_t pmalloc, jwt_realloc_t prealloc, jwt_free_t pfree)
{
	/* Set allocator functions for LibJWT. */
	pfn_malloc = pmalloc;
	pfn_realloc = prealloc;
	pfn_free = pfree;

	/* Set same allocator functions for Jansson. */
	json_set_alloc_funcs(jwt_malloc, jwt_freemem);

	return 0;
}

void jwt_get_alloc(jwt_malloc_t *pmalloc, jwt_realloc_t *prealloc, jwt_free_t *pfree)
{
	if (pmalloc)
		*pmalloc = pfn_malloc;

	if (prealloc)
		*prealloc = pfn_realloc;

	if (pfree)
		*pfree = pfn_free;
}

int jwt_valid_new(jwt_valid_t **jwt_valid, jwt_alg_t alg)
{
	if (!jwt_valid)
		return EINVAL;

	*jwt_valid = jwt_malloc(sizeof(jwt_valid_t));
	if (!*jwt_valid)
		return ENOMEM;

	memset(*jwt_valid, 0, sizeof(jwt_valid_t));
	(*jwt_valid)->alg = alg;

	(*jwt_valid)->status = JWT_VALIDATION_ERROR;

	(*jwt_valid)->nbf_leeway = 0;
	(*jwt_valid)->exp_leeway = 0;

	(*jwt_valid)->req_grants = json_object();
	if (!(*jwt_valid)->req_grants) {
		jwt_freemem(*jwt_valid);
		*jwt_valid = NULL;
		return ENOMEM;
	}

	return 0;
}

void jwt_valid_free(jwt_valid_t *jwt_valid)
{
	if (!jwt_valid)
		return;

	json_decref(jwt_valid->req_grants);

	jwt_freemem(jwt_valid);
}

unsigned int jwt_valid_get_status(jwt_valid_t *jwt_valid)
{
	if (!jwt_valid)
		return JWT_VALIDATION_ERROR;

	return jwt_valid->status;
}

time_t jwt_valid_get_nbf_leeway(jwt_valid_t *jwt_valid)
{
	if (!jwt_valid)
		return EINVAL;

	return jwt_valid->nbf_leeway;
}

time_t jwt_valid_get_exp_leeway(jwt_valid_t *jwt_valid)
{
	if (!jwt_valid)
		return EINVAL;

	return jwt_valid->exp_leeway;
}

int jwt_valid_add_grant(jwt_valid_t *jwt_valid, const char *grant, const char *val)
{
	if (!jwt_valid || !grant || !strlen(grant) || !val)
		return EINVAL;

	if (get_js_string(jwt_valid->req_grants, grant) != NULL)
		return EEXIST;

	if (json_object_set_new(jwt_valid->req_grants, grant, json_string(val)))
		return EINVAL;

	return 0;
}

int jwt_valid_add_grant_int(jwt_valid_t *jwt_valid, const char *grant, long val)
{
	if (!jwt_valid || !grant || !strlen(grant))
		return EINVAL;

	if (get_js_int(jwt_valid->req_grants, grant) != -1)
		return EEXIST;

	if (json_object_set_new(jwt_valid->req_grants, grant, json_integer((json_int_t)val)))
		return EINVAL;

	return 0;
}

int jwt_valid_add_grant_bool(jwt_valid_t *jwt_valid, const char *grant, int val)
{
	if (!jwt_valid || !grant || !strlen(grant))
		return EINVAL;

	if (get_js_bool(jwt_valid->req_grants, grant) != -1)
		return EEXIST;

	if (json_object_set_new(jwt_valid->req_grants, grant, json_boolean(val)))
		return EINVAL;

	return 0;
}

int jwt_valid_add_grants_json(jwt_valid_t *jwt_valid, const char *json)
{
	json_t *js_val;
	int ret = -1;

	if (!jwt_valid)
		return EINVAL;

	js_val = json_loads(json, JSON_REJECT_DUPLICATES, NULL);

	if (json_is_object(js_val))
		ret = json_object_update(jwt_valid->req_grants, js_val);

	json_decref(js_val);

	return ret ? EINVAL : 0;
}

char *jwt_valid_get_grants_json(jwt_valid_t *jwt_valid, const char *grant)
{
	json_t *js_val = NULL;

	errno = EINVAL;

	if (!jwt_valid)
		return NULL;

	if (grant && strlen(grant))
		js_val = json_object_get(jwt_valid->req_grants, grant);
	else
		js_val = jwt_valid->req_grants;

	if (js_val == NULL)
		return NULL;

	errno = 0;

	return json_dumps(js_val, JSON_SORT_KEYS | JSON_COMPACT | JSON_ENCODE_ANY);
}

const char *jwt_valid_get_grant(jwt_valid_t *jwt_valid, const char *grant)
{
	if (!jwt_valid || !grant || !strlen(grant)) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;

	return get_js_string(jwt_valid->req_grants, grant);
}

long jwt_valid_get_grant_int(jwt_valid_t *jwt_valid, const char *grant)
{
	if (!jwt_valid || !grant || !strlen(grant)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_int(jwt_valid->req_grants, grant);
}

int jwt_valid_get_grant_bool(jwt_valid_t *jwt_valid, const char *grant)
{
	if (!jwt_valid || !grant || !strlen(grant)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_bool(jwt_valid->req_grants, grant);
}

int jwt_valid_set_now(jwt_valid_t *jwt_valid, const time_t now)
{
	if (!jwt_valid)
		return EINVAL;

	jwt_valid->now = now;

	return 0;
}

int jwt_valid_set_nbf_leeway(jwt_valid_t *jwt_valid, const time_t nbf_leeway)
{
	if (!jwt_valid)
		return EINVAL;

	jwt_valid->nbf_leeway = nbf_leeway;

	return 0;
}

int jwt_valid_set_exp_leeway(jwt_valid_t *jwt_valid, const time_t exp_leeway)
{
	if (!jwt_valid)
		return EINVAL;

	jwt_valid->exp_leeway = exp_leeway;

	return 0;
}

int jwt_valid_set_headers(jwt_valid_t *jwt_valid, int hdr)
{
	if (!jwt_valid)
		return EINVAL;

	jwt_valid->hdr = hdr;

	return 0;
}

int jwt_valid_del_grants(jwt_valid_t *jwt_valid, const char *grant)
{
	if (!jwt_valid)
		return EINVAL;

	if (grant == NULL || !strlen(grant))
		json_object_clear(jwt_valid->req_grants);
	else
		json_object_del(jwt_valid->req_grants, grant);

	return 0;
}

#define _SET_AND_RET(__v, __e) do {	\
	__v->status |= __e;		\
	return __v->status;		\
} while (0)

unsigned int jwt_validate(jwt_t *jwt, jwt_valid_t *jwt_valid)
{
	const char *jwt_hdr_str, *jwt_body_str, *req_grant;
	json_t *js_val_1, *js_val_2;
	time_t t;

	if (!jwt_valid)
		return JWT_VALIDATION_ERROR;

	if (!jwt) {
		jwt_valid->status = JWT_VALIDATION_ERROR;
		return jwt_valid->status;
	}

	jwt_valid->status = JWT_VALIDATION_SUCCESS;

	/* Validate algorithm */
	if (jwt_valid->alg != jwt_get_alg(jwt))
		jwt_valid->status |= JWT_VALIDATION_ALG_MISMATCH;

	/* Validate expires */
	t = get_js_int(jwt->grants, "exp");
	if (jwt_valid->now && t != -1 && jwt_valid->now - jwt_valid->exp_leeway >= t)
		jwt_valid->status |= JWT_VALIDATION_EXPIRED;

	/* Validate not-before */
	t = get_js_int(jwt->grants, "nbf");
	if (jwt_valid->now && t != -1 && jwt_valid->now + jwt_valid->nbf_leeway < t)
		jwt_valid->status |= JWT_VALIDATION_TOO_NEW;

	/* Validate replicated issuer */
	jwt_hdr_str = get_js_string(jwt->headers, "iss");
	jwt_body_str = get_js_string(jwt->grants, "iss");
	if (jwt_hdr_str && jwt_body_str && strcmp(jwt_hdr_str, jwt_body_str))
		jwt_valid->status |= JWT_VALIDATION_ISS_MISMATCH;

	/* Validate replicated subject */
	jwt_hdr_str = get_js_string(jwt->headers, "sub");
	jwt_body_str = get_js_string(jwt->grants, "sub");
	if (jwt_hdr_str && jwt_body_str && strcmp(jwt_hdr_str, jwt_body_str))
		jwt_valid->status |= JWT_VALIDATION_SUB_MISMATCH;

	/* Validate replicated audience (might be array or string) */
	js_val_1 = json_object_get(jwt->headers, "aud");
	js_val_2 = json_object_get(jwt->grants, "aud");
	if (js_val_1 && js_val_2 && !json_equal(js_val_1, js_val_2))
		jwt_valid->status |= JWT_VALIDATION_AUD_MISMATCH;

	/* Validate required grants */
	json_object_foreach(jwt_valid->req_grants, req_grant, js_val_1) {
		json_t *act_js_val = json_object_get(jwt->grants, req_grant);

		if (act_js_val && json_equal(js_val_1, act_js_val))
			continue;

		if (act_js_val)
			jwt_valid->status |= JWT_VALIDATION_GRANT_MISMATCH;
		else
			jwt_valid->status |= JWT_VALIDATION_GRANT_MISSING;
	}

	return jwt_valid->status;
}

typedef struct {
	int error;
	char *str;
} jwt_exception_dict_t;

static jwt_exception_dict_t jwt_exceptions[] = {
	/* { JWT_VALIDATION_SUCCESS, "SUCCESS" }, */
	{ JWT_VALIDATION_ERROR, "general failures" },
	{ JWT_VALIDATION_ALG_MISMATCH, "algorithm mismatch" },
	{ JWT_VALIDATION_EXPIRED, "token expired" },
	{ JWT_VALIDATION_TOO_NEW, "token future dated" },
	{ JWT_VALIDATION_ISS_MISMATCH, "issuer mismatch" },
	{ JWT_VALIDATION_SUB_MISMATCH, "subject mismatch" },
	{ JWT_VALIDATION_AUD_MISMATCH, "audience mismatch" },
	{ JWT_VALIDATION_GRANT_MISSING, "grant missing" },
	{ JWT_VALIDATION_GRANT_MISMATCH, "grant mismatch" },
};

char *jwt_exception_str(unsigned int exceptions)
{
	int rc, i;
	char *str = NULL;

	if (exceptions == JWT_VALIDATION_SUCCESS) {
		if ((rc = __append_str(&str, "success")))
			goto fail;
		return str;
	}

	for (i = 0; i < ARRAY_SIZE(jwt_exceptions); i++) {
		if (!(jwt_exceptions[i].error & exceptions))
			continue;

		if (str && (rc = __append_str(&str, ", ")))
			goto fail;

		if ((rc = __append_str(&str, jwt_exceptions[i].str)))
			goto fail;
	}

	/* check if none of the exceptions matched? */
	if (!str && (rc = __append_str(&str, "unknown exceptions")))
		goto fail;

	return str;
fail:
	errno = rc;
	jwt_freemem(str);
	return NULL;
}
