/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifndef SOUP_AUTH_DIGEST_H
#define SOUP_AUTH_DIGEST_H 1

#include "soup-auth.h"

#define SOUP_AUTH_DIGEST(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), SOUP_TYPE_AUTH_DIGEST, SoupAuthDigest))
#define SOUP_AUTH_DIGEST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_AUTH_DIGEST, SoupAuthDigestClass))
#define SOUP_IS_AUTH_DIGEST(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), SOUP_TYPE_AUTH_DIGEST))
#define SOUP_IS_AUTH_DIGEST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUP_TYPE_AUTH_DIGEST))
#define SOUP_AUTH_DIGEST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_AUTH_DIGEST, SoupAuthDigestClass))

typedef struct {
	SoupAuth parent;

} SoupAuthDigest;

typedef struct {
	SoupAuthClass parent_class;

} SoupAuthDigestClass;

/* Utility routines (also used by SoupAuthDomainDigest) */

typedef enum {
	SOUP_AUTH_DIGEST_ALGORITHM_NONE,
	SOUP_AUTH_DIGEST_ALGORITHM_MD5,
	SOUP_AUTH_DIGEST_ALGORITHM_MD5_SESS
} SoupAuthDigestAlgorithm;

typedef enum {
	SOUP_AUTH_DIGEST_QOP_AUTH     = 1 << 0,
	SOUP_AUTH_DIGEST_QOP_AUTH_INT = 1 << 1
} SoupAuthDigestQop;

SoupAuthDigestAlgorithm  soup_auth_digest_parse_algorithm (const char *algorithm);
char                    *soup_auth_digest_get_algorithm   (SoupAuthDigestAlgorithm algorithm);

SoupAuthDigestQop        soup_auth_digest_parse_qop       (const char *qop);
char                    *soup_auth_digest_get_qop         (SoupAuthDigestQop qop);

void soup_auth_digest_compute_hex_urp  (const char              *username,
					const char              *realm,
					const char              *password,
					char                     hex_urp[33]);
void soup_auth_digest_compute_hex_a1   (const char              *hex_urp,
					SoupAuthDigestAlgorithm  algorithm,
					const char              *nonce,
					const char              *cnonce,
					char                     hex_a1[33]);
void soup_auth_digest_compute_response (const char              *method,
					const char              *uri,
					const char              *hex_a1,
					SoupAuthDigestQop        qop,
					const char              *nonce,
					const char              *cnonce,
					int                      nc,
					char                     response[33]);

#endif /*SOUP_AUTH_DIGEST_H*/
