/*
 * Processor capabilities determination functions.
 *
 * Copyright (C) xxxx  the Anonymous
 * Copyright (C) 1994 - 2006 Ralf Baechle
 * Copyright (C) 2003, 2004  Maciej W. Rozycki
 * Copyright (C) 2001, 2004, 2011, 2012	 MIPS Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ptrace.h>
#include <linux/smp.h>
#include <linux/stddef.h>
#include <linux/export.h>

#include <asm/bugs.h>
#include <asm/cpu.h>
#include <asm/cpu-features.h>
#include <asm/cpu-type.h>
#include <asm/fpu.h>
#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/msa.h>
#include <asm/watch.h>
#include <asm/elf.h>
#include <asm/pgtable-bits.h>
#include <asm/spram.h>
#include <asm/uaccess.h>

/*
 * Get the FPU Implementation/Revision.
 */
static inline unsigned long cpu_get_fpu_id(void)
{
	unsigned long tmp, fpu_id;

	tmp = read_c0_status();
	__enable_fpu(FPU_AS_IS);
	fpu_id = read_32bit_cp1_register(CP1_REVISION);
	write_c0_status(tmp);
	return fpu_id;
}

/*
 * Check if the CPU has an external FPU.
 */
static inline int __cpu_has_fpu(void)
{
	return (cpu_get_fpu_id() & FPIR_IMP_MASK) != FPIR_IMP_NONE;
}

static inline unsigned long cpu_get_msa_id(void)
{
	unsigned long status, msa_id;

	status = read_c0_status();
	__enable_fpu(FPU_64BIT);
	enable_msa();
	msa_id = read_msa_ir();
	disable_msa();
	write_c0_status(status);
	return msa_id;
}

/*
 * Determine the FCSR mask for FPU hardware.
 */
static inline void cpu_set_fpu_fcsr_mask(struct cpuinfo_mips *c)
{
	unsigned long sr, mask, fcsr, fcsr0, fcsr1;

	fcsr = c->fpu_csr31;
	mask = FPU_CSR_ALL_X | FPU_CSR_ALL_E | FPU_CSR_ALL_S | FPU_CSR_RM;

	sr = read_c0_status();
	__enable_fpu(FPU_AS_IS);

	fcsr0 = fcsr & mask;
	write_32bit_cp1_register(CP1_STATUS, fcsr0);
	fcsr0 = read_32bit_cp1_register(CP1_STATUS);

	fcsr1 = fcsr | ~mask;
	write_32bit_cp1_register(CP1_STATUS, fcsr1);
	fcsr1 = read_32bit_cp1_register(CP1_STATUS);

	write_32bit_cp1_register(CP1_STATUS, fcsr);

	write_c0_status(sr);

	c->fpu_msk31 = ~(fcsr0 ^ fcsr1) & ~mask;
}

/*
 * Set the FIR feature flags for the FPU emulator.
 */
static void cpu_set_nofpu_id(struct cpuinfo_mips *c)
{
	u32 value;

	value = 0;
	if (c->isa_level & (MIPS_CPU_ISA_M32R1 | MIPS_CPU_ISA_M64R1 |
			    MIPS_CPU_ISA_M32R2 | MIPS_CPU_ISA_M64R2 |
			    MIPS_CPU_ISA_M32R6 | MIPS_CPU_ISA_M64R6))
		value |= MIPS_FPIR_D | MIPS_FPIR_S;
	if (c->isa_level & (MIPS_CPU_ISA_M32R2 | MIPS_CPU_ISA_M64R2 |
			    MIPS_CPU_ISA_M32R6 | MIPS_CPU_ISA_M64R6))
		value |= MIPS_FPIR_F64 | MIPS_FPIR_L | MIPS_FPIR_W;
	c->fpu_id = value;
}

/* Determined FPU emulator mask to use for the boot CPU with "nofpu".  */
static unsigned int mips_nofpu_msk31;

/*
 * Set options for FPU hardware.
 */
static void cpu_set_fpu_opts(struct cpuinfo_mips *c)
{
	c->fpu_id = cpu_get_fpu_id();
	mips_nofpu_msk31 = c->fpu_msk31;

	if (c->isa_level & (MIPS_CPU_ISA_M32R1 | MIPS_CPU_ISA_M64R1 |
			    MIPS_CPU_ISA_M32R2 | MIPS_CPU_ISA_M64R2 |
			    MIPS_CPU_ISA_M32R6 | MIPS_CPU_ISA_M64R6)) {
		if (c->fpu_id & MIPS_FPIR_3D)
			c->ases |= MIPS_ASE_MIPS3D;
		if (c->fpu_id & MIPS_FPIR_FREP)
			c->options |= MIPS_CPU_FRE;
	}

	cpu_set_fpu_fcsr_mask(c);
}

/*
 * Set options for the FPU emulator.
 */
static void cpu_set_nofpu_opts(struct cpuinfo_mips *c)
{
	c->options &= ~MIPS_CPU_FPU;
	c->fpu_msk31 = mips_nofpu_msk31;

	cpu_set_nofpu_id(c);
}

static int mips_fpu_disabled;

static int __init fpu_disable(char *s)
{
	cpu_set_nofpu_opts(&boot_cpu_data);
	mips_fpu_disabled = 1;

	return 1;
}

__setup("nofpu", fpu_disable);

int mips_dsp_disabled;

static int __init dsp_disable(char *s)
{
	cpu_data[0].ases &= ~(MIPS_ASE_DSP | MIPS_ASE_DSP2P);
	mips_dsp_disabled = 1;

	return 1;
}

__setup("nodsp", dsp_disable);

static int mips_htw_disabled;

static int __init htw_disable(char *s)
{
	mips_htw_disabled = 1;
	cpu_data[0].options &= ~MIPS_CPU_HTW;
	write_c0_pwctl(read_c0_pwctl() &
		       ~(1 << MIPS_PWCTL_PWEN_SHIFT));

	return 1;
}

__setup("nohtw", htw_disable);

static int mips_ftlb_disabled;
static int mips_has_ftlb_configured;

static void set_ftlb_enable(struct cpuinfo_mips *c, int enable);

static int __init ftlb_disable(char *s)
{
	unsigned int config4, mmuextdef;

	/*
	 * If the core hasn't done any FTLB configuration, there is nothing
	 * for us to do here.
	 */
	if (!mips_has_ftlb_configured)
		return 1;

	/* Disable it in the boot cpu */
	set_ftlb_enable(&cpu_data[0], 0);

	back_to_back_c0_hazard();

	config4 = read_c0_config4();

	/* Check that FTLB has been disabled */
	mmuextdef = config4 & MIPS_CONF4_MMUEXTDEF;
	/* MMUSIZEEXT == VTLB ON, FTLB OFF */
	if (mmuextdef == MIPS_CONF4_MMUEXTDEF_FTLBSIZEEXT) {
		/* This should never happen */
		pr_warn("FTLB could not be disabled!\n");
		return 1;
	}

	mips_ftlb_disabled = 1;
	mips_has_ftlb_configured = 0;

	/*
	 * noftlb is mainly used for debug purposes so print
	 * an informative message instead of using pr_debug()
	 */
	pr_info("FTLB has been disabled\n");

	/*
	 * Some of these bits are duplicated in the decode_config4.
	 * MIPS_CONF4_MMUEXTDEF_MMUSIZEEXT is the only possible case
	 * once FTLB has been disabled so undo what decode_config4 did.
	 */
	cpu_data[0].tlbsize -= cpu_data[0].tlbsizeftlbways *
			       cpu_data[0].tlbsizeftlbsets;
	cpu_data[0].tlbsizeftlbsets = 0;
	cpu_data[0].tlbsizeftlbways = 0;

	return 1;
}

__setup("noftlb", ftlb_disable);


static inline void check_errata(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	switch (current_cpu_type()) {
	case CPU_34K:
		/*
		 * Erratum "RPS May Cause Incorrect Instruction Execution"
		 * This code only handles VPE0, any SMP/RTOS code
		 * making use of VPE1 will be responsable for that VPE.
		 */
		if ((c->processor_id & PRID_REV_MASK) <= PRID_REV_34K_V1_0_2)
			write_c0_config7(read_c0_config7() | MIPS_CONF7_RPS);
		break;
	default:
		break;
	}
}

void __init check_bugs32(void)
{
	check_errata();
}

/*
 * Probe whether cpu has config register by trying to play with
 * alternate cache bit and see whether it matters.
 * It's used by cpu_probe to distinguish between R3000A and R3081.
 */
static inline int cpu_has_confreg(void)
{
#ifdef CONFIG_CPU_R3000
	extern unsigned long r3k_cache_size(unsigned long);
	unsigned long size1, size2;
	unsigned long cfg = read_c0_conf();

	size1 = r3k_cache_size(ST0_ISC);
	write_c0_conf(cfg ^ R30XX_CONF_AC);
	size2 = r3k_cache_size(ST0_ISC);
	write_c0_conf(cfg);
	return size1 != size2;
#else
	return 0;
#endif
}

static inline void set_elf_platform(int cpu, const char *plat)
{
	if (cpu == 0)
		__elf_platform = plat;
}

static inline void cpu_probe_vmbits(struct cpuinfo_mips *c)
{
#ifdef __NEED_VMBITS_PROBE
	write_c0_entryhi(0x3fffffffffffe000ULL);
	back_to_back_c0_hazard();
	c->vmbits = fls64(read_c0_entryhi() & 0x3fffffffffffe000ULL);
#endif
}

static void set_isa(struct cpuinfo_mips *c, unsigned int isa)
{
	switch (isa) {
	case MIPS_CPU_ISA_M64R2:
		c->isa_level |= MIPS_CPU_ISA_M32R2 | MIPS_CPU_ISA_M64R2;
	case MIPS_CPU_ISA_M64R1:
		c->isa_level |= MIPS_CPU_ISA_M32R1 | MIPS_CPU_ISA_M64R1;
	case MIPS_CPU_ISA_V:
		c->isa_level |= MIPS_CPU_ISA_V;
	case MIPS_CPU_ISA_IV:
		c->isa_level |= MIPS_CPU_ISA_IV;
	case MIPS_CPU_ISA_III:
		c->isa_level |= MIPS_CPU_ISA_II | MIPS_CPU_ISA_III;
		break;

	/* R6 incompatible with everything else */
	case MIPS_CPU_ISA_M64R6:
		c->isa_level |= MIPS_CPU_ISA_M32R6 | MIPS_CPU_ISA_M64R6;
	case MIPS_CPU_ISA_M32R6:
		c->isa_level |= MIPS_CPU_ISA_M32R6;
		/* Break here so we don't add incompatible ISAs */
		break;
	case MIPS_CPU_ISA_M32R2:
		c->isa_level |= MIPS_CPU_ISA_M32R2;
	case MIPS_CPU_ISA_M32R1:
		c->isa_level |= MIPS_CPU_ISA_M32R1;
	case MIPS_CPU_ISA_II:
		c->isa_level |= MIPS_CPU_ISA_II;
		break;
	}
}

static char unknown_isa[] = KERN_ERR \
	"Unsupported ISA type, c0.config0: %d.";

static unsigned int calculate_ftlb_probability(struct cpuinfo_mips *c)
{

	unsigned int probability = c->tlbsize / c->tlbsizevtlb;

	/*
	 * 0 = All TLBWR instructions go to FTLB
	 * 1 = 15:1: For every 16 TBLWR instructions, 15 go to the
	 * FTLB and 1 goes to the VTLB.
	 * 2 = 7:1: As above with 7:1 ratio.
	 * 3 = 3:1: As above with 3:1 ratio.
	 *
	 * Use the linear midpoint as the probability threshold.
	 */
	if (probability >= 12)
		return 1;
	else if (probability >= 6)
		return 2;
	else
		/*
		 * So FTLB is less than 4 times bigger than VTLB.
		 * A 3:1 ratio can still be useful though.
		 */
		return 3;
}

static void set_ftlb_enable(struct cpuinfo_mips *c, int enable)
{
	unsigned int config6;

	/* It's implementation dependent how the FTLB can be enabled */
	switch (c->cputype) {
	case CPU_PROAPTIV:
	case CPU_P5600:
		/* proAptiv & related cores use Config6 to enable the FTLB */
		config6 = read_c0_config6();
		/* Clear the old probability value */
		config6 &= ~(3 << MIPS_CONF6_FTLBP_SHIFT);
		if (enable)
			/* Enable FTLB */
			write_c0_config6(config6 |
					 (calculate_ftlb_probability(c)
					  << MIPS_CONF6_FTLBP_SHIFT)
					 | MIPS_CONF6_FTLBEN);
		else
			/* Disable FTLB */
			write_c0_config6(config6 &  ~MIPS_CONF6_FTLBEN);
		back_to_back_c0_hazard();
		break;
	}
}

static inline unsigned int decode_config0(struct cpuinfo_mips *c)
{
	unsigned int config0;
	int isa;

	config0 = read_c0_config();

	/*
	 * Look for Standard TLB or Dual VTLB and FTLB
	 */
	if ((((config0 & MIPS_CONF_MT) >> 7) == 1) ||
	    (((config0 & MIPS_CONF_MT) >> 7) == 4))
		c->options |= MIPS_CPU_TLB;

	isa = (config0 & MIPS_CONF_AT) >> 13;
	switch (isa) {
	case 0:
		switch ((config0 & MIPS_CONF_AR) >> 10) {
		case 0:
			set_isa(c, MIPS_CPU_ISA_M32R1);
			break;
		case 1:
			set_isa(c, MIPS_CPU_ISA_M32R2);
			break;
		case 2:
			set_isa(c, MIPS_CPU_ISA_M32R6);
			break;
		default:
			goto unknown;
		}
		break;
	case 2:
		switch ((config0 & MIPS_CONF_AR) >> 10) {
		case 0:
			set_isa(c, MIPS_CPU_ISA_M64R1);
			break;
		case 1:
			set_isa(c, MIPS_CPU_ISA_M64R2);
			break;
		case 2:
			set_isa(c, MIPS_CPU_ISA_M64R6);
			break;
		default:
			goto unknown;
		}
		break;
	default:
		goto unknown;
	}

	return config0 & MIPS_CONF_M;

unknown:
	panic(unknown_isa, config0);
}

static inline unsigned int decode_config1(struct cpuinfo_mips *c)
{
	unsigned int config1;

	config1 = read_c0_config1();

	if (config1 & MIPS_CONF1_MD)
		c->ases |= MIPS_ASE_MDMX;
	if (config1 & MIPS_CONF1_WR)
		c->options |= MIPS_CPU_WATCH;
	if (config1 & MIPS_CONF1_CA)
		c->ases |= MIPS_ASE_MIPS16;
	if (config1 & MIPS_CONF1_EP)
		c->options |= MIPS_CPU_EJTAG;
	if (config1 & MIPS_CONF1_FP) {
		c->options |= MIPS_CPU_FPU;
		c->options |= MIPS_CPU_32FPR;
	}
	if (cpu_has_tlb) {
		c->tlbsize = ((config1 & MIPS_CONF1_TLBS) >> 25) + 1;
		c->tlbsizevtlb = c->tlbsize;
		c->tlbsizeftlbsets = 0;
	}

	return config1 & MIPS_CONF_M;
}

static inline unsigned int decode_config2(struct cpuinfo_mips *c)
{
	unsigned int config2;

	config2 = read_c0_config2();

	if (config2 & MIPS_CONF2_SL)
		c->scache.flags &= ~MIPS_CACHE_NOT_PRESENT;

	return config2 & MIPS_CONF_M;
}

static inline unsigned int decode_config3(struct cpuinfo_mips *c)
{
	unsigned int config3;

	config3 = read_c0_config3();

	if (config3 & MIPS_CONF3_SM) {
		c->ases |= MIPS_ASE_SMARTMIPS;
		c->options |= MIPS_CPU_RIXI;
	}
	if (config3 & MIPS_CONF3_RXI)
		c->options |= MIPS_CPU_RIXI;
	if (config3 & MIPS_CONF3_DSP)
		c->ases |= MIPS_ASE_DSP;
	if (config3 & MIPS_CONF3_DSP2P)
		c->ases |= MIPS_ASE_DSP2P;
	if (config3 & MIPS_CONF3_VINT)
		c->options |= MIPS_CPU_VINT;
	if (config3 & MIPS_CONF3_VEIC)
		c->options |= MIPS_CPU_VEIC;
	if (config3 & MIPS_CONF3_MT)
		c->ases |= MIPS_ASE_MIPSMT;
	if (config3 & MIPS_CONF3_ULRI)
		c->options |= MIPS_CPU_ULRI;
	if (config3 & MIPS_CONF3_ISA)
		c->options |= MIPS_CPU_MICROMIPS;
	if (config3 & MIPS_CONF3_VZ)
		c->ases |= MIPS_ASE_VZ;
	if (config3 & MIPS_CONF3_SC)
		c->options |= MIPS_CPU_SEGMENTS;
	if (config3 & MIPS_CONF3_MSA)
		c->ases |= MIPS_ASE_MSA;
	/* Only tested on 32-bit cores */
	if ((config3 & MIPS_CONF3_PW) && config_enabled(CONFIG_32BIT)) {
		c->htw_seq = 0;
		c->options |= MIPS_CPU_HTW;
	}
	if (config3 & MIPS_CONF3_CDMM)
		c->options |= MIPS_CPU_CDMM;

	return config3 & MIPS_CONF_M;
}

static inline unsigned int decode_config4(struct cpuinfo_mips *c)
{
	unsigned int config4;
	unsigned int newcf4;
	unsigned int mmuextdef;
	unsigned int ftlb_page = MIPS_CONF4_FTLBPAGESIZE;

	config4 = read_c0_config4();

	if (cpu_has_tlb) {
		if (((config4 & MIPS_CONF4_IE) >> 29) == 2)
			c->options |= MIPS_CPU_TLBINV;
		mmuextdef = config4 & MIPS_CONF4_MMUEXTDEF;
		switch (mmuextdef) {
		case MIPS_CONF4_MMUEXTDEF_MMUSIZEEXT:
			c->tlbsize += (config4 & MIPS_CONF4_MMUSIZEEXT) * 0x40;
			c->tlbsizevtlb = c->tlbsize;
			break;
		case MIPS_CONF4_MMUEXTDEF_VTLBSIZEEXT:
			c->tlbsizevtlb +=
				((config4 & MIPS_CONF4_VTLBSIZEEXT) >>
				  MIPS_CONF4_VTLBSIZEEXT_SHIFT) * 0x40;
			c->tlbsize = c->tlbsizevtlb;
			ftlb_page = MIPS_CONF4_VFTLBPAGESIZE;
			/* fall through */
		case MIPS_CONF4_MMUEXTDEF_FTLBSIZEEXT:
			if (mips_ftlb_disabled)
				break;
			newcf4 = (config4 & ~ftlb_page) |
				(page_size_ftlb(mmuextdef) <<
				 MIPS_CONF4_FTLBPAGESIZE_SHIFT);
			write_c0_config4(newcf4);
			back_to_back_c0_hazard();
			config4 = read_c0_config4();
			if (config4 != newcf4) {
				pr_err("PAGE_SIZE 0x%lx is not supported by FTLB (config4=0x%x)\n",
				       PAGE_SIZE, config4);
				/* Switch FTLB off */
				set_ftlb_enable(c, 0);
				break;
			}
			c->tlbsizeftlbsets = 1 <<
				((config4 & MIPS_CONF4_FTLBSETS) >>
				 MIPS_CONF4_FTLBSETS_SHIFT);
			c->tlbsizeftlbways = ((config4 & MIPS_CONF4_FTLBWAYS) >>
					      MIPS_CONF4_FTLBWAYS_SHIFT) + 2;
			c->tlbsize += c->tlbsizeftlbways * c->tlbsizeftlbsets;
			mips_has_ftlb_configured = 1;
			break;
		}
	}

	c->kscratch_mask = (config4 >> 16) & 0xff;

	return config4 & MIPS_CONF_M;
}

static inline unsigned int decode_config5(struct cpuinfo_mips *c)
{
	unsigned int config5;

	config5 = read_c0_config5();
	config5 &= ~(MIPS_CONF5_UFR | MIPS_CONF5_UFE);
	write_c0_config5(config5);

	if (config5 & MIPS_CONF5_EVA)
		c->options |= MIPS_CPU_EVA;
	if (config5 & MIPS_CONF5_MRP)
		c->options |= MIPS_CPU_MAAR;
	if (config5 & MIPS_CONF5_LLB)
		c->options |= MIPS_CPU_RW_LLB;
#ifdef CONFIG_XPA
	if (config5 & MIPS_CONF5_MVH)
		c->options |= MIPS_CPU_XPA;
#endif

	return config5 & MIPS_CONF_M;
}

static void decode_configs(struct cpuinfo_mips *c)
{
	int ok;

	/* MIPS32 or MIPS64 compliant CPU.  */
	c->options = MIPS_CPU_4KEX | MIPS_CPU_4K_CACHE | MIPS_CPU_COUNTER |
		     MIPS_CPU_DIVEC | MIPS_CPU_LLSC | MIPS_CPU_MCHECK;

	c->scache.flags = MIPS_CACHE_NOT_PRESENT;

	/* Enable FTLB if present and not disabled */
	set_ftlb_enable(c, !mips_ftlb_disabled);

	ok = decode_config0(c);			/* Read Config registers.  */
	BUG_ON(!ok);				/* Arch spec violation!	 */
	if (ok)
		ok = decode_config1(c);
	if (ok)
		ok = decode_config2(c);
	if (ok)
		ok = decode_config3(c);
	if (ok)
		ok = decode_config4(c);
	if (ok)
		ok = decode_config5(c);

	mips_probe_watch_registers(c);

	if (cpu_has_rixi) {
		/* Enable the RIXI exceptions */
		set_c0_pagegrain(PG_IEC);
		back_to_back_c0_hazard();
		/* Verify the IEC bit is set */
		if (read_c0_pagegrain() & PG_IEC)
			c->options |= MIPS_CPU_RIXIEX;
	}

#ifndef CONFIG_MIPS_CPS
	if (cpu_has_mips_r2_r6) {
		c->core = get_ebase_cpunum();
		if (cpu_has_mipsmt)
			c->core >>= fls(core_nvpes()) - 1;
	}
#endif
}

#define R4K_OPTS (MIPS_CPU_TLB | MIPS_CPU_4KEX | MIPS_CPU_4K_CACHE \
		| MIPS_CPU_COUNTER)

static inline void cpu_probe_legacy(struct cpuinfo_mips *c, unsigned int cpu)
{
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_R2000:
		c->cputype = CPU_R2000;
		__cpu_name[cpu] = "R2000";
		c->fpu_msk31 |= FPU_CSR_CONDX | FPU_CSR_FS;
		c->options = MIPS_CPU_TLB | MIPS_CPU_3K_CACHE |
			     MIPS_CPU_NOFPUEX;
		if (__cpu_has_fpu())
			c->options |= MIPS_CPU_FPU;
		c->tlbsize = 64;
		break;
	case PRID_IMP_R3000:
		if ((c->processor_id & PRID_REV_MASK) == PRID_REV_R3000A) {
			if (cpu_has_confreg()) {
				c->cputype = CPU_R3081E;
				__cpu_name[cpu] = "R3081";
			} else {
				c->cputype = CPU_R3000A;
				__cpu_name[cpu] = "R3000A";
			}
		} else {
			c->cputype = CPU_R3000;
			__cpu_name[cpu] = "R3000";
		}
		c->fpu_msk31 |= FPU_CSR_CONDX | FPU_CSR_FS;
		c->options = MIPS_CPU_TLB | MIPS_CPU_3K_CACHE |
			     MIPS_CPU_NOFPUEX;
		if (__cpu_has_fpu())
			c->options |= MIPS_CPU_FPU;
		c->tlbsize = 64;
		break;
	case PRID_IMP_R4000:
		if (read_c0_config() & CONF_SC) {
			if ((c->processor_id & PRID_REV_MASK) >=
			    PRID_REV_R4400) {
				c->cputype = CPU_R4400PC;
				__cpu_name[cpu] = "R4400PC";
			} else {
				c->cputype = CPU_R4000PC;
				__cpu_name[cpu] = "R4000PC";
			}
		} else {
			int cca = read_c0_config() & CONF_CM_CMASK;
			int mc;

			/*
			 * SC and MC versions can't be reliably told apart,
			 * but only the latter support coherent caching
			 * modes so assume the firmware has set the KSEG0
			 * coherency attribute reasonably (if uncached, we
			 * assume SC).
			 */
			switch (cca) {
			case CONF_CM_CACHABLE_CE:
			case CONF_CM_CACHABLE_COW:
			case CONF_CM_CACHABLE_CUW:
				mc = 1;
				break;
			default:
				mc = 0;
				break;
			}
			if ((c->processor_id & PRID_REV_MASK) >=
			    PRID_REV_R4400) {
				c->cputype = mc ? CPU_R4400MC : CPU_R4400SC;
				__cpu_name[cpu] = mc ? "R4400MC" : "R4400SC";
			} else {
				c->cputype = mc ? CPU_R4000MC : CPU_R4000SC;
				__cpu_name[cpu] = mc ? "R4000MC" : "R4000SC";
			}
		}

		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_WATCH | MIPS_CPU_VCE |
			     MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	case PRID_IMP_VR41XX:
		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS;
		c->tlbsize = 32;
		switch (c->processor_id & 0xf0) {
		case PRID_REV_VR4111:
			c->cputype = CPU_VR4111;
			__cpu_name[cpu] = "NEC VR4111";
			break;
		case PRID_REV_VR4121:
			c->cputype = CPU_VR4121;
			__cpu_name[cpu] = "NEC VR4121";
			break;
		case PRID_REV_VR4122:
			if ((c->processor_id & 0xf) < 0x3) {
				c->cputype = CPU_VR4122;
				__cpu_name[cpu] = "NEC VR4122";
			} else {
				c->cputype = CPU_VR4181A;
				__cpu_name[cpu] = "NEC VR4181A";
			}
			break;
		case PRID_REV_VR4130:
			if ((c->processor_id & 0xf) < 0x4) {
				c->cputype = CPU_VR4131;
				__cpu_name[cpu] = "NEC VR4131";
			} else {
				c->cputype = CPU_VR4133;
				c->options |= MIPS_CPU_LLSC;
				__cpu_name[cpu] = "NEC VR4133";
			}
			break;
		default:
			printk(KERN_INFO "Unexpected CPU of NEC VR4100 series\n");
			c->cputype = CPU_VR41XX;
			__cpu_name[cpu] = "NEC Vr41xx";
			break;
		}
		break;
	case PRID_IMP_R4300:
		c->cputype = CPU_R4300;
		__cpu_name[cpu] = "R4300";
		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_LLSC;
		c->tlbsize = 32;
		break;
	case PRID_IMP_R4600:
		c->cputype = CPU_R4600;
		__cpu_name[cpu] = "R4600";
		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	#if 0
	case PRID_IMP_R4650:
		/*
		 * This processor doesn't have an MMU, so it's not
		 * "real easy" to run Linux on it. It is left purely
		 * for documentation.  Commented out because it shares
		 * it's c0_prid id number with the TX3900.
		 */
		c->cputype = CPU_R4650;
		__cpu_name[cpu] = "R4650";
		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	#endif
	case PRID_IMP_TX39:
		c->fpu_msk31 |= FPU_CSR_CONDX | FPU_CSR_FS;
		c->options = MIPS_CPU_TLB | MIPS_CPU_TX39_CACHE;

		if ((c->processor_id & 0xf0) == (PRID_REV_TX3927 & 0xf0)) {
			c->cputype = CPU_TX3927;
			__cpu_name[cpu] = "TX3927";
			c->tlbsize = 64;
		} else {
			switch (c->processor_id & PRID_REV_MASK) {
			case PRID_REV_TX3912:
				c->cputype = CPU_TX3912;
				__cpu_name[cpu] = "TX3912";
				c->tlbsize = 32;
				break;
			case PRID_REV_TX3922:
				c->cputype = CPU_TX3922;
				__cpu_name[cpu] = "TX3922";
				c->tlbsize = 64;
				break;
			}
		}
		break;
	case PRID_IMP_R4700:
		c->cputype = CPU_R4700;
		__cpu_name[cpu] = "R4700";
		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	case PRID_IMP_TX49:
		c->cputype = CPU_TX49XX;
		__cpu_name[cpu] = "R49XX";
		set_isa(c, MIPS_CPU_ISA_III);
		c->fpu_msk31 |= FPU_CSR_CONDX;
		c->options = R4K_OPTS | MIPS_CPU_LLSC;
		if (!(c->processor_id & 0x08))
			c->options |= MIPS_CPU_FPU | MIPS_CPU_32FPR;
		c->tlbsize = 48;
		break;
	case PRID_IMP_R5000:
		c->cputype = CPU_R5000;
		__cpu_name[cpu] = "R5000";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	case PRID_IMP_R5432:
		c->cputype = CPU_R5432;
		__cpu_name[cpu] = "R5432";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_WATCH | MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	case PRID_IMP_R5500:
		c->cputype = CPU_R5500;
		__cpu_name[cpu] = "R5500";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_WATCH | MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	case PRID_IMP_NEVADA:
		c->cputype = CPU_NEVADA;
		__cpu_name[cpu] = "Nevada";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_DIVEC | MIPS_CPU_LLSC;
		c->tlbsize = 48;
		break;
	case PRID_IMP_R6000:
		c->cputype = CPU_R6000;
		__cpu_name[cpu] = "R6000";
		set_isa(c, MIPS_CPU_ISA_II);
		c->fpu_msk31 |= FPU_CSR_CONDX | FPU_CSR_FS;
		c->options = MIPS_CPU_TLB | MIPS_CPU_FPU |
			     MIPS_CPU_LLSC;
		c->tlbsize = 32;
		break;
	case PRID_IMP_R6000A:
		c->cputype = CPU_R6000A;
		__cpu_name[cpu] = "R6000A";
		set_isa(c, MIPS_CPU_ISA_II);
		c->fpu_msk31 |= FPU_CSR_CONDX | FPU_CSR_FS;
		c->options = MIPS_CPU_TLB | MIPS_CPU_FPU |
			     MIPS_CPU_LLSC;
		c->tlbsize = 32;
		break;
	case PRID_IMP_RM7000:
		c->cputype = CPU_RM7000;
		__cpu_name[cpu] = "RM7000";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = R4K_OPTS | MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_LLSC;
		/*
		 * Undocumented RM7000:	 Bit 29 in the info register of
		 * the RM7000 v2.0 indicates if the TLB has 48 or 64
		 * entries.
		 *
		 * 29	   1 =>	   64 entry JTLB
		 *	   0 =>	   48 entry JTLB
		 */
		c->tlbsize = (read_c0_info() & (1 << 29)) ? 64 : 48;
		break;
	case PRID_IMP_R8000:
		c->cputype = CPU_R8000;
		__cpu_name[cpu] = "RM8000";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = MIPS_CPU_TLB | MIPS_CPU_4KEX |
			     MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_LLSC;
		c->tlbsize = 384;      /* has weird TLB: 3-way x 128 */
		break;
	case PRID_IMP_R10000:
		c->cputype = CPU_R10000;
		__cpu_name[cpu] = "R10000";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = MIPS_CPU_TLB | MIPS_CPU_4K_CACHE | MIPS_CPU_4KEX |
			     MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_COUNTER | MIPS_CPU_WATCH |
			     MIPS_CPU_LLSC;
		c->tlbsize = 64;
		break;
	case PRID_IMP_R12000:
		c->cputype = CPU_R12000;
		__cpu_name[cpu] = "R12000";
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = MIPS_CPU_TLB | MIPS_CPU_4K_CACHE | MIPS_CPU_4KEX |
			     MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_COUNTER | MIPS_CPU_WATCH |
			     MIPS_CPU_LLSC;
		c->tlbsize = 64;
		break;
	case PRID_IMP_R14000:
		if (((c->processor_id >> 4) & 0x0f) > 2) {
			c->cputype = CPU_R16000;
			__cpu_name[cpu] = "R16000";
		} else {
			c->cputype = CPU_R14000;
			__cpu_name[cpu] = "R14000";
		}
		set_isa(c, MIPS_CPU_ISA_IV);
		c->options = MIPS_CPU_TLB | MIPS_CPU_4K_CACHE | MIPS_CPU_4KEX |
			     MIPS_CPU_FPU | MIPS_CPU_32FPR |
			     MIPS_CPU_COUNTER | MIPS_CPU_WATCH |
			     MIPS_CPU_LLSC;
		c->tlbsize = 64;
		break;
	case PRID_IMP_LOONGSON_64:  /* Loongson-2/3 */
		switch (c->processor_id & PRID_REV_MASK) {
		case PRID_REV_LOONGSON2E:
			c->cputype = CPU_LOONGSON2;
			__cpu_name[cpu] = "ICT Loongson-2";
			set_elf_platform(cpu, "loongson2e");
			set_isa(c, MIPS_CPU_ISA_III);
			c->fpu_msk31 |= FPU_CSR_CONDX;
			break;
		case PRID_REV_LOONGSON2F:
			c->cputype = CPU_LOONGSON2;
			__cpu_name[cpu] = "ICT Loongson-2";
			set_elf_platform(cpu, "loongson2f");
			set_isa(c, MIPS_CPU_ISA_III);
			c->fpu_msk31 |= FPU_CSR_CONDX;
			break;
		case PRID_REV_LOONGSON3A:
			c->cputype = CPU_LOONGSON3;
			__cpu_name[cpu] = "ICT Loongson-3";
			set_elf_platform(cpu, "loongson3a");
			set_isa(c, MIPS_CPU_ISA_M64R1);
			break;
		case PRID_REV_LOONGSON3B_R1:
		case PRID_REV_LOONGSON3B_R2:
			c->cputype = CPU_LOONGSON3;
			__cpu_name[cpu] = "ICT Loongson-3";
			set_elf_platform(cpu, "loongson3b");
			set_isa(c, MIPS_CPU_ISA_M64R1);
			break;
		}

		c->options = R4K_OPTS |
			     MIPS_CPU_FPU | MIPS_CPU_LLSC |
			     MIPS_CPU_32FPR;
		c->tlbsize = 64;
		c->writecombine = _CACHE_UNCACHED_ACCELERATED;
		break;
	case PRID_IMP_LOONGSON_32:  /* Loongson-1 */
		decode_configs(c);

		c->cputype = CPU_LOONGSON1;

		switch (c->processor_id & PRID_REV_MASK) {
		case PRID_REV_LOONGSON1B:
			__cpu_name[cpu] = "Loongson 1B";
			break;
		}

		break;
	}
}

static inline void cpu_probe_mips(struct cpuinfo_mips *c, unsigned int cpu)
{
	c->writecombine = _CACHE_UNCACHED_ACCELERATED;
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_QEMU_GENERIC:
		c->writecombine = _CACHE_UNCACHED;
		c->cputype = CPU_QEMU_GENERIC;
		__cpu_name[cpu] = "MIPS GENERIC QEMU";
		break;
	case PRID_IMP_4KC:
		c->cputype = CPU_4KC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 4Kc";
		break;
	case PRID_IMP_4KEC:
	case PRID_IMP_4KECR2:
		c->cputype = CPU_4KEC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 4KEc";
		break;
	case PRID_IMP_4KSC:
	case PRID_IMP_4KSD:
		c->cputype = CPU_4KSC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 4KSc";
		break;
	case PRID_IMP_5KC:
		c->cputype = CPU_5KC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 5Kc";
		break;
	case PRID_IMP_5KE:
		c->cputype = CPU_5KE;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 5KE";
		break;
	case PRID_IMP_20KC:
		c->cputype = CPU_20KC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 20Kc";
		break;
	case PRID_IMP_24K:
		c->cputype = CPU_24K;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 24Kc";
		break;
	case PRID_IMP_24KE:
		c->cputype = CPU_24K;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 24KEc";
		break;
	case PRID_IMP_25KF:
		c->cputype = CPU_25KF;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 25Kc";
		break;
	case PRID_IMP_34K:
		c->cputype = CPU_34K;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 34Kc";
		break;
	case PRID_IMP_74K:
		c->cputype = CPU_74K;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 74Kc";
		break;
	case PRID_IMP_M14KC:
		c->cputype = CPU_M14KC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS M14Kc";
		break;
	case PRID_IMP_M14KEC:
		c->cputype = CPU_M14KEC;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS M14KEc";
		break;
	case PRID_IMP_1004K:
		c->cputype = CPU_1004K;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 1004Kc";
		break;
	case PRID_IMP_1074K:
		c->cputype = CPU_1074K;
		c->writecombine = _CACHE_UNCACHED;
		__cpu_name[cpu] = "MIPS 1074Kc";
		break;
	case PRID_IMP_INTERAPTIV_UP:
		c->cputype = CPU_INTERAPTIV;
		__cpu_name[cpu] = "MIPS interAptiv";
		break;
	case PRID_IMP_INTERAPTIV_MP:
		c->cputype = CPU_INTERAPTIV;
		__cpu_name[cpu] = "MIPS interAptiv (multi)";
		break;
	case PRID_IMP_PROAPTIV_UP:
		c->cputype = CPU_PROAPTIV;
		__cpu_name[cpu] = "MIPS proAptiv";
		break;
	case PRID_IMP_PROAPTIV_MP:
		c->cputype = CPU_PROAPTIV;
		__cpu_name[cpu] = "MIPS proAptiv (multi)";
		break;
	case PRID_IMP_P5600:
		c->cputype = CPU_P5600;
		__cpu_name[cpu] = "MIPS P5600";
		break;
	case PRID_IMP_M5150:
		c->cputype = CPU_M5150;
		__cpu_name[cpu] = "MIPS M5150";
		break;
	}

	decode_configs(c);

	spram_config();
}

static inline void cpu_probe_alchemy(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_AU1_REV1:
	case PRID_IMP_AU1_REV2:
		c->cputype = CPU_ALCHEMY;
		switch ((c->processor_id >> 24) & 0xff) {
		case 0:
			__cpu_name[cpu] = "Au1000";
			break;
		case 1:
			__cpu_name[cpu] = "Au1500";
			break;
		case 2:
			__cpu_name[cpu] = "Au1100";
			break;
		case 3:
			__cpu_name[cpu] = "Au1550";
			break;
		case 4:
			__cpu_name[cpu] = "Au1200";
			if ((c->processor_id & PRID_REV_MASK) == 2)
				__cpu_name[cpu] = "Au1250";
			break;
		case 5:
			__cpu_name[cpu] = "Au1210";
			break;
		default:
			__cpu_name[cpu] = "Au1xxx";
			break;
		}
		break;
	}
}

static inline void cpu_probe_sibyte(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);

	c->writecombine = _CACHE_UNCACHED_ACCELERATED;
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_SB1:
		c->cputype = CPU_SB1;
		__cpu_name[cpu] = "SiByte SB1";
		/* FPU in pass1 is known to have issues. */
		if ((c->processor_id & PRID_REV_MASK) < 0x02)
			c->options &= ~(MIPS_CPU_FPU | MIPS_CPU_32FPR);
		break;
	case PRID_IMP_SB1A:
		c->cputype = CPU_SB1A;
		__cpu_name[cpu] = "SiByte SB1A";
		break;
	}
}

static inline void cpu_probe_sandcraft(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_SR71000:
		c->cputype = CPU_SR71000;
		__cpu_name[cpu] = "Sandcraft SR71000";
		c->scache.ways = 8;
		c->tlbsize = 64;
		break;
	}
}

static inline void cpu_probe_nxp(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_PR4450:
		c->cputype = CPU_PR4450;
		__cpu_name[cpu] = "Philips PR4450";
		set_isa(c, MIPS_CPU_ISA_M32R1);
		break;
	}
}

static inline void cpu_probe_broadcom(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_BMIPS32_REV4:
	case PRID_IMP_BMIPS32_REV8:
		c->cputype = CPU_BMIPS32;
		__cpu_name[cpu] = "Broadcom BMIPS32";
		set_elf_platform(cpu, "bmips32");
		break;
	case PRID_IMP_BMIPS3300:
	case PRID_IMP_BMIPS3300_ALT:
	case PRID_IMP_BMIPS3300_BUG:
		c->cputype = CPU_BMIPS3300;
		__cpu_name[cpu] = "Broadcom BMIPS3300";
		set_elf_platform(cpu, "bmips3300");
		break;
	case PRID_IMP_BMIPS43XX: {
		int rev = c->processor_id & PRID_REV_MASK;

		if (rev >= PRID_REV_BMIPS4380_LO &&
				rev <= PRID_REV_BMIPS4380_HI) {
			c->cputype = CPU_BMIPS4380;
			__cpu_name[cpu] = "Broadcom BMIPS4380";
			set_elf_platform(cpu, "bmips4380");
		} else {
			c->cputype = CPU_BMIPS4350;
			__cpu_name[cpu] = "Broadcom BMIPS4350";
			set_elf_platform(cpu, "bmips4350");
		}
		break;
	}
	case PRID_IMP_BMIPS5000:
	case PRID_IMP_BMIPS5200:
		c->cputype = CPU_BMIPS5000;
		__cpu_name[cpu] = "Broadcom BMIPS5000";
		set_elf_platform(cpu, "bmips5000");
		c->options |= MIPS_CPU_ULRI;
		break;
	}
}

static inline void cpu_probe_cavium(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_CAVIUM_CN38XX:
	case PRID_IMP_CAVIUM_CN31XX:
	case PRID_IMP_CAVIUM_CN30XX:
		c->cputype = CPU_CAVIUM_OCTEON;
		__cpu_name[cpu] = "Cavium Octeon";
		goto platform;
	case PRID_IMP_CAVIUM_CN58XX:
	case PRID_IMP_CAVIUM_CN56XX:
	case PRID_IMP_CAVIUM_CN50XX:
	case PRID_IMP_CAVIUM_CN52XX:
		c->cputype = CPU_CAVIUM_OCTEON_PLUS;
		__cpu_name[cpu] = "Cavium Octeon+";
platform:
		set_elf_platform(cpu, "octeon");
		break;
	case PRID_IMP_CAVIUM_CN61XX:
	case PRID_IMP_CAVIUM_CN63XX:
	case PRID_IMP_CAVIUM_CN66XX:
	case PRID_IMP_CAVIUM_CN68XX:
	case PRID_IMP_CAVIUM_CNF71XX:
		c->cputype = CPU_CAVIUM_OCTEON2;
		__cpu_name[cpu] = "Cavium Octeon II";
		set_elf_platform(cpu, "octeon2");
		break;
	case PRID_IMP_CAVIUM_CN70XX:
	case PRID_IMP_CAVIUM_CN78XX:
		c->cputype = CPU_CAVIUM_OCTEON3;
		__cpu_name[cpu] = "Cavium Octeon III";
		set_elf_platform(cpu, "octeon3");
		break;
	default:
		printk(KERN_INFO "Unknown Octeon chip!\n");
		c->cputype = CPU_UNKNOWN;
		break;
	}
}

static inline void cpu_probe_ingenic(struct cpuinfo_mips *c, unsigned int cpu)
{
	decode_configs(c);
	/* JZRISC does not implement the CP0 counter. */
	c->options &= ~MIPS_CPU_COUNTER;
	BUG_ON(!__builtin_constant_p(cpu_has_counter) || cpu_has_counter);
	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_JZRISC:
		c->cputype = CPU_JZRISC;
		c->writecombine = _CACHE_UNCACHED_ACCELERATED;
		__cpu_name[cpu] = "Ingenic JZRISC";
		break;
	default:
		panic("Unknown Ingenic Processor ID!");
		break;
	}
}

static inline void cpu_probe_netlogic(struct cpuinfo_mips *c, int cpu)
{
	decode_configs(c);

	if ((c->processor_id & PRID_IMP_MASK) == PRID_IMP_NETLOGIC_AU13XX) {
		c->cputype = CPU_ALCHEMY;
		__cpu_name[cpu] = "Au1300";
		/* following stuff is not for Alchemy */
		return;
	}

	c->options = (MIPS_CPU_TLB	 |
			MIPS_CPU_4KEX	 |
			MIPS_CPU_COUNTER |
			MIPS_CPU_DIVEC	 |
			MIPS_CPU_WATCH	 |
			MIPS_CPU_EJTAG	 |
			MIPS_CPU_LLSC);

	switch (c->processor_id & PRID_IMP_MASK) {
	case PRID_IMP_NETLOGIC_XLP2XX:
	case PRID_IMP_NETLOGIC_XLP9XX:
	case PRID_IMP_NETLOGIC_XLP5XX:
		c->cputype = CPU_XLP;
		__cpu_name[cpu] = "Broadcom XLPII";
		break;

	case PRID_IMP_NETLOGIC_XLP8XX:
	case PRID_IMP_NETLOGIC_XLP3XX:
		c->cputype = CPU_XLP;
		__cpu_name[cpu] = "Netlogic XLP";
		break;

	case PRID_IMP_NETLOGIC_XLR732:
	case PRID_IMP_NETLOGIC_XLR716:
	case PRID_IMP_NETLOGIC_XLR532:
	case PRID_IMP_NETLOGIC_XLR308:
	case PRID_IMP_NETLOGIC_XLR532C:
	case PRID_IMP_NETLOGIC_XLR516C:
	case PRID_IMP_NETLOGIC_XLR508C:
	case PRID_IMP_NETLOGIC_XLR308C:
		c->cputype = CPU_XLR;
		__cpu_name[cpu] = "Netlogic XLR";
		break;

	case PRID_IMP_NETLOGIC_XLS608:
	case PRID_IMP_NETLOGIC_XLS408:
	case PRID_IMP_NETLOGIC_XLS404:
	case PRID_IMP_NETLOGIC_XLS208:
	case PRID_IMP_NETLOGIC_XLS204:
	case PRID_IMP_NETLOGIC_XLS108:
	case PRID_IMP_NETLOGIC_XLS104:
	case PRID_IMP_NETLOGIC_XLS616B:
	case PRID_IMP_NETLOGIC_XLS608B:
	case PRID_IMP_NETLOGIC_XLS416B:
	case PRID_IMP_NETLOGIC_XLS412B:
	case PRID_IMP_NETLOGIC_XLS408B:
	case PRID_IMP_NETLOGIC_XLS404B:
		c->cputype = CPU_XLR;
		__cpu_name[cpu] = "Netlogic XLS";
		break;

	default:
		pr_info("Unknown Netlogic chip id [%02x]!\n",
		       c->processor_id);
		c->cputype = CPU_XLR;
		break;
	}

	if (c->cputype == CPU_XLP) {
		set_isa(c, MIPS_CPU_ISA_M64R2);
		c->options |= (MIPS_CPU_FPU | MIPS_CPU_ULRI | MIPS_CPU_MCHECK);
		/* This will be updated again after all threads are woken up */
		c->tlbsize = ((read_c0_config6() >> 16) & 0xffff) + 1;
	} else {
		set_isa(c, MIPS_CPU_ISA_M64R1);
		c->tlbsize = ((read_c0_config1() >> 25) & 0x3f) + 1;
	}
	c->kscratch_mask = 0xf;
}

#ifdef CONFIG_64BIT
/* For use by uaccess.h */
u64 __ua_limit;
EXPORT_SYMBOL(__ua_limit);
#endif

const char *__cpu_name[NR_CPUS];
const char *__elf_platform;

void cpu_probe(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;
	unsigned int cpu = smp_processor_id();

	c->processor_id = PRID_IMP_UNKNOWN;
	c->fpu_id	= FPIR_IMP_NONE;
	c->cputype	= CPU_UNKNOWN;
	c->writecombine = _CACHE_UNCACHED;

	c->fpu_csr31	= FPU_CSR_RN;
	c->fpu_msk31	= FPU_CSR_RSVD | FPU_CSR_ABS2008 | FPU_CSR_NAN2008;

	c->processor_id = read_c0_prid();
	switch (c->processor_id & PRID_COMP_MASK) {
	case PRID_COMP_LEGACY:
		cpu_probe_legacy(c, cpu);
		break;
	case PRID_COMP_MIPS:
		cpu_probe_mips(c, cpu);
		break;
	case PRID_COMP_ALCHEMY:
		cpu_probe_alchemy(c, cpu);
		break;
	case PRID_COMP_SIBYTE:
		cpu_probe_sibyte(c, cpu);
		break;
	case PRID_COMP_BROADCOM:
		cpu_probe_broadcom(c, cpu);
		break;
	case PRID_COMP_SANDCRAFT:
		cpu_probe_sandcraft(c, cpu);
		break;
	case PRID_COMP_NXP:
		cpu_probe_nxp(c, cpu);
		break;
	case PRID_COMP_CAVIUM:
		cpu_probe_cavium(c, cpu);
		break;
	case PRID_COMP_INGENIC:
		cpu_probe_ingenic(c, cpu);
		break;
	case PRID_COMP_NETLOGIC:
		cpu_probe_netlogic(c, cpu);
		break;
	}

	BUG_ON(!__cpu_name[cpu]);
	BUG_ON(c->cputype == CPU_UNKNOWN);

	/*
	 * Platform code can force the cpu type to optimize code
	 * generation. In that case be sure the cpu type is correctly
	 * manually setup otherwise it could trigger some nasty bugs.
	 */
	BUG_ON(current_cpu_type() != c->cputype);

	if (mips_fpu_disabled)
		c->options &= ~MIPS_CPU_FPU;

	if (mips_dsp_disabled)
		c->ases &= ~(MIPS_ASE_DSP | MIPS_ASE_DSP2P);

	if (mips_htw_disabled) {
		c->options &= ~MIPS_CPU_HTW;
		write_c0_pwctl(read_c0_pwctl() &
			       ~(1 << MIPS_PWCTL_PWEN_SHIFT));
	}

	if (c->options & MIPS_CPU_FPU)
		cpu_set_fpu_opts(c);
	else
		cpu_set_nofpu_opts(c);

	if (cpu_has_mips_r2_r6) {
		c->srsets = ((read_c0_srsctl() >> 26) & 0x0f) + 1;
		/* R2 has Performance Counter Interrupt indicator */
		c->options |= MIPS_CPU_PCI;
	}
	else
		c->srsets = 1;

	if (cpu_has_msa) {
		c->msa_id = cpu_get_msa_id();
		WARN(c->msa_id & MSA_IR_WRPF,
		     "Vector register partitioning unimplemented!");
	}

	cpu_probe_vmbits(c);

#ifdef CONFIG_64BIT
	if (cpu == 0)
		__ua_limit = ~((1ull << cpu_vmbits) - 1);
#endif
}

void cpu_report(void)
{
	struct cpuinfo_mips *c = &current_cpu_data;

	pr_info("CPU%d revision is: %08x (%s)\n",
		smp_processor_id(), c->processor_id, cpu_name_string());
	if (c->options & MIPS_CPU_FPU)
		printk(KERN_INFO "FPU revision is: %08x\n", c->fpu_id);
	if (cpu_has_msa)
		pr_info("MSA revision is: %08x\n", c->msa_id);
}
