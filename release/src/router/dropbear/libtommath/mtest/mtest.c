/* makes a bignum test harness with NUM tests per operation
 *
 * the output is made in the following format [one parameter per line]

operation
operand1
operand2
[... operandN]
result1
result2
[... resultN]

So for example "a * b mod n" would be

mulmod
a
b
n
a*b mod n

e.g. if a=3, b=4 n=11 then

mulmod
3
4
11
1

 */

#ifdef MP_8BIT
#define THE_MASK 127
#else
#define THE_MASK 32767
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.c"

#ifdef LTM_MTEST_REAL_RAND
#define getRandChar() fgetc(rng)
FILE *rng;
#else
#define getRandChar() (rand()&0xFF)
#endif

void rand_num(mp_int *a)
{
   int size;
   unsigned char buf[2048];
   size_t sz;

   size = 1 + ((getRandChar()<<8) + getRandChar()) % 101;
   buf[0] = (getRandChar()&1)?1:0;
#ifdef LTM_MTEST_REAL_RAND
   sz = fread(buf+1, 1, size, rng);
#else
   sz = 1;
   while (sz < (unsigned)size) {
       buf[sz] = getRandChar();
       ++sz;
   }
#endif
   if (sz != (unsigned)size) {
       fprintf(stderr, "\nWarning: fread failed\n\n");
   }
   while (buf[1] == 0) buf[1] = getRandChar();
   mp_read_raw(a, buf, 1+size);
}

void rand_num2(mp_int *a)
{
   int size;
   unsigned char buf[2048];
   size_t sz;

   size = 10 + ((getRandChar()<<8) + getRandChar()) % 101;
   buf[0] = (getRandChar()&1)?1:0;
#ifdef LTM_MTEST_REAL_RAND
   sz = fread(buf+1, 1, size, rng);
#else
   sz = 1;
   while (sz < (unsigned)size) {
       buf[sz] = getRandChar();
       ++sz;
   }
#endif
   if (sz != (unsigned)size) {
       fprintf(stderr, "\nWarning: fread failed\n\n");
   }
   while (buf[1] == 0) buf[1] = getRandChar();
   mp_read_raw(a, buf, 1+size);
}

#define mp_to64(a, b) mp_toradix(a, b, 64)

int main(int argc, char *argv[])
{
   int n, tmp;
   long long max;
   mp_int a, b, c, d, e;
#ifdef MTEST_NO_FULLSPEED
   clock_t t1;
#endif
   char buf[4096];

   mp_init(&a);
   mp_init(&b);
   mp_init(&c);
   mp_init(&d);
   mp_init(&e);

   if (argc > 1) {
       max = strtol(argv[1], NULL, 0);
       if (max < 0) {
           if (max > -64) {
               max = (1 << -(max)) + 1;
           } else {
               max = 1;
           }
       } else if (max == 0) {
           max = 1;
       }
   }
   else {
       max = 0;
   }


   /* initial (2^n - 1)^2 testing, makes sure the comba multiplier works [it has the new carry code] */
/*
   mp_set(&a, 1);
   for (n = 1; n < 8192; n++) {
       mp_mul(&a, &a, &c);
       printf("mul\n");
       mp_to64(&a, buf);
       printf("%s\n%s\n", buf, buf);
       mp_to64(&c, buf);
       printf("%s\n", buf);

       mp_add_d(&a, 1, &a);
       mp_mul_2(&a, &a);
       mp_sub_d(&a, 1, &a);
   }
*/

#ifdef LTM_MTEST_REAL_RAND
   rng = fopen("/dev/urandom", "rb");
   if (rng == NULL) {
      rng = fopen("/dev/random", "rb");
      if (rng == NULL) {
         fprintf(stderr, "\nWarning:  stdin used as random source\n\n");
         rng = stdin;
      }
   }
#else
   srand(23);
#endif

#ifdef MTEST_NO_FULLSPEED
   t1 = clock();
#endif
   for (;;) {
#ifdef MTEST_NO_FULLSPEED
      if (clock() - t1 > CLOCKS_PER_SEC) {
         sleep(2);
         t1 = clock();
      }
#endif
       n = getRandChar() % 15;

       if (max != 0) {
           --max;
           if (max == 0)
             n = 255;
       }

   if (n == 0) {
       /* add tests */
       rand_num(&a);
       rand_num(&b);
       mp_add(&a, &b, &c);
       printf("add\n");
       mp_to64(&a, buf);
       printf("%s\n", buf);
       mp_to64(&b, buf);
       printf("%s\n", buf);
       mp_to64(&c, buf);
       printf("%s\n", buf);
   } else if (n == 1) {
      /* sub tests */
       rand_num(&a);
       rand_num(&b);
       mp_sub(&a, &b, &c);
       printf("sub\n");
       mp_to64(&a, buf);
       printf("%s\n", buf);
       mp_to64(&b, buf);
       printf("%s\n", buf);
       mp_to64(&c, buf);
       printf("%s\n", buf);
   } else if (n == 2) {
       /* mul tests */
       rand_num(&a);
       rand_num(&b);
       mp_mul(&a, &b, &c);
       printf("mul\n");
       mp_to64(&a, buf);
       printf("%s\n", buf);
       mp_to64(&b, buf);
       printf("%s\n", buf);
       mp_to64(&c, buf);
       printf("%s\n", buf);
   } else if (n == 3) {
      /* div tests */
       rand_num(&a);
       rand_num(&b);
       mp_div(&a, &b, &c, &d);
       printf("div\n");
       mp_to64(&a, buf);
       printf("%s\n", buf);
       mp_to64(&b, buf);
       printf("%s\n", buf);
       mp_to64(&c, buf);
       printf("%s\n", buf);
       mp_to64(&d, buf);
       printf("%s\n", buf);
   } else if (n == 4) {
      /* sqr tests */
       rand_num(&a);
       mp_sqr(&a, &b);
       printf("sqr\n");
       mp_to64(&a, buf);
       printf("%s\n", buf);
       mp_to64(&b, buf);
       printf("%s\n", buf);
   } else if (n == 5) {
      /* mul_2d test */
      rand_num(&a);
      mp_copy(&a, &b);
      n = getRandChar() & 63;
      mp_mul_2d(&b, n, &b);
      mp_to64(&a, buf);
      printf("mul2d\n");
      printf("%s\n", buf);
      printf("%d\n", n);
      mp_to64(&b, buf);
      printf("%s\n", buf);
   } else if (n == 6) {
      /* div_2d test */
      rand_num(&a);
      mp_copy(&a, &b);
      n = getRandChar() & 63;
      mp_div_2d(&b, n, &b, NULL);
      mp_to64(&a, buf);
      printf("div2d\n");
      printf("%s\n", buf);
      printf("%d\n", n);
      mp_to64(&b, buf);
      printf("%s\n", buf);
   } else if (n == 7) {
      /* gcd test */
      rand_num(&a);
      rand_num(&b);
      a.sign = MP_ZPOS;
      b.sign = MP_ZPOS;
      mp_gcd(&a, &b, &c);
      printf("gcd\n");
      mp_to64(&a, buf);
      printf("%s\n", buf);
      mp_to64(&b, buf);
      printf("%s\n", buf);
      mp_to64(&c, buf);
      printf("%s\n", buf);
   } else if (n == 8) {
      /* lcm test */
      rand_num(&a);
      rand_num(&b);
      a.sign = MP_ZPOS;
      b.sign = MP_ZPOS;
      mp_lcm(&a, &b, &c);
      printf("lcm\n");
      mp_to64(&a, buf);
      printf("%s\n", buf);
      mp_to64(&b, buf);
      printf("%s\n", buf);
      mp_to64(&c, buf);
      printf("%s\n", buf);
   } else if (n == 9) {
      /* exptmod test */
      rand_num2(&a);
      rand_num2(&b);
      rand_num2(&c);
//      if (c.dp[0]&1) mp_add_d(&c, 1, &c);
      a.sign = b.sign = c.sign = 0;
      mp_exptmod(&a, &b, &c, &d);
      printf("expt\n");
      mp_to64(&a, buf);
      printf("%s\n", buf);
      mp_to64(&b, buf);
      printf("%s\n", buf);
      mp_to64(&c, buf);
      printf("%s\n", buf);
      mp_to64(&d, buf);
      printf("%s\n", buf);
   } else if (n == 10) {
      /* invmod test */
      do {
      rand_num2(&a);
      rand_num2(&b);
      b.sign = MP_ZPOS;
      a.sign = MP_ZPOS;
      mp_gcd(&a, &b, &c);
      } while (mp_cmp_d(&c, 1) != 0 || mp_cmp_d(&b, 1) == 0);
      mp_invmod(&a, &b, &c);
      printf("invmod\n");
      mp_to64(&a, buf);
      printf("%s\n", buf);
      mp_to64(&b, buf);
      printf("%s\n", buf);
      mp_to64(&c, buf);
      printf("%s\n", buf);
   } else if (n == 11) {
      rand_num(&a);
      mp_mul_2(&a, &a);
      mp_div_2(&a, &b);
      printf("div2\n");
      mp_to64(&a, buf);
      printf("%s\n", buf);
      mp_to64(&b, buf);
      printf("%s\n", buf);
   } else if (n == 12) {
      rand_num2(&a);
      mp_mul_2(&a, &b);
      printf("mul2\n");
      mp_to64(&a, buf);
      printf("%s\n", buf);
      mp_to64(&b, buf);
      printf("%s\n", buf);
   } else if (n == 13) {
      rand_num2(&a);
      tmp = abs(rand()) & THE_MASK;
      mp_add_d(&a, tmp, &b);
      printf("add_d\n");
      mp_to64(&a, buf);
      printf("%s\n%d\n", buf, tmp);
      mp_to64(&b, buf);
      printf("%s\n", buf);
   } else if (n == 14) {
      rand_num2(&a);
      tmp = abs(rand()) & THE_MASK;
      mp_sub_d(&a, tmp, &b);
      printf("sub_d\n");
      mp_to64(&a, buf);
      printf("%s\n%d\n", buf, tmp);
      mp_to64(&b, buf);
      printf("%s\n", buf);
   } else if (n == 255) {
      printf("exit\n");
      break;
   }

   }
#ifdef LTM_MTEST_REAL_RAND
   fclose(rng);
#endif
   return 0;
}

/* $Source$ */
/* $Revision$ */
/* $Date$ */
