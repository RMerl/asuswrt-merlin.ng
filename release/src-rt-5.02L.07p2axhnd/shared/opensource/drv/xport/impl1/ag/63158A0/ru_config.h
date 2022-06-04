/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

:>*/

/**
 * \brief Register Utility configuration and common includes
 */
#ifndef _RU_CONFIG_H_
#define _RU_CONFIG_H_


/******************************************************************************
 * Optimizations
 ******************************************************************************/

/* Register access bypasses function interface and error checking instead
   generating register accesses as inline memory dereference. This must be used 
   with RU_EXTERNAL_REGISTER_ADDRESSING and RU_OFFLINE_TEST disabled. */
#ifndef RU_FUNCTION_REG_ACCESS
#define RU_FUNCTION_REG_ACCESS 0
#endif

/* Remove field bounds checking from RU_FIELD_SET.  This may be disabled once
   software has stabilized and bounds checking begins to hurt performance. */
#ifndef RU_FIELD_CHECK_ENABLE
#define RU_FIELD_CHECK_ENABLE 1
#endif

/* Include title and description strings from RBD.  Note, this will take up a
   very large amount of RAM on bigger systems. */ 
#ifndef RU_INCLUDE_DESC
#define RU_INCLUDE_DESC 0
#endif

/* Include field defitions in the register database */
#ifndef RU_INCLUDE_FIELD_DB
#define RU_INCLUDE_FIELD_DB 1
#endif

/* Include read/write access field in database */
#ifndef RU_INCLUDE_ACCESS
#define RU_INCLUDE_ACCESS 1
#endif

/******************************************************************************
 * Logging
 ******************************************************************************/

/* Number of register accesses in the register logging ring buffer */
#ifndef RU_LOG_SIZE
#define RU_LOG_SIZE 65536
#endif

/* Number of unique logging rules that may be used at any time */
#ifndef RU_RULE_POOL_SIZE
#define RU_RULE_POOL_SIZE 256
#endif

/******************************************************************************
 * Print and error
 ******************************************************************************/

/* Optional assert function to call when field bounds checking fails or logging
   rule allocation fails.  Print function will also be called when this 
   occurs. */
#define RU_ASSERT()

/* Print function to use for register logging output. */
#ifdef _CFE_
#define RU_PRINT(...) xprintf(__VA_ARGS__)
#else
#define RU_PRINT(...) pr_info(__VA_ARGS__)
#endif

/******************************************************************************
 * Test and miscellaneous
 ******************************************************************************/

/* Registers are directly accessed as pointers to RAM using base and offset
   calculations from the block and register records.  If register access is 
   redirected over another bus set this to 0 and implement new versions of
   ru_reg_write and ru_reg_read. */
#ifndef RU_EXTERNAL_REGISTER_ADDRESSING
#define RU_EXTERNAL_REGISTER_ADDRESSING 0
#endif

/* Register access is redirected to an internal buffer for testing functions
   when hardware access is not available. It is only meaningful if 
   RU_DIRECT_REGISTER_ADDRESSING is enabled. */
#ifndef RU_OFFLINE_TEST
#define RU_OFFLINE_TEST 0
#endif

/* Set this to 1 to include a stub main function for simple compile checks */
#ifndef RU_TEST_COMPILE_STUB
#define RU_TEST_COMPILE_STUB 0
#endif

#endif /* End of file _RU_CONFIG_H_ */

