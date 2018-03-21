/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <tomcrypt_test.h>

#if defined(LTC_PKCS_1) && defined(LTC_TEST_MPI)

#include "../notes/rsa-testvectors/pss-vect.c"



int pkcs_1_pss_test(void)
{
  struct ltc_prng_descriptor* no_prng_desc = no_prng_desc_get();
  int prng_idx = register_prng(no_prng_desc);
  int hash_idx = find_hash("sha1");
  unsigned int i;
  unsigned int j;

  DO(prng_is_valid(prng_idx));
  DO(hash_is_valid(hash_idx));

  for (i = 0; i < sizeof(testcases_pss)/sizeof(testcases_pss[0]); ++i) {
    testcase_t* t = &testcases_pss[i];
    rsa_key k, *key = &k;
    DOX(mp_init_multi(&key->e, &key->d, &key->N, &key->dQ,
                       &key->dP, &key->qP, &key->p, &key->q, NULL), t->name);

    DOX(mp_read_unsigned_bin(key->e, t->rsa.e, t->rsa.e_l), t->name);
    DOX(mp_read_unsigned_bin(key->d, t->rsa.d, t->rsa.d_l), t->name);
    DOX(mp_read_unsigned_bin(key->N, t->rsa.n, t->rsa.n_l), t->name);
    DOX(mp_read_unsigned_bin(key->dQ, t->rsa.dQ, t->rsa.dQ_l), t->name);
    DOX(mp_read_unsigned_bin(key->dP, t->rsa.dP, t->rsa.dP_l), t->name);
    DOX(mp_read_unsigned_bin(key->qP, t->rsa.qInv, t->rsa.qInv_l), t->name);
    DOX(mp_read_unsigned_bin(key->q, t->rsa.q, t->rsa.q_l), t->name);
    DOX(mp_read_unsigned_bin(key->p, t->rsa.p, t->rsa.p_l), t->name);
    key->type = PK_PRIVATE;

    for (j = 0; j < sizeof(t->data)/sizeof(t->data[0]); ++j) {
        rsaData_t* s = &t->data[j];
        unsigned char buf[20], obuf[256];
        unsigned long buflen = sizeof(buf), obuflen = sizeof(obuf);
        int stat;
        prng_descriptor[prng_idx].add_entropy(s->o2, s->o2_l, (prng_state*)no_prng_desc);
        DOX(hash_memory(hash_idx, s->o1, s->o1_l, buf, &buflen), s->name);
        DOX(rsa_sign_hash(buf, buflen, obuf, &obuflen, (prng_state*)no_prng_desc, prng_idx, hash_idx, s->o2_l, key), s->name);
        DOX(obuflen == (unsigned long)s->o3_l?CRYPT_OK:CRYPT_FAIL_TESTVECTOR, s->name);
        DOX(memcmp(s->o3, obuf, s->o3_l)==0?CRYPT_OK:CRYPT_FAIL_TESTVECTOR, s->name);
        DOX(rsa_verify_hash(obuf, obuflen, buf, buflen, hash_idx, s->o2_l, &stat, key), s->name);
        DOX(stat == 1?CRYPT_OK:CRYPT_FAIL_TESTVECTOR, s->name);
    } /* for */

    mp_clear_multi(key->d,  key->e, key->N, key->dQ, key->dP, key->qP, key->p, key->q, NULL);
  } /* for */

  unregister_prng(no_prng_desc);
  no_prng_desc_free(no_prng_desc);

  return 0;
}

#else

int pkcs_1_pss_test(void)
{
   return CRYPT_NOP;
}

#endif


/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
