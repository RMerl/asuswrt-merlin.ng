/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
