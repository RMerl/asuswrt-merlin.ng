/* Tune the Karatsuba parameters
 *
 * Tom St Denis, tstdenis82@gmail.com
 */
#include <tommath.h>
#include <time.h>
#include <stdint.h>

/* how many times todo each size mult.  Depends on your computer.  For slow computers
 * this can be low like 5 or 10.  For fast [re: Athlon] should be 25 - 50 or so
 */
#define TIMES (1UL<<14UL)

#ifndef X86_TIMER

/* RDTSC from Scott Duplichan */
static uint64_t TIMFUNC (void)
   {
   #if defined __GNUC__
      #if defined(__i386__) || defined(__x86_64__)
        /* version from http://www.mcs.anl.gov/~kazutomo/rdtsc.html
         * the old code always got a warning issued by gcc, clang did not complain...
         */
        unsigned hi, lo;
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)lo)|( ((uint64_t)hi)<<32);
      #else /* gcc-IA64 version */
         unsigned long result;
         __asm__ __volatile__("mov %0=ar.itc" : "=r"(result) :: "memory");
         while (__builtin_expect ((int) result == -1, 0))
         __asm__ __volatile__("mov %0=ar.itc" : "=r"(result) :: "memory");
         return result;
      #endif

   // Microsoft and Intel Windows compilers
   #elif defined _M_IX86
     __asm rdtsc
   #elif defined _M_AMD64
     return __rdtsc ();
   #elif defined _M_IA64
     #if defined __INTEL_COMPILER
       #include <ia64intrin.h>
     #endif
      return __getReg (3116);
   #else
     #error need rdtsc function for this build
   #endif
   }


/* generic ISO C timer */
uint64_t LBL_T;
void t_start(void) { LBL_T = TIMFUNC(); }
uint64_t t_read(void) { return TIMFUNC() - LBL_T; }

#else
extern void t_start(void);
extern uint64_t t_read(void);
#endif

uint64_t time_mult(int size, int s)
{
  unsigned long     x;
  mp_int  a, b, c;
  uint64_t t1;

  mp_init (&a);
  mp_init (&b);
  mp_init (&c);

  mp_rand (&a, size);
  mp_rand (&b, size);

  if (s == 1) {
      KARATSUBA_MUL_CUTOFF = size;
  } else {
      KARATSUBA_MUL_CUTOFF = 100000;
  }

  t_start();
  for (x = 0; x < TIMES; x++) {
      mp_mul(&a,&b,&c);
  }
  t1 = t_read();
  mp_clear (&a);
  mp_clear (&b);
  mp_clear (&c);
  return t1;
}

uint64_t time_sqr(int size, int s)
{
  unsigned long     x;
  mp_int  a, b;
  uint64_t t1;

  mp_init (&a);
  mp_init (&b);

  mp_rand (&a, size);

  if (s == 1) {
      KARATSUBA_SQR_CUTOFF = size;
  } else {
      KARATSUBA_SQR_CUTOFF = 100000;
  }

  t_start();
  for (x = 0; x < TIMES; x++) {
      mp_sqr(&a,&b);
  }
  t1 = t_read();
  mp_clear (&a);
  mp_clear (&b);
  return t1;
}

int
main (void)
{
  uint64_t t1, t2;
  int x, y;

  for (x = 8; ; x += 2) {
     t1 = time_mult(x, 0);
     t2 = time_mult(x, 1);
     printf("%d: %9llu %9llu, %9llu\n", x, t1, t2, t2 - t1);
     if (t2 < t1) break;
  }
  y = x;

  for (x = 8; ; x += 2) {
     t1 = time_sqr(x, 0);
     t2 = time_sqr(x, 1);
     printf("%d: %9llu %9llu, %9llu\n", x, t1, t2, t2 - t1);
     if (t2 < t1) break;
  }
  printf("KARATSUBA_MUL_CUTOFF = %d\n", y);
  printf("KARATSUBA_SQR_CUTOFF = %d\n", x);

  return 0;
}

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
