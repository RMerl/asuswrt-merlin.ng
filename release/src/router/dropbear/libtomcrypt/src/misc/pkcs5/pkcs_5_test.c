/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include "tomcrypt.h"

/**
  @file hkdf_test.c
  PKCS #5 support, self-test, Steffen Jaeckel
*/

#ifdef LTC_PKCS_5

/*
    TEST CASES SOURCE:

Internet Engineering Task Force (IETF)                      S. Josefsson
Request for Comments: 6070                                        SJD AB
Category: Informational                                     January 2011
ISSN: 2070-1721
*/

/**
  PKCS #5 self-test
  @return CRYPT_OK if successful, CRYPT_NOP if tests have been disabled.
*/
int pkcs_5_test (void)
{
 #ifndef LTC_TEST
    return CRYPT_NOP;
 #else

    typedef struct {
        const char* P;
        unsigned long P_len;
        const char* S;
        unsigned long S_len;
        int c;
        unsigned long dkLen;
        unsigned char DK[40];
    } case_item;

    static const case_item cases_5_2[] = {
        {
            "password",
            8,
            "salt",
            4,
            1,
            20,
            { 0x0c, 0x60, 0xc8, 0x0f, 0x96, 0x1f, 0x0e, 0x71,
              0xf3, 0xa9, 0xb5, 0x24, 0xaf, 0x60, 0x12, 0x06,
              0x2f, 0xe0, 0x37, 0xa6 }
        },
        {
            "password",
            8,
            "salt",
            4,
            2,
            20,
            { 0xea, 0x6c, 0x01, 0x4d, 0xc7, 0x2d, 0x6f, 0x8c,
              0xcd, 0x1e, 0xd9, 0x2a, 0xce, 0x1d, 0x41, 0xf0,
              0xd8, 0xde, 0x89, 0x57 }
        },
#ifdef LTC_TEST_EXT
        {
            "password",
            8,
            "salt",
            4,
            4096,
            20,
            { 0x4b, 0x00, 0x79, 0x01, 0xb7, 0x65, 0x48, 0x9a,
              0xbe, 0xad, 0x49, 0xd9, 0x26, 0xf7, 0x21, 0xd0,
              0x65, 0xa4, 0x29, 0xc1 }
        },
        {
            "password",
            8,
            "salt",
            4,
            16777216,
            20,
            { 0xee, 0xfe, 0x3d, 0x61, 0xcd, 0x4d, 0xa4, 0xe4,
              0xe9, 0x94, 0x5b, 0x3d, 0x6b, 0xa2, 0x15, 0x8c,
              0x26, 0x34, 0xe9, 0x84 }
        },
        {
            "passwordPASSWORDpassword",
            25,
            "saltSALTsaltSALTsaltSALTsaltSALTsalt",
            36,
            4096,
            25,
            { 0x3d, 0x2e, 0xec, 0x4f, 0xe4, 0x1c, 0x84, 0x9b,
              0x80, 0xc8, 0xd8, 0x36, 0x62, 0xc0, 0xe4, 0x4a,
              0x8b, 0x29, 0x1a, 0x96, 0x4c, 0xf2, 0xf0, 0x70,
              0x38 }
        },
        {
            "pass\0word",
            9,
            "sa\0lt",
            5,
            4096,
            16,
            { 0x56, 0xfa, 0x6a, 0xa7, 0x55, 0x48, 0x09, 0x9d,
              0xcc, 0x37, 0xd7, 0xf0, 0x34, 0x25, 0xe0, 0xc3 }
        },
#endif /* LTC_TEST_EXT */
    };

    static const case_item cases_5_1[] = {
        {
            "password",
            8,
            "saltsalt", /* must be 8 octects */
            8,          /* ignored by alg1 */
            1,
            20,
            { 0xca, 0xb8, 0x6d, 0xd6, 0x26, 0x17, 0x10, 0x89, 0x1e, 0x8c,
              0xb5, 0x6e, 0xe3, 0x62, 0x56, 0x91, 0xa7, 0x5d, 0xf3, 0x44 }
        },
    };

    static const case_item cases_5_1o[] = {
        {
            "password",
            8,
            "saltsalt", /* must be 8 octects */
            8,          /* ignored by alg1_openssl */
            1,
            20,
            { 0xca, 0xb8, 0x6d, 0xd6, 0x26, 0x17, 0x10, 0x89, 0x1e, 0x8c,
              0xb5, 0x6e, 0xe3, 0x62, 0x56, 0x91, 0xa7, 0x5d, 0xf3, 0x44 }

        },
        {
            "password",
            8,
            "saltsalt", /* must be 8 octects */
            8,          /* ignored by alg1_openssl */
            1,
            30,
            { 0xca, 0xb8, 0x6d, 0xd6, 0x26, 0x17, 0x10, 0x89, 0x1e, 0x8c,
              0xb5, 0x6e, 0xe3, 0x62, 0x56, 0x91, 0xa7, 0x5d, 0xf3, 0x44,
              0xf0, 0xbf, 0xf4, 0xc1, 0x2c, 0xf3, 0x59, 0x6f, 0xc0, 0x0b }

        }
    };

    unsigned char DK[40];
    unsigned long dkLen;
    int i, err;
    int tested=0, failed=0;
    int hash = find_hash("sha1");
    if (hash == -1)
    {
#ifdef LTC_TEST_DBG
      printf("PKCS#5 test failed: 'sha1' hash not found\n");
#endif
      return CRYPT_ERROR;
    }

    /* testing alg 2 */
    for(i=0; i < (int)(sizeof(cases_5_2) / sizeof(cases_5_2[0])); i++) {
        ++tested;
        dkLen = cases_5_2[i].dkLen;
        if((err = pkcs_5_alg2((unsigned char*)cases_5_2[i].P, cases_5_2[i].P_len,
                              (unsigned char*)cases_5_2[i].S, cases_5_2[i].S_len,
                              cases_5_2[i].c, hash,
                              DK, &dkLen)) != CRYPT_OK) {
#ifdef LTC_TEST_DBG
            printf("\npkcs_5_alg2() #%d: Failed/1 (%s)\n", i, error_to_string(err));
#endif
            ++failed;
        }
        else if (compare_testvector(DK, dkLen, cases_5_2[i].DK, cases_5_2[i].dkLen, "PKCS#5_2", i)) {
            ++failed;
        }
    }

    /* testing alg 1 */
    for(i=0; i < (int)(sizeof(cases_5_1) / sizeof(case_item)); i++, tested++) {
        dkLen = cases_5_1[i].dkLen;
        if((err = pkcs_5_alg1((unsigned char*)cases_5_1[i].P, cases_5_1[i].P_len,
                              (unsigned char*)cases_5_1[i].S,
                              cases_5_1[i].c, hash,
                              DK, &dkLen)) != CRYPT_OK) {
#ifdef LTC_TEST_DBG
            printf("\npkcs_5_alg1() #%d: Failed/1 (%s)\n", i, error_to_string(err));
#endif
            ++failed;
        }
        else if (compare_testvector(DK, dkLen, cases_5_1[i].DK, cases_5_1[i].dkLen, "PKCS#5_1", i)) {
            ++failed;
        }
    }

    /* testing alg 1_openssl */
    for(i = 0; i < (int)(sizeof(cases_5_1o) / sizeof(cases_5_1o[0])); i++, tested++) {
        dkLen = cases_5_1o[i].dkLen;
        if ((err = pkcs_5_alg1_openssl((unsigned char*)cases_5_1o[i].P, cases_5_1o[i].P_len,
                                       (unsigned char*)cases_5_1o[i].S,
                                       cases_5_1o[i].c, hash,
                                       DK, &dkLen)) != CRYPT_OK) {
#ifdef LTC_TEST_DBG
            printf("\npkcs_5_alg1_openssl() #%d: Failed/1 (%s)\n", i, error_to_string(err));
#endif
            ++failed;
        }
        else if (compare_testvector(DK, dkLen, cases_5_1o[i].DK, cases_5_1o[i].dkLen, "PKCS#5_1o", i)) {
            ++failed;
        }
    }

    return (failed != 0) ? CRYPT_FAIL_TESTVECTOR : CRYPT_OK;
 #endif
}

#endif


/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
