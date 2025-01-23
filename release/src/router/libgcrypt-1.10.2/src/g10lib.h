/* g10lib.h - Internal definitions for libgcrypt
 * Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005
 *               2007, 2011 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* This header is to be used inside of libgcrypt in place of gcrypt.h.
   This way we can better distinguish between internal and external
   usage of gcrypt.h. */

#ifndef G10LIB_H
#define G10LIB_H 1

#ifdef _GCRYPT_H
#error  gcrypt.h already included
#endif

#ifndef _GCRYPT_IN_LIBGCRYPT
#error something is wrong with config.h
#endif

#include <stdio.h>
#include <stdarg.h>

#include "visibility.h"
#include "types.h"




/* Attribute handling macros.  */

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5 )
#define JNLIB_GCC_M_FUNCTION 1
#define JNLIB_GCC_A_NR 	     __attribute__ ((noreturn))
#define JNLIB_GCC_A_PRINTF( f, a )  __attribute__ ((format (printf,f,a)))
#define JNLIB_GCC_A_NR_PRINTF( f, a ) \
			    __attribute__ ((noreturn, format (printf,f,a)))
#define GCC_ATTR_NORETURN  __attribute__ ((__noreturn__))
#else
#define JNLIB_GCC_A_NR
#define JNLIB_GCC_A_PRINTF( f, a )
#define JNLIB_GCC_A_NR_PRINTF( f, a )
#define GCC_ATTR_NORETURN
#endif

#if __GNUC__ >= 3
/* According to glibc this attribute is available since 2.8 however we
   better play safe and use it only with gcc 3 or newer. */
#define GCC_ATTR_FORMAT_ARG(a)  __attribute__ ((format_arg (a)))
#else
#define GCC_ATTR_FORMAT_ARG(a)
#endif

/* I am not sure since when the unused attribute is really supported.
   In any case it it only needed for gcc versions which print a
   warning.  Thus let us require gcc >= 3.5.  */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 5 )
#define GCC_ATTR_UNUSED  __attribute__ ((unused))
#else
#define GCC_ATTR_UNUSED
#endif

#if __GNUC__ > 3
#define NOINLINE_FUNC     __attribute__((noinline))
#else
#define NOINLINE_FUNC
#endif

#if __GNUC__ >= 3
#define LIKELY(expr)      __builtin_expect( !!(expr), 1 )
#define UNLIKELY(expr)    __builtin_expect( !!(expr), 0 )
#define CONSTANT_P(expr)  __builtin_constant_p( expr )
#else
#define LIKELY(expr)      (!!(expr))
#define UNLIKELY(expr)    (!!(expr))
#define CONSTANT_P(expr)  (0)
#endif

/* Gettext macros.  */

#define _(a)  _gcry_gettext(a)
#define N_(a) (a)

/* Some handy macros */
#ifndef STR
#define STR(v) #v
#endif
#define STR2(v) STR(v)
#define DIM(v) (sizeof(v)/sizeof((v)[0]))
#define DIMof(type,member)   DIM(((type *)0)->member)

#define my_isascii(c) (!((c) & 0x80))




/*-- src/global.c -*/
extern int _gcry_global_any_init_done;
int _gcry_global_is_operational (void);
gcry_err_code_t _gcry_vcontrol (enum gcry_ctl_cmds cmd, va_list arg_ptr);
void _gcry_check_heap (const void *a);
void _gcry_pre_syscall (void);
void _gcry_post_syscall (void);
int _gcry_get_debug_flag (unsigned int mask);

char *_gcry_get_config (int mode, const char *what);

/* Malloc functions and common wrapper macros.  */
void *_gcry_malloc (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_calloc (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_malloc_secure (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_calloc_secure (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_realloc (void *a, size_t n);
char *_gcry_strdup (const char *string) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xmalloc (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xcalloc (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xmalloc_secure (size_t n) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xcalloc_secure (size_t n, size_t m) _GCRY_GCC_ATTR_MALLOC;
void *_gcry_xrealloc (void *a, size_t n);
char *_gcry_xstrdup (const char * a) _GCRY_GCC_ATTR_MALLOC;
void  _gcry_free (void *a);
int   _gcry_is_secure (const void *a) _GCRY_GCC_ATTR_PURE;

#define xtrymalloc(a)    _gcry_malloc ((a))
#define xtrycalloc(a,b)  _gcry_calloc ((a),(b))
#define xtrymalloc_secure(a)   _gcry_malloc_secure ((a))
#define xtrycalloc_secure(a,b) _gcry_calloc_secure ((a),(b))
#define xtryrealloc(a,b) _gcry_realloc ((a),(b))
#define xtrystrdup(a)    _gcry_strdup ((a))
#define xmalloc(a)       _gcry_xmalloc ((a))
#define xcalloc(a,b)     _gcry_xcalloc ((a),(b))
#define xmalloc_secure(a)   _gcry_xmalloc_secure ((a))
#define xcalloc_secure(a,b) _gcry_xcalloc_secure ((a),(b))
#define xrealloc(a,b)    _gcry_xrealloc ((a),(b))
#define xstrdup(a)       _gcry_xstrdup ((a))
#define xfree(a)         _gcry_free ((a))


/*-- src/misc.c --*/

#if defined(JNLIB_GCC_M_FUNCTION) || __STDC_VERSION__ >= 199901L
void _gcry_bug (const char *file, int line,
                const char *func) GCC_ATTR_NORETURN;
void _gcry_assert_failed (const char *expr, const char *file, int line,
                          const char *func) GCC_ATTR_NORETURN;
#else
void _gcry_bug (const char *file, int line);
void _gcry_assert_failed (const char *expr, const char *file, int line);
#endif

void _gcry_divide_by_zero (void) JNLIB_GCC_A_NR;

const char *_gcry_gettext (const char *key) GCC_ATTR_FORMAT_ARG(1);
void _gcry_fatal_error(int rc, const char *text ) JNLIB_GCC_A_NR;
void _gcry_logv (int level,
                 const char *fmt, va_list arg_ptr) JNLIB_GCC_A_PRINTF(2,0);
void _gcry_log( int level, const char *fmt, ... ) JNLIB_GCC_A_PRINTF(2,3);
void _gcry_log_bug( const char *fmt, ... )   JNLIB_GCC_A_NR_PRINTF(1,2);
void _gcry_log_fatal( const char *fmt, ... ) JNLIB_GCC_A_NR_PRINTF(1,2);
void _gcry_log_error( const char *fmt, ... ) JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_info( const char *fmt, ... )  JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_debug( const char *fmt, ... ) JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_printf ( const char *fmt, ... ) JNLIB_GCC_A_PRINTF(1,2);
void _gcry_log_printhex (const char *text, const void *buffer, size_t length);
void _gcry_log_printmpi (const char *text, gcry_mpi_t mpi);
void _gcry_log_printsxp (const char *text, gcry_sexp_t sexp);

void _gcry_set_log_verbosity( int level );
int _gcry_log_verbosity( int level );


#ifdef JNLIB_GCC_M_FUNCTION
#define BUG() _gcry_bug( __FILE__ , __LINE__, __FUNCTION__ )
#define gcry_assert(expr) (LIKELY(expr)? (void)0 \
         : _gcry_assert_failed (STR(expr), __FILE__, __LINE__, __FUNCTION__))
#elif __STDC_VERSION__ >= 199901L
#define BUG() _gcry_bug( __FILE__ , __LINE__, __func__ )
#define gcry_assert(expr) (LIKELY(expr)? (void)0 \
         : _gcry_assert_failed (STR(expr), __FILE__, __LINE__, __func__))
#else
#define BUG() _gcry_bug( __FILE__ , __LINE__ )
#define gcry_assert(expr) (LIKELY(expr)? (void)0 \
         : _gcry_assert_failed (STR(expr), __FILE__, __LINE__))
#endif


#define log_bug     _gcry_log_bug
#define log_fatal   _gcry_log_fatal
#define log_error   _gcry_log_error
#define log_info    _gcry_log_info
#define log_debug   _gcry_log_debug
#define log_printf  _gcry_log_printf
#define log_printhex _gcry_log_printhex
#define log_printmpi _gcry_log_printmpi
#define log_printsxp _gcry_log_printsxp

/* Compatibility macro.  */
#define log_mpidump _gcry_log_printmpi

/* Tokeninze STRING and return a malloced array.  */
char **_gcry_strtokenize (const char *string, const char *delim);


/*-- src/hwfeatures.c --*/
#if defined(HAVE_CPU_ARCH_X86)

#define HWF_PADLOCK_RNG         (1 << 0)
#define HWF_PADLOCK_AES         (1 << 1)
#define HWF_PADLOCK_SHA         (1 << 2)
#define HWF_PADLOCK_MMUL        (1 << 3)

#define HWF_INTEL_CPU           (1 << 4)
#define HWF_INTEL_FAST_SHLD     (1 << 5)
#define HWF_INTEL_BMI2          (1 << 6)
#define HWF_INTEL_SSSE3         (1 << 7)
#define HWF_INTEL_SSE4_1        (1 << 8)
#define HWF_INTEL_PCLMUL        (1 << 9)
#define HWF_INTEL_AESNI         (1 << 10)
#define HWF_INTEL_RDRAND        (1 << 11)
#define HWF_INTEL_AVX           (1 << 12)
#define HWF_INTEL_AVX2          (1 << 13)
#define HWF_INTEL_FAST_VPGATHER (1 << 14)
#define HWF_INTEL_RDTSC         (1 << 15)
#define HWF_INTEL_SHAEXT        (1 << 16)
#define HWF_INTEL_VAES_VPCLMUL  (1 << 17)

#elif defined(HAVE_CPU_ARCH_ARM)

#define HWF_ARM_NEON            (1 << 0)
#define HWF_ARM_AES             (1 << 1)
#define HWF_ARM_SHA1            (1 << 2)
#define HWF_ARM_SHA2            (1 << 3)
#define HWF_ARM_PMULL           (1 << 4)

#elif defined(HAVE_CPU_ARCH_PPC)

#define HWF_PPC_VCRYPTO         (1 << 0)
#define HWF_PPC_ARCH_3_00       (1 << 1)
#define HWF_PPC_ARCH_2_07       (1 << 2)
#define HWF_PPC_ARCH_3_10       (1 << 3)

#elif defined(HAVE_CPU_ARCH_S390X)

#define HWF_S390X_MSA           (1 << 0)
#define HWF_S390X_MSA_4         (1 << 1)
#define HWF_S390X_MSA_8         (1 << 2)
#define HWF_S390X_MSA_9         (1 << 3)
#define HWF_S390X_VX            (1 << 4)

#endif

gpg_err_code_t _gcry_disable_hw_feature (const char *name);
void _gcry_detect_hw_features (void);
unsigned int _gcry_get_hw_features (void);
const char *_gcry_enum_hw_features (int idx, unsigned int *r_feature);


/*-- mpi/mpiutil.c --*/
const char *_gcry_mpi_get_hw_config (void);


/*-- cipher/pubkey.c --*/

/* FIXME: shouldn't this go into mpi.h?  */
#ifndef mpi_powm
#define mpi_powm(w,b,e,m)   gcry_mpi_powm( (w), (b), (e), (m) )
#endif

/*-- primegen.c --*/
gcry_err_code_t _gcry_primegen_init (void);
gcry_mpi_t _gcry_generate_secret_prime (unsigned int nbits,
                                 gcry_random_level_t random_level,
                                 int (*extra_check)(void*, gcry_mpi_t),
                                 void *extra_check_arg);
gcry_mpi_t _gcry_generate_public_prime (unsigned int nbits,
                                 gcry_random_level_t random_level,
                                 int (*extra_check)(void*, gcry_mpi_t),
                                 void *extra_check_arg);
gcry_err_code_t _gcry_generate_elg_prime (int mode,
                                          unsigned int pbits,
                                          unsigned int qbits,
                                          gcry_mpi_t g,
                                          gcry_mpi_t *r_prime,
                                          gcry_mpi_t **factors);
gcry_mpi_t _gcry_derive_x931_prime (const gcry_mpi_t xp,
                                    const gcry_mpi_t xp1, const gcry_mpi_t xp2,
                                    const gcry_mpi_t e,
                                    gcry_mpi_t *r_p1, gcry_mpi_t *r_p2);
gpg_err_code_t _gcry_generate_fips186_2_prime
                 (unsigned int pbits, unsigned int qbits,
                  const void *seed, size_t seedlen,
                  gcry_mpi_t *r_q, gcry_mpi_t *r_p,
                  int *r_counter,
                  void **r_seed, size_t *r_seedlen);
gpg_err_code_t _gcry_generate_fips186_3_prime
                 (unsigned int pbits, unsigned int qbits,
                  const void *seed, size_t seedlen,
                  gcry_mpi_t *r_q, gcry_mpi_t *r_p,
                  int *r_counter,
                  void **r_seed, size_t *r_seedlen, int *r_hashalgo);

gpg_err_code_t _gcry_fips186_4_prime_check (const gcry_mpi_t x,
                                            unsigned int bits);


/* Replacements of missing functions (missing-string.c).  */
#ifndef HAVE_STPCPY
char *stpcpy (char *a, const char *b);
#endif
#ifndef HAVE_STRCASECMP
int strcasecmp (const char *a, const char *b) _GCRY_GCC_ATTR_PURE;
#endif

#include "../compat/libcompat.h"


/* Macros used to rename missing functions.  */
#ifndef HAVE_STRTOUL
#define strtoul(a,b,c)  ((unsigned long)strtol((a),(b),(c)))
#endif
#ifndef HAVE_MEMMOVE
#define memmove(d, s, n) bcopy((s), (d), (n))
#endif
#ifndef HAVE_STRICMP
#define stricmp(a,b)	 strcasecmp( (a), (b) )
#endif
#ifndef HAVE_ATEXIT
#define atexit(a)    (on_exit((a),0))
#endif
#ifndef HAVE_RAISE
#define raise(a) kill(getpid(), (a))
#endif


/* Stack burning.  */

#ifdef HAVE_GCC_ASM_VOLATILE_MEMORY
#define  __gcry_burn_stack_dummy() asm volatile ("":::"memory")
#else
void __gcry_burn_stack_dummy (void);
#endif

void __gcry_burn_stack (unsigned int bytes);
#define _gcry_burn_stack(bytes) \
	do { __gcry_burn_stack (bytes); \
	     __gcry_burn_stack_dummy (); } while(0)

/* To avoid that a compiler optimizes certain memset calls away, this
   macro may be used instead.  For constant length buffers, memory
   wiping is inlined.  Dead store elimination of inlined memset is
   avoided here by using assembly block after memset.  For non-constant
   length buffers, memory is wiped through _gcry_fast_wipememory.  */
#ifdef HAVE_GCC_ASM_VOLATILE_MEMORY
#define fast_wipememory2_inline(_ptr,_set,_len) do { \
	      memset((_ptr), (_set), (_len)); \
	      asm volatile ("\n" :: "r" (_ptr) : "memory"); \
	    } while(0)
#else
#define fast_wipememory2_inline(_ptr,_set,_len) \
	    _gcry_fast_wipememory2((void *)_ptr, _set, _len)
#endif
#define wipememory2(_ptr,_set,_len) do { \
	      if (!CONSTANT_P(_len) || !CONSTANT_P(_set)) { \
		if (CONSTANT_P(_set) && (_set) == 0) \
		  _gcry_fast_wipememory((void *)(_ptr), (_len)); \
		else \
		  _gcry_fast_wipememory2((void *)(_ptr), (_set), (_len)); \
	      } else { \
		fast_wipememory2_inline((void *)(_ptr), (_set), (_len)); \
	      } \
	    } while(0)
#define wipememory(_ptr,_len) wipememory2((_ptr),0,(_len))

void _gcry_fast_wipememory(void *ptr, size_t len);
void _gcry_fast_wipememory2(void *ptr, int set, size_t len);

/* Digit predicates.  */

#define digitp(p)   (*(p) >= '0' && *(p) <= '9')
#define octdigitp(p) (*(p) >= '0' && *(p) <= '7')
#define alphap(a)    (   (*(a) >= 'A' && *(a) <= 'Z')  \
                      || (*(a) >= 'a' && *(a) <= 'z'))
#define hexdigitp(a) (digitp (a)                     \
                      || (*(a) >= 'A' && *(a) <= 'F')  \
                      || (*(a) >= 'a' && *(a) <= 'f'))

/* Init functions.  */

gcry_err_code_t _gcry_cipher_init (void);
gcry_err_code_t _gcry_md_init (void);
gcry_err_code_t _gcry_mac_init (void);
gcry_err_code_t _gcry_pk_init (void);
gcry_err_code_t _gcry_secmem_module_init (void);
gcry_err_code_t _gcry_mpi_init (void);

/* Memory management.  */
#define GCRY_ALLOC_FLAG_SECURE (1 << 0)
#define GCRY_ALLOC_FLAG_XHINT  (1 << 1)  /* Called from xmalloc.  */


/*-- sexp.c --*/
gcry_err_code_t _gcry_sexp_vbuild (gcry_sexp_t *retsexp, size_t *erroff,
                                   const char *format, va_list arg_ptr);
char *_gcry_sexp_nth_string (const gcry_sexp_t list, int number);
gpg_err_code_t _gcry_sexp_vextract_param (gcry_sexp_t sexp, const char *path,
                                          const char *list, va_list arg_ptr);


/*-- fips.c --*/

extern int _gcry_no_fips_mode_required;

void _gcry_initialize_fips_mode (int force);
int _gcry_fips_to_activate (void);

/* This macro returns true if fips mode is enabled.  This is
   independent of the fips required finite state machine and only used
   to enable fips specific code.

   No locking is required because we have the requirement that this
   variable is only initialized once with no other threads
   existing.  */
#define fips_mode() (!_gcry_no_fips_mode_required)


void _gcry_fips_signal_error (const char *srcfile,
                              int srcline,
                              const char *srcfunc,
                              int is_fatal,
                              const char *description);
#ifdef JNLIB_GCC_M_FUNCTION
# define fips_signal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, __FUNCTION__, 0, (a))
# define fips_signal_fatal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, __FUNCTION__, 1, (a))
#else
# define fips_signal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, NULL, 0, (a))
# define fips_signal_fatal_error(a) \
           _gcry_fips_signal_error (__FILE__, __LINE__, NULL, 1, (a))
#endif

int _gcry_fips_indicator_cipher (va_list arg_ptr);
int _gcry_fips_indicator_mac (va_list arg_ptr);
int _gcry_fips_indicator_md (va_list arg_ptr);
int _gcry_fips_indicator_kdf (va_list arg_ptr);
int _gcry_fips_indicator_function (va_list arg_ptr);
int _gcry_fips_indicator_pk_flags (va_list arg_ptr);

int _gcry_fips_is_operational (void);

/* Return true if the library is in the operational state.  */
#define fips_is_operational()   \
        (!_gcry_global_any_init_done ? \
                _gcry_global_is_operational() : \
                (!fips_mode () || _gcry_global_is_operational ()))

#define fips_not_operational()  (GPG_ERR_NOT_OPERATIONAL)

int _gcry_fips_test_operational (void);
int _gcry_fips_test_error_or_operational (void);

gpg_err_code_t _gcry_fips_run_selftests (int extended);

void _gcry_fips_noreturn (void);
#define fips_noreturn()  (_gcry_fips_noreturn ())



#endif /* G10LIB_H */
