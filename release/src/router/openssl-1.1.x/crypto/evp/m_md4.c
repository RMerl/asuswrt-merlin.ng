/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"

#ifndef OPENSSL_NO_MD4

# include <openssl/evp.h>
# include <openssl/objects.h>
# include <openssl/x509.h>
# include <openssl/md4.h>
# include <openssl/rsa.h>
# include "crypto/evp.h"

static int init(EVP_MD_CTX *ctx)
{
    return MD4_Init(EVP_MD_CTX_md_data(ctx));
}

static int update(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    return MD4_Update(EVP_MD_CTX_md_data(ctx), data, count);
}

static int final(EVP_MD_CTX *ctx, unsigned char *md)
{
    return MD4_Final(md, EVP_MD_CTX_md_data(ctx));
}

static const EVP_MD md4_md = {
    NID_md4,
    NID_md4WithRSAEncryption,
    MD4_DIGEST_LENGTH,
    0,
    init,
    update,
    final,
    NULL,
    NULL,
    MD4_CBLOCK,
    sizeof(EVP_MD *) + sizeof(MD4_CTX),
};

const EVP_MD *EVP_md4(void)
{
    return &md4_md;
}
#endif
