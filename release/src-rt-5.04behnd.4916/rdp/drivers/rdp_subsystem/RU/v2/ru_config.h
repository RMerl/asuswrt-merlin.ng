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

/**
 * $Id: //xpon_co_tools/cur/hal_generator/libru/ru_config.h#11 $
 * $Date: 2020/03/30 $
 */

/**
 * \brief Register Utility configuration and common includes
 */
#ifndef _RU_CONIFIG_H_
#define _RU_CONIFIG_H_

/******************************************************************************
 * Environment-specific includes
 ******************************************************************************/
#if RU_CUSTOM_INCLUDE
#include "ru_custom.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#endif

/******************************************************************************
 * Environment-specific macros
 ******************************************************************************/

/* Optional assert function to call when field bounds checking fails or logging
   rule allocation fails.  Print function will also be called when this
   occurs. */
#ifndef RU_ASSERT
#define RU_ASSERT()
#endif

/* Macros to lock/unlock fastlock for RU logger (for thread safety).
   RU_FASTLOCK_LOCK returns an unsigned long.
   RU_FASTLOCK_UNLOCK accepts an unsigned long and returns void. */
#ifndef RU_FASTLOCK_LOCK
#define RU_FASTLOCK_LOCK() 0
#endif
#ifndef RU_FASTLOCK_UNLOCK
#define RU_FASTLOCK_UNLOCK(flags) (void)flags
#endif

/* Get the current 32-bit system timestamp in microseconds. */
#ifndef RU_TIMESTAMP_US
static inline uint32_t ru_timestamp_us_posix(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (1000000 * tv.tv_sec) + tv.tv_usec;
}
#define RU_TIMESTAMP_US ru_timestamp_us_posix
#endif

/* Get the index of the currently running chip revision (if multiple chip revisions are supported). */
#ifndef RU_CHIP_REV_IDX_GET
#define RU_CHIP_REV_IDX_GET() 0
#endif

/* Get the name of a chip revision (e.g. "A0", "B0") given its index. */
#ifndef RU_CHIP_REV_NAME_GET
#define RU_CHIP_REV_NAME_GET(idx) ""
#endif

/* If set, READ32 is assumed to take an additional argument with the name of a
   variable to load the result into, rather than just returning the result.
   If this is set, RU_REG_READ and RU_REG_RAM_READ will also take an additional
   variable. */
#ifndef RU_READ32_IN_PLACE
#define RU_READ32_IN_PLACE 0
#endif

/******************************************************************************
 * Optimizations
 ******************************************************************************/

/* Register access bypasses function interface and error checking instead
   generating register accesses as inline memory dereference. */
#ifndef RU_FUNCTION_REG_ACCESS
#define RU_FUNCTION_REG_ACCESS 1
#endif

/* Remove field bounds checking from RU_FIELD_SET.  This may be disabled once
   software has stabilized and bounds checking begins to hurt performance. */
#ifndef RU_FIELD_CHECK_ENABLE
#define RU_FIELD_CHECK_ENABLE 1
#endif

/* Include title and description strings from RDB.  Note, this will take up a
   very large amount of RAM on bigger systems. */
#ifndef RU_INCLUDE_DESC
#define RU_INCLUDE_DESC 1
#endif

/* Include field defitions in the register database */
#ifndef RU_INCLUDE_FIELD_DB
#define RU_INCLUDE_FIELD_DB 1
#endif

/* Include read/write access field in database */
#ifndef RU_INCLUDE_ACCESS
#define RU_INCLUDE_ACCESS 1
#endif

/* Number of chip revisions */
#ifndef RU_CHIP_REV_COUNT
#define RU_CHIP_REV_COUNT 1
#endif

/******************************************************************************
 * Logging/printing
 ******************************************************************************/

/* Number of register accesses in the register logging ring buffer */
#ifndef RU_LOG_SIZE
#define RU_LOG_SIZE 65536
#endif

/* Number of unique logging rules that may be used at any time */
#ifndef RU_RULE_POOL_SIZE
#define RU_RULE_POOL_SIZE 0
#endif

/* Function to print a string with printf-style arguments */
#ifndef RU_PRINT
#define RU_PRINT(...) printf(__VA_ARGS__)
#endif

/******************************************************************************
 * Test and miscellaneous
 ******************************************************************************/

/* Build for "simulation" mode as opposed to live chip mode. This enables
   a RAM scratchpad for all registers that can be manipulated offline, as well
   as options to perform reads/writes on an external server via UDP. */
#ifndef RU_SIMULATION
#define RU_SIMULATION 1
#endif

/* Set this to 1 in simulation builds to print to stdout on read/write. */
#ifndef RU_SIMULATION_ACCESS_PRINT
#define RU_SIMULATION_ACCESS_PRINT 1
#endif

/* Registers are directly accessed as pointers to RAM using base and offset
   calculations from the block and register records.  If register access is
   redirected over another bus set this to 0 and implement new versions of
   ru_reg_write and ru_reg_read. */
#ifndef RU_EXTERNAL_REGISTER_ADDRESSING
#define RU_EXTERNAL_REGISTER_ADDRESSING 0
#endif

/* always match rule */
#ifndef RU_RULE_ALWAYS_MATCH
#define RU_RULE_ALWAYS_MATCH 1
#endif

/* Set this to 1 to include a stub main function for simple compile checks */
#ifndef RU_TEST_COMPILE_STUB
#define RU_TEST_COMPILE_STUB 0
#endif

#endif /* End of file ru_config.h */
