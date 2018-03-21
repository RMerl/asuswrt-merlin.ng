#include <string.h>
#include <time.h>

#ifdef IOWNANATHLON
#include <unistd.h>
#define SLEEP sleep(4)
#else
#define SLEEP
#endif

/*
 * Configuration
 */
#ifndef LTM_DEMO_TEST_VS_MTEST
#define LTM_DEMO_TEST_VS_MTEST 1
#endif

#ifndef LTM_DEMO_TEST_REDUCE_2K_L
/* This test takes a moment so we disable it by default, but it can be:
 * 0 to disable testing
 * 1 to make the test with P = 2^1024 - 0x2A434 B9FDEC95 D8F9D550 FFFFFFFF FFFFFFFF
 * 2 to make the test with P = 2^2048 - 0x1 00000000 00000000 00000000 00000000 4945DDBF 8EA2A91D 5776399B B83E188F
 */
#define LTM_DEMO_TEST_REDUCE_2K_L 0
#endif

#ifdef LTM_DEMO_REAL_RAND
#define LTM_DEMO_RAND_SEED  time(NULL)
#else
#define LTM_DEMO_RAND_SEED  23
#endif

#include "tommath.h"

void ndraw(mp_int * a, char *name)
{
   char buf[16000];

   printf("%s: ", name);
   mp_toradix(a, buf, 10);
   printf("%s\n", buf);
   mp_toradix(a, buf, 16);
   printf("0x%s\n", buf);
}

#if LTM_DEMO_TEST_VS_MTEST
static void draw(mp_int * a)
{
   ndraw(a, "");
}
#endif


unsigned long lfsr = 0xAAAAAAAAUL;

int lbit(void)
{
   if (lfsr & 0x80000000UL) {
      lfsr = ((lfsr << 1) ^ 0x8000001BUL) & 0xFFFFFFFFUL;
      return 1;
   } else {
      lfsr <<= 1;
      return 0;
   }
}

#if defined(LTM_DEMO_REAL_RAND) && !defined(_WIN32)
static FILE* fd_urandom;
#endif
int myrng(unsigned char *dst, int len, void *dat)
{
   int x;
   (void)dat;
#if defined(LTM_DEMO_REAL_RAND)
   if (!fd_urandom) {
#if !defined(_WIN32)
      fprintf(stderr, "\nno /dev/urandom\n");
#endif
   }
   else {
      return fread(dst, 1, len, fd_urandom);
   }
#endif
   for (x = 0; x < len; ) {
      unsigned int r = (unsigned int)rand();
      do {
         dst[x++] = r & 0xFF;
         r >>= 8;
      } while((r != 0) && (x < len));
   }
   return len;
}

#if LTM_DEMO_TEST_VS_MTEST != 0
static void _panic(int l)
{
  fprintf(stderr, "\n%d: fgets failed\n", l);
  exit(EXIT_FAILURE);
}
#endif

mp_int a, b, c, d, e, f;

static void _cleanup(void)
{
  mp_clear_multi(&a, &b, &c, &d, &e, &f, NULL);
  printf("\n");

#ifdef LTM_DEMO_REAL_RAND
  if(fd_urandom)
     fclose(fd_urandom);
#endif
}
struct mp_sqrtmod_prime_st {
   unsigned long p;
   unsigned long n;
   mp_digit r;
};
struct mp_sqrtmod_prime_st sqrtmod_prime[] = {
      { 5, 14, 3 },
      { 7, 9, 4 },
      { 113, 2, 62 }
};
struct mp_jacobi_st {
   unsigned long n;
   int c[16];
};
struct mp_jacobi_st jacobi[] = {
      { 3, {  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1, -1,  0,  1 } },
      { 5, {  0,  1, -1, -1,  1,  0,  1, -1, -1,  1,  0,  1, -1, -1,  1,  0 } },
      { 7, {  1, -1,  1, -1, -1,  0,  1,  1, -1,  1, -1, -1,  0,  1,  1, -1 } },
      { 9, { -1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1,  1,  0,  1 } },
};

char cmd[4096], buf[4096];
int main(void)
{
   unsigned rr;
   int cnt, ix;
#if LTM_DEMO_TEST_VS_MTEST
   unsigned long expt_n, add_n, sub_n, mul_n, div_n, sqr_n, mul2d_n, div2d_n,
      gcd_n, lcm_n, inv_n, div2_n, mul2_n, add_d_n, sub_d_n;
   char* ret;
#else
   unsigned long s, t;
   unsigned long long q, r;
   mp_digit mp;
   int i, n, err, should;
#endif

   if (mp_init_multi(&a, &b, &c, &d, &e, &f, NULL)!= MP_OKAY)
     return EXIT_FAILURE;

   atexit(_cleanup);

#if defined(LTM_DEMO_REAL_RAND)
   if (!fd_urandom) {
      fd_urandom = fopen("/dev/urandom", "r");
      if (!fd_urandom) {
#if !defined(_WIN32)
         fprintf(stderr, "\ncould not open /dev/urandom\n");
#endif
      }
   }
#endif
   srand(LTM_DEMO_RAND_SEED);

#ifdef MP_8BIT
   printf("Digit size 8 Bit \n");
#endif
#ifdef MP_16BIT
   printf("Digit size 16 Bit \n");
#endif
#ifdef MP_32BIT
   printf("Digit size 32 Bit \n");
#endif
#ifdef MP_64BIT
   printf("Digit size 64 Bit \n");
#endif
   printf("Size of mp_digit: %u\n", (unsigned int)sizeof(mp_digit));
   printf("Size of mp_word: %u\n", (unsigned int)sizeof(mp_word));
   printf("DIGIT_BIT: %d\n", DIGIT_BIT);
   printf("MP_PREC: %d\n", MP_PREC);

#if LTM_DEMO_TEST_VS_MTEST == 0
   // trivial stuff
   // a: 0->5
   mp_set_int(&a, 5);
   // a: 5-> b: -5
   mp_neg(&a, &b);
   if (mp_cmp(&a, &b) != MP_GT) {
      return EXIT_FAILURE;
   }
   if (mp_cmp(&b, &a) != MP_LT) {
      return EXIT_FAILURE;
   }
   // a: 5-> a: -5
   mp_neg(&a, &a);
   if (mp_cmp(&b, &a) != MP_EQ) {
      return EXIT_FAILURE;
   }
   // a: -5-> b: 5
   mp_abs(&a, &b);
   if (mp_isneg(&b) != MP_NO) {
      return EXIT_FAILURE;
   }
   // a: -5-> b: -4
   mp_add_d(&a, 1, &b);
   if (mp_isneg(&b) != MP_YES) {
      return EXIT_FAILURE;
   }
   if (mp_get_int(&b) != 4) {
      return EXIT_FAILURE;
   }
   // a: -5-> b: 1
   mp_add_d(&a, 6, &b);
   if (mp_get_int(&b) != 1) {
      return EXIT_FAILURE;
   }
   // a: -5-> a: 1
   mp_add_d(&a, 6, &a);
   if (mp_get_int(&a) != 1) {
      return EXIT_FAILURE;
   }
   mp_zero(&a);
   // a: 0-> a: 6
   mp_add_d(&a, 6, &a);
   if (mp_get_int(&a) != 6) {
      return EXIT_FAILURE;
   }


   mp_set_int(&a, 0);
   mp_set_int(&b, 1);
   if ((err = mp_jacobi(&a, &b, &i)) != MP_OKAY) {
      printf("Failed executing mp_jacobi(0 | 1) %s.\n", mp_error_to_string(err));
      return EXIT_FAILURE;
   }
   if (i != 1) {
      printf("Failed trivial mp_jacobi(0 | 1) %d != 1\n", i);
      return EXIT_FAILURE;
   }
   for (cnt = 0; cnt < (int)(sizeof(jacobi)/sizeof(jacobi[0])); ++cnt) {
      mp_set_int(&b, jacobi[cnt].n);
      /* only test positive values of a */
      for (n = -5; n <= 10; ++n) {
         mp_set_int(&a, abs(n));
         should = MP_OKAY;
         if (n < 0) {
            mp_neg(&a, &a);
            /* Until #44 is fixed the negative a's must fail */
            should = MP_VAL;
         }
         if ((err = mp_jacobi(&a, &b, &i)) != should) {
            printf("Failed executing mp_jacobi(%d | %lu) %s.\n", n, jacobi[cnt].n, mp_error_to_string(err));
            return EXIT_FAILURE;
         }
         if (err == MP_OKAY && i != jacobi[cnt].c[n + 5]) {
            printf("Failed trivial mp_jacobi(%d | %lu) %d != %d\n", n, jacobi[cnt].n, i, jacobi[cnt].c[n + 5]);
            return EXIT_FAILURE;
         }
      }
   }

   // test mp_get_int
   printf("\n\nTesting: mp_get_int");
   for (i = 0; i < 1000; ++i) {
      t = ((unsigned long) rand () * rand () + 1) & 0xFFFFFFFF;
      mp_set_int (&a, t);
      if (t != mp_get_int (&a)) {
         printf ("\nmp_get_int() bad result!");
         return EXIT_FAILURE;
      }
   }
   mp_set_int(&a, 0);
   if (mp_get_int(&a) != 0) {
      printf("\nmp_get_int() bad result!");
      return EXIT_FAILURE;
   }
   mp_set_int(&a, 0xffffffff);
   if (mp_get_int(&a) != 0xffffffff) {
      printf("\nmp_get_int() bad result!");
      return EXIT_FAILURE;
   }

   printf("\n\nTesting: mp_get_long\n");
   for (i = 0; i < (int)(sizeof(unsigned long)*CHAR_BIT) - 1; ++i) {
      t = (1ULL << (i+1)) - 1;
      if (!t)
         t = -1;
      printf(" t = 0x%lx i = %d\r", t, i);
      do {
         if (mp_set_long(&a, t) != MP_OKAY) {
            printf("\nmp_set_long() error!");
            return EXIT_FAILURE;
         }
         s = mp_get_long(&a);
         if (s != t) {
            printf("\nmp_get_long() bad result! 0x%lx != 0x%lx", s, t);
            return EXIT_FAILURE;
         }
         t <<= 1;
      } while(t);
   }

   printf("\n\nTesting: mp_get_long_long\n");
   for (i = 0; i < (int)(sizeof(unsigned long long)*CHAR_BIT) - 1; ++i) {
      r = (1ULL << (i+1)) - 1;
      if (!r)
         r = -1;
      printf(" r = 0x%llx i = %d\r", r, i);
      do {
         if (mp_set_long_long(&a, r) != MP_OKAY) {
            printf("\nmp_set_long_long() error!");
            return EXIT_FAILURE;
         }
         q = mp_get_long_long(&a);
         if (q != r) {
            printf("\nmp_get_long_long() bad result! 0x%llx != 0x%llx", q, r);
            return EXIT_FAILURE;
         }
         r <<= 1;
      } while(r);
   }

   // test mp_sqrt
   printf("\n\nTesting: mp_sqrt\n");
   for (i = 0; i < 1000; ++i) {
      printf ("%6d\r", i);
      fflush (stdout);
      n = (rand () & 15) + 1;
      mp_rand (&a, n);
      if (mp_sqrt (&a, &b) != MP_OKAY) {
         printf ("\nmp_sqrt() error!");
         return EXIT_FAILURE;
      }
      mp_n_root_ex (&a, 2, &c, 0);
      mp_n_root_ex (&a, 2, &d, 1);
      if (mp_cmp_mag (&c, &d) != MP_EQ) {
         printf ("\nmp_n_root_ex() bad result!");
         return EXIT_FAILURE;
      }
      if (mp_cmp_mag (&b, &c) != MP_EQ) {
         printf ("mp_sqrt() bad result!\n");
         return EXIT_FAILURE;
      }
   }

   printf("\n\nTesting: mp_is_square\n");
   for (i = 0; i < 1000; ++i) {
      printf ("%6d\r", i);
      fflush (stdout);

      /* test mp_is_square false negatives */
      n = (rand () & 7) + 1;
      mp_rand (&a, n);
      mp_sqr (&a, &a);
      if (mp_is_square (&a, &n) != MP_OKAY) {
         printf ("\nfn:mp_is_square() error!");
         return EXIT_FAILURE;
      }
      if (n == 0) {
         printf ("\nfn:mp_is_square() bad result!");
         return EXIT_FAILURE;
      }

      /* test for false positives */
      mp_add_d (&a, 1, &a);
      if (mp_is_square (&a, &n) != MP_OKAY) {
         printf ("\nfp:mp_is_square() error!");
         return EXIT_FAILURE;
      }
      if (n == 1) {
         printf ("\nfp:mp_is_square() bad result!");
         return EXIT_FAILURE;
      }

   }
   printf("\n\n");

   // r^2 = n (mod p)
   for (i = 0; i < (int)(sizeof(sqrtmod_prime)/sizeof(sqrtmod_prime[0])); ++i) {
      mp_set_int(&a, sqrtmod_prime[i].p);
      mp_set_int(&b, sqrtmod_prime[i].n);
      if (mp_sqrtmod_prime(&b, &a, &c) != MP_OKAY) {
         printf("Failed executing %d. mp_sqrtmod_prime\n", (i+1));
         return EXIT_FAILURE;
      }
      if (mp_cmp_d(&c, sqrtmod_prime[i].r) != MP_EQ) {
         printf("Failed %d. trivial mp_sqrtmod_prime\n", (i+1));
         ndraw(&c, "r");
         return EXIT_FAILURE;
      }
   }

   /* test for size */
   for (ix = 10; ix < 128; ix++) {
      printf ("Testing (not safe-prime): %9d bits    \r", ix);
      fflush (stdout);
      err = mp_prime_random_ex (&a, 8, ix,
                                (rand () & 1) ? 0 : LTM_PRIME_2MSB_ON, myrng,
                                NULL);
      if (err != MP_OKAY) {
         printf ("failed with err code %d\n", err);
         return EXIT_FAILURE;
      }
      if (mp_count_bits (&a) != ix) {
         printf ("Prime is %d not %d bits!!!\n", mp_count_bits (&a), ix);
         return EXIT_FAILURE;
      }
   }
   printf("\n");

   for (ix = 16; ix < 128; ix++) {
      printf ("Testing (    safe-prime): %9d bits    \r", ix);
      fflush (stdout);
      err = mp_prime_random_ex (
            &a, 8, ix, ((rand () & 1) ? 0 : LTM_PRIME_2MSB_ON) | LTM_PRIME_SAFE,
            myrng, NULL);
      if (err != MP_OKAY) {
         printf ("failed with err code %d\n", err);
         return EXIT_FAILURE;
      }
      if (mp_count_bits (&a) != ix) {
         printf ("Prime is %d not %d bits!!!\n", mp_count_bits (&a), ix);
         return EXIT_FAILURE;
      }
      /* let's see if it's really a safe prime */
      mp_sub_d (&a, 1, &a);
      mp_div_2 (&a, &a);
      mp_prime_is_prime (&a, 8, &cnt);
      if (cnt != MP_YES) {
         printf ("sub is not prime!\n");
         return EXIT_FAILURE;
      }
   }

   printf("\n\n");

   // test montgomery
   printf("Testing: montgomery...\n");
   for (i = 1; i <= 10; i++) {
      if (i == 10)
         i = 1000;
      printf(" digit size: %2d\r", i);
      fflush(stdout);
      for (n = 0; n < 1000; n++) {
         mp_rand(&a, i);
         a.dp[0] |= 1;

         // let's see if R is right
         mp_montgomery_calc_normalization(&b, &a);
         mp_montgomery_setup(&a, &mp);

         // now test a random reduction
         for (ix = 0; ix < 100; ix++) {
             mp_rand(&c, 1 + abs(rand()) % (2*i));
             mp_copy(&c, &d);
             mp_copy(&c, &e);

             mp_mod(&d, &a, &d);
             mp_montgomery_reduce(&c, &a, mp);
             mp_mulmod(&c, &b, &a, &c);

             if (mp_cmp(&c, &d) != MP_EQ) {
printf("d = e mod a, c = e MOD a\n");
mp_todecimal(&a, buf); printf("a = %s\n", buf);
mp_todecimal(&e, buf); printf("e = %s\n", buf);
mp_todecimal(&d, buf); printf("d = %s\n", buf);
mp_todecimal(&c, buf); printf("c = %s\n", buf);
printf("compare no compare!\n"); return EXIT_FAILURE; }
             /* only one big montgomery reduction */
             if (i > 10)
             {
                n = 1000;
                ix = 100;
             }
         }
      }
   }

   printf("\n\n");

   mp_read_radix(&a, "123456", 10);
   mp_toradix_n(&a, buf, 10, 3);
   printf("a == %s\n", buf);
   mp_toradix_n(&a, buf, 10, 4);
   printf("a == %s\n", buf);
   mp_toradix_n(&a, buf, 10, 30);
   printf("a == %s\n", buf);


#if 0
   for (;;) {
      fgets(buf, sizeof(buf), stdin);
      mp_read_radix(&a, buf, 10);
      mp_prime_next_prime(&a, 5, 1);
      mp_toradix(&a, buf, 10);
      printf("%s, %lu\n", buf, a.dp[0] & 3);
   }
#endif

   /* test mp_cnt_lsb */
   printf("\n\nTesting: mp_cnt_lsb");
   mp_set(&a, 1);
   for (ix = 0; ix < 1024; ix++) {
      if (mp_cnt_lsb (&a) != ix) {
         printf ("Failed at %d, %d\n", ix, mp_cnt_lsb (&a));
         return EXIT_FAILURE;
      }
      mp_mul_2 (&a, &a);
   }

/* test mp_reduce_2k */
   printf("\n\nTesting: mp_reduce_2k\n");
   for (cnt = 3; cnt <= 128; ++cnt) {
      mp_digit tmp;

      mp_2expt (&a, cnt);
      mp_sub_d (&a, 2, &a); /* a = 2**cnt - 2 */

      printf ("\r %4d bits", cnt);
      printf ("(%d)", mp_reduce_is_2k (&a));
      mp_reduce_2k_setup (&a, &tmp);
      printf ("(%lu)", (unsigned long) tmp);
      for (ix = 0; ix < 1000; ix++) {
         if (!(ix & 127)) {
            printf (".");
            fflush (stdout);
         }
         mp_rand (&b, (cnt / DIGIT_BIT + 1) * 2);
         mp_copy (&c, &b);
         mp_mod (&c, &a, &c);
         mp_reduce_2k (&b, &a, 2);
         if (mp_cmp (&c, &b)) {
            printf ("FAILED\n");
            return EXIT_FAILURE;
         }
      }
   }

/* test mp_div_3  */
   printf("\n\nTesting: mp_div_3...\n");
   mp_set(&d, 3);
   for (cnt = 0; cnt < 10000;) {
      mp_digit r2;

      if (!(++cnt & 127))
      {
        printf("%9d\r", cnt);
        fflush(stdout);
      }
      mp_rand(&a, abs(rand()) % 128 + 1);
      mp_div(&a, &d, &b, &e);
      mp_div_3(&a, &c, &r2);

      if (mp_cmp(&b, &c) || mp_cmp_d(&e, r2)) {
	 printf("\nmp_div_3 => Failure\n");
      }
   }
   printf("\nPassed div_3 testing");

/* test the DR reduction */
   printf("\n\nTesting: mp_dr_reduce...\n");
   for (cnt = 2; cnt < 32; cnt++) {
      printf ("\r%d digit modulus", cnt);
      mp_grow (&a, cnt);
      mp_zero (&a);
      for (ix = 1; ix < cnt; ix++) {
         a.dp[ix] = MP_MASK;
      }
      a.used = cnt;
      a.dp[0] = 3;

      mp_rand (&b, cnt - 1);
      mp_copy (&b, &c);

      rr = 0;
      do {
         if (!(rr & 127)) {
            printf (".");
            fflush (stdout);
         }
         mp_sqr (&b, &b);
         mp_add_d (&b, 1, &b);
         mp_copy (&b, &c);

         mp_mod (&b, &a, &b);
         mp_dr_setup(&a, &mp),
         mp_dr_reduce (&c, &a, mp);

         if (mp_cmp (&b, &c) != MP_EQ) {
            printf ("Failed on trial %u\n", rr);
            return EXIT_FAILURE;
         }
      } while (++rr < 500);
      printf (" passed");
      fflush (stdout);
   }

#if LTM_DEMO_TEST_REDUCE_2K_L
/* test the mp_reduce_2k_l code */
#if LTM_DEMO_TEST_REDUCE_2K_L == 1
/* first load P with 2^1024 - 0x2A434 B9FDEC95 D8F9D550 FFFFFFFF FFFFFFFF */
   mp_2expt(&a, 1024);
   mp_read_radix(&b, "2A434B9FDEC95D8F9D550FFFFFFFFFFFFFFFF", 16);
   mp_sub(&a, &b, &a);
#elif LTM_DEMO_TEST_REDUCE_2K_L == 2
/*  p = 2^2048 - 0x1 00000000 00000000 00000000 00000000 4945DDBF 8EA2A91D 5776399B B83E188F  */
   mp_2expt(&a, 2048);
   mp_read_radix(&b,
		 "1000000000000000000000000000000004945DDBF8EA2A91D5776399BB83E188F",
		 16);
   mp_sub(&a, &b, &a);
#else
#error oops
#endif

   mp_todecimal(&a, buf);
   printf("\n\np==%s\n", buf);
/* now mp_reduce_is_2k_l() should return */
   if (mp_reduce_is_2k_l(&a) != 1) {
      printf("mp_reduce_is_2k_l() return 0, should be 1\n");
      return EXIT_FAILURE;
   }
   mp_reduce_2k_setup_l(&a, &d);
   /* now do a million square+1 to see if it varies */
   mp_rand(&b, 64);
   mp_mod(&b, &a, &b);
   mp_copy(&b, &c);
   printf("Testing: mp_reduce_2k_l...");
   fflush(stdout);
   for (cnt = 0; cnt < (int)(1UL << 20); cnt++) {
      mp_sqr(&b, &b);
      mp_add_d(&b, 1, &b);
      mp_reduce_2k_l(&b, &a, &d);
      mp_sqr(&c, &c);
      mp_add_d(&c, 1, &c);
      mp_mod(&c, &a, &c);
      if (mp_cmp(&b, &c) != MP_EQ) {
	 printf("mp_reduce_2k_l() failed at step %d\n", cnt);
	 mp_tohex(&b, buf);
	 printf("b == %s\n", buf);
	 mp_tohex(&c, buf);
	 printf("c == %s\n", buf);
	 return EXIT_FAILURE;
      }
   }
   printf("...Passed\n");
#endif /* LTM_DEMO_TEST_REDUCE_2K_L */

#else

   div2_n = mul2_n = inv_n = expt_n = lcm_n = gcd_n = add_n =
      sub_n = mul_n = div_n = sqr_n = mul2d_n = div2d_n = cnt = add_d_n =
      sub_d_n = 0;

   /* force KARA and TOOM to enable despite cutoffs */
   KARATSUBA_SQR_CUTOFF = KARATSUBA_MUL_CUTOFF = 8;
   TOOM_SQR_CUTOFF = TOOM_MUL_CUTOFF = 16;

   for (;;) {
      /* randomly clear and re-init one variable, this has the affect of triming the alloc space */
      switch (abs(rand()) % 7) {
      case 0:
	 mp_clear(&a);
	 mp_init(&a);
	 break;
      case 1:
	 mp_clear(&b);
	 mp_init(&b);
	 break;
      case 2:
	 mp_clear(&c);
	 mp_init(&c);
	 break;
      case 3:
	 mp_clear(&d);
	 mp_init(&d);
	 break;
      case 4:
	 mp_clear(&e);
	 mp_init(&e);
	 break;
      case 5:
	 mp_clear(&f);
	 mp_init(&f);
	 break;
      case 6:
	 break;			/* don't clear any */
      }


      printf
	 ("%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu/%4lu ",
	  add_n, sub_n, mul_n, div_n, sqr_n, mul2d_n, div2d_n, gcd_n, lcm_n,
	  expt_n, inv_n, div2_n, mul2_n, add_d_n, sub_d_n);
      ret=fgets(cmd, 4095, stdin); if(!ret){_panic(__LINE__);}
      cmd[strlen(cmd) - 1] = 0;
      printf("%-6s ]\r", cmd);
      fflush(stdout);
      if (!strcmp(cmd, "mul2d")) {
	 ++mul2d_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 sscanf(buf, "%d", &rr);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);

	 mp_mul_2d(&a, rr, &a);
	 a.sign = b.sign;
	 if (mp_cmp(&a, &b) != MP_EQ) {
	    printf("mul2d failed, rr == %d\n", rr);
	    draw(&a);
	    draw(&b);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "div2d")) {
	 ++div2d_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 sscanf(buf, "%d", &rr);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);

	 mp_div_2d(&a, rr, &a, &e);
	 a.sign = b.sign;
	 if (a.used == b.used && a.used == 0) {
	    a.sign = b.sign = MP_ZPOS;
	 }
	 if (mp_cmp(&a, &b) != MP_EQ) {
	    printf("div2d failed, rr == %d\n", rr);
	    draw(&a);
	    draw(&b);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "add")) {
	 ++add_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 mp_copy(&a, &d);
	 mp_add(&d, &b, &d);
	 if (mp_cmp(&c, &d) != MP_EQ) {
	    printf("add %lu failure!\n", add_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }

	 /* test the sign/unsigned storage functions */

	 rr = mp_signed_bin_size(&c);
	 mp_to_signed_bin(&c, (unsigned char *) cmd);
	 memset(cmd + rr, rand() & 255, sizeof(cmd) - rr);
	 mp_read_signed_bin(&d, (unsigned char *) cmd, rr);
	 if (mp_cmp(&c, &d) != MP_EQ) {
	    printf("mp_signed_bin failure!\n");
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }


	 rr = mp_unsigned_bin_size(&c);
	 mp_to_unsigned_bin(&c, (unsigned char *) cmd);
	 memset(cmd + rr, rand() & 255, sizeof(cmd) - rr);
	 mp_read_unsigned_bin(&d, (unsigned char *) cmd, rr);
	 if (mp_cmp_mag(&c, &d) != MP_EQ) {
	    printf("mp_unsigned_bin failure!\n");
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }

      } else if (!strcmp(cmd, "sub")) {
	 ++sub_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 mp_copy(&a, &d);
	 mp_sub(&d, &b, &d);
	 if (mp_cmp(&c, &d) != MP_EQ) {
	    printf("sub %lu failure!\n", sub_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "mul")) {
	 ++mul_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 mp_copy(&a, &d);
	 mp_mul(&d, &b, &d);
	 if (mp_cmp(&c, &d) != MP_EQ) {
	    printf("mul %lu failure!\n", mul_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "div")) {
	 ++div_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&d, buf, 64);

	 mp_div(&a, &b, &e, &f);
	 if (mp_cmp(&c, &e) != MP_EQ || mp_cmp(&d, &f) != MP_EQ) {
	    printf("div %lu %d, %d, failure!\n", div_n, mp_cmp(&c, &e),
		   mp_cmp(&d, &f));
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    draw(&e);
	    draw(&f);
	    return EXIT_FAILURE;
	 }

      } else if (!strcmp(cmd, "sqr")) {
	 ++sqr_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 mp_copy(&a, &c);
	 mp_sqr(&c, &c);
	 if (mp_cmp(&b, &c) != MP_EQ) {
	    printf("sqr %lu failure!\n", sqr_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "gcd")) {
	 ++gcd_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 mp_copy(&a, &d);
	 mp_gcd(&d, &b, &d);
	 d.sign = c.sign;
	 if (mp_cmp(&c, &d) != MP_EQ) {
	    printf("gcd %lu failure!\n", gcd_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "lcm")) {
	 ++lcm_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 mp_copy(&a, &d);
	 mp_lcm(&d, &b, &d);
	 d.sign = c.sign;
	 if (mp_cmp(&c, &d) != MP_EQ) {
	    printf("lcm %lu failure!\n", lcm_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "expt")) {
	 ++expt_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&d, buf, 64);
	 mp_copy(&a, &e);
	 mp_exptmod(&e, &b, &c, &e);
	 if (mp_cmp(&d, &e) != MP_EQ) {
	    printf("expt %lu failure!\n", expt_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    draw(&e);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "invmod")) {
	 ++inv_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&c, buf, 64);
	 mp_invmod(&a, &b, &d);
	 mp_mulmod(&d, &a, &b, &e);
	 if (mp_cmp_d(&e, 1) != MP_EQ) {
	    printf("inv [wrong value from MPI?!] failure\n");
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    draw(&d);
	    draw(&e);
	    mp_gcd(&a, &b, &e);
	    draw(&e);
	    return EXIT_FAILURE;
	 }

      } else if (!strcmp(cmd, "div2")) {
	 ++div2_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 mp_div_2(&a, &c);
	 if (mp_cmp(&c, &b) != MP_EQ) {
	    printf("div_2 %lu failure\n", div2_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "mul2")) {
	 ++mul2_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 mp_mul_2(&a, &c);
	 if (mp_cmp(&c, &b) != MP_EQ) {
	    printf("mul_2 %lu failure\n", mul2_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "add_d")) {
	 ++add_d_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 sscanf(buf, "%d", &ix);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 mp_add_d(&a, ix, &c);
	 if (mp_cmp(&b, &c) != MP_EQ) {
	    printf("add_d %lu failure\n", add_d_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    printf("d == %d\n", ix);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "sub_d")) {
	 ++sub_d_n;
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&a, buf, 64);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 sscanf(buf, "%d", &ix);
	 ret=fgets(buf, 4095, stdin); if(!ret){_panic(__LINE__);}
	 mp_read_radix(&b, buf, 64);
	 mp_sub_d(&a, ix, &c);
	 if (mp_cmp(&b, &c) != MP_EQ) {
	    printf("sub_d %lu failure\n", sub_d_n);
	    draw(&a);
	    draw(&b);
	    draw(&c);
	    printf("d == %d\n", ix);
	    return EXIT_FAILURE;
	 }
      } else if (!strcmp(cmd, "exit")) {
         printf("\nokay, exiting now\n");
         break;
      }
   }
#endif
   return 0;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
