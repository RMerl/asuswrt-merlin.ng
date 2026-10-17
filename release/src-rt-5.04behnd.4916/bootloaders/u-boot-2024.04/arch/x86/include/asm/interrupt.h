/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Graeme Russ, graeme.russ@gmail.com
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 */

#ifndef __ASM_INTERRUPT_H_
#define __ASM_INTERRUPT_H_ 1

#include <asm/types.h>

#define SYS_NUM_IRQS	16

/* Architecture defined exceptions */
enum x86_exception {
	EXC_DE = 0,
	EXC_DB,
	EXC_NMI,
	EXC_BP,
	EXC_OF,
	EXC_BR,
	EXC_UD,
	EXC_NM,
	EXC_DF,
	EXC_CSO,
	EXC_TS,
	EXC_NP,
	EXC_SS,
	EXC_GP,
	EXC_PF,
	EXC_MF = 16,
	EXC_AC,
	EXC_MC,
	EXC_XM,
	EXC_VE
};

/**
 * struct idt_ptr - Holds the IDT (Interrupt Descriptor Table)
 *
 * @size: Size of IDT in bytes
 */
struct idt_ptr {
	unsigned short size;
	unsigned long address;
} __packed;

/* arch/x86/cpu/interrupts.c */
void set_vector(u8 intnum, void *routine);

/* Architecture specific functions */
void mask_irq(int irq);
void unmask_irq(int irq);
void specific_eoi(int irq);

extern char exception_stack[];

/**
 * configure_irq_trigger() - Configure IRQ triggering
 *
 * Switch the given interrupt to be level / edge triggered
 *
 * @param int_num legacy interrupt number (3-7, 9-15)
 * @param is_level_triggered true for level triggered interrupt, false for
 *	edge triggered interrupt
 */
void configure_irq_trigger(int int_num, bool is_level_triggered);

void *x86_get_idt(void);

/**
 * interrupt_read_idt() - Read the IDT
 *
 * @ptr: Place to put IDT contents
 */
void interrupt_read_idt(struct idt_ptr *ptr);

#endif
