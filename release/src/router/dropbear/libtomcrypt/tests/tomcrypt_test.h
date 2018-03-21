/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#ifndef __TEST_H_
#define __TEST_H_

#include <tomcrypt.h>

#include "common.h"

#ifdef USE_LTM
/* Use libtommath as MPI provider */
#define LTC_TEST_MPI
#elif defined(USE_TFM)
/* Use tomsfastmath as MPI provider */
#define LTC_TEST_MPI
#elif defined(USE_GMP)
/* Use GNU Multiple Precision Arithmetic Library as MPI provider */
#define LTC_TEST_MPI
#elif defined(EXT_MATH_LIB)
/* The user must define his own MPI provider! */
#define LTC_TEST_MPI
#endif

typedef struct {
    char *name, *prov, *req;
    int  (*entry)(void);
} test_entry;

/* TESTS */
int cipher_hash_test(void);
int modes_test(void);
int mac_test(void);
int pkcs_1_test(void);
int pkcs_1_pss_test(void);
int pkcs_1_oaep_test(void);
int pkcs_1_emsa_test(void);
int pkcs_1_eme_test(void);
int store_test(void);
int rotate_test(void);
int rsa_test(void);
int dh_test(void);
int katja_test(void);
int ecc_tests(void);
int dsa_test(void);
int der_test(void);
int misc_test(void);
int base64_test(void);
int file_test(void);
int multi_test(void);
int prng_test(void);
int mpi_test(void);

#ifdef LTC_PKCS_1
struct ltc_prng_descriptor* no_prng_desc_get(void);
void no_prng_desc_free(struct ltc_prng_descriptor*);
#endif

#endif

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
