/* Copyright (c) 2001, Matej Pfajfar.
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2020, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file aes_nss.c
 * \brief Use NSS to implement AES_CTR.
 **/

#include "orconfig.h"
#include "lib/crypt_ops/aes.h"
#include "lib/crypt_ops/crypto_nss_mgt.h"
#include "lib/crypt_ops/crypto_util.h"
#include "lib/log/util_bug.h"

DISABLE_GCC_WARNING("-Wstrict-prototypes")
#include <pk11pub.h>
#include <secerr.h>
ENABLE_GCC_WARNING("-Wstrict-prototypes")

aes_cnt_cipher_t *
aes_new_cipher(const uint8_t *key, const uint8_t *iv,
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
  return (aes_cnt_cipher_t *)result;
}

void
aes_cipher_free_(aes_cnt_cipher_t *cipher)
{
  if (!cipher)
    return;
  PK11_DestroyContext((PK11Context*) cipher, PR_TRUE);
}

void
aes_crypt_inplace(aes_cnt_cipher_t *cipher, char *data_, size_t len_)
{
  tor_assert(len_ <= INT_MAX);

  SECStatus s;
  PK11Context *ctx = (PK11Context*)cipher;
  unsigned char *data = (unsigned char *)data_;
  int len = (int) len_;
  int result_len = 0;

  s = PK11_CipherOp(ctx, data, &result_len, len, data, len);
  tor_assert(s == SECSuccess);
  tor_assert(result_len == len);
}

int
evaluate_evp_for_aes(int force_value)
{
  (void)force_value;
  return 0;
}

int
evaluate_ctr_for_aes(void)
{
  return 0;
}
