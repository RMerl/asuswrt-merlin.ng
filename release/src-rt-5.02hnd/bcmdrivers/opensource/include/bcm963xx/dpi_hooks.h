#ifndef __DPI_HOOKS_H_INCLUDED__
#define __DPI_HOOKS_H_INCLUDED__
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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
#include <bcmdpi.h>

typedef struct {
    int (* nl_handler)(struct sk_buff *skb);
    int (* cpu_enqueue)(pNBuff_t pNBuff, struct net_device *dev);
} dpi_hooks_t;

int dpi_bind(dpi_hooks_t *hooks_p);

void dpi_info_get(void *conn_p, unsigned int *app_id_p, uint16_t *dev_key_p);
void dpi_blog_key_get(void *conn_p, unsigned int *blog_key_0_p, unsigned int *blog_key_1_p);

void dpi_nl_msg_reply(struct sk_buff *skb, DpictlNlMsgType_t msgType,
                      unsigned short msgLen, void *msgData_p);

int dpi_cpu_enqueue(pNBuff_t pNBuff, struct net_device *dev);

#endif /* __DPI_HOOKS_H_INCLUDED__ */
