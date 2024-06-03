/*---------------------------------------------------------------------------

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
 ------------------------------------------------------------------------- */
/**
 * \file HostCommon.c
 * \brief Common host/embedded communication protocol objects
 *
 */

#include "HostCommon.h"


void hm_mailbox_to_message(hm_mailbox mailbox, hm_message *msg)
{
    msg->sync =     GET_FIELD(mailbox, HM, MSG, SYNC);
    msg->status =   GET_FIELD(mailbox, HM, MSG, STAT);
    msg->seq =      GET_FIELD(mailbox, HM, MSG, SEQ);
    msg->id =       GET_FIELD(mailbox, HM, MSG, ID);
    msg->value =    GET_FIELD(mailbox, HM, MSG, DATA);
}


void hm_message_to_mailbox(const hm_message *msg, hm_mailbox *mailbox)
{
    SET_FIELD(*mailbox, HM, MSG, SYNC, (uint32_t)msg->sync);
    SET_FIELD(*mailbox, HM, MSG, STAT, (uint32_t)msg->status);
    SET_FIELD(*mailbox, HM, MSG, SEQ, (uint32_t)msg->seq);
    SET_FIELD(*mailbox, HM, MSG, ID, (uint32_t)msg->id);
    SET_FIELD(*mailbox, HM, MSG, DATA, (uint32_t)msg->value);
}


/* End of file HostCommon.c */
