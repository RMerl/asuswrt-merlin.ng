/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */

#ifndef _ASM_RISCV_SMP_H
#define _ASM_RISCV_SMP_H

#include <linux/types.h>

/**
 * struct ipi_data - Inter-processor interrupt (IPI) data structure
 *
 * IPIs are used for SMP support to communicate to other harts what function to
 * call. Functions are in the form
 * void (*addr)(ulong hart, ulong arg0, ulong arg1).
 *
 * The function address and the two arguments, arg0 and arg1, are stored in the
 * IPI data structure. The hart ID is inserted by the hart handling the IPI and
 * calling the function.
 *
 * @valid is used to determine whether a sent IPI originated from U-Boot. It is
 * initialized to zero by board_init_f_alloc_reserve. When U-Boot sends its
 * first IPI, it is set to 1. This prevents already-pending IPIs not sent by
 * U-Boot from being taken.
 *
 * @addr: Address of function
 * @arg0: First argument of function
 * @arg1: Second argument of function
 * @valid: Whether this IPI is valid
 */
struct ipi_data {
	ulong addr;
	ulong arg0;
	ulong arg1;
	unsigned int valid;
};

/**
 * handle_ipi() - interrupt handler for software interrupts
 *
 * The IPI interrupt handler must be called to handle software interrupts. It
 * calls the function specified in the hart's IPI data structure.
 *
 * @hart: Hart ID of the current hart
 */
void handle_ipi(ulong hart);

/**
 * smp_call_function() - Call a function on all other harts
 *
 * Send IPIs with the specified function call to all harts.
 *
 * @addr: Address of function
 * @arg0: First argument of function
 * @arg1: Second argument of function
 * @wait: Wait for harts to acknowledge request
 * Return: 0 if OK, -ve on error
 */
int smp_call_function(ulong addr, ulong arg0, ulong arg1, int wait);

/**
 * riscv_init_ipi() - Initialize inter-process interrupt (IPI) driver
 *
 * Platform code must provide this function. This function is called once after
 * the cpu driver is initialized. No other riscv_*_ipi() calls will be made
 * before this function is called.
 *
 * Return: 0 if OK, -ve on error
 */
int riscv_init_ipi(void);

/**
 * riscv_send_ipi() - Send inter-processor interrupt (IPI)
 *
 * Platform code must provide this function.
 *
 * @hart: Hart ID of receiving hart
 * Return: 0 if OK, -ve on error
 */
int riscv_send_ipi(int hart);

/**
 * riscv_clear_ipi() - Clear inter-processor interrupt (IPI)
 *
 * Platform code must provide this function.
 *
 * @hart: Hart ID of hart to be cleared
 * Return: 0 if OK, -ve on error
 */
int riscv_clear_ipi(int hart);

/**
 * riscv_get_ipi() - Get status of inter-processor interrupt (IPI)
 *
 * Platform code must provide this function.
 *
 * @hart: Hart ID of hart to be checked
 * @pending: Pointer to variable with result of the check,
 *           1 if IPI is pending, 0 otherwise
 * Return: 0 if OK, -ve on error
 */
int riscv_get_ipi(int hart, int *pending);

#endif
