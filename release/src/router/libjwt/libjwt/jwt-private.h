/* Copyright (C) 2015-2024 Ben Collins <bcollins@maclara-llc.com>
   This file is part of the JWT C Library

   SPDX-License-Identifier:  MPL-2.0
   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef JWT_PRIVATE_H
#define JWT_PRIVATE_H

#include <jansson.h>
#include <time.h>

struct jwt {
	jwt_alg_t alg;
	unsigned char *key;
	int key_len;
	json_t *grants;
	json_t *headers;
};

struct jwt_valid {
	jwt_alg_t alg;
	time_t now;
	time_t nbf_leeway;
	time_t exp_leeway;
	int hdr;
	json_t *req_grants;
	unsigned int status;
};

/* Memory allocators. */
void *jwt_malloc(size_t size);
void jwt_freemem(void *ptr);

/* Helper routines. */
void jwt_base64uri_encode(char *str);
void *jwt_b64_decode(const char *src, int *ret_len);

/* These routines are implemented by the crypto backend. */
int jwt_sign_sha_hmac(jwt_t *jwt, char **out, unsigned int *len,
		      const char *str, unsigned int str_len);

int jwt_verify_sha_hmac(jwt_t *jwt, const char *head, unsigned int head_len, const char *sig);

int jwt_sign_sha_pem(jwt_t *jwt, char **out, unsigned int *len,
		     const char *str, unsigned int str_len);

int jwt_verify_sha_pem(jwt_t *jwt, const char *head, unsigned int head_len, const char *sig_b64);

/* A time-safe strcmp function */
int jwt_strcmp(const char *str1, const char *str2);

#endif /* JWT_PRIVATE_H */
