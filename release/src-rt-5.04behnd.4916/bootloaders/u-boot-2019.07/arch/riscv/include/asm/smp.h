/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2019 Fraunhofer AISEC,
 * Lukas Auer <lukas.auer@aisec.fraunhofer.de>
 */

#ifndef _ASM_RISCV_SMP_H
#define _ASM_RISCV_SMP_H

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
 * @addr: Address of function
 * @arg0: First argument of function
 * @arg1: Second argument of function
 */
struct ipi_data {
	ulong addr;
	ulong arg0;
	ulong arg1;
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
 * @return 0 if OK, -ve on error
 */
int smp_call_function(ulong addr, ulong arg0, ulong arg1);

#endif
