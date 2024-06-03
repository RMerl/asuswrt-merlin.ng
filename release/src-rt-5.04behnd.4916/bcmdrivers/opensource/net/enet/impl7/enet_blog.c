/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom 
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

#include "enet.h"
#include "bcmenet_common.h"

enetx_port_t *blog_chnl_to_port[BLOG_EGPHY_CHNL_MAX];

void blog_chnl_unit_port_set(enetx_port_t *p)
{
    int unit = PORT_ON_ROOT_SW(p) ? 0 : 1;
    int port = p->port_info.port;
    int blog_idx = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);

    blog_chnl_set(BLOG_ENETPHY, blog_idx, blog_idx, p);
}

void blog_chnl_unit_port_unset(enetx_port_t *p)
{
    int unit = PORT_ON_ROOT_SW(p) ? 0 : 1;
    int port = p->port_info.port;
    int blog_idx = PHYSICAL_PORT_TO_LOGICAL_PORT(port, unit);

    blog_chnl_set(BLOG_ENETPHY, blog_idx, blog_idx, NULL);
}

void blog_chnl_with_mark_set(int blog_phy, enetx_port_t *p)
{
   p->n.blog_phy = blog_phy;
   p->n.set_channel_in_mark = 1;
}

static void _blog_chnl_set(int blog_phy, int blog_channel, int blog_channel_tx, enetx_port_t *p)
{
    p->n.blog_phy = blog_phy;
    p->n.set_channel_in_mark = 0;
    p->n.blog_chnl_rx = blog_channel;
    p->n.blog_chnl = blog_channel_tx;
}

void blog_chnl_set(int blog_phy, int blog_channel, int blog_channel_tx, enetx_port_t *p)
{
    if (blog_phy != BLOG_ENETPHY)
    {
        _blog_chnl_set(blog_phy, blog_channel, blog_channel_tx, p);
        return;
    }

    if (blog_channel >= BLOG_EGPHY_CHNL_MAX)
    {
        printk("Blog channel %d larger than BLOG_EGPHY_CHNL_MAX %d for port %s\n", blog_channel, BLOG_EGPHY_CHNL_MAX, p->obj_name);
        BUG();
    }

    if (p)
    {
        if (blog_chnl_to_port[blog_channel])
        {
            printk("Failed to register Blog channel %d for port %s, used by port %s\n", blog_channel, p->obj_name, blog_chnl_to_port[blog_channel]->obj_name);
            BUG();
        }

        _blog_chnl_set(blog_phy, blog_channel, blog_channel_tx, p);
    }
    enet_dbgv("blog_phy=%d, blog_chnl=%x\n", blog_phy, blog_channel);

    blog_chnl_to_port[blog_channel] = p;
}

