/* 
 <:copyright-BRCM:2020:DUAL/GPL:standard
 
    Copyright (c) 2020 Broadcom 
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

#ifndef _BCM_UTIL_FUNC_H
#define _BCM_UTIL_FUNC_H

struct blog_t;
typedef struct blog_t Blog_t;

#if !(defined(CONFIG_BRCM_QEMU) && (defined(CONFIG_BCM96878) || defined(CONFIG_BCM96855)))

#if defined(RDP_SIM)
static inline int __isEnetWanPort(Blog_t *blog_p, int is_rx)
{
    return rdpa_blog_is_wan_port(blog_p, is_rx);
}
#else
static inline int __isEnetWanPort(Blog_t *blog_p, int is_rx)
{
    int is_wan_port = FALSE;
#if !defined(CONFIG_BCM_ENET_SYSPORT)
    bcmFun_t *enet_is_wan_port_fun = bcmFun_get(BCM_FUN_ID_ENET_IS_WAN_PORT);
    void *ctx = NULL;

#ifdef CONFIG_BCM_PON
    ctx = is_rx ? (void *)blog_p->rx_dev_p : (void *)blog_p->tx_dev_p;
#else
    uint32_t logical_port = is_rx ? blog_p->rx.info.channel : blog_p->tx.info.channel;
    ctx = &logical_port;
#endif

    BCM_ASSERT(enet_is_wan_port_fun != NULL);
    is_wan_port = enet_is_wan_port_fun(ctx);
#endif
    return is_wan_port;
}

#endif

#else

static inline int __isEnetWanPort(Blog_t *blog_p, int is_rx)
{
    return FALSE;
}

#endif /* ! CONFIG_BRCM_QEMU */

#define __isRxEnetWanPort(b) __isEnetWanPort(b, 1)
#define __isTxEnetWanPort(b) __isEnetWanPort(b, 0)

#endif /* _BCM_UTIL_FUNC_H */
