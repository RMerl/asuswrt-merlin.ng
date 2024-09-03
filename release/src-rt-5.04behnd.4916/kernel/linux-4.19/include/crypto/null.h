/* SPDX-License-Identifier: GPL-2.0 */
/* Values for NULL algorithms */

#ifndef _CRYPTO_NULL_H
#define _CRYPTO_NULL_H

#define NULL_KEY_SIZE		0
#define NULL_BLOCK_SIZE		1
#define NULL_DIGEST_SIZE	0
#define NULL_IV_SIZE		0

#if defined(CONFIG_BCM_KF_VLA_REMOVAL_BACKPORT)
struct crypto_sync_skcipher *crypto_get_default_null_skcipher(void);
#else
struct crypto_skcipher *crypto_get_default_null_skcipher(void);
#endif
void crypto_put_default_null_skcipher(void);

#endif
