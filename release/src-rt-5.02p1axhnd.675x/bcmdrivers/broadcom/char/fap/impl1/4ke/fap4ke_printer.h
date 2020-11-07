#ifndef __FAP4KE_PRINTER_H_INCLUDED__
#define __FAP4KE_PRINTER_H_INCLUDED__

/*
 <:copyright-BRCM:2009:DUAL/GPL:standard
 
    Copyright (c) 2009 Broadcom 
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
 *******************************************************************************
 * File Name  : fap4ke_printer.h
 *
 * Description: This file contains the implementation of the FAP print support.
 *
 *******************************************************************************
 */

#include "fap4keLib_snprintf.h"
#ifndef FAP_4KE
#define fapMailBox_4kePrint(fmt, arg...)
#endif /* FAP_4KE */
#include "fap4ke_colors.h"

#define CC_FAP4KE_PRINT_DEBUG
#define CC_FAP4KE_PRINT_INFO
#define CC_FAP4KE_PRINT_NOTICE
#define CC_FAP4KE_PRINT_ERROR

#define fap4kePrt_Print(fmt, arg...)                            \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_PRINT, fmt, ##arg)

#if defined(CC_FAP4KE_PRINT_DEBUG)
#define fap4kePrt_Debug(fmt, arg...)                                  \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_DEBUG, CLRm "%s: " fmt CLRnorm, __FUNCTION__, ##arg)
#else
#define fap4kePrt_Debug(fmt, arg...)
#endif

#if defined(CC_FAP4KE_PRINT_INFO)
#define fap4kePrt_Info(fmt, arg...)                                   \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_INFO, CLRg "%s: " fmt CLRnorm, __FUNCTION__, ##arg)
#else
#define fap4kePrt_Info(fmt, arg...)
#endif

#if defined(CC_FAP4KE_PRINT_NOTICE)
#define fap4kePrt_Notice(fmt, arg...)                                 \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_NOTICE, CLRb "%s: " fmt CLRnorm, __FUNCTION__, ##arg)
#else
#define fap4kePrt_Notice(fmt, arg...)
#endif

#if defined(CC_FAP4KE_PRINT_ERROR)
#define fap4kePrt_Error(fmt, arg...)                                  \
    fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_ERROR, CLRerr "%s,%d: " fmt CLRnorm, __FUNCTION__, __LINE__, ##arg)
#else
#define fap4kePrt_Error(fmt, arg...)
#endif

#define fap4kePrt_Assert(_condition)                                    \
    do {                                                                \
        if(!(_condition)) {                                             \
            fapMailBox_4kePrint(FAP_MAILBOX_MSGID_LOG_ASSERT, CLRerr "%s,%d: " #_condition CLRnl, \
                                __FUNCTION__, __LINE__);      \
        }                                                               \
    } while(0)

#endif  /* defined(__FAP4KE_PRINTER_H_INCLUDED__) */
