#if defined(CONFIG_BCM_KF_MIPS_BCM963XX) && defined(CONFIG_MIPS_BCM963XX)
/***********************************************************
 *
 * Copyright (c) 2009 Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license 
 * agreement governing use of this software, this software is licensed 
 * to you under the terms of the GNU General Public License version 2 
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give 
 *    you permission to link this software with independent modules, and 
 *    to copy and distribute the resulting executable under terms of your 
 *    choice, provided that you also meet, for each linked independent 
 *    module, the terms and conditions of the license of that module. 
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications 
 *    of the software.  
 * 
 * Not withstanding the above, under no circumstances may you combine 
 * this software in any way with any other Broadcom software provided 
 * under a license other than the GPL, without Broadcom's express prior 
 * written consent. 
 * 
 * :>
 *
 ************************************************************/

/***********************************************************
 *
 *    SMP support for Broadcom 63xx and 68xx chips
 *
 *    05/2009    Created by Xi Wang
 *
 ************************************************************/

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>
#include <linux/compiler.h>
#include <linux/irq.h>

#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/hardirq.h>
#include <asm/mmu_context.h>
#include <asm/smp.h>
#include <asm/time.h>
#include <asm/mipsregs.h>
#include <asm/mipsmtregs.h>
#include <asm/mips_mt.h>

#include <bcm_cpu.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>


static int cpu_ipi_irq;
DEFINE_PER_CPU(unsigned int, ipi_pending);
DEFINE_PER_CPU(unsigned int, ipi_flags);

extern spinlock_t brcm_irqlock;

// boot parameters
struct BootConfig {
    unsigned int func_addr;
    unsigned int gp;
    unsigned int sp;
};

static struct BootConfig boot_config;

void install_smp_boot_ex_handler(void);
static void core_send_ipi_single(int cpu, unsigned int action);
static void core_send_ipi_mask(const struct cpumask *mask, unsigned int action);


void install_smp_boot_ex_handler(void)
{

	asm (
        ".set push\n"
        ".set noreorder\n"
        "lui    $8, 0xa000 \n"
        "ori    $8, $8, 0x0200  # alternative mips exception vector\n"
        "la     $9, 2f\n"
        "la     $10, 3f\n"
    "1:\n"
        "lw     $11, 0($9)\n"
        "sw     $11, 0($8)\n"
        "addiu  $9, $9, 4\n"
        "bne    $9, $10, 1b\n"
        "addiu  $8, $8, 4\n"
        "b      3f\n"
        "nop\n"
    "2:    # begin exception handler code\n"
        "mfc0   $26, $13, 0\n"
        "li     $27, 0x800100   # change back to normal exception vector & ack interrupt\n"
        "xor    $26, $27\n"
        "mtc0   $26, $13, 0\n"
        "la     $27, %0         # pointer to boot_config structure\n"
        "lw     $24, 0($27)     # func_addr - will be loaded into EPC before eret\n"
        "lw     $28, 4($27)     # gp\n"
        "lw     $29, 8($27)     # sp\n"
        "mtc0   $24, $14        # load return address into EPC\n"
        "eret\n"
    "3:\n"
        ".set pop\n"
        :
        : "X" (&boot_config)
	);
}

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS
extern void bcm_timer_interrupt_handler_TP1(void);
#endif

static irqreturn_t ipi_interrupt(int irq, void *dev_id)
{
	unsigned int old_ipi_flags;

	spin_lock(&brcm_irqlock);
	old_ipi_flags = *this_cpu_ptr(&ipi_flags);
	*this_cpu_ptr(&ipi_flags) = 0;
	spin_unlock(&brcm_irqlock);

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS
	/* Process TIMER related interrupt first */
	if (old_ipi_flags & 1<<2) {
		bcm_timer_interrupt_handler_TP1();
	}
#endif

	if (old_ipi_flags & 1<<0) {
		scheduler_ipi();
	}

	if (old_ipi_flags & 1<<1) {
		smp_call_function_interrupt();
	}

	return IRQ_HANDLED;
}


static struct irqaction irq_ipi = {
	.handler	= ipi_interrupt,
	.flags		= 0x00|IRQF_PERCPU,
	.name		= "IPI"
};


static void __init brcm_smp_setup(void)
{
	int i;

	init_cpu_possible(cpu_possible_mask);

	for (i = 0; i < 2; i++) {
		set_cpu_possible(i, true);
		__cpu_number_map[i]	= i;
		__cpu_logical_map[i]	= i;
	}
}


static void __init brcm_prepare_cpus(unsigned int max_cpus)
{
	unsigned int c0tmp;
	int cpu;

	c0tmp = __read_32bit_c0_register($22, 0);
	c0tmp |= CP0_BCM_CFG_NBK;    /* non blocking */
	__write_32bit_c0_register($22, 0, c0tmp);

	c0tmp = __read_32bit_c0_register($22, 2);
	c0tmp &= ~(CP0_CMT_PRIO_TP0 | CP0_CMT_PRIO_TP1); /* equal (random) D-cache priority */
	__write_32bit_c0_register($22, 2, c0tmp);

	//printk("bcm config0 %08x\n", __read_32bit_c0_register($22, 0));
	//printk("cmt control %08x\n", __read_32bit_c0_register($22, 2));
	//printk("cmt local %08x\n", __read_32bit_c0_register($22, 3));
	//printk("bcm config1 %08x\n", __read_32bit_c0_register($22, 5));

	for_each_possible_cpu(cpu) {
		per_cpu(ipi_pending, cpu) = 0;
		per_cpu(ipi_flags, cpu) = 0;
	}

	c0tmp = __read_32bit_c0_register($22, 1);
	c0tmp |= CP0_CMT_SIR_0;
	__write_32bit_c0_register($22, 1, c0tmp);

	cpu_ipi_irq = INTERRUPT_ID_SOFTWARE_0;
	setup_irq(cpu_ipi_irq, &irq_ipi);
	irq_set_handler(cpu_ipi_irq, handle_percpu_irq);
}


// Pass PC, SP, and GP to a secondary core and start it up by sending an inter-core interrupt
static void __cpuinit brcm_boot_secondary(int cpu, struct task_struct *idle)
{
	unsigned int cause;

	boot_config.func_addr = (unsigned long) smp_bootstrap;
	boot_config.sp = (unsigned int) __KSTK_TOS(idle);
	boot_config.gp = (unsigned int) task_thread_info(idle);

	install_smp_boot_ex_handler();

	cause = read_c0_cause();
	cause |= CAUSEF_IP0;
	write_c0_cause(cause);
}


static void __cpuinit brcm_init_secondary(void)
{
	//printk("bcm config0 %08x\n", __read_32bit_c0_register($22, 0));
	//printk("cmt control %08x\n", __read_32bit_c0_register($22, 2));
	//printk("cmt local %08x\n", __read_32bit_c0_register($22, 3));
	//printk("bcm config1 %08x\n", __read_32bit_c0_register($22, 5));

	clear_c0_status(ST0_BEV);

#if defined(CONFIG_BCM96838) 

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS
	// CP0 timer interrupt (IRQ5) is not used for TP1 when pwrsave is enabled
	change_c0_status(ST0_IM, IE_SW0 | IE_IRQ1 | IE_IRQ2 /*| IE_IRQ5*/);
#else
	change_c0_status(ST0_IM, IE_SW0 | IE_IRQ1 | IE_IRQ2 | IE_IRQ5);
#endif

#else

#ifdef CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS
	// CP0 timer interrupt (IRQ5) is not used for TP1 when pwrsave is enabled
	change_c0_status(ST0_IM, IE_SW0 | IE_IRQ0 | IE_IRQ1 /*| IE_IRQ5*/);
#else
	change_c0_status(ST0_IM, IE_SW0 | IE_IRQ0 | IE_IRQ1 | IE_IRQ5);
#endif

#endif
}


static void __cpuinit brcm_smp_finish(void)
{
}



// send inter-core interrupts
static void core_send_ipi_single(int cpu, unsigned int action)
{
	unsigned long flags;
	unsigned int cause;
	
	//	printk("== from_cpu %d    to_cpu %d    action %u\n", smp_processor_id(), cpu, action);

	spin_lock_irqsave(&brcm_irqlock, flags);

	switch (action) {
	case SMP_RESCHEDULE_YOURSELF:
		per_cpu(ipi_pending, cpu) = 1;
		per_cpu(ipi_flags, cpu) |= 1<<0;
		break;
	case SMP_CALL_FUNCTION:
		per_cpu(ipi_pending, cpu) = 1;
		per_cpu(ipi_flags, cpu) |= 1<<1;
		break;
#if defined(CONFIG_BCM_HOSTMIPS_PWRSAVE_TIMERS)
	case SMP_BCM_PWRSAVE_TIMER:
		per_cpu(ipi_pending, cpu) = 1;
		per_cpu(ipi_flags, cpu) |= 1<<2;
		break;
#endif
	default:
		goto errexit;
	}

	mb();

	cause = read_c0_cause();
	cause |= CAUSEF_IP0;
	write_c0_cause(cause);

errexit:
	spin_unlock_irqrestore(&brcm_irqlock, flags);
}


static void core_send_ipi_mask(const struct cpumask *mask, unsigned int action)
{
	unsigned int cpu;

	for_each_cpu(cpu, mask) {
		core_send_ipi_single(cpu, action);
	}
}


struct plat_smp_ops brcm_smp_ops = {
	.send_ipi_single	= core_send_ipi_single,
	.send_ipi_mask		= core_send_ipi_mask,
	.init_secondary		= brcm_init_secondary,
	.smp_finish		= brcm_smp_finish,
	.boot_secondary		= brcm_boot_secondary,
	.smp_setup		= brcm_smp_setup,
	.prepare_cpus		= brcm_prepare_cpus
};

#endif // defined(CONFIG_BCM_KF_MIPS_BCM963XX)
