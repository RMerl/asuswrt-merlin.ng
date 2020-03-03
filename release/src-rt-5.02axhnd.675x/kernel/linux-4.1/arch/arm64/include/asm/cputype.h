/*
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_CPUTYPE_H
#define __ASM_CPUTYPE_H

#define INVALID_HWID		ULONG_MAX

#define MPIDR_UP_BITMASK	(0x1 << 30)
#define MPIDR_MT_BITMASK	(0x1 << 24)
#define MPIDR_HWID_BITMASK	0xff00ffffff

#define MPIDR_LEVEL_BITS_SHIFT	3
#define MPIDR_LEVEL_BITS	(1 << MPIDR_LEVEL_BITS_SHIFT)
#define MPIDR_LEVEL_MASK	((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
	(((1 << level) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
	((mpidr >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

#define read_cpuid(reg) ({						\
	u64 __val;							\
	asm("mrs	%0, " #reg : "=r" (__val));			\
	__val;								\
})

#define MIDR_REVISION_MASK	0xf
#define MIDR_REVISION(midr)	((midr) & MIDR_REVISION_MASK)
#define MIDR_PARTNUM_SHIFT	4
#define MIDR_PARTNUM_MASK	(0xfff << MIDR_PARTNUM_SHIFT)
#define MIDR_PARTNUM(midr)	\
	(((midr) & MIDR_PARTNUM_MASK) >> MIDR_PARTNUM_SHIFT)
#define MIDR_ARCHITECTURE_SHIFT	16
#define MIDR_ARCHITECTURE_MASK	(0xf << MIDR_ARCHITECTURE_SHIFT)
#define MIDR_ARCHITECTURE(midr)	\
	(((midr) & MIDR_ARCHITECTURE_MASK) >> MIDR_ARCHITECTURE_SHIFT)
#define MIDR_VARIANT_SHIFT	20
#define MIDR_VARIANT_MASK	(0xf << MIDR_VARIANT_SHIFT)
#define MIDR_VARIANT(midr)	\
	(((midr) & MIDR_VARIANT_MASK) >> MIDR_VARIANT_SHIFT)
#define MIDR_IMPLEMENTOR_SHIFT	24
#define MIDR_IMPLEMENTOR_MASK	(0xff << MIDR_IMPLEMENTOR_SHIFT)
#define MIDR_IMPLEMENTOR(midr)	\
	(((midr) & MIDR_IMPLEMENTOR_MASK) >> MIDR_IMPLEMENTOR_SHIFT)

#define MIDR_CPU_PART(imp, partnum) \
	(((imp)			<< MIDR_IMPLEMENTOR_SHIFT) | \
	(0xf			<< MIDR_ARCHITECTURE_SHIFT) | \
	((partnum)		<< MIDR_PARTNUM_SHIFT))

#define ARM_CPU_IMP_ARM		0x41
#define ARM_CPU_IMP_APM		0x50
#if defined CONFIG_BCM_KF_ARM64_BCM963XX
#define ARM_CPU_IMP_BRCM	0x42
#endif

#define ARM_CPU_PART_AEM_V8	0xD0F
#define ARM_CPU_PART_FOUNDATION	0xD00
#define ARM_CPU_PART_CORTEX_A57	0xD07
#define ARM_CPU_PART_CORTEX_A53	0xD03

#define APM_CPU_PART_POTENZA	0x000
#if defined CONFIG_BCM_KF_ARM64_BCM963XX
#define ARM_CPU_PART_CORTEX_B53	0x100
#endif

#define ID_AA64MMFR0_BIGENDEL0_SHIFT	16
#define ID_AA64MMFR0_BIGENDEL0_MASK	(0xf << ID_AA64MMFR0_BIGENDEL0_SHIFT)
#define ID_AA64MMFR0_BIGENDEL0(mmfr0)	\
	(((mmfr0) & ID_AA64MMFR0_BIGENDEL0_MASK) >> ID_AA64MMFR0_BIGENDEL0_SHIFT)
#define ID_AA64MMFR0_BIGEND_SHIFT	8
#define ID_AA64MMFR0_BIGEND_MASK	(0xf << ID_AA64MMFR0_BIGEND_SHIFT)
#define ID_AA64MMFR0_BIGEND(mmfr0)	\
	(((mmfr0) & ID_AA64MMFR0_BIGEND_MASK) >> ID_AA64MMFR0_BIGEND_SHIFT)

#define SCTLR_EL1_CP15BEN	(0x1 << 5)
#define SCTLR_EL1_SED		(0x1 << 8)

#ifndef __ASSEMBLY__

/*
 * The CPU ID never changes at run time, so we might as well tell the
 * compiler that it's constant.  Use this function to read the CPU ID
 * rather than directly reading processor_id or read_cpuid() directly.
 */
static inline u32 __attribute_const__ read_cpuid_id(void)
{
	return read_cpuid(MIDR_EL1);
}

static inline u64 __attribute_const__ read_cpuid_mpidr(void)
{
	return read_cpuid(MPIDR_EL1);
}

static inline unsigned int __attribute_const__ read_cpuid_implementor(void)
{
	return MIDR_IMPLEMENTOR(read_cpuid_id());
}

static inline unsigned int __attribute_const__ read_cpuid_part_number(void)
{
	return MIDR_PARTNUM(read_cpuid_id());
}

static inline u32 __attribute_const__ read_cpuid_cachetype(void)
{
	return read_cpuid(CTR_EL0);
}

static inline bool id_aa64mmfr0_mixed_endian_el0(u64 mmfr0)
{
	return (ID_AA64MMFR0_BIGEND(mmfr0) == 0x1) ||
		(ID_AA64MMFR0_BIGENDEL0(mmfr0) == 0x1);
}
#endif /* __ASSEMBLY__ */

#endif
