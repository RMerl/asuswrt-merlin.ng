/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __ELL_UTIL_H
#define __ELL_UTIL_H

#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <endian.h>
#include <byteswap.h>
#include <sys/uio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define l_container_of(ptr, type, member) ({				\
_Pragma("GCC diagnostic push")						\
_Pragma("GCC diagnostic ignored \"-Wcast-align\"")			\
		const __typeof__(((type *) 0)->member) *__mptr = (ptr);	\
		(type *)((char *) __mptr - offsetof(type, member));	\
_Pragma("GCC diagnostic pop")						\
	})

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define L_STRINGIFY(val) L_STRINGIFY_ARG(val)
#define L_STRINGIFY_ARG(contents) #contents

#define L_WARN_ON(condition) __extension__ ({				\
		bool r = !!(condition);					\
		if (unlikely(r))					\
			l_warn("WARNING: %s:%s() condition %s failed",	\
				__FILE__, __func__,			\
				#condition);				\
		unlikely(r);						\
	})

#define L_PTR_TO_UINT(p) ((unsigned int) ((uintptr_t) (p)))
#define L_UINT_TO_PTR(u) ((void *) ((uintptr_t) (u)))

#define L_PTR_TO_INT(p) ((int) ((intptr_t) (p)))
#define L_INT_TO_PTR(u) ((void *) ((intptr_t) (u)))

#define L_GET_UNALIGNED(ptr) __extension__	\
({						\
	struct __attribute__((packed)) {	\
            __typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v;				\
})

#define L_PUT_UNALIGNED(val, ptr)		\
do {						\
	struct __attribute__((packed)) {	\
		__typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v = (val);			\
} while(0)

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define L_LE16_TO_CPU(val) (val)
#define L_LE32_TO_CPU(val) (val)
#define L_LE64_TO_CPU(val) (val)
#define L_CPU_TO_LE16(val) (val)
#define L_CPU_TO_LE32(val) (val)
#define L_CPU_TO_LE64(val) (val)
#define L_BE16_TO_CPU(val) bswap_16(val)
#define L_BE32_TO_CPU(val) bswap_32(val)
#define L_BE64_TO_CPU(val) bswap_64(val)
#define L_CPU_TO_BE16(val) bswap_16(val)
#define L_CPU_TO_BE32(val) bswap_32(val)
#define L_CPU_TO_BE64(val) bswap_64(val)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define L_LE16_TO_CPU(val) bswap_16(val)
#define L_LE32_TO_CPU(val) bswap_32(val)
#define L_LE64_TO_CPU(val) bswap_64(val)
#define L_CPU_TO_LE16(val) bswap_16(val)
#define L_CPU_TO_LE32(val) bswap_32(val)
#define L_CPU_TO_LE64(val) bswap_64(val)
#define L_BE16_TO_CPU(val) (val)
#define L_BE32_TO_CPU(val) (val)
#define L_BE64_TO_CPU(val) (val)
#define L_CPU_TO_BE16(val) (val)
#define L_CPU_TO_BE32(val) (val)
#define L_CPU_TO_BE64(val) (val)
#else
#error "Unknown byte order"
#endif

#if __STDC_VERSION__ <= 199409L
#define inline __inline__
#endif

static inline uint8_t l_get_u8(const void *ptr)
{
	return *((const uint8_t *) ptr);
}

static inline void l_put_u8(uint8_t val, void *ptr)
{
	*((uint8_t *) ptr) = val;
}

static inline uint16_t l_get_u16(const void *ptr)
{
	return L_GET_UNALIGNED((const uint16_t *) ptr);
}

static inline void l_put_u16(uint16_t val, void *ptr)
{
	L_PUT_UNALIGNED(val, (uint16_t *) ptr);
}

static inline uint32_t l_get_u32(const void *ptr)
{
	return L_GET_UNALIGNED((const uint32_t *) ptr);
}

static inline void l_put_u32(uint32_t val, void *ptr)
{
	L_PUT_UNALIGNED(val, (uint32_t *) ptr);
}

static inline uint64_t l_get_u64(const void *ptr)
{
	return L_GET_UNALIGNED((const uint64_t *) ptr);
}

static inline void l_put_u64(uint64_t val, void *ptr)
{
	L_PUT_UNALIGNED(val, (uint64_t *) ptr);
}

static inline int16_t l_get_s16(const void *ptr)
{
	return L_GET_UNALIGNED((const int16_t *) ptr);
}

static inline int32_t l_get_s32(const void *ptr)
{
	return L_GET_UNALIGNED((const int32_t *) ptr);
}

static inline int64_t l_get_s64(const void *ptr)
{
	return L_GET_UNALIGNED((const int64_t *) ptr);
}

static inline uint16_t l_get_le16(const void *ptr)
{
	return L_LE16_TO_CPU(L_GET_UNALIGNED((const uint16_t *) ptr));
}

static inline uint16_t l_get_be16(const void *ptr)
{
	return L_BE16_TO_CPU(L_GET_UNALIGNED((const uint16_t *) ptr));
}

static inline uint32_t l_get_le32(const void *ptr)
{
	return L_LE32_TO_CPU(L_GET_UNALIGNED((const uint32_t *) ptr));
}

static inline uint32_t l_get_be32(const void *ptr)
{
	return L_BE32_TO_CPU(L_GET_UNALIGNED((const uint32_t *) ptr));
}

static inline uint64_t l_get_le64(const void *ptr)
{
	return L_LE64_TO_CPU(L_GET_UNALIGNED((const uint64_t *) ptr));
}

static inline uint64_t l_get_be64(const void *ptr)
{
	return L_BE64_TO_CPU(L_GET_UNALIGNED((const uint64_t *) ptr));
}

static inline void l_put_le16(uint16_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_LE16(val), (uint16_t *) ptr);
}

static inline void l_put_be16(uint16_t val, const void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_BE16(val), (uint16_t *) ptr);
}

static inline void l_put_le32(uint32_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_LE32(val), (uint32_t *) ptr);
}

static inline void l_put_be32(uint32_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_BE32(val), (uint32_t *) ptr);
}

static inline void l_put_le64(uint64_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_LE64(val), (uint64_t *) ptr);
}

static inline void l_put_be64(uint64_t val, void *ptr)
{
	L_PUT_UNALIGNED(L_CPU_TO_BE64(val), (uint64_t *) ptr);
}

#define L_AUTO_CLEANUP_VAR(vartype,varname,destroy) \
	vartype varname __attribute__((cleanup(destroy)))

#define L_AUTO_FREE_VAR(vartype,varname) \
	vartype varname __attribute__((cleanup(auto_free)))

#define L_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void *l_malloc(size_t size) __attribute__ ((warn_unused_result, malloc));
void *l_memdup(const void *mem, size_t size)
			__attribute__ ((warn_unused_result, malloc));
void l_free(void *ptr);

void *l_realloc(void *mem, size_t size)
			__attribute__ ((warn_unused_result, malloc));

static inline void auto_free(void *a)
{
	void **p = (void **)a;
	l_free(*p);
}

static inline size_t minsize(size_t a, size_t b)
{
	if (a <= b)
		return a;

	return b;
}

/**
 * l_new:
 * @type: type of structure
 * @count: amount of structures
 *
 * Returns: pointer to allocated memory
 **/
#define l_new(type, count)			\
	(type *) (__extension__ ({		\
		size_t __n = (size_t) (count);	\
		size_t __s = sizeof(type);	\
		void *__p;			\
		__p = l_malloc(__n * __s);	\
		memset(__p, 0, __n * __s);	\
		__p;				\
	}))

char *l_strdup(const char *str);
char *l_strndup(const char *str, size_t max);
char *l_strdup_printf(const char *format, ...)
			__attribute__((format(printf, 1, 2)));
char *l_strdup_vprintf(const char *format, va_list args)
			__attribute__((format(printf, 1, 0)));

size_t l_strlcpy(char* dst, const char *src, size_t len);

bool l_str_has_prefix(const char *str, const char *prefix);
bool l_str_has_suffix(const char *str, const char *suffix);

char *l_util_hexstring(const unsigned char *buf, size_t len);
char *l_util_hexstring_upper(const unsigned char *buf, size_t len);
unsigned char *l_util_from_hexstring(const char *str, size_t *out_len);

typedef void (*l_util_hexdump_func_t) (const char *str, void *user_data);

void l_util_hexdump(bool in, const void *buf, size_t len,
			l_util_hexdump_func_t function, void *user_data);
void l_util_hexdump_two(bool in, const void *buf1, size_t len1,
			const void *buf2, size_t len2,
			l_util_hexdump_func_t function, void *user_data);
void l_util_hexdumpv(bool in, const struct iovec *iov, size_t n_iov,
					l_util_hexdump_func_t function,
					void *user_data);
void l_util_debug(l_util_hexdump_func_t function, void *user_data,
						const char *format, ...)
			__attribute__((format(printf, 3, 4)));

const char *l_util_get_debugfs_path(void);

#define L_TFR(expression)                          \
  (__extension__                                   \
    ({ long int __result;                          \
       do __result = (long int) (expression);      \
       while (__result == -1L && errno == EINTR);  \
       __result; }))

#define _L_IN_SET_CMP(val, type, cmp, ...) __extension__ ({		\
		const type __v = (val);					\
		const typeof(__v) __elems[] = {__VA_ARGS__};		\
		unsigned int __i;					\
		static const unsigned int __n = L_ARRAY_SIZE(__elems);	\
		bool __r = false;					\
		for (__i = 0; __i < __n && !__r; __i++)			\
			__r = (cmp);					\
		__r;							\
	})

/* Warning: evaluates all set elements even after @val has matched one */
#define L_IN_SET(val, ...)	\
	_L_IN_SET_CMP((val), __auto_type, __v == __elems[__i], ##__VA_ARGS__)

#define L_IN_STRSET(val, ...)						\
	_L_IN_SET_CMP((val), const char *, __v == __elems[__i] ||	\
				(__v && __elems[__i] &&			\
				 !strcmp(__v, __elems[__i])), ##__VA_ARGS__)

/*
 * Taken from https://github.com/chmike/cst_time_memcmp, adding a volatile to
 * ensure the compiler does not try to optimize the constant time behavior.
 * The code has been modified to add comments and project specific code
 * styling.
 * This specific piece of code is subject to the following copyright:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Christophe Meessen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * This function performs a secure memory comparison of two buffers of size
 * bytes, representing an integer (byte order is big endian). It returns
 * a negative, zero or positif value if a < b, a == b or a > b respectively.
 */
static inline int l_secure_memcmp(const void *a, const void *b,
					size_t size)
{
	const volatile uint8_t *aa = a;
	const volatile uint8_t *bb = b;
	int res = 0, diff, mask;

	/*
	 * We will compare all bytes, starting with the less significant. When
	 * we find a non-zero difference, we update the result accordingly.
	 */
	if (size > 0) {
		/*
		 * The following couple of lines can be summarized as a
		 * constant time/memory access version of:
		 * if (diff != 0) res = diff;
		 *
		 * From the previous operation, we know that diff is in
		 * [-255, 255]
		 *
		 * The following figure show the possible value of mask, based
		 * on different cases of diff:
		 *
		 * diff  |   diff-1   |   ~diff    | ((diff-1) & ~diff) |  mask
		 * ------|------------|------------|--------------------|------
		 *   < 0 | 0xFFFFFFXX | 0x000000YY |     0x000000ZZ     |   0
		 *  == 0 | 0xFFFFFFFF | 0xFFFFFFFF |     0xFFFFFFFF     | 0xF..F
		 *  > 0  | 0x000000XX | 0xFFFFFFYY |     0x000000ZZ     |   0
		 *
		 * Hence, the mask allows to keep res when diff == 0, and to
		 * set res to diff otherwise.
		*/
		do {
			--size;
			diff = aa[size] - bb[size];
			mask = (((diff - 1) & ~diff) >> 8);
			res = (res & mask) | diff;
		} while (size != 0);
	}

	return res;
}

bool l_memeq(const void *field, size_t size, uint8_t byte);
bool l_secure_memeq(const void *field, size_t size, uint8_t byte);

static inline bool l_memeqzero(const void *field, size_t size)
{
	return l_memeq(field, size, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* __ELL_UTIL_H */
