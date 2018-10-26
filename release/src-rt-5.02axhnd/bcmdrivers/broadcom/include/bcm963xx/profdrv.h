/*
<:copyright-BRCM:2004:DUAL/GPL:standard

   Copyright (c) 2004 Broadcom 
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
/*****************************************************************************
*
*    Filename: profdrv.h
*
****************************************************************************
*    Description:
*
*      This file contains the profiler device driver
*
****************************************************************************/
#ifndef PROFDRV_DEVICE_DRIVER__H__INCLUDED
#define PROFDRV_DEVICE_DRIVER__H__INCLUDED

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROFILER_NAME_MAX_LENGTH                32
#define PROFILER_MAX_RECSEQ                     2048

#define PROFILER_CPU_UTIL_VALID_START        0x01
#define PROFILER_CPU_UTIL_VALID_STOP         0x02

#define PROFILER_SOURCE_USER              0x00
#define PROFILER_SOURCE_KERNEL               0x01

#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

#define profdrv_read_32bit_cp0_register(source)                \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
    ".set\tpush\n\t"                    \
    ".set\treorder\n\t"                 \
        "mfc0\t%0,"STR(source)"\n\t"                            \
    ".set\tpop"                     \
        : "=r" (__res));                                        \
        __res;})

/*
    Structure used to pass profiling information data at the user/kernel interface.
*/
typedef struct
{
    char name[PROFILER_NAME_MAX_LENGTH];

} PROFILER_IOCTL_DATA;

/*
    This structure is used to keep track of the CPU utilization during the profiling period.
*/
typedef struct
{
    unsigned tick_uptime_start;
    unsigned tick_idle_start;
    unsigned tick_user_start;
    unsigned tick_kernel_start;
    unsigned tick_uptime_stop;
    unsigned tick_idle_stop;
    unsigned tick_user_stop;
    unsigned tick_kernel_stop;
   unsigned char valid_data;

} PROFILER_CPU_UTILIZATION;

/*
   A generic structure to pass information about the profiler status.
*/
typedef struct
{
   unsigned status;
   unsigned cpu_jiffies_start;
   unsigned cpu_jiffies_stop;
   unsigned cpu_jiffies_factor;
   unsigned cpu_clock;

} PROFILER_STATUS;

/*
    The ioctl action index.
*/
typedef enum
{
   PROFILER_IOCTL_GET_DATA_DUMP_INDEX,
   PROFILER_IOCTL_GET_RECSEQ_DATA_DUMP_INDEX,
   PROFILER_IOCTL_SET_DATA_CLEAN_INDEX,
   PROFILER_IOCTL_SET_PROF_OPS_INDEX,
   PROFILER_IOCTL_REGISTER_CALL_INDEX,
   PROFILER_IOCTL_DEREGISTER_CALL_INDEX,
   PROFILER_IOCTL_START_CALL_INDEX,
   PROFILER_IOCTL_STOP_CALL_INDEX,
   PROFILER_IOCTL_PROFILER_STATUS_INDEX,
   PROFILER_IOCTL_SET_CPU_UTIL_INDEX,
   PROFILER_IOCTL_GET_CPU_UTIL_INDEX,
   PROFILER_IOCTL_GET_RECSEQ_DI_INDEX

} PROFILER_IOCTL_INDEX;

/*
    Assigning a device driver major number for the sake of making this application work
*/
#define PROFDRV_DEVICE_DRIVER_MAJOR     3030

#define PROFILER_IOCTL_GET_DATA_DUMP \
    _IOR( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_GET_DATA_DUMP_INDEX, unsigned )
#define PROFILER_IOCTL_GET_RECSEQ_DATA_DUMP \
    _IOR( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_GET_RECSEQ_DATA_DUMP_INDEX, unsigned )
#define PROFILER_IOCTL_SET_DATA_CLEAN \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_SET_DATA_CLEAN_INDEX, unsigned )
#define PROFILER_IOCTL_SET_PROF_OPS \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_SET_PROF_OPS_INDEX, unsigned )
#define PROFILER_IOCTL_REGISTER_CALL \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_REGISTER_CALL_INDEX, PROFILER_IOCTL_DATA )
#define PROFILER_IOCTL_DEREGISTER_CALL \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_DEREGISTER_CALL_INDEX, PROFILER_IOCTL_DATA )
#define PROFILER_IOCTL_START_CALL \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_START_CALL_INDEX, PROFILER_IOCTL_DATA )
#define PROFILER_IOCTL_STOP_CALL \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_STOP_CALL_INDEX, PROFILER_IOCTL_DATA )
#define PROFILER_IOCTL_PROFILER_STATUS_DATA \
    _IOR( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_PROFILER_STATUS_INDEX, PROFILER_STATUS )
#define PROFILER_IOCTL_SET_CPU_UTIL \
    _IOW( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_SET_CPU_UTIL_INDEX, PROFILER_CPU_UTILIZATION )
#define PROFILER_IOCTL_GET_CPU_UTIL \
    _IOR( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_GET_CPU_UTIL_INDEX, PROFILER_CPU_UTILIZATION )
#define PROFILER_IOCTL_GET_RECSEQ_DATA_INDEX \
    _IOR( PROFDRV_DEVICE_DRIVER_MAJOR, PROFILER_IOCTL_GET_RECSEQ_DI_INDEX, unsigned )

#ifdef __cplusplus
}
#endif

#endif /* PROFDRV_DEVICE_DRIVER__H__INCLUDED */
