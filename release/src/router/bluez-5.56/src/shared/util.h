/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>
#include <byteswap.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define BIT(n)  (1 << (n))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le16_to_cpu(val) (val)
#define le32_to_cpu(val) (val)
#define le64_to_cpu(val) (val)
#define cpu_to_le16(val) (val)
#define cpu_to_le32(val) (val)
#define cpu_to_le64(val) (val)
#define be16_to_cpu(val) bswap_16(val)
#define be32_to_cpu(val) bswap_32(val)
#define be64_to_cpu(val) bswap_64(val)
#define cpu_to_be16(val) bswap_16(val)
#define cpu_to_be32(val) bswap_32(val)
#define cpu_to_be64(val) bswap_64(val)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define le16_to_cpu(val) bswap_16(val)
#define le32_to_cpu(val) bswap_32(val)
#define le64_to_cpu(val) bswap_64(val)
#define cpu_to_le16(val) bswap_16(val)
#define cpu_to_le32(val) bswap_32(val)
#define cpu_to_le64(val) bswap_64(val)
#define be16_to_cpu(val) (val)
#define be32_to_cpu(val) (val)
#define be64_to_cpu(val) (val)
#define cpu_to_be16(val) (val)
#define cpu_to_be32(val) (val)
#define cpu_to_be64(val) (val)
#else
#error "Unknown byte order"
#endif

#define get_unaligned(ptr)			\
__extension__ ({				\
	struct __attribute__((packed)) {	\
		__typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v;				\
})

#define put_unaligned(val, ptr)			\
do {						\
	struct __attribute__((packed)) {	\
		__typeof__(*(ptr)) __v;		\
	} *__p = (__typeof__(__p)) (ptr);	\
	__p->__v = (val);			\
} while (0)

#define PTR_TO_UINT(p) ((unsigned int) ((uintptr_t) (p)))
#define UINT_TO_PTR(u) ((void *) ((uintptr_t) (u)))

#define PTR_TO_INT(p) ((int) ((intptr_t) (p)))
#define INT_TO_PTR(u) ((void *) ((intptr_t) (u)))

#define new0(type, count)			\
	(type *) (__extension__ ({		\
		size_t __n = (size_t) (count);	\
		size_t __s = sizeof(type);	\
		void *__p;			\
		__p = btd_malloc(__n * __s);	\
		memset(__p, 0, __n * __s);	\
		__p;				\
	}))

#define newa(t, n) ((t*) alloca(sizeof(t)*(n)))
#define malloc0(n) (calloc((n), 1))

char *strdelimit(char *str, char *del, char c);
int strsuffix(const char *str, const char *suffix);

void *btd_malloc(size_t size);

typedef void (*util_debug_func_t)(const char *str, void *user_data);

void util_debug_va(util_debug_func_t function, void *user_data,
				const char *format, va_list va);

void util_debug(util_debug_func_t function, void *user_data,
						const char *format, ...)
					__attribute__((format(printf, 3, 4)));

void util_hexdump(const char dir, const unsigned char *buf, size_t len,
				util_debug_func_t function, void *user_data);

unsigned char util_get_dt(const char *parent, const char *name);

uint8_t util_get_uid(unsigned int *bitmap, uint8_t max);
void util_clear_uid(unsigned int *bitmap, uint8_t id);

const char *bt_uuid16_to_str(uint16_t uuid);
const char *bt_uuid32_to_str(uint32_t uuid);
const char *bt_uuidstr_to_str(const char *uuid);
const char *bt_appear_to_str(uint16_t appearance);

static inline int8_t get_s8(const void *ptr)
{
	return *((int8_t *) ptr);
}

static inline uint8_t get_u8(const void *ptr)
{
	return *((uint8_t *) ptr);
}

static inline uint16_t get_le16(const void *ptr)
{
	return le16_to_cpu(get_unaligned((const uint16_t *) ptr));
}

static inline uint16_t get_be16(const void *ptr)
{
	return be16_to_cpu(get_unaligned((const uint16_t *) ptr));
}

static inline uint32_t get_le32(const void *ptr)
{
	return le32_to_cpu(get_unaligned((const uint32_t *) ptr));
}

static inline uint32_t get_be32(const void *ptr)
{
	return be32_to_cpu(get_unaligned((const uint32_t *) ptr));
}

static inline uint64_t get_le64(const void *ptr)
{
	return le64_to_cpu(get_unaligned((const uint64_t *) ptr));
}

static inline uint64_t get_be64(const void *ptr)
{
	return be64_to_cpu(get_unaligned((const uint64_t *) ptr));
}

static inline void put_le16(uint16_t val, void *dst)
{
	put_unaligned(cpu_to_le16(val), (uint16_t *) dst);
}

static inline void put_be16(uint16_t val, const void *ptr)
{
	put_unaligned(cpu_to_be16(val), (uint16_t *) ptr);
}

static inline void put_le32(uint32_t val, void *dst)
{
	put_unaligned(cpu_to_le32(val), (uint32_t *) dst);
}

static inline void put_be32(uint32_t val, void *dst)
{
	put_unaligned(cpu_to_be32(val), (uint32_t *) dst);
}

static inline void put_le64(uint64_t val, void *dst)
{
	put_unaligned(cpu_to_le64(val), (uint64_t *) dst);
}

static inline void put_be64(uint64_t val, void *dst)
{
	put_unaligned(cpu_to_be64(val), (uint64_t *) dst);
}
