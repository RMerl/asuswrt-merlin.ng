/*
 * This was automagically generated from arch/nds/tools/mach-types!
 * Do NOT edit
 */

#ifndef __ASM_NDS32_MACH_TYPE_H
#define __ASM_NDS32_MACH_TYPE_H

#ifndef __ASSEMBLY__
/* The type of machine we're running on */
extern unsigned int __machine_arch_type;
#endif

/* see arch/arm/kernel/arch.c for a description of these */
#define MACH_TYPE_ADPAG101P            1
#define MACH_TYPE_ADPAE3XX	2

#ifdef CONFIG_ARCH_ADPAG101P
# ifdef machine_arch_type
#  undef machine_arch_type
#  define machine_arch_type	__machine_arch_type
# else
#  define machine_arch_type	MACH_TYPE_ADPAG101P
# endif
# define machine_is_adpag101p()	(machine_arch_type == MACH_TYPE_ADPAG101P)
#else
# define machine_is_adpag101p()	(1)
#endif

#endif /* __ASM_NDS32_MACH_TYPE_H */
