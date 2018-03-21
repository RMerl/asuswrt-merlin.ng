/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#include <tomcrypt_test.h>

#ifndef GIT_VERSION
#define GIT_VERSION "Undefined version"
#endif

#define LTC_TEST_FN(f)  { f, #f }

typedef struct {
   int (*fn)(void);
   const char* name;
} test_function;

static const test_function test_functions[] =
{
      LTC_TEST_FN(store_test),
      LTC_TEST_FN(rotate_test),
      LTC_TEST_FN(misc_test),
      LTC_TEST_FN(mpi_test),
      LTC_TEST_FN(cipher_hash_test),
      LTC_TEST_FN(mac_test),
      LTC_TEST_FN(modes_test),
      LTC_TEST_FN(der_test),
      LTC_TEST_FN(pkcs_1_test),
      LTC_TEST_FN(pkcs_1_pss_test),
      LTC_TEST_FN(pkcs_1_oaep_test),
      LTC_TEST_FN(pkcs_1_emsa_test),
      LTC_TEST_FN(pkcs_1_eme_test),
      LTC_TEST_FN(rsa_test),
      LTC_TEST_FN(dh_test),
      LTC_TEST_FN(ecc_tests),
      LTC_TEST_FN(dsa_test),
      LTC_TEST_FN(katja_test),
      LTC_TEST_FN(file_test),
      LTC_TEST_FN(multi_test),
      /* keep the prng_test always at the end as
       * it has to be handled specially when
       * testing with LTC_PTHREAD enabled
       */
      LTC_TEST_FN(prng_test),
};


#if defined(_WIN32)
  #include <windows.h> /* GetSystemTimeAsFileTime */
#else
  #include <sys/time.h>
#endif

/* microseconds since 1970 (UNIX epoch) */
static ulong64 epoch_usec(void)
{
#if defined(LTC_NO_TEST_TIMING)
  return 0;
#elif defined(_WIN32)
  FILETIME CurrentTime;
  ulong64 cur_time;
  ULARGE_INTEGER ul;
  GetSystemTimeAsFileTime(&CurrentTime);
  ul.LowPart  = CurrentTime.dwLowDateTime;
  ul.HighPart = CurrentTime.dwHighDateTime;
  cur_time = ul.QuadPart;
  cur_time -= CONST64(116444736000000000); /* subtract epoch in microseconds */
  cur_time /= 10; /* nanoseconds > microseconds */
  return cur_time;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (ulong64)(tv.tv_sec) * 1000000 + (ulong64)(tv.tv_usec); /* get microseconds */
#endif
}

#ifdef LTC_PTHREAD
typedef struct
{
   pthread_t thread_id;
   const test_function* t;
   int err;
   ulong64 delta;
} thread_info;

static void *run(void *arg)
{
   thread_info *tinfo = arg;
   ulong64 ts;

   ts = epoch_usec();
   tinfo->err = tinfo->t->fn();
   tinfo->delta = epoch_usec() - ts;

   return arg;
}
#endif


/*
 * unregister ciphers, hashes & prngs
 */
static void _unregister_all(void)
{
#ifdef LTC_RIJNDAEL
#ifdef ENCRYPT_ONLY
   /* alternative would be
    * unregister_cipher(&rijndael_enc_desc);
    */
   unregister_cipher(&aes_enc_desc);
#else
   /* alternative would be
    * unregister_cipher(&rijndael_desc);
    */
   unregister_cipher(&aes_desc);
#endif
#endif
#ifdef LTC_BLOWFISH
  unregister_cipher(&blowfish_desc);
#endif
#ifdef LTC_XTEA
  unregister_cipher(&xtea_desc);
#endif
#ifdef LTC_RC5
  unregister_cipher(&rc5_desc);
#endif
#ifdef LTC_RC6
  unregister_cipher(&rc6_desc);
#endif
#ifdef LTC_SAFERP
  unregister_cipher(&saferp_desc);
#endif
#ifdef LTC_TWOFISH
  unregister_cipher(&twofish_desc);
#endif
#ifdef LTC_SAFER
  unregister_cipher(&safer_k64_desc);
  unregister_cipher(&safer_sk64_desc);
  unregister_cipher(&safer_k128_desc);
  unregister_cipher(&safer_sk128_desc);
#endif
#ifdef LTC_RC2
  unregister_cipher(&rc2_desc);
#endif
#ifdef LTC_DES
  unregister_cipher(&des_desc);
  unregister_cipher(&des3_desc);
#endif
#ifdef LTC_CAST5
  unregister_cipher(&cast5_desc);
#endif
#ifdef LTC_NOEKEON
  unregister_cipher(&noekeon_desc);
#endif
#ifdef LTC_SKIPJACK
  unregister_cipher(&skipjack_desc);
#endif
#ifdef LTC_KHAZAD
  unregister_cipher(&khazad_desc);
#endif
#ifdef LTC_ANUBIS
  unregister_cipher(&anubis_desc);
#endif
#ifdef LTC_KSEED
  unregister_cipher(&kseed_desc);
#endif
#ifdef LTC_KASUMI
  unregister_cipher(&kasumi_desc);
#endif
#ifdef LTC_MULTI2
  unregister_cipher(&multi2_desc);
#endif
#ifdef LTC_CAMELLIA
  unregister_cipher(&camellia_desc);
#endif

#ifdef LTC_TIGER
  unregister_hash(&tiger_desc);
#endif
#ifdef LTC_MD2
  unregister_hash(&md2_desc);
#endif
#ifdef LTC_MD4
  unregister_hash(&md4_desc);
#endif
#ifdef LTC_MD5
  unregister_hash(&md5_desc);
#endif
#ifdef LTC_SHA1
  unregister_hash(&sha1_desc);
#endif
#ifdef LTC_SHA224
  unregister_hash(&sha224_desc);
#endif
#ifdef LTC_SHA256
  unregister_hash(&sha256_desc);
#endif
#ifdef LTC_SHA384
  unregister_hash(&sha384_desc);
#endif
#ifdef LTC_SHA512
  unregister_hash(&sha512_desc);
#endif
#ifdef LTC_SHA512_224
  unregister_hash(&sha512_224_desc);
#endif
#ifdef LTC_SHA512_256
  unregister_hash(&sha512_256_desc);
#endif
#ifdef LTC_SHA3
  unregister_hash(&sha3_224_desc);
  unregister_hash(&sha3_256_desc);
  unregister_hash(&sha3_384_desc);
  unregister_hash(&sha3_512_desc);
#endif
#ifdef LTC_RIPEMD128
  unregister_hash(&rmd128_desc);
#endif
#ifdef LTC_RIPEMD160
  unregister_hash(&rmd160_desc);
#endif
#ifdef LTC_RIPEMD256
  unregister_hash(&rmd256_desc);
#endif
#ifdef LTC_RIPEMD320
  unregister_hash(&rmd320_desc);
#endif
#ifdef LTC_WHIRLPOOL
  unregister_hash(&whirlpool_desc);
#endif
#ifdef LTC_BLAKE2S
  unregister_hash(&blake2s_128_desc);
  unregister_hash(&blake2s_160_desc);
  unregister_hash(&blake2s_224_desc);
  unregister_hash(&blake2s_256_desc);
#endif
#ifdef LTC_BLAKE2B
  unregister_hash(&blake2b_160_desc);
  unregister_hash(&blake2b_256_desc);
  unregister_hash(&blake2b_384_desc);
  unregister_hash(&blake2b_512_desc);
#endif
#ifdef LTC_CHC_HASH
  unregister_hash(&chc_desc);
#endif

  unregister_prng(&yarrow_desc);
#ifdef LTC_FORTUNA
  unregister_prng(&fortuna_desc);
#endif
#ifdef LTC_RC4
  unregister_prng(&rc4_desc);
#endif
#ifdef LTC_CHACHA20_PRNG
  unregister_prng(&chacha20_prng_desc);
#endif
#ifdef LTC_SOBER128
  unregister_prng(&sober128_desc);
#endif
#ifdef LTC_SPRNG
  unregister_prng(&sprng_desc);
#endif
} /* _cleanup() */

static void register_algs(void)
{
  int err;

  atexit(_unregister_all);

#ifndef LTC_YARROW
   #error This demo requires Yarrow.
#endif
   if ((err = register_all_ciphers()) != CRYPT_OK) {
      fprintf(stderr, "register_all_ciphers err=%s\n", error_to_string(err));
      exit(EXIT_FAILURE);
   }
   if ((err = register_all_hashes()) != CRYPT_OK) {
      fprintf(stderr, "register_all_hashes err=%s\n", error_to_string(err));
      exit(EXIT_FAILURE);
   }
   if ((err = register_all_prngs()) != CRYPT_OK) {
      fprintf(stderr, "register_all_prngs err=%s\n", error_to_string(err));
      exit(EXIT_FAILURE);
   }

   if ((err = rng_make_prng(128, find_prng("yarrow"), &yarrow_prng, NULL)) != CRYPT_OK) {
      fprintf(stderr, "rng_make_prng failed: %s\n", error_to_string(err));
      exit(EXIT_FAILURE);
   }

   if (strcmp("CRYPT_OK", error_to_string(err))) {
       exit(EXIT_FAILURE);
   }
}

int main(int argc, char **argv)
{
#ifdef LTC_PTHREAD
   thread_info *tinfo, *res;
#endif
   int x, pass = 0, fail = 0, nop = 0;
   size_t fn_len, i, dots;
   char *single_test = NULL;
   ulong64 ts;
   long delta, dur, real = 0;
   register_algs();

   printf("LTC_VERSION  = %s\n%s\n\n", GIT_VERSION, crypt_build_settings);

#ifdef USE_LTM
   ltc_mp = ltm_desc;
   printf("MP_PROVIDER  = LibTomMath\n");
#elif defined(USE_TFM)
   ltc_mp = tfm_desc;
   printf("MP_PROVIDER  = TomsFastMath\n");
#elif defined(USE_GMP)
   ltc_mp = gmp_desc;
   printf("MP_PROVIDER  = GnuMP\n");
#elif defined(EXT_MATH_LIB)
   {
      extern ltc_math_descriptor EXT_MATH_LIB;
      ltc_mp = EXT_MATH_LIB;
   }

#define NAME_VALUE(s) #s"="NAME(s)
#define NAME(s) #s
   printf("MP_PROVIDER  = %s\n", NAME_VALUE(EXT_MATH_LIB));
#undef NAME_VALUE
#undef NAME

#endif
#ifdef LTC_TEST_MPI
   printf("MP_DIGIT_BIT = %d\n", MP_DIGIT_BIT);
#else
   printf("NO math provider selected, all tests requiring MPI were disabled and will 'nop'\n");
#endif

   printf("sizeof(ltc_mp_digit) = %d\n", (int)sizeof(ltc_mp_digit));

#ifdef LTC_PTHREAD
   tinfo = XCALLOC(sizeof(test_functions)/sizeof(test_functions[0]), sizeof(thread_info));
   if (tinfo == NULL) {
      printf("\n\nFAILURE: XCALLOC\n");
      return EXIT_FAILURE;
   }
#endif

   fn_len = 0;
   for (i = 0; i < sizeof(test_functions) / sizeof(test_functions[0]); ++i) {
      size_t len = strlen(test_functions[i].name);
      if (fn_len < len) fn_len = len;

#ifdef LTC_PTHREAD
      if(test_functions[i].fn == prng_test) continue;
      tinfo[i].t = &test_functions[i];
      x = pthread_create(&tinfo[i].thread_id, NULL, run, &tinfo[i]);
      if (x != 0)  {
         printf("\n\nFAILURE: pthread_create\n");
         return EXIT_FAILURE;
      }
#endif
   }

   fn_len = fn_len + (4 - (fn_len % 4));

   /* single test name from commandline */
   if (argc > 1) single_test = argv[1];

   dur = epoch_usec();
   for (i = 0; i < sizeof(test_functions)/sizeof(test_functions[0]); ++i) {
      if (single_test && strstr(test_functions[i].name, single_test) == NULL) {
        continue;
      }
      dots = fn_len - strlen(test_functions[i].name);

      printf("\n%s", test_functions[i].name);
      while(dots--) printf(".");
      fflush(stdout);

#ifdef LTC_PTHREAD
      if(test_functions[i].fn != prng_test) {
         x = pthread_join(tinfo[i].thread_id, (void**)&res);
         if (x != 0){
            printf("\n\nFAILURE: pthread_join\n");
            return EXIT_FAILURE;
         }
         x = res->err;
         delta = res->delta;
      }
      else {
         ts = epoch_usec();
         x = test_functions[i].fn();
         delta = (long)(epoch_usec() - ts);
      }
#else
      ts = epoch_usec();
      x = test_functions[i].fn();
      delta = (long)(epoch_usec() - ts);
#endif
      real += delta;

      if (x == CRYPT_OK) {
         printf("passed %10.3fms", (double)(delta)/1000);
         pass++;
      }
      else if (x == CRYPT_NOP) {
         printf("nop");
         nop++;
      }
      else {
         printf("failed (%s) %10.3fms", error_to_string(x), (double)(delta)/1000);
         fail++;
      }
   }
   dur = epoch_usec() - dur;

#ifdef LTC_PTHREAD
   XFREE(tinfo);
#endif

   x = (fail > 0 || fail+pass+nop == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
   printf("\n\n%s: passed=%d failed=%d nop=%d duration=%.1fsec real=%.1fsec\n", x ? "FAILURE" : "SUCCESS", pass, fail, nop, (double)(dur)/(1000*1000), (double)(real)/(1000*1000));
   return x;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
