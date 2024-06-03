// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * Portions of this file are derived from the Linux kernel source
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <common.h>
#include <dm.h>
#include <asm/control_regs.h>
#include <asm/i8259.h>
#include <asm/interrupt.h>
#include <asm/io.h>
#include <asm/lapic.h>
#include <asm/processor-flags.h>

DECLARE_GLOBAL_DATA_PTR;

#define DECLARE_INTERRUPT(x) \
	".globl irq_"#x"\n" \
	".hidden irq_"#x"\n" \
	".type irq_"#x", @function\n" \
	"irq_"#x":\n" \
	"pushl $"#x"\n" \
	"jmp.d32 irq_common_entry\n"

static char *exceptions[] = {
	"Divide Error",
	"Debug",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode (Undefined Opcode)",
	"Device Not Available (No Math Coprocessor)",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack Segment Fault",
	"General Protection",
	"Page Fault",
	"Reserved",
	"x87 FPU Floating-Point Error",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

static void dump_regs(struct irq_regs *regs)
{
	unsigned long cs, eip, eflags;
	unsigned long cr0 = 0L, cr2 = 0L, cr3 = 0L, cr4 = 0L;
	unsigned long d0, d1, d2, d3, d6, d7;
	unsigned long sp;

	/*
	 * Some exceptions cause an error code to be saved on the current stack
	 * after the EIP value. We should extract CS/EIP/EFLAGS from different
	 * position on the stack based on the exception number.
	 */
	switch (regs->irq_id) {
	case EXC_DF:
	case EXC_TS:
	case EXC_NP:
	case EXC_SS:
	case EXC_GP:
	case EXC_PF:
	case EXC_AC:
		cs = regs->context.ctx2.xcs;
		eip = regs->context.ctx2.eip;
		eflags = regs->context.ctx2.eflags;
		/* We should fix up the ESP due to error code */
		regs->esp += 4;
		break;
	default:
		cs = regs->context.ctx1.xcs;
		eip = regs->context.ctx1.eip;
		eflags = regs->context.ctx1.eflags;
		break;
	}

	printf("EIP: %04x:[<%08lx>] EFLAGS: %08lx\n",
			(u16)cs, eip, eflags);
	if (gd->flags & GD_FLG_RELOC)
		printf("Original EIP :[<%08lx>]\n", eip - gd->reloc_off);

	printf("EAX: %08lx EBX: %08lx ECX: %08lx EDX: %08lx\n",
		regs->eax, regs->ebx, regs->ecx, regs->edx);
	printf("ESI: %08lx EDI: %08lx EBP: %08lx ESP: %08lx\n",
		regs->esi, regs->edi, regs->ebp, regs->esp);
	printf(" DS: %04x ES: %04x FS: %04x GS: %04x SS: %04x\n",
	       (u16)regs->xds, (u16)regs->xes, (u16)regs->xfs,
	       (u16)regs->xgs, (u16)regs->xss);

	cr0 = read_cr0();
	cr2 = read_cr2();
	cr3 = read_cr3();
	cr4 = read_cr4();

	printf("CR0: %08lx CR2: %08lx CR3: %08lx CR4: %08lx\n",
			cr0, cr2, cr3, cr4);

	d0 = get_debugreg(0);
	d1 = get_debugreg(1);
	d2 = get_debugreg(2);
	d3 = get_debugreg(3);

	printf("DR0: %08lx DR1: %08lx DR2: %08lx DR3: %08lx\n",
			d0, d1, d2, d3);

	d6 = get_debugreg(6);
	d7 = get_debugreg(7);
	printf("DR6: %08lx DR7: %08lx\n",
			d6, d7);

	printf("Stack:\n");
	sp = regs->esp;

	sp += 64;

	while (sp > (regs->esp - 16)) {
		if (sp == regs->esp)
			printf("--->");
		else
			printf("    ");
		printf("0x%8.8lx : 0x%8.8lx\n", sp, (ulong)readl(sp));
		sp -= 4;
	}
}

static void do_exception(struct irq_regs *regs)
{
	printf("%s\n", exceptions[regs->irq_id]);
	dump_regs(regs);
	hang();
}

struct idt_entry {
	u16	base_low;
	u16	selector;
	u8	res;
	u8	access;
	u16	base_high;
} __packed;

struct desc_ptr {
	unsigned short size;
	unsigned long address;
} __packed;

struct idt_entry idt[256] __aligned(16);

struct desc_ptr idt_ptr;

static inline void load_idt(const struct desc_ptr *dtr)
{
	asm volatile("cs lidt %0" : : "m" (*dtr));
}

void set_vector(u8 intnum, void *routine)
{
	idt[intnum].base_high = (u16)((ulong)(routine) >> 16);
	idt[intnum].base_low = (u16)((ulong)(routine) & 0xffff);
}

/*
 * Ideally these would be defined static to avoid a checkpatch warning, but
 * the compiler cannot see them in the inline asm and complains that they
 * aren't defined
 */
void irq_0(void);
void irq_1(void);

int cpu_init_interrupts(void)
{
	int i;

	int irq_entry_size = irq_1 - irq_0;
	void *irq_entry = (void *)irq_0;

	/* Setup the IDT */
	for (i = 0; i < 256; i++) {
		idt[i].access = 0x8e;
		idt[i].res = 0;
		idt[i].selector = X86_GDT_ENTRY_32BIT_CS * X86_GDT_ENTRY_SIZE;
		set_vector(i, irq_entry);
		irq_entry += irq_entry_size;
	}

	idt_ptr.size = 256 * 8 - 1;
	idt_ptr.address = (unsigned long) idt;

	load_idt(&idt_ptr);

	return 0;
}

void *x86_get_idt(void)
{
	return &idt_ptr;
}

void __do_irq(int irq)
{
	printf("Unhandled IRQ : %d\n", irq);
}
void do_irq(int irq) __attribute__((weak, alias("__do_irq")));

void enable_interrupts(void)
{
	asm("sti\n");
}

int disable_interrupts(void)
{
	long flags;

#if CONFIG_IS_ENABLED(X86_64)
	asm volatile ("pushfq ; popq %0 ; cli\n" : "=g" (flags) : );
#else
	asm volatile ("pushfl ; popl %0 ; cli\n" : "=g" (flags) : );
#endif
	return flags & X86_EFLAGS_IF;
}

int interrupt_init(void)
{
	struct udevice *dev;
	int ret;

	/* Try to set up the interrupt router, but don't require one */
	ret = uclass_first_device_err(UCLASS_IRQ, &dev);
	if (ret && ret != -ENODEV)
		return ret;

	/*
	 * When running as an EFI application we are not in control of
	 * interrupts and should leave them alone.
	 */
#ifndef CONFIG_EFI_APP
	/* Just in case... */
	disable_interrupts();

#ifdef CONFIG_I8259_PIC
	/* Initialize the master/slave i8259 pic */
	i8259_init();
#endif

#ifdef CONFIG_APIC
	lapic_setup();
#endif

	/* Initialize core interrupt and exception functionality of CPU */
	cpu_init_interrupts();

	/*
	 * It is now safe to enable interrupts.
	 *
	 * TODO(sjg@chromium.org): But we don't handle these correctly when
	 * booted from EFI.
	 */
	if (ll_boot_init())
		enable_interrupts();
#endif

	return 0;
}

/* IRQ Low-Level Service Routine */
void irq_llsr(struct irq_regs *regs)
{
	/*
	 * For detailed description of each exception, refer to:
	 * Intel® 64 and IA-32 Architectures Software Developer's Manual
	 * Volume 1: Basic Architecture
	 * Order Number: 253665-029US, November 2008
	 * Table 6-1. Exceptions and Interrupts
	 */
	if (regs->irq_id < 32) {
		/* Architecture defined exception */
		do_exception(regs);
	} else {
		/* Hardware or User IRQ */
		do_irq(regs->irq_id);
	}
}

/*
 * OK - This looks really horrible, but it serves a purpose - It helps create
 * fully relocatable code.
 *  - The call to irq_llsr will be a relative jump
 *  - The IRQ entries will be guaranteed to be in order
 *  Interrupt entries are now very small (a push and a jump) but they are
 *  now slower (all registers pushed on stack which provides complete
 *  crash dumps in the low level handlers
 *
 * Interrupt Entry Point:
 *  - Interrupt has caused eflags, CS and EIP to be pushed
 *  - Interrupt Vector Handler has pushed orig_eax
 *  - pt_regs.esp needs to be adjusted by 40 bytes:
 *      12 bytes pushed by CPU (EFLAGSF, CS, EIP)
 *      4 bytes pushed by vector handler (irq_id)
 *      24 bytes pushed before SP (SS, GS, FS, ES, DS, EAX)
 *      NOTE: Only longs are pushed on/popped off the stack!
 */
asm(".globl irq_common_entry\n" \
	".hidden irq_common_entry\n" \
	".type irq_common_entry, @function\n" \
	"irq_common_entry:\n" \
	"cld\n" \
	"pushl %ss\n" \
	"pushl %gs\n" \
	"pushl %fs\n" \
	"pushl %es\n" \
	"pushl %ds\n" \
	"pushl %eax\n" \
	"movl  %esp, %eax\n" \
	"addl  $40, %eax\n" \
	"pushl %eax\n" \
	"pushl %ebp\n" \
	"pushl %edi\n" \
	"pushl %esi\n" \
	"pushl %edx\n" \
	"pushl %ecx\n" \
	"pushl %ebx\n" \
	"mov   %esp, %eax\n" \
	"call irq_llsr\n" \
	"popl %ebx\n" \
	"popl %ecx\n" \
	"popl %edx\n" \
	"popl %esi\n" \
	"popl %edi\n" \
	"popl %ebp\n" \
	"popl %eax\n" \
	"popl %eax\n" \
	"popl %ds\n" \
	"popl %es\n" \
	"popl %fs\n" \
	"popl %gs\n" \
	"popl %ss\n" \
	"add  $4, %esp\n" \
	"iret\n" \
	DECLARE_INTERRUPT(0) \
	DECLARE_INTERRUPT(1) \
	DECLARE_INTERRUPT(2) \
	DECLARE_INTERRUPT(3) \
	DECLARE_INTERRUPT(4) \
	DECLARE_INTERRUPT(5) \
	DECLARE_INTERRUPT(6) \
	DECLARE_INTERRUPT(7) \
	DECLARE_INTERRUPT(8) \
	DECLARE_INTERRUPT(9) \
	DECLARE_INTERRUPT(10) \
	DECLARE_INTERRUPT(11) \
	DECLARE_INTERRUPT(12) \
	DECLARE_INTERRUPT(13) \
	DECLARE_INTERRUPT(14) \
	DECLARE_INTERRUPT(15) \
	DECLARE_INTERRUPT(16) \
	DECLARE_INTERRUPT(17) \
	DECLARE_INTERRUPT(18) \
	DECLARE_INTERRUPT(19) \
	DECLARE_INTERRUPT(20) \
	DECLARE_INTERRUPT(21) \
	DECLARE_INTERRUPT(22) \
	DECLARE_INTERRUPT(23) \
	DECLARE_INTERRUPT(24) \
	DECLARE_INTERRUPT(25) \
	DECLARE_INTERRUPT(26) \
	DECLARE_INTERRUPT(27) \
	DECLARE_INTERRUPT(28) \
	DECLARE_INTERRUPT(29) \
	DECLARE_INTERRUPT(30) \
	DECLARE_INTERRUPT(31) \
	DECLARE_INTERRUPT(32) \
	DECLARE_INTERRUPT(33) \
	DECLARE_INTERRUPT(34) \
	DECLARE_INTERRUPT(35) \
	DECLARE_INTERRUPT(36) \
	DECLARE_INTERRUPT(37) \
	DECLARE_INTERRUPT(38) \
	DECLARE_INTERRUPT(39) \
	DECLARE_INTERRUPT(40) \
	DECLARE_INTERRUPT(41) \
	DECLARE_INTERRUPT(42) \
	DECLARE_INTERRUPT(43) \
	DECLARE_INTERRUPT(44) \
	DECLARE_INTERRUPT(45) \
	DECLARE_INTERRUPT(46) \
	DECLARE_INTERRUPT(47) \
	DECLARE_INTERRUPT(48) \
	DECLARE_INTERRUPT(49) \
	DECLARE_INTERRUPT(50) \
	DECLARE_INTERRUPT(51) \
	DECLARE_INTERRUPT(52) \
	DECLARE_INTERRUPT(53) \
	DECLARE_INTERRUPT(54) \
	DECLARE_INTERRUPT(55) \
	DECLARE_INTERRUPT(56) \
	DECLARE_INTERRUPT(57) \
	DECLARE_INTERRUPT(58) \
	DECLARE_INTERRUPT(59) \
	DECLARE_INTERRUPT(60) \
	DECLARE_INTERRUPT(61) \
	DECLARE_INTERRUPT(62) \
	DECLARE_INTERRUPT(63) \
	DECLARE_INTERRUPT(64) \
	DECLARE_INTERRUPT(65) \
	DECLARE_INTERRUPT(66) \
	DECLARE_INTERRUPT(67) \
	DECLARE_INTERRUPT(68) \
	DECLARE_INTERRUPT(69) \
	DECLARE_INTERRUPT(70) \
	DECLARE_INTERRUPT(71) \
	DECLARE_INTERRUPT(72) \
	DECLARE_INTERRUPT(73) \
	DECLARE_INTERRUPT(74) \
	DECLARE_INTERRUPT(75) \
	DECLARE_INTERRUPT(76) \
	DECLARE_INTERRUPT(77) \
	DECLARE_INTERRUPT(78) \
	DECLARE_INTERRUPT(79) \
	DECLARE_INTERRUPT(80) \
	DECLARE_INTERRUPT(81) \
	DECLARE_INTERRUPT(82) \
	DECLARE_INTERRUPT(83) \
	DECLARE_INTERRUPT(84) \
	DECLARE_INTERRUPT(85) \
	DECLARE_INTERRUPT(86) \
	DECLARE_INTERRUPT(87) \
	DECLARE_INTERRUPT(88) \
	DECLARE_INTERRUPT(89) \
	DECLARE_INTERRUPT(90) \
	DECLARE_INTERRUPT(91) \
	DECLARE_INTERRUPT(92) \
	DECLARE_INTERRUPT(93) \
	DECLARE_INTERRUPT(94) \
	DECLARE_INTERRUPT(95) \
	DECLARE_INTERRUPT(97) \
	DECLARE_INTERRUPT(96) \
	DECLARE_INTERRUPT(98) \
	DECLARE_INTERRUPT(99) \
	DECLARE_INTERRUPT(100) \
	DECLARE_INTERRUPT(101) \
	DECLARE_INTERRUPT(102) \
	DECLARE_INTERRUPT(103) \
	DECLARE_INTERRUPT(104) \
	DECLARE_INTERRUPT(105) \
	DECLARE_INTERRUPT(106) \
	DECLARE_INTERRUPT(107) \
	DECLARE_INTERRUPT(108) \
	DECLARE_INTERRUPT(109) \
	DECLARE_INTERRUPT(110) \
	DECLARE_INTERRUPT(111) \
	DECLARE_INTERRUPT(112) \
	DECLARE_INTERRUPT(113) \
	DECLARE_INTERRUPT(114) \
	DECLARE_INTERRUPT(115) \
	DECLARE_INTERRUPT(116) \
	DECLARE_INTERRUPT(117) \
	DECLARE_INTERRUPT(118) \
	DECLARE_INTERRUPT(119) \
	DECLARE_INTERRUPT(120) \
	DECLARE_INTERRUPT(121) \
	DECLARE_INTERRUPT(122) \
	DECLARE_INTERRUPT(123) \
	DECLARE_INTERRUPT(124) \
	DECLARE_INTERRUPT(125) \
	DECLARE_INTERRUPT(126) \
	DECLARE_INTERRUPT(127) \
	DECLARE_INTERRUPT(128) \
	DECLARE_INTERRUPT(129) \
	DECLARE_INTERRUPT(130) \
	DECLARE_INTERRUPT(131) \
	DECLARE_INTERRUPT(132) \
	DECLARE_INTERRUPT(133) \
	DECLARE_INTERRUPT(134) \
	DECLARE_INTERRUPT(135) \
	DECLARE_INTERRUPT(136) \
	DECLARE_INTERRUPT(137) \
	DECLARE_INTERRUPT(138) \
	DECLARE_INTERRUPT(139) \
	DECLARE_INTERRUPT(140) \
	DECLARE_INTERRUPT(141) \
	DECLARE_INTERRUPT(142) \
	DECLARE_INTERRUPT(143) \
	DECLARE_INTERRUPT(144) \
	DECLARE_INTERRUPT(145) \
	DECLARE_INTERRUPT(146) \
	DECLARE_INTERRUPT(147) \
	DECLARE_INTERRUPT(148) \
	DECLARE_INTERRUPT(149) \
	DECLARE_INTERRUPT(150) \
	DECLARE_INTERRUPT(151) \
	DECLARE_INTERRUPT(152) \
	DECLARE_INTERRUPT(153) \
	DECLARE_INTERRUPT(154) \
	DECLARE_INTERRUPT(155) \
	DECLARE_INTERRUPT(156) \
	DECLARE_INTERRUPT(157) \
	DECLARE_INTERRUPT(158) \
	DECLARE_INTERRUPT(159) \
	DECLARE_INTERRUPT(160) \
	DECLARE_INTERRUPT(161) \
	DECLARE_INTERRUPT(162) \
	DECLARE_INTERRUPT(163) \
	DECLARE_INTERRUPT(164) \
	DECLARE_INTERRUPT(165) \
	DECLARE_INTERRUPT(166) \
	DECLARE_INTERRUPT(167) \
	DECLARE_INTERRUPT(168) \
	DECLARE_INTERRUPT(169) \
	DECLARE_INTERRUPT(170) \
	DECLARE_INTERRUPT(171) \
	DECLARE_INTERRUPT(172) \
	DECLARE_INTERRUPT(173) \
	DECLARE_INTERRUPT(174) \
	DECLARE_INTERRUPT(175) \
	DECLARE_INTERRUPT(176) \
	DECLARE_INTERRUPT(177) \
	DECLARE_INTERRUPT(178) \
	DECLARE_INTERRUPT(179) \
	DECLARE_INTERRUPT(180) \
	DECLARE_INTERRUPT(181) \
	DECLARE_INTERRUPT(182) \
	DECLARE_INTERRUPT(183) \
	DECLARE_INTERRUPT(184) \
	DECLARE_INTERRUPT(185) \
	DECLARE_INTERRUPT(186) \
	DECLARE_INTERRUPT(187) \
	DECLARE_INTERRUPT(188) \
	DECLARE_INTERRUPT(189) \
	DECLARE_INTERRUPT(190) \
	DECLARE_INTERRUPT(191) \
	DECLARE_INTERRUPT(192) \
	DECLARE_INTERRUPT(193) \
	DECLARE_INTERRUPT(194) \
	DECLARE_INTERRUPT(195) \
	DECLARE_INTERRUPT(196) \
	DECLARE_INTERRUPT(197) \
	DECLARE_INTERRUPT(198) \
	DECLARE_INTERRUPT(199) \
	DECLARE_INTERRUPT(200) \
	DECLARE_INTERRUPT(201) \
	DECLARE_INTERRUPT(202) \
	DECLARE_INTERRUPT(203) \
	DECLARE_INTERRUPT(204) \
	DECLARE_INTERRUPT(205) \
	DECLARE_INTERRUPT(206) \
	DECLARE_INTERRUPT(207) \
	DECLARE_INTERRUPT(208) \
	DECLARE_INTERRUPT(209) \
	DECLARE_INTERRUPT(210) \
	DECLARE_INTERRUPT(211) \
	DECLARE_INTERRUPT(212) \
	DECLARE_INTERRUPT(213) \
	DECLARE_INTERRUPT(214) \
	DECLARE_INTERRUPT(215) \
	DECLARE_INTERRUPT(216) \
	DECLARE_INTERRUPT(217) \
	DECLARE_INTERRUPT(218) \
	DECLARE_INTERRUPT(219) \
	DECLARE_INTERRUPT(220) \
	DECLARE_INTERRUPT(221) \
	DECLARE_INTERRUPT(222) \
	DECLARE_INTERRUPT(223) \
	DECLARE_INTERRUPT(224) \
	DECLARE_INTERRUPT(225) \
	DECLARE_INTERRUPT(226) \
	DECLARE_INTERRUPT(227) \
	DECLARE_INTERRUPT(228) \
	DECLARE_INTERRUPT(229) \
	DECLARE_INTERRUPT(230) \
	DECLARE_INTERRUPT(231) \
	DECLARE_INTERRUPT(232) \
	DECLARE_INTERRUPT(233) \
	DECLARE_INTERRUPT(234) \
	DECLARE_INTERRUPT(235) \
	DECLARE_INTERRUPT(236) \
	DECLARE_INTERRUPT(237) \
	DECLARE_INTERRUPT(238) \
	DECLARE_INTERRUPT(239) \
	DECLARE_INTERRUPT(240) \
	DECLARE_INTERRUPT(241) \
	DECLARE_INTERRUPT(242) \
	DECLARE_INTERRUPT(243) \
	DECLARE_INTERRUPT(244) \
	DECLARE_INTERRUPT(245) \
	DECLARE_INTERRUPT(246) \
	DECLARE_INTERRUPT(247) \
	DECLARE_INTERRUPT(248) \
	DECLARE_INTERRUPT(249) \
	DECLARE_INTERRUPT(250) \
	DECLARE_INTERRUPT(251) \
	DECLARE_INTERRUPT(252) \
	DECLARE_INTERRUPT(253) \
	DECLARE_INTERRUPT(254) \
	DECLARE_INTERRUPT(255));
