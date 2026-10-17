/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __ASM_RISCV_SYSTEM_H
#define __ASM_RISCV_SYSTEM_H

#include <asm/csr.h>

struct event;

/*
 * Interupt configuration macros
 */

#define local_irq_save(__flags)                                 \
    do {                                                        \
        __flags = csr_read_clear(CSR_SSTATUS, SR_SIE) & SR_SIE; \
    } while (0)

#define local_irq_restore(__flags)              \
    do {                                        \
        csr_set(CSR_SSTATUS, __flags & SR_SIE); \
    } while (0)

/* Hook to set up the CPU (called from SPL too) */
int riscv_cpu_setup(void);

#endif	/* __ASM_RISCV_SYSTEM_H */
