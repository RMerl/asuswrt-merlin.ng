/*
 * BPF Jit compiler defines
 *
 * Copyright IBM Corp. 2012,2015
 *
 * Author(s): Martin Schwidefsky <schwidefsky@de.ibm.com>
 *	      Michael Holzheu <holzheu@linux.vnet.ibm.com>
 */

#ifndef __ARCH_S390_NET_BPF_JIT_H
#define __ARCH_S390_NET_BPF_JIT_H

#ifndef __ASSEMBLY__

#include <linux/filter.h>
#include <linux/types.h>

extern u8 sk_load_word_pos[], sk_load_half_pos[], sk_load_byte_pos[];
extern u8 sk_load_word[], sk_load_half[], sk_load_byte[];

#endif /* __ASSEMBLY__ */

/*
 * Stackframe layout (packed stack):
 *
 *				    ^ high
 *	      +---------------+     |
 *	      | old backchain |     |
 *	      +---------------+     |
 *	      |   r15 - r6    |     |
 * BFP	   -> +===============+     |
 *	      |		      |     |
 *	      |   BPF stack   |     |
 *	      |		      |     |
 *	      +---------------+     |
 *	      | 8 byte hlen   |     |
 * R15+168 -> +---------------+     |
 *	      | 4 byte align  |     |
 *	      +---------------+     |
 *	      | 4 byte temp   |     |
 *	      | for bpf_jit.S |     |
 * R15+160 -> +---------------+     |
 *	      | new backchain |     |
 * R15+152 -> +---------------+     |
 *	      | + 152 byte SA |     |
 * R15	   -> +---------------+     + low
 *
 * We get 160 bytes stack space from calling function, but only use
 * 11 * 8 byte (old backchain + r15 - r6) for storing registers.
 */
#define STK_SPACE	(MAX_BPF_STACK + 8 + 4 + 4 + 160)
#define STK_160_UNUSED	(160 - 11 * 8)
#define STK_OFF		(STK_SPACE - STK_160_UNUSED)
#define STK_OFF_TMP	160	/* Offset of tmp buffer on stack */
#define STK_OFF_HLEN	168	/* Offset of SKB header length on stack */

/* Offset to skip condition code check */
#define OFF_OK		4

#endif /* __ARCH_S390_NET_BPF_JIT_H */
