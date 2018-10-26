/* 
    Copyright 2000-2015 Broadcom Corporation

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
    
    :>

*/

/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *
    *  Main Module                              File: bcm63xx_ipc.c
    *
    *  This module contains the various "C" rudimentary implementation 
    *  of mailbox functionality .
    *
    *  Author: Igor Kakhaia (ikakhaia@broadcom.com)
    *
*/

#include "cfe.h"
#include "bcm63xx_ipc.h"

int cfe_mailbox_status_isset(void)
{
    return  ((CFE_MAILBOX_STATUS&CFE_MAILBOX_STATUS_MASK)>>
                  CFE_MAILBOX_STATUS_SHIFT) == CFE_MAILBOX_STATUS_VAL;
}

void cfe_mailbox_status_set(void)
{
    CFE_MAILBOX_STATUS = 
      (CFE_MAILBOX_STATUS&(~CFE_MAILBOX_STATUS_MASK))|(CFE_MAILBOX_STATUS_VAL << CFE_MAILBOX_STATUS_SHIFT);
}
/*
* Get 32 bit composite message
*/
unsigned int cfe_mailbox_message_get(void)
{
    return  (CFE_MAILBOX_MSG & CFE_MAILBOX_MSG_MASK);
}

void cfe_mailbox_message_set(unsigned int val)
{
    static int init = 0;

    if (! init ) {
        CFE_MAILBOX_MSG = 0;
        init = 1;
    }

    CFE_MAILBOX_MSG |= val & CFE_MAILBOX_MSG_MASK;
}
/*
* Retreives embedded API version (not sources' major/minor)
*
*/
unsigned int cfe_get_api_version(void)
{
    return CFE_API_VERSION;
}
