#ifndef _GENERIC_UNALIGNED_H
#define _GENERIC_UNALIGNED_H

#include <asm/byteorder.h>

#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/generic.h>

/*
 * Select endianness
 */
#if defined(__LITTLE_ENDIAN)
#define get_unaligned	__get_unaligned_le
#define put_unaligned	__put_unaligned_le
#elif defined(__BIG_ENDIAN)
#define get_unaligned	__get_unaligned_be
#define put_unaligned	__put_unaligned_be
#else
#error invalid endian
#endif

/* Allow unaligned memory access */
void allow_unaligned(void);

#endif
