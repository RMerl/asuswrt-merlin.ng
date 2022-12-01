/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "internal/nelem.h"
#include "testutil.h"
#include <openssl/ec.h>
#include "ec_local.h"
#include <openssl/objects.h>

static size_t crv_len = 0;
static EC_builtin_curve *curves = NULL;

/* sanity checks field_inv function pointer in EC_METHOD */
static int group_field_tests(const EC_GROUP *group, BN_CTX *ctx)
{
    BIGNUM *a = NULL, *b = NULL, *c = NULL;
    int ret = 0;

    if (group->meth->field_inv == NULL || group->meth->field_mul == NULL)
        return 1;

    BN_CTX_start(ctx);
    a = BN_CTX_get(ctx);
    b = BN_CTX_get(ctx);
    if (!TEST_ptr(c = BN_CTX_get(ctx))
        /* 1/1 = 1 */
        || !TEST_true(group->meth->field_inv(group, b, BN_value_one(), ctx))
        || !TEST_true(BN_is_one(b))
        /* (1/a)*a = 1 */
        || !TEST_true(BN_pseudo_rand(a, BN_num_bits(group->field) - 1,
                                     BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ANY))
        || !TEST_true(group->meth->field_inv(group, b, a, ctx))
        || (group->meth->field_encode &&
            !TEST_true(group->meth->field_encode(group, a, a, ctx)))
        || (group->meth->field_encode &&
            !TEST_true(group->meth->field_encode(group, b, b, ctx)))
        || !TEST_true(group->meth->field_mul(group, c, a, b, ctx))
        || (group->meth->field_decode &&
            !TEST_true(group->meth->field_decode(group, c, c, ctx)))
        || !TEST_true(BN_is_one(c)))
        goto err;

    /* 1/0 = error */
    BN_zero(a);
    if (!TEST_false(group->meth->field_inv(group, b, a, ctx))
        || !TEST_true(ERR_GET_LIB(ERR_peek_last_error()) == ERR_LIB_EC)
        || !TEST_true(ERR_GET_REASON(ERR_peek_last_error()) ==
                      EC_R_CANNOT_INVERT)
        /* 1/p = error */
        || !TEST_false(group->meth->field_inv(group, b, group->field, ctx))
        || !TEST_true(ERR_GET_LIB(ERR_peek_last_error()) == ERR_LIB_EC)
        || !TEST_true(ERR_GET_REASON(ERR_peek_last_error()) ==
                      EC_R_CANNOT_INVERT))
        goto err;

    ERR_clear_error();
    ret = 1;
 err:
    BN_CTX_end(ctx);
    return ret;
}

/* wrapper for group_field_tests for explicit curve params and EC_METHOD */
static int field_tests(const EC_METHOD *meth, const unsigned char *params,
                       int len)
{
    BN_CTX *ctx = NULL;
    BIGNUM *p = NULL, *a = NULL, *b = NULL;
    EC_GROUP *group = NULL;
    int ret = 0;

    if (!TEST_ptr(ctx = BN_CTX_new()))
        return 0;

    BN_CTX_start(ctx);
    p = BN_CTX_get(ctx);
    a = BN_CTX_get(ctx);
    if (!TEST_ptr(b = BN_CTX_get(ctx))
        || !TEST_ptr(group = EC_GROUP_new(meth))
        || !TEST_true(BN_bin2bn(params, len, p))
        || !TEST_true(BN_bin2bn(params + len, len, a))
        || !TEST_true(BN_bin2bn(params + 2 * len, len, b))
        || !TEST_true(EC_GROUP_set_curve(group, p, a, b, ctx))
        || !group_field_tests(group, ctx))
        goto err;
    ret = 1;

 err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    if (group != NULL)
        EC_GROUP_free(group);
    return ret;
}

/* NIST prime curve P-256 */
static const unsigned char params_p256[] = {
    /* p */
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* a */
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,
    /* b */
    0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7, 0xB3, 0xEB, 0xBD, 0x55,
    0x76, 0x98, 0x86, 0xBC, 0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53, 0xB0, 0xF6,
    0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B
};

#ifndef OPENSSL_NO_EC2M
/* NIST binary curve B-283 */
static const unsigned char params_b283[] = {
    /* p */
    0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xA1,
    /* a */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    /* b */
    0x02, 0x7B, 0x68, 0x0A, 0xC8, 0xB8, 0x59, 0x6D, 0xA5, 0xA4, 0xAF, 0x8A,
    0x19, 0xA0, 0x30, 0x3F, 0xCA, 0x97, 0xFD, 0x76, 0x45, 0x30, 0x9F, 0xA2,
    0xA5, 0x81, 0x48, 0x5A, 0xF6, 0x26, 0x3E, 0x31, 0x3B, 0x79, 0xA2, 0xF5
};
#endif

/* test EC_GFp_simple_method directly */
static int field_tests_ecp_simple(void)
{
    TEST_info("Testing EC_GFp_simple_method()\n");
    return field_tests(EC_GFp_simple_method(), params_p256,
                       sizeof(params_p256) / 3);
}

/* test EC_GFp_mont_method directly */
static int field_tests_ecp_mont(void)
{
    TEST_info("Testing EC_GFp_mont_method()\n");
    return field_tests(EC_GFp_mont_method(), params_p256,
                       sizeof(params_p256) / 3);
}

#ifndef OPENSSL_NO_EC2M
/* test EC_GF2m_simple_method directly */
static int field_tests_ec2_simple(void)
{
    TEST_info("Testing EC_GF2m_simple_method()\n");
    return field_tests(EC_GF2m_simple_method(), params_b283,
                       sizeof(params_b283) / 3);
}
#endif

/* test default method for a named curve */
static int field_tests_default(int n)
{
    BN_CTX *ctx = NULL;
    EC_GROUP *group = NULL;
    int nid = curves[n].nid;
    int ret = 0;

    TEST_info("Testing curve %s\n", OBJ_nid2sn(nid));

    if (!TEST_ptr(group = EC_GROUP_new_by_curve_name(nid))
        || !TEST_ptr(ctx = BN_CTX_new())
        || !group_field_tests(group, ctx))
        goto err;

    ret = 1;
 err:
    if (group != NULL)
        EC_GROUP_free(group);
    if (ctx != NULL)
        BN_CTX_free(ctx);
    return ret;
}

/*
 * Tests behavior of the EC_KEY_set_private_key
 */
static int set_private_key(void)
{
    EC_KEY *key = NULL, *aux_key = NULL;
    int testresult = 0;

    key = EC_KEY_new_by_curve_name(NID_secp224r1);
    aux_key = EC_KEY_new_by_curve_name(NID_secp224r1);
    if (!TEST_ptr(key)
        || !TEST_ptr(aux_key)
        || !TEST_int_eq(EC_KEY_generate_key(key), 1)
        || !TEST_int_eq(EC_KEY_generate_key(aux_key), 1))
        goto err;

    /* Test setting a valid private key */
    if (!TEST_int_eq(EC_KEY_set_private_key(key, aux_key->priv_key), 1))
        goto err;

    /* Test compliance with legacy behavior for NULL private keys */
    if (!TEST_int_eq(EC_KEY_set_private_key(key, NULL), 0)
        || !TEST_ptr_null(key->priv_key))
        goto err;

    testresult = 1;

 err:
    EC_KEY_free(key);
    EC_KEY_free(aux_key);
    return testresult;
}

/*
 * Tests behavior of the decoded_from_explicit_params flag and API
 */
static int decoded_flag_test(void)
{
    EC_GROUP *grp;
    EC_GROUP *grp_copy = NULL;
    ECPARAMETERS *ecparams = NULL;
    ECPKPARAMETERS *ecpkparams = NULL;
    EC_KEY *key = NULL;
    unsigned char *encodedparams = NULL;
    const unsigned char *encp;
    int encodedlen;
    int testresult = 0;

    /* Test EC_GROUP_new not setting the flag */
    grp = EC_GROUP_new(EC_GFp_simple_method());
    if (!TEST_ptr(grp)
        || !TEST_int_eq(grp->decoded_from_explicit_params, 0))
        goto err;
    EC_GROUP_free(grp);

    /* Test EC_GROUP_new_by_curve_name not setting the flag */
    grp = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    if (!TEST_ptr(grp)
        || !TEST_int_eq(grp->decoded_from_explicit_params, 0))
        goto err;

    /* Test EC_GROUP_new_from_ecparameters not setting the flag */
    if (!TEST_ptr(ecparams = EC_GROUP_get_ecparameters(grp, NULL))
        || !TEST_ptr(grp_copy = EC_GROUP_new_from_ecparameters(ecparams))
        || !TEST_int_eq(grp_copy->decoded_from_explicit_params, 0))
        goto err;
    EC_GROUP_free(grp_copy);
    grp_copy = NULL;
    ECPARAMETERS_free(ecparams);
    ecparams = NULL;

    /* Test EC_GROUP_new_from_ecpkparameters not setting the flag */
    if (!TEST_int_eq(EC_GROUP_get_asn1_flag(grp), OPENSSL_EC_NAMED_CURVE)
        || !TEST_ptr(ecpkparams = EC_GROUP_get_ecpkparameters(grp, NULL))
        || !TEST_ptr(grp_copy = EC_GROUP_new_from_ecpkparameters(ecpkparams))
        || !TEST_int_eq(grp_copy->decoded_from_explicit_params, 0)
        || !TEST_ptr(key = EC_KEY_new())
    /* Test EC_KEY_decoded_from_explicit_params on key without a group */
        || !TEST_int_eq(EC_KEY_decoded_from_explicit_params(key), -1)
        || !TEST_int_eq(EC_KEY_set_group(key, grp_copy), 1)
    /* Test EC_KEY_decoded_from_explicit_params negative case */
        || !TEST_int_eq(EC_KEY_decoded_from_explicit_params(key), 0))
        goto err;
    EC_GROUP_free(grp_copy);
    grp_copy = NULL;
    ECPKPARAMETERS_free(ecpkparams);
    ecpkparams = NULL;

    /* Test d2i_ECPKParameters with named params not setting the flag */
    if (!TEST_int_gt(encodedlen = i2d_ECPKParameters(grp, &encodedparams), 0)
        || !TEST_ptr(encp = encodedparams)
        || !TEST_ptr(grp_copy = d2i_ECPKParameters(NULL, &encp, encodedlen))
        || !TEST_int_eq(grp_copy->decoded_from_explicit_params, 0))
        goto err;
    EC_GROUP_free(grp_copy);
    grp_copy = NULL;
    OPENSSL_free(encodedparams);
    encodedparams = NULL;

    /* Asn1 flag stays set to explicit with EC_GROUP_new_from_ecpkparameters */
    EC_GROUP_set_asn1_flag(grp, OPENSSL_EC_EXPLICIT_CURVE);
    if (!TEST_ptr(ecpkparams = EC_GROUP_get_ecpkparameters(grp, NULL))
        || !TEST_ptr(grp_copy = EC_GROUP_new_from_ecpkparameters(ecpkparams))
        || !TEST_int_eq(EC_GROUP_get_asn1_flag(grp_copy), OPENSSL_EC_EXPLICIT_CURVE)
        || !TEST_int_eq(grp_copy->decoded_from_explicit_params, 0))
        goto err;
    EC_GROUP_free(grp_copy);
    grp_copy = NULL;

    /* Test d2i_ECPKParameters with explicit params setting the flag */
    if (!TEST_int_gt(encodedlen = i2d_ECPKParameters(grp, &encodedparams), 0)
        || !TEST_ptr(encp = encodedparams)
        || !TEST_ptr(grp_copy = d2i_ECPKParameters(NULL, &encp, encodedlen))
        || !TEST_int_eq(EC_GROUP_get_asn1_flag(grp_copy), OPENSSL_EC_EXPLICIT_CURVE)
        || !TEST_int_eq(grp_copy->decoded_from_explicit_params, 1)
        || !TEST_int_eq(EC_KEY_set_group(key, grp_copy), 1)
    /* Test EC_KEY_decoded_from_explicit_params positive case */
        || !TEST_int_eq(EC_KEY_decoded_from_explicit_params(key), 1))
        goto err;

    testresult = 1;

 err:
    EC_KEY_free(key);
    EC_GROUP_free(grp);
    EC_GROUP_free(grp_copy);
    ECPARAMETERS_free(ecparams);
    ECPKPARAMETERS_free(ecpkparams);
    OPENSSL_free(encodedparams);

    return testresult;
}

static
int ecpkparams_i2d2i_test(int n)
{
    EC_GROUP *g1 = NULL, *g2 = NULL;
    FILE *fp = NULL;
    int nid = curves[n].nid;
    int testresult = 0;

    /* create group */
    if (!TEST_ptr(g1 = EC_GROUP_new_by_curve_name(nid)))
        goto end;

    /* encode params to file */
    if (!TEST_ptr(fp = fopen("params.der", "wb"))
            || !TEST_true(i2d_ECPKParameters_fp(fp, g1)))
        goto end;

    /* flush and close file */
    if (!TEST_int_eq(fclose(fp), 0)) {
        fp = NULL;
        goto end;
    }
    fp = NULL;

    /* decode params from file */
    if (!TEST_ptr(fp = fopen("params.der", "rb"))
            || !TEST_ptr(g2 = d2i_ECPKParameters_fp(fp, NULL)))
        goto end;

    testresult = 1; /* PASS */

end:
    if (fp != NULL)
        fclose(fp);

    EC_GROUP_free(g1);
    EC_GROUP_free(g2);

    return testresult;
}

int setup_tests(void)
{
    crv_len = EC_get_builtin_curves(NULL, 0);
    if (!TEST_ptr(curves = OPENSSL_malloc(sizeof(*curves) * crv_len))
        || !TEST_true(EC_get_builtin_curves(curves, crv_len)))
        return 0;

    ADD_TEST(field_tests_ecp_simple);
    ADD_TEST(field_tests_ecp_mont);
#ifndef OPENSSL_NO_EC2M
    ADD_TEST(field_tests_ec2_simple);
#endif
    ADD_ALL_TESTS(field_tests_default, crv_len);
    ADD_TEST(set_private_key);
    ADD_TEST(decoded_flag_test);
    ADD_ALL_TESTS(ecpkparams_i2d2i_test, crv_len);

    return 1;
}

void cleanup_tests(void)
{
    OPENSSL_free(curves);
}
