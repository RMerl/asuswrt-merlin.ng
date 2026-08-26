/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2021, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file aes_nss.c
 * \brief Use NSS to implement AES_CTR.
 **/

#define USE_AES_RAW
#define TOR_AES_PRIVATE

#include "orconfig.h"
#include "lib/crypt_ops/aes.h"
#include "lib/crypt_ops/crypto_nss_mgt.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/log/util_bug.h"

DISABLE_GCC_WARNING("-Wstrict-prototypes")
#include <pk11pub.h>
#include <secerr.h>
ENABLE_GCC_WARNING("-Wstrict-prototypes")

struct aes_cnt_cipher_t {
  PK11Context *context;
  // We need to keep a copy of the key here since we can't set the IV only.
  // It would be nice to fix that, but NSS doesn't see a huge number of
  // users.
  uint8_t kbytes;
  uint8_t key[32];
};

static PK11Context *
aes_new_cipher_internal(const uint8_t *key, const uint8_t *iv,
                        int key_bits)
{
  const CK_MECHANISM_TYPE ckm = CKM_AES_CTR;
  SECItem keyItem = { .type = siBuffer,
                      .data = (unsigned char *)key,
                      .len = (key_bits / 8) };
  CK_AES_CTR_PARAMS params;
  params.ulCounterBits = 128;
  memcpy(params.cb, iv, 16);
  SECItem ivItem = { .type = siBuffer,
                     .data = (unsigned char *)&params,
                     .len = sizeof(params) };
  PK11SlotInfo *slot = NULL;
  PK11SymKey *keyObj = NULL;
  SECItem *ivObj = NULL;
  PK11Context *result = NULL;

  slot = PK11_GetBestSlot(ckm, NULL);
  if (!slot)
    goto err;

  keyObj = PK11_ImportSymKey(slot, ckm, PK11_OriginUnwrap,
                             CKA_ENCRYPT, &keyItem, NULL);
  if (!keyObj)
    goto err;

  ivObj = PK11_ParamFromIV(ckm, &ivItem);
  if (!ivObj)
    goto err;

  PORT_SetError(SEC_ERROR_IO);
  result = PK11_CreateContextBySymKey(ckm, CKA_ENCRYPT, keyObj, ivObj);

 err:
  memwipe(&params, 0, sizeof(params));
  if (ivObj)
    SECITEM_FreeItem(ivObj, PR_TRUE);
  if (keyObj)
    PK11_FreeSymKey(keyObj);
  if (slot)
    PK11_FreeSlot(slot);

  tor_assert(result);
  return result;
}

aes_cnt_cipher_t *
aes_new_cipher(const uint8_t *key, const uint8_t *iv,
                        int key_bits)
{
  aes_cnt_cipher_t *cipher = tor_malloc_zero(sizeof(*cipher));
  cipher->context = aes_new_cipher_internal(key, iv, key_bits);
  cipher->kbytes = key_bits / 8;
  memcpy(cipher->key, key, cipher->kbytes);
  return cipher;
}

void
aes_cipher_free_(aes_cnt_cipher_t *cipher)
{
  if (!cipher)
    return;
  PK11_DestroyContext(cipher->context, PR_TRUE);
  memwipe(cipher, 0, sizeof(*cipher));
  tor_free(cipher);
}

void
aes_cipher_set_iv_aligned(aes_cnt_cipher_t *cipher, const uint8_t *iv)
{
  // For NSS, I could not find a method to change the IV
  // of an existing context.  Maybe I missed one?
  PK11_DestroyContext(cipher->context, PR_TRUE);
  cipher->context = aes_new_cipher_internal(cipher->key, iv,
                                            8*(int)cipher->kbytes);
}

void
aes_cipher_set_key(aes_cnt_cipher_t *cipher,
                   const uint8_t *key, int key_bits)
{
  const uint8_t iv[16] = {0};
  // For NSS, I could not find a method to change the key
  // of an existing context. Maybe I missed one?
  PK11_DestroyContext(cipher->context, PR_TRUE);
  memwipe(cipher->key, 0, sizeof(cipher->key));

  cipher->context = aes_new_cipher_internal(key, iv, key_bits);
  cipher->kbytes = key_bits / 8;
  memcpy(cipher->key, key, cipher->kbytes);
}

void
aes_crypt_inplace(aes_cnt_cipher_t *cipher, char *data_, size_t len_)
{
  tor_assert(len_ <= INT_MAX);

  SECStatus s;
  unsigned char *data = (unsigned char *)data_;
  int len = (int) len_;
  int result_len = 0;

  s = PK11_CipherOp(cipher->context, data, &result_len, len, data, len);
  tor_assert(s == SECSuccess);
  tor_assert(result_len == len);
}

aes_raw_t *
aes_raw_new(const uint8_t *key, int key_bits, bool encrypt)
{
  const CK_MECHANISM_TYPE ckm = CKM_AES_ECB;
  SECItem keyItem = { .type = siBuffer, // ????
                      .data = (unsigned char *)key,
                      .len = (key_bits / 8) };
  SECItem ivItem = { .type = siBuffer,
                     .data = NULL,
                     .len = 0 };
  PK11SlotInfo *slot = NULL;
  PK11SymKey *keyObj = NULL;
  SECItem *ivObj = NULL;
  PK11Context *result = NULL;

  slot = PK11_GetBestSlot(ckm, NULL);
  if (!slot)
    goto err;

  CK_ATTRIBUTE_TYPE mode = encrypt ? CKA_ENCRYPT : CKA_DECRYPT;

  keyObj = PK11_ImportSymKey(slot, ckm, PK11_OriginUnwrap,
                             mode, &keyItem, NULL);
  if (!keyObj)
    goto err;

  ivObj = PK11_ParamFromIV(ckm, &ivItem);
  if (!ivObj)
    goto err;

  PORT_SetError(SEC_ERROR_IO);
  result = PK11_CreateContextBySymKey(ckm, mode, keyObj, ivObj);

 err:

  if (ivObj)
    SECITEM_FreeItem(ivObj, PR_TRUE);
  if (keyObj)
    PK11_FreeSymKey(keyObj);
  if (slot)
    PK11_FreeSlot(slot);

  tor_assert(result);
  return (aes_raw_t *)result;
}
void
aes_raw_free_(aes_raw_t *cipher_)
{
  if (!cipher_)
    return;
  PK11Context *ctx = (PK11Context*)cipher_;
  PK11_DestroyContext(ctx, PR_TRUE);
}
void
aes_raw_set_key(aes_raw_t **cipher, const uint8_t *key,
                int key_bits, bool encrypt)
{
  // For NSS, I could not find a method to change the key
  // of an existing context.
  aes_raw_free(*cipher);
  *cipher = aes_raw_new(key, key_bits, encrypt);
}
void
aes_raw_encrypt(const aes_raw_t *cipher, uint8_t *block)
{
  SECStatus s;
  PK11Context *ctx = (PK11Context*)cipher;
  int result_len = 0;
  s = PK11_CipherOp(ctx, block, &result_len, 16, block, 16);
  tor_assert(s == SECSuccess);
  tor_assert(result_len == 16);
}
void
aes_raw_decrypt(const aes_raw_t *cipher, uint8_t *block)
{
  /* This is the same function call for NSS. */
  aes_raw_encrypt(cipher, block);
}
