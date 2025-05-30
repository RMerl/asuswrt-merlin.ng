/* hmac_sha256.h
 * Code copied from openssl distribution and
 * Modified just enough so that compiles and runs standalone
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hmac_sha256.h 821104 2023-02-02 07:27:10Z $
 */
/* ====================================================================
 * Copyright (c) 2004 The OpenSSL Project.  All rights reserved
 * according to the OpenSSL license [found in ../../LICENSE].
 * ====================================================================
 */
void hmac_sha256(const void *key, int key_len,
                 const unsigned char *text, size_t text_len,
                 unsigned char *digest,
                 unsigned int *digest_len);
void hmac_sha256_n(const void *key, int key_len,
                   const unsigned char *text, size_t text_len,
                   unsigned char *digest,
                   unsigned int digest_len);
void sha256(const unsigned char *text, size_t text_len, unsigned char *digest,
            unsigned int digest_len);
int
KDF(unsigned char *key, int key_len, unsigned char *prefix,
              int prefix_len, unsigned char *data, int data_len,
              unsigned char *output, int len);
