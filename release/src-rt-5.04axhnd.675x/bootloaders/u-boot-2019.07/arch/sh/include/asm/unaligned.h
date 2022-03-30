#ifndef _ASM_SH_UNALIGNED_H
#define _ASM_SH_UNALIGNED_H

/* Copy from linux-kernel. */

#ifdef CONFIG_CPU_SH4A
/* SH-4A can handle unaligned loads in a relatively neutered fashion. */
#include <asm/unaligned-sh4a.h>
#else
/* Otherwise, SH can't handle unaligned accesses. */
#include <linux/compiler.h>
#if defined(__BIG_ENDIAN__)
#define get_unaligned   __get_unaligned_be
#define put_unaligned   __put_unaligned_be
#elif defined(__LITTLE_ENDIAN__)
#define get_unaligned   __get_unaligned_le
#define put_unaligned   __put_unaligned_le
#endif

#include <linux/unaligned/le_byteshift.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/generic.h>
#endif

#endif /* _ASM_SH_UNALIGNED_H */
