#ifndef __ASM_SECURE_H
#define __ASM_SECURE_H

#include <config.h>

#define __secure __attribute__ ((section ("._secure.text")))
#define __secure_data __attribute__ ((section ("._secure.data")))

#ifndef __ASSEMBLY__

typedef struct secure_svc_tbl {
	u32	id;
#ifdef CONFIG_ARMV8_PSCI
	u8	pad[4];
#endif
	void	*func;
} secure_svc_tbl_t;

/*
 * Macro to declare a SiP function service in '_secure_svc_tbl_entries' section
 */
#define DECLARE_SECURE_SVC(_name, _id, _fn) \
	static const secure_svc_tbl_t __secure_svc_ ## _name \
		__attribute__((used, section("._secure_svc_tbl_entries"))) \
			 = { \
				.id = _id, \
				.func = _fn }

#else

#ifdef CONFIG_ARMV8_PSCI
#define SECURE_SVC_TBL_OFFSET		16
#else
#define SECURE_SVC_TBL_OFFSET		8

#endif

#endif /* __ASSEMBLY__ */

#if defined(CONFIG_ARMV7_SECURE_BASE) || defined(CONFIG_ARMV8_SECURE_BASE)
/*
 * Warning, horror ahead.
 *
 * The target code lives in our "secure ram", but u-boot doesn't know
 * that, and has blindly added reloc_off to every relocation
 * entry. Gahh. Do the opposite conversion. This hack also prevents
 * GCC from generating code veeners, which u-boot doesn't relocate at
 * all...
 */
#define secure_ram_addr(_fn) ({						\
			DECLARE_GLOBAL_DATA_PTR;			\
			void *__fn = _fn;				\
			typeof(_fn) *__tmp = (__fn - gd->reloc_off);	\
			__tmp;						\
		})
#else
#define secure_ram_addr(_fn)	(_fn)
#endif

#endif
