#ifndef _ASM_ARM_UNALIGNED_H
#define _ASM_ARM_UNALIGNED_H

#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/generic.h>

/*
 * Select endianness
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define get_unaligned	__get_unaligned_le
#define put_unaligned	__put_unaligned_le
#else
#define get_unaligned	__get_unaligned_be
#define put_unaligned	__put_unaligned_be
#endif

#endif /* _ASM_ARM_UNALIGNED_H */
