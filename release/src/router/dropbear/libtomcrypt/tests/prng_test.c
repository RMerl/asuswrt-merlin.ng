/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <tomcrypt_test.h>

#ifdef LTC_PRNG_ENABLE_LTC_RNG

static unsigned long my_test_rng_read;

static unsigned long my_test_rng(unsigned char *buf, unsigned long len,
                             void (*callback)(void))
{
   unsigned long n;
   LTC_UNUSED_PARAM(callback);
   for (n = 0; n < len; ++n) {
      buf[n] = 4;
   }
   my_test_rng_read += n;
   return n;
}

#endif

int prng_test(void)
{
   int           err = CRYPT_NOP;
   int           x;
   unsigned char buf[4096] = { 0 };
   unsigned long n, one;
   prng_state    nprng;

#ifdef LTC_PRNG_ENABLE_LTC_RNG
   unsigned long before;

   unsigned long (*previous)(unsigned char *, unsigned long , void (*)(void)) = ltc_rng;
   ltc_rng = my_test_rng;

   before = my_test_rng_read;

   if ((err = rng_make_prng(128, find_prng("yarrow"), &yarrow_prng, NULL)) != CRYPT_OK) {
      fprintf(stderr, "rng_make_prng with 'my_test_rng' failed: %s\n", error_to_string(err));
      exit(EXIT_FAILURE);
   }

   if (before == my_test_rng_read) {
      fprintf(stderr, "somehow there was no read from the ltc_rng! %lu == %lu\n", before, my_test_rng_read);
      exit(EXIT_FAILURE);
   }

   ltc_rng = previous;
#endif

   /* test prngs (test, import/export) */
   for (x = 0; prng_descriptor[x].name != NULL; x++) {
      if(strstr(prng_descriptor[x].name, "no_prng") == prng_descriptor[x].name) continue;
      err = CRYPT_OK;
      DOX(prng_descriptor[x].test(), prng_descriptor[x].name);
      DOX(prng_descriptor[x].start(&nprng), prng_descriptor[x].name);
      DOX(prng_descriptor[x].add_entropy((unsigned char *)"helloworld12", 12, &nprng), prng_descriptor[x].name);
      DOX(prng_descriptor[x].ready(&nprng), prng_descriptor[x].name);
      n = sizeof(buf);
      if (strcmp(prng_descriptor[x].name, "sprng")) {
         one = 1;
         if (prng_descriptor[x].pexport(buf, &one, &nprng) != CRYPT_BUFFER_OVERFLOW) {
            fprintf(stderr, "Error testing pexport with a short buffer (%s)\n", prng_descriptor[x].name);
            return CRYPT_ERROR;
         }
      }
      DOX(prng_descriptor[x].pexport(buf, &n, &nprng), prng_descriptor[x].name);
      prng_descriptor[x].done(&nprng);
      DOX(prng_descriptor[x].pimport(buf, n, &nprng), prng_descriptor[x].name);
      DOX(prng_descriptor[x].pimport(buf, 4096, &nprng), prng_descriptor[x].name); /* try to import larger data */
      DOX(prng_descriptor[x].ready(&nprng), prng_descriptor[x].name);
      if (prng_descriptor[x].read(buf, 100, &nprng) != 100) {
         fprintf(stderr, "Error reading from imported PRNG (%s)!\n", prng_descriptor[x].name);
         return CRYPT_ERROR;
      }
      prng_descriptor[x].done(&nprng);
   }
   return err;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
