/*
<:copyright-BRCM:2021:DUAL/GPL:standard

   Copyright (c) 2021 Broadcom 
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

/* ru_custom.h: Customizations for libru in the CPE FW environment. */

#ifndef _RU_CUSTOM_H_
#define _RU_CUSTOM_H_


#include <access_macros.h>

#if defined(_CFE_) || defined(__UBOOT__) 
#include <stdlib.h>
#include <stdio.h>
#include <linux/kernel.h>
#include <string.h>
#include <errno.h>
#include <vsprintf.h>
#include <common.h>
#else
#include <bdmf_system.h>
#include <bdmf_shell.h>
#endif

#define RU_V2 1
#define RU_READ32_IN_PLACE 1
#ifndef READ32
#define READ32 READ_32
#endif
#define WRITE32 WRITE_32

#ifndef RU_FUNCTION_REG_ACCESS
#define RU_FUNCTION_REG_ACCESS 0
#endif

#ifndef RU_INCLUDE_DESC
#define RU_INCLUDE_DESC RU_FUNCTION_REG_ACCESS
#endif

#ifndef RU_INCLUDE_FIELD_DB
#define RU_INCLUDE_FIELD_DB RU_FUNCTION_REG_ACCESS
#endif

#ifndef RU_INCLUDE_ACCESS
#define RU_INCLUDE_ACCESS RU_FUNCTION_REG_ACCESS
#endif

#if !defined(_CFE_) && !defined(__UBOOT__) 
#define RU_FASTLOCK_LOCK() ru_lock()
extern bdmf_fastlock ru_fastlock;
#endif

#if !defined(_CFE_) && !defined(__UBOOT__) 
#define RU_TIMESTAMP_US() bdmf_time_since_epoch_usec()
#define RU_FASTLOCK_UNLOCK(flags) (void)flags, bdmf_fastlock_unlock(&ru_fastlock)
#else
#define RU_TIMESTAMP_US() timer_get_us()
#endif

#if !defined(_CFE_) && !defined(__UBOOT__) && !defined(NO_BDMF_HANDLE)
extern bdmf_session_handle ru_session;
#define RU_PRINT(...) bdmfmon_print(ru_session, __VA_ARGS__)
#else
#define RU_PRINT(...)
#endif

#define RU_FASTLOCK_LOCK() ru_lock()
static unsigned long inline ru_lock(void)
{
#if !defined(_CFE_) && !defined(__UBOOT__) 
   bdmf_fastlock_lock(&ru_fastlock);
#endif
   return 0;
}


#ifndef RDP_SIM

/*#define RU_SIMULATION 0
#define RU_SIMULATION_ACCESS_PRINT 0*/
#if !defined(_CFE_) && !defined(__UBOOT__) 
#define malloc(x) NULL
#define UINT32_MAX  (0xFFFFFFFF)
#endif
typedef int in_addr_t;
typedef uintptr_t intptr_t;
#endif
#ifdef USE_BDMF_SHELL
void ru_cli_init(bdmfmon_handle_t driver_dir);
void ru_cli_exit(bdmfmon_handle_t driver_dir);
#endif
#endif
