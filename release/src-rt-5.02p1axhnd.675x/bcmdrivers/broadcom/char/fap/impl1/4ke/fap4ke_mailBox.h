#ifndef __FAP4KE_MAILBOX_H_INCLUDED__
#define __FAP4KE_MAILBOX_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
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
 * File Name  : fap4ke_MailBox.h
 *
 * Description: This file contains global definitions and API of the 63268 FAP
 *              Message FIFOs.
 *
 *******************************************************************************
 */

#include <stdarg.h>

#define FAP_MAILBOX_PRINTBUF_SIZE 2048 /* bytes */

typedef enum {
    FAP_MAILBOX_MSGID_SUCCESS,
    FAP_MAILBOX_MSGID_ERROR,
    FAP_MAILBOX_MSGID_PRINT,
    FAP_MAILBOX_MSGID_LOG_ASSERT,
    FAP_MAILBOX_MSGID_LOG_ERROR,
    FAP_MAILBOX_MSGID_LOG_NOTICE,
    FAP_MAILBOX_MSGID_LOG_INFO,
    FAP_MAILBOX_MSGID_LOG_DEBUG,
    FAP_MAILBOX_MSGID_KEEPALIVE,
    FAP_MAILBOX_MSGID_WORD_HI,
    FAP_MAILBOX_MSGID_WORD_LO,
    FAP_MAILBOX_MSGID_SYSRQ,
    FAP_MAILBOX_MSGID_MAX
} fapMailBox_msgId_t;

typedef union {
    struct {
        uint32 msgId : 16;
        uint32 data  : 16;
    };
    uint32 u32;
} fapMailBox_msg_t;


/*******************************************************************
 * FAP API
 *******************************************************************/

extern char fapMailBox_printBuffer[FAP_MAILBOX_PRINTBUF_SIZE];

/* #define fapMailBox_4kePrint(_msgId, fmt, arg...)                        \ */
/*     do {                                                                \ */
/*         uint32 _flags;                                                  \ */
/*         fapMailBox_msg_t _msg;                                          \ */
/*         FAP4KE_LOCK(_flags);                                            \ */
/*         fap4ke_snprintf(fapMailBox_printBuffer, FAP_MAILBOX_PRINTBUF_SIZE, \ */
/*                         fmt, ##arg);                                    \ */
/*         _msg.msgId = (_msgId);                                          \ */
/*         _msg.data = ++fapMailBox_4kePrintCount;                         \ */
/*         fapToHost_xmitAndWaitForAck(_msg);                              \ */
/*         FAP4KE_UNLOCK(_flags);                                          \ */
/*     } while(0) */

void fapMailBox_4kePrint(fapMailBox_msgId_t msgId, const char *templat, ...);
void fapMailBox_4kePrintk(const char *templat, ...);
void fapMailBox_4keSendKeepAlive(void);
void fapMailBox_4keSendWord(uint32 val32);
void fapMailBox_4keInit(void);

#endif  /* defined(__FAP4KE_MAILBOX_H_INCLUDED__) */
