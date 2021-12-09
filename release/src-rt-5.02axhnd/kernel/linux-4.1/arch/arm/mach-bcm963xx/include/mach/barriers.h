#if defined(CONFIG_BCM_KF_ARM_BCM963XX)
/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
#ifndef _MACH_BCM963XX_BARRIERS_H
#define _MACH_BCM963XX_BARRIERS_H

#include <asm/outercache.h>

#ifdef CONFIG_BCM_B15_MEGA_BARRIER
void BcmMegaBarrier(void); /*Implemented in board_963xx.c*/
#endif

#if defined(CONFIG_BCM_KF_ARM_ERRATA_798181)
 /*WARNING: don't use isb()/dsb()/dmb() macros as a Write-Memory-Memory Barrier. Correctness is not guaranteed.
  *If you need a write memory barrier, use the wmb()/smp_wmb() macros below.*/
 #define isb(option) __asm__ __volatile__ ("isb " #option : : : "memory")
 #define dsb(option) __asm__ __volatile__ ("dsb " #option : : : "memory")
 #define dmb(option) __asm__ __volatile__ ("dmb " #option : : : "memory")
#else
 #define isb() __asm__ __volatile__ ("isb" : : : "memory")
 #define dsb() __asm__ __volatile__ ("dsb" : : : "memory")
 #define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#endif

#if defined(CONFIG_ARM_DMA_MEM_BUFFERABLE) || defined(CONFIG_SMP)
 #if defined(CONFIG_BCM_B15_MEGA_BARRIER)
  #define mb()		BcmMegaBarrier()
  #define wmb()		BcmMegaBarrier()
 #elif defined(CONFIG_BCM_KF_ARM_ERRATA_798181)
  #define mb()		do { dsb(); outer_sync(); } while (0)
  #define wmb()		do { dsb(st); outer_sync(); } while (0)
 #else
  #define mb()		do { dsb(); outer_sync(); } while (0)
  #define wmb()		mb()
 #endif
 #define rmb()		dsb()
#else
 #include <asm/memory.h>
 #define mb()	do { if (arch_is_coherent()) dmb(); else barrier(); } while (0)
 #define rmb()	do { if (arch_is_coherent()) dmb(); else barrier(); } while (0)
 #define wmb()	do { if (arch_is_coherent()) dmb(); else barrier(); } while (0)
#endif

#ifndef CONFIG_SMP
 #ifdef CONFIG_BCM_B15_MEGA_BARRIER
  #define smp_mb()	BcmMegaBarrier()
 #else
  #define smp_mb()	barrier()
 #endif
 #define smp_rmb()	barrier()
 #define smp_wmb()	smp_mb()
#else /*CONFIG_SMP:*/
 #if defined(CONFIG_BCM_B15_MEGA_BARRIER)
  #define smp_mb()	BcmMegaBarrier()
  #define smp_rmb()	dmb(ish)
  #define smp_wmb()	BcmMegaBarrier()
 #elif defined(CONFIG_BCM_KF_ARM_ERRATA_798181)
  #define smp_mb()	dmb(ish)
  #define smp_rmb()	smp_mb()
  #define smp_wmb()	dmb(ishst)
 #else
  #define smp_mb()	dmb()
  #define smp_rmb()	dmb()
  #define smp_wmb()	dmb()
 #endif
#endif /*CONFIG_SMP*/
#endif /*_MACH_BCM963XX_BARRIERS_H*/
#endif /*(CONFIG_BCM_KF_ARM_BCM963XX)*/
