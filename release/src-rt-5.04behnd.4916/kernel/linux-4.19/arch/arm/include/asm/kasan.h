#if defined(CONFIG_BCM_KF_ARM_KASAN)
/*
 * arch/arm/include/asm/kasan.h
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 * Author: Andrey Ryabinin <ryabinin.a.a@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_KASAN_H
#define __ASM_KASAN_H

#ifdef CONFIG_KASAN

#include <asm/kasan_def.h>

#define KASAN_SHADOW_SCALE_SHIFT 3

/*
 * Compiler uses shadow offset assuming that addresses start
 * from 0. Kernel addresses don't start from 0, so shadow
 * for kernel really starts from 'compiler's shadow offset' +
 * ('kernel address space start' >> KASAN_SHADOW_SCALE_SHIFT)
 */

extern void kasan_init(void);

#else
static inline void kasan_init(void) { }
#endif

#endif
#endif
