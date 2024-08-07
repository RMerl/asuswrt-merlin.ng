/*
 * Copyright 1995-2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "internal/constant_time.h"

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>

int RSA_padding_add_PKCS1_type_1(unsigned char *to, int tlen,
                                 const unsigned char *from, int flen)
{
    int j;
    unsigned char *p;

    if (flen > (tlen - RSA_PKCS1_PADDING_SIZE)) {
        RSAerr(RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_1,
               RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
        return 0;
    }

    p = (unsigned char *)to;

    *(p++) = 0;
    *(p++) = 1;                 /* Private Key BT (Block Type) */

    /* pad out with 0xff data */
    j = tlen - 3 - flen;
    memset(p, 0xff, j);
    p += j;
    *(p++) = '\0';
    memcpy(p, from, (unsigned int)flen);
    return 1;
}

int RSA_padding_check_PKCS1_type_1(unsigned char *to, int tlen,
                                   const unsigned char *from, int flen,
                                   int num)
{
    int i, j;
    const unsigned char *p;

    p = from;

    /*
     * The format is
     * 00 || 01 || PS || 00 || D
     * PS - padding string, at least 8 bytes of FF
     * D  - data.
     */

    if (num < RSA_PKCS1_PADDING_SIZE)
        return -1;

    /* Accept inputs with and without the leading 0-byte. */
    if (num == flen) {
        if ((*p++) != 0x00) {
            RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,
                   RSA_R_INVALID_PADDING);
            return -1;
        }
        flen--;
    }

    if ((num != (flen + 1)) || (*(p++) != 0x01)) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,
               RSA_R_BLOCK_TYPE_IS_NOT_01);
        return -1;
    }

    /* scan over padding data */
    j = flen - 1;               /* one for type. */
    for (i = 0; i < j; i++) {
        if (*p != 0xff) {       /* should decrypt to 0xff */
            if (*p == 0) {
                p++;
                break;
            } else {
                RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,
                       RSA_R_BAD_FIXED_HEADER_DECRYPT);
                return -1;
            }
        }
        p++;
    }

    if (i == j) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,
               RSA_R_NULL_BEFORE_BLOCK_MISSING);
        return -1;
    }

    if (i < 8) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1,
               RSA_R_BAD_PAD_BYTE_COUNT);
        return -1;
    }
    i++;                        /* Skip over the '\0' */
    j -= i;
    if (j > tlen) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1, RSA_R_DATA_TOO_LARGE);
        return -1;
    }
    memcpy(to, p, (unsigned int)j);

    return j;
}

int RSA_padding_add_PKCS1_type_2(unsigned char *to, int tlen,
                                 const unsigned char *from, int flen)
{
    int i, j;
    unsigned char *p;

    if (flen > (tlen - RSA_PKCS1_PADDING_SIZE)) {
        RSAerr(RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_2,
               RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
        return 0;
    }

    p = (unsigned char *)to;

    *(p++) = 0;
    *(p++) = 2;                 /* Public Key BT (Block Type) */

    /* pad out with non-zero random data */
    j = tlen - 3 - flen;

    if (RAND_bytes(p, j) <= 0)
        return 0;
    for (i = 0; i < j; i++) {
        if (*p == '\0')
            do {
                if (RAND_bytes(p, 1) <= 0)
                    return 0;
            } while (*p == '\0');
        p++;
    }

    *(p++) = '\0';

    memcpy(p, from, (unsigned int)flen);
    return 1;
}

int RSA_padding_check_PKCS1_type_2(unsigned char *to, int tlen,
                                   const unsigned char *from, int flen,
                                   int num)
{
    int i;
    /* |em| is the encoded message, zero-padded to exactly |num| bytes */
    unsigned char *em = NULL;
    unsigned int good, found_zero_byte, mask;
    int zero_index = 0, msg_index, mlen = -1;

    if (tlen <= 0 || flen <= 0)
        return -1;

    /*
     * PKCS#1 v1.5 decryption. See "PKCS #1 v2.2: RSA Cryptography Standard",
     * section 7.2.2.
     */

    if (flen > num || num < RSA_PKCS1_PADDING_SIZE) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2,
               RSA_R_PKCS_DECODING_ERROR);
        return -1;
    }

    em = OPENSSL_malloc(num);
    if (em == NULL) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    /*
     * Caller is encouraged to pass zero-padded message created with
     * BN_bn2binpad. Trouble is that since we can't read out of |from|'s
     * bounds, it's impossible to have an invariant memory access pattern
     * in case |from| was not zero-padded in advance.
     */
    for (from += flen, em += num, i = 0; i < num; i++) {
        mask = ~constant_time_is_zero(flen);
        flen -= 1 & mask;
        from -= 1 & mask;
        *--em = *from & mask;
    }

    good = constant_time_is_zero(em[0]);
    good &= constant_time_eq(em[1], 2);

    /* scan over padding data */
    found_zero_byte = 0;
    for (i = 2; i < num; i++) {
        unsigned int equals0 = constant_time_is_zero(em[i]);

        zero_index = constant_time_select_int(~found_zero_byte & equals0,
                                              i, zero_index);
        found_zero_byte |= equals0;
    }

    /*
     * PS must be at least 8 bytes long, and it starts two bytes into |em|.
     * If we never found a 0-byte, then |zero_index| is 0 and the check
     * also fails.
     */
    good &= constant_time_ge(zero_index, 2 + 8);

    /*
     * Skip the zero byte. This is incorrect if we never found a zero-byte
     * but in this case we also do not copy the message out.
     */
    msg_index = zero_index + 1;
    mlen = num - msg_index;

    /*
     * For good measure, do this check in constant time as well.
     */
    good &= constant_time_ge(tlen, mlen);

    /*
     * Move the result in-place by |num|-RSA_PKCS1_PADDING_SIZE-|mlen| bytes to the left.
     * Then if |good| move |mlen| bytes from |em|+RSA_PKCS1_PADDING_SIZE to |to|.
     * Otherwise leave |to| unchanged.
     * Copy the memory back in a way that does not reveal the size of
     * the data being copied via a timing side channel. This requires copying
     * parts of the buffer multiple times based on the bits set in the real
     * length. Clear bits do a non-copy with identical access pattern.
     * The loop below has overall complexity of O(N*log(N)).
     */
    tlen = constant_time_select_int(constant_time_lt(num - RSA_PKCS1_PADDING_SIZE, tlen),
                                    num - RSA_PKCS1_PADDING_SIZE, tlen);
    for (msg_index = 1; msg_index < num - RSA_PKCS1_PADDING_SIZE; msg_index <<= 1) {
        mask = ~constant_time_eq(msg_index & (num - RSA_PKCS1_PADDING_SIZE - mlen), 0);
        for (i = RSA_PKCS1_PADDING_SIZE; i < num - msg_index; i++)
            em[i] = constant_time_select_8(mask, em[i + msg_index], em[i]);
    }
    for (i = 0; i < tlen; i++) {
        mask = good & constant_time_lt(i, mlen);
        to[i] = constant_time_select_8(mask, em[i + RSA_PKCS1_PADDING_SIZE], to[i]);
    }

    OPENSSL_clear_free(em, num);
    RSAerr(RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2, RSA_R_PKCS_DECODING_ERROR);
    err_clear_last_constant_time(1 & good);

    return constant_time_select_int(good, mlen, -1);
}

static int ossl_rsa_prf(unsigned char *to, int tlen,
                        const char *label, int llen,
                        const unsigned char *kdk,
                        uint16_t bitlen)
{
    int pos;
    int ret = -1;
    uint16_t iter = 0;
    unsigned char be_iter[sizeof(iter)];
    unsigned char be_bitlen[sizeof(bitlen)];
    HMAC_CTX *hmac = NULL;
    const EVP_MD *md = NULL;
    unsigned char hmac_out[SHA256_DIGEST_LENGTH];
    unsigned int md_len;

    if (tlen * 8 != bitlen) {
        RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
        return ret;
    }

    be_bitlen[0] = (bitlen >> 8) & 0xff;
    be_bitlen[1] = bitlen & 0xff;

    hmac = HMAC_CTX_new();
    if (hmac == NULL) {
        RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
        goto err;
    }

    /*
     * we use hardcoded hash so that migrating between versions that use
     * different hash doesn't provide a Bleichenbacher oracle:
     * if the attacker can see that different versions return different
     * messages for the same ciphertext, they'll know that the message is
     * syntethically generated, which means that the padding check failed
     */
    md = EVP_sha256();
    if (md == NULL) {
        RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
        goto err;
    }

    if (HMAC_Init_ex(hmac, kdk, SHA256_DIGEST_LENGTH, md, NULL) <= 0) {
        RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
        goto err;
    }

    for (pos = 0; pos < tlen; pos += SHA256_DIGEST_LENGTH, iter++) {
        if (HMAC_Init_ex(hmac, NULL, 0, NULL, NULL) <= 0) {
            RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
            goto err;
        }

        be_iter[0] = (iter >> 8) & 0xff;
        be_iter[1] = iter & 0xff;

        if (HMAC_Update(hmac, be_iter, sizeof(be_iter)) <= 0) {
            RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        if (HMAC_Update(hmac, (unsigned char *)label, llen) <= 0) {
            RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        if (HMAC_Update(hmac, be_bitlen, sizeof(be_bitlen)) <= 0) {
            RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
            goto err;
        }

        /*
         * HMAC_Final requires the output buffer to fit the whole MAC
         * value, so we need to use the intermediate buffer for the last
         * unaligned block
         */
        md_len = SHA256_DIGEST_LENGTH;
        if (pos + SHA256_DIGEST_LENGTH > tlen) {
            if (HMAC_Final(hmac, hmac_out, &md_len) <= 0) {
                RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            memcpy(to + pos, hmac_out, tlen - pos);
        } else {
            if (HMAC_Final(hmac, to + pos, &md_len) <= 0) {
                RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
                goto err;
            }
        }
    }

    ret = 0;

err:
    HMAC_CTX_free(hmac);
    return ret;
}

/*
 * ossl_rsa_padding_check_PKCS1_type_2() checks and removes the PKCS#1 type 2
 * padding from a decrypted RSA message. Unlike the
 * RSA_padding_check_PKCS1_type_2() it will not return an error in case it
 * detects a padding error, rather it will return a deterministically generated
 * random message. In other words it will perform an implicit rejection
 * of an invalid padding. This means that the returned value does not indicate
 * if the padding of the encrypted message was correct or not, making
 * side channel attacks like the ones described by Bleichenbacher impossible
 * without access to the full decrypted value and a brute-force search of
 * remaining padding bytes
 */
int ossl_rsa_padding_check_PKCS1_type_2(unsigned char *to, int tlen,
                                        const unsigned char *from, int flen,
                                        int num, unsigned char *kdk)
{
/*
 * We need to generate a random length for the synthethic message, to avoid
 * bias towards zero and avoid non-constant timeness of DIV, we prepare
 * 128 values to check if they are not too large for the used key size,
 * and use 0 in case none of them are small enough, as 2^-128 is a good enough
 * safety margin
 */
#define MAX_LEN_GEN_TRIES 128
    unsigned char *synthetic = NULL;
    int synthethic_length;
    uint16_t len_candidate;
    unsigned char candidate_lengths[MAX_LEN_GEN_TRIES * sizeof(len_candidate)];
    uint16_t len_mask;
    uint16_t max_sep_offset;
    int synth_msg_index = 0;
    int ret = -1;
    int i, j;
    unsigned int good, found_zero_byte;
    int zero_index = 0, msg_index;

    /*
     * If these checks fail then either the message in publicly invalid, or
     * we've been called incorrectly. We can fail immediately.
     * Since this code is called only internally by openssl, those are just
     * sanity checks
     */
    if (num != flen || tlen <= 0 || flen <= 0) {
        RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
        return -1;
    }

    /* Generate a random message to return in case the padding checks fail */
    synthetic = OPENSSL_malloc(flen);
    if (synthetic == NULL) {
        RSAerr(ERR_LIB_RSA, ERR_R_MALLOC_FAILURE);
        return -1;
    }

    if (ossl_rsa_prf(synthetic, flen, "message", 7, kdk, flen * 8) < 0)
        goto err;

    /* decide how long the random message should be */
    if (ossl_rsa_prf(candidate_lengths, sizeof(candidate_lengths),
                     "length", 6, kdk,
                     MAX_LEN_GEN_TRIES * sizeof(len_candidate) * 8) < 0)
        goto err;

    /*
     * max message size is the size of the modulus size less 2 bytes for
     * version and padding type and a minimum of 8 bytes padding
     */
    len_mask = max_sep_offset = flen - 2 - 8;
    /*
     * we want a mask so lets propagate the high bit to all positions less
     * significant than it
     */
    len_mask |= len_mask >> 1;
    len_mask |= len_mask >> 2;
    len_mask |= len_mask >> 4;
    len_mask |= len_mask >> 8;

    synthethic_length = 0;
    for (i = 0; i < MAX_LEN_GEN_TRIES * (int)sizeof(len_candidate);
            i += sizeof(len_candidate)) {
        len_candidate = (candidate_lengths[i] << 8) | candidate_lengths[i + 1];
        len_candidate &= len_mask;

        synthethic_length = constant_time_select_int(
            constant_time_lt(len_candidate, max_sep_offset),
            len_candidate, synthethic_length);
    }

    synth_msg_index = flen - synthethic_length;

    /* we have alternative message ready, check the real one */
    good = constant_time_is_zero(from[0]);
    good &= constant_time_eq(from[1], 2);

    /* then look for the padding|message separator (the first zero byte) */
    found_zero_byte = 0;
    for (i = 2; i < flen; i++) {
        unsigned int equals0 = constant_time_is_zero(from[i]);
        zero_index = constant_time_select_int(~found_zero_byte & equals0,
                                              i, zero_index);
        found_zero_byte |= equals0;
    }

    /*
     * padding must be at least 8 bytes long, and it starts two bytes into
     * |from|. If we never found a 0-byte, then |zero_index| is 0 and the check
     * also fails.
     */
    good &= constant_time_ge(zero_index, 2 + 8);

    /*
     * Skip the zero byte. This is incorrect if we never found a zero-byte
     * but in this case we also do not copy the message out.
     */
    msg_index = zero_index + 1;

    /*
     * old code returned an error in case the decrypted message wouldn't fit
     * into the |to|, since that would leak information, return the synthethic
     * message instead
     */
    good &= constant_time_ge(tlen, num - msg_index);

    msg_index = constant_time_select_int(good, msg_index, synth_msg_index);

    /*
     * since at this point the |msg_index| does not provide the signal
     * indicating if the padding check failed or not, we don't have to worry
     * about leaking the length of returned message, we still need to ensure
     * that we read contents of both buffers so that cache accesses don't leak
     * the value of |good|
     */
    for (i = msg_index, j = 0; i < flen && j < tlen; i++, j++)
        to[j] = constant_time_select_8(good, from[i], synthetic[i]);
    ret = j;

err:
    /*
     * the only time ret < 0 is when the ciphertext is publicly invalid
     * or we were called with invalid parameters, so we don't have to perform
     * a side-channel secure raising of the error
     */
    if (ret < 0)
        RSAerr(ERR_LIB_RSA, ERR_R_INTERNAL_ERROR);
    OPENSSL_free(synthetic);
    return ret;
}

