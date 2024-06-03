#if defined(CONFIG_BCM_KF_ARM_KASAN)
/*
 *  arch/arm/include/asm/kasan_def.h
 *
 *  Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 *  Author: Abbott Liu <liuwenliang@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_KASAN_DEF_H
#define __ASM_KASAN_DEF_H

#ifdef CONFIG_KASAN

/*
 *    +----+ 0xffffffff
 *    |    |
 *    |    |
 *    |    |
 *    +----+ CONFIG_PAGE_OFFSET
 *    |    |\
 *    |    | |->  module virtual address space area.
 *    |    |/
 *    +----+ MODULE_VADDR = KASAN_SHADOW_END
 *    |    |\
 *    |    | |-> the shadow area of kernel virtual address.
 *    |    |/
 *    +----+ TASK_SIZE(start of kernel space) = KASAN_SHADOW_START  the
 *    |    |\  shadow address of MODULE_VADDR
 *    |    | ---------------------+
 *    |    |                      |
 *    +    + KASAN_SHADOW_OFFSET  |-> the user space area. Kernel address
 *    |    |                      |    sanitizer do not use this space.
 *    |    | ---------------------+
 *    |    |/
 *    ------ 0
 *
 *1)KASAN_SHADOW_OFFSET:
 *    This value is used to map an address to the corresponding shadow
 * address by the following formula:
 * shadow_addr = (address >> 3) + KASAN_SHADOW_OFFSET;
 *
 * 2)KASAN_SHADOW_START
 *     This value is the MODULE_VADDR's shadow address. It is the start
 * of kernel virtual space.
 *
 * 3) KASAN_SHADOW_END
 *   This value is the 0x100000000's shadow address. It is the end of
 * kernel addresssanitizer's shadow area. It is also the start of the
 * module area.
 *
 */

#define KASAN_SHADOW_OFFSET     (KASAN_SHADOW_END - (1<<29))

#define KASAN_SHADOW_START      ((KASAN_SHADOW_END >> 3) + KASAN_SHADOW_OFFSET)

#define KASAN_SHADOW_END        (UL(CONFIG_PAGE_OFFSET) - UL(SZ_16M))

#endif
#endif
#endif
