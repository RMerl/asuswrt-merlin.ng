/*
 * Non-physical true random number generator based on timing jitter.
 *
 * Copyright Stephan Mueller <smueller@chronox.de>, 2013 - 2021
 *
 * License
 * =======
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef _JITTERENTROPY_BASE_USER_H
#define _JITTERENTROPY_BASE_USER_H

/*
 * Set the following defines as needed for your environment
 */
/* Compilation for libgcrypt */
#ifndef LIBGCRYPT
#undef LIBGCRYPT
#endif

/* Compilation for OpenSSL */
#ifndef OPENSSL
#undef OPENSSL
#endif

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* Timer-less entropy source */
#ifdef JENT_CONF_ENABLE_INTERNAL_TIMER
#include <pthread.h>
#endif /* JENT_CONF_ENABLE_INTERNAL_TIMER */

#ifdef LIBGCRYPT
#include <config.h>
#include "g10lib.h"
#endif

#ifdef OPENSSL
#include <openssl/crypto.h>
#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#endif
#endif

#ifdef __MACH__
#include <assert.h>
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#endif

#ifdef __x86_64__

# define DECLARE_ARGS(val, low, high)    unsigned long low, high
# define EAX_EDX_VAL(val, low, high)     ((low) | (high) << 32)
# define EAX_EDX_RET(val, low, high)     "=a" (low), "=d" (high)

static inline void jent_get_nstime(uint64_t *out)
{
	DECLARE_ARGS(val, low, high);
	asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));
	*out = EAX_EDX_VAL(val, low, high);
}

#else /* __x86_64__ */

static inline void jent_get_nstime(uint64_t *out)
{
	/* OSX does not have clock_gettime -- taken from
	 * http://developer.apple.com/library/mac/qa/qa1398/_index.html */
# ifdef __MACH__
	*out = mach_absolute_time();
# elif _AIX
	/* clock_gettime() on AIX returns a timer value that increments in
	 * steps of 1000
	 */
	uint64_t tmp = 0;
	timebasestruct_t aixtime;
	read_real_time(&aixtime, TIMEBASE_SZ);
	tmp = aixtime.tb_high;
	tmp = tmp << 32;
	tmp = tmp | aixtime.tb_low;
	*out = tmp;
# else /* __MACH__ */
	/* we could use CLOCK_MONOTONIC(_RAW), but with CLOCK_REALTIME
	 * we get some nice extra entropy once in a while from the NTP actions
	 * that we want to use as well... though, we do not rely on that
	 * extra little entropy */
	uint64_t tmp = 0;
	struct timespec time;
	if (clock_gettime(CLOCK_REALTIME, &time) == 0)
	{
		tmp = ((uint64_t)time.tv_sec & 0xFFFFFFFF) * 1000000000UL;
		tmp = tmp + (uint64_t)time.tv_nsec;
	}
	*out = tmp;
# endif /* __MACH__ */
}

#endif /* __x86_64__ */

static inline void *jent_zalloc(size_t len)
{
	void *tmp = NULL;
#ifdef LIBGCRYPT
	/* When using the libgcrypt secure memory mechanism, all precautions
	 * are taken to protect our state. If the user disables secmem during
	 * runtime, it is his decision and we thus try not to overrule his
	 * decision for less memory protection. */
#define CONFIG_CRYPTO_CPU_JITTERENTROPY_SECURE_MEMORY
	tmp = gcry_xmalloc_secure(len);
#elif defined(OPENSSL)
	/* Does this allocation implies secure memory use? */
	tmp = OPENSSL_malloc(len);
#else
	/* we have no secure memory allocation! Hence
	 * we do not set CONFIG_CRYPTO_CPU_JITTERENTROPY_SECURE_MEMORY */
	tmp = malloc(len);
#endif /* LIBGCRYPT */
	if(NULL != tmp)
		memset(tmp, 0, len);
	return tmp;
}

static inline void jent_zfree(void *ptr, unsigned int len)
{
#ifdef LIBGCRYPT
	memset(ptr, 0, len);
	gcry_free(ptr);
#elif defined(OPENSSL)
	OPENSSL_cleanse(ptr, len);
	OPENSSL_free(ptr);
#else
	memset(ptr, 0, len);
	free(ptr);
#endif /* LIBGCRYPT */
}

static inline int jent_fips_enabled(void)
{
#ifdef LIBGCRYPT
	return fips_mode();
#elif defined(OPENSSL)
#ifdef OPENSSL_FIPS
	return FIPS_mode();
#else
	return 0;
#endif
#else
#define FIPS_MODE_SWITCH_FILE "/proc/sys/crypto/fips_enabled"
	char buf[2] = "0";
	int fd = 0;

	if ((fd = open(FIPS_MODE_SWITCH_FILE, O_RDONLY)) >= 0) {
		while (read(fd, buf, sizeof(buf)) < 0 && errno == EINTR);
		close(fd);
	}
	if (buf[0] == '1')
		return 1;
	else
		return 0;
#endif
}

static inline void jent_memset_secure(void *s, size_t n)
{
	memset(s, 0, n);
	__asm__ __volatile__("" : : "r" (s) : "memory");
}

/* --- helpers needed in user space -- */

static inline uint64_t rol64(uint64_t x, int n)
{
	return ( (x << (n&(64-1))) | (x >> ((64-n)&(64-1))) );
}

#endif /* _JITTERENTROPY_BASE_USER_H */
