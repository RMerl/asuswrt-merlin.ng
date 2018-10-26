/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
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

/*
 * access_logging.h
 */

#ifndef _ACCESS_LOGGING_H_
#define _ACCESS_LOGGING_H_

#if defined(CONFIG_GPL_RDP_GEN) || defined(CONFIG_GPL_RDP)

/* These include files are required by the library */
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#else
#include <stdlib.h>
#if defined LINUX_KERNEL || __KERNEL__
#include <linux/types.h>
#else
#include <stdio.h>
#include <stdint.h>
#endif
#endif

typedef enum
{
    ACCESS_LOG_OP_WRITE,
    ACCESS_LOG_OP_MWRITE,
    ACCESS_LOG_OP_MEMSET,
    ACCESS_LOG_OP_MEMSET_32,
    ACCESS_LOG_OP_SLEEP,
    ACCESS_LOG_OP_SET_CORE_ADDR,
    ACCESS_LOG_OP_STOP
} access_log_op_t;

typedef struct access_log_tuple
{
    access_log_op_t op;
    uint16_t size;
    unsigned long addr;
    uint32_t value;
} access_log_tuple_t;

#if defined(CONFIG_GPL_RDP_GEN)

extern int access_log_enable;

static inline void access_log_enable_set(int enable)
{
    access_log_enable = enable;
}

static inline int access_log_enable_get(void)
{
    return access_log_enable;
}

#define ACCESS_LOG_ENTRY_MARK   ">>>"

#define ACCESS_LOG_PRINT(op, addr, value, size) \
    printf(ACCESS_LOG_ENTRY_MARK " %d 0x%lx 0x%x %u\n", op, addr, value, size)

static inline void access_log_log(access_log_op_t op, unsigned long addr, uint32_t value, uint32_t size)
{
    if (access_log_enable_get())
    {
        ACCESS_LOG_PRINT(op, addr, value, size);
    }
}

#define ACCESS_LOG_ENABLE_SET(_e)  access_log_enable_set(_e)

#define ACCESS_LOG(_op, _a, _v, _sz)  access_log_log(_op, (unsigned long )(_a), (uint32_t)(_v), (uint32_t)(_sz))

#ifdef _CFE_

#undef xrdp_usleep
#define xrdp_usleep(_d) \
    do { \
        ACCESS_LOG(ACCESS_LOG_OP_SLEEP, 0, _d, 0); \
        cfe_usleep(_d); \
    } while(0)

#endif /* #ifdef _CFE_ */


#else /* #if defined(CONFIG_GPL_RDP_GEN) */

extern int access_log_restore(const access_log_tuple_t *entry_array);

#endif /* #if defined(CONFIG_GPL_RDP_GEN) */

#endif /* #if defined(CONFIG_GPL_RDP_GEN) || defined(CONFIG_GPL_RDP) */

#endif /* _ACCESS_LOGGING_H_ */
