// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/mp.h>
#include <asm/arch/soc.h>
#include "cpu.h"
#include <asm/arch-fsl-layerscape/soc.h>

DECLARE_GLOBAL_DATA_PTR;

void *get_spin_tbl_addr(void)
{
	return &__spin_table;
}

phys_addr_t determine_mp_bootpg(void)
{
	return (phys_addr_t)&secondary_boot_code;
}

void update_os_arch_secondary_cores(uint8_t os_arch)
{
	u64 *table = get_spin_tbl_addr();
	int i;

	for (i = 1; i < CONFIG_MAX_CPUS; i++) {
		if (os_arch == IH_ARCH_DEFAULT)
			table[i * WORDS_PER_SPIN_TABLE_ENTRY +
				SPIN_TABLE_ELEM_ARCH_COMP_IDX] = OS_ARCH_SAME;
		else
			table[i * WORDS_PER_SPIN_TABLE_ENTRY +
				SPIN_TABLE_ELEM_ARCH_COMP_IDX] = OS_ARCH_DIFF;
	}
}

#ifdef CONFIG_FSL_LSCH3
void wake_secondary_core_n(int cluster, int core, int cluster_cores)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_reset __iomem *rst = (void *)(CONFIG_SYS_FSL_RST_ADDR);
	u32 mpidr = 0;

	mpidr = ((cluster << 8) | core);
	/*
	 * mpidr_el1 register value of core which needs to be released
	 * is written to scratchrw[6] register
	 */
	gur_out32(&gur->scratchrw[6], mpidr);
	asm volatile("dsb st" : : : "memory");
	rst->brrl |= 1 << ((cluster * cluster_cores) + core);
	asm volatile("dsb st" : : : "memory");
	/*
	 * scratchrw[6] register value is polled
	 * when the value becomes zero, this means that this core is up
	 * and running, next core can be released now
	 */
	while (gur_in32(&gur->scratchrw[6]) != 0)
		;
}
#endif

int fsl_layerscape_wake_seconday_cores(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
#ifdef CONFIG_FSL_LSCH3
	struct ccsr_reset __iomem *rst = (void *)(CONFIG_SYS_FSL_RST_ADDR);
	u32 svr, ver, cluster, type;
	int j = 0, cluster_cores = 0;
#elif defined(CONFIG_FSL_LSCH2)
	struct ccsr_scfg __iomem *scfg = (void *)(CONFIG_SYS_FSL_SCFG_ADDR);
#endif
	u32 cores, cpu_up_mask = 1;
	int i, timeout = 10;
	u64 *table = get_spin_tbl_addr();

#ifdef COUNTER_FREQUENCY_REAL
	/* update for secondary cores */
	__real_cntfrq = COUNTER_FREQUENCY_REAL;
	flush_dcache_range((unsigned long)&__real_cntfrq,
			   (unsigned long)&__real_cntfrq + 8);
#endif

	cores = cpu_mask();
	/* Clear spin table so that secondary processors
	 * observe the correct value after waking up from wfe.
	 */
	memset(table, 0, CONFIG_MAX_CPUS*SPIN_TABLE_ELEM_SIZE);
	flush_dcache_range((unsigned long)table,
			   (unsigned long)table +
			   (CONFIG_MAX_CPUS*SPIN_TABLE_ELEM_SIZE));

	printf("Waking secondary cores to start from %lx\n", gd->relocaddr);

#ifdef CONFIG_FSL_LSCH3
	gur_out32(&gur->bootlocptrh, (u32)(gd->relocaddr >> 32));
	gur_out32(&gur->bootlocptrl, (u32)gd->relocaddr);

	svr = gur_in32(&gur->svr);
	ver = SVR_SOC_VER(svr);
	if (ver == SVR_LS2080A || ver == SVR_LS2085A) {
		gur_out32(&gur->scratchrw[6], 1);
		asm volatile("dsb st" : : : "memory");
		rst->brrl = cores;
		asm volatile("dsb st" : : : "memory");
	} else {
		/*
		 * Release the cores out of reset one-at-a-time to avoid
		 * power spikes
		 */
		i = 0;
		cluster = in_le32(&gur->tp_cluster[i].lower);
		for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
			type = initiator_type(cluster, j);
			if (type &&
			    TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
				cluster_cores++;
		}

		do {
			cluster = in_le32(&gur->tp_cluster[i].lower);
			for (j = 0; j < TP_INIT_PER_CLUSTER; j++) {
				type = initiator_type(cluster, j);
				if (type &&
				    TP_ITYP_TYPE(type) == TP_ITYP_TYPE_ARM)
					wake_secondary_core_n(i, j,
							      cluster_cores);
			}
		i++;
		} while ((cluster & TP_CLUSTER_EOC) != TP_CLUSTER_EOC);
	}
#elif defined(CONFIG_FSL_LSCH2)
	scfg_out32(&scfg->scratchrw[0], (u32)(gd->relocaddr >> 32));
	scfg_out32(&scfg->scratchrw[1], (u32)gd->relocaddr);
	asm volatile("dsb st" : : : "memory");
	gur_out32(&gur->brrl, cores);
	asm volatile("dsb st" : : : "memory");

	/* Bootup online cores */
	scfg_out32(&scfg->corebcr, cores);
#endif
	/* This is needed as a precautionary measure.
	 * If some code before this has accidentally  released the secondary
	 * cores then the pre-bootloader code will trap them in a "wfe" unless
	 * the scratchrw[6] is set. In this case we need a sev here to get these
	 * cores moving again.
	 */
	asm volatile("sev");

	while (timeout--) {
		flush_dcache_range((unsigned long)table, (unsigned long)table +
				   CONFIG_MAX_CPUS * 64);
		for (i = 1; i < CONFIG_MAX_CPUS; i++) {
			if (table[i * WORDS_PER_SPIN_TABLE_ENTRY +
					SPIN_TABLE_ELEM_STATUS_IDX])
				cpu_up_mask |= 1 << i;
		}
		if (hweight32(cpu_up_mask) == hweight32(cores))
			break;
		udelay(10);
	}
	if (timeout <= 0) {
		printf("Not all cores (0x%x) are up (0x%x)\n",
		       cores, cpu_up_mask);
		return 1;
	}
	printf("All (%d) cores are up.\n", hweight32(cores));

	return 0;
}

int is_core_valid(unsigned int core)
{
	return !!((1 << core) & cpu_mask());
}

static int is_pos_valid(unsigned int pos)
{
	return !!((1 << pos) & cpu_pos_mask());
}

int is_core_online(u64 cpu_id)
{
	u64 *table;
	int pos = id_to_core(cpu_id);
	table = (u64 *)get_spin_tbl_addr() + pos * WORDS_PER_SPIN_TABLE_ENTRY;
	return table[SPIN_TABLE_ELEM_STATUS_IDX] == 1;
}

int cpu_reset(u32 nr)
{
	puts("Feature is not implemented.\n");

	return 0;
}

int cpu_disable(u32 nr)
{
	puts("Feature is not implemented.\n");

	return 0;
}

static int core_to_pos(int nr)
{
	u32 cores = cpu_pos_mask();
	int i, count = 0;

	if (nr == 0) {
		return 0;
	} else if (nr >= hweight32(cores)) {
		puts("Not a valid core number.\n");
		return -1;
	}

	for (i = 1; i < 32; i++) {
		if (is_pos_valid(i)) {
			count++;
			if (count == nr)
				break;
		}
	}

	if (count != nr)
		return -1;

	return i;
}

int cpu_status(u32 nr)
{
	u64 *table;
	int pos;

	if (nr == 0) {
		table = (u64 *)get_spin_tbl_addr();
		printf("table base @ 0x%p\n", table);
	} else {
		pos = core_to_pos(nr);
		if (pos < 0)
			return -1;
		table = (u64 *)get_spin_tbl_addr() + pos *
			WORDS_PER_SPIN_TABLE_ENTRY;
		printf("table @ 0x%p\n", table);
		printf("   addr - 0x%016llx\n",
		       table[SPIN_TABLE_ELEM_ENTRY_ADDR_IDX]);
		printf("   status   - 0x%016llx\n",
		       table[SPIN_TABLE_ELEM_STATUS_IDX]);
		printf("   lpid  - 0x%016llx\n",
		       table[SPIN_TABLE_ELEM_LPID_IDX]);
	}

	return 0;
}

int cpu_release(u32 nr, int argc, char * const argv[])
{
	u64 boot_addr;
	u64 *table = (u64 *)get_spin_tbl_addr();
	int pos;

	pos = core_to_pos(nr);
	if (pos <= 0)
		return -1;

	table += pos * WORDS_PER_SPIN_TABLE_ENTRY;
	boot_addr = simple_strtoull(argv[0], NULL, 16);
	table[SPIN_TABLE_ELEM_ENTRY_ADDR_IDX] = boot_addr;
	flush_dcache_range((unsigned long)table,
			   (unsigned long)table + SPIN_TABLE_ELEM_SIZE);
	asm volatile("dsb st");
	smp_kick_all_cpus();	/* only those with entry addr set will run */
	/*
	 * When the first release command runs, all cores are set to go. Those
	 * without a valid entry address will be trapped by "wfe". "sev" kicks
	 * them off to check the address again. When set, they continue to run.
	 */
	asm volatile("sev");

	return 0;
}
