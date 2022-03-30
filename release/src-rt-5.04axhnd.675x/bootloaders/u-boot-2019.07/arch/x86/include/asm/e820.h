#ifndef _ASM_X86_E820_H
#define _ASM_X86_E820_H

#define E820MAX		128	/* number of entries in E820MAP */

#define E820_RAM	1
#define E820_RESERVED	2
#define E820_ACPI	3
#define E820_NVS	4
#define E820_UNUSABLE	5

#ifndef __ASSEMBLY__
#include <linux/types.h>

struct e820_entry {
	__u64 addr;	/* start of memory segment */
	__u64 size;	/* size of memory segment */
	__u32 type;	/* type of memory segment */
} __attribute__((packed));

#define ISA_START_ADDRESS	0xa0000
#define ISA_END_ADDRESS		0x100000

#endif /* __ASSEMBLY__ */

/* Implementation defined function to install an e820 map */
unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *);

#endif /* _ASM_X86_E820_H */
