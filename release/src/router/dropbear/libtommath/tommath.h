/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tstdenis82@gmail.com, http://math.libtomcrypt.com
 */
#ifndef BN_H_
#define BN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include "tommath_class.h"

#ifdef __cplusplus
extern "C" {
#endif

/* detect 64-bit mode if possible */
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64) || \
    defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
    defined(__s390x__) || defined(__arch64__) || defined(__aarch64__) || \
    defined(__sparcv9) || defined(__sparc_v9__) || defined(__sparc64__) || \
    defined(__ia64) || defined(__ia64__) || defined(__itanium__) || defined(_M_IA64) || \
    defined(__LP64__) || defined(_LP64) || defined(__64BIT__)
   #if !(defined(MP_32BIT) || defined(MP_16BIT) || defined(MP_8BIT))
      #define MP_64BIT
   #endif
#endif

/* some default configurations.
 *
 * A "mp_digit" must be able to hold DIGIT_BIT + 1 bits
 * A "mp_word" must be able to hold 2*DIGIT_BIT + 1 bits
 *
 * At the very least a mp_digit must be able to hold 7 bits
 * [any size beyond that is ok provided it doesn't overflow the data type]
 */
#ifdef MP_8BIT
   typedef uint8_t              mp_digit;
   typedef uint16_t             mp_word;
#define MP_SIZEOF_MP_DIGIT      1
#ifdef DIGIT_BIT
#error You must not define DIGIT_BIT when using MP_8BIT
#endif
#elif defined(MP_16BIT)
   typedef uint16_t             mp_digit;
   typedef uint32_t             mp_word;
#define MP_SIZEOF_MP_DIGIT      2
#ifdef DIGIT_BIT
#error You must not define DIGIT_BIT when using MP_16BIT
#endif
#elif defined(MP_64BIT)
   /* for GCC only on supported platforms */
   typedef uint64_t mp_digit;
#if defined(_WIN32)
   typedef unsigned __int128    mp_word;
#elif defined(__GNUC__)
   typedef unsigned long        mp_word __attribute__ ((mode(TI)));
#else
   /* it seems you have a problem
    * but we assume you can somewhere define your own uint128_t */
   typedef uint128_t            mp_word;
#endif

   #define DIGIT_BIT            60
#else
   /* this is the default case, 28-bit digits */

   /* this is to make porting into LibTomCrypt easier :-) */
   typedef uint32_t             mp_digit;
   typedef uint64_t             mp_word;

#ifdef MP_31BIT
   /* this is an extension that uses 31-bit digits */
   #define DIGIT_BIT            31
#else
   /* default case is 28-bit digits, defines MP_28BIT as a handy macro to test */
   #define DIGIT_BIT            28
   #define MP_28BIT
#endif
#endif

/* otherwise the bits per digit is calculated automatically from the size of a mp_digit */
#ifndef DIGIT_BIT
   #define DIGIT_BIT     (((CHAR_BIT * MP_SIZEOF_MP_DIGIT) - 1))  /* bits per digit */
   typedef uint_least32_t mp_min_u32;
#else
   typedef mp_digit mp_min_u32;
#endif

/* use arc4random on platforms that support it */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #define MP_GEN_RANDOM()    arc4random()
    #define MP_GEN_RANDOM_MAX  0xffffffff
#endif

/* use rand() as fall-back if there's no better rand function */
#ifndef MP_GEN_RANDOM
    #define MP_GEN_RANDOM()    rand()
    #define MP_GEN_RANDOM_MAX  RAND_MAX
#endif

#define MP_DIGIT_BIT     DIGIT_BIT
#define MP_MASK          ((((mp_digit)1)<<((mp_digit)DIGIT_BIT))-((mp_digit)1))
#define MP_DIGIT_MAX     MP_MASK

/* equalities */
#define MP_LT        -1   /* less than */
#define MP_EQ         0   /* equal to */
#define MP_GT         1   /* greater than */

#define MP_ZPOS       0   /* positive integer */
#define MP_NEG        1   /* negative */

#define MP_OKAY       0   /* ok result */
#define MP_MEM        -2  /* out of mem */
#define MP_VAL        -3  /* invalid input */
#define MP_RANGE      MP_VAL

#define MP_YES        1   /* yes response */
#define MP_NO         0   /* no response */

/* Primality generation flags */
#define LTM_PRIME_BBS      0x0001 /* BBS style prime */
#define LTM_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define LTM_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

typedef int           mp_err;

/* you'll have to tune these... */
extern int KARATSUBA_MUL_CUTOFF,
           KARATSUBA_SQR_CUTOFF,
           TOOM_MUL_CUTOFF,
           TOOM_SQR_CUTOFF;

/* define this to use lower memory usage routines (exptmods mostly) */
/* #define MP_LOW_MEM */

/* default precision */
#ifndef MP_PREC
   #ifndef MP_LOW_MEM
      #define MP_PREC                 32     /* default digits of precision */
   #else
      #define MP_PREC                 8      /* default digits of precision */
   #endif
#endif

/* size of comba arrays, should be at least 2 * 2**(BITS_PER_WORD - BITS_PER_DIGIT*2) */
#define MP_WARRAY               (1 << (((sizeof(mp_word) * CHAR_BIT) - (2 * DIGIT_BIT)) + 1))

/* the infamous mp_int structure */
typedef struct  {
    int used, alloc, sign;
    mp_digit *dp;
} mp_int;

/* callback for mp_prime_random, should fill dst with random bytes and return how many read [upto len] */
typedef int ltm_prime_callback(unsigned char *dst, int len, void *dat);


#define USED(m)    ((m)->used)
#define DIGIT(m,k) ((m)->dp[(k)])
#define SIGN(m)    ((m)->sign)

/* error code to char* string */
const char *mp_error_to_string(int code);

/* ---> init and deinit bignum functions <--- */
/* init a bignum */
int mp_init(mp_int *a);

/* free a bignum */
void mp_clear(mp_int *a);

/* init a null terminated series of arguments */
int mp_init_multi(mp_int *mp, ...);

/* clear a null terminated series of arguments */
void mp_clear_multi(mp_int *mp, ...);

/* exchange two ints */
void mp_exch(mp_int *a, mp_int *b);

/* shrink ram required for a bignum */
int mp_shrink(mp_int *a);

/* grow an int to a given size */
int mp_grow(mp_int *a, int size);

/* init to a given number of digits */
int mp_init_size(mp_int *a, int size);

/* ---> Basic Manipulations <--- */
#define mp_iszero(a) (((a)->used == 0) ? MP_YES : MP_NO)
#define mp_iseven(a) ((((a)->used > 0) && (((a)->dp[0] & 1u) == 0u)) ? MP_YES : MP_NO)
#define mp_isodd(a)  ((((a)->used > 0) && (((a)->dp[0] & 1u) == 1u)) ? MP_YES : MP_NO)
#define mp_isneg(a)  (((a)->sign != MP_ZPOS) ? MP_YES : MP_NO)

/* set to zero */
void mp_zero(mp_int *a);

/* set to a digit */
void mp_set(mp_int *a, mp_digit b);

/* set a 32-bit const */
int mp_set_int(mp_int *a, unsigned long b);

/* set a platform dependent unsigned long value */
int mp_set_long(mp_int *a, unsigned long b);

/* set a platform dependent unsigned long long value */
int mp_set_long_long(mp_int *a, unsigned long long b);

/* get a 32-bit value */
unsigned long mp_get_int(mp_int * a);

/* get a platform dependent unsigned long value */
unsigned long mp_get_long(mp_int * a);

/* get a platform dependent unsigned long long value */
unsigned long long mp_get_long_long(mp_int * a);

/* initialize and set a digit */
int mp_init_set (mp_int * a, mp_digit b);

/* initialize and set 32-bit value */
int mp_init_set_int (mp_int * a, unsigned long b);

/* copy, b = a */
int mp_copy(mp_int *a, mp_int *b);

/* inits and copies, a = b */
int mp_init_copy(mp_int *a, mp_int *b);

/* trim unused digits */
void mp_clamp(mp_int *a);

/* import binary data */
int mp_import(mp_int* rop, size_t count, int order, size_t size, int endian, size_t nails, const void* op);

/* export binary data */
int mp_export(void* rop, size_t* countp, int order, size_t size, int endian, size_t nails, mp_int* op);

/* ---> digit manipulation <--- */

/* right shift by "b" digits */
void mp_rshd(mp_int *a, int b);

/* left shift by "b" digits */
int mp_lshd(mp_int *a, int b);

/* c = a / 2**b, implemented as c = a >> b */
int mp_div_2d(mp_int *a, int b, mp_int *c, mp_int *d);

/* b = a/2 */
int mp_div_2(mp_int *a, mp_int *b);

/* c = a * 2**b, implemented as c = a << b */
int mp_mul_2d(mp_int *a, int b, mp_int *c);

/* b = a*2 */
int mp_mul_2(mp_int *a, mp_int *b);

/* c = a mod 2**b */
int mp_mod_2d(mp_int *a, int b, mp_int *c);

/* computes a = 2**b */
int mp_2expt(mp_int *a, int b);

/* Counts the number of lsbs which are zero before the first zero bit */
int mp_cnt_lsb(mp_int *a);

/* I Love Earth! */

/* makes a pseudo-random int of a given size */
int mp_rand(mp_int *a, int digits);

/* ---> binary operations <--- */
/* c = a XOR b  */
int mp_xor(mp_int *a, mp_int *b, mp_int *c);

/* c = a OR b */
int mp_or(mp_int *a, mp_int *b, mp_int *c);

/* c = a AND b */
int mp_and(mp_int *a, mp_int *b, mp_int *c);

/* ---> Basic arithmetic <--- */

/* b = -a */
int mp_neg(mp_int *a, mp_int *b);

/* b = |a| */
int mp_abs(mp_int *a, mp_int *b);

/* compare a to b */
int mp_cmp(mp_int *a, mp_int *b);

/* compare |a| to |b| */
int mp_cmp_mag(mp_int *a, mp_int *b);

/* c = a + b */
int mp_add(mp_int *a, mp_int *b, mp_int *c);

/* c = a - b */
int mp_sub(mp_int *a, mp_int *b, mp_int *c);

/* c = a * b */
int mp_mul(mp_int *a, mp_int *b, mp_int *c);

/* b = a*a  */
int mp_sqr(mp_int *a, mp_int *b);

/* a/b => cb + d == a */
int mp_div(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* c = a mod b, 0 <= c < b  */
int mp_mod(mp_int *a, mp_int *b, mp_int *c);

/* ---> single digit functions <--- */

/* compare against a single digit */
int mp_cmp_d(mp_int *a, mp_digit b);

/* c = a + b */
int mp_add_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a - b */
int mp_sub_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a * b */
int mp_mul_d(mp_int *a, mp_digit b, mp_int *c);

/* a/b => cb + d == a */
int mp_div_d(mp_int *a, mp_digit b, mp_int *c, mp_digit *d);

/* a/3 => 3c + d == a */
int mp_div_3(mp_int *a, mp_int *c, mp_digit *d);

/* c = a**b */
int mp_expt_d(mp_int *a, mp_digit b, mp_int *c);
int mp_expt_d_ex (mp_int * a, mp_digit b, mp_int * c, int fast);

/* c = a mod b, 0 <= c < b  */
int mp_mod_d(mp_int *a, mp_digit b, mp_digit *c);

/* ---> number theory <--- */

/* d = a + b (mod c) */
int mp_addmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* d = a - b (mod c) */
int mp_submod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* d = a * b (mod c) */
int mp_mulmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* c = a * a (mod b) */
int mp_sqrmod(mp_int *a, mp_int *b, mp_int *c);

/* c = 1/a (mod b) */
int mp_invmod(mp_int *a, mp_int *b, mp_int *c);

/* c = (a, b) */
int mp_gcd(mp_int *a, mp_int *b, mp_int *c);

/* produces value such that U1*a + U2*b = U3 */
int mp_exteuclid(mp_int *a, mp_int *b, mp_int *U1, mp_int *U2, mp_int *U3);

/* c = [a, b] or (a*b)/(a, b) */
int mp_lcm(mp_int *a, mp_int *b, mp_int *c);

/* finds one of the b'th root of a, such that |c|**b <= |a|
 *
 * returns error if a < 0 and b is even
 */
int mp_n_root(mp_int *a, mp_digit b, mp_int *c);
int mp_n_root_ex (mp_int * a, mp_digit b, mp_int * c, int fast);

/* special sqrt algo */
int mp_sqrt(mp_int *arg, mp_int *ret);

/* special sqrt (mod prime) */
int mp_sqrtmod_prime(mp_int *arg, mp_int *prime, mp_int *ret);

/* is number a square? */
int mp_is_square(mp_int *arg, int *ret);

/* computes the jacobi c = (a | n) (or Legendre if b is prime)  */
int mp_jacobi(mp_int *a, mp_int *n, int *c);

/* used to setup the Barrett reduction for a given modulus b */
int mp_reduce_setup(mp_int *a, mp_int *b);

/* Barrett Reduction, computes a (mod b) with a precomputed value c
 *
 * Assumes that 0 < a <= b*b, note if 0 > a > -(b*b) then you can merely
 * compute the reduction as -1 * mp_reduce(mp_abs(a)) [pseudo code].
 */
int mp_reduce(mp_int *a, mp_int *b, mp_int *c);

/* setups the montgomery reduction */
int mp_montgomery_setup(mp_int *a, mp_digit *mp);

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
int mp_montgomery_calc_normalization(mp_int *a, mp_int *b);

/* computes x/R == x (mod N) via Montgomery Reduction */
int mp_montgomery_reduce(mp_int *a, mp_int *m, mp_digit mp);

/* returns 1 if a is a valid DR modulus */
int mp_dr_is_modulus(mp_int *a);

/* sets the value of "d" required for mp_dr_reduce */
void mp_dr_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b using the Diminished Radix method */
int mp_dr_reduce(mp_int *a, mp_int *b, mp_digit mp);

/* returns true if a can be reduced with mp_reduce_2k */
int mp_reduce_is_2k(mp_int *a);

/* determines k value for 2k reduction */
int mp_reduce_2k_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
int mp_reduce_2k(mp_int *a, mp_int *n, mp_digit d);

/* returns true if a can be reduced with mp_reduce_2k_l */
int mp_reduce_is_2k_l(mp_int *a);

/* determines k value for 2k reduction */
int mp_reduce_2k_setup_l(mp_int *a, mp_int *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
int mp_reduce_2k_l(mp_int *a, mp_int *n, mp_int *d);

/* d = a**b (mod c) */
int mp_exptmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* ---> Primes <--- */

/* number of primes */
#ifdef MP_8BIT
   #define PRIME_SIZE      31
#else
   #define PRIME_SIZE      256
#endif

/* table of first PRIME_SIZE primes */
extern const mp_digit ltm_prime_tab[PRIME_SIZE];

/* result=1 if a is divisible by one of the first PRIME_SIZE primes */
int mp_prime_is_divisible(mp_int *a, int *result);

/* performs one Fermat test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
int mp_prime_fermat(mp_int *a, mp_int *b, int *result);

/* performs one Miller-Rabin test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
int mp_prime_miller_rabin(mp_int *a, mp_int *b, int *result);

/* This gives [for a given bit size] the number of trials required
 * such that Miller-Rabin gives a prob of failure lower than 2^-96
 */
int mp_prime_rabin_miller_trials(int size);

/* performs t rounds of Miller-Rabin on "a" using the first
 * t prime bases.  Also performs an initial sieve of trial
 * division.  Determines if "a" is prime with probability
 * of error no more than (1/4)**t.
 *
 * Sets result to 1 if probably prime, 0 otherwise
 */
int mp_prime_is_prime(mp_int *a, int t, int *result);

/* finds the next prime after the number "a" using "t" trials
 * of Miller-Rabin.
 *
 * bbs_style = 1 means the prime must be congruent to 3 mod 4
 */
int mp_prime_next_prime(mp_int *a, int t, int bbs_style);

/* makes a truly random prime of a given size (bytes),
 * call with bbs = 1 if you want it to be congruent to 3 mod 4
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 * The prime generated will be larger than 2^(8*size).
 */
#define mp_prime_random(a, t, size, bbs, cb, dat) mp_prime_random_ex(a, t, ((size) * 8) + 1, (bbs==1)?LTM_PRIME_BBS:0, cb, dat)

/* makes a truly random prime of a given size (bits),
 *
 * Flags are as follows:
 *
 *   LTM_PRIME_BBS      - make prime congruent to 3 mod 4
 *   LTM_PRIME_SAFE     - make sure (p-1)/2 is prime as well (implies LTM_PRIME_BBS)
 *   LTM_PRIME_2MSB_ON  - make the 2nd highest bit one
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 */
int mp_prime_random_ex(mp_int *a, int t, int size, int flags, ltm_prime_callback cb, void *dat);

/* ---> radix conversion <--- */
int mp_count_bits(mp_int *a);

int mp_unsigned_bin_size(mp_int *a);
int mp_read_unsigned_bin(mp_int *a, const unsigned char *b, int c);
int mp_to_unsigned_bin(mp_int *a, unsigned char *b);
int mp_to_unsigned_bin_n (mp_int * a, unsigned char *b, unsigned long *outlen);

int mp_signed_bin_size(mp_int *a);
int mp_read_signed_bin(mp_int *a, const unsigned char *b, int c);
int mp_to_signed_bin(mp_int *a,  unsigned char *b);
int mp_to_signed_bin_n (mp_int * a, unsigned char *b, unsigned long *outlen);

int mp_read_radix(mp_int *a, const char *str, int radix);
int mp_toradix(mp_int *a, char *str, int radix);
int mp_toradix_n(mp_int * a, char *str, int radix, int maxlen);
int mp_radix_size(mp_int *a, int radix, int *size);

#ifndef LTM_NO_FILE
int mp_fread(mp_int *a, int radix, FILE *stream);
int mp_fwrite(mp_int *a, int radix, FILE *stream);
#endif

#define mp_read_raw(mp, str, len) mp_read_signed_bin((mp), (str), (len))
#define mp_raw_size(mp)           mp_signed_bin_size(mp)
#define mp_toraw(mp, str)         mp_to_signed_bin((mp), (str))
#define mp_read_mag(mp, str, len) mp_read_unsigned_bin((mp), (str), (len))
#define mp_mag_size(mp)           mp_unsigned_bin_size(mp)
#define mp_tomag(mp, str)         mp_to_unsigned_bin((mp), (str))

#define mp_tobinary(M, S)  mp_toradix((M), (S), 2)
#define mp_tooctal(M, S)   mp_toradix((M), (S), 8)
#define mp_todecimal(M, S) mp_toradix((M), (S), 10)
#define mp_tohex(M, S)     mp_toradix((M), (S), 16)

#ifdef __cplusplus
   }
#endif

#endif


/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
