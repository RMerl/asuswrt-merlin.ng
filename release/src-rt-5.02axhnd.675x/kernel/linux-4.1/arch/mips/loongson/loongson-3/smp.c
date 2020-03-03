/*
 * Copyright (C) 2010, 2011, 2012, Lemote, Inc.
 * Author: Chen Huacai, chenhc@lemote.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/cpufreq.h>
#include <asm/processor.h>
#include <asm/time.h>
#include <asm/clock.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <loongson.h>
#include <workarounds.h>

#include "smp.h"

DEFINE_PER_CPU(int, cpu_state);
DEFINE_PER_CPU(uint32_t, core0_c0count);

static void *ipi_set0_regs[16];
static void *ipi_clear0_regs[16];
static void *ipi_status0_regs[16];
static void *ipi_en0_regs[16];
static void *ipi_mailbox_buf[16];

/* read a 32bit value from ipi register */
#define loongson3_ipi_read32(addr) readl(addr)
/* read a 64bit value from ipi register */
#define loongson3_ipi_read64(addr) readq(addr)
/* write a 32bit value to ipi register */
#define loongson3_ipi_write32(action, addr)	\
	do {					\
		writel(action, addr);		\
		__wbflush();			\
	} while (0)
/* write a 64bit value to ipi register */
#define loongson3_ipi_write64(action, addr)	\
	do {					\
		writeq(action, addr);		\
		__wbflush();			\
	} while (0)

static void ipi_set0_regs_init(void)
{
	ipi_set0_regs[0] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE0_OFFSET + SET0);
	ipi_set0_regs[1] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE1_OFFSET + SET0);
	ipi_set0_regs[2] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE2_OFFSET + SET0);
	ipi_set0_regs[3] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE3_OFFSET + SET0);
	ipi_set0_regs[4] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE0_OFFSET + SET0);
	ipi_set0_regs[5] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE1_OFFSET + SET0);
	ipi_set0_regs[6] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE2_OFFSET + SET0);
	ipi_set0_regs[7] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE3_OFFSET + SET0);
	ipi_set0_regs[8] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE0_OFFSET + SET0);
	ipi_set0_regs[9] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE1_OFFSET + SET0);
	ipi_set0_regs[10] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE2_OFFSET + SET0);
	ipi_set0_regs[11] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE3_OFFSET + SET0);
	ipi_set0_regs[12] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE0_OFFSET + SET0);
	ipi_set0_regs[13] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE1_OFFSET + SET0);
	ipi_set0_regs[14] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE2_OFFSET + SET0);
	ipi_set0_regs[15] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE3_OFFSET + SET0);
}

static void ipi_clear0_regs_init(void)
{
	ipi_clear0_regs[0] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE0_OFFSET + CLEAR0);
	ipi_clear0_regs[1] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE1_OFFSET + CLEAR0);
	ipi_clear0_regs[2] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE2_OFFSET + CLEAR0);
	ipi_clear0_regs[3] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE3_OFFSET + CLEAR0);
	ipi_clear0_regs[4] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE0_OFFSET + CLEAR0);
	ipi_clear0_regs[5] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE1_OFFSET + CLEAR0);
	ipi_clear0_regs[6] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE2_OFFSET + CLEAR0);
	ipi_clear0_regs[7] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE3_OFFSET + CLEAR0);
	ipi_clear0_regs[8] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE0_OFFSET + CLEAR0);
	ipi_clear0_regs[9] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE1_OFFSET + CLEAR0);
	ipi_clear0_regs[10] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE2_OFFSET + CLEAR0);
	ipi_clear0_regs[11] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE3_OFFSET + CLEAR0);
	ipi_clear0_regs[12] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE0_OFFSET + CLEAR0);
	ipi_clear0_regs[13] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE1_OFFSET + CLEAR0);
	ipi_clear0_regs[14] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE2_OFFSET + CLEAR0);
	ipi_clear0_regs[15] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE3_OFFSET + CLEAR0);
}

static void ipi_status0_regs_init(void)
{
	ipi_status0_regs[0] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE0_OFFSET + STATUS0);
	ipi_status0_regs[1] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE1_OFFSET + STATUS0);
	ipi_status0_regs[2] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE2_OFFSET + STATUS0);
	ipi_status0_regs[3] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE3_OFFSET + STATUS0);
	ipi_status0_regs[4] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE0_OFFSET + STATUS0);
	ipi_status0_regs[5] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE1_OFFSET + STATUS0);
	ipi_status0_regs[6] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE2_OFFSET + STATUS0);
	ipi_status0_regs[7] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE3_OFFSET + STATUS0);
	ipi_status0_regs[8] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE0_OFFSET + STATUS0);
	ipi_status0_regs[9] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE1_OFFSET + STATUS0);
	ipi_status0_regs[10] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE2_OFFSET + STATUS0);
	ipi_status0_regs[11] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE3_OFFSET + STATUS0);
	ipi_status0_regs[12] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE0_OFFSET + STATUS0);
	ipi_status0_regs[13] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE1_OFFSET + STATUS0);
	ipi_status0_regs[14] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE2_OFFSET + STATUS0);
	ipi_status0_regs[15] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE3_OFFSET + STATUS0);
}

static void ipi_en0_regs_init(void)
{
	ipi_en0_regs[0] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE0_OFFSET + EN0);
	ipi_en0_regs[1] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE1_OFFSET + EN0);
	ipi_en0_regs[2] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE2_OFFSET + EN0);
	ipi_en0_regs[3] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE3_OFFSET + EN0);
	ipi_en0_regs[4] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE0_OFFSET + EN0);
	ipi_en0_regs[5] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE1_OFFSET + EN0);
	ipi_en0_regs[6] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE2_OFFSET + EN0);
	ipi_en0_regs[7] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE3_OFFSET + EN0);
	ipi_en0_regs[8] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE0_OFFSET + EN0);
	ipi_en0_regs[9] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE1_OFFSET + EN0);
	ipi_en0_regs[10] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE2_OFFSET + EN0);
	ipi_en0_regs[11] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE3_OFFSET + EN0);
	ipi_en0_regs[12] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE0_OFFSET + EN0);
	ipi_en0_regs[13] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE1_OFFSET + EN0);
	ipi_en0_regs[14] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE2_OFFSET + EN0);
	ipi_en0_regs[15] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE3_OFFSET + EN0);
}

static void ipi_mailbox_buf_init(void)
{
	ipi_mailbox_buf[0] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE0_OFFSET + BUF);
	ipi_mailbox_buf[1] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE1_OFFSET + BUF);
	ipi_mailbox_buf[2] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE2_OFFSET + BUF);
	ipi_mailbox_buf[3] = (void *)
		(SMP_CORE_GROUP0_BASE + SMP_CORE3_OFFSET + BUF);
	ipi_mailbox_buf[4] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE0_OFFSET + BUF);
	ipi_mailbox_buf[5] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE1_OFFSET + BUF);
	ipi_mailbox_buf[6] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE2_OFFSET + BUF);
	ipi_mailbox_buf[7] = (void *)
		(SMP_CORE_GROUP1_BASE + SMP_CORE3_OFFSET + BUF);
	ipi_mailbox_buf[8] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE0_OFFSET + BUF);
	ipi_mailbox_buf[9] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE1_OFFSET + BUF);
	ipi_mailbox_buf[10] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE2_OFFSET + BUF);
	ipi_mailbox_buf[11] = (void *)
		(SMP_CORE_GROUP2_BASE + SMP_CORE3_OFFSET + BUF);
	ipi_mailbox_buf[12] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE0_OFFSET + BUF);
	ipi_mailbox_buf[13] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE1_OFFSET + BUF);
	ipi_mailbox_buf[14] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE2_OFFSET + BUF);
	ipi_mailbox_buf[15] = (void *)
		(SMP_CORE_GROUP3_BASE + SMP_CORE3_OFFSET + BUF);
}

/*
 * Simple enough, just poke the appropriate ipi register
 */
static void loongson3_send_ipi_single(int cpu, unsigned int action)
{
	loongson3_ipi_write32((u32)action, ipi_set0_regs[cpu_logical_map(cpu)]);
}

static void
loongson3_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	unsigned int i;

	for_each_cpu(i, mask)
		loongson3_ipi_write32((u32)action, ipi_set0_regs[cpu_logical_map(i)]);
}

void loongson3_ipi_interrupt(struct pt_regs *regs)
{
	int i, cpu = smp_processor_id();
	unsigned int action, c0count;

	/* Load the ipi register to figure out what we're supposed to do */
	action = loongson3_ipi_read32(ipi_status0_regs[cpu_logical_map(cpu)]);

	/* Clear the ipi register to clear the interrupt */
	loongson3_ipi_write32((u32)action, ipi_clear0_regs[cpu_logical_map(cpu)]);

	if (action & SMP_RESCHEDULE_YOURSELF)
		scheduler_ipi();

	if (action & SMP_CALL_FUNCTION)
		smp_call_function_interrupt();

	if (action & SMP_ASK_C0COUNT) {
		BUG_ON(cpu != 0);
		c0count = read_c0_count();
		for (i = 1; i < num_possible_cpus(); i++)
			per_cpu(core0_c0count, i) = c0count;
	}
}

#define MAX_LOOPS 1111
/*
 * SMP init and finish on secondary CPUs
 */
static void loongson3_init_secondary(void)
{
	int i;
	uint32_t initcount;
	unsigned int cpu = smp_processor_id();
	unsigned int imask = STATUSF_IP7 | STATUSF_IP6 |
			     STATUSF_IP3 | STATUSF_IP2;

	/* Set interrupt mask, but don't enable */
	change_c0_status(ST0_IM, imask);

	for (i = 0; i < num_possible_cpus(); i++)
		loongson3_ipi_write32(0xffffffff, ipi_en0_regs[cpu_logical_map(i)]);

	per_cpu(cpu_state, cpu) = CPU_ONLINE;
	cpu_data[cpu].core =
		cpu_logical_map(cpu) % loongson_sysconf.cores_per_package;
	cpu_data[cpu].package =
		cpu_logical_map(cpu) / loongson_sysconf.cores_per_package;

	i = 0;
	__this_cpu_write(core0_c0count, 0);
	loongson3_send_ipi_single(0, SMP_ASK_C0COUNT);
	while (!__this_cpu_read(core0_c0count)) {
		i++;
		cpu_relax();
	}

	if (i > MAX_LOOPS)
		i = MAX_LOOPS;
	initcount = __this_cpu_read(core0_c0count) + i;
	write_c0_count(initcount);
}

static void loongson3_smp_finish(void)
{
	int cpu = smp_processor_id();

	write_c0_compare(read_c0_count() + mips_hpt_frequency/HZ);
	local_irq_enable();
	loongson3_ipi_write64(0,
			(void *)(ipi_mailbox_buf[cpu_logical_map(cpu)]+0x0));
	pr_info("CPU#%d finished, CP0_ST=%x\n",
			smp_processor_id(), read_c0_status());
}

static void __init loongson3_smp_setup(void)
{
	int i = 0, num = 0; /* i: physical id, num: logical id */

	init_cpu_possible(cpu_none_mask);

	/* For unified kernel, NR_CPUS is the maximum possible value,
	 * loongson_sysconf.nr_cpus is the really present value */
	while (i < loongson_sysconf.nr_cpus) {
		if (loongson_sysconf.reserved_cpus_mask & (1<<i)) {
			/* Reserved physical CPU cores */
			__cpu_number_map[i] = -1;
		} else {
			__cpu_number_map[i] = num;
			__cpu_logical_map[num] = i;
			set_cpu_possible(num, true);
			num++;
		}
		i++;
	}
	pr_info("Detected %i available CPU(s)\n", num);

	while (num < loongson_sysconf.nr_cpus) {
		__cpu_logical_map[num] = -1;
		num++;
	}

	ipi_set0_regs_init();
	ipi_clear0_regs_init();
	ipi_status0_regs_init();
	ipi_en0_regs_init();
	ipi_mailbox_buf_init();
	cpu_data[0].core = cpu_logical_map(0) % loongson_sysconf.cores_per_package;
	cpu_data[0].package = cpu_logical_map(0) / loongson_sysconf.cores_per_package;
}

static void __init loongson3_prepare_cpus(unsigned int max_cpus)
{
	init_cpu_present(cpu_possible_mask);
	per_cpu(cpu_state, smp_processor_id()) = CPU_ONLINE;
}

/*
 * Setup the PC, SP, and GP of a secondary processor and start it runing!
 */
static void loongson3_boot_secondary(int cpu, struct task_struct *idle)
{
	unsigned long startargs[4];

	pr_info("Booting CPU#%d...\n", cpu);

	/* startargs[] are initial PC, SP and GP for secondary CPU */
	startargs[0] = (unsigned long)&smp_bootstrap;
	startargs[1] = (unsigned long)__KSTK_TOS(idle);
	startargs[2] = (unsigned long)task_thread_info(idle);
	startargs[3] = 0;

	pr_debug("CPU#%d, func_pc=%lx, sp=%lx, gp=%lx\n",
			cpu, startargs[0], startargs[1], startargs[2]);

	loongson3_ipi_write64(startargs[3],
			(void *)(ipi_mailbox_buf[cpu_logical_map(cpu)]+0x18));
	loongson3_ipi_write64(startargs[2],
			(void *)(ipi_mailbox_buf[cpu_logical_map(cpu)]+0x10));
	loongson3_ipi_write64(startargs[1],
			(void *)(ipi_mailbox_buf[cpu_logical_map(cpu)]+0x8));
	loongson3_ipi_write64(startargs[0],
			(void *)(ipi_mailbox_buf[cpu_logical_map(cpu)]+0x0));
}

#ifdef CONFIG_HOTPLUG_CPU

static int loongson3_cpu_disable(void)
{
	unsigned long flags;
	unsigned int cpu = smp_processor_id();

	if (cpu == 0)
		return -EBUSY;

	set_cpu_online(cpu, false);
	cpumask_clear_cpu(cpu, &cpu_callin_map);
	local_irq_save(flags);
	fixup_irqs();
	local_irq_restore(flags);
	flush_cache_all();
	local_flush_tlb_all();

	return 0;
}


static void loongson3_cpu_die(unsigned int cpu)
{
	while (per_cpu(cpu_state, cpu) != CPU_DEAD)
		cpu_relax();

	mb();
}

/* To shutdown a core in Loongson 3, the target core should go to CKSEG1 and
 * flush all L1 entries at first. Then, another core (usually Core 0) can
 * safely disable the clock of the target core. loongson3_play_dead() is
 * called via CKSEG1 (uncached and unmmaped) */
static void loongson3a_play_dead(int *state_addr)
{
	register int val;
	register long cpuid, core, node, count;
	register void *addr, *base, *initfunc;

	__asm__ __volatile__(
		"   .set push                     \n"
		"   .set noreorder                \n"
		"   li %[addr], 0x80000000        \n" /* KSEG0 */
		"1: cache 0, 0(%[addr])           \n" /* flush L1 ICache */
		"   cache 0, 1(%[addr])           \n"
		"   cache 0, 2(%[addr])           \n"
		"   cache 0, 3(%[addr])           \n"
		"   cache 1, 0(%[addr])           \n" /* flush L1 DCache */
		"   cache 1, 1(%[addr])           \n"
		"   cache 1, 2(%[addr])           \n"
		"   cache 1, 3(%[addr])           \n"
		"   addiu %[sets], %[sets], -1    \n"
		"   bnez  %[sets], 1b             \n"
		"   addiu %[addr], %[addr], 0x20  \n"
		"   li    %[val], 0x7             \n" /* *state_addr = CPU_DEAD; */
		"   sw    %[val], (%[state_addr]) \n"
		"   sync                          \n"
		"   cache 21, (%[state_addr])     \n" /* flush entry of *state_addr */
		"   .set pop                      \n"
		: [addr] "=&r" (addr), [val] "=&r" (val)
		: [state_addr] "r" (state_addr),
		  [sets] "r" (cpu_data[smp_processor_id()].dcache.sets));

	__asm__ __volatile__(
		"   .set push                         \n"
		"   .set noreorder                    \n"
		"   .set mips64                       \n"
		"   mfc0  %[cpuid], $15, 1            \n"
		"   andi  %[cpuid], 0x3ff             \n"
		"   dli   %[base], 0x900000003ff01000 \n"
		"   andi  %[core], %[cpuid], 0x3      \n"
		"   sll   %[core], 8                  \n" /* get core id */
		"   or    %[base], %[base], %[core]   \n"
		"   andi  %[node], %[cpuid], 0xc      \n"
		"   dsll  %[node], 42                 \n" /* get node id */
		"   or    %[base], %[base], %[node]   \n"
		"1: li    %[count], 0x100             \n" /* wait for init loop */
		"2: bnez  %[count], 2b                \n" /* limit mailbox access */
		"   addiu %[count], -1                \n"
		"   ld    %[initfunc], 0x20(%[base])  \n" /* get PC via mailbox */
		"   beqz  %[initfunc], 1b             \n"
		"   nop                               \n"
		"   ld    $sp, 0x28(%[base])          \n" /* get SP via mailbox */
		"   ld    $gp, 0x30(%[base])          \n" /* get GP via mailbox */
		"   ld    $a1, 0x38(%[base])          \n"
		"   jr    %[initfunc]                 \n" /* jump to initial PC */
		"   nop                               \n"
		"   .set pop                          \n"
		: [core] "=&r" (core), [node] "=&r" (node),
		  [base] "=&r" (base), [cpuid] "=&r" (cpuid),
		  [count] "=&r" (count), [initfunc] "=&r" (initfunc)
		: /* No Input */
		: "a1");
}

static void loongson3b_play_dead(int *state_addr)
{
	register int val;
	register long cpuid, core, node, count;
	register void *addr, *base, *initfunc;

	__asm__ __volatile__(
		"   .set push                     \n"
		"   .set noreorder                \n"
		"   li %[addr], 0x80000000        \n" /* KSEG0 */
		"1: cache 0, 0(%[addr])           \n" /* flush L1 ICache */
		"   cache 0, 1(%[addr])           \n"
		"   cache 0, 2(%[addr])           \n"
		"   cache 0, 3(%[addr])           \n"
		"   cache 1, 0(%[addr])           \n" /* flush L1 DCache */
		"   cache 1, 1(%[addr])           \n"
		"   cache 1, 2(%[addr])           \n"
		"   cache 1, 3(%[addr])           \n"
		"   addiu %[sets], %[sets], -1    \n"
		"   bnez  %[sets], 1b             \n"
		"   addiu %[addr], %[addr], 0x20  \n"
		"   li    %[val], 0x7             \n" /* *state_addr = CPU_DEAD; */
		"   sw    %[val], (%[state_addr]) \n"
		"   sync                          \n"
		"   cache 21, (%[state_addr])     \n" /* flush entry of *state_addr */
		"   .set pop                      \n"
		: [addr] "=&r" (addr), [val] "=&r" (val)
		: [state_addr] "r" (state_addr),
		  [sets] "r" (cpu_data[smp_processor_id()].dcache.sets));

	__asm__ __volatile__(
		"   .set push                         \n"
		"   .set noreorder                    \n"
		"   .set mips64                       \n"
		"   mfc0  %[cpuid], $15, 1            \n"
		"   andi  %[cpuid], 0x3ff             \n"
		"   dli   %[base], 0x900000003ff01000 \n"
		"   andi  %[core], %[cpuid], 0x3      \n"
		"   sll   %[core], 8                  \n" /* get core id */
		"   or    %[base], %[base], %[core]   \n"
		"   andi  %[node], %[cpuid], 0xc      \n"
		"   dsll  %[node], 42                 \n" /* get node id */
		"   or    %[base], %[base], %[node]   \n"
		"   dsrl  %[node], 30                 \n" /* 15:14 */
		"   or    %[base], %[base], %[node]   \n"
		"1: li    %[count], 0x100             \n" /* wait for init loop */
		"2: bnez  %[count], 2b                \n" /* limit mailbox access */
		"   addiu %[count], -1                \n"
		"   ld    %[initfunc], 0x20(%[base])  \n" /* get PC via mailbox */
		"   beqz  %[initfunc], 1b             \n"
		"   nop                               \n"
		"   ld    $sp, 0x28(%[base])          \n" /* get SP via mailbox */
		"   ld    $gp, 0x30(%[base])          \n" /* get GP via mailbox */
		"   ld    $a1, 0x38(%[base])          \n"
		"   jr    %[initfunc]                 \n" /* jump to initial PC */
		"   nop                               \n"
		"   .set pop                          \n"
		: [core] "=&r" (core), [node] "=&r" (node),
		  [base] "=&r" (base), [cpuid] "=&r" (cpuid),
		  [count] "=&r" (count), [initfunc] "=&r" (initfunc)
		: /* No Input */
		: "a1");
}

void play_dead(void)
{
	int *state_addr;
	unsigned int cpu = smp_processor_id();
	void (*play_dead_at_ckseg1)(int *);

	idle_task_exit();
	switch (loongson_sysconf.cputype) {
	case Loongson_3A:
	default:
		play_dead_at_ckseg1 =
			(void *)CKSEG1ADDR((unsigned long)loongson3a_play_dead);
		break;
	case Loongson_3B:
		play_dead_at_ckseg1 =
			(void *)CKSEG1ADDR((unsigned long)loongson3b_play_dead);
		break;
	}
	state_addr = &per_cpu(cpu_state, cpu);
	mb();
	play_dead_at_ckseg1(state_addr);
}

void loongson3_disable_clock(int cpu)
{
	uint64_t core_id = cpu_data[cpu].core;
	uint64_t package_id = cpu_data[cpu].package;

	if (loongson_sysconf.cputype == Loongson_3A) {
		LOONGSON_CHIPCFG(package_id) &= ~(1 << (12 + core_id));
	} else if (loongson_sysconf.cputype == Loongson_3B) {
		if (!(loongson_sysconf.workarounds & WORKAROUND_CPUHOTPLUG))
			LOONGSON_FREQCTRL(package_id) &= ~(1 << (core_id * 4 + 3));
	}
}

void loongson3_enable_clock(int cpu)
{
	uint64_t core_id = cpu_data[cpu].core;
	uint64_t package_id = cpu_data[cpu].package;

	if (loongson_sysconf.cputype == Loongson_3A) {
		LOONGSON_CHIPCFG(package_id) |= 1 << (12 + core_id);
	} else if (loongson_sysconf.cputype == Loongson_3B) {
		if (!(loongson_sysconf.workarounds & WORKAROUND_CPUHOTPLUG))
			LOONGSON_FREQCTRL(package_id) |= 1 << (core_id * 4 + 3);
	}
}

#define CPU_POST_DEAD_FROZEN	(CPU_POST_DEAD | CPU_TASKS_FROZEN)
static int loongson3_cpu_callback(struct notifier_block *nfb,
	unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;

	switch (action) {
	case CPU_POST_DEAD:
	case CPU_POST_DEAD_FROZEN:
		pr_info("Disable clock for CPU#%d\n", cpu);
		loongson3_disable_clock(cpu);
		break;
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		pr_info("Enable clock for CPU#%d\n", cpu);
		loongson3_enable_clock(cpu);
		break;
	}

	return NOTIFY_OK;
}

static int register_loongson3_notifier(void)
{
	hotcpu_notifier(loongson3_cpu_callback, 0);
	return 0;
}
early_initcall(register_loongson3_notifier);

#endif

struct plat_smp_ops loongson3_smp_ops = {
	.send_ipi_single = loongson3_send_ipi_single,
	.send_ipi_mask = loongson3_send_ipi_mask,
	.init_secondary = loongson3_init_secondary,
	.smp_finish = loongson3_smp_finish,
	.boot_secondary = loongson3_boot_secondary,
	.smp_setup = loongson3_smp_setup,
	.prepare_cpus = loongson3_prepare_cpus,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_disable = loongson3_cpu_disable,
	.cpu_die = loongson3_cpu_die,
#endif
};
