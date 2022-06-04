/*
 *  arch/arm/include/asm/bugs.h
 *
 *  Copyright (C) 1995-2003 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_BUGS_H
#define __ASM_BUGS_H

#if defined(CONFIG_BCM_KF_SPECTRE_PATCH) && defined(CONFIG_BCM_SPECTRE_PATCH_ENABLE)
extern void check_writebuffer_bugs(void);
#ifdef CONFIG_MMU
extern void check_bugs(void);
extern void check_other_bugs(void);
#else
#define check_bugs() do { } while (0)
#define check_other_bugs() do { } while (0)
#endif
#else
#ifdef CONFIG_MMU
extern void check_writebuffer_bugs(void);

#define check_bugs() check_writebuffer_bugs()
#else
#define check_bugs() do { } while (0)
#endif
#endif /* CONFIG_BCM_KF_SPECTRE_PATCH && CONFIG_BCM_SPECTRE_PATCH_ENABLE*/

#endif
