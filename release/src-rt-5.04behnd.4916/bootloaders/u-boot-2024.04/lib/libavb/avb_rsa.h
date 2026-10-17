/* SPDX-License-Identifier: MIT OR BSD-3-Clause */
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

/* Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifdef AVB_INSIDE_LIBAVB_H
#error "You can't include avb_rsa.h in the public header libavb.h."
#endif

#ifndef AVB_COMPILATION
#error "Never include this file, it may only be used from internal avb code."
#endif

#ifndef AVB_RSA_H_
#define AVB_RSA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "avb_crypto.h"
#include "avb_sysdeps.h"

/* Using the key given by |key|, verify a RSA signature |sig| of
 * length |sig_num_bytes| against an expected |hash| of length
 * |hash_num_bytes|. The padding to expect must be passed in using
 * |padding| of length |padding_num_bytes|.
 *
 * The data in |key| must match the format defined in
 * |AvbRSAPublicKeyHeader|, including the two large numbers
 * following. The |key_num_bytes| must be the size of the entire
 * serialized key.
 *
 * Returns false if verification fails, true otherwise.
 */
bool avb_rsa_verify(const uint8_t* key,
                    size_t key_num_bytes,
                    const uint8_t* sig,
                    size_t sig_num_bytes,
                    const uint8_t* hash,
                    size_t hash_num_bytes,
                    const uint8_t* padding,
                    size_t padding_num_bytes) AVB_ATTR_WARN_UNUSED_RESULT;

#ifdef __cplusplus
}
#endif

#endif /* AVB_RSA_H_ */
