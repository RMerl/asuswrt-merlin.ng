#if defined(CONFIG_BCM_KF_ASSERT) || !defined(CONFIG_BCM_IN_KERNEL)

/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/*
 *--------------------------------------------------------------------------
 *
 * Asserts are controlled from top level make menuconfig, under
 * Debug Selection>Enable Asserts and Enable Fatal Asserts
 *
 * If Asserts are not enabled, they will be compiled out of the image.
 * If Fatal Asserts are not enabled, code which fails an assert will be
 * allowed to continue to execute (see details below.)
 *
 *--------------------------------------------------------------------------
 */
#ifndef __BCM_ASSERT_H__
#define __BCM_ASSERT_H__

#include <linux/bcm_colors.h>


/************************************************************************
 * Various helpers and conditional macros for the 3 main assert
 * statements below.
 ************************************************************************/

#ifdef CONFIG_BCM_ASSERTS
#define COND_ASSERT_CODE(code)      code
#else
#define COND_ASSERT_CODE(code)
#endif


#ifdef __KERNEL__
// need to include header file for printk
#define KUW_PRINT              printk
#else
#include <stdio.h>
#include <string.h>
#define KUW_PRINT              printf
#endif


#ifdef __KERNEL__
#include <linux/bug.h>
#define FATAL_ACTION       BUG()
/* For the kernel, it would be nice to use WARN for the non-fatal assert,
 * but WARN also kills the process.  What I want is a stack trace and some
 * info, but the code keeps executing.  For now, just do nothing in the
 * non-fatal assert case.  */
#define NON_FATAL_ACTION
#else
// could also use abort here
#define FATAL_ACTION       exit(-1);
#define NON_FATAL_ACTION
#endif


#ifdef CONFIG_BCM_FATAL_ASSERTS
#define COND_FATAL         FATAL_ACTION
#else
#define COND_FATAL         NON_FATAL_ACTION
#endif


#define BCM_ASSERT_PRINT(cond)                                           \
               do {                                                      \
                       KUW_PRINT(CLRerr "ASSERT[%s:%d]:" #cond CLRnl,    \
                             __FUNCTION__, __LINE__);                    \
               } while (0)

/************************************************************************
 * Here are the 3 main ASSERT statements.
 * They differ in how they behave when an assertion fails when fatal
 * asserts are not enabled.
 * BCM_ASSERT_C will continue to execute the rest of the function.
 * BCM_ASSERT_V will return from the current function.
 * BCM_ASSERT_R will return a value from the current function.
 * If fatal asserts are enabled, all 3 asserts behave the same, i.e.
 * the current process is killed and no more code from this path of
 * execution is executed.
 *
 * BCM_ASSERT_A is a special case assert.  It is always compiled in
 * and failure will always be fatal.  Use this one sparingly.
 ************************************************************************/

#define BCM_ASSERT_C(cond)                                               \
               COND_ASSERT_CODE(                                         \
                       if (!(cond)) {                                    \
                          BCM_ASSERT_PRINT(#cond);                       \
                          COND_FATAL;                                    \
                       }                                                 \
               )

#define BCM_ASSERT_V(cond)                                               \
               COND_ASSERT_CODE(                                         \
                       if (!(cond)) {                                    \
                          BCM_ASSERT_PRINT(#cond);                       \
                          COND_FATAL;                                    \
                          return;                                        \
                       }                                                 \
               )

#define BCM_ASSERT_R(cond, ret)                                          \
               COND_ASSERT_CODE(                                         \
                       if (!(cond)) {                                    \
                          BCM_ASSERT_PRINT(#cond);                       \
                          COND_FATAL;                                    \
                          return ret;                                    \
                       }                                                 \
               )

#define BCM_ASSERT_A(cond)                                               \
                       if (!(cond)) {                                    \
                          BCM_ASSERT_PRINT(#cond);                       \
                          FATAL_ACTION;                                  \
                       }


#endif /* __BCM_ASSERT_H__ */

#endif // defined(CONFIG_BRCM_KF_ASSERT)
