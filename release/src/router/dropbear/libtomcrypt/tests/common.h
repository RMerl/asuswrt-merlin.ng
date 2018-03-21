/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */
#ifndef DEMOS_COMMON_H_
#define DEMOS_COMMON_H_

#include <tomcrypt.h>

extern prng_state yarrow_prng;

#ifdef LTC_VERBOSE
#define DO(x) do { fprintf(stderr, "%s:\n", #x); run_cmd((x), __LINE__, __FILE__, #x, NULL); } while (0)
#define DOX(x, str) do { fprintf(stderr, "%s - %s:\n", #x, (str)); run_cmd((x), __LINE__, __FILE__, #x, (str)); } while (0)
#else
#define DO(x) do { run_cmd((x), __LINE__, __FILE__, #x, NULL); } while (0)
#define DOX(x, str) do { run_cmd((x), __LINE__, __FILE__, #x, (str)); } while (0)
#endif

void run_cmd(int res, int line, const char *file, const char *cmd, const char *algorithm);

void print_hex(const char* what, const void* v, const unsigned long l);

#endif /* DEMOS_COMMON_H_ */

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
