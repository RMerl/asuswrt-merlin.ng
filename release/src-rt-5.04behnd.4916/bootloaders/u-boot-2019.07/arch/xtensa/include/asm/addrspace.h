/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2008-2013 Tensilica Inc.
 * Copyright (C) 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_ADDRSPACE_H
#define _XTENSA_ADDRSPACE_H

#include <asm/arch/core.h>

/*
 * MMU Memory Map
 *
 * noMMU and v3 MMU have identity mapped address space on reset.
 * V2 MMU:
 *   IO (uncached)	f0000000..ffffffff	-> f000000
 *   IO (cached)	e0000000..efffffff	-> f000000
 *   MEM (uncached)	d8000000..dfffffff	-> 0000000
 *   MEM (cached)	d0000000..d7ffffff	-> 0000000
 *
 * The actual location of memory and IO is the board property.
 */

#define IOADDR(x)		(CONFIG_SYS_IO_BASE + (x))
#define MEMADDR(x)		(CONFIG_SYS_MEMORY_BASE + (x))
#define PHYSADDR(x)		((x) - XCHAL_VECBASE_RESET_VADDR + \
				 XCHAL_VECBASE_RESET_PADDR)

#endif	/* _XTENSA_ADDRSPACE_H */
