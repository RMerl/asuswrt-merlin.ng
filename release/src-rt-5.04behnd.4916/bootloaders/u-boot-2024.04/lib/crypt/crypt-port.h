/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (C) 2020 Steffen Jaeckel <jaeckel-floss@eyet-services.de> */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>
#include <vsprintf.h>

#define NO_GENSALT
#define CRYPT_OUTPUT_SIZE 384
#define ALG_SPECIFIC_SIZE 8192

#define ARG_UNUSED(x) (x)

#define static_assert(a, b) _Static_assert(a, b)

#define strtoul(cp, endp, base) simple_strtoul(cp, endp, base)

extern const unsigned char ascii64[65];

#define b64t ((const char *)ascii64)

int crypt_sha256crypt_rn_wrapped(const char *phrase, size_t phr_size,
				 const char *setting,
				 size_t ARG_UNUSED(set_size), uint8_t *output,
				 size_t out_size, void *scratch,
				 size_t scr_size);
int crypt_sha512crypt_rn_wrapped(const char *phrase, size_t phr_size,
				 const char *setting,
				 size_t ARG_UNUSED(set_size), uint8_t *output,
				 size_t out_size, void *scratch,
				 size_t scr_size);
