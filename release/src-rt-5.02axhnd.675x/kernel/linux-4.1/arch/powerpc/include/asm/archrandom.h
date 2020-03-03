#ifndef _ASM_POWERPC_ARCHRANDOM_H
#define _ASM_POWERPC_ARCHRANDOM_H

#ifdef CONFIG_ARCH_RANDOM

#include <asm/machdep.h>

static inline int arch_get_random_long(unsigned long *v)
{
	if (ppc_md.get_random_long)
		return ppc_md.get_random_long(v);

	return 0;
}

static inline int arch_get_random_int(unsigned int *v)
{
	unsigned long val;
	int rc;

	rc = arch_get_random_long(&val);
	if (rc)
		*v = val;

	return rc;
}

static inline int arch_has_random(void)
{
	return !!ppc_md.get_random_long;
}

static inline int arch_get_random_seed_long(unsigned long *v)
{
	return 0;
}
static inline int arch_get_random_seed_int(unsigned int *v)
{
	return 0;
}
static inline int arch_has_random_seed(void)
{
	return 0;
}

#endif /* CONFIG_ARCH_RANDOM */

#ifdef CONFIG_PPC_POWERNV
int powernv_hwrng_present(void);
int powernv_get_random_long(unsigned long *v);
int powernv_get_random_real_mode(unsigned long *v);
#else
static inline int powernv_hwrng_present(void) { return 0; }
static inline int powernv_get_random_real_mode(unsigned long *v) { return 0; }
#endif

#endif /* _ASM_POWERPC_ARCHRANDOM_H */
